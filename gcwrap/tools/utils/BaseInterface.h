#ifndef CASA_BASEINTERFACE_H
#define CASA_BASEINTERFACE_H
//
//BaseInterface class, uses a casacore::Record to produce a simple readline based Command Shell
//for a task or method.
//

#include <casa/aips.h>
#include <vector>
#include <string>
#include <map>
#include <casa/Containers/Record.h>

using std::vector;
using std::string;
using std::map;

namespace casa {

struct COMMAND {
	char *name;
	char *doc; }; 

class BaseInterface {
   public :
	BaseInterface(casacore::Record &);
       ~BaseInterface();
        casacore::Int inputs(casacore::Record &);
	bool go(const casacore::Record &);
	static vector<string> &getcommands(){return commands;}
        static vector<string> commands;
   private :
	void init();
	void parse_it(casacore::Record &params, const char *line);
	casacore::Bool done;
	casacore::Record xmldescribes;  // Name
	                      // Scope
			      // Help text
			      // parameters - record
        //map< string, int (runfunction *)(casacore::Record &) > stand_alones;
        map< string, int> stand_alones;
};



}
#endif
