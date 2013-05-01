//# WCLinearScaleHandler.h: linear data scaling for WorldCanvases
//# Copyright (C) 1993,1994,1995,1996,1999,2000,2001
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

#ifndef TRIALDISPLAY_WCLINEARSCALEHANDLER_H
#define TRIALDISPLAY_WCLINEARSCALEHANDLER_H

#include <casa/aips.h>
#include <display/DisplayCanvas/WCDataScaleHandler.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Linear scaling of data for WorldCanvases.
// </summary>
//
// <prerequisite>
// <li> <linkto class="WCDataScaleHandler">WCDataScaleHandler</linkto>
// </prerequisite>
//
// <etymology>
// WCLinearScaleHandler : WorldCanvas Linear Scale Handler
// </etymology>
//
// <synopsis>
// The WCLinearScaleHandler is an implementation of the
// <linkto class="WCDataScaleHandler">WCDataScaleHandler</linkto> that
// uses linear scaling to get from real values to discrete values in
// the range of 0 to rangeMax.  Typically, rangeMax is equivalent to the
// available color resolution minus 1.
// </synopsis>
//
// <motivation>
// Need to provide some basic implementations of the WCDataScaleHandler
// so users can model other custom scale handlers after it.  Linear scaling
// is a common need, so it was implemented.
// </motivation>
//
// <example>
// see the test program tWCLinearScaleHandler.cc in the Display/test directory.
// </example>
//
// <todo>
// <li> stream ops
// </todo>
//

class WCLinearScaleHandler  : public WCDataScaleHandler
{
public:

  // Default Constructor Required
  WCLinearScaleHandler();

  // Destructor
  virtual ~WCLinearScaleHandler();

  // apply returns True if the array in was converted to the
  // array out successfully
  // the last parameter sets the output range
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

};


} //# NAMESPACE CASA - END

#endif
