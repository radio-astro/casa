#include <iostream>
#include <vector>
#include <string>
using namespace std;

#include <limits>
#include <math.h>

#include <atmosphere/ATM/ATMSpectralGrid.h>


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
    //  cout << "SpectralGrid constructor" << endl;
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
    // cout <<  "SpectralGrid copy constructor" << endl;
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
      cout <<"SpectralGrid: ERROR: "<<spwId<<" is a wrong spectral window identifier" << endl;
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
    return Frequency(v_maxFreq_[0]-v_minFreq_[0],"Hz");
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
      if(v_sidebandType_[spwId]==DSB)return "DSB";
      if(v_sidebandType_[spwId]==SSB)return "SSB";
      if(v_sidebandType_[spwId]==TWOSB)return "2SB";
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



#if     defined(_TESTBED_)
#include <iostream>
unsigned int main()
{
  unsigned int     numChan         = 64;
  unsigned int     refChan         = 32;
  
  Frequency myRefFreq(90.0,"GHz");
  Frequency myChanSep(0.01,"GHz");
  
  SpectralGrid* sgPtr1;
  
  cout << "Test 1:" <<endl;
  cout << "Build using constructor SpectralGrid(unsigned int numChan, unsigned int refChan, Frequency refFreq, Frequency chanSep):" << endl;
  sgPtr1 = new SpectralGrid(numChan, refChan, myRefFreq, myChanSep);
  cout << "Number of channels retrieved:  " << sgPtr1->getNumChan() << " Input:" << numChan << endl;
  cout << "Reference frequency retrieved: "
       << sgPtr1->getRefFreq().get("GHz")
       << " Input:" << myRefFreq.get("GHz") << "GHz" << endl;
  cout << "Reference frequency retrieved: "
       << sgPtr1->getRefFreq().get("GHz")<< "GHz "  
       <<"Input:" << myRefFreq.get("GHz") << "GHz" << endl;
  cout << "Reference channel retrieved:   " << sgPtr1->getRefChan() << " Input:" << refChan << endl;
  cout << "Channel separation retrieved:  "
       << sgPtr1->getChanSep().get("Hz") << "Hz "
       << "Input:" << myChanSep.get("GHz") << "GHz" << endl;
  cout << "Channel separation retrieved:  "
       << sgPtr1->getChanSep<.get("kHz") << "kHz "
       << "Input:" << myChanSep.get("GHz") << "GHz" << endl;
  cout << "Number of spectral windows: "
       << sgPtr1->getNumSpectralWindow()
       << " Expected: 0" << endl;
  cout << "Number of channels for spectral window identifier 1: "
       << sgPtr1->getNumChan(1)
       << " Expected: trigger an error message " << endl;
  if(sgPtr1->isRegular()){
    cout << "the first spectral window is regularily sampled" << endl;
  }else{
    cout << "the first spectral window is not regularily sampled" << endl;
  }
  if(sgPtr1->getAssocNature(0).size()==0){
    cout<<" There is a warning as expected" << endl;
  }else{
    cout<<" ERROR detected with this test: \n this spectral window has NO associated spectral window "<<endl;
  } 
  if(sgPtr1->getAssocSpwId(0).size()==0){
    cout<<" There is a warning as expected" << endl;
  }else{
    cout<<" ERROR detected with this test: \n this spectral window has NO associated spectral window "<<endl;
  }
  
  cout << "New spectral window using add(unsigned int numChan, unsigned int refChan, Frequency refFreq, Frequency chanSep):" << endl;
  unsigned int numChan1         = 128;
  unsigned int refChan1         = 32;
  Frequency myNewRefFreq(215.0,"GHz");
  Frequency myNewChanSep(0.02,"GHz");
  unsigned int spwId = sgPtr1->add( numChan1, refChan1, myNewRefFreq, myNewChanSep);
  cout << "A new spectral window has been appended and got the identifier " << spwId << endl;
  cout << endl;
  cout << "Number of spectral windows: "
       << sgPtr1->getNumSpectralWindow()
       << " Expected: 2" << endl;
  cout << endl;
  
  cout << "Channel frequency and number for the first spectral window: " << endl;
  double chFreq[sgPtr1->getNumChan()];           // one dynamic alloc
  vector<double> chanFreq;
  chanFreq.reserve(sgPtr1->getNumChan());        // a more versatil dynamic alloc (allowing eg resizing)
  for(unsigned int i=0; i<sgPtr1->getNumChan(); i++){
    chanFreq[i] = sgPtr1->getChanFreq(i).get();
    chFreq[i] = chanFreq[i];
    cout << i << "channel: " << i-refChan+1 << " freq: " << chanFreq[i] << endl;
  }
  cout << endl;
  
  spwId=0;
  cout << "Number of channels retrieved for spwId "<<spwId<<": "
       << sgPtr1->getNumChan(spwId) << " Input:" << numChan << endl;
  cout << "Reference frequency retrieved: "
       << sgPtr1->getRefFreq(spwId).get("GHz")
       << " Input:" << myRefFreq.get("GHz") << "GHz" << endl;
  cout << "Reference frequency retrieved: "
       << sgPtr1->getRefFreq(spwId).get("GHz")<< "GHz "  
       <<"Input:" << myRefFreq.get("GHz") << "GHz" << endl;
  cout << "Reference channel retrieved:   " << sgPtr1->getRefChan() << " Input:" << refChan << endl;
  cout << "Channel separation retrieved:  "
       << sgPtr1->getChanSep(spwId).get("Hz") << "Hz "
       << "Input:" << myChanSep.get("GHz") << "GHz" << endl;
  cout << "Channel separation retrieved:  "
       << sgPtr1->getChanSep(spwId,"kHz").get("kHz") << "kHz "
       << "Input:" << myChanSep.get("GHz") << "GHz" << endl;
  cout << endl;
  spwId=1;
  cout << "Number of channels retrieved for spwId "<<spwId<<": "
       << sgPtr1->getNumChan(spwId) << " Input:" << numChan1 << endl;
  cout << "Reference frequency retrieved: "
       << sgPtr1->getRefFreq(spwId).get("GHz")
       << " Input:" << myNewRefFreq.get("GHz") << "GHz" << endl;
  cout << "Reference frequency retrieved: "
       << sgPtr1->getRefFreq(spwId).get("GHz")<< "GHz "  
       <<"Input:" << myNewRefFreq.get("GHz") << "GHz" << endl;
  cout << "Reference channel retrieved:   " << sgPtr1->getRefChan() << " Input:" << refChan1 << endl;
  cout << "Channel separation retrieved:  "
       << sgPtr1->getChanSep(spwId).get("Hz") << "Hz "
       << "Input:" << myNewChanSep.get("GHz") << "GHz" << endl;
  cout << "Channel separation retrieved:  "
       << sgPtr1->getChanSep(spwId,"kHz").get("kHz") << "kHz "
       << "Input:" << myNewChanSep.get("GHz") << "GHz" << endl;
  if(sgPtr1->isRegular(spwId)){
    cout << "the spectral window with id "<<spwId<<" is regularily sampled" << endl;
  }else{
    cout << "the spectral window with id "<<spwId<<" is not regularily sampled" << endl;
  }
  
  cout << "Number of spectral windows: "
       << sgPtr1->getNumSpectralWindow()
       << " Expected: 2" << endl;
  
  if(sgPtr1->getSideband(spwId).size()==0)
    cout << "As expected this spectral window with spwid="<<spwId
	 << " has no sideband specification" << endl;
  unsigned int id=10;
  if(sgPtr1->getSideband(id).size()==0)
    cout << "As expected error because spectral window with id="<<id
	 << " does not exist!" << endl;
  
  cout << endl;
  
  
  delete sgPtr1; sgPtr1=0;
  
  
  
  double  refFreq         = 90.0E9;
  
  SpectralGrid* sgPtr2;
  cout << "Test 2:" <<endl;
  cout << "Build using SpectralGrid( unsigned int numChan, unsigned int refChan, double* chFreq, string units):" << endl;
  sgPtr2 = new SpectralGrid::SpectralGrid( numChan, refChan, chFreq, "Hz");
  cout << "Number of channels retrieved: " << sgPtr2->getNumChan()      << "    Input:  " << numChan << endl;
  cout << "Reference frequency retrieved:" << sgPtr2->getRefFreq().get("Hz")      << "Hz  Initial: none" << endl;
  cout << "Reference frequency retrieved:" << sgPtr2->getRefFreq().get("MHz") << "MHz Initial: none" << endl;
  cout << "Reference channel retrieved:  " << sgPtr2->getRefChan()    << "    Input:  " << refChan << endl;
  cout << "Channel separation retrieved: " << sgPtr2->getChanSep().get("Hz")      << "Hz  Initial: none" << endl;
  cout << "Channel separation retrieved: " << sgPtr2->getChanSep().get("MHz") << "MHz Initial: none" << endl;
  if(sgPtr2->isRegular()){
    cout << "the first spectral window with id 0 is regularily sampled" << endl;
  }else{
    cout << "the first spectral window with id 0 is not regularily sampled" << endl;
  }
  chFreq[sgPtr2->getNumChan()/4]=chFreq[sgPtr2->getNumChan()/4]+1.;
  cout << "Add a second irregular spectral window using add( unsigned int numChan, unsigned int refChan, double* chFreq, string units):" << endl;
  sgPtr2->add( numChan, refChan, chFreq, "Hz");
  if(sgPtr2->isRegular()){
    cout << "the first spectral window with id 0 is regularily sampled as expected" << endl;
  }else{
    cout << "the first spectral window with id 0 is not regularily sampled ==> ERROR in the code" << endl;
  }
  if(sgPtr2->isRegular(spwId)){
    cout << "the spectral window with id "<<spwId<<" is regularily sampled ==> ERROR in the code" << endl;
  }else{
    cout << "the spectral window with id "<<spwId<<" is not regularily sampled as expected" << endl;
  }
  delete sgPtr2; sgPtr2=0;
  
  cout << endl;
  cout << endl;
  
  SpectralGrid* sgPtr3;
  
  cout << "Test 3:" << endl;
  cout << "Build using SpectralGrid( unsigned int numChan, double refFreq, double* chFreq, string freqUnits):" << endl;
  sgPtr3 = new SpectralGrid::SpectralGrid( numChan, refFreq, chFreq, "Hz");
  cout << "Number of channels retrieved: " << sgPtr3->getNumChan() << " Input: " << numChan << endl;
  cout << "Reference frequency retrieved:" << sgPtr3->getRefFreq().get("Hz")     << "Hz  Initial:" << refFreq << "Hz" << endl;
  cout << "Reference frequency retrieved:" << sgPtr3->getRefFreq().get("MHz") << "MHz Initial:" << refFreq << "Hz" << endl;
  cout << "Reference channel retrieved:  " << sgPtr3->getRefChan() << " Initial: " << refChan << endl;
  cout << "Channel separation retrieved: " << sgPtr3->getChanSep().get("Hz") << " Initial: none"<< endl;
  
  { double chan=16.123456;
  cout << "Position (GU) retrieved: "
       << sgPtr3->getChanNum(refFreq+sgPtr3->getChanSep().get("Hz")*chan)
       << " Exact: " << chan << endl;
  }
  cout << "Total bandwidth retrieved: " << sgPtr3->getBandwidth().get()
       << " Initial: "<< sgPtr3->getChanSep().get("Hz")*(numChan-1) << endl;
  
  cout << "Frequency range: from "<< sgPtr3->getMinFreq().get() <<" to "<< sgPtr3->getMaxFreq().get() <<"Hz"<< endl;
  cout << "Frequency range: from "<< sgPtr3->getMinFreq().get("GHz") <<" to "<< sgPtr3->getMaxFreq().get("GHz") <<"GHz"<< endl;
  
  delete sgPtr3;
  
  cout << endl;
  cout << endl;
  
  numChan         = 128;
  refChan         = 64;
  Frequency refFreq2(215.0,"GHz");
  Frequency chanSep2(0.02,"GHz");
  Frequency intermediateFreq(2.0,"GHz");
  Frequency bandWidth(1.0,"GHz");
  SidebandSide sbSide=LSB;        
  SidebandType sbType=SSB;
  
  sgPtr1 = new SpectralGrid(numChan, refChan, refFreq2, chanSep2,
                            intermediateFreq, LSB, SSB);
  
  cout   << "Number of spectral windows:            " << sgPtr1->getNumSpectralWindow() << " Expected: 2" << endl;
  
  for(unsigned int spwId=0; spwId<sgPtr1->getNumSpectralWindow(); spwId++){
    cout << "Sideband:                              " << sgPtr1->getSideband(spwId) << endl;
    cout << "LO frequency:                          " << sgPtr1->getLoFrequency(spwId) << "Hz " <<  endl;
    cout << "Number of channels retrieved:          " << sgPtr1->getNumChan(spwId) << " for spwId " <<  spwId<<": "
         << " Input:" << numChan << endl;
    cout << "Reference frequency retrieved:         " << sgPtr1->getRefFreq(spwId).get("GHz")
         << " Input:" << refFreq2.get("GHz") << "GHz" << endl;
    cout << "Reference frequency retrieved:         " << sgPtr1->getRefFreq(spwId).get("GHz")<< "GHz "  
         << " Input:" << refFreq2.get("GHz") << "GHz" << endl;
    cout << "Reference channel retrieved:           " << sgPtr1->getRefChan()
         << " Input:" << refChan << endl;
    cout << "Channel separation retrieved:          " << sgPtr1->getChanSep(spwId).get("Hz") << "Hz "
         << " Input: |" << chanSep2.get("GHz") << "| GHz" << endl;
    cout << "Channel separation retrieved:          " << sgPtr1->getChanSep(spwId,"kHz").get("kHz") << "kHz "
         << " Input: |" << chanSep2.get("GHz") << "| GHz" << endl;
    cout << "minFreq:                               " << sgPtr1->getMinFreq(spwId).get("GHz") << " GHz" << endl;
    cout << "maxFreq:                               " << sgPtr1->getMaxFreq(spwId).get("GHz") << " GHz" << endl;
    cout << "Channel (grid units) for the min:      " << sgPtr1->getChanNum(spwId,sgPtr1->getMinFreq(spwId).get()) << endl;
    cout << "Channel (grid units) for the max:      " << sgPtr1->getChanNum(spwId,sgPtr1->getMaxFreq(spwId).get()) << endl;
    
    if(sgPtr1->isRegular(spwId)){
      cout << "the spectral window with id "<<spwId<<" is regularily sampled" << endl;
    }else{
      cout << "the spectral window with id "<<spwId<<" is not regularily sampled" << endl;
    }
    
    if(sgPtr1->getAssocSpwId(spwId).size()==0){
      cout << "the spectral window with id "<< spwId <<" has no associated spectral window" << endl;
    }else{
      for(unsigned int n=0; n<sgPtr1->getAssocSpwId(spwId).size(); n++){
	unsigned int assocSpwId = sgPtr1->getAssocSpwId(spwId)[n];
	cout << "the spectral window with id "<< spwId 
	     << " has the associated spec. win. with id " <<  assocSpwId 
	     << " (" <<  sgPtr1->getAssocNature(spwId)[n] << ")" << endl;
	
	for(unsigned int i=0; i<sgPtr1->getNumChan(spwId); i++){
	  cout << "chan index:" << i << " "
	       <<  sgPtr1->getSideband(spwId) <<" "<<sgPtr1->getChanFreq(spwId,i).get("GHz")<<"GHz  "
	       <<  sgPtr1->getAssocNature(spwId)[n] <<" "<<sgPtr1->getChanFreq(assocSpwId,i).get("GHz")<<"GHz"<<endl;
	}
      }
    }
    cout << endl;
    
    
    
  }
  
  cout << "TESTBED done" << endl;
  return 0;
}
#endif
