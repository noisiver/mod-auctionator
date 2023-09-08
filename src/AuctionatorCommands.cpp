
#include "ScriptMgr.h"
#include "Chat.h"
#include "Auctionator.h"

using namespace Acore::ChatCommands;

class AuctionatorCommands : public CommandScript
{
    public:
        AuctionatorCommands() : CommandScript("AuctionatorCommandScript") { }

    private:
        std::vector<ChatCommand> GetCommands() const override
        {
            return std::vector<ChatCommand>
            {
               {
                  "auctionator",
                  SEC_GAMEMASTER,
                  true,
                  &HandleCommandOptions,
                  ""
               }
            };
        }

        static bool HandleCommandOptions(ChatHandler* handler, const char* args)
        {
            if(!*args)
            {
                handler->SendSysMessage("Auctionator commands:");
                handler->SendSysMessage("    auctionator list");
                return true;
            }

            return true;
        }

};

void AddAuctionatorCommands()
{
    new AuctionatorCommands();
}
