//# DOnewsimulator: defines classes for simulator DO.
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002
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
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id$

#ifndef SYNTHESIS_SIMULATOR_H
#define SYNTHESIS_SIMULATOR_H

#include <casa/aips.h>
#include <casa/Quanta/Quantum.h>
#include <measures/Measures/MPosition.h>
#include <synthesis/MeasurementComponents/BeamSquint.h>
#include <synthesis/MeasurementComponents/VPSkyJones.h>
#include <synthesis/MeasurementEquations/VisEquation.h>
//#include <synthesis/MeasurementComponents/EPJones.h>

#include <casa/namespace.h>
namespace casa { //# NAMESPACE CASA - BEGIN
class MeasurementSet;
class VisSet;
class VisCal;
class ACoh;
class SkyEquation;
class ComponentList;
class CleanImageSkyModel;
class FTMachine;
class ComponentFTMachine;
class MEpoch;
class NewMSSimulator;


template<class T> class PagedImage;



// <summary>Simulates MeasurementSets from SkyModel and SkyEquation</summary>


// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> <linkto class="MeasurementSet">MeasurementSet</linkto>
//   <li> <linkto class="SkyEquation">SkyEquation</linkto>
//   <li> <linkto class="SkyModel">SkyModel</linkto>
// </prerequisite>
//
// <etymology>
// The name MUST have the 'DO' prefix as this class is derived from
// ApplicationObject, and hence is classified as a distributed object. For the
// same reason the rest of its name must be in lower case. This class 
// simulates visibility data, so it is called DOnewsimulator.
// </etymology>
//
// <synopsis>
// </synopsis>
//
// <motivation> 
// This class was written to make the simulation capability useful from glish
// </motivation>
//
// <thrown>
// <li> AipsError - if an error occurs
// </thrown>
//
// <todo asof="1999/07/22">
//   <li> everything
// </todo>

class Simulator 
{

public:
 
  Simulator();
  // Construct from string
  Simulator(String& msname);

  // "newsimulator" ctor
  Simulator(MeasurementSet &thems);
  
  // Return the name of the MeasurementSet
  String name() const;
  
  Simulator(const Simulator &other);

  Simulator &operator=(const Simulator &other);
 
  ~Simulator();
  
  Bool close();


  // Select the data to be predicted or corrupted
  Bool setdata(const Vector<Int>& spectralwindowids,
	       const Vector<Int>& fieldids,
	       const String& msSelect);
  
  Bool settimes(const Quantity& integrationTime, 
		const Bool      useHourAngle,
		const MEpoch&   refTime);

  // set the configuration; NOTE: the telname used
  // here will determine the Voltage Pattern to be used in the
  // simulation
  Bool setconfig(const String& telname,
		 const Vector<Double>& x, 
		 const Vector<Double>& y, 
		 const Vector<Double>& z,
		 const Vector<Double>& dishDiameter,
		 const Vector<Double>& offset,
		 const Vector<String>& mount,
		 const Vector<String>& antName,
		 const Vector<String>& padName,
		 const String& coordsystem,
		 const MPosition& referenceLocation);
  // get info back from e.g. loaded ms in newmssimulator
  Bool getconfig();

  // set the observed fields for the simulation
  Bool setfield(const String& sourceName,           
		const MDirection& sourceDirection,  
		const String& calCode,
		const Quantity& distance);


  // set automatic fields for a mosaic
  Bool setmosaicfield(const String& sourcename, const String& calcode, 
		      const MDirection& fieldcenter, 
		      const Int xmosp, const Int ymosp, 
		      const Quantity& mosspacing, const Quantity& distance);


  // set one or more spectral windows and their characteristics
  Bool setspwindow(const String& spwName,
		   const Quantity& freq,
		   const Quantity& deltafreq,
		   const Quantity& freqresolution,
		   const Int nchannels,
		   const String& stokes);

  // Set the simulated feed characteristics
  Bool setfeed(const String& mode,
	       const Vector<Double>& x,
	       const Vector<Double>& y,
	       const Vector<String>& pol);		

  // Set the voltage pattern
  Bool setvp(const Bool dovp,
             const Bool defaultVP,
             const String& vpTable,
             const Bool doSquint,
             const Quantity &parAngleInc,
	     const Quantity &skyPosThreshold,
	     const Float &pbLimit);

  // Set the random number generator seed for the addition of errors
  Bool setseed(const Int seed);

  // Arrange to corrupt with existing calibration
  //   (cf Calibrater setapply)
  Bool setapply (const String& type,
                 const Double& t,
                 const String& table,
                 const String& spw,
                 const String& field,
                 const String& interp,
                 const Bool& calwt,
                 const Vector<Int>& spwmap,
                 const Float& opacity);

