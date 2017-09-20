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
#include "logger.h"


class Poisson {
public:
	Poisson(double r) :dist(r) { gen.seed(std::rand()); }
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
	Node(int deltarResource=0, int deltarCommodity=0, int costr=0);
	void initQueues();	
	//void addNeighbor(Node* neighbor);
	void addOutputLink(Link* linkPtr);
	void addFlow(PacketID* pid, double rate);
	void setLogger(Logger* l) {logger = l;}
	void setParameter(double, vector<double>, vector<double>);
	void timeIncrement();
	//void scheduleTx();
	//int getWeight(int q, Link* link);
	//int getMaxWeightQueue(Link* link);
	void preparePx(int numRes, PacketID* pid);
	void tx(Link* link, PacketID* pid, int rate);
	void px(PacketID* pid, int rate);
	int receivePacket(Packet* pkPtr);
	void extArrivals(int t);
	void initReportQueue();
	void reportQueue();
	static int getNumNodes() {
		return numNodes;
	}
	int getNodeID();
	int getQueueLen(PacketID* pid);
	int getQueueDiff(PacketID* pid);
	int getReconfig() { return reconfigDelay; }
	int getReconfigCost() { return reconfigCost; }
	int getCostr() { return costr; }
	void printQueues(Logger& logger) {
		logger << "Node " << nodeID << ": # queues = " << queues.size() << std::endl;

		logger << "Node " << nodeID << ": queues = ";
		for (auto it = queues.begin(); it != queues.end(); it++) {
			auto temp = std::get<0>(*it);
			if (temp != NULL) logger << temp << ", "; 
		}
		logger << std::endl;
	}
private:
	int nodeID;
	std::unordered_map<PacketID*, std::queue<Packet*>> queues;
	std::vector<Link*> outputLinks;
	std::vector<int> scheduledQueues;
	std::vector<std::tuple<PacketID*, Poisson*>> arrivalGenerators;
	Logger* logger;

	// records configuration
	int numResource;
	int resourceCost;
	int capacity;
	PacketID* packetID;	

	// parameters and reconfiguration variable
	double pxCost;
	vector<double> allocCosts;
	vector<double> allocCaps;

	int deltar = 0;  // delay duration of reconfiguration
	int deltarResource = 0;  // delay for resource reconfiguration
	int deltarCommodity = 0;  // delay for commodity reconfiguration
	int reconfigDelay = 0;
	int costr = 0;  // cost of reconfiguration
	int reconfigCost = 0;
	int pxScaling = 1;
};

#endif /* !defined(_NODE_H_) */

