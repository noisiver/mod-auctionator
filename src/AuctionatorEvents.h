
#ifndef AUCTIONATOR_EVENTS_H
#define AUCTIONATOR_EVENTS_H

#include "AuctionatorBase.h"
#include "AuctionatorConfig.h"
#include "EventMap.h"
#include "ObjectMgr.h"
#include <unordered_map>
#include <functional>

using FunctionType = void (*)();

class AuctionatorEvents : public AuctionatorBase
{
    private:
        ObjectGuid auctionatorGuid;
        EventMap events;
        std::unordered_map<std::string, std::function<void()>> eventFunctions;
        std::unordered_map<uint32, std::string> eventToFunction;
        AuctionatorConfig* config;

    public:
        AuctionatorEvents() {};
        AuctionatorEvents(AuctionatorConfig* auctionatorConfig);
        ~AuctionatorEvents();
        void InitializeEvents();
        void ExecuteEvents();
        void Update(uint32 deltaMinutes = 1);
        void EventAllianceBidder();
        void EventHordeBidder();
        void EventNeutralBidder();
        void SetPlayerGuid(ObjectGuid playerGuid);
        EventMap GetEvents();
};

#endif
