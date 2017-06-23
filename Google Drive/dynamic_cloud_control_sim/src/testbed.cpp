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
void Testbed::addFlow(vector<tuple<int, int, double>> flows) {
	int src, dst;
	PacketID* pid;
	double rate;
	for (auto it = flows.begin(); it != flows.end(); it++) {
		src = get<0>(*it);
		dst = get<1>(*it);
		// Check if a matched packet identifier exists, otherwise create one
		PacketID tempID = PacketID(dst);
		pid = NULL;
		for (int i = 0; i < packetIDs.size(); i++) {
			if ( *(packetIDs[i]) == tempID ) {
				pid = packetIDs[i];
			}
		}
		if (pid == NULL) {
			pid = new PacketID(dst);
			packetIDs.push_back(pid);
		}

		rate = get<2>(*it);
		nodes[src]->addFlow(pid, rate);
	}
}
void Testbed::init(){
	// Initialize queues and for each node after creation of all nodes
	// one queue for each node/destination
	for (int i = 0; i < numNodes; i++) {
		nodes[i]->initQueues(packetIDs);
	}

	// Initialize scheduler
	scheduler.init(nodes, links, packetIDs, schedulingPolicy);
	setParam();
}

void Testbed::setParam() {
	scheduler.setParam();
	assignCost();
	assignCap();
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
void Testbed::scheduleTx() {
	scheduler.scheduleTx();
	/*
	for (int i = 0; i < numNodes; i++) {
		nodes[i]->scheduleTx();
	}
	*/
}
void Testbed::tx() {
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
}
void Testbed::extArrivals(int t) {
	for (int i = 0; i < numNodes; i++) {
		nodes[i]->extArrivals(t);
	}
}
void Testbed::reportQueue() {
	for (int i = 0; i < numNodes; i++) {
		nodes[i]->reportQueue();
	}
}

