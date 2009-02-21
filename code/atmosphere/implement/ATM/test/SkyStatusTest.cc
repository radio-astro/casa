#include <string>
#include <vector>
#include <iostream>
#include <fstream>
using namespace std;



#include "ATMPercent.h"
#include "ATMPressure.h"
#include "ATMNumberDensity.h"
#include "ATMMassDensity.h"
#include "ATMTemperature.h"
#include "ATMLength.h"
#include "ATMInverseLength.h"
#include "ATMOpacity.h"
#include "ATMAngle.h"
#include "ATMHumidity.h"
#include "ATMFrequency.h"
#include "ATMWaterVaporRadiometer.h"
#include "ATMWVRMeasurement.h"
#include "ATMAtmosphereType.h"
#include "ATMType.h"
#include "ATMProfile.h"
#include "ATMSpectralGrid.h"
#include "ATMRefractiveIndexProfile.h"
#include "ATMSkyStatus.h"
#include "ATMTypeName.h"
using namespace atm;

int main()
{  
  //  double h_div_k=0.04799274551;        // plank/boltz in un of K/GHz

  Atmospheretype   atmType = tropical; // Atmospheric type (to reproduce behavior above the tropopause)
  // int                 atmType = 1; // Atmospheric type (to reproduce behavior above the tropopause)
  Temperature      T( 270.0,"K" );     // Ground temperature
  Pressure         P( 560.0,"mb");     // Ground Pressure
  Humidity         H(  20.0,"%" );     // Ground Relative Humidity (indication)
  Length         Alt(  5000,"m" );     // Altitude of the site 
  Length         WVL(   2.0,"km");     // Water vapor scale height
  double         TLR=  -5.6      ;     // Tropospheric lapse rate (must be in K/km)
  Length      topAtm(  48.0,"km");     // Upper atm. boundary for calculations
  Pressure     Pstep(  10.0,"mb");     // Primary pressure step (10.0 mb)
  double   PstepFact=         1.2;     // Pressure step ratio between two consecutive layers


  cout<<" SkyStatusTest: BASIC ATMOSPHERIC PARAMETERS TO GENERATE REFERENCE ATMOSPHERIC PROFILE"<<endl; 
  cout<<" SkyStatusTest:   "<<endl;
  cout<<" SkyStatusTest: Ground temperature T:         " << T.get()         << " K"    <<endl; 
  cout<<" SkyStatusTest: Ground pressure P:            " << P.get("mb")     << " mb"   <<endl; 
  cout<<" SkyStatusTest: Relative humidity rh:         " << H.get("%")      << " %"    <<endl; 
  cout<<" SkyStatusTest: Scale height h0:              " << WVL.get("km")   << " km"   <<endl; 
  cout<<" SkyStatusTest: Pressure step dp:             " << Pstep.get("mb") << " mb"   <<endl; 
  cout<<" SkyStatusTest: Altitude alti:                " << Alt.get()       << " m"    <<endl; 
  cout<<" SkyStatusTest: Attitude top atm profile atmh:" << topAtm.get("km")<< " km"   <<endl; 
  cout<<" SkyStatusTest: Pressure step factordp1:      " << PstepFact          << " "    <<endl; 
  cout<<" SkyStatusTest: Tropospherique lapse rate:    " << TLR                << " K/km" <<endl;

  AtmProfile myProfile( Alt, P, T, TLR, H, WVL, Pstep, PstepFact,  topAtm, atmType );

  cout<<" SkyStatusTest: Atmospheric type:             " << myProfile.getAtmosphereType() <<endl;
  cout<<" SkyStatusTest:   "<<endl;
  cout<<" SkyStatusTest:   "<<endl;

  cout<<" SkyStatusTest: Object myProfile built with the AtmProfile CONSTRUCTOR and the above entries"<<endl; 
  cout<<" SkyStatusTest:   "<<endl;
  cout<<" SkyStatusTest: Number of layers returned:  " << myProfile.getNumLayer() <<endl;
  cout<<" SkyStatusTest: Layer parameters:  " <<endl;
  
    
  cout << " SkyStatusTest: size of Temperature Profile: " << myProfile.getTemperatureProfile().size() << endl;
  for(unsigned int i=0; i<myProfile.getNumLayer(); i++){
    cout << " SkyStatusTest:  P: "          << myProfile.getLayerPressure(i).get("mb")    << " mb" 
	 << " T: "          << myProfile.getLayerTemperature(i).get("K")   << " K"
	 << " Thickness: "  << myProfile.getLayerThickness(i).get("m")   << " m"
	 << " WaterVapor: " << myProfile.getLayerWaterVaporMassDensity(i).get("kgm**-3")  << " kg m-3"
	 << " WaterVapor: " << myProfile.getLayerWaterVapor(i).get("m**-3")  << " m-3"
	 << " CO: "         << myProfile.getLayerCO(i).get("m**-3")      << " m-3"
         << " O3: "         << myProfile.getLayerO3(i).get("m**-3")          << " m-3"
         << " N2O: "        << myProfile.getLayerN2O(i).get("m**-3")         << " m-3" << endl;
  }

  cout << " SkyStatusTest: First guess precipitable water vapor content: " << myProfile.getGroundWH2O().get("mm") << "mm" << endl;
  cout << " SkyStatusTest: (This value is estimated from the relative humidity at ground level and the water vapor scale height)" << endl;
  cout<<" SkyStatusTest:   "<<endl;
  cout<<" SkyStatusTest:   "<<endl;

  cout << " SkyStatusTest:    TEST FOR JUST 1 FREQUENCY  " << endl;
  cout << " SkyStatusTest:    =========================  " << endl;
  cout << " SkyStatusTest:  " << endl;

  Frequency  mySingleFreq(850,"GHz");

  RefractiveIndexProfile myRefractiveIndexProfile(mySingleFreq, myProfile);

  SkyStatus myRadiance(myRefractiveIndexProfile);

  Length   new_wh2o0(1.0,"mm");  
  myRadiance.setUserWH2O(new_wh2o0);


  for(unsigned int i=0; i<myRadiance.getNumSpectralWindow(); i++){

    for(unsigned int j=0; j<myRadiance.getNumChan(i); j++){

      cout << " SkyStatusTest: Frequency      : " << myRadiance.getChanFreq(j).get("GHz") << " GHz" << endl; 

      cout << " SkyStatusTest: Wet opacity    : " << myRadiance.getWetOpacity(j).get() << " nepers, for " << myRadiance.getUserWH2O().get("mm") << " mm H2O" << endl; 

      cout << " SkyStatusTest: Dry opacity    : " << myRadiance.getDryOpacity(j).get() << " nepers " << endl; 
      
      cout << " SkyStatusTest: Sky brightness : " << myRadiance.getAverageTebbSky(j).get("K") << " K" << endl; 

    }

  }

  Length   new_wh2o(0.4,"mm");  
  cout << " SkyStatusTest: (INPUT CHANGE) water vapor column:           " << new_wh2o.get("mm")                       << " mm" << endl;
  cout << " SkyStatusTest: (NEW OUTPUT) T_EBB =                         " << myRadiance.getAverageTebbSky(new_wh2o).get("K") << " K " << endl;
  cout << " SkyStatusTest: Current water vapor column in Radiance Class:" << myRadiance.getUserWH2O().get("mm")           << " mm" << endl;
  cout << " SkyStatusTest:  " <<endl;


  myRadiance.setAirMass(2.0);

  cout << " SkyStatusTest: (INPUT CHANGE) Air mass:                     " <<  myRadiance.getAirMass()              << "   " <<endl;
  cout << " SkyStatusTest: (NEW OUTPUT) T_EBB =                         " <<  myRadiance.getAverageTebbSky().get("K")     << " K " <<endl;
  cout << endl;

  Length   new_wh2o1(0.8,"mm" ); 
 
  myRadiance.setUserWH2O(new_wh2o1);

  cout<< " SkyStatusTest: (INPUT CHANGE) water vapor column:           " << myRadiance.getUserWH2O().get("mm")        << " mm" <<endl;
  cout<< " SkyStatusTest: (NEW OUTPUT) T_EBB =                         " << myRadiance.getAverageTebbSky().get("K")      << " K " <<endl;
  cout<<endl;

  cout << " SkyStatusTest:    TEST FOR JUST 1 SPECTRAL WINDOW WITH SEVERAL CHANNELS  " << endl;
  cout << " SkyStatusTest:    =====================================================  " << endl;
  cout << " SkyStatusTest:  " << endl;

  unsigned int numchan=8;   /* 64; */
  unsigned int refchan=4;    /* 32; */
  Frequency reffreq(850.00,"GHz");
  Frequency chansep(  1.00,"GHz");

  SpectralGrid band(numchan, refchan, reffreq, chansep);
  RefractiveIndexProfile abs_band(band, myProfile); 
  SkyStatus rad_band(abs_band);


  cout << " SkyStatusTest: water vapor column: " << rad_band.getUserWH2O().get("mm") << " mm" << endl;
  cout << " SkyStatusTest: Air mass          : " << rad_band.getAirMass() << endl;
  cout << " " << endl;
  for(unsigned int i=0; i<rad_band.getNumChan(0); i++){

    cout << " SkyStatusTest: Freq: " <<  rad_band.getChanFreq(i).get() << " T_EBB=" << rad_band.getTebbSky(i).get("K")  << " K " <<endl;

  }

  // rad_band.readWVRData("radiometer_data.dat",);

  return 0;

}







