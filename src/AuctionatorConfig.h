
#ifndef AUCTIONATOR_CONFIG_H
#define AUCTIONATOR_CONFIG_H

#include "Common.h"

struct AuctionatorHouseConfig
{
    public:
        uint32 enabled = 0;
        uint32 maxAuctions = 100;
};

struct AuctionatorBidderConfig
{
    public:
        uint32 enabled;
        uint32 cycleMinutes;
        uint32 maxPerCycle;
};

class AuctionatorConfig
{
    private:

    public:
        AuctionatorConfig() {};
        bool isEnabled = false;
        uint32 characterId = 0;
        uint32 characterGuid = 0;
        uint32 auctionHouseId = 7;

        AuctionatorHouseConfig hordeSeller;
        AuctionatorHouseConfig allianceSeller;
        AuctionatorHouseConfig neutralSeller;

        AuctionatorBidderConfig allianceBidder;
        AuctionatorBidderConfig hordeBidder;
        AuctionatorBidderConfig neutralBidder;
};

#endif
