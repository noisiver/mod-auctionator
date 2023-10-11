#include "AuctionatorBidder.h"
#include "Auctionator.h"
#include "ObjectMgr.h"


AuctionatorBidder::AuctionatorBidder(Auctionator* natorParam, uint32 auctionHouseIdParam)
{
    nator = natorParam;
    auctionHouseId = auctionHouseIdParam;
    ahMgr = nator->GetAuctionMgr(auctionHouseId);
}

AuctionatorBidder::~AuctionatorBidder()
{

}

void AuctionatorBidder::SpendSomeCash()
{
    uint32 auctionatPlayerGuid = nator->config->characterGuid;

    std::string query = "SELECT id FROM auctionhouse WHERE itemowner <> {}; ";

    QueryResult result = CharacterDatabase.Query(query, auctionatPlayerGuid);

    if (!result) {
        return;
    }

    if (result->GetRowCount() == 0) {
        return;
    }

    std::vector<uint32> biddableAuctionIds;
    do {
        biddableAuctionIds.push_back(result->Fetch()->Get<uint32>());
    } while(result->NextRow());

    nator->logInfo("Found " + std::to_string(biddableAuctionIds.size()) + " biddable auctions");

    uint32 purchasePerCycle = 1;

    while(purchasePerCycle > 0 && biddableAuctionIds.size() > 0) {
        AuctionEntry* auction = GetAuctionForPurchase(biddableAuctionIds);

        if (auction == nullptr) {
            return;
        }

        Item *item = sAuctionMgr->GetAItem(auction->item_guid);
        ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(auction->item_template);

        nator->logInfo("Buying auction: " + itemTemplate->Name1 + "(" + std::to_string(auction->Id) + ")");

        bool success = false;

        if (auction->buyout > 0) {
            success = BuyoutAuction(auction);
        } else {
            success = BidOnAuction(auction);
        }

        if (!success) {
            purchasePerCycle--;
        }
    }

}

AuctionEntry* AuctionatorBidder::GetAuctionForPurchase(std::vector<uint32> auctionIds)
{
    if (auctionIds.size() == 0) {
        return nullptr;
    }

    uint32 auctionId = auctionIds[0];
    auctionIds.erase(auctionIds.begin());

    AuctionEntry* auction = ahMgr->GetAuction(auctionId);
    return auction;
}

bool AuctionatorBidder::BidOnAuction(AuctionEntry* auction)
{
    return true;
}

bool AuctionatorBidder::BuyoutAuction(AuctionEntry* auction)
{
    return true;
}
