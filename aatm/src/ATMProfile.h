#ifndef _ATM_PROFILE_H
#define _ATM_PROFILE_H
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
 * "@(#) $Id: ATMProfile.h,v 1.5 2009/09/10 16:21:29 dbroguie Exp $"
 *
 * who       when      what
 * --------  --------  ----------------------------------------------
 * pardo     24/03/09  created
 */

#ifndef __cplusplus
#error This is a C++ include file and cannot be used from plain C
#endif

#include "ATMPressure.h"
#include "ATMMassDensity.h"
#include "ATMNumberDensity.h"
#include "ATMTemperature.h"
#include "ATMLength.h"
#include "ATMHumidity.h"
#include "ATMEnumerations.h"

#include <string>
#include <vector>

namespace atm
{
/** \brief Class for an atmospheric profile object.
 *
 *         An  atmospheric profile is composed of 4 quantities as a function of altitude z:
 *         - the layer thickness
 *         - the pressure P
 *         - the temperature T and
 *         - the gas densities for H2O, O3, CO and N2O.<br>
 *
 *   This object is needed  for computing the absorption and
 *   phase coefficients, as well as for performing radiative transfer
 *   calculations (only layer thickness/T are needed).
 *
 *   This class builds an atmospheric profile that can be used to
 *   calculate absorption and phase coefficients, as well as to
 *   perform forward and/or retrieval radiative transfer calculations.
 *   It is composed of a set of parameters needed to build a layer thickness/P/T/gas densities
 *   densities profile from simple parameters currently available at observatories
 *   (from weather stations for example) using functions from
 *   the ATM library. The set of input parameters consists of the
 *   pressure P, the temperature T and the relative humidity at the ground, the altitude of
 *   the site, the tropospheric temperature lapse rate,... The profile is built as:
 *   thickness of the considered atmospheric layers above the site, and mean
 *   P,T,H2O,O3,CO,N2O in
 *   them. The total number of atmospheric layers in the particular profile
 *   is also available (a negative value indicates an error).<br>
 *   The zenith column  of water vapor can be calculated
 *   by simply integrating the H2O profile. \f[ column\;\;H_{2}O\;\; (mm) =\sum_{layers} \Delta z \cdot [ H_{2}O ] \f]
 */
class AtmProfile
{
public:

  /** A constructor of an empty profile with n layers, that can be filled up later. */
  AtmProfile(unsigned int n);

  /** A long constructor of the atmospheric profile from the basic set of parameters described above. */
  AtmProfile(Length altitude,
             Pressure groundPressure,
             Temperature groundTemperature,
             double tropoLapseRate,
             Humidity relativeHumidity,
             Length wvScaleHeight,
             Pressure pressureStep,
             double pressureStepFactor,
             Length topAtmProfile,
             unsigned int atmType); //Atmospheretype atmType);

  /** A long constructor of the atmospheric profile from the basic set of parameters described above. */
  /*
   AtmProfile(Length altitude, Pressure    groundPressure,
   Temperature groundTemperature, double  tropoLapseRate,
   Humidity relativeHumidity, Length wvScaleHeight,
   Pressure pressureStep, double pressureStepFactor,
   Length topAtmProfile, int typeAtm);

   */

  /** A short constructor of the atmospheric profile. With respect to the long constructor, this one considers
   the following (recomended) default values: pressureStep = 10 mb,
   pressureStepFactor = 1.2, topAtmProfile = 48.0 km */
  AtmProfile(Length altitude,
             Pressure groundPressure,
             Temperature groundTemperature,
             double tropoLapseRate,
             Humidity relativeHumidity,
             Length wvScaleHeight,
             unsigned int atmType); //Atmospheretype atmType);

