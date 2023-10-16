
#ifndef AUCTIONATOR_CONFIG_H
#define AUCTIONATOR_CONFIG_H

#include "Common.h"

struct AuctionatorHouseConfig
{
    public:
        uint32 enabled;
        uint32 maxAuctions;
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
        bool isEnabled;
        uint32 characterId;
        uint32 characterGuid;
        uint32 auctionHouseId;

        AuctionatorHouseConfig hordeSeller;
        AuctionatorHouseConfig allianceSeller;
        AuctionatorHouseConfig neutralSeller;

        AuctionatorBidderConfig allianceBidder;
        AuctionatorBidderConfig hordeBidder;
        AuctionatorBidderConfig neutralBidder;
};

#endif
