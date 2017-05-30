#pragma once

/*
	The nsHostManager class records the IP address of each host as well as their names.
*/

#include <iostream>
#include <iomanip>
#include <unordered_map>
#include <string>

#include "nsFlow.h"
#include "nsPacket.h"
#include "nsLink.h"

using namespace std;

class nsFlow;

class nsHost {
public:
	string id;
	unsigned int ip;

	//logs how many bytes have been sent and recieved by this host in one iteration of the priority queue
	unsigned int sentBytes, receivedBytes;

	nsLink *getLink();
	string getRouterID();
};

class nsHostManager {
	static nsHostManager* s_object;
	unordered_map<string, nsHost*> hosts;
	void sendPacket(nsFlow * flow, double now, bool timeout);
public:
	unordered_map<string, nsHost*>* getHosts();
	nsHost* addHost(string host_id, unsigned int ip);
	nsHost* getHost(string host_id);
	nsHost* getHost(unsigned int ip);
	bool isHost(string host_id);
	string getHostName(unsigned int ip);
	unsigned int getHostIP(string host_id);

	void initWindow(nsFlow *flow);
	void resetWindow(nsFlow *flow);
	void eraseWindow(nsFlow *flow);
	void resetTimer(nsFlow *flow, double now);
	void timeoutEvent(nsFlow *flow, double now);
	void recvPacket(nsPacket *packet, double now);

	void resetSentRecieved();

	static nsHostManager *Object() {
		if (!s_object)
			s_object = new nsHostManager;
		return s_object;
	}
};