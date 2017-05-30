#include "nsLink.h"

nsLink* nsLinkManager::addLink(string link_id, string src_id, string dst_id, double rate, double delay, double size)
{
	nsLink *nLink = new nsLink;
	nLink->id = link_id;
	nLink->src_id = src_id;
	nLink->dst_id = dst_id;
	nLink->rate = rate;
	nLink->delay = delay;
	nLink->size = size;
	nLink->buffer_current = 0;
	nLink->buffer_size = (unsigned int)round(size * 1000);
	nLink->link_current = 0;
	nLink->link_size = (unsigned int)round(delay / 1000 * rate * 1000000);
	nLink->link_rate = 0;
	nLink->link_rates.clear();
	links.insert(make_pair(link_id, nLink));
	return nLink;
}

nsLink * nsLinkManager::getLink(string link_id)
{
	if (links.find(link_id) != links.end())
		return links[link_id];
	else
		return nullptr;
}

unordered_map<string, nsLink*>* nsLinkManager::getLinks()
{
	return &links;
}

// Calls processBuffer() for all links
void nsLinkManager::processBuffer(double now)
{
	for (auto i = links.begin(); i != links.end(); i++)
		i->second->processBuffer(now);
}

// Creates an event for EVENT_SENDPACKET on a link
void nsLinkManager::queuePacket(nsPacket * packet, double & now)
{
	nsEvent *e_new = new nsEvent;
	e_new->type = EVENT_SENDPACKET;
	e_new->time = now;
	e_new->packet = packet;
	nsEventManager::Object()->addEvent(e_new);
	//now += ((double)packet->pac_size) / (packet->link->rate * 1000000);
}

// Creates an event for EVENT_PACKET_ARRIVE on a link
void nsLinkManager::sendPacket(nsPacket * packet, double now)
{
	// Discard packets that are outside the window
	if (packet->pac_type == TCP_DATA)
		if (packet->tcp_seq < packet->flow->s_base || packet->tcp_seq > packet->flow->s_base + packet->flow->getWindow())
			return;
	nsEvent *e_new = new nsEvent;
	e_new->type = EVENT_PACKET_ARRIVE;
	e_new->time = now + packet->link->delay / 1000;
	e_new->packet = packet;
	nsEventManager::Object()->addEvent(e_new);
	packet->link->link_current += packet->pac_size;
}

void nsLinkManager::processSendPacket(nsPacket * packet, double now)
{
	nsLink *link = packet->link;
	processBuffer(now);

	// If the link and buffer are both empty, flip the link direction
	if (link->buffer.empty() && !link->isLinkFull(packet->pac_size) && link->link_current == 0 && link->src_id != packet->router_src) {
		string tmp = link->src_id;
		link->src_id = link->dst_id;
		link->dst_id = tmp;
	}
	// Send the packet if the buffer is empty, the link can hold the packet, and the direction is correct
	if (link->buffer.empty() && !link->isLinkFull(packet->pac_size) && link->src_id == packet->router_src)
		sendPacket(packet, now);
	else
		link->queueBuffer(packet);
}

// Packet arrived at next location
void nsLinkManager::processArrival(nsPacket * packet, double now)
{
	packet->link->link_current -= packet->pac_size;
	processBuffer(now);
}

// Checks if all link buffers are empty
bool nsLinkManager::empty()
{
	for (auto i = links.begin(); i != links.end(); i++)
		if (!i->second->buffer.empty())
			return false;
	return true;
}

// Attempt to queue a packet into the buffer
void nsLink::queueBuffer(nsPacket * packet)
{
	if (buffer_size - buffer_current >= packet->pac_size) {
		buffer_current += packet->pac_size;
		buffer.push(packet);
	} else {
		// Dropped the baby
		nsStatistics::Object()->logPacketDrop(packet->link->id);
		cout << "packet dropped" << endl;
	}
}

// Attempt to send packets in the buffer
void nsLink::processBuffer(double now)
{
	while (!buffer.empty()) {
		nsPacket *packet = buffer.front();
		// Packet too big
		if (packet->pac_size > link_size - link_current)
			break;
		// Packet going the other direction
		if (link_current != 0 && src_id != packet->router_src)
			break;
		// Flip the channel
		if (link_current == 0 && src_id != packet->router_src) {
			string tmp = src_id;
			src_id = dst_id;
			dst_id = tmp;
		}
		buffer.pop();
		buffer_current -= packet->pac_size;
		nsLinkManager::Object()->sendPacket(packet, now);
	}
}

// Returns true if the link is full and can't hold any more packets
bool nsLink::isLinkFull(unsigned int pac_size)
{
	return link_size - link_current >= pac_size ? false : true;
}
