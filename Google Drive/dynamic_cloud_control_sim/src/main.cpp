#include <iostream>
#include <fstream>
#include <stdio.h>
#include <queue>
#include <vector>
#include <tuple>
#include <random>
#include <ctime>
#include "packet.h"
#include "testbed.h"

using namespace std;


vector<tuple<int, int>> readTopo (const char* filename) {
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

vector<tuple<int, int, double>> readFlow (const char* filename) {
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

	// Build topology
	vector<tuple<int, int>> links = readTopo("topo.in");
	testbed.buildTopo(links, 0);

	// Read flow allocation and assign arrival generators for each flow
	vector<tuple<int, int, double>> flows = readFlow("flow.in");
	testbed.addFlow(flows);

	// Initialize node queues and scheduler
	testbed.init();

	// Simulation loop
	for (int t = 0; t < simTime; t++) {
		// Print time slot
		cout << "Time " << t << endl;

		testbed.timeIncrement();
		// schedule
		testbed.scheduleTx();
		// transmission
		testbed.tx();

		// external arrivals
		testbed.extArrivals(t);

		// report queue lengths
		testbed.reportQueue();

	}


}

