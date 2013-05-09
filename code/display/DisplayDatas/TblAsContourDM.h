//# TblAsContourDM.h: Display Method for contour displays of data from a table
//# Copyright (C) 2000,2001
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
//#
//# $Id$

#ifndef TRIALDISPLAY_TBLASCONTOURDM_H
#define TRIALDISPLAY_TBLASCONTOURDM_H

#include <casa/aips.h>
#include <tables/Tables/Table.h>
#include <display/Display/WorldCanvas.h>
#include <display/DisplayDatas/CachingDisplayMethod.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//# Forward Declarations
// <summary>
// A DisplayMethod to draw Contours from table data
// </summary>
//
// <use visibility=local>   or   <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">
// </reviewed>
//
// <prerequisite>
//   <li> DisplayMethod
//   <li> CachingDisplayMethod
// </prerequisite>
//
// <etymology>
// </etymology>
// <synopsis>
// This class adds to the interface defined by DisplayMethod to provide
// the necessary infrastructure to draw contour displays of data from a
// table when requested to do so by the TblAsContourDD class.
// </synopsis>

	class TblAsContourDM : public CachingDisplayMethod {

	public:

		// constructor
		TblAsContourDM(WorldCanvas *worldCanvas,
		               AttributeBuffer *wchAttributes,
		               AttributeBuffer *ddAttributes,
		               CachingDisplayData *dd);

		// Destructor.
		virtual ~TblAsContourDM();

		// Clean up (ie. delete any existing cached display list).
		virtual void cleanup();

		// Draw into a cached drawing list, called by draw function.
		virtual Bool drawIntoList(Display::RefreshReason reason,
		                          WorldCanvasHolder &wcHolder);

	protected:

		// (Required) default constructor.
		TblAsContourDM();

		// (Required) copy constructor.
		TblAsContourDM(const TblAsContourDM &other);

		// (Required) copy assignment.
		void operator=(const TblAsContourDM &other);

	private:

	};


} //# NAMESPACE CASA - END

#endif
