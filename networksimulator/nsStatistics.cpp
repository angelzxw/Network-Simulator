#include "nsStatistics.h"

bool nsStatistics::setOutput(string filename)
{
	olink_rate.open(filename + "Links.rate.csv");
	if (!olink_rate.is_open())
		return false;
	olink_buffer.open(filename + "Links.buffer.csv");
	if (!olink_buffer.is_open())
		return false;
	olink_loss.open(filename + "Links.loss.csv");
	if (!olink_loss.is_open())
		return false;
	ohost_received.open(filename + "Host.receive.csv");
	if (!ohost_received.is_open())
		return false;
	ohost_send.open(filename + "Host.send.csv");
	if (!ohost_send.is_open())
		return false;
	oflow_window.open(filename + "Window.csv");
	if (!oflow_window.is_open())
		return false;
	oflow_rtt.open(filename + "RTT.csv");
	if (!oflow_rtt.is_open())
		return false;
	oflow_received.open(filename + "Flow.receive.csv");
	if (!oflow_received.is_open())
		return false;
	oflow_send.open(filename + "Flow.send.csv");
	if (!oflow_send.is_open())
		return false;
	olink_rate << "\"Time\",";
	olink_buffer << "\"Time\",";
	olink_loss << "\"Time\",";
	ohost_received << "\"Time\",";
	ohost_send << "\"Time\",";
	oflow_window << "\"Time\",";
	oflow_rtt << "\"Time\",";	
	oflow_received << "\"Time\",";
	oflow_send << "\"Time\",";
	for (auto i = nsLinkManager::Object()->getLinks()->begin(); i != nsLinkManager::Object()->getLinks()->end(); i++) {
		olink_rate << "\"" << i->second->id << "\",";
		olink_buffer << "\"" << i->second->id << "\",";
		olink_loss << "\"" << i->second->id << "\",";
	}
	olink_rate << endl;
	olink_buffer << endl;
	olink_loss << endl;

	for (auto i = nsHostManager::Object()->getHosts()->begin(); i != nsHostManager::Object()->getHosts()->end(); i++) {
		ohost_received << "\"" << i->second->id << "\",";
		ohost_send << "\"" << i->second->id << "\",";
	}
	ohost_received << endl;
	ohost_send << endl;

	for (auto i = nsFlowManager::Object()->getFlows()->begin(); i != nsFlowManager::Object()->getFlows()->end(); i++) {
		oflow_window <<"\""<< i->second->id << "\",";
		oflow_rtt << "\"" << i->second->id << "\",";
		oflow_received << "\"" << i->second->id << "\",";
		oflow_send << "\"" << i->second->id << "\",";
	}
	oflow_window << endl;
	oflow_rtt << endl;
	oflow_received << endl;
	oflow_send << endl;
	return true;
}

void nsStatistics::logPacketDrop(string link)
{
	packetdrop[link]++;
}

void nsStatistics::logResetPacketDrop()
{
	// Log packet drop data to file
	olink_loss << time << ",";
	for (auto i = nsLinkManager::Object()->getLinks()->begin(); i != nsLinkManager::Object()->getLinks()->end(); i++) {
		olink_loss << packetdrop[i->second->id] << ",";
	}
	olink_loss << endl;
	packetdrop.clear();
}

void nsStatistics::logLink(double time_delta)
{
	// Log link data to file
	olink_rate << time << ",";
	for (auto i = nsLinkManager::Object()->getLinks()->begin(); i != nsLinkManager::Object()->getLinks()->end(); i++) {
		nsLink *link = i->second;
		double link_rate = ((double)link->link_current / (double)link->link_size * link->rate * 1000);
		link->link_rates.push_back(link_rate);
		if (link->link_rates.size() > 6)
			link->link_rates.erase(link->link_rates.begin());
		link->link_rate = 0;
		for (auto j = link->link_rates.begin(); j != link->link_rates.end(); j++)
			link->link_rate += *j;
		link->link_rate = round(link->link_rate / (double)link->link_rates.size());
		olink_rate << link->link_rate << ",";
	}
	olink_rate << endl;

	olink_buffer << time << ",";
	for (auto i = nsLinkManager::Object()->getLinks()->begin(); i != nsLinkManager::Object()->getLinks()->end(); i++) {
		olink_buffer << nsLinkManager::Object()->getLink(i->second->id)->buffer_current << ",";
	}
	olink_buffer << endl;
}

void nsStatistics::logFlow(double time_delta)
{
	oflow_rtt << time << ",";
	oflow_window << time << ",";
	oflow_send << time << ",";
	oflow_received << time << ",";
	for (auto i = nsFlowManager::Object()->getFlows()->begin(); i != nsFlowManager::Object()->getFlows()->end(); i++) {
		oflow_window << i->second->getWindow() << ",";
		oflow_rtt << i->second->s_rtt << ",";
		oflow_send << nsHostManager::Object()->getHost(i->second->src_ip)->sentBytes / time_delta / 1000000 << ",";
		oflow_received << nsHostManager::Object()->getHost(i->second->src_ip)->receivedBytes / time_delta / 1000000 << ",";
	}
	oflow_rtt << endl;
	oflow_window << endl;
	oflow_send << endl;
	oflow_received << endl;
}

void nsStatistics::logHost(double time_delta)
{
	ohost_received << time << ",";
	ohost_send << time << ",";

	for (auto i = nsHostManager::Object()->getHosts()->begin(); i != nsHostManager::Object()->getHosts()->end(); i++) {
		ohost_received << nsHostManager::Object()->getHost(i->second->id)->receivedBytes / time_delta / 1000000 << ",";
		ohost_send << nsHostManager::Object()->getHost(i->second->id)->sentBytes / time_delta / 1000000 << ",";
	}
	ohost_received << endl;
	ohost_send << endl;
}
