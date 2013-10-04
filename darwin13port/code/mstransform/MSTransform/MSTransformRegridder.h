//# MSTransformRegridder.h: This file contains the interface definition of the MSTransformRegridder class.
//#
//#  CASA - Common Astronomy Software Applications (http://casa.nrao.edu/)
//#  Copyright (C) Associated Universities, Inc. Washington DC, USA 2011, All rights reserved.
//#  Copyright (C) European Southern Observatory, 2011, All rights reserved.
//#
//#  This library is free software; you can redistribute it and/or
//#  modify it under the terms of the GNU Lesser General Public
//#  License as published by the Free software Foundation; either
//#  version 2.1 of the License, or (at your option) any later version.
//#
//#  This library is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY, without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//#  Lesser General Public License for more details.
//#
//#  You should have received a copy of the GNU Lesser General Public
//#  License along with this library; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
//#  MA 02111-1307  USA
//# $Id: $

#ifndef MSTransformRegridder_H_
#define MSTransformRegridder_H_

// Logging
#include <casa/Logging/LogIO.h>

// Coordinate systems
#include <measures/Measures/MeasTable.h>

// MS handling
#include <ms/MeasurementSets.h>

// To use ostringstream
#include <iomanip>

// Enable record conversions
#include <casa/Quanta/QuantumHolder.h>


namespace casa { //# NAMESPACE CASA - BEGIN

//  MSTransformRegridder definition
class MSTransformRegridder
{

public:

	MSTransformRegridder();
	~MSTransformRegridder();

	// Make one spectral window from all SPWs given by the SPW Ids vector
	static Bool combineSpws(	LogIO& os,
								String msName,
								const Vector<Int>& spwids, // Vector<Int>(1,-1) means: use all SPWs
								Vector<Double>& newCHAN_FREQ, // Will return the grid of the resulting SPW
								Vector<Double>& newCHAN_WIDTH,
								Bool verbose = False);

	// A wrapper for regridChanBounds() which takes the user interface type re-gridding parameters
	// The ready-made grid is returned in newCHAN_FREQ and newCHAN_WIDTH
	static Bool calcChanFreqs(	LogIO& os,
								// Output
								Vector<Double>& newCHAN_FREQ,
								Vector<Double>& newCHAN_WIDTH,
								// Input (original grid)
								const Vector<Double>& oldCHAN_FREQ,
								const Vector<Double>& oldCHAN_WIDTH,
								// Re-gridding parameters
								const MDirection  phaseCenter,
								const MFrequency::Types theOldRefFrame,
								const MEpoch theObsTime,
								const MPosition mObsPos,
								const String& mode,
								const int nchan,
								const String& start,
								const String& width,
								const String& restfreq,
								const String& outframe,
								const String& veltype,
								const Bool verbose=False,
								// Additional radial velocity shift to apply, used e.g. when outframe=="SOURCE"
								const MRadialVelocity mRV=MRadialVelocity() );

	  // Helper function for handling the re-gridding parameter user input
	  static Bool convertGridPars(LogIO& os,
				      	  	  	  const String& mode,
				      	  	  	  const int nchan,
				      	  	  	  const String& start,
				      	  	  	  const String& width,
				      	  	  	  const String& interp,
				      	  	  	  const String& restfreq,
				      	  	  	  const String& outframe,
				      	  	  	  const String& veltype,
				      	  	  	  String& t_mode,
				      	  	  	  String& t_outframe,
				      	  	  	  String& t_regridQuantity,
				      	  	  	  Double& t_restfreq,
				      	  	  	  String& t_regridInterpMeth,
				      	  	  	  Double& t_cstart,
				      	  	  	  Double& t_bandwidth,
				      	  	  	  Double& t_cwidth,
				      	  	  	  Bool& t_centerIsStart,
				      	  	  	  Bool& t_startIsEnd,
				      	  	  	  Int& t_nchan,
				      	  	  	  Int& t_width,
				      	  	  	  Int& t_start);

	  // Calculate the final new channel boundaries from the re-regridding parameters and
	  // the old channel boundaries (already transformed to the desired reference frame).
	  // Returns False if input parameters were invalid and no useful boundaries could be created
	  static Bool regridChanBounds(Vector<Double>& newChanLoBound,
				       	   	   	   Vector<Double>& newChanHiBound,
				       	   	   	   const Double regridCenter,
				       	   	   	   const Double regridBandwidth,
				       	   	   	   const Double regridChanWidth,
				       	   	   	   const Double regridVeloRestfrq,
				       	   	   	   const String regridQuant,
				       	   	   	   const Vector<Double>& transNewXin,
				       	   	   	   const Vector<Double>& transCHAN_WIDTH,
				       	   	   	   String& message, // message to the user, especially in case of error
				       	   	   	   const Bool centerIsStart=False, // if true, the parameter regridCenter specifies the start
				       	   	   	   const Bool startIsEnd=False, // if true, and centerIsStart is true, regridCenter specifies the upper end in frequency
				       	   	   	   const Int nchan=0, // if != 0 : used instead of regridBandwidth, -1 means use all channels
				       	   	   	   const Int width=0, // if >0 and regridQuant=="freq": used instead of regridChanWidth
				       	   	   	   const Int start=-1); // if >=0 and regridQuant=="freq": used instead of regridCenter

	  // The following inline convenience methods for regridSpw bypass the whole CASA measure system
	  // because when they are used, they can assume that the frame stays the same and the units are OK
	  static lDouble vrad(const lDouble freq, const lDouble rest){ return (C::c * (1. - freq/rest)); };
	  static lDouble vopt(const lDouble freq, const lDouble rest){ return (C::c *(rest/freq - 1.)); };
	  static lDouble lambda(const lDouble freq){ return (C::c/freq); };
	  static lDouble freq_from_vrad(const lDouble vrad, const lDouble rest){ return (rest * (1. - vrad/C::c)); };
	  static lDouble freq_from_vopt(const lDouble vopt, const lDouble rest){ return (rest / (1. + vopt/C::c)); };
	  static lDouble freq_from_lambda(const lDouble lambda){ return (C::c/lambda); };
};

} //# NAMESPACE CASA - END

#endif /* MSTransformRegridder_H_ */
