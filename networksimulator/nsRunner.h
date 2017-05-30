#pragma once

#include <iostream>
#include <string>
#include <ctime>

#include "nsHost.h"
#include "nsRouter.h"
#include "nsEvent.h"

using namespace std;

class nsRunner {
	static nsRunner* s_object;
public:
	void runSimulation(double log_interval);
	static nsRunner *Object() {
		if (!s_object)
			s_object = new nsRunner;
		return s_object;
	}
};