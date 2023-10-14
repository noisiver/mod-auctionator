
#include "AuctionatorEvents.h"
#include "Log.h"
#include <functional>

AuctionatorEvents::AuctionatorEvents()
{
    events = EventMap();
    InitializeEvents();
}

AuctionatorEvents::~AuctionatorEvents()
{}

void AuctionatorEvents::InitializeEvents()
{
    // eventFunctions[1] = AllianceBuyer;

    logInfo("Initializing events");

    events.ScheduleEvent(1, 3);
    events.ScheduleEvent(2, 3);
    events.ScheduleEvent(3, 2);

    eventFunctions["AllianceBidder"] =
        std::bind(&AuctionatorEvents::EventAllianceBidder, this);
    eventFunctions["HordeBidder"] =
        std::bind(&AuctionatorEvents::EventHordeBidder, this);
    eventFunctions["NeutralBidder"] =
        std::bind(&AuctionatorEvents::EventNeutralBidder, this);
}

void AuctionatorEvents::ExecuteEvents()
{
    logInfo("Executing events");
    uint32 currentEvent = events.ExecuteEvent();
    while (currentEvent != 0) {
        logInfo("Executing event: " + eventToFunction[currentEvent]);

        if (eventToFunction[currentEvent] != "") {
            try {
                eventFunctions[eventToFunction[currentEvent]]();
            } catch(const std::exception& e) {
                logInfo("Attempt to call event with no handler");
            }
        }

        events.ScheduleEvent(currentEvent, 3);

        currentEvent = events.ExecuteEvent();
    }
}

void AuctionatorEvents::Update(uint32 deltaSeconds)
{
    events.Update(deltaSeconds);
    ExecuteEvents();
}

void AuctionatorEvents::logInfo(std::string message) {
    LOG_INFO("auctionator", "[AuctionatorEvents]: " + message);
}

void AuctionatorEvents::EventAllianceBidder()
{
    logInfo("Executing Alliance Bidder");
}

void AuctionatorEvents::EventHordeBidder()
{
    logInfo("Executing Horde Bidder");
}

void AuctionatorEvents::EventNeutralBidder()
{
    logInfo("Executing Neutral Bidder");
}
