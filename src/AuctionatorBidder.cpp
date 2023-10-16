#include "AuctionatorBidder.h"
#include "Auctionator.h"
#include "ObjectMgr.h"
#include <random>


AuctionatorBidder::AuctionatorBidder(uint32 auctionHouseIdParam, ObjectGuid buyer, AuctionatorConfig* auctionatorConfig)
{
    SetLogPrefix("[AuctionatorBidder] ");
    auctionHouseId = auctionHouseIdParam;
    buyerGuid = buyer;
    ahMgr = sAuctionMgr->GetAuctionsMapByHouseId(auctionHouseId);
    config = auctionatorConfig;
    bidOnOwn = 0;
}

AuctionatorBidder::~AuctionatorBidder()
{

}

void AuctionatorBidder::SpendSomeCash()
{
    uint32 auctionatorPlayerGuid = buyerGuid.GetRawValue();

    std::string query = "SELECT id FROM auctionhouse WHERE itemowner <> {} AND houseid = {}; ";

    // for testing we may want to bid on our own auctions.
    // if we do we set the ownerToSkip to 0 so we will pick
    // up all auctions including our own.
    uint32 ownerToSkip = auctionatorPlayerGuid;
    if (bidOnOwn) {
        ownerToSkip = 0;
    }

    QueryResult result = CharacterDatabase.Query(query, ownerToSkip, auctionHouseId);

    if (!result) {
        logInfo("Can't see player auctions, moving on.");
        return;
    }

    if (result->GetRowCount() == 0) {
        logInfo("No player auctions, taking my money elsewhere.");
        return;
    }

    std::vector<uint32> biddableAuctionIds;
    do {
        biddableAuctionIds.push_back(result->Fetch()->Get<uint32>());
    } while(result->NextRow());

    // shuffle our vector to try to inject some randomness
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(
        biddableAuctionIds.begin(),
        biddableAuctionIds.end(),
        std::default_random_engine(seed)
    );

    logInfo("Found " + std::to_string(biddableAuctionIds.size()) + " biddable auctions");

    uint32 purchasePerCycle = GetAuctionsPerCycle();
    uint32 counter = 0;
    uint32 total = biddableAuctionIds.size();

    while(purchasePerCycle > 0 && biddableAuctionIds.size() > 0) {
        counter++;
        AuctionEntry* auction = GetAuctionForPurchase(biddableAuctionIds);

        if (auction == nullptr) {
            return;
        }

        // Item *item = sAuctionMgr->GetAItem(auction->item_guid);
        ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(auction->item_template);

        logInfo("Considering auction: "
            + itemTemplate->Name1
            + " [AuctionId: " + std::to_string(auction->Id) + "]"
            + " [ItemId: " + std::to_string(itemTemplate->ItemId) + "]"
            + " <> " + std::to_string(counter) + " of "
            + std::to_string(total)
        );

        bool success = false;

        if (auction->buyout > 0) {
            success = BuyoutAuction(auction, itemTemplate);
        } else {
            success = BidOnAuction(auction, itemTemplate);
        }

        if (success) {
            purchasePerCycle--;
            logInfo("Purchase made, remaining: " + std::to_string(purchasePerCycle));
        }
    }

}

AuctionEntry* AuctionatorBidder::GetAuctionForPurchase(std::vector<uint32>& auctionIds)
{
    if (auctionIds.size() == 0) {
        return nullptr;
    }

    uint32 auctionId = auctionIds[0];
    auctionIds.erase(auctionIds.begin());

    logTrace("Auction removed, remaining items: " + std::to_string(auctionIds.size()));

    AuctionEntry* auction = ahMgr->GetAuction(auctionId);
    return auction;
}

bool AuctionatorBidder::BidOnAuction(AuctionEntry* auction, ItemTemplate const* itemTemplate)
{
    uint32 currentPrice;

    if (auction->bid) {
        if (auction->bidder == buyerGuid) {
            logInfo("Skipping auction, I have already bid: "
                + std::to_string(auction->bid) + ".");
        } else {
            logInfo("Skipping auction, someone else has already bid "
                + std::to_string(auction->bid) + ".");
        }
        return false;
    } else {
        currentPrice = auction->startbid;
    }

    if (currentPrice > itemTemplate->BuyPrice) {
        logInfo("Skipping auction ("
            + std::to_string(auction->Id) + "), price of "
            + std::to_string(currentPrice) + " is higher than template price of "
            + std::to_string(itemTemplate->BuyPrice)
        );
        return false;
    }

    uint32 bidPrice = currentPrice + (itemTemplate->BuyPrice - currentPrice) / 2;

    auction->bidder = buyerGuid;
    auction->bid = bidPrice;

    CharacterDatabase.Execute(R"(
            UPDATE
                auctionhouse
            SET
                buyguid = {},
                lastbid = {}.
            WHERE
                id = {}
        )",
        auction->bidder.GetCounter(),
        auction->bid,
        auction->Id
    );

    logInfo("Bid on auction of "
        + itemTemplate->Name1 + " ("
        + std::to_string(auction->Id) + ") of "
        + std::to_string(bidPrice) + " copper."
    );

    return true;
}

bool AuctionatorBidder::BuyoutAuction(AuctionEntry* auction, ItemTemplate const* itemTemplate)
{
    if (auction->buyout > itemTemplate->BuyPrice) {
        logInfo("Skipping buyout, price ("
            + std::to_string(auction->buyout) +") is higher than template buyprice "
            + std::to_string(itemTemplate->BuyPrice) +")");
        return false;
    }

    auto trans = CharacterDatabase.BeginTransaction();
    auction->bidder = buyerGuid;
    auction->bid = auction->buyout;

    sAuctionMgr->SendAuctionSuccessfulMail(auction, trans);
    auction->DeleteFromDB(trans);

    logInfo("Purchased auction of "
        + itemTemplate->Name1 + " ("
        + std::to_string(auction->Id) + ") for "
        + std::to_string(auction->buyout) + " copper."
    );

    sAuctionMgr->RemoveAItem(auction->item_guid);
    ahMgr->RemoveAuction(auction);

    CharacterDatabase.CommitTransaction(trans);

    return true;
}

uint32 AuctionatorBidder::GetAuctionsPerCycle()
{
    switch(auctionHouseId) {
        case AUCTIONHOUSE_ALLIANCE:
            return config->allianceBidder.maxPerCycle;
            break;
        case AUCTIONHOUSE_HORDE:
            return config->hordeBidder.maxPerCycle;
            break;
        case AUCTIONHOUSE_NEUTRAL:
            return config->neutralBidder.maxPerCycle;
            break;
        default:
            return 0;
    }
}
