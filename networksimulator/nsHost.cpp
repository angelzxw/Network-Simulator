#include "nsHost.h"

// Adds a host to the host collection
nsHost* nsHostManager::addHost(string host_id, unsigned int ip)
{
	nsHost *nHost = new nsHost;
	nHost->sentBytes = 0;
	nHost->receivedBytes = 0;
	nHost->id = host_id;
	nHost->ip = ip;
	hosts.insert(make_pair(host_id, nHost));
	return nHost;
}

unordered_map<string, nsHost*>* nsHostManager::getHosts()
{
	return &hosts;
}

// Finds a host based on host name
nsHost* nsHostManager::getHost(string host_id)
{
	if (hosts.find(host_id) != hosts.end())
		return hosts[host_id];
	else
		return nullptr;
}

// Finds a host based on ip address
nsHost * nsHostManager::getHost(unsigned int ip)
{
	for (auto i = hosts.begin(); i != hosts.end(); i++)
		if (i->second->ip == ip)
			return i->second;
	return nullptr;
}

bool nsHostManager::isHost(string host_id)
{
	if (hosts.find(host_id) != hosts.end())
		return true;
	else
		return false;
}

string nsHostManager::getHostName(unsigned int ip)
{
	return getHost(ip)->id;
}

unsigned int nsHostManager::getHostIP(string host_id)
{
	return getHost(host_id)->ip;
}

// Sends all packets within a window from window base
void nsHostManager::sendPacket(nsFlow *flow, double now, bool timeout)
{
	int pck_count = 0;
	unsigned int tcp_seq = flow->s_base;
	if (flow->s_base * 1000 > flow->size * 1000000)
		flow->bytesRemaining = 0;
	// TCP
	unsigned int bytesRemaining = flow->bytesRemaining;
	while (bytesRemaining != 0 && (unsigned int)pck_count < flow->getWindow()) {
		// Only send packets that have not yet been sent (unless timeout, then resend window base packet)
		if (flow->s_frames[tcp_seq] == TCP_NOT_SENT || (timeout && (tcp_seq == flow->s_base))) {
			cout << std::fixed << std::setprecision(3) << now << " Send: Sending Packet [" << tcp_seq << "] With Window Base [" << flow->s_base << "] And Window Size [" << flow->getWindow() << "]" << endl;
			flow->s_frames[tcp_seq] = TCP_SENT;
			// Construct tcp data packet
			nsPacket *packet = nsPacketFactory::Object()->createDataPacket(flow);
			packet->tcp_seq = tcp_seq;
			if (bytesRemaining < 1000)
				packet->pac_size = flow->bytesRemaining;
			else
				packet->pac_size = 1000;
			bytesRemaining -= packet->pac_size;
			packet->transmitTime = now;

			//updates number of sent bytes for sending host
			nsHost *host = getHost(flow->src_ip);
			host->sentBytes += packet->pac_size;

			nsLinkManager::Object()->queuePacket(packet, now);
		}
		tcp_seq++;
		pck_count++;
	}
	// We sent stuff, so reset timer
	resetTimer(flow, now);
}

void nsHostManager::initWindow(nsFlow * flow)
{
	flow->s_base = 0;
	flow->s_next_window_update = 0;
	flow->s_window = 1;
	flow->s_ack_good = false;
	flow->s_ack_num = -1;
	flow->s_ack_count = 0;
	flow->s_rtt = 0.3;
	flow->s_first_rtt = true;
	flow->ss_thresh = LONG_MAX;

	flow->r_base = 0;
	flow->r_window = 10;
}

// Resets the window and clears sent packets
void nsHostManager::resetWindow(nsFlow * flow)
{
	// Half the window and set ss_thresh to half the old threshhold
	flow->s_window /= 2;
	if (flow->getWindow() == 0 || flow->getWindow() == 1)
		flow->s_window = 1;
	if (flow->ss_thresh == LONG_MAX)
		flow->ss_thresh = flow->s_window;
	else
		flow->ss_thresh /= 2;
	flow->s_next_window_update = flow->s_base + flow->getWindow();
}

void nsHostManager::eraseWindow(nsFlow * flow)
{
	// Reset frames sent beyond the current window (so we don't assume they have been sent)
	for (auto i = flow->s_frames.begin(); i != flow->s_frames.end(); i++) {
		if (i->first >= flow->s_base + flow->getWindow() && i->second == TCP_SENT)
			flow->s_frames[i->first] = TCP_NOT_SENT;
	}
	flow->s_frames.clear();
}

void nsHostManager::resetTimer(nsFlow * flow, double now)
{
	flow->s_timeout = now + flow->s_rtt + 0.010; // RTT + 10 ms
}

