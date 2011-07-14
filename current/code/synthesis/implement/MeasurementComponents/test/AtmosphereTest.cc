// AtmTest.cpp

#include <iostream>
//#include <stdlib.h>
#include "../Atmosphere.h"
#include "TelCalTrace.h"

using namespace std;


int main()
{
#ifdef AIPS_USEATM
  TELCAL_TRACE("Test of Atmosphere class\n\n");

  /// Test Atmosphere()
  double altitude    = 2550.;     // m
  double temperature = 270.32;    // K
  double pressure    = 73585;     // Pascals
  double maxAltitude = 45000;     // m
  double humidity    = 20;        // % 
  double dTempdH     = -5.6/1000.; // K/m
  double dP          = 500.;       // Pascals
  double dPm         = 1.25;
  double h0          = 2000.;     // m

  TELCAL_TRACE("Test of constructor  Atmosphere(altitude,temperature,pressure,maxAltitude,humidity,dTempdH,dP,dPm, h0, MIDLAT_SUMMER)\n\n");
  TELCAL_TRACE1("altitude    = %f m\n",altitude);
  TELCAL_TRACE1("temperature = %f K\n",temperature);
  TELCAL_TRACE1("pressure    = %f Pascals\n",pressure);
  TELCAL_TRACE1("maxAltitude = %f m\n",maxAltitude);
  TELCAL_TRACE1("humidity    = %f \n",humidity);
  TELCAL_TRACE1("dTempdH     = %f K/m\n",dTempdH);
  TELCAL_TRACE1("dP          = %f pascals\n",dP);
  TELCAL_TRACE1("dPm         = %f \n",dPm);
  TELCAL_TRACE1("h0          = %f m\n\n",h0);

  casa::Atmosphere *atm = new casa::Atmosphere(altitude,temperature,pressure,maxAltitude,humidity,dTempdH,dP,dPm, h0, casa::MIDLAT_SUMMER);
  
  TELCAL_TRACE("Test: getStartupWaterContent()\n");
  double water = atm->getStartupWaterContent();
  TELCAL_TRACE1("Guessed water content %8.4f meter\n\n",water);

  /// Profile
  int npp;
  TELCAL_TRACE("Test: Profile getProfile()\n");

  casa::Profile p=atm->getProfile();
  npp = p.thickness_m.size(); 
  for (int i=0; i<npp;i++) {
    printf("%8.2f %10.3f %10.6f %10.3f\n",p.thickness_m[i], p.temperature_m[i], p.water_m[i], p.pressure_m[i]);
  }
  cout<<endl;

  TELCAL_TRACE("Test: void getProfile(Profile)\n");
  casa::Profile p2;
  atm->getProfile(p2);
  /*
  for (int i=0; i<npp;i++) {
    printf("%8.2f %10.3f %10.6f %10.3f\n",p2.thickness[i], p2.temperature[i], p2.water[i], p2.pressure[i]);
  }
  */

  /// Test initWindow()
  
  int nbands = 1;
  double *fCenter = new double [nbands];
  double *fWidth  = new double [nbands];
  double *fRes    = new double [nbands];

  fCenter[0] = 88.e9;
  fWidth[0]  =  5.e8;
  fRes[0]    =  5.e8;
  double precWater = water;

  TELCAL_TRACE("Test of initWindow()\n");
  atm->initWindow(nbands,
		  fCenter,
		  fWidth,
		  fRes);

  int n=atm->getNdata(0);
  TELCAL_TRACE1("%d band(s)\n",n);


  /// Test getOpacity()
  TELCAL_TRACE("Test: Opacity getOpacity()\n");
  casa::Opacity o=atm->getOpacity();
  TELCAL_TRACE2(" - dryOpacity %6.4f wetOpacity/mm %7.5f\n",o.dryOpacity_m[0],o.wetOpacity_m[0]);

  TELCAL_TRACE("Test: getOpacity(Opacity)\n");
  atm->getOpacity(o);
  TELCAL_TRACE2(" - dryOpacity %6.4f wetOpacity/mm %7.5f\n",o.dryOpacity_m[0],o.wetOpacity_m[0]);
  

  /// Test getAbsCoeff
  casa::AbsCoeff a=atm->getAbsCoeff();
  TELCAL_TRACE3("AbsCoeff getAbsCoeff() (3 first layers): %f %f %f\n",a.kH2OLines_m[0],a.kH2OLines_m[1*NMAX_DATOS*npp],a.kH2OLines_m[2*NMAX_DATOS*npp]);

  /// Test getAbsCoeffDer
  casa::AbsCoeffDer aDer=atm->getAbsCoeffDer();
  TELCAL_TRACE3("AbsCoeff getAbsCoeff() (3 first layers): %f %f %f\n",aDer.kH2OLinesDer_m[0],aDer.kH2OLinesDer_m[1*NMAX_DATOS*npp],aDer.kH2OLinesDer_m[2*NMAX_DATOS*npp]);

  /// Compute SkyBrightness()
  double airMass = 1.51;
  double tbgr    = 2.73;
  precWater = 4.05/1000;

  TELCAL_TRACE("Test of computeSkyBrightness()\n");
  atm->computeSkyBrightness(airMass, tbgr, precWater);

  vector<double> tBand = atm->getSkyBrightness(casa::BLACKBODY);
  TELCAL_TRACE1("SkyBrightness = %f K (TEBB) \n",tBand[0]);
  tBand = atm->getSkyBrightness(casa::RAYLEIGH_JEANS);
  TELCAL_TRACE1("SkyBrightness = %f K (Rayleigh Jeans) \n\n",tBand[0]);


  ///==================================================================
  /// Test spectral routines
  ///==================================================================

  TELCAL_COUT("=====================================================");
  TELCAL_COUT("Test with spectral data\n");

  delete [] fCenter;
  delete [] fWidth;
  delete [] fRes;

  nbands = 2;
  fCenter = new double [nbands];
  fWidth  = new double [nbands];
  fRes    = new double [nbands];

  for (int i=0;i<nbands;i++) {
    fCenter[i] = 88.e9;
    fWidth[i]  =  5.e8;
    fRes[i]    =  fWidth[0]/4.;           // 4 channels
  }

  TELCAL_TRACE("Test of initWindow()\n");
  atm->initWindow(nbands,
		  fCenter,
		  fWidth,
		  fRes);

  n=atm->getNdata(0);
  TELCAL_TRACE2("%d bands %d channels(s)\n",nbands,n);

  /// Test getOpacitySpec()
  TELCAL_TRACE("Test: Opacity getOpacitySpec()\n");
  casa::OpacitySpec ospec=atm->getOpacitySpec();
  cout<<ospec.dryOpacitySpec_m.size()<<" "<<ospec.dryOpacitySpec_m[0].size()<<endl;
  cout<<ospec.wetOpacitySpec_m.size()<<" "<<ospec.wetOpacitySpec_m[0].size()<<endl;
  cout<<ospec.dryOpacitySpec_m.size()<<" "<<ospec.dryOpacitySpec_m[1].size()<<endl;
  cout<<ospec.wetOpacitySpec_m.size()<<" "<<ospec.wetOpacitySpec_m[1].size()<<endl;

  TELCAL_COUT("band 0");
  for (int i=0; i<n; i++) {
    TELCAL_TRACE2(" - dryOpacity %6.4f  wetOpacity/mm %7.5f \n",
		  ospec.dryOpacitySpec_m[0][i],
		  ospec.wetOpacitySpec_m[0][i]);
  }
  TELCAL_COUT("band 1");
  for (int i=0; i<n; i++) {
    TELCAL_TRACE2(" - dryOpacity %6.4f  wetOpacity/mm %7.5f \n",
		  ospec.dryOpacitySpec_m[1][i],
		  ospec.wetOpacitySpec_m[1][i]);
  }
  cout<<endl;


  TELCAL_TRACE("Test of computeSkyBrightness()\n");
  atm->computeSkyBrightness(airMass, tbgr, precWater);

  vector< vector<double> > tBandSpec = atm->getSkyBrightnessSpec(casa::BLACKBODY);
  for (int ib=0;ib<nbands;ib++)
    for (int i=0;i<4;i++) {
      TELCAL_TRACE3("Band %d channel %d SkyBrightness = %f K (TEBB) \n",ib,i,tBandSpec[ib][i]);
    }
  tBandSpec = atm->getSkyBrightnessSpec(casa::RAYLEIGH_JEANS);
  for (int ib=0;ib<nbands;ib++)
    for (int i=0;i<4;i++) {
      TELCAL_TRACE3("Band %d channel %d SkyBrightness = %f K (Rayleigh Jeans) \n",ib,i,tBandSpec[ib][i]);
    }
  cout<<endl;


  delete atm;
  cout <<"end of Atmosphere test"<<endl;
  exit(0);

#else
  cout <<"Atmosphere package not available" << endl;
  exit(3);    //untested
#endif  
}
