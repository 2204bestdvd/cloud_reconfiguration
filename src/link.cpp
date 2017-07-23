#include "link.h"

Link::Link(int deltar, int costr) :deltar(deltar), costr(costr) {
	linkID = numLinks;
	numLinks++;
	sender = NULL;
	receiver = NULL;
	packetID = NULL;
}

void Link::setSender(Node* s) {
	sender = s;
}
void Link::setReceiver(Node* r) {
	receiver = r;
}

void Link::timeIncrement() {
	if (reconfigDelay > 0) {
		reconfigDelay -= 1;
	}
	if (reconfigCost > 0) {
		reconfigCost = 0;
	}
}

string Link::getString() {
	string ret = "(";
	ret = ret + to_string(sender->getNodeID()) + "_" + to_string(receiver->getNodeID()) + ")";
	return ret;
}


int Link::getQueueDiff(PacketID* pid) {
	return sender->getQueueLen(pid) - receiver->getQueueLen(pid);
}


void Link::prepareTx(int numRes, PacketID* pid) {
	if ((numRes != numResource) || (pid != packetID)) {
	//if ((numRes != numResource)) {
		// Start reconfiguration
		reconfigDelay = deltar;
		numResource = numRes;
		packetID = pid;

		reconfigCost = costr;
	}
}

int Link::tx(Packet* pkPtr) {
	// return status of tx: 0 - tx success, 1 - tx fail due to reconfiguration delay
	assert(reconfigDelay >= 0);
	if (reconfigDelay > 0) {
		*logger << "Link (" << sender->getNodeID() << ", " << receiver->getNodeID() << ") deltar = " 
				<< reconfigDelay << std::endl;
		return 1;
	} else {
		*logger << "Link (" << sender->getNodeID() << ", " << receiver->getNodeID() << ") transmit a packet" 
				<< std::endl;
		receiver->receivePacket(pkPtr);
		return 0;
	}

}

Node* Link::getSender() {
	return sender;
}

Node* Link::getReceiver() {
	return receiver;
}
