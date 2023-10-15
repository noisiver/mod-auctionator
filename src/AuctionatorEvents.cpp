
#include "AuctionatorEvents.h"
#include "AuctionatorBidder.h"
#include "Log.h"
#include <functional>

AuctionatorEvents::AuctionatorEvents()
{
    SetLogPrefix("[AuctionatorEvents] ");
    events = EventMap();
    InitializeEvents();
}

AuctionatorEvents::~AuctionatorEvents()
{}

void AuctionatorEvents::InitializeEvents()
{
    logInfo("Initializing events");

    // register the functions for each of our events
    // these strings are mapped to the events below
    eventFunctions["AllianceBidder"] =
        std::bind(&AuctionatorEvents::EventAllianceBidder, this);
    eventFunctions["HordeBidder"] =
        std::bind(&AuctionatorEvents::EventHordeBidder, this);
    eventFunctions["NeutralBidder"] =
        std::bind(&AuctionatorEvents::EventNeutralBidder, this);

    // map our event functions from above to our events below
    eventToFunction = {
            {1, "AllianceBidder"},
            {2, "HordeBidder"},
            {3, "NeutralBidder"}
        };
    //
    // schedule the events for our bidders
    //

    // AllianceBidder
    events.ScheduleEvent(1, 3);
    // HordeBidder
    events.ScheduleEvent(2, 2);
    // NeutralBidder
    events.ScheduleEvent(3, 1);
}

void AuctionatorEvents::ExecuteEvents()
{
    logInfo("Executing events");
    uint32 currentEvent = events.ExecuteEvent();
    while (currentEvent != 0) {
        logInfo("Executing event: " + eventToFunction[currentEvent]);

        if (eventToFunction[currentEvent] != "") {
            try {
                // eventFunctions[eventToFunction[currentEvent]]();
                // if (eventToFunction[currentEvent] == "NeutralBidder") {
                //     EventNeutralBidder();
                // }
                switch(currentEvent) {
                    case 1:
                        EventAllianceBidder();
                        events.ScheduleEvent(currentEvent, 3);
                        break;
                    case 2:
                        EventHordeBidder();
                        events.ScheduleEvent(currentEvent, 2);
                        break;
                    case 3:
                        EventNeutralBidder();
                        events.ScheduleEvent(currentEvent, 1);
                        break;
                }
            } catch(const std::exception& e) {
                logError("Issue calling handler");
                logError(e.what());
            }
        }

        currentEvent = events.ExecuteEvent();
    }
}

void AuctionatorEvents::Update(uint32 deltaMinutes)
{
    events.Update(deltaMinutes);
    ExecuteEvents();
}

void AuctionatorEvents::SetPlayerGuid(ObjectGuid playerGuid)
{
    auctionatorGuid = playerGuid;
}

void AuctionatorEvents::EventAllianceBidder()
{
    logInfo("Starting Alliance Bidder");
    AuctionatorBidder bidder = AuctionatorBidder(AUCTIONHOUSE_ALLIANCE, auctionatorGuid);
    bidder.SpendSomeCash();
}

void AuctionatorEvents::EventHordeBidder()
{
    logInfo("Starting Horde Bidder");
    AuctionatorBidder bidder = AuctionatorBidder(AUCTIONHOUSE_HORDE, auctionatorGuid);
    bidder.SpendSomeCash();
}

void AuctionatorEvents::EventNeutralBidder()
{
    logInfo("Starting Neutral Bidder");
    AuctionatorBidder bidder = AuctionatorBidder(AUCTIONHOUSE_NEUTRAL, auctionatorGuid);
    bidder.SpendSomeCash();
}
