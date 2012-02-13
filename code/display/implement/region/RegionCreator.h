
#ifndef REGION_REGIONCREATOR_H_
#define REGION_REGIONCREATOR_H_
#include <display/region/Region.h>

namespace casa {
    namespace viewer {
	class RegionCreator {
	    public:
		// called when the user indicates that a region should be deleted...
		virtual void revokeRegion( Region * ) = 0;
	};
    }
}

#endif
