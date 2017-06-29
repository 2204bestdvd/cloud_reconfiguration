#if !defined(_LOGGER_H_)
#define _LOGGER_H_

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

class Logger {
public:
	Logger(string logFilename, string queueFilename, bool disp=true, bool logg=true) 
	:displaying(disp), logging{logg} {
		logFile.open(logFilename);
		queueFile.open(queueFilename);
	}
	//Log() {} 
	~Logger() {
        logFile.close();
        queueFile.close();
    }
    ofstream* getQFile() {return &queueFile;}
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
};

#endif /* !defined(_LOGGER_H_) */

