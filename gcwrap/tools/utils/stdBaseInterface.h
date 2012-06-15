#ifndef CASA_BASEINTERFACE_H
#define CASA_BASEINTERFACE_H
//
//BaseInterface class, uses a Record to produce a simple readline based Command Shell
//for a task or method.
//

#include <casa/aips.h>
#include <vector>
#include <string>
#include <map>
#include <stdcasa/record.h>
#include <iostream>

using std::vector;
using std::string;
using std::map;
using std::ostream;

namespace casa {
	class LogIO;
};

namespace casac {

void dumpRecord(ostream &oss, const record &theRec, string spaces = "    ");

struct COMMAND {
	char *name;
	char *doc; }; 

class stdBaseInterface {
   public :
	stdBaseInterface(record &);
       ~stdBaseInterface();
        int inputs(record &);
	bool go(const record &);
	static vector<string> &getcommands(){return commands;}
        static vector<string> commands;
	static bool verify(record &theRec, record &theContraints, casa::LogIO &itsLog);
	static bool verifyOne(record &theRec, record &theContraints, casa::LogIO &itsLog, bool silent=true);
	static bool checkme(const string &name, variant &user, record &constraint, casa::LogIO &itsLog, bool silent=false);
	static bool checkQuanta(const string &name, variant &user, record &constraint, casa::LogIO &itsLog, bool silent=false);
	static variant *expandEnum(variant &allowed, const variant &value, casa::LogIO &itsLog, bool silent=false);
   private :
	void init();
	void parse_it(record &params, const char *line);
	bool done;
	record xmldescribes;  // Name
	                      // Scope
			      // Help text
			      // parameters - record
        //map< string, int (runfunction *)(record &) > stand_alones;
        map< string, int> stand_alones;
};


}
#endif
