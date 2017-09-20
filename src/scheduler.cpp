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
	} else if (!strcmp(schedulingPolicy, "EADCNC")) {
		EADCNC();
	} else if (!strcmp(schedulingPolicy, "ADCNC2Stage")) {
		ADCNC2Stage();
	} else if (!strcmp(schedulingPolicy, "ADCNCR")) {
		ADCNCR();
	}
}

enum Operation {PROCESSING = 0, TRANSMISSION};

std::tuple<int, PacketID*, double, double, double> Scheduler::getMaxWeight(int ind, int operation) {
	// Note: we have assumed that the cost is zero when allocating no resources
	double maxDiff = 0, maxWeight = 0, maxScore = 0;
	double diff, weight;
	int resource = 0;
	PacketID* maxPid = NULL;

	switch(operation) {
		case PROCESSING:
			// Find the packetID with the highest queue length differential
			for (int p = 0; p < PacketID::packetIDs.size(); p++) {
				diff = nodes[ind]->getQueueDiff(PacketID::packetIDs[p]);
				if (diff > maxDiff) {
					maxDiff = diff;
					maxPid = PacketID::packetIDs[p];
				}
			}

			// Determine the optimal resource allocation (subject to allocation cost) and thus MaxWeight
			maxScore = std::max(0.0, maxDiff - V * nodePxCosts[ind]);
			if (maxScore > 0) {
				for (int r = 0; r < nodeAllocCaps[ind].size(); r++) {
					weight = maxScore * nodeAllocCaps[ind][r] - V * nodeAllocCosts[ind][r];
					if (weight > maxWeight) {
						maxWeight = weight;
						//rate = nodeAllocCaps[ind][r];
						resource = r;
					}
				}
			}
			if (resource == 0) {
				maxPid = NULL;
			}
			return std::make_tuple(resource, maxPid, maxWeight, maxDiff, maxScore);
		case TRANSMISSION:
			// Find the packetID with the highest queue length differential
			for (int p = 0; p < PacketID::packetIDs.size(); p++) {
				diff = links[ind]->getQueueDiff(PacketID::packetIDs[p]);
				if (diff > maxDiff) {
					maxDiff = diff;
					maxPid = PacketID::packetIDs[p];
				}
			}
			// Determine the optimal resource allocation (subject to allocation cost) and thus MaxWeight
			maxScore = std::max(0.0, maxDiff - V * linkTxCosts[ind]);
			if (maxScore > 0) {
				for (int r = 0; r < linkAllocCaps[ind].size(); r++) {
					weight = maxScore * linkAllocCaps[ind][r] - V * linkAllocCosts[ind][r];
					if (weight > maxWeight) {
						maxWeight = weight;
						//rate = linkAllocCaps[ind][r];
						resource = r;
					}
				}
			}
			if (resource == 0) {
				maxPid = NULL;
			}
			return std::make_tuple(resource, maxPid, maxWeight, maxDiff, maxScore);
		default:
			resource = 0;
			maxPid = NULL;
			return std::make_tuple(resource, maxPid, maxWeight, maxDiff, maxScore);
	}	
}

std::tuple<double, double, double> Scheduler::getCurrentWeight(int ind, int operation) {
	double currentDiff = 0, currentWeight = 0, currentScore = 0;

	switch(operation) {
		case PROCESSING:
			if (nodePxPackets[ind] != NULL) {
				currentDiff = nodes[ind]->getQueueDiff(nodePxPackets[ind]);
			}
			currentScore = std::max(0.0, currentDiff - V * nodePxCosts[ind]);
			if (currentScore > 0) {
				currentWeight = currentScore * nodeRates[ind] - V * nodeAllocCosts[ind][nodeResources[ind]];
			} else {
				currentWeight = - V * nodeAllocCosts[ind][nodeResources[ind]];
			}
			return std::make_tuple(currentWeight, currentDiff, currentScore);
		case TRANSMISSION:
			if (linkTxPackets[ind] != NULL) {
				currentDiff = links[ind]->getQueueDiff(linkTxPackets[ind]);
			}
			currentScore = std::max(0.0, currentDiff - V * linkTxCosts[ind]);
			if (currentScore > 0) {
				currentWeight = currentScore * linkRates[ind] - V * linkAllocCosts[ind][linkResources[ind]];
			} else {
				currentWeight = - V * linkAllocCosts[ind][linkResources[ind]];
			}
			return std::make_tuple(currentWeight, currentDiff, currentScore);
		default:
			return std::make_tuple(currentWeight, currentDiff, currentScore);
	}	
}



