#include "nsReader.h"

// Generates a random IP address
unsigned int nsReader::generateIP()
{
	return (rand() % 0xFF) + ((rand() % 0xFF) << 8) + ((rand() % 0xFF) << 16) + ((rand() % 0xFF) << 24);
}

// Loads a network topology file
void nsReader::loadTopology(string filename)
{
	ifstream input;
	string line;
	input.open(filename);
	if (!input.is_open())
		return;
	while (getline(input, line)) {
		stringstream data(line);
		data >> line;
		if (line == "H") {
			data >> line;
			nsHostManager::Object()->addHost(line, generateIP());
		}
		if (line == "R") {
			data >> line;
			nsRouterManager::Object()->addRouter(line);
		}
		if (line == "L") {
			string id, src, dst;
			double rate, delay, size;
			data >> id >> src >> dst >> rate >> delay >> size;
			nsLink* link = nsLinkManager::Object()->addLink(id, src, dst, rate, delay, size);
			nsRouterManager::Object()->addLink(src, dst, link);
		}
	}
}

void nsReader::loadFlows(string filename)
{
	ifstream input;
	string line;
	input.open(filename);
	if (!input.is_open())
		return;
	while (getline(input, line)) {
		stringstream data(line);
		string id, src, dst, type;
		double size, start;
		data >> id >> src >> dst >> size >> start >> type;
		nsFlowManager::Object()->addFlow(id, src, dst, size, start, type);
	}
}