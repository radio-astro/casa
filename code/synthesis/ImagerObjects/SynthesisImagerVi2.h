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

namespace casacore{

class MeasurementSet;
template<class T> class ImageInterface;
}

namespace casa { //# NAMESPACE CASA - BEGIN
class VisImagingWeight;
 class SynthesisImagerVi2  : public SynthesisImager
{

public:
  // Default constructor

  SynthesisImagerVi2();
  virtual ~SynthesisImagerVi2();
  virtual casacore::Bool selectData(const SynthesisParamsSelect& selpars);
  virtual casacore::Bool defineImage(SynthesisParamsImage& impars, const SynthesisParamsGrid& gridpars);
  virtual casacore::Bool weight(const casacore::String& type="natural", 
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
  
  casacore::Bool setWeightDensity();
  void predictModel();

  void dryGridding(const casacore::Vector<casacore::String>& cfList);
  void fillCFCache(const casacore::Vector<casacore::String>& cfList,
		   const casacore::String& ftmName,
		   const casacore::String& cfcPath,
		   const casacore::Bool& psTermOn,
		   const casacore::Bool& aTermOn,
		   const casacore::Bool& conjBeams);
  void reloadCFCache();

 protected:
  void appendToMapperList(casacore::String imagename, 
			  casacore::CoordinateSystem& csys, 
			  casacore::IPosition imshape,
			  casacore::CountedPtr<refim::FTMachine>& ftm,
			  casacore::CountedPtr<refim::FTMachine>& iftm,
		  	  casacore::Quantity distance=casacore::Quantity(0.0, "m"), 
			  casacore::Int facets=1, 
			  casacore::Int chanchunks=1,
			  const casacore::Bool overwrite=false,
			  casacore::String mappertype=casacore::String("default"),
			  casacore::Float padding=1.0,
			  casacore::uInt ntaylorterms=1,
			  casacore::Vector<casacore::String> startmodel=casacore::Vector<casacore::String>(0));
  virtual void unlockMSs();
  virtual void createVisSet(const casacore::Bool writeaccess=false);
  void createFTMachine(casacore::CountedPtr<casa::refim::FTMachine>& theFT, 
		       casacore::CountedPtr<casa::refim::FTMachine>& theIFT,  
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
		       const casacore::Int tile=16,
		       const casacore::String stokes="I",
		       const casacore::String imageNamePrefix="");

  void createAWPFTMachine(casacore::CountedPtr<refim::FTMachine>& theFT, casacore::CountedPtr<refim::FTMachine>& theIFT, 
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
			  const casacore::Int tile,
			  const casacore::String imageNamePrefix="");
 
// Do the major cycle
  virtual void runMajorCycle(const casacore::Bool dopsf=false, const casacore::Bool savemodel=false);

  // Version of major cycle code with mappers in a loop outside vi/vb.
  virtual void runMajorCycle2(const casacore::Bool dopsf=false, const casacore::Bool savemodel=false);
 
 void createMosFTMachine(casacore::CountedPtr<casa::refim::FTMachine>& theFT,
                         casacore::CountedPtr<casa::refim::FTMachine>&  theIFT,
                         const casacore::Float  padding,
                         const casacore::Bool useAutoCorr,
                         const casacore::Bool useDoublePrec,
                         const casacore::Float rotatePAStep,
                         const casacore::String Stokes="I");
  casacore::CountedPtr<SIMapper> createSIMapper(casacore::String mappertype,  
				      casacore::CountedPtr<SIImageStore> imagestore, //// make this inside !!!!!
				      casacore::CountedPtr<refim::FTMachine> ftmachine,
				      casacore::CountedPtr<refim::FTMachine> iftmachine,
				      casacore::uInt ntaylorterms=1);

  bool makePB();
  bool makePrimaryBeam(PBMath& pbMath);
  void  andFreqSelection(const casacore::Int msId, const casacore::Int spwId,  const casacore::Double freqBeg, const casacore::Double freqEnd, const casacore::MFrequency::Types frame);
  void tuneChunk(const casacore::Int gmap);
   // Other Options
  //casacore::Block<const casacore::MeasurementSet *> mss_p;
  casacore::CountedPtr<vi::VisibilityIterator2>  vi_p;
  casacore::CountedPtr<vi::FrequencySelections> fselections_p;
  std::vector<std::pair<casacore::Int, casacore::Double> >freqBegs_p;
  std::vector<std::pair<casacore::Int, casacore::Double> > freqEnds_p;
  std::vector<std::pair<casacore::Int, casacore::Double> > freqSpws_p;
  //	///temporary variable as we carry that for tunechunk
  casacore::MFrequency::Types selFreqFrame_p;
};
} //# NAMESPACE CASA - END

#endif

