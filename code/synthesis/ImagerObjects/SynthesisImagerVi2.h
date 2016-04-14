//# SynthesisImagerVi2.h: Imager functionality sits here; 
//# Copyright (C) 2016
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
#ifndef SYNTHESIS_SYNTHESISIMAGERVI2_H
#define SYNTHESIS_SYNTHESISIMAGERVI2_H

#include <synthesis/ImagerObjects/SynthesisImager.h>
#include <synthesis/TransformMachines2/FTMachine.h>
#include <msvis/MSVis/ViFrequencySelection.h>
#include <msvis/MSVis/VisibilityIterator2.h>
#include <msvis/MSVis/VisBuffer2.h>

namespace casa { //# NAMESPACE CASA - BEGIN
class MeasurementSet;
template<class T> class ImageInterface;
class VisImagingWeight;
 class SynthesisImagerVi2  : public SynthesisImager
{

public:
  // Default constructor

  SynthesisImagerVi2();
  virtual ~SynthesisImagerVi2();
  virtual Bool selectData(const SynthesisParamsSelect& selpars);
  virtual Bool defineImage(SynthesisParamsImage& impars, const SynthesisParamsGrid& gridpars);
  virtual Bool weight(const String& type="natural", 
	      const String& rmode="norm",
	      const Quantity& noise=Quantity(0.0, "Jy"), 
	      const Double robust=0.0,
	      const Quantity& fieldofview=Quantity(0.0, "arcsec"),
	      const Int npixels=0, 
	      const Bool multiField=False,
	      const String& filtertype=String("Gaussian"),
	      const Quantity& filterbmaj=Quantity(0.0,"deg"),
	      const Quantity& filterbmin=Quantity(0.0,"deg"),
	      const Quantity& filterbpa=Quantity(0.0,"deg")  );
 protected:
  void appendToMapperList(String imagename, 
			  CoordinateSystem& csys, 
			  IPosition imshape,
			  CountedPtr<refim::FTMachine>& ftm,
			  CountedPtr<refim::FTMachine>& iftm,
		  	  Quantity distance=Quantity(0.0, "m"), 
			  Int facets=1, 
			  Int chanchunks=1,
			  const Bool overwrite=False,
			  String mappertype=String("default"),
			  uInt ntaylorterms=1,
			  Vector<String> startmodel=Vector<String>(0));
  virtual void unlockMSs();
  virtual void createVisSet(const Bool writeaccess=False);
  void createFTMachine(CountedPtr<casa::refim::FTMachine>& theFT, 
		       CountedPtr<casa::refim::FTMachine>& theIFT,  
		       const String& ftname,
		       const uInt nTaylorTerms=1, 
		       const String mType="default",
		       const Int facets=1,
		       //------------------------------
		       const Int wprojplane=1,
		       const Float padding=1.0,
		       const Bool useAutocorr=False,
		       const Bool useDoublePrec=True,
		       const String gridFunction=String("SF"),
		       //------------------------------
		       const Bool aTermOn    = True,
		       const Bool psTermOn   = True,
		       const Bool mTermOn    = False,
		       const Bool wbAWP      = True,
		       const String cfCache  = "",
		       const Bool doPointing = False,
		       const Bool doPBCorr   = True,
		       const Bool conjBeams  = True,
		       const Float computePAStep   = 360.0,
		       const Float rotatePAStep    = 5.0,
		       const String interpolation = String("linear"),
		       const Bool freqFrameValid = True,
		       const Int cache=1000000000,
		       const Int tile=16,
		       const String stokes="I",
		       const String imageNamePrefix="");

  void createAWPFTMachine(CountedPtr<refim::FTMachine>& theFT, CountedPtr<refim::FTMachine>& theIFT, 
			  const String& ftmName,
			  const Int facets,          
			  //----------------------------
			  const Int wprojPlane,     
			  const Float padding,      
			  const Bool useAutocorr,   
			  const Bool useDoublePrec, 
			  const String gridFunction,
			  //---------------------------
			  const Bool aTermOn,      
			  const Bool psTermOn,     
			  const Bool mTermOn,      
			  const Bool wbAWP,        
			  const String cfCache,    
			  const Bool doPointing,   
			  const Bool doPBCorr,     
			  const Bool conjBeams,    
			  const Float computePAStep,
			  const Float rotatePAStep, 
			  const Int cache,          
			  const Int tile,
			  const String imageNamePrefix="");
 
// Do the major cycle
  virtual void runMajorCycle(const Bool dopsf=False, const Bool savemodel=False);

  // Version of major cycle code with mappers in a loop outside vi/vb.
  virtual void runMajorCycle2(const Bool dopsf=False, const Bool savemodel=False);
 
 void createMosFTMachine(CountedPtr<casa::refim::FTMachine>& theFT, CountedPtr<casa::refim::FTMachine>&  theIFT, const Float  padding, const Bool useAutoCorr, const Bool useDoublePrec, const Float rotatePAStep, const String Stokes="I");
  CountedPtr<SIMapper> createSIMapper(String mappertype,  
				      CountedPtr<SIImageStore> imagestore, //// make this inside !!!!!
				      CountedPtr<refim::FTMachine> ftmachine,
				      CountedPtr<refim::FTMachine> iftmachine,
				      uInt ntaylorterms=1);

 
   // Other Options
  Block<const MeasurementSet *> mss_p;
  vi::FrequencySelections fselections_p;
  CountedPtr<vi::VisibilityIterator2>  vi_p;


};
} //# NAMESPACE CASA - END

#endif

