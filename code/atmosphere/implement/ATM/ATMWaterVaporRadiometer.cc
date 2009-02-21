using namespace std;

#include <vector>
#include <string>
#include <stdio.h>
#include <iostream>


#include <atmosphere/ATM/ATMWaterVaporRadiometer.h>






namespace atm {
  
  WaterVaporRadiometer::WaterVaporRadiometer(){}
  
  WaterVaporRadiometer::WaterVaporRadiometer(vector<unsigned int> IdChannels){
    
    spilloverTemperature_ = Temperature(-999.0,"K");
    IdChannels_ = IdChannels;
    
    Percent sg(50,"%");  // IF DOUBLE SIDE BAND, Default Sideband Gain is 50%
    
    for(unsigned int i=0; i<IdChannels.size(); i++){
      
      skyCoupling_.push_back(1.0);
      signalGain_.push_back(sg);
      
    }
    
  }
  
  WaterVaporRadiometer::WaterVaporRadiometer(vector<unsigned int> IdChannels, vector<double> skyCoupling){
    
    spilloverTemperature_ = Temperature(-999.0,"K");
    IdChannels_ = IdChannels;
    
    if(IdChannels.size()<skyCoupling.size()){
      for(unsigned int i=0; i<IdChannels.size(); i++){
	skyCoupling_.push_back(skyCoupling[i]);
      }
    }else{
      if(IdChannels.size()==skyCoupling.size()){
	skyCoupling_ = skyCoupling;
      }else{
	for(unsigned int i=0; i<skyCoupling.size(); i++){
	  skyCoupling_.push_back(skyCoupling[i]);
	}
	for(unsigned int i=skyCoupling.size(); i<IdChannels.size(); i++){
	  skyCoupling_.push_back(skyCoupling[skyCoupling.size()-1]);
	}
      }
    }
    
  }
  
  WaterVaporRadiometer::WaterVaporRadiometer(vector<unsigned int> IdChannels, vector<Percent> signalGain){
    
    spilloverTemperature_ = Temperature(-999.0,"K");
    IdChannels_ = IdChannels;
    
    if(IdChannels.size()<signalGain.size()){
      for(unsigned int i=0; i<IdChannels.size(); i++){
	signalGain_.push_back(signalGain[i]);
      }
    }else{
      if(IdChannels.size()==signalGain.size()){
	signalGain_ = signalGain;
      }else{
	for(unsigned int i=0; i<signalGain.size(); i++){
	  signalGain_.push_back(signalGain[i]);
	}
	for(unsigned int i=signalGain.size(); i<IdChannels.size(); i++){
	  signalGain_.push_back(signalGain[signalGain.size()-1]);
	}
      }
    }
    
  }
  
  WaterVaporRadiometer::WaterVaporRadiometer(vector<unsigned int> IdChannels, vector<double> skyCoupling, vector<Percent> signalGain){
    
    spilloverTemperature_ = Temperature(-999.0,"K");
    IdChannels_ = IdChannels;
    
    if(IdChannels.size()<skyCoupling.size()){
      for(unsigned int i=0; i<IdChannels.size(); i++){
	skyCoupling_.push_back(skyCoupling[i]);
      }
    }else{
      if(IdChannels.size()==skyCoupling.size()){
	skyCoupling_ = skyCoupling;
      }else{
	for(unsigned int i=0; i<skyCoupling.size(); i++){
	  skyCoupling_.push_back(skyCoupling[i]);
	}
	for(unsigned int i=skyCoupling.size(); i<IdChannels.size(); i++){
	  skyCoupling_.push_back(skyCoupling[skyCoupling.size()-1]);
	}
      }
    }
    
    if(IdChannels.size()<signalGain.size()){
      for(unsigned int i=0; i<IdChannels.size(); i++){
	signalGain_.push_back(signalGain[i]);
      }
    }else{
      if(IdChannels.size()==signalGain.size()){
	signalGain_ = signalGain;
      }else{
	for(unsigned int i=0; i<signalGain.size(); i++){
	  signalGain_.push_back(signalGain[i]);
	}
	for(unsigned int i=signalGain.size(); i<IdChannels.size(); i++){
	  signalGain_.push_back(signalGain[signalGain.size()-1]);
	}
      }
    }
    
  }
  
  
  
  
  WaterVaporRadiometer::WaterVaporRadiometer(vector<unsigned int> IdChannels, Temperature spilloverTemperature){
    
    spilloverTemperature_ = spilloverTemperature;
    IdChannels_ = IdChannels;
    
    Percent sg(50,"%");  // IF DOUBLE SIDE BAND, Default Sideband Gain is 50%
    
    for(unsigned int i=0; i<IdChannels.size(); i++){
      
      skyCoupling_.push_back(1.0);
      signalGain_.push_back(sg);
      
    }
    
  }
  
