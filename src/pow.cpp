// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2018 The PIVX developers
// Copyright (c) 2018-2019 The Simplicity developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "pow.h"

#include "chain.h"
#include "chainparams.h"
#include "main.h"
#include "primitives/block.h"
#include "uint256.h"
#include "util.h"

#include <math.h>

const CBlockIndex* GetLastBlockIndex(const CBlockIndex* pindex, bool fProofOfStake)
{
    while (pindex && pindex->pprev && (pindex->IsProofOfStake() != fProofOfStake))
        pindex = pindex->pprev;
    return pindex;
}

const CBlockIndex* GetLastBlockIndex(const CBlockIndex* pindex, int algo)
{
    bool newDiff = algo == POW_SCRYPT_SQUARED && pindex->nTime >= Params().BadScryptDiffTimeEnd();
    while (pindex && pindex->pprev && (CBlockHeader::GetAlgo(pindex->nVersion) != algo || (newDiff && pindex->nTime < Params().BadScryptDiffTimeEnd() && pindex->nTime >= Params().BadScryptDiffTimeStart())))
        pindex = pindex->pprev;
    return pindex;
}

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader* pblock, bool fProofOfStake)
{
    uint256 bnTargetLimit = fProofOfStake ? Params().ProofOfStakeLimit() : Params().ProofOfWorkLimit(pblock->nVersion >= Params().WALLET_UPGRADE_VERSION() ? CBlockHeader::GetAlgo(pblock->nVersion) : POW_QUARK);
    //if (Params().NetworkID() != CBaseChainParams::MAIN && !fProofOfStake) return bnTargetLimit.GetCompact(); // for testing

    if (pindexLast == NULL)
        return bnTargetLimit.GetCompact(); // genesis block

    const CBlockIndex* pindexPrev = pblock->nVersion >= Params().WALLET_UPGRADE_VERSION() ? GetLastBlockIndex(pindexLast, CBlockHeader::GetAlgo(pblock->nVersion)) : GetLastBlockIndex(pindexLast, fProofOfStake);
    if (pindexPrev->pprev == NULL)
        return bnTargetLimit.GetCompact(); // first block

    const CBlockIndex* pindexPrevPrev = pblock->nVersion >= Params().WALLET_UPGRADE_VERSION() ? GetLastBlockIndex(pindexPrev->pprev, CBlockHeader::GetAlgo(pblock->nVersion)) : GetLastBlockIndex(pindexPrev->pprev, fProofOfStake);
    if (pindexPrevPrev->pprev == NULL)
        return bnTargetLimit.GetCompact(); // second block

    int64_t nActualSpacing = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime(); // difficulty for PoW and PoS are calculated separately

    if (nActualSpacing <= 0)
        nActualSpacing = 1;

    // ppcoin: target change every block
    // ppcoin: retarget with exponential moving toward target spacing
    uint256 bnNew;
    bnNew.SetCompact(pindexPrev->nBits);

    if (!fProofOfStake) {
        bnNew *= ((Params().Interval() - 1) * ((ALGO_COUNT-1) * 2 * Params().TargetSpacing()) + nActualSpacing + nActualSpacing);
        bnNew /= ((Params().Interval() + 1) * ((ALGO_COUNT-1) * 2 * Params().TargetSpacing()));
    } else {
        bnNew *= ((Params().Interval() - 1) * (2 * Params().TargetSpacing()) + nActualSpacing + nActualSpacing);
        bnNew /= ((Params().Interval() + 1) * (2 * Params().TargetSpacing())); // 160 second block time for PoW + 160 second block time for PoS = 80 second effective block time
    }

    /*if (Params().NetworkID() == CBaseChainParams::MAIN) {
        int height = pindexLast->nHeight + 1;

        if (height < (Params().WALLET_UPGRADE_BLOCK()+10) && height >= Params().WALLET_UPGRADE_BLOCK())
            bnNew *= (int)pow(4.0, 10.0+Params().WALLET_UPGRADE_BLOCK()-height); // slash difficulty and gradually ramp back up over 10 blocks
    }*/

    if (bnNew <= 0 || bnNew > bnTargetLimit)
        bnNew = bnTargetLimit;

    return bnNew.GetCompact();
}