void Scheduler::DCNC() {
	int resource = 0;
	PacketID* maxPid = NULL;

	for (int n = 0; n < nodes.size(); n++) {
		auto optimal = getMaxWeight(n, PROCESSING);
		resource = std::get<0>(optimal);
		maxPid = std::get<1>(optimal);

		// Assign the schedule
		nodePxPackets[n] = maxPid;
		if (maxPid != NULL) {
			nodeResources[n] = resource;
			nodeRates[n] = nodeAllocCaps[n][resource];
		} else {
			nodeResources[n] = 0;
			nodeRates[n] = 0;
		}
	}
	for (int l = 0; l < links.size(); l++) {
		auto optimal = getMaxWeight(l, TRANSMISSION);
		resource = std::get<0>(optimal);
		maxPid = std::get<1>(optimal);

		// Assign the schedule
		linkTxPackets[l] = maxPid;
		if (maxPid != NULL) {
			linkResources[l] = resource;
			linkRates[l] = linkAllocCaps[l][resource];
		} else {
			linkResources[l] = 0;
			linkRates[l] = 0;
		}
	}
}


void Scheduler::ADCNC () {
	int resource = 0;
	PacketID* maxPid = NULL;
	double maxWeight = 0, maxDiff = 0, weight, diff;
	double rate = 0;
	double currentDiff, currentWeight;

	for (int n = 0; n < nodes.size(); n++) {
		auto optimal = getMaxWeight(n, PROCESSING);
		resource = std::get<0>(optimal);
		maxPid = std::get<1>(optimal);
		maxWeight = std::get<2>(optimal);
		maxDiff = std::get<3>(optimal);

		// Determine the current weight
		auto current = getCurrentWeight(n, PROCESSING);
		currentWeight = std::get<0>(current);
		currentDiff = std::get<1>(current);
		
		// Reconfigure only when the weight difference is larger than the threshold
		//if ((maxWeight - currentWeight > hysteresis(maxWeight)) || (maxWeight == 0)) {
		if ((maxWeight - currentWeight > hysteresis(maxDiff * nodeRates[n]))) {
		//if ((maxWeight - currentWeight > hysteresis(maxDiff * std::max(nodeRates[n], rate) ))) {
			nodePxPackets[n] = maxPid;
			if (maxPid != NULL) {
				nodeResources[n] = resource;
				nodeRates[n] = nodeAllocCaps[n][resource];
			} else {
				nodeResources[n] = 0;
				nodeRates[n] = 0;
				nodePxPackets[n] = NULL;
			}
		}  else {
			// Set the rate to zero if queue differential <= 0
			if (currentDiff <= 0) {
				nodeResources[n] = 0;
				nodeRates[n] = 0;
				nodePxPackets[n] = NULL;
			}
		}

	}

	for (int l = 0; l < links.size(); l++) {
		auto optimal = getMaxWeight(l, TRANSMISSION);
		resource = std::get<0>(optimal);
		maxPid = std::get<1>(optimal);
		maxWeight = std::get<2>(optimal);
		maxDiff = std::get<3>(optimal);
		
		// Determine the current weight
		auto current = getCurrentWeight(l, TRANSMISSION);
		currentWeight = std::get<0>(current);
		currentDiff = std::get<1>(current);


		// Reconfigure only when the weight difference is larger than the threshold
		//if ((maxWeight - currentWeight > hysteresis(maxWeight)) || (maxWeight == 0)) {
		if ((maxWeight - currentWeight > hysteresis(maxDiff * linkRates[l]))) {
		//if ((maxWeight - currentWeight > hysteresis(maxDiff * std::max(linkRates[l], rate) ))) {
			linkTxPackets[l] = maxPid;
			if (maxPid != NULL) {
				linkResources[l] = resource;
				linkRates[l] = linkAllocCaps[l][resource];
			} else {
				linkResources[l] = 0;
				linkRates[l] = 0;
				linkTxPackets[l] = NULL;
			}
		} else {
			// Set the rate to zero if queue differential <= 0
			if (currentDiff <= 0) {
				linkResources[l] = 0;
				linkRates[l] = 0;
				linkTxPackets[l] = NULL;
			}
		}
	}
}

