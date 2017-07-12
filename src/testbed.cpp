#include "testbed.h"

int Node::numNodes = 0;
int Link::numLinks = 0;

Testbed::Testbed(int num, int deltar, const char* s) :numNodes(num), deltar(deltar) {
	schedulingPolicy = new char[strlen(s)];
	strcpy(schedulingPolicy, s);

	createNodes();
}

void Testbed::createNodes() {
	// Create nodes
	for (int i = 0; i < numNodes; i++) {
		Node* tempNodePointer = new Node(deltar);
		nodes.push_back(tempNodePointer);
	}
}

void Testbed::buildLink(int sender, int receiver, double txCost, vector<double> allocCosts, 
						vector<double> allocCaps, int linkType) {
	// linkType 0:bidirectional, 1:unidirectional
	Link* tempLinkPointer = new Link(deltar);
	tempLinkPointer->setSender(nodes[sender]);
	tempLinkPointer->setReceiver(nodes[receiver]);
	links.push_back(tempLinkPointer);
	nodes[sender]->addOutputLink(tempLinkPointer);

	linkTxCosts.push_back(txCost);
	linkAllocCosts.push_back(allocCosts);
	linkAllocCaps.push_back(allocCaps);

	if (linkType == 0) {
		// add a reverse link
		buildLink(receiver, sender, txCost, allocCosts, allocCaps, 1);
	}
}

/*
void Testbed::buildTopo(vector<tuple<int, int>> connections, int linkType=0) {
	// linkType 0:bidirectional, 1:unidirectional
	int sender, receiver;
	for (auto it = connections.begin(); it != connections.end(); it++) {
		sender = get<0>(*it);
		receiver = get<1>(*it);

		double costsArray[] = {0, 1, 3, 6, 10};
		std::vector<double> allocCosts (costsArray, costsArray + sizeof(costsArray) / sizeof(double) );

		double capsArray[] = {0, 1, 2, 3, 4};
		std::vector<double> allocCaps (capsArray, capsArray + sizeof(capsArray) / sizeof(double) );

		buildLink(sender, receiver, 1, allocCosts, allocCaps, linkType);
	}
}
*/

void Testbed::readService (string filename) {
	// Read service chain definition from file
	// Service chain format: service, numStage, [scaling ratio]
	ifstream flowFile;
	char temp[256];
	int service, numStage;
	vector<int> scaling;
	flowFile.open(filename);
	if (flowFile.is_open()) {
		while (flowFile.good()) {
			flowFile.getline(temp, 256, ',');
			service = std::atoi(temp);
			flowFile.getline(temp, 256, ',');
			numStage = std::atoi(temp);

			scaling.clear();
			for (int i = 1; i < numStage; i++) {
				flowFile.getline(temp, 256, ',');
				scaling.push_back(std::atoi(temp));
			}
			

			flowFile.getline(temp, 256, '\n');
			scaling.push_back(std::atoi(temp));

			PacketID::numServiceStage[service] = numStage;
			//PacketID::serviceScaling.insert(make_pair(service, scaling));
			PacketID::serviceScaling[service] = scaling;
		}
		flowFile.close();
	} else {
		cout << "Error opening file";
	}
}

void Testbed::readFlow (string filename) {
	// Read flow allocation from file
	// Flow format: src, dst, service, numStage, rate
	ifstream flowFile;
	char temp[256];
	int src, dst, service;
	double rate;
	flowFile.open(filename);
	if (flowFile.is_open()) {
		while (flowFile.good()) {
			flowFile.getline(temp, 256, ',');
			src = std::atoi(temp);
			flowFile.getline(temp, 256, ',');
			dst = std::atoi(temp);
			flowFile.getline(temp, 256, ',');
			service = std::atoi(temp);
			flowFile.getline(temp, 256, '\n');
			rate = std::atof(temp);

			addFlow(src, dst, service, rate);
		}
		flowFile.close();
	} else {
		cout << "Error opening file";
	}
}
void Testbed::addFlow(int src, int dst, int service, double rate) {
	PacketID* pid;

	// Check if a matched packet identifier exists, otherwise create one
	PacketID tempID = PacketID(dst, service);
	pid = NULL;
	for (int i = 0; i < PacketID::packetIDs.size(); i++) {
		if ( *(PacketID::packetIDs[i]) == tempID ) {
			pid = PacketID::packetIDs[i];
		}
	}
	if (pid == NULL) {
		// Insert flow generator for the first commodity in the service chain
		pid = new PacketID(dst, service, 0);
		PacketID::packetIDs.push_back(pid);
		nodes[src]->addFlow(pid, rate);

		// Create packetID for the rest in the service chain
		int numStage = PacketID::numServiceStage[service];
		for (int k = 1; k < numStage+1; k++) {
			pid = new PacketID(dst, service, k);
			PacketID::packetIDs.push_back(pid);
		}
	}
}

