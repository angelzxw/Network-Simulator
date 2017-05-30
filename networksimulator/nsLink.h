#pragma once

/*
	The nsLink class manages a link buffer and transmits packets through it
	Packets are queued up in the link through bufferPacket() and as time pass, 
	 transmitPacket() is called to propagate packets to the next router
	The packetArrival() function is called after a packet arrives at the next 
	 router to clear buffer occupancy statistics.
*/

#include <iostream>
#include <string>
#include <queue>
#include <unordered_map>

#include "nsPacket.h"
#include "nsEvent.h"
#include "nsStatistics.h"

using namespace std;

class nsLink {
public:
	string id;
	string src_id, dst_id;
	double rate, delay, size;

	queue<nsPacket*> buffer;
	unsigned int buffer_current; // Amount of bytes inside the link buffer
	unsigned int buffer_size; // Amount of bytes the buffer can hold (size * 1000)

	unsigned int link_current; // Amount of bytes currently in the link
	unsigned int link_size; // Amount of bytes the link can take (rate * delay)
	vector<double> link_rates; // Running time average
	double link_rate; // Time average link rate

	void queueBuffer(nsPacket *packet);
	void processBuffer(double now);
	bool isLinkFull(unsigned int pac_size);
};

class nsLinkManager {
	static nsLinkManager *s_object;
	unordered_map<string, nsLink*> links;
	unordered_map<string, unsigned int> occupied;
public:
	nsLink* addLink(string link_id, string src_id, string dst_id, double rate, double delay, double size);
	nsLink* getLink(string link_id);
	unordered_map<string, nsLink*>* getLinks();

	void processBuffer(double now);
	void queuePacket(nsPacket *packet, double &now);
	void sendPacket(nsPacket *packet, double now);
	void processSendPacket(nsPacket *packet, double now);
	void processArrival(nsPacket *packet, double now);

	bool empty();

	static nsLinkManager *Object() {
		if (!s_object)
			s_object = new nsLinkManager;
		return s_object;
	}
};