  /** The user provides his own atmospheric profile (basic one: four vectors for layer thickness in m,
   average pressure in each layer in mb, average temperature in each layer in K, and average water
   vapor density in each layer in kg/m**3). QUESTION: SHOULD O3, CO and N2O PROFILES BE FILLED UP
   INTERNALLY FROM A STANDARD ATMOSPHERE OR LEFT ITS ABUNDANCES EQUAL TO ZERO ? */
  AtmProfile(vector<Length> v_layerThickness,
             vector<Pressure> v_layerPressure,
             vector<Temperature> v_layerTemperature,
             vector<MassDensity> v_layerWaterVapor);
  AtmProfile(vector<Length> v_layerThickness,
             vector<Pressure> v_layerPressure,
             vector<Temperature> v_layerTemperature,
             vector<NumberDensity> v_layerWaterVapor);

  /** The user provides his own atmospheric profile (in this case five vectors for layer thickness in m,
   average pressure in each layer in mb, average temperature in each layer in K, average water vapor
   density in each layer in kg/m**3, and average ozone number density in each layer in molecules/m**3)
   QUESTION: SHOULD CO and N2O PROFILES BE FILLED UP
   INTERNALLY FROM A STANDARD ATMOSPHERE OR LEFT ITS ABUNDANCES EQUAL TO ZERO ? */
  AtmProfile(vector<Length> v_layerThickness,
             vector<Pressure> v_layerPressure,
             vector<Temperature> v_layerTemperature,
             vector<MassDensity> v_layerWaterVapor,
             vector<NumberDensity> v_layerO3);
  AtmProfile(vector<Length> v_layerThickness,
             vector<Pressure> v_layerPressure,
             vector<Temperature> v_layerTemperature,
             vector<NumberDensity> v_layerWaterVapor,
             vector<NumberDensity> v_layerO3);

  /** The user provides his own atmospheric profile (in this case seven vectors for layer thickness in m,
   average pressure in each layer in mb, average temperature in each layer in K, average water vapor
   density in each layer in kg/m**3, average ozone number density in each layer in molecules/m**3,
   average CO number density in each layer in molecules/m**3, average N2O number density in each
   layer in molecules/m**3) */
  AtmProfile(vector<Length> v_layerThickness,
             vector<Pressure> v_layerPressure,
             vector<Temperature> v_layerTemperature,
             vector<MassDensity> v_layerWaterVapor,
             vector<NumberDensity> v_layerO3,
             vector<NumberDensity> v_layerCO,
             vector<NumberDensity> v_layerN2O);
  AtmProfile(vector<Length> v_layerThickness,
             vector<Pressure> v_layerPressure,
             vector<Temperature> v_layerTemperature,
             vector<NumberDensity> v_layerWaterVapor,
             vector<NumberDensity> v_layerO3,
             vector<NumberDensity> v_layerCO,
             vector<NumberDensity> v_layerN2O);

  AtmProfile(const AtmProfile &); // copy constructor

  virtual ~AtmProfile();

  /** Setter to update the AtmProfile if some basic atmospheric parameter has changed.
   * @pre   an atmospheric profile already exists
   * @param altitude          the new altitude, a Length
   * @param groundPressure    the Pressure at the ground level
   * @param groundTemperature the Temperature at the ground level
   * @param tropoLapseRate    the tropospheric lapse rate
   * @param relativeHumidity  the relative Humidity
   * @param wvScaleHeight     the scale height of the water vapor, a Length
   * @return true if the atmospheric profile has been update, else false because the basic parameters have not changed
   * @post   the atmospheric profile has been updated, only if at least one of the basic parameters has changed
   *
   * \note  there is an overriding on this method in the sub-class AbsorptionPhaseProfile which in turn
   *        has an  overriding in its WaterVaporRetrieval sub-class. Hence this method must not be overloaded
   *        in this AtmProfile class.
   */
  bool setBasicAtmosphericParameters(Length altitude,
                                     Pressure groundPressure,
                                     Temperature groundTemperature,
                                     double tropoLapseRate,
                                     Humidity relativeHumidity,
                                     Length wvScaleHeight);

  /** Accessor to the type of current atmosphere **/
  string getAtmosphereType();

  /** Accessor to the type of atmosphere specified by the number**/
  static string getAtmosphereType(unsigned int typeAtm);

