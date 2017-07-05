#include "scheduler.h"

Scheduler::Scheduler() {} 

void Scheduler::init(std::vector<Node *> n, std::vector<Link *> l,const char* s) {
	nodes = n;
	links = l;
	for (int i = 0; i < nodes.size(); i++) {
		nodeResources.push_back(0);
		nodeRates.push_back(0);
		nodePxPackets.push_back(NULL);
	}
	for (int i = 0; i < links.size(); i++) {
		linkResources.push_back(0);
		linkRates.push_back(0);
		linkTxPackets.push_back(NULL);
	}
	schedulingPolicy = new char[strlen(s)];
	strcpy(schedulingPolicy, s);
}

void Scheduler::assignCost(std::vector<double> nPC, std::vector<std::vector<double>> nAC, 
				std::vector<double> lTC, std::vector<std::vector<double>> lAC) {
	nodePxCosts = std::vector<double>(nPC);
	nodeAllocCosts = std::vector<std::vector<double>>(nAC);
	linkTxCosts = std::vector<double>(lTC);
	linkAllocCosts = std::vector<std::vector<double>>(lAC);
}
void Scheduler::assignCap(std::vector<std::vector<double>> nAC, std::vector<std::vector<double>> lAC) {
	nodeAllocCaps = std::vector<std::vector<double>>(nAC);
	linkAllocCaps = std::vector<std::vector<double>>(lAC);
}


void Scheduler::setParam(double setV) {
	V = setV;
}

void Scheduler::schedule() {
	if (!strcmp(schedulingPolicy, "DCNC")) {
		DCNC();
	} else if (!strcmp(schedulingPolicy, "ADCNC")) {
		ADCNC();
	}
}

void Scheduler::DCNC() {
	// Note: we have assumed that the cost is zero when allocating no resources
	double maxDiff, maxWeight, diff, weight;
	double resource, rate, rateMaxWeight;
	PacketID* maxPid = NULL;
	for (int l = 0; l < links.size(); l++) {
		// Find the packetID with the highest queue length differential
		maxPid = NULL;
		maxDiff = 0;
		for (int p = 0; p < PacketID::packetIDs.size(); p++) {
			diff = links[l]->getQueueDiff(PacketID::packetIDs[p]);
			if (diff > maxDiff) {
				maxDiff = diff;
				maxPid = PacketID::packetIDs[p];
			}
		}
		// Determine the optimal resource allocation (subject to allocation cost) and thus MaxWeight
		resource = 0;
		rate = 0;
		maxWeight = 0;
		if (maxDiff > V * linkTxCosts[l]) {
			for (int r = 0; r < linkAllocCaps[l].size(); r++) {
				weight = (maxDiff - V * linkTxCosts[l]) * linkAllocCaps[l][r] - V * linkAllocCosts[l][r];
				if (weight > maxWeight) {
					maxWeight = weight;
					rate = linkAllocCaps[l][r];
					resource = r;
				}
			}
		}
		// Assign the schedule
		linkTxPackets[l] = maxPid;
		if (maxPid != NULL) {
			linkResources[l] = resource;
			linkRates[l] = rate;
		} else {
			linkResources[l] = 0;
			linkRates[l] = 0;
		}
	}
	for (int n = 0; n < nodes.size(); n++) {
		// Find the packetID with the highest queue length differential
		maxPid = NULL;
		maxDiff = 0;
		for (int p = 0; p < PacketID::packetIDs.size(); p++) {
			diff = nodes[n]->getQueueDiff(PacketID::packetIDs[p]);
			if (diff > maxDiff) {
				maxDiff = diff;
				maxPid = PacketID::packetIDs[p];
			}
		}

		// Determine the optimal resource allocation (subject to allocation cost) and thus MaxWeight
		resource = 0;
		rate = 0;
		maxWeight = 0;
		if (maxDiff > V * nodePxCosts[n]) {
			for (int r = 0; r < nodeAllocCaps[n].size(); r++) {
				weight = (maxDiff - V * nodePxCosts[n]) * nodeAllocCaps[n][r] - V * nodeAllocCosts[n][r];
				if (weight > maxWeight) {
					maxWeight = weight;
					rate = nodeAllocCaps[n][r];
					resource = r;
				}
			}
		}

		// Assign the schedule
		nodePxPackets[n] = maxPid;
		if (maxPid != NULL) {
			nodeResources[n] = resource;
			nodeRates[n] = rate;
		} else {
			nodeResources[n] = 0;
			nodeRates[n] = 0;
		}
	}
}

