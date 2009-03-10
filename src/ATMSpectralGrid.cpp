#include <iostream>
#include <vector>
#include <string>
using namespace std;

#include <limits>
#include <math.h>

#include "ATMSpectralGrid.h"


namespace atm {
  
  // public methods:
  // constructors
  SpectralGrid::SpectralGrid(Frequency oneFreq)
  {
    v_chanFreq_.reserve(1);
    freqUnits_    = "Hz";
    v_transfertId_.resize(0);                   // not sure this is necessary!
    unsigned int numChan=1;
    unsigned int refChan=0;
    Frequency chanSep(0.0);
    add(numChan, refChan, oneFreq, chanSep);
    vector<unsigned int> v_dummyInt;
    vv_assocSpwId_.push_back(v_dummyInt);      // put an empty vector
    vector<string> v_dummyString;
    vv_assocNature_.push_back(v_dummyString);  // put an empty vector
  }
  
  SpectralGrid::SpectralGrid(unsigned int numChan, unsigned int refChan, Frequency refFreq, Frequency chanSep)
  {
    //  cout << " SpectralGrid constructor" << endl;
    v_chanFreq_.reserve(numChan);
    freqUnits_    = "Hz";
    v_transfertId_.resize(0);                  // not sure this is necessary!
    add(numChan, refChan, refFreq, chanSep);
    vector<unsigned int> v_dummyInt;
    vv_assocSpwId_.push_back(v_dummyInt);      // put an empty vector
    vector<string> v_dummyString;
    vv_assocNature_.push_back(v_dummyString);  // put an empty vector
  }
  
  SpectralGrid::SpectralGrid(unsigned int numChan, unsigned int refChan, Frequency refFreq, Frequency chanSep,
			     Frequency intermediateFreq, SidebandSide sbSide, SidebandType sbType)
  {
    freqUnits_    = "Hz";
    v_transfertId_.resize(0);                   // not sure this is necessary!
    v_chanFreq_.reserve(numChan);
    /* cout << " la" << endl; */
    add(numChan, refChan, refFreq, chanSep, intermediateFreq, sbSide, sbType);
  }
  
  SpectralGrid::SpectralGrid( unsigned int numChan, unsigned int refChan, double* chanFreq, string freqUnits)
  {
    v_chanFreq_.reserve(numChan);
    freqUnits_    = "Hz";
    v_transfertId_.resize(0);                   // not sure this is necessary!
    add(numChan, refChan, chanFreq, freqUnits);
  }
  
