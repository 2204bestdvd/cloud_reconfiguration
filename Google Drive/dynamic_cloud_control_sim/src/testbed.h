#if !defined(_TESTBED_H_)
#define _TESTBED_H_

#include <vector>
#include <tuple>
#include "node.h"
#include "link.h"
#include "scheduler.h"

using namespace std;

class Logger {
public:
	Logger() {}
	void log() {
		logNodes();
		logLinks();
	}
	void logNodes() {}
	void logLinks() {}

private:
	std::vector<Node*> nodes;
	std::vector<Link*> links;
	std::vector<PacketID*> packetIDs;	
};

class Testbed {
public:
	Testbed(int num, int deltar = 0, const char* s="DCNC");
	void createNodes();
	void buildTopo(vector<tuple<int, int>> links, int linkType);
	void addFlow(vector<tuple<int, int, double>> flows);
	void init();
	void setParam();
	void assignCost();
	void assignCap();
	void timeIncrement();
	void scheduleTx();
	void tx();
	void extArrivals(int t);
	void reportQueue();


private:
	int numNodes;
	int deltar;
	std::vector<Node*> nodes;
	std::vector<Link*> links;
	std::vector<PacketID*> packetIDs;
	Scheduler scheduler;

	char* schedulingPolicy;
	std::vector<double> nodePxCosts;  // process unit cost (for each node)
	std::vector<double> linkTxCosts;  // tx unit cost (for each link) 
	std::vector<std::vector<double>> nodeAllocCosts;  // cost for allocating k process units (for each node)
	std::vector<std::vector<double>> linkAllocCosts;  // cost for allocating k tx units (for each link)
	std::vector<std::vector<double>> nodeAllocCaps;  // capacity from allocating k process units (for each node)
	std::vector<std::vector<double>> linkAllocCaps;  // capacity from allocating k tx units (for each link)
};


#endif /* !defined(_TESTBED_H_) */
