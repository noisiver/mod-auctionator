
#include "Log.h"
#include "Auctionator.h"
#include "Config.h"
#include "WorldSession.h"
#include "AuctionHouseMgr.h"
#include "GameTime.h"
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

    logDebug("Creating Auction for item: " + std::to_string(newItem.itemId));
    // Create the item (and add it to the update queue for the player ")
    Item* item = Item::CreateItem(newItem.itemId, 1, &player);

    logTrace("adding item to player queue");
    item->AddToUpdateQueueOf(&player);
    uint32 randomPropertyId = Item::GenerateItemRandomPropertyId(newItem.itemId);
    if (randomPropertyId != 0) {
        logDebug("adding random properties");
        item->SetItemRandomProperties(randomPropertyId);
    }

    logTrace("starting character transaction");
    auto trans = CharacterDatabase.BeginTransaction();

    // set our quantity. If this is a stack it needs to get set here
    // on the item instance and not on the auction item.
    item->SetCount(1);
    if (newItem.stackSize > 1) {
        item->SetCount(newItem.stackSize);
    }

    logTrace("creating auction entry");
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

    logTrace("savem item to db");
    item->SaveToDB(trans);

    logTrace("removed from character queue");
    item->RemoveFromUpdateQueueOf(&player);

    logTrace("add item to auction mgr");
    sAuctionMgr->AddAItem(item);
 
    logTrace("add item to auction house");
    HordeAh->AddAuction(auctionEntry);
 
    logTrace("save auction entry");
    auctionEntry->SaveToDB(trans);
 
    logTrace("commit character transaction");
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
    uint32 maxAuctions = 10000;

    logDebug("Auctionator tick");

    logInfo("Neutral count: " + std::to_string(NeutralAh->Getcount()));
    logInfo("Alliance count: " + std::to_string(AllianceAh->Getcount()));
    logInfo("Horde count: " + std::to_string(HordeAh->Getcount()));

    AuctionatorSeller seller = 
        AuctionatorSeller(gAuctionator, static_cast<uint32>(AUCTIONHOUSE_HORDE));

    uint32 auctionCountHorde = gAuctionator->HordeAh->Getcount();

    if (auctionCountHorde <= maxAuctions) {
        gAuctionator->logInfo(
            "Count is good, here we go: "
            + std::to_string(auctionCountHorde)
            + " of " + std::to_string(maxAuctions)
        );

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

void Auctionator::ExpireAllAuctions(uint32 houseId)
{
    if (houseId != AUCTIONHOUSE_ALLIANCE &&
        houseId != AUCTIONHOUSE_HORDE &&
        houseId != AUCTIONHOUSE_NEUTRAL
    ) {
        logDebug("Invalid houseId: " + std::to_string(houseId)); 
        return;
    }

    logDebug("Clearing auctions for houseId: " + std::to_string(houseId));

    // std::string query = R"(
    //     UPDATE acore_characters.auctionhouse
    //     SET time = 0
    //     WHERE 
    //         houseId = {}
    //         AND itemowner = {}
    // )";

    // CharacterDatabase.Query(
    //     query,
    //     houseId,
    //     gAuctionator->config->characterGuid
    // );

    time_t checkTime = GameTime::GetGameTime().count() + 60;
    ///- Handle expired auctions

    AuctionHouseObject* ah = sAuctionMgr->GetAuctionsMapByHouseId(houseId);
    for (
        AuctionHouseObject::AuctionEntryMap::iterator itr,
        iter = ah->GetAuctionsBegin();
        iter != ah->GetAuctionsEnd(); 
        )
    {
        itr = iter++;
        AuctionEntry* auction = (*itr).second;
        logDebug("got one! " + std::to_string(auction->expire_time));
        auction->expire_time = 0;
    }

    logDebug("House auctions expired: " + std::to_string(houseId));
}

void Auctionator::logInfo(std::string message) {
    LOG_INFO("auctionator", "[Auctionator]: " + message); 
}

void Auctionator::logDebug(std::string message) {
    LOG_DEBUG("auctionator", "[Auctionator]: " + message); 
}

void Auctionator::logTrace(std::string message) {
    LOG_TRACE("auctionator", "[Auctionator]: " + message);
}
