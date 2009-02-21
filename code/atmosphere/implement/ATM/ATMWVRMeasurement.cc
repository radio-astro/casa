using namespace std;

#include <vector>
#include <string>
#include <stdio.h>
#include <iostream>


#include <atmosphere/ATM/ATMWVRMeasurement.h>



namespace atm {
  
  WVRMeasurement::WVRMeasurement(){}
  
  WVRMeasurement::WVRMeasurement(Angle elevation, vector<Temperature> measuredSkyBrightness){
    elevation_ = elevation;
    v_measuredSkyBrightness_ = measuredSkyBrightness;
    retrievedWaterVaporColumn_ = Length(-999,"mm");           // !< Retrieved zenith water vapor column for each event  
    sigma_fittedSkyBrightness_ = Temperature(-999,"K");          // !< Sigma on the fitted sky brightness temperatures (average sigma over the WVR channels for each event). 
    for(unsigned int i=0; i<v_measuredSkyBrightness_.size(); i++){v_fittedSkyBrightness_.push_back(Temperature(-999,"K"));}        // !< Fitted sky brightness temperatures over all WVR channels for each event
  }
  WVRMeasurement::WVRMeasurement(Angle elevation, vector<Temperature> measuredSkyBrightness, 
				 vector<Temperature> fittedSkyBrightness, 
				 Length retrievedWaterVaporColumn, 
				 Temperature sigma_fittedSkyBrightness){
    elevation_ = elevation;
    v_measuredSkyBrightness_ = measuredSkyBrightness;
    v_fittedSkyBrightness_ = fittedSkyBrightness;
    retrievedWaterVaporColumn_ = retrievedWaterVaporColumn;           // !< Retrieved zenith water vapor column for each event  
    sigma_fittedSkyBrightness_ = sigma_fittedSkyBrightness;          // !< Sigma on the fitted sky brightness temperatures (average sigma over the WVR channels for each event). 
  }
  
  
  WVRMeasurement::~WVRMeasurement(){}
  
}

