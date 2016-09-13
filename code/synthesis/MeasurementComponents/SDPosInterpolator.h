//# SDPosInterpolator.h: Definition for SDPosInterpolator
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003
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

#ifndef SYNTHESIS_SDPOSINTERPOLATOR_H
#define SYNTHESIS_SDPOSINTERPOLATOR_H

#include <casa/Arrays/Vector.h>
#include <casa/BasicSL/Constants.h>
#include <casa/Quanta/MVAngle.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Quanta/UnitMap.h>
#include <casa/Quanta/UnitVal.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/Measure.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <msvis/MSVis/VisBuffer.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class SDPosInterpolator {
public:
  SDPosInterpolator(const VisBuffer& vb,
		    const String& pointingDirCol_p);
  SDPosInterpolator(const Vector<Vector<Double> >& time,
		    const Vector<Vector<Vector<Double> > >& dir);
  ~SDPosInterpolator();
  Vector<Bool> doSplineInterpolation;   //(antid)
  MDirection interpolateDirectionMeasSpline(const ROMSPointingColumns& mspc,
					    const Double& time,
					    const Int& index,
					    const Int& antid);
  Vector<Vector<Vector<Vector<Double> > > > getSplineCoeff();
private:
  Vector<Vector<Double> > timePointing; //(antid)(index)
  Vector<Vector<Vector<Double> > > dirPointing; //(antid)(index)(xy)
  Vector<Vector<Vector<Vector<Double> > > > splineCoeff; //(antid)(index)(xy)(order)
  void setup(const VisBuffer& vb,
	     const String& pointingDirCol_p);
  void setup(const Vector<Vector<Double> >& time,
	     const Vector<Vector<Vector<Double> > >& dir);
  void calcSplineCoeff(const Vector<Double>& time,
		       const Vector<Vector<Double> >& dir,
		       Vector<Vector<Vector<Double> > >& coeff);
};

} //# NAMESPACE CASA - END

#endif
