// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014-2015 The Oro developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"

#include "random.h"
#include "util.h"
#include "utilstrencodings.h"
#include <stdio.h>
#include <assert.h>

#include <string>
#include <iostream>

using namespace std;
#include <boost/assign/list_of.hpp>

using namespace std;
using namespace boost::assign;

struct SeedSpec6 {
    uint8_t addr[16];
    uint16_t port;
};

#include "chainparamsseeds.h"

/**
 * Main network
 */

//! Convert the pnSeeds6 array into usable address objects.
static void convertSeed6(std::vector<CAddress> &vSeedsOut, const SeedSpec6 *data, unsigned int count)
{
    // It'll only connect to one or two seed nodes because once it connects,
    // it'll get a pile of addresses with newer timestamps.
    // Seed nodes are given a random 'last seen time' of between one and two
    // weeks ago.
    const int64_t nOneWeek = 7*24*60*60;
    for (unsigned int i = 0; i < count; i++)
    {
        struct in6_addr ip;
        memcpy(&ip, data[i].addr, sizeof(ip));
        CAddress addr(CService(ip, data[i].port));
        addr.nTime = GetTime() - GetRand(nOneWeek) - nOneWeek;
        vSeedsOut.push_back(addr);
    }
}

/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */
static Checkpoints::MapCheckpoints mapCheckpoints =
        boost::assign::map_list_of
        ( 0, uint256("0x00000007afe3f727654b0853bc3aaaa7f75afac7e84d7838806e09f097cff7f9"))
		( 10, uint256("0x4593438cc671c19d2786ab985dc0d03c637d57f6378b3cdb9f0e8f5a2349b7df"))
		( 100, uint256("0x13c1f0f98509319389b5f328e10fe5fa7db408461be509ecc5df717caaf89dff"))
		( 200, uint256("0x53787ec038e667cc4a819ff19176b60ff2c28a5347936fa8574f888749d1e8d9"))
		( 4000, uint256("0x7cc25b10ab8c14fefce83e16d30f805f48735d25bdee3ed39b00d0f648258ad1"))
        ;
static const Checkpoints::CCheckpointData data = {
        &mapCheckpoints,
		1527746400, // * UNIX timestamp of last checkpoint block
        0,          // * total number of transactions between genesis and last checkpoint
                    //   (the tx=... number in the SetBestChain debug.log lines)
        0           // * estimated number of transactions per day after checkpoint
    };

static Checkpoints::MapCheckpoints mapCheckpointsTestnet =
        boost::assign::map_list_of
        ( 0, uint256("0x485bc2699c4523113d411423cce7c77ad4f4cbae4d8ea34d8e3ee0c69620b086"))
        ;
static const Checkpoints::CCheckpointData dataTestnet = {
        &mapCheckpointsTestnet,
        1429025142,
        0,
        0
    };

static Checkpoints::MapCheckpoints mapCheckpointsRegtest =
        boost::assign::map_list_of
        ( 0, uint256("0x2d08bd6fb78ac5ed5c2ade5cdb993ea237f33a926dbd213e922f26d5149859d4"))
        ;
static const Checkpoints::CCheckpointData dataRegtest = {
        &mapCheckpointsRegtest,
        0,
        0,
        0
    };

