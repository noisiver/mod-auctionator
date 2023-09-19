
#include "Log.h"
#include "Auctionator.h"
#include "Config.h"
#include "WorldSession.h"
#include "AuctionHouseMgr.h"
#include "ObjectMgr.h"
#include "DatabaseEnv.h"
#include "Player.h"
#include "AuctionatorConfig.h"
#include "AuctionatorSeller.h"
#include <vector>

Auctionator::Auctionator()
{
    InitializeConfig();
    Initialize();
};

Auctionator::~Auctionator()
{}

void Auctionator::CreateAuction(AuctionatorItem newItem)
{
    // will need this when we want to know details of the item for filtering
    // ItemTemplate const* prototype = sObjectMgr->GetItemTemplate(itemId);


    Player player(session);
    player.Initialize(config->characterGuid);
    ObjectAccessor::AddObject(&player);

    logInfo("Creating Auction for item: " + std::to_string(newItem.itemId));
    // Create the item (and add it to the update queue for the player ")
    Item* item = Item::CreateItem(newItem.itemId, 1, &player);

    // logInfo("adding item to player queue");
    item->AddToUpdateQueueOf(&player);
    uint32 randomPropertyId = Item::GenerateItemRandomPropertyId(newItem.itemId);
    if (randomPropertyId != 0) {
        logInfo("adding random properties");
        item->SetItemRandomProperties(randomPropertyId);
    }

    // logInfo("starting character transaction");
    auto trans = CharacterDatabase.BeginTransaction();

    // set our quantity. If this is a stack it needs to get set here
    // on the item instance and not on the auction item.
    item->SetCount(1);
    if (newItem.stackSize > 1) {
        item->SetCount(newItem.stackSize);
    }

    // logInfo("creating auction entry");
    AuctionEntry* auctionEntry = new AuctionEntry();
    auctionEntry->Id = sObjectMgr->GenerateAuctionID();
    auctionEntry->houseId = AUCTIONHOUSE_HORDE;
    auctionEntry->item_guid = item->GetGUID();
    auctionEntry->item_template = item->GetEntry();
    auctionEntry->owner = player.GetGUID();
    auctionEntry->startbid = newItem.bid;
    auctionEntry->buyout = newItem.buyout;
    auctionEntry->bid = 0;
    auctionEntry->deposit = 100;
    auctionEntry->expire_time = (time_t) newItem.time + time(NULL);
    auctionEntry->auctionHouseEntry = HordeAhEntry;

    // logInfo("savem item to db");
    item->SaveToDB(trans);

    // logInfo("removed from character queue");
    item->RemoveFromUpdateQueueOf(&player);

    // logInfo("add item to auction mgr");
    sAuctionMgr->AddAItem(item);
 
    // logInfo("add item to auction house");
    HordeAh->AddAuction(auctionEntry);
 
    // logInfo("save auction entry");
    auctionEntry->SaveToDB(trans);
 
    // logInfo("commit character transaction");
    CharacterDatabase.CommitTransaction(trans);

    ObjectAccessor::RemoveObject(&player);
}

void Auctionator::Initialize()
{
    std::string accountName = "Auctionator";

    HordeAh = sAuctionMgr->GetAuctionsMap(AUCTIONHOUSE_HORDE);
    HordeAhEntry = sAuctionHouseStore.LookupEntry(AUCTIONHOUSE_HORDE);

    AllianceAh = sAuctionMgr->GetAuctionsMap(AUCTIONHOUSE_ALLIANCE);
    AllianceAhEntry = sAuctionHouseStore.LookupEntry(AUCTIONHOUSE_ALLIANCE);

    NeutralAh = sAuctionMgr->GetAuctionsMap(AUCTIONHOUSE_NEUTRAL);
    NeutralAhEntry = sAuctionHouseStore.LookupEntry(AUCTIONHOUSE_NEUTRAL);

    WorldSession _session(
        config->characterId,
        std::move(accountName),
        nullptr, 
        SEC_GAMEMASTER,
        sWorld->getIntConfig(CONFIG_EXPANSION),
        0,
        LOCALE_enUS,
        0,
        false,
        false,
        0
    );

    session = &_session;
}

