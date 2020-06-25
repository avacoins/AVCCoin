// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2019 The PIVX developers
// Copyright (c) 2020 The AVC developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "libzerocoin/Params.h"
#include "chainparams.h"
#include "consensus/merkle.h"
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
(0, uint256("0x000004997e94dbfc68499dd14eda5bb2111fc9094b468c4182dfb4fe9864ca2a"))
(196, uint256("0x00000026d8a2298b193da6fa18b3ad19af5e340b2aba1ab340d764959f4557f0"))
(2047, uint256("0xeb16b54e49d3d9354f3437fa2ebb664396c18d6cc4344b25acd15a1a482ec608"))
(4375, uint256("0x295b5b546f480faf5ffd2fe11f5879243602f95e2a1324e9c5fdde07fa0ac599"))
(7099, uint256("0x4ed1531f1169f7e7abea644a7999f16a3f6f9f4f018e56e4f5137ce963dd806f"))
(15487, uint256("0xad93f6157d04b84c5235d311f3fcabc8bbd6c6070cfeef788aef02ed36e0831a"))
(26843, uint256("0x7dcae45c3aabdaa99421dd553b70136ba899fb1aa0c30a0906cd6dee4098eaf0"))
(85492, uint256("0xc794e64712b061bdd8b70e82ff3e582902ce750791836615d997ad0a5e81329c"))
(105487, uint256("0x3b10ae076b6cdb2d6764b340b8f5f84cccccd177f9a3b0318e759610b64cbe02"))
(122847, uint256("0xbd78c8608aef2ee34db44edd1a176bb64d28f0d22b31d3733b3f268c8c91ee2d"))
(153426, uint256("0xa82a702e83290e8c256bf4038c4cc7994a8459512fb02c615299531f9dfb89f8"))
(166856, uint256("0xe8f8e20265366130f7669299f200a2e808cf3e654f7622af376a404d21d03b7a"))

;
static const Checkpoints::CCheckpointData data = {
	&mapCheckpoints,
	1593040080, // * UNIX timestamp of last checkpoint block
	338476,     // * total number of transactions between genesis and last checkpoint
	    	    //   (the tx=... number in the SetBestChain debug.log lines)
	1460        // * estimated number of transactions per day after checkpoint
};

static Checkpoints::MapCheckpoints mapCheckpointsTestnet =
boost::assign::map_list_of
(0, uint256("0x001"))
;
static const Checkpoints::CCheckpointData dataTestnet = {
	&mapCheckpointsTestnet,
	1560843157,
	2501682,
	250 };

static Checkpoints::MapCheckpoints mapCheckpointsRegtest =
boost::assign::map_list_of(0, uint256("0x001"));
static const Checkpoints::CCheckpointData dataRegtest = {
	&mapCheckpointsRegtest,
	1579132800,
	0,
	100 };

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
	// before stake modifier V2, the age required was 60 * 60 (1 hour). Not required for regtest
	if (!IsStakeModifierV2(contextHeight))
		return NetworkID() == CBaseChainParams::REGTEST || (utxoFromBlockTime + nStakeMinAge <= contextTime);

	// after stake modifier V2, we require the utxo to be nStakeMinDepth deep in the chain
	return (contextHeight - utxoFromBlockHeight >= nStakeMinDepth);
}

int CChainParams::FutureBlockTimeDrift(const int nHeight) const
{
	if (IsTimeProtocolV2(nHeight))
		// PoS (TimeV2): 14 seconds
		return TimeSlotLength() - 1;

	// PoS (TimeV1): 3 minutes
	// PoW: 2 hours
	return (nHeight > LAST_POW_BLOCK()) ? nFutureTimeDriftPoS : nFutureTimeDriftPoW;
}