  void SpectralGrid::add(unsigned int numChan, unsigned int refChan, Frequency refFreq, Frequency chanSep,
			 Frequency intermediateFreq, SidebandSide sbSide, SidebandType sbType)
  {

    double chSep;
    vector<string> v_assocNature;
    vector<unsigned int>    v_assocSpwId;
    
    unsigned int    spwId = v_transfertId_.size();
    
    if(sbSide==LSB){                          // LSB tunin
      // the LSB:
      chSep = -fabs(chanSep.get());
      add(numChan, refChan, refFreq, chanSep);     // LSB
      v_sidebandSide_[spwId] = LSB;                  
      v_sidebandType_[spwId] = sbType;
      v_loFreq_[spwId]       = refFreq.get()+intermediateFreq.get();
      v_assocSpwId.push_back(v_numChan_.size());
      vv_assocSpwId_[vv_assocSpwId_.size()-1]=v_assocSpwId;
      v_assocNature.push_back("USB");
      vv_assocNature_[vv_assocNature_.size()-1]=v_assocNature;
      
      // the USB:
      chSep = fabs(chanSep.get());
      spwId = v_transfertId_.size();
      v_loFreq_.push_back(refFreq.get()+intermediateFreq.get());
      refChan = (unsigned int) ( (double) refChan + 2.*intermediateFreq.get()/chSep);
      chSep = -chSep;
      add(numChan, refChan, refFreq, Frequency(chSep));
      v_sidebandSide_[spwId] = USB;
      v_sidebandType_[spwId] = sbType;
      v_loFreq_[spwId]       = refFreq.get()+intermediateFreq.get();
      v_assocSpwId[0]        = v_numChan_.size()-2;
      vv_assocSpwId_[vv_assocSpwId_.size()-1]=v_assocSpwId;
      v_assocNature[0]       = "LSB";
      vv_assocNature_[vv_assocNature_.size()-1]=v_assocNature;
      
    }else{                                    // USB tuning
      // the USB:
      chSep = fabs(chanSep.get());
      add(numChan, refChan, refFreq, Frequency(chSep));
      v_sidebandSide_[spwId] = USB;
      v_sidebandType_[spwId] = sbType;
      v_loFreq_[spwId]       = refFreq.get()-intermediateFreq.get();
      v_assocSpwId.push_back(v_numChan_.size());
      vv_assocSpwId_[vv_assocSpwId_.size()-1]=v_assocSpwId;
      v_assocNature.push_back("LSB");
      vv_assocNature_[vv_assocNature_.size()-1]=v_assocNature;
      
      // the LSB:
      spwId = v_transfertId_.size();
      chSep = -fabs(chanSep.get());
      refChan = (unsigned int)( (double) refChan + 2.*intermediateFreq.get()/chSep );
      add(numChan, refChan, refFreq, Frequency(chSep));  // LSB
      v_sidebandSide_[spwId] = LSB;
      v_sidebandType_[spwId] = sbType;
      v_loFreq_[spwId]       = refFreq.get()-intermediateFreq.get();
      v_assocSpwId[0]        = v_numChan_.size()-2;
      vv_assocSpwId_[vv_assocSpwId_.size()-1]=v_assocSpwId;
      v_assocNature[0]       = "USB";
      vv_assocNature_[vv_assocNature_.size()-1]=v_assocNature;
    }
  }
  
  
  unsigned int SpectralGrid::add(unsigned int numChan, unsigned int refChan, Frequency refFreq, Frequency chanSep)
  {
    freqUnits_    = "Hz";
    
    unsigned int spwId = v_transfertId_.size();
    v_loFreq_.push_back(refFreq.get());
    if(spwId==0){
      v_transfertId_.push_back(0);
    }else{
      v_transfertId_.push_back(v_transfertId_[spwId-1]+v_numChan_[spwId-1]);
    }
    
    
    
    v_numChan_.push_back(numChan);
    v_refChan_.push_back(refChan);
    
    v_refFreq_.push_back(refFreq.get(freqUnits_));
    v_chanSep_.push_back(chanSep.get(freqUnits_));
    
    
    double* chanFreq = new double[numChan];
    { 
      double freqOffset = v_refFreq_[spwId] - v_chanSep_[spwId] * (double) (v_refChan_[spwId]-1.);

      for(unsigned int i=0; i<numChan; i++){
	chanFreq[i] = freqOffset + (double) i * v_chanSep_[spwId];
      }
      appendChanFreq(numChan, chanFreq);
    
      if (numChan>1) {
	if(chanFreq[0]>chanFreq[1]){
	  v_minFreq_.push_back(chanFreq[numChan-1]);
	  v_maxFreq_.push_back(chanFreq[0]);
	}else{
	  v_minFreq_.push_back(chanFreq[0]);
	  v_maxFreq_.push_back(chanFreq[numChan-1]);
	}
      } else {
	v_minFreq_.push_back(chanFreq[0]);
	v_maxFreq_.push_back(chanFreq[0]);
      }

      v_sidebandSide_ .push_back(NOSB);
      v_sidebandType_ .push_back(NOTYPE);
      v_intermediateFrequency_.push_back(0.0);
    }
    vector<unsigned int> v_dummyAssoc; 
    vv_assocSpwId_.push_back(v_dummyAssoc);
    vector<string> v_dummyNature;
    vv_assocNature_.push_back(v_dummyNature);
    
    return spwId;
  }
  
  void SpectralGrid::appendChanFreq(unsigned int numChan, double* chanFreq){
    
    //    unsigned int k=v_chanFreq_.size();
    for(unsigned int i=0; i<numChan; i++){
      v_chanFreq_.push_back(chanFreq[i]);  // cout << i << "v_chanFreq_="<<v_chanFreq_[k+i]<<endl;
    }
  }
  
