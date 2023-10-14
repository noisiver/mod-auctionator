
#ifndef AUCTIONATOR_EVENTS_H
#define AUCTIONATOR_EVENTS_H

#include "EventMap.h"
#include <unordered_map>
#include <functional>

using FunctionType = void (*)();

class AuctionatorEvents
{
    private:
        EventMap events;
        std::unordered_map<std::string, std::function<void()>> eventFunctions;
        std::unordered_map<uint32, std::string> eventToFunction = {
            {1, "AllianceBidder"},
            {2, "HordeBidder"},
            {3, "NeutralBidder"}
        };

    public:
        AuctionatorEvents();
        ~AuctionatorEvents();
        void InitializeEvents();
        void ExecuteEvents();
        void Update(uint32 deltaSeconds = 60);
        void logInfo(std::string message);
        void EventAllianceBidder();
        void EventHordeBidder();
        void EventNeutralBidder();

};

#endif
