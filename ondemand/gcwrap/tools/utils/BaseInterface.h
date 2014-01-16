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
	BaseInterface(Record &);
       ~BaseInterface();
        Int inputs(Record &);
	bool go(const Record &);
	static vector<string> &getcommands(){return commands;}
        static vector<string> commands;
   private :
	void init();
	void parse_it(Record &params, const char *line);
	Bool done;
	Record xmldescribes;  // Name
	                      // Scope
			      // Help text
			      // parameters - record
        //map< string, int (runfunction *)(Record &) > stand_alones;
        map< string, int> stand_alones;
};



}
#endif
