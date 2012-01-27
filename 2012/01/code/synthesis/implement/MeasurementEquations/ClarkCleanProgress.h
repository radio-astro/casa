//# ClarkCleanProgress.h: Abstract base class to monitor progress in lattice operations
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

#ifndef SYNTHESIS_CLARKCLEANPROGRESS_H
#define SYNTHESIS_CLARKCLEANPROGRESS_H
 
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
// Abstract base class to monitor progress in lattice operations
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <synopsis>
// This is an abstract base class for classes to monitor the
// progress of an operation on a Lattice. The default implementation
// offered by this class does nothing.
// However, a derived class could show the progress using for example
// a <linkto class=ProgressMeter>ProgressMeter</linkto>. A derived
// class should override the virtual functions from this class.
//
// The user of the ClarkCleanProgress object should first call
// function <src>init</src> with the total number of steps
// that are to be done.   Thereafter, after each step has been
// executed, function <src>nstepsDone</src> should be called
// after each step.  Finally, function <src>done</src> should
// be called.
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


class ClarkCleanProgress {
public:
  ClarkCleanProgress(PGPlotter* pgplotter=0, Int plotterIncrement=10);

  // delete its PGPlotter
  virtual ~ClarkCleanProgress();
  
  // Plot the information if it has a PGPlotter
  Bool info(const Bool lastcall,
	    const Int iteration,
	    const Int numberIterations,
	    const Float& maximum,
	    const IPosition& posMaximum,
	    const Float& totalFlux,
	    const Bool majorIteration,
	    const Bool resetBase=False);
  
  // Finish up any aspects of the plot which need to be finished
  Bool finalize();

  // returns true if we have a live pgplotter_pointer
  // Use: do a     try { progress_pointer->hasPGPlotter(); }
  Bool hasPGPlotter();

protected:

private:

  // draw the outline of the plot;
  // If doplot = True, redraw all the past data as well.
  // THIS ASSUMES WE HAVE A VALID PGPLOTTER
  void basicSetUp(Bool doPlot = False);

  // plot just the current points
  // THIS ASSUMES WE HAVE A VALID PGPLOTTER
  void plotOne(const Int iteration, 
	       const Float resid, const Float flux);

  // replot all points, but don't redraw plotter
  // THIS ASSUMES WE HAVE A VALID PGPLOTTER
  void plotVectors();

  PGPlotter* itsPgplotter;

  Vector<Float> iterationNumber;   	
  Vector<Float> maxResiduals;		
  Vector<Float> posResiduals;		
  Vector<Float> negResiduals;		
  Vector<Float> totalFluxes;
  uInt currentIndex;
  uInt currentTotalIterations;
  Float currentFluxScale;
  Float currentMinFluxScale;
  Float currentMaxResidual;
  Float currentMinResidual;

  // we are going to plot all points, but not all at once; every Increment iterations
  Int plottingIncrement;

  Float baseFlux;

};



} //# NAMESPACE CASA - END

#endif