unsigned int GetLegacyNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader* pblock, bool fProofOfStake)
{
    int64_t nTargetSpacing = 80;
    int64_t nTargetTimespan = 20 * 60;

    uint256 bnTargetLimit = fProofOfStake ? Params().ProofOfStakeLimit() : Params().ProofOfWorkLimit(POW_QUARK);

    if (pindexLast == NULL)
        return bnTargetLimit.GetCompact(); // genesis block

    const CBlockIndex* pindexPrev = GetLastBlockIndex(pindexLast, fProofOfStake);
    if (pindexPrev->pprev == NULL)
        return bnTargetLimit.GetCompact(); // first block

    const CBlockIndex* pindexPrevPrev = GetLastBlockIndex(pindexPrev->pprev, fProofOfStake);
    if (pindexPrevPrev->pprev == NULL)
        return bnTargetLimit.GetCompact(); // second block

    int64_t nActualSpacing = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime();

    if (nActualSpacing < 0)
        nActualSpacing = nTargetSpacing;

    uint256 bnNew;
    bnNew.SetCompact(pindexPrev->nBits);
    int64_t nInterval = nTargetTimespan / nTargetSpacing;
    bnNew *= ((nInterval - 1) * nTargetSpacing + nActualSpacing + nActualSpacing);
    bnNew /= ((nInterval + 1) * nTargetSpacing);

    if (bnNew <= 0 || bnNew > bnTargetLimit)
        bnNew = bnTargetLimit;

    return bnNew.GetCompact();
}

bool CheckProofOfWork(const CBlockHeader* pblock)
{
    bool fNegative;
    bool fOverflow;
    uint256 bnTarget;

    if (Params().SkipProofOfWorkCheck())
        return true;

    bnTarget.SetCompact(pblock->nBits, &fNegative, &fOverflow);

    // Check range
    if (fNegative || bnTarget == 0 || fOverflow || bnTarget > Params().ProofOfWorkLimit(pblock->nVersion >= Params().WALLET_UPGRADE_VERSION() ? CBlockHeader::GetAlgo(pblock->nVersion) : POW_QUARK))
        return error("CheckProofOfWork() : nBits below minimum work");

    if (CBlockHeader::GetAlgo(pblock->nVersion) == POW_SCRYPT_SQUARED && pblock->nTime < Params().BadScryptDiffTimeEnd() && pblock->nTime >= Params().BadScryptDiffTimeStart()) {
        LogPrintf("CheckProofOfWork() : skipping block %s affected by difficulty bug\n", pblock->GetHash().GetHex());
        return true;
    }
    
    // Check proof of work matches claimed amount
    if (pblock->GetPoWHash() > bnTarget) {
        if (Params().MineBlocksOnDemand())
            return false;
        else
            return error("CheckProofOfWork() : hash doesn't match nBits");
    }

    return true;
}

uint256 GetBlockProof(const CBlockIndex& block)
{
    uint256 bnTarget;
    bool fNegative;
    bool fOverflow;
    bnTarget.SetCompact(block.nBits, &fNegative, &fOverflow);
    if (fNegative || fOverflow || bnTarget == 0)
        return 0;
    // We need to compute 2**256 / (bnTarget+1), but we can't represent 2**256
    // as it's too large for a uint256. However, as 2**256 is at least as large
    // as bnTarget+1, it is equal to ((2**256 - bnTarget - 1) / (bnTarget+1)) + 1,
    // or ~bnTarget / (nTarget+1) + 1.
    return (~bnTarget / (bnTarget + 1)) + 1;
}