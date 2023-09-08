
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
        LOG_INFO("server.loading", "Auctionator initializing...");
        if (sConfigMgr->GetOption<bool>("Auctionator.Enabled", true, true)) {
            LOG_INFO("server.loading", "Auctionator disabled.");
        } else {
            LOG_INFO("server.loading", "Auctionator enabled.");
        }

        gAuctionator->InitializeConfig();

        LOG_INFO("server.loading", "Adding item");
        gAuctionator->CreateAuction();
        LOG_INFO("server.loading", "Item added");
    }
};

void AddAuctionatorScripts()
{
    new AuctionatorScript();
}
