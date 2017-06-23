#include "link.h"

Link::Link(int deltar) :deltar(deltar) {
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
}

int Link::getQueueDiff(PacketID* pid) {
	return sender->getQueueLen(pid) - receiver->getQueueLen(pid);
}


void Link::prepareTx(int numRes, PacketID* pid) {
	if ((numRes != numResource) || (pid != packetID)) {
		// Start reconfiguration
		reconfigDelay = deltar;
		numResource = numRes;
		packetID = pid;
	}
}

int Link::tx(Packet* pkPtr) {
	// return status of tx: 0 - tx success, 1 - tx fail due to reconfiguration delay
	assert(reconfigDelay >= 0);
	if (reconfigDelay > 0) {
std::cout << "Link (" << sender->getNodeID() << ", " << receiver->getNodeID() << ") deltar = " << reconfigDelay << std::endl;
		return 1;
	} else {
std::cout << "Link (" << sender->getNodeID() << ", " << receiver->getNodeID() << ") transmit a packet" << std::endl;
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
