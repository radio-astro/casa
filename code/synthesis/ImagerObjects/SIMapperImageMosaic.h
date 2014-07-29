//# SIMapper.h: Imager functionality sits here; 
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

#ifndef SYNTHESIS_SIMAPPERMOSAIC_H
#define SYNTHESIS_SIMAPPERMOSAIC_H

#include <casa/aips.h>
#include <casa/OS/Timer.h>
#include <casa/Containers/Record.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Quanta/Quantum.h>
#include <measures/Measures/MDirection.h>
#include <msvis/MSVis/VisBuffer.h>
#include <msvis/MSVis/VisBufferImpl2.h>
#include <synthesis/TransformMachines/FTMachine.h>
#include <synthesis/ImagerObjects/SIMapper.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Forward declarations
  class ComponentFTMachine;
  class SkyJones;
template<class T> class ImageInterface;

// <summary> Class that contains functions needed for imager </summary>

  class SIMapperImageMosaic : public SIMapper
{
 public:
  // Default constructor

  SIMapperImageMosaic( CountedPtr<SIImageStore>& imagestore,
            CountedPtr<FTMachine>& ftm, 
		       CountedPtr<FTMachine>& iftm);
  //	    CountedPtr<VPSkyJones>& vp);
  SIMapperImageMosaic(const ComponentList& cl, 
		      String& whichMachine);
  //	   CountedPtr<VPSkyJones>& vp);
  virtual ~SIMapperImageMosaic();

  ///// Major Cycle Functions

  /////////////////////// NEW VI/VB versions
  void initializeGrid(const vi::VisBuffer2& vb, Bool dopsf);
  void grid(const vi::VisBuffer2& vb, Bool dopsf, FTMachine::Type col);
  void finalizeGrid(const vi::VisBuffer2& vb, const Bool dopsf);
  void initializeDegrid(const vi::VisBuffer2& vb, const Int row=-1);
  void degrid(vi::VisBuffer2& vb);
  //  void finalizeDegrid();

  /////////////////////// OLD VI/VB versions
  void initializeGrid(VisBuffer& vb, Bool dopsf, Bool firstaccess=False);
  void grid(VisBuffer& vb, Bool dopsf, FTMachine::Type col);
  void finalizeGrid(VisBuffer& vb, Bool dopsf);
  void initializeDegrid(VisBuffer& vb, Int row=-1);
  void degrid(VisBuffer& vb);

  //////////////the return value is False if no valid record is being returned
  //  Bool getCLRecord(Record& rec);
  //  Bool getFTMRecord(Record& rec);

  virtual String getImageName(){return itsImages->getName();};
  virtual CountedPtr<SIImageStore> imageStore(){return itsImages;};
  virtual Bool releaseImageLocks(){return itsImages->releaseLocks();};

protected:
  //  Bool changedSkyJonesLogic(const vi::VisBuffer2& vb, Bool& firstRow, Bool& internalRow, const Bool grid=True);
  //////////////OLD vb version
  //  Bool changedSkyJonesLogic(const VisBuffer& vb, Bool& firstRow, Bool& internalRow, const Bool grid=True);
  ////////////////////////////////////////////

  ComponentList clCorrupted_p;
  //  CountedPtr<VPSkyJones>  ejgrid_p, ejdegrid_p;
  VisBuffer ovb_p;

  Bool firstaccess_p;

};


} //# NAMESPACE CASA - END

#endif
