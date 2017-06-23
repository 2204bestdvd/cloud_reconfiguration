#include "packet.h"

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

