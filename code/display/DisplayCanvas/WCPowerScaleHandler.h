//# WCPowerScaleHandler.h: lin, log and exp scaling of data for WorldCanvas
//# Copyright (C) 1998,1999,2000,2001
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

#ifndef TRIALDISPLAY_WCPOWERSCALEHANDLER_H
#define TRIALDISPLAY_WCPOWERSCALEHANDLER_H

#include <casa/aips.h>
#include <casa/Containers/Record.h>
#include <display/DisplayCanvas/WCDataScaleHandler.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Linear, logarithmic and exponential scaling of data for the WorldCanvas
// </summary>
//
// <prerequisite>
// <li> <linkto class="WCDataScaleHandler">WCDataScaleHandler</linkto>
// </prerequisite>
//
// <etymology>
// WCPowerScaleHandler : WorldCanvas Power Scale Handler
// </etymology>
//
// <synopsis>
// The WCPowerScaleHandler is an implementation of the
// <linkto class="WCDataScaleHandler">WCDataScaleHandler</linkto> that
// uses linear, logarithmic or exponential scaling to transform
// (probably non-discrete) input data into discrete output values
// in the range 0 to a settable maximum.  Typically, this maximum would
// be set to the available number of colorcells minus one, for indexed
// raster drawing, or to a large integer (say 1000000) for use in color
// cube raster drawing.  In the latter case, the output is then divided
// by the chosen large number in order to provide values in the range
// 0 to 1.
// </synopsis>
//
// <motivation>
// This class is motivated by the need to provide the most common
// data scaling requirements to the user.
// </motivation>
//
// <example>
// ...
// </example>
//
// <todo>
// <li> stream ops
// </todo>
//

	class WCPowerScaleHandler  : public WCDataScaleHandler {

	public:

		// Default Constructor Required
		WCPowerScaleHandler();

		// Destructor
		virtual ~WCPowerScaleHandler();

		// set/get the number of log cycles into which to compress the data
		// <group>
		void setCycles(Float cyc) {
			itsOptionsPowerCycles = cyc;
		}
		Float cycles() {
			return itsOptionsPowerCycles;
		}
		// </group>

		// install the default options for this DisplayData
		virtual void setDefaultOptions();

		// apply options stored in val to the DisplayData; return value
		// True means a refresh is needed...
		virtual Bool setOptions(Record &rec, Record &recOut);

		// retrieve the current and default options and parameter types.
		virtual Record getOptions( bool scrub=false ) const;

		// apply returns True if the array in was converted to the array out
		// successfully the last parameter sets the output range
		// <group>
		virtual Bool operator()(Array<uInt> & out, const Array<Bool> & in);
		virtual Bool operator()(Array<uInt> & out, const Array<uChar> & in);
		virtual Bool operator()(Array<uInt> & out, const Array<Char> & in);
		virtual Bool operator()(Array<uInt> & out, const Array<uShort> & in);
		virtual Bool operator()(Array<uInt> & out, const Array<Short> & in);
		virtual Bool operator()(Array<uInt> & out, const Array<uInt> & in);
		virtual Bool operator()(Array<uInt> & out, const Array<Int> & in);
		virtual Bool operator()(Array<uInt> & out, const Array<uLong> & in);
		virtual Bool operator()(Array<uInt> & out, const Array<Long> & in);
		virtual Bool operator()(Array<uInt> & out, const Array<Float> & in);
		virtual Bool operator()(Array<uInt> & out, const Array<Double> & in);
		virtual Bool operator()(Array<uInt> & out, const Array<Complex> & in);
		virtual Bool operator()(Array<uInt> & out, const Array<DComplex> & in);
		// </group>

		Bool histoEqualise(Array<Float> &out, const Array<Float> &in);
		static const String POWER_CYCLES;
	private:

		// Scaling power cycles: < 0 => log scaling, > 0 => exp scaling,
		// == 0 => linear scaling
		Float itsOptionsPowerCycles;

		// Is histogram equalisation required?
		Bool itsOptionsHistoEqualisation;

	};


} //# NAMESPACE CASA - END

#endif
