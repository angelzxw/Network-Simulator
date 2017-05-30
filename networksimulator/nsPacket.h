#pragma once

#include <string>
#include <unordered_map>

using namespace std;

enum nsPacketType {TCP_DATA, TCP_ACK, ROUTER_UPDATE};

class nsLink;
class nsFlow;
class nsRoute;

class nsPacket {
public:
	nsPacketType pac_type;
	unsigned int pac_size;

	// Router to router src and dst (router/host names)
	nsLink *link;
	string router_src;
	string router_dst;

	// Host to host src and dst (host ip addresses)
	nsFlow *flow;
	unsigned int ip_src;
	unsigned int ip_dst;

	// TCP packet information
	unsigned int tcp_seq;
	unsigned int tcp_ack;
	unsigned int tcp_recv;

	// RTT calculation parameter
	double transmitTime;

	// Routing packet information
	unordered_map<string, unordered_map<string, nsRoute*>*> *routes;
};

class nsPacketFactory {
	static nsPacketFactory *s_object;
public:
	nsPacket* createDataPacket(nsFlow *flow);
	nsPacket* createAckPacket(nsFlow *flow);
	nsPacket* createRoutePacket(string router);
	static nsPacketFactory *Object() {
		if (!s_object)
			s_object = new nsPacketFactory;
		return s_object;
	}
};