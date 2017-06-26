#include "node.h"
#include "link.h"

Node::Node(int deltar) :deltar(deltar) {
	nodeID = numNodes;
	numNodes++;
}
void Node::initQueues(std::vector<PacketID*> packetIDs) {
	/*
	for (int i = 0; i < numNodes; i++) {
		std::queue<Packet *> q;
		queues.push_back(q);
	}
	*/
	for (auto it = packetIDs.begin(); it != packetIDs.end(); it++) {
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

/*
void Node::scheduleTx() {
	int q = 0;
	for (int i = 0; i < outputLinks.size(); i++) {
		q = getMaxWeightQueue(outputLinks[i]);
		scheduledQueues[i] = q;
	}
}

int Node::getWeight(int q, Link* link) {
	int weight;
	weight = queues[q].size() - link->getReceiver()->queues[q].size();
	return weight;
}

int Node::getMaxWeightQueue(Link* link) {
	int maxWeight = -1;
	int maxWeightQueue = -1;
	int tempWeight = 0;
	for (int i = 0; i < numNodes; i++) {
		tempWeight = getWeight(i, link);
		if (tempWeight > maxWeight) {
			maxWeight = tempWeight;
			maxWeightQueue = i;
		}
	}
	// Transmit only when the drift is strictly larger than 0
	if (maxWeight == 0) {
		maxWeightQueue = -1;
	}
	return maxWeightQueue;
}
*/

void Node::tx(Link* link, PacketID* pid, int rate) {
//printQueues();
	//int queueNumber;
	Packet* pkPtr;
	
	/*
	for (int i = 0; i < outputLinks.size(); i++) {
		queueNumber = scheduledQueues[i];
		
		// Transmit if there is a scheduled queue and it is nonempty
		// If tx fails (return != 0), keep packet at queue
		if ((queueNumber != -1) && !queues[queueNumber].empty()) {
			pkPtr = queues[queueNumber].front();
			int e = outputLinks[i]->tx(pkPtr);
			if (e == 0) {
				// tx success, remove packet from queue
				queues[queueNumber].pop();
			}
		}
	}
	*/
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

int Node::receivePacket(Packet* pkPtr) {
	if (pkPtr->getDst() == nodeID){
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
			std::cout << "Time " << t << ": node " << nodeID << " generates " << numPacket 
			          << " packets to node " << pid->getDst() << std::endl; 
		}
		for (int i = 0; i < numPacket; i++) {
			Packet* pkPtr = new Packet(t, nodeID, pid->getDst(), pid);
			queues[pid].push(pkPtr);
		}
	}
}

void Node::reportQueue(std::ofstream& file) {
	std::cout << "Node " << nodeID << ": ";
	for (auto it = queues.begin(); it != queues.end(); it++) {
		std::cout << std::get<1>(*it).size() << ", ";
		file << ", " << std::get<1>(*it).size();
	}
	std::cout << std::endl;
}

int Node::getNodeID() {
	return nodeID;
}

int Node::getQueueLen(PacketID* pid) {
	return queues[pid].size();
}
