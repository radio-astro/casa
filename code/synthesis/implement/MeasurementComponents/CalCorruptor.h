//# StandardVisCal.h: Declaration of standard (Solvable)VisCal types
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

using namespace std;

#ifndef CASA_STANDALONE
#include <ATMRefractiveIndexProfile.h>
#include <ATMPercent.h>
#include <ATMPressure.h>
#include <ATMNumberDensity.h>
#include <ATMMassDensity.h>
#include <ATMTemperature.h>
#include <ATMLength.h>
#include <ATMInverseLength.h>
#include <ATMOpacity.h>
#include <ATMHumidity.h>
#include <ATMFrequency.h>
#include <ATMWaterVaporRadiometer.h>
#include <ATMWVRMeasurement.h>
#include <ATMAtmosphereType.h>
#include <ATMType.h>
#include <ATMProfile.h>
#include <ATMSpectralGrid.h>
#include <ATMRefractiveIndex.h>
#include <ATMSkyStatus.h>
#include <ATMTypeName.h>
#include <ATMAngle.h>
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
  inline Bool& initialized() { return initialized_; };
  inline Int& curr_slot() { return curr_slot_; };
  inline Double& curr_time() { return curr_time_; };
  inline Double& startTime() { return starttime_; };
  inline Double& stopTime() { return stoptime_; };
  inline Double& slot_time(const Int i) { return slot_times_(i); };
  inline Double& slot_time() { return slot_times_(curr_slot()); };
  inline Float& amp() { return amp_;};
  virtual void initialize() {};

  // a generic initializer that just takes amplitude and simpar
  void initialize(const Float amp, const Record& simpar) {
    amp_=amp;
    simpar_=simpar;
    initialized_=True;
  };
  inline Record& simpar() {return simpar_;}
  inline String& mode() { return mode_; };

  inline Int& currCorr() { return curr_corr_; };
  inline Int& nCorr() { return nCorr_; };
  inline Int& currChan() { return curr_chan_; };  
  inline Int& nChan() { return fnChan_[currSpw()]; };  

  // inherited from VC
  inline Int& prtlev() { return prtlev_; };
  inline Int& nAnt() { return nAnt_; };
  inline Int& nSpw() { return nSpw_; };  
  inline Int& currAnt() { return curr_ant_; };
  inline Int& currSpw() { return curr_spw_; };
  inline Vector<Float>& fRefFreq() { return fRefFreq_; };
  inline Vector<Float>& fWidth() { return fWidth_; };
  inline Vector<Int>& fnChan() { return fnChan_; };
 
 protected:
   
   Int nSim_;
   Bool initialized_;
   Int curr_slot_;
   Int nCorr_,curr_corr_,curr_chan_;
   Double curr_time_,starttime_,stoptime_;
   Float amp_;
   Vector<Double> slot_times_;   
   Record simpar_;
   String mode_; // general parameter for different kinds of corruptions

   Int prtlev_;
   Int nAnt_,curr_ant_,nSpw_,curr_spw_;
   Vector<Float> fRefFreq_,fWidth_; // for each spw
   Vector<Int> fnChan_;

 private:

};







class ANoiseCorruptor : public CalCorruptor {

  public:
    ANoiseCorruptor();
    virtual ~ANoiseCorruptor();
    //Float &amp() {return amplitude_; };
    virtual void initialize() {
      initialize(1234,1.0);
    }
    void initialize(const Int seed, const Float amp) {
      rndGen_p = new MLCG(seed);
      nDist_p = new Normal(rndGen_p, 0.0, 1.0); // sigma=1.
      amp_=amp;
      initialized_=True;
    };
    //Array<Complex> noise(const IPosition shape);
    Array<Complex> noise(const Int nrow, const Int ncol);

  private:
    //Float amplitude_;
    MLCG *rndGen_p;
    Normal *nDist_p;
  };










class TJonesCorruptor : public CalCorruptor {

 public:
   TJonesCorruptor(const Int nSim);
   virtual ~TJonesCorruptor();

   Float& pwv(const Int i); 
   Vector<Float>* pwv();
   void initAtm();
   inline Float& mean_pwv() { return mean_pwv_; };
   inline Matrix<Float>& screen() { return *screen_p; };
   inline Float screen(const Int i, const Int j) { 
     // RI_TODO out of bounds check or is that done by Vector?
     return screen_p->operator()(i,j); };
   virtual void initialize();
   void initialize(const Int Seed, const Float Beta, const Float scale);
   void initialize(const Int Seed, const Float Beta, const Float scale,
		   const ROMSAntennaColumns& antcols);
   Complex gain(const Int islot);
   Complex gain(const Int ix, const Int iy, const Int islot);
   inline Vector<Float>& antx() { return antx_; };
   inline Vector<Float>& anty() { return anty_; };
   inline Float& windspeed() { return windspeed_; };
   inline Float& pixsize() { return pixsize_; };

 protected:

 private:   
   Float mean_pwv_,windspeed_,pixsize_;
   Matrix<Float>* screen_p; 

   atm::AtmProfile *itsatm;
   atm::RefractiveIndexProfile *itsRIP;
   atm::SkyStatus *itsSkyStatus;
   atm::SpectralGrid *itsSpecGrid;

   PtrBlock<Vector<Float>*> pwv_p;
   Vector<Float> antx_,anty_;   
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










/// intended successor to TCorruptor, TfCorruptor, MMcorruptor, MfMcorruptor

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
     // RI_TODO out of bounds check or is that done by Vector?
     return screen_p->operator()(i,j); };
   virtual void initialize();
  // use ATM but no time dependence - e.g. for B[Tsys]
   void initialize(const Record& simpar);
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
   Float opac(const Int ichan);
   inline Float& tsys0() { return tsys0_; };  // const in T_A* scale
   inline Float& tsys1() { return tsys1_; };  // scale with exp(+tau)
   inline Bool& freqDep() { return freqdep_; };
   inline Float& tauscale() { return tauscale_; };

 protected:

 private:   
   Float mean_pwv_,windspeed_,pixsize_,tsys0_,tsys1_,tauscale_;
   Matrix<Float>* screen_p; 
   Bool freqdep_;

   atm::AtmProfile *itsatm;
   atm::RefractiveIndexProfile *itsRIP;
   atm::SkyStatus *itsSkyStatus;
   atm::SpectralGrid *itsSpecGrid;

   PtrBlock<Vector<Float>*> pwv_p;
   Vector<Float> antx_,anty_;   
};



}
#endif
