#ifndef _ATM_WATERVAPORRADIOMETER_H
#define _ATM_WATERVAPORRADIOMETER_H
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
 * "@(#) $Id: ATMWaterVaporRadiometer.h,v 1.4 2009/05/04 21:30:54 dbroguie Exp $"
 *
 * who       when      what
 * --------  --------  ----------------------------------------------
 * pardo     24/03/09  created
 */

#ifndef __cplusplus
#error This is a C++ include file and cannot be used from plain C
#endif

#include "ATMPercent.h"
#include "ATMTemperature.h"

#include <vector>

using namespace std;

namespace atm
{
/*! \brief This is an axuliary class to SkyStatus that allows to identify the spectral windows
 *         that actually correspond to a dedicated water vapor radiometry system, and also to
 *         associate to those spectral windows some instrumental caracteristics such as the signal
 *         side band gain, the sky coupling or the spillover temperature.
 */
class WaterVaporRadiometer
{
public:

  WaterVaporRadiometer() {}

  /** Class constructor with identifiers of radiometric channels. Sky Coupling = 1.0, Sideband Gain: 50% (no spilloverTemperature given) */
  WaterVaporRadiometer(vector<unsigned int> IdChannels);

  /** Class constructor with identifiers of radiometric channels. Sideband Gain: 50% (no spilloverTemperature given) */
  WaterVaporRadiometer(vector<unsigned int> IdChannels,
                       vector<double> skyCoupling);

  inline WaterVaporRadiometer(const vector<unsigned int> &IdChannels,
                              double skyCoupling)
  {
    WaterVaporRadiometer(IdChannels, vector<double> (IdChannels.size(),
                                                     skyCoupling));
  }

  /** Class constructor with identifiers of radiometric channels. Sky Coupling = 1.0 (no spilloverTemperature given) */
  WaterVaporRadiometer(vector<unsigned int> IdChannels,
                       vector<Percent> signalGain);

  /* WaterVaporRadiometer(vector<unsigned int> IdChannels, Percent signalGain); */

  /** Full class constructor (no spilloverTemperature given) */
  WaterVaporRadiometer(vector<unsigned int> IdChannels,
                       vector<double> skyCoupling,
                       vector<Percent> signalGain);

  /** Class constructor with identifiers of radiometric channels. Sky Coupling = 1.0, Sideband Gain: 50% (spilloverTemperature given) */
  WaterVaporRadiometer(vector<unsigned int> IdChannels,
                       Temperature spilloverTemperature);

  /** Class constructor with identifiers of radiometric channels. Sideband Gain: 50% (spilloverTemperature given) */
  WaterVaporRadiometer(vector<unsigned int> IdChannels,
                       vector<double> skyCoupling,
                       Temperature spilloverTemperature);

  /** Class constructor with identifiers of radiometric channels. Sky Coupling = 1.0 (spilloverTemperature given) */
  WaterVaporRadiometer(vector<unsigned int> IdChannels,
                       vector<Percent> signalGain,
                       Temperature spilloverTemperature);

  /** Full class constructor (spilloverTemperature given) */
  WaterVaporRadiometer(vector<unsigned int> IdChannels,
                       vector<double> skyCoupling,
                       vector<Percent> signalGain,
                       Temperature spilloverTemperature);

  virtual ~WaterVaporRadiometer();

  /** Accessor to identifiers of radiometric channels (vector of int) */
  inline vector<unsigned int> getIdChannels() { return IdChannels_; }

  /** Accessor to Sky coupling of the different radiometric channels (vector of double) */
  inline vector<double> getSkyCoupling() { return skyCoupling_; }

  /** Multiplier of the Sky coupling vector  */
  inline void multiplySkyCoupling(double factor)
  {
    for(unsigned int i = 0; i < skyCoupling_.size(); i++) {
      skyCoupling_[i] = skyCoupling_[i] * factor;
    }
  }

  /** Setter of the Sky coupling vector to a single Sky Coupling value */
  inline void setSkyCoupling(double factor)
  {
    /* for(unsigned int i=0; i<skyCoupling_.size(); i++){ skyCoupling_[i] = factor;} */
    for(vector<double>::iterator it(skyCoupling_.begin()),
        itmax(skyCoupling_.end()); it != itmax; ++it) {
      (*it) = factor;
    }
  }
  inline void setSkyCoupling(unsigned int i, double factor)
  {
    if(i < skyCoupling_.size()) {
      skyCoupling_[i] = factor;
    } else { /* throw(Error()); */
    }
  }

  /** Accessor to signalGain of the different radiometric channels (vector of percent) */
  inline vector<Percent> getsignalGain() { return signalGain_; }
  /** Accessor to SpilloverTemperature */
  inline Temperature getSpilloverTemperature() { return spilloverTemperature_; }
  /** Setter of SpilloverTemperature */
  inline void setSpilloverTemperature(Temperature spilloverTemperature) { spilloverTemperature_ = spilloverTemperature; }

protected:
  vector<unsigned int> IdChannels_;
  vector<double> skyCoupling_;
  vector<Percent> signalGain_;
  Temperature spilloverTemperature_;

private:
  //     /** Inaccessible Copy Constructor */
  //     WaterVaporRadiometer(const WaterVaporRadiometer&);

  //      /** Inaccessible assignment operator */
  //      WaterVaporRadiometer& operator=(const WaterVaporRadiometer&);


}; // class WaterVaporRadiometer

} // namespace atm

#endif /*!_ATM_WATERVAPORRADIOMETER_H*/

