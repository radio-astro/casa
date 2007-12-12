//# Imager.h: Imager functionality sits here; 
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

#ifndef SYNTHESIS_IMAGER_H
#define SYNTHESIS_IMAGER_H

#include <casa/aips.h>
#include <casa/OS/Timer.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Quanta/Quantum.h>

#include <measures/Measures/MDirection.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MRadialVelocity.h>

#include <synthesis/MeasurementComponents/CleanImageSkyModel.h>
#include <synthesis/MeasurementComponents/BeamSquint.h>
#include <synthesis/MeasurementComponents/WFCleanImageSkyModel.h>
#include <synthesis/MeasurementComponents/ClarkCleanImageSkyModel.h>
#include <synthesis/MeasurementEquations/SkyEquation.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Forward declarations
class VisSet;
class MSHistoryHandler;
class PBMath;
class MeasurementSet;
class MFrequency;
class File;
class VPSkyJones;
class PGPlotter;
class EPJones;

template<class T> class ImageInterface;

// <summary> Class that contains functions needed for imager </summary>


class Imager 
{
 public:
  // Default constructor

  Imager();

  Imager(MeasurementSet& ms, Bool compress=False);
  Imager(MeasurementSet& ms, PGPlotter& pgplotter, Bool compress=False);

  // Copy constructor and assignment operator
  Imager(const Imager&);
  Imager& operator=(const Imager&);

  // Destructor
  virtual ~Imager();
  
  // open all the subtables as userNoReadLock
  virtual Bool openSubTables();


  // Lock the ms and its subtables
  virtual Bool lock();

  // Unlock the ms and its subtables
  virtual Bool unlock();


  // Utility function to do channel selection

  Bool selectDataChannel(VisSet& vs, Vector<Int>& spectralwindowids, 
				 String& dataMode, 
				 Vector<Int>& dataNchan, 
				 Vector<Int>& dataStart, Vector<Int>& dataStep,
				 MRadialVelocity& mDataStart, 
				 MRadialVelocity& mDataStep);
  //Utility function to check coordinate match with existing image

  virtual Bool checkCoord(CoordinateSystem& coordsys, 
			  String& imageName); 

  virtual void setImageParam(Int& nx, Int& ny, Int& npol, Int& nchan); 

  //VisSet and resort 
  virtual void makeVisSet(VisSet* & vs, MeasurementSet& ms, 
			  Bool compress=False, Bool mosaicOrder=False);
  //Just to create the SORTED_TABLE
  virtual void makeVisSet(MeasurementSet& ms, 
			  Bool compress=False, Bool mosaicOrder=False);

  virtual void writeHistory(LogIO& os);

  virtual void writeCommand(LogIO& os);

  //make an empty image
  Bool makeEmptyImage(CoordinateSystem& imageCoord, String& name, Int fieldID=0);

  //Functions to make Primary beams 
  Bool makePBImage(ImageInterface<Float>& pbImage, 
		   Bool useSymmetricBeam=True);
  Bool makePBImage(const CoordinateSystem& imageCoord, 
		   const String& telescopeName, const String& diskPBName, 
		   Bool useSymmetricBeam=True);
  
  Bool makePBImage(const CoordinateSystem& imageCoord, 
		   const Table& vpTable, const String& diskPBName);
  
  Bool makePBImage(const Table& vpTable, ImageInterface<Float>& pbImage);
  
  Bool makePBImage(const CoordinateSystem& imageCoord, PBMath& pbMath, const String& diskPBName);
  
  Bool makePBImage(PBMath& pbMath, ImageInterface<Float>& pbImage);
  
  void setObsInfo(ObsInfo& obsinfo);
  ObsInfo& latestObsInfo();
// Close the current ms, and replace it with the supplied ms.
  // Optionally compress the attached calibration data
  // columns if they are created here.
  Bool open(MeasurementSet &thems, Bool compress=False);
  
  // Flush the ms to disk and detach from the ms file. All function
  // calls after this will be a no-op.
  Bool close();
  
  // Return the name of the MeasurementSet
  String name() const;
  
  // The following setup methods define the state of the imager.
  // <group>
  // Set image construction parameters
  virtual Bool setimage(const Int nx, const Int ny,
		const Quantity& cellx, const Quantity& celly,
		const String& stokes,
                Bool doShift,
		const MDirection& phaseCenter, 
                const Quantity& shiftx, const Quantity& shifty,
		const String& mode, const Int nchan,
                const Int start, const Int step,
		const MRadialVelocity& mStart, const MRadialVelocity& mStep,
		const Vector<Int>& spectralwindowids, const Int fieldid,
		const Int facets, const Quantity& distance);

