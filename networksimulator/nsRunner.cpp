#include "nsRunner.h"

void nsRunner::runSimulation(double log_interval)
{
	double time = 0.0;
	srand((unsigned int)std::time(0));
	// Create a timeout event for each flow
	for (auto i = nsFlowManager::Object()->getFlows()->begin(); i != nsFlowManager::Object()->getFlows()->end(); i++) {
		nsEvent *e_new = new nsEvent;
		e_new->type = EVENT_TIMEOUT;
		e_new->time = i->second->start;
		e_new->flow = i->second;
		nsEventManager::Object()->addEvent(e_new);
	}
	// Create a timeout event for router updates
	nsEvent *e_new = new nsEvent;
	e_new->type = EVENT_ROUTER_UPDATE;
	e_new->time = nsEventManager::Object()->peek()->time;
	nsEventManager::Object()->addEvent(e_new);
	// Run initial routing algorithm without dynamic weights
	for (auto i = nsRouterManager::Object()->getRouters()->begin(); i != nsRouterManager::Object()->getRouters()->end(); i++)
		i->second->updateRoutes();
	// Create a timeout event for logging results (at the start of the first event in queue)
	e_new = new nsEvent;
	e_new->type = EVENT_LOG;
	e_new->time = nsEventManager::Object()->peek()->time + 0.0000001; // This ensures that the logging event is triggered after the timeslot has been processed (smart I know!)
	nsEventManager::Object()->addEvent(e_new);
	// Start event queue
	while (!nsEventManager::Object()->empty()) {
		nsEvent *e = nsEventManager::Object()->peek();
		nsEventManager::Object()->pop();
		time = e->time;
		// Parse event
		switch (e->type) {
			case EVENT_LOG:
			{
				nsStatistics::Object()->time = time;
				nsStatistics::Object()->logLink(log_interval);
				nsStatistics::Object()->logHost(log_interval);
				nsStatistics::Object()->logFlow(log_interval);
				nsStatistics::Object()->logResetPacketDrop();
				nsHostManager::Object()->resetSentRecieved();

				// Stop logging events if all flows have completed their transfers
				bool complete = true;
				for (auto i = nsFlowManager::Object()->getFlows()->begin(); i != nsFlowManager::Object()->getFlows()->end(); i++)
					complete &= i->second->bytesRemaining == 0 ? true : false;
				if (!complete) {
					nsEvent *e_new = new nsEvent;
					e_new->type = EVENT_LOG;
					e_new->time = time + log_interval;
					nsEventManager::Object()->addEvent(e_new);
				}
				break;
			}
			case EVENT_ROUTER_UPDATE:
			{
				nsRouterManager::Object()->updateRoutes(time);

				// Stop routing update events if all flows have completed sending
				bool complete = true;
				for (auto i = nsFlowManager::Object()->getFlows()->begin(); i != nsFlowManager::Object()->getFlows()->end(); i++)
					complete &= i->second->bytesRemaining == 0 ? true : false;
				if (!complete) {
					nsEvent *e_new = new nsEvent;
					e_new->type = EVENT_ROUTER_UPDATE;
					e_new->time = time + 0.3;
					nsEventManager::Object()->addEvent(e_new);
				}
				break;
			}
			case EVENT_SENDPACKET:
			{
				nsLinkManager::Object()->processSendPacket(e->packet, time);
				break;
			}
			case EVENT_TIMEOUT:
			{
				nsHostManager::Object()->timeoutEvent(e->flow, time);
				// Stop timer events if all flows have completed their transfers
				if (e->flow->bytesRemaining != 0) {
					nsEvent *e_new = new nsEvent;
					e_new->type = EVENT_TIMEOUT;
					e_new->time = time + 0.005;
					e_new->flow = e->flow;
					nsEventManager::Object()->addEvent(e_new);
				}
				break;
			}
			case EVENT_PACKET_ARRIVE:
				nsLinkManager::Object()->processArrival(e->packet, time);
				if (nsHostManager::Object()->isHost(e->packet->router_dst))
					nsHostManager::Object()->recvPacket(e->packet, time);
				if (nsRouterManager::Object()->isRouter(e->packet->router_dst))
					nsRouterManager::Object()->routePacket(e->packet, time);
				break;
		}
		delete e;
	}
}
