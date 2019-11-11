// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2019 The PIVX developers
// Copyright (c) 2018-2019 The Simplicity developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "libzerocoin/Params.h"
#include "base58.h"
#include "chainparams.h"
#include "random.h"
#include "util.h"
#include "utilstrencodings.h"

#include <assert.h>

#include <boost/assign/list_of.hpp>
#include <limits>


struct SeedSpec6 {
    uint8_t addr[16];
    uint16_t port;
};

#include "chainparamsseeds.h"

/**
 * Main network
 */

//! Convert the pnSeeds6 array into usable address objects.
static void convertSeed6(std::vector<CAddress>& vSeedsOut, const SeedSpec6* data, unsigned int count)
{
    // It'll only connect to one or two seed nodes because once it connects,
    // it'll get a pile of addresses with newer timestamps.
    // Seed nodes are given a random 'last seen time' of between one and two
    // weeks ago.
    const int64_t nOneWeek = 7 * 24 * 60 * 60;
    for (unsigned int i = 0; i < count; i++) {
        struct in6_addr ip;
        memcpy(&ip, data[i].addr, sizeof(ip));
        CAddress addr(CService(ip, data[i].port));
        addr.nTime = GetTime() - GetRand(nOneWeek) - nOneWeek;
        vSeedsOut.push_back(addr);
    }
}

//   What makes a good checkpoint block?
// + Is surrounded by blocks with reasonable timestamps
//   (no blocks before with a timestamp after, none after with
//    timestamp before)
// + Contains no strange transactions
static Checkpoints::MapCheckpoints mapCheckpoints =
    boost::assign::map_list_of
    (0, uint256("f4bbfc518aa3622dbeb8d2818a606b82c2b8b1ac2f28553ebdb6fc04d7abaccf"))
    (50000, uint256("525c080ed904eeaaf00ac0c088c6cd0ceabe3918bebbd80ec2ed494939077965"))
    (100000, uint256("9c8f67b0d656a451250b1f4e1fca9980e23ae5eb2d70e0798b76ea4c30e63bad"))
    (150000, uint256("b9d5e51f05e23034c6da1347ca28a704ad75d2249ca5ac16e97ed9c25b261e3a"))
    (200000, uint256("505d2588728736dce8985830f0cce34cc3ae766482fba2e46ae522c98423a560"))
    (250000, uint256("a66859b5e88ef9db8194eddde917a40adfca707958d56f97d727373090152250"))
    (300000, uint256("25a1eaa7edd3e06afecb7de062d0a713d09766ffa0685997ae01ca5950b4ff61"))
    (350000, uint256("0457c2096e529aab2855d7338e77f3a3a3da9a45bb272a0d74ca35f0d234ef16"))
    (400000, uint256("02709ff59f09b08684bdab6000b7ad134f1d54ac4079202c94173378fabc44e7"))
    (450000, uint256("54fbd621e99ed213de13a273935cf29ec8b086f8498351f1bc103c43f6f356d1"))
    (500000, uint256("8c9974c78873ca6f3636c096007b90a3d23ed6f79b645a6d3e83d875c90b79f5"))
    (550000, uint256("c5f0a3c19ecfd9ccdb2818a71663e063486cd75874fe68a84755092417e4ae85"))
    (600000, uint256("5698cbf27f591da46ababc40c9ed7718086493e3542c00c35a6de86de6b6add6"))
    (650000, uint256("cf6076eda981af1097e52f505c0c3dfefc60af9b93ba674fda8e22235ff50df3"))
    (700000, uint256("2f852dfbc9b767905400c0e706ff63eca8d5e4090d4d49f873f9be1a754cc243"))
    (950000, uint256("0cbb4dfac570e6cbf7aa10b3f8a138b3dda3e908ab78e301f12354731bbce560"));
static const Checkpoints::CCheckpointData data = {
    &mapCheckpoints,
    1567628624, // * UNIX timestamp of last checkpoint block
    1558608,    // * total number of transactions between genesis and last checkpoint
                //   (the tx=... number in the SetBestChain debug.log lines)
    2000        // * estimated number of transactions per day after checkpoint
};

static Checkpoints::MapCheckpoints mapCheckpointsTestnet =
    boost::assign::map_list_of
    (0, uint256("0x001"));
