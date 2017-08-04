#if !defined(_SCHEDULER_H_)
#define _SCHEDULER_H_

#include <vector>
#include <queue>
#include <tuple>
#include <cmath>
#include <algorithm>
#include "node.h"
#include "link.h"
#include "packet.h"

class Scheduler {
public:
	Scheduler();
	void init(std::vector<Node *> n, std::vector<Link *> l, const char* s);
	void assignCost(std::vector<double> nPC, std::vector<std::vector<double>> nAC, 
					std::vector<double> lTC, std::vector<std::vector<double>> lAC);
	void assignCap(std::vector<std::vector<double>> nAC, std::vector<std::vector<double>> lAC);
	void setParam(double setV);
	void setLogger(Logger* l) {logger = l;}
	void schedule();

	void DCNC();
	void ADCNC();
	void EADCNC();
	double hysteresis(double x, double delta = 0.1, double gamma = 0.1);
	std::tuple<int, int, PacketID*> getScheduleTx(int l);
	std::tuple<int, int, PacketID*> getSchedulePx(int n);

	void initReportSchedule();
	void initReportCost();
	void reportSchedule(int time);
	void reportCost(int time);

private:
	double V;
	char* schedulingPolicy;
	std::vector<Node*> nodes;
	std::vector<Link*> links;
	std::vector<PacketID*> packetIDs;
	Logger* logger;

	// Parameters
	std::vector<double> nodePxCosts;  // process unit cost (for each node)
	std::vector<double> linkTxCosts;  // tx unit cost (for each link) 
	std::vector<std::vector<double>> nodeAllocCosts;  // cost for allocating k process units (for each node)
	std::vector<std::vector<double>> linkAllocCosts;  // cost for allocating k tx units (for each link)
	std::vector<std::vector<double>> nodeAllocCaps;  // capacity from allocating k process units (for each node)
	std::vector<std::vector<double>> linkAllocCaps;  // capacity from allocating k tx units (for each link)

	// Scheduling decisions
	//std::vector<int> nodeResources;
	//std::vector<int> nodeRates;
	std::vector<int> nodeResources;
	std::vector<double> nodeRates;
	std::vector<PacketID*> nodePxPackets;
	std::vector<int> linkResources;	
	std::vector<double> linkRates;	
	//std::vector<int> linkResources;	
	//std::vector<int> linkRates;	
	std::vector<PacketID*> linkTxPackets;
};


#endif