  /** Accessor to the current Ground Temperature used in the object */
  Temperature getGroundTemperature() { return groundTemperature_; }

  /** Accessor to the current Tropospheric Lapse Rate used in the object (temperature/length units) */
  double getTropoLapseRate() { return tropoLapseRate_; }

  /** Accessor to the current Ground Pressure used in the object (pressure units) */
  Pressure getGroundPressure() { return groundPressure_; }

  /** Accessor to the current Tropopause Temperature used in the object */
  Temperature getTropopauseTemperature() { return tropoTemperature_; }

  /** Accessor to the current Ground Relative Humidity in the object (humidity units) */
  Humidity getRelativeHumidity() { return relativeHumidity_; }

  /** Accessor to the current Water Vapor Scale Height in the object (length units) */
  Length getWvScaleHeight() { return wvScaleHeight_; }

  /** Accessor to the current Primary Pressure Step in the object. The Primary Pressure Step
   (pressure units) is used to define the thickness of the first layer in the profile.
   Pressure difference between the boundaries of first layer will be equal to the
   Primary Pressure Step. */
  Pressure getPressureStep() { return pressureStep_; }

  /** Accessor to the current Pressure_Step_Factor in the object. The Pressure_Step_Factor
   (no units) is the Pressure step change between consecutive layers when moving upwards.
   Pressure difference between the boundaries of the (n+1)-th layer (DP_n+1) will
   be DP_n**DP1. */
  Pressure getPressureStepFactor() { return pressureStepFactor_; }

  /** Accessor to the altitude of site (length units) */
  Length getAltitude() { return altitude_; }

  /** Accessor to the altitude of the tropopause (length units) */
  Length getTropopauseAltitude() { return tropoAltitude_; }

  /** Accessor to the Maximum allowed altitude for the Atmospheric Profile above the site (length units) */
  Length getTopAtmProfile() { return topAtmProfile_; }

  /** Accessor to the number of layers of the atmospheric profile */
  unsigned int getNumLayer() { return numLayer_; }

  /** Method to access the Temperature Profile  */
  vector<Temperature> getTemperatureProfile();

  /** Method to access the average Temperature in layer i (thickness of layers in ThicknessProfile)  */
  Temperature getLayerTemperature(unsigned int i);

  /** Setter for the average Temperature in layer i (allows to touch one layer each time once a profile has been defined) */
  void setLayerTemperature(unsigned int i, Temperature layerTemperature);
  void setLayerTemperature(Temperature layerTemperature, unsigned int i){setLayerTemperature(i, layerTemperature);}

  /** Method to retrieve the layer thickness from site altitude upwards.
   *  Use Altitude to + ThicknessProfile to know the vertical grid. */
  vector<Length> getThicknessProfile();


  /** Method to access the layer thickness of layer i  */
  Length getLayerThickness(unsigned int i);


  /** Setter for the thickness of layer i (allows to touch one layer each time once a profile has been defined). We do
   *  not advise to use this one unless you change P and T accordingly */
  void setLayerThickness(unsigned int i, Length layerThickness);
  void setLayerThickness(Length layerThickness, unsigned int i){setLayerThickness(i, layerThickness);}

  /** Function to retrieve Average Water vapor density in a given layer in kg/m**3. 
   *  The thickness of the layer can be accessed with ThicknessProfile.  */
  MassDensity getLayerWaterVaporMassDensity(unsigned int i);

  /** Function to retrieve Average Water vapor number density in a given layer (in m**-3).  
   *  The thickness of the layer can be accessed with ThicknessProfile.  */
  NumberDensity getLayerWaterVaporNumberDensity(unsigned int i);

  /** Setter for the average Water vapor density in layer i in kg/m**3 */
  void setLayerWaterVaporMassDensity(unsigned int i, MassDensity layerWaterVapor);
  void setLayerWaterVaporMassDensity(MassDensity layerWaterVapor, unsigned int i){setLayerWaterVaporMassDensity(i, layerWaterVapor);}

