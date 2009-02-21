#include <string>
#include <vector>
#include <iostream>
#include <math.h>
using namespace std;


#include "ATMPercent.h"
#include "ATMPressure.h"
#include "ATMAngle.h"
#include "ATMNumberDensity.h"
#include "ATMMassDensity.h"
#include "ATMTemperature.h"
#include "ATMLength.h"
#include "ATMInverseLength.h"
#include "ATMOpacity.h"
#include "ATMHumidity.h"
#include "ATMFrequency.h"
#include "ATMAtmosphereType.h"
#include "ATMType.h"
#include "ATMProfile.h"
#include "ATMSpectralGrid.h"
#include "ATMRefractiveIndexProfile.h"
#include "ATMTypeName.h"

using namespace atm;

int main()
{  
  //  double h_div_k=0.04799274551;        // plank/boltz in units of K/GHz

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


  cout<<" RefractiveIndexProfileTest: BASIC ATMOSPHERIC PARAMETERS TO GENERATE REFERENCE ATMOSPHERIC PROFILE"<<endl; 
  cout<<"  "<<endl;
  cout<<" RefractiveIndexProfileTest: Ground temperature T:         " << T.get()         << " K"    <<endl; 
  cout<<" RefractiveIndexProfileTest: Ground pressure P:            " << P.get("mb")     << " mb"   <<endl; 
  cout<<" RefractiveIndexProfileTest: Relative humidity rh:         " << H.get("%")      << " %"    <<endl; 
  cout<<" RefractiveIndexProfileTest: Scale height h0:              " << WVL.get("km")   << " km"   <<endl; 
  cout<<" RefractiveIndexProfileTest: Pressure step dp:             " << Pstep.get("mb") << " mb"   <<endl; 
  cout<<" RefractiveIndexProfileTest: Altitude alti:                " << Alt.get()       << " m"    <<endl; 
  cout<<" RefractiveIndexProfileTest: Attitude top atm profile atmh:" << topAtm.get("km")<< " km"   <<endl; 
  cout<<" RefractiveIndexProfileTest: Pressure step factordp1:      " << PstepFact          << " "    <<endl; 
  cout<<" RefractiveIndexProfileTest: Tropospherique lapse rate:    " << TLR                << " K/km" <<endl;

  AtmProfile myProfile( Alt, P, T, TLR, H, WVL, Pstep, PstepFact,  topAtm, atmType );

  cout<<" RefractiveIndexProfileTest: Atmospheric type:             " << myProfile.getAtmosphereType() <<endl;
  cout<<" RefractiveIndexProfileTest:   "<<endl;
  cout<<" RefractiveIndexProfileTest:   "<<endl;

  cout<<" RefractiveIndexProfileTest: Object myProfile built with the AtmProfile CONSTRUCTOR and the above entries"<<endl; 
  cout<<" RefractiveIndexProfileTest:   "<<endl;
  cout<<" RefractiveIndexProfileTest: Number of layers returned:  " << myProfile.getNumLayer() <<endl;
  cout<<" RefractiveIndexProfileTest: Layer parameters:  " <<endl;
  
    
  for(unsigned int i=0; i<myProfile.getNumLayer(); i++){
    cout << " RefractiveIndexProfileTest:  P: "          << myProfile.getLayerPressure(i).get("mb")    << " mb" 
	 << " T: "          << myProfile.getLayerTemperature(i).get("K")   << " K"
	 << " Thickness: "  << myProfile.getLayerThickness(i).get("m")   << " m"
	 << " WaterVapor: " << myProfile.getLayerWaterVaporMassDensity(i).get("kgm**-3")  << " kg m-3"
	 << " WaterVapor: " << myProfile.getLayerWaterVapor(i).get("m**-3")  << " m-3"
	 << " CO: "         << myProfile.getLayerCO(i).get("m**-3")          << " m-3"
         << " O3: "         << myProfile.getLayerO3(i).get("m**-3")          << " m-3"
         << " N2O: "        << myProfile.getLayerN2O(i).get("m**-3")         << " m-3" << endl;
  }


  //  int mylayers=myProfile.getNumLayer();

 
  cout << " RefractiveIndexProfileTest: First guess precipitable water vapor content: " << myProfile.getGroundWH2O().get("mm") << "mm" << endl;
  cout << " RefractiveIndexProfileTest: (This value is estimated from the relative humidity at ground level and the water vapor scale height)" << endl;
  cout<<" RefractiveIndexProfileTest:   "<<endl;
  cout<<" RefractiveIndexProfileTest:   "<<endl;

  Frequency          mySingleFreq(850,"GHz");
  cout << " RefractiveIndexProfileTest: Example 1: Absorption profile for a single frequency: " << mySingleFreq.get("GHz") << " GHz" << endl;
  cout<<"  "<<endl;


  RefractiveIndexProfile myRefractiveIndexProfile(mySingleFreq, myProfile);
 

  cout<<" RefractiveIndexProfileTest: Absorption Profile built from RefractiveIndexProfile CONSTRUCTOR. Summary of results:"<<endl; 
  cout<<endl;
  cout<<" RefractiveIndexProfileTest: Total Dry Opacity at      "<<mySingleFreq.get("GHz") << " GHz for 1.0 air mass: " << myRefractiveIndexProfile.getDryOpacity().get() <<endl;
  cout<<endl;
  cout<<" RefractiveIndexProfileTest: Total Dry Cont Opacity at      "<<mySingleFreq.get("GHz") << " GHz for 1.0 air mass: " << myRefractiveIndexProfile.getDryContOpacity().get() <<endl;
  cout<<" RefractiveIndexProfileTest: Total O2 lines Opacity at      "<<mySingleFreq.get("GHz") << " GHz for 1.0 air mass: " << myRefractiveIndexProfile.getO2LinesOpacity().get() <<endl;
  cout<<" RefractiveIndexProfileTest: Total O3 lines Opacity at      "<<mySingleFreq.get("GHz") << " GHz for 1.0 air mass: " << myRefractiveIndexProfile.getO3LinesOpacity().get() <<endl;
  cout<<" RefractiveIndexProfileTest: Total CO lines Opacity at      "<<mySingleFreq.get("GHz") << " GHz for 1.0 air mass: " << myRefractiveIndexProfile.getCOLinesOpacity().get() <<endl;
  cout<<" RefractiveIndexProfileTest: Total N2O lines Opacity at      "<<mySingleFreq.get("GHz") << " GHz for 1.0 air mass: " << myRefractiveIndexProfile.getN2OLinesOpacity().get() <<endl;
  cout<<endl;
  cout<<" RefractiveIndexProfileTest: Total Wet Opacity at      "<<mySingleFreq.get("GHz") << " GHz for 1.0 air mass: " <<  myRefractiveIndexProfile.getWetOpacity().get() << endl; 
  cout<<endl;
  cout<<" RefractiveIndexProfileTest: Total H2O lines Opacity at      "<<mySingleFreq.get("GHz") << " GHz for 1.0 air mass: " <<  myRefractiveIndexProfile.getH2OLinesOpacity().get() << endl; 
  cout<<" RefractiveIndexProfileTest: Total H2O Cont Opacity at      "<<mySingleFreq.get("GHz") << " GHz for 1.0 air mass: " <<  myRefractiveIndexProfile.getH2OContOpacity().get() << endl; 
  cout<<endl;
  cout<<endl;
   cout<<" RefractiveIndexProfileTest: Total Dispersive Delay at "<<mySingleFreq.get("GHz") << " GHz for 1.0 air mass: " << 
     myRefractiveIndexProfile.getDispersiveWetPathLength().get() << " meters " << endl;

   cout<<" RefractiveIndexProfileTest: Total Non-Dispersive Delay at "<<mySingleFreq.get("GHz") << " GHz for 1.0 air mass: " << 
     myRefractiveIndexProfile.getNonDispersiveWetPathLength().get() << " meters " << endl;
 
   cout<<" RefractiveIndexProfileTest: Total Dry Delay at "<<mySingleFreq.get("GHz") << " GHz for 1.0 air mass: " << 
     myRefractiveIndexProfile.getNonDispersiveDryPathLength().get() << " meters " << endl;
  
   cout<<" RefractiveIndexProfileTest: Total O2 lines Delay at "<<mySingleFreq.get("GHz") << " GHz for 1.0 air mass: " << 
     myRefractiveIndexProfile.getO2LinesPathLength().get() << " meters " << endl;
   cout<<" RefractiveIndexProfileTest: Total O3 lines Delay at "<<mySingleFreq.get("GHz") << " GHz for 1.0 air mass: " << 
     myRefractiveIndexProfile.getO3LinesPathLength().get() << " meters " << endl;
   cout<<" RefractiveIndexProfileTest: Total CO lines Delay at "<<mySingleFreq.get("GHz") << " GHz for 1.0 air mass: " << 
     myRefractiveIndexProfile.getCOLinesPathLength().get() << " meters " << endl;
   cout<<" RefractiveIndexProfileTest: Total N2O lines Delay at "<<mySingleFreq.get("GHz") << " GHz for 1.0 air mass: " << 
     myRefractiveIndexProfile.getN2OLinesPathLength().get() << " meters " << endl;


  cout<<endl;
  cout<<endl;
  cout << " RefractiveIndexProfileTest: (your actual water vapor column is " << (myProfile.getGroundWH2O()).get("mm") << " mm; " << (myRefractiveIndexProfile.getGroundWH2O()).get("mm") << " mm" <<endl;
  cout<<endl;



  /*
  cout << "change the basic parameter"<<endl;
  cout << "=========================="<<endl;
  cout << "Old ground temperature: "<< T.get() << " K"    <<endl; 
  Temperature newT(275.0,"K");   
  cout << "New ground temperature:" << newT.get() << " K"    <<endl; 
  
  myRefractiveIndexProfile.setBasicAtmosphericParameters(Alt, P, newT, TLR, H, WVL); 

  cout << "(your actual water vapor column is " << (myRefractiveIndexProfile.getGroundWH2O()).get("mm") << " mm" <<endl;


  cout<<"Absorption Profile with this new temperature. Summary of results:"<<endl; 
  cout<<endl;
  cout<<"Total Dry Opacity at      "<<mySingleFreq.get("GHz") << " GHz for 1.0 air mass: " << myRefractiveIndexProfile.getDryOpacity().get() <<endl;
  cout<<"Total Wet Opacity at      "<<mySingleFreq.get("GHz") << " GHz for 1.0 air mass: " << 
    myRefractiveIndexProfile.getWetOpacity().get()/(myProfile.getGroundWH2O()).get("mm") << " per mm " << endl; 


  cout<<"Total Dispersive Delay at "<<mySingleFreq.get("GHz") << " GHz for 1.0 air mass: " << 
    (myRefractiveIndexProfile.getDispersivePathLength().get())/((myProfile.getGroundWH2O()).get("mm")) << " meters per mm of water vapor (" << 
    (100*(myRefractiveIndexProfile.getDispersivePathLength().get())/(myProfile.getGroundWH2O().get("mm")))/
    ((myRefractiveIndexProfile.getNonDispersivePathLength().get())/((myProfile.getGroundWH2O().get("mm")))) <<" % of the Non-dispersive one)" << endl;  



  cout << "(your actual water vapor column is " << (myProfile.getGroundWH2O()).get("mm") << " mm)" <<endl;
  cout<<endl;

  cout << "Add a spectral window"<<endl;
  cout << "====================="<<endl;

  int numChan = 4;
  int refChan = 2;
  Frequency refFreq(284.97346,"GHz");   // 350
  Frequency chanSep(2.0,"MHz");

  myRefractiveIndexProfile.add(numChan, refChan, refFreq, chanSep);

  int numSpw = myRefractiveIndexProfile.getNumSpectralWindow();

  cout << "There are now " << numSpw << " spectral windows" << endl;


  cout<<"Absorption profiles including this new spectral window. Summary of results:"<<endl; 
  cout<<endl;
  cout<<"Total Dry Opacity at      "<<mySingleFreq.get("GHz") << " GHz for 1.0 air mass: " << myRefractiveIndexProfile.getDryOpacity().get() <<endl;
  double freq;
  int    spwid=0;
  int    numCh;
  int    k=0;
  for(spwid=0; spwid<numSpw; spwid++){

    numCh = myRefractiveIndexProfile.getNumChan(spwid); cout <<"Spectral window "<<spwid<<" has "<<numCh<<" frequency channels"<<endl;
    for(int n=0; n<numCh; n++){

      freq = myRefractiveIndexProfile.getChanFreq(spwid,n).get("GHz"); 
      cout<<"Total Wet Opacity at      "<< freq << " GHz for 1.0 air mass: " <<
	myRefractiveIndexProfile.getWetOpacity(k).get()/(myRefractiveIndexProfile.getGroundWH2O()).get("mm") << " per mm " << endl; 

      cout<<"Total Non-Dispersive Delay at "<< freq << " GHz for 1.0 air mass: " << 
	(myRefractiveIndexProfile.getNonDispersivePathLength(k).get())/(myProfile.getGroundWH2O().get("mm")) << " meters per mm of water vapor " << endl; 

      cout<<"Total Dispersive Delay at "<< freq << " GHz for 1.0 air mass: " << 
	(myRefractiveIndexProfile.getDispersivePathLength(k).get())/(myProfile.getGroundWH2O().get("mm")) << " meters per mm of water vapor (" <<
	(100*(myRefractiveIndexProfile.getDispersivePathLength(k).get())/(myRefractiveIndexProfile.getGroundWH2O().get("mm")))/
	((myRefractiveIndexProfile.getNonDispersivePathLength(k).get())/(myRefractiveIndexProfile.getGroundWH2O().get("mm"))) <<" % of the Non-dispersive one)" << endl;  
      
      cout<<"Total Dry Delay at "<< freq << " GHz for 1.0 air mass: " << 
	(myRefractiveIndexProfile.getDryPathLength(k).get("micron")) << " microns " << endl;

      
      cout << "(your actual water vapor column is " << (myRefractiveIndexProfile.getGroundWH2O()).get("mm") << " mm)." <<endl;
      cout<<endl;
      k++;
    }
  }
  */


}

