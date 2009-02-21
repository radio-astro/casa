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
#define STRLEN  40        // Max length of a row in a tpoint file
using namespace atm;

int main()
{  
  cout << " WVRRetrievalTest: " << endl;
  cout << " WVRRetrievalTest:  THIS PROPOSED TEST OF THE ATM INTERFACE SOFTWARE IS BASED ON THE SIDE-BY-SIDE COMPARISON " << endl;
  cout << " WVRRetrievalTest:  OF FOURIER TRANSFORM SPECTROSCOPY AND WATER VAPOR RADIOMETRY, PERFORMED AT MAUNA KEA ON  " << endl;
  cout << " WVRRetrievalTest:  MARCH 3, 2002, THAT WAS PUBLISHED IN THE ASTROPHYSICAL JOURNAL SUPPL. SER., 153:363-367  " << endl;
  cout << " WVRRetrievalTest:          To better understand the example you can download the reference from:            " << endl;
  cout << " WVRRetrievalTest:           http://damir.iem.csic.es/PARDO/publications.html (paper 34)                     " << endl;
  cout << " WVRRetrievalTest:  " << endl;

  cout << " WVRRetrievalTest: STEP 1: CREATES REFERENCE ATMOSPHERIC PROFILE CORRESTONDIGN TO THE FOLLOWING BASIC PARAMETERS:" << endl;
  cout << " WVRRetrievalTest: Atmosphere Type: TROPICAL" << endl;
  cout << " WVRRetrievalTest: Site Altitude: 4100 m above sea level" << endl; 
  cout << " WVRRetrievalTest: Ground Temperature: 268.15 K" << endl;
  cout << " WVRRetrievalTest: Ground Pressure: 623.0 mb" << endl;
  cout << " WVRRetrievalTest: Relative Humidity at ground level: 11.3 % " << endl;
  cout << " WVRRetrievalTest: Water vapor scale height: 2.2 km" << endl;
  cout << " WVRRetrievalTest: Tropospheric Temperature lapse rate: -5.6 K/km" << endl;
  cout << " WVRRetrievalTest: Top of the atmosphere for calculations: 48 km" << endl;
  cout << " WVRRetrievalTest: Primary pressure step: 10 mb (please check the above reference)" << endl;
  cout << " WVRRetrievalTest: Pressure step factor: 1.2 (please check the above reference)" << endl;

  Atmospheretype   atmType = tropical; // Atmospheric type (to reproduce behavior above the tropopause)
  Temperature      T( 268.15,"K" );    // Ground temperature
  Pressure         P( 623.0,"mb");     // Ground Pressure
  Humidity         H(  11.30,"%" );    // Ground Relative Humidity (indication)
  Length         Alt(  4100,"m" );     // Altitude of the site 
  Length         WVL(   2.2,"km");     // Water vapor scale height
  double         TLR=  -5.6      ;     // Tropospheric lapse rate (must be in K/km)
  Length      topAtm(  48.0,"km");     // Upper atm. boundary for calculations
  Pressure     Pstep(  10.0,"mb");     // Primary pressure step (10.0 mb)
  double   PstepFact=         1.2;     // Pressure step ratio between two consecutive layers

  AtmProfile myProfile( Alt, P, T, TLR, H, WVL, Pstep, PstepFact,  topAtm, atmType );

  cout << " WVRRetrievalTest: First guess precipitable water vapor content: " << myProfile.getGroundWH2O().get("mm") << "mm" << endl;
  cout << " WVRRetrievalTest:  " << endl;

  cout << " WVRRetrievalTest: STEP 2: CREATES SpectralGrid and RefractiveIndexProfile OBJECTS CONTAINING" << endl;
  cout << " WVRRetrievalTest:       3 WATER VAPOR RADIOMETER CHANNELS + 1 ASTRONOMICAL BAND       " << endl; 
  cout << " WVRRetrievalTest:         (Negative frequency resolutions are used for image side bands)    " << endl; 

  vector<unsigned int> WVR_signalId;    // IDs of the Signal Side Band of each Water Vapor Radiometer Channel in the SpectralGrid object. 

  unsigned int numchan1=11;  unsigned int refchan1=6;
  Frequency reffreq1(182.11,"GHz"); Frequency chansep1(  0.04,"GHz"); Frequency intfreq1(  1.20,"GHz");
  SidebandSide sidebandside1=LSB; SidebandType sidebandtype1=DSB;
  SpectralGrid alma_SpectralGrid(numchan1, refchan1, reffreq1, chansep1, intfreq1, sidebandside1, sidebandtype1); 
  WVR_signalId.push_back(0); // this is the Id of the 1st spectral window of the 1st pair 

  RefractiveIndexProfile alma_RefractiveIndexProfile(alma_SpectralGrid, myProfile);

  unsigned int numchan2=11;  unsigned int refchan2=6;
  Frequency reffreq2(179.11,"GHz"); Frequency chansep2(  0.09,"GHz"); Frequency intfreq2(  4.20,"GHz");
  SidebandSide sidebandside2=LSB; SidebandType sidebandtype2=DSB;
  WVR_signalId.push_back(alma_RefractiveIndexProfile.getNumSpectralWindow());  // this will be the Id of the 1st spw of the 2nd pair 
  alma_RefractiveIndexProfile.addNewSpectralWindow(numchan2, refchan2, reffreq2, chansep2, intfreq2, sidebandside2, sidebandtype2); 

  unsigned int numchan3=11; unsigned int refchan3=6;
  Frequency reffreq3(175.51,"GHz"); Frequency chansep3(  0.10,"GHz"); Frequency intfreq3(  7.80,"GHz");
  SidebandSide sidebandside3=LSB; SidebandType sidebandtype3=DSB;
  WVR_signalId.push_back(alma_RefractiveIndexProfile.getNumSpectralWindow()); 
  alma_RefractiveIndexProfile.addNewSpectralWindow(numchan3, refchan3, reffreq3, chansep3, intfreq3, sidebandside3, sidebandtype3); 

  Frequency  mySingleFreq_astro2(411,"GHz");  Frequency  chanSep_astro2(0.050,"GHz");
  unsigned int numChan_astro2=11;  unsigned int refChan_astro2=6; 


  alma_RefractiveIndexProfile.addNewSpectralWindow(numChan_astro2, refChan_astro2, mySingleFreq_astro2, chanSep_astro2); 


  for(unsigned int j=0; j<alma_RefractiveIndexProfile.getNumSpectralWindow(); j++){
    cout << " WVRRetrievalTest: Spectral Window " << j 
	 << " Central Frequency: " <<  alma_RefractiveIndexProfile.getRefFreq(j).get("GHz") << " GHz, "
	 << " Freq. Resolution: " <<  alma_RefractiveIndexProfile.getChanSep(j).get("MHz") << " MHz, "
	 << " Num. of channels: " << alma_RefractiveIndexProfile.getNumChan(j)    << endl;
  }
  cout << " " << endl;

  cout << " WVRRetrievalTest: Spectral windows associations:    " << endl; 
  for(unsigned int j=0; j<alma_RefractiveIndexProfile.getNumSpectralWindow(); j++){
    if(alma_RefractiveIndexProfile.getAssocSpwId(j).size()==1){
      cout << " WVRRetrievalTest: Spectral Window " << j << " associated to spectral window: " 
	   <<  alma_RefractiveIndexProfile.getAssocSpwId(j)[0] << " (double band)" << endl; 
    }else{
      cout << " WVRRetrievalTest: Spectral Window " << j << " associated to spectral window: " 
	   <<  j << " (single band)" << endl; 
    }
  }
  cout << " WVRRetrievalTest:  " << endl;


 

  cout << " WVRRetrievalTest: STEP 4: CREATES SkyStatus object and associates a WaterVaporRadiometer to it " << endl; 

  SkyStatus skyAntenna1(alma_RefractiveIndexProfile);
  vector<double> skycoupling183; // Sky couplings the WVR Channels (set to 0.7 below)
  vector<Percent> signalgain183; // Signal Side Band Gain of the WVR Channels (set to 50% below)
  double skyCoupling_1stGuess = 0.8;
  Temperature tspill(285.15,"K"); // Spillover Temperature see by the WVR 
  for(unsigned int i=0; i<WVR_signalId.size(); i++){
    skycoupling183.push_back(skyCoupling_1stGuess); 
    signalgain183.push_back(Percent(50.0,"%"));
  }
  WaterVaporRadiometer wvr183ghz(WVR_signalId,skycoupling183,signalgain183,tspill); 
  skyAntenna1.setWaterVaporRadiometer(wvr183ghz);

  cout << " WVRRetrievalTest:  " << endl;

  cout << " WVRRetrievalTest: WaterVaporRadiometer characteristics: " << endl;
  unsigned int Ids;
  for(unsigned int i=0; i<skyAntenna1.getWaterVaporRadiometer().getIdChannels().size(); i++){
    Ids=skyAntenna1.getWaterVaporRadiometer().getIdChannels()[i];
    cout << " WVRRetrievalTest: WVR Channel " << i << " SpectralGrid Id of Signal sideband: " << 
      Ids << " / SpectralGrid Id of Image sideband: " << skyAntenna1.getAssocSpwId(Ids)[0] << endl;
    cout << " WVRRetrievalTest: " << " Sky Coupling: " << skyAntenna1.getWaterVaporRadiometerSkyCoupling(i) << 
      " / Gain of Signal sideband: " << skyAntenna1.getWaterVaporRadiometerSignalGain(i).get("%") << " %" << endl; 
  }

  cout << " WVRRetrievalTest:  " << endl;
  cout << " WVRRetrievalTest: STEP 5: Reads a block of real WVRMeasurements taken at Mauna Kea on March 3, 2002 (local code)" << endl;

  vector<WVRMeasurement> RadiometerData;
  WVRMeasurement singleRadiometerData;
  vector<double> time;
  Angle aaa;
  FILE*  fp;
  fp = fopen("../test/radiometer_data.dat", "r");
  if (fp != 0) {
    char  aRow[STRLEN+1];
    char* token;
    vector<Temperature> v_tsky;
    unsigned int numWVRChannels = 0;
    fgets( aRow, STRLEN, fp );
    unsigned int inilen=strlen(aRow);
    unsigned int lacum=0;
    token = strtok(aRow,","); time.push_back(atof(token));
    lacum=lacum+strlen(token)+1;
    token = 0; token = strtok(token,","); aaa=Angle(atof(token),"deg"); 
    lacum=lacum+strlen(token)+1;
    while (lacum<=inilen){
      numWVRChannels++;
      token = 0; token = strtok(token,","); Temperature tt(atof(token),"K"); v_tsky.push_back(tt); 
      lacum=lacum+strlen(token)+1;
    }
    singleRadiometerData=WVRMeasurement(aaa,v_tsky);
    RadiometerData.push_back(singleRadiometerData);
    fgets( aRow, STRLEN, fp );
    while (feof(fp)==0){
      if(strncmp(aRow," ",1)==0){
	token = strtok(aRow,","); time.push_back(atof(token));
	token = 0; token = strtok(token,","); aaa=Angle(atof(token),"deg"); 
	for(unsigned int j=0; j<numWVRChannels-1; j++){
	  token = 0; token = strtok(token,","); v_tsky[j]=Temperature(atof(token),"K");
	}
	token = 0; token = strtok(token,"\n"); v_tsky[numWVRChannels-1]=Temperature(atof(token),"K");
      }
      singleRadiometerData=WVRMeasurement(aaa,v_tsky);
      RadiometerData.push_back(singleRadiometerData);
      fgets( aRow, STRLEN, fp );
    }
  }
  fclose( fp );

  cout << " WVRRetrievalTest: Total number of WVR data: " << RadiometerData.size() << endl;
  cout << " WVRRetrievalTest:  " << endl;

  unsigned int FirstMeasurementAnalyzed=9300;
  unsigned int NumberofMeasurementsAnalyzed=1; // 7
  unsigned int NumberofMeasurementsforSkyCoupligRetrieval=5;

  
  cout << " WVRRetrievalTest: STEP 6: Performing Water Vapor Retrieval over " << NumberofMeasurementsAnalyzed << 
    " WVR measurements starting at " << FirstMeasurementAnalyzed/3600 << " UT on March/3/2002 " << endl; 

  skyAntenna1.WaterVaporRetrieval_fromWVR(RadiometerData,FirstMeasurementAnalyzed,FirstMeasurementAnalyzed+NumberofMeasurementsAnalyzed);
  cout << " WVRRetrievalTest: The average Sigma of this ensemble of fits is: " << 
    skyAntenna1.getWVRAverageSigmaTskyFit(RadiometerData,FirstMeasurementAnalyzed,FirstMeasurementAnalyzed+NumberofMeasurementsAnalyzed).get("K") 
       << " K" << endl;
  cout << " WVRRetrievalTest: User Water Vapor Column: " << skyAntenna1.getUserWH2O().get("mm") << " mm" << endl;

  cout << " WVRRetrievalTest:  " << endl;
  

  cout << " WVRRetrievalTest: STEP 7: RETRIEVES THE BEST SKY COUPLING OF THE WVR CHANNELS USING DATA " <<  FirstMeasurementAnalyzed << 
    " TO " << FirstMeasurementAnalyzed+NumberofMeasurementsforSkyCoupligRetrieval << endl;

  skyAntenna1.updateSkyCoupling_fromWVR(RadiometerData,FirstMeasurementAnalyzed,FirstMeasurementAnalyzed+NumberofMeasurementsforSkyCoupligRetrieval); 

  cout << " WVRRetrievalTest: The best sky coupling is: " << endl;
  for(unsigned int i=0; i<skyAntenna1.getWaterVaporRadiometer().getIdChannels().size(); i++){
    cout << " WVRRetrievalTest: WVR Channel " << i << ": " << skyAntenna1.getWaterVaporRadiometerSkyCoupling(i) << ": " << endl;
  } 
  cout << " WVRRetrievalTest: For which the AverageSigmaTskyFit over those measurements is: " << 
    skyAntenna1.getWVRAverageSigmaTskyFit(RadiometerData,FirstMeasurementAnalyzed,FirstMeasurementAnalyzed+NumberofMeasurementsforSkyCoupligRetrieval).get("K") 
       << " K" << endl;
  cout << " WVRRetrievalTest: User Water Vapor Column: " << skyAntenna1.getUserWH2O().get("mm") << " mm" << endl;
  cout << " " << endl;


  cout << " WVRRetrievalTest: STEP 8: DATA ANALYSIS USING THE NEW SKY COUPLING " << endl;

  skyAntenna1.WaterVaporRetrieval_fromWVR(RadiometerData,FirstMeasurementAnalyzed,FirstMeasurementAnalyzed+NumberofMeasurementsAnalyzed);

  for(unsigned int i=FirstMeasurementAnalyzed; i<FirstMeasurementAnalyzed+NumberofMeasurementsAnalyzed; i++){
    cout << " WVRRetrievalTest: Data point " << i << " (UT time: " << time[i]/3600 << " hours on 2002, March 3)" << endl; 
    cout << " WVRRetrievalTest: Measured and fitted Sky Tebb's (in K): " << endl;
    for(unsigned int j=0; j<RadiometerData[i].getmeasuredSkyBrightness().size(); j++){
      cout << " WVRRetrievalTest:   Channel " << j << ": " << RadiometerData[i].getmeasuredSkyBrightness()[j].get("K") << 
	"  "<< RadiometerData[i].getfittedSkyBrightness()[j].get("K") << endl;
    }
    cout << " WVRRetrievalTest:   Retrieved Zenith Water Vapor Column: " << RadiometerData[i].getretrievedWaterVaporColumn().get("mm") << " mm /" 
	 << " Sigma Fit: " << RadiometerData[i].getSigmaFit().get("K") << " K" << endl;


    skyAntenna1.setUserWH2O(Length(2.0,"mm"));

    cout << " WVRRetrievalTest: Alternative method: Retrieved Zenith Water Vapor Column: " <<
      skyAntenna1.WaterVaporRetrieval_fromTEBB(WVR_signalId,Percent(50.0,"%"),RadiometerData[i].getmeasuredSkyBrightness(),
					       RadiometerData[i].getAirMass(),skyAntenna1.getWaterVaporRadiometerSkyCoupling(0),
					       Temperature(285.15,"K")).get("mm") << " mm" << endl;

  }

 

  cout << " WVRRetrievalTest: STEP 9: Now comparing the results of the WVR analysis with a simultaneous FTS Spectrum " << endl;
  cout << " WVRRetrievalTest: Reading FTS data file..." << endl;

  vector<Frequency> v_frequencyFTS;
  vector<double> v_transmissionFTS;


  fp = fopen("../test/fts2002.dat", "r");
  if (fp != 0) {
    char  aRow[STRLEN+1];
    char* token;
    fgets( aRow, STRLEN, fp );
    token = strtok(aRow,","); v_frequencyFTS.push_back(Frequency(atof(token),"MHz"));
    token = 0; token = strtok(token,","); v_transmissionFTS.push_back(atof(token));
    fgets( aRow, STRLEN, fp );
    while (feof(fp)==0){
      if(strncmp(aRow," ",1)==0){
	token = strtok(aRow,",");  v_frequencyFTS.push_back(Frequency(atof(token),"MHz"));
	token = 0; token = strtok(token,",");  v_transmissionFTS.push_back(atof(token));
      }
      fgets( aRow, STRLEN, fp );
    }    
  }
  fclose( fp );

  cout << " WVRRetrievalTest: ...done!" << endl;

  skyAntenna1.addNewSpectralWindow(v_frequencyFTS);
  unsigned int spwid=skyAntenna1.getNumSpectralWindow()-1;

  cout << " WVRRetrievalTest: Number of spectral windows: " << skyAntenna1.getNumSpectralWindow() << endl;
  cout << " WVRRetrievalTest: Number of FTS frequencies:  " << skyAntenna1.getNumChan(spwid) << endl;

  cout << " WVRRetrievalTest: Performing Water Vapor Retrieval" << endl;

  cout << " WVRRetrievalTest: Retrieved zenith water vapor column from data between 450 and 920 GHz: " << 
    skyAntenna1.WaterVaporRetrieval_fromFTS(spwid,v_transmissionFTS,Frequency(450.0,"GHz"),
    Frequency(920.0,"GHz")).get("mm") << " mm " << endl;  

  cout << " WVRRetrievalTest: FTS sigma fit (in terms of transmission for those channels where the fit has been performed): " << 
    skyAntenna1.getSigmaTransmissionFit(spwid,v_transmissionFTS,1.0,Frequency(450.0,"GHz"),Frequency(920.0,"GHz")) << endl;

  cout << " WVRRetrievalTest: Comparison of Measurement vs fit in channel num. 600 of FTS scan: " << v_frequencyFTS[600].get("GHz") 
       << " GHz" << endl;
  cout << " WVRRetrievalTest:               Meas. transmission:  " << v_transmissionFTS[600] << "   Fitted transmission: " << 
    exp(-1.0*(skyAntenna1.getWetOpacity(spwid,600).get()+skyAntenna1.getDryOpacity(spwid,600).get()))   << endl;
  

  cout << " WVRRetrievalTest: Reading TEBB data file..." << endl;

  vector<Frequency> v_frequency_tebbspec;
  vector<Temperature> v_tebbspec;

  fp = fopen("../test/tebb_spec_withnoise_withspillover.dat", "r");
  // fp = fopen("../test/tebb_spec_withnois1.dat", "r");
 if (fp != 0) {
    char  aRow[STRLEN+1];
    char* token;
    fgets( aRow, STRLEN, fp );
    token = strtok(aRow,","); v_frequency_tebbspec.push_back(Frequency(atof(token),"GHz"));
    token = 0; token = strtok(token,","); v_tebbspec.push_back(Temperature(atof(token),"K"));
    fgets( aRow, STRLEN, fp );
    while (feof(fp)==0){
      if(strncmp(aRow," ",1)==0){
	token = strtok(aRow,",");  v_frequency_tebbspec.push_back(Frequency(atof(token),"GHz"));
	token = 0; token = strtok(token,",");  v_tebbspec.push_back(Temperature(atof(token),"K"));
      }
      fgets( aRow, STRLEN, fp );
    }    
  }
  fclose( fp );

  cout << " WVRRetrievalTest: ...done!" << endl;


  skyAntenna1.addNewSpectralWindow(v_frequency_tebbspec);
  skyAntenna1.setUserWH2O(Length(2.5,"mm"));
  spwid=skyAntenna1.getNumSpectralWindow()-1;


  // skyAntenna1.setAirMass(1.41);


  cout << " WVRRetrievalTest: Number of spectral windows: " << skyAntenna1.getNumSpectralWindow() << endl;
  cout << " WVRRetrievalTest: Number of TEBB frequencies:  " << skyAntenna1.getNumChan(skyAntenna1.getNumSpectralWindow()-1) << endl;
  
  cout << " WVRRetrievalTest: Average Dry Opacity:  " << skyAntenna1.getAverageDryOpacity(skyAntenna1.getNumSpectralWindow()-1).get() << endl;
  cout << " WVRRetrievalTest: Average TEBB       :  " << skyAntenna1.getAverageTebbSky(skyAntenna1.getNumSpectralWindow()-1).get() << endl;

  cout << " WVRRetrievalTest: Performing Water Vapor Retrieval" << endl;

  cout << " WVRRetrievalTest: Retrieved zenith water vapor column from TEBB data using a 1.0 sky coupling: " << 
    skyAntenna1.WaterVaporRetrieval_fromTEBB(spwid,v_tebbspec,1.0,Temperature(268.15,"K")).get("mm") << " mm " << endl;  

  vector<Frequency> v_frequency_tebbspec_1;
  vector<Temperature> v_tebbspec_1;
  vector<Frequency> v_frequency_tebbspec_2;
  vector<Temperature> v_tebbspec_2;


  for(unsigned int i=0; i<300; i++){
    v_frequency_tebbspec_1.push_back(v_frequency_tebbspec[i]);
    v_tebbspec_1.push_back(v_tebbspec[i]);
  }


  for(unsigned int i=0; i<skyAntenna1.getNumChan(skyAntenna1.getNumSpectralWindow()-1)-300; i++){
    v_frequency_tebbspec_2.push_back(v_frequency_tebbspec[i+300]);
    v_tebbspec_2.push_back(v_tebbspec[i+300]);
  }


  skyAntenna1.addNewSpectralWindow(v_frequency_tebbspec_1);
  skyAntenna1.addNewSpectralWindow(v_frequency_tebbspec_2);


  vector<unsigned int> spwId0;
  vector<vector<Temperature> > vv_tebb0;  
  vector<double> skycoupling0;
  vector<Temperature> tspill0;

  spwId0.push_back(skyAntenna1.getNumSpectralWindow()-2);
  spwId0.push_back(skyAntenna1.getNumSpectralWindow()-1);

  cout << " WVRRetrievalTest: Number of spectral windows: " << skyAntenna1.getNumSpectralWindow() << endl;

  vv_tebb0.push_back(v_tebbspec_1);
  vv_tebb0.push_back(v_tebbspec_2);

  skycoupling0.push_back(1.0);
  skycoupling0.push_back(1.0);

  tspill0.push_back(Temperature(268.15,"K"));
  tspill0.push_back(Temperature(268.15,"K"));

  cout << " WVRRetrievalTest: 2nd method: " << 
    skyAntenna1.WaterVaporRetrieval_fromTEBB(spwId0, vv_tebb0,  skycoupling0, tspill0).get("mm") << " mm " << endl; 


   

  cout << " WVRRetrievalTest: FTS sigma fit (in terms of Temperature for those channels where the fit has been performed): " << 
    skyAntenna1.getSigmaFit(spwid,v_tebbspec,1.0,Temperature(268.15,"K")).get("K") << " K" << endl;

  unsigned int channum=400;
  cout << " WVRRetrievalTest: Comparison of Measurement vs fit in channel num. " << channum << " of TEBB spectrum at : " << v_frequency_tebbspec[channum].get("GHz") << " GHz" << endl;
  cout << " WVRRetrievalTest: Meas. TEBB:  " << v_tebbspec[channum].get("K") << " K" << "   Fitted TEBB: "  << 
    skyAntenna1.getTebbSky(spwid,channum).get("K") << " K" <<   endl;

  double retrievedSkyCoupling=skyAntenna1.SkyCouplingRetrieval_fromTEBB(spwid,v_tebbspec,0.8,Temperature(268.15,"K"));
  cout << " WVRRetrievalTest: Retrieved sky coupling: " << retrievedSkyCoupling << endl;
  cout << " WVRRetrievalTest: Retrieved zenith water vapor: " << skyAntenna1.getUserWH2O().get("mm") << " mm" << endl;
  cout << " WVRRetrievalTest: Sigma fit (in terms of Temperature for those channels where the fit has been performed): " << 
    skyAntenna1.getSigmaFit(spwid,v_tebbspec,retrievedSkyCoupling,Temperature(268.15,"K")).get("K") << " K" << endl;

  cout << " WVRRetrievalTest: Comparison of Measurement vs fit in channel num. 400 of TEBB spectrum at : " << v_frequency_tebbspec[channum].get("GHz") 
       << " GHz" << endl;


  cout << " WVRRetrievalTest:               Meas. TEBB:  " << v_tebbspec[channum].get("K") << " K" << "   Fitted TEBB: " << 
    skyAntenna1.getTebbSky(spwid,channum,retrievedSkyCoupling,Temperature(268.15,"K")).get("K") << " K" <<   endl;

  retrievedSkyCoupling=skyAntenna1.SkyCouplingRetrieval_fromTEBB(spwid,v_tebbspec,1.41,0.8,Temperature(268.15,"K"));
  cout << " WVRRetrievalTest: Retrieved sky coupling: " << retrievedSkyCoupling << endl;
  cout << " WVRRetrievalTest: Retrieved zenith water vapor: " << skyAntenna1.getUserWH2O().get("mm") << " mm" << endl;
  cout << " WVRRetrievalTest: Sigma fit (in terms of Temperature for those channels where the fit has been performed): " << 
    skyAntenna1.getSigmaFit(spwid,v_tebbspec,1.41,retrievedSkyCoupling,Temperature(268.15,"K")).get("K") << " K" << endl;

  cout << " WVRRetrievalTest: Comparison of Measurement vs fit in channel num. 400 of TEBB spectrum at : " << v_frequency_tebbspec[channum].get("GHz") 
       << " GHz" << endl;
  cout << " WVRRetrievalTest:               Meas. TEBB:  " << v_tebbspec[400].get("K") << " K" << "   Fitted TEBB: " << 
    skyAntenna1.getTebbSky(spwid,channum,1.41,retrievedSkyCoupling,Temperature(268.15,"K")).get("K") << " K" <<   endl;

   

  return 0;

}


