//# SynthesisImager.h: Imager functionality sits here; 
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
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#
//# $Id$

#ifndef SYNTHESIS_SYNTHESISIMAGER_H
#define SYNTHESIS_SYNTHESISIMAGER_H

#include <casa/aips.h>
#include <casa/OS/Timer.h>
#include <casa/Containers/Record.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Quanta/Quantum.h>
#include <measures/Measures/MDirection.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Forward declarations
class MeasurementSet;
class ViewerProxy;
template<class T> class ImageInterface;

// <summary> Class that contains functions needed for imager </summary>

class SynthesisImager 
{
 public:
  // Default constructor

  SynthesisImager();
  ~SynthesisImager();

  // Copy constructor and assignment operator
  //Imager(const Imager&);
  //Imager& operator=(const Imager&);


  void selectData(Record selpars);
  void defineImage(Record impars);
  void setupImaging(Record gridpars);
  void setupDeconvolution(Record recpars);
  void setupIteration(Record iterpars);
  //  void setOtherOptions(Bool usescratch);
  // Record initLoops();
  void endLoops(Record& loopcontrols);
  void runMajorCycle(){Record rec; runMajorCycle(rec);}
  void runMajorCycle(Record& loopcontrols);
  void runMinorCycle(Record& loopcontrols);

protected:

  // All input parameters

  // Data Selection
  // Image Definition
  String startmodel_p;

  // Iteration Control
  Int niter_p;
  // Imaging/Gridding
  // Deconvolution

  // Other Options
  Bool usescratch_p;

  // Internal functions
  Bool checkLoopControls(Record &loopcontrols);
 
};


} //# NAMESPACE CASA - END

#endif
