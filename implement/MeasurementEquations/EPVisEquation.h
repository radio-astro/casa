//# VisEquation.h: Interface definition for Vis Equation
//# Copyright (C) 1996,1997,1999,2000,2001,2002,2003
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
//-*-C++-*-
//# $Id$

#ifndef SYNTHESIS_EPVISEQUATION_H
#define SYNTHESIS_EPVISEQUATION_H

#include <casa/aips.h>
#include <casa/BasicSL/Complex.h>
#include <casa/Arrays/Matrix.h>
#include <synthesis/MeasurementComponents/VisJones.h>
#include <synthesis/MeasurementComponents/XCorr.h>
#include <synthesis/MeasurementComponents/MJones.h>
#include <synthesis/MeasurementComponents/ACoh.h>
#include <synthesis/MeasurementEquations/VisEquation.h>
#include <msvis/MSVis/VisibilityIterator.h>
#include <msvis/MSVis/VisSet.h>
#include <msvis/MSVis/VisBuffer.h>
#include <synthesis/MeasurementComponents/EPJones.h>

namespace casa {
class EPVisEquation: public VisEquation {
public:

  EPVisEquation(VisSet& vs);
  
  virtual ~EPVisEquation();
  
  EPVisEquation(const EPVisEquation& other);

  EPVisEquation& operator=(const EPVisEquation& other);

  void getResiduals(VisJones& VJ,VisBuffer& VRes)   {};
  void getGradients(VisJones& VJ, VisBuffer& GradV) {};
  void predict(EPJones& EPVJ, VisBuffer& VMod);
  void getResAndGrad(EPJones& VJ, 
		     VisBuffer& VRes, VisBuffer& grad1, VisBuffer& grad2);
  //----------------------------------------------------------------------
  void setVisJones(EPJones& j) { epj_=&j;};
  void initChiSquare(EPJones& vj);
protected:
  VisJones* epj_;
};
}
#endif