  virtual Bool defineImage(const Int nx, const Int ny,
			   const Quantity& cellx, const Quantity& celly,
			   const String& stokes,
			   const MDirection& phaseCenter, 
			   const Int fieldid,
			   const String& mode, const Int nchan,
			   const Int start, const Int step,
			   const MFrequency& mFreqStart,
			   const MRadialVelocity& mStart, 
			   const Quantity& qStep,
			   const Vector<Int>& spectralwindowids, 
			   const Quantity& restFreq,
			   const Int facets, const Quantity& distance);
  // Set the data selection parameters
 
  virtual  Bool setDataPerMS(const String& msname, const String& mode, 
			     const Vector<Int>& nchan, 
			     const Vector<Int>& start,
			     const Vector<Int>& step,
			     const Vector<Int>& spectralwindowids,
			     const Vector<Int>& fieldid,
			     const String& msSelect="",
                             const String& timerng="",
			     const String& fieldnames="",
			     const Vector<Int>& antIndex=Vector<Int>(),
			     const String& antnames="",
			     const String& spwstring="",
			     const String& uvdist="",
			     const String& scan="");


  Bool setdata(const String& mode, const Vector<Int>& nchan, 
	       const Vector<Int>& start,
	       const Vector<Int>& step, const MRadialVelocity& mStart,
	       const MRadialVelocity& mStep,
	       const Vector<Int>& spectralwindowids,
	       const Vector<Int>& fieldid,
	       const String& msSelect="",
	       const String& timerng="",
	       const String& fieldnames="",
	       const Vector<Int>& antIndex=Vector<Int>(),
	       const String& antnames="",
	       const String& spwstring="",
	       const String& uvdist="",
	       const String& scan="");
  
  // Set the processing options
  Bool setoptions(const String& ftmachine, const Long cache, const Int tile,
		  const String& gridfunction, const MPosition& mLocation,
		  const Float padding, const Bool usemodelcol=True, 
		  const Int wprojplanes=1,
		  const String& epJTableName="",
		  const Bool applyPointingOffsets=True,
		  const Bool doPointingCorrection=True,
		  const String& cfCacheDirName="", 
		  const Float& pastep=5.0,
		  const Float& pbLimit=5.0e-2);

  // Set the single dish processing options
  Bool setsdoptions(const Float scale, const Float weight, 
		    const Int convsupport=-1);

  // Set the voltage pattern
  Bool setvp(const Bool dovp,
	     const Bool defaultVP,
	     const String& vpTable,
	     const Bool doSquint,
	     const Quantity &parAngleInc,
	     const Quantity &skyPosThreshold,
	     String defaultTel="");

  // Set the scales to be searched in Multi Scale clean
  Bool setscales(const String& scaleMethod,          // "nscales"  or  "uservector"
		 const Int inscales,
		 const Vector<Float>& userScaleSizes);
  // Set the number of taylor series terms in the expansion of the
  // image as a function of frequency.
  Bool settaylorterms(const Int intaylor);

  // </group>
  
  // Advise on suitable values
  Bool advise(const Bool takeAdvice, const Float amplitudeloss,
              const Quantity& fieldOfView,
	      Quantity& cell, Int& npixels, Int& facets,
	      MDirection& phaseCenter);

  // Output a summary of the state of the object
  Bool summary();
  
  // Return the state of the object as a string
  String state();
  
  // Return the image coordinates
  Bool imagecoordinates(CoordinateSystem& coordInfo);

  // Return the image shape
  IPosition imageshape() const;

  // Weight the MeasurementSet
  Bool weight(const String& algorithm, const String& rmode,
	      const Quantity& noise, const Double robust,
              const Quantity& fieldofview, const Int npixels);
  
  // Filter the MeasurementSet
  Bool filter(const String& type, const Quantity& bmaj, const Quantity& bmin,
	      const Quantity& bpa);
  
  // Apply a uvrange
  Bool uvrange(const Double& uvmin, const Double& uvmax);
  
  // Sensitivity
  Bool sensitivity(Quantity& pointsourcesens, Double& relativesens, Double& sumwt);
  
