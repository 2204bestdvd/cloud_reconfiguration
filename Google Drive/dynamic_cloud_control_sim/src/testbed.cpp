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
void Testbed::buildTopo(vector<tuple<int, int>> connections, int linkType=0) {
	// linkType 0:bidirectional, 1:unidirectional
	int sender, receiver;
	for (auto it = connections.begin(); it != connections.end(); it++) {
		sender = get<0>(*it);
		receiver = get<1>(*it);
		//nodes[sender]->addNeighbor(nodes[receiver]);

		Link* tempLinkPointer = new Link(deltar);
		tempLinkPointer->setSender(nodes[sender]);
		tempLinkPointer->setReceiver(nodes[receiver]);
		links.push_back(tempLinkPointer);
		nodes[sender]->addOutputLink(tempLinkPointer);
		if (linkType == 0) {
			//nodes[receiver]->addNeighbor(nodes[sender]);
			Link* tempLinkPointer = new Link(deltar);
			tempLinkPointer->setSender(nodes[receiver]);
			tempLinkPointer->setReceiver(nodes[sender]);			
			links.push_back(tempLinkPointer);
			nodes[receiver]->addOutputLink(tempLinkPointer);
		}

	}
}

void Testbed::readService (string filename) {
	// Read service chain definition from file
	// Service chain format: service, numStage
	ifstream flowFile;
	char temp[256];
	int service, numStage;
	flowFile.open(filename);
	if (flowFile.is_open()) {
		while (flowFile.good()) {
			flowFile.getline(temp, 256, ',');
			service = std::atoi(temp);
			flowFile.getline(temp, 256, '\n');
			numStage = std::atoi(temp);

			PacketID::numServiceStage[service] = numStage;
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

void Testbed::init(){
	// Initialize queues and for each node after creation of all nodes
	// one queue for each node/destination
	for (int i = 0; i < numNodes; i++) {
		nodes[i]->initQueues();
	}

	// Initialize scheduler
	scheduler.init(nodes, links, schedulingPolicy);
	setParam();
}

void Testbed::setParam() {
	scheduler.setParam();
	assignCost();
	assignCap();
}
void Testbed::setLogger(Logger* _logger) {
	for (int n = 0; n < nodes.size(); n++) {
		nodes[n]->setLogger(_logger);
	}
	for (int l = 0; l < links.size(); l++) {
		links[l]->setLogger(_logger);
	}
	logger = _logger;
}

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
	/*
	for (int i = 0; i < numNodes; i++) {
		nodes[i]->scheduleTx();
	}
	*/
}
void Testbed::run() {
	/*
	for (int i = 0; i < numNodes; i++) {
		nodes[i]->tx();
	}
	*/
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
void Testbed::initReportQueue() {
	std::ofstream* file = logger->getQFile();
	*file << "time";
	for (int i = 0; i < numNodes; i++) {
		nodes[i]->initReportQueue();
	}
	*file << std::endl;;
}

void Testbed::reportQueue(int t) {
	std::ofstream* file = logger->getQFile();
	*file << t;
	for (int i = 0; i < numNodes; i++) {
		nodes[i]->reportQueue();
	}
	*file << std::endl;
}

