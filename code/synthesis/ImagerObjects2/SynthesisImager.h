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

#ifndef SYNTHESIS_VB2_SYNTHESISIMAGER_H
#define SYNTHESIS_VB2_SYNTHESISIMAGER_H

#include <casa/aips.h>
#include <casa/OS/Timer.h>
#include <casa/Containers/Record.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Quanta/Quantum.h>
#include <measures/Measures/MDirection.h>
#include<synthesis/ImagerObjects/SynthesisUtilMethods.h>

#include<synthesis/ImagerObjects/SIMapperCollection.h>
#include <msvis/MSVis/ViFrequencySelection.h>

#include <msvis/MSVis/VisibilityIterator2.h>
#include <synthesis/TransformMachines2/FTMachine.h>
#include <synthesis/TransformMachines2/ATerm.h>

namespace casacore{

class MeasurementSet;
template<class T> class ImageInterface;
}

namespace casa { //# NAMESPACE CASA - BEGIN
// Forward declarations
 class SIIterBot;
 class VisImagingWeight;

// <summary> Class that contains functions needed for imager </summary>
  namespace refim { //# NAMESPACE refim - BEGIN

class SynthesisImager 
{
 public:
  // Default constructor

  SynthesisImager();
  virtual ~SynthesisImager();

  // Copy constructor and assignment operator
  //Imager(const Imager&);
  //Imager& operator=(const Imager&);

  virtual casacore::Bool selectData(const SynthesisParamsSelect& selpars);

  // make all pure-inputs const
  virtual casacore::Bool selectData(const casacore::String& msname, 
			  const casacore::String& spw="*", 
			  const casacore::String& freqBeg="", 
			  const casacore::String& freqEnd="",
			  const casacore::MFrequency::Types freqFrame=casacore::MFrequency::LSRK, 
			  const casacore::String& field="*", 
			  const casacore::String& antenna="",  
			  const casacore::String& timestr="", 
			  const casacore::String& scan="", 
			  const casacore::String& obs="",
			  const casacore::String& state="",
			  const casacore::String& uvdist="", 
			  const casacore::String& taql="",
			  const casacore::Bool usescratch=false, 
			  const casacore::Bool readonly=false, 
			  const casacore::Bool incrementModel=false);

  virtual casacore::Bool defineImage(SynthesisParamsImage& impars, const SynthesisParamsGrid& gridpars);

  //When having a facetted image ...call with (facets > 1)  first and  once only ..
  //Easier to keep track of the imstores that way
  ////CAREFUL: make sure you donot overwrite if you want to predict the model or subtract it to make residual
  virtual casacore::Bool defineImage(const casacore::String& imagename, const casacore::Int nx, const casacore::Int ny,
			   const casacore::Quantity& cellx, const casacore::Quantity& celly,
			   const casacore::String& stokes,
			   const casacore::MDirection& phaseCenter, 
			   const casacore::Int nchan,
			   const casacore::Quantity& freqStart,
			   const casacore::Quantity& freqStep, 
			   const casacore::Vector<casacore::Quantity>& restFreq,
			   const casacore::Int facets=1,
			   const casacore::String ftmachine="GridFT",
			   const casacore::Int nTaylorTerms=1,
			   const casacore::Quantity& refFreq = casacore::Quantity(0,"Hz"),
			   const casacore::Projection& projection=casacore::Projection::SIN,
			   const casacore::Quantity& distance=casacore::Quantity(0,"m"),
			   const casacore::MFrequency::Types& freqFrame=casacore::MFrequency::LSRK,
			   const casacore::Bool trackSource=false, 
			   const casacore::MDirection& trackDir=casacore::MDirection(casacore::Quantity(0.0, "deg"), casacore::Quantity(90.0, "deg")), 
			   const casacore::Bool overwrite=false,
			   const casacore::Float padding=1.0, 
			   const casacore::Bool useAutocorr=false, 
			   const bool useDoublePrec=true, 
			   const casacore::Int wprojplanes=1, 
			   const casacore::String convFunc="SF", 
			   const casacore::String startmodel="",
			   // The extra params for WB-AWP
			   const casacore::Bool aTermOn    = true,
			   const casacore::Bool psTermOn   = true,
			   const casacore::Bool mTermOn    = false,
			   const casacore::Bool wbAWP      = true,
			   const casacore::String cfCache  = "",
			   const casacore::Bool doPointing = false,
			   const casacore::Bool doPBCorr   = true,
			   const casacore::Bool conjBeams  = true,
			   const casacore::Float computePAStep=360.0,
			   const casacore::Float rotatePAStep=5.0
			   );
  //Define image via a predefine SIImageStore object
  virtual casacore::Bool defineImage(casacore::CountedPtr<SIImageStore> imstor, 
			   const casacore::String& ftmachine);
  //Defining componentlist to use while degriding
  //This should be called once...if multiple lists are used..they can be merged in one
  //if sdgrid=true then image plane degridding is done
  virtual void setComponentList(const ComponentList& cl, 
				casacore::Bool sdgrid=false);
  casacore::Bool weight(const casacore::String& type="natural", 
	      const casacore::String& rmode="norm",
	      const casacore::Quantity& noise=casacore::Quantity(0.0, "Jy"), 
	      const casacore::Double robust=0.0,
	      const casacore::Quantity& fieldofview=casacore::Quantity(0.0, "arcsec"),
	      const casacore::Int npixels=0, 
	      const casacore::Bool multiField=false,
	      const casacore::String& filtertype=casacore::String("Gaussian"),
	      const casacore::Quantity& filterbmaj=casacore::Quantity(0.0,"deg"),
	      const casacore::Quantity& filterbmin=casacore::Quantity(0.0,"deg"),
	      const casacore::Quantity& filterbpa=casacore::Quantity(0.0,"deg")  );