void Scheduler::ADCNCR () {
	// ADCNC-Resource: Only apply adaptivity on resource reconfiguration
	// use optimal commodity at every time slot
	int resource = 0;
	PacketID* maxPid = NULL;
	double maxWeight = 0, maxDiff = 0, weight, diff;
	double rate = 0;
	double currentDiff, currentWeight;

	for (int n = 0; n < nodes.size(); n++) {
		auto optimal = getMaxWeight(n, PROCESSING);
		resource = std::get<0>(optimal);
		maxPid = std::get<1>(optimal);
		maxWeight = std::get<2>(optimal);
		maxDiff = std::get<3>(optimal);

		// Use the optimal commodity
		nodePxPackets[n] = maxPid;

		// Determine the current weight (based on updated optimal commodity)
		auto current = getCurrentWeight(n, PROCESSING);
		currentWeight = std::get<0>(current);
		currentDiff = std::get<1>(current);
		
		// Reconfigure only when the weight difference is larger than the threshold
		if ((maxWeight - currentWeight > hysteresis(maxDiff * nodeRates[n]))) {
			if (maxPid != NULL) {
				nodeResources[n] = resource;
				nodeRates[n] = nodeAllocCaps[n][resource];
			} else {
				nodeResources[n] = 0;
				nodeRates[n] = 0;
				nodePxPackets[n] = NULL;
			}
		}  else {
			if (nodeResources[n] == 0) {
				nodePxPackets[n] = NULL;
			}
			// Set the rate to zero if queue differential <= 0
			if (currentDiff <= 0) {
				nodeResources[n] = 0;
				nodeRates[n] = 0;
				nodePxPackets[n] = NULL;
			}
		}

	}

	for (int l = 0; l < links.size(); l++) {
		auto optimal = getMaxWeight(l, TRANSMISSION);
		resource = std::get<0>(optimal);
		maxPid = std::get<1>(optimal);
		maxWeight = std::get<2>(optimal);
		maxDiff = std::get<3>(optimal);
		
		// Use the optimal commodity
		linkTxPackets[l] = maxPid;

		// Determine the current weight (based on updated optimal commodity)
		auto current = getCurrentWeight(l, TRANSMISSION);
		currentWeight = std::get<0>(current);
		currentDiff = std::get<1>(current);


		// Reconfigure only when the weight difference is larger than the threshold
		if ((maxWeight - currentWeight > hysteresis(maxDiff * linkRates[l]))) {
			if (maxPid != NULL) {
				linkResources[l] = resource;
				linkRates[l] = linkAllocCaps[l][resource];
			} else {
				linkResources[l] = 0;
				linkRates[l] = 0;
				linkTxPackets[l] = NULL;
			}
		} else {
			// Set the rate to zero if queue differential <= 0
			if (currentDiff <= 0) {
				linkResources[l] = 0;
				linkRates[l] = 0;
				linkTxPackets[l] = NULL;
			}
		}
	}
}


