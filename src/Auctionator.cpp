
#include "Log.h"
#include "Auctionator.h"
#include "Config.h"
#include "WorldSession.h"
#include "AuctionHouseMgr.h"
#include "ObjectMgr.h"
#include "DatabaseEnv.h"
#include "Player.h"
#include "AuctionatorConfig.h"
#include <vector>

Auctionator::Auctionator()
{
    InitializeConfig();
};

Auctionator::~Auctionator()
{}

void Auctionator::CreateAuction(uint32 itemId)
{
    //int32 itemId = 40896; // glyph of frenzied regeneration

    // will need this when we want to know details of the item for filtering
    // ItemTemplate const* prototype = sObjectMgr->GetItemTemplate(itemId);


    Player player(session);
    player.Initialize(config->characterGuid);
    ObjectAccessor::AddObject(&player);

    logInfo("Creating Auction for item: " + std::to_string(itemId));
    // Create the item (and add it to the update queue for the player ")
    Item* item = Item::CreateItem(itemId, 1, &player);

    logInfo("adding item to player queue");
    item->AddToUpdateQueueOf(&player);
    uint32 randomPropertyId = Item::GenerateItemRandomPropertyId(itemId);
    if (randomPropertyId != 0) {
        logInfo("adding random properties");
        item->SetItemRandomProperties(randomPropertyId);
    }

    logInfo("starting character transaction");
    auto trans = CharacterDatabase.BeginTransaction();

    logInfo("creating auction entry");
    AuctionEntry* auctionEntry = new AuctionEntry();
    auctionEntry->Id = sObjectMgr->GenerateAuctionID();
    auctionEntry->houseId = AUCTIONHOUSE_HORDE;
    auctionEntry->item_guid = item->GetGUID();
    auctionEntry->item_template = item->GetEntry();
    auctionEntry->itemCount = item->GetCount();
    auctionEntry->owner = player.GetGUID();
    auctionEntry->startbid = 1000;
    auctionEntry->buyout = 2000;
    auctionEntry->bid = 0;
    auctionEntry->deposit = 100;
    auctionEntry->expire_time = (time_t) 172800 + time(NULL);
    auctionEntry->auctionHouseEntry = HordeAhEntry;

    logInfo("savem item to db");
    item->SaveToDB(trans);

    logInfo("removed from character queue");
    item->RemoveFromUpdateQueueOf(&player);

    logInfo("add item to auction mgr");
    sAuctionMgr->AddAItem(item);
 
    logInfo("add item to auction house");
    HordeAh->AddAuction(auctionEntry);
 
    logInfo("save auction entry");
    auctionEntry->SaveToDB(trans);
 
    logInfo("commit character transaction");
    CharacterDatabase.CommitTransaction(trans);
}

void Auctionator::InitializeConfig()
{
    logInfo("Initializing Auctionator Config");

    config = new AuctionatorConfig(AUCTIONHOUSE_HORDE);
    config->isEnabled = sConfigMgr->GetOption<bool>("Auctionator.Enabled", false);
    logInfo("config->isEnabled: "
        + std::to_string(config->isEnabled));

    config->characterId = sConfigMgr->GetOption<bool>("Auctionator.CharacterId", 0);
    config->characterGuid = sConfigMgr->GetOption<bool>("Auctionator.CharacterGuid", 0);
    logInfo("CharacterIds: "
        + std::to_string(config->characterId)
        + "::"
        + std::to_string(config->characterGuid)
    );

    logInfo("Auctionator config initialized");

    int32 accountId = 1;
    int32 accountGuid = 1;
    std::string accountName = "Auctionator";

    HordeAh = 
        sAuctionMgr->GetAuctionsMap(AUCTIONHOUSE_HORDE);

    HordeAhEntry = 
        sAuctionHouseStore.LookupEntry(AUCTIONHOUSE_HORDE);

    WorldSession _session(accountId,
        std::move(accountName),
        nullptr, 
        SEC_PLAYER,
        sWorld->getIntConfig(CONFIG_EXPANSION),
        0,
        LOCALE_enUS,
        0,
        false,
        false,
        0
    );

    session = &_session;

    // Player playerObject(session);
    // AhPlayer = &playerObject;
    // AhPlayer->Initialize(accountGuid);
    // ObjectAccessor::AddObject(AhPlayer);
}

void Auctionator::logInfo(std::string message) {
    LOG_INFO("server.loading", "[Auctionator]: " + message); 
}
