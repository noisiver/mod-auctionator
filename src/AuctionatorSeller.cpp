#include "Auctionator.h"
#include "AuctionHouseMgr.h"
#include "AuctionatorSeller.h"
#include "Item.h"
#include "DatabaseEnv.h"
#include "PreparedStatement.h"


AuctionatorSeller::AuctionatorSeller(Auctionator* natorParam, uint32 auctionHouseIdParam)
{
    nator = natorParam;
    auctionHouseId = auctionHouseIdParam;

    ahMgr = nator->GetAuctionMgr(auctionHouseId);
};

AuctionatorSeller::~AuctionatorSeller()
{
    // TODO: clean up
};

void AuctionatorSeller::LetsGetToIt(uint32 itemClass, uint32 subClass, uint32 maxCount)
{
    std::string itemQuery = R"(
        SELECT
            it.entry
            , it.name
            , it.BuyPrice
            , aic.name as classname
            , aic2.name as subclassname
            , aiq.name
            , it.RequiredLevel 
        FROM
            item_template it
        LEFT JOIN auctionator_item_class aic ON it.class = aic.class AND aic.subclass IS NULL
        LEFT JOIN auctionator_item_class aic2 on it.class = aic2.class AND it.subclass = aic2.subclass
        LEFT JOIN auctionator_item_quality aiq ON it.Quality = aiq.quality
        WHERE
            1
            AND aic.class = {}
            AND aic2.subclass = {}
            AND it.bonding = 0
            AND it.Flags != 16
            AND it.BuyPrice > 0
        ORDER BY RAND()
        LIMIT {}
        ;
        )";

    QueryResult result = WorldDatabase.Query(
        itemQuery,
        itemClass,
        subClass,
        maxCount
    );

    if (!result)
    {
        nator->logInfo("No results for LetsGo item query");
        return;
    }

    uint32 count = 0;

    do
    {
        count++;
        Field* fields = result->Fetch();
        std::string itemName = fields[1].Get<std::string>();

        AuctionatorItem newItem = AuctionatorItem();
        newItem.itemId = fields[0].Get<uint32>();
        newItem.quantity = 1;
        newItem.buyout = fields[2].Get<uint32>();
        newItem.time = 60 * 60 * 12;

        nator->CreateAuction(newItem);
        if (count > 5) {
            break;
        }
    } while (result -> NextRow());

    nator->logInfo("LetsGo count: " + std::to_string(count));

};