  //the following get rid of the mappers in this object
  void resetMappers();

  casacore::CountedPtr<SIImageStore> imageStore(const casacore::Int id=0);

  //casacore::Record getMajorCycleControls();
  void executeMajorCycle(casacore::Record& controls);

  // make the psf images  i.e grid weight rather than data
  void makePSF();

  void predictModel();
  /* Access method to the Loop Controller held in this class */
  //SIIterBot& getLoopControls();

 

protected:
 
  /////////////// Internal Functions

  // Choose between different types of FTMs
  void createFTMachine(casacore::CountedPtr<refim::FTMachine>& theFT, 
		       casacore::CountedPtr<refim::FTMachine>& theIFT,  
		       const casacore::String& ftname,
		       const casacore::uInt nTaylorTerms=1, 
		       const casacore::String mType="default",
		       const casacore::Int facets=1,
		       //------------------------------
		       const casacore::Int wprojplane=1,
		       const casacore::Float padding=1.0,
		       const casacore::Bool useAutocorr=false,
		       const casacore::Bool useDoublePrec=true,
		       const casacore::String gridFunction=casacore::String("SF"),
		       //------------------------------
		       const casacore::Bool aTermOn    = true,
		       const casacore::Bool psTermOn   = true,
		       const casacore::Bool mTermOn    = false,
		       const casacore::Bool wbAWP      = true,
		       const casacore::String cfCache  = "",
		       const casacore::Bool doPointing = false,
		       const casacore::Bool doPBCorr   = true,
		       const casacore::Bool conjBeams  = true,
		       const casacore::Float computePAStep   = 360.0,
		       const casacore::Float rotatePAStep    = 5.0,
		       const casacore::String interpolation = casacore::String("linear"),
		       const casacore::Bool freqFrameValid = true,
		       const casacore::Int cache=1000000000,
		       const casacore::Int tile=16);

  // Choose between different types of ImageStore types (single term, multiterm, faceted)
  casacore::CountedPtr<SIImageStore> createIMStore(casacore::String imageName, 
					 casacore::CoordinateSystem& cSys,
					 casacore::IPosition imShape, 
					 const casacore::Bool overwrite,
					 casacore::String mappertype="default", 
					 casacore::uInt ntaylorterms=1,
					 casacore::Quantity distance=casacore::Quantity(0.0, "m"),
					 casacore::uInt facets=1,
					 casacore::Bool useweightimage=false);
  
