#if !defined(_LINK_H_)
#define _LINK_H_

#include <iostream>
#include <vector>
#include <queue>
#include <tuple>
#include <random>
#include <assert.h>
#include "packet.h"
#include "node.h"
#include "logger.h"

class Node;

class Link {
public:
	static int numLinks;
	Link(int deltarResource=0, int deltarCommodity=0, int costr=0);
	void setSender(Node* s);
	void setReceiver(Node* r);
	void setLogger(Logger* l) {logger = l;}
	void setParameter(double, vector<double>, vector<double>);
	void timeIncrement();

	string getString();
	int getQueueDiff(PacketID* pid);
	int getReconfig() { return reconfigDelay; }
	int getReconfigCost() { return reconfigCost; }
	int getCostr() { return costr; }
	void prepareTx(int numRes, PacketID* pid);
	int tx(Packet* pkPtr);
	Node* getSender();
	Node* getReceiver();

private:
	int linkID;
	Node* sender;
	Node* receiver;
	Logger* logger;

	// records configuration
	int numResource;
	int resourceCost;
	int capacity;
	PacketID* packetID;

	// parameters and reconfiguration variables
	double txCost;
	vector<double> allocCosts;
	vector<double> allocCaps;

	int deltar = 0;  // delay duration of reconfiguration
	int deltarResource = 0;  // delay for resource reconfiguration
	int deltarCommodity = 0;  // delay for commodity reconfiguration
	int reconfigDelay = 0;
	int costr = 0;  // cost of reconfiguration
	int reconfigCost = 0;
};


#endif /* !defined(_LINK_H_) */