static const Checkpoints::CCheckpointData dataTestnet = {
    &mapCheckpointsTestnet,
    1560843157,
    0,
    250};

static Checkpoints::MapCheckpoints mapCheckpointsRegtest =
    boost::assign::map_list_of
    (0, uint256("0x001"));
static const Checkpoints::CCheckpointData dataRegtest = {
    &mapCheckpointsRegtest,
    1454124731,
    0,
    100};

libzerocoin::ZerocoinParams* CChainParams::Zerocoin_Params(bool useModulusV1) const
{
    assert(this);
    static CBigNum bnHexModulus = 0;
    if (!bnHexModulus)
        bnHexModulus.SetHex(zerocoinModulus);
    static libzerocoin::ZerocoinParams ZCParamsHex = libzerocoin::ZerocoinParams(bnHexModulus);
    static CBigNum bnDecModulus = 0;
    if (!bnDecModulus)
        bnDecModulus.SetDec(zerocoinModulus);
    static libzerocoin::ZerocoinParams ZCParamsDec = libzerocoin::ZerocoinParams(bnDecModulus);

    if (useModulusV1)
        return &ZCParamsHex;

    return &ZCParamsDec;
}

bool CChainParams::HasStakeMinAgeOrDepth(const int contextHeight, const uint32_t contextTime,
        const int utxoFromBlockHeight, const uint32_t utxoFromBlockTime) const
{
    // before stake modifier V2, the age required was 2 * 60 * 60 (2 hours) / not required on regtest
    if (!IsStakeModifierV2(contextHeight) || contextHeight < WALLET_UPGRADE_BLOCK())
        return (NetworkID() == CBaseChainParams::REGTEST || (utxoFromBlockTime + 7200 <= contextTime));

    // after stake modifier V2, we require the utxo to be nStakeMinDepth deep in the chain
    return (contextHeight - utxoFromBlockHeight >= nStakeMinDepth);
}

std::map<CScript, int> CChainParams::GetTreasuryRewardScriptAtHeight(int nHeight) const {
    std::map<CScript, int> payees;
    CBitcoinAddress vTreasuryRewardAddress(vCommunityFundWallet);
    assert(vTreasuryRewardAddress.IsValid());

    payees.emplace(GetScriptForRawPubKey(vDevFundPubKey1), 25); //2.5%
    payees.emplace(GetScriptForRawPubKey(vDevFundPubKey2), 25); //2.5%
    payees.emplace(GetScriptForDestination(vTreasuryRewardAddress.Get()), 50); //5%

    assert(payees.size() == 3);
    return payees;
}

