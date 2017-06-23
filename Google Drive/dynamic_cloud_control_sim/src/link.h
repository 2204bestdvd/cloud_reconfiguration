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

class Node;

class Link {
public:
	static int numLinks;
	Link(int deltar = 0);
	void setSender(Node* s);
	void setReceiver(Node* r);
	void timeIncrement();

	int getQueueDiff(PacketID* pid);
	void prepareTx(int numRes, PacketID* pid);
	int tx(Packet* pkPtr);
	Node* getSender();
	Node* getReceiver();

private:
	int linkID;
	Node* sender;
	Node* receiver;
	int numResource;
	int resourceCost;
	int capacity;
	PacketID* packetID;
	int deltar = 0;
	int reconfigDelay = 0;
};


#endif /* !defined(_LINK_H_) */