  /** Setter for the average Water vapor number density in layer i */
  void setLayerWaterVaporNumberDensity(unsigned int i, NumberDensity layerWaterVapor);
  void setLayerWaterVaporNumberDensity(NumberDensity layerWaterVapor, unsigned int i){setLayerWaterVaporNumberDensity(i, layerWaterVapor);}

  /** Method to get the Pressure Profile */
  vector<Pressure> getPressureProfile();

  /** Method to access the average Pressure in layer i */
  Pressure getLayerPressure(unsigned int i);

  /** Setter for the average Pressure in layer i (allows to touch one layer each
   *  time once a profile has been defined) */
  void setLayerPressure(unsigned int i, Pressure layerPressure) { v_layerPressure_[i] = layerPressure.get("mb"); }
  void setLayerPressure(Pressure layerPressure, unsigned int i) { setLayerPressure(i, layerPressure); }

  /** Function to retrieve CO density in a given layer (thickness of layers
   *  in ThicknessProfile)  */
  NumberDensity getLayerCO(unsigned int i) { return NumberDensity(v_layerCO_[i], "m**-3"); }
  /** Setter for the average number density of CO in layer i in molecules/m**3 (allows to touch one layer each
   *  time once a profile has been defined) */
  void setLayerCO(unsigned int i, NumberDensity layerCO) { v_layerCO_[i] = layerCO.get("m**-3"); }
  void setLayerCO(NumberDensity layerCO, unsigned int i) { setLayerCO(i, layerCO); }

  /** Function to retrieve O3 density in a given layer (thickness of layers
   *  in ThicknessProfile) */
  NumberDensity getLayerO3(unsigned int i) { return NumberDensity(v_layerO3_[i], "m**-3"); }
  /** Setter for the average number density of O3 in layer i in molecules/m**3 (allows to touch one layer each
   *  time once a profile has been defined) */
  void setLayerO3(unsigned int i, NumberDensity layerO3) { v_layerO3_[i] = layerO3.get("m**-3"); }
  void setLayerO3(NumberDensity layerO3, unsigned int i) { setLayerO3(i, layerO3); }

  /** Function to retrieve N2O density in a given layer (thickness of layers
   *  in ThicknessProfile)   */
  NumberDensity getLayerN2O(unsigned int i) { return NumberDensity(v_layerN2O_[i], "m**-3"); }
  /** Setter for the average number density of N2O in layer i in molecules/m**3 (allows to touch one layer each
   *  time once a profile has been defined) */
  void setLayerN2O(unsigned int i, NumberDensity layerN2O) { v_layerN2O_[i] = layerN2O.get("m**-3"); }
  void setLayerN2O(NumberDensity layerN2O, unsigned int i) { setLayerN2O(i, layerN2O); }

  /** Method to get the zenith column  of water vapor. It is computed by
   *  simply integrating the H2O profile:
   *  \f$ column\;\;H_{2}O\;\; (mm) =\sum_{layers} \Delta z \cdot [ H_{2}O ] \f$
   */
  Length getGroundWH2O();

  //@}

protected:
  unsigned int typeAtm_; //!< 1: tropical, 2: midlatSummer, 3: midlatWinter, 4: subarcticSummer, 5: subarcticWinter
  Temperature groundTemperature_; //!< Ambient temperature at the site (K)
  double tropoLapseRate_; //!< tropospheric lapse rate in K/km
  Temperature tropoTemperature_; //!< Temperature at the tropopause
  unsigned int tropoLayer_; //!< Layer where tropopause starts
  Length tropoAltitude_; //!< Altitude where tropopause starts
  Pressure groundPressure_; //!< Ground pressure at the site (mb)
  Humidity relativeHumidity_; /** Relative humidity at the site (%)
   used only to make an estimate
   of the water vapor column, first guess) */
  Length wvScaleHeight_; //!< scale height of water vapor distribution (km)
  Pressure pressureStep_; //!< Pressure basic step (mb)
  double pressureStepFactor_; /** Multiplicative factor for presure steps.
   Example of pressure parameters:
   P_ground=550; DP: 10; DP1:
   1.2 ==> The pressure levels will
   then be 550, 560, 572, 586.4, .... */
  Length altitude_; //!< Altitude of the site (km)
  Length topAtmProfile_; //!< Top of atmospheric profile (km)
  unsigned int numLayer_; //!< Total number of layers in the output	atmospherice profiles
  bool newBasicParam_;
  vector<double> v_layerThickness_; //!< Thickness of layer (m)
  vector<double> v_layerTemperature_; //!< Temp. of layer (K)
  vector<double> v_layerWaterVapor_; //!< Water vapor kg/m**3
  vector<double> v_layerPressure_; //!< Pressure in mb
  vector<double> v_layerCO_; //!< CO in molecules per m**3
  vector<double> v_layerO3_; //!< O3 in molecules per m**3
  vector<double> v_layerN2O_; //!< N2O in molecules per m**3

