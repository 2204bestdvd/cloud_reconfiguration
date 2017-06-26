#if !defined(_NODE_H_)
#define _NODE_H_

#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <tuple>
#include <unordered_map>
#include <random>
#include "packet.h"
#include "link.h"


class Poisson {
public:
	Poisson(double r) :dist(r) { gen.seed(time(0)); }
	int rand() { return dist(gen); }
	double getRate() { return dist.mean(); }
private:
	//double rate;
	std::default_random_engine gen;
	std::poisson_distribution<int> dist;
};

class Link;

class Node {
public:
	static int numNodes;
	Node(int deltar = 0);
	void initQueues(std::vector<PacketID*> packetIDs);	
	//void addNeighbor(Node* neighbor);
	void addOutputLink(Link* linkPtr);
	void addFlow(PacketID* pid, double rate);
	void timeIncrement();
	//void scheduleTx();
	//int getWeight(int q, Link* link);
	//int getMaxWeightQueue(Link* link);
	void tx(Link* link, PacketID* pid, int rate);
	//void px(PacketID* pid, int rate);
	int receivePacket(Packet* pkPtr);
	void extArrivals(int t);
	void reportQueue(std::ofstream& file);
	static int getNumNodes() {
		return numNodes;
	}
	int getNodeID();
	int getQueueLen(PacketID* pid);
	void printQueues() {
		std::cout << "Node " << nodeID << ": # queues = " << queues.size() << std::endl;

		std::cout << "Node " << nodeID << ": queues = ";
		for (auto it = queues.begin(); it != queues.end(); it++) {
			auto temp = std::get<0>(*it);
			if (temp != NULL) std::cout << temp << ", "; 
		}
		std::cout << std::endl;
	}
private:
	int nodeID;
	std::unordered_map<PacketID*, std::queue<Packet*>> queues;
	//std::vector<std::queue<Packet*>> queues;
	std::vector<Link*> outputLinks;
	std::vector<int> scheduledQueues;
	std::vector<std::tuple<PacketID*, Poisson*>> arrivalGenerators;
	int deltar = 0;
	int reconfigDelay = 0;

};

#endif /* !defined(_NODE_H_) */