  // Make plain image
  Bool makeimage(const String& type, const String& imageName);

  // Make plain image: keep the complex image as well
  Bool makeimage(const String& type, const String& imageName,
	     const String& complexImageName);
  
  // Fill in a region of a mask
  Bool boxmask(const String& mask, const Vector<Int>& blc,
	       const Vector<Int>& trc, const Float value);

  //Make a region either from record or array of blc trc 
  //(Matrix(nboxes,4)) into a mask image
  //value is the value of the mask pixels
  Bool regionmask(const String& maskimage, Record* imageRegRec, 
		  Matrix<Quantity>& blctrcs, const Float& value=1.0);
  // Clip on Stokes I
  Bool clipimage(const String& image, const Quantity& threshold);

  // Make a mask image
  Bool mask(const String& mask, const String& imageName,
	    const Quantity& threshold);
  
  // Restore
  Bool restore(const Vector<String>& model, const String& complist,
	       const Vector<String>& image, const Vector<String>& residual);

  // Setbeam
  Bool setbeam(const Quantity& bmaj, const Quantity& bmin, const Quantity& bpa);

  // Residual
  Bool residual(const Vector<String>& model, const String& complist,
	       const Vector<String>& image);

  // Approximate PSF
  Bool approximatepsf(const String& psf);

  // Smooth
  Bool smooth(const Vector<String>& model, 
	      const Vector<String>& image, Bool usefit,
	      Quantity& bmaj, Quantity& bmin, Quantity& bpa,
	      Bool normalizeVolume);

  // Clean algorithm
  Bool clean(const String& algorithm,
	     const Int niter, 
	     const Float gain, 
	     const Quantity& threshold, 
	     const Bool displayProgress, 
	     const Vector<String>& model, const Vector<Bool>& fixed,
	     const String& complist,
	     const Vector<String>& mask,
	     const Vector<String>& restored,
	     const Vector<String>& residual);
  
  // MEM algorithm
  Bool mem(const String& algorithm,
	   const Int niter, const Quantity& sigma, 
	   const Quantity& targetflux,
	   const Bool constrainflux,
	   const Bool displayProgress, 
	   const Vector<String>& model, const Vector<Bool>& fixed,
	   const String& complist,
	   const Vector<String>& prior,
	   const Vector<String>& mask,
	   const Vector<String>& restored,
	   const Vector<String>& residual);
  
  // pixon algorithm
  Bool pixon(const String& algorithm,
	     const Quantity& sigma, 
	     const String& model);
  
  // NNLS algorithm
  Bool nnls(const String& algorithm, const Int niter, const Float tolerance,
	    const Vector<String>& model, const Vector<Bool>& fixed,
	    const String& complist,
	    const Vector<String>& fluxMask, const Vector<String>& dataMask,
	    const Vector<String>& restored,
	    const Vector<String>& residual);

  // Multi-field control parameters
  Bool setmfcontrol(const Float cyclefactor,
		    const Float cyclespeedup,
		    const Int stoplargenegatives, 
		    const Int stoppointmode,
		    const String& scaleType,
		    const Float  minPB,
		    const Float constPB,
		    const Vector<String>& fluxscale);
  
  // Feathering algorithm
  Bool feather(const String& image,
	       const String& highres,
	       const String& lowres,
	       const String& lowpsf);
  
  // Apply or correct for Primary Beam or Voltage Pattern
  Bool pb(const String& inimage,
	  const String& outimage,
	  const String& incomps,
	  const String& outcomps,
	  const String& operation,
	  const MDirection& pointngCenter,
	  const Quantity& pa,
	  const String& pborvp);

  // Make a linear mosaic of several images
  Bool linearmosaic(const String& mosaic,
		    const String& fluxscale,
		    const String& sensitivity,
		    const Vector<String>& images,
		    const Vector<Int>& fieldids);
  
  // Fourier transform the model and componentlist
  Bool ft(const Vector<String>& model, const String& complist,
	  Bool incremental=False);

  // Compute the model visibility using specified source flux densities
  Bool setjy(const Int fieldid, const Int spectralwindowid,
	     const Vector<Double>& fluxDensity, const String& standard);
  Bool setjy(const Vector<Int>& fieldid, const Vector<Int>& spectralwindowid, 
	     const String& fieldnames, const String& spwstring, 
	     const Vector<Double>& fluxDensity, const String& standard);