  void SpectralGrid::appendChanFreq(unsigned int numChan, vector<double> chanFreq){
    
    //    unsigned int k=v_chanFreq_.size();
    for(unsigned int i=0; i<numChan; i++){
      v_chanFreq_.push_back(chanFreq[i]);  // cout << i << "v_chanFreq_="<<v_chanFreq_[k+i]<<endl;
    }
  }
  
  
  unsigned int SpectralGrid::add( unsigned int numChan, unsigned int refChan, double* chanFreq, string freqUnits)
  {
    double fact=1.0;
    if(freqUnits=="GHz")fact=1.0E9;
    if(freqUnits=="MHz")fact=1.0E6;
    if(freqUnits=="kHz")fact=1.0E3;
    
    unsigned int spwId = v_transfertId_.size();
    if(spwId==0){
      v_transfertId_.push_back(0);
    }else{
      v_transfertId_.push_back(v_transfertId_[spwId-1]+v_numChan_[spwId-1]);
    }
    
    v_numChan_.push_back(numChan);
    v_refChan_.push_back(refChan);
    
    bool   regular = true;
    double minFreq = 1.E30;
    double maxFreq = 0;
    double chanSep = 0;
    if(numChan>1)chanSep = fact*(chanFreq[1]-chanFreq[0]);  
    
    chanFreq[0] = fact*chanFreq[0];
    for(unsigned int i=1; i<numChan; i++){
      chanFreq[i] = fact*chanFreq[i];
      if( fabs(chanFreq[i]-chanFreq[i-1]-chanSep)>1.0E-12 )regular=false;
      if(chanFreq[i]<minFreq)minFreq=chanFreq[i];
      if(chanFreq[i]>maxFreq)maxFreq=chanFreq[i];
    }
    appendChanFreq(numChan, chanFreq);
    
    v_refFreq_.push_back(chanFreq[refChan-1]);
    if(regular){
      v_chanSep_.push_back(chanSep);
    }else{
      v_chanSep_.push_back(0);
    }
    v_sidebandSide_ .push_back(NOSB);
    v_sidebandType_ .push_back(NOTYPE);
    v_intermediateFrequency_.push_back(0.0);
    
    return spwId;
  }
  
  SpectralGrid::SpectralGrid( unsigned int numChan, double refFreq, double* chanFreq, string freqUnits)
  {
    v_chanFreq_.reserve(numChan);
    freqUnits_    = "Hz";
    v_transfertId_.resize(0);                  // not sure this is necessary!
    add( numChan, refFreq, chanFreq, freqUnits);
    vector<unsigned int> v_dummyInt;
    vv_assocSpwId_.push_back(v_dummyInt);      // put an empty vector
    vector<string> v_dummyString;
    vv_assocNature_.push_back(v_dummyString);  // put an empty vector
  }
  
  SpectralGrid::SpectralGrid( double refFreq, vector<double> chanFreq, string freqUnits)
  {
    v_chanFreq_.reserve(chanFreq.size());
    freqUnits_    = "Hz";
    v_transfertId_.resize(0);                  // not sure this is necessary!
    add( chanFreq.size(), refFreq, chanFreq, freqUnits);
    vector<unsigned int> v_dummyInt;
    vv_assocSpwId_.push_back(v_dummyInt);      // put an empty vector
    vector<string> v_dummyString;
    vv_assocNature_.push_back(v_dummyString);  // put an empty vector
  }
  
  SpectralGrid::SpectralGrid( vector<double> chanFreq, string freqUnits)
  {
    v_chanFreq_.reserve(chanFreq.size());
    freqUnits_    = "Hz";
    v_transfertId_.resize(0);     // not sure this is necessary!
    double refFreq = (Frequency(chanFreq[0],freqUnits)).get("Hz");  // We take the frequency of the first channel as 
    // reference frequency because it has not been specified
    add( chanFreq.size(), refFreq, chanFreq, freqUnits);
    vector<unsigned int> v_dummyInt;
    vv_assocSpwId_.push_back(v_dummyInt);      // put an empty vector
    vector<string> v_dummyString;
    vv_assocNature_.push_back(v_dummyString);  // put an empty vector
  }
  
