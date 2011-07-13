// Copyright (2008) Juan Pardo  (presumed)
// Copyright European Southern Observatory (possibly)
// Copyright (2008) Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>
// 
// This file is part of AATM
//
// AATM is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// AATM is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
// License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with AATM.  If not, see <http://www.gnu.org/licenses/>.
//
// Comments regarding this example welcome at: 
// Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>
//

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include <boost/shared_ptr.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>


// Unfortunately the using statement below is required because of poor
// desgin of the header files
using namespace std;

#include "ATMRefractiveIndexProfile.h"
#include "ATMPercent.h"
#include "ATMPressure.h"
#include "ATMNumberDensity.h"
#include "ATMMassDensity.h"
#include "ATMTemperature.h"
#include "ATMLength.h"
#include "ATMInverseLength.h"
#include "ATMOpacity.h"  
#include "ATMHumidity.h"
#include "ATMFrequency.h"
#include "ATMWaterVaporRadiometer.h"
#include "ATMWVRMeasurement.h"
#include "ATMProfile.h"
#include "ATMSpectralGrid.h"
#include "ATMRefractiveIndex.h"
#include "ATMSkyStatus.h"
#include "ATMAngle.h"


/** 

This the information contained in the input file
*/
struct FInputs
{
  std::vector<double> time;
  std::vector<atm::Angle> el;
  std::vector<atm::WVRMeasurement> tsky;
};

/**
   Turn string s into a vector of types T
 */
template<class T>
void tokenizeV(const std::string &s,
	       std::vector<T> &o)
{
  typedef boost::tokenizer<boost::escaped_list_separator<char> >  tok_t;

  tok_t tok(s);
  for(tok_t::iterator j (tok.begin());
      j != tok.end();
      ++j)
  {
    std::string f(*j);
    boost::trim(f);    
    o.push_back(boost::lexical_cast<T>(f));
  }
}

/**
   Parse inputs in the file fname.

   \param fname Input file name

   \param i Store the parsed information in this structure
 */
void parseFile(const char * fname,
	       FInputs &i)
{


  std::ifstream ifs(fname);
  
  std::string   scratch;
  while(ifs.good())
  {
    std::getline(ifs,scratch);
    std::vector<double> ld;
    tokenizeV(scratch,ld);
    
    i.time.push_back(ld[0]);
    i.el.push_back(ld[1]);

    std::vector<atm::Temperature> v_tsky(&ld[3],
					 &ld[11]);
    i.tsky.push_back(atm::WVRMeasurement(atm::Angle(ld[1]),
					 v_tsky));
  }
}


/**

   \bug First parameter should be const reference but there is a bug
   in ATM preventing this
 */
void printSpecWnd(atm::RefractiveIndexProfile &rip,
		  std::ostream &o)
{

  for(size_t j=0;
      j<rip.getNumSpectralWindow(); 
      ++j)
  {
    o << "Spectral Window " << j 
      << " Central Frequency: "<< rip.getRefFreq(j).get("GHz") << " GHz, "
      << " Freq. Resolution: " << rip.getChanSep(j).get("MHz") << " MHz, "
      << " Num. of channels: " << rip.getNumChan(j)    
      << std::endl;
  }

}

typedef boost::shared_ptr<atm::AtmProfile> pAtmProf;

pAtmProf
simpleMKAtmo(void)
{
  // Atmospheric type (to reproduce behavior above the tropopause)
  unsigned int  atmType=1; 
  // Ground temperature
  atm::Temperature T( 268.15,"K");
  // Ground Pressure
  atm::Pressure P( 623.0,"mb");
  // Ground Relative Humidity (indication)
  atm::Humidity H(11.30,"%");    
  // Altitude of the site 
  atm::Length Alt(4100,"m");     
  // Water vapor scale height
  atm::Length WVL(2.2,"km");     
  // Tropospheric lapse rate (must be in K/km)
  double TLR=-5.6;     
  // Upper atm. boundary for calculations
  atm::Length topAtm(  48.0,"km");     
  // Primary pressure step (10.0 mb)
  atm::Pressure Pstep(  10.0,"mb");
  // Pressure step ratio between two consecutive layers     
  double PstepFact=1.2;

  pAtmProf mkprof (new atm::AtmProfile(Alt,
				       P,
				       T,
				       TLR,
				       H,
				       WVL,
				       Pstep,
				       PstepFact,
				       topAtm,
				       atmType));
  return mkprof;

}

/**
   Parameters about the spectral windows to use -- this is specialised
   for the ALMA Prototype WVRs
 */
struct specWndPars
{
  /// Number of channels per spectral window
  const size_t nc;
  /// The referece chanel for the spectral windows
  const size_t rc;
  /// LO frequency of the system
  const double cf;

  /// Filter IF Frequencies
  std::vector<double> f_cent;
  /// Filter bandwidths
  std::vector<double> f_bw;

  specWndPars(void):
    nc(25),
    rc(13),
    cf(183.310)
  {
    const double filter_c[] = {-5.225, -3.18, -1.9475, -0.882, 0.882, 1.9475, 3.18, 5.225};
    const double filter_i[] = {2.650,  1.4, 0.845, 0.206, 0.206, 0.845, 1.4, 2.650};  
    f_cent=std::vector<double>(&filter_c[0], &filter_c[8]);
    f_bw=std::vector<double>(&filter_i[0], &filter_c[8]);
  }
};

