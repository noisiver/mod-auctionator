
#ifndef AUCTIONATOR_H
#define AUCTIONATOR_H

#include "Common.h"
#include "ObjectGuid.h"

struct AuctionEntry;
class Player;
class WorldSession;

#include "ItemTemplate.h"
#include "AuctionatorConfig.h"
#include "AuctionHouseMgr.h"

class Auctionator
{
    private:
        Auctionator();
        AuctionHouseObject* HordeAh;
        AuctionHouseObject* AllianceAh;
        AuctionHouseObject* NeutralAh;
        AuctionHouseEntry const* HordeAhEntry;
        AuctionHouseEntry const* AllianceAhEntry;
        AuctionHouseEntry const* NeutralAhEntry;
        WorldSession *session;
        Player *AhPlayer;
        void logInfo(std::string message);

    public:
        ~Auctionator();
        void CreateAuction(uint32 itemId);
        void InitializeConfig();
        AuctionatorConfig *config;

        static Auctionator *getInstance()
        {
            static Auctionator instance;
            return &instance;
        }
};

#define gAuctionator Auctionator::getInstance()

#endif