  WaterVaporRadiometer::WaterVaporRadiometer(vector<unsigned int> IdChannels, vector<double> skyCoupling, Temperature spilloverTemperature){
    
    spilloverTemperature_ = spilloverTemperature;
    IdChannels_ = IdChannels;
    
    if(IdChannels.size()<skyCoupling.size()){
      for(unsigned int i=0; i<IdChannels.size(); i++){
	skyCoupling_.push_back(skyCoupling[i]);
      }
    }else{
      if(IdChannels.size()==skyCoupling.size()){
	skyCoupling_ = skyCoupling;
      }else{
	for(unsigned int i=0; i<skyCoupling.size(); i++){
	  skyCoupling_.push_back(skyCoupling[i]);
	}
	for(unsigned int i=skyCoupling.size(); i<IdChannels.size(); i++){
	  skyCoupling_.push_back(skyCoupling[skyCoupling.size()-1]);
	}
      }
    }
    
  }
  
  WaterVaporRadiometer::WaterVaporRadiometer(vector<unsigned int> IdChannels, vector<Percent> signalGain, Temperature spilloverTemperature){
    
    spilloverTemperature_ = spilloverTemperature;
    IdChannels_ = IdChannels;
    
    if(IdChannels.size()<signalGain.size()){
      for(unsigned int i=0; i<IdChannels.size(); i++){
	signalGain_.push_back(signalGain[i]);
      }
    }else{
      if(IdChannels.size()==signalGain.size()){
	signalGain_ = signalGain;
      }else{
	for(unsigned int i=0; i<signalGain.size(); i++){
	  signalGain_.push_back(signalGain[i]);
	}
	for(unsigned int i=signalGain.size(); i<IdChannels.size(); i++){
	  signalGain_.push_back(signalGain[signalGain.size()-1]);
	}
      }
    }
    
  }
  
  WaterVaporRadiometer::WaterVaporRadiometer(vector<unsigned int> IdChannels, vector<double> skyCoupling, vector<Percent> signalGain, Temperature spilloverTemperature){
    
    spilloverTemperature_ = spilloverTemperature;
    IdChannels_ = IdChannels;
    
    if(IdChannels.size()<skyCoupling.size()){
      for(unsigned int i=0; i<IdChannels.size(); i++){
	skyCoupling_.push_back(skyCoupling[i]);
      }
    }else{
      if(IdChannels.size()==skyCoupling.size()){
	skyCoupling_ = skyCoupling;
      }else{
	for(unsigned int i=0; i<skyCoupling.size(); i++){
	  skyCoupling_.push_back(skyCoupling[i]);
	}
	for(unsigned int i=skyCoupling.size(); i<IdChannels.size(); i++){
	  skyCoupling_.push_back(skyCoupling[skyCoupling.size()-1]);
	}
      }
    }
    
    if(IdChannels.size()<signalGain.size()){
      for(unsigned int i=0; i<IdChannels.size(); i++){
	signalGain_.push_back(signalGain[i]);
      }
    }else{
      if(IdChannels.size()==signalGain.size()){
	signalGain_ = signalGain;
      }else{
	for(unsigned int i=0; i<signalGain.size(); i++){
	  signalGain_.push_back(signalGain[i]);
	}
	for(unsigned int i=signalGain.size(); i<IdChannels.size(); i++){
	  signalGain_.push_back(signalGain[signalGain.size()-1]);
	}
      }
    }
    
  }
  
  
  
  WaterVaporRadiometer::~WaterVaporRadiometer(){}
  
  
}
