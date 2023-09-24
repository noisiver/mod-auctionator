
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
                handler->SendSysMessage("    auctionator add");
                handler->SendSysMessage("    auctionator expireall");
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

            gAuctionator->logDebug("Executing command: " + commandString);

            if (commandString.compare("add") == 0) {
                gAuctionator->logInfo("Adding new Item for GM");
                uint32 auctionHouseId = std::stoi(param1);
                uint32 itemId = std::stoi(param2);
                uint32 price = std::stoi(param3);
                AuctionatorCommands::AddItemForBuyout(auctionHouseId, itemId, price);
            } else if (commandString == "expireall") {
                if (!param1) {
                    handler->SendSysMessage("[Auctionator] expireall: No Auction House Specified!");
                    gAuctionator->logInfo("expireall: No Auction House Specified!");
                    return true;
                }
                uint32 houseId = std::stoi(param1); 
                
                handler->SendSysMessage("[Auctionator] Expiring all Auctions for house: " + std::to_string(houseId));
                gAuctionator->logInfo("expireall: Expiring all auctions for house: " + std::to_string(houseId));

                gAuctionator->ExpireAllAuctions(houseId);

                gAuctionator->logInfo("expireall: All auctions expired for house: " + std::to_string(houseId));
            } else if (commandString == "enable") {
                if (!param1) {
                    handler->SendSysMessage("[Auctionator] enable: No Auction House Specified!");
                    gAuctionator->logInfo("enable: No Auction House Specified!");
                    return true;
                }

                std::string toEnable(param1);
                if (toEnable == "hordeseller") {
                    gAuctionator->config->hordeSellerEnabled = 1;
                    return true;
                } else if (toEnable == "allianceseller") {
                    gAuctionator->config->allianceSellerEnabled = 1;
                    return true;
                } else if (toEnable == "neutralseller") {
                    gAuctionator->config->neutralSellerEnabled = 1;
                    return true;
                } else if (toEnable == "all") {
                    gAuctionator->config->hordeSellerEnabled = 1;
                    gAuctionator->config->allianceSellerEnabled = 1;
                    gAuctionator->config->neutralSellerEnabled = 1;
                    return true;
                }
            } else if (commandString == "disable") {
                if (!param1) {
                    handler->SendSysMessage("[Auctionator] disable: No Auction House Specified!");
                    gAuctionator->logInfo("disable: No Auction House Specified!");
                    return true;
                }

                std::string toEnable(param1);
                if (toEnable == "hordeseller") {
                    gAuctionator->config->hordeSellerEnabled = 0;
                    return true;
                } else if (toEnable == "allianceseller") {
                    gAuctionator->config->allianceSellerEnabled = 0;
                    return true;
                } else if (toEnable == "neutralseller") {
                    gAuctionator->config->neutralSellerEnabled = 0;
                    return true;
                } else if (toEnable == "all") {
                    gAuctionator->config->hordeSellerEnabled = 0;
                    gAuctionator->config->allianceSellerEnabled = 0;
                    gAuctionator->config->neutralSellerEnabled = 0;
                    return true;
                }
            }


            return true;
        }

        static void AddItemForBuyout(uint32 auctionHouseId, uint32 itemId, uint32 price)
        {
            AuctionatorItem newItem;
            newItem.houseId = auctionHouseId;
            newItem.itemId = itemId;
            newItem.buyout = price;

            gAuctionator->CreateAuction(newItem, AUCTIONHOUSE_NEUTRAL);
        } 

};

void AddAuctionatorCommands()
{
    new AuctionatorCommands();
}
