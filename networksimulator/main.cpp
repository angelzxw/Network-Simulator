#include <iostream>

#include "nsReader.h"
#include "nsHost.h"
#include "nsRunner.h"
#include "nsEvent.h"
#include "nsPacket.h"
#include "nsStatistics.h"

using namespace std;

// Setup singletons
nsReader *nsReader::s_object = 0;
nsHostManager *nsHostManager::s_object = 0;
nsRouterManager *nsRouterManager::s_object = 0;
nsLinkManager *nsLinkManager::s_object = 0;
nsFlowManager *nsFlowManager::s_object = 0;
nsRunner *nsRunner::s_object = 0;
nsEventManager *nsEventManager::s_object = 0;
nsPacketFactory *nsPacketFactory::s_object = 0;
nsStatistics *nsStatistics::s_object = 0;

int main(int argc, char* argv[]) {
	if (argc < 4) {
		cout << "Usage: " << argv[0] << " topology flow output" << endl;
		cout << "topology - name of topology file (test_case_0.top)" << endl;
		cout << "flow - name of flow file (test_case_0.flow)" << endl;
		cout << "output - name of output folder (Output)" << endl;
		return 0;
	}
	string line;
	line = argv[1];
	nsReader::Object()->loadTopology(line);
	line = argv[2];
	nsReader::Object()->loadFlows(line);
	line = argv[3];
	if (*line.rbegin() != '/')
		line += '/';
	nsStatistics::Object()->setOutput(line);
	nsRunner::Object()->runSimulation(0.01);
	cout << "Simulation Complete!" << endl;
	cerr << "Simulation Complete!" << endl;
	return 0;
}