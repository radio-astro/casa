//# SynthesisImager.h: Imager functionality sits here; 
//# Copyright (C) 2012-2013
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
#include <synthesis/MSVis/ViFrequencySelection.h>
#include <synthesis/MSVis/VisibilityIterator2.h>
#include <synthesis/MSVis/VisBuffer2.h>
#include<synthesis/ImagerObjects/SIMapperCollection.h>

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
  virtual ~SynthesisImager();

  // Copy constructor and assignment operator
  //Imager(const Imager&);
  //Imager& operator=(const Imager&);

  // make all pure-inputs const
  void selectData(Record selpars);
  virtual Bool selectData(const String& msname, const String& spw, const String& field, const String& taql,  const String& antenna,  const String& uvdist, const String& scan, const String& obs, const String& timestr, const Bool usescratch=False, const Bool readonly=False);
  void defineImage(Record impars);
  virtual Bool defineImage(const String& imagename, const Int nx, const Int ny,
			   const Quantity& cellx, const Quantity& celly,
			   const String& stokes,
			   const MDirection& phaseCenter, 
			   const Int nchan,
			   const Quantity& freqStart,
			   const Quantity& freqStep, 
			   const Vector<Quantity>& restFreq,
			   const Int facets=1,
			   const String& ftmachine="GridFT", 
			   const Projection& projection=Projection::SIN,
			   const Quantity& distance=Quantity(0,"m"),
			   const MFrequency::Types& freqFrame=MFrequency::LSRK,
			   const Bool trackSource=False, const MDirection& 
			   trackDir=MDirection(Quantity(0.0, "deg"), 
					       Quantity(90.0, "deg")));
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

  CoordinateSystem buildCoordSys(const MDirection& phasecenter, const Quantity& cellx, const Quantity& celly, const Int nx, const Int ny, const String& stokes, const Projection& projection, const Int nchan, const Quantity& freqStart, const Quantity& freqStep, const Vector<Quantity>& restFreq);

  void createFTMachine(CountedPtr<FTMachine>& theFT, const String& ftname);
  void createVisSet();
  
  void runMajorCycle();
  Vector<Int> decideNPolPlanes(const String& stokes);
  void appendToMapperList(String imagename, CoordinateSystem& csys, String ftmachine, Quantity distance=Quantity(0.0, "m"), Int facets=1);
  /////////////// Member Objects

  SIMapperCollection itsMappers;

  CountedPtr<FTMachine> itsCurrentFTMachine;
  CountedPtr<CoordinateSystem> itsCurrentCoordSys;
  CountedPtr<SIImageStore> itsCurrentImages;

  /////////////// All input parameters

  // Data Selection
  // Image Definition
  // Imaging/Gridding

  // Other Options
  Bool writeAccess_p;
  Block<const MeasurementSet *> mss_p;
  vi::FrequencySelections fselections_p;
  CountedPtr<vi::VisibilityIterator2>  vi_p;
  Int nx_p, ny_p, nstokes_p, nchan_p, facets_p;
  Quantity cellx_p, celly_p, distance_p;
  String stokes_p;
  MDirection phasecenter_p;
  Quantity freqStart_p, freqStep_p;
  MFrequency::Types freqFrame_p;
  MPosition mLocation_p;
  Bool freqFrameValid_p;
  Int wprojPlanes_p;
  Bool useAutocorr_p;
  Bool useDoublePrec_p;
  Float padding_p;
  Int cache_p, tile_p;
  String gridFunction_p;
};


} //# NAMESPACE CASA - END

#endif
