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

#ifndef SYNTHESIS_SIMAPPERCOLLECTION_H
#define SYNTHESIS_SIMAPPERCOLLECTION_H

#include <casa/aips.h>
#include <casa/OS/Timer.h>
#include <casa/Containers/Record.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Quanta/Quantum.h>
#include <measures/Measures/MDirection.h>

#include <synthesis/ImagerObjects/SIMapperBase.h>
#include <synthesis/ImagerObjects/SIMapper.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class SIMapperCollection 
{
 public:
  // Default constructor

  SIMapperCollection();
  ~SIMapperCollection();

  void addMapper( String mappertype, 
		  CountedPtr<SIImageStore> imagestore,
		  CountedPtr<FTMachine> ftmachine);

  void addMapper(CountedPtr <SIMapperBase>& map);
  Int nMappers();
  Vector<String> getImageNames();

  //// For KG : Need to add the interface for 'vb' flowing through here....
  void initializeGrid(Int mapperid);
  void grid(Int mapperid);
  void finalizeGrid(Int mapperid);

  void initializeDegrid(Int mapperid);
  void degrid(Int mapperid);
  void finalizeDegrid(Int mapperid);
////////////The following will loop over the SIMappers internally
  void initializeGrid(const vi::VisBuffer2& vb);
  void grid(vi::VisBuffer2& vb, const Bool dopsf=False, const FTMachine::Type col=FTMachine::CORRECTED);
  void finalizeGrid(const vi::VisBuffer2& vb, const Bool dopsf=False);
  void initializeDegrid(const vi::VisBuffer2& vb);
  void degrid(vi::VisBuffer2& vb, const Bool useScratch=False);
  void saveVirtualModel(vi::VisBuffer2& vb);
  void finalizeDegrid(const vi::VisBuffer2& vb);
  ////////////Version that use old vi/vb can be removed the vi2/vb2 works
  void initializeGrid(const VisBuffer& vb);
  void grid(VisBuffer& vb, const Bool dopsf=False, const FTMachine::Type col=FTMachine::CORRECTED);
  void finalizeGrid(const VisBuffer& vb, const Bool dopsf=False);
  void initializeDegrid(const VisBuffer& vb);
  void degrid(VisBuffer& vb, const Bool useScratch=False);
  void saveVirtualModel(VisBuffer& vb);
  void finalizeDegrid(const VisBuffer& vb);
  ////////////////////////////////////////////////////

  Record getFTMRecord(Int mapperid);

  CountedPtr<SIImageStore> imageStore(const Int id=0);
  Bool releaseImageLocks();

protected:

  ///////////////////// Member Objects

  Block<CountedPtr<SIMapperBase> >  itsMappers;
  Int oldMsId_p;

};


} //# NAMESPACE CASA - END

#endif