void Testbed::init(double V){
	// Initialize queues and for each node after creation of all nodes
	// one queue for each node/destination
	for (int i = 0; i < numNodes; i++) {
		nodes[i]->initQueues();
	}

	// Initialize scheduler
	scheduler.init(nodes, links, schedulingPolicy);
	setParam(V);
}

void Testbed::setParam(double V) {
	scheduler.setParam(V);
	//assignCost();
	//assignCap();
	scheduler.assignCost(nodePxCosts, nodeAllocCosts, linkTxCosts, linkAllocCosts);
	scheduler.assignCap(nodeAllocCaps, linkAllocCaps);
}
void Testbed::setLogger(Logger* _logger) {
	for (int n = 0; n < nodes.size(); n++) {
		nodes[n]->setLogger(_logger);
	}
	for (int l = 0; l < links.size(); l++) {
		links[l]->setLogger(_logger);
	}
	scheduler.setLogger(_logger);
	logger = _logger;
}

vector<double> Testbed::parseAllocation(string s) {
	size_t pos = 0;
	string temp;
	vector<double> allocation;
	while ((pos = s.find("_")) != string::npos) {
		temp = s.substr(0, pos);
		allocation.push_back(std::stod(temp));
		s.erase(0, pos + 1);
	}

	temp = s.substr(0, string::npos);
	allocation.push_back(std::stod(temp));

	return allocation;
}
void Testbed::readTopo (string filename) {
	// Read topology and parameters from file
	// Node format: nodeID, px cost, alloc costs, alloc caps
	// Link format: sender, receiver, tx cost, alloc costs, alloc caps
	vector<tuple<int, int>> links;
	ifstream topoFile;
	string temp;
	int N, node, sender, receiver;
	double pxCost, txCost;
	vector<double> allocCosts, allocCaps;

	topoFile.open(filename);
	if (topoFile.is_open()) {
		// Read number of nodes
		getline(topoFile, temp,'\n');
		N = std::stoi(temp);

		// Read node parameters
		for (int n = 0; n < N; n++) {
			getline(topoFile, temp,',');
			node = std::stoi(temp);
			assert(node == n);

			// get px cost
			getline(topoFile, temp,',');
			pxCost = std::stod(temp);
			nodePxCosts.push_back(pxCost);

			// parse allocation costs
			getline(topoFile, temp,',');
			allocCosts = parseAllocation(temp);
			nodeAllocCosts.push_back(allocCosts);

			// parse allocation capacity
			getline(topoFile, temp,'\n');
			allocCaps = parseAllocation(temp);
			nodeAllocCaps.push_back(allocCaps);
		}

		// Read connections
		while (topoFile.good()) {
			getline(topoFile, temp,',');
			sender = std::stoi(temp);
			getline(topoFile, temp,',');
			receiver = std::stoi(temp);

			// get tx cost
			getline(topoFile, temp,',');
			txCost = std::stod(temp);

			// parse allocation costs
			getline(topoFile, temp,',');
			allocCosts = parseAllocation(temp);

			// parse allocation capacity
			getline(topoFile, temp,'\n');
			allocCaps = parseAllocation(temp);

			buildLink(sender, receiver, txCost, allocCosts, allocCaps);
		}
		topoFile.close();
	} else {
		cout << "Error opening file";
	}	
}