void Scheduler::ADCNC2Stage () {
	int resource = 0;
	PacketID* maxPid = NULL;
	double maxWeight = 0, maxDiff = 0, maxScore = 0;
	double currentDiff, currentWeight, currentScore;

	for (int n = 0; n < nodes.size(); n++) {
		auto optimal = getMaxWeight(n, PROCESSING);
		resource = std::get<0>(optimal);
		maxPid = std::get<1>(optimal);
		maxWeight = std::get<2>(optimal);
		maxDiff = std::get<3>(optimal);
		maxScore = std::get<4>(optimal);

		// Determine the current weight
		auto current = getCurrentWeight(n, PROCESSING);
		currentWeight = std::get<0>(current);
		currentDiff = std::get<1>(current);
		currentScore = std::get<2>(current);
		
		// Reconfigure only when the weight difference is larger than the threshold
		//if ((maxWeight - currentWeight > hysteresis(maxWeight)) || (maxWeight == 0)) {
		if ((maxWeight - currentWeight > hysteresis(maxDiff * nodeRates[n]))) {
		//if ((maxWeight - currentWeight > hysteresis(maxDiff * std::max(nodeRates[n], rate) ))) {
			nodePxPackets[n] = maxPid;
			if (maxPid != NULL) {
				nodeResources[n] = resource;
				nodeRates[n] = nodeAllocCaps[n][resource];
			} else {
				nodeResources[n] = 0;
				nodeRates[n] = 0;
				nodePxPackets[n] = NULL;
			}
		}  else {
			//if (nodeRates[n] * (maxScore - currentScore) > hysteresis(maxDiff * nodeRates[n])) {
			//if (maxScore - currentScore > hysteresis(maxDiff)) {
			if (maxDiff - currentDiff > hysteresis(maxDiff) / 10) {
				nodePxPackets[n] = maxPid;
			}
			// Set the rate to zero if queue differential <= 0
			if (currentDiff <= 0) {
				nodeResources[n] = 0;
				nodeRates[n] = 0;
				nodePxPackets[n] = NULL;
			}
		}

	}

	for (int l = 0; l < links.size(); l++) {
		auto optimal = getMaxWeight(l, TRANSMISSION);
		resource = std::get<0>(optimal);
		maxPid = std::get<1>(optimal);
		maxWeight = std::get<2>(optimal);
		maxDiff = std::get<3>(optimal);
		maxScore = std::get<4>(optimal);

		// Determine the current weight
		auto current = getCurrentWeight(l, TRANSMISSION);
		currentWeight = std::get<0>(current);
		currentDiff = std::get<1>(current);
		currentScore = std::get<2>(current);

		// Reconfigure only when the weight difference is larger than the threshold
		//if ((maxWeight - currentWeight > hysteresis(maxWeight)) || (maxWeight == 0)) {
		if ((maxWeight - currentWeight > hysteresis(maxDiff * linkRates[l]))) {
		//if ((maxWeight - currentWeight > hysteresis(maxDiff * std::max(linkRates[l], rate) ))) {
			linkTxPackets[l] = maxPid;
			if (maxPid != NULL) {
				linkResources[l] = resource;
				linkRates[l] = linkAllocCaps[l][resource];
			} else {
				linkResources[l] = 0;
				linkRates[l] = 0;
				linkTxPackets[l] = NULL;
			}
		} else {
			//if (linkRates[l] * (maxScore - currentScore) > hysteresis(maxDiff * linkRates[l])) {
			//if (maxScore - currentScore > hysteresis(maxDiff)) {
			if (maxDiff - currentDiff > hysteresis(maxDiff) / 10) {
				linkTxPackets[l] = maxPid;
			}
			// Set the rate to zero if queue differential <= 0
			if (currentDiff <= 0) {
				linkResources[l] = 0;
				linkRates[l] = 0;
				linkTxPackets[l] = NULL;
			}
		}
	}
}




