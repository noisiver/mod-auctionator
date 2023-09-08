
#include <iostream>
#include <string>
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
            gAuctionator->logInfo("Excecuting Auctionator!");

            const char* command = strtok((char*)args, " ");

            if(!command)
            {
                handler->SendSysMessage("Auctionator commands:");
                handler->SendSysMessage("    auctionator list");
                handler->SendSysMessage("    auctionator add");
                return true;
            }

            const char* param1 = strtok(NULL, " ");
            const char* param2 = strtok(NULL, " ");
            const char* param3 = strtok(NULL, " ");
            const char* param4 = strtok(NULL, " ");

            if(strlen(command) == 0) {
                return true;
            }

            std::string commandString(command);

            if (commandString.compare("add") == 0) {
                gAuctionator->logInfo("Adding new Item for GM");
                uint32 auctionHouseId = std::stoi(param1);
                uint32 itemId = std::stoi(param2);
                uint32 price = std::stoi(param3);
                AuctionatorCommands::AddItemForBuyout(auctionHouseId, itemId, price);
            } else if (command == "list") {

            } 

            return true;
        }

        static void AddItemForBuyout(uint32 auctionHouseId, uint32 itemId, uint32 price)
        {
            AuctionatorItem newItem;
            newItem.houseId = auctionHouseId;
            newItem.itemId = itemId;
            newItem.buyout = price;

            gAuctionator->CreateAuction(newItem);
        } 

};

void AddAuctionatorCommands()
{
    new AuctionatorCommands();
}
