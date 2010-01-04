#if     !defined(ATM_WATERVAPORRADIOMETER_H)

#include "ATMPercent.h"
#include "ATMTemperature.h"

namespace atm {
  /*! \brief This is an axuliary class to SkyStatus that allows to identify the spectral windows  
   *         that actually correspond to a dedicated water vapor radiometry system, and also to 
   *         associate to those spectral windows some instrumental caracteristics such as the signal 
   *         side band gain, the sky coupling or the spillover temperature.
   */
  class WaterVaporRadiometer
    {
    public:
      
      /** Class constructor with no radiometric channels */
      WaterVaporRadiometer();
      
      /** Class constructor with identifiers of radiometric channels. Sky Coupling = 1.0, Sideband Gain: 50% (no spilloverTemperature given) */
      WaterVaporRadiometer(vector<unsigned int> IdChannels);
      
      /** Class constructor with identifiers of radiometric channels. Sideband Gain: 50% (no spilloverTemperature given) */
      WaterVaporRadiometer(vector<unsigned int> IdChannels, vector<double> skyCoupling);
      
      /** Class constructor with identifiers of radiometric channels. Sky Coupling = 1.0 (no spilloverTemperature given) */
      WaterVaporRadiometer(vector<unsigned int> IdChannels, vector<Percent> signalGain);
      
      /** Full class constructor (no spilloverTemperature given) */
      WaterVaporRadiometer(vector<unsigned int> IdChannels, vector<double> skyCoupling, vector<Percent> signalGain);
      
      /** Class constructor with identifiers of radiometric channels. Sky Coupling = 1.0, Sideband Gain: 50% (spilloverTemperature given) */
      WaterVaporRadiometer(vector<unsigned int> IdChannels, Temperature spilloverTemperature);
      
      /** Class constructor with identifiers of radiometric channels. Sideband Gain: 50% (spilloverTemperature given) */
      WaterVaporRadiometer(vector<unsigned int> IdChannels, vector<double> skyCoupling, Temperature spilloverTemperature);
      
      /** Class constructor with identifiers of radiometric channels. Sky Coupling = 1.0 (spilloverTemperature given) */
      WaterVaporRadiometer(vector<unsigned int> IdChannels, vector<Percent> signalGain, Temperature spilloverTemperature);
      
      /** Full class constructor (spilloverTemperature given) */
      WaterVaporRadiometer(vector<unsigned int> IdChannels, vector<double> skyCoupling, vector<Percent> signalGain, Temperature spilloverTemperature);
      
      virtual ~WaterVaporRadiometer();
      
      /** Accessor to identifiers of radiometric channels (vector of int) */
      vector<unsigned int> getIdChannels(){return IdChannels_;}
      /** Accessor to Sky coupling of the different radiometric channels (vector of double) */
      vector<double> getSkyCoupling(){return skyCoupling_;}
      /** Multiplier of the Sky coupling vector  */
      void multiplySkyCoupling(double factor){for(unsigned int i=0; i<skyCoupling_.size(); i++){ skyCoupling_[i] = skyCoupling_[i]*factor;}}
      /** Setter of the Sky coupling vector to a single Sky Coupling value */
      void setSkyCoupling(double factor){for(unsigned int i=0; i<skyCoupling_.size(); i++){ skyCoupling_[i] = factor;}}
      void setSkyCoupling(double factor, unsigned int i){if(i<skyCoupling_.size()){ skyCoupling_[i] = factor;}}
      void setSkyCoupling(unsigned int i, double factor){if(i<skyCoupling_.size()){ skyCoupling_[i] = factor;}}
      /** Accessor to signalGain of the different radiometric channels (vector of percent) */
      vector<Percent> getsignalGain(){return signalGain_;}
      /** Accessor to SpilloverTemperature */
      Temperature getSpilloverTemperature(){return spilloverTemperature_;}
      /** Setter of SpilloverTemperature */
      void setSpilloverTemperature(Temperature spilloverTemperature){spilloverTemperature_=spilloverTemperature;}
      
    protected:
      vector<unsigned int> IdChannels_;
      vector<double> skyCoupling_;
      vector<Percent> signalGain_;
      Temperature spilloverTemperature_;
    };
}

#define ATM_WATERVAPORRADIOMETER_H
#endif
