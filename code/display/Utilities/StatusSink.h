//# StatusSink.h: abstract base class (and null sink) for posting status messages.
//# Copyright (C) 2013
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
//# $Id: QtDisplayPanelGui.qo.h,v 1.7 2006/10/10 21:42:05 dking Exp $

#ifndef DISPLAY_STATUSSINK_H
#define DISPLAY_STATUSSINK_H
#include <string>

namespace casa {
	namespace viewer {
		class StatusSink {
		public:
			StatusSink( ) { }
			virtual ~StatusSink( ) { }
			virtual void status( const std::string &s, const std::string &type="info" ) = 0;
		};
	}
}

#endif