void Auctionator::InitializeConfig()
{
    logInfo("Initializing Auctionator Config");

    config = new AuctionatorConfig(AUCTIONHOUSE_HORDE);
    config->isEnabled = sConfigMgr->GetOption<bool>("Auctionator.Enabled", false);
    logInfo("config->isEnabled: "
        + std::to_string(config->isEnabled));

    config->characterId = sConfigMgr->GetOption<uint32>("Auctionator.CharacterId", 0);
    config->characterGuid = sConfigMgr->GetOption<uint32>("Auctionator.CharacterGuid", 0);
    logInfo("CharacterIds: "
        + std::to_string(config->characterId)
        + "::"
        + std::to_string(config->characterGuid)
    );

    logInfo("Auctionator config initialized");
}

/**
 * Update gets called on each "tick" of the global sAuctionHouseManager.
*/
void Auctionator::Update()
{
    logDebug("Auctionator tick");

    logInfo("Neutral count: " + std::to_string(NeutralAh->Getcount()));
    logInfo("Alliance count: " + std::to_string(AllianceAh->Getcount()));
    logInfo("Horde count: " + std::to_string(HordeAh->Getcount()));

    AuctionatorSeller seller = 
        AuctionatorSeller(gAuctionator, static_cast<uint32>(AUCTIONHOUSE_HORDE));

    uint32 auctionCountHorde = gAuctionator->HordeAh->Getcount();

    if (auctionCountHorde <= 10000) {
        gAuctionator->logInfo("Count is good, here we go: " + std::to_string(auctionCountHorde));
        // seller.LetsGetToIt(1, 0, 0, 2);
        // seller.LetsGetToIt(1, 0, 2, 2);
        seller.LetsGetToIt(100);
    } else {
        gAuctionator->logInfo("Count over max: " + std::to_string(auctionCountHorde));
    }
}

AuctionHouseObject* Auctionator::GetAuctionMgr(uint32 auctionHouseId)
{
    switch(auctionHouseId) {
        case AUCTIONHOUSE_ALLIANCE:
            return AllianceAh;
            break;
        case AUCTIONHOUSE_HORDE:
            return HordeAh;
            break;
        default:
            return NeutralAh;
            break;
    }
}

void Auctionator::logInfo(std::string message) {
    LOG_INFO("server.loading", "[Auctionator]: " + message); 
}

void Auctionator::logDebug(std::string message) {
    LOG_DEBUG("server.loading", "[Auctionator]: " + message); 
}


        // seller.LetsGetToIt(2, 0, 2, 2);
        // seller.LetsGetToIt(2, 2, 2, 2);
        // seller.LetsGetToIt(2, 3, 2, 2);
        // seller.LetsGetToIt(2, 4, 2, 2);
        // seller.LetsGetToIt(2, 6, 2, 2);
        // seller.LetsGetToIt(2, 7, 2, 2);
        // seller.LetsGetToIt(2, 10, 2, 2);
        // seller.LetsGetToIt(2, 11, 2, 2);
        // seller.LetsGetToIt(2, 13, 2, 2);
        // seller.LetsGetToIt(2, 14, 2, 2);
        // seller.LetsGetToIt(2, 15, 2, 2);
        // seller.LetsGetToIt(2, 16, 2, 2);
        // seller.LetsGetToIt(2, 17, 2, 2);
        // seller.LetsGetToIt(2, 18, 2, 2);
        // seller.LetsGetToIt(2, 19, 2, 2);
        // seller.LetsGetToIt(2, 20, 2, 2);

        // seller.LetsGetToIt(4, 1, 2, 2);
        // seller.LetsGetToIt(4, 2, 2, 2);
        // seller.LetsGetToIt(4, 3, 2, 2);
        // seller.LetsGetToIt(4, 4, 2, 2);
        // seller.LetsGetToIt(4, 5, 2, 2);
        // seller.LetsGetToIt(4, 6, 2, 2);
        // seller.LetsGetToIt(4, 7, 2, 2);
        // seller.LetsGetToIt(4, 8, 2, 2);
        // seller.LetsGetToIt(4, 9, 2, 2);
        // seller.LetsGetToIt(4, 10, 2, 2);
