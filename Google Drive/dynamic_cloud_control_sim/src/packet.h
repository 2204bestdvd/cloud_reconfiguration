#if !defined(_PACKET_H_)
#define _PACKET_H_

#include <vector>
#include <string>

class PacketID {
public:
	PacketID() :dstID(-1) {
		service = 0;
		stage = 0;
	}
	PacketID(int dst) {
		dstID = dst;
		service = 0;
		stage = 0;
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
	std::string getString() { 
		std::string s ("(");
		s += std::to_string(dstID);
		s += (")");
		return s;
	}

private:
	int dstID;
	int service;
	int stage;
	static std::vector<int> numServiceStage;
};

class Packet {
public:
	Packet();
	Packet(int t, int src, int dst, PacketID* pid);
	Packet(const Packet &p);
	void setArrTime(int n);
	void setServeTime(int n);
	int getArrTime();
	int getServeTime();
	int getSrc();
	int getDst();
	PacketID* getPID();
private:
	int src;
	int dst;
	int arrTime;
	int serveTime;
	PacketID* identifier;
};

#endif