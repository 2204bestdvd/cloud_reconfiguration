#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <string>
#include <stdio.h>
#include <queue>
#include <vector>
#include <tuple>
#include <random>
#include <ctime>
#include "packet.h"
#include "testbed.h"
#include "logger.h"

using namespace std;

int main(int argc, char* argv[]) {
	srand(time(0));

	// arguments: t=simTime, N=N, deltar=deltar
	int simTime = 10;
	//int N = 4;
	int deltar = 0;
	int deltarResource = 0;
	int deltarCommodity = 0;
	int costr = 0;
	char schedulingPolicy[20] = "DCNC";
	double V = 1;
	bool logging = true;
	char flowFile[20] = "flow.in";
	char topoFile[20] = "topo.in";
	if (argc >= 2) {
		char* s;
		char* equalsSign;
		char* result = 0;
		for (int i = 1; i < argc; i++) {
			s = argv[i];
			equalsSign = strchr(s, '=');
			if (equalsSign) {
				*equalsSign = 0;
				equalsSign++;
				result = static_cast<char*>(malloc(strlen(equalsSign) + 1));
				strcpy(result, equalsSign);
				if (!strcmp(s, "t")) simTime = atoi(result);
				//else if (!strcmp(s, "N")) N = atoi(result);
				else if (!strcmp(s, "deltar")) deltar = atoi(result);
				else if (!strcmp(s, "deltarResource")) deltarResource = atoi(result);
				else if (!strcmp(s, "deltarCommodity")) deltarCommodity = atoi(result);
				else if (!strcmp(s, "costr")) costr = atoi(result);
				else if (!strcmp(s, "policy")) strcpy(schedulingPolicy, result);
				else if (!strcmp(s, "V")) V = atof(result);
				else if (!strcmp(s, "flowFile")) strcpy(flowFile, result);
				else if (!strcmp(s, "topoFile")) strcpy(topoFile, result);
				else if (!strcmp(s, "logging")) {
					if (!strcmp(result, "true")) logging = true;
					else if (!strcmp(result, "false")) logging = false;
					else cout << "Error reading logging parameter" << endl;
				}
			}
		}
	}

	string inputDir = "input/";
	// Read topology and instantiate testbed
	Testbed testbed(inputDir + string(topoFile), deltarResource, deltarCommodity, costr, schedulingPolicy);

	// Read service chain definition
	testbed.readService(inputDir + string("service.in"));
	// Read flow allocation and assign arrival generators for each flow
	testbed.readFlow(inputDir + string(flowFile));

	// Initialize node queues and scheduler
	testbed.init(V);

	// Open result file and init
	stringstream ss_V;
	ss_V << fixed << setprecision(1) << V;
	string simIdentifier = "N_" + to_string(testbed.getNumNodes()) + "_t_" + to_string(simTime) 
							+ "_deltarResource_" + to_string(deltarResource) 
							+ "_deltarCommodity_" + to_string(deltarCommodity) 
							+ "_costr_" + to_string(costr) + "_" + string(schedulingPolicy) + "_V_" + ss_V.str();
	string logFilename = "output/log/log_" + simIdentifier + ".txt";
	string queueFilename = "output/sim/queue_" + simIdentifier + ".csv";
	string scheduleFilename = "output/sim/schedule_" + simIdentifier + ".csv";
	string costFilename = "output/sim/cost_" + simIdentifier + ".csv";

	Logger logger(logFilename, queueFilename, scheduleFilename, costFilename, false, logging);
	testbed.setLogger(&logger);
	logger << "Scheduling policy = " << schedulingPolicy << endl;

	// Header for each report file
	testbed.initReport();

	testbed.printCost();


	// Simulation loop
	for (int t = 0; t < simTime; t++) {
		// Print time slot
		logger << "Time " << t << endl;
		cout << "Time " << t << "\r";

		testbed.timeIncrement();
		// schedule
		testbed.schedule();
		// transmission and processing
		testbed.run();

		// external arrivals
		testbed.extArrivals(t);

		// report queue lengths, schedules, and costs
		testbed.report(t);

	}

	//queueFile.close();

}

