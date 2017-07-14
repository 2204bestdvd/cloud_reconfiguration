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

int readNumNodes(string filename) {
	ifstream topoFile;
	string temp;
	int N;

	topoFile.open(filename);
	if (topoFile.is_open()) {
		// Read number of nodes
		getline(topoFile, temp,'\n');
		N = std::stoi(temp);
		topoFile.close();
	} else {
		cout << "Error opening file";
	}	

	return N;
}

int main(int argc, char* argv[]) {
	srand(time(0));

	// arguments: t=simTime, N=N, deltar=deltar
	int simTime = 10;
	int N = 4;
	int deltar = 0;
	char schedulingPolicy[20] = "DCNC";
	double V = 1;
	bool logging = true;
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
				else if (!strcmp(s, "N")) N = atoi(result);
				else if (!strcmp(s, "deltar")) deltar = atoi(result);
				else if (!strcmp(s, "policy")) strcpy(schedulingPolicy, result);
				else if (!strcmp(s, "V")) V = atof(result);
				else if (!strcmp(s, "logging")) {
					if (!strcmp(result, "true")) logging = true;
					else if (!strcmp(result, "false")) logging = false;
					else cout << "Error reading logging parameter" << endl;
				}
			}
		}
	}

	string inputDir = "input/";
	// Read number of nodes and instantiate testbed
	N = readNumNodes(inputDir + string("topo.in"));
	Testbed testbed(N, deltar, schedulingPolicy);

	// Read topology
	testbed.readTopo(inputDir + string("topo.in"));
	//vector<tuple<int, int>> links = readTopo(inputDir + string("topo.in"), N);
	//testbed.buildTopo(links, 0);

	// Read service chain definition
	testbed.readService(inputDir + string("service.in"));
	// Read flow allocation and assign arrival generators for each flow
	testbed.readFlow(inputDir + string("flow.in"));

	// Initialize node queues and scheduler
	testbed.init(V);

	// Open result file and init
	stringstream ss_V;
	ss_V << fixed << setprecision(1) << V;
	string simIdentifier = "N_" + to_string(N) + "_t_" + to_string(simTime) + "_deltar_" + to_string(deltar) 
							+ "_" + string(schedulingPolicy) + "_V_" + ss_V.str();
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

