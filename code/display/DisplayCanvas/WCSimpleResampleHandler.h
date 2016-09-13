//# WCSimpleResampleHandler.h: Simple (nearest,linear,cubic) resampling for the WorldCanvas
//# Copyright (C) 1996,1997,1998,1999,2000,2001
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

#ifndef TRIALDISPLAY_WCSIMPLERESAMPLEHANDLER_H
#define TRIALDISPLAY_WCSIMPLERESAMPLEHANDLER_H

#include <casa/aips.h>
#include <display/DisplayCanvas/WCResampleHandler.h>
#include <scimath/Mathematics/Interpolate2D.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Class which implements simple resampling for the WorldCanvas.
// </summary>
//
// <prerequisite>
// <li> <linkto class="WCResampleHandler>WCResampleHandler</linkto>
// </prerequisite>
//
// <etymology>
// The name "WCSimpleResampleHandler" is a contraction of the terms
// "WorldCanvas," "Simple," "Resample" and "Handler."
// </etymology>
//
// <synopsis>
// This class implements the interface defined in WCResampleHandler
// to provide pixel interpolation (resampling) of data pixels to screen pixels.
// It offers simple nearest, linear and cubic interpolation.
// </synopsis>
//
// <motivation>
// The provision of raster images which are smooth in appearance.
// </motivation>

	class WCSimpleResampleHandler : public WCResampleHandler {

	public:

		// Default constructor (Provides NEAREST)
		WCSimpleResampleHandler();

		// Constructor
		WCSimpleResampleHandler(casacore::Interpolate2D::Method type=casacore::Interpolate2D::NEAREST);

		// Copy Constructor (copy semantics)
		WCSimpleResampleHandler (const WCSimpleResampleHandler& other);

		// Assignment operator (copy semantics)
		WCSimpleResampleHandler& operator=(const WCSimpleResampleHandler& other);

		// Destructor
		virtual ~WCSimpleResampleHandler();

		// Reset interpolation method
		void setInterpolationType (casacore::Interpolate2D::Method type);

		// The output array is presized by the caller to the correct size.  It will
		// be filled using information in the input array combined with other
		// resample-specific information.  Here again the interface is type expanded
		// rather than templated because C++ doesn't yet handle templated member
		// functions in a non-templated class.
		// <group>
		virtual void operator()(casacore::Matrix<casacore::Bool> & out, const casacore::Matrix<casacore::Bool> & in);
		virtual void operator()(casacore::Matrix<casacore::uChar> & out, const casacore::Matrix<casacore::uChar> & in);
		virtual void operator()(casacore::Matrix<casacore::Char> & out, const casacore::Matrix<casacore::Char> & in);
		virtual void operator()(casacore::Matrix<casacore::uShort> & out, const casacore::Matrix<casacore::uShort> & in);
		virtual void operator()(casacore::Matrix<casacore::Short> & out, const casacore::Matrix<casacore::Short> & in);
		virtual void operator()(casacore::Matrix<casacore::uInt> & out, const casacore::Matrix<casacore::uInt> & in);
		virtual void operator()(casacore::Matrix<casacore::Int> & out, const casacore::Matrix<casacore::Int> & in);
		virtual void operator()(casacore::Matrix<casacore::uLong> & out, const casacore::Matrix<casacore::uLong> & in);
		virtual void operator()(casacore::Matrix<casacore::Long> & out, const casacore::Matrix<casacore::Long> & in);
		virtual void operator()(casacore::Matrix<casacore::Float> & out, const casacore::Matrix<casacore::Float> & in);
		virtual void operator()(casacore::Matrix<casacore::Double> & out, const casacore::Matrix<casacore::Double> & in);
		virtual void operator()(casacore::Matrix<casacore::Complex> & out, const casacore::Matrix<casacore::Complex> & in);
		virtual void operator()(casacore::Matrix<casacore::DComplex> & out, const casacore::Matrix<casacore::DComplex> & in);
		// </group>

		// This function resamples the input matrix to the output.
		// pixblc is the location relative to the input matrix of the
		// bottom-left pixel of the output (sim. for pixtrc); blank is
		// the output value where none of the input data is useful.
		// The output matrix must be presized to the required size.
		// <group>
		virtual void operator()(casacore::Matrix<casacore::Float> &out, const casacore::Matrix<casacore::Float> &in,
		                        const casacore::Vector<casacore::Float> &pixblc,
		                        const casacore::Vector<casacore::Float> &pixtrc,
		                        const casacore::Float blank = 0.0);
		virtual void operator()(casacore::Matrix<casacore::Float> &out, casacore::Matrix<casacore::Bool>& outMask,
		                        const casacore::Matrix<casacore::Float> &in, const casacore::Matrix<casacore::Bool> &inMask,
		                        const casacore::Vector<casacore::Float> &inblc,
		                        const casacore::Vector<casacore::Float> &intrc,
		                        const casacore::Float blank = 0.0);
		virtual void operator()(casacore::Matrix<casacore::Bool> &out, const casacore::Matrix<casacore::Bool> &in,
		                        const casacore::Vector<casacore::Float> &pixblc,
		                        const casacore::Vector<casacore::Float> &pixtrc,
		                        const casacore::Bool blank = false);
		// </group>


	private:
		casacore::Interpolate2D itsInterp;
	};


} //# NAMESPACE CASA - END

#endif

