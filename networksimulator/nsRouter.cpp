#include "nsRouter.h"

unordered_map<string, nsRouter*>* nsRouterManager::getRouters()
{
	return &routers;
}

void nsRouterManager::addRouter(string id)
{
	nsRouter *nRouter = new nsRouter;
	nRouter->id = id;
	routers.insert(make_pair(id, nRouter));
}

nsRouter * nsRouterManager::getRouter(string id)
{
	if (routers.find(id) != routers.end())
		return routers[id];
	else
		return nullptr;
}

bool nsRouterManager::isRouter(string router_id)
{
	if (routers.find(router_id) != routers.end())
		return true;
	else
		return false;
}

// Adds a link to all the static router tables
void nsRouterManager::addLink(string src, string dst, nsLink * lnk)
{
	for (auto i = routers.begin(); i != routers.end(); i++) {
		nsRoute *route = new nsRoute;
		route->cost = lnk->delay;
		route->link = lnk->id;
		if (i->second->routes.find(src) == i->second->routes.end())
			i->second->routes[src] = new unordered_map<string, nsRoute*>;
		if (i->second->routes.find(dst) == i->second->routes.end())
			i->second->routes[dst] = new unordered_map<string, nsRoute*>;
		i->second->routes[src]->insert(make_pair(dst, route));
		i->second->routes[dst]->insert(make_pair(src, route));

		route = new nsRoute;
		route->cost = 0;
		route->link = lnk->id;
		if (i->second->routes_dynamic.find(src) == i->second->routes_dynamic.end())
			i->second->routes_dynamic[src] = new unordered_map<string, nsRoute*>;
		if (i->second->routes_dynamic.find(dst) == i->second->routes_dynamic.end())
			i->second->routes_dynamic[dst] = new unordered_map<string, nsRoute*>;
		i->second->routes_dynamic[src]->insert(make_pair(dst, route));
		i->second->routes_dynamic[dst]->insert(make_pair(src, route));
	}
}

// Routes a packet to the next link
void nsRouterManager::routePacket(nsPacket * packet, double now)
{
	// Parse router updates
	if (packet->pac_type == ROUTER_UPDATE) {
		nsRouter *router = getRouter(packet->router_dst);
		router->recvPacket(packet, now);
		return;
	}
	// The soft whisperings of the old gods routes each packet
	packet->router_src = packet->router_dst;
	packet->router_dst = routers[packet->router_src]->table[packet->ip_dst];
	string link = routers[packet->router_src]->routes[packet->router_src]->at(packet->router_dst)->link;
	packet->link = nsLinkManager::Object()->getLink(link);
	nsLinkManager::Object()->queuePacket(packet, now);
}

void nsRouterManager::updateRoutes(double now)
{
	for (auto i = routers.begin(); i != routers.end(); i++) {
		nsRouter *router = i->second;
		// Adjust weight (equal to current buffer size)
		for (auto j = router->routes_dynamic[router->id]->begin(); j != router->routes_dynamic[router->id]->end(); j++) {
			nsLink *link = nsLinkManager::Object()->getLink(j->second->link);
			j->second->cost = link->link_current / 1000;
			j->second->time = now;
		}
		// Send update packet to adjacent routers
		for (auto j = router->routes_dynamic[router->id]->begin(); j != router->routes_dynamic[router->id]->end(); j++) {
			string dst = j->first;
			if (nsHostManager::Object()->isHost(dst))
				continue;
			nsLink *link = nsLinkManager::Object()->getLink(j->second->link);
			nsPacket *packet = nsPacketFactory::Object()->createRoutePacket(dst);
			packet->link = link;
			packet->routes = &router->routes_dynamic;
			packet->pac_size = router->routes_dynamic.size() * 4;
			nsLinkManager::Object()->queuePacket(packet, now);
		}
	}
}

void nsRouter::updateRoutes()
{
	unordered_map<string, nsRouterNode*> set;
	// Add everything to the set
	for (auto i = nsHostManager::Object()->getHosts()->begin(); i != nsHostManager::Object()->getHosts()->end(); i++) {
		nsRouterNode *n = new nsRouterNode;
		n->back = NULL;
		n->cost = DBL_MAX;
		n->id = i->second->id;
		n->isHost = true;
		n->processed = false;
		set.insert(make_pair(i->second->id, n));
	}
	for (auto i = nsRouterManager::Object()->getRouters()->begin(); i != nsRouterManager::Object()->getRouters()->end(); i++) {
		nsRouterNode *n = new nsRouterNode;
		n->back = NULL;
		n->cost = DBL_MAX;
		if (i->second->id == id)
			n->cost = 0;
		n->id = i->second->id;
		n->isHost = false;
		n->processed = false;
		set.insert(make_pair(i->second->id, n));
	}
	// Do Dijkstra magic
	int nodes_left = set.size();
	while (nodes_left) {
		// Find node with smallest cost
		nsRouterNode *n = NULL;
		double lowest = DBL_MAX;
		for (auto i = set.begin(); i != set.end(); i++) {
			if (i->second->cost < lowest && !i->second->processed) {
				lowest = i->second->cost;
				n = i->second;
			}
		}
		if (n == NULL)
			break;
		n->processed = true;
		nodes_left--;
		// For each edge
		for (auto i = routes[n->id]->begin(); i != routes[n->id]->end(); i++) {
			if (set[i->first]->processed)
				continue;
			if (set[i->first]->cost > n->cost + i->second->cost + routes_dynamic[n->id]->at(i->first)->cost) {
				set[i->first]->cost = n->cost + i->second->cost + routes_dynamic[n->id]->at(i->first)->cost;
				set[i->first]->back = n;
			}
		}
	}
	// Update table for each host which tells you which router to put the packet on for each IP
	table.clear();
	for (auto i = nsHostManager::Object()->getHosts()->begin(); i != nsHostManager::Object()->getHosts()->end(); i++) {
		nsRouterNode *n = set[i->second->id];
		string path_name;
		while (n->id != id) {
			path_name = n->id;
			n = n->back;
		}
		table[i->second->ip] = path_name;
	}
}

void nsRouter::recvPacket(nsPacket * packet, double now)
{
	for (auto i = packet->routes->begin(); i != packet->routes->end(); i++) {
		for (auto j = i->second->begin(); j != i->second->end(); j++) {
			if (routes_dynamic[i->first]->at(j->first)->time < j->second->time) {
				routes_dynamic[i->first]->at(j->first)->cost = j->second->cost;
				routes_dynamic[i->first]->at(j->first)->time = j->second->time;
			}
		}
	}
	updateRoutes();
}
