
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

void Auctionator::CreateAuction()
{
    int32 accountId = 1;
    int32 accountGuid = 1;
    std::string accountName = "Auctionator";

    AuctionHouseObject* auctionHouse = 
        sAuctionMgr->GetAuctionsMap(AUCTIONHOUSE_HORDE);

    AuctionHouseEntry const* auctionHouseEntry = 
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

    Player player(&_session);
    player.Initialize(accountGuid);
    ObjectAccessor::AddObject(&player);

    int32 itemId = 40896; // glyph of frenzied regeneration

    // will need this when we want to know details of the item for filtering
    // ItemTemplate const* prototype = sObjectMgr->GetItemTemplate(itemId);

    Item* item = Item::CreateItem(itemId, 1, &player);
    item->AddToUpdateQueueOf(&player);
    uint32 randomPropertyId = Item::GenerateItemRandomPropertyId(itemId);
    if (randomPropertyId != 0) {
        item->SetItemRandomProperties(randomPropertyId);
    }

    auto trans = CharacterDatabase.BeginTransaction();

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
    auctionEntry->auctionHouseEntry = auctionHouseEntry;

    item->SaveToDB(trans);
    item->RemoveFromUpdateQueueOf(&player);

    sAuctionMgr->AddAItem(item);
 
    auctionHouse->AddAuction(auctionEntry);
 
    auctionEntry->SaveToDB(trans);
 
    CharacterDatabase.CommitTransaction(trans);
}

void Auctionator::InitializeConfig()
{
    LOG_DEBUG("server.loading", "Initializing Auctionator Config");

    config = new AuctionatorConfig(AUCTIONHOUSE_HORDE);
    config->isEnabled = sConfigMgr->GetOption<bool>("Auctionator.Enabled", false);

    LOG_DEBUG("server.loading", "Auctionator config initialized");
}