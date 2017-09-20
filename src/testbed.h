#if !defined(_TESTBED_H_)
#define _TESTBED_H_

#include <fstream>
#include <vector>
#include <tuple>
#include <string>
#include "node.h"
#include "link.h"
#include "scheduler.h"
#include "logger.h"

using namespace std;


class Testbed {
public:
	Testbed(string topoFile, int deltarResource=0, int deltarCommodity=0, int costr=0, const char* s="DCNC");
	void buildNode(int, double, vector<double>, vector<double>);
	void buildLink(int, int, double, vector<double>, vector<double>, int linkType);
	//void buildTopo(vector<tuple<int, int>> links, int linkType);
	void readTopo(string filename);
	void readService (string filename);
	void readFlow (string filename);
	void addFlow(int src, int dst, int service, double rate);
	void init(double V);
	void setParam(double V);
	void setLogger(Logger* l);
	vector<double> parseAllocation(string s);
	//void assignCost();
	//void assignCap();
	void timeIncrement();
	void schedule();
	void run();
	void extArrivals(int t);
	void initReport();
	void report(int t);

	void printCost();
	int getNumNodes() {return numNodes;}


private:
	int numNodes;
	int deltar;
	int deltarResource;
	int deltarCommodity;
	int costr;
	std::vector<Node*> nodes;
	std::vector<Link*> links;
	std::vector<PacketID*> packetIDs;
	Scheduler scheduler;
	Logger* logger;

	char* schedulingPolicy;

	// Cost, capacity parameters
	std::vector<double> nodePxCosts;  // process unit cost (for each node)
	std::vector<double> linkTxCosts;  // tx unit cost (for each link) 
	std::vector<std::vector<double>> nodeAllocCosts;  // cost for allocating k process units (for each node)
	std::vector<std::vector<double>> linkAllocCosts;  // cost for allocating k tx units (for each link)
	std::vector<std::vector<double>> nodeAllocCaps;  // capacity from allocating k process units (for each node)
	std::vector<std::vector<double>> linkAllocCaps;  // capacity from allocating k tx units (for each link)
};


#endif /* !defined(_TESTBED_H_) */
