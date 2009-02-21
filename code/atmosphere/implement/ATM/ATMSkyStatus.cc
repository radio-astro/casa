#include <string>
#include <vector>
#include <iostream>
#include <math.h>

using namespace std;


#include <atmosphere/ATM/ATMSkyStatus.h>

#define STRLEN  40        // Max length of a row in a tpoint file

namespace atm {
  
  // Constructors
  
  SkyStatus::SkyStatus(RefractiveIndexProfile refractiveIndexProfile):
    RefractiveIndexProfile(refractiveIndexProfile),  
    airMass_(1.0),
    skyBackgroundTemperature_(2.73,"K")
  {
    
    iniSkyStatus();  
    
  }
  
  
  SkyStatus::SkyStatus(RefractiveIndexProfile refractiveIndexProfile, double airMass):
    RefractiveIndexProfile(refractiveIndexProfile),  
    airMass_(airMass),
    skyBackgroundTemperature_(2.73,"K")
  {
    
    iniSkyStatus();  
    
  }
  
  SkyStatus::SkyStatus(RefractiveIndexProfile refractiveIndexProfile, Temperature temperatureBackground):
    RefractiveIndexProfile(refractiveIndexProfile),  
    airMass_(1.0),
    skyBackgroundTemperature_(temperatureBackground)
  {
    
    iniSkyStatus();  
    
  }
  
  SkyStatus::SkyStatus(RefractiveIndexProfile refractiveIndexProfile, Length wh2o):   
    RefractiveIndexProfile(refractiveIndexProfile),  
    airMass_(1.0),
    skyBackgroundTemperature_(2.73,"K"),
    wh2o_user_(wh2o)
  {
    
    iniSkyStatus();  
    
  }
  
  SkyStatus::SkyStatus(RefractiveIndexProfile refractiveIndexProfile, Temperature temperatureBackground, double airMass):  
    RefractiveIndexProfile(refractiveIndexProfile),  
    airMass_(airMass),
    skyBackgroundTemperature_(temperatureBackground)
  {
    
    iniSkyStatus();  
    
  }
  
  SkyStatus::SkyStatus(RefractiveIndexProfile refractiveIndexProfile, double airMass, Temperature temperatureBackground):   
    RefractiveIndexProfile(refractiveIndexProfile),  
    airMass_(airMass),
    skyBackgroundTemperature_(temperatureBackground)
  {
    
    iniSkyStatus();  
    
  }
  
  SkyStatus::SkyStatus(RefractiveIndexProfile refractiveIndexProfile, Length wh2o, double airMass):
    RefractiveIndexProfile(refractiveIndexProfile),  
    airMass_(airMass),
    skyBackgroundTemperature_(2.73,"K"),
    wh2o_user_(wh2o)
  {
    
    iniSkyStatus();  
    
  }
  
  SkyStatus::SkyStatus(RefractiveIndexProfile refractiveIndexProfile, double airMass, Length wh2o):   
    RefractiveIndexProfile(refractiveIndexProfile),  
    airMass_(airMass),
    skyBackgroundTemperature_(2.73,"K"),
    wh2o_user_(wh2o)
  {
    
    iniSkyStatus();  
    
  }
  
  SkyStatus::SkyStatus(RefractiveIndexProfile refractiveIndexProfile, Length wh2o, Temperature temperatureBackground):   
    RefractiveIndexProfile(refractiveIndexProfile),  
    airMass_(1.0),
    skyBackgroundTemperature_(temperatureBackground),
    wh2o_user_(wh2o)
  {
    
    iniSkyStatus();  
    
  }
  
  SkyStatus::SkyStatus(RefractiveIndexProfile refractiveIndexProfile, Temperature temperatureBackground, Length wh2o):   
    RefractiveIndexProfile(refractiveIndexProfile),  
    airMass_(1.0),
    skyBackgroundTemperature_(temperatureBackground),
    wh2o_user_(wh2o)
  {
    
    iniSkyStatus();  
    
  }
  
  SkyStatus::SkyStatus(RefractiveIndexProfile refractiveIndexProfile, Temperature temperatureBackground, double airMass, Length wh2o):   
    RefractiveIndexProfile(refractiveIndexProfile),  
    airMass_(airMass),
    skyBackgroundTemperature_(temperatureBackground),
    wh2o_user_(wh2o)
  {
    
    iniSkyStatus();  
    
  }
  
  SkyStatus::SkyStatus(RefractiveIndexProfile refractiveIndexProfile, Temperature temperatureBackground, Length wh2o, double airMass):
    RefractiveIndexProfile(refractiveIndexProfile),  
    airMass_(airMass),
    skyBackgroundTemperature_(temperatureBackground),
    wh2o_user_(wh2o)
  {
    
    iniSkyStatus();  
    
  }
  
  SkyStatus::SkyStatus(RefractiveIndexProfile refractiveIndexProfile, double airMass, Temperature temperatureBackground, Length wh2o):   
    RefractiveIndexProfile(refractiveIndexProfile),  
    airMass_(airMass),
    skyBackgroundTemperature_(temperatureBackground),
    wh2o_user_(wh2o)
  {
    
    iniSkyStatus();  
    
  }
  
  SkyStatus::SkyStatus(RefractiveIndexProfile refractiveIndexProfile, double airMass, Length wh2o, Temperature temperatureBackground):   
    RefractiveIndexProfile(refractiveIndexProfile),  
    airMass_(airMass),
    skyBackgroundTemperature_(temperatureBackground),
    wh2o_user_(wh2o)
  {
    
    iniSkyStatus();  
    
  }
  
  SkyStatus::SkyStatus(RefractiveIndexProfile refractiveIndexProfile, Length wh2o, Temperature temperatureBackground, double airMass):   
    RefractiveIndexProfile(refractiveIndexProfile),  
    airMass_(airMass),
    skyBackgroundTemperature_(temperatureBackground),
    wh2o_user_(wh2o)
  {
    
    iniSkyStatus();  
    
  }
  
  SkyStatus::SkyStatus(RefractiveIndexProfile refractiveIndexProfile, Length wh2o, double airMass, Temperature temperatureBackground):   
    RefractiveIndexProfile(refractiveIndexProfile),  
    airMass_(airMass),
    skyBackgroundTemperature_(temperatureBackground),
    wh2o_user_(wh2o)
  {
    
    iniSkyStatus();  
    
  }
  
  
  SkyStatus::SkyStatus( const SkyStatus & a)
  {
    //   cout<<"Enter RefractiveIndexProfile copy constructor version Fri May 20 00:59:47 CEST 2005"<<endl;
    
    // level AtmProfile
    type_ = a.type_;
    prLimit_ = a.prLimit_;
    v_hx_.reserve(a.v_hx_.size());
    v_px_.reserve(a.v_px_.size());
    v_tx_.reserve(a.v_tx_.size());
    for(unsigned int n=0; n<a.v_hx_.size(); n++){
      v_hx_.push_back(a.v_hx_[n]);
      v_px_.push_back(a.v_px_[n]);
      v_tx_.push_back(a.v_tx_[n]);
    }
    
    groundTemperature_ = a.groundTemperature_;
    tropoLapseRate_    = a.tropoLapseRate_;
    groundPressure_    = a.groundPressure_;
    relativeHumidity_  = a.relativeHumidity_;
    wvScaleHeight_     = a.wvScaleHeight_;
    pressureStep_      = a.pressureStep_;
    pressureStepFactor_= a.pressureStepFactor_;
    altitude_          = a.altitude_;
    topAtmProfile_     = a.topAtmProfile_;
    numLayer_          = a.numLayer_;
    newBasicParam_     = a.newBasicParam_;
    v_layerThickness_.reserve(numLayer_);
    v_layerTemperature_.reserve(numLayer_);
    v_layerWaterVapor_.reserve(numLayer_);
    v_layerCO_.reserve(numLayer_);
    v_layerO3_.reserve(numLayer_);
    v_layerN2O_.reserve(numLayer_);
    
    for(unsigned int n=0; n<numLayer_; n++){
      v_layerThickness_.push_back(a.v_layerThickness_[n]);
      v_layerTemperature_.push_back(a.v_layerTemperature_[n]);
      //v_layerDeltaT_.push_back(a.v_layerDeltaT_[n]);
      //cout << "n=" << n << endl;
      v_layerWaterVapor_.push_back(a.v_layerWaterVapor_[n]);
      v_layerPressure_.push_back(a.v_layerPressure_[n]);
      v_layerCO_.push_back(a.v_layerCO_[n]);
      v_layerO3_.push_back(a.v_layerO3_[n]);
      v_layerN2O_.push_back(a.v_layerN2O_[n]);
    }
    
    // level Spectral Grid
    freqUnits_               = a.freqUnits_;
    v_chanFreq_              = a.v_chanFreq_;
    
    v_numChan_               = a.v_numChan_;
    v_refChan_               = a.v_refChan_; 
    v_refFreq_               = a.v_refFreq_; 
    v_chanSep_               = a.v_chanSep_; 
    v_maxFreq_               = a.v_maxFreq_; 
    v_minFreq_               = a.v_minFreq_; 
    v_intermediateFrequency_ = a.v_intermediateFrequency_;
    v_loFreq_                = a.v_loFreq_;
    
    v_sidebandSide_          = a.v_sidebandSide_;
    v_sidebandType_          = a.v_sidebandType_;
    
    vv_assocSpwId_           = a.vv_assocSpwId_;
    vv_assocNature_          = a.vv_assocNature_;
    
    v_transfertId_           = a.v_transfertId_;
    
    
    // level Absorption Profile  
    vv_N_H2OLinesPtr_.reserve(a.v_chanFreq_.size());
    vv_N_H2OContPtr_.reserve(a.v_chanFreq_.size());
    vv_N_O2LinesPtr_.reserve(a.v_chanFreq_.size());
    vv_N_DryContPtr_.reserve(a.v_chanFreq_.size());
    vv_N_O3LinesPtr_.reserve(a.v_chanFreq_.size());
    vv_N_COLinesPtr_.reserve(a.v_chanFreq_.size());
    vv_N_N2OLinesPtr_.reserve(a.v_chanFreq_.size());

    
    vector<complex<double> >* v_N_H2OLinesPtr;
    vector<complex<double> >* v_N_H2OContPtr;
    vector<complex<double> >* v_N_O2LinesPtr;
    vector<complex<double> >* v_N_DryContPtr;
    vector<complex<double> >* v_N_O3LinesPtr;
    vector<complex<double> >* v_N_COLinesPtr;
    vector<complex<double> >* v_N_N2OLinesPtr;

    
    
    for(unsigned int nc=0; nc<v_chanFreq_.size(); nc++){
      
      
      v_N_H2OLinesPtr = new vector<complex<double> >;  v_N_H2OLinesPtr->reserve(numLayer_);
      v_N_H2OContPtr  = new vector<complex<double> >;  v_N_H2OContPtr->reserve(numLayer_);
      v_N_O2LinesPtr  = new vector<complex<double> >;  v_N_O2LinesPtr->reserve(numLayer_);
      v_N_DryContPtr  = new vector<complex<double> >;  v_N_DryContPtr->reserve(numLayer_);
      v_N_O3LinesPtr  = new vector<complex<double> >;  v_N_O3LinesPtr->reserve(numLayer_);
      v_N_COLinesPtr  = new vector<complex<double> >;  v_N_COLinesPtr->reserve(numLayer_);
      v_N_N2OLinesPtr = new vector<complex<double> >;  v_N_N2OLinesPtr->reserve(numLayer_);

      
      for(unsigned int n=0; n<numLayer_; n++){
	
	// cout << "numLayer_=" << nc << " " << n << endl; // COMMENTED OUT BY JUAN MAY/16/2005
	
	v_N_H2OLinesPtr->push_back(a.vv_N_H2OLinesPtr_[nc]->at(n));
	v_N_H2OContPtr->push_back(a.vv_N_H2OContPtr_[nc]->at(n));
	v_N_O2LinesPtr->push_back(a.vv_N_O2LinesPtr_[nc]->at(n));
	v_N_DryContPtr->push_back(a.vv_N_DryContPtr_[nc]->at(n));
	v_N_O3LinesPtr->push_back(a.vv_N_O3LinesPtr_[nc]->at(n));
	v_N_COLinesPtr->push_back(a.vv_N_COLinesPtr_[nc]->at(n));
	v_N_N2OLinesPtr->push_back(a.vv_N_N2OLinesPtr_[nc]->at(n));

      }
      
      
      vv_N_H2OLinesPtr_.push_back(v_N_H2OLinesPtr);
      vv_N_H2OContPtr_.push_back(v_N_H2OContPtr);    
      vv_N_O2LinesPtr_.push_back(v_N_O2LinesPtr);    
      vv_N_DryContPtr_.push_back(v_N_DryContPtr);    
      vv_N_O3LinesPtr_.push_back(v_N_O3LinesPtr);    
      vv_N_COLinesPtr_.push_back(v_N_COLinesPtr);    
      vv_N_N2OLinesPtr_.push_back(v_N_N2OLinesPtr);

    }
    
    // level Atm Radiance
    
    airMass_ = a.airMass_;                                
    skyBackgroundTemperature_ = a.skyBackgroundTemperature_;               
    wh2o_user_ = a.wh2o_user_;                                                                                      
    
    
  }
  
  
  
