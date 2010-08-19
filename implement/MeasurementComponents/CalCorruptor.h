//# CalCorruptor.h
//# Copyright (C) 1996,1997,2000,2001,2002,2003
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#

#ifndef SYNTHESIS_CALCORRUPTOR_H
#define SYNTHESIS_CALCORRUPTOR_H

// for simulation 
#include <casa/BasicMath/Random.h>
#include <scimath/Mathematics/FFTServer.h>
#include <casa/Containers/Record.h>
#include <ms/MeasurementSets/MSAntennaColumns.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <synthesis/MeasurementComponents/VisCal.h>

using namespace std;

#ifndef CASA_STANDALONE
#include <ATM/ATMRefractiveIndexProfile.h>
#include <ATM/ATMPercent.h>
#include <ATM/ATMPressure.h>
#include <ATM/ATMNumberDensity.h>
#include <ATM/ATMMassDensity.h>
#include <ATM/ATMTemperature.h>
#include <ATM/ATMLength.h>
#include <ATM/ATMInverseLength.h>
#include <ATM/ATMOpacity.h>
#include <ATM/ATMHumidity.h>
#include <ATM/ATMFrequency.h>
#include <ATM/ATMWaterVaporRadiometer.h>
#include <ATM/ATMWVRMeasurement.h>
#include <ATM/ATMProfile.h>
#include <ATM/ATMSpectralGrid.h>
#include <ATM/ATMRefractiveIndex.h>
#include <ATM/ATMSkyStatus.h>
#include <ATM/ATMAngle.h>
#else
//#ATM Not available; mimic the classes and functions used
namespace atm{
class Angle
{
public:
  double get(string) const {return 0.0;}
};
class RefractiveIndexProfile
{
public:
  Angle getDispersiveWetPhaseDelay(int,int) const {return Angle();}
};
class AtmProfile;
class SkyStatus;
class SpectralGrid;

}
#endif


namespace casa { //# NAMESPACE CASA - BEGIN


// for simulating corruptions
class CalCorruptor {
  
 public:
  
  CalCorruptor(const Int nSim);
  virtual ~CalCorruptor();
  inline Int& nSim() { return nSim_; };
  inline Bool& times_initialized() { return times_initialized_; };
  inline Int& curr_slot() { return curr_slot_; };
  inline Double& curr_time() { return curr_time_; };
  inline Double& startTime() { return starttime_; };
  inline Double& stopTime() { return stoptime_; };
  inline Double& slot_time(const Int i) { return slot_times_(i); };
  inline Double& slot_time() { return slot_times_(curr_slot()); };
  inline Vector<Double>& slot_times() { return slot_times_; };
  inline Float& amp() { return amp_;};
  virtual void initialize() {};

  // a generic initializer that just takes amplitude and simpar
  void initialize(const Float amp, const Record& simpar) {
    amp_=amp;
    simpar_=simpar;   
  };
  inline Record& simpar() {return simpar_;}
  inline String& mode() { return mode_; };

  void setEvenSlots(const Double& dt);
  virtual Complex simPar(const VisIter& vi, VisCal::Type type,Int ipar);

  inline Int& nPar() { return nPar_; };
  inline Int& nChan() { return fnChan_[currSpw()]; };  
  inline const Int& focusChan() {return curr_chan_[currSpw()];};
  inline const Double& focusFreq() {return curr_freq_;};
  virtual void setFocusChan(Int chan) {
    curr_chan_[currSpw()]=chan;
    // WARN:  this assumes constant channel width - more detailed 
    // channel freq may be inaccurate
    Double fRes(fWidth()[currSpw()]/Double(fnChan()[currSpw()]));
    curr_freq_=fRefFreq()[currSpw()]+chan*fRes;
  };
  
  // inherited from VC
  inline Int& prtlev() { return prtlev_; };
  inline Int& nAnt() { return nAnt_; };
  inline Int& nSpw() { return nSpw_; };  
  inline Int& currAnt() { return curr_ant_; };
  inline Int& currAnt2() { return curr_ant2_; };
  inline Int& currSpw() { return curr_spw_; };
  inline Vector<Float>& fRefFreq() { return fRefFreq_; };
  inline Vector<Float>& fWidth() { return fWidth_; };
  inline Vector<Int>& fnChan() { return fnChan_; };
  inline Vector<Int>& currChans() { return curr_chan_; };

