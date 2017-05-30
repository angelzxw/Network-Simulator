#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "nsRouter.h"
#include "nsFlow.h"
#include "nsHost.h"

using namespace std;

class nsReader {
	static nsReader *s_object;
	unsigned int generateIP();
public:
	void loadTopology(string filename);
	void loadFlows(string filename);
	static nsReader *Object() {
		if (!s_object)
			s_object = new nsReader;
		return s_object;
	}
};