  SkyStatus::~SkyStatus()
  { 
    void rmSkyStatus();
  }
  
  
  bool  SkyStatus::setBasicAtmosphericParameters(Length altitude, Pressure groundPressure, 
						 Temperature groundTemperature, double tropoLapseRate, 
						 Humidity relativeHumidity, Length wvScaleHeight )
  {
    bool update = updateProfilesAndRadiance( altitude, groundPressure, 
					     groundTemperature, tropoLapseRate, 
					     relativeHumidity, wvScaleHeight);
    return update;
  }
  
  bool SkyStatus::setBasicAtmosphericParameters(Length altitude, Length wvScaleHeight) 
  { 
    bool update = updateProfilesAndRadiance( altitude, groundPressure_, 
					     groundTemperature_, tropoLapseRate_, 
					     relativeHumidity_, wvScaleHeight);
    return update;
  }
  
  bool SkyStatus::setBasicAtmosphericParameters(Length altitude)
  { 
    bool update = updateProfilesAndRadiance( altitude, groundPressure_, 
					     groundTemperature_, tropoLapseRate_, 
					     relativeHumidity_, wvScaleHeight_);
    return update;
  }
  
  bool SkyStatus::setBasicAtmosphericParameters(Temperature groundTemperature)
  { 
    bool update = updateProfilesAndRadiance( altitude_, groundPressure_, 
					     groundTemperature, tropoLapseRate_, 
					     relativeHumidity_, wvScaleHeight_);
    return update;
  }
  
  bool SkyStatus::setBasicAtmosphericParameters(Pressure groundPressure)
  { 
    
    bool update = updateProfilesAndRadiance( altitude_, groundPressure, 
					     groundTemperature_, tropoLapseRate_, 
					     relativeHumidity_, wvScaleHeight_);
    
    return update;
  }
  
  bool SkyStatus::setBasicAtmosphericParameters(Humidity relativeHumidity)
  {
    bool update = updateProfilesAndRadiance( altitude_, groundPressure_, 
					     groundTemperature_, tropoLapseRate_, 
					     relativeHumidity, wvScaleHeight_);
    return update;
  } 
  
  bool SkyStatus::setBasicAtmosphericParameters(double tropoLapseRate)
  {
    bool update = updateProfilesAndRadiance( altitude_, groundPressure_, 
					     groundTemperature_, tropoLapseRate, 
					     relativeHumidity_, wvScaleHeight_);
    return update;
  }  
  
  bool SkyStatus::setBasicAtmosphericParameters(Length altitude, Temperature groundTemperature) 
  {
    bool update = updateProfilesAndRadiance( altitude, groundPressure_, 
					     groundTemperature, tropoLapseRate_, 
					     relativeHumidity_, wvScaleHeight_);
    return update;
  } 
  
  bool SkyStatus::setBasicAtmosphericParameters(Length altitude, Pressure groundPressure)
  {
    bool update = updateProfilesAndRadiance( altitude, groundPressure, 
					     groundTemperature_, tropoLapseRate_, 
					     relativeHumidity_, wvScaleHeight_);
    return update;
  } 
  
  bool SkyStatus::setBasicAtmosphericParameters(Length altitude, Humidity relativeHumidity)
  {
    bool update = updateProfilesAndRadiance( altitude, groundPressure_, 
					     groundTemperature_, tropoLapseRate_, 
					     relativeHumidity, wvScaleHeight_);
    return update;
  } 
  
  bool SkyStatus::setBasicAtmosphericParameters(Length altitude, double tropoLapseRate) 
  {
    bool update = updateProfilesAndRadiance( altitude, groundPressure_, 
					     groundTemperature_, tropoLapseRate, 
					     relativeHumidity_, wvScaleHeight_);
    return update;
  } 
  
  bool SkyStatus::setBasicAtmosphericParameters(Temperature groundTemperature, Pressure groundPressure)
  {
    bool update = updateProfilesAndRadiance( altitude_, groundPressure, 
					     groundTemperature, tropoLapseRate_, 
					     relativeHumidity_, wvScaleHeight_);
    return update;
  } 
  bool SkyStatus::setBasicAtmosphericParameters(Pressure groundPressure, Temperature groundTemperature)
  {
    bool update = updateProfilesAndRadiance( altitude_, groundPressure, 
					     groundTemperature, tropoLapseRate_, 
					     relativeHumidity_, wvScaleHeight_);
    return update;
  } 
  
  bool SkyStatus::setBasicAtmosphericParameters(Temperature groundTemperature, Humidity relativeHumidity)
  {
    bool update = updateProfilesAndRadiance( altitude_, groundPressure_, 
					     groundTemperature, tropoLapseRate_, 
					     relativeHumidity, wvScaleHeight_);
    return update;
  } 
  bool SkyStatus::setBasicAtmosphericParameters(Humidity relativeHumidity, Temperature groundTemperature)
  {
    bool update = updateProfilesAndRadiance( altitude_, groundPressure_, 
					     groundTemperature, tropoLapseRate_, 
					     relativeHumidity, wvScaleHeight_);
    return update;
  } 
  
  bool SkyStatus::setBasicAtmosphericParameters(Temperature groundTemperature, double tropoLapseRate)
  {
    bool update = updateProfilesAndRadiance( altitude_, groundPressure_, 
					     groundTemperature, tropoLapseRate, 
					     relativeHumidity_, wvScaleHeight_);
    return update;
  } 
  bool SkyStatus::setBasicAtmosphericParameters(double tropoLapseRate, Temperature groundTemperature)
  {
    bool update = updateProfilesAndRadiance( altitude_, groundPressure_, 
					     groundTemperature, tropoLapseRate, 
					     relativeHumidity_, wvScaleHeight_);
    return update;
  } 
  
  bool SkyStatus::setBasicAtmosphericParameters(Temperature groundTemperature, Length wvScaleHeight) 
  {
    bool update = updateProfilesAndRadiance( altitude_, groundPressure_, 
					     groundTemperature, tropoLapseRate_, 
					     relativeHumidity_, wvScaleHeight);
    return update;
  } 
  
  bool SkyStatus::setBasicAtmosphericParameters(Pressure groundPressure, Humidity relativeHumidity)
  {
    bool update = updateProfilesAndRadiance( altitude_, groundPressure, 
					     groundTemperature_, tropoLapseRate_, 
					     relativeHumidity, wvScaleHeight_);
    return update;
  } 
  bool SkyStatus::setBasicAtmosphericParameters(Humidity relativeHumidity, Pressure groundPressure)
  {
    bool update = updateProfilesAndRadiance( altitude_, groundPressure, 
					     groundTemperature_, tropoLapseRate_, 
					     relativeHumidity_, wvScaleHeight_);
    return update;
  } 
  
  bool SkyStatus::setBasicAtmosphericParameters(Pressure groundPressure, double tropoLapseRate)
  {
    bool update = updateProfilesAndRadiance( altitude_, groundPressure, 
					     groundTemperature_, tropoLapseRate, 
					     relativeHumidity_, wvScaleHeight_);
    return update;
  } 
  bool SkyStatus::setBasicAtmosphericParameters(double tropoLapseRate, Pressure groundPressure)
  {
    bool update = updateProfilesAndRadiance( altitude_, groundPressure, 
					     groundTemperature_, tropoLapseRate, 
					     relativeHumidity_, wvScaleHeight_);
    return update;
  } 
  
  bool SkyStatus::setBasicAtmosphericParameters(Pressure groundPressure, Length wvScaleHeight)
  {
    bool update = updateProfilesAndRadiance( altitude_, groundPressure, 
					     groundTemperature_, tropoLapseRate_, 
					     relativeHumidity_, wvScaleHeight);
    return update;
  } 
  
  bool SkyStatus::setBasicAtmosphericParameters(Humidity relativeHumidity, double tropoLapseRate)
  {
    bool update = updateProfilesAndRadiance( altitude_, groundPressure_, 
					     groundTemperature_, tropoLapseRate, 
					     relativeHumidity, wvScaleHeight_);
    return update;
  } 
  bool SkyStatus::setBasicAtmosphericParameters(double tropoLapseRate, Humidity relativeHumidity)
  {
    bool update = updateProfilesAndRadiance( altitude_, groundPressure_, 
					     groundTemperature_, tropoLapseRate, 
					     relativeHumidity, wvScaleHeight_);
    return update;
  } 
  
