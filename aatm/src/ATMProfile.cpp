/*******************************************************************************
 * ALMA - Atacama Large Millimiter Array
 * (c) Institut de Radioastronomie Millimetrique, 2009
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 *
 * "@(#) $Id: ATMProfile.cpp,v 1.7 2009/09/10 16:22:17 dbroguie Exp $"
 *
 * who       when      what
 * --------  --------  ----------------------------------------------
 * pardo     24/03/09  created
 */

#include "ATMProfile.h"

#include <iostream>
#include <math.h>

using namespace std;

namespace atm
{

// Constructors

AtmProfile::AtmProfile()
{
}

AtmProfile::AtmProfile(unsigned int n)
{
  numLayer_ = n;
  for(unsigned int i = 0; i < numLayer_; n++) {
    v_layerO3_.push_back(0.0);
    v_layerCO_.push_back(0.0);
    v_layerN2O_.push_back(0.0);
    v_layerThickness_.push_back(0.0);
    v_layerTemperature_.push_back(0.0);
    v_layerPressure_.push_back(0.0);
    v_layerWaterVapor_.push_back(0.0);
  }
}

AtmProfile::AtmProfile(Length altitude,
                       Pressure groundPressure,
                       Temperature groundTemperature,
                       double tropoLapseRate,
                       Humidity relativeHumidity,
                       Length wvScaleHeight,
                       Pressure pressureStep,
                       double pressureStepFactor,
                       Length topAtmProfile,
                       unsigned int atmType) :
      //			 Atmospheretype atmType):
      typeAtm_(atmType), groundTemperature_(groundTemperature),
      tropoLapseRate_(tropoLapseRate), groundPressure_(groundPressure),
      relativeHumidity_(relativeHumidity), wvScaleHeight_(wvScaleHeight),
      pressureStep_(pressureStep), pressureStepFactor_(pressureStepFactor),
      altitude_(altitude), topAtmProfile_(topAtmProfile)
{
  numLayer_ = 0;
  numLayer_ = mkAtmProfile();
  newBasicParam_ = true;
}

AtmProfile::AtmProfile(Length altitude,
                       Pressure groundPressure,
                       Temperature groundTemperature,
                       double tropoLapseRate,
                       Humidity relativeHumidity,
                       Length wvScaleHeight,
                       unsigned int atmType) :
      //			 Atmospheretype atmType):
      typeAtm_(atmType), groundTemperature_(groundTemperature),
      tropoLapseRate_(tropoLapseRate), groundPressure_(groundPressure),
      relativeHumidity_(relativeHumidity), wvScaleHeight_(wvScaleHeight),
      pressureStep_(10.0, "mb"), pressureStepFactor_(1.2), altitude_(altitude),
      topAtmProfile_(48.0, "km")
{
  numLayer_ = 0;
  numLayer_ = mkAtmProfile();
  newBasicParam_ = true;
}

AtmProfile::AtmProfile(vector<Length> v_layerThickness,
                       vector<Pressure> v_layerPressure,
                       vector<Temperature> v_layerTemperature,
                       vector<MassDensity> v_layerWaterVapor)
{
  newBasicParam_ = true;
  unsigned int nL1 = v_layerThickness.size();
  unsigned int nL2 = v_layerPressure.size();
  unsigned int nL3 = v_layerTemperature.size();
  unsigned int nL4 = v_layerWaterVapor.size();

  if(nL1 == nL2 && nL2 == nL3 && nL3 == nL4) {
    numLayer_ = nL1;
    for(unsigned int n = 0; n < numLayer_; n++) {
      v_layerO3_.push_back(0.0);
      v_layerCO_.push_back(0.0);
      v_layerN2O_.push_back(0.0);
      v_layerThickness_.push_back(v_layerThickness[n].get("m"));
      v_layerTemperature_.push_back(v_layerTemperature[n].get("K"));
      v_layerPressure_.push_back(v_layerPressure[n].get("mb"));
      v_layerWaterVapor_.push_back(v_layerWaterVapor[n].get("kgm**-3"));
    }
  } else {
    numLayer_ = 0;
  }
}

AtmProfile::AtmProfile(vector<Length> v_layerThickness,
                       vector<Pressure> v_layerPressure,
                       vector<Temperature> v_layerTemperature,
                       vector<NumberDensity> v_layerWaterVapor)
{
  newBasicParam_ = true;
  unsigned int nL1 = v_layerThickness.size();
  unsigned int nL2 = v_layerPressure.size();
  unsigned int nL3 = v_layerTemperature.size();
  unsigned int nL4 = v_layerWaterVapor.size();

  if(nL1 == nL2 && nL2 == nL3 && nL3 == nL4) {
    numLayer_ = nL1;
    for(unsigned int n = 0; n < numLayer_; n++) {
      v_layerO3_.push_back(0.0);
      v_layerCO_.push_back(0.0);
      v_layerN2O_.push_back(0.0);
      v_layerThickness_.push_back(v_layerThickness[n].get("m"));
      v_layerTemperature_.push_back(v_layerTemperature[n].get("K"));
      v_layerPressure_.push_back(v_layerPressure[n].get("mb"));
      v_layerWaterVapor_.push_back(v_layerWaterVapor[n].get("m**-3") * 18.0
          / (1000.0 * 6.023e23));
    }
  } else {
    numLayer_ = 0;
  }
}

AtmProfile::AtmProfile(vector<Length> v_layerThickness,
                       vector<Pressure> v_layerPressure,
                       vector<Temperature> v_layerTemperature,
                       vector<MassDensity> v_layerWaterVapor,
                       vector<NumberDensity> v_layerO3)
{
  newBasicParam_ = true;
  unsigned int nL1 = v_layerThickness.size();
  unsigned int nL2 = v_layerPressure.size();
  unsigned int nL3 = v_layerTemperature.size();
  unsigned int nL4 = v_layerWaterVapor.size();
  unsigned int nL5 = v_layerO3.size();

  if(nL1 == nL2 && nL2 == nL3 && nL3 == nL4 && nL4 == nL5) {
    numLayer_ = nL1;
    for(unsigned int n = 0; n < numLayer_; n++) {
      v_layerO3_.push_back(v_layerO3[n].get("m**-3"));
      v_layerCO_.push_back(0.0);
      v_layerN2O_.push_back(0.0);
      v_layerThickness_.push_back(v_layerThickness[n].get("m"));
      v_layerTemperature_.push_back(v_layerTemperature[n].get("K"));
      v_layerPressure_.push_back(v_layerPressure[n].get("mb"));
      v_layerWaterVapor_.push_back(v_layerWaterVapor[n].get("kgm**-3"));
    }
  } else {
    numLayer_ = 0;
  }
}

AtmProfile::AtmProfile(vector<Length> v_layerThickness,
                       vector<Pressure> v_layerPressure,
                       vector<Temperature> v_layerTemperature,
                       vector<NumberDensity> v_layerWaterVapor,
                       vector<NumberDensity> v_layerO3)
{
  newBasicParam_ = true;
  unsigned int nL1 = v_layerThickness.size();
  unsigned int nL2 = v_layerPressure.size();
  unsigned int nL3 = v_layerTemperature.size();
  unsigned int nL4 = v_layerWaterVapor.size();
  unsigned int nL5 = v_layerO3.size();

  if(nL1 == nL2 && nL2 == nL3 && nL3 == nL4 && nL4 == nL5) {
    numLayer_ = nL1;
    for(unsigned int n = 0; n < numLayer_; n++) {
      v_layerO3_.push_back(v_layerO3[n].get("m**-3"));
      v_layerCO_.push_back(0.0);
      v_layerN2O_.push_back(0.0);
      v_layerThickness_.push_back(v_layerThickness[n].get("m"));
      v_layerTemperature_.push_back(v_layerTemperature[n].get("K"));
      v_layerPressure_.push_back(v_layerPressure[n].get("mb"));
      v_layerWaterVapor_.push_back(v_layerWaterVapor[n].get("m**-3") * 18.0
          / (1000.0 * 6.023e23));
    }
  } else {
    numLayer_ = 0;
  }
}

AtmProfile::AtmProfile(vector<Length> v_layerThickness,
                       vector<Pressure> v_layerPressure,
                       vector<Temperature> v_layerTemperature,
                       vector<MassDensity> v_layerWaterVapor,
                       vector<NumberDensity> v_layerO3,
                       vector<NumberDensity> v_layerCO,
                       vector<NumberDensity> v_layerN2O)
{
  newBasicParam_ = true;
  unsigned int nL1 = v_layerThickness.size();
  unsigned int nL2 = v_layerPressure.size();
  unsigned int nL3 = v_layerTemperature.size();
  unsigned int nL4 = v_layerWaterVapor.size();
  unsigned int nL5 = v_layerO3.size();
  unsigned int nL6 = v_layerCO.size();
  unsigned int nL7 = v_layerN2O.size();

  if(nL1 == nL2 && nL2 == nL3 && nL3 == nL4 && nL4 == nL5 && nL5 == nL6 && nL6
      == nL7) {
    numLayer_ = nL1;
    for(unsigned int n = 0; n < numLayer_; n++) {
      v_layerO3_.push_back(v_layerO3[n].get("m**-3"));
      v_layerCO_.push_back(v_layerCO[n].get("m**-3"));
      v_layerN2O_.push_back(v_layerN2O[n].get("m**-3"));
      v_layerThickness_.push_back(v_layerThickness[n].get("m"));
      v_layerTemperature_.push_back(v_layerTemperature[n].get("K"));
      v_layerPressure_.push_back(v_layerPressure[n].get("mb"));
      v_layerWaterVapor_.push_back(v_layerWaterVapor[n].get("kgm**-3"));
    }
  } else {
    numLayer_ = 0;
  }
}

AtmProfile::AtmProfile(vector<Length> v_layerThickness,
                       vector<Pressure> v_layerPressure,
                       vector<Temperature> v_layerTemperature,
                       vector<NumberDensity> v_layerWaterVapor,
                       vector<NumberDensity> v_layerO3,
                       vector<NumberDensity> v_layerCO,
                       vector<NumberDensity> v_layerN2O)
{
  newBasicParam_ = true;
  unsigned int nL1 = v_layerThickness.size();
  unsigned int nL2 = v_layerPressure.size();
  unsigned int nL3 = v_layerTemperature.size();
  unsigned int nL4 = v_layerWaterVapor.size();
  unsigned int nL5 = v_layerO3.size();
  unsigned int nL6 = v_layerCO.size();
  unsigned int nL7 = v_layerN2O.size();

  if(nL1 == nL2 && nL2 == nL3 && nL3 == nL4 && nL4 == nL5 && nL5 == nL6 && nL6
      == nL7) {
    numLayer_ = nL1;
    for(unsigned int n = 0; n < numLayer_; n++) {
      v_layerO3_.push_back(v_layerO3[n].get("m**-3"));
      v_layerCO_.push_back(v_layerCO[n].get("m**-3"));
      v_layerN2O_.push_back(v_layerN2O[n].get("m**-3"));
      v_layerThickness_.push_back(v_layerThickness[n].get("m"));
      v_layerTemperature_.push_back(v_layerTemperature[n].get("K"));
      v_layerPressure_.push_back(v_layerPressure[n].get("mb"));
      v_layerWaterVapor_.push_back(v_layerWaterVapor[n].get("m**-3") * 18.0
          / (1000.0 * 6.023e23));
    }
  } else {
    numLayer_ = 0;
  }
}

AtmProfile::AtmProfile(const AtmProfile & a)
{ //:AtmType(a.type_){
  // cout<<"AtmProfile copy constructor"<<endl;  COMMENTED OUT BY JUAN MAY/16/2005
  typeAtm_ = a.typeAtm_;
  groundTemperature_ = a.groundTemperature_;
  tropoLapseRate_ = a.tropoLapseRate_;
  groundPressure_ = a.groundPressure_;
  relativeHumidity_ = a.relativeHumidity_;
  wvScaleHeight_ = a.wvScaleHeight_;
  pressureStep_ = a.pressureStep_;
  pressureStepFactor_ = a.pressureStepFactor_;
  altitude_ = a.altitude_;
  topAtmProfile_ = a.topAtmProfile_;
  numLayer_ = a.numLayer_;
  newBasicParam_ = a.newBasicParam_;
  v_layerThickness_.reserve(numLayer_);
  v_layerTemperature_.reserve(numLayer_);
  v_layerWaterVapor_.reserve(numLayer_);
  v_layerCO_.reserve(numLayer_);
  v_layerO3_.reserve(numLayer_);
  v_layerN2O_.reserve(numLayer_);
  // cout << "numLayer_=" << numLayer_ << endl;  COMMENTED OUT BY JUAN MAY/16/2005
  for(unsigned int n = 0; n < numLayer_; n++) {
    v_layerThickness_.push_back(a.v_layerThickness_[n]);
    v_layerTemperature_.push_back(a.v_layerTemperature_[n]);
    //cout << "n=" << n << endl;
    v_layerWaterVapor_.push_back(a.v_layerWaterVapor_[n]);
    v_layerPressure_.push_back(a.v_layerPressure_[n]);
    v_layerCO_.push_back(a.v_layerCO_[n]);
    v_layerO3_.push_back(a.v_layerO3_[n]);
    v_layerN2O_.push_back(a.v_layerN2O_[n]);
  }
}

AtmProfile::~AtmProfile()
{
}

bool AtmProfile::updateAtmProfile(Length altitude,
                                  Pressure groundPressure,
                                  Temperature groundTemperature,
                                  double tropoLapseRate,
                                  Humidity relativeHumidity,
                                  Length wvScaleHeight)
{

  /* TODO A faire: pour decider s'il faut recalculer le profile on devrait plutot donner des seuils, eg
   if(fabs(altitude_.get()-altitude.get())>0.1)mkNewProfile=true;
   */

  unsigned int numLayer;
  bool mkNewProfile = false;
  if(altitude_.get() != altitude.get()) mkNewProfile = true; //if(mkNewProfile)cout<<"altitude has changed"          <<endl;
  if(groundPressure_.get() != groundPressure.get()) mkNewProfile = true; //if(mkNewProfile)cout<<"ground pressure has changed"   <<endl;
  if(groundTemperature_.get() != groundTemperature.get()) mkNewProfile = true; //if(mkNewProfile)cout<<"ground temperature has changed"<<endl;
  if(wvScaleHeight_.get() != wvScaleHeight.get()) mkNewProfile = true; //if(mkNewProfile)cout<<"wv scale height has changed"   <<endl;
  if(tropoLapseRate_ != tropoLapseRate) mkNewProfile = true; //if(mkNewProfile)cout<<"tropo lapse rate has changed"  <<endl;
  if(relativeHumidity_.get() != relativeHumidity.get()) mkNewProfile = true; //if(mkNewProfile)cout<<"relative humidity has changed" <<endl;
  if(mkNewProfile) {
    newBasicParam_ = true;
    altitude_ = altitude;
    groundPressure_ = groundPressure;
    groundTemperature_ = groundTemperature;
    tropoLapseRate_ = tropoLapseRate;
    relativeHumidity_ = relativeHumidity;
    wvScaleHeight_ = wvScaleHeight;
    numLayer = mkAtmProfile();
    numLayer_ = numLayer;
    //      cout << "There are new basic parameters, with " << numLayer_ << " layers " << endl;
  } else {
    numLayer = getNumLayer();
    numLayer_ = numLayer;
  }

  return mkNewProfile;
}

// Note that this setBasicAtmosphericParameters will be overrided by the subclasses.
bool AtmProfile::setBasicAtmosphericParameters(Length altitude,
                                               Pressure groundPressure,
                                               Temperature groundTemperature,
                                               double tropoLapseRate,
                                               Humidity relativeHumidity,
                                               Length wvScaleHeight)
{
  bool newAtmProfile = updateAtmProfile(altitude,
                                        groundPressure,
                                        groundTemperature,
                                        tropoLapseRate,
                                        relativeHumidity,
                                        wvScaleHeight);
  return newAtmProfile;
}


vector<Temperature> AtmProfile::getTemperatureProfile()
{
  vector<Temperature> t;
  t.reserve(v_layerTemperature_.size());
  for(unsigned int i = 0; i < v_layerTemperature_.size(); i++) {
    Temperature tt(v_layerTemperature_[i], "K");
    t.push_back(tt);
  }
  return t;
}

Temperature AtmProfile::getLayerTemperature(unsigned int i)
{
  if(i > v_layerTemperature_.size() - 1) {
    Temperature t(-999.0, "K");
    return t;
  } else {
    Temperature t(v_layerTemperature_[i], "K");
    return t;
  }
}

void AtmProfile::setLayerTemperature(unsigned int i, Temperature layerTemperature)
{
  if(i < v_layerTemperature_.size()) {
    v_layerTemperature_[i] = layerTemperature.get("K");
  }
}

vector<Length> AtmProfile::getThicknessProfile()
{
  vector<Length> l;
  l.reserve(v_layerThickness_.size());
  for(unsigned int i = 0; i < v_layerThickness_.size(); i++) {
    Length ll(v_layerThickness_[i], "m");
    l.push_back(ll);
  }
  return l;
}

Length AtmProfile::getLayerThickness(unsigned int i)
{
  if(i > v_layerThickness_.size() - 1) {
    Length l(-999.0, "m");
    return l;
  } else {
    Length l(v_layerThickness_[i], "m");
    return l;
  }
}

void AtmProfile::setLayerThickness(unsigned int i, Length layerThickness)
{
  if(i < v_layerThickness_.size()) {
    v_layerThickness_[i] = layerThickness.get("m");
  }
}

MassDensity AtmProfile::getLayerWaterVaporMassDensity(unsigned int i)
{
  if(i > v_layerWaterVapor_.size() - 1) {
    MassDensity m(-999.0, "kgm**-3");
    return m;
  } else {
    MassDensity m(v_layerWaterVapor_[i], "kgm**-3");
    return m;
  }
}

NumberDensity AtmProfile::getLayerWaterVaporNumberDensity(unsigned int i)
{
  if(i > v_layerWaterVapor_.size() - 1) {
    NumberDensity m(-999.0, "m**-3");
    return m;
  } else {
    NumberDensity
      m(v_layerWaterVapor_[i] * 6.023e23 * 1000.0 / 18.0, "m**-3");
    return m;
  }
}

void AtmProfile::setLayerWaterVaporMassDensity(unsigned int i, MassDensity layerWaterVapor)
{
  if(i <= v_layerWaterVapor_.size() - 1) {
    v_layerWaterVapor_[i] = layerWaterVapor.get("kgm**-3");
  }
}

void AtmProfile::setLayerWaterVaporNumberDensity(unsigned int i, NumberDensity layerWaterVapor)
{
  if(i <= v_layerWaterVapor_.size() - 1) {
    v_layerWaterVapor_[i] = layerWaterVapor.get("m**-3") * 18.0 / (6.023e23* 1000.0);
  }
}

vector<Pressure> AtmProfile::getPressureProfile()
{
  vector<Pressure> p;
  p.reserve(v_layerPressure_.size());
  for(unsigned int i = 0; i < v_layerPressure_.size(); i++) {
    Pressure pp(v_layerPressure_[i], "mb");
    p.push_back(pp);
  }
  return p;
}

Pressure AtmProfile::getLayerPressure(unsigned int i)
{
  if(i > v_layerPressure_.size() - 1) {
    Pressure p(-999.0, "mb");
    return p;
  } else {
    Pressure p(v_layerPressure_[i], "mb");
    return p;
  }
}



string AtmProfile::getAtmosphereType()
{
  return getAtmosphereType(typeAtm_);
}

string AtmProfile::getAtmosphereType(unsigned int typeAtm)
{
  string type;
  string typeNames[] = { "TROPICAL", "MIDLATSUMMER", "MIDLATWINTER",
			 "SUBARTSUMMER", "SUBARTWINTER"};

  if(typeAtm < typeATM_end) {
    type = typeNames[typeAtm-1];
  } else {
    type = "UNKNOWN";
  }

  return type;

}

Length AtmProfile::getGroundWH2O()
{
  double wm = 0;
  for(unsigned int j = 0; j < numLayer_; j++) {
    wm = wm + v_layerWaterVapor_[j] * v_layerThickness_[j]; // kg/m**2 or mm (from m*kg/m**3 IS units)
  }
  wm = wm * 1e-3; // (pasar de mm a m)
  Length wh2o(wm);
  return wh2o;
}

MassDensity AtmProfile::rwat(Temperature tt, Humidity rh, Pressure pp)
{

  double t = tt.get("K");
  double p = pp.get("mb");
  double u = rh.get("%");
  double e, es, rwat0;

  if(p <= 0 || t <= 0 || u <= 0) {
    return MassDensity(0.0, "gm**-3");
  } else {
    es = 6.105 * exp(25.22 / t * (t - 273.0) - 5.31 * log(t / 273.0));
    e = 1.0 - (1.0 - u / 100.0) * es / p;
    e = es * u / 100.0 / e;
    rwat0 = e * 216.502 / t; //(en g/m*3)
  }
  return MassDensity(rwat0, "gm**-3");

}

Humidity AtmProfile::rwat_inv(Temperature tt, MassDensity dd, Pressure pp)
{

  double p = pp.get("mb");
  double t = tt.get("K");
  double r = dd.get("gm**-3");
  double es, e, rinv;

  if(p <= 0 || t <= 0 || r <= 0) {
    rinv = 0.0;
  } else {
    es = 6.105 * exp(25.22 / t * (t - 273.0) - 5.31 * log(t / 273.0));
    e = r * t / 216.502;
    rinv = 100 * (e * (p - es) / (es * (p - e)));
    if(rinv < 0 && p < 3) rinv = 0.0;
  }
  return Humidity(rinv, "%");
}

vector<NumberDensity> AtmProfile::st76(Length h, unsigned int tip)
{

  unsigned int i1, i2, i3, i_layer;
  double x1, x2, x3, d;
  vector<NumberDensity> minorden;
  NumberDensity o3den, n2oden, coden;
  static const double avogad = 6.022045E+23;
  static const double airmwt = 28.964;
  //    static const double h2omwt=18.015;
  double ha = h.get("km");

  //      /,AMWT/18.015,44.010,47.998,44.01,28.011,
  //     16.043,31.999,30.01,64.06,46.01,17.03,63.01,17.00,20.01,
  //     36.46,80.92,127.91,51.45,60.08,30.03,52.46,28.014,
  //     27.03, 50.49, 34.01, 26.03, 30.07, 34.00,66.0,146.,0.,0.,
  //     0.,0.,0./
  //     -----------------------------------------------------------------

  //     ALT (KM)  /

  static const double
      alt[50] = { 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0, 18.0, 19.0, 20.0, 21.0, 22.0, 23.0, 24.0, 25.0, 27.5, 30.0, 32.5, 35.0, 37.5, 40.0, 42.5, 45.0, 47.5, 50.0, 55.0, 60.0, 65.0, 70.0, 75.0, 80.0, 85.0, 90.0, 95.0, 100.0, 105.0, 110.0, 115.0, 120.0 };

  //     PRESSURE  /

  /*      static const double p[6][50]={
   {1.013E+03, 9.040E+02, 8.050E+02, 7.150E+02, 6.330E+02,
   5.590E+02, 4.920E+02, 4.320E+02, 3.780E+02, 3.290E+02,
   2.860E+02, 2.470E+02, 2.130E+02, 1.820E+02, 1.560E+02,
   1.320E+02, 1.110E+02, 9.370E+01, 7.890E+01, 6.660E+01,
   5.650E+01, 4.800E+01, 4.090E+01, 3.500E+01, 3.000E+01,
   2.570E+01, 1.763E+01, 1.220E+01, 8.520E+00, 6.000E+00,
   4.260E+00, 3.050E+00, 2.200E+00, 1.590E+00, 1.160E+00,
   8.540E-01, 4.560E-01, 2.390E-01, 1.210E-01, 5.800E-02,
   2.600E-02, 1.100E-02, 4.400E-03, 1.720E-03, 6.880E-04,
   2.890E-04, 1.300E-04, 6.470E-05, 3.600E-05, 2.250E-05},
   {1.013E+03, 9.020E+02, 8.020E+02, 7.100E+02, 6.280E+02,
   5.540E+02, 4.870E+02, 4.260E+02, 3.720E+02, 3.240E+02,
   2.810E+02, 2.430E+02, 2.090E+02, 1.790E+02, 1.530E+02,
   1.300E+02, 1.110E+02, 9.500E+01, 8.120E+01, 6.950E+01,
   5.950E+01, 5.100E+01, 4.370E+01, 3.760E+01, 3.220E+01,
   2.770E+01, 1.907E+01, 1.320E+01, 9.300E+00, 6.520E+00,
   4.640E+00, 3.330E+00, 2.410E+00, 1.760E+00, 1.290E+00,
   9.510E-01, 5.150E-01, 2.720E-01, 1.390E-01, 6.700E-02,
   3.000E-02, 1.200E-02, 4.480E-03, 1.640E-03, 6.250E-04,
   2.580E-04, 1.170E-04, 6.110E-05, 3.560E-05, 2.270E-05},
   {1.018E+03, 8.973E+02, 7.897E+02, 6.938E+02, 6.081E+02,
   5.313E+02, 4.627E+02, 4.016E+02, 3.473E+02, 2.993E+02,
   2.568E+02, 2.199E+02, 1.882E+02, 1.611E+02, 1.378E+02,
   1.178E+02, 1.007E+02, 8.610E+01, 7.360E+01, 6.280E+01,
   5.370E+01, 4.580E+01, 3.910E+01, 3.340E+01, 2.860E+01,
   2.440E+01, 1.646E+01, 1.110E+01, 7.560E+00, 5.180E+00,
   3.600E+00, 2.530E+00, 1.800E+00, 1.290E+00, 9.400E-01,
   6.830E-01, 3.620E-01, 1.880E-01, 9.500E-02, 4.700E-02,
   2.220E-02, 1.030E-02, 4.560E-03, 1.980E-03, 8.770E-04,
   4.074E-04, 2.000E-04, 1.057E-04, 5.980E-05, 3.600E-05},
   {1.010E+03, 8.960E+02, 7.929E+02, 7.000E+02, 6.160E+02,
   5.410E+02, 4.740E+02, 4.130E+02, 3.590E+02, 3.108E+02,
   2.677E+02, 2.300E+02, 1.977E+02, 1.700E+02, 1.460E+02,
   1.260E+02, 1.080E+02, 9.280E+01, 7.980E+01, 6.860E+01,
   5.900E+01, 5.070E+01, 4.360E+01, 3.750E+01, 3.228E+01,
   2.780E+01, 1.923E+01, 1.340E+01, 9.400E+00, 6.610E+00,
   4.720E+00, 3.400E+00, 2.480E+00, 1.820E+00, 1.340E+00,
   9.870E-01, 5.370E-01, 2.880E-01, 1.470E-01, 7.100E-02,
   3.200E-02, 1.250E-02, 4.510E-03, 1.610E-03, 6.060E-04,
   2.480E-04, 1.130E-04, 6.000E-05, 3.540E-05, 2.260E-05},
   {1.013E+03, 8.878E+02, 7.775E+02, 6.798E+02, 5.932E+02,
   5.158E+02, 4.467E+02, 3.853E+02, 3.308E+02, 2.829E+02,
   2.418E+02, 2.067E+02, 1.766E+02, 1.510E+02, 1.291E+02,
   1.103E+02, 9.431E+01, 8.058E+01, 6.882E+01, 5.875E+01,
   5.014E+01, 4.277E+01, 3.647E+01, 3.109E+01, 2.649E+01,
   2.256E+01, 1.513E+01, 1.020E+01, 6.910E+00, 4.701E+00,
   3.230E+00, 2.243E+00, 1.570E+00, 1.113E+00, 7.900E-01,
   5.719E-01, 2.990E-01, 1.550E-01, 7.900E-02, 4.000E-02,
   2.000E-02, 9.660E-03, 4.500E-03, 2.022E-03, 9.070E-04,
   4.230E-04, 2.070E-04, 1.080E-04, 6.000E-05, 3.590E-05},
   {1.013E+03, 8.988E+02, 7.950E+02, 7.012E+02, 6.166E+02,
   5.405E+02, 4.722E+02, 4.111E+02, 3.565E+02, 3.080E+02,
   2.650E+02, 2.270E+02, 1.940E+02, 1.658E+02, 1.417E+02,
   1.211E+02, 1.035E+02, 8.850E+01, 7.565E+01, 6.467E+01,
   5.529E+01, 4.729E+01, 4.047E+01, 3.467E+01, 2.972E+01,
   2.549E+01, 1.743E+01, 1.197E+01, 8.010E+00, 5.746E+00,
   4.150E+00, 2.871E+00, 2.060E+00, 1.491E+00, 1.090E+00,
   7.978E-01, 4.250E-01, 2.190E-01, 1.090E-01, 5.220E-02,
   2.400E-02, 1.050E-02, 4.460E-03, 1.840E-03, 7.600E-04,
   3.200E-04, 1.450E-04, 7.100E-05, 4.010E-05, 2.540E-05}};

   //     TEMPERATURE/

   static const double t[6][50]={
   {299.70,    293.70,    287.70,    283.70,    277.00,
   270.30,    263.60,    257.00,    250.30,    243.60,
   237.00,    230.10,    223.60,    217.00,    210.30,
   203.70,    197.00,    194.80,    198.80,    202.70,
   206.70,    210.70,    214.60,    217.00,    219.20,
   221.40,    227.00,    232.30,    237.70,    243.10,
   248.50,    254.00,    259.40,    264.80,    269.60,
   270.20,    263.40,    253.10,    236.00,    218.90,
   201.80,    184.80,    177.10,    177.00,    184.30,
   190.70,    212.00,    241.60,    299.70,    380.00},
   {294.20,    289.70,    285.20,    279.20,    273.20,
   267.20,    261.20,    254.70,    248.20,    241.70,
   235.30,    228.80,    222.30,    215.80,    215.70,
   215.70,    215.70,    215.70,    216.80,    217.90,
   219.20,    220.40,    221.60,    222.80,    223.90,
   225.10,    228.45,    233.70,    239.00,    245.20,
   251.30,    257.50,    263.70,    269.90,    275.20,
   275.70,    269.30,    257.10,    240.10,    218.10,
   196.10,    174.10,    165.10,    165.00,    178.30,
   190.50,    222.20,    262.40,    316.80,    380.00},
   {272.20,    268.70,    265.20,    261.70,    255.70,
   249.70,    243.70,    237.70,    231.70,    225.70,
   219.70,    219.20,    218.70,    218.20,    217.70,
   217.20,    216.70,    216.20,    215.70,    215.20,
   215.20,    215.20,    215.20,    215.20,    215.20,
   215.20,    215.50,    217.40,    220.40,    227.90,
   235.50,    243.20,    250.80,    258.50,    265.10,
   265.70,    260.60,    250.80,    240.90,    230.70,
   220.40,    210.10,    199.80,    199.50,    208.30,
   218.60,    237.10,    259.50,    293.00,    333.00},
   {287.20,    281.70,    276.30,    270.90,    265.50,
   260.10,    253.10,    246.10,    239.20,    232.20,
   225.20,    225.20,    225.20,    225.20,    225.20,
   225.20,    225.20,    225.20,    225.20,    225.20,
   225.20,    225.20,    225.20,    225.20,    226.60,
   228.10,    231.00,    235.10,    240.00,    247.20,
   254.60,    262.10,    269.50,    273.60,    276.20,
   277.20,    274.00,    262.70,    239.70,    216.60,
   193.60,    170.60,    161.70,    161.60,    176.80,
   190.40,    226.00,    270.10,    322.70,    380.00},
   {257.20,    259.10,    255.90,    252.70,    247.70,
   240.90,    234.10,    227.30,    220.60,    217.20,
   217.20,    217.20,    217.20,    217.20,    217.20,
   217.20,    216.60,    216.00,    215.40,    214.80,
   214.20,    213.60,    213.00,    212.40,    211.80,
   211.20,    213.60,    216.00,    218.50,    222.30,
   228.50,    234.70,    240.80,    247.00,    253.20,
   259.30,    259.10,    250.90,    248.40,    245.40,
   234.70,    223.90,    213.10,    202.30,    211.00,
   218.50,    234.00,    252.60,    288.50,    333.00},
   {288.20,    281.70,    275.20,    268.70,    262.20,
   255.70,    249.20,    242.70,    236.20,    229.70,
   223.30,    216.80,    216.70,    216.70,    216.70,
   216.70,    216.70,    216.70,    216.70,    216.70,
   216.70,    217.60,    218.60,    219.60,    220.60,
   221.60,    224.00,    226.50,    230.00,    236.50,
   242.90,    250.40,    257.30,    264.20,    270.60,
   270.70,    260.80,    247.00,    233.30,    219.60,
   208.40,    198.60,    188.90,    186.90,    188.40,
   195.10,    208.80,    240.00,    300.00,    360.00}};

   //     DATA  H2O      /

   static const double agua[6][50]={
   {2.593E+04, 1.949E+04, 1.534E+04, 8.600E+03, 4.441E+03,
   3.346E+03, 2.101E+03, 1.289E+03, 7.637E+02, 4.098E+02,
   1.912E+02, 7.306E+01, 2.905E+01, 9.900E+00, 6.220E+00,
   4.000E+00, 3.000E+00, 2.900E+00, 2.750E+00, 2.600E+00,
   2.600E+00, 2.650E+00, 2.800E+00, 2.900E+00, 3.200E+00,
   3.250E+00, 3.600E+00, 4.000E+00, 4.300E+00, 4.600E+00,
   4.900E+00, 5.200E+00, 5.500E+00, 5.700E+00, 5.900E+00,
   6.000E+00, 6.000E+00, 6.000E+00, 5.400E+00, 4.500E+00,
   3.300E+00, 2.100E+00, 1.300E+00, 8.500E-01, 5.400E-01,
   4.000E-01, 3.400E-01, 2.800E-01, 2.400E-01, 2.000E-01},
   {1.876E+04, 1.378E+04, 9.680E+03, 5.984E+03, 3.813E+03,
   2.225E+03, 1.510E+03, 1.020E+03, 6.464E+02, 4.129E+02,
   2.472E+02, 9.556E+01, 2.944E+01, 8.000E+00, 5.000E+00,
   3.400E+00, 3.300E+00, 3.200E+00, 3.150E+00, 3.200E+00,
   3.300E+00, 3.450E+00, 3.600E+00, 3.850E+00, 4.000E+00,
   4.200E+00, 4.450E+00, 4.700E+00, 4.850E+00, 4.950E+00,
   5.000E+00, 5.100E+00, 5.300E+00, 5.450E+00, 5.500E+00,
   5.500E+00, 5.350E+00, 5.000E+00, 4.400E+00, 3.700E+00,
   2.950E+00, 2.100E+00, 1.330E+00, 8.500E-01, 5.400E-01,
   4.000E-01, 3.400E-01, 2.800E-01, 2.400E-01, 2.000E-01},
   {4.316E+03, 3.454E+03, 2.788E+03, 2.088E+03, 1.280E+03,
   8.241E+02, 5.103E+02, 2.321E+02, 1.077E+02, 5.566E+01,
   2.960E+01, 1.000E+01, 6.000E+00, 5.000E+00, 4.800E+00,
   4.700E+00, 4.600E+00, 4.500E+00, 4.500E+00, 4.500E+00,
   4.500E+00, 4.500E+00, 4.530E+00, 4.550E+00, 4.600E+00,
   4.650E+00, 4.700E+00, 4.750E+00, 4.800E+00, 4.850E+00,
   4.900E+00, 4.950E+00, 5.000E+00, 5.000E+00, 5.000E+00,
   4.950E+00, 4.850E+00, 4.500E+00, 4.000E+00, 3.300E+00,
   2.700E+00, 2.000E+00, 1.330E+00, 8.500E-01, 5.400E-01,
   4.000E-01, 3.400E-01, 2.800E-01, 2.400E-01, 2.000E-01},
   {1.194E+04, 8.701E+03, 6.750E+03, 4.820E+03, 3.380E+03,
   2.218E+03, 1.330E+03, 7.971E+02, 3.996E+02, 1.300E+02,
   4.240E+01, 1.330E+01, 6.000E+00, 4.450E+00, 4.000E+00,
   4.000E+00, 4.000E+00, 4.050E+00, 4.300E+00, 4.500E+00,
   4.600E+00, 4.700E+00, 4.800E+00, 4.830E+00, 4.850E+00,
   4.900E+00, 4.950E+00, 5.000E+00, 5.000E+00, 5.000E+00,
   5.000E+00, 5.000E+00, 5.000E+00, 5.000E+00, 5.000E+00,
   4.950E+00, 4.850E+00, 4.500E+00, 4.000E+00, 3.300E+00,
   2.700E+00, 2.000E+00, 1.330E+00, 8.500E-01, 5.400E-01,
   4.000E-01, 3.400E-01, 2.800E-01, 2.400E-01, 2.000E-01},
   {1.405E+03, 1.615E+03, 1.427E+03, 1.166E+03, 7.898E+02,
   4.309E+02, 2.369E+02, 1.470E+02, 3.384E+01, 2.976E+01,
   2.000E+01, 1.000E+01, 6.000E+00, 4.450E+00, 4.500E+00,
   4.550E+00, 4.600E+00, 4.650E+00, 4.700E+00, 4.750E+00,
   4.800E+00, 4.850E+00, 4.900E+00, 4.950E+00, 5.000E+00,
   5.000E+00, 5.000E+00, 5.000E+00, 5.000E+00, 5.000E+00,
   5.000E+00, 5.000E+00, 5.000E+00, 5.000E+00, 5.000E+00,
   4.950E+00, 4.850E+00, 4.500E+00, 4.000E+00, 3.300E+00,
   2.700E+00, 2.000E+00, 1.330E+00, 8.500E-01, 5.400E-01,
   4.000E-01, 3.400E-01, 2.800E-01, 2.400E-01, 2.000E-01},
   {7.745E+03, 6.071E+03, 4.631E+03, 3.182E+03, 2.158E+03,
   1.397E+03, 9.254E+02, 5.720E+02, 3.667E+02, 1.583E+02,
   6.996E+01, 3.613E+01, 1.906E+01, 1.085E+01, 5.927E+00,
   5.000E+00, 3.950E+00, 3.850E+00, 3.825E+00, 3.850E+00,
   3.900E+00, 3.975E+00, 4.065E+00, 4.200E+00, 4.300E+00,
   4.425E+00, 4.575E+00, 4.725E+00, 4.825E+00, 4.900E+00,
   4.950E+00, 5.025E+00, 5.150E+00, 5.225E+00, 5.250E+00,
   5.225E+00, 5.100E+00, 4.750E+00, 4.200E+00, 3.500E+00,
   2.825E+00, 2.050E+00, 1.330E+00, 8.500E-01, 5.400E-01,
   4.000E-01, 3.400E-01, 2.800E-01, 2.400E-01, 2.000E-01}};

   */

  //     DATA CO2       /
  //      DATA AMOL12/
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.280E+02, 3.200E+02, 3.100E+02, 2.700E+02,
  //     C 1.950E+02, 1.100E+02, 6.000E+01, 4.000E+01, 3.500E+01/
  //      DATA AMOL22/
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.280E+02, 3.200E+02, 3.100E+02, 2.700E+02,
  //     C 1.950E+02, 1.100E+02, 6.000E+01, 4.000E+01, 3.500E+01/
  //      DATA AMOL32/
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.280E+02, 3.200E+02, 3.100E+02, 2.700E+02,
  //     C 1.950E+02, 1.100E+02, 6.000E+01, 4.000E+01, 3.500E+01/
  //      DATA AMOL42/
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.280E+02, 3.200E+02, 3.100E+02, 2.700E+02,
  //     C 1.950E+02, 1.100E+02, 6.000E+01, 4.000E+01, 3.500E+01/
  //      DATA AMOL52/
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.280E+02, 3.200E+02, 3.100E+02, 2.700E+02,
  //     C 1.950E+02, 1.100E+02, 6.000E+01, 4.000E+01, 3.500E+01/
  //      DATA AMOL62/
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02, 3.300E+02,
  //     C 3.300E+02, 3.280E+02, 3.200E+02, 3.100E+02, 2.700E+02,
  //     C 1.950E+02, 1.100E+02, 6.000E+01, 4.000E+01, 3.500E+01/
  //     DATA OZONE     /

  static const double
      ozone[6][50] = { { 2.869E-02, 3.150E-02, 3.342E-02, 3.504E-02, 3.561E-02, 3.767E-02, 3.989E-02, 4.223E-02, 4.471E-02, 5.000E-02, 5.595E-02, 6.613E-02, 7.815E-02, 9.289E-02, 1.050E-01, 1.256E-01, 1.444E-01, 2.500E-01, 5.000E-01, 9.500E-01, 1.400E+00, 1.800E+00, 2.400E+00, 3.400E+00, 4.300E+00, 5.400E+00, 7.800E+00, 9.300E+00, 9.850E+00, 9.700E+00, 8.800E+00, 7.500E+00, 5.900E+00, 4.500E+00, 3.450E+00, 2.800E+00, 1.800E+00, 1.100E+00, 6.500E-01, 3.000E-01, 1.800E-01, 3.300E-01, 5.000E-01, 5.200E-01, 5.000E-01, 4.000E-01, 2.000E-01, 5.000E-02, 5.000E-03, 5.000E-04 }, { 3.017E-02, 3.337E-02, 3.694E-02, 4.222E-02, 4.821E-02, 5.512E-02, 6.408E-02, 7.764E-02, 9.126E-02, 1.111E-01, 1.304E-01, 1.793E-01, 2.230E-01, 3.000E-01, 4.400E-01, 5.000E-01, 6.000E-01, 7.000E-01, 1.000E+00, 1.500E+00, 2.000E+00, 2.400E+00, 2.900E+00, 3.400E+00, 4.000E+00, 4.800E+00, 6.000E+00, 7.000E+00, 8.100E+00, 8.900E+00, 8.700E+00, 7.550E+00, 5.900E+00, 4.500E+00, 3.500E+00, 2.800E+00, 1.800E+00, 1.300E+00, 8.000E-01, 4.000E-01, 1.900E-01, 2.000E-01, 5.700E-01, 7.500E-01, 7.000E-01, 4.000E-01, 2.000E-01, 5.000E-02, 5.000E-03, 5.000E-04 }, { 2.778E-02, 2.800E-02, 2.849E-02, 3.200E-02, 3.567E-02, 4.720E-02, 5.837E-02, 7.891E-02, 1.039E-01, 1.567E-01, 2.370E-01, 3.624E-01, 5.232E-01, 7.036E-01, 8.000E-01, 9.000E-01, 1.100E+00, 1.400E+00, 1.800E+00, 2.300E+00, 2.900E+00, 3.500E+00, 3.900E+00, 4.300E+00, 4.700E+00, 5.100E+00, 5.600E+00, 6.100E+00, 6.800E+00, 7.100E+00, 7.200E+00, 6.900E+00, 5.900E+00, 4.600E+00, 3.700E+00, 2.750E+00, 1.700E+00, 1.000E-00, 5.500E-01, 3.200E-01, 2.500E-01, 2.300E-01, 5.500E-01, 8.000E-01, 8.000E-01, 4.000E-01, 2.000E-01, 5.000E-02, 5.000E-03, 5.000E-04 }, { 2.412E-02, 2.940E-02, 3.379E-02, 3.887E-02, 4.478E-02, 5.328E-02, 6.564E-02, 7.738E-02, 9.114E-02, 1.420E-01, 1.890E-01, 3.050E-01, 4.100E-01, 5.000E-01, 6.000E-01, 7.000E-01, 8.500E-01, 1.000E+00, 1.300E+00, 1.700E+00, 2.100E+00, 2.700E+00, 3.300E+00, 3.700E+00, 4.200E+00, 4.500E+00, 5.300E+00, 5.700E+00, 6.900E+00, 7.700E+00, 7.800E+00, 7.000E+00, 5.400E+00, 4.200E+00, 3.200E+00, 2.500E+00, 1.700E+00, 1.200E+00, 8.000E-01, 4.000E-01, 2.000E-01, 1.800E-01, 6.500E-01, 9.000E-01, 8.000E-01, 4.000E-01, 2.000E-01, 5.000E-02, 5.000E-03, 5.000E-04 }, { 1.802E-02, 2.072E-02, 2.336E-02, 2.767E-02, 3.253E-02, 3.801E-02, 4.446E-02, 7.252E-02, 1.040E-01, 2.100E-01, 3.000E-01, 3.500E-01, 4.000E-01, 6.500E-01, 9.000E-01, 1.200E+00, 1.500E+00, 1.900E+00, 2.450E+00, 3.100E+00, 3.700E+00, 4.000E+00, 4.200E+00, 4.500E+00, 4.600E+00, 4.700E+00, 4.900E+00, 5.400E+00, 5.900E+00, 6.200E+00, 6.250E+00, 5.900E+00, 5.100E+00, 4.100E+00, 3.000E+00, 2.600E+00, 1.600E+00, 9.500E-01, 6.500E-01, 5.000E-01, 3.300E-01, 1.300E-01, 7.500E-01, 8.000E-01, 8.000E-01, 4.000E-01, 2.000E-01, 5.000E-02, 5.000E-03, 5.000E-04 }, { 2.660E-02, 2.931E-02, 3.237E-02, 3.318E-02, 3.387E-02, 3.768E-02, 4.112E-02, 5.009E-02, 5.966E-02, 9.168E-02, 1.313E-01, 2.149E-01, 3.095E-01, 3.846E-01, 5.030E-01, 6.505E-01, 8.701E-01, 1.187E+00, 1.587E+00, 2.030E+00, 2.579E+00, 3.028E+00, 3.647E+00, 4.168E+00, 4.627E+00, 5.118E+00, 5.803E+00, 6.553E+00, 7.373E+00, 7.837E+00, 7.800E+00, 7.300E+00, 6.200E+00, 5.250E+00, 4.100E+00, 3.100E+00, 1.800E+00, 1.100E+00, 7.000E-01, 3.000E-01, 2.500E-01, 3.000E-01, 5.000E-01, 7.000E-01, 7.000E-01, 4.000E-01, 2.000E-01, 5.000E-02, 5.000E-03, 5.000E-04 } };

  //     DATA  N2O      /

  static const double
      n2o[6][50] = { { 3.200E-01, 3.200E-01, 3.200E-01, 3.200E-01, 3.200E-01, 3.200E-01, 3.200E-01, 3.200E-01, 3.200E-01, 3.195E-01, 3.179E-01, 3.140E-01, 3.095E-01, 3.048E-01, 2.999E-01, 2.944E-01, 2.877E-01, 2.783E-01, 2.671E-01, 2.527E-01, 2.365E-01, 2.194E-01, 2.051E-01, 1.967E-01, 1.875E-01, 1.756E-01, 1.588E-01, 1.416E-01, 1.165E-01, 9.275E-02, 6.693E-02, 4.513E-02, 2.751E-02, 1.591E-02, 9.378E-03, 4.752E-03, 3.000E-03, 2.065E-03, 1.507E-03, 1.149E-03, 8.890E-04, 7.056E-04, 5.716E-04, 4.708E-04, 3.932E-04, 3.323E-04, 2.837E-04, 2.443E-04, 2.120E-04, 1.851E-04 }, { 3.200E-01, 3.200E-01, 3.200E-01, 3.200E-01, 3.200E-01, 3.200E-01, 3.200E-01, 3.200E-01, 3.195E-01, 3.163E-01, 3.096E-01, 2.989E-01, 2.936E-01, 2.860E-01, 2.800E-01, 2.724E-01, 2.611E-01, 2.421E-01, 2.174E-01, 1.843E-01, 1.607E-01, 1.323E-01, 1.146E-01, 1.035E-01, 9.622E-02, 8.958E-02, 8.006E-02, 6.698E-02, 4.958E-02, 3.695E-02, 2.519E-02, 1.736E-02, 1.158E-02, 7.665E-03, 5.321E-03, 3.215E-03, 2.030E-03, 1.397E-03, 1.020E-03, 7.772E-04, 6.257E-04, 5.166E-04, 4.352E-04, 3.727E-04, 3.237E-04, 2.844E-04, 2.524E-04, 2.260E-04, 2.039E-04, 1.851E-04 }, { 3.200E-01, 3.200E-01, 3.200E-01, 3.200E-01, 3.200E-01, 3.200E-01, 3.200E-01, 3.200E-01, 3.195E-01, 3.163E-01, 3.096E-01, 2.989E-01, 2.936E-01, 2.860E-01, 2.800E-01, 2.724E-01, 2.611E-01, 2.421E-01, 2.174E-01, 1.843E-01, 1.621E-01, 1.362E-01, 1.230E-01, 1.124E-01, 1.048E-01, 9.661E-02, 8.693E-02, 7.524E-02, 6.126E-02, 5.116E-02, 3.968E-02, 2.995E-02, 2.080E-02, 1.311E-02, 8.071E-03, 4.164E-03, 2.629E-03, 1.809E-03, 1.321E-03, 1.007E-03, 7.883E-04, 6.333E-04, 5.194E-04, 4.333E-04, 3.666E-04, 3.140E-04, 2.717E-04, 2.373E-04, 2.089E-04, 1.851E-04 }, { 3.100E-01, 3.100E-01, 3.100E-01, 3.100E-01, 3.079E-01, 3.024E-01, 2.906E-01, 2.822E-01, 2.759E-01, 2.703E-01, 2.651E-01, 2.600E-01, 2.549E-01, 2.494E-01, 2.433E-01, 2.355E-01, 2.282E-01, 2.179E-01, 2.035E-01, 1.817E-01, 1.567E-01, 1.350E-01, 1.218E-01, 1.102E-01, 9.893E-02, 8.775E-02, 7.327E-02, 5.941E-02, 4.154E-02, 3.032E-02, 1.949E-02, 1.274E-02, 9.001E-03, 6.286E-03, 4.558E-03, 2.795E-03, 1.765E-03, 1.214E-03, 8.866E-04, 6.756E-04, 5.538E-04, 4.649E-04, 3.979E-04, 3.459E-04, 3.047E-04, 2.713E-04, 2.439E-04, 2.210E-04, 2.017E-04, 1.851E-04 }, { 3.200E-01, 3.200E-01, 3.200E-01, 3.200E-01, 3.200E-01, 3.200E-01, 3.200E-01, 3.200E-01, 3.195E-01, 3.163E-01, 3.096E-01, 2.989E-01, 2.936E-01, 2.860E-01, 2.800E-01, 2.724E-01, 2.611E-01, 2.421E-01, 2.174E-01, 1.843E-01, 1.621E-01, 1.362E-01, 1.230E-01, 1.122E-01, 1.043E-01, 9.570E-02, 8.598E-02, 7.314E-02, 5.710E-02, 4.670E-02, 3.439E-02, 2.471E-02, 1.631E-02, 1.066E-02, 7.064E-03, 3.972E-03, 2.508E-03, 1.726E-03, 1.260E-03, 9.602E-04, 7.554E-04, 6.097E-04, 5.024E-04, 4.210E-04, 3.579E-04, 3.080E-04, 2.678E-04, 2.350E-04, 2.079E-04, 1.851E-04 }, { 3.200E-01, 3.200E-01, 3.200E-01, 3.200E-01, 3.200E-01, 3.200E-01, 3.200E-01, 3.200E-01, 3.200E-01, 3.195E-01, 3.179E-01, 3.140E-01, 3.095E-01, 3.048E-01, 2.999E-01, 2.944E-01, 2.877E-01, 2.783E-01, 2.671E-01, 2.527E-01, 2.365E-01, 2.194E-01, 2.051E-01, 1.967E-01, 1.875E-01, 1.756E-01, 1.588E-01, 1.416E-01, 1.165E-01, 9.275E-02, 6.693E-02, 4.513E-02, 2.751E-02, 1.591E-02, 9.378E-03, 4.752E-03, 3.000E-03, 2.065E-03, 1.507E-03, 1.149E-03, 8.890E-04, 7.056E-04, 5.716E-04, 4.708E-04, 3.932E-04, 3.323E-04, 2.837E-04, 2.443E-04, 2.120E-04, 1.851E-04 } };

  //    DATA CO        /
  static const double
      co[6][50] = { { 1.500E-01, 1.450E-01, 1.399E-01, 1.349E-01, 1.312E-01, 1.303E-01, 1.288E-01, 1.247E-01, 1.185E-01, 1.094E-01, 9.962E-02, 8.964E-02, 7.814E-02, 6.374E-02, 5.025E-02, 3.941E-02, 3.069E-02, 2.489E-02, 1.966E-02, 1.549E-02, 1.331E-02, 1.232E-02, 1.232E-02, 1.307E-02, 1.400E-02, 1.521E-02, 1.722E-02, 1.995E-02, 2.266E-02, 2.487E-02, 2.738E-02, 3.098E-02, 3.510E-02, 3.987E-02, 4.482E-02, 5.092E-02, 5.985E-02, 6.960E-02, 9.188E-02, 1.938E-01, 5.688E-01, 1.549E+00, 3.849E+00, 6.590E+00, 1.044E+01, 1.705E+01, 2.471E+01, 3.358E+01, 4.148E+01, 5.000E+01 }, { 1.500E-01, 1.450E-01, 1.399E-01, 1.349E-01, 1.312E-01, 1.303E-01, 1.288E-01, 1.247E-01, 1.185E-01, 1.094E-01, 9.962E-02, 8.964E-02, 7.814E-02, 6.374E-02, 5.025E-02, 3.941E-02, 3.069E-02, 2.489E-02, 1.966E-02, 1.549E-02, 1.331E-02, 1.232E-02, 1.232E-02, 1.307E-02, 1.400E-02, 1.521E-02, 1.722E-02, 1.995E-02, 2.266E-02, 2.487E-02, 2.716E-02, 2.962E-02, 3.138E-02, 3.307E-02, 3.487E-02, 3.645E-02, 3.923E-02, 4.673E-02, 6.404E-02, 1.177E-01, 2.935E-01, 6.815E-01, 1.465E+00, 2.849E+00, 5.166E+00, 1.008E+01, 1.865E+01, 2.863E+01, 3.890E+01, 5.000E+01 }, { 1.500E-01, 1.450E-01, 1.399E-01, 1.349E-01, 1.312E-01, 1.303E-01, 1.288E-01, 1.247E-01, 1.185E-01, 1.094E-01, 9.962E-02, 8.964E-02, 7.814E-02, 6.374E-02, 5.025E-02, 3.941E-02, 3.069E-02, 2.489E-02, 1.966E-02, 1.549E-02, 1.331E-02, 1.232E-02, 1.232E-02, 1.307E-02, 1.400E-02, 1.498E-02, 1.598E-02, 1.710E-02, 1.850E-02, 1.997E-02, 2.147E-02, 2.331E-02, 2.622E-02, 3.057E-02, 3.803E-02, 6.245E-02, 1.480E-01, 2.926E-01, 5.586E-01, 1.078E+00, 1.897E+00, 2.960E+00, 4.526E+00, 6.862E+00, 1.054E+01, 1.709E+01, 2.473E+01, 3.359E+01, 4.149E+01, 5.000E+01 }, { 1.500E-01, 1.450E-01, 1.399E-01, 1.349E-01, 1.312E-01, 1.303E-01, 1.288E-01, 1.247E-01, 1.185E-01, 1.094E-01, 9.962E-02, 8.964E-02, 7.814E-02, 6.374E-02, 5.025E-02, 3.941E-02, 3.069E-02, 2.489E-02, 1.966E-02, 1.549E-02, 1.331E-02, 1.232E-02, 1.232E-02, 1.307E-02, 1.400E-02, 1.510E-02, 1.649E-02, 1.808E-02, 1.997E-02, 2.183E-02, 2.343E-02, 2.496E-02, 2.647E-02, 2.809E-02, 2.999E-02, 3.220E-02, 3.650E-02, 4.589E-02, 6.375E-02, 1.176E-01, 3.033E-01, 7.894E-01, 1.823E+00, 3.402E+00, 5.916E+00, 1.043E+01, 1.881E+01, 2.869E+01, 3.892E+01, 5.000E+01 }, { 1.500E-01, 1.450E-01, 1.399E-01, 1.349E-01, 1.312E-01, 1.303E-01, 1.288E-01, 1.247E-01, 1.185E-01, 1.094E-01, 9.962E-02, 8.964E-02, 7.814E-02, 6.374E-02, 5.025E-02, 3.941E-02, 3.069E-02, 2.489E-02, 1.966E-02, 1.549E-02, 1.331E-02, 1.232E-02, 1.232E-02, 1.307E-02, 1.400E-02, 1.521E-02, 1.722E-02, 2.037E-02, 2.486E-02, 3.168E-02, 4.429E-02, 6.472E-02, 1.041E-01, 1.507E-01, 2.163E-01, 3.141E-01, 4.842E-01, 7.147E-01, 1.067E+00, 1.516E+00, 2.166E+00, 3.060E+00, 4.564E+00, 6.877E+00, 1.055E+01, 1.710E+01, 2.473E+01, 3.359E+01, 4.149E+01, 5.000E+01 }, { 1.500E-01, 1.450E-01, 1.399E-01, 1.349E-01, 1.312E-01, 1.303E-01, 1.288E-01, 1.247E-01, 1.185E-01, 1.094E-01, 9.962E-02, 8.964E-02, 7.814E-02, 6.374E-02, 5.025E-02, 3.941E-02, 3.069E-02, 2.489E-02, 1.966E-02, 1.549E-02, 1.331E-02, 1.232E-02, 1.232E-02, 1.307E-02, 1.400E-02, 1.498E-02, 1.598E-02, 1.710E-02, 1.850E-02, 2.009E-02, 2.220E-02, 2.497E-02, 2.824E-02, 3.241E-02, 3.717E-02, 4.597E-02, 6.639E-02, 1.073E-01, 1.862E-01, 3.059E-01, 6.375E-01, 1.497E+00, 3.239E+00, 5.843E+00, 1.013E+01, 1.692E+01, 2.467E+01, 3.356E+01, 4.148E+01, 5.000E+01 } };

  //    DATA  CH4      /
  //      DATA AMOL16/
  //     C 1.700E+00, 1.700E+00, 1.700E+00, 1.700E+00, 1.700E+00,
  //     C 1.700E+00, 1.700E+00, 1.699E+00, 1.697E+00, 1.693E+00,
  //     C 1.685E+00, 1.675E+00, 1.662E+00, 1.645E+00, 1.626E+00,
  //     C 1.605E+00, 1.582E+00, 1.553E+00, 1.521E+00, 1.480E+00,
  //     C 1.424E+00, 1.355E+00, 1.272E+00, 1.191E+00, 1.118E+00,
  //     C 1.055E+00, 9.870E-01, 9.136E-01, 8.300E-01, 7.460E-01,
  //     C 6.618E-01, 5.638E-01, 4.614E-01, 3.631E-01, 2.773E-01,
  //     C 2.100E-01, 1.651E-01, 1.500E-01, 1.500E-01, 1.500E-01,
  //     C 1.500E-01, 1.500E-01, 1.500E-01, 1.400E-01, 1.300E-01,
  //     C 1.200E-01, 1.100E-01, 9.500E-02, 6.000E-02, 3.000E-02/
  //      DATA AMOL26/
  //     C 1.700E+00, 1.700E+00, 1.700E+00, 1.700E+00, 1.697E+00,
  //     C 1.687E+00, 1.672E+00, 1.649E+00, 1.629E+00, 1.615E+00,
  //     C 1.579E+00, 1.542E+00, 1.508E+00, 1.479E+00, 1.451E+00,
  //     C 1.422E+00, 1.390E+00, 1.356E+00, 1.323E+00, 1.281E+00,
  //     C 1.224E+00, 1.154E+00, 1.066E+00, 9.730E-01, 8.800E-01,
  //     C 7.888E-01, 7.046E-01, 6.315E-01, 5.592E-01, 5.008E-01,
  //     C 4.453E-01, 3.916E-01, 3.389E-01, 2.873E-01, 2.384E-01,
  //     C 1.944E-01, 1.574E-01, 1.500E-01, 1.500E-01, 1.500E-01,
  //     C 1.500E-01, 1.500E-01, 1.500E-01, 1.400E-01, 1.300E-01,
  //     C 1.200E-01, 1.100E-01, 9.500E-02, 6.000E-02, 3.000E-02/
  //      DATA AMOL36/
  //     C 1.700E+00, 1.700E+00, 1.700E+00, 1.700E+00, 1.697E+00,
  //     C 1.687E+00, 1.672E+00, 1.649E+00, 1.629E+00, 1.615E+00,
  //     C 1.579E+00, 1.542E+00, 1.508E+00, 1.479E+00, 1.451E+00,
  //     C 1.422E+00, 1.390E+00, 1.356E+00, 1.323E+00, 1.281E+00,
  //     C 1.224E+00, 1.154E+00, 1.066E+00, 9.730E-01, 8.800E-01,
  //     C 7.931E-01, 7.130E-01, 6.438E-01, 5.746E-01, 5.050E-01,
  //     C 4.481E-01, 3.931E-01, 3.395E-01, 2.876E-01, 2.386E-01,
  //     C 1.944E-01, 1.574E-01, 1.500E-01, 1.500E-01, 1.500E-01,
  //     C 1.500E-01, 1.500E-01, 1.500E-01, 1.400E-01, 1.300E-01,
  //     C 1.200E-01, 1.100E-01, 9.500E-02, 6.000E-02, 3.000E-02/
  //      DATA AMOL46/
  //     C 1.700E+00, 1.700E+00, 1.700E+00, 1.700E+00, 1.697E+00,
  //     C 1.687E+00, 1.672E+00, 1.649E+00, 1.629E+00, 1.615E+00,
  //     C 1.579E+00, 1.542E+00, 1.506E+00, 1.471E+00, 1.434E+00,
  //     C 1.389E+00, 1.342E+00, 1.290E+00, 1.230E+00, 1.157E+00,
  //     C 1.072E+00, 9.903E-01, 9.170E-01, 8.574E-01, 8.013E-01,
  //     C 7.477E-01, 6.956E-01, 6.442E-01, 5.888E-01, 5.240E-01,
  //     C 4.506E-01, 3.708E-01, 2.992E-01, 2.445E-01, 2.000E-01,
  //     C 1.660E-01, 1.500E-01, 1.500E-01, 1.500E-01, 1.500E-01,
  //     C 1.500E-01, 1.500E-01, 1.500E-01, 1.400E-01, 1.300E-01,
  //     C 1.200E-01, 1.100E-01, 9.500E-02, 6.000E-02, 3.000E-02/
  //      DATA AMOL56/
  //     C 1.700E+00, 1.700E+00, 1.700E+00, 1.700E+00, 1.697E+00,
  //     C 1.687E+00, 1.672E+00, 1.649E+00, 1.629E+00, 1.615E+00,
  //     C 1.579E+00, 1.542E+00, 1.506E+00, 1.471E+00, 1.434E+00,
  //     C 1.389E+00, 1.342E+00, 1.290E+00, 1.230E+00, 1.161E+00,
  //     C 1.084E+00, 1.014E+00, 9.561E-01, 9.009E-01, 8.479E-01,
  //     C 7.961E-01, 7.449E-01, 6.941E-01, 6.434E-01, 5.883E-01,
  //     C 5.238E-01, 4.505E-01, 3.708E-01, 3.004E-01, 2.453E-01,
  //     C 1.980E-01, 1.590E-01, 1.500E-01, 1.500E-01, 1.500E-01,
  //     C 1.500E-01, 1.500E-01, 1.500E-01, 1.400E-01, 1.300E-01,
  //     C 1.200E-01, 1.100E-01, 9.500E-02, 6.000E-02, 3.000E-02/
  //      DATA AMOL66/
  //     C 1.700E+00, 1.700E+00, 1.700E+00, 1.700E+00, 1.700E+00,
  //     C 1.700E+00, 1.700E+00, 1.699E+00, 1.697E+00, 1.693E+00,
  //     C 1.685E+00, 1.675E+00, 1.662E+00, 1.645E+00, 1.626E+00,
  //     C 1.605E+00, 1.582E+00, 1.553E+00, 1.521E+00, 1.480E+00,
  //     C 1.424E+00, 1.355E+00, 1.272E+00, 1.191E+00, 1.118E+00,
  //     C 1.055E+00, 9.870E-01, 9.136E-01, 8.300E-01, 7.460E-01,
  //     C 6.618E-01, 5.638E-01, 4.614E-01, 3.631E-01, 2.773E-01,
  //     C 2.100E-01, 1.650E-01, 1.500E-01, 1.500E-01, 1.500E-01,
  //     C 1.500E-01, 1.500E-01, 1.500E-01, 1.400E-01, 1.300E-01,
  //     C 1.200E-01, 1.100E-01, 9.500E-02, 6.000E-02, 3.000E-02/


  //     DATA O2        /
  //      DATA AMOL17/
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.000E+05, 1.900E+05, 1.800E+05,
  //     C 1.600E+05, 1.400E+05, 1.200E+05, 9.400E+04, 7.250E+04/
  //      DATA AMOL27/
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.000E+05, 1.900E+05, 1.800E+05,
  //     C 1.600E+05, 1.400E+05, 1.200E+05, 9.400E+04, 7.250E+04/
  //      DATA AMOL37/
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.000E+05, 1.900E+05, 1.800E+05,
  //     C 1.600E+05, 1.400E+05, 1.200E+05, 9.400E+04, 7.250E+04/
  //      DATA AMOL47/
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.000E+05, 1.900E+05, 1.800E+05,
  //     C 1.600E+05, 1.400E+05, 1.200E+05, 9.400E+04, 7.250E+04/
  //      DATA AMOL57/
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.000E+05, 1.900E+05, 1.800E+05,
  //     C 1.600E+05, 1.400E+05, 1.200E+05, 9.400E+04, 7.250E+04/
  //      DATA AMOL67/
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05, 2.090E+05,
  //     C 2.090E+05, 2.090E+05, 2.000E+05, 1.900E+05, 1.800E+05,
  //     C 1.600E+05, 1.400E+05, 1.200E+05, 9.400E+04, 7.250E+04/

  //     DATA DENSITY   /

  static const double
      den[6][50] = { { 2.450E+19, 2.231E+19, 2.028E+19, 1.827E+19, 1.656E+19, 1.499E+19, 1.353E+19, 1.218E+19, 1.095E+19, 9.789E+18, 8.747E+18, 7.780E+18, 6.904E+18, 6.079E+18, 5.377E+18, 4.697E+18, 4.084E+18, 3.486E+18, 2.877E+18, 2.381E+18, 1.981E+18, 1.651E+18, 1.381E+18, 1.169E+18, 9.920E+17, 8.413E+17, 5.629E+17, 3.807E+17, 2.598E+17, 1.789E+17, 1.243E+17, 8.703E+16, 6.147E+16, 4.352E+16, 3.119E+16, 2.291E+16, 1.255E+16, 6.844E+15, 3.716E+15, 1.920E+15, 9.338E+14, 4.314E+14, 1.801E+14, 7.043E+13, 2.706E+13, 1.098E+13, 4.445E+12, 1.941E+12, 8.706E+11, 4.225E+11 }, { .496E+19, 2.257E+19, 2.038E+19, 1.843E+19, 1.666E+19, 1.503E+19, 1.351E+19, 1.212E+19, 1.086E+19, 9.716E+18, 8.656E+18, 7.698E+18, 6.814E+18, 6.012E+18, 5.141E+18, 4.368E+18, 3.730E+18, 3.192E+18, 2.715E+18, 2.312E+18, 1.967E+18, 1.677E+18, 1.429E+18, 1.223E+18, 1.042E+18, 8.919E+17, 6.050E+17, 4.094E+17, 2.820E+17, 1.927E+17, 1.338E+17, 9.373E+16, 6.624E+16, 4.726E+16, 3.398E+16, 2.500E+16, 1.386E+16, 7.668E+15, 4.196E+15, 2.227E+15, 1.109E+15, 4.996E+14, 1.967E+14, 7.204E+13, 2.541E+13, 9.816E+12, 3.816E+12, 1.688E+12, 8.145E+11, 4.330E+11 }, { .711E+19, 2.420E+19, 2.158E+19, 1.922E+19, 1.724E+19, 1.542E+19, 1.376E+19, 1.225E+19, 1.086E+19, 9.612E+18, 8.472E+18, 7.271E+18, 6.237E+18, 5.351E+18, 4.588E+18, 3.931E+18, 3.368E+18, 2.886E+18, 2.473E+18, 2.115E+18, 1.809E+18, 1.543E+18, 1.317E+18, 1.125E+18, 9.633E+17, 8.218E+17, 5.536E+17, 3.701E+17, 2.486E+17, 1.647E+17, 1.108E+17, 7.540E+16, 5.202E+16, 3.617E+16, 2.570E+16, 1.863E+16, 1.007E+16, 5.433E+15, 2.858E+15, 1.477E+15, 7.301E+14, 3.553E+14, 1.654E+14, 7.194E+13, 3.052E+13, 1.351E+13, 6.114E+12, 2.952E+12, 1.479E+12, 7.836E+11 }, { .549E+19, 2.305E+19, 2.080E+19, 1.873E+19, 1.682E+19, 1.508E+19, 1.357E+19, 1.216E+19, 1.088E+19, 9.701E+18, 8.616E+18, 7.402E+18, 6.363E+18, 5.471E+18, 4.699E+18, 4.055E+18, 3.476E+18, 2.987E+18, 2.568E+18, 2.208E+18, 1.899E+18, 1.632E+18, 1.403E+18, 1.207E+18, 1.033E+18, 8.834E+17, 6.034E+17, 4.131E+17, 2.839E+17, 1.938E+17, 1.344E+17, 9.402E+16, 6.670E+16, 4.821E+16, 3.516E+16, 2.581E+16, 1.421E+16, 7.946E+15, 4.445E+15, 2.376E+15, 1.198E+15, 5.311E+14, 2.022E+14, 7.221E+13, 2.484E+13, 9.441E+12, 3.624E+12, 1.610E+12, 7.951E+11, 4.311E+11 }, { .855E+19, 2.484E+19, 2.202E+19, 1.950E+19, 1.736E+19, 1.552E+19, 1.383E+19, 1.229E+19, 1.087E+19, 9.440E+18, 8.069E+18, 6.898E+18, 5.893E+18, 5.039E+18, 4.308E+18, 3.681E+18, 3.156E+18, 2.704E+18, 2.316E+18, 1.982E+18, 1.697E+18, 1.451E+18, 1.241E+18, 1.061E+18, 9.065E+17, 7.742E+17, 5.134E+17, 3.423E+17, 2.292E+17, 1.533E+17, 1.025E+17, 6.927E+16, 4.726E+16, 3.266E+16, 2.261E+16, 1.599E+16, 8.364E+15, 4.478E+15, 2.305E+15, 1.181E+15, 6.176E+14, 3.127E+14, 1.531E+14, 7.244E+13, 3.116E+13, 1.403E+13, 6.412E+12, 3.099E+12, 1.507E+12, 7.814E+11 }, { .548E+19, 2.313E+19, 2.094E+19, 1.891E+19, 1.704E+19, 1.532E+19, 1.373E+19, 1.228E+19, 1.094E+19, 9.719E+18, 8.602E+18, 7.589E+18, 6.489E+18, 5.546E+18, 4.739E+18, 4.050E+18, 3.462E+18, 2.960E+18, 2.530E+18, 2.163E+18, 1.849E+18, 1.575E+18, 1.342E+18, 1.144E+18, 9.765E+17, 8.337E+17, 5.640E+17, 3.830E+17, 2.524E+17, 1.761E+17, 1.238E+17, 8.310E+16, 5.803E+16, 4.090E+16, 2.920E+16, 2.136E+16, 1.181E+16, 6.426E+15, 3.386E+15, 1.723E+15, 8.347E+14, 3.832E+14, 1.711E+14, 7.136E+13, 2.924E+13, 1.189E+13, 5.033E+12, 2.144E+12, 9.688E+11, 5.114E+11 } };
  //
  //      DATA ANO        /
  //     C  3.00E-04,  3.00E-04,  3.00E-04,  3.00E-04,  3.00E-04,
  //     C  3.00E-04,  3.00E-04,  3.00E-04,  3.00E-04,  3.00E-04,
  //     C  3.00E-04,  3.00E-04,  3.00E-04,  2.99E-04,  2.95E-04,
  //     C  2.83E-04,  2.68E-04,  2.52E-04,  2.40E-04,  2.44E-04,
  //     C  2.55E-04,  2.77E-04,  3.07E-04,  3.60E-04,  4.51E-04,
  //     C  6.85E-04,  1.28E-03,  2.45E-03,  4.53E-03,  7.14E-03,
  //     C  9.34E-03,  1.12E-02,  1.19E-02,  1.17E-02,  1.10E-02,
  //     C  1.03E-02,  1.01E-02,  1.01E-02,  1.03E-02,  1.15E-02,
  //     C  1.61E-02,  2.68E-02,  7.01E-02,  2.13E-01,  7.12E-01,
  //     C  2.08E+00,  4.50E+00,  7.98E+00,  1.00E+01,  1.00E+01/
  //      DATA SO2       /
  //     C  3.00E-04,  2.74E-04,  2.36E-04,  1.90E-04,  1.46E-04,
  //     C  1.18E-04,  9.71E-05,  8.30E-05,  7.21E-05,  6.56E-05,
  //     C  6.08E-05,  5.79E-05,  5.60E-05,  5.59E-05,  5.64E-05,
  //     C  5.75E-05,  5.75E-05,  5.37E-05,  4.78E-05,  3.97E-05,
  //     C  3.19E-05,  2.67E-05,  2.28E-05,  2.07E-05,  1.90E-05,
  //     C  1.75E-05,  1.54E-05,  1.34E-05,  1.21E-05,  1.16E-05,
  //     C  1.21E-05,  1.36E-05,  1.65E-05,  2.10E-05,  2.77E-05,
  //     C  3.56E-05,  4.59E-05,  5.15E-05,  5.11E-05,  4.32E-05,
  //     C  2.83E-05,  1.33E-05,  5.56E-06,  2.24E-06,  8.96E-07,
  //     C  3.58E-07,  1.43E-07,  5.73E-08,  2.29E-08,  9.17E-09/
  //      DATA ANO2       /
  //     C  2.30E-05,  2.30E-05,  2.30E-05,  2.30E-05,  2.30E-05,
  //     C  2.30E-05,  2.30E-05,  2.30E-05,  2.30E-05,  2.32E-05,
  //     C  2.38E-05,  2.62E-05,  3.15E-05,  4.45E-05,  7.48E-05,
  //     C  1.71E-04,  3.19E-04,  5.19E-04,  7.71E-04,  1.06E-03,
  //     C  1.39E-03,  1.76E-03,  2.16E-03,  2.58E-03,  3.06E-03,
  //     C  3.74E-03,  4.81E-03,  6.16E-03,  7.21E-03,  7.28E-03,
  //     C  6.26E-03,  4.03E-03,  2.17E-03,  1.15E-03,  6.66E-04,
  //     C  4.43E-04,  3.39E-04,  2.85E-04,  2.53E-04,  2.31E-04,
  //     C  2.15E-04,  2.02E-04,  1.92E-04,  1.83E-04,  1.76E-04,
  //     C  1.70E-04,  1.64E-04,  1.59E-04,  1.55E-04,  1.51E-04/
  //      DATA ANH3       /
  //     C  5.00E-04,  5.00E-04,  4.63E-04,  3.80E-04,  2.88E-04,
  //     C  2.04E-04,  1.46E-04,  9.88E-05,  6.48E-05,  3.77E-05,
  //     C  2.03E-05,  1.09E-05,  6.30E-06,  3.12E-06,  1.11E-06,
  //     C  4.47E-07,  2.11E-07,  1.10E-07,  6.70E-08,  3.97E-08,
  //     C  2.41E-08,  1.92E-08,  1.72E-08,  1.59E-08,  1.44E-08,
  //     C  1.23E-08,  9.37E-09,  6.35E-09,  3.68E-09,  1.82E-09,
  //     C  9.26E-10,  2.94E-10,  8.72E-11,  2.98E-11,  1.30E-11,
  //     C  7.13E-12,  4.80E-12,  3.66E-12,  3.00E-12,  2.57E-12,
  //     C  2.27E-12,  2.04E-12,  1.85E-12,  1.71E-12,  1.59E-12,
  //     C  1.48E-12,  1.40E-12,  1.32E-12,  1.25E-12,  1.19E-12/
  //      DATA HNO3      /
  //     C  5.00E-05,  5.96E-05,  6.93E-05,  7.91E-05,  8.87E-05,
  //     C  9.75E-05,  1.11E-04,  1.26E-04,  1.39E-04,  1.53E-04,
  //     C  1.74E-04,  2.02E-04,  2.41E-04,  2.76E-04,  3.33E-04,
  //     C  4.52E-04,  7.37E-04,  1.31E-03,  2.11E-03,  3.17E-03,
  //     C  4.20E-03,  4.94E-03,  5.46E-03,  5.74E-03,  5.84E-03,
  //     C  5.61E-03,  4.82E-03,  3.74E-03,  2.59E-03,  1.64E-03,
  //     C  9.68E-04,  5.33E-04,  2.52E-04,  1.21E-04,  7.70E-05,
  //     C  5.55E-05,  4.45E-05,  3.84E-05,  3.49E-05,  3.27E-05,
  //     C  3.12E-05,  3.01E-05,  2.92E-05,  2.84E-05,  2.78E-05,
  //     C  2.73E-05,  2.68E-05,  2.64E-05,  2.60E-05,  2.57E-05/
  //      DATA OH        /
  //     C  4.40E-08,  4.40E-08,  4.40E-08,  4.40E-08,  4.40E-08,
  //     C  4.40E-08,  4.40E-08,  4.41E-08,  4.45E-08,  4.56E-08,
  //     C  4.68E-08,  4.80E-08,  4.94E-08,  5.19E-08,  5.65E-08,
  //     C  6.75E-08,  8.25E-08,  1.04E-07,  1.30E-07,  1.64E-07,
  //     C  2.16E-07,  3.40E-07,  5.09E-07,  7.59E-07,  1.16E-06,
  //     C  2.18E-06,  5.00E-06,  1.17E-05,  3.40E-05,  8.35E-05,
  //     C  1.70E-04,  2.85E-04,  4.06E-04,  5.11E-04,  5.79E-04,
  //     C  6.75E-04,  9.53E-04,  1.76E-03,  3.74E-03,  7.19E-03,
  //     C  1.12E-02,  1.13E-02,  6.10E-03,  1.51E-03,  2.42E-04,
  //     C  4.47E-05,  1.77E-05,  1.19E-05,  1.35E-05,  2.20E-05/
  //      DATA HF        /
  //     C  1.00E-08,  1.00E-08,  1.23E-08,  1.97E-08,  3.18E-08,
  //     C  5.63E-08,  9.18E-08,  1.53E-07,  2.41E-07,  4.04E-07,
  //     C  6.57E-07,  1.20E-06,  1.96E-06,  3.12E-06,  4.62E-06,
  //     C  7.09E-06,  1.05E-05,  1.69E-05,  2.57E-05,  4.02E-05,
  //     C  5.77E-05,  7.77E-05,  9.90E-05,  1.23E-04,  1.50E-04,
  //     C  1.82E-04,  2.30E-04,  2.83E-04,  3.20E-04,  3.48E-04,
  //     C  3.72E-04,  3.95E-04,  4.10E-04,  4.21E-04,  4.24E-04,
  //     C  4.25E-04,  4.25E-04,  4.25E-04,  4.25E-04,  4.25E-04,
  //     C  4.25E-04,  4.25E-04,  4.25E-04,  4.25E-04,  4.25E-04,
  //     C  4.25E-04,  4.25E-04,  4.25E-04,  4.25E-04,  4.25E-04/
  //      DATA HCL/
  //     C  1.00E-03,  7.49E-04,  5.61E-04,  4.22E-04,  3.19E-04,
  //     C  2.39E-04,  1.79E-04,  1.32E-04,  9.96E-05,  7.48E-05,
  //     C  5.68E-05,  4.59E-05,  4.36E-05,  6.51E-05,  1.01E-04,
  //     C  1.63E-04,  2.37E-04,  3.13E-04,  3.85E-04,  4.42E-04,
  //     C  4.89E-04,  5.22E-04,  5.49E-04,  5.75E-04,  6.04E-04,
  //     C  6.51E-04,  7.51E-04,  9.88E-04,  1.28E-03,  1.57E-03,
  //     C  1.69E-03,  1.74E-03,  1.76E-03,  1.79E-03,  1.80E-03,
  //     C  1.80E-03,  1.80E-03,  1.80E-03,  1.80E-03,  1.80E-03,
  //     C  1.80E-03,  1.80E-03,  1.80E-03,  1.80E-03,  1.80E-03,
  //     C  1.80E-03,  1.80E-03,  1.80E-03,  1.80E-03,  1.80E-03/
  //      DATA HBR       /
  //     C  1.70E-06,  1.70E-06,  1.70E-06,  1.70E-06,  1.70E-06,
  //     C  1.70E-06,  1.70E-06,  1.70E-06,  1.70E-06,  1.70E-06,
  //     C  1.70E-06,  1.70E-06,  1.70E-06,  1.70E-06,  1.70E-06,
  //     C  1.70E-06,  1.70E-06,  1.70E-06,  1.70E-06,  1.70E-06,
  //     C  1.70E-06,  1.70E-06,  1.70E-06,  1.70E-06,  1.70E-06,
  //     C  1.71E-06,  1.76E-06,  1.90E-06,  2.26E-06,  2.82E-06,
  //     C  3.69E-06,  4.91E-06,  6.13E-06,  6.85E-06,  7.08E-06,
  //     C  7.14E-06,  7.15E-06,  7.15E-06,  7.15E-06,  7.15E-06,
  //     C  7.15E-06,  7.15E-06,  7.15E-06,  7.15E-06,  7.15E-06,
  //     C  7.15E-06,  7.15E-06,  7.15E-06,  7.15E-06,  7.15E-06/
  //      DATA HI        /
  //     C  3.00E-06,  3.00E-06,  3.00E-06,  3.00E-06,  3.00E-06,
  //     C  3.00E-06,  3.00E-06,  3.00E-06,  3.00E-06,  3.00E-06,
  //     C  3.00E-06,  3.00E-06,  3.00E-06,  3.00E-06,  3.00E-06,
  //     C  3.00E-06,  3.00E-06,  3.00E-06,  3.00E-06,  3.00E-06,
  //     C  3.00E-06,  3.00E-06,  3.00E-06,  3.00E-06,  3.00E-06,
  //     C  3.00E-06,  3.00E-06,  3.00E-06,  3.00E-06,  3.00E-06,
  //     C  3.00E-06,  3.00E-06,  3.00E-06,  3.00E-06,  3.00E-06,
  //     C  3.00E-06,  3.00E-06,  3.00E-06,  3.00E-06,  3.00E-06,
  //     C  3.00E-06,  3.00E-06,  3.00E-06,  3.00E-06,  3.00E-06,
  //     C  3.00E-06,  3.00E-06,  3.00E-06,  3.00E-06,  3.00E-06/
  //      DATA CLO       /
  //     C  1.00E-08,  1.00E-08,  1.00E-08,  1.00E-08,  1.00E-08,
  //     C  1.00E-08,  1.00E-08,  1.00E-08,  1.01E-08,  1.05E-08,
  //     C  1.21E-08,  1.87E-08,  3.18E-08,  5.61E-08,  9.99E-08,
  //     C  1.78E-07,  3.16E-07,  5.65E-07,  1.04E-06,  2.04E-06,
  //     C  4.64E-06,  8.15E-06,  1.07E-05,  1.52E-05,  2.24E-05,
  //     C  3.97E-05,  8.48E-05,  1.85E-04,  3.57E-04,  5.08E-04,
  //     C  6.07E-04,  5.95E-04,  4.33E-04,  2.51E-04,  1.56E-04,
  //     C  1.04E-04,  7.69E-05,  6.30E-05,  5.52E-05,  5.04E-05,
  //     C  4.72E-05,  4.49E-05,  4.30E-05,  4.16E-05,  4.03E-05,
  //     C  3.93E-05,  3.83E-05,  3.75E-05,  3.68E-05,  3.61E-05/
  //      DATA OCS       /
  //     C  6.00E-04,  5.90E-04,  5.80E-04,  5.70E-04,  5.62E-04,
  //     C  5.55E-04,  5.48E-04,  5.40E-04,  5.32E-04,  5.25E-04,
  //     C  5.18E-04,  5.09E-04,  4.98E-04,  4.82E-04,  4.60E-04,
  //     C  4.26E-04,  3.88E-04,  3.48E-04,  3.09E-04,  2.74E-04,
  //     C  2.41E-04,  2.14E-04,  1.88E-04,  1.64E-04,  1.37E-04,
  //     C  1.08E-04,  6.70E-05,  2.96E-05,  1.21E-05,  4.31E-06,
  //     C  1.60E-06,  6.71E-07,  4.35E-07,  3.34E-07,  2.80E-07,
  //     C  2.47E-07,  2.28E-07,  2.16E-07,  2.08E-07,  2.03E-07,
  //     C  1.98E-07,  1.95E-07,  1.92E-07,  1.89E-07,  1.87E-07,
  //     C  1.85E-07,  1.83E-07,  1.81E-07,  1.80E-07,  1.78E-07/
  //      DATA H2CO      /
  //     C  2.40E-03,  1.07E-03,  4.04E-04,  2.27E-04,  1.40E-04,
  //     C  1.00E-04,  7.44E-05,  6.04E-05,  5.01E-05,  4.22E-05,
  //     C  3.63E-05,  3.43E-05,  3.39E-05,  3.50E-05,  3.62E-05,
  //     C  3.62E-05,  3.58E-05,  3.50E-05,  3.42E-05,  3.39E-05,
  //     C  3.43E-05,  3.68E-05,  4.03E-05,  4.50E-05,  5.06E-05,
  //     C  5.82E-05,  7.21E-05,  8.73E-05,  1.01E-04,  1.11E-04,
  //     C  1.13E-04,  1.03E-04,  7.95E-05,  4.82E-05,  1.63E-05,
  //     C  5.10E-06,  2.00E-06,  1.05E-06,  6.86E-07,  5.14E-07,
  //     C  4.16E-07,  3.53E-07,  3.09E-07,  2.76E-07,  2.50E-07,
  //     C  2.30E-07,  2.13E-07,  1.98E-07,  1.86E-07,  1.75E-07/
  //      DATA HOCL      /
  //     C  7.70E-06,  1.06E-05,  1.22E-05,  1.14E-05,  9.80E-06,
  //     C  8.01E-06,  6.42E-06,  5.42E-06,  4.70E-06,  4.41E-06,
  //     C  4.34E-06,  4.65E-06,  5.01E-06,  5.22E-06,  5.60E-06,
  //     C  6.86E-06,  8.77E-06,  1.20E-05,  1.63E-05,  2.26E-05,
  //     C  3.07E-05,  4.29E-05,  5.76E-05,  7.65E-05,  9.92E-05,
  //     C  1.31E-04,  1.84E-04,  2.45E-04,  2.96E-04,  3.21E-04,
  //     C  3.04E-04,  2.48E-04,  1.64E-04,  9.74E-05,  4.92E-05,
  //     C  2.53E-05,  1.50E-05,  1.05E-05,  8.34E-06,  7.11E-06,
  //     C  6.33E-06,  5.78E-06,  5.37E-06,  5.05E-06,  4.78E-06,
  //     C  4.56E-06,  4.37E-06,  4.21E-06,  4.06E-06,  3.93E-06/
  //      DATA AN2        /
  //     C  7.81E+05,  7.81E+05,  7.81E+05,  7.81E+05,  7.81E+05,
  //     C  7.81E+05,  7.81E+05,  7.81E+05,  7.81E+05,  7.81E+05,
  //     C  7.81E+05,  7.81E+05,  7.81E+05,  7.81E+05,  7.81E+05,
  //     C  7.81E+05,  7.81E+05,  7.81E+05,  7.81E+05,  7.81E+05,
  //     C  7.81E+05,  7.81E+05,  7.81E+05,  7.81E+05,  7.81E+05,
  //     C  7.81E+05,  7.81E+05,  7.81E+05,  7.81E+05,  7.81E+05,
  //     C  7.81E+05,  7.81E+05,  7.81E+05,  7.81E+05,  7.81E+05,
  //     C  7.81E+05,  7.81E+05,  7.81E+05,  7.81E+05,  7.81E+05,
  //     C  7.81E+05,  7.81E+05,  7.81E+05,  7.80E+05,  7.79E+05,
  //     C  7.77E+05,  7.74E+05,  7.70E+05,  7.65E+05,  7.60E+05/
  //      DATA HCN       /
  //     C  1.70E-04,  1.65E-04,  1.63E-04,  1.61E-04,  1.60E-04,
  //     C  1.60E-04,  1.60E-04,  1.60E-04,  1.60E-04,  1.60E-04,
  //     C  1.60E-04,  1.60E-04,  1.60E-04,  1.59E-04,  1.57E-04,
  //     C  1.55E-04,  1.52E-04,  1.49E-04,  1.45E-04,  1.41E-04,
  //     C  1.37E-04,  1.34E-04,  1.30E-04,  1.25E-04,  1.19E-04,
  //     C  1.13E-04,  1.05E-04,  9.73E-05,  9.04E-05,  8.46E-05,
  //     C  8.02E-05,  7.63E-05,  7.30E-05,  7.00E-05,  6.70E-05,
  //     C  6.43E-05,  6.21E-05,  6.02E-05,  5.88E-05,  5.75E-05,
  //     C  5.62E-05,  5.50E-05,  5.37E-05,  5.25E-05,  5.12E-05,
  //     C  5.00E-05,  4.87E-05,  4.75E-05,  4.62E-05,  4.50E-05/
  //      DATA CH3CL     /
  //     C  7.00E-04,  6.70E-04,  6.43E-04,  6.22E-04,  6.07E-04,
  //     C  6.02E-04,  6.00E-04,  6.00E-04,  5.98E-04,  5.94E-04,
  //     C  5.88E-04,  5.79E-04,  5.66E-04,  5.48E-04,  5.28E-04,
  //     C  5.03E-04,  4.77E-04,  4.49E-04,  4.21E-04,  3.95E-04,
  //     C  3.69E-04,  3.43E-04,  3.17E-04,  2.86E-04,  2.48E-04,
  //     C  1.91E-04,  1.10E-04,  4.72E-05,  1.79E-05,  7.35E-06,
  //     C  3.03E-06,  1.32E-06,  8.69E-07,  6.68E-07,  5.60E-07,
  //     C  4.94E-07,  4.56E-07,  4.32E-07,  4.17E-07,  4.05E-07,
  //     C  3.96E-07,  3.89E-07,  3.83E-07,  3.78E-07,  3.73E-07,
  //     C  3.69E-07,  3.66E-07,  3.62E-07,  3.59E-07,  3.56E-07/
  //      DATA H2O2      /
  //     C  2.00E-04,  1.95E-04,  1.92E-04,  1.89E-04,  1.84E-04,
  //     C  1.77E-04,  1.66E-04,  1.49E-04,  1.23E-04,  9.09E-05,
  //     C  5.79E-05,  3.43E-05,  1.95E-05,  1.08E-05,  6.59E-06,
  //     C  4.20E-06,  2.94E-06,  2.30E-06,  2.24E-06,  2.68E-06,
  //     C  3.68E-06,  5.62E-06,  1.03E-05,  1.97E-05,  3.70E-05,
  //     C  6.20E-05,  1.03E-04,  1.36E-04,  1.36E-04,  1.13E-04,
  //     C  8.51E-05,  6.37E-05,  5.17E-05,  4.44E-05,  3.80E-05,
  //     C  3.48E-05,  3.62E-05,  5.25E-05,  1.26E-04,  3.77E-04,
  //     C  1.12E-03,  2.00E-03,  1.68E-03,  4.31E-04,  4.98E-05,
  //     C  6.76E-06,  8.38E-07,  9.56E-08,  1.00E-08,  1.00E-09/
  //      DATA C2H2      /
  //     C  3.00E-04,  1.72E-04,  9.57E-05,  6.74E-05,  5.07E-05,
  //     C  3.99E-05,  3.19E-05,  2.80E-05,  2.55E-05,  2.40E-05,
  //     C  2.27E-05,  2.08E-05,  1.76E-05,  1.23E-05,  7.32E-06,
  //     C  4.52E-06,  2.59E-06,  1.55E-06,  8.63E-07,  5.30E-07,
  //     C  3.10E-07,  1.89E-07,  1.04E-07,  5.75E-08,  2.23E-08,
  //     C  8.51E-09,  4.09E-09,  2.52E-09,  1.86E-09,  1.52E-09,
  //     C  1.32E-09,  1.18E-09,  1.08E-09,  9.97E-10,  9.34E-10,
  //     C  8.83E-10,  8.43E-10,  8.10E-10,  7.83E-10,  7.60E-10,
  //     C  7.40E-10,  7.23E-10,  7.07E-10,  6.94E-10,  6.81E-10,
  //     C  6.70E-10,  6.59E-10,  6.49E-10,  6.40E-10,  6.32E-10/
  //      DATA C2H6      /
  //     C  2.00E-03,  2.00E-03,  2.00E-03,  2.00E-03,  1.98E-03,
  //     C  1.95E-03,  1.90E-03,  1.85E-03,  1.79E-03,  1.72E-03,
  //     C  1.58E-03,  1.30E-03,  9.86E-04,  7.22E-04,  4.96E-04,
  //     C  3.35E-04,  2.14E-04,  1.49E-04,  1.05E-04,  7.96E-05,
  //     C  6.01E-05,  4.57E-05,  3.40E-05,  2.60E-05,  1.89E-05,
  //     C  1.22E-05,  5.74E-06,  2.14E-06,  8.49E-07,  3.42E-07,
  //     C  1.34E-07,  5.39E-08,  2.25E-08,  1.04E-08,  6.57E-09,
  //     C  4.74E-09,  3.79E-09,  3.28E-09,  2.98E-09,  2.79E-09,
  //     C  2.66E-09,  2.56E-09,  2.49E-09,  2.43E-09,  2.37E-09,
  //     C  2.33E-09,  2.29E-09,  2.25E-09,  2.22E-09,  2.19E-09/
  //      DATA PH3       /
  //     C  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,
  //     C  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,
  //     C  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,
  //     C  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,
  //     C  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,
  //     C  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,
  //     C  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,
  //     C  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,
  //     C  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,
  //     C  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14/
  //      DATA cof2       /
  //     C  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,
  //     C  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,
  //     C  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,
  //     C  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,
  //     C  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,
  //     C  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,
  //     C  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,
  //     C  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,
  //     C  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,
  //     C  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14/
  //      DATA sf6       /
  //     C  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,
  //     C  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,
  //     C  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,
  //     C  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,
  //     C  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,
  //     C  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,
  //     C  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,
  //     C  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,
  //     C  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,
  //     C  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14,  1.00E-14/
  //
  //c
  //   --------------------------------------------[ altitude for interpolation ]

  if((ha < 0.0) || (ha > 120.0)) {

    o3den = NumberDensity(0.0, "m**-3");
    n2oden = NumberDensity(0.0, "m**-3");
    coden = NumberDensity(0.0, "m**-3");

  } else {

    i1 = 0;
    i2 = 0;
    i3 = 0;
    x1 = 0.0;
    x2 = 0.0;
    x3 = 0.0;

    for(i_layer = 0; i_layer < 50; i_layer++) {

      if(ha < alt[i_layer]) {
        if(i_layer == 49) {
          i1 = i_layer - 2;
          i2 = i_layer - 1;
          i3 = i_layer;
        } else {
          if(i_layer == 0) {
            i1 = i_layer;
            i2 = i_layer + 1;
            i3 = i_layer + 2;
          } else {
            i1 = i_layer - 1;
            i2 = i_layer;
            i3 = i_layer + 1;
          }
        }
        x1 = alt[i1];
        x2 = alt[i2];
        x3 = alt[i3];
        goto calc;

      }
    }

    calc:

    if(x1 == 0.0 && x2 == 0.0 && x3 == 0.0) {

      o3den = NumberDensity(0.0, "m**-3");
      n2oden = NumberDensity(0.0, "m**-3");
      coden = NumberDensity(0.0, "m**-3");

    } else {

      //     --------------------------------------------[ pressure ]

      //	p=poli2(ha,x1,x2,x3,p[tip][i1],p[tip][i2],p[tip][i3]);

      //     --------------------------------------------[ TEMPERATURE ]

      //	t=poli2(ha,x1,x2,x3,t[tip][i1],t[tip][i2],t[tip][i3]);

      //     --------------------------------------------[ DENSITY ]

      d = poli2(ha,
                x1,
                x2,
                x3,
                den[tip - 1][i1],
                den[tip - 1][i2],
                den[tip - 1][i3]) * airmwt * 1e6 / avogad; // en g/m**3


      //	   D=A+B*HA+C*HA2         // en cm**-3, pero dividido por 1E19
      //   D=D*airmwt*1e6/avogad  // en g/m**3


      //     --------------------------------------------[ H2O ]

      //	  rr=poli2(ha,x1,x2,x3,agua[tip][i1],agua[tip][i2],agua[tip][i3]);   // RR=A+B*HA+C*HA2  en ppmv
      //      rr=rr*1e-6*(h2omwt/airmwt)*d;  // en g/m**3

      //     --------------------------------------------[ OZONE ]

      o3den
          = NumberDensity(poli2(ha, x1, x2, x3, ozone[tip - 1][i1], ozone[tip
                              - 1][i2], ozone[tip - 1][i3]) * 1e-12 * d
                              * avogad / airmwt,
                          "cm**-3");
      //  OZONO=A+B*HA+C*HA2     // en ppmv

      //     --------------------------------------------[ N2O ]

      n2oden = NumberDensity(poli2(ha, x1, x2, x3, n2o[tip - 1][i1], n2o[tip
          - 1][i2], n2o[tip - 1][i3]) * 1e-12 * d * avogad / airmwt, "cm**-3");
      //	   N2O=A+B*HA+C*HA2       // en ppmv

      //    --------------------------------------------[ NO2 ]
      //
      //    C               y1=ANO2(I1)
      //    C               y2=ANO2(I2)
      //    C               y3=ANO2(I3)
      //    C
      //    C         call poli2(X1,X2,X3,Y1,Y2,Y3,A,B,C)
      //    C         NO2=A+B*HA+C*HA2   !en ppmv
      //    C         NO2=NO2*1e-12*D*avogad/airmwt !en part/cm**3
      //
      //    --------------------------------------------[ CLO ]
      //    C               y1=CLO(I1)
      //    C               y2=CLO(I2)
      //    C               y3=CLO(I3)
      //    C
      //    C         call poli2(X1,X2,X3,Y1,Y2,Y3,A,B,C)
      //    C         a_CLO=A+B*HA+C*HA2   !en ppmv
      //    C         a_clo=a_clo*1000   !en ppbv
      //    C         a_CLO=a_CLO*1e-12*D*avogad/airmwt !en part/cm**3
      //    C
      //    --------------------------------------------[ NO ]
      //    C
      //    C               y1=ANO(I1)
      //    C               y2=ANO(I2)
      //    C               y3=ANO(I3)
      //    C
      //    C         call poli2(X1,X2,X3,Y1,Y2,Y3,A,B,C)
      //    C         NO=A+B*HA+C*HA2   !en ppmv
      //    C         NO=NO*1e-12*D*avogad/airmwt !en part/cm**3
      //    C
      //    --------------------------------------------[ SO2 ]
      //    C
      //    C               y1=SO2(I1)
      //    C               y2=SO2(I2)
      //    C               y3=SO2(I3)
      //    C
      //    C         call poli2(X1,X2,X3,Y1,Y2,Y3,A,B,C)
      //    C
      //    C         A_SO2=A+B*HA+C*HA2   !en ppmv
      //    C         A_SO2=(A+B*HA+C*HA2)*1d-12*D*avogad/airmwt !en part/cm**3
      //    C
      //    --------------------------------------------[ CO ]

      coden = NumberDensity(poli2(ha,
                                  x1,
                                  x2,
                                  x3,
                                  co[tip][i1],
                                  co[tip][i2],
                                  co[tip][i3]) * 1e-12 * d * avogad / airmwt,
                            "cm**-3");
      //	   CO=A+B*HA+C*HA2        // en ppmv

    }

  }

  minorden.push_back(o3den);
  minorden.push_back(n2oden);
  minorden.push_back(coden);

  return minorden;

}

double AtmProfile::poli2(double ha,
                         double x1,
                         double x2,
                         double x3,
                         double y1,
                         double y2,
                         double y3)
{

  double a, b, c;

  c = (y3 - y2) * (x2 - x1) - (y2 - y1) * (x3 - x2);
  b = (x2 - x1) * (x3 * x3 - x2 * x2) - (x2 * x2 - x1 * x1) * (x3 - x2);
  c = c / b;
  b = (y2 - y1) - c * (x2 * x2 - x1 * x1);
  b = b / (x2 - x1);
  a = y1 - c * x1 * x1 - b * x1;

  return a + b * ha + c * pow(ha, 2.0);

}

unsigned int AtmProfile::mkAtmProfile()
{

  static const double
      hx[20] = { 9.225, 10.225, 11.225, 12.850, 14.850, 16.850, 18.850, 22.600, 26.600, 30.600, 34.850, 40.850, 46.850, 52.850, 58.850, 65.100, 73.100, 81.100, 89.100, 95.600 };

  static const double
      px[5][20] = { { 0.3190E+03, 0.2768E+03, 0.2391E+03, 0.1864E+03, 0.1354E+03, 0.9613E+02, 0.6833E+02, 0.3726E+02, 0.2023E+02, 0.1121E+02, 0.6142E+01, 0.2732E+01, 0.1260E+01, 0.6042E+00, 0.2798E+00, 0.1202E+00, 0.3600E-01, 0.9162E-02, 0.2076E-02, 0.6374E-03 }, { 0.3139E+03, 0.2721E+03, 0.2350E+03, 0.1833E+03, 0.1332E+03, 0.9726E+02, 0.7115E+02, 0.3992E+02, 0.2185E+02, 0.1216E+02, 0.6680E+01, 0.2985E+01, 0.1400E+01, 0.6780E+00, 0.3178E+00, 0.1380E+00, 0.4163E-01, 0.9881E-02, 0.2010E-02, 0.5804E-03 }, { 0.2892E+03, 0.2480E+03, 0.2124E+03, 0.1649E+03, 0.1206E+03, 0.8816E+02, 0.6433E+02, 0.3558E+02, 0.1901E+02, 0.1014E+02, 0.5316E+01, 0.2255E+01, 0.1022E+01, 0.4814E+00, 0.2206E+00, 0.9455E-01, 0.3000E-01, 0.8729E-02, 0.2332E-02, 0.8164E-03 }, { 0.3006E+03, 0.2587E+03, 0.2223E+03, 0.1739E+03, 0.1288E+03, 0.9495E+02, 0.7018E+02, 0.3983E+02, 0.2199E+02, 0.1232E+02, 0.6771E+01, 0.3056E+01, 0.1452E+01, 0.7051E+00, 0.3353E+00, 0.1459E+00, 0.4431E-01, 0.1024E-01, 0.1985E-02, 0.5627E-03 }, { 0.2731E+03, 0.2335E+03, 0.1995E+03, 0.1546E+03, 0.1130E+03, 0.8252E+02, 0.6017E+02, 0.3314E+02, 0.1751E+02, 0.9306E+01, 0.4826E+01, 0.1988E+01, 0.8645E+00, 0.4000E+00, 0.1819E+00, 0.7866E-01, 0.2639E-01, 0.8264E-02, 0.2364E-02, 0.8439E-03 } };

  static const double
      tx[5][20] = { { 0.2421E+03, 0.2354E+03, 0.2286E+03, 0.2180E+03, 0.2046E+03, 0.1951E+03, 0.2021E+03, 0.2160E+03, 0.2250E+03, 0.2336E+03, 0.2428E+03, 0.2558E+03, 0.2686E+03, 0.2667E+03, 0.2560E+03, 0.2357E+03, 0.2083E+03, 0.1826E+03, 0.1767E+03, 0.1841E+03 }, { 0.2403E+03, 0.2338E+03, 0.2273E+03, 0.2167E+03, 0.2157E+03, 0.2157E+03, 0.2177E+03, 0.2223E+03, 0.2271E+03, 0.2349E+03, 0.2448E+03, 0.2596E+03, 0.2740E+03, 0.2727E+03, 0.2603E+03, 0.2394E+03, 0.2045E+03, 0.1715E+03, 0.1644E+03, 0.1785E+03 }, { 0.2239E+03, 0.2196E+03, 0.2191E+03, 0.2183E+03, 0.2173E+03, 0.2163E+03, 0.2153E+03, 0.2152E+03, 0.2152E+03, 0.2178E+03, 0.2275E+03, 0.2458E+03, 0.2637E+03, 0.2633E+03, 0.2531E+03, 0.2407E+03, 0.2243E+03, 0.2072E+03, 0.1991E+03, 0.2091E+03 }, { 0.2301E+03, 0.2252E+03, 0.2252E+03, 0.2252E+03, 0.2252E+03, 0.2252E+03, 0.2252E+03, 0.2252E+03, 0.2298E+03, 0.2361E+03, 0.2468E+03, 0.2648E+03, 0.2756E+03, 0.2763E+03, 0.2662E+03, 0.2392E+03, 0.2023E+03, 0.1681E+03, 0.1609E+03, 0.1770E+03 }, { 0.2172E+03, 0.2172E+03, 0.2172E+03, 0.2172E+03, 0.2172E+03, 0.2161E+03, 0.2149E+03, 0.2126E+03, 0.2127E+03, 0.2165E+03, 0.2222E+03, 0.2368E+03, 0.2516E+03, 0.2601E+03, 0.2523E+03, 0.2486E+03, 0.2388E+03, 0.2215E+03, 0.2033E+03, 0.2113E+03 } };

  double T_ground = groundTemperature_.get("K"); //  cout<<"T_ground: " << T_ground <<"K"<<endl;
  double P_ground = groundPressure_.get("mb"); //  cout<<"P_ground: " << P_ground <<"mb"<<endl;
  double rh = relativeHumidity_.get("%"); //  cout<<"rh:       " << rh <<"%"<<endl;
  double h0 = wvScaleHeight_.get("km"); //  cout<<"h0:       " << h0 <<"km"<<endl;
  double dp = pressureStep_.get("mb"); //  cout<<"dp:       " << dp <<"mb"<<endl;
  double alti = altitude_.get("km"); //  cout<<"alti:     " << alti <<"km"<<endl;
  double atmh = topAtmProfile_.get("km"); //  cout<<"atmh:     " << atmh <<"km"<<endl;
  double dp1 = pressureStepFactor_; //  cout<<"dp1:      " << dp1 <<" "<<endl;
  double dt = tropoLapseRate_; // TODO implementer des unites (K/km) ici localement
  double prLimit;

  if(typeAtm_ == 1) {
    prLimit = 230.2;
  } else if(typeAtm_ == 2) {
    prLimit = 198.0;
  } else if(typeAtm_ == 3) {
    prLimit = 300.0;
  } else if(typeAtm_ == 4) {
    prLimit = 311.0;
  } else if(typeAtm_ == 5) {
    prLimit = 332.0;
  } else {
    prLimit = 250.0;  
  }

  unsigned int npp = 0; // number of layers initialized

  double rt = 6371.2E+0; // Earth radius in km
  double g0 = 9.80665E+0; // Earth gravity at the surface  (m/s**2)

  static bool first = true;

  unsigned int i;
  unsigned int i0;
  unsigned int j;
  unsigned int k;
  double wh2o, wgr0, wh2o0;
  double g=0, www=0, altura;
  // double abun_ozono, abun_n2o, abun_co;
  NumberDensity ozono, n2o, co;
  double wgr, dh, humrel;
  // bool   errcode;


  //int    nmaxLayers=40;  // FV peut etre devrions nos avoir un garde-fou au cas ou le nb de couches serait stupidement trop grand


  vector<double> v_layerPressure;
  vector<double> v_layerTemperature;
  vector<double> v_layerThickness;
  vector<double> v_layerWaterVapor;
  vector<double> v_layerO3;
  vector<double> v_layerCO;
  vector<double> v_layerN2O;

  vector<double> v_layerO3_aux;
  vector<double> v_layerCO_aux;
  vector<double> v_layerN2O_aux;
  vector<double> v_layerPressure_aux;
  vector<double> v_layerTemperature_aux;
  vector<double> v_layerThickness_aux;
  vector<double> v_layerWaterVapor_aux;
  vector<NumberDensity> minorden;

  v_layerPressure.push_back(P_ground);
  v_layerThickness.push_back(alti * 1000);
  v_layerTemperature.push_back(T_ground);

  wgr = rwat(Temperature(v_layerTemperature[0], "K"),
             Humidity(rh, "%"),
             Pressure(v_layerPressure[0], "mb")).get("gm**-3");

  // Absolute Humidity in gr/m**3 at altitude alti
  wgr0 = wgr * exp(alti / h0); // h0 in km ==> wgr0 would be the absolute humidity (gr/m**3) at sea level
  wh2o0 = wgr0 * h0; // wh2o0 is the zenith column of water above sea level (kgr/m**2 or mm)
  wh2o = wh2o0 * exp(-alti / h0); // wh2o is the zenith column of water above alti (kgr/m**2 or mm)

  v_layerWaterVapor.push_back(wgr); // in gr/m**3

  i0 = 0;
  i = 0;
  j = 0;
  npp = 0;

  //  cout << "layer " << i << " v_layerThickness[" << i  << "]=" << v_layerThickness[i] << " v_layerPressure[" << i << "]=" << v_layerPressure[i] << endl;


  bool control = true;
  bool control2 = true;
  while(true) {
    i++;

    //    cout << "layer: " << i << " " << v_layerPressure[i - 1] - dp * pow(dp1, (double) (i - 1)) << " " << prLimit << endl;

    if(v_layerPressure[i - 1] - dp * pow(dp1, (double) (i - 1)) <= prLimit) {

      if(control){
	for(k = 0; k < 20; k++) {
	  if(control2){
	    if(v_layerPressure[i - 1] - dp * pow(dp1, (double) (i - 1)) >= px[typeAtm_ - 1][k]){
	      j=k;
	      //	      cout << "P=" << v_layerPressure[i - 1] - dp * pow(dp1, (double) (i - 1)) << " prLimit=" << prLimit << " px[" << typeAtm_-1 << "][" << k << "]=" << px[typeAtm_ - 1][k] << endl;
	      control2 = false;
	    }
	  }
	}
      }

      //      cout << "i,j,v_layerPressure.size()-1=" << i << "," << j << "," << v_layerPressure.size() - 1 << endl;

      if(i0 == 0) i0 = i - 1;
     
      if(i < v_layerPressure.size() - 1) {
	if(control){
	  v_layerPressure[i] = px[typeAtm_ - 1][j-1];
	  v_layerTemperature[i] = tx[typeAtm_ - 1][j-1] - tx[typeAtm_ - 1][0]
	    + v_layerTemperature[i0];
	  dh = 288.6948 * v_layerTemperature[i - 1] * (1.0 + 0.61 * www / 1000.0)
            * log(v_layerPressure[i - 1] / v_layerPressure[i]) / g;
	  v_layerThickness[i] = v_layerThickness[i - 1] + dh;
	}else{
	  j++;
	  v_layerPressure[i] = px[typeAtm_ - 1][j-1];
	  v_layerTemperature[i] = tx[typeAtm_ - 1][j-1] - tx[typeAtm_ - 1][0]
	    + v_layerTemperature[i0];
	  v_layerThickness[i] = (hx[j] - hx[j-1]) * 1000.0 + v_layerThickness[i-1];
	}
	//	cout << "layer " << i << " j=" << j << " v_layerThickness[" << i  << "]=" << v_layerThickness[i] <<  " v_layerPressure[" << i << "]=" << v_layerPressure[i] <<endl;
        v_layerWaterVapor[i] = wgr0 * exp(-v_layerThickness[i] / (1000.0 * h0));
      } else {
	if(control) { 
	  v_layerPressure.push_back(px[typeAtm_ - 1][j-1]);
	  v_layerTemperature.push_back(tx[typeAtm_ - 1][j-1] - tx[typeAtm_ - 1][0]
				       + v_layerTemperature[i0]);
	  dh = 288.6948 * v_layerTemperature[i - 1] * (1.0 + 0.61 * www / 1000.0)
            * log(v_layerPressure[i - 1] / v_layerPressure[i]) / g;
	  v_layerThickness.push_back(v_layerThickness[i - 1] + dh);
	}else{
	  j++;
	  v_layerPressure.push_back(px[typeAtm_ - 1][j-1]);
	  v_layerTemperature.push_back(tx[typeAtm_ - 1][j-1] - tx[typeAtm_ - 1][0]
				       + v_layerTemperature[i0]);
	  v_layerThickness.push_back((hx[j] - hx[j-1]) * 1000.0+ v_layerThickness[i-1]);
	}
	//	cout << "layer " << i << " j=" << j << " v_layerThickness[" << i  << "]=" << v_layerThickness[i] <<  " v_layerPressure[" << i << "]=" << v_layerPressure[i] <<endl;
	v_layerWaterVapor.push_back(wgr0 * exp(-v_layerThickness[i] / (1000.0
								       * h0)));
      }
      if(control) {
        tropoLayer_ = i - 1;
	//	cout << "tropoLayer_=" << tropoLayer_ << endl;
        control = false;
      }
      if(v_layerThickness[i] > (atmh * 1000.0)) break;
    } else {
    
      //      cout << "i,j,v_layerPressure.size()-1=" << i << "," << j << "," << v_layerPressure.size() - 1 << endl;

      if(i > v_layerPressure.size() - 1) {
        v_layerPressure.push_back(v_layerPressure[i - 1] - dp * pow(dp1,
                                                                    (double) (i
                                                                        - 1)));
        www = v_layerWaterVapor[i - 1] / 1000.0; // in kg/m**3
        g = g0 * pow((double) (1. + ((v_layerThickness[i - 1] / 1000.0)) / rt),
                     (double) (-2)); // gravity corrected for the height
        dh = 288.6948 * v_layerTemperature[i - 1] * (1.0 + 0.61 * www / 1000.0)
            * log(v_layerPressure[i - 1] / v_layerPressure[i]) / g;
        v_layerThickness.push_back(v_layerThickness[i - 1] + dh);
	//	cout << "layer " << i << " v_layerThickness[" << i  << "]=" << v_layerThickness[i] << " v_layerPressure[" << i << "]=" << v_layerPressure[i] << endl;
        v_layerTemperature.push_back(v_layerTemperature[i - 1] + dt * dh
            / 1000.0);
        v_layerWaterVapor.push_back(wgr0 * exp(-v_layerThickness[i] / (1000.0
            * h0))); //r[i] in kgr/(m**2*1000m) [gr/m**3]
      } else {
        v_layerPressure[i] = v_layerPressure[i - 1] - dp * pow(dp1, (double) (i
            - 1));
        www = v_layerWaterVapor[i - 1] / 1000.0; // in kg/m**3
        g = g0 * pow((double) (1. + ((v_layerThickness[i - 1] / 1000.0)) / rt),
                     (double) (-2)); // gravity corrected for the height
        dh = 288.6948 * v_layerTemperature[i - 1] * (1.0 + 0.61 * www / 1000.0)
            * log(v_layerPressure[i - 1] / v_layerPressure[i]) / g;
        v_layerThickness[i] = v_layerThickness[i - 1] + dh;
	//	cout << "layer " << i << " v_layerThickness[" << i  << "]=" << v_layerThickness[i] <<  " v_layerPressure[" << i << "]=" << v_layerPressure[i] << endl;
        v_layerTemperature[i] = v_layerTemperature[i - 1] + dt * dh / 1000.0;
        v_layerWaterVapor[i] = wgr0 * exp(-v_layerThickness[i] / (1000.0 * h0)); //r[i] in kgr/(m**2*1000m) [gr/m**3]
      }

      humrel = rwat_inv(Temperature(v_layerTemperature[i], "K"),
                        MassDensity(v_layerWaterVapor[i], "gm**-3"),
                        Pressure(v_layerPressure[i], "mb")).get("%");

      /*	cout << "layer " << i
       << " P " << v_layerPressure[i] << " prLimit " << prLimit
       << " T " << v_layerTemperature[i]
       << " Alti (layer top in m) " << v_layerThickness[i]
       << " WaterVapor " << v_layerWaterVapor[i] << endl;  	*/

      if(v_layerThickness[i] > (atmh * 1000.0)) break;
    }
  }

  npp = i - 1;

  // cout << "npp=" << npp << endl;
  // cout << "tropoLayer=" << tropoLayer_ << endl;
  tropoTemperature_ = Temperature(v_layerTemperature[tropoLayer_], "K");
  tropoAltitude_ = Length(v_layerThickness[tropoLayer_], "m");
  // cout << "tropoAltitude=" << tropoAltitude_.get("km") << " km" << endl;
  // cout << "tropoTemperature=" << tropoTemperature_.get("K") << " K" << endl;
  // cout << "ground Altitude=" << altitude_.get("km") << " km" << endl;
  // cout << "ground Temperature=" << groundTemperature_.get("K") << " K" << endl;
  // cout << "Calculated Lapse Rate=" << (tropoTemperature_.get("K")-groundTemperature_.get("K"))/(tropoAltitude_.get("km")-altitude_.get("km")) << " K/km" << endl;


  altura = alti;

  /*
   if(first){
   v_layerO3_.reserve(npp);
   v_layerCO_.reserve(npp);
   v_layerN2O_.reserve(npp);
   }
   */

  for(j = 0; j < npp; j++) {

    v_layerThickness[j] = (v_layerThickness[j + 1] - v_layerThickness[j]); // in m
    altura = altura + v_layerThickness[j] / 2.0E3; // in km
    v_layerTemperature[j] = (v_layerTemperature[j + 1] + v_layerTemperature[j])
        / 2.; // in K
    v_layerPressure[j] = exp((log(v_layerPressure[j + 1])
        + log(v_layerPressure[j])) / 2.0); // in mb
    v_layerWaterVapor[j] = 1.0E-3 * exp((log(v_layerWaterVapor[j + 1]) + // in kg/m**3
        log(v_layerWaterVapor[j])) / 2.0); //  1.0E-3 ?


    //      cout << "type_=" << type_ << endl;
    //      cout << "typeAtm_=" << typeAtm_ << endl;

    unsigned int atmType = typeAtm_; // conversion in int

    //      cout << "going to minorden with atmType=" << atmType << endl;

    minorden = st76(Length(altura, "km"), atmType);

    //      cout << "Ozone: " << abun_ozono << "  " << ozono.get("cm**-3") << endl;
    // cout << "N2O  : " << abun_n2o << "  " << n2o.get("cm**-3") << endl;
    // cout << "CO   : " << abun_co  << "  " << co.get("cm**-3") << endl;


    /* cout << j << " " << v_layerO3_.size()-1 << endl; */

    /*    if(j>v_layerO3_.size()-1){
     v_layerO3_.push_back(  1.E6*abun_ozono); // in m**-3
     v_layerCO_.push_back(  1.E6*abun_co   ); // in m**-3
     v_layerN2O_.push_back( 1.E6*abun_n2o  ); // in m**-3
     cout << "uno" << endl;
     }else{ */

    /*    v_layerO3_[j]  = 1.E6*abun_ozono;        // in m**-3
     v_layerCO_[j]  = 1.E6*abun_co;           // in m**-3
     v_layerN2O_[j] = 1.E6*abun_n2o;          // in m**-3 */

    // v_layerO3.push_back(1.E6*abun_ozono);        // in m**-3
    // v_layerCO.push_back(1.E6*abun_co);           // in m**-3
    // v_layerN2O.push_back(1.E6*abun_n2o);          // in m**-3

    v_layerO3.push_back(1.E6 * minorden[0].get("cm**-3")); // in m**-3
    v_layerCO.push_back(1.E6 * minorden[2].get("cm**-3")); // in m**-3
    v_layerN2O.push_back(1.E6 * minorden[1].get("cm**-3")); // in m**-3

    /*     } */

    altura = altura + v_layerThickness[j] / 2.0E3;

    /* cout << "j=" << j << "v_layerThickness.size()=" << v_layerThickness.size() <<  endl; */

  }

  /*  cout << "j=" << j << " v_layerThickness_aux.size()= " << v_layerThickness_aux.size() <<  endl; */

  /* if(v_layerThickness.size()>npp){
   for(j=npp; j<v_layerThickness.size(); j++){
   delete v_layerThickness[j];
   }
   } */

  for(j = 0; j < npp; j++) {

    v_layerPressure_aux.push_back(v_layerPressure[j]);
    v_layerTemperature_aux.push_back(v_layerTemperature[j]);
    v_layerThickness_aux.push_back(v_layerThickness[j]);
    v_layerWaterVapor_aux.push_back(v_layerWaterVapor[j]);
    v_layerO3_aux.push_back(v_layerO3[j]);
    v_layerCO_aux.push_back(v_layerCO[j]);
    v_layerN2O_aux.push_back(v_layerN2O[j]);

  }

  if(j > v_layerPressure_.size() - 1) { // ?????
    v_layerPressure_.reserve(npp);
    v_layerTemperature_.reserve(npp);
    v_layerThickness_.reserve(npp);
    v_layerWaterVapor_.reserve(npp);
    v_layerO3_.reserve(npp);
    v_layerCO_.reserve(npp);
    v_layerN2O_.reserve(npp);
  }

  v_layerPressure_ = v_layerPressure_aux;
  v_layerTemperature_ = v_layerTemperature_aux;
  v_layerThickness_ = v_layerThickness_aux;
  v_layerWaterVapor_ = v_layerWaterVapor_aux;
  v_layerO3_ = v_layerO3_aux;
  v_layerCO_ = v_layerCO_aux;
  v_layerN2O_ = v_layerN2O_aux;

  first = false; // ?????
  /* cout << " " << endl;
   cout << "v_layerPressure_.size() " << v_layerPressure_.size() << " P_ground=" << P_ground << endl;
   cout << "v_layerTemperature_.size() " << v_layerTemperature_.size() << endl;
   cout << "v_layerO3_.size() " << v_layerO3_.size() << endl; */
  return npp;
}

} // namespace atm

