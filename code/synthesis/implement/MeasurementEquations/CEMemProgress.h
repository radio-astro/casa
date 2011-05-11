//# CEMemProgress.h:  monitor progress in Mem
//# Copyright (C) 1997,1998,1999,2000
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

#ifndef SYNTHESIS_CEMEMPROGRESS_H
#define SYNTHESIS_CEMEMPROGRESS_H
 
//# Includes
#include <casa/aips.h>
#include <casa/Quanta/Quantum.h>
#include <lattices/Lattices/Lattice.h>
#include <lattices/Lattices/TempLattice.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Vector.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//# Forward Declarations
template <class T> class Vector;
class PGPlotter;

// <summary>
// Class to monitor progress in MEM deconvolution
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <synopsis>
// </synopsis>

// <example>
// <srcblock>
// </srcblock>
// </example>

// <motivation>
// Since operations on Lattices can take a while, it can be useful
// to show the progress. However, making module Lattices dependent on
// the class ProgressMeter sounded bad. This abstract class serves
// as a bridge between the Lattice module and the ProgressMeter class
// (or any other class showing the progress).
// </motivation>
//
//# <todo asof="1997/08/01">   
//#   <li> 
//# </todo>


class CEMemProgress {
public:

  enum EntType {
    ENTROPY=0,
    EMPINESS=1
  };
  


  CEMemProgress(PGPlotter* pgplotter=0);

  virtual ~CEMemProgress();
  
 // people usually don't worry about maximum residual with MEM; we'll do it, though

  void info(const Bool lastcall,
	    const Int iteration,
	    const Int numberIterations,
	    const Lattice<Float>& model,
	    const Lattice<Float>& resid,
	    const Float& maximumRes,         
	    const IPosition& posMaximum,
	    const Float& totalFlux,
	    const Float& sigma,
	    const Float& normGrad,
	    const Float& entropy);
  
protected:

private:

  void basicSetUp(Bool doPlot = False);

  void plotOne(const Int iteration, 
	       const Float sigma, const Float maxResid, 
	       const Float flux);

  PGPlotter* itsPgplotter;

  Vector<Float> itsIterationNumbers;
  Vector<Float> itsTotalFluxes;
  Vector<Float> itsMaxResiduals;
  Vector<Float> itsSigmas;
  Vector<Float> itsNormGrads;
  Vector<Float> itsEntropies;
  uInt itsCurrentIndex;
  uInt itsCurrentTotalIterations;
  Float itsCurrentFluxScale;
  Float itsCurrentMaxResidualScale;
  Float itsCurrentSigmaScale;        
  Float itsCurrentNormGradScale;        
  Float itsCurrentEntropyScale;        
};



} //# NAMESPACE CASA - END

#endif
