#include <display/DisplayDatas/IterationUnit.h>
#include <display/DisplayDatas/IterationClient.qo.h>

namespace casa {
    namespace viewer {
	void IterationUnit::locateFrame( unsigned int movie_axis, IPosition &start ) {
	    if ( ic ) ic->locateFrame( movie_axis, start );
	}
    }
}