  Bool setjy(const Vector<Int>& fieldid, 
	     const Vector<Int>& spectralwindowid, 
	     const String& fieldnames, const String& spwstring, 
	     const String& model,
	     const Vector<Double>& fluxDensity, const String& standard);

  // Make an empty image
  Bool make(const String& model);

  // make a model from a SD image. 
  // This model then can be used as initial clean model to include the 
  // shorter spacing.
  Bool makemodelfromsd(const String& sdImage, const String& modelimage,
		       const String& lowPSF,
		       String& maskImage);

  // Clone an image
  Bool clone(const String& imageName, const String& newImageName);
  
  // Fit the psf
  Bool fitpsf(const String& psf, Quantity& mbmaj, Quantity& mbmin,
	      Quantity& mbpa);

  // Correct the visibility data (OBSERVED->CORRECTED)
  Bool correct(const Bool doparallactic, const Quantity& t);

  // Plot the uv plane
  Bool plotuv(const Bool rotate);

  // Plot the visibilities
  Bool plotvis(const String& type, const Int increment);

  // Plot the weights
  Bool plotweights(const Bool gridded, const Int increment);

  // Plot a summary
  Bool plotsummary();

  // Clip visibilities
  Bool clipvis(const Quantity& threshold);


  //This is necessary for setting from the DO...but once its set properly from 
  // the constructor...these two functions should go 

  PGPlotter& getPGPlotter();
  void setPGPlotter(PGPlotter& thePlotter);

  //Check if can proceed with this object
  Bool valid() const;


  //Interactive mask drawing
  Int interactivemask(const String& imagename, const String& maskname, 
		      Int& niter, Int& ncycles, String& threshold);

protected:

  MeasurementSet* ms_p;
  MSHistoryHandler *hist_p;
  Table antab_p;
  Table datadesctab_p;
  Table feedtab_p;
  Table fieldtab_p;
  Table obstab_p;
  Table pointingtab_p;
  Table poltab_p;
  Table proctab_p;
  Table spwtab_p;
  Table statetab_p;
  Table dopplertab_p;
  Table flagcmdtab_p;
  Table freqoffsettab_p;
  Table historytab_p;
  Table sourcetab_p;
  Table syscaltab_p;
  Table weathertab_p;
  Int lockCounter_p;
  Int nx_p, ny_p, npol_p, nchan_p;
  ObsInfo latestObsInfo_p;  
  



  String msname_p;
  MeasurementSet *mssel_p;
  VisSet *vs_p;
  FTMachine *ft_p;
  ComponentFTMachine *cft_p;
  SkyEquation* se_p;
  CleanImageSkyModel* sm_p;
  VPSkyJones* vp_p;
  VPSkyJones* gvp_p;

  Bool setimaged_p, nullSelect_p;
  Bool redoSkyModel_p;   // if clean is run multiply ..use this to check
                         // if setimage was changed hence redo the skyModel.
  Float paStep_p, pbLimit_p;
  Int facets_p;
  Int wprojPlanes_p;
  Quantity mcellx_p, mcelly_p;
  String stokes_p;
  String dataMode_p, imageMode_p;
  Vector<Int> dataNchan_p;
  Int imageNchan_p;
  Vector<Int> dataStart_p, dataStep_p;
  Int imageStart_p, imageStep_p;
  MRadialVelocity mDataStart_p, mImageStart_p;
  MRadialVelocity mDataStep_p,  mImageStep_p;
  MFrequency mfImageStart_p, mfImageStep_p;
  MDirection phaseCenter_p;
  Quantity restFreq_p;
  Quantity distance_p;
  Bool doShift_p;
  Quantity shiftx_p;
  Quantity shifty_p;
  String ftmachine_p, gridfunction_p;
  Bool wfGridding_p;
  Long cache_p;
  Int  tile_p;
  MPosition mLocation_p;
  Bool doVP_p;
  Quantity bmaj_p, bmin_p, bpa_p;
  Bool beamValid_p;
  Float padding_p;
  Float sdScale_p;
  Float sdWeight_p;
  Int sdConvSupport_p;
  // special mf control parms, etc
  Float cyclefactor_p;
  Float cyclespeedup_p;
  Int stoplargenegatives_p;
  Int stoppointmode_p;
  Vector<String> fluxscale_p;
  String scaleType_p;		// type of image-plane scaling: NONE, SAULT
  Float minPB_p;		// minimum value of generalized-PB pattern
  Float constPB_p;		// above this level, constant flux-scale

