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
		    const casacore::String& pointingDirCol_p);
  SDPosInterpolator(const casacore::Vector<casacore::Vector<casacore::Double> >& time,
		    const casacore::Vector<casacore::Vector<casacore::Vector<casacore::Double> > >& dir);
  ~SDPosInterpolator();
  casacore::Vector<casacore::Bool> doSplineInterpolation;   //(antid)
  casacore::MDirection interpolateDirectionMeasSpline(const casacore::ROMSPointingColumns& mspc,
					    const casacore::Double& time,
					    const casacore::Int& index,
					    const casacore::Int& antid);
  casacore::Vector<casacore::Vector<casacore::Vector<casacore::Vector<casacore::Double> > > > getSplineCoeff();
private:
  casacore::Vector<casacore::Vector<casacore::Double> > timePointing; //(antid)(index)
  casacore::Vector<casacore::Vector<casacore::Vector<casacore::Double> > > dirPointing; //(antid)(index)(xy)
  casacore::Vector<casacore::Vector<casacore::Vector<casacore::Vector<casacore::Double> > > > splineCoeff; //(antid)(index)(xy)(order)
  void setup(const VisBuffer& vb,
	     const casacore::String& pointingDirCol_p);
  void setup(const casacore::Vector<casacore::Vector<casacore::Double> >& time,
	     const casacore::Vector<casacore::Vector<casacore::Vector<casacore::Double> > >& dir);
  void calcSplineCoeff(const casacore::Vector<casacore::Double>& time,
		       const casacore::Vector<casacore::Vector<casacore::Double> >& dir,
		       casacore::Vector<casacore::Vector<casacore::Vector<casacore::Double> > >& coeff);
};

} //# NAMESPACE CASA - END

#endif
