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

#include<synthesis/MeasurementEquations/SIMapperCollection.h>
#include<synthesis/MeasurementEquations/SISkyEquation.h>

#include <boost/scoped_ptr.hpp>

namespace casa { //# NAMESPACE CASA - BEGIN

// Forward declarations
class MeasurementSet;
template<class T> class ImageInterface;
 class SIIterBot;

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

  // make all pure-inputs const
  void selectData(Record selpars);
  void defineImage(Record impars);
  void setupImaging(Record gridpars);

  void initMapper();

  //Record getMajorCycleControls();
  void   executeMajorCycle(Record& controls);

  /* Access method to the Loop Controller held in this class */
  //SIIterBot& getLoopControls();

protected:

  /////////////// Internal Functions
  CountedPtr<CoordinateSystem> buildImageCoordinateSystem(String phasecenter, 
							  Double cellx, Double celly, 
							  uInt imx, uInt imy,
							  uInt npol, uInt nchan);

  /////////////// Member Objects

  SIMapperCollection itsMappers;
  //  CountedPtr<VisSet> itsVisSet;
  CountedPtr<FTMachine> itsCurrentFTMachine;
  CountedPtr<CoordinateSystem> itsCurrentCoordSys;
  IPosition itsCurrentImageShape;
  String itsCurrentImageName;

  SISkyEquation itsSkyEquation;

  /////////////// All input parameters

  // Data Selection
  // Image Definition
  // Iteration Control
  // Imaging/Gridding
  // Deconvolution

  // Other Options
  Bool itsUseScratch;

  //////////////// Internal functions
 
};


} //# NAMESPACE CASA - END

#endif
