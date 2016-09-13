//# Lowlevel.h: useful low-level tools used in multiple places by the viewer.
//# Copyright (C) 2010
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
//# $Id$

#ifndef DISPLAY_LOWLEVEL_H
#define DISPLAY_LOWLEVEL_H
#include <casa/System/Casarc.h>
#include <string>

namespace casa { //# NAMESPACE CASA - BEGIN
	namespace viewer {
		// ensure that path is a directory, returns true upon success false upon failure...
		bool create_dir( const char *path, bool recurse=false );
		inline bool create_dir( const std::string &s, bool recurse=false ) {
			return create_dir(s.c_str( ),recurse);
		}
		// get the current time
		std::string time( );
		// get the current data
		std::string date( );
		// get viewer's rc file
		Casarc &getrc( );
		// set the path for the viewer's casarc file...
		bool setrcDir( const std::string & );

		class guiwait {
		public:
			guiwait( );
			~guiwait( );
		private:
			int noop;

			// Prevent heap allocation
			void * operator new   (size_t);
			void * operator new[] (size_t);
		};
	}
}

#endif