class CMainParams : public CChainParams
{
public:
    CMainParams()
    {
        networkID = CBaseChainParams::MAIN;
        strNetworkID = "main";
        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 4-byte int at any alignment.
         */
        pchMessageStart[0] = 0xb3;
        pchMessageStart[1] = 0x07;
        pchMessageStart[2] = 0x9a;
        pchMessageStart[3] = 0x1e;
        vAlertPubKey = ParseHex("03a728481601bb6f2e1873624fe15df816b0633b4c499406843c666800fbe45d5a");
        nDefaultPort = 11957;
        bnProofOfWorkLimit[POS] = ~uint256(0) >> 20;
        bnProofOfWorkLimit[POW_QUARK] = ~uint256(0) >> 16;
        bnProofOfWorkLimit[POW_SCRYPT_SQUARED] = ~uint256(0) >> 11;
        nMaxReorganizationDepth = 100;
        nEnforceBlockUpgradeMajority = 6075; // 75%
        nRejectBlockOutdatedMajority = 7695; // 95%
        nToCheckBlockUpgradeMajority = 8100; // Approximate expected amount of blocks in 7 days (1080*7.5)
        nMinerThreads = 0;
        nTargetTimespan = 20 * 60; // Simplicity: 20 minutes
        nTargetSpacing = 80; // Simplicity: 80 seconds
        nMaturity = 50;
        nStakeMinDepth = 600;
        nFutureTimeDriftPoW = 7200;
        nFutureTimeDriftPoS = 180;
        nMasternodeCountDrift = 20;
        nMaxMoneyOut = 7000000000 * COIN; // 7 billion

        /** Height or Time Based Activations **/
        nMandatoryUpgradeBlock = 1030000;
        nUpgradeBlockVersion = 8; //Block headers must be this version after upgrade block
        nModifierUpdateBlock = 0;
        nZerocoinStartHeight = 2100000000;
        //nZerocoinStartTime = 1508214600;
        nBlockEnforceSerialRange = 0; //Enforce serial range starting this block
        nBlockRecalculateAccumulators = nZerocoinStartHeight + 10; //Trigger a recalculation of accumulators
        nBlockFirstFraudulent = nZerocoinStartHeight; //First block that bad serials emerged
        nBlockLastGoodCheckpoint = nZerocoinStartHeight; //Last valid accumulator checkpoint
        nBlockEnforceInvalidUTXO = 0; //Start enforcing the invalid UTXO's
        nInvalidAmountFiltered = 0 * COIN; //Amount of invalid coins filtered through exchanges, that should be considered valid
        nBlockZerocoinV2 = nZerocoinStartHeight + 20; //!> The block that zerocoin v2 becomes active - roughly Tuesday, May 8, 2018 4:00:00 AM GMT
        nEnforceNewSporkKey = 1566860400; //!> Sporks signed after Monday, August 26, 2019 11:00:00 PM GMT must use the new spork key
        nRejectOldSporkKey = 1569538800; //!> Fully reject old spork key after Thursday, September 26, 2019 11:00:00 PM GMT
        nBlockStakeModifierlV2 = 0;
        // Public coin spend enforcement
        nPublicZCSpends = nZerocoinStartHeight + 30;

        nStartTreasuryBlock = nMandatoryUpgradeBlock;
        nTreasuryBlockStep = 1 * 24 * 60 * 60 / nTargetSpacing; // Once per day
        nMasternodeTiersStartHeight = 2100000000;
        vDevFundPubKey1 = CPubKey(ParseHex("03a728481601bb6f2e1873624fe15df816b0633b4c499406843c666800fbe45d5a"));
        vDevFundPubKey2 = CPubKey(ParseHex("0254121b1cbfcb42e0d53410f0db9c1c51fc79a0a376dd3e0d3c7431915f9fed44"));
        vCommunityFundWallet = "QwvSVBtpEWYcJohvLjaNgF2uKh4twzFNMy";
        assert(vDevFundPubKey1.IsFullyValid());
        assert(vDevFundPubKey2.IsFullyValid());
        //vBurnAddresses.emplace_back("");

        /**
         * Build the genesis block. Note that the output of the genesis coinbase cannot
         * be spent as it did not originally exist in the database.
         *
         * CBlock(hash=00000ffd590b14, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=e0028e, nTime=1390095618, nBits=1e0ffff0, nNonce=28917698, vtx=1)
         *   CTransaction(hash=e0028e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
         *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d01044c5957697265642030392f4a616e2f3230313420546865204772616e64204578706572696d656e7420476f6573204c6976653a204f76657273746f636b2e636f6d204973204e6f7720416363657074696e6720426974636f696e73)
         *     CTxOut(nValue=50.00000000, scriptPubKey=0xA9037BAC7050C479B121CF)
         *   vMerkleTree: e0028e
         */
        const char* pszTimestamp = "http://www.bbc.co.uk/news/world-us-canada-42926976"; // Trump Russia: Democrats say firing special counsel could cause crisis
        CMutableTransaction txNew;
        txNew.nVersion = 1;
        txNew.nTime = 1517690700;
        txNew.vin.resize(1);
        txNew.vout.resize(1);
        txNew.vin[0].scriptSig = CScript() << 4867816 << CScriptNum(42) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
        txNew.vout[0].SetEmpty();
        genesis.vtx.push_back(txNew);
        genesis.hashPrevBlock = 0;
        genesis.hashMerkleRoot = genesis.BuildMerkleTree();
        genesis.nVersion = 1;
        genesis.nTime = 1517690700;
        genesis.nBits = 0x1f00ffff;
        genesis.nNonce = 561379;

        hashGenesisBlock = genesis.GetHash();
        assert(genesis.hashMerkleRoot == uint256("0x40bdd3d5ae84b91a71190094a82948400eb3356e87c5376b64d79509cf552d84"));
        assert(hashGenesisBlock == uint256("0xf4bbfc518aa3622dbeb8d2818a606b82c2b8b1ac2f28553ebdb6fc04d7abaccf"));

        vSeeds.push_back(CDNSSeedData("1", "seed1.simplicity-coin.com"));
        vSeeds.push_back(CDNSSeedData("2", "seed2.simplicity-coin.com"));
        vSeeds.push_back(CDNSSeedData("3", "seed3.simplicity-coin.com"));
        vSeeds.push_back(CDNSSeedData("4", "zentec.ddns.net"));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 18);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 59);
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 93);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x44)(0xD5)(0xBC).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x44)(0xF0)(0xA3).convert_to_container<std::vector<unsigned char> >();
        // BIP44 coin type is from https://github.com/satoshilabs/slips/blob/master/slip-0044.md
        base58Prefixes[EXT_COIN_TYPE] = boost::assign::list_of(0x80)(0x00)(0x00)(0x77).convert_to_container<std::vector<unsigned char> >();

        convertSeed6(vFixedSeeds, pnSeed6_main, ARRAYLEN(pnSeed6_main));

        fMiningRequiresPeers = true;
        fAllowMinDifficultyBlocks = false;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fSkipProofOfWorkCheck = false;
        fTestnetToBeDeprecatedFieldRPC = false;
        fHeadersFirstSyncingActive = true;

        nPoolMaxTransactions = 3;
        nBudgetCycleBlocks = 30 * 24 * 60 * 60 / nTargetSpacing; //!< Amount of blocks in a months period of time (using 1 minutes per) = (60*24*30)
        strSporkKey = "03fdfa718ec40be6ce1b5fadf36022a4f0ff2f1efc872291ffbe42af127bdd2859";
        strSporkKeyOld = "03fdfa718ec40be6ce1b5fadf36022a4f0ff2f1efc872291ffbe42af127bdd2859";
        strObfuscationPoolDummyAddress = "D87q2gC9j6nNrnzCsg4aY6bHMLsT9nUhEw";
        nStartMasternodePayments = 1403728576; //Wed, 25 Jun 2014 20:36:16 GMT

        /** Zerocoin */
        zerocoinModulus = "25195908475657893494027183240048398571429282126204032027777137836043662020707595556264018525880784"
            "4069182906412495150821892985591491761845028084891200728449926873928072877767359714183472702618963750149718246911"
            "6507761337985909570009733045974880842840179742910064245869181719511874612151517265463228221686998754918242243363"
            "7259085141865462043576798423387184774447920739934236584823824281198163815010674810451660377306056201619676256133"
            "8441436038339044149526344321901146575444541784240209246165157233507787077498171257724679629263863563732899121548"
            "31438167899885040445364023527381951378636564391212010397122822120720357";
        nMaxZerocoinSpendsPerTransaction = 7; // Assume about 20kb each
        nMaxZerocoinPublicSpendsPerTransaction = 637; // Assume about 220 bytes each input
        nMinZerocoinMintFee = 1 * CENT; //high fee required for zerocoin mints
        nMintRequiredConfirmations = 20; //the maximum amount of confirmations until accumulated in 19
        nRequiredAccumulation = 1;
        nDefaultSecurityLevel = 100; //full security level for accumulators
        //nZerocoinHeaderVersion = 20; //Block headers must be this version once zerocoin is active
        nZerocoinRequiredStakeDepth = 200; //The required confirmations for a zspl to be stakable

        nBudget_Fee_Confirmations = 6; // Number of confirmations for the finalization fee
        nProposalEstablishmentTime = 60 * 60 * 24; // Proposals must be at least a day old to make it into a budget
    }

    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        return data;
    }

};

