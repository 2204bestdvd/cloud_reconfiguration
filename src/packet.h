#if !defined(_PACKET_H_)
#define _PACKET_H_

#include <unordered_map>
#include <vector>
#include <string>
#include <cassert>

class PacketID {
public:
	PacketID() :dstID(-1) {
		service = 0;
		stage = 0;
	}
	PacketID(int dst, int serv = 0, int st = 0) {
		dstID = dst;
		service = serv;
		stage = st;
	}

	bool operator ==(const PacketID& target) const {
		if (dstID == target.dstID) {
			if (service == target.service) {
				if (stage == target.stage) {
					return true;
				}
			}
		}
		return false;
	}
	bool operator !=(const PacketID& target) const {
    	return !(*this == target);
	}
	int getDst() { return dstID; }
	int getService() { return service; }
	int getStage() { return stage; }
	int getPxScaling() { return serviceScaling[service][stage]; }
	bool isLastStage() { return stage == numServiceStage[service]; }
	PacketID* findPid(int dst, int service, int stage) {
		// Return null if no corresponding pid found
		PacketID tempID(dst, service, stage);
		PacketID* pid = NULL;
		for (int i = 0; i < packetIDs.size(); i++) {
			if ( *(packetIDs[i]) == tempID ) {
				pid = packetIDs[i];
			}
		}
		return pid;
	}
	PacketID* getNextPid() {
		// Note: returns the same PID if it reaches the max stage of the service
		int dst = getDst();
		int service = getService();
		int stage = getStage();
		if (!isLastStage()) {
			stage++;
		}

		return findPid(dst, service, stage);
	}
	std::string getString() { 
		std::string s ("(");
		s += std::to_string(dstID);
		s += ("_");
		s += std::to_string(service);
		s += ("_");
		s += std::to_string(stage);
		s += (")");
		return s;
	}
	static std::unordered_map<int, int> numServiceStage;
	static std::unordered_map<int, std::vector<int>> serviceScaling;
	static std::vector<PacketID*> packetIDs;

private:
	int dstID;
	int service;
	int stage;
};

class Packet {
public:
	Packet();
	Packet(int t, int src, int dst, PacketID* pid);
	Packet(const Packet &p);
	void setArrTime(int n);
	void setServeTime(int n);
	int process();
	int getArrTime();
	int getServeTime();
	int getSrc();
	int getDst();
	PacketID* getPID();
	bool isLastStage();
private:
	int src;
	int dst;
	int arrTime;
	int serveTime;
	PacketID* identifier;
};

#endif