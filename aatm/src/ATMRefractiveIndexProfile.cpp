#include <string>
#include <vector>
#include <iostream>
#include <math.h>

using namespace std;

#include "ATMRefractiveIndexProfile.h"

namespace atm {

  // Constructors 

  RefractiveIndexProfile::RefractiveIndexProfile( Frequency freq, AtmProfile atmProfile):
    AtmProfile(atmProfile), SpectralGrid(freq)
  {
    mkRefractiveIndexProfile();
  }

  RefractiveIndexProfile::RefractiveIndexProfile( AtmProfile atmProfile, Frequency freq):
    AtmProfile(atmProfile), SpectralGrid(freq)
  {
    mkRefractiveIndexProfile();
  }
  
  RefractiveIndexProfile::RefractiveIndexProfile( SpectralGrid spectralGrid, AtmProfile atmProfile):
    AtmProfile(atmProfile), SpectralGrid(spectralGrid)
  {
    mkRefractiveIndexProfile();
  }
  
  RefractiveIndexProfile::RefractiveIndexProfile( AtmProfile atmProfile, SpectralGrid spectralGrid):
    AtmProfile(atmProfile), SpectralGrid(spectralGrid)
  {
    mkRefractiveIndexProfile();
  }
  
  RefractiveIndexProfile::RefractiveIndexProfile( const RefractiveIndexProfile & a)
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
    
  }
  
  RefractiveIndexProfile::RefractiveIndexProfile(){ }
  
  RefractiveIndexProfile::~RefractiveIndexProfile()
  {
    //void rmRefractiveIndexProfile();
    rmRefractiveIndexProfile();
  }
  
  void RefractiveIndexProfile::rmRefractiveIndexProfile(){
    // for every frequency channel delete the pointer to the absorption profile
    for(unsigned int nc=0; nc<v_chanFreq_.size(); nc++){
      delete vv_N_H2OLinesPtr_[nc];
      delete vv_N_H2OContPtr_[nc];
      delete vv_N_O2LinesPtr_[nc];
      delete vv_N_DryContPtr_[nc];
      delete vv_N_O3LinesPtr_[nc];
      delete vv_N_COLinesPtr_[nc];
      delete vv_N_N2OLinesPtr_[nc];
    }
  }
  
  
  bool RefractiveIndexProfile::updateRefractiveIndexProfile( Length altitude, Pressure    groundPressure, 
							     Temperature groundTemperature, double  tropoLapseRate, 
							     Humidity relativeHumidity, Length wvScaleHeight)
  {
    
    bool updated = false;
    bool mkNewAtmProfile = updateAtmProfile( altitude, groundPressure, 
					     groundTemperature, tropoLapseRate, 
					     relativeHumidity, wvScaleHeight);
    unsigned int  numLayer     = getNumLayer();
    
    
    if(vv_N_H2OLinesPtr_.size() < v_chanFreq_.size()){
      mkNewAtmProfile=true; 
      cout<<" RefractiveIndexProfile: number of spectral windows has increased"<<endl;
    }
    if(mkNewAtmProfile){
      if(numLayer){
	mkRefractiveIndexProfile();
	updated=true;
      }else{
	cout << " RefractiveIndexProfile: ERROR:  getNumLayer() returns 0" << endl;
      }
    }
    return updated;
  }
   
  
  
  
  // NB: this interface is required because the sub-class .... overrides this method.
  bool  RefractiveIndexProfile::setBasicAtmosphericParameters(Length altitude, Pressure    groundPressure, 
							      Temperature groundTemperature, double  tropoLapseRate, 
							      Humidity relativeHumidity, Length wvScaleHeight)
  {
    bool updated = updateRefractiveIndexProfile( altitude, groundPressure, 
						 groundTemperature, tropoLapseRate, 
						 relativeHumidity, wvScaleHeight);
    return updated;
  }
  
  void RefractiveIndexProfile::mkRefractiveIndexProfile(){
    

    static const double abun_18o=0.0020439;
    static const double abun_17o=0.0003750;
    static const double abun_D=0.000298444;
    static const double o2_mixing_ratio=0.2092;
    static const double mmol_h2o=18.005059688;  //   20*0.0020439+19*(0.0003750+2*0.000298444)+18*(1-0.0020439-0.0003750-2*0.000298444)
    static const double pi=3.141592654;

    static bool first=true;

    double abun_O3, abun_CO, abun_N2O;
    double wvt, wv, t;
    double nu, nu2, nu_pi;
    RefractiveIndex atm;
    //    double sumAbsO3Lines1, sumAbsCOLines1, sumAbsN2OLines1;

    
    
    
    
    //TODO we will have to put numLayer_ and v_chanFreq_.size() const
    //we do not want to resize! ==> pas de setter pour SpectralGrid
    
    //cout << "vv_N_H2OLinesPtr_.size()=" << vv_N_H2OLinesPtr_.size() << endl;
    if(vv_N_H2OLinesPtr_.size()==0){                    // first time
      vv_N_H2OLinesPtr_.reserve(v_chanFreq_.size());
      vv_N_H2OContPtr_.reserve(v_chanFreq_.size());
      vv_N_O2LinesPtr_.reserve(v_chanFreq_.size());
      vv_N_DryContPtr_.reserve(v_chanFreq_.size());
      vv_N_O3LinesPtr_.reserve(v_chanFreq_.size());
      vv_N_COLinesPtr_.reserve(v_chanFreq_.size());
      vv_N_N2OLinesPtr_.reserve(v_chanFreq_.size());
    }else{
      if(vv_N_H2OLinesPtr_.size()==v_chanFreq_.size())    // there are new basic param
	rmRefractiveIndexProfile();  // delete all the layer profiles for all the frequencies
    }
    
    vector<complex<double> >* v_N_H2OLinesPtr;     
    vector<complex<double> >* v_N_H2OContPtr;      
    vector<complex<double> >* v_N_O2LinesPtr;      
    vector<complex<double> >* v_N_DryContPtr;      
    vector<complex<double> >* v_N_O3LinesPtr;      
    vector<complex<double> >* v_N_COLinesPtr;      
    vector<complex<double> >* v_N_N2OLinesPtr;     

    
    // cout << "v_chanFreq_.size()=" << v_chanFreq_.size() << endl;
    // cout << "numLayer_=" << numLayer_ << endl;
    // cout << "v_chanFreq_[0]=" << v_chanFreq_[0] << endl;
    // check if new spectral windows have been added
    unsigned int ncmin;
    /*  cout << "vv_N_H2OLinesPtr_.size()="<<vv_N_H2OLinesPtr_.size()<<endl; */
    ncmin=vv_N_H2OLinesPtr_.size();        // will be > 0 if spectral window(s) have been added
    if(newBasicParam_)ncmin=0;
    
    //    cout << "ncmin=" << ncmin << endl;

    for(unsigned int nc=ncmin; nc<v_chanFreq_.size(); nc++){
      
      
      v_N_H2OLinesPtr = new vector<complex<double> >;
      v_N_H2OContPtr  = new vector<complex<double> >;
      v_N_O2LinesPtr  = new vector<complex<double> >;
      v_N_DryContPtr  = new vector<complex<double> >;
      v_N_O3LinesPtr  = new vector<complex<double> >;
      v_N_COLinesPtr  = new vector<complex<double> >;
      v_N_N2OLinesPtr = new vector<complex<double> >;
      v_N_H2OLinesPtr->reserve(numLayer_);
      v_N_H2OContPtr->reserve(numLayer_);
      v_N_O2LinesPtr->reserve(numLayer_);
      v_N_DryContPtr->reserve(numLayer_);
      v_N_O3LinesPtr->reserve(numLayer_);
      v_N_COLinesPtr->reserve(numLayer_);
      v_N_N2OLinesPtr->reserve(numLayer_);
      
      nu    = 1.0E-9*v_chanFreq_[nc];    // ATM uses GHz units
      nu2   = nu*nu;
      nu_pi = nu/pi;
      
      for(unsigned int j=0; j<numLayer_; j++){
	

	wv  = v_layerWaterVapor_[j]*1000.0;       // se multiplica por 10**3 por cuestión de unidades en las rutinas fortran. 
	wvt = wv*v_layerTemperature_[j]/217.0;    // v_layerWaterVapor_[j] está en kg/m**3
	t = v_layerTemperature_[j]/300.0;


	//	complex<double>  o2lines = atm.getMeaningfulSpecificRefractivity_16o2(v_layerTemperature_[j],v_layerPressure_[j],wvt,nu);  
	//       o2lines = o2lines*(1.0-2.0*(abun_18o+abun_17o))*o2_mixing_ratio*(1.0-exp(-1556.38*1.43/v_layerTemperature_[j]))*v_layerPressure_[j]*100.0/(1.380662e-23*v_layerTemperature_[j]);
	
	//	complex<double>  o2lines_vib = atm.getMeaningfulSpecificRefractivity_16o2_vib(v_layerTemperature_[j],v_layerPressure_[j],wvt,nu);  
	//	o2lines_vib = o2lines_vib*(1.0-2.0*(abun_18o+abun_17o))*o2_mixing_ratio*exp(-1556.38*1.43/v_layerTemperature_[j])*v_layerPressure_[j]*100.0/(1.380662e-23*v_layerTemperature_[j]);
	
	//	complex<double>  o16o18lines = atm.getMeaningfulSpecificRefractivity_16o18o(v_layerTemperature_[j],v_layerPressure_[j],wvt,nu);  
	//	o16o18lines = o16o18lines*2.0*abun_18o*o2_mixing_ratio*v_layerPressure_[j]*100.0/(1.380662e-23*v_layerTemperature_[j]);
	
	//	complex<double>  o16o17lines = atm.getMeaningfulSpecificRefractivity_16o17o(v_layerTemperature_[j],v_layerPressure_[j],wvt,nu);  
	//	o16o17lines = o16o17lines*2.0*abun_17o*o2_mixing_ratio*v_layerPressure_[j]*100.0/(1.380662e-23*v_layerTemperature_[j]);
	
	
	//	v_N_O2LinesPtr->push_back(o2lines+o16o17lines+o16o18lines+o2lines_vib);

  	 v_N_O2LinesPtr->push_back(atm.getRefractivity_o2(v_layerTemperature_[j],v_layerPressure_[j],wvt,nu));

	
       	complex<double>  cont_h2o = atm.getMeaningfulSpecificRefractivity_cnth2o(v_layerTemperature_[j],v_layerPressure_[j],wvt,nu);
     	complex<double>  cont_dry = atm.getMeaningfulSpecificRefractivity_cntdry(v_layerTemperature_[j],v_layerPressure_[j],wvt,nu);

	v_N_H2OContPtr->push_back(cont_h2o);
	v_N_DryContPtr->push_back(cont_dry);


	if(v_layerWaterVapor_[j]>0){
	  
	  // complex<double>  h2olines = atm.getMeaningfulSpecificRefractivity_hh16o(v_layerTemperature_[j],v_layerPressure_[j],wvt,nu);  
	  //  h2olines = h2olines*(6.023e23*v_layerWaterVapor_[j]*1000/mmol_h2o)*(1-abun_18o-abun_17o-2.0*abun_D)*(1.0-exp(-2322.92/v_layerTemperature_[j]));  // (m^2 * m^-3 = m -1)

	  //  complex<double>  hh18olines = atm.getMeaningfulSpecificRefractivity_hh18o(v_layerTemperature_[j],v_layerPressure_[j],wvt,nu);  
	  //  hh18olines = hh18olines*(6.023e23*v_layerWaterVapor_[j]*1000/mmol_h2o)*abun_18o;  // (m^2 * m^-3 = m -1)

	  //  complex<double>  hh17olines = atm.getMeaningfulSpecificRefractivity_hh17o(v_layerTemperature_[j],v_layerPressure_[j],wvt,nu);  
	  //  hh17olines = hh17olines*(6.023e23*v_layerWaterVapor_[j]*1000/mmol_h2o)*abun_17o;  // (m^2 * m^-3 = m -1)

	  // HDO LINES DOES NOT WORK PROPERLY 5/3/2007 
	  //  complex<double>  hdolines = atm.getMeaningfulSpecificRefractivity_hdo(v_layerTemperature_[j],v_layerPressure_[j],wvt,nu);  
	  //  hdolines = hdolines*(6.023e23*v_layerWaterVapor_[j]*1000.0/mmol_h2o)*2.0*abun_D;  // (m^2 * m^-3 = m -1)

	  //  complex<double>  h2olines_v2 = atm.getMeaningfulSpecificRefractivity_hh16o_v2(v_layerTemperature_[j],v_layerPressure_[j],wvt,nu);  
	  //  h2olines_v2 = h2olines_v2*(6.023e23*v_layerWaterVapor_[j]*1000/mmol_h2o)*(1-abun_18o-abun_17o-2.0*abun_D)*exp(-2322.92/v_layerTemperature_[j]);  // (m^2 * m^-3 = m -1)


	  // v_N_H2OLinesPtr->push_back(h2olines+h2olines_v2+hh18olines+hh17olines);
	  
	  v_N_H2OLinesPtr->push_back(atm.getRefractivity_h2o(v_layerTemperature_[j],v_layerPressure_[j],wvt,nu));
	  
	} else {
	  v_N_H2OLinesPtr->push_back(0.0);
	}
	


	if(v_layerO3_[j]>0){
	  abun_O3=v_layerO3_[j]*1E-6;
	  v_N_O3LinesPtr->push_back(atm.getRefractivity_o3(v_layerTemperature_[j],v_layerPressure_[j],nu,abun_O3*1e6));
	  //	     v_N_O3LinesPtr->push_back(
	  //		       (atm.getMeaningfulSpecificRefractivity_16o16o16o(v_layerTemperature_[j],v_layerPressure_[j],nu)+
	  //			atm.getMeaningfulSpecificRefractivity_16o16o17o(v_layerTemperature_[j],v_layerPressure_[j],nu)*(2*abun_17o)+
	  //			atm.getMeaningfulSpecificRefractivity_16o16o18o(v_layerTemperature_[j],v_layerPressure_[j],nu)*(2*abun_18o)+
	  //			atm.getMeaningfulSpecificRefractivity_16o17o16o(v_layerTemperature_[j],v_layerPressure_[j],nu)*(abun_17o)+
	  //			atm.getMeaningfulSpecificRefractivity_16o18o16o(v_layerTemperature_[j],v_layerPressure_[j],nu)*(abun_18o))
	  //		       *abun_O3*1e6/(1.0+3.0*(abun_18o+abun_17o)));  // m^-1

	} else {
	  v_N_O3LinesPtr->push_back(0.0);
	}

	
	if(v_layerCO_[j]>0){
	  abun_CO=v_layerCO_[j]*1E-6;  // in cm^-3
	  v_N_COLinesPtr->push_back(atm.getMeaningfulSpecificRefractivity_co(v_layerTemperature_[j],v_layerPressure_[j],nu)*abun_CO*1e6);  // m^2 * m^-3 = m^-1
	} else {
	  v_N_COLinesPtr->push_back(0.0);
	}
	

	if(v_layerN2O_[j]>0){        
	  abun_N2O=v_layerN2O_[j]*1E-6;
	  v_N_N2OLinesPtr->push_back(atm.getMeaningfulSpecificRefractivity_n2o(v_layerTemperature_[j],v_layerPressure_[j],nu)*abun_N2O*1e6);  // m^2 * m^-3 = m^-1
	} else {
	  v_N_N2OLinesPtr->push_back(0.0);
	}


      }
      
      
      
      if(vv_N_H2OLinesPtr_.size()==0)first=true;
      


      if(vv_N_H2OLinesPtr_.size()<v_chanFreq_.size()){
	vv_N_H2OLinesPtr_.push_back(v_N_H2OLinesPtr); 
	vv_N_H2OContPtr_.push_back(v_N_H2OContPtr);
	vv_N_O2LinesPtr_.push_back(v_N_O2LinesPtr);
	vv_N_DryContPtr_.push_back(v_N_DryContPtr);
	vv_N_O3LinesPtr_.push_back(v_N_O3LinesPtr);
	vv_N_COLinesPtr_.push_back(v_N_COLinesPtr);
	vv_N_N2OLinesPtr_.push_back(v_N_N2OLinesPtr);
      }else{
	vv_N_H2OLinesPtr_[nc] = v_N_H2OLinesPtr;
	vv_N_H2OContPtr_[nc] = v_N_H2OContPtr;
	vv_N_O2LinesPtr_[nc] = v_N_O2LinesPtr;
	vv_N_DryContPtr_[nc] = v_N_DryContPtr;
	vv_N_O3LinesPtr_[nc] = v_N_O3LinesPtr;
	vv_N_COLinesPtr_[nc] = v_N_COLinesPtr;
	vv_N_N2OLinesPtr_[nc] = v_N_N2OLinesPtr;
      }
      
    }
    
    newBasicParam_     = false;
    first = false;
  }
  
  Opacity RefractiveIndexProfile::getDryOpacity(){
    return getDryOpacity(0);
  }
  
  Opacity RefractiveIndexProfile::getDryOpacity( unsigned int nc){
    if(!chanIndexIsValid(nc))return Opacity(-999.0);
    double kv = 0;
    for(unsigned int j=0; j<numLayer_; j++){
       kv = kv + imag( vv_N_O2LinesPtr_[nc]->at(j) + vv_N_DryContPtr_[nc]->at(j) + 
		  vv_N_O3LinesPtr_[nc]->at(j) + vv_N_COLinesPtr_[nc]->at(j) + 
		  vv_N_N2OLinesPtr_[nc]->at(j) )* v_layerThickness_[j]; 
    } 
    return Opacity(kv); 
  }
  
  Opacity RefractiveIndexProfile::getDryOpacity(unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc))return Opacity(-999.0);
    return getDryOpacity( v_transfertId_[spwid]+nc);
  }
  

  Opacity RefractiveIndexProfile::getAverageDryOpacity(unsigned int spwid){
    if(!spwidAndIndexAreValid(spwid,0))return Opacity(-999.0);
    Opacity totalaverage;
    totalaverage=Opacity(0.0,"np");
    for(unsigned int nc=0; nc<getNumChan(spwid); nc++){
      totalaverage=totalaverage+getDryOpacity(spwid,nc);
    }
    totalaverage=totalaverage/getNumChan(spwid);
    return totalaverage;}



  
  Opacity RefractiveIndexProfile::getDryContOpacity(){
    return getDryContOpacity(0);
  }
  
  Opacity RefractiveIndexProfile::getDryContOpacity(unsigned int nc){
    if(!chanIndexIsValid(nc))return Opacity(-999.0);
    double kv = 0;
    for(unsigned int j=0; j<numLayer_; j++){
      kv = kv + imag( vv_N_DryContPtr_[nc]->at(j) ) * v_layerThickness_[j]; 
    } 
    return Opacity(kv); 
  }

  Opacity RefractiveIndexProfile::getDryContOpacity(unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc))return Opacity(-999.0);
    return getDryContOpacity( v_transfertId_[spwid]+nc);
  }
  
  
  Opacity RefractiveIndexProfile::getO2LinesOpacity(){
    return getO2LinesOpacity(0);
  }
  
  Opacity RefractiveIndexProfile::getO2LinesOpacity(unsigned int nc){
    if(!chanIndexIsValid(nc))return Opacity(-999.0);
    double kv = 0;
    for(unsigned int j=0; j<numLayer_; j++){
      kv = kv + imag( vv_N_O2LinesPtr_[nc]->at(j) ) * v_layerThickness_[j]; 
    } 
    return Opacity(kv); 
  }
  
  Opacity RefractiveIndexProfile::getO2LinesOpacity(unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc))return Opacity(-999.0);
    return getO2LinesOpacity( v_transfertId_[spwid]+nc);
  }


  Opacity RefractiveIndexProfile::getCOLinesOpacity(){
    return getCOLinesOpacity(0); 
  }
  
  Opacity RefractiveIndexProfile::getCOLinesOpacity(unsigned int nc){
    if(!chanIndexIsValid(nc))return Opacity(-999.0);
    double kv = 0;
    for(unsigned int j=0; j<numLayer_; j++){
      kv = kv + imag( vv_N_COLinesPtr_[nc]->at(j) ) * v_layerThickness_[j]; 
    } 
    return Opacity(kv); 
  }

  Opacity RefractiveIndexProfile::getCOLinesOpacity(unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc))return Opacity(-999.0);
    return getCOLinesOpacity( v_transfertId_[spwid]+nc);
  }

  
  Opacity RefractiveIndexProfile::getN2OLinesOpacity(){
    return getN2OLinesOpacity(0); 
  }
  
  Opacity RefractiveIndexProfile::getN2OLinesOpacity(unsigned int nc){
    if(!chanIndexIsValid(nc))return Opacity(-999.0);
    double kv = 0;
    for(unsigned int j=0; j<numLayer_; j++){
      kv = kv + imag( vv_N_N2OLinesPtr_[nc]->at(j) ) * v_layerThickness_[j]; 
    } 
    return Opacity(kv); 
  }

  Opacity RefractiveIndexProfile::getN2OLinesOpacity(unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc))return Opacity(-999.0);
    return getN2OLinesOpacity( v_transfertId_[spwid]+nc);
  }

  
  Opacity RefractiveIndexProfile::getO3LinesOpacity(){
    return getO3LinesOpacity(0); 
  }
  
  Opacity RefractiveIndexProfile::getO3LinesOpacity(unsigned int nc){
    if(!chanIndexIsValid(nc))return Opacity(-999.0);
    double kv = 0;
    for(unsigned int j=0; j<numLayer_; j++){
      kv = kv + imag( vv_N_O3LinesPtr_[nc]->at(j) ) * v_layerThickness_[j]; 
    } 
    return Opacity(kv); 
  }

  Opacity RefractiveIndexProfile::getO3LinesOpacity(unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc))return Opacity(-999.0);
    return getO3LinesOpacity( v_transfertId_[spwid]+nc);
  }

  
  Opacity RefractiveIndexProfile::getWetOpacity(){
    return getWetOpacity(0);
  }
  
  
  Opacity RefractiveIndexProfile::getWetOpacity( unsigned int nc){
    if(!chanIndexIsValid(nc))return Opacity(-999.0);
    double kv = 0;
    /*  cout<<"nc="<<nc<<endl; */
    for(unsigned int j=0; j<numLayer_; j++){
      kv = kv + imag( vv_N_H2OLinesPtr_[nc]->at(j) + vv_N_H2OContPtr_[nc]->at(j) ) 
	* v_layerThickness_[j]; 



    } 
    return Opacity(kv); 
  }
  
  Opacity RefractiveIndexProfile::getWetOpacity(unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc))return Opacity(-999.0);
    return getWetOpacity( v_transfertId_[spwid]+nc);
  }
  
  Opacity RefractiveIndexProfile::getAverageWetOpacity(unsigned int spwid){
    if(!spwidAndIndexAreValid(spwid,0))return Opacity(-999.0);
    Opacity totalaverage;
    totalaverage=Opacity(0.0,"np");
    for(unsigned int nc=0; nc<getNumChan(spwid); nc++){
      totalaverage=totalaverage+getWetOpacity(spwid,nc);
    }
    totalaverage=totalaverage/getNumChan(spwid);
    return totalaverage;}



  Opacity RefractiveIndexProfile::getH2OLinesOpacity(){
    return getH2OLinesOpacity(0);
  }
  
  
  Opacity RefractiveIndexProfile::getH2OLinesOpacity( unsigned int nc){
    if(!chanIndexIsValid(nc))return Opacity(-999.0);
    double kv = 0;
    for(unsigned int j=0; j<numLayer_; j++){
      /*    cout <<"j="<<j<<" abs H2O Lines ="<<vv_N_H2OLinesPtr_[nc]->at(j) <<endl; */
      kv = kv + imag( vv_N_H2OLinesPtr_[nc]->at(j) ) 
	* v_layerThickness_[j]; 
    } 
    return Opacity(kv); 
  }
  
  Opacity RefractiveIndexProfile::getH2OLinesOpacity( unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc))return Opacity(-999.0);
    return getH2OLinesOpacity( v_transfertId_[spwid]+nc);
  }
  

  Opacity RefractiveIndexProfile::getAverageH2OLinesOpacity(unsigned int spwid){
    if(!spwidAndIndexAreValid(spwid,0))return Opacity(-999.0);
    Opacity totalaverage;
    totalaverage=Opacity(0.0,"np");
    for(unsigned int nc=0; nc<getNumChan(spwid); nc++){
      totalaverage=totalaverage+getH2OLinesOpacity(spwid,nc);
    }
    totalaverage=totalaverage/getNumChan(spwid);
    return totalaverage;}



  Opacity RefractiveIndexProfile::getH2OContOpacity(){
    return getH2OContOpacity(0);
  }
  
  
  Opacity RefractiveIndexProfile::getH2OContOpacity( unsigned int nc){
    if(!chanIndexIsValid(nc))return Opacity(-999.0);
    double kv = 0;
    for(unsigned int j=0; j<numLayer_; j++){
      kv = kv + imag( vv_N_H2OContPtr_[nc]->at(j) ) 
	* v_layerThickness_[j]; 
    } 
    return Opacity(kv); 
  }
  
  Opacity RefractiveIndexProfile::getH2OContOpacity( unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc))return Opacity(-999.0);
    return getH2OContOpacity( v_transfertId_[spwid]+nc);
  }
  
  
  Opacity RefractiveIndexProfile::getAverageH2OContOpacity(unsigned int spwid){
    if(!spwidAndIndexAreValid(spwid,0))return Opacity(-999.0);
    Opacity totalaverage;
    totalaverage=Opacity(0.0,"np");
    for(unsigned int nc=0; nc<getNumChan(spwid); nc++){
      totalaverage=totalaverage+getH2OLinesOpacity(spwid,nc);
    }
    totalaverage=totalaverage/getNumChan(spwid);
    return totalaverage;}



  Angle RefractiveIndexProfile::getDispersiveWetPhaseDelay(){
    return getDispersiveWetPhaseDelay(0);
  }
  
  Length RefractiveIndexProfile::getDispersiveWetPathLength(){
    return getDispersiveWetPathLength(0);
  }
  
  Angle RefractiveIndexProfile::getDispersiveWetPhaseDelay( unsigned int nc){
    if(!chanIndexIsValid(nc)){return Angle(-999.0,"deg");}
    double kv = 0;
    for(unsigned int j=0; j<numLayer_; j++){
      kv = kv + real( vv_N_H2OLinesPtr_[nc]->at(j) ) 
	* v_layerThickness_[j]; 
    } 
    Angle aa(kv*57.29578,"deg");
    return aa; 
  }
  
  Length RefractiveIndexProfile::getDispersiveWetPathLength( unsigned int nc){
    if(!chanIndexIsValid(nc)){return Length(-999.0,"m");}
    double wavelength =  299792458.0/v_chanFreq_[nc];    // in m
    Length ll((wavelength/360.0)*getDispersiveWetPhaseDelay(nc).get("deg"),"m");
    return ll;
  }
    
  Angle RefractiveIndexProfile::getDispersiveWetPhaseDelay(unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc)){return Angle(-999.0,"deg");}
    return getDispersiveWetPhaseDelay( v_transfertId_[spwid]+nc);
  }
  
  Angle RefractiveIndexProfile::getAverageDispersiveWetPhaseDelay(unsigned int spwid){
    if(!spwidAndIndexAreValid(spwid,0)){return Angle(-999.0,"deg");}
    double av=0.0;
    for(unsigned int i=0; i<getNumChan(spwid); i++){
      av=av+getDispersiveWetPhaseDelay( v_transfertId_[spwid]+i).get("deg");
    }
    av=av/getNumChan(spwid);
    Angle average(av,"deg");
    return average;
  }
  
  Length RefractiveIndexProfile::getDispersiveWetPathLength(unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc)){return Length (-999.0,"m");}
    return getDispersiveWetPathLength( v_transfertId_[spwid]+nc);
  }
    
  Length RefractiveIndexProfile::getAverageDispersiveWetPathLength(unsigned int spwid){
    if(!spwidAndIndexAreValid(spwid,0)){return Length (-999.0,"m");}
    double av=0.0;
    for(unsigned int i=0; i<getNumChan(spwid); i++){
      av=av+getDispersiveWetPathLength( v_transfertId_[spwid]+i).get("mm");
    }
    av=av/getNumChan(spwid);
    Length average(av,"mm");
    return average;
  }
  
  

  
  Angle RefractiveIndexProfile::getNonDispersiveDryPhaseDelay(){
    return getNonDispersiveDryPhaseDelay(0);
  }
  
  Length RefractiveIndexProfile::getNonDispersiveDryPathLength(){
    return getNonDispersiveDryPathLength(0);
  }
  
  Angle RefractiveIndexProfile::getNonDispersiveDryPhaseDelay( unsigned int nc){
    if(!chanIndexIsValid(nc)){return Angle(-999.0,"deg");}
    double kv = 0;
    for(unsigned int j=0; j<numLayer_; j++){
      kv = kv + real( vv_N_DryContPtr_[nc]->at(j) ) 
	* v_layerThickness_[j]; 
    } 
    Angle aa(kv*57.29578,"deg");
    return aa; 
  }
  
  Length RefractiveIndexProfile::getNonDispersiveDryPathLength( unsigned int nc){
    if(!chanIndexIsValid(nc)){return Length(-999.0,"m");}
    double wavelength =  299792458.0/v_chanFreq_[nc];    // in m
    Length ll((wavelength/360.0)*getNonDispersiveDryPhaseDelay(nc).get("deg"),"m");
    return ll;
  }
    
  Angle RefractiveIndexProfile::getNonDispersiveDryPhaseDelay(unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc)){return Angle(-999.0,"deg");}
    return getNonDispersiveDryPhaseDelay( v_transfertId_[spwid]+nc);
  }
  
  Angle RefractiveIndexProfile::getAverageNonDispersiveDryPhaseDelay(unsigned int spwid){
    if(!spwidAndIndexAreValid(spwid,0)){return Angle(-999.0,"deg");}
    double av=0.0;
    for(unsigned int i=0; i<getNumChan(spwid); i++){
      av=av+getNonDispersiveDryPhaseDelay( v_transfertId_[spwid]+i).get("deg");
    }
    av=av/getNumChan(spwid);
    Angle average(av,"deg");
    return average;
  }
  
  Length RefractiveIndexProfile::getNonDispersiveDryPathLength(unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc)){return Length (-999.0,"m");}
    return getNonDispersiveDryPathLength( v_transfertId_[spwid]+nc);
  }
    
  Length RefractiveIndexProfile::getAverageNonDispersiveDryPathLength(unsigned int spwid){
    if(!spwidAndIndexAreValid(spwid,0)){return Length (-999.0,"m");}
    double av=0.0;
    for(unsigned int i=0; i<getNumChan(spwid); i++){
      av=av+getNonDispersiveDryPathLength( v_transfertId_[spwid]+i).get("mm");
    }
    av=av/getNumChan(spwid);
    Length average(av,"mm");
    return average;
  }



  Angle RefractiveIndexProfile::getO2LinesPhaseDelay(){
    return getO2LinesPhaseDelay(0);
  }
  
  Length RefractiveIndexProfile::getO2LinesPathLength(){
    return getO2LinesPathLength(0);
  }
  
  Angle RefractiveIndexProfile::getO2LinesPhaseDelay( unsigned int nc){
    if(!chanIndexIsValid(nc)){return Angle(-999.0,"deg");}
    double kv = 0;
    for(unsigned int j=0; j<numLayer_; j++){
      kv = kv + real( vv_N_O2LinesPtr_[nc]->at(j) ) 
	* v_layerThickness_[j]; 
    } 
    Angle aa(kv*57.29578,"deg");
    return aa; 
  }
  
  Length RefractiveIndexProfile::getO2LinesPathLength( unsigned int nc){
    if(!chanIndexIsValid(nc)){return Length(-999.0,"m");}
    double wavelength =  299792458.0/v_chanFreq_[nc];    // in m
    Length ll((wavelength/360.0)*getO2LinesPhaseDelay(nc).get("deg"),"m");
    return ll;
  }
    
  Angle RefractiveIndexProfile::getO2LinesPhaseDelay(unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc)){return Angle(-999.0,"deg");}
    return getO2LinesPhaseDelay( v_transfertId_[spwid]+nc);
  }
  
  Angle RefractiveIndexProfile::getAverageO2LinesPhaseDelay(unsigned int spwid){
    if(!spwidAndIndexAreValid(spwid,0)){return Angle(-999.0,"deg");}
    double av=0.0;
    for(unsigned int i=0; i<getNumChan(spwid); i++){
      av=av+getO2LinesPhaseDelay( v_transfertId_[spwid]+i).get("deg");
    }
    av=av/getNumChan(spwid);
    Angle average(av,"deg");
    return average;
  }
  
  Length RefractiveIndexProfile::getO2LinesPathLength(unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc)){return Length (-999.0,"m");}
    return getO2LinesPathLength( v_transfertId_[spwid]+nc);
  }
    
  Length RefractiveIndexProfile::getAverageO2LinesPathLength(unsigned int spwid){
    if(!spwidAndIndexAreValid(spwid,0)){return Length (-999.0,"m");}
    double av=0.0;
    for(unsigned int i=0; i<getNumChan(spwid); i++){
      av=av+getO2LinesPathLength( v_transfertId_[spwid]+i).get("mm");
    }
    av=av/getNumChan(spwid);
    Length average(av,"mm");
    return average;
  }










  Angle RefractiveIndexProfile::getO3LinesPhaseDelay(){
    return getO3LinesPhaseDelay(0);
  }
  
  Length RefractiveIndexProfile::getO3LinesPathLength(){
    return getO3LinesPathLength(0);
  }
  
  Angle RefractiveIndexProfile::getO3LinesPhaseDelay( unsigned int nc){
    if(!chanIndexIsValid(nc)){return Angle(-999.0,"deg");}
    double kv = 0;
    for(unsigned int j=0; j<numLayer_; j++){
      kv = kv + real( vv_N_O3LinesPtr_[nc]->at(j) ) 
	* v_layerThickness_[j]; 
    } 
    Angle aa(kv*57.29578,"deg");
    return aa; 
  }
  
  Length RefractiveIndexProfile::getO3LinesPathLength( unsigned int nc){
    if(!chanIndexIsValid(nc)){return Length(-999.0,"m");}
    double wavelength =  299792458.0/v_chanFreq_[nc];    // in m
    Length ll((wavelength/360.0)*getO3LinesPhaseDelay(nc).get("deg"),"m");
    return ll;
  }
    
  Angle RefractiveIndexProfile::getO3LinesPhaseDelay(unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc)){return Angle(-999.0,"deg");}
    return getO3LinesPhaseDelay( v_transfertId_[spwid]+nc);
  }
  
  Angle RefractiveIndexProfile::getAverageO3LinesPhaseDelay(unsigned int spwid){
    if(!spwidAndIndexAreValid(spwid,0)){return Angle(-999.0,"deg");}
    double av=0.0;
    for(unsigned int i=0; i<getNumChan(spwid); i++){
      av=av+getO3LinesPhaseDelay( v_transfertId_[spwid]+i).get("deg");
    }
    av=av/getNumChan(spwid);
    Angle average(av,"deg");
    return average;
  }
  
  Length RefractiveIndexProfile::getO3LinesPathLength(unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc)){return Length (-999.0,"m");}
    return getO3LinesPathLength( v_transfertId_[spwid]+nc);
  }
    
  Length RefractiveIndexProfile::getAverageO3LinesPathLength(unsigned int spwid){
    if(!spwidAndIndexAreValid(spwid,0)){return Length (-999.0,"m");}
    double av=0.0;
    for(unsigned int i=0; i<getNumChan(spwid); i++){
      av=av+getO3LinesPathLength( v_transfertId_[spwid]+i).get("mm");
    }
    av=av/getNumChan(spwid);
    Length average(av,"mm");
    return average;
  }



  Angle RefractiveIndexProfile::getCOLinesPhaseDelay(){
    return getCOLinesPhaseDelay(0);
  }
  
  Length RefractiveIndexProfile::getCOLinesPathLength(){
    return getCOLinesPathLength(0);
  }
  
  Angle RefractiveIndexProfile::getCOLinesPhaseDelay( unsigned int nc){
    if(!chanIndexIsValid(nc)){return Angle(-999.0,"deg");}
    double kv = 0;
    for(unsigned int j=0; j<numLayer_; j++){
      kv = kv + real( vv_N_COLinesPtr_[nc]->at(j) ) 
	* v_layerThickness_[j]; 
    } 
    Angle aa(kv*57.29578,"deg");
    return aa; 
  }
  
  Length RefractiveIndexProfile::getCOLinesPathLength( unsigned int nc){
    if(!chanIndexIsValid(nc)){return Length(-999.0,"m");}
    double wavelength =  299792458.0/v_chanFreq_[nc];    // in m
    Length ll((wavelength/360.0)*getCOLinesPhaseDelay(nc).get("deg"),"m");
    return ll;
  }
    
  Angle RefractiveIndexProfile::getCOLinesPhaseDelay(unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc)){return Angle(-999.0,"deg");}
    return getCOLinesPhaseDelay( v_transfertId_[spwid]+nc);
  }
  
  Angle RefractiveIndexProfile::getAverageCOLinesPhaseDelay(unsigned int spwid){
    if(!spwidAndIndexAreValid(spwid,0)){return Angle(-999.0,"deg");}
    double av=0.0;
    for(unsigned int i=0; i<getNumChan(spwid); i++){
      av=av+getCOLinesPhaseDelay( v_transfertId_[spwid]+i).get("deg");
    }
    av=av/getNumChan(spwid);
    Angle average(av,"deg");
    return average;
  }
  
  Length RefractiveIndexProfile::getCOLinesPathLength(unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc)){return Length (-999.0,"m");}
    return getCOLinesPathLength( v_transfertId_[spwid]+nc);
  }
    
  Length RefractiveIndexProfile::getAverageCOLinesPathLength(unsigned int spwid){
    if(!spwidAndIndexAreValid(spwid,0)){return Length (-999.0,"m");}
    double av=0.0;
    for(unsigned int i=0; i<getNumChan(spwid); i++){
      av=av+getCOLinesPathLength( v_transfertId_[spwid]+i).get("mm");
    }
    av=av/getNumChan(spwid);
    Length average(av,"mm");
    return average;
  }





  Angle RefractiveIndexProfile::getN2OLinesPhaseDelay(){
    return getN2OLinesPhaseDelay(0);
  }
  
  Length RefractiveIndexProfile::getN2OLinesPathLength(){
    return getN2OLinesPathLength(0);
  }
  
  Angle RefractiveIndexProfile::getN2OLinesPhaseDelay( unsigned int nc){
    if(!chanIndexIsValid(nc)){return Angle(-999.0,"deg");}
    double kv = 0;
    for(unsigned int j=0; j<numLayer_; j++){
      kv = kv + real( vv_N_N2OLinesPtr_[nc]->at(j) ) 
	* v_layerThickness_[j]; 
    } 
    Angle aa(kv*57.29578,"deg");
    return aa; 
  }
  
  Length RefractiveIndexProfile::getN2OLinesPathLength( unsigned int nc){
    if(!chanIndexIsValid(nc)){return Length(-999.0,"m");}
    double wavelength =  299792458.0/v_chanFreq_[nc];    // in m
    Length ll((wavelength/360.0)*getN2OLinesPhaseDelay(nc).get("deg"),"m");
    return ll;
  }
    
  Angle RefractiveIndexProfile::getN2OLinesPhaseDelay(unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc)){return Angle(-999.0,"deg");}
    return getN2OLinesPhaseDelay( v_transfertId_[spwid]+nc);
  }
  
  Angle RefractiveIndexProfile::getAverageN2OLinesPhaseDelay(unsigned int spwid){
    if(!spwidAndIndexAreValid(spwid,0)){return Angle(-999.0,"deg");}
    double av=0.0;
    for(unsigned int i=0; i<getNumChan(spwid); i++){
      av=av+getN2OLinesPhaseDelay( v_transfertId_[spwid]+i).get("deg");
    }
    av=av/getNumChan(spwid);
    Angle average(av,"deg");
    return average;
  }
  
  Length RefractiveIndexProfile::getN2OLinesPathLength(unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc)){return Length (-999.0,"m");}
    return getN2OLinesPathLength( v_transfertId_[spwid]+nc);
  }
    
  Length RefractiveIndexProfile::getAverageN2OLinesPathLength(unsigned int spwid){
    if(!spwidAndIndexAreValid(spwid,0)){return Length (-999.0,"m");}
    double av=0.0;
    for(unsigned int i=0; i<getNumChan(spwid); i++){
      av=av+getN2OLinesPathLength( v_transfertId_[spwid]+i).get("mm");
    }
    av=av/getNumChan(spwid);
    Length average(av,"mm");
    return average;
  }



  Angle RefractiveIndexProfile::getNonDispersiveWetPhaseDelay(){
    return getNonDispersiveWetPhaseDelay(0); 
  }
  
  Length RefractiveIndexProfile::getNonDispersiveWetPathLength(){
    return getNonDispersiveWetPathLength(0); 
  }
  
  Angle RefractiveIndexProfile::getNonDispersiveWetPhaseDelay( unsigned int nc){
    double kv = 0;
    if(!chanIndexIsValid(nc)){return Angle(-999.0,"deg");}
    for(unsigned int j=0; j<numLayer_; j++){
      kv = kv + real( vv_N_H2OContPtr_[nc]->at(j) ) 
	* v_layerThickness_[j]; 
    } 
    Angle aa(kv*57.29578,"deg");
    return aa; 
  }
  
  Length RefractiveIndexProfile::getNonDispersiveWetPathLength( unsigned int nc){
    if(!chanIndexIsValid(nc)){return Length(-999.0,"m");}
    double wavelength =  299792458.0/v_chanFreq_[nc];    // in m
    Length ll((wavelength/360.0)*getNonDispersiveWetPhaseDelay(nc).get("deg"),"m");
    return ll;
  }
  
  Angle RefractiveIndexProfile::getNonDispersiveWetPhaseDelay(unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc)){return Angle(-999.0,"deg");}
    return getNonDispersiveWetPhaseDelay( v_transfertId_[spwid]+nc);
  }
  
  Angle RefractiveIndexProfile::getAverageNonDispersiveWetPhaseDelay(unsigned int spwid){
    if(!spwidAndIndexAreValid(spwid,0)){return Angle(-999.0,"deg");}
    double av=0.0;
    for(unsigned int i=0; i<getNumChan(spwid); i++){
      av=av+getNonDispersiveWetPhaseDelay( v_transfertId_[spwid]+i).get("deg");
    }
    av=av/getNumChan(spwid);
    Angle average(av,"deg");
    return average;
  }
  
  Length RefractiveIndexProfile::getNonDispersiveWetPathLength(unsigned int spwid, unsigned int nc){
    if(!spwidAndIndexAreValid(spwid,nc)){return Length(-999.0);}
    return getNonDispersiveWetPathLength( v_transfertId_[spwid]+nc);
  }
  
  Length RefractiveIndexProfile::getAverageNonDispersiveWetPathLength(unsigned int spwid){
    if(!spwidAndIndexAreValid(spwid,0)){return Length(-999.0);}
    double av=0.0;
    for(unsigned int i=0; i<getNumChan(spwid); i++){
      av=av+getNonDispersiveWetPathLength( v_transfertId_[spwid]+i).get("deg");
    }
    av=av/getNumChan(spwid);
    Length average(av,"deg");
    return average;
  }
  
  
  
  
  // NB: the function chanIndexIsValid will be overrided by ....
  bool RefractiveIndexProfile::chanIndexIsValid(unsigned int nc){
    if( nc < vv_N_H2OLinesPtr_.size() )return true;
    if( nc < v_chanFreq_.size() ){
      cout << " RefractiveIndexProfile: Requested index in a new spectral window ==> update profile" << endl;
      mkRefractiveIndexProfile();
      // cout << "...and we return" << endl;
      return true;
    }
    cout << " RefractiveIndexProfile: ERROR: Invalid channel frequency index" << endl;
    return false;
  }
  
  // NB: the function spwidAndIndexAreValid will be overrided by ...
  bool RefractiveIndexProfile::spwidAndIndexAreValid(unsigned int spwid, unsigned int idx){

    if(spwid>getNumSpectralWindow()-1 || spwid<0 ){
      cout << " RefractiveIndexProfile: ERROR: spectral window identifier out of range " << endl;
      return false;
    }
    if(idx>getNumChan(spwid)-1 || idx<0){
      cout << " RefractiveIndexProfile: ERROR: channel index out of range " << endl;
      return false;
    }
    unsigned int  nc = v_transfertId_[spwid]+idx;
    bool valid=chanIndexIsValid(nc);
    return valid;
  }
  
  void RefractiveIndexProfile::updateNewSpectralWindows(){
    mkRefractiveIndexProfile();
  }
  

  

}
