
#ifndef AUCTIONATOR_CONFIG_H
#define AUCTIONATOR_CONFIG_H

#include "Common.h"

class AuctionatorConfig
{
    private:

    public:
        AuctionatorConfig();
        bool isEnabled;
        uint32 characterId;
        uint32 characterGuid;
        uint32 auctionHouseId;
        uint32 internalHouseId;

        uint32 hordeSellerEnabled;
        uint32 allianceSellerEnabled;
        uint32 neutralSellerEnabled;

        AuctionatorConfig(uint32 id)
        {
            auctionHouseId = id;
            switch(auctionHouseId)
            {
                case 2:
                    internalHouseId = 55;
                    break;
                case 6:
                    internalHouseId = 29;
                    break;
                case 7:
                default:
                    internalHouseId = 120;
                    break;
            }
        }

};

#endif
