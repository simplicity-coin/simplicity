// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Bitcoin developers
// Copyright (c) 2015-2018 The PIVX developers
// Copyright (c) 2018-2019 The Simplicity developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_PRIMITIVES_BLOCK_H
#define BITCOIN_PRIMITIVES_BLOCK_H

#include "primitives/transaction.h"
#include "keystore.h"
#include "serialize.h"
#include "uint256.h"

/** The maximum allowed size for a serialized block, in bytes (network rule) */
static const unsigned int MAX_BLOCK_SIZE_CURRENT = 100000000;
static const unsigned int MAX_BLOCK_SIZE_LEGACY = 85673194;

enum BlockType {
    POS = 0,
    POW_QUARK = 1,
    POW_SCRYPT_SQUARED = 2,
    ALGO_COUNT
};

enum AlgoFlags {
    ALGO_POS = 1<<29,
    ALGO_POW_QUARK = 2<<29,
    ALGO_POW_SCRYPT_SQUARED = 3<<29,
    ALGO_VERSION = 7<<29,
    ALGO_POS_VERSION = ALGO_POS,
    ALGO_POW_VERSION = 6<<29
};

/** Nodes collect new transactions into a block, hash them into a hash tree,
 * and scan through nonce values to make the block's hash satisfy proof-of-work
 * requirements.  When they solve the proof-of-work, they broadcast the block
 * to everyone and the block is added to the block chain.  The first transaction
 * in the block is a special one that creates a new coin owned by the creator
 * of the block.
 */
class CBlockHeader
{
public:
    // header
    static const int32_t CURRENT_VERSION=8;
    uint32_t nVersion;
    uint256 hashPrevBlock;
    uint256 hashMerkleRoot;
    uint32_t nTime;
    uint32_t nBits;
    uint32_t nNonce;
    //uint256 nAccumulatorCheckpoint;

    // memory only
    bool fPreForkPoS = false;

    CBlockHeader()
    {
        SetNull();
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(this->nVersion);
        nVersion = this->nVersion;
        READWRITE(hashPrevBlock);
        READWRITE(hashMerkleRoot);
        READWRITE(nTime);
        READWRITE(nBits);
        READWRITE(nNonce);

        //zerocoin active, header changes to include accumulator checksum
        //if (nVersion > 19)
            //READWRITE(nAccumulatorCheckpoint);
    }

    void SetNull()
    {
        nVersion = CBlockHeader::CURRENT_VERSION;
        hashPrevBlock.SetNull();
        hashMerkleRoot.SetNull();
        nTime = 0;
        nBits = 0;
        nNonce = 0;
        fPreForkPoS = false;
        //nAccumulatorCheckpoint = 0;
    }

    bool IsNull() const
    {
        return (nBits == 0);
    }

    // ppcoin: two types of block: proof-of-work or proof-of-stake
    bool IsProofOfStake() const
    {
        //return (vtx.size() > 1 && vtx[1].IsCoinStake());
        return (nVersion & ALGO_VERSION) == ALGO_POS_VERSION || (nVersion < CBlockHeader::CURRENT_VERSION && fPreForkPoS);
    }

    bool IsProofOfWork() const
    {
        return (nVersion & ALGO_POW_VERSION) || (nVersion < CBlockHeader::CURRENT_VERSION && !fPreForkPoS);
    }

    static int GetAlgo(int version)
    {
        switch (version & ALGO_VERSION) {
            case ALGO_POS:
                return POS;
            case ALGO_POW_QUARK:
                return POW_QUARK;
            case ALGO_POW_SCRYPT_SQUARED:
                return POW_SCRYPT_SQUARED;
            default:
                return -1;
        }
    }

    static int GetVer(int algo)
    {
        switch (algo) {
            case POS:
                return ALGO_POS;
            case POW_QUARK:
                return ALGO_POW_QUARK;
            case POW_SCRYPT_SQUARED:
                return ALGO_POW_SCRYPT_SQUARED;
            default:
                return CBlockHeader::CURRENT_VERSION;
        }
    }

    uint256 GetPoWHash() const;
    uint256 GetHash() const;

    int64_t GetBlockTime() const
    {
        return (int64_t)nTime;
    }
};


class CBlock : public CBlockHeader
{
public:
    // network and disk
    std::vector<CTransaction> vtx;

    // ppcoin: block signature - signed by one of the coin base txout[N]'s owner
    std::vector<unsigned char> vchBlockSig;

    // memory only
    mutable CScript payee;
    mutable std::vector<uint256> vMerkleTree;

    CBlock()
    {
        SetNull();
    }

    CBlock(const CBlockHeader &header)
    {
        SetNull();
        *((CBlockHeader*)this) = header;
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(*(CBlockHeader*)this);
        READWRITE(vtx);
        if (vtx.size() > 1 && vtx[1].IsCoinStake())
            READWRITE(vchBlockSig);
    }

    void SetNull()
    {
        CBlockHeader::SetNull();
        vtx.clear();
        vMerkleTree.clear();
        payee = CScript();
        vchBlockSig.clear();
    }

    CBlockHeader GetBlockHeader() const
    {
        CBlockHeader block;
        block.nVersion       = nVersion;
        block.hashPrevBlock  = hashPrevBlock;
        block.hashMerkleRoot = hashMerkleRoot;
        block.nTime          = nTime;
        block.nBits          = nBits;
        block.nNonce         = nNonce;
        block.fPreForkPoS    = fPreForkPoS;
        //block.nAccumulatorCheckpoint = nAccumulatorCheckpoint;
        return block;
    }

    bool IsZerocoinStake() const;

    std::pair<COutPoint, unsigned int> GetProofOfStake() const
    {
        return IsProofOfStake() ? std::make_pair(vtx[1].vin[0].prevout, nTime) : std::make_pair(COutPoint(), (unsigned int)0);
    }

    // Build the in-memory merkle tree for this block and return the merkle root.
    // If non-NULL, *mutated is set to whether mutation was detected in the merkle
    // tree (a duplication of transactions in the block leading to an identical
    // merkle root).
    uint256 BuildMerkleTree(bool* mutated = NULL) const;

    std::vector<uint256> GetMerkleBranch(int nIndex) const;
    static uint256 CheckMerkleBranch(uint256 hash, const std::vector<uint256>& vMerkleBranch, int nIndex);
    std::string ToString() const;
    void print() const;
};


/** Describes a place in the block chain to another node such that if the
 * other node doesn't have the same branch, it can find a recent common trunk.
 * The further back it is, the further before the fork it may be.
 */
struct CBlockLocator
{
    std::vector<uint256> vHave;

    CBlockLocator() {}

    CBlockLocator(const std::vector<uint256>& vHaveIn)
    {
        vHave = vHaveIn;
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        if (!(nType & SER_GETHASH))
            READWRITE(nVersion);
        READWRITE(vHave);
    }

    void SetNull()
    {
        vHave.clear();
    }

    bool IsNull()
    {
        return vHave.empty();
    }
};

#endif // BITCOIN_PRIMITIVES_BLOCK_H
