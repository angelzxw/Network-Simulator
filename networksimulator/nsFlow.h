#pragma once

/*
	The nsFlow class contains information on the flows within the simulation.
	This class keeps track of all the TCP windows and ACKs for both send and recv.
*/

#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "nsHost.h"

#define TCP_NOT_SENT 0
#define TCP_SENT 1
#define TCP_GOT_ACK 2
#define TCP_NOT_RECV 0
#define TCP_RECV 1

using namespace std;

enum nsFlowType {TCP_TAHOE, TCP_RENO};

class nsFlow {
public:
	// Configuration
	string id; // Name of flow
	unsigned int src_ip, dst_ip; // Source and destination IP addresses (UINT)
	double size, start; // Size of flow (MB) and start time (Seconds)
	unsigned int bytesRemaining; // Number of bytes remaining to be sent
	unsigned int bytesReceived; // Number of bytes received

	nsFlowType type;
	unsigned int s_base;
	unsigned int s_next_window_update;
	double s_window;
	bool s_ack_good; // True if a good ACK has been received won't trigger triple duplicate ACK again until a good one
	unsigned int s_ack_num, s_ack_count; // Counts the lowest ACK for triple duplicate ACK
	double ss_thresh;
	bool s_first_rtt;
	double s_rtt, s_timeout;
	double alpha;//rolloff factor
	unordered_map<unsigned int, int> s_frames; // 0 = TCP_NOT_SENT, 1 = TCP_SENT, 2 = TCP_GOT_ACK

	unsigned int r_base;
	unsigned int r_window;
	unordered_map<unsigned int, int> r_frames; // 0 = TCP_NOT_RECV, 1 = TCP_RECV

	unsigned int getWindow();
	unsigned int getThresh();
};

class nsFlowManager {
	static nsFlowManager *s_object;
	unordered_map<string, nsFlow*> flows;
public:
	nsFlow* addFlow(string flow_id, string src_id, string dst_id, double size, double start, string type);
	nsFlow* getFlow(string flow_id);
	nsFlow* getFlow(string src_id, string dst_id);
	nsFlow* getFlow(unsigned int src_ip, unsigned int dst_ip);
	unordered_map<string, nsFlow*>* getFlows();

	static nsFlowManager *Object() {
		if (!s_object)
			s_object = new nsFlowManager;
		return s_object;
	}
};
