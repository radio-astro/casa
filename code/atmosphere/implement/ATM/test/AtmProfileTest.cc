#include <string>
#include <vector>
#include <iostream>
using namespace std;


#include "ATMPercent.h"
#include "ATMPressure.h"
#include "ATMNumberDensity.h"
#include "ATMMassDensity.h"
#include "ATMTemperature.h"
#include "ATMLength.h"
#include "ATMHumidity.h"
#include "ATMAtmosphereType.h"
#include "ATMType.h"
#include "ATMProfile.h"
#include "ATMTypeName.h"
using namespace atm;


int main()
{  
  // double h_div_k=0.04799274551;        // plank/boltz in units of K/GHz

  Atmospheretype   atmType = tropical; // Atmospheric type (to reproduce behavior above the tropopause)
  Temperature      T( 270.0,"K" );     // Ground temperature
  Pressure         P( 560.0,"mb");     // Ground Pressure
  Humidity         H(  20.0,"%" );     // Ground Relative Humidity (indication)
  Length         Alt(  5000,"m" );     // Altitude of the site 
  Length         WVL(   2.0,"km");     // Water vapor scale height
  double         TLR=  -5.6      ;     // Tropospheric lapse rate (must be in K/km)
  Length      topAtm(  48.0,"km");     // Upper atm. boundary for calculations
  Pressure     Pstep(  10.0,"mb");     // Primary pressure step
  double   PstepFact=         1.2;     // Pressure step ratio between two consecutive layers


  cout<<" AtmProfileTest: BASIC ATMOSPHERIC PARAMETERS TO GENERATE REFERENCE ATMOSPHERIC PROFILE"<<endl; 
  cout<<" AtmProfileTest:   "<<endl;
  cout<<" AtmProfileTest: Ground temperature T:         " << T.get()         << " K"    <<endl; 
  cout<<" AtmProfileTest: Ground pressure P:            " << P.get("mb")     << " mb"   <<endl; 
  cout<<" AtmProfileTest: Relative humidity rh:         " << H.get("%")      << " %"    <<endl; 
  cout<<" AtmProfileTest: Scale height h0:              " << WVL.get("km")   << " km"   <<endl; 
  cout<<" AtmProfileTest: Pressure step dp:             " << Pstep.get("mb") << " mb"   <<endl; 
  cout<<" AtmProfileTest: Altitude alti:                " << Alt.get()       << " m"    <<endl; 
  cout<<" AtmProfileTest: Attitude top atm profile atmh:" << topAtm.get("km")<< " km"   <<endl; 
  cout<<" AtmProfileTest: Pressure step factordp1:      " << PstepFact          << " "    <<endl; 
  cout<<" AtmProfileTest: Tropospherique lapse rate:    " << TLR                << " K/km" <<endl;

  AtmProfile myProfile( Alt, P, T, TLR, H, WVL, Pstep, PstepFact,  topAtm, atmType );

  cout<<" AtmProfileTest: Atmospheric type:             " << myProfile.getAtmosphereType() <<endl;
  cout<<" AtmProfileTest:   "<<endl;
  cout<<" AtmProfileTest:   "<<endl;

  cout<<" AtmProfileTest: Object myProfile built with the AtmProfile CONSTRUCTOR and the above entries"<<endl; 
  cout<<"  "<<endl;
  cout<<" AtmProfileTest: Number of layers returned:  " << myProfile.getNumLayer() <<endl;
  cout<<" AtmProfileTest: Layer parameters:  " <<endl;
  
    
  for(unsigned int i=0; i<myProfile.getNumLayer(); i++){
    cout << " AtmProfileTest:  P: "          << myProfile.getLayerPressure(i).get("mb")    << " mb" 
	 << "  T: "          << myProfile.getLayerTemperature(i).get("K")   << " K"
	 << "  Thickness: "  << myProfile.getLayerThickness(i).get("m")   << " m"
	 << "  WaterVapor: " << myProfile.getLayerWaterVaporMassDensity(i).get("kgm**-3")  << " kg m-3"
	 << "  WaterVapor: " << myProfile.getLayerWaterVapor(i).get("m**-3")  << " m-3"
	 << "  CO: "         << myProfile.getLayerCO(i).get("m**-3")          << " m-3"
         << "  O3: "         << myProfile.getLayerO3(i).get("m**-3")          << " m-3"
         << "  N2O: "        << myProfile.getLayerN2O(i).get("m**-3")         << " m-3" << endl;
  }

}
