//# WCResampleHandler: base class for resampling data pixels to screen pixels
//# Copyright (C) 1993,1994,1995,1996,1999,2000
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

#include <casa/Arrays/Matrix.h>
#include <display/DisplayCanvas/WCResampleHandler.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	WCResampleHandler::WCResampleHandler()
		: itsBlc(2),
		  itsTrc(2) {
		itsBlc(0) = 0.0;
		itsBlc(1) = 0.0;
		itsTrc(0) = 1.0;
		itsTrc(1) = 1.0;
	}

	WCResampleHandler::WCResampleHandler (const WCResampleHandler& other)
		: itsBlc(other.itsBlc.copy()),
		  itsTrc(other.itsTrc.copy())
	{}



	WCResampleHandler& WCResampleHandler::operator= (const WCResampleHandler& other) {
		if (this != &other) {
			itsBlc.resize(other.itsBlc.nelements());
			itsTrc.resize(other.itsTrc.nelements());
//
			itsBlc = other.itsBlc;
			itsTrc = other.itsTrc;

		}
		return *this;
	}



// Destructor.
	WCResampleHandler::~WCResampleHandler() {
	}


} //# NAMESPACE CASA - END

