//# SIMapperCollection.h: Imager functionality sits here; 
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

#ifndef SYNTHESIS_VB2_SIMAPPERCOLLECTION_H
#define SYNTHESIS_VB2_SIMAPPERCOLLECTION_H

#include <casa/aips.h>
#include <casa/OS/Timer.h>
#include <casa/Containers/Record.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Quanta/Quantum.h>
#include <measures/Measures/MDirection.h>

#include <synthesis/ImagerObjects2/SIMapper.h>
#include <synthesis/ImagerObjects2/SIMapperImageMosaic.h>

namespace casa { //# NAMESPACE CASA - BEGIN
  namespace refim {

class SIMapperCollection 
{
 public:
  // Default constructor

  SIMapperCollection();
  ~SIMapperCollection();

  void addMapper(CountedPtr <SIMapper> map);
  Int nMappers();
  Vector<String> getImageNames();

  /*  // No need for now.
      // Write this function only if we need to support multiple facets on outlier fields too.
      // In that case, hold a list of original ImageStores and a map to the list of Mappers
      //  to keep track of which original image points to which subset of the Mapper list.
      //  Also add code into "finalizeGrid" to copy only one PSF to the original. 
  void addMapper( Int mapperId, 
		  String mapperType,
		  uInt nTaylorTerms,
		  CountedPtr<FTMachine> ftm,
		  CountedPtr<FTMachine> iftm,
		  String imageName,
		  CoordinateSystem& cSys,
		  IPosition imShape, 
		  const Bool overwrite );
  */

  /////////// NEW VI/VB
  void initializeGrid(vi::VisBuffer2& vb, const Bool dopsf=False);
  void grid(vi::VisBuffer2& vb, const Bool dopsf=False, const FTMachine::Type col=FTMachine::CORRECTED);
  void finalizeGrid(vi::VisBuffer2& vb, const Bool dopsf=False);
  void initializeDegrid(vi::VisBuffer2& vb);
  void degrid(vi::VisBuffer2& vb, const Bool saveVirtualMod=False);
  void saveVirtualModel(vi::VisBuffer2& vb);
  void finalizeDegrid(const vi::VisBuffer2& vb);

  Record getFTMRecord(Int mapperid);

  CountedPtr<SIImageStore> imageStore(const Int id=0);
  Bool releaseImageLocks();

  void checkOverlappingModels(String action); // action='blank' or 'restore'

protected:

  ///////////////////// Member Objects

  Block<CountedPtr<SIMapper> >  itsMappers;
  Int oldMsId_p;

};

  }//# namespace refim
} //# NAMESPACE CASA - END

#endif
