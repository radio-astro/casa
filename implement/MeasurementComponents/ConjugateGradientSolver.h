//# ConjugateGradientSolver.h: Definition for an iterative ConjugateGradientSolver
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

#ifndef SYNTHESIS_CONJUGATEGRADIENT_H
#define SYNTHESIS_CONJUGATEGRADIENT_H
//# ConjugateGradientSolver.h: Definition of ConjugateGradientSolver
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2003
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
//----------------------------------------------------------------------------

#include <casa/aips.h>

#include <synthesis/MeasurementEquations/EPVisEquation.h>
#include <synthesis/MeasurementEquations/Iterate.h>
#include <synthesis/MeasurementComponents/EPJones.h>
#include <msvis/MSVis/VisBuffer.h>
#include <casa/Arrays/Vector.h>
#include <casa/OS/Timer.h>

#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogMessage.h>

namespace casa {

  class ConjugateGradientSolver: public Iterate
  {
  public:
    ConjugateGradientSolver(Int nParams, Int nIter=20, Double tol=1E-5);
    ~ConjugateGradientSolver(){};

    void setMaxParams(Int nParams){maxParams=nParams;};
    
    Double solve(EPVisEquation& ve, EPJones& vj,Int nAnt, Int SlotNo,Array<Float>& guess);

    LogIO& logIO() {return logIO_p;};
    
  private:
    Vector<Complex> getVj(VisBuffer& vb, Int NAnt, Int whichAnt, Double& sumWt);
    Double getGOF(VisBuffer& residual,Double& sumWt);

    Int maxIter,maxParams;
    
    VisBuffer residual,gradient0,gradient1;
    //    Vector<VisBuffer> gradient;

    LogIO logIO_p;
  };
};

#endif
