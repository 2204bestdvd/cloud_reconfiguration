#include "node.h"
#include "link.h"


Node::Node(int deltar) :deltar(deltar) {
	nodeID = numNodes;
	numNodes++;
}
void Node::initQueues() {
	/*
	for (int i = 0; i < numNodes; i++) {
		std::queue<Packet *> q;
		queues.push_back(q);
	}
	*/
	for (auto it = PacketID::packetIDs.begin(); it != PacketID::packetIDs.end(); it++) {
		std::queue<Packet *> q;
		queues.insert(std::make_tuple(*it, q));		
	}
}
//void Node::addNeighbor(Node* neighbor) {
//	neighbors.push_back(neighbor);
//}
void Node::addOutputLink(Link* linkPtr) {
	outputLinks.push_back(linkPtr);
}
void Node::addFlow(PacketID* pid, double rate) {
	Poisson* arr;
	arr = new Poisson(rate);
	arrivalGenerators.push_back(std::make_tuple(pid, arr));
}

void Node::timeIncrement() {
	if (reconfigDelay > 0) {
		reconfigDelay -= 1;
	}
}

void Node::preparePx(int numRes, PacketID* pid) {
	if ((numRes != numResource) || (pid != packetID)) {
		// Start reconfiguration
		reconfigDelay = deltar;
		numResource = numRes;
		packetID = pid;
	}
}


void Node::tx(Link* link, PacketID* pid, int rate) {
	Packet* pkPtr;

	for (int i = 0; i < rate; i++) {
		if ((pid != NULL) && !queues[pid].empty()) {
			pkPtr = queues[pid].front();
			int e = link->tx(pkPtr);
			if (e == 0) {
				// tx success, remove packet from queue
				queues[pid].pop();
			}
		}
	}
}
void Node::px(PacketID* pid, int rate) {
	Packet *pkPtr, *tempPtr;
	PacketID* nextPid;

	for (int i = 0; i < rate; i++) {
		if ((pid != NULL) && !queues[pid].empty()) {
			pkPtr = queues[pid].front();

			if (reconfigDelay == 0) {
				*logger << "Node " << nodeID << " process a packet" << std::endl;

				// Process the packet and transfer to the next queue
				if (!pid->isLastStage()) {
					pkPtr->process();
					queues[pid].pop();
					nextPid = pid->getNextPid();
					if ((nextPid->isLastStage()) && (nextPid->getDst() == nodeID)) {
						delete pkPtr;
					} else {
						queues[nextPid].push(pkPtr);
					}
				}

				// Only implement integer ratio here
				int expand = pxScaling - 1;
				while (expand > 0) {
					// Duplicate packet for flow expansion
					tempPtr = new Packet(*pkPtr);
					queues[nextPid].push(pkPtr);
					expand--;
				}
			}
		}
	}
}

int Node::receivePacket(Packet* pkPtr) {
	if ((pkPtr->getDst() == nodeID) && (pkPtr->isLastStage())){
		delete pkPtr;
	} else {
		PacketID* pid = pkPtr->getPID();
		queues[pid].push(pkPtr);
	}
	return 0;
}

void Node::extArrivals(int t) {
	//int dst;
	PacketID* pid;
	int numPacket;
	for (auto it = arrivalGenerators.begin(); it != arrivalGenerators.end(); it++) {
		//dst = std::get<0>(*it);
		pid = std::get<0>(*it);
		numPacket = std::get<1>(*it)->rand();
		if (numPacket > 0) {
			*logger << "Time " << t << ": node " << nodeID << " generates " << numPacket 
			          << " packets to node " << pid->getDst() << std::endl; 
		}
		for (int i = 0; i < numPacket; i++) {
			Packet* pkPtr = new Packet(t, nodeID, pid->getDst(), pid);
			queues[pid].push(pkPtr);
		}
	}
}

void Node::initReportQueue() {
	std::ofstream* file = logger->getQFile();
	for (auto it = queues.begin(); it != queues.end(); it++) {
		*file << ", node" << nodeID << "_" << std::get<0>(*it)->getString();
	}
}
void Node::reportQueue() {
	std::ofstream* file = logger->getQFile();
	*logger << "Node " << nodeID << ": ";
	for (auto it = queues.begin(); it != queues.end(); it++) {
		*logger << std::get<1>(*it).size() << ", ";
		*file << ", " << std::get<1>(*it).size();
	}
	*logger << std::endl;
}

int Node::getNodeID() {
	return nodeID;
}

int Node::getQueueLen(PacketID* pid) {
	return queues[pid].size();
}

int Node::getQueueDiff(PacketID* pid) {
	if (pid->isLastStage()) return 0;

	return (queues[pid].size() - pxScaling * queues[pid->getNextPid()].size());
}