  inline Bool& freqDepPar() { return freqdep_; };
 
 protected:
   
   Int nSim_;
   Int curr_slot_;
   Bool times_initialized_,freqdep_;
   Int nPar_;
   Double curr_time_,starttime_,stoptime_,curr_freq_;
   Float amp_;
   Vector<Double> slot_times_;   
   Record simpar_;
   String mode_; // general parameter for different kinds of corruptions

   Int prtlev_;   
   Int nAnt_,curr_ant_,nSpw_,curr_spw_,curr_ant2_;
   Vector<Float> fRefFreq_,fWidth_; // for each spw
   Vector<Int> fnChan_,curr_chan_;

 private:

};







class ANoiseCorruptor : public CalCorruptor {

  public:
    ANoiseCorruptor(): CalCorruptor(1) {};
    virtual ~ANoiseCorruptor();
    virtual void initialize() {
      initialize(1234,1.0);
    }
    void initialize(const Int seed, const Float amp) {
      rndGen_p = new MLCG(seed);
      nDist_p = new Normal(rndGen_p, 0.0, 1.0); // sigma=1.
      amp_=amp;
    };
    virtual Complex simPar(const VisIter& vi,VisCal::Type type,Int ipar);
    virtual Complex simPar();

  private:
    MLCG *rndGen_p;
    Normal *nDist_p;
  };





// D is like ANoise but has a complex amplitude (different sigma in real/imag), and 
// a systematic offset
class DJonesCorruptor : public CalCorruptor {

  public:
    DJonesCorruptor(): CalCorruptor(1) {};
    virtual ~DJonesCorruptor();
    virtual void initialize() {
      initialize(1234,Complex(1.0,1.0),Complex(0.0));
    }
    void initialize(const Int seed, const Complex camp, const Complex offset) {
      rndGen_p = new MLCG(seed);
      nDist_p = new Normal(rndGen_p, 0.0, 1.0); // sigma=1.
      camp_=camp;
      offset_=offset;
    };
    virtual Complex simPar(const VisIter& vi,VisCal::Type type,Int ipar);
    inline Complex& camp() { return camp_; };
    inline Complex& offset() { return offset_; };

  private:
    MLCG *rndGen_p;
    Normal *nDist_p;
    Complex camp_,offset_;
  };







// this generates fractional brownian motion aka generalized 1/f noise
// class fBM : public Array<Double> {
class fBM {

 public:

  fBM(uInt i1);
  fBM(uInt i1, uInt i2);
  fBM(uInt i1, uInt i2, uInt i3);
  // virtual ~fBM(); // not ness if we don't derive from this
  inline Bool& initialized() { return initialized_; };
  void initialize(const Int seed, const Float beta);

  inline Array<Float> data() { return *data_; };
  inline Float data(uInt i1) { return data_->operator()(IPosition(1,i1)); };
  inline Float data(uInt i1, uInt i2) { return data_->operator()(IPosition(2,i1,i2)); };
  inline Float data(uInt i1, uInt i2, uInt i3) { return data_->operator()(IPosition(3,i1,i2,i3)); };


 private:
  Bool initialized_;
  Array<Float>* data_;

};






class AtmosCorruptor : public CalCorruptor {

 public:
   AtmosCorruptor();
   AtmosCorruptor(const Int nSim);
   virtual ~AtmosCorruptor();

   Float& pwv(const Int i); 
   Vector<Float>* pwv();
   void initAtm();
   inline Float& mean_pwv() { return mean_pwv_; };
   // pwv screen e.g. for a T
   inline Matrix<Float>& screen() { return *screen_p; };
   inline Float screen(const Int i, const Int j) { 
     return screen_p->operator()(i,j); };
   virtual void initialize();
   // use ATM but no time dependence - e.g. for B[Tsys]
   void initialize(const VisIter& vi, const Record& simpar);
   Vector<Double> antDiams;

