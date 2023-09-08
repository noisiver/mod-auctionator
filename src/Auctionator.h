
#ifndef AUCTIONATOR_H
#define AUCTIONATOR_H

#include "Common.h"
#include "ObjectGuid.h"

struct AuctionEntry;
class Player;
class WorldSession;

#include "ItemTemplate.h"
#include "AuctionatorConfig.h"

class Auctionator
{
    private:
        Auctionator();
        AuctionatorConfig *config;

    public:
        ~Auctionator();
        void CreateAuction();
        void InitializeConfig();

        static Auctionator *getInstance()
        {
            static Auctionator instance;
            return &instance;
        }
};

#define gAuctionator Auctionator::getInstance()

#endif