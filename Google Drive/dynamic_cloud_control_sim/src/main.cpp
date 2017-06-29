#include <iostream>
#include <fstream>
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

vector<tuple<int, int>> readTopo (string filename) {
	// Read topology from file
	vector<tuple<int, int>> links;
	ifstream topoFile;
	char temp[256];
	int node1, node2;
	topoFile.open(filename);
	if (topoFile.is_open()) {
		while (topoFile.good()) {
			topoFile.getline(temp, 256, ',');
			node1 = std::atoi(temp);
			topoFile.getline(temp, 256, '\n');
			node2 = std::atoi(temp);

			links.push_back(make_tuple(node1, node2));
		}
		topoFile.close();
	} else {
		cout << "Error opening file";
	}	

	return links;
}

vector<tuple<int, int, double>> readFlow (string filename) {
	// Read flow allocation from file
	vector<tuple<int, int, double>> flows;
	ifstream flowFile;
	char temp[256];
	int node1, node2;
	double rate;
	flowFile.open(filename);
	if (flowFile.is_open()) {
		while (flowFile.good()) {
			flowFile.getline(temp, 256, ',');
			node1 = std::atoi(temp);
			flowFile.getline(temp, 256, ',');
			node2 = std::atoi(temp);
			flowFile.getline(temp, 256, '\n');
			rate = std::atof(temp);

			flows.push_back(make_tuple(node1, node2, rate));
		}
		flowFile.close();
	} else {
		cout << "Error opening file";
	}

	return flows;
}


int main(int argc, char* argv[]) {
	// arguments: t=simTime, N=N, deltar=deltar
	int simTime = 10;
	int N = 4;
	int deltar = 0;
	char schedulingPolicy[20] = "DCNC";
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
			}
		}
	}

	Testbed testbed(N, deltar, schedulingPolicy);

	string inputDir = "input/";
	// Build topology
	vector<tuple<int, int>> links = readTopo(inputDir + string("topo.in"));
	testbed.buildTopo(links, 0);

	// Read service chain definition
	testbed.readService(inputDir + string("service.in"));
	// Read flow allocation and assign arrival generators for each flow
	testbed.readFlow(inputDir + string("flow.in"));

	// Initialize node queues and scheduler
	testbed.init();

	// Open result file and init
	string logFilename = "output/log/log.txt";
	string queueFilename = "output/sim/queue_" + string(schedulingPolicy) + "_N_" + to_string(N) + "_t_" + to_string(simTime) 
							+ "_deltar_" + to_string(deltar) + ".csv";

	Logger logger(logFilename, queueFilename, false, true);
	testbed.setLogger(&logger);
	logger << "Scheduling policy = " << schedulingPolicy << endl;

	//ofstream queueFile;
	//queueFile.open(queueFilename);
	auto queueFile = logger.getQFile();
	testbed.initReportQueue();


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

		// report queue lengths
		testbed.reportQueue(t);

	}

	//queueFile.close();

}