   void initialize(const Int Seed, const Float Beta, const Float scale);
   void initialize(const Int Seed, const Float Beta, const Float scale,
		   const ROMSAntennaColumns& antcols);
   // phase corruption gain for a T
   Complex cphase(const Int islot);
   Complex cphase(const Int ix, const Int iy, const Int islot);
   inline Vector<Float>& antx() { return antx_; };
   inline Vector<Float>& anty() { return anty_; };
   inline Float& windspeed() { return windspeed_; };
   inline Float& pixsize() { return pixsize_; };

   inline Float& tauscale() { return tauscale_; };
   Float tsys(const Float& airmass);
   Float opac(const Int ichan);
   inline Float& spilleff() { return spilleff_; };

   inline Float& tground() { return tground_; };
   inline Float& tatmos() { return tatmos_; };
   inline Float& trx() { return trx_; };
   inline Float& tcmb() { return tcmb_; };
   // gets slow to calculate 1/exp(hv/kt)-1 all the time so 
   inline Double& Rtground() { return Rtground_; };
   inline Double& Rtatmos() { return Rtatmos_; };
   inline Double& Rtrx() { return Rtrx_; };
   inline Double& Rtcmb() { return Rtcmb_; };

   virtual Complex simPar(const VisIter& vi, VisCal::Type type,Int ipar);
   
   virtual void setFocusChan(Int chan) {
     curr_chan_[currSpw()]=chan;
     // WARN:  this assumes constant channel width - more detailed 
     // channel freq may be inaccurate
     Double fRes(fWidth()[currSpw()]/Double(fnChan()[currSpw()]));
     curr_freq_=fRefFreq()[currSpw()]+chan*fRes;
     // for temp calculations, recalculate the radiances 1/exp(hn/kt)-1
     double hn_k = 0.04799274551*1e-9*focusFreq(); 
     Rtcmb() = 1./(exp(hn_k/tcmb())-1.);
     Rtground() = 1./(exp(hn_k/tground())-1.);
     Rtrx() = 1./(exp(hn_k/trx())-1.);
     Rtatmos() = 1./(exp(hn_k/tatmos())-1.);
  };

 protected:

 private:   
   Float mean_pwv_,windspeed_,pixsize_,tauscale_,
     tground_,spilleff_,trx_,tatmos_,tcmb_;
   Double Rtatmos_,Rtcmb_,Rtrx_,Rtground_;
   Matrix<Float>* screen_p; 

   atm::AtmProfile *itsatm;
   atm::RefractiveIndexProfile *itsRIP;
   atm::SkyStatus *itsSkyStatus;
   atm::SpectralGrid *itsSpecGrid;

   PtrBlock<Vector<Float>*> pwv_p;
   Vector<Float> antx_,anty_;   
};





class GJonesCorruptor : public CalCorruptor {

 public:
   GJonesCorruptor(const Int nSim);
   virtual ~GJonesCorruptor();

   //Complex& drift(const Int i);  // drift as fBM
   Matrix<Complex>* drift();   
   inline Float& tsys() { return tsys_; };
   virtual void initialize();
   void initialize(const Int Seed, const Float Beta, const Float scale);
   Complex gain(const Int icorr, const Int islot);  // tsys scale and time-dep drift   
   virtual Complex simPar(const VisIter& vi, VisCal::Type type,Int ipar);

   // for the residual/gaussian noise
   void initialize(const Int seed, const Complex camp) {
     rndGen_p = new MLCG(seed);
     nDist_p = new Normal(rndGen_p, 0.0, 1.0); // sigma=1.
     camp_=camp;
    };
    inline Complex& camp() { return camp_; };

 protected:

 private:   
   Float tsys_;
   PtrBlock<Matrix<Complex>*> drift_p;
   // RI todo rearrange so there's a Gauss corruptor for AN,D,G, a fBMcorrupt,etc
   MLCG *rndGen_p;
   Normal *nDist_p;
   Complex camp_;
};





}
#endif
