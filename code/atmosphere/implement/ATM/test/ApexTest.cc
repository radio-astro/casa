#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <math.h>

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
#define STRLEN  200        // Max length of a row in a tpoint file
using namespace atm;

int main()
{  
  double airm;
  double airmwvr;
  double opacitycomparison;
  double coupling_astroband;
  unsigned int jj;
  Frequency rff;
  Frequency if1;
  Frequency chansep;
  cout << " ApexTest:" << endl;
  cout << " ApexTest: THIS PROPOSED TEST OF THE ATM INTERFACE SOFTWARE IS BASED ON APEX DATA SCAN NUMBER 17875 (27/July/2005/21H49M27S UT+32s)" << endl;
  cout << " " << endl;

  cout << " ApexTest: STEP 1: CREATES REFERENCE ATMOSPHERIC PROFILE CORRESTONDING TO THE FOLLOWING BASIC PARAMETERS:" << endl;

  Atmospheretype   atmType = tropical; // Atmospheric type (to reproduce behavior above the tropopause)
  Temperature      T( 270.0,"K" );     // Ground temperature
  Pressure         P( 555.0,"mb");     // Ground Pressure
  Humidity         H(   8.0,"%" );     // Ground Relative Humidity (indication)
  Length         Alt(  5105,"m" );     // Altitude of the site 
  Length         WVL(   3.0,"km");     // Water vapor scale height
  double         TLR=  -6.5      ;     // Tropospheric lapse rate (must be in K/km)
  Length      topAtm(  48.0,"km");     // Upper atm. boundary for calculations
  Length      wvr_retrieved;
  Length      wvr_retrieved_astro;
  Temperature sigma_wvr_retrieved_astro;
  Pressure     Pstep(  10.0,"mb");     // Primary pressure step (10.0 mb)
  double   PstepFact=         1.2;     // Pressure step ratio between two consecutive layers

  AtmProfile apex_AtmProfile( Alt, P, T, TLR, H, WVL, Pstep, PstepFact,  topAtm, atmType );

  cout << " ApexTest: First guess precipitable water vapor content: " << apex_AtmProfile.getGroundWH2O().get("mm") << "mm" << endl;
  cout << " ApexTest:  " << endl;


  cout << " ApexTest: STEP 2: CREATES SpectralGrid and RefractiveIndexProfile objects" << endl;

  vector<unsigned int> WVR_signalId;

  unsigned int numchan3=25;  unsigned int refchan3=13;
   Frequency reffreq3(182.01,"GHz"); Frequency chansep3(  0.02,"GHz"); Frequency intfreq3(  1.30,"GHz");
   // Frequency reffreq3(179.11,"GHz"); Frequency chansep3(  0.04,"GHz"); Frequency intfreq3(  4.20,"GHz");
   // Frequency reffreq3(176.81,"GHz"); Frequency chansep3(0.04,"GHz"); Frequency intfreq3(6.50,"GHz");
  SidebandSide sidebandside3=LSB; SidebandType sidebandtype3=DSB;
  SpectralGrid apex_SpectralGrid(numchan3, refchan3, reffreq3, chansep3, intfreq3, sidebandside3, sidebandtype3); 
  WVR_signalId.push_back(0); // this is the Id of the 1st spectral window of the 1st pair (WVR channel 1)

  // Creates RefractiveIndexProfile for the current atm profile (apex_AtmProfile) and spectral grid 
  // (apex_SpectralGrid, so far with only the first WVR channel). Later on we will add new spectral 
  // windows directly at the level of this RefractiveIndexProfile object.
  RefractiveIndexProfile apex_RefractiveIndexProfile(apex_SpectralGrid, apex_AtmProfile);

  
  
  unsigned int numchan4=25;  unsigned int refchan4=13;
  Frequency reffreq4(179.11,"GHz"); Frequency chansep4(  0.04,"GHz"); Frequency intfreq4(  4.20,"GHz");
  SidebandSide sidebandside4=LSB; SidebandType sidebandtype4=DSB;
  WVR_signalId.push_back(apex_RefractiveIndexProfile.getNumSpectralWindow()); // this is the Id of the 1st spectral window of the 2nd pair (WVR channel 2)
  apex_RefractiveIndexProfile.addNewSpectralWindow(numchan4, refchan4, reffreq4, chansep4, intfreq4, sidebandside4, sidebandtype4); 

  unsigned int numchan5=25; unsigned int refchan5=13;
  Frequency reffreq5(176.81,"GHz"); Frequency chansep5(  0.04,"GHz"); Frequency intfreq5(  6.50,"GHz");
  SidebandSide sidebandside5=LSB; SidebandType sidebandtype5=DSB;
  WVR_signalId.push_back(apex_RefractiveIndexProfile.getNumSpectralWindow()); // this is the Id of the 1st spectral window of the 3rd pair (WVR channel 3)
  apex_RefractiveIndexProfile.addNewSpectralWindow(numchan5, refchan5, reffreq5, chansep5, intfreq5, sidebandside5, sidebandtype5); 
  


  for(unsigned int j=0; j<apex_RefractiveIndexProfile.getNumSpectralWindow(); j++){
    cout << " ApexTest: Spectral Window " << j 
	 << " Central Frequency: " <<  apex_RefractiveIndexProfile.getRefFreq(j).get("GHz") << " GHz, "
	 << " Freq. Resolution: " <<  apex_RefractiveIndexProfile.getChanSep(j).get("MHz") << " MHz, "
	 << " Num. of channels: " << apex_RefractiveIndexProfile.getNumChan(j)    << endl;
  }
  cout << " " << endl;

  cout << " ApexTest: Spectral windows associations:    " << endl; 
  for(unsigned int j=0; j<apex_RefractiveIndexProfile.getNumSpectralWindow(); j++){
    if(apex_RefractiveIndexProfile.getAssocSpwId(j).size()==1){
      cout << " ApexTest: Spectral Window " << j << " associated to spectral window: " 
	   <<  apex_RefractiveIndexProfile.getAssocSpwId(j)[0] << " (double band)" << endl; 
    }else{
      cout << " ApexTest: Spectral Window " << j << " associated to spectral window: " 
	   <<  j << " (single band)" << endl; 
    }
  }
  cout << " ApexTest:  " << endl;


  cout << " ApexTest:  STEP 3: CREATES SkyStatus object " << endl;

  SkyStatus apex_SkyStatus(apex_RefractiveIndexProfile);

  vector<double> skycouplingastro; // Sky couplings the astro channel taken as WWR
  vector<Percent> signalgainastro; // Signal Side Band Gain of the astro channel taken as WWR
  for(unsigned int i=0; i<1; i++){
    skycouplingastro.push_back(1.0); 
    signalgainastro.push_back(Percent(50.0,"%"));
  }


  vector<double> skycoupling183; // Sky couplings the WVR Channels (set to 0.7 below)
  vector<Percent> signalgain183; // Signal Side Band Gain of the WVR Channels (set to 50% below)
  double skyCoupling_1stGuess = 0.935;

  Temperature tspill(283.15,"K"); // Spillover Temperature seen by the WVR 



  for(unsigned int i=0; i<WVR_signalId.size(); i++){
    skycoupling183.push_back(skyCoupling_1stGuess); 
    signalgain183.push_back(Percent(50.0,"%"));
  }
  WaterVaporRadiometer wvr183ghz(WVR_signalId,skycoupling183,signalgain183,tspill); 

  /*
  int numchan=25;  int refchan=13; Frequency chansep(  0.04,"GHz");
  Frequency if1(  6.00,"GHz");
  SidebandSide sidebandside_1=LSB; SidebandType sidebandtype_1=DSB;
  Frequency rff(342.505469,"GHz");
  apex_SkyStatus.addNewSpectralWindow(numchan, refchan, rff, chansep, if1, sidebandside_1, sidebandtype_1);
  */
  
  cout << " ApexTest: skycoupling183.size()=" << skycoupling183.size() << endl;
  cout << " ApexTest: wvr183ghz.getSkyCoupling().size()=" << wvr183ghz.getSkyCoupling().size() << endl;
  cout << " ApexTest: wvr183ghz.getIdChannels().size()=" << wvr183ghz.getIdChannels().size() << endl;

  cout << " ApexTest:  " << endl;

  cout << " ApexTest: WaterVaporRadiometer characteristics: " << endl;
  unsigned int Ids;
  cout << " ApexTest: apex_SkyStatus.getWaterVaporRadiometer().getIdChannels().size()=" << apex_SkyStatus.getWaterVaporRadiometer().getIdChannels().size() << endl;

  for(unsigned int i=0; i<apex_SkyStatus.getWaterVaporRadiometer().getIdChannels().size(); i++){
    cout << " ApexTest: i=" << i << endl;
    Ids=apex_SkyStatus.getWaterVaporRadiometer().getIdChannels()[i];
    cout << " ApexTest: WVR Channel " << i << " SpectralGrid Id of Signal sideband: " << 
      Ids << " / SpectralGrid Id of Image sideband: " << apex_SkyStatus.getAssocSpwId(Ids)[0] << endl;
    cout << "              " << " Sky Coupling: " << apex_SkyStatus.getWaterVaporRadiometerSkyCoupling(i) << 
      " / Gain of Signal sideband: " << apex_SkyStatus.getWaterVaporRadiometerSignalGain(i).get("%") << " %" << endl; 
  }


  cout << " ApexTest:  " << endl;
  cout << " ApexTest: STEP 5: Reads a block of real WVRMeasurements taken with APEX on May 18, 2006" << endl;

  vector<WVRMeasurement> RadiometerData;
  WVRMeasurement singleRadiometerData;
  vector<double> time_mjd;
  vector<Angle> elevation;
  vector<Frequency> astrosignalfreq; 
  vector<Frequency> astroimagefreq;
  vector<Frequency> astrobandwidth;
  vector<Temperature> groundtemp;
  vector<Pressure> groundpressure;
  vector<Percent> groundhumidity;
  vector<Temperature> astrotebb;
  vector<Length> astrowater;
  vector<Length> wvrwater;

  Angle aaa;
  FILE*  fp;
  fp = fopen("../test/apex_radiometer_data_MJD53965_skydip.dat", "r");
  if (fp != 0) {
    cout << " ApexTest: file open" << endl;
    char  aRow[STRLEN+1];
    char* token;
    vector<Temperature> v_tsky;
    unsigned int numWVRChannels = 0;
    fgets( aRow, STRLEN, fp );
    unsigned int inilen=strlen(aRow);
    unsigned int lacum=0;

    token = strtok(aRow,","); lacum=lacum+strlen(token)+1;
    time_mjd.push_back(atof(token));
    token = 0; token = strtok(token,","); 
    aaa=Angle(atof(token)+7.8,"deg");
    elevation.push_back(aaa);
    lacum=lacum+strlen(token)+1;

    token = 0; token = strtok(token,","); lacum=lacum+strlen(token)+1; astrosignalfreq.push_back(Frequency(atof(token),"GHz")); 
    token = 0; token = strtok(token,","); lacum=lacum+strlen(token)+1; astroimagefreq.push_back(Frequency(atof(token),"GHz"));
    token = 0; token = strtok(token,","); lacum=lacum+strlen(token)+1; astrobandwidth.push_back(Frequency(atof(token),"GHz"));
    token = 0; token = strtok(token,","); lacum=lacum+strlen(token)+1; groundtemp.push_back(Temperature(atof(token),"K"));
    token = 0; token = strtok(token,","); lacum=lacum+strlen(token)+1; groundpressure.push_back(Pressure(atof(token),"mb"));
    token = 0; token = strtok(token,","); lacum=lacum+strlen(token)+1; groundhumidity.push_back(Percent(atof(token),"%"));
    token = 0; token = strtok(token,","); lacum=lacum+strlen(token)+1; astrotebb.push_back(Temperature(atof(token),"K"));
    token = 0; token = strtok(token,","); lacum=lacum+strlen(token)+1; astrowater.push_back(Length(atof(token),"mm"));
    token = 0; token = strtok(token,","); lacum=lacum+strlen(token)+1; wvrwater.push_back(Length(atof(token),"mm"));

    while (lacum<=inilen){
      numWVRChannels++;
      token = 0; token = strtok(token,","); Temperature tt(atof(token)+0.0,"K"); v_tsky.push_back(tt); 
      lacum=lacum+strlen(token)+1;
    }
    singleRadiometerData=WVRMeasurement(aaa,v_tsky);
    RadiometerData.push_back(singleRadiometerData);
    fgets( aRow, STRLEN, fp );
    while (feof(fp)==0){
      if(strncmp(aRow," ",1)==0){
	token = strtok(aRow,","); lacum=lacum+strlen(token)+1; time_mjd.push_back(atof(token));
	token = 0; token = strtok(token,","); aaa=Angle(atof(token)+7.8,"deg");     elevation.push_back(aaa);

	token = 0; token = strtok(token,","); lacum=lacum+strlen(token)+1; astrosignalfreq.push_back(Frequency(atof(token),"GHz")); 
	token = 0; token = strtok(token,","); lacum=lacum+strlen(token)+1; astroimagefreq.push_back(Frequency(atof(token),"GHz"));
	token = 0; token = strtok(token,","); lacum=lacum+strlen(token)+1; astrobandwidth.push_back(Frequency(atof(token),"GHz"));
	token = 0; token = strtok(token,","); lacum=lacum+strlen(token)+1; groundtemp.push_back(Temperature(atof(token),"K"));
	token = 0; token = strtok(token,","); lacum=lacum+strlen(token)+1; groundpressure.push_back(Pressure(atof(token),"mb"));
	token = 0; token = strtok(token,","); lacum=lacum+strlen(token)+1; groundhumidity.push_back(Percent(atof(token),"%"));
	token = 0; token = strtok(token,","); lacum=lacum+strlen(token)+1; astrotebb.push_back(Temperature(atof(token),"K"));
	token = 0; token = strtok(token,","); lacum=lacum+strlen(token)+1; astrowater.push_back(Length(atof(token),"mm"));
	token = 0; token = strtok(token,","); lacum=lacum+strlen(token)+1; wvrwater.push_back(Length(atof(token),"mm"));

	for(unsigned int j=0; j<numWVRChannels-1; j++){
	  token = 0; token = strtok(token,","); v_tsky[j]=Temperature(atof(token)+0.0,"K");
	}
	token = 0; token = strtok(token,"\n"); v_tsky[numWVRChannels-1]=Temperature(atof(token),"K");
	singleRadiometerData=WVRMeasurement(aaa,v_tsky);
	RadiometerData.push_back(singleRadiometerData);
      }
      fgets( aRow, STRLEN, fp );
    }
  }
  fclose( fp );

  cout << " ApexTest: Total number of WVR data: " << RadiometerData.size() << endl;
  cout << " ApexTest: Elevation of last measurement: " << RadiometerData[RadiometerData.size()-1].getElevation().get("deg") << " deg" << endl;
  cout << " " << endl;

  unsigned int FirstMeasurementAnalyzed=0;
  unsigned int NumberofMeasurementsAnalyzed=22;

  //  apex_SkyStatus.setUserWH2O(Length(1.18686,"mm"));
  // apex_SkyStatus.setAirMass(1.213137262);   ELV 55.5185
  // apex_SkyStatus.setAirMass(1.063907694);   // ELV 70.04
  // apex_SkyStatus.setAirMass(1.2569);   // ELV 52.71

  /*  cout << "STEP 6: Performing Water Vapor Retrieval over " << NumberofMeasurementsAnalyzed << 
    " WVR measurements starting " << FirstMeasurementAnalyzed/3600 << " on May/18/2002 " << endl; 

  apex_SkyStatus.WaterVaporRetrieval_fromWVR(RadiometerData,FirstMeasurementAnalyzed,FirstMeasurementAnalyzed+NumberofMeasurementsAnalyzed);

  cout << "The average Sigma of this ensemble of fits is: " << 
    apex_SkyStatus.getWVRAverageSigmaTskyFit(RadiometerData,FirstMeasurementAnalyzed,FirstMeasurementAnalyzed+NumberofMeasurementsAnalyzed).get("K") 
       << " K" << endl;
  for(unsigned int i=FirstMeasurementAnalyzed; i<FirstMeasurementAnalyzed+NumberofMeasurementsAnalyzed; i++){
    cout << "Data point analyzed: " << i << "/ Measured (fitted) Sky Tebb's (in K): " << 
      RadiometerData[i].getmeasuredSkyBrightness()[0].get("K") <<"  K ("<< RadiometerData[i].getfittedSkyBrightness()[0].get("K") << " K) "  << 
      RadiometerData[i].getmeasuredSkyBrightness()[1].get("K") <<"  K ("<< RadiometerData[i].getfittedSkyBrightness()[1].get("K") << " K) "  << 
      RadiometerData[i].getmeasuredSkyBrightness()[2].get("K") <<"  K ("<< RadiometerData[i].getfittedSkyBrightness()[2].get("K") << " K) "  << endl;

    cout << " Sigma Fit: " << RadiometerData[i].getSigmaFit().get("K") << " K / Retrieved Water Vapor Column: " << 
      RadiometerData[i].getretrievedWaterVaporColumn().get("mm") << " mm / AstroBand WaterVapor:" << 
      astrowater[i].get("mm") << " mm / Old WVR WaterVapor:" << 
      wvrwater[i].get("mm") << " mm" << endl;

    if(astrosignalfreq[i].get()==astrosignalfreq[i-1].get() && astroimagefreq[i].get()==astroimagefreq[i-1].get() && astrobandwidth[i].get()==astrobandwidth[i-1].get()){

    }else{
      if(astrosignalfreq[i].get()<astroimagefreq[i].get()){
	rff = astrosignalfreq[i];
	if1 = Frequency(0.5*(astroimagefreq[i].get("GHz")-astrosignalfreq[i].get("GHz")),"GHz");
      }else{
	rff = astroimagefreq[i];
	if1 = Frequency(0.5*(astrosignalfreq[i].get("GHz")-astroimagefreq[i].get("GHz")),"GHz");
      }
      chansep = Frequency((astrobandwidth[i].get("GHz")/25.0),"GHz");
      SidebandSide sidebandside_1=LSB; SidebandType sidebandtype_1=DSB;
      apex_SkyStatus.addNewSpectralWindow(25, 13, rff, astrobandwidth[i].get("GHz")/25.0, if1, sidebandside_1, sidebandtype_1);
    }

    jj = apex_SkyStatus.getNumSpectralWindow();
    airm = (1/sin(RadiometerData[i].getElevation().get("rad")-(7.8/180.0)*3.1415927));
    coupling_astroband = 1.00;
    cout << " Sky TEBB from Retrieved Water Vapor Column: " << (apex_SkyStatus.getAverageTebbSky(jj-2,RadiometerData[i].getretrievedWaterVaporColumn(),airm,coupling_astroband,T).get("K")+apex_SkyStatus.getAverageTebbSky(jj-1,RadiometerData[i].getretrievedWaterVaporColumn(),airm,coupling_astroband,T).get("K"))/2.0 << " K" << endl;
    cout << " Sky TEBB measured in ASTROBAND (" <<  astrosignalfreq[i].get("GHz") << " GHz):  " << astrotebb[i].get("K") << " K" << endl;
  } 

  */

  /*  for(unsigned int i=FirstMeasurementAnalyzed; i<FirstMeasurementAnalyzed+NumberofMeasurementsAnalyzed; i++){
    cout << "Measurement analyzed " << i << ": " << RadiometerData[i].getretrievedWaterVaporColumn().get("mm") << " mm" << endl;
} */

  cout << " " << endl;


  cout << " ApexTest: STEP 7: RETRIEVES THE BEST SKY COUPLING AND WATER VAPOR FOR EACH MEASUREMENT INDIVIDUALLY " <<  FirstMeasurementAnalyzed << 
    " TO " << FirstMeasurementAnalyzed+NumberofMeasurementsAnalyzed << endl;


  // apex_SkyStatus.setWaterVaporRadiometer(wvr183ghz);

  for(unsigned int i=FirstMeasurementAnalyzed; i<FirstMeasurementAnalyzed+NumberofMeasurementsAnalyzed; i++){

    if(i>0 && (fabs(groundtemp[i].get("K")-groundtemp[i-1].get("K"))>1.5 ||  fabs(groundpressure[i].get("mb")-groundpressure[i-1].get("mb"))>2.0 )){
      apex_SkyStatus.setBasicAtmosphericParameters(groundtemp[i]);  
      apex_SkyStatus.setBasicAtmosphericParameters(groundpressure[i]);
    }
    



    if(astrosignalfreq[i].get()==astrosignalfreq[i-1].get() && astroimagefreq[i].get()==astroimagefreq[i-1].get() && astrobandwidth[i].get()==astrobandwidth[i-1].get()){
      
    }else{
      if(astrosignalfreq[i].get()<astroimagefreq[i].get()){
	rff = astrosignalfreq[i];
	if1 = Frequency(0.5*(astroimagefreq[i].get("GHz")-astrosignalfreq[i].get("GHz")),"GHz");
      }else{
	rff = astroimagefreq[i];
	if1 = Frequency(0.5*(astrosignalfreq[i].get("GHz")-astroimagefreq[i].get("GHz")),"GHz");
      }
      chansep = Frequency((astrobandwidth[i].get("GHz")/25.0),"GHz");
      SidebandSide sidebandside_1=LSB; SidebandType sidebandtype_1=DSB;
      cout << " ApexTest: addNewSpectralWindow" << endl;
      apex_SkyStatus.addNewSpectralWindow(25, 13, rff, astrobandwidth[i].get("GHz")/25.0, if1, sidebandside_1, sidebandtype_1);
    }
    
    for(unsigned int ii=0; ii<apex_SkyStatus.getNumSpectralWindow(); ii++){
      cout << " ApexTest: " << ii << "  " << apex_SkyStatus.getAssocSpwId(ii)[0] << " " <<  apex_SkyStatus.getMinFreq(ii).get("GHz") 
	   << " GHz " <<  apex_SkyStatus.getMaxFreq(ii).get("GHz") << " GHz " << apex_SkyStatus.getNumChan(ii) << " " 
	   << apex_SkyStatus.getChanFreq(ii,13).get("GHz") << " GHz" <<   endl;
    }


    vector<unsigned int> WVRASTRO_signalId;
    WVRASTRO_signalId.push_back(apex_SkyStatus.getNumSpectralWindow()-2);
    WaterVaporRadiometer wvr_astro(WVRASTRO_signalId,skycouplingastro,signalgainastro,groundtemp[i]); 
    Angle astroelevation(RadiometerData[i].getElevation().get("rad")-(7.8/180.0)*3.1415927,"rad");
    vector<Temperature> astrowvrtebb;
    astrowvrtebb.push_back(astrotebb[i]);
    WVRMeasurement singleAstroRadiometerData(astroelevation,astrowvrtebb);
    vector<WVRMeasurement> astroRadiometerData;
    astroRadiometerData.push_back(singleAstroRadiometerData);



    /*    apex_SkyStatus.setWaterVaporRadiometer(wvr_astro);
    cout << " OK " << endl;
    unsigned int measurementNumber=0;
    apex_SkyStatus.WaterVaporRetrieval_fromWVR(astroRadiometerData,measurementNumber);
    cout << " OK 1" << endl;
    wvr_retrieved_astro = astroRadiometerData[measurementNumber].getretrievedWaterVaporColumn();
    sigma_wvr_retrieved_astro = apex_SkyStatus.getWVRAverageSigmaTskyFit(astroRadiometerData,measurementNumber);
    cout << wvr_retrieved_astro.get("mm") << "   " << sigma_wvr_retrieved_astro.get("mm") << endl;
    */


   
    apex_SkyStatus.setWaterVaporRadiometer(wvr183ghz);
    apex_SkyStatus.updateSkyCoupling_fromWVR(RadiometerData,i,wvr183ghz);
    apex_SkyStatus.WaterVaporRetrieval_fromWVR(RadiometerData,i);



    jj = apex_SkyStatus.getNumSpectralWindow();
    airm = (1/sin(RadiometerData[i].getElevation().get("rad")-(7.8/180.0)*3.1415927));
    airmwvr = (1/sin(RadiometerData[i].getElevation().get("rad")));
    //    cout << airm << endl;
    coupling_astroband = 1.00;
    //    cout << " Sky TEBB from Retrieved Water Vapor Column: " << (apex_SkyStatus.getAverageTebbSky(jj-2,RadiometerData[i].getretrievedWaterVaporColumn(),airm,coupling_astroband,T).get("K")+apex_SkyStatus.getAverageTebbSky(jj-1,RadiometerData[i].getretrievedWaterVaporColumn(),airm,coupling_astroband,T).get("K"))/2.0 << " K" << endl;
    //    cout << " Sky TEBB measured in ASTROBAND (" <<  astrosignalfreq[i].get("GHz") << " GHz):  " << astrotebb[i].get("K") << " K" << endl;

    wvr_retrieved = RadiometerData[i].getretrievedWaterVaporColumn();

    if(i==0){opacitycomparison=(apex_SkyStatus.getAverageTotalOpacity(0).get()+
				apex_SkyStatus.getAverageTotalOpacity(1).get())/2.0;}

    double tebb_astro = (apex_SkyStatus.getAverageTebbSky(jj-2,wvr_retrieved,airm,coupling_astroband,groundtemp[i]).get("K")+
		  apex_SkyStatus.getAverageTebbSky(jj-1,wvr_retrieved,airm,coupling_astroband,groundtemp[i]).get("K"))/2.0 ;

    double trj_astro= (0.04799274551*(astrosignalfreq[i].get("GHz")+6.0))/(exp(0.04799274551*(astrosignalfreq[i].get("GHz")+6.0)/tebb_astro)-1.0);


    if(i==0){
      cout << " ApexTest: ----------------------------------------------------" <<  endl;
      cout << " ApexTest: day fraction: " << time_mjd[i]-53965.0 <<  endl;
      cout <<         " ApexTest: WVR elevation: " << elevation[i].get("deg") <<  endl;
      cout <<         " ApexTest: ASTRO Signal Freq.:" << astrosignalfreq[i].get("GHz") <<  endl;
      cout <<         " ApexTest: WVR Sky Coupling: " <<  apex_SkyStatus.getWaterVaporRadiometerSkyCoupling(0) <<  endl;
      cout <<         " ApexTest: WVR Spillover Temp.:" << apex_SkyStatus.getWaterVaporRadiometer().getSpilloverTemperature().get() <<  endl;
      cout <<         " ApexTest: WVR water: " << wvr_retrieved.get("mm") <<  endl;
      cout <<         " ApexTest: Sigmafit (K): " << apex_SkyStatus.getWVRAverageSigmaTskyFit(RadiometerData,i).get("K") <<  endl;
      cout <<         " ApexTest: Pr. Astro TEBB: " <<  tebb_astro <<  endl;
      cout <<         " ApexTest: Pr. Astro TRJ: " <<  trj_astro <<  endl;
      cout <<         " ApexTest: coup. ASTRO: " << coupling_astroband <<  endl;
      cout <<         " ApexTest: Gr. Temp: " << groundtemp[i].get("K") <<  endl;
      cout <<         " ApexTest: Meas. ASTRO Tebb: " <<  astrotebb[i].get("K") << endl; 
      cout <<         " ApexTest: Measured WWR Channel 1: " << RadiometerData[i].getmeasuredSkyBrightness()[0].get("K") <<endl;
      cout <<         " ApexTest: Measured WWR Channel 2: " << RadiometerData[i].getmeasuredSkyBrightness()[1].get("K") <<endl;
      cout <<         " ApexTest: Measured WWR Channel 3: " << RadiometerData[i].getmeasuredSkyBrightness()[2].get("K") <<endl;

    }




    cout << " ApexTest: " << time_mjd[i]-53965.0 << " " << elevation[i].get("deg") << " " << astrosignalfreq[i].get("GHz") << " " <<
      apex_SkyStatus.getWaterVaporRadiometerSkyCoupling(0) << " " << 
      apex_SkyStatus.getWaterVaporRadiometer().getSpilloverTemperature().get() << " " << 
      wvr_retrieved.get("mm") << " " << apex_SkyStatus.getWVRAverageSigmaTskyFit(RadiometerData,i).get("K") << " " <<  
      tebb_astro << " " <<  trj_astro << " " << coupling_astroband << " " << 
      groundtemp[i].get("K") << " " <<  astrotebb[i].get("K") << " " << 
      //      wvr_retrieved_astro.get("mm") << " " <<
      //      sigma_wvr_retrieved_astro.get("K") << " " <<
      RadiometerData[i].getmeasuredSkyBrightness()[0].get("K") << " " <<
      RadiometerData[i].getmeasuredSkyBrightness()[1].get("K") << " " <<
      RadiometerData[i].getmeasuredSkyBrightness()[2].get("K") << " " << endl;




     /* cout << (RadiometerData[i].getmeasuredSkyBrightness())[0].get("K") << "  " << (RadiometerData[i].getfittedSkyBrightness())[0].get("K") << endl;
    cout << (RadiometerData[i].getmeasuredSkyBrightness())[1].get("K") << "  " << (RadiometerData[i].getfittedSkyBrightness())[1].get("K") << endl;
    cout << (RadiometerData[i].getmeasuredSkyBrightness())[2].get("K") << "  " << (RadiometerData[i].getfittedSkyBrightness())[2].get("K") << endl;
    */


    /*
    cout << "Time (days after 53887)=" << time_mjd[i]-53887.0 << " " << endl;
     cout <<  "Elevation(deg)=" << elevation[i].get("deg") << " " << endl;
     cout <<  "AstroFreq=" << astrosignalfreq[i].get("GHz") << " " <<endl;
     cout <<  "WVR_Coupling=" << apex_SkyStatus.getWaterVaporRadiometerSkyCoupling(0) << " " <<endl;
     cout <<  "WVR_Spillover=" << apex_SkyStatus.getWaterVaporRadiometer().getSpilloverTemperature().get() << " " <<endl;
    cout <<   "WVR_Watercolumn=" << wvr_retrieved.get("mm") << " " << endl;
    cout <<   "WVR_SigmaFit=" << apex_SkyStatus.getWVRAverageSigmaTskyFit(RadiometerData,i).get("K") << " " << endl;
    cout <<   "TEBB_ASTRO_fromWVR=" << (apex_SkyStatus.getAverageTebbSky(jj-2,wvr_retrieved,airm,coupling_astroband,groundtemp[i]).get("K")+apex_SkyStatus.getAverageTebbSky(jj-1,wvr_retrieved,airm,coupling_astroband,groundtemp[i]).get("K"))/2.0 << "  " << endl;
    cout <<   "coupling_astroband=" << coupling_astroband << " " << endl;
    cout <<   "TSPILL_ASTRO=" << groundtemp[i].get("K") << " " << endl;
    cout <<   "TEBB_ASTRO measured=" << astrotebb[i].get("K") << endl;
    cout << endl;
    cout << endl;
    */


  } 


  
  return 0;

}