static CMainParams mainParams;

/**
 * Testnet (v3)
 */
class CTestNetParams : public CMainParams
{
public:
    CTestNetParams()
    {
        networkID = CBaseChainParams::TESTNET;
        strNetworkID = "test";
        pchMessageStart[0] = 0xf1;
        pchMessageStart[1] = 0xe3;
        pchMessageStart[2] = 0xdc;
        pchMessageStart[3] = 0xc6;
        vAlertPubKey = ParseHex("03b95000b2b06e391c058ea14d47ac3c525753c68460864f254ada5a63e27a8134");
        nDefaultPort = 21957;
        bnProofOfWorkLimit[POW_QUARK] = ~uint256(0) >> 12;
        bnProofOfWorkLimit[POW_SCRYPT_SQUARED] = ~uint256(0) >> 8;
        nEnforceBlockUpgradeMajority = 3780; // 70%
        nRejectBlockOutdatedMajority = 4050; // 75%
        nToCheckBlockUpgradeMajority = 5400; // 4 days (1350*4)
        nMinerThreads = 0;
        nTargetTimespan = 20 * 60; // Simplicity: 20 minutes
        nTargetSpacing = 64; // Simplicity: 64 seconds
        nMaturity = 15;
        nStakeMinDepth = 100;
        nMasternodeCountDrift = 4;
        nMandatoryUpgradeBlock = 0;
        nUpgradeBlockVersion = 8;
        nModifierUpdateBlock = 0; //approx Mon, 17 Apr 2017 04:00:00 GMT
        nMaxMoneyOut = 500000000 * COIN;
        nZerocoinStartHeight = 2100000000;
        //nZerocoinStartTime = 1501776000;
        nBlockEnforceSerialRange = 0; //Enforce serial range starting this block
        nBlockRecalculateAccumulators = nZerocoinStartHeight + 10; //Trigger a recalculation of accumulators
        nBlockFirstFraudulent = nZerocoinStartHeight; //First block that bad serials emerged
        nBlockLastGoodCheckpoint = nZerocoinStartHeight; //Last valid accumulator checkpoint
        nBlockEnforceInvalidUTXO = 0; //Start enforcing the invalid UTXO's
        nInvalidAmountFiltered = 0; //Amount of invalid coins filtered through exchanges, that should be considered valid
        nBlockZerocoinV2 = nZerocoinStartHeight + 20; //!> The block that zerocoin v2 becomes active
        nEnforceNewSporkKey = 1566860400; //!> Sporks signed after Monday, August 26, 2019 11:00:00 PM GMT must use the new spork key
        nRejectOldSporkKey = 1569538800; //!> Reject old spork key after Thursday, September 26, 2019 11:00:00 PM GMT
        nBlockStakeModifierlV2 = 0;
        // Public coin spend enforcement
        nPublicZCSpends = nZerocoinStartHeight + 30;

        nStartTreasuryBlock = 10;
        nTreasuryBlockStep = 20; //24 * 6 * 60 / nTargetSpacing; // Ten times per day
        nMasternodeTiersStartHeight = 0;
        vDevFundPubKey1 = CPubKey(ParseHex("03b95000b2b06e391c058ea14d47ac3c525753c68460864f254ada5a63e27a8134"));
        vDevFundPubKey2 = CPubKey(ParseHex("0242044552e508e9379b2fa8a9011aa017ddcc5b54b975100f5f43ebd53de3d194"));
        vCommunityFundWallet = "8r9dZJmXFidaZyXijEtD7LuDB3wThJKiut";
        assert(vDevFundPubKey1.IsFullyValid());
        assert(vDevFundPubKey2.IsFullyValid());
        //vBurnAddresses.clear();
        //vBurnAddresses.emplace_back("yJbqydKwNnV5zisR7kRWDrQQeG9mBAzqM5");
        //vBurnAddresses.emplace_back("xzd3LKsihYn1CKBESTQP7EresFECXEMivk");

        //! Modify the testnet genesis block so the timestamp is valid for a later start.
        //genesis.nTime = 1454124731;
        genesis.nBits = 0x1f00ffff;
        genesis.nNonce = 93481;

        hashGenesisBlock = genesis.GetHash();
        //printf("Merkle hash test: %s\n", genesis.hashMerkleRoot.ToString().c_str());
        //printf("Block hash test: %s\n", hashGenesisBlock.ToString().c_str());
        assert(genesis.hashMerkleRoot == uint256("0x40bdd3d5ae84b91a71190094a82948400eb3356e87c5376b64d79509cf552d84"));
        assert(hashGenesisBlock == uint256("0xfcfc1b5bc930bc0a74643462617264e4f7aa39e276c637353bda6960b5726fb8"));

        vFixedSeeds.clear();
        vSeeds.clear();
        // vSeeds.push_back(CDNSSeedData("45.76.61.28", "207.148.0.129"));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 139); // Testnet simplicity addresses start with 'x' or 'y'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 19);  // Testnet simplicity script addresses start with '8' or '9'
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 239);     // Testnet private keys start with '9' or 'c' (Bitcoin defaults)
        // Testnet simplicity BIP32 pubkeys start with 'DRKV'
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x3a)(0x80)(0x61)(0xa0).convert_to_container<std::vector<unsigned char> >();
        // Testnet simplicity BIP32 prvkeys start with 'DRKP'
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x3a)(0x80)(0x58)(0x37).convert_to_container<std::vector<unsigned char> >();
        // Testnet simplicity BIP44 coin type is '1' (All coin's testnet default)
        base58Prefixes[EXT_COIN_TYPE] = boost::assign::list_of(0x80)(0x00)(0x00)(0x01).convert_to_container<std::vector<unsigned char> >();

        convertSeed6(vFixedSeeds, pnSeed6_test, ARRAYLEN(pnSeed6_test));

        fMiningRequiresPeers = true;
        fAllowMinDifficultyBlocks = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = true;

        nPoolMaxTransactions = 2;
        nBudgetCycleBlocks = 24 * 6 * 60 / nTargetSpacing; //!< Ten cycles per day on testnet
        strSporkKey = "03b95000b2b06e391c058ea14d47ac3c525753c68460864f254ada5a63e27a8134";
        strSporkKeyOld = "03b95000b2b06e391c058ea14d47ac3c525753c68460864f254ada5a63e27a8134";
        strObfuscationPoolDummyAddress = "y6S5YPwPCXi2oemSRJGitNPwPjcFJfwbED";
        nStartMasternodePayments = 1420837558; //Fri, 09 Jan 2015 21:05:58 GMT
        nBudget_Fee_Confirmations = 3; // Number of confirmations for the finalization fee. We have to make this very short
                                       // here because we only have a 8 block finalization window on testnet

        nProposalEstablishmentTime = 60 * 5; // Proposals must be at least 5 mns old to make it into a test budget
    }
    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        return dataTestnet;
    }
};
static CTestNetParams testNetParams;

