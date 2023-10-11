
#ifndef AUCTIONATORBIDDER_H
#define AUCTIONATORBIDDER_H

#include "Auctionator.h"

class AuctionatorBidder
{
    private:
        Auctionator* nator;
        uint32 auctionHouseId;
        AuctionHouseObject* ahMgr;

    public:
        AuctionatorBidder(Auctionator* natorParam, uint32 auctionHouseIdParam);
        ~AuctionatorBidder();
        void SpendSomeCash();
        AuctionEntry* GetAuctionForPurchase(std::vector<uint32> biddableAuctionIds);
 };

#endif  //AUCTIONATORBIDDER_H