  Vector<Int> spectralwindowids_p;
  Int fieldid_p;

  Vector<Int> dataspectralwindowids_p;
  Vector<Int> datadescids_p;
  Vector<Int> datafieldids_p;

  String telescope_p;
  String vpTableStr_p;         // description of voltage patterns for various telescopes
                               //  in the MS
  Quantity parAngleInc_p;
  Quantity skyPosThreshold_p;
  BeamSquint::SquintType  squintType_p;
  Bool doDefaultVP_p;          // make default VPs, rather than reading in a vpTable

  PGPlotter* pgplotter_p;

  Bool  doMultiFields_p;      // Do multiple fields?
  Bool  multiFields_p; 	      // multiple fields have been specified in setdata

  Bool doWideBand_p;          // Do Multi Frequency Synthesis Imaging

  // Set the defaults
  void defaults();

  // check if it is  dettahced from ms.
  Bool detached() const;

  // Create the FTMachines when necessary or when the control parameters
  // have changed. 
  Bool createFTMachine();

  Bool removeTable(const String& tablename);

  Bool createSkyEquation(const String complist="");
  Bool createSkyEquation(const Vector<String>& image, 
			 const Vector<Bool>& fixed,
			 const String complist="");
  Bool createSkyEquation(const Vector<String>& image, 
			 const String complist="");
  Bool createSkyEquation(const Vector<String>& image, 
			 const Vector<Bool>& fixed,
			 const Vector<String>& mask,
			 const String complist="");
  Bool createSkyEquation(const Vector<String>& image, 
			 const Vector<Bool>& fixed,
			 const Vector<String>& mask,
			 const Vector<String>& fluxMask,
			 const String complist="");
  void destroySkyEquation();

  // Add the residuals to the SkyEquation
  Bool addResidualsToSkyEquation(const Vector<String>& residual);

  // Add or replace the masks
  Bool addMasksToSkyEquation(const Vector<String>& mask);

  // Get the rest frequency ..returns 1 element in restfreq 
  // if user specified or try to get the info from the SOURCE table 
  Bool getRestFreq(Vector<Double>& restFreq, const Int& spw);

  Bool restoreImages(const Vector<String>& restored);

  // names of flux scale images
  Bool writeFluxScales(const Vector<String>& fluxScaleNames);

  String imageName();

  Bool pbguts(ImageInterface<Float>& in,  
	      ImageInterface<Float>& out, 
	      const MDirection&,
	      const Quantity&);

  Bool assertDefinedImageParameters() const;
 // Virtual methods to set the ImageSkyModel and SkyEquation.
  // This allows derived class pimager to set parallelized
  // specializations.
  //
  virtual void setWFCleanImageSkyModel() 
    {sm_p = new WFCleanImageSkyModel(facets_p, wfGridding_p); return;}; 
    
  virtual void setClarkCleanImageSkyModel()
    {sm_p = new ClarkCleanImageSkyModel(); return;};
  virtual void setSkyEquation();
    

  String frmtTime(const Double time);

 
  ComponentList* componentList_p;

  String scaleMethod_p;   // "nscales"   or  "uservector"
  Int nscales_p;
  Int ntaylor_p;
  Vector<Float> userScaleSizes_p;
  Bool scaleInfoValid_p;  // This means that we have set the information, not the scale beams

  Int nmodels_p;
  // Everything here must be a real class since we make, handle and
  // destroy these.
  PtrBlock<PagedImage<Float>* > images_p;
  PtrBlock<PagedImage<Float>* > masks_p;
  PtrBlock<PagedImage<Float>* > fluxMasks_p;
  PtrBlock<PagedImage<Float>* > residuals_p;
  
  // Freq frame is good and valid conversions can be done (or not)
  Bool freqFrameValid_p;

  // Preferred complex polarization representation
  SkyModel::PolRep polRep_p;

  //Whether to use model column or use it in memory on the fly
  Bool useModelCol_p;

  //sink used to store history mainly
  LogSink logSink_p;


  //
  // Objects required for pointing correction (ftmachine=PBWProject)
  //
  EPJones *epJ;
  String epJTableName_p, cfCacheDirName_p;
  Bool doPointing, doPBCorr;




};


} //# NAMESPACE CASA - END

#endif














