//# WCResampleHandler.h: base class for resampling data pixels to the screen
//# Copyright (C) 1993,1994,1995,1996,1998,1999,2000
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

#ifndef TRIALDISPLAY_WCRESAMPLEHANDLER_H
#define TRIALDISPLAY_WCRESAMPLEHANDLER_H

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>

namespace casacore{

	template <class T> class Matrix;
}

namespace casa { //# NAMESPACE CASA - BEGIN


// <summary>
// Base class for resampling data pixels to World/PixelCanvas pixels.
// </summary>
//
// <prerequisite>
// <li> Display library image drawing process
// </prerequisite>
//
// <etymology>
// The name of WCResampleHandler comes from
// WorldCanvas + Resample + Handler
// </etymology>
//
// <synopsis>
// WCResampleHandler is the tool used to extract a rectangular
// subregion of an image for purposes of display.  This class is
// abstract and defines the interface the
// <linkto class="WorldCanvas">WorldCanvas</linkto> sees.
//
// When the resample handler fires, it extracts a rectangular subregion
// of its input matrix and stores it in its output matrix.  If
// the default subregion is the whole image and the output image is
// the same size as the input image, then the output image is
// a copy of the input image.
//
// If not, the subregion of the image is resampled to fit the output
// matrix size.
// </synopsis>
//
// <motivation>
// Objectify the concept of resampling to allow programmers
// to write their own, more complicated versions and register
// them with the WorldCanvas.
// </motivation>

	class WCResampleHandler {

	public:

		// (Required) default constructor.
		WCResampleHandler();

		// Copy Constructor (copy semantics)
		WCResampleHandler (const WCResampleHandler& other);

		// Assignment operator (copy semantics)
		WCResampleHandler& operator=(const WCResampleHandler& other);

		// Destructor.
		virtual ~WCResampleHandler();


		// The output array is presized by the caller to the correct size.  It will
		// be filled using information in the input array combined with other
		// resample-specific information.  Here again the interface is type expanded
		// rather than templated because C++ doesn't yet handle templated member
		// functions in a non-templated class.
		// <group>
		virtual void operator()(casacore::Matrix<casacore::Bool> &out, const casacore::Matrix<casacore::Bool> &in) = 0;
		virtual void operator()(casacore::Matrix<casacore::uChar> &out, const casacore::Matrix<casacore::uChar> &in) = 0;
		virtual void operator()(casacore::Matrix<casacore::Char> &out, const casacore::Matrix<casacore::Char> &in) = 0;
		virtual void operator()(casacore::Matrix<casacore::uShort> &out, const casacore::Matrix<casacore::uShort> &in) = 0;
		virtual void operator()(casacore::Matrix<casacore::Short> &out, const casacore::Matrix<casacore::Short> &in) = 0;
		virtual void operator()(casacore::Matrix<casacore::uInt> &out, const casacore::Matrix<casacore::uInt> &in) = 0;
		virtual void operator()(casacore::Matrix<casacore::Int> &out, const casacore::Matrix<casacore::Int> &in) = 0;
		virtual void operator()(casacore::Matrix<casacore::uLong> &out, const casacore::Matrix<casacore::uLong> &in) = 0;
		virtual void operator()(casacore::Matrix<casacore::Long> &out, const casacore::Matrix<casacore::Long> &in) = 0;
		virtual void operator()(casacore::Matrix<casacore::Float> &out, const casacore::Matrix<casacore::Float> &in) = 0;
		virtual void operator()(casacore::Matrix<casacore::Double> &out, const casacore::Matrix<casacore::Double> &in) = 0;
		virtual void operator()(casacore::Matrix<casacore::Complex> &out, const casacore::Matrix<casacore::Complex> &in) = 0;
		virtual void operator()(casacore::Matrix<casacore::DComplex> &out, const casacore::Matrix<casacore::DComplex> &in) = 0;
		// </group>

		// These functions resample the input matrix to the output.
		// inblc is the location 'within' the input matrix for the
		// bottom-left pixel of the output (sim. for intrc); blank is
		// the output value where none of the input data is useful.
		// The output matrix must be presized to the required size.
		// <group>
		virtual void operator()(casacore::Matrix<casacore::Float> &out, const casacore::Matrix<casacore::Float> &in,
		                        const casacore::Vector<casacore::Float> &inblc,
		                        const casacore::Vector<casacore::Float> &intrc,
		                        const casacore::Float blank = 0.0) = 0;
		virtual void operator()(casacore::Matrix<casacore::Float> &out, casacore::Matrix<casacore::Bool>& outMask,
		                        const casacore::Matrix<casacore::Float> &in, const casacore::Matrix<casacore::Bool> &inMask,
		                        const casacore::Vector<casacore::Float> &inblc,
		                        const casacore::Vector<casacore::Float> &intrc,
		                        const casacore::Float blank = 0.0) = 0;
		virtual void operator()(casacore::Matrix<casacore::Bool> &out, const casacore::Matrix<casacore::Bool> &in,
		                        const casacore::Vector<casacore::Float> &inblc,
		                        const casacore::Vector<casacore::Float> &intrc,
		                        const casacore::Bool blank = false) = 0;
		// </group>

		// These functions manipulate which subregion in "in" gets expanded
		// to "out" Coordinates are the fraction of the image to use, with
		// <0.0, 0.0> representing the bottom-left corner of the first pixel
		// and <1.0, 1.0> representing the top-right corner of the last
		// pixel.  These parameters are interpreted according to the derived
		// class.
		// <group>
		void setSubregion(const casacore::Vector<casacore::Double> &blc,
		                  const casacore::Vector<casacore::Double> &trc) {
			itsBlc = blc;
			itsTrc = trc;
		}
		void getSubregion(casacore::Vector<casacore::Double> &blc,
		                  casacore::Vector<casacore::Double> &trc) const {
			blc = itsBlc;
			trc = itsTrc;
		}
		// </group>


	protected:

		// Get the bottom left corner (range <0-1,0-1>) of the subregion
		const casacore::Vector<casacore::Double> &blc() const {
			return itsBlc;
		}

		// Get the top right corner (range <0-1,0-1>) of the subregion
		const casacore::Vector<casacore::Double> &trc() const {
			return itsTrc;
		}

	private:

		// Current subregion
		casacore::Vector<casacore::Double> itsBlc;
		casacore::Vector<casacore::Double> itsTrc;

	};


} //# NAMESPACE CASA - END

#endif