void addWVRSpecWnd(double ifreq,
		   double bw,
		   const specWndPars &sp,
		   atm::RefractiveIndexProfile &rip,
		   std::vector<unsigned int> &ids)
{

  atm::Frequency reffreq1(sp.cf + ifreq,"GHz"); 
  atm::Frequency chansep1(bw/float(sp.nc),"GHz"); 

  ids.push_back(rip.getNumSpectralWindow()); 

  rip.addNewSpectralWindow(sp.nc,
			   sp.rc, 
			   reffreq1, 
			   chansep1); 
}

void mkWVRSpecWnds(const specWndPars &sp,
		   atm::RefractiveIndexProfile &rip,
		   std::vector<unsigned int> &ids)
{
  // Start from one because of the bug desribed below
  for (size_t i =1; i < sp.f_cent.size(); ++i)
  {
    addWVRSpecWnd(sp.f_cent[i],
		  sp.f_bw[i],
		  sp,
		  rip,
		  ids);
  }
}

void printDataSummary(std::vector<atm::WVRMeasurement> &d,
		      std::ostream &o)
{
  o<<"Total number of WVR data: " 
   <<d.size()
   <<std::endl
   <<"Elevation of last measurement: "
   <<d[d.size()-1].getElevation().get("deg")
   <<"deg"
   <<std::endl;
}

void printWVRFit(atm::SkyStatus &ss,
		 std::vector<atm::WVRMeasurement> &data,
		 size_t f,
		 size_t l,
		 std::ostream &o)
{
  o<<"Analysing "<<l<<"meausrements starting at: "<<f
   << std::endl;   


  ss.WaterVaporRetrieval_fromWVR(data,
				 f,
				 f+l);

  o<<"The average Sigma of this ensemble of fits is: " 
   <<ss.getWVRAverageSigmaTskyFit(data,
				  f,
				  f+l).get("K") 
   <<"K"<<std::endl;

  for(size_t i=f;
      i<f+l; 
      ++i)
  {
    o<<"Measurement #"<<i 
     <<"/ Measured (fitted) Sky Tebb's (in K): ";

    for (size_t j =0; j < 8 ; ++j) 
    {
      o<<data[i].getmeasuredSkyBrightness()[j].get("K") 
       <<"("
       <<data[i].getfittedSkyBrightness()[j].get("K") 
       << ") ";
    }
    o<<std::endl;

    o<<" Sigma Fit: "
     <<data[i].getSigmaFit().get("K")
     <<" K / Retrieved Water Vapor Column: " 
     <<data[i].getretrievedWaterVaporColumn().get("mm") <<" mm "
     <<std::endl;
  }   

}

void analyse(const char *fname,
	     std::ostream &output)
{   

  pAtmProf aprof(simpleMKAtmo());
  
  std::vector<unsigned int> WVR_signalId;

  specWndPars sp;

  // --------------------------------------------------
  // This block is due to the fact that RefractiveIndexProfile needs
  // to be constructed with one spectral window at least..., would be
  // better off not having that

  atm::Frequency reffreq0(sp.cf+sp.f_cent[0],"GHz"); 
  atm::Frequency chansep0(sp.f_bw[0]/double(sp.nc),"GHz"); 
  atm::SpectralGrid sma_SpectralGrid(sp.nc,
				     sp.rc,
				     reffreq0,
				     chansep0);
  WVR_signalId.push_back(0); 
  // --------------------------------------------------


  atm::RefractiveIndexProfile sma_RefractiveIndexProfile(sma_SpectralGrid, 
							 *aprof);

  mkWVRSpecWnds(sp,
		sma_RefractiveIndexProfile,
		WVR_signalId);		
		

  printSpecWnd(sma_RefractiveIndexProfile,
	       output);


  atm::SkyStatus sma_SkyStatus(sma_RefractiveIndexProfile);

  const double c_guess=0.8;
  atm::WaterVaporRadiometer wvr183ghz(WVR_signalId,
				      std::vector<double>(WVR_signalId.size(),
							  c_guess),
				      std::vector<atm::Percent>(WVR_signalId.size(),
								atm::Percent(100.0,"%")),
				      atm::Temperature(268.15,"K")); 


  std::vector<atm::WVRMeasurement> RadiometerData;

  FInputs indata;
  parseFile(fname,
	    indata);

  printDataSummary(indata.tsky,
		   output);

  sma_SkyStatus.setWaterVaporRadiometer(wvr183ghz);

  printWVRFit(sma_SkyStatus,
	      indata.tsky,
	      0,5,
	      output);

  sma_SkyStatus.updateSkyCoupling_fromWVR(indata.tsky,
					  100,
					  105);

  output <<"Sky Coupling="
	 <<sma_SkyStatus.getWaterVaporRadiometerSkyCoupling(0)
	 <<std::endl;

  printWVRFit(sma_SkyStatus,
	      indata.tsky,
	      501,1,
	      output);

}


int main()
{   
  const char * datafname = "data/SMA_17JUL2006_skydip1.dat";

  analyse(datafname,
	  std::cout);

  return 0;
}  