/*
void Testbed::assignCost() {
	double costsArray[] = {0, 1, 3, 6, 10};
	std::vector<double> allocCosts (costsArray, costsArray + sizeof(costsArray) / sizeof(double) );
	for (int i = 0; i < nodes.size(); i++) {
		// cost structure (need revise)
		nodePxCosts.push_back(1);
		std::vector<double> temp(allocCosts);
		nodeAllocCosts.push_back(temp);
	}
	for (int i = 0; i < links.size(); i++) {
		// cost structure (need revise)
		linkTxCosts.push_back(1);
		std::vector<double> temp(allocCosts); 
		linkAllocCosts.push_back(temp);
	}

	scheduler.assignCost(nodePxCosts, nodeAllocCosts, linkTxCosts, linkAllocCosts);
}

void Testbed::assignCap() {
	double capsArray[] = {0, 1, 2, 3, 4};
	std::vector<double> allocCaps (capsArray, capsArray + sizeof(capsArray) / sizeof(double) );
	for (int i = 0; i < nodes.size(); i++) {
		std::vector<double> temp(allocCaps);
		nodeAllocCaps.push_back(temp);
	}
	for (int i = 0; i < links.size(); i++) {
		std::vector<double> temp(allocCaps); 
		linkAllocCaps.push_back(temp);
	}

	scheduler.assignCap(nodeAllocCaps, linkAllocCaps);
}
*/

void Testbed::timeIncrement() {
	for (int n = 0; n < numNodes; n++) {
		nodes[n]->timeIncrement();
	}
	for (int l = 0; l < links.size(); l++) {
		links[l]->timeIncrement();
	}
}
void Testbed::schedule() {
	scheduler.schedule();
}
void Testbed::run() {
	PacketID* pid;
	int resource, rate;

	for (int l = 0; l < links.size(); l++) {
		// retrieve tx schedule for each link from scheduler
		auto alloc = scheduler.getScheduleTx(l);
		resource = std::get<0>(alloc);
		rate = std::get<1>(alloc);
		pid = std::get<2>(alloc);
		if ((rate == 0) || (pid == NULL)) continue;

		// prepare tx (check if reconfiguration occurs)
		links[l]->prepareTx(resource, pid);
		// start tx
		links[l]->getSender()->tx(links[l], pid, rate);
	}

	for (int n = 0; n < nodes.size(); n++) {
		// retrieve tx schedule for each link from scheduler
		auto alloc = scheduler.getSchedulePx(n);
		resource = std::get<0>(alloc);
		rate = std::get<1>(alloc);
		pid = std::get<2>(alloc);
		if ((rate == 0) || (pid == NULL)) continue;

		// prepare px (check if reconfiguration occurs)
		nodes[n]->preparePx(resource, pid);
		// start tx
		nodes[n]->px(pid, rate);
	}
}
void Testbed::extArrivals(int t) {
	for (int i = 0; i < numNodes; i++) {
		nodes[i]->extArrivals(t);
	}
}
void Testbed::initReport() {
	std::ofstream* file = logger->getQFile();
	*file << "time";
	for (int i = 0; i < numNodes; i++) {
		nodes[i]->initReportQueue();
	}
	*file << std::endl;

	scheduler.initReportSchedule();
	scheduler.initReportCost();
}

void Testbed::report(int t) {
	std::ofstream* file = logger->getQFile();
	*file << t;
	for (int i = 0; i < numNodes; i++) {
		nodes[i]->reportQueue();
	}
	*file << std::endl;

	scheduler.reportSchedule(t);
	scheduler.reportCost(t);
}

void Testbed::printCost() {
	for (int i = 0; i < nodePxCosts.size(); i++) {
		std::cout << nodePxCosts[i] << ",";
	}
	std::cout << std::endl;
	for (int i = 0; i < linkTxCosts.size(); i++) {
		std::cout << linkTxCosts[i] << ",";
	}
	std::cout << std::endl;
	for (int i = 0; i < nodeAllocCosts.size(); i++) {
		for (int j = 0; j < nodeAllocCosts[i].size(); j++) {
			std::cout << nodeAllocCosts[i][j] << ",";
		}
	}
	std::cout << std::endl;
	for (int i = 0; i < linkAllocCosts.size(); i++) {
		for (int j = 0; j < linkAllocCosts[i].size(); j++) {
			std::cout << linkAllocCosts[i][j] << ",";
		}
	}
	std::cout << std::endl;
	for (int i = 0; i < nodeAllocCaps.size(); i++) {
		for (int j = 0; j < nodeAllocCaps[i].size(); j++) {
			std::cout << nodeAllocCaps[i][j] << ",";
		}
	}
	std::cout << std::endl;
	for (int i = 0; i < linkAllocCaps.size(); i++) {
		for (int j = 0; j < linkAllocCaps[i].size(); j++) {
			std::cout << linkAllocCaps[i][j] << ",";
		}
	}
	std::cout << std::endl;
}