// Process a flow's timeout event
void nsHostManager::timeoutEvent(nsFlow *flow, double now)
{
	if (now >= flow->s_timeout) {
		cout << std::fixed << std::setprecision(3) << now << " Send: Timeout Sequence [" << flow->s_base << "]" << endl;
		if (now != flow->start) {
			resetWindow(flow);
			eraseWindow(flow);
		}
		sendPacket(flow, now, true);
	}
}

// Receives a packet from a link (data or ack)
void nsHostManager::recvPacket(nsPacket * packet, double now)
{
	if (packet->pac_type == TCP_DATA) {
		nsFlow *flow = packet->flow;
		
		if (packet->tcp_seq >= flow->r_base)
			flow->r_frames[packet->tcp_seq] = TCP_RECV;

		nsPacket *r = nsPacketFactory::Object()->createAckPacket(packet->flow);
		r->tcp_ack = flow->r_base;
		r->transmitTime = packet->transmitTime;

		while (flow->r_frames[r->tcp_ack] == TCP_RECV) {
			//flow->r_frames.erase(r->tcp_ack);
			r->tcp_ack++;
		}

		// Gets the amount of data recieved and size of ACK packet sent and puts them in corresponding variables for the host
		nsHost * host = getHost(packet->ip_dst);
		host->receivedBytes += packet->pac_size;
		host->sentBytes += r->pac_size;

		if (flow->r_base > packet->tcp_seq)
			return;

		flow->r_base = r->tcp_ack;

		cout << std::fixed << std::setprecision(3) << now << " Recv: Received Packet [" << packet->tcp_seq << "], Sending Ack [" << flow->r_base << "]" << endl;

		nsLinkManager::Object()->queuePacket(r, now);
	}
	if (packet->pac_type == TCP_ACK) {
		nsFlow *flow = packet->flow;
		// Update the recieved packets for the host recieving ACK
		nsHost * host = getHost(packet->ip_dst);
		host->receivedBytes += packet->pac_size;

		cout << std::fixed << std::setprecision(3) << now << " Send: Received Ack [" << packet->tcp_ack << "]" << endl;
		// Update window
		if (packet->tcp_ack > flow->s_base) {
			if (flow->bytesRemaining != 0)
				flow->bytesRemaining -= (packet->tcp_ack - flow->s_base) * 1000;
			flow->s_base = packet->tcp_ack;
			// Calculate RTT
			double currRTT = now - packet->transmitTime;
			packet->flow->s_rtt = packet->flow->alpha * packet->flow->s_rtt + (1 - packet->flow->alpha) * currRTT; // RTT(i) = a * RTT(i - 1) + (1 - a) * currRTT
			// Set whatever received as first RTT
			if (packet->flow->s_first_rtt) {
				packet->flow->s_first_rtt = false;
				packet->flow->s_rtt = currRTT;
			}
			// Window updates for every window number of packets
			if (flow->s_base > flow->s_next_window_update || flow->s_next_window_update == 0) {
				if (flow->getWindow() < flow->getThresh())
					flow->s_window *= 2;
				else
					flow->s_window += 1;
				flow->s_next_window_update = flow->s_base + flow->getWindow();
				cout << std::fixed << std::setprecision(3) << now << " Send: Update Window Size To [" << flow->s_base + flow->s_window << "]" << endl;;
			}
			// Window updates for every received ACK
			if (flow->getWindow() < flow->getThresh())
				flow->s_window += 1;
			else
				flow->s_window += 1 / flow->s_window;
		}
		// Triple duplicate ack
		if (flow->s_ack_num != packet->tcp_ack) {
			flow->s_ack_num = packet->tcp_ack;
			flow->s_ack_count = 1;
			flow->s_ack_good = true;
		} else {
			flow->s_ack_count++;
			if (flow->s_ack_count >= 3 && flow->s_ack_good) {
				cout << std::fixed << std::setprecision(3) << now << " Triple Duplicate Ack" << endl;
				flow->s_ack_count = 0;
				// Fast recovery
				resetWindow(flow);
				if (flow->type == TCP_TAHOE)
					flow->s_window = 1;
				eraseWindow(flow);
				flow->s_ack_good = false;
			}
		}
		// Transmit more
		sendPacket(packet->flow, now, false);
	}
}

void nsHostManager::resetSentRecieved()
{
	for (auto i = hosts.begin(); i != hosts.end(); i++)
	{
		nsHost * n = i->second;
		n->receivedBytes = 0;
		n->sentBytes = 0;
	}
}

nsLink * nsHost::getLink()
{
	for (auto i = nsLinkManager::Object()->getLinks()->begin(); i != nsLinkManager::Object()->getLinks()->end(); i++)
		if (i->second->src_id == id || i->second->dst_id == id)
			return i->second;
	return nullptr;
}

string nsHost::getRouterID()
{
	nsLink *link = getLink();
	return link->src_id == id ? link->dst_id : link->src_id;
}
