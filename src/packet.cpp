#include "packet.h"

std::unordered_map<int, int> PacketID::numServiceStage;
std::unordered_map<int, std::vector<int>> PacketID::serviceScaling;
std::vector<PacketID*> PacketID::packetIDs;

Packet::Packet() {
	arrTime = 0;
	serveTime = -1;
	src = -1;
	dst = -1;
	identifier = nullptr;
}

Packet::Packet(int t, int src, int dst, PacketID* pid) 
:arrTime(t), src(src), dst(dst), identifier(pid) {
	serveTime = -1;
}

Packet::Packet(const Packet &p) {
	arrTime = p.arrTime;
	serveTime = p.serveTime;
	src = p.src;
	dst = p.dst;
	identifier = p.identifier;
}

void Packet::setArrTime(int n) {
	arrTime = n;
}

void Packet::setServeTime(int n) {
	serveTime = n;
}

int Packet::process() {
	assert( !(identifier->isLastStage()) );
	int scaling = identifier->getPxScaling();
	identifier = identifier->getNextPid();

	return scaling;
}

int Packet::getArrTime() {
	return arrTime;
}

int Packet::getServeTime() {
	return serveTime;
}

int Packet::getSrc() {
	return src;
}

int Packet::getDst() {
	return dst;
}

PacketID* Packet::getPID() {
	return identifier;
}

bool Packet::isLastStage() {
	return identifier->isLastStage();
}

