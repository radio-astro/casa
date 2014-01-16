//# CalInterpolation.h: A class to hold calibration interpolation parameters
//# Copyright (C) 1996,1997,1998,1999,2001,2002
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
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id$

#ifndef CALIBRATION_CALINTERPOLATION_H
#define CALIBRATION_CALINTERPOLATION_H

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Quanta/Quantum.h>
#include <synthesis/MSVis/MSCalEnums.h>
#include <ms/MeasurementSets/MSSelection.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> 
// CalInterpolation: a class to hold calibration interpolation parameters
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="MSCalEnums">MSCalEnums</linkto> module
//   <li> <linkto class="CalBuffer">CalBuffer</linkto> module
//   <li> <linkto class="MSSelection">MSSelection</linkto> module
// </prerequisite>
//
// <etymology>
// From "calibration" and "interpolation".
// </etymology>
//
// <synopsis>
// The CalInterpolation class holds calibration interpolation parameters 
// which define how calibration is to be applied. Specializations for 
// parametrized Jones matrices, which required sampling before 
// interpolation, are provided through inheritance.
// </etymology>
//
// <example>
// <srcblock>
// </srcblock>
// </example>
//
// <motivation>
// This class is used by the calibration interpolater classes.
// </motivation>
//
// <todo asof="2001/08/25">
// (i)
// </todo>

class CalInterpolation
{
 public:
  // Basic interpolation type
  enum Type {
    // Linear two-point
    LINEAR = 0,

    // Nearest calibration solution only
    NEAREST_NEIGHBOUR = 1,

    // Fit a polynomial and interpolate
    POLYNOMIAL = 2,

    // Use natural cubic splines to interpolate
    SPLINE = 3
  };

  // Interpolation weighting type
  enum Weighting {
    // Weight by the calibration solution weight
    WEIGHTED = 1,

    // Use unit weighting for all points 
    UNWEIGHTED = 0
  };

  // Default null constructor, and destructor
  CalInterpolation();
  virtual ~CalInterpolation();

  // Copy constructor and assignment operator
  CalInterpolation (const CalInterpolation& other);
  virtual CalInterpolation& operator= (const CalInterpolation& other);

  // Set interpolation axes
  virtual void setAxes (const Vector<Int>& axes) {axes_p = axes;}

  // Set interpolation type
  virtual void setType (const Type& type) {type_p = type;};

  // Set interpolation weighting
  virtual void setWeighting (const Weighting& weighting) 
    {weighting_p = weighting;};

  // Set interpolation window (per axis)
  virtual void setWindows (const Vector<Quantity>& windows) 
    {windows_p = windows;};

  // Set polynomial order
  virtual void setNpoly (const Int& npoly) {npoly_p = npoly;};

  // Set interpolation index mapping
  virtual void setIndexMap (const Vector<MSSelection>& msIndex,
			    const Vector<MSSelection>& calIndex)
    {msIndex_p = msIndex; calIndex_p = calIndex;};   

  // Get number and type of interpolation axes
  virtual Vector<Int> axes() {return axes_p;};
  virtual Int nAxes() {return axes_p.nelements();};

  // Get interpolation type
  virtual Type type() {return type_p;};

  // Get interpolation weighting type
  virtual Weighting weighting() {return weighting_p;};

  // Get interpolation windows for each axis
  virtual Vector<Quantity> windows() {return windows_p;};

  // Get polynomial order
  virtual Int nPoly() {return npoly_p;};

  // Get interpolation index mapping
  virtual Vector<MSSelection> msIndex() {return msIndex_p;};
  virtual Vector<MSSelection> calIndex() {return calIndex_p;};

 protected:

 private:
  // Interpolation axes
  Vector<Int> axes_p;

  // Interpolation type
  Type type_p;

  // Interpolation weighting type
  Weighting weighting_p;

  // Interpolation windows
  Vector<Quantity> windows_p;

  // Polynomial order
  Int npoly_p;

  // Interpolation index mapping
  Vector<MSSelection> msIndex_p, calIndex_p;
 };


} //# NAMESPACE CASA - END

#endif
   
