
#include "AuctionatorEvents.h"
#include "AuctionatorBidder.h"
#include "Log.h"
#include <functional>

AuctionatorEvents::AuctionatorEvents(AuctionatorConfig* auctionatorConfig)
{
    SetLogPrefix("[AuctionatorEvents] ");
    events = EventMap();
    config = auctionatorConfig;
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
    if (config->allianceBidder.enabled) {
        events.ScheduleEvent(1, config->allianceBidder.cycleMinutes);
    }
    // HordeBidder
    if (config->hordeBidder.enabled) {
        events.ScheduleEvent(2, config->hordeBidder.cycleMinutes);
    }
    // NeutralBidder
    if (config->neutralBidder.enabled) {
        events.ScheduleEvent(3, config->neutralBidder.cycleMinutes);
    }
}

void AuctionatorEvents::ExecuteEvents()
{
    logInfo("Executing events");
    uint32 currentEvent = events.ExecuteEvent();
    while (currentEvent != 0) {
        logInfo("Executing event: " + eventToFunction[currentEvent]);

        if (eventToFunction[currentEvent] != "") {
            try {
                // this shit stopped working and I have no idea why
                // so until i revisit we will do it the hard way below.
                // eventFunctions[eventToFunction[currentEvent]]();

                switch(currentEvent) {
                    case 1:
                        EventAllianceBidder();
                        if (config->allianceBidder.enabled) {
                            events.ScheduleEvent(currentEvent, config->allianceBidder.cycleMinutes);
                        }
                        break;
                    case 2:
                        EventHordeBidder();
                        if (config->hordeBidder.enabled) {
                            events.ScheduleEvent(currentEvent, config->hordeBidder.cycleMinutes);
                        }
                        break;
                    case 3:
                        EventNeutralBidder();
                        if (config->neutralBidder.enabled) {
                            events.ScheduleEvent(currentEvent, config->neutralBidder.cycleMinutes);
                        }
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
    AuctionatorBidder bidder = AuctionatorBidder(AUCTIONHOUSE_ALLIANCE, auctionatorGuid, config);
    bidder.SpendSomeCash();
}

void AuctionatorEvents::EventHordeBidder()
{
    logInfo("Starting Horde Bidder");
    AuctionatorBidder bidder = AuctionatorBidder(AUCTIONHOUSE_HORDE, auctionatorGuid, config);
    bidder.SpendSomeCash();
}

void AuctionatorEvents::EventNeutralBidder()
{
    logInfo("Starting Neutral Bidder");
    AuctionatorBidder bidder = AuctionatorBidder(AUCTIONHOUSE_NEUTRAL, auctionatorGuid, config);
    bidder.SpendSomeCash();
}

EventMap AuctionatorEvents::GetEvents()
{
    return events;
}