/**
 * Regression test
 */
class CRegTestParams : public CTestNetParams
{
public:
    CRegTestParams()
    {
        networkID = CBaseChainParams::REGTEST;
        strNetworkID = "regtest";
        pchMessageStart[0] = 0xa1;
        pchMessageStart[1] = 0xcf;
        pchMessageStart[2] = 0x7e;
        pchMessageStart[3] = 0xac;
        nDefaultPort = 51476;
        nEnforceBlockUpgradeMajority = 750;
        nRejectBlockOutdatedMajority = 950;
        nToCheckBlockUpgradeMajority = 1000;
        nMinerThreads = 1;
        nTargetTimespan = 10 * 60; // Simplicity: 10 minutes
        nTargetSpacing = 48; // Simplicity: 48 seconds
        bnProofOfWorkLimit[POW_QUARK] = ~uint256(0) >> 1;
        bnProofOfWorkLimit[POW_SCRYPT_SQUARED] = ~uint256(0) >> 1;
        nMaturity = 100;
        nStakeMinDepth = 0;
        nMasternodeCountDrift = 4;
        //nUpgradeBlockVersion = 8;
        nModifierUpdateBlock = 0; //approx Mon, 17 Apr 2017 04:00:00 GMT
        nMaxMoneyOut = 43199500 * COIN;
        nZerocoinStartHeight = 300;
        nBlockZerocoinV2 = 300;
        //nZerocoinStartTime = 1501776000;
        nBlockEnforceSerialRange = 0; //Enforce serial range starting this block
        nBlockRecalculateAccumulators = 2100000000; //Trigger a recalculation of accumulators
        nBlockFirstFraudulent = 2100000000; //First block that bad serials emerged
        nBlockLastGoodCheckpoint = 2100000000; //Last valid accumulator checkpoint
        nBlockStakeModifierlV2 = std::numeric_limits<int>::max(); // max integer value (never switch on regtest)
        // Public coin spend enforcement
        nPublicZCSpends = 350;

        //! Modify the regtest genesis block so the timestamp is valid for a later start.
        genesis.nTime = 1454124731;
        genesis.nBits = 0x207fffff;
        genesis.nNonce = 12345;

        hashGenesisBlock = genesis.GetHash();
        //printf("Merkle hash reg: %s\n", genesis.hashMerkleRoot.ToString().c_str());
        //printf("Block hash reg: %s\n", hashGenesisBlock.ToString().c_str());
        assert(genesis.hashMerkleRoot == uint256("0x40bdd3d5ae84b91a71190094a82948400eb3356e87c5376b64d79509cf552d84"));
        assert(hashGenesisBlock == uint256("0xf38094d1d2ccb97e06248813ee887b48ee7326d68cb663ebf59eeca7a0bde2c4"));

        vFixedSeeds.clear(); //! Testnet mode doesn't have any fixed seeds.
        vSeeds.clear();      //! Testnet mode doesn't have any DNS seeds.

        fMiningRequiresPeers = false;
        fAllowMinDifficultyBlocks = true;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;
        fSkipProofOfWorkCheck = true;
        fTestnetToBeDeprecatedFieldRPC = false;

        /* Spork Key for RegTest:
        WIF private key: 932HEevBSujW2ud7RfB1YF91AFygbBRQj3de3LyaCRqNzKKgWXi
        private key hex: bd4960dcbd9e7f2223f24e7164ecb6f1fe96fc3a416f5d3a830ba5720c84b8ca
        Address: yCvUVd72w7xpimf981m114FSFbmAmne7j9
        */
        strSporkKey = "043969b1b0e6f327de37f297a015d37e2235eaaeeb3933deecd8162c075cee0207b13537618bde640879606001a8136091c62ec272dd0133424a178704e6e75bb7";
    }
    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        return dataRegtest;
    }
};
static CRegTestParams regTestParams;

