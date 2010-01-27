//# State.cc: manage global display state
//# Copyright (C) 2009
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#

#include <display/Display/State.h>

namespace casa {
    namespace display {

	bool state::initialized = false;
	state *state::singleton = 0;
	unsigned int stateCleanup::creation_count = 0;

	state &state::startup( ) {
	    if ( initialized == false ) {
		initialized = true;
		singleton = new state( );
	    }
	    return *singleton;
	}

	void state::shutdown( ) {
	    if ( initialized == true ) {
		initialized = false;
		delete singleton;
		singleton = 0;
	    }
	}

	bool state::fileOutputMode( ) {
	    return file_output_mode_count_ > 0 ? true : false;
	}

	void state::beginFileOutputMode( ) {
	    ++file_output_mode_count_;
	}

	void state::endFileOutputMode( ) {
	    if ( file_output_mode_count_ > 0 ) --file_output_mode_count_;
	}
    }
}
