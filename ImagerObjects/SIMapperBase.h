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

#ifndef SYNTHESIS_SIMAPPERBASE_H
#define SYNTHESIS_SIMAPPERBASE_H

#include <casa/aips.h>
#include <casa/OS/Timer.h>
#include <casa/Containers/Record.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Quanta/Quantum.h>
#include <measures/Measures/MDirection.h>

#include <msvis/MSVis/VisibilityIterator2.h>
#include <msvis/MSVis/VisBuffer2.h>
#include <msvis/MSVis/VisBufferImpl2.h>
#include <msvis/MSVis/VisBuffer2Adapter.h>

#include <synthesis/TransformMachines/FTMachine.h>
#include <synthesis/ImagerObjects/SIImageStore.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Forward declarations
template<class T> class ImageInterface;

// <summary> Class that contains functions needed for imager </summary>

class SIMapperBase 
{
 public:
  // Default constructor

  SIMapperBase( CountedPtr<VPSkyJones>& vp );

  virtual ~SIMapperBase();

  ///// Major Cycle Functions

  virtual void finalizeDegrid(){};

  //////////////// NEW VI/VB versions
  virtual void initializeGrid(const vi::VisBuffer2& , const Bool dopsf=False)=0;
  virtual void grid(const vi::VisBuffer2& , Bool , FTMachine::Type )=0;
  virtual void finalizeGrid(const vi::VisBuffer2& , const Bool )=0;
  virtual void initializeDegrid(const vi::VisBuffer2& , const Int row=-1 )=0;
  virtual void degrid(vi::VisBuffer2& )=0;

  //////////////// OLD VI/VB versions
  virtual void initializeGrid(VisBuffer& , Bool dopsf=False)=0;
  virtual void grid(VisBuffer& , Bool , FTMachine::Type )=0;
  virtual void finalizeGrid(VisBuffer& , Bool dopsf=False)=0;
  virtual void initializeDegrid(VisBuffer& , Int row=-1)=0;
  virtual void degrid(VisBuffer& )=0;

  ////////////////////////////////////Return value is false if no valid ftm or CompList is available
  virtual Bool getFTMRecord(Record & rec, const String diskimage=String(""))=0;
  virtual Bool getCLRecord(Record & rec)=0;

  virtual String getImageName()=0;//{return itsImages->getName();};
  virtual CountedPtr<SIImageStore> imageStore()=0;//{return itsImages;};

  virtual Bool releaseImageLocks()=0;

protected:
  //////////////////// Member Functions
  void initializeGridCore2(const vi::VisBuffer2& vb, 
			   CountedPtr<FTMachine>&  ftm,
			   ImageInterface<Complex>& complexImage);
  //			   Matrix<Float>& sumWeights);
  void initializeGridCore(const VisBuffer& vb, 
			  CountedPtr<FTMachine>&  ftm,
			  ImageInterface<Complex>& complexImage);
			  //			  Matrix<Float>& sumWeights);
  
  void gridCore2(const vi::VisBuffer2& vb, Bool dopsf, FTMachine::Type col,
		CountedPtr<FTMachine>&  ftm, Int row);
  void gridCore(const VisBuffer& vb, Bool dopsf, FTMachine::Type col,
		CountedPtr<FTMachine>&  ftm, Int row);

  
  void finalizeGridCore(const Bool dopsf,
			CountedPtr<FTMachine>&  ftm,
			ImageInterface<Float>& targetImage,
			ImageInterface<Float>& sumwtImage,
			ImageInterface<Float>& weightImage,
			Bool fillWeightImage);
  

  void initializeDegridCore2(const vi::VisBuffer2& vb, 
			    CountedPtr<FTMachine>&  ftm,
			    ImageInterface<Float>& modelImage,
			    ImageInterface<Complex>& complexImage);
  void initializeDegridCore(const VisBuffer& vb, 
			    CountedPtr<FTMachine>&  ftm,
			    ImageInterface<Float>& modelImage,
			    ImageInterface<Complex>& complexImage);
  
  
  void degridCore2(vi::VisBuffer2& vb, CountedPtr<FTMachine>& ftm, 
		  CountedPtr<ComponentFTMachine>& cftm, ComponentList& cl);
  void degridCore(VisBuffer& vb, CountedPtr<FTMachine>& ftm, 
		  CountedPtr<ComponentFTMachine>& cftm, ComponentList& cl);

  void addImageMiscInfo(ImageInterface<Float>& target, 
			CountedPtr<FTMachine>& ftm);
  //			Matrix<Float>& sumwt);

  Bool useWeightImage(CountedPtr<FTMachine>& ftm);
  
  ///////////////////// Member Objects

  // These are supporting params
  Bool itsIsModelUpdated;
  Int itsMapperId;


  // Members used only for image-domain mosaics.

  Bool itsDoImageMosaic;

  Bool changedSkyJonesLogic(const VisBuffer& vb, Bool& firstRow, Bool& internalRow, const Bool grid=True);
  ComponentList clCorrupted_p;
  CountedPtr<VPSkyJones> ejgrid_p, ejdegrid_p;
  VisBuffer ovb_p;
  Bool firstaccess_p;

  void initializeGridCoreMos(const VisBuffer& vb, 
			  CountedPtr<FTMachine>&  ftm,
			     ImageInterface<Complex>& complexImage,
			     Bool firstaccess=False);
  //			  Matrix<Float>& sumWeights);
  
  void gridCoreMos(const VisBuffer& vb, Bool dopsf, FTMachine::Type col,
		   CountedPtr<FTMachine>&  ftm, Int row,
		   ImageInterface<Float>& targetImage,
		   ImageInterface<Float>& sumwtImage,
		   ImageInterface<Float>& weightImage,
		   //				 Matrix<Float>& sumWeights,
		   ImageInterface<Complex>& complexImage);

  
  void finalizeGridCoreMos(const Bool dopsf,
			   CountedPtr<FTMachine>&  ftm,
			   ImageInterface<Float>& targetImage,
			   ImageInterface<Float>& sumwtImage,
			   ImageInterface<Float>& weightImage,
			   //			   Matrix<Float>& sumWeights,
			   ImageInterface<Complex>& compleximage,
			   VisBuffer& vb);
				 
  void initializeDegridCoreMos(const VisBuffer& vb, 
			    CountedPtr<FTMachine>&  ftm,
			    ImageInterface<Float>& modelImage,
			       ImageInterface<Complex>& complexImage,
			       CountedPtr<ComponentFTMachine>& cftm,
			       ComponentList& cl);

  void degridCoreMos(VisBuffer& vb, CountedPtr<FTMachine>& ftm, 
		     CountedPtr<ComponentFTMachine>& cftm,
		     ImageInterface<Float>& modelImage,
		     ImageInterface<Complex>& complexImage);
		     
  void finalizeDegridCoreMos(){};

};


} //# NAMESPACE CASA - END

#endif