  SpectralGrid::SpectralGrid( vector<Frequency> chanFreq)
  {
    v_chanFreq_.reserve(chanFreq.size());
    freqUnits_    = "Hz";
    v_transfertId_.resize(0);                  // not sure this is necessary!
    double refFreq = chanFreq[0].get("Hz");    // We take the frequency of the first channel as 
    // reference frequency because it has not been specified
    vector<double> chanFreq_double;
    for(unsigned int i=0; i<chanFreq.size(); i++){
      chanFreq_double.push_back(chanFreq[i].get("GHz"));
    }
    
    add( chanFreq.size(), refFreq, chanFreq_double, "GHz");
    vector<unsigned int> v_dummyInt;
    vv_assocSpwId_.push_back(v_dummyInt);      // put an empty vector
    vector<string> v_dummyString;
    vv_assocNature_.push_back(v_dummyString);  // put an empty vector
  }
  
  
  
 
  unsigned int SpectralGrid::add( unsigned int numChan, double refFreq, double* chanFreq, string freqUnits)
  {
    
    bool   regular   = true;
    double fact      = 1.0;
    if(freqUnits=="GHz")fact=1.0E9;
    if(freqUnits=="MHz")fact=1.0E6;
    if(freqUnits=="kHz")fact=1.0E3;
    
    freqUnits_    = "Hz";
    
    unsigned int spwId = v_transfertId_.size();    
    if(spwId==0){
      v_transfertId_.push_back(0);
    }else{
      v_transfertId_.push_back(v_transfertId_[spwId-1]+v_numChan_[spwId-1]);
    }
    
    v_numChan_.push_back(numChan);
    v_refFreq_.push_back(fact*refFreq);
    
    double chanSep = fact*(chanFreq[1]-chanFreq[0]);
    double minFreq = 1.E30;
    double maxFreq = 0;
    
    chanFreq[0] = fact*chanFreq[0];
    for(unsigned int i=1; i<numChan; i++){
      chanFreq[i] = fact*chanFreq[i];
      if( fabs(chanFreq[i]-chanFreq[i-1]-chanSep)>1.0E-12 )regular=false;
      if(chanFreq[i]<minFreq)minFreq=chanFreq[i];
      if(chanFreq[i]>maxFreq)maxFreq=chanFreq[i];
    }
    appendChanFreq(numChan, chanFreq);
    v_minFreq_.push_back(minFreq);
    v_maxFreq_.push_back(maxFreq);
    
    if(numChan>1){
      if(regular){
	v_refChan_.push_back((unsigned int) (1.+(refFreq-v_chanFreq_[0]+1.E-12)/chanSep));
	v_chanSep_.push_back(chanSep);
      }else{
	v_refChan_.push_back(0);
	v_chanSep_.push_back(0.0);
      }
    }else{
      v_refChan_.push_back(0);
      v_chanSep_.push_back(0.0);
    }
    v_sidebandSide_ .push_back(NOSB);
    v_sidebandType_ .push_back(NOTYPE);
    v_intermediateFrequency_.push_back(0.0);
    
    return spwId;
  }
  
  
  unsigned int SpectralGrid::add( unsigned int numChan, double refFreq, vector<double> chanFreq, string freqUnits)
  {
    
    bool   regular   = true;
    double fact      = 1.0;
    if(freqUnits=="GHz")fact=1.0E9;
    if(freqUnits=="MHz")fact=1.0E6;
    if(freqUnits=="kHz")fact=1.0E3;
    
    freqUnits_    = "Hz";
    
    unsigned int spwId = v_transfertId_.size();    
    if(spwId==0){
      v_transfertId_.push_back(0);
    }else{
      v_transfertId_.push_back(v_transfertId_[spwId-1]+v_numChan_[spwId-1]);
    }
    
    v_numChan_.push_back(numChan);
    v_refFreq_.push_back(fact*refFreq);
    
    double chanSep = fact*(chanFreq[1]-chanFreq[0]);
    double minFreq = 1.E30;
    double maxFreq = 0;
    
    chanFreq[0] = fact*chanFreq[0];
    for(unsigned int i=1; i<numChan; i++){
      chanFreq[i] = fact*chanFreq[i];
      if( fabs(chanFreq[i]-chanFreq[i-1]-chanSep)>1.0E-12 )regular=false;
      if(chanFreq[i]<minFreq)minFreq=chanFreq[i];
      if(chanFreq[i]>maxFreq)maxFreq=chanFreq[i];
    }
    appendChanFreq(numChan, chanFreq);
    v_minFreq_.push_back(minFreq);
    v_maxFreq_.push_back(maxFreq);
    
    if(numChan>1){
      if(regular){
	v_refChan_.push_back((unsigned int) (1.+(refFreq-v_chanFreq_[0]+1.E-12)/chanSep));
	v_chanSep_.push_back(chanSep);
      }else{
	v_refChan_.push_back(0);
	v_chanSep_.push_back(0.0);
      }
    }else{
      v_refChan_.push_back(0);
      v_chanSep_.push_back(0.0);
    }
    v_sidebandSide_ .push_back(NOSB);
    v_sidebandType_ .push_back(NOTYPE);
    v_intermediateFrequency_.push_back(0.0);
    
    return spwId;
  }
  
  
  
  
  SpectralGrid::SpectralGrid(const SpectralGrid & a){
    // cout <<  " SpectralGrid copy constructor" << endl;
    freqUnits_               = a.freqUnits_;
    v_chanFreq_              = a.v_chanFreq_;
    
    v_numChan_               = a.v_numChan_;            // cout << numChan_ << endl;
    v_refChan_               = a.v_refChan_;            // cout << refChan_ << endl;
    v_refFreq_               = a.v_refFreq_;            // cout << refChan_ << endl;
    v_chanSep_               = a.v_chanSep_;            // cout << chanSep_ << endl;
    v_maxFreq_               = a.v_maxFreq_;            // cout << maxFreq_ << endl;
    v_minFreq_               = a.v_minFreq_;            // cout << minFreq_ << endl;
    v_intermediateFrequency_ = a.v_intermediateFrequency_;
    v_loFreq_                = a.v_loFreq_;
    v_sidebandSide_          = a.v_sidebandSide_;
    v_sidebandType_          = a.v_sidebandType_;
    vv_assocSpwId_           = a.vv_assocSpwId_;
    vv_assocNature_          = a.vv_assocNature_;
    v_transfertId_           = a.v_transfertId_;
    
    // cout << "v_chanFreq_.size()=" << v_chanFreq_.size() << endl;
  }
  