  bool SkyStatus::setBasicAtmosphericParameters(Humidity relativeHumidity, Length wvScaleHeight)
  {
    bool update = updateProfilesAndRadiance( altitude_, groundPressure_, 
					     groundTemperature_, tropoLapseRate_, 
					     relativeHumidity, wvScaleHeight);
    return update;
  } 
  
  
  bool SkyStatus::setBasicAtmosphericParameters(double tropoLapseRate, Length wvScaleHeight)
  {
    bool update = updateProfilesAndRadiance( altitude_, groundPressure_, 
					     groundTemperature_, tropoLapseRate, 
					     relativeHumidity_, wvScaleHeight);
    return update;
  } 
  
  bool SkyStatus::updateProfilesAndRadiance( Length altitude, Pressure groundPressure, 
					     Temperature groundTemperature, double  tropoLapseRate, 
					     Humidity relativeHumidity, Length wvScaleHeight){
    
    bool updated = false;
    
    
    
    bool mkNewAbsPhaseProfile = updateRefractiveIndexProfile( altitude, groundPressure, 
							      groundTemperature, tropoLapseRate, 
							      relativeHumidity, wvScaleHeight);
    
    if(mkNewAbsPhaseProfile){
      updated = true;
    }
    return updated;
  }
  
  
  

  Opacity SkyStatus::getH2OLinesOpacity( unsigned int nc){
    if(!chanIndexIsValid(nc))return (double) -999.0;
    double kv = 0;
    for(unsigned int j=0; j<numLayer_; j++){
      kv = kv + imag( vv_N_H2OLinesPtr_[nc]->at(j) ) 
	* v_layerThickness_[j]; 
    } 
    return ((getUserWH2O().get())/(getGroundWH2O().get()))*kv; 
  }
  
  Opacity SkyStatus::getH2OContOpacity( unsigned int nc){
    if(!chanIndexIsValid(nc))return (double) -999.0;
    double kv = 0;
    for(unsigned int j=0; j<numLayer_; j++){
      kv = kv + imag( vv_N_H2OContPtr_[nc]->at(j) ) 
	* v_layerThickness_[j]; 
    } 
    return ((getUserWH2O().get())/(getGroundWH2O().get()))*kv; 
  }
  
  
  Angle SkyStatus::getDispersivePhaseDelay( unsigned int nc){
    if(!chanIndexIsValid(nc)){Angle aa(0.0,"deg"); return aa;}
    double kv = 0;
    for(unsigned int j=0; j<numLayer_; j++){
      kv = kv + real( vv_N_H2OLinesPtr_[nc]->at(j) ) 
	* v_layerThickness_[j]; 
    } 
    Angle aa(((getUserWH2O().get())/(getGroundWH2O().get()))*kv*57.29578,"deg");
    return aa; 
  }
  
  Length SkyStatus::getDispersivePathLength( unsigned int nc){
    if(!chanIndexIsValid(nc)){Length ll(0.0,"mm"); return ll;}
    double wavelength =  299792458.0/v_chanFreq_[nc];    // in m
    Length ll((wavelength/360.0)*getDispersivePhaseDelay(nc).get("deg"),"m");
    return ll;
  }
  
  Angle SkyStatus::getNonDispersivePhaseDelay( unsigned int nc){
    double kv = 0;
    if(!chanIndexIsValid(nc)){Angle aa(0.0,"deg"); return aa;}
    for(unsigned int j=0; j<numLayer_; j++){
      kv = kv + real( vv_N_DryContPtr_[nc]->at(j) ) 
	* v_layerThickness_[j]; 
    } 
    Angle aa(((getUserWH2O().get())/(getGroundWH2O().get()))*kv*57.29578,"deg");
    return aa; 
  }
  
  Length SkyStatus::getNonDispersivePathLength( unsigned int nc){
    if(!chanIndexIsValid(nc)){Length ll(0.0,"mm"); return ll;}
    double wavelength =  299792458.0/v_chanFreq_[nc];    // in m
    Length ll((wavelength/360.0)*getNonDispersivePhaseDelay(nc).get("deg"),"m");
    return ll;
  }
  
  Angle SkyStatus::getAverageDispersivePhaseDelay(unsigned int spwid){
    if(!spwidAndIndexAreValid(spwid,0)){Angle aa(-999.0,"deg"); return aa;}
    double av=0.0;
    for(unsigned int i=0; i<getNumChan(spwid); i++){
      av=av+getDispersivePhaseDelay( v_transfertId_[spwid]+i).get("deg");
    }
    av=av/getNumChan(spwid);
    Angle average(av,"deg");
    return average;
  }
  
  Length SkyStatus::getAverageDispersivePathLength(unsigned int spwid){
    if(!spwidAndIndexAreValid(spwid,0)){Length ll(0.0,"mm"); return ll;}
    double av=0.0;
    for(unsigned int i=0; i<getNumChan(spwid); i++){
      av=av+getDispersivePathLength( v_transfertId_[spwid]+i).get("mm");
    }
    av=av/getNumChan(spwid);
    Length average(av,"mm");
    return average;
  }
  
  Angle SkyStatus::getAverageNonDispersivePhaseDelay(unsigned int spwid){
    if(!spwidAndIndexAreValid(spwid,0)){Angle aa(0.0,"deg"); return aa;}
    double av=0.0;
    for(unsigned int i=0; i<getNumChan(spwid); i++){
      av=av+getNonDispersivePhaseDelay( v_transfertId_[spwid]+i).get("deg");
    }
    av=av/getNumChan(spwid);
    Angle average(av,"deg");
    return average;
  }
  
  Length SkyStatus::getAverageNonDispersivePathLength(unsigned int spwid){
    if(!spwidAndIndexAreValid(spwid,0)){Length ll(0.0,"mm"); return ll;}
    double av=0.0;
    for(unsigned int i=0; i<getNumChan(spwid); i++){
      av=av+getNonDispersivePathLength( v_transfertId_[spwid]+i).get("deg");
    }
    av=av/getNumChan(spwid);
    Length average(av,"mm");
    return average;
  }
  
  
  Temperature SkyStatus::getAverageTebbSky(unsigned int spwid, Length wh2o, double airmass, double skycoupling, Temperature Tspill){
    Temperature tt(-999,"K"); 
    if(!spwidAndIndexAreValid(spwid,0)){return tt;}
    if(wh2o.get()<0.0){return tt;}
    // if(skycoupling<0.0 || skycoupling>1.0){return tt;}
    if(airmass<1.0){return tt;}
    if(Tspill.get("K")<0.0 || Tspill.get("K")>350.0){return tt;}
    return RT(((wh2o.get())/(getGroundWH2O().get())),skycoupling,Tspill.get("K"),airmass,spwid); 
  }



  Temperature SkyStatus::getTebbSky(unsigned int spwid, unsigned int nc, Length wh2o, double airmass, double skycoupling, Temperature Tspill){
    Temperature tt(-999,"K"); 
    if(!spwidAndIndexAreValid(spwid,nc)){return tt;}
    if(wh2o.get()<0.0){return tt;}
    if(skycoupling<0.0 || skycoupling>1.0){return tt;}
    if(airmass<1.0){return tt;}
    if(Tspill.get("K")<0.0 || Tspill.get("K")>350.0){return tt;}
    return RT(((wh2o.get())/(getGroundWH2O().get())),skycoupling,Tspill.get("K"),airmass,spwid,nc); 
  }

  
  
  
  
