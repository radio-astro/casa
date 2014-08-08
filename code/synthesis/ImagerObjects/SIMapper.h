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

#ifndef SYNTHESIS_SIMAPPER_H
#define SYNTHESIS_SIMAPPER_H

#include <casa/aips.h>
#include <casa/OS/Timer.h>
#include <casa/Containers/Record.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Quanta/Quantum.h>
#include <measures/Measures/MDirection.h>


#include <msvis/MSVis/VisBuffer.h>
#include <synthesis/TransformMachines/FTMachine.h>


namespace casa { //# NAMESPACE CASA - BEGIN

// Forward declarations
  class ComponentFTMachine;
  class SkyJones;
template<class T> class ImageInterface;

// <summary> Class that contains functions needed for imager </summary>

  class SIMapper// : public SIMapperBase
{
 public:
  // Default constructor

  SIMapper( CountedPtr<SIImageStore>& imagestore,
            CountedPtr<FTMachine>& ftm, 
	    CountedPtr<FTMachine>& iftm);
  SIMapper(const ComponentList& cl, 
	   String& whichMachine);
  virtual ~SIMapper();

  ///// Major Cycle Functions

  /////////////////////// OLD VI/VB versions
  virtual void initializeGrid(VisBuffer& vb, Bool dopsf, Bool firstaccess=False);
  virtual void grid(VisBuffer& vb, Bool dopsf, FTMachine::Type col);
  virtual void finalizeGrid(VisBuffer& vb, Bool dopsf);
  virtual void initializeDegrid(VisBuffer& vb, Int row=-1);
  virtual void degrid(VisBuffer& vb);

  virtual void finalizeDegrid();

  //////////////the return value is False if no valid record is being returned
  Bool getCLRecord(Record& rec);
  Bool getFTMRecord(Record& rec, const String diskimage="");

  virtual String getImageName(){return itsImages->getName();};
  virtual CountedPtr<SIImageStore> imageStore(){return itsImages;};
  virtual Bool releaseImageLocks(){return itsImages->releaseLocks();};

protected:

  CountedPtr<FTMachine> ft_p, ift_p; 

  CountedPtr<ComponentFTMachine> cft_p;
  ComponentList cl_p;

  CountedPtr<SIImageStore> itsImages;

};


} //# NAMESPACE CASA - END

#endif