class CMainParams : public CChainParams {
public:
    CMainParams() {
        networkID = CBaseChainParams::MAIN;
        strNetworkID = "main";
        /** 
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 4-byte int at any alignment.
         */
        pchMessageStart[0] = 0xf9;
        pchMessageStart[1] = 0xbe;
        pchMessageStart[2] = 0xb4;
        pchMessageStart[3] = 0xe0;
        vAlertPubKey = ParseHex("0426d1c5aac0e7b98f37f5f8ca10a18bb915820516723a727093cca65108ac24cdf3467ff06a39ad388ccc3d83802c85df73dba14e3db3835cec6892b9647e92fa");
        nDefaultPort = 9777;
        bnProofOfWorkLimit = ~uint256(0) >> 24;
        nEnforceBlockUpgradeMajority = 750;
        nRejectBlockOutdatedMajority = 950;
        nToCheckBlockUpgradeMajority = 1000;
        nMinerThreads = 0;
        nTargetTimespan = 3 * 60 * 60; // 3 hours in seconds
        nTargetSpacing = 15;    // seconds
        nLiveFeedSwitchTime = 1527746400;   //TODO(oro): 07/30/2016 22:00 (UTC)
        nPoWSwitchHeight = 0;

        /**
         * Build the genesis block. Note that the output of the genesis coinbase cannot
         * be spent as it did not originally exist in the database.
         */
        const char* pszTimestamp = "OroCoin genesis / MainNet";
        CMutableTransaction txNew;
        txNew.vin.resize(1);
        txNew.vout.resize(1);
        txNew.vin[0].scriptSig = CScript() << 0 << bnProofOfWorkLimit.GetCompact() << CScriptNum(4) << vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
//        cout << "script size--------" << txNew.vin[0].scriptSig.size() << endl;
        txNew.vout[0].nValue = 0;
        txNew.vout[0].scriptPubKey = CScript() << ParseHex("04b36556d4e6822708431cce73eaf447a0ec89a8ae6eb48aa412cb5b56bb6410acaa7cda7000e270b9900eb77667bb421728cab77e720c7ca2118150430c4f418a") << OP_CHECKSIG;
        genesis.vtx.resize(1);
        genesis.vtx[0] = txNew;
        genesis.hashPrevBlock = 0;
        genesis.hashMerkleRoot = genesis.BuildMerkleTree();
        genesis.nVersion = BLOCK_VERSION_0_3;
        genesis.nTime    = 1527746400;
        genesis.nBits    = bnProofOfWorkLimit.GetCompact();
       	genesis.nNonce   = 160817071;
        hashGenesisBlock = genesis.GetHash();

        LogPrintf("chainparams------%s: coinbaseTx.vin[0].scriptSig=%s\n", std::string(pszTimestamp), txNew.vin[0].scriptSig.ToString());

        assert(hashGenesisBlock == uint256("0x00000007afe3f727654b0853bc3aaaa7f75afac7e84d7838806e09f097cff7f9"));
        assert(genesis.hashMerkleRoot == uint256("0xa63c3a572fcd0467afce40768413df46d7854859d091749d46c53fd82b66f6cd"));

        vFixedSeeds.clear(); //! No seeds for now
        vSeeds.clear();
        vSeeds.push_back(CDNSSeedData("oro.exchange", "Main.seeds.oro.exchange"));

        vSeeds.push_back(CDNSSeedData("dynamiccoin.net", "ns1.dynamiccoin.net"));
        vSeeds.push_back(CDNSSeedData("dynamiccoin.net", "ns2.dynamiccoin.net"));
        vSeeds.push_back(CDNSSeedData("dynamiccoin.net", "ns3.dynamiccoin.net"));
        vSeeds.push_back(CDNSSeedData("dynamiccoin.net", "ns4.dynamiccoin.net"));
        vSeeds.push_back(CDNSSeedData("dynamiccoin.net", "ns5.dynamiccoin.net"));
        vSeeds.push_back(CDNSSeedData("dynamiccoin.net", "ns6.dynamiccoin.net"));
        vSeeds.push_back(CDNSSeedData("dynamiccoin.net", "ns7.dynamiccoin.net"));
        vSeeds.push_back(CDNSSeedData("dynamiccoin.net", "ns8.dynamiccoin.net"));
        vSeeds.push_back(CDNSSeedData("dynamiccoin.net", "ns9.dynamiccoin.net"));
        vSeeds.push_back(CDNSSeedData("dynamiccoin.net", "ns10.dynamiccoin.net"));
        vSeeds.push_back(CDNSSeedData("dynamiccoin.net", "ns11.dynamiccoin.net"));
        vSeeds.push_back(CDNSSeedData("dynamiccoin.net", "ns12.dynamiccoin.net"));
        vSeeds.push_back(CDNSSeedData("dynamiccoin.net", "ns13.dynamiccoin.net"));
        vSeeds.push_back(CDNSSeedData("dynamiccoin.net", "ns14.dynamiccoin.net"));
        vSeeds.push_back(CDNSSeedData("dynamiccoin.net", "ns15.dynamiccoin.net"));
        vSeeds.push_back(CDNSSeedData("dynamiccoin.net", "ns16.dynamiccoin.net"));

        base58Prefixes[PUBKEY_ADDRESS] = list_of(0);
        base58Prefixes[SCRIPT_ADDRESS] = list_of(5);
        base58Prefixes[SECRET_KEY] =     list_of(128);
        base58Prefixes[EXT_PUBLIC_KEY] = list_of(0x04)(0x88)(0xB2)(0x1E);
        base58Prefixes[EXT_SECRET_KEY] = list_of(0x04)(0x88)(0xAD)(0xE4);

        convertSeed6(vFixedSeeds, pnSeed6_main, ARRAYLEN(pnSeed6_main));

        fRequireRPCPassword = true;
        fMiningRequiresPeers = true;
        fAllowMinDifficultyBlocks = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fSkipProofOfWorkCheck = false;
        fTestnetToBeDeprecatedFieldRPC = false;
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
class CTestNetParams : public CMainParams {
public:
    CTestNetParams() {
        networkID = CBaseChainParams::TESTNET;
        strNetworkID = "test";
        pchMessageStart[0] = 0x0b;
        pchMessageStart[1] = 0x11;
        pchMessageStart[2] = 0x09;
        pchMessageStart[3] = 0x08;
        vAlertPubKey = ParseHex("04302390343f91cc401d56d68b123028bf52e5fca1939df127f63c6467cdf9c8e2c14b61104cf817d0b780da337893ecc4aaff1309e536162dabbdb45200ca2b0a");
        nDefaultPort = 19777;
        nEnforceBlockUpgradeMajority = 51;
        nRejectBlockOutdatedMajority = 75;
        nToCheckBlockUpgradeMajority = 100;
        nMinerThreads = 0;
        nTargetTimespan = 3 * 60 * 60; // 3 hours
        nTargetSpacing = 15;

        const char* pszTimestamp = "OroCoin genesis / TestNet";
        CMutableTransaction txNew;
        txNew.vin.resize(1);
        txNew.vout.resize(1);
        txNew.vin[0].scriptSig = CScript() << 0 << bnProofOfWorkLimit.GetCompact() << CScriptNum(4) << vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
        txNew.vout[0].nValue = 1024 * COIN;
        txNew.vout[0].scriptPubKey = CScript() << ParseHex("04b36556d4e6822708431cce73eaf447a0ec89a8ae6eb48aa412cb5b56bb6410acaa7cda7000e270b9900eb77667bb421728cab77e720c7ca2118150430c4f418a") << OP_CHECKSIG;
        genesis.vtx.resize(1);
        genesis.vtx[0] = txNew;
        genesis.hashPrevBlock = 0;
        genesis.hashMerkleRoot = genesis.BuildMerkleTree();
        genesis.nVersion = BLOCK_VERSION_0_3;
        genesis.nTime    = 1436316222;
        genesis.nBits    = bnProofOfWorkLimit.GetCompact();
        genesis.nNonce   = 42228550;
        hashGenesisBlock = genesis.GetHash();

        LogPrintf("%s: coinbaseTx.vin[0].scriptSig=%s\n", std::string(pszTimestamp), txNew.vin[0].scriptSig.ToString());

        assert(hashGenesisBlock == uint256("0x485bc2699c4523113d411423cce7c77ad4f4cbae4d8ea34d8e3ee0c69620b086"));
        assert(genesis.hashMerkleRoot == uint256("0x22cce82456074c4d0420f65f552797f40ff6d7564ee0ad75d860c3e101169c9e"));

        vFixedSeeds.clear(); //! No seeds for now
        vSeeds.clear();
        vSeeds.push_back(CDNSSeedData("oro.exchange", "Test.seeds.oro.exchange"));

        base58Prefixes[PUBKEY_ADDRESS] = list_of(111);
        base58Prefixes[SCRIPT_ADDRESS] = list_of(196);
        base58Prefixes[SECRET_KEY]     = list_of(239);
        base58Prefixes[EXT_PUBLIC_KEY] = list_of(0x04)(0x35)(0x87)(0xCF);
        base58Prefixes[EXT_SECRET_KEY] = list_of(0x04)(0x35)(0x83)(0x94);

        convertSeed6(vFixedSeeds, pnSeed6_test, ARRAYLEN(pnSeed6_test));

        fRequireRPCPassword = true;
        fMiningRequiresPeers = true;
        fAllowMinDifficultyBlocks = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = true;
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
class CRegTestParams : public CTestNetParams {
public:
    CRegTestParams() {
        networkID = CBaseChainParams::REGTEST;
        strNetworkID = "regtest";
        pchMessageStart[0] = 0xfa;
        pchMessageStart[1] = 0xbf;
        pchMessageStart[2] = 0xb5;
        pchMessageStart[3] = 0xdb;
        nEnforceBlockUpgradeMajority = 750;
        nRejectBlockOutdatedMajority = 950;
        nToCheckBlockUpgradeMajority = 1000;
        nMinerThreads = 1;
        nTargetTimespan = 3 * 60 * 60; //! 3 hours
        nTargetSpacing = 15;
        bnProofOfWorkLimit = ~uint256(0) >> 1;
        nDefaultPort = 17444;

        const char* pszTimestamp = "OroCoin genesis / Reg test";
        CMutableTransaction txNew;
        txNew.vin.resize(1);
        txNew.vout.resize(1);
        txNew.vin[0].scriptSig = CScript() << 0 << bnProofOfWorkLimit.GetCompact() << CScriptNum(4) << vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
        txNew.vout[0].nValue = 1024 * COIN;
        txNew.vout[0].scriptPubKey = CScript() << ParseHex("04b36556d4e6822708431cce73eaf447a0ec89a8ae6eb48aa412cb5b56bb6410acaa7cda7000e270b9900eb77667bb421728cab77e720c7ca2118150430c4f418a") << OP_CHECKSIG;
        genesis.vtx.resize(1);
        genesis.vtx[0] = txNew;
        genesis.hashPrevBlock = 0;
        genesis.hashMerkleRoot = genesis.BuildMerkleTree();
        genesis.nVersion = BLOCK_VERSION_0_3;
        genesis.nTime    = 1425370890;
        genesis.nBits    = bnProofOfWorkLimit.GetCompact();
        genesis.nNonce   = 48961;
        hashGenesisBlock = genesis.GetHash();

        LogPrintf("%s: coinbaseTx.vin[0].scriptSig=%s\n", std::string(pszTimestamp), txNew.vin[0].scriptSig.ToString());

        assert(hashGenesisBlock == uint256("0x2d08bd6fb78ac5ed5c2ade5cdb993ea237f33a926dbd213e922f26d5149859d4"));
        assert(genesis.hashMerkleRoot == uint256("0x4182255c0801f5e75b89ffbab51ad9a0a259095be7d0dac39f37d6dcaa367db9"));

        vFixedSeeds.clear(); //! Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();  //! Regtest mode doesn't have any DNS seeds.

        fRequireRPCPassword = false;
        fMiningRequiresPeers = false;
        fAllowMinDifficultyBlocks = true;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;
        fTestnetToBeDeprecatedFieldRPC = false;
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
class CUnitTestParams : public CMainParams, public CModifiableParams {
public:
    CUnitTestParams() {
        networkID = CBaseChainParams::UNITTEST;
        strNetworkID = "unittest";
        nDefaultPort = 17445;
        vFixedSeeds.clear(); //! Unit test mode doesn't have any fixed seeds.
        vSeeds.clear();  //! Unit test mode doesn't have any DNS seeds.

        const char* pszTimestamp = "OroCoin genesis / Unit test";
        CMutableTransaction txNew;
        txNew.vin.resize(1);
        txNew.vout.resize(1);
        txNew.vin[0].scriptSig = CScript() << 0 << bnProofOfWorkLimit.GetCompact() << CScriptNum(4) << vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
        txNew.vout[0].nValue = 1024 * COIN;
        txNew.vout[0].scriptPubKey = CScript() << ParseHex("04b36556d4e6822708431cce73eaf447a0ec89a8ae6eb48aa412cb5b56bb6410acaa7cda7000e270b9900eb77667bb421728cab77e720c7ca2118150430c4f418a") << OP_CHECKSIG;
        genesis.vtx.resize(1);
        genesis.vtx[0] = txNew;
        genesis.hashPrevBlock = 0;
        genesis.hashMerkleRoot = genesis.BuildMerkleTree();
        genesis.nVersion = BLOCK_VERSION_0_3;
        genesis.nTime    = 1429025148;
        genesis.nBits    = bnProofOfWorkLimit.GetCompact();
        genesis.nNonce   = 6505914;
        hashGenesisBlock = genesis.GetHash();

        LogPrintf("%s: coinbaseTx.vin[0].scriptSig=%s\n", std::string(pszTimestamp), txNew.vin[0].scriptSig.ToString());

        assert(hashGenesisBlock == uint256("0x2bbcf3dde65b0ccdf43e21f37712a04169e7ab3ec6e2b78fb9c2f6e7a3cf39e2"));
        assert(genesis.hashMerkleRoot == uint256("0x0af5a8e55983661850bd7f0f1666fabdd6a3d524bf86fa8727d0a241d8ca7ea8"));

        fRequireRPCPassword = false;
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
    virtual void setEnforceBlockUpgradeMajority(int anEnforceBlockUpgradeMajority)  { nEnforceBlockUpgradeMajority=anEnforceBlockUpgradeMajority; }
    virtual void setRejectBlockOutdatedMajority(int anRejectBlockOutdatedMajority)  { nRejectBlockOutdatedMajority=anRejectBlockOutdatedMajority; }
    virtual void setToCheckBlockUpgradeMajority(int anToCheckBlockUpgradeMajority)  { nToCheckBlockUpgradeMajority=anToCheckBlockUpgradeMajority; }
    virtual void setDefaultConsistencyChecks(bool afDefaultConsistencyChecks)  { fDefaultConsistencyChecks=afDefaultConsistencyChecks; }
    virtual void setAllowMinDifficultyBlocks(bool afAllowMinDifficultyBlocks) {  fAllowMinDifficultyBlocks=afAllowMinDifficultyBlocks; }
    virtual void setSkipProofOfWorkCheck(bool afSkipProofOfWorkCheck) { fSkipProofOfWorkCheck = afSkipProofOfWorkCheck; }
};
static CUnitTestParams unitTestParams;


static CChainParams *pCurrentParams = 0;

CModifiableParams *ModifiableParams()
{
   assert(pCurrentParams);
   assert(pCurrentParams==&unitTestParams);
   return (CModifiableParams*)&unitTestParams;
}

const CChainParams &Params() {
    assert(pCurrentParams);
    return *pCurrentParams;
}

CChainParams &Params(CBaseChainParams::Network network) {
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

void SelectParams(CBaseChainParams::Network network) {
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
