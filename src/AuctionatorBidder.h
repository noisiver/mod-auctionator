
#ifndef AUCTIONATORBIDDER_H
#define AUCTIONATORBIDDER_H

#include "Auctionator.h"

class AuctionatorBidder
{
    private:
        Auctionator* nator;
        uint32 auctionHouseId;
        AuctionHouseObject* ahMgr;
        ObjectGuid buyerGuid;

    public:
        AuctionatorBidder(Auctionator* natorParam, uint32 auctionHouseIdParam);
        ~AuctionatorBidder();
        void SpendSomeCash();
        AuctionEntry* GetAuctionForPurchase(std::vector<uint32>& biddableAuctionIds);

        bool BidOnAuction(AuctionEntry* auction, Item* item, ItemTemplate const* itemTemplate);
        bool BuyoutAuction(AuctionEntry* auction, Item* item, ItemTemplate const* itemTemplate);
 };

#endif  //AUCTIONATORBIDDER_H