  // Apply antenna-based gain errors
  Bool setgain(const String& mode, 
	       const String& table,
	       const Quantity& interval,
	       const Vector<Double>& amplitude);

  Bool settrop(const String& mode, 
	       const String& table,
	       const Float pwv,
	       const Float deltapwv,
	       const Float beta,
	       const Float windspeed);

  // Apply antenna pointing and squint errors
  Bool setpointingerror(const String& epJTableName,
			const Bool applyPointingOffsets,
			const Bool doPBCorrection);

  // Apply polarization leakage errors
  Bool setleakage(const String& mode, const String& table,
		  //const Quantity& interval, 
		  const Vector<Double>& amplitude,
		  const Vector<Double>& offset);

  // Apply bandpass errors
  Bool setbandpass(const String& mode, const String& table,
		   const Quantity& interval, const Vector<Double>& amplitude);

  // Simulate the parallactic angle phase effect
  Bool setpa(const String& mode, const String& table,
	     const Quantity& interval);

  // Simulate quasi-realistic thermal noise, which can depend upon
  // elevation, bandwidth, antenna diameter, as expected
  Bool oldsetnoise(const String& mode, 
		   const String& table,
		   const Quantity& simplenoise,
		   const Float antefficiency,
		   const Float correfficiency,
		   const Float spillefficiency,
		   const Float tau,
		   const Float trx,
		   const Float tatmos, 
		   const Float tcmb);

  Bool setnoise(const String& mode, 
		const String& caltable,			 
		const Quantity& simplenoise,
		// if blank, not stored
		// or ATM calculation
		const Quantity& pground,
		const Float relhum,
		const Quantity& altitude,
		const Quantity& waterheight,
		const Quantity& pwv,
		// user-specified tau and tatmos 
		const Float tatmos, 
		const Float tau,
		//
		const Float antefficiency,
		const Float spillefficiency,
		const Float correfficiency,
		const Float trx, 
		const Float tground,		
		const Float tcmb, 
		const Bool OTF);

  // apply errors to the data in our MS
  Bool corrupt();

  // Set limits
  Bool setlimits(const Double shadowFraction,
		 const Quantity& elevationLimit);

  // Set autocorrelation weight
  Bool setauto(const Double autocorrwt);


  // add new visibilities as described by the set methods to an existing
  // or just created measurement set
  Bool observe(const String& sourcename, const String& spwname,
	       const Quantity& startTime, 
	       const Quantity& stopTime,
	       const Bool add_observation,
	       const Bool state_sig,
	       const Bool state_ref,
	       const double& state_cal,
	       const double& state_load,
	       const unsigned int state_sub_scan,
	       const String& state_obs_mode);


  Bool observemany(const Vector<String>& sourcenames, const String& spwname,
		   const Vector<Quantity>& startTimes, 
		   const Vector<Quantity>& stopTimes,
		   const Vector<MDirection>& directions,
		   const Bool add_observation,
		   const Bool state_sig,
		   const Bool state_ref,
		   const double& state_cal,
		   const double& state_load,
		   const unsigned int state_sub_scan,
		   const String& state_obs_mode);

  // Given a model image, predict the visibilities onto the (u,v) coordinates
  // of our MS
  Bool predict(const Vector<String>& modelImage, 
	       const String& compList, 
	       const Bool incremental);

  String state();

  Bool summary();

  Bool resetviscal();
  Bool resetimcal();
  Bool reset();

  // Set the processing options
  Bool setoptions(const String& ftmachine, const Int cache, const Int tile,
		  const String& gridfunction, const MPosition& mLocation,
		  const Float padding, const Int facets,
		  const Double maxData,const Int wprojPlanes);

 
  // Set the print level
  inline void setPrtlev(const Int& prtlev) { prtlev_=prtlev; };
  // Return print (cout) level
  inline Int& prtlev() { return prtlev_; };

  
private:

  
  // Arrange to corrupt with simulated calibration
  //   (cf Calibrater setapply)
  Bool create_corrupt(Record& simpar);

  // Prints an error message if the newsimulator DO is detached and returns True.
  Bool detached() const;

  // set up some defaults
  void defaults();

  // Make a VisSet if needed
  void makeVisSet();

  // print out some help about create()
  // Format direction nicely
  String formatDirection(const MDirection& direction);

  // Format time nicely
  String formatTime(const Double time);
  
  // individual summary() functions
  // <group>
  Bool createSummary(LogIO& os);
  Bool configSummary(LogIO& os);
  Bool fieldSummary(LogIO& os);
  Bool spWindowSummary(LogIO& os);
  Bool feedSummary(LogIO& os);
  Bool timeSummary(LogIO& os);

