//# GridCalc.cc: spectral regridding utilities
//# Copyright (C) 2010 by ESO (in the framework of the ALMA collaboration)
//# Copyright (C) 1996,1997,1998,1999,2000,2001
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
//# $Id: $

#include <xmlcasa/ms/GridCalc.h>
#include <msvis/MSVis/SubMS.h>
#include <casa/Logging/LogIO.h>
#include <casa/BasicSL/String.h>
#include <casa/sstream.h>
#include <casa/iomanip.h>
#include <xmlcasa/StdCasa/CasacSupport.h>

using namespace casa;

namespace casac { //# NAMESPACE CASAC - BEGIN

  Bool GridCalc::convertGridPars(LogIO& os,
				 const std::string& mode, 
				 const int nchan, 
				 const casac::variant& start, 
				 const casac::variant& width,
				 const std::string& interp, 
				 const casac::variant& restfreq, 
				 const std::string& outframe,
				 const std::string& veltype,
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
				 Int& t_start
				 )
  {
    Bool rstat(False);
    
    try {
      
      os << LogOrigin("GridCalc", "convertGridPars");
      
      t_mode = toCasaString(mode);
      t_restfreq = 0.; // rest frequency, convert to Hz
      if(!restfreq.toString().empty()){
	t_restfreq = casaQuantity(restfreq).getValue("Hz");
      }
      
      // Determine grid
      t_cstart = -9e99; // default value indicating that the original start of the SPW should be used
      t_bandwidth = -1.; // default value indicating that the original width of the SPW should be used
      t_cwidth = -1.; // default value indicating that the original channel width of the SPW should be used
      t_nchan = -1; 
      t_width = 0;
      t_start = -1;
      t_startIsEnd = False; // False means that start specifies the lower end in frequency (default)
      // True means that start specifies the upper end in frequency

      if(!start.toString().empty()){ // start was set
	if(t_mode == "channel"){
	  t_start = atoi(start.toString().c_str());
	}
	if(t_mode == "channel_b"){
	  t_cstart = Double(atoi(start.toString().c_str()));
	}
	else if(t_mode == "frequency"){
	  t_cstart = casaQuantity(start).getValue("Hz");
	}
	else if(t_mode == "velocity"){
	  t_cstart = casaQuantity(start).getValue("m/s");
	}
      }
      if(!width.toString().empty()){ // channel width was set
	if(t_mode == "channel"){
	  Int w = atoi(width.toString().c_str());
	  t_width = abs(w);
	  if(w<0){
	    t_startIsEnd = True;
	  }
	}
	else if(t_mode == "channel_b"){
	  Double w = atoi(width.toString().c_str());
	  t_cwidth = abs(w);
	  if(w<0){
	    t_startIsEnd = True;
	  }	
	}
	else if(t_mode == "frequency"){
	  Double w = casaQuantity(width).getValue("Hz");
	  t_cwidth = abs(w);
	  if(w<0){
	    t_startIsEnd = True;
	  }	
	}
	else if(t_mode == "velocity"){
	  Double w = casaQuantity(width).getValue("m/s");
	  t_cwidth = abs(w);
	  if(w>=0){
	    t_startIsEnd = True; 
	  }		
	}
      }
      if(nchan > 0){ // number of output channels was set
	if(t_mode == "channel_b"){
	  if(t_cwidth>0){
	    t_bandwidth = Double(nchan*t_cwidth);
	  }
	  else{
	    t_bandwidth = Double(nchan);	  
	  }
	}
	else{
	  t_nchan = nchan;
	}
      }
      
      if(t_mode == "channel"){
	t_regridQuantity = "freq";
      }
      else if(t_mode == "channel_b"){
	t_regridQuantity = "chan";
      }
      else if(t_mode == "frequency"){
	t_regridQuantity = "freq";
      }
      else if(t_mode == "velocity"){
	if(t_restfreq == 0.){
	  os << LogIO::SEVERE << "Need to set restfreq in velocity mode." << LogIO::POST; 
	  return False;
	}	
	t_regridQuantity = "vrad";
	if(veltype == "optical"){
	  t_regridQuantity = "vopt";
	}
	else if(veltype != "radio"){
	  os << LogIO::WARN << "Invalid velocity type "<< veltype 
	     << ", setting type to \"radio\"" << LogIO::POST; 
	}
      }   
      else{
	os << LogIO::WARN << "Invalid mode " << t_mode << LogIO::POST;
	return False;
      }
      
      t_outframe=toCasaString(outframe);
      t_regridInterpMeth=toCasaString(interp);
      t_centerIsStart = True;
            
      // end prepare regridding parameters
      
      rstat = True;

    } catch (AipsError x) {
      os << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
      RETHROW(x);
    }
    return rstat;
  }


