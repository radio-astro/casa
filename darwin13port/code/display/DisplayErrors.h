//# DisplayErrors.h: internal viewer errors
//# Copyright (C) 2012
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

#ifndef DISPLAY_ERRORS_H_
#define DISPLAY_ERRORS_H_

namespace casa {
	namespace viewer {
		struct internal_error : public std::exception {
			internal_error( const char *err="internal viewer inconsistency" ) : str(err) { }
			internal_error( const std::string &err ) : str(err) { }
			~internal_error( ) throw( ) { }
			const char* what() const throw() {
				return str.c_str( );
			}
			const std::string str;
		};

		struct runtime_error : public std::exception {
			runtime_error( const char *err="runtime viewer error" ) : str(err) { }
			~runtime_error( ) throw( ) { }
			const char* what() const throw() {
				return str.c_str( );
			}
			const std::string str;
		};

	}
}

#endif