void Scheduler::ADCNC () {
	// Note: we have assumed that the cost is zero when allocating no resources
	double maxDiff, maxWeight, diff, weight;
	double resource, rate, rateMaxWeight;
	double currentDiff, currentWeight;
	PacketID* maxPid = NULL;
	for (int l = 0; l < links.size(); l++) {
		// Find the packetID with the highest queue length differential
		maxPid = NULL;
		maxDiff = 0;
		for (int p = 0; p < PacketID::packetIDs.size(); p++) {
			diff = links[l]->getQueueDiff(PacketID::packetIDs[p]);
			if (diff > maxDiff) {
				maxDiff = diff;
				maxPid = PacketID::packetIDs[p];
			}
		}

		// Determine the optimal resource allocation (subject to allocation cost) and thus MaxWeight
		resource = 0;
		rate = 0;
		maxWeight = 0;
		if (maxDiff > V * linkTxCosts[l]) {
			for (int r = 0; r < linkAllocCaps[l].size(); r++) {
				weight = (maxDiff - V * linkTxCosts[l]) * linkAllocCaps[l][r] - V * linkAllocCosts[l][r];
				if (weight > maxWeight) {
					maxWeight = weight;
					rate = linkAllocCaps[l][r];
					resource = r;
				}
			}
		}

		// Determine the current weight and determine whether to reconfigure
		currentDiff = 0;
		currentWeight = 0;
		if (linkTxPackets[l] != NULL) {
			currentDiff = links[l]->getQueueDiff(linkTxPackets[l]);
		}
		if (currentDiff > V * linkTxCosts[l]) {
			currentWeight = (currentDiff - V * linkTxCosts[l]) * linkRates[l] - V * linkAllocCosts[l][linkResources[l]];
		}

		// Reconfigure only when the weight difference is larger than the threshold
		if (maxWeight - currentWeight > hysteresis(maxWeight)) {
			linkTxPackets[l] = maxPid;
			if (maxPid != NULL) {
				linkResources[l] = resource;
				linkRates[l] = rate;
			} else {
				linkResources[l] = 0;
				linkRates[l] = 0;
			}
		} 
	}		

	for (int n = 0; n < nodes.size(); n++) {
		// Find the packetID with the highest queue length differential
		maxPid = NULL;
		maxDiff = 0;
		for (int p = 0; p < PacketID::packetIDs.size(); p++) {
			diff = nodes[n]->getQueueDiff(PacketID::packetIDs[p]);
			if (diff > maxDiff) {
				maxDiff = diff;
				maxPid = PacketID::packetIDs[p];
			}
		}

		// Determine the optimal resource allocation (subject to allocation cost) and thus MaxWeight
		resource = 0;
		rate = 0;
		maxWeight = 0;
		if (maxDiff > V * nodePxCosts[n]) {
			for (int r = 0; r < nodeAllocCaps[n].size(); r++) {
				weight = (maxDiff - V * nodePxCosts[n]) * nodeAllocCaps[n][r] - V * nodeAllocCosts[n][r];
				if (weight > maxWeight) {
					maxWeight = weight;
					rate = nodeAllocCaps[n][r];
					resource = r;
				}
			}
		}

		// Determine the current weight and determine whether to reconfigure
		currentDiff = 0;
		currentWeight = 0;
		if (nodePxPackets[n] != NULL) {
			currentDiff = nodes[n]->getQueueDiff(nodePxPackets[n]);
		}
		if (currentDiff > V * nodePxCosts[n]) {
			currentWeight = (currentDiff - V * nodePxCosts[n]) * nodeRates[n] - V * nodeAllocCosts[n][nodeResources[n]];
		}
		// Reconfigure only when the weight difference is larger than the threshold
		if (maxWeight - currentWeight > hysteresis(maxWeight)) {
			nodePxPackets[n] = maxPid;
			if (maxPid != NULL) {
				nodeResources[n] = resource;
				nodeRates[n] = rate;
			} else {
				nodeResources[n] = 0;
				nodeRates[n] = 0;
			}
		} 
	}

}

double Scheduler::hysteresis (double x, double delta, double gamma) {
	// g(x) = (1 - gamma) * x^{1-delta}
	return (1 - gamma) * pow(x, 1-delta);
}

std::tuple<int, int, PacketID*> Scheduler::getScheduleTx(int l) {
	PacketID* pid = linkTxPackets[l];
	return std::make_tuple(linkResources[l], linkRates[l], pid);
}
std::tuple<int, int, PacketID*> Scheduler::getSchedulePx(int n) {
	PacketID* pid = nodePxPackets[n];
	return std::make_tuple(nodeResources[n], nodeRates[n], pid);
}

void Scheduler::initReportSchedule() {
	std::ofstream* file = logger->getSFile();
	*file << "time";
	for (int n = 0; n < nodes.size(); n++) {
		*file << ",node" << n << "_packet" << ",node" << n << "_resource" 
			  << ",node" << n << "_rate";
	}
	for (int l = 0; l < links.size(); l++) {
		*file << ",link" << l << "_packet" << ",link" << l << "_resource" 
			  << ",link" << l << "_rate";
	}
	*file << std::endl;
}
void Scheduler::initReportCost() {
	std::ofstream* file = logger->getCFile();
	*file << "time";
	for (int n = 0; n < nodes.size(); n++) {
		*file << ",node" << n << "_px" << ",node" << n << "_resource" ;
	}
	for (int l = 0; l < links.size(); l++) {
		*file << ",link" << l << "_tx" << ",link" << l << "_resource";
	}
	*file << std::endl;
}
void Scheduler::reportSchedule(int time) {
	std::ofstream* file = logger->getSFile();
	string packetID;

	*file << time;
	for (int n = 0; n < nodes.size(); n++) {
		if (nodePxPackets[n]) packetID = nodePxPackets[n]->getString();
		*file << "," << packetID << "," << nodeResources[n] << "," << nodeRates[n];
	}
	for (int l = 0; l < links.size(); l++) {
		if (linkTxPackets[l]) packetID = linkTxPackets[l]->getString();
		*file << "," << packetID << "," << linkResources[l] << "," << linkRates[l];
	}
	*file << std::endl;
}
void Scheduler::reportCost(int time) {
	std::ofstream* file = logger->getCFile();

	*file << time;
	for (int n = 0; n < nodes.size(); n++) {
		*file << "," << nodePxCosts[n] * nodeRates[n] << "," << nodeAllocCosts[n][nodeResources[n]];
	}
	for (int l = 0; l < links.size(); l++) {
		*file << "," << linkTxCosts[l] * linkRates[l] << "," << linkAllocCosts[l][linkResources[l]];
	}
	*file << std::endl;
}