  SpectralGrid::SpectralGrid(){}
  
  // destructor
  SpectralGrid::~SpectralGrid(){}
  
  
  bool SpectralGrid::wrongSpwId(unsigned int spwId){
    if(spwId>(v_transfertId_.size()-1)){
      cout <<" SpectralGrid: ERROR: "<<spwId<<" is a wrong spectral window identifier" << endl;
      return (bool) true;
    }
    return (bool) false;
  }
  
  // accessors and utilities:
  unsigned int       SpectralGrid::getNumSpectralWindow(){return v_transfertId_.size();}
  unsigned int       SpectralGrid::getNumChan(){return v_numChan_[0];}
  unsigned int       SpectralGrid::getNumChan(unsigned int spwId){if(wrongSpwId(spwId))return 0;  return v_numChan_[spwId];}
  
  unsigned int       SpectralGrid::getRefChan(){return v_refChan_[0];}
  unsigned int       SpectralGrid::getRefChan(unsigned int spwId){if(wrongSpwId(spwId))return 32767; return v_refChan_[spwId];}
  
  Frequency    SpectralGrid::getRefFreq(){return Frequency(v_refFreq_[0],"Hz");}
  Frequency    SpectralGrid::getRefFreq(unsigned int spwId){if(wrongSpwId(spwId))return 32767.; return Frequency(v_refFreq_[spwId],"Hz");}

  
  Frequency    SpectralGrid::getChanSep(){return Frequency(v_chanSep_[0],"Hz");}
  Frequency    SpectralGrid::getChanSep(unsigned int spwId){if(wrongSpwId(spwId))return 32767.; return Frequency(v_chanSep_[spwId],"Hz");}

  
  Frequency    SpectralGrid::getChanFreq(unsigned int i){return Frequency(v_chanFreq_[i],"Hz");}
  Frequency    SpectralGrid::getChanFreq(unsigned int spwId, unsigned int chanIdx){if(wrongSpwId(spwId))return 32767.; return Frequency(v_chanFreq_[v_transfertId_[spwId]+chanIdx],"Hz");}

  
  vector<double> SpectralGrid::getSbChanFreq(unsigned int spwId, unsigned int chanIdx, string units){
    vector<double> v_dummyVector;
    if(wrongSpwId(spwId))return v_dummyVector;
    v_dummyVector.push_back(getChanFreq(spwId, chanIdx).get(units));
    for(unsigned int n=0; n<vv_assocNature_[spwId].size(); n++){
      if( vv_assocNature_[spwId][n]=="USB" || vv_assocNature_[spwId][n]=="LSB" ){
	unsigned int assocSpwId = vv_assocSpwId_[spwId][n];
	v_dummyVector.push_back(getChanFreq(assocSpwId, chanIdx).get(units));
      }
    }
    return  v_dummyVector;
  }
  