  unsigned int ier_;
  AtmProfile(); /** Default constructor (required if copy constructor in derived classes) */
  /*
   * \fn Method to build the profile,
   */
  unsigned int mkAtmProfile(); /** returns error code: <0 unsuccessful           */

  /** Method to update an atmospheric profile based on one or more new basic parameter(s)
   * @param altitude          the new altitude, a Length
   * @param groundPressure    the Pressure at the ground level
   * @param groundTemperature the Temperature at the ground level
   * @param tropoLapseRate    the tropospheric lapse rate
   * @param relativeHumidity  the relative Humidity
   * @param wvScaleHeight     the scale height of the water vapor, a Length
   * @return true if the atmospheric profile has been update, else false because the basic parameters have not changed
   * @post   the atmospheric profile has been updated only if at least one of the basic parameters has changed
   */
  bool updateAtmProfile(Length altitude,
                        Pressure groundPressure,
                        Temperature groundTemperature,
                        double tropoLapseRate,
                        Humidity relativeHumidity,
                        Length wvScaleHeight);

private:
  MassDensity rwat(Temperature t, Humidity rh, Pressure p);
  Humidity rwat_inv(Temperature tt, MassDensity dd, Pressure pp);
  vector<NumberDensity> st76(Length ha, unsigned int tip);
  double poli2(double ha,
               double x1,
               double x2,
               double x3,
               double y1,
               double y2,
               double y3);
  /*
   // static Pressure pressureStep_default(  10.0,"mb");
   //  static double pSd; // = 10.0;  // pressureStep_default in mb
   static Pressure pressureStep_default_;


   static double pressureStepFactor_default_; // =    1.2;
   //static double pSFd; // = 1.2;



   static Length  topAtmProfile_default_; //(  48.0,"km");
   //static tAPd; // = 48.0;
   */

  /** Default value of Atmospheric type (to reproduce behavior above the tropopause) */
  //static Atmospheretype atmType_default =  tropical;

}; // class AtmProfile

} // namespace atm

/*
  Pressure AtmProfile::pressureStep_default_(Pressure(1.2,"mb"));
  double AtmProfile::pressureStepFactor_default_(1.2);
  Length AtmProfile::topAtmProfile_default_(Length(48,"km"));
*/

/** \page AtmProfile_example Example with the AtmProfile class.
 *  Our main function always starts like this:
 *  \skip #include "AtmTypeName.h"
 *  \until {
 *
 *  First we set a constant that will be used in the calculations:
 *  \skip double
 *  \until 0.04799274551
 *  Then we set the parameters necessary to create an atmospheric profile (if you do not want to enter the whole profile yourself):
 *  \skip  Atmospheretype
 *  \until 1.2
 *  The above input information is printed on the screen
 *  \skip cout
 *  \until K/km
 *  With all this information an AtmProfile object can now be created:
 *  \skipline AtmProfile
 *  The AtmProfile object "myProflie" is accessed and partially shown on the screen
 *  \skip cout
 *  \until scale height */

#endif /*!_ATM_PROFILE_H*/