  Bool GridCalc::calcChanFreqs(LogIO& os,
			       Vector<Double>& newCHAN_FREQ, 
			       Vector<Double>& newCHAN_WIDTH,
			       const Vector<Double>& oldCHAN_FREQ, 
			       const Vector<Double>& oldCHAN_WIDTH,
			       const MDirection  phaseCenter,
			       const MFrequency::Types theOldRefFrame,
			       const MEpoch theObsTime,
			       const MPosition mObsPos,
			       const std::string& mode, 
			       const int nchan, 
			       const casac::variant& start, 
			       const casac::variant& width,
			       const casac::variant& restfreq, 
			       const std::string& outframe,
			       const std::string& veltype
			       ){

    Vector<Double> newChanLoBound; 
    Vector<Double> newChanHiBound;
    String t_phasec;

    String t_mode;
    String t_outframe;
    String t_regridQuantity;
    Double t_restfreq;
    String t_regridInterpMeth;
    Double t_cstart;
    Double t_bandwidth;
    Double t_cwidth;
    Bool t_centerIsStart;
    Bool t_startIsEnd;
    Int t_nchan;
    Int t_width;
    Int t_start;

    if(!GridCalc::convertGridPars(os,
				  mode, 
				  nchan, 
				  start, 
				  width,
				  "linear", // a dummy value in this context
				  restfreq, 
				  outframe,
				  veltype,
				  ////// output ////
				  t_mode,
				  t_outframe,
				  t_regridQuantity,
				  t_restfreq,
				  t_regridInterpMeth,
				  t_cstart, 
				  t_bandwidth,
				  t_cwidth,
				  t_centerIsStart, 
				  t_startIsEnd,			      
				  t_nchan,
				  t_width,
				  t_start
				  )
       ){
      // an error occured
      return False;
    }

    // reference frame transformation
    Bool needTransform = True;
    MFrequency::Types theFrame;
    if(outframe==""){ // no ref frame given 
      // keep the reference frame as is
      theFrame = theOldRefFrame;
      needTransform = False;
    }
    else if(!MFrequency::getType(theFrame, outframe)){
      os << LogIO::SEVERE
	 << "Parameter \"outframe\" value " << outframe << " is invalid." 
	 << LogIO::POST;
      return False;
    }
    else if (theFrame == theOldRefFrame){
      needTransform = False;
    }

    uInt oldNUM_CHAN = oldCHAN_FREQ.size();
    if(oldNUM_CHAN == 0){
      newCHAN_FREQ.resize(0);
      newCHAN_WIDTH.resize(0);
      return True;
    }

    if(oldNUM_CHAN != oldCHAN_WIDTH.size()){
      os << LogIO::SEVERE
	 << "Internal error: inconsistent dimensions of input channel freq and width arrays." 
	 << LogIO::POST;
      return False;
    }      

    Vector<Double> transNewXin;
    Vector<Double> transCHAN_WIDTH(oldNUM_CHAN);

    if(needTransform){
      transNewXin.resize(oldNUM_CHAN);
      // set up conversion
      Unit unit(String("Hz"));
      MFrequency::Ref fromFrame = MFrequency::Ref(theOldRefFrame, MeasFrame(phaseCenter, mObsPos, theObsTime));
      MFrequency::Ref toFrame = MFrequency::Ref(theFrame, MeasFrame(phaseCenter, mObsPos, theObsTime));
      MFrequency::Convert freqTrans(unit, fromFrame, toFrame);
      
      for(uInt i=0; i<oldNUM_CHAN; i++){
	transNewXin[i] = freqTrans(oldCHAN_FREQ[i]).get(unit).getValue();
	transCHAN_WIDTH[i] = freqTrans(oldCHAN_FREQ[i] +
				       oldCHAN_WIDTH[i]/2.).get(unit).getValue()
	  - freqTrans(oldCHAN_FREQ[i] -
		      oldCHAN_WIDTH[i]/2.).get(unit).getValue(); // eliminate possible offsets
      }
    }
    else {
      // just copy
      transNewXin.assign(oldCHAN_FREQ);
      transCHAN_WIDTH.assign(oldCHAN_WIDTH);
    }

    // calculate new grid

    String message;

    if(!SubMS::regridChanBounds(newChanLoBound, 
				newChanHiBound,
				t_cstart,  
				t_bandwidth, 
				t_cwidth, 
				t_restfreq,
				t_regridQuantity,
				transNewXin, 
				transCHAN_WIDTH,
				message,
				t_centerIsStart,
				t_startIsEnd,
				t_nchan,
				t_width,
				t_start
				)
       ){ // there was an error
      os << LogIO::WARN << message << LogIO::POST;
      return False;
    }
    
    os << LogIO::NORMAL << message << LogIO::POST;

    // we have a useful set of channel boundaries
    uInt newNUM_CHAN = newChanLoBound.size();
    
    // complete the calculation of the new channel centers and widths
    // from newNUM_CHAN, newChanLoBound, and newChanHiBound 
    newCHAN_FREQ.resize(newNUM_CHAN);
    newCHAN_WIDTH.resize(newNUM_CHAN);
    for(uInt i=0; i<newNUM_CHAN; i++){
      newCHAN_FREQ[i] = (newChanLoBound[i]+newChanHiBound[i])/2.;
      newCHAN_WIDTH[i] = newChanHiBound[i]-newChanLoBound[i];
    }
    
    return True;

  }
  
}  //# NAMESPACE CASAC - END
