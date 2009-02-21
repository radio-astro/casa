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


  Atmospheretype   atmType = tropical; // Atmospheric type (to reproduce behavior above the tropopause)
  Temperature      T( 268.15,"K" );    // Ground temperature
  Pressure         P( 623.0,"mb");     // Ground Pressure
  Humidity         H(  11.30,"%" );    // Ground Relative Humidity (indication)
  Length         Alt(  4100,"m" );     // Altitude of the site 
  Length         WVL(   2.2,"km");     // Water vapor scale height
  double         TLR=  -5.6      ;     // Tropospheric lapse rate (must be in K/km)
  Length      topAtm(  48.0,"km");     // Upper atm. boundary for calculations
  Pressure     Pstep(  10.0,"mb");     // Primary pressure step (10.0 mb)
  double   PstepFact=         1.5;     // Pressure step ratio between two consecutive layers

  AtmProfile MaunaKeaProfile( Alt, P, T, TLR, H, WVL, Pstep, PstepFact,  topAtm, atmType );

  cout << " FTSRetrievalTest: 1st guess water vapor" << MaunaKeaProfile.getGroundWH2O().get("mm") << endl;

  cout << " FTSRetrievalTest: Reading FTS data file..." << endl;

  vector<Frequency> v_frequencyFTS;
  vector<double> v_transmissionFTS;


  FILE*  fp;
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

  cout << " FTSRetrievalTest: ...done " << endl;
  
  cout << " FTSRetrievalTest: Creating SpectralGrid object..." << endl;
  SpectralGrid  FTSFreqs(v_frequencyFTS);
  cout << " FTSRetrievalTest: ...done" << endl;
  
  cout << " FTSRetrievalTest: Creating  RefractiveIndexProfile object...(this is the time consuming " << 
    "part because absoprtion coefficients for " << v_frequencyFTS.size() << 
    " different frequencies and " << MaunaKeaProfile.getNumLayer() << 
    " atmospheric layers have to be calculated."  << endl;

  RefractiveIndexProfile fts_RefractiveIndexProfile(FTSFreqs, MaunaKeaProfile);

  cout << " FTSRetrievalTest: ...done" << endl;

  cout << " FTSRetrievalTest: Creating  SkyStatus object..." << endl;
  SkyStatus fts1(fts_RefractiveIndexProfile); 
  cout << " FTSRetrievalTest: ...done" << endl;

  cout << " FTSRetrievalTest: Number of spectral windows: " << fts1.getNumSpectralWindow() << endl;
  cout << " FTSRetrievalTest: Number of FTS frequencies:  " << fts1.getNumChan(0) << endl;
  
  cout << " FTSRetrievalTest: Performing Water Vapor Retrieval" << endl;

  cout << " FTSRetrievalTest: Retrieved zenith water vapor column from data between 450 and 920 GHz: " << 
    fts1.WaterVaporRetrieval_fromFTS(v_transmissionFTS,Frequency(450.0,"GHz"),
				     Frequency(920.0,"GHz")).get("mm") << " mm " << endl;
  cout << " FTSRetrievalTest: Retrieved zenith water vapor column using the whole spectrum:          " << 
    fts1.WaterVaporRetrieval_fromFTS(v_transmissionFTS).get("mm") << " mm " << endl;

  return 0;

}


