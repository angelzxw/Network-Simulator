#pragma once

#include <iostream>
#include <fstream>
#include <string>

#include "nsLink.h"
#include "nsHost.h"
using namespace std;

#define LINK_ROLLOFF 0.95

class nsStatistics {
	static nsStatistics *s_object;
	ofstream olink_rate, olink_buffer, olink_loss;
	ofstream oflow_send,oflow_received, oflow_window, oflow_rtt;
	ofstream ohost_send, ohost_received;
	unordered_map<string, int> packetdrop;
public:
	double time;
	bool setOutput(string filename);
	void logPacketDrop(string link);
	void logResetPacketDrop();
	void logHost(double time_delta);
	void logLink(double time_delta);
	void logFlow(double time_delta);

	static nsStatistics *Object() {
		if (!s_object)
			s_object = new nsStatistics;
		return s_object;
	}
};