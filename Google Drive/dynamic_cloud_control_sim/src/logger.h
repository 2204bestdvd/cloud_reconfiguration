#if !defined(_LOGGER_H_)
#define _LOGGER_H_

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

class Logger {
public:
	Logger(string logFilename, string queueFilename, string scheduleFilename, string costFilename, 
			bool disp=true, bool logg=true) :displaying(disp), logging{logg} {
		logFile.open(logFilename);
		queueFile.open(queueFilename);
		scheduleFile.open(scheduleFilename);
		costFile.open(costFilename);
		//readConfig(configFilename);
	}
	~Logger() {
        logFile.close();
        queueFile.close();
        scheduleFile.close();
    }
    /*
    void readConfig(string configFilename) {
    	ifstream configFile;
    	char temp[256];
    	string parameter, value;
    	configFile.open(configFilename);

		if (configFile.is_open()) {
			while (configFile.good()) {
				configFile.getline(temp, 256, '=');
				parameter = temp;
				configFile.getline(temp, 256, '\n');
				value = temp;
				if (!strcmp(parameter, "displyOutput")) {
					displaying = strcmp(value, "false");
				} else if (!strcmp(parameter, "logging")) {
					logging = strcmp(value, "false");
				}
			}
			configFile.close();
		} else {
			cout << "Error opening file";
		}
    }
    */
    ofstream* getQFile() {return &queueFile;}
    ofstream* getSFile() {return &scheduleFile;}
    ofstream* getCFile() {return &costFile;}
	template<class T> 
	Logger &operator<<(const T &msg) {
		if (logging) {
			logFile << msg;
		}
		if (displaying) {
	        cout << msg;
	    }
        return *this;
    }

    Logger &operator<<(ostream & (*manip)(ostream &)) {
    	if (logging) {
	    	manip(logFile);
	    } 
	   	if (displaying) {
	    	manip(cout);
	    }
		return *this;
	}

private:
	bool displaying, logging;
	ofstream logFile;
	ofstream queueFile;
	ofstream scheduleFile;
	ofstream costFile;
};

#endif /* !defined(_LOGGER_H_) */