/**
 * Unit test
 */
class CUnitTestParams : public CMainParams, public CModifiableParams
{
public:
    CUnitTestParams()
    {
        networkID = CBaseChainParams::UNITTEST;
        strNetworkID = "unittest";
        nDefaultPort = 51478;
        vFixedSeeds.clear(); //! Unit test mode doesn't have any fixed seeds.
        vSeeds.clear();      //! Unit test mode doesn't have any DNS seeds.

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fAllowMinDifficultyBlocks = false;
        fMineBlocksOnDemand = true;
    }

    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        // UnitTest share the same checkpoints as MAIN
        return data;
    }

    //! Published setters to allow changing values in unit test cases
    virtual void setEnforceBlockUpgradeMajority(int anEnforceBlockUpgradeMajority) { nEnforceBlockUpgradeMajority = anEnforceBlockUpgradeMajority; }
    virtual void setRejectBlockOutdatedMajority(int anRejectBlockOutdatedMajority) { nRejectBlockOutdatedMajority = anRejectBlockOutdatedMajority; }
    virtual void setToCheckBlockUpgradeMajority(int anToCheckBlockUpgradeMajority) { nToCheckBlockUpgradeMajority = anToCheckBlockUpgradeMajority; }
    virtual void setDefaultConsistencyChecks(bool afDefaultConsistencyChecks) { fDefaultConsistencyChecks = afDefaultConsistencyChecks; }
    virtual void setAllowMinDifficultyBlocks(bool afAllowMinDifficultyBlocks) { fAllowMinDifficultyBlocks = afAllowMinDifficultyBlocks; }
    virtual void setSkipProofOfWorkCheck(bool afSkipProofOfWorkCheck) { fSkipProofOfWorkCheck = afSkipProofOfWorkCheck; }
};
static CUnitTestParams unitTestParams;

static CChainParams* pCurrentParams = 0;

CModifiableParams* ModifiableParams()
{
    assert(pCurrentParams);
    assert(pCurrentParams == &unitTestParams);
    return (CModifiableParams*)&unitTestParams;
}

const CChainParams& Params()
{
    assert(pCurrentParams);
    return *pCurrentParams;
}

CChainParams& Params(CBaseChainParams::Network network)
{
    switch (network) {
    case CBaseChainParams::MAIN:
        return mainParams;
    case CBaseChainParams::TESTNET:
        return testNetParams;
    case CBaseChainParams::REGTEST:
        return regTestParams;
    case CBaseChainParams::UNITTEST:
        return unitTestParams;
    default:
        assert(false && "Unimplemented network");
        return mainParams;
    }
}

void SelectParams(CBaseChainParams::Network network)
{
    SelectBaseParams(network);
    pCurrentParams = &Params(network);
}

bool SelectParamsFromCommandLine()
{
    CBaseChainParams::Network network = NetworkIdFromCommandLine();
    if (network == CBaseChainParams::MAX_NETWORK_TYPES)
        return false;

    SelectParams(network);
    return true;
}