  // Choose between different types of Mappers (single term, multiterm, imagemosaic, faceted)
  casacore::CountedPtr<SIMapper> createSIMapper(casacore::String mappertype,  
					  casacore::CountedPtr<SIImageStore> imagestore, //// make this inside !!!!!
				      casacore::CountedPtr<refim::FTMachine> ftmachine,
				      casacore::CountedPtr<refim::FTMachine> iftmachine,
					  casacore::uInt ntaylorterms=1);

  casacore::Block<casacore::CountedPtr<SIImageStore> > createFacetImageStoreList(
							     casacore::CountedPtr<SIImageStore> imagestore,
							     casacore::Int facets);
  void setPsfFromOneFacet();

  casacore::Bool toUseWeightImage(casacore::CountedPtr<refim::FTMachine>& ftm, casacore::String mappertype);

  void createVisSet(const casacore::Bool writeaccess=false);
  
  void createAWPFTMachine(casacore::CountedPtr<FTMachine>& theFT, casacore::CountedPtr<FTMachine>& theIFT, 
			  const casacore::String& ftmName,
			  const casacore::Int facets,          
			  //----------------------------
			  const casacore::Int wprojPlane,     
			  const casacore::Float padding,      
			  const casacore::Bool useAutocorr,   
			  const casacore::Bool useDoublePrec, 
			  const casacore::String gridFunction,
			  //---------------------------
			  const casacore::Bool aTermOn,      
			  const casacore::Bool psTermOn,     
			  const casacore::Bool mTermOn,      
			  const casacore::Bool wbAWP,        
			  const casacore::String cfCache,    
			  const casacore::Bool doPointing,   
			  const casacore::Bool doPBCorr,     
			  const casacore::Bool conjBeams,    
			  const casacore::Float computePAStep,
			  const casacore::Float rotatePAStep, 
			  const casacore::Int cache,          
			  const casacore::Int tile);
  refim::ATerm* createTelescopeATerm(const casacore::MeasurementSet& ms, const casacore::Bool& isATermOn);

  void runMajorCycle(const casacore::Bool dopsf=false, const casacore::Bool savemodel=false);

  /////This function should be called at every define image
  /////It associated the ftmachine with a given field
  ////For facetted image distinct  ft machines will associated with each facets and 
  //// Only one facetted image allowed
  //  void appendToMapperList(casacore::String imagename, casacore::CoordinateSystem& csys, casacore::String ftmachine,
  //		  	  casacore::Quantity distance=casacore::Quantity(0.0, "m"), casacore::Int facets=1, const casacore::Bool overwrite=false);

  void appendToMapperList(casacore::String imagename, 
			  casacore::CoordinateSystem& csys, 
			  casacore::IPosition imshape,
			  casacore::CountedPtr<refim::FTMachine>& ftm,
			  casacore::CountedPtr<refim::FTMachine>& iftm,
		  	  casacore::Quantity distance=casacore::Quantity(0.0, "m"), 
			  casacore::Int facets=1, 
			  const casacore::Bool overwrite=false,
			  casacore::String mappertype=casacore::String("default"),
			  casacore::uInt ntaylorterms=1);
  /////////////// Member Objects

  SIMapperCollection itsMappers;
  ///if facetting this storage will keep the unsliced version 
  casacore::CountedPtr<SIImageStore> unFacettedImStore_p;


  casacore::CoordinateSystem itsMaxCoordSys;
  casacore::IPosition itsMaxShape;


  /////////////// All input parameters

  // casacore::Data Selection
  // Image Definition
  // Imaging/Gridding

  // Other Options
  casacore::Block<const casacore::MeasurementSet *> mss_p;
  vi::FrequencySelections fselections_p;
  casacore::CountedPtr<vi::VisibilityIterator2>  vi_p;

  /////////////////////////////////////////////////////////////////////////////////
  casacore::Bool writeAccess_p;
  casacore::MPosition mLocation_p;
  casacore::MDirection phaseCenter_p;
  casacore::Int facetsStore_p;
  VisImagingWeight imwgt_p;
  casacore::Bool imageDefined_p;
  casacore::Bool useScratch_p,readOnly_p;
  //
  //  casacore::Bool freqFrameValid_p;

  refim::FTMachine::Type datacol_p;

};

  }//# NAMESPACE refim - END
} //# NAMESPACE CASA - END

#endif
