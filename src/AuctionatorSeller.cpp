#include "Auctionator.h"
#include "AuctionHouseMgr.h"
#include "AuctionatorSeller.h"
#include "Item.h"
#include "DatabaseEnv.h"
#include "PreparedStatement.h"
#include <random>


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

void AuctionatorSeller::LetsGetToIt(uint32 maxCount, uint32 houseId)
{

    // Set the maximum number of items to query for. Changing this <might>
    // affect how random our auctoin listing are at the cost of memory/cpu
    // but it is something i need to test.
    uint32 queryLimit = nator->config->sellerConfig.queryLimit;

    // Get the name of the character database so we can do our join below.
    std::string characterDbName = CharacterDatabase.GetConnectionInfo()->database;

    // soooo, let's talk query.
    // I like doing this with a query because it's easy to me but this isn't
    // really how acore works AND this makes this module completely incompatible
    // with Trinitycore and other cores that don't have a template table and instead
    // rely on the DBC files. I really should use the memory store of templates
    // for this but it would mean a TON of extra knarly c++ code that i just don't
    // want to write. If you want to run this on trinity or want to write that nasty
    // c++ in a trinity specific iffy block thing and submit a PR i would be happy
    // to consider it, but left on my own I am going to stick with this sql query.
    std::string itemQuery = R"(
        SELECT
            it.entry
            , it.name
            , it.BuyPrice
            , it.stackable
            , it.quality
            , mp.average_price
        FROM
            mod_auctionator_itemclass_config aicconf
            LEFT JOIN item_template it ON
                aicconf.class = it.class
                AND aicconf.subclass = it.subclass
                -- skip BoP
                AND it.bonding != 1
                AND (
                    it.bonding >= aicconf.bonding
                    OR it.bonding = 0
                )
            LEFT JOIN mod_auctionator_disabled_items dis on it.entry = dis.item
            LEFT JOIN (
                -- this sub query lets us get the current count of each item already in the AH
                -- so that we can filter out any items where itemCount >= max_count and not add
                -- anymore of them.
                SELECT
                    count(ii.itemEntry) as itemCount
                    , ii.itemEntry AS itemEntry
                FROM
                    acore_characters.item_instance ii
                    INNER JOIN {}.auctionhouse ah ON ii.guid = ah.itemguid
                    LEFT JOIN item_template it ON ii.itemEntry = it.entry
                WHERE ah.houseId = {}
                GROUP BY ii.itemEntry, it.name
            ) ic ON ic.itemEntry = it.entry
            LEFT JOIN {}.mod_auctionator_market_price mp ON it.entry = mp.entry
        WHERE
            -- filter out items from the disabled table
            dis.item IS NULL
            -- filter out items with 'depreacted' anywhere in the name
            AND it.name NOT LIKE '%deprecated%'
            -- filter out items that start with 'Test'
            AND it.name NOT LIKE 'Test%'
            AND it.name NOT LIKE 'NPC %'
            -- filter out items where we are already at or above max_count for uniques in this class to limit dups
            AND (ic.itemCount IS NULL OR ic.itemCount < aicconf.max_count)
            AND VerifiedBuild != 1
        ORDER BY RAND()
        LIMIT {}
        ;
    )";

    QueryResult result = WorldDatabase.Query(
        itemQuery,
        characterDbName,
        houseId,
        characterDbName,
        queryLimit
    );

    if (!result)
    {
        nator->logDebug("No results for LetsGo item query");
        return;
    }

    AuctionatorPriceMultiplierConfig multiplierConfig = nator->config->multipliers;
    uint32 count = 0;
    do
    {
        count++;
        Field* fields = result->Fetch();

        // TODO: refactor listing an item into a testable method
        std::string itemName = fields[1].Get<std::string>();

        uint32 stackSize = fields[3].Get<uint32>();

        uint32 quality = fields[4].Get<uint32>();
        float qualityMultiplier = Auctionator::GetQualityMultiplier(multiplierConfig, quality);

        uint32 price = fields[2].Get<uint32>();
        if (price == 0) {
            price = 10000000 * qualityMultiplier;
        }


        AuctionatorItem newItem = AuctionatorItem();
        newItem.itemId = fields[0].Get<uint32>();
        newItem.quantity = 1;
        newItem.buyout = uint32(price * stackSize * qualityMultiplier);
        newItem.time = 60 * 60 * 12;
        newItem.stackSize = stackSize;

        nator->logDebug("Adding item: " + itemName
            + " with quantity of " + std::to_string(newItem.quantity)
            + " at price of " +  std::to_string(newItem.buyout)
            + " to house " + std::to_string(houseId)
        );


        nator->CreateAuction(newItem, houseId);
        if (count == maxCount) {
            break;
        }
    } while (result->NextRow());

    nator->logInfo("Items added houseId("
        + std::to_string(houseId)
        + ") this run: " + std::to_string(count));

};