void Scheduler::EADCNC () {
	// Note: we have assumed that the cost is zero when allocating no resources
	int resource = 0;
	PacketID* maxPid = NULL;
	double maxWeight = 0, maxDiff = 0;
	double currentDiff, currentWeight;
	int costr;
	for (int n = 0; n < nodes.size(); n++) {
		auto optimal = getMaxWeight(n, PROCESSING);
		resource = std::get<0>(optimal);
		maxPid = std::get<1>(optimal);
		maxWeight = std::get<2>(optimal);
		maxDiff = std::get<3>(optimal);


		// Determine the current weight and determine whether to reconfigure
		currentDiff = 0;
		currentWeight = 0;
		if (nodePxPackets[n] != NULL) {
			currentDiff = nodes[n]->getQueueDiff(nodePxPackets[n]);
		}
		if (currentDiff > V * nodePxCosts[n]) {
			currentWeight = (currentDiff - V * nodePxCosts[n]) * nodeRates[n] 
							- V * nodeAllocCosts[n][nodeResources[n]];
		} else {
			currentWeight = - V * nodeAllocCosts[n][nodeResources[n]];
		}
		
		// Reconfigure only when the weight difference is larger than the threshold
		// EADCNC: Add reconfiguration cost as bias term
		//if ((maxWeight - currentWeight > hysteresis(maxWeight)) || (maxWeight == 0)) {
		costr = nodes[n]->getCostr();
		if ((maxWeight - currentWeight > hysteresis(maxDiff * nodeRates[n] + costr) )) {
			nodePxPackets[n] = maxPid;
			if (maxPid != NULL) {
				nodeResources[n] = resource;
				nodeRates[n] = nodeAllocCaps[n][resource];
			} else {
				nodeResources[n] = 0;
				nodeRates[n] = 0;
				nodePxPackets[n] = NULL;
			}
		}  else {
			// Set the rate to zero if queue differential <= 0
			if (currentDiff <= 0) {
				nodeResources[n] = 0;
				nodeRates[n] = 0;
				nodePxPackets[n] = NULL;
			}
		}

	}

	for (int l = 0; l < links.size(); l++) {
		auto optimal = getMaxWeight(l, TRANSMISSION);
		resource = std::get<0>(optimal);
		maxPid = std::get<1>(optimal);
		maxWeight = std::get<2>(optimal);
		maxDiff = std::get<3>(optimal);

		// Determine the current weight and determine whether to reconfigure
		currentDiff = 0;
		currentWeight = 0;
		if (linkTxPackets[l] != NULL) {
			currentDiff = links[l]->getQueueDiff(linkTxPackets[l]);
		}
		if (currentDiff > V * linkTxCosts[l]) {
			currentWeight = (currentDiff - V * linkTxCosts[l]) * linkRates[l] 
							- V * linkAllocCosts[l][linkResources[l]];
		} else {
			currentWeight = - V * linkAllocCosts[l][linkResources[l]];
		}

		// Reconfigure only when the weight difference is larger than the threshold
		// EADCNC: Add reconfiguration cost as bias term
		//if ((maxWeight - currentWeight > hysteresis(maxWeight)) || (maxWeight == 0)) {
		costr = links[l]->getCostr();
		if ((maxWeight - currentWeight > hysteresis(maxDiff * linkRates[l] + costr))) {
			linkTxPackets[l] = maxPid;
			if (maxPid != NULL) {
				linkResources[l] = resource;
				linkRates[l] = linkAllocCaps[l][resource];
			} else {
				linkResources[l] = 0;
				linkRates[l] = 0;
				linkTxPackets[l] = NULL;
			}
		} else {
			// Set the rate to zero if queue differential <= 0
			if (currentDiff <= 0) {
				linkResources[l] = 0;
				linkRates[l] = 0;
				linkTxPackets[l] = NULL;
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
			  << ",node" << n << "_rate" << ",node" << n << "_reconfig";
	}
	for (int l = 0; l < links.size(); l++) {
		*file << ",link" << links[l]->getString() << "_packet" 
			  << ",link" << links[l]->getString() << "_resource" 
			  << ",link" << links[l]->getString() << "_rate" 
			  << ",link" << links[l]->getString() << "_reconfig";
	}
	*file << std::endl;
}
void Scheduler::initReportCost() {
	std::ofstream* file = logger->getCFile();
	*file << "time";
	for (int n = 0; n < nodes.size(); n++) {
		*file << ",node" << n << "_px" << ",node" << n << "_resource" << ",node" << n << "_reconfig";
	}
	for (int l = 0; l < links.size(); l++) {
		*file << ",link" << links[l]->getString() << "_tx" 
			  << ",link" << links[l]->getString() << "_resource"
			  << ",link" << links[l]->getString() << "_reconfig";
	}
	*file << std::endl;
}
void Scheduler::reportSchedule(int time) {
	std::ofstream* file = logger->getSFile();
	string packetID;

	*file << time;
	for (int n = 0; n < nodes.size(); n++) {
		if (nodePxPackets[n]) packetID = nodePxPackets[n]->getString();
		*file << "," << packetID << "," << nodeResources[n] << "," << nodeRates[n] 
			<< "," << nodes[n]->getReconfig();
	}
	for (int l = 0; l < links.size(); l++) {
		if (linkTxPackets[l]) packetID = linkTxPackets[l]->getString();
		*file << "," << packetID << "," << linkResources[l] << "," << linkRates[l] 
			<< "," << links[l]->getReconfig();
	}
	*file << std::endl;
}
void Scheduler::reportCost(int time) {
	std::ofstream* file = logger->getCFile();

	*file << time;
	for (int n = 0; n < nodes.size(); n++) {
		*file << "," << nodePxCosts[n] * nodeRates[n] << "," << nodeAllocCosts[n][nodeResources[n]] 
			  << "," << nodes[n]->getReconfigCost();
	}
	for (int l = 0; l < links.size(); l++) {
		*file << "," << linkTxCosts[l] * linkRates[l] << "," << linkAllocCosts[l][linkResources[l]]
			  << "," << links[l]->getReconfigCost();
	}
	*file << std::endl;
}


