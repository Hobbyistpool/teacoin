// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <boost/foreach.hpp>

#include "checkpoints.h"

#include "main.h"
#include "uint256.h"

namespace Checkpoints
{
    typedef std::map<int, uint256> MapCheckpoints;

    // How many times we expect transactions after the last checkpoint to
    // be slower. This number is a compromise, as it can't be accurate for
    // every system. When reindexing from a fast disk with a slow CPU, it
    // can be up to 20, while when downloading from a slow network with a
    // fast multicore CPU, it won't be much higher than 1.
    static const double fSigcheckVerificationFactor = 5.0;

    struct CCheckpointData {
        const MapCheckpoints *mapCheckpoints;
        int64 nTimeLastCheckpoint;
        int64 nTransactionsLastCheckpoint;
        double fTransactionsPerDay;
    };

    bool fEnabled = true;

    // What makes a good checkpoint block?
    // + Is surrounded by blocks with reasonable timestamps
    //   (no blocks before with a timestamp after, none after with
    //    timestamp before)
    // + Contains no strange transactions
    static MapCheckpoints mapCheckpoints =
        boost::assign::map_list_of
        ( 0,     uint256("0x00000bb87d9514374986aeea9a275fab8465d575b2383654ebb6944460f00037"))
        ( 20160, uint256("0x00000000000176d0af32856ba528354707338bc3890192e5988712331e10bbba"))
        ( 40320, uint256("0x00000000000149f1222dffe7b8698389326d9473cbc1b4c80e271bf620ff7551"))
        ( 60479, uint256("0x0000000000004526af0bf7d1e8bce21e1ad8800ec01e8cba3ea3d22233186356"))
        ( 80640, uint256("0x0000000000002aaaa55261cdf4db9e01caaf7965e55c05b12a1f35332813481d"))
        ( 100800, uint256("0x000000000002420afffb6b9e5eec1dba41bdf47bb7d7dae27db1e7aed2ad70de"))
        ( 120960, uint256("0x000000000000d1818f2b275a068b20303249dfac7d262941f3aabf2650d4d7b6"))
        ( 141120, uint256("0x0000000000015ae6f67bb2806a912e2ae669eef2996c4b0b9ebfbe3a787d4f1c"))
        ( 161281, uint256("0x0000000000004b4ec886c89c546213f67fd34f167fc16ace4717b7c360de4124"))
        ( 181440, uint256("0x000000000000f438765707f0a885e6db106484995ef078262643ace0226c64b5"))
        ( 201600, uint256("0x000000000006a10a0bb5261071946ade1940e8a4c7c5ccd17284981672f06e31"))
        ( 204292, uint256("0x00000000000abbb36a5a991cdc2d1fecfe5299d3aea7d64b1229d848175611bb")) 
        ( 300000, uint256("0x00000000000a85b19595226fa69d7d14bc1da50a1c002823fabb9edb61d5ad47")) 
        ( 400000, uint256("0x000000000004922fb991ccb1f7f4d4e9f6751ade39ef059102b1e11400d89b90")) 
        ( 500000, uint256("0x0000000000172719d8421c53586b8f6d827f70b5706b7b9aa219a0ed3b0af308")) 
        ( 540000, uint256("0x0000000000125d22ce58f7037f8742f5fad7ca02b2a217f88aff5a656f142eed")) 
        ( 593524, uint256("0x0000000000228e379a537ce9326946ba912804eb9663c492618bd8e21b2abc90")) 
        ( 1000000, uint256("0x00000000000512eadc7e204f08ad3491c5d01b639f58113b4823ff54a9ae20bf")) 
        ( 2000000, uint256("0x000000002106ba2114f8633431782160004177250a51d87c5758dcb92554d08e")) 
        ( 3000000, uint256("0x000000005e3270cf8be5af4e258556eed5885cf0fb52e3cb300052cd2632a511")) 
        ( 4000000, uint256("0x0000000024e55e8a4dbd6755bf0cc6ad5e30e4117a9f5bf7d1876f363627cd5f")) 
        ( 5034231, uint256("0x000000004446c81dc673184bd74d17623a08dcd3366c9980318bb06a30d0bbee"))
        //( 0, uint256("0x"))

        ;
    static const CCheckpointData data = {
        &mapCheckpoints,
        1504905885, // * UNIX timestamp of last checkpoint block
        5311341, //842057,     // * total number of transactions between genesis and last checkpoint
                    //   (the tx=... number in the SetBestChain debug.log lines)
        2880        // * estimated number of transactions per day after checkpoint
    };

    static MapCheckpoints mapCheckpointsTestnet =
        boost::assign::map_list_of
        ( 0, uint256("0x00000d6feb89798a1bfb43642647d3549b69efeece1da75d7edc8730ab5e933d"))
        ;
    static const CCheckpointData dataTestnet = {
        &mapCheckpointsTestnet,
        1391126157,
        0,
        2880
    };

    const CCheckpointData &Checkpoints() {
        if (TestNet())
            return dataTestnet;
        else
            return data;
    }

    bool CheckBlock(int nHeight, const uint256& hash)
    {
        if (!fEnabled)
            return true;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        MapCheckpoints::const_iterator i = checkpoints.find(nHeight);
        if (i == checkpoints.end()) return true;
        
        //fake123
        return hash == i->second;
        //return true;
    }

    // Guess how far we are in the verification process at the given block index
    double GuessVerificationProgress(CBlockIndex *pindex) {
        if (pindex==NULL)
            return 0.0;

        int64 nNow = time(NULL);

        double fWorkBefore = 0.0; // Amount of work done before pindex
        double fWorkAfter = 0.0;  // Amount of work left after pindex (estimated)
        // Work is defined as: 1.0 per transaction before the last checkoint, and
        // fSigcheckVerificationFactor per transaction after.

        const CCheckpointData &data = Checkpoints();

        if (pindex->nChainTx <= data.nTransactionsLastCheckpoint) {
            double nCheapBefore = pindex->nChainTx;
            double nCheapAfter = data.nTransactionsLastCheckpoint - pindex->nChainTx;
            double nExpensiveAfter = (nNow - data.nTimeLastCheckpoint)/86400.0*data.fTransactionsPerDay;
            fWorkBefore = nCheapBefore;
            fWorkAfter = nCheapAfter + nExpensiveAfter*fSigcheckVerificationFactor;
        } else {
            double nCheapBefore = data.nTransactionsLastCheckpoint;
            double nExpensiveBefore = pindex->nChainTx - data.nTransactionsLastCheckpoint;
            double nExpensiveAfter = (nNow - pindex->nTime)/86400.0*data.fTransactionsPerDay;
            fWorkBefore = nCheapBefore + nExpensiveBefore*fSigcheckVerificationFactor;
            fWorkAfter = nExpensiveAfter*fSigcheckVerificationFactor;
        }

        return fWorkBefore / (fWorkBefore + fWorkAfter);
    }

    int GetTotalBlocksEstimate()
    {
        if (!fEnabled)
            return 0;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;
		//fake123
        return checkpoints.rbegin()->first;
        //return 0;
    }

    CBlockIndex* GetLastCheckpoint(const std::map<uint256, CBlockIndex*>& mapBlockIndex)
    {
        if (!fEnabled)
            return NULL;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, checkpoints)
        {
            const uint256& hash = i.second;
            std::map<uint256, CBlockIndex*>::const_iterator t = mapBlockIndex.find(hash);
            if (t != mapBlockIndex.end())
                //fake123
                return t->second;
                //return NULL;
        }
        return NULL;
    }
}
