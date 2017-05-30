#include "nsFlow.h"

nsFlow * nsFlowManager::addFlow(string flow_id, string src_id, string dst_id, double size, double start, string type)
{
	nsFlow *nFlow = new nsFlow;
	nFlow->id = flow_id;
	transform(type.begin(), type.end(), type.begin(), toupper);
	if (type == "TAHOE")
		nFlow->type = TCP_TAHOE;
	if (type == "RENO") // We're going to be rich!
		nFlow->type = TCP_RENO;
	nFlow->size = size;
	nFlow->start = start;
	nFlow->src_ip = nsHostManager::Object()->getHostIP(src_id);
	nFlow->dst_ip = nsHostManager::Object()->getHostIP(dst_id);
	nFlow->bytesRemaining = (unsigned int)round(nFlow->size * 1000000);
	nFlow->bytesReceived = 0;
	nFlow->s_timeout = 0;
	nFlow->alpha = 0.8;
	nFlow->s_frames.clear();
	nFlow->r_frames.clear();
	nsHostManager::Object()->initWindow(nFlow);
	flows.insert(make_pair(flow_id, nFlow));
	return nFlow;
}

nsFlow * nsFlowManager::getFlow(string flow_id)
{
	if (flows.find(flow_id) != flows.end())
		return flows[flow_id];
	else
		return 0;
}

nsFlow * nsFlowManager::getFlow(string src_id, string dst_id)
{
	return getFlow(nsHostManager::Object()->getHostIP(src_id), nsHostManager::Object()->getHostIP(dst_id));
}

nsFlow * nsFlowManager::getFlow(unsigned int src_ip, unsigned int dst_ip)
{
	for (auto i = flows.begin(); i != flows.end(); i++)
		if (i->second->src_ip == src_ip && i->second->dst_ip == dst_ip)
			return i->second;
	return nullptr;
}

// Get all flows
unordered_map<string, nsFlow*> * nsFlowManager::getFlows()
{
	return &flows;
}

unsigned int nsFlow::getWindow()
{
	return (unsigned int)round(s_window);
}

unsigned int nsFlow::getThresh()
{
	return (unsigned int)round(ss_thresh);
}