bool CChainParams::IsValidBlockTimeStamp(const int64_t nTime, const int nHeight) const
{
	// Before time protocol V2, blocks can have arbitrary timestamps
	if (!IsTimeProtocolV2(nHeight))
		return true;

	// Time protocol v2 requires time in slots
	return (nTime % TimeSlotLength()) == 0;
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
		pchMessageStart[0] = 0xad;
		pchMessageStart[1] = 0xc5;
		pchMessageStart[2] = 0x1e;
		pchMessageStart[3] = 0xf2;
		vAlertPubKey = ParseHex("0000098d3ba6ba6e7423fa5cbd6a89e0a9a5348f88d332b66a5cb1a8b7ed2c1eaa665fc8dc4f012cb8241880bdafd6ca70c5f5747916e4e6f511abd746ed57dc50");
		nDefaultPort = 8220;
		bnProofOfWorkLimit = ~uint256(0) >> 20; // AVC starting difficulty is 1 / 2^12
		bnProofOfStakeLimit = ~uint256(0) >> 24;
		bnProofOfStakeLimit_V2 = ~uint256(0) >> 20; // 60/4 = 15 ==> use 2**4 higher limit
													//nSubsidyHalvingInterval = 210000;
		nMaxReorganizationDepth = 100;
		nEnforceBlockUpgradeMajority = 8100; // 75%
		nRejectBlockOutdatedMajority = 10260; // 95%
		nToCheckBlockUpgradeMajority = 10800; // Approximate expected amount of blocks in 7 days (1440*7.5)
		nMinerThreads = 0;
		nTargetSpacing = 1 * 60;                        // 1 minute
		nTargetTimespan = 10 * 60;                      // 10 minutes
		nTimeSlotLength = 15;                           // 15 seconds
		nTargetTimespan_V2 = nTimeSlotLength * 60;		// 15 minutes
		nMaturity = 60;
		nStakeMinAge = 60 * 60;                         // 1 hour
		nStakeMinDepth = 200;
		nFutureTimeDriftPoW = 7200;
		nFutureTimeDriftPoS = 180;
		nMasternodeCountDrift = 20;
		nMaxMoneyOut = 25000000 * COIN;
		nMinColdStakingAmount = 10 * COIN;

		/** Height or Time Based Activations **/
		nLastPOWBlock = 200;
		nModifierUpdateBlock = 11000;
		nZerocoinStartHeight = 235;
		nZerocoinStartTime = 1581489000; // Friday, 17 January 2020 06:30:00 AM

		nBlockEnforceInvalidUTXO = 9902850; //Start enforcing the invalid UTXO's
		nInvalidAmountFiltered = 0; //Amount of invalid coins filtered through exchanges, that should be considered valid
		nBlockZerocoinV2 = 235; //!> The block that zerocoin v2 becomes active - roughly Tuesday, May 8, 2018 4:00:00 AM GMT
		nBlockDoubleAccumulated = 0;
		nEnforceNewSporkKey = 1578459600; //!> Sporks signed after Monday, August 26, 2019 11:00:00 PM GMT must use the new spork key
		nRejectOldSporkKey = 1578463200; //!> Fully reject old spork key after Thursday, September 26, 2019 11:00:00 PM GMT
		nBlockStakeModifierlV2 = 10000;
		nBIP65ActivationHeight = 11300;
		// Activation height for TimeProtocolV2, Blocks V7 and newMessageSignatures
		nBlockTimeProtocolV2 = 11500;

		// Public coin spend enforcement
		nPublicZCSpends = 300;

		// New P2P messages signatures
		nBlockEnforceNewMessageSignatures = nBlockTimeProtocolV2;

		// Blocks v7
		nBlockLastAccumulatorCheckpoint = 1;
		nBlockV7StartHeight = nBlockTimeProtocolV2;

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
		const char* pszTimestamp = "Kraken’s Dan Held on What’s Different About Bitcoin at $10K This Time";
		CMutableTransaction txNew;
		txNew.vin.resize(1);
		txNew.vout.resize(1);
		txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
		txNew.vout[0].SetEmpty();
		txNew.vout[0].scriptPubKey = CScript() << ParseHex("04c10e83b2703ccf322f7dbd62dd5855ac7c10bd055814ce121ba32607d573b8810c02c0582aed05b4deb9c4b77b26d92428c61256cd42774babea0a073b2ed0c9") << OP_CHECKSIG;
		genesis.vtx.push_back(txNew);
		genesis.hashPrevBlock = 0;
		genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
		genesis.nVersion = 1;
		genesis.nTime = 1581487200;
		genesis.nBits = 0x1e0ffff0;
		genesis.nNonce = 3480760;

		hashGenesisBlock = genesis.GetHash();
		assert(hashGenesisBlock == uint256("0x000004997e94dbfc68499dd14eda5bb2111fc9094b468c4182dfb4fe9864ca2a"));
		assert(genesis.hashMerkleRoot == uint256("0xdbdf57e602f8c09c1abc9b5336eec53f2b4a57936ddce2940bf9a611affbe6cc"));

		vSeeds.push_back(CDNSSeedData("178.128.110.122", "178.128.110.122"));
		vSeeds.push_back(CDNSSeedData("2400:6180:0:d1::629:3001", "2400:6180:0:d1::629:3001"));
		vSeeds.push_back(CDNSSeedData("167.71.101.46", "167.71.101.46"));
		vSeeds.push_back(CDNSSeedData("2604:a880:800:c1::25e:9001", "2604:a880:800:c1::25e:9001"));
		vSeeds.push_back(CDNSSeedData("138.197.200.243", "138.197.200.243"));
		vSeeds.push_back(CDNSSeedData("2604:a880:2:d0::2185:2001", "2604:a880:2:d0::2185:2001"));
		vSeeds.push_back(CDNSSeedData("188.166.59.127", "188.166.59.127"));
		vSeeds.push_back(CDNSSeedData("2a03:b0c0:2:d0::140:9001", "2a03:b0c0:2:d0::140:9001"));
		vSeeds.push_back(CDNSSeedData("178.62.94.160", "178.62.94.160"));
		vSeeds.push_back(CDNSSeedData("2a03:b0c0:1:d0::2cd:b001", "2a03:b0c0:1:d0::2cd:b001"));
		vSeeds.push_back(CDNSSeedData("167.172.98.30", "167.172.98.30"));
		vSeeds.push_back(CDNSSeedData("2a03:b0c0:3:e0::33b:1001", "2a03:b0c0:3:e0::33b:1001"));
		vSeeds.push_back(CDNSSeedData("138.197.160.48", "138.197.160.48"));
		vSeeds.push_back(CDNSSeedData("2604:a880:cad:d0::9f3:1001", "2604:a880:cad:d0::9f3:1001"));
		vSeeds.push_back(CDNSSeedData("159.65.154.96", "159.65.154.96"));
		vSeeds.push_back(CDNSSeedData("2400:6180:100:d0::8a4:c001", "2400:6180:100:d0::8a4:c001"));

		base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 23);
		base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 13);
		base58Prefixes[STAKING_ADDRESS] = std::vector<unsigned char>(1, 70);     // starting with 'V'
		base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 212);
		base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x02)(0x2D)(0x25)(0x33).convert_to_container<std::vector<unsigned char> >();
		base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x02)(0x21)(0x31)(0x2B).convert_to_container<std::vector<unsigned char> >();
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
		fHeadersFirstSyncingActive = false;

		nPoolMaxTransactions = 3;
		nBudgetCycleBlocks = 43200; //!< Amount of blocks in a months period of time (using 1 minutes per) = (60*24*30)
		strSporkPubKey = "04a0820c439aa732c36934597a182e9faec622a4184a044af7df9f5facb8c16c9fb8a0e34e9d6303f52e4df3b0683bf8ae3b7e3e5fa4c33d373e388763959e79c0";
		strSporkPubKeyOld = "04a0820c439aa732c36934597a182e9faec622a4184a044af7df9f5facb8c16c9fb8a0e34e9d6303f52e4df3b0683bf8ae3b7e3e5fa4c33d373e388763959e79c0";
		strObfuscationPoolDummyAddress = "AG5bcjvnjV48RUM7YLGTyCRABJ57oGAcjQ";
		nStartMasternodePayments = 1581919200; //Saturday, January 18, 2020 6:00:00 AM GMT

											   /** Zerocoin */
		zerocoinModulus = "25195908475657893494027183240048398571429282126204032027777137836043662020707595556264018525880784"
			"4069182906412495150821892985591491761845028084891200728449926873928072877767359714183472702618963750149718246911"
			"6507761337985909570009733045974880842840179742910064245869181719511874612151517265463228221686998754918242243363"
			"725908514186546204357679842338718477444792073993423658898924281198163815010674810451660377306056208220676256133"
			"8441436038339044149526344321901146575444823884240209246165157233507787077498171257724679629263863563732899121548"
			"31438167899885040445364023527381951378636564391212010397122822120720357";
		nMaxZerocoinSpendsPerTransaction = 7; // Assume about 20kb each
		nMaxZerocoinPublicSpendsPerTransaction = 637; // Assume about 220 bytes each input
		nMinZerocoinMintFee = 1 * CENT; //high fee required for zerocoin mints
		nMintRequiredConfirmations = 20; //the maximum amount of confirmations until accumulated in 19
		nRequiredAccumulation = 1;
		nDefaultSecurityLevel = 100; //full security level for accumulators
		nZerocoinHeaderVersion = 4; //Block headers must be this version once zerocoin is active
		nZerocoinRequiredStakeDepth = 200; //The required confirmations for a zAVC to be stakable

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
		pchMessageStart[0] = 0x45;
		pchMessageStart[1] = 0x76;
		pchMessageStart[2] = 0x65;
		pchMessageStart[3] = 0xba;
		vAlertPubKey = ParseHex("000010e83b2703ccf322f7dbd62dd5855ac7c10bd055814ce121ba32607d573b8810c02c0582aed05b4deb9c4b77b26d92428c61256cd42774babea0a073b2ed0c9");
		nDefaultPort = 51474;
		nEnforceBlockUpgradeMajority = 4320; // 75%
		nRejectBlockOutdatedMajority = 5472; // 95%
		nToCheckBlockUpgradeMajority = 5760; // 4 days
		nMinerThreads = 0;
		nLastPOWBlock = 200;
		nMaturity = 15;
		nStakeMinDepth = 100;
		nMasternodeCountDrift = 4;
		nModifierUpdateBlock = 51197; //approx Mon, 17 Apr 2017 04:00:00 GMT
		nMaxMoneyOut = 43199500 * COIN;
		nZerocoinStartHeight = 201576;
		nZerocoinStartTime = 1501776000;

		nBlockEnforceInvalidUTXO = 9902850; //Start enforcing the invalid UTXO's
		nInvalidAmountFiltered = 0; //Amount of invalid coins filtered through exchanges, that should be considered valid
		nBlockZerocoinV2 = 444020; //!> The block that zerocoin v2 becomes active
		nEnforceNewSporkKey = 1566860400; //!> Sporks signed after Monday, August 26, 2019 11:00:00 PM GMT must use the new spork key
		nRejectOldSporkKey = 1569538800; //!> Reject old spork key after Thursday, September 26, 2019 11:00:00 PM GMT
		nBlockStakeModifierlV2 = 1214000;
		nBIP65ActivationHeight = 851019;
		// Activation height for TimeProtocolV2, Blocks V7 and newMessageSignatures
		nBlockTimeProtocolV2 = 1347000;

		// Public coin spend enforcement
		nPublicZCSpends = 1106100;

		// New P2P messages signatures
		nBlockEnforceNewMessageSignatures = nBlockTimeProtocolV2;

		// Blocks v7
		nBlockLastAccumulatorCheckpoint = nPublicZCSpends - 10;
		nBlockV7StartHeight = nBlockTimeProtocolV2;

		//! Modify the testnet genesis block so the timestamp is valid for a later start.
		genesis.nTime = 1579132800;
		genesis.nNonce = 2402015;

		hashGenesisBlock = genesis.GetHash();
		//assert(hashGenesisBlock == uint256("0x0000041e482b9b9691d98eefb48473405c0b8ec31b76df3797c74a78680ef818"));

		vFixedSeeds.clear();
		vSeeds.clear();
		vSeeds.push_back(CDNSSeedData("fuzzbawls.pw", "AVC-testnet.seed.fuzzbawls.pw"));
		vSeeds.push_back(CDNSSeedData("fuzzbawls.pw", "AVC-testnet.seed2.fuzzbawls.pw"));
		vSeeds.push_back(CDNSSeedData("warrows.dev", "testnet.dnsseed.AVC.warrows.dev"));

		base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 139); // Testnet AVC addresses start with 'x' or 'y'
		base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 19);  // Testnet AVC script addresses start with '8' or '9'
		base58Prefixes[STAKING_ADDRESS] = std::vector<unsigned char>(1, 73);     // starting with 'W'
		base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 239);     // Testnet private keys start with '9' or 'c' (Bitcoin defaults)
																			 // Testnet AVC BIP32 pubkeys start with 'DRKV'
		base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x3a)(0x80)(0x61)(0xa0).convert_to_container<std::vector<unsigned char> >();
		// Testnet AVC BIP32 prvkeys start with 'DRKP'
		base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x3a)(0x80)(0x58)(0x37).convert_to_container<std::vector<unsigned char> >();
		// Testnet AVC BIP44 coin type is '1' (All coin's testnet default)
		base58Prefixes[EXT_COIN_TYPE] = boost::assign::list_of(0x80)(0x00)(0x00)(0x01).convert_to_container<std::vector<unsigned char> >();

		convertSeed6(vFixedSeeds, pnSeed6_test, ARRAYLEN(pnSeed6_test));

		fMiningRequiresPeers = true;
		fAllowMinDifficultyBlocks = true;
		fDefaultConsistencyChecks = false;
		fRequireStandard = true;
		fMineBlocksOnDemand = false;
		fTestnetToBeDeprecatedFieldRPC = true;

		nPoolMaxTransactions = 2;
		nBudgetCycleBlocks = 144; //!< Ten cycles per day on testnet
		strSporkPubKey = "04E88BB455E2A04E65FCC41D88CD367E9CCE1F5A409BE94D8C2B4B35D223DED9C8E2F4E061349BA3A38839282508066B6DC4DB72DD432AC4067991E6BF20176127";
		strSporkPubKeyOld = "04A8B319388C0F8588D238B9941DC26B26D3F9465266B368A051C5C100F79306A557780101FE2192FE170D7E6DEFDCBEE4C8D533396389C0DAFFDBC842B002243C";
		strObfuscationPoolDummyAddress = "y57cqfGRkekRyDRNeJiLtYVEbvhXrNbmox";
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
		//nSubsidyHalvingInterval = 150;
		nEnforceBlockUpgradeMajority = 750;
		nRejectBlockOutdatedMajority = 950;
		nToCheckBlockUpgradeMajority = 1000;
		nMinerThreads = 1;
		bnProofOfWorkLimit = ~uint256(0) >> 1;
		nLastPOWBlock = 250;
		nMaturity = 100;
		nStakeMinAge = 0;
		nStakeMinDepth = 0;
		nMasternodeCountDrift = 4;
		nModifierUpdateBlock = 0;       //approx Mon, 17 Apr 2017 04:00:00 GMT
		nMaxMoneyOut = 43199500 * COIN;
		nZerocoinStartHeight = 300;
		nBlockZerocoinV2 = 300;
		nZerocoinStartTime = 1501776000;

		nBlockStakeModifierlV2 = 1000;
		nBlockTimeProtocolV2 = 999999999;

		// Public coin spend enforcement
		nPublicZCSpends = 350;

		// Blocks v7
		nBlockV7StartHeight = nPublicZCSpends + 1;
		nBlockLastAccumulatorCheckpoint = nPublicZCSpends - 10;

		// New P2P messages signatures
		nBlockEnforceNewMessageSignatures = 1;

		//! Modify the regtest genesis block so the timestamp is valid for a later start.
		genesis.nTime = 1579132800;
		genesis.nNonce = 2402015;

		hashGenesisBlock = genesis.GetHash();
		//assert(hashGenesisBlock == uint256("0x0000041e482b9b9691d98eefb48473405c0b8ec31b76df3797c74a78680ef818"));
		//assert(hashGenesisBlock == uint256("0x4f023a2120d9127b21bbad01724fdb79b519f593f2a85b60d3d79160ec5f29df"));

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
		strSporkPubKey = "043969b1b0e6f327de37f297a015d37e2235eaaeeb3933deecd8162c075cee0207b13537618bde640879606001a8136091c62ec272dd0133424a178704e6e75bb7";
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
	//virtual void setSubsidyHalvingInterval(int anSubsidyHalvingInterval) { nSubsidyHalvingInterval = anSubsidyHalvingInterval; }
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
