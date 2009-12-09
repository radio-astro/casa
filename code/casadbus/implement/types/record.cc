#include <casadbus/types/record.h>

namespace casa {
    namespace dbus {

	record::record() : rec_map() { }
	int record::compare(const record*) const { return -1; }

	std::pair<rec_map::iterator,bool> record::insert(const std::string &s,const variant &v) {
	    return rec_map::insert(rec_map::value_type(s,v));
	}
    }	// dbus namespace
}	// casa namespace