  vector<double> SpectralGrid::getSpectralWindow(unsigned int spwId){
    vector<double> v_chanFreq;
    if(wrongSpwId(spwId))return v_chanFreq;
    v_chanFreq.reserve(v_numChan_[spwId]);
    for(unsigned int n=0; n<v_numChan_[spwId]; n++)v_chanFreq.push_back(v_chanFreq_[v_transfertId_[spwId]+n]);
    return v_chanFreq;
  }
  
  Frequency    SpectralGrid::getMinFreq(){return Frequency(v_minFreq_[0],"Hz");}
  Frequency    SpectralGrid::getMinFreq(unsigned int spwId){if(wrongSpwId(spwId))return 32767.; return Frequency(v_minFreq_[spwId],"Hz");}

  
  Frequency    SpectralGrid::getMaxFreq(){return Frequency(v_maxFreq_[0],"Hz");}
  Frequency    SpectralGrid::getMaxFreq(unsigned int spwId){if(wrongSpwId(spwId))return 32767.; return Frequency(v_maxFreq_[spwId],"Hz");}
  
  
  double    SpectralGrid::getChanNum(double freq){
    if(v_numChan_[0]==1)return 1;
    if(v_chanSep_[0]==0.0){           // irregular grid, look for the nearest channel
      double sep = 1.E30;
      int k=-1;
      for(unsigned int i=0; i<v_numChan_[0]; i++){
	if(sep>fabs(v_chanFreq_[v_transfertId_[0]+i]-freq)){
	  sep = fabs(v_chanFreq_[v_transfertId_[0]+i]-freq);
	  k = i;
	}
      }
      return (double) 1-v_refChan_[0]+k;       // channel the nearest
    }else{                                // regular spectral grid
      return (freq-v_refFreq_[0])/v_chanSep_[0];
    }
  }
  double    SpectralGrid::getChanNum(unsigned int spwId, double freq){
    if(wrongSpwId(spwId))return 32767.;
    if(v_numChan_[spwId]==1)return 1;
    if(v_chanSep_[spwId]==0.0){           // irregular grid, look for the nearest channel
      double sep = 1.E30;
      int k=-1;
      for(unsigned int i=0; i<v_numChan_[spwId]; i++){
	if(sep>fabs(v_chanFreq_[v_transfertId_[spwId]+i]-freq)){
	  sep = fabs(v_chanFreq_[v_transfertId_[spwId]+i]-freq);
	  k = i;
	}
      }
      return (double) 1-v_refChan_[spwId]+k;       // channel the nearest
    }else{                                // regular spectral grid
      return (freq-v_refFreq_[spwId])/v_chanSep_[spwId];
    }
  }
  
  Frequency    SpectralGrid::getBandwidth(){return Frequency(v_maxFreq_[0]-v_minFreq_[0],"Hz");}
  
  Frequency    SpectralGrid::getBandwidth(unsigned int spwId){
    if(wrongSpwId(spwId))return 32767.; 
    //return Frequency(v_maxFreq_[0]-v_minFreq_[0],"Hz");
    return Frequency(v_maxFreq_[spwId]-v_minFreq_[spwId],"Hz");
  }
  

  
  bool      SpectralGrid::isRegular(){ if(v_chanSep_[0]==0.0)return false; return true; }
  
