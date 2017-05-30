#include "nsPacket.h"
#include "nsFlow.h"

nsPacket * nsPacketFactory::createDataPacket(nsFlow *flow)
{
	nsHost *host_src = nsHostManager::Object()->getHost(flow->src_ip);
	nsHost *host_dst = nsHostManager::Object()->getHost(flow->dst_ip);
	nsPacket *packet = new nsPacket;
	packet->pac_type = TCP_DATA;

	packet->link = host_src->getLink();
	packet->router_src = host_src->id;
	packet->router_dst = host_src->getRouterID();

	packet->flow = flow;
	packet->ip_src = host_src->ip;
	packet->ip_dst = host_dst->ip;

	packet->transmitTime = 0;

	return packet;
}

nsPacket * nsPacketFactory::createAckPacket(nsFlow *flow)
{
	nsHost *host_src = nsHostManager::Object()->getHost(flow->dst_ip);
	nsHost *host_dst = nsHostManager::Object()->getHost(flow->src_ip);
	nsPacket *packet = new nsPacket;
	packet->pac_type = TCP_ACK;
	packet->pac_size = 64;

	packet->link = host_src->getLink();
	packet->router_src = host_src->id;
	packet->router_dst = host_src->getRouterID();

	packet->flow = flow;
	packet->ip_src = host_src->ip;
	packet->ip_dst = host_dst->ip;

	packet->transmitTime = 0;

	return packet;
}

nsPacket * nsPacketFactory::createRoutePacket(string router)
{
	nsPacket *packet = new nsPacket;
	packet->pac_type = ROUTER_UPDATE;
	packet->router_dst = router;
	return packet;
}
