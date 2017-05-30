#pragma once

/*
	The nsEvent class keeps track of events.
*/

#include <iostream>
#include <string>
#include <queue>

#include "nsPacket.h"
#include "nsFlow.h"

enum nsEventType {EVENT_SENDPACKET, EVENT_PACKET_ARRIVE, EVENT_TIMEOUT, EVENT_LOG, EVENT_ROUTER_UPDATE};

class nsEvent {
public:
	double time;
	int order; // The order is given!
	nsEventType type;
	// EVENT_SENDPACKET (At time to send packet)
	// EVENT_PACKET_ARRIVE (Process packet arrival)
	nsPacket *packet;
	// EVENT_TIMEOUT (Timeout event for a flow)
	nsFlow *flow;
	// EVENT_LOG (Output data to log)
};

struct nsEventCompare {
	bool operator() (const nsEvent *lhs, const nsEvent *rhs) const
	{
		if (lhs->time == rhs->time)
			return lhs->order > rhs->order;
		else
			return lhs->time > rhs->time;
	}
};

class nsEventManager {
	static nsEventManager* s_object;
	priority_queue<nsEvent*, vector<nsEvent*>, nsEventCompare> queue;
	int global_order;
public:
	nsEventManager();
	void addEvent(nsEvent* e);
	bool empty();
	nsEvent* peek();
	void pop();
	static nsEventManager *Object() {
		if (!s_object)
			s_object = new nsEventManager;
		return s_object;
	}
};