  bool      SpectralGrid::isRegular(unsigned int spwId){
    if(wrongSpwId(spwId))return 32767.; 
    if(v_chanSep_[spwId]==0.0)return false; 
    return true; 
  }
  
  string    SpectralGrid::getSidebandSide(unsigned int spwId){
    if(!wrongSpwId(spwId)){
      if(vv_assocSpwId_[spwId].size()==0){
	/* cout << "WARNING: the spectral window with the identifier "<< spwId
	   << " has no associated spectral window "<< endl; */ 
      }
      return "";
      if(v_sidebandSide_[spwId]==NOSB)return "NoSB";
      if(v_sidebandSide_[spwId]==LSB)return "LSB";
      if(v_sidebandSide_[spwId]==USB)return "USB";
    }
    return "";
  }
  
  string    SpectralGrid::getSidebandType(unsigned int spwId){
    if(!wrongSpwId(spwId)){
      if(vv_assocSpwId_[spwId].size()==0){
	/* cout << "WARNING: the spectral window with the identifier "<< spwId
	   << " has no associated spectral window "<< endl; */ 
      }
      return "";
      if(v_sidebandType_[spwId]==DSB)return " DSB";
      if(v_sidebandType_[spwId]==SSB)return " SSB";
      if(v_sidebandType_[spwId]==TWOSB)return " 2SB";
    }
    return "";
  }
  
  string    SpectralGrid::getSideband(unsigned int spwId){
    if(!wrongSpwId(spwId)){
      if(vv_assocSpwId_[spwId].size()==0){
	/* cout << "WARNING: the spectral window with the identifier "<< spwId
	   << " has no associated spectral window "<< endl; */
      }
      return "";
      if(getSidebandSide(spwId)=="NoSB"){
	return getSidebandSide(spwId);
      }else{
	string sbTypeSide = getSidebandSide(spwId) + " with type ";
	return (sbTypeSide + getSidebandType(spwId));
      }
    }
    return "";   
  }
  
  
  vector<string> SpectralGrid::getAssocNature(unsigned int spwId){
    if(!wrongSpwId(spwId)){
      if(vv_assocNature_[spwId].size()==0){
	/* cout << "WARNING: the spectral window with the identifier "<< spwId
	   << " has no associated spectral window "<< endl; */
      }
      return vv_assocNature_[spwId];
    } 
    vector<string> v_dummyVector; 
    return v_dummyVector;
  }
  
  vector<unsigned int>    SpectralGrid::getAssocSpwId(unsigned int spwId){
    if(!wrongSpwId(spwId)){
      if(vv_assocSpwId_[spwId].size()==0){
	/* cout << "WARNING: the spectral window with the identifier "<< spwId
	   << " has no associated spectral window "<< endl; */ 
	/* cout << "vv_assocSpwId_[" <<spwId<<"]=" <<  vv_assocSpwId_[spwId][0] << endl; */
      }
      return vv_assocSpwId_[spwId];
      
    }
    vector<unsigned int> v_dummyVector; 
    return v_dummyVector; 
  }
  
  
  
  vector<unsigned int>    SpectralGrid::getAssocSpwIds(vector<unsigned int> spwIds){
    
    unsigned int spwId;
    vector<unsigned int> assoc_spwIds;
    
    for(unsigned int n=0; n<spwIds.size(); n++){
      
      spwId = spwIds[n];
      
      if(!wrongSpwId(spwId)){
	if(vv_assocSpwId_[spwId].size()==0){
	  /* cout << "WARNING: the spectral window with the identifier "<< spwId
	     << " has no associated spectral window "<< endl; */ 
	  /* cout << "vv_assocSpwId_[" <<spwId<<"]=" <<  vv_assocSpwId_[spwId][0] << endl; */
	}
	
	assoc_spwIds.push_back((vv_assocSpwId_[spwId])[0]);
	
      }else{
	assoc_spwIds.push_back(spwId);
      }
      
    }
    
    return assoc_spwIds;
    
  }
  
  
  
  double    SpectralGrid::getLoFrequency(){return v_loFreq_[0];}
  double    SpectralGrid::getLoFrequency(unsigned int spwId){if(wrongSpwId(spwId))return 32767.; return v_loFreq_[spwId];}
  
}



