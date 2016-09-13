//# WCDataScaleHandler.h: class offering simple data scaling for WorldCanvases
//# Copyright (C) 1993,1994,1995,1996,1998,1999,2000,2001
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

#ifndef TRIALDISPLAY_WCDATASCALEHANDLER_H
#define TRIALDISPLAY_WCDATASCALEHANDLER_H

#include <casa/aips.h>
#include <casa/Containers/Record.h>
#include <display/Utilities/DisplayOptions.h>

namespace casacore{

	template <class T> class Array;
}

namespace casa { //# NAMESPACE CASA - BEGIN


// <summary>
// Class implementing simple data scaling for the WorldCanvas.
// </summary>
//
// <prerequisite>
// <li> Understanding of pseudocolor display of single-channel graphic
// primitives.
// </prerequisite>
//
// <etymology>
// WCDataScaleHandler : WorldCanvas casacore::Data Scale Handler
//
// This class is responsible for scaling data into a range suitable
// for display.
// </etymology>
//
// <synopsis>
// WCDataScaleHandler defines a set of functions whose purpose is to
// transform real data values, such as those from raster images or
// contour lines, into an image whose value ranges from 0 to N-1 for
// some N, normally taken to be the color resolution available for display.
//
// The Display library defines the following process of image display:
// <ol>
// <li> fitting of images to display
// (<linkto class="WCResampleHandler">WCResampleHandler</linkto>)
// <li> transformation of values from real values to discrete values
// in the range of [0,N-1], where N is the output color resolution.
// (<linkto class="WCDataScaleHandler">WCDataScaleHandler</linkto>)
// <li> translating [0,N-1] into color indicies
// (<linkto class="PixelCanvas">PixelCanvas</linkto>'s mapToColor() function)
// </ol>
//
// The function suite includes all the normal scalar types.  Default
// implementations exist for
// <ul>
// <li>casacore::Bool version which maps false to 0 and true to N-1
// <li>casacore::Complex version which extracts magnitude and forwards the
//   call to the casacore::Float version.
// <li>casacore::DComplex version which extracts magnitude and forwards the
//   call to the casacore::Double version.
// </ul>
//
// Each scalar implementation must scale the data according to the
// domain minimum and maximum values and the N-1 value called the
// rangeMax.
//
// This class would idealy be implemented by a templated member function, but
// C++ does not yet support templated member functions in non-templated
// classes.  Thus the manual expansion-by-type.
// </synopsis>
//
// <motivation>
// Wanted a way for users to customize the scaling process from
// real values to discrete values for color display.
// </motivation>
//
// <example>
// see <linkto class="WCLinearScaleHandler">WCLinearScaleHandler</linkto>
// </example>
//
// <todo>
// <li> add ComplexToRealMethod field to base class to allow
// for extracting values other than magnitude for casacore::Complex and DComplex
// variables.
// <li> consider changing Matrices to Arrays to support a casacore::Vector of
// transformations.
// </todo>
//

	class WCDataScaleHandler : public DisplayOptions {

	public:

		// Default Constructor Required
		WCDataScaleHandler();

		// Destructor (does nothing)
		virtual ~WCDataScaleHandler();

		// Set min/max at construction time
		WCDataScaleHandler(casacore::Double min, casacore::Double max);

		// op() returns true if the casacore::Array in was converted to the
		// casacore::Array out successfully.  Default implentation for Bool
		// maps false to 0 and true to rangeMax
		//
		// Types casacore::uChar to casacore::Double not implemented (They return false).
		// User should override the types needed (or perhaps all
		// depending on your compiler.
		//
		// Default casacore::Complex and casacore::DComplex extract amplitude and call
		// the casacore::Float or casacore::Double version.
		// <group>
		virtual casacore::Bool operator()(casacore::Array<casacore::uInt> & out, const casacore::Array<casacore::Bool> & in);
		virtual casacore::Bool operator()(casacore::Array<casacore::uInt> & out, const casacore::Array<casacore::uChar> & in);
		virtual casacore::Bool operator()(casacore::Array<casacore::uInt> & out, const casacore::Array<casacore::Char> & in);
		virtual casacore::Bool operator()(casacore::Array<casacore::uInt> & out, const casacore::Array<casacore::uShort> & in);
		virtual casacore::Bool operator()(casacore::Array<casacore::uInt> & out, const casacore::Array<casacore::Short> & in);
		virtual casacore::Bool operator()(casacore::Array<casacore::uInt> & out, const casacore::Array<casacore::uInt> & in);
		virtual casacore::Bool operator()(casacore::Array<casacore::uInt> & out, const casacore::Array<casacore::Int> & in);
		virtual casacore::Bool operator()(casacore::Array<casacore::uInt> & out, const casacore::Array<casacore::uLong> & in);
		virtual casacore::Bool operator()(casacore::Array<casacore::uInt> & out, const casacore::Array<casacore::Long> & in);
		virtual casacore::Bool operator()(casacore::Array<casacore::uInt> & out, const casacore::Array<casacore::Float> & in);
		virtual casacore::Bool operator()(casacore::Array<casacore::uInt> & out, const casacore::Array<casacore::Double> & in);
		virtual casacore::Bool operator()(casacore::Array<casacore::uInt> & out, const casacore::Array<casacore::Complex> & in);
		virtual casacore::Bool operator()(casacore::Array<casacore::uInt> & out, const casacore::Array<casacore::DComplex> & in);
		// </group>

		// set/get the min/max to use for the input Array
		// <group>
		casacore::Double domainMin() const {
			return domainMin_;
		}
		casacore::Double domainMax() const {
			return domainMax_;
		}
		void setDomainMinMax(casacore::Double min, casacore::Double max);
		// </group>

		// set/get the maximum value to use for the output array
		// Note: the minimum value is always zero
		// <group>
		casacore::uInt rangeMax() const {
			return rangeMax_;
		}
		void setRangeMax(casacore::uInt max) {
			rangeMax_ = max;
		}
		// </group>

		// install the default options for this DisplayData
		virtual void setDefaultOptions();

		// apply options stored in val to the DisplayData; return value
		// true means a refresh is needed...
		virtual casacore::Bool setOptions(casacore::Record &rec, casacore::Record &recOut);

		// retrieve the current and default options and parameter types.
		virtual casacore::Record getOptions( bool scrub=false ) const;

	private:

		// domain limits to use
		casacore::Double domainMin_;
		casacore::Double domainMax_;

		// range limits
		// rangeMin is always zero
		casacore::uInt rangeMax_;

	};


} //# NAMESPACE CASA - END

#endif
