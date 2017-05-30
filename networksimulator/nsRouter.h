#pragma once

#include <string>
#include <unordered_map>

#include "nsLink.h"
#include "nsHost.h"

using namespace std;

class nsRoute {
public:
	double time;
	double cost;
	string link;
};

class nsRouter {
private:
public:
	string id;
	unordered_map<string, unordered_map<string, nsRoute*>*> routes;
	unordered_map<string, unordered_map<string, nsRoute*>*> routes_dynamic;
	unordered_map<unsigned int, string> table;
	void updateRoutes();
	void recvPacket(nsPacket *packet, double now);
};

class nsRouterNode {
public:
	string id;
	bool isHost;
	double cost;
	nsRouterNode *back;
	bool processed;
};

struct nsRouterNodeCompare {
	bool operator() (const nsRouterNode *lhs, const nsRouterNode *rhs) const
	{
		return lhs->cost > rhs->cost;
	}
};

class nsRouterManager {
	static nsRouterManager *s_object;
	unordered_map<string, nsRouter*> routers;
public:
	unordered_map<string, nsRouter*>* getRouters();
	void addRouter(string id);
	nsRouter* getRouter(string id);
	bool isRouter(string router_id);
	void addLink(string src, string dst, nsLink* lnk);

	void routePacket(nsPacket *packet, double now);
	void updateRoutes(double now);
	static nsRouterManager *Object() {
		if (!s_object)
			s_object = new nsRouterManager;
		return s_object;
	}
};