  Bool predictSummary(LogIO& os);
  Bool vpSummary(LogIO& os);
  Bool optionsSummary(LogIO& os);

  Bool corruptSummary(LogIO& os);
  Bool noiseSummary(LogIO& os);
  // </group>


  // SkyEquation management
  // <group>
  Bool createSkyEquation( const Vector<String>& image, const String complist);
  void destroySkyEquation();
  // </group>

  String msname_p;
  MeasurementSet* ms_p;
  MeasurementSet* mssel_p;
  VisSet* vs_p;

  Int seed_p;

  ACoh     *ac_p;

  SkyEquation* se_p;
  CleanImageSkyModel* sm_p;
  FTMachine *ft_p;
  ComponentFTMachine *cft_p;

  Int nmodels_p;
  PtrBlock<PagedImage<Float>* > images_p;
  ComponentList *componentList_p;

  String ftmachine_p, gridfunction_p;
  Int cache_p, tile_p;
  MPosition mLocation_p;
  Float padding_p;
  Bool MSMayBeOK;
  Int facets_p;
  Int wprojPlanes_p;
  Long maxData_p;


  // info for coordinates and station locations
  // <group>
  Bool           areStationCoordsSet_p;
  String         telescope_p;
  Vector<Double> x_p;
  Vector<Double> y_p;
  Vector<Double> z_p;
  Vector<Double>  diam_p;
  Vector<Double>  offset_p;
  Vector<String> mount_p;
  Vector<String> antName_p;
  Vector<String> padName_p;
  String         coordsystem_p;
  MPosition      mRefLocation_p;
  // </group>

  // info for observed field parameters
  // <group>

  Int nField;
  Vector<String> 	sourceName_p;
  Vector<String>        calCode_p;
  Vector<MDirection>	sourceDirection_p;
  Vector<Quantity>      distance_p;

  // </group>
  // VisEquation handles corruption by visibility calibration effects
  VisEquation ve_p;
  // Generic container for any number of calibration effects to corrupt with
  PtrBlock<VisCal*> vc_p;

  // info for spectral window parameters
  // <group>

  // spectral windows data
  // <group>
  // RI 20091107 durn. whoever build this didn't enable multiple spw.
  // we'll at least get some functionality here, but there are probably
  // combinations of pols etc that won't properly report here. 
  // better than now, when it doesn't even report more than one spw.
  Int nSpw;
  Vector<String> 	spWindowName_p; 
  Vector<Int>		nChan_p;
  Vector<Quantity>     	startFreq_p;
  Vector<Quantity>     	freqInc_p;
  Vector<Quantity>     	freqRes_p;
  Vector<String>     	stokesString_p;   
  // </group>
  // </group>


  // Feed information (there will be much more coming,
  // but we are brain dead at this moment).
  String feedMode_p;
  Int nFeeds_p;
  Bool feedsHaveBeenSet;
  Bool feedsInitialized;

  // Some times which are required for settimes
  // <group>
  Quantity integrationTime_p;
  Bool     useHourAngle_p;
  MEpoch   refTime_p;
  Bool timesHaveBeenSet_p;
  // </group>

  // Some parameters for voltage pattern (vp):
  // <group>
  Bool doVP_p;			// Do we apply VP or not?
  Bool doDefaultVP_p;		// Do we use the default VP for this telescope?
  String vpTableStr_p;		// Otherwise, use the VP specified in this Table
  Quantity  parAngleInc_p;	// Parallactic Angle increment
  Quantity  skyPosThreshold_p;  // a tolerance in the pointing center position
  Float pbLimit_p;              // The PB level (in percentage) after which the PB is assumed to be zero
  BeamSquint::SquintType  squintType_p;	// Control of squint to use
  VPSkyJones* vp_p;		// pointer to VPSkyJones for the sky equation
  VPSkyJones* gvp_p;		// pointer to VPSkyJones for the sky equation
  // </group>

  // Saving some information about the various corrupting terms
  // <group>
  String noisemode_p;
  // </group>

  // Cache the newsimulator
  NewMSSimulator* sim_p;

  // The Jones matrix to hold the antenna pointing offsets and the
  // associated table name.  if applyPointingOffsets is False, only
  // VLA polarization squint will be included in EPJones.  If
  // doPBCorrection is True, the model image will be divided by the
  // primary beam before being used to predict the visibilities.
  // <group>
  //  EPJones *epJ_p;
  String epJTableName_p;
  Bool applyPointingOffsets_p;
  Bool doPBCorrection_p;
  // </group>
  
  Int prtlev_;

};

} //# NAMESPACE CASA - END

#endif
