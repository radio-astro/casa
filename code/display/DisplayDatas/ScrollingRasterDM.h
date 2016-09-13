//# ScrollingRasterDM.h: Base class for scrolling DisplayData objects
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003,2004
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
//

#ifndef TRIALDISPLAY_SCROLLINGRASTERDM_H
#define TRIALDISPLAY_SCROLLINGRASTERDM_H

#include <display/DisplayDatas/PrincipalAxesDM.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Base class for scrolling DisplayData objects
// </summary>

	class ScrollingRasterDM : public PrincipalAxesDM {

	public:

		ScrollingRasterDM(uInt xAxis, uInt yAxis, Int mAxis,
		                  IPosition fixedPos, PrincipalAxesDD *padd);

	protected:

		virtual void setup(IPosition fixedPos);
		virtual IPosition dataShape();

		virtual uInt dataDrawSelf(WorldCanvas *wCanvas,
		                          const Vector<Double> &blc,
		                          const Vector<Double> &trc,
		                          const IPosition &start,
		                          const IPosition &sliceShape,
		                          const IPosition &stride,
		                          const Bool usePixelEdges = False);


		virtual Bool dataGetSlice(Matrix<Float>& data,
		                          Matrix<Bool> &mask,
		                          const IPosition& start,
		                          const IPosition& sliceShape,
		                          const IPosition& stride);

	private:

	};


} //# NAMESPACE CASA - END

#endif
