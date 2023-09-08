
#include "ScriptMgr.h"
#include "WorldSession.h"
#include "Config.h"
#include "Chat.h"
#include "Auctionator.h"

class AuctionatorScript : public WorldScript
{
public:
    AuctionatorScript() : WorldScript("Auctionator_WorldScript") { }

    void OnStartup() override
    {
        LOG_INFO("server.loading", "[Auctionator]: Auctionator initializing...");
        if (gAuctionator->config->isEnabled) {
            LOG_INFO("server.loading", "[Auctionator]: Auctionator enabled.");
        } else {
            LOG_INFO("server.loading", "[Auctionator]: Auctionator disabled.");
        }

        gAuctionator->CreateAuction(40896);
    }
};

void AddAuctionatorScripts()
{
    new AuctionatorScript();
}