  Angle SkyStatus::getDispersivePhaseDelay(unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc)){
      Angle aa(0.0,"deg"); 
      return aa;
    }
    return getDispersivePhaseDelay( v_transfertId_[spwid]+nc);
  }
  
  Length SkyStatus::getDispersivePathLength(unsigned int spwid, unsigned int nc){  
    if(!spwidAndIndexAreValid(spwid,nc)){
      Length ll(0.0,"mm"); 
      return ll;
    }
    return getDispersivePathLength( v_transfertId_[spwid]+nc);
  }
  
  Angle SkyStatus::getNonDispersivePhaseDelay(unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc)){
      Angle aa(0.0,"deg"); 
      return aa;
    }
    return getNonDispersivePhaseDelay( v_transfertId_[spwid]+nc);
  }
  
  Length SkyStatus::getNonDispersivePathLength(unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc))return (double) 0.0;
    return getNonDispersivePathLength( v_transfertId_[spwid]+nc);
  }
  
  Length SkyStatus::WaterVaporRetrieval_fromFTS(unsigned int spwId, vector<double> v_transmission, 
						Frequency f1, Frequency f2){
    if(f1.get()>f2.get()){return Length(-999,"mm");}
    if(v_transmission.size()==getSpectralWindow(spwId).size()){
      return mkWaterVaporRetrieval_fromFTS(spwId, v_transmission, getAirMass(), f1, f2);
    }else{
      return Length(-999.0,"mm"); 
    }
  }
  




  Length SkyStatus::WaterVaporRetrieval_fromTEBB(unsigned int spwId, Percent signalGain, vector<Temperature> v_tebb, double skycoupling, Temperature tspill){
    vector<double> spwId_filter; for(unsigned int n=0; n<v_numChan_[spwId]; n++){spwId_filter.push_back(1.0);}
    return WaterVaporRetrieval_fromTEBB(spwId, signalGain, v_tebb, spwId_filter, skycoupling, tspill);
  }
  Length SkyStatus::WaterVaporRetrieval_fromTEBB(vector<unsigned int> spwId, vector<Percent> signalGain, vector<vector<Temperature> > vv_tebb, vector<double> skycoupling, vector<Temperature> tspill){
    vector<vector<double> > spwId_filters; 
    vector<double> spwId_filter;
    for(unsigned int i=0; i<spwId.size(); i++){
      for(unsigned int n=0; n<v_numChan_[spwId[i]]; n++){spwId_filter.push_back(1.0);}
      spwId_filters.push_back(spwId_filter);
      spwId_filter.clear();
    }
    return WaterVaporRetrieval_fromTEBB(spwId, signalGain, vv_tebb, spwId_filters, skycoupling, tspill);
  }
  Length SkyStatus::WaterVaporRetrieval_fromTEBB(vector<unsigned int> spwId, vector<Percent> signalGain, vector<Temperature> v_tebb, vector<double> skycoupling, vector<Temperature> tspill){
    vector<vector<double> > spwId_filters; 
    vector<double> spwId_filter;
    for(unsigned int i=0; i<spwId.size(); i++){
      for(unsigned int n=0; n<v_numChan_[spwId[i]]; n++){spwId_filter.push_back(1.0);}
      spwId_filters.push_back(spwId_filter);
      spwId_filter.clear();
    }
    return WaterVaporRetrieval_fromTEBB(spwId, signalGain, v_tebb, spwId_filters, skycoupling, tspill);
  }


  Length SkyStatus::WaterVaporRetrieval_fromTEBB(unsigned int spwId, vector<Temperature> v_tebb, 
						 vector<double> spwId_filter, double skycoupling, Temperature tspill){
    if(v_tebb.size()==getSpectralWindow(spwId).size()){
      return mkWaterVaporRetrieval_fromTEBB(spwId, Percent(100.0,"%"), v_tebb, getAirMass(), spwId_filter, skycoupling, tspill);
    }else{
      return Length(-999.0,"mm"); 
    }
  }
  Length SkyStatus::WaterVaporRetrieval_fromTEBB(vector<unsigned int> spwId, vector<vector<Temperature> > vv_tebb, 
						 vector<vector<double> > spwId_filters, vector<double> skycoupling, vector<Temperature> tspill){
    vector<Percent> signalGain;
    for(unsigned int i=0; i<spwId.size(); i++){
      signalGain.push_back(Percent(100.0,"%"));
    }
    return mkWaterVaporRetrieval_fromTEBB(spwId, signalGain, vv_tebb, getAirMass(), spwId_filters, skycoupling, tspill);
  }
  Length SkyStatus::WaterVaporRetrieval_fromTEBB(vector<unsigned int> spwId, vector<Temperature> v_tebb, 
						 vector<vector<double> > spwId_filters, 
						 vector<double> skycoupling, vector<Temperature> tspill){
    vector<Percent> signalGain;
    for(unsigned int i=0; i<spwId.size(); i++){
      signalGain.push_back(Percent(100.0,"%"));
    }
    return mkWaterVaporRetrieval_fromTEBB(spwId, signalGain, v_tebb, getAirMass(), spwId_filters, skycoupling, tspill);
  }


  Length SkyStatus::WaterVaporRetrieval_fromTEBB(unsigned int spwId, Percent signalGain, vector<Temperature> v_tebb, 
						 vector<double> spwId_filter, 
						 double skycoupling, Temperature tspill){
    if(v_tebb.size()==getSpectralWindow(spwId).size()){
      return mkWaterVaporRetrieval_fromTEBB(spwId, signalGain, v_tebb, getAirMass(), spwId_filter, skycoupling, tspill);
    }else{
      return Length(-999.0,"mm"); 
    }
  }
  Length SkyStatus::WaterVaporRetrieval_fromTEBB(vector<unsigned int> spwId, vector<Percent> signalGain, 
						 vector<vector<Temperature> > vv_tebb, 
						 vector<vector<double> > spwId_filters, vector<double> skycoupling, vector<Temperature> tspill){
      return mkWaterVaporRetrieval_fromTEBB(spwId, signalGain, vv_tebb, getAirMass(), spwId_filters, skycoupling, tspill);
  }
  Length SkyStatus::WaterVaporRetrieval_fromTEBB(vector<unsigned int> spwId, vector<Percent> signalGain, vector<Temperature> v_tebb, 
						 vector<vector<double> > spwId_filters, vector<double> skycoupling, vector<Temperature> tspill){
      return mkWaterVaporRetrieval_fromTEBB(spwId, signalGain, v_tebb, getAirMass(), spwId_filters, skycoupling, tspill);
  }


  Length SkyStatus::WaterVaporRetrieval_fromTEBB(unsigned int spwId, vector<Temperature> v_tebb, double airmass, double skycoupling, Temperature tspill){
    vector<double> spwId_filter; for(unsigned int n=0; n<v_numChan_[spwId]; n++){spwId_filter.push_back(1.0);}
    return WaterVaporRetrieval_fromTEBB(spwId, v_tebb, spwId_filter, airmass, skycoupling, tspill);
  }
  Length SkyStatus::WaterVaporRetrieval_fromTEBB(vector<unsigned int> spwId, vector<vector<Temperature> > vv_tebb, double airmass, vector<double> skycoupling, vector<Temperature> tspill){
    vector<vector<double> > spwId_filters; 
    vector<double> spwId_filter;
    for(unsigned int i=0; i<spwId.size(); i++){
      for(unsigned int n=0; n<v_numChan_[spwId[i]]; n++){spwId_filter.push_back(1.0);}
      spwId_filters.push_back(spwId_filter);
      spwId_filter.clear();
    }
    return WaterVaporRetrieval_fromTEBB(spwId, vv_tebb, spwId_filters, airmass, skycoupling, tspill);
  }
  Length SkyStatus::WaterVaporRetrieval_fromTEBB(vector<unsigned int> spwId, vector<Temperature> v_tebb, double airmass, vector<double> skycoupling, vector<Temperature> tspill){
    vector<vector<double> > spwId_filters; 
    vector<double> spwId_filter;
    for(unsigned int i=0; i<spwId.size(); i++){
      for(unsigned int n=0; n<v_numChan_[spwId[i]]; n++){spwId_filter.push_back(1.0);}
      spwId_filters.push_back(spwId_filter);
      spwId_filter.clear();
    }
    return WaterVaporRetrieval_fromTEBB(spwId, v_tebb, spwId_filters, airmass, skycoupling, tspill);
  }


  Length SkyStatus::WaterVaporRetrieval_fromTEBB(unsigned int spwId, Percent signalGain, vector<Temperature> v_tebb, double airmass, double skycoupling, Temperature tspill){
    vector<double> spwId_filter; for(unsigned int n=0; n<v_numChan_[spwId]; n++){spwId_filter.push_back(1.0);}
    return WaterVaporRetrieval_fromTEBB(spwId, signalGain, v_tebb, spwId_filter, airmass, skycoupling, tspill);
  }
  Length SkyStatus::WaterVaporRetrieval_fromTEBB(vector<unsigned int> spwId, vector<Percent> signalGain, vector<vector<Temperature> > vv_tebb, double airmass, vector<double> skycoupling, vector<Temperature> tspill){
    vector<vector<double> > spwId_filters; 
    vector<double> spwId_filter;
    for(unsigned int i=0; i<spwId.size(); i++){
      for(unsigned int n=0; n<v_numChan_[spwId[i]]; n++){spwId_filter.push_back(1.0);}
      spwId_filters.push_back(spwId_filter);
      spwId_filter.clear();
    }
    return WaterVaporRetrieval_fromTEBB(spwId, signalGain, vv_tebb, spwId_filters, airmass, skycoupling, tspill);
  }
  Length SkyStatus::WaterVaporRetrieval_fromTEBB(vector<unsigned int> spwId, vector<Percent> signalGain, 
						 vector<Temperature> v_tebb, double airmass, 
						 vector<double> skycoupling, vector<Temperature> tspill){
    vector<vector<double> > spwId_filters; 
    vector<double> spwId_filter;
    for(unsigned int i=0; i<spwId.size(); i++){
      for(unsigned int n=0; n<v_numChan_[spwId[i]]; n++){spwId_filter.push_back(1.0);}
      spwId_filters.push_back(spwId_filter);
      spwId_filter.clear();
    }
    return WaterVaporRetrieval_fromTEBB(spwId, signalGain, v_tebb, spwId_filters, airmass, skycoupling, tspill);
  }


  Length SkyStatus::WaterVaporRetrieval_fromTEBB(unsigned int spwId, vector<Temperature> v_tebb, 
						 vector<double> spwId_filter, double airmass, double skycoupling, Temperature tspill){
    if(v_tebb.size()==getSpectralWindow(spwId).size()){
      return mkWaterVaporRetrieval_fromTEBB(spwId, Percent(100.0,"%"), v_tebb, airmass, spwId_filter, skycoupling, tspill);
    }else{
      return Length(-999.0,"mm"); 
    }
  }
  Length SkyStatus::WaterVaporRetrieval_fromTEBB(vector<unsigned int> spwId, vector<vector<Temperature> > vv_tebb,  
						 vector<vector<double> > spwId_filters, double airmass, vector<double> skycoupling, vector<Temperature> tspill){
    for(unsigned int j=0; j<spwId.size(); j++){
      if(vv_tebb[j].size()!=getSpectralWindow(spwId[j]).size()){
	return Length(-999.0,"mm"); 
      }
    }
    vector<Percent> signalGain;
    for(unsigned int i=0; i<spwId.size(); i++){
      signalGain.push_back(Percent(100.0,"%"));
    }
    return mkWaterVaporRetrieval_fromTEBB(spwId, signalGain, vv_tebb, airmass, spwId_filters, skycoupling, tspill);
  }
  Length SkyStatus::WaterVaporRetrieval_fromTEBB(vector<unsigned int> spwId, vector<Temperature> v_tebb, 
						 vector<vector<double> > spwId_filters, double airmass, 
						 vector<double> skycoupling, vector<Temperature> tspill){
    vector<Percent> signalGain;
    for(unsigned int i=0; i<spwId.size(); i++){
      signalGain.push_back(Percent(100.0,"%"));
    }
    return mkWaterVaporRetrieval_fromTEBB(spwId, signalGain, v_tebb, airmass, spwId_filters, skycoupling, tspill);
  }




  Length SkyStatus::WaterVaporRetrieval_fromTEBB(unsigned int spwId, Percent signalGain, vector<Temperature> v_tebb, 
						 vector<double> spwId_filter, double airmass, double skycoupling, Temperature tspill){
    if(v_tebb.size()==getSpectralWindow(spwId).size()){
      return mkWaterVaporRetrieval_fromTEBB(spwId, signalGain, v_tebb, airmass, spwId_filter, skycoupling, tspill);
    }else{
      return Length(-999.0,"mm"); 
    }
  }


  Length SkyStatus::WaterVaporRetrieval_fromTEBB(vector<unsigned int> spwId, vector<Percent> signalGain, 
						 vector<vector<Temperature> > vv_tebb,  
						 vector<vector<double> > spwId_filter, double airmass, 
						 vector<double> skycoupling, vector<Temperature> tspill){

    if(spwId.size()!=signalGain.size()){return Length(-999.0,"mm");}
    for(unsigned int j=0; j<spwId.size(); j++){
      if(vv_tebb[j].size()!=getSpectralWindow(spwId[j]).size()){
	return Length(-999.0,"mm"); 
      }
    }
    if(spwId.size()!=spwId_filter.size()){return Length(-999.0,"mm");}
    if(spwId.size()!=skycoupling.size()){return Length(-999.0,"mm");}
    if(spwId.size()!=tspill.size()){return Length(-999.0,"mm");}

    return mkWaterVaporRetrieval_fromTEBB(spwId, signalGain, vv_tebb, airmass, spwId_filter, skycoupling, tspill);
  }
  Length SkyStatus::WaterVaporRetrieval_fromTEBB(vector<unsigned int> spwId, vector<Percent> signalGain, 
						 vector<Temperature> v_tebb,  
						 vector<vector<double> > spwId_filter, double airmass, 
						 vector<double> skycoupling, vector<Temperature> tspill){
    if(spwId.size()!=signalGain.size()){return Length(-999.0,"mm");}
    if(spwId.size()!=v_tebb.size()){return Length(-999.0,"mm");}
    if(spwId.size()!=spwId_filter.size()){return Length(-999.0,"mm");}
    if(spwId.size()!=skycoupling.size()){return Length(-999.0,"mm");}
    if(spwId.size()!=tspill.size()){return Length(-999.0,"mm");}

    return mkWaterVaporRetrieval_fromTEBB(spwId, signalGain, v_tebb, airmass, spwId_filter, skycoupling, tspill);
  }

  Length SkyStatus::WaterVaporRetrieval_fromTEBB(vector<unsigned int> spwId, Percent signalGain, vector<Temperature> v_tebb,  
						 vector<vector<double> > spwId_filters, double airmass, 
						 double skycoupling, Temperature tspill){
    vector<Percent> v_signalGain;
    vector<double> v_skycoupling; 
    vector<Temperature> v_tspill;
    v_signalGain.reserve(spwId.size());
    v_skycoupling.reserve(spwId.size());
    v_tspill.reserve(spwId.size());

    for(unsigned int j=0; j<spwId.size(); j++){
      v_signalGain.push_back(signalGain);
      v_skycoupling.push_back(skycoupling);
      v_tspill.push_back(tspill);
    }
    return WaterVaporRetrieval_fromTEBB(spwId, v_signalGain, v_tebb, spwId_filters, airmass, v_skycoupling, v_tspill);
  }
  Length SkyStatus::WaterVaporRetrieval_fromTEBB(vector<unsigned int> spwId, Percent signalGain, vector<Temperature> v_tebb, double airmass, double skycoupling, Temperature tspill){
    vector<vector<double> > spwId_filters; 
    vector<double> spwId_filter;
    for(unsigned int i=0; i<spwId.size(); i++){
      for(unsigned int n=0; n<v_numChan_[spwId[i]]; n++){spwId_filter.push_back(1.0);}
      spwId_filters.push_back(spwId_filter);
      spwId_filter.clear();
    }
    return WaterVaporRetrieval_fromTEBB(spwId, signalGain, v_tebb, spwId_filters, airmass, skycoupling, tspill);
  }








  double SkyStatus::SkyCouplingRetrieval_fromTEBB(unsigned int spwId, vector<Temperature> v_tebb, double skycoupling, Temperature tspill){
    vector<double> spwId_filter; for(unsigned int n=0; n<v_numChan_[spwId]; n++){spwId_filter.push_back(1.0);}
    return SkyCouplingRetrieval_fromTEBB(spwId, v_tebb, spwId_filter, skycoupling, tspill);
  }

  double SkyStatus::SkyCouplingRetrieval_fromTEBB(unsigned int spwId, vector<Temperature> v_tebb, 
						  vector<double> spwId_filter, double skycoupling, Temperature tspill){
    if(v_tebb.size()==getSpectralWindow(spwId).size()){
      return mkSkyCouplingRetrieval_fromTEBB(spwId, Percent(100,"%"), v_tebb, getAirMass(), spwId_filter, skycoupling, tspill);
    }else{
      return -999.0; 
    }
  }


  double SkyStatus::SkyCouplingRetrieval_fromTEBB(unsigned int spwId, vector<Temperature> v_tebb, double airmass, double skycoupling, Temperature tspill){
    vector<double> spwId_filter; for(unsigned int n=0; n<v_numChan_[spwId]; n++){spwId_filter.push_back(1.0);}
    return SkyCouplingRetrieval_fromTEBB(spwId, v_tebb, spwId_filter, airmass, skycoupling, tspill);
  }

  double SkyStatus::SkyCouplingRetrieval_fromTEBB(unsigned int spwId, vector<Temperature> v_tebb, vector<double> spwId_filter, double airmass, double skycoupling, Temperature tspill){
    if(v_tebb.size()==getSpectralWindow(spwId).size()){
      return mkSkyCouplingRetrieval_fromTEBB(spwId, Percent(100,"%"), v_tebb, airmass, spwId_filter, skycoupling, tspill);
    }else{
      return -999.0; 
    }
  }


    
  double SkyStatus::getSigmaTransmissionFit(unsigned int spwId, vector<double> v_transmission, double airm, Frequency f1, Frequency f2){
    if(f1.get()>f2.get()){return -999.0;}
    if(v_transmission.size()==getSpectralWindow(spwId).size()){
      double rms=0.0;
      unsigned int num=0;
      //      cout << getUserWH2O().get("mm") << " mm" << endl;
      for(unsigned int i=0; i<v_transmission.size(); i++){
	if((getSpectralWindow(spwId)[i]*1E-09>=f1.get("GHz") && getSpectralWindow(spwId)[i]*1E-09<=f2.get("GHz"))){
	  num++;
	  rms=rms+pow((v_transmission[i]-exp(-airm*(getDryOpacity(spwId,i).get()+getWetOpacity(spwId,i).get()))),2.0);
	}
      }
      rms=sqrt(rms/num);
      return rms;
    }else{
      return -999.0; 
    }
  }



  Temperature SkyStatus::getSigmaFit(unsigned int spwId, vector<Temperature> v_tebbspec, Length wh2o, double airmass, double skyCoupling, Temperature Tspill){
    Temperature ttt(-999,"K");
    if(!spwidAndIndexAreValid(spwId,0)){return ttt;}
    if(!v_tebbspec.size()==getSpectralWindow(spwId).size()){return ttt;}
    if(wh2o.get("mm")<0.0){return ttt;}
    if(skyCoupling<0.0 || skyCoupling>1.0){return ttt;}
    if(airmass<1.0){return ttt;}
    if(Tspill.get("K")<0.0 || Tspill.get("K")>350.0){return ttt;}
    double rms=0.0;
    unsigned int num=0;
    for(unsigned int i=0; i<v_tebbspec.size(); i++){
      if(v_tebbspec[i].get()>0.0){
	num++;
	rms=rms+pow((v_tebbspec[i].get("K")-getTebbSky(spwId, i, wh2o, airmass, skyCoupling, Tspill).get("K")),2.0);
      }
    }
    rms=sqrt(rms/num);
    return Temperature(rms,"K");
  }
    



  Length SkyStatus::mkWaterVaporRetrieval_fromFTS(unsigned int spwId, vector<double> measuredSkyTransmission, double airm, Frequency fre1, Frequency fre2)
  {
    double pfit_wh2o;
    double deltaa = 0.02;
    double sig_fit = -999.0;
    double eps=0.01;
    vector<double> transmission_fit;
    transmission_fit.reserve(measuredSkyTransmission.size());        // allows resizing
    
    unsigned int num;
    double flamda;
    unsigned int niter=20;
    double alpha;
    double beta;
    double array;
    double f1;
    double psave;
    double f2;
    double deriv;
    double chisq1;
    double chisqr;
    double pfit_wh2o_b;
    double res;
    Length wh2o_retrieved(-999.0,"mm");
    Length werr(-888,"mm");
    double sigma_fit_transm0;
    Length sigma_wh2o;
    
    num = 0;
    
    flamda=0.001;
    pfit_wh2o=1.0;    // (getUserWH2O().get("mm"))/(getGroundWH2O().get("mm"));
    
    unsigned int nl=0;
    
    if(fre1.get("GHz")<0){
      nl=getSpectralWindow(spwId).size();
    }else{
      for(unsigned int i=0; i<getSpectralWindow(spwId).size(); i++){   
	if(getSpectralWindow(spwId)[i]*1E-09>=fre1.get("GHz") && getSpectralWindow(spwId)[i]*1E-09<=fre2.get("GHz")){
	  nl=nl+1;
	}
      }
    }
    

    for(unsigned int kite=0; kite<niter; kite++){

      num = num+1;
      
      beta = 0.0;
      alpha = 0.0;
      
      //    for(unsigned int i=0; i<getSpectralWindow(spwId).size(); i++){    

      for(unsigned int i=0; i<getSpectralWindow(spwId).size(); i++){    
	
	if( nl==getSpectralWindow(spwId).size() || 
	    (getSpectralWindow(spwId)[i]*1E-09>=fre1.get("GHz") && getSpectralWindow(spwId)[i]*1E-09<=fre2.get("GHz")) ){

	  transmission_fit[i] = exp(-(getDryOpacity(spwId,i).get()+pfit_wh2o*getWetOpacity(spwId,i).get())); 
	  f1 = transmission_fit[i];
	  psave = pfit_wh2o;
	  pfit_wh2o = pfit_wh2o + deltaa; 
	  f2 = exp(-(getDryOpacity(spwId,i).get()+pfit_wh2o*getWetOpacity(spwId,i).get()));
	  deriv=(f2-f1)/deltaa;
	  pfit_wh2o = psave;
	  beta = beta + (measuredSkyTransmission[i]-transmission_fit[i])*deriv;
	  alpha = alpha+deriv*deriv;
	}
      }
      
      chisq1 = 0;
      for(unsigned int i=0; i<getSpectralWindow(spwId).size(); i++){ 
	if( nl==getSpectralWindow(spwId).size() || 
	    (getSpectralWindow(spwId)[i]*1E-09>=fre1.get("GHz") && getSpectralWindow(spwId)[i]*1E-09<=fre2.get("GHz")) ){
	  res=-transmission_fit[i]+measuredSkyTransmission[i];
	  chisq1 = chisq1+res*res;
	}
      }
      if(nl>1){
	chisq1 = chisq1/(nl-1);
      }
      
      
      
    adjust:    array = 1.0/(1.0+flamda);
      pfit_wh2o_b = pfit_wh2o;
      pfit_wh2o_b = pfit_wh2o_b + beta*array/alpha;
      if(pfit_wh2o_b<0.0)pfit_wh2o_b=0.9*pfit_wh2o;
      
      chisqr = 0;
      for(unsigned int i=0; i<getSpectralWindow(spwId).size(); i++){    
	if( nl==getSpectralWindow(spwId).size() || 
	    (getSpectralWindow(spwId)[i]*1E-09>=fre1.get("GHz") && getSpectralWindow(spwId)[i]*1E-09<=fre2.get("GHz")) ){
	  transmission_fit[i] = exp(-(getDryOpacity(spwId,i).get()+pfit_wh2o_b*getWetOpacity(spwId,i).get())); 
	  res=-transmission_fit[i]+measuredSkyTransmission[i];
	  chisqr = chisqr+res*res;
	}
      }
      
      
      if(nl>1){
	chisqr = chisqr/(nl-1);
      }
      
      if(fabs(chisq1-chisqr)>0.001){
	if(chisq1<chisqr){
	  flamda = flamda*10.0;
	  goto adjust;
	}
      }
      
      flamda = flamda/10.0;
      sig_fit=sqrt(chisqr);
      pfit_wh2o=pfit_wh2o_b;
      sigma_wh2o = Length(sqrt(array/alpha)*sig_fit*pfit_wh2o*(getUserWH2O().get()),"mm");
      
      if(fabs(sqrt(chisq1)-sqrt(chisqr))<eps){

	/*	for(unsigned int i=0; i<getSpectralWindow(spwId).size(); i++){    
	  cout << getSpectralWindow(spwId)[i]*1E-09 << "  "  << measuredSkyTransmission[i] << "  "  << transmission_fit[i] << endl;
	  } */
	
	sigma_fit_transm0 = sig_fit;

	wh2o_retrieved = Length(pfit_wh2o*getUserWH2O().get("mm"),"mm");
	
	goto salir;
	
      }
      
    }
    
    wh2o_retrieved = werr;                              // Extra error code, fit not reached after 20 iterations 
    sigma_fit_transm0 = -888.0;                         // Extra error code, fit not reached after 20 iterations 
    sigma_wh2o = werr;                                  // Extra error code, fit not reached after 20 iterations 
    
  salir: 
    
    sigma_transmission_FTSfit_ = sigma_fit_transm0;


    if(wh2o_retrieved.get()>0.0){wh2o_user_ = wh2o_retrieved;}
    return wh2o_retrieved;

  }
  



  Length SkyStatus::mkWaterVaporRetrieval_fromTEBB(vector<unsigned int> spwId, vector<Percent> signalGain, 
						   vector<Temperature> measuredAverageSkyTEBB, double airm, 
						   vector<vector<double> > spwId_filter, vector<double> skycoupling, 
						   vector<Temperature> tspill)
  {

    double pfit_wh2o;
    double deltaa = 0.02;
    double sig_fit = -999.0;
    double eps=0.001;
    vector<Temperature> v_tebb_fit;     

    unsigned int num;
    double flamda;
    unsigned int niter=20;
    double alpha;
    double beta;
    double array;
    double f1;
    double psave;
    double f2;
    double deriv;
    double chisq1;
    double chisqr;
    double pfit_wh2o_b;
    double res;
    Length wh2o_retrieved(-999.0,"mm");
    Length werr(-888,"mm");
    double sigma_fit_transm0;
    Length sigma_wh2o;
    v_tebb_fit.reserve(measuredAverageSkyTEBB.size());

    num = 0;
    flamda=0.001;

    pfit_wh2o=(getUserWH2O().get("mm"))/(getGroundWH2O().get("mm"));


    for(unsigned int kite=0; kite<niter; kite++){
      
      num = num+1;
      
      beta = 0.0;
      alpha = 0.0;
      
      for(unsigned int j=0; j<spwId.size(); j++){

	f1 = RT(pfit_wh2o,skycoupling[j],tspill[j].get("K"),airm,spwId[j],spwId_filter[j],signalGain[j]);
	v_tebb_fit[j] = Temperature(f1,"K");

	psave = pfit_wh2o;
	pfit_wh2o = pfit_wh2o + deltaa;
	    
	f2 = RT(pfit_wh2o,skycoupling[j],tspill[j].get("K"),airm,spwId[j],spwId_filter[j],signalGain[j]);

	deriv=(f2-f1)/deltaa;
	
	pfit_wh2o = psave;
	beta = beta + ((measuredAverageSkyTEBB[j]).get("K")-f1)*deriv;
	alpha = alpha + deriv*deriv;

      }


      chisq1 = 0;



      for(unsigned int j=0; j<measuredAverageSkyTEBB.size(); j++){

	res=-v_tebb_fit[j].get("K")+(measuredAverageSkyTEBB[j]).get("K");
	chisq1 = chisq1+res*res;
	
      }
	
      if(measuredAverageSkyTEBB.size()>1){
	chisq1 = chisq1/(measuredAverageSkyTEBB.size()-1);
      }

      
    adjust:    array = 1.0/(1.0+flamda);
      pfit_wh2o_b = pfit_wh2o;
      pfit_wh2o_b = pfit_wh2o_b + beta*array/alpha;
      if(pfit_wh2o_b<0.0)pfit_wh2o_b=0.9*pfit_wh2o;
      
      chisqr = 0;


      for(unsigned int j=0; j<spwId.size(); j++){

	v_tebb_fit[j]=Temperature(RT(pfit_wh2o_b,skycoupling[j],tspill[j].get("K"),airm,spwId[j],spwId_filter[j],signalGain[j]),"K");

	res=-v_tebb_fit[j].get("K")+(measuredAverageSkyTEBB[j]).get("K");
	chisqr = chisqr+res*res;
	    
      }
      
      if(spwId.size()>1){
	chisqr = chisqr/(spwId.size()-1);
      }
      
      if(fabs(chisq1-chisqr)>0.001){
	if(chisq1<chisqr){
	  flamda = flamda*10.0;
	  goto adjust;
	}
      }
      
      flamda = flamda/10.0;
      sig_fit=sqrt(chisqr);
      pfit_wh2o=pfit_wh2o_b;
      sigma_wh2o = Length(sqrt(array/alpha)*sig_fit*pfit_wh2o*(getGroundWH2O().get("mm")),"mm");
    

      if(fabs(sqrt(chisq1)-sqrt(chisqr))<eps){
	
	sigma_fit_transm0 = sig_fit;

	wh2o_retrieved = Length(pfit_wh2o*getGroundWH2O().get("mm"),"mm");
	
	goto salir;
	
      }
      
    }
    
    wh2o_retrieved = werr;                              // Extra error code, fit not reached after 20 iterations 
    sigma_fit_transm0 = -888.0;                         // Extra error code, fit not reached after 20 iterations 
    sigma_wh2o = werr;                                  // Extra error code, fit not reached after 20 iterations 
    
  salir: 

    
    sigma_TEBBfit_ = Temperature(sigma_fit_transm0,"K");
    if(wh2o_retrieved.get()>0.0){wh2o_user_ = wh2o_retrieved;}
    return wh2o_retrieved;

  }

  Length SkyStatus::mkWaterVaporRetrieval_fromTEBB(vector<unsigned int> spwId, vector<Percent> signalGain, 
						   vector<vector<Temperature> > measuredSkyTEBB, double airm, 
						   vector<vector<double> > spwId_filter, vector<double> skycoupling, 
						   vector<Temperature> tspill)
  {

    double pfit_wh2o;
    double deltaa = 0.02;
    double sig_fit = -999.0;
    double eps=0.01;
    vector<vector<Temperature> > vv_tebb_fit;
    vector<Temperature> v_tebb_fit;     

    unsigned int num;
    double flamda;
    unsigned int niter=20;
    double alpha;
    double beta;
    double array;
    double f1;
    double psave;
    double f2;
    double deriv;
    double chisq1;
    double chisqr;
    double pfit_wh2o_b;
    double res;
    Length wh2o_retrieved(-999.0,"mm");
    Length werr(-888,"mm");
    double sigma_fit_transm0;
    Length sigma_wh2o;
    vector<double> spwIdNorm;
    vector<double> validchannels;
    spwIdNorm.reserve(spwId.size());
    validchannels.reserve(spwId.size());
    vv_tebb_fit.reserve(spwId.size());


    for(unsigned int j=0; j<spwId.size(); j++){
      spwIdNorm[j]=0.0;
      validchannels[j]=0.0;
      for(unsigned int i=0; i<getSpectralWindow(spwId[j]).size(); i++){   
	if( spwId_filter[j][i]>0){
	  spwIdNorm[j]=spwIdNorm[j]+spwId_filter[j][i];
	  validchannels[j]=validchannels[j]+1.0;
	}
      }
    }
    
    num = 0;
    flamda=0.001;
    pfit_wh2o=(getUserWH2O().get("mm"))/(getGroundWH2O().get("mm"));

    for(unsigned int kite=0; kite<niter; kite++){
      
      num = num+1;
      
      beta = 0.0;
      alpha = 0.0;
      
      for(unsigned int j=0; j<spwId.size(); j++){

	v_tebb_fit.clear();  

	for(unsigned int i=0; i<getSpectralWindow(spwId[j]).size(); i++){   

	  if( spwId_filter[j][i]>0){
	    
	    if(signalGain[j].get()<1.0){
	      f1 = RT(pfit_wh2o,skycoupling[j],tspill[j].get("K"),airm,spwId[j],i)*signalGain[j].get()+RT(pfit_wh2o,skycoupling[j],tspill[j].get("K"),airm,getAssocSpwId(spwId[j])[0],i)*(1-signalGain[j].get());
	    }else{
	      f1 = RT(pfit_wh2o,skycoupling[j],tspill[j].get("K"),airm,spwId[j],i);
	    }
	    
	    psave = pfit_wh2o;
	    pfit_wh2o = pfit_wh2o + deltaa;
	    
	    v_tebb_fit.push_back(Temperature(f1,"K"));

	    if(signalGain[j].get()<1.0){
	      f2 = (RT(pfit_wh2o,skycoupling[j],tspill[j].get("K"),airm,spwId[j],i)*signalGain[j].get()+RT(pfit_wh2o,skycoupling[j],tspill[j].get("K"),airm,getAssocSpwId(spwId[j])[0],i)*(1-signalGain[j].get()));
	    }else{
	      f2 = (RT(pfit_wh2o,skycoupling[j],tspill[j].get("K"),airm,spwId[j],i)*signalGain[j].get());
	    }
	    
	    f2=f2*spwId_filter[j][i]*validchannels[j]/spwIdNorm[j];
	    f1=f1*spwId_filter[j][i]*validchannels[j]/spwIdNorm[j];
	    deriv=(f2-f1)/deltaa;
	    
	    pfit_wh2o = psave;
	    beta = beta + ((measuredSkyTEBB[j][i]).get("K")-f1)*deriv; //*spwId_filter[j][i]/spwIdNorm[j];
	    alpha = alpha + deriv*deriv;
	    
	  }else{

	    v_tebb_fit.push_back(Temperature(-999,"K"));

	  }

	}

	vv_tebb_fit.push_back(v_tebb_fit);

      }
      

      chisq1 = 0;


      for(unsigned int j=0; j<spwId.size(); j++){

	for(unsigned int i=0; i<getSpectralWindow(spwId[j]).size(); i++){
 	  if( spwId_filter[j][i]>0){
	    res=(-vv_tebb_fit[j][i].get("K")+(measuredSkyTEBB[j][i]).get("K"))*spwId_filter[j][i]*validchannels[j]/spwIdNorm[j]; 
	    chisq1 = chisq1+res*res;
	  }
	}

      }
	
      if(spwId.size()>1){
      	chisq1 = chisq1/(spwId.size()-1);
      }

      
    adjust:    array = 1.0/(1.0+flamda);
      pfit_wh2o_b = pfit_wh2o;
      pfit_wh2o_b = pfit_wh2o_b + beta*array/alpha;
      if(pfit_wh2o_b<0.0)pfit_wh2o_b=0.9*pfit_wh2o;
      
      chisqr = 0;


      for(unsigned int j=0; j<spwId.size(); j++){

	for(unsigned int i=0; i<getSpectralWindow(spwId[j]).size(); i++){    

 	  if( spwId_filter[j][i]>0){

	    if(signalGain[j].get()<1.0){
	      vv_tebb_fit[j][i] = Temperature(RT(pfit_wh2o_b,skycoupling[j],tspill[j].get("K"),airm,spwId[j],i)*signalGain[j].get()+RT(pfit_wh2o_b,skycoupling[j],tspill[j].get("K"),airm,getAssocSpwId(spwId[j])[0],i)*(1-signalGain[j].get()),"K");
	    }else{
	      vv_tebb_fit[j][i] = Temperature(RT(pfit_wh2o_b,skycoupling[j],tspill[j].get("K"),airm,spwId[j],i)*signalGain[j].get(),"K");
	    }

	    res=(-vv_tebb_fit[j][i].get("K")+(measuredSkyTEBB[j][i]).get("K"))*spwId_filter[j][i]*validchannels[j]/spwIdNorm[j];

	    chisqr = chisqr+res*res;
	  }
	}

      }
      
      if(spwId.size()>1){
      	chisqr = chisqr/(spwId.size()-1);
      }
      
      if(fabs(chisq1-chisqr)>0.001){
	if(chisq1<chisqr){
	  flamda = flamda*10.0;
	  goto adjust;
	}
      }
      
      flamda = flamda/10.0;
      sig_fit=sqrt(chisqr);
      pfit_wh2o=pfit_wh2o_b;
      sigma_wh2o = Length(sqrt(array/alpha)*sig_fit*pfit_wh2o*(getGroundWH2O().get("mm")),"mm");
      
      if(fabs(sqrt(chisq1)-sqrt(chisqr))<eps){
	
	sigma_fit_transm0 = sig_fit;

	wh2o_retrieved = Length(pfit_wh2o*getGroundWH2O().get("mm"),"mm");
	
	goto salir;
	
      }
      
    }
    
    wh2o_retrieved = werr;                              // Extra error code, fit not reached after 20 iterations 
    sigma_fit_transm0 = -888.0;                         // Extra error code, fit not reached after 20 iterations 
    sigma_wh2o = werr;                                  // Extra error code, fit not reached after 20 iterations 
    
  salir: 
    
    sigma_TEBBfit_ = Temperature(sigma_fit_transm0,"K");
    if(wh2o_retrieved.get()>0.0){wh2o_user_ = wh2o_retrieved;}
    return wh2o_retrieved;

  }




  double SkyStatus::mkSkyCouplingRetrieval_fromTEBB(unsigned int spwId, Percent signalGain, 
						    vector<Temperature> measuredSkyTEBB, double airm, 
						    vector<double> spwId_filter, double skycoupling, Temperature tspill)
  {

    double pfit_wh2o;
    double pfit_skycoupling;
    double pfit_skycoupling_b;
    double deltaa = 0.02;
    double sig_fit = -999.0;
    double eps=0.01;
    vector<Temperature> tebb_fit;
    tebb_fit.reserve(measuredSkyTEBB.size());        // allows resizing
    
    unsigned int num;
    double flamda;
    unsigned int niter=20;
    double alpha;
    double beta;
    double array;
    double f1;
    double psave;
    double f2;
    double deriv;
    double chisq1;
    double chisqr;
    double res;
    Length wh2o_retrieved(-999.0,"mm");
    Length werr(-888,"mm");
    double sigma_fit_transm0;
    Length sigma_wh2o;
    
    num = 0;
    
    flamda=0.001;
    pfit_wh2o=(getUserWH2O().get("mm"))/(getGroundWH2O().get("mm"));
    pfit_skycoupling=1.0;

    
    
    for(unsigned int kite=0; kite<niter; kite++){
      
      num = num+1;
      
      beta = 0.0;
      alpha = 0.0;
      
      //    for(unsigned int i=0; i<getSpectralWindow(spwId).size(); i++){    

      mkWaterVaporRetrieval_fromTEBB(spwId, signalGain, measuredSkyTEBB, airm, spwId_filter, pfit_skycoupling*skycoupling, tspill);
      f1 =  sigma_TEBBfit_.get("K");
      // cout << "pfit_skycoupling =" << pfit_skycoupling << "  f1=" << f1 << "  wh2o_user_=" << wh2o_user_.get("mm") << " mm" << endl;
      psave = pfit_skycoupling;
      pfit_skycoupling = pfit_skycoupling + deltaa;
      mkWaterVaporRetrieval_fromTEBB(spwId, signalGain, measuredSkyTEBB, airm, spwId_filter, pfit_skycoupling*skycoupling, tspill);
      f2 =  sigma_TEBBfit_.get("K");
      // cout << "pfit_skycoupling =" << pfit_skycoupling << "  f2=" << f2 << "  wh2o_user_=" << wh2o_user_.get("mm") << " mm" << endl;
      deriv=(f2-f1)/deltaa;
      pfit_skycoupling = psave;
      beta = beta - f1*deriv;
      alpha = alpha+deriv*deriv;

      chisq1 = 0;
      res = f1;
      chisq1 = chisq1+res*res;

      
    adjust:    
      array = 1.0/(1.0+flamda);
      pfit_skycoupling_b = pfit_skycoupling;
      pfit_skycoupling_b = pfit_skycoupling_b + beta*array/alpha;
      if(pfit_skycoupling_b<0.0)pfit_skycoupling_b=0.9*pfit_skycoupling;
      chisqr = 0;
      mkWaterVaporRetrieval_fromTEBB(spwId, signalGain, measuredSkyTEBB, airm, spwId_filter, pfit_skycoupling_b*skycoupling, tspill);
      res = sigma_TEBBfit_.get("K");
      // cout << "pfit_skycoupling =" << pfit_skycoupling_b << "  res=" << res << "  wh2o_user_=" << wh2o_user_.get("mm") << " mm" << endl;
      chisqr = chisqr+res*res;

      if(fabs(chisq1-chisqr)>0.001){
	if(chisq1<chisqr){
	  flamda = flamda*10.0;
	  goto adjust;
	}
      }
      
      flamda = flamda/10.0;
      sig_fit=sqrt(chisqr);
      pfit_skycoupling = pfit_skycoupling_b;
      
      if(fabs(sqrt(chisq1)-sqrt(chisqr))<eps){goto salir;}
      
    }
    
    wh2o_retrieved = werr;                              // Extra error code, fit not reached after 20 iterations 
    sigma_fit_transm0 = -888.0;                         // Extra error code, fit not reached after 20 iterations 
    sigma_wh2o = werr;                                  // Extra error code, fit not reached after 20 iterations 
    
  salir: return  pfit_skycoupling*skycoupling;

  }


  double SkyStatus::RT(double pfit_wh2o, double skycoupling, double tspill, double airm, unsigned int spwid, unsigned int nc) {

    double radiance;
    double singlefreq;
    double tebb;
    double h_div_k = 0.04799274551; /* plank=6.6262e-34,boltz=1.3806E-23 */
    double kv;
    double tau_layer;
    double tbgr=skyBackgroundTemperature_.get("K");
    double ratioWater= pfit_wh2o;

    kv         = 0.0;
    radiance   = 0.0;
    singlefreq = getChanFreq(spwid,nc).get("GHz"); 

    getDryOpacity(spwid,nc).get(); 

    for(unsigned int i=0; i<getNumLayer(); i++){

      tau_layer=((getAbsTotalWet(spwid,nc,i).get())*ratioWater+getAbsTotalDry(spwid,nc,i).get())*getLayerThickness(i).get();

      radiance = radiance + (1.0/(exp(h_div_k*singlefreq/getLayerTemperature(i).get())-1.0))*
	exp(-kv*airm)*(1.0-exp(-airm*tau_layer));
 
      kv=kv+tau_layer;

    }
    
    radiance = skycoupling*(radiance + (1.0/(exp(h_div_k*singlefreq/tbgr)-1.0))*exp(-kv*airm))+
      (1.0/(exp(h_div_k*singlefreq/tspill)-1.0))*(1-skycoupling);

    tebb = h_div_k*singlefreq/log(1+(1/radiance));

    return tebb;  
    
  }
  
  
  
  
  void SkyStatus::iniSkyStatus()
  {
    
    Length wh2o_default(1,"mm");
    Length wh2o_default_neg(-999,"mm");
    Temperature temp_default_neg(-999,"K");
    
    if(wh2o_user_.get()<=0.0 || wh2o_user_.get()>(getGroundWH2O().get())*(200/(getRelativeHumidity().get("%"))) ){
      wh2o_user_ = wh2o_default;
    }
    
  }
  
  
  Temperature SkyStatus::getWVRAverageSigmaTskyFit(vector<WVRMeasurement> RadiometerData, unsigned int n, unsigned int m){
    double sigma = 0.0;
    double tr;
    Temperature sigmaT;
    if(m<n){return Temperature(-999,"K");}
    for(unsigned int i=n; i<m; i++){
      tr = RadiometerData[i].getSigmaFit().get("K");
      if(tr<0){return Temperature(-999,"K");}
      sigma=sigma+tr*tr;
    }
    if(m==n){
      sigmaT=RadiometerData[n].getSigmaFit();
    }else{
      sigma=sqrt(sigma/(m-n));
      sigmaT=Temperature(sigma,"K");
    }
    return sigmaT;
  } 
  
  
  
  
  
  
  
  WVRMeasurement SkyStatus::mkWaterVaporRetrieval_fromWVR(vector<Temperature> measuredSkyBrightnessVector, 
							  vector<unsigned int> IdChannels, 
							  vector<double> skyCoupling, vector<Percent> signalGain, 
							  Temperature spilloverTemperature, Angle elevation){
    double tspill=spilloverTemperature.get("K");
    double pfit_wh2o;
    double deltaa = 0.02;
    double sig_fit = -999.0;
    double eps=0.01;
    vector<double> tebb_fit;
    tebb_fit.reserve(measuredSkyBrightnessVector.size());       
    double airm = 1.0/sin((3.1415926*elevation.get("deg"))/180.0);
    unsigned int num;
    double flamda;
    unsigned int niter=20;
    double alpha;
    double beta;
    double array;
    double f1;
    double psave;
    double f2;
    double deriv;
    double chisq1;
    double chisqr;
    double pfit_wh2o_b;
    double res;
    Length wh2o_retrieved(-999.0,"mm");
    Length werr(-888,"mm");
    Temperature sigma_fit_temp0;
    Temperature t_astro;
    Length sigma_wh2o;
    
    
    num = 0;
    

    flamda=0.001;

    pfit_wh2o=(getUserWH2O().get("mm"))/(getGroundWH2O().get("mm"));
    

    for(unsigned int kite=0; kite<niter; kite++){
      
      num = num+1;
      
      beta = 0.0;
      alpha = 0.0;
      
      for(unsigned int i=0; i<IdChannels.size(); i++){    
	
	tebb_fit[i] = RT(pfit_wh2o,skyCoupling[i],tspill,airm,IdChannels[i],signalGain[i]);

	f1 = tebb_fit[i];
	psave = pfit_wh2o;
	pfit_wh2o = pfit_wh2o + deltaa;
	f2 = RT(pfit_wh2o,skyCoupling[i],tspill,airm,IdChannels[i],signalGain[i]);
	deriv=(f2-f1)/deltaa;
	pfit_wh2o = psave;
	beta = beta + (measuredSkyBrightnessVector[i].get("K")-tebb_fit[i])*deriv;
	alpha = alpha+deriv*deriv;

      }
      


      chisq1 = 0;
      for(unsigned int i=0; i<measuredSkyBrightnessVector.size(); i++){ 
	res=-tebb_fit[i]+measuredSkyBrightnessVector[i].get("K");
	chisq1 = chisq1+res*res;
      }
      if(measuredSkyBrightnessVector.size()>1){
	chisq1 = chisq1/(measuredSkyBrightnessVector.size()-1);
      }
      
    adjust:    array = 1.0/(1.0+flamda);
      pfit_wh2o_b = pfit_wh2o;
      pfit_wh2o_b = pfit_wh2o_b + beta*array/alpha;
      if(pfit_wh2o_b<0.0)pfit_wh2o_b=0.9*pfit_wh2o;
      
      for(unsigned int i=0; i<IdChannels.size(); i++){    
	
	tebb_fit[i] = RT(pfit_wh2o_b,skyCoupling[i],tspill,airm,IdChannels[i],signalGain[i]);
	
      }
      
      chisqr = 0;
      for(unsigned int i=0; i<IdChannels.size(); i++){ 
	res=-tebb_fit[i]+measuredSkyBrightnessVector[i].get("K");
	chisqr = chisqr+res*res;
      }
      if(IdChannels.size()>1){
	chisqr = chisqr/(IdChannels.size()-1);
      }
      

      if(fabs(chisq1-chisqr)>0.001){
	if(chisq1<chisqr){
	  flamda = flamda*10.0;
	  goto adjust;
	}
      }
      
      flamda = flamda/10.0;
      sig_fit=sqrt(chisqr);
      pfit_wh2o=pfit_wh2o_b;
      sigma_wh2o = Length(sqrt(array/alpha)*sig_fit*pfit_wh2o*(getGroundWH2O().get("mm")),"mm");



      if(fabs(sqrt(chisq1)-sqrt(chisqr))<eps){
	
	sigma_fit_temp0 = Temperature(sig_fit,"K");
	
	
	wh2o_retrieved = Length(pfit_wh2o*getGroundWH2O().get("mm"),"mm");
	
	goto salir;
	
      }
      
    }
    
    wh2o_retrieved = werr;                              // Extra error code, fit not reached after 20 iterations 
    sigma_fit_temp0 = Temperature(sig_fit,"K");         // Extra error code, fit not reached after 20 iterations 
    sigma_wh2o = werr;                                 // Extra error code, fit not reached after 20 iterations 
    
  salir: 
    
    vector<Temperature> ttt;
    
    for(unsigned int i=0; i<IdChannels.size(); i++){
      ttt.push_back(Temperature(tebb_fit[i],"K"));
    }
    
    if(wh2o_retrieved.get()>0.0){wh2o_user_ = wh2o_retrieved; }
    return WVRMeasurement(elevation, measuredSkyBrightnessVector, ttt, wh2o_retrieved, sigma_fit_temp0); 
    
  }
  
  
  
  
  
  
  
  void SkyStatus::WaterVaporRetrieval_fromWVR(vector<WVRMeasurement> &RadiometerData, unsigned int n, unsigned int m){
    
    for(unsigned int i=n; i<m; i++){
      
      WaterVaporRetrieval_fromWVR(RadiometerData[i]);
      
    }
    
  }
  
  
  void SkyStatus::WaterVaporRetrieval_fromWVR(WVRMeasurement &RadiometerData){
    
    WVRMeasurement RadiometerData_withRetrieval;

    //    cout << waterVaporRadiometer_.getIdChannels().size() << endl;
    //    cout << RadiometerData.getmeasuredSkyBrightness()[0].get("K") << " K" << endl;
    //    cout << waterVaporRadiometer_.getIdChannels()[1] << endl;
    //    cout << getAssocSpwIds(waterVaporRadiometer_.getIdChannels())[1] << endl;
    
    RadiometerData_withRetrieval = 
      mkWaterVaporRetrieval_fromWVR(RadiometerData.getmeasuredSkyBrightness(),
				    waterVaporRadiometer_.getIdChannels(),
				    waterVaporRadiometer_.getSkyCoupling(),
				    waterVaporRadiometer_.getsignalGain(),
				    waterVaporRadiometer_.getSpilloverTemperature(),
				    RadiometerData.getElevation() );
    
    // cout << "_fromWVR Sky Coupling = " <<  waterVaporRadiometer_.getSkyCoupling()[0] << endl;
    // cout << "Signal Gain = " << waterVaporRadiometer_.getsignalGain()[0].get("%") << " %" <<  endl;
    // cout << "Spillover Temp. = " << waterVaporRadiometer_.getSpilloverTemperature().get("K") << " K" << endl;
    // cout << "Elevation = " << RadiometerData.getElevation().get("deg") << endl;
    // cout << "PWV=" << RadiometerData_withRetrieval.getretrievedWaterVaporColumn().get("mm") << " mm" << endl;
    
    RadiometerData.setretrievedWaterVaporColumn(RadiometerData_withRetrieval.getretrievedWaterVaporColumn());
    RadiometerData.setfittedSkyBrightness(RadiometerData_withRetrieval.getfittedSkyBrightness());
    RadiometerData.setSigmaFit(RadiometerData_withRetrieval.getSigmaFit());
    
  }
  
  
  
  
  
  
  
  
  
  
  void SkyStatus::updateSkyCoupling_fromWVR(vector<WVRMeasurement> &RadiometerData, unsigned int n, unsigned int m){
    double pfit;
    double deltaa = 0.02;
    double sig_fit = -999.0;
    double eps=0.01;
    
    unsigned int num;
    double flamda;
    unsigned int niter=20;
    double alpha;
    double beta;
    double array;
    double f1;
    double psave;
    double f2;
    double deriv;
    double chisq1;
    double chisqr;
    double pfit_b;
    double res;
    
    num = 0;
    
    flamda=0.001;
    pfit=0.5;
    
    
    for(unsigned int kite=0; kite<niter; kite++){

      num = num+1;
      beta = 0.0;
      alpha = 0.0;
      f1 = sigmaSkyCouplingRetrieval_fromWVR(pfit,waterVaporRadiometer_,RadiometerData,n,m); 
      psave = pfit;
      pfit = pfit + deltaa; 
      f2 = sigmaSkyCouplingRetrieval_fromWVR(pfit,waterVaporRadiometer_,RadiometerData,n,m); 
      deriv=(f2-f1)/deltaa;
      pfit = psave;
      beta = beta - f1*deriv;
      alpha = alpha+deriv*deriv;
      chisq1 = f1*f1;
      
    adjust:    array = 1.0/(1.0+flamda);
      pfit_b = pfit;
      pfit_b = pfit_b + beta*array/alpha;

      chisqr = 0.;
      
      if(pfit_b<0.0){pfit_b=0.9*pfit;}
      res=sigmaSkyCouplingRetrieval_fromWVR(pfit_b,waterVaporRadiometer_,RadiometerData,n,m); 
      chisqr = chisqr+res*res;
      
      if(fabs(chisq1-chisqr)>0.001){
	if(chisq1<chisqr){
	  flamda = flamda*10.0;
	  goto adjust;
	}
      }
      
      flamda = flamda/10.0;
      sig_fit=sqrt(chisqr);
      pfit=pfit_b;
      
      if(fabs(sqrt(chisq1)-sqrt(chisqr))<eps){ goto salir;}
      
    }
    
  salir:         waterVaporRadiometer_.multiplySkyCoupling(pfit);

    //    cout << "pfit=" << pfit << "  sky_coupling: " << waterVaporRadiometer_.getSkyCoupling()[0]   << endl;


    
  }
  
  
  
  
  double SkyStatus::sigmaSkyCouplingRetrieval_fromWVR(double par_fit, WaterVaporRadiometer wvr, 
						      vector<WVRMeasurement> &RadiometerData, unsigned int n, unsigned int m){
    
    vector<double> skyCoupling = wvr.getSkyCoupling();
    
    for(unsigned int i=0; i<skyCoupling.size(); i++){
      skyCoupling[i] = skyCoupling[i]*par_fit;
    }
    
    WVRMeasurement RadiometerData_withRetrieval;
    
    for(unsigned int i=n; i<m; i++){
      
      RadiometerData_withRetrieval = 
	mkWaterVaporRetrieval_fromWVR(RadiometerData[i].getmeasuredSkyBrightness(),
				      wvr.getIdChannels(),skyCoupling,
				      wvr.getsignalGain(),wvr.getSpilloverTemperature(),
				      RadiometerData[i].getElevation() );   
      
      RadiometerData[i].setretrievedWaterVaporColumn(RadiometerData_withRetrieval.getretrievedWaterVaporColumn());
      RadiometerData[i].setfittedSkyBrightness(RadiometerData_withRetrieval.getfittedSkyBrightness());
      RadiometerData[i].setSigmaFit(RadiometerData_withRetrieval.getSigmaFit());
      
    } 
    
    return getWVRAverageSigmaTskyFit(RadiometerData,n,m).get("K");
    
  }
  
  
  
  void SkyStatus::rmSkyStatus()
  {
    
    
  }
  
}

