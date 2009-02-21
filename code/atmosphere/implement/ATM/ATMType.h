#if     !defined(ATM_TYPE_H)


#include <atmosphere/ATM/ATMAtmosphereType.h>

namespace atm {
  /*! \brief Defines atmospheric behavior above the tropopause.
   *
   *   This class defines the behavior of the atmospheric profile above the tropopause 
   *   based on the location of the observatory (ex. for the ALMA site, the Standard Tropical 
   *   atmosphere is selected as model. AtmType will not act on the profile below the tropopause, 
   *   that will be defined by the ground P7T/hum, tropospheric temperature lapse rate and water 
   *   vapor scale height, as defined directly in the AtmProfile Class. 
   *   
   */
    class AtmType
    {
    public:
      /** A full constructor for the atmosphere type (to be considered only above the tropopause) */
      AtmType(Atmospheretype type);
      ~AtmType();
      
      //@{ 
      /** Accessor to get the type of atmosphere used to describe the behavior above the tropopause */
      string  getAtmosphereType();
      /** Accessor to get the type of atmosphere used to describe the behavior above the tropopause */
      Atmospheretype  getAtmospheretype();
      /** Accessor to get the size of the arrays that describe the behavior above the tropopause    */
      double getTropopausePressure(){return prLimit_;}
      /** Accessor to get the size of the arrays that describe the behavior above the tropopause    */
      int getArraySize(){return v_hx_.size();}
      /** Accessor to get one element of the altitude array associted to T/P above the tropopause   */
      double getHx(int n){return v_hx_[n];}
      /** Accessor to get one element of the pressure array that describes the behavior above the tropopause */
      double getPx(int n){return v_px_[n];}
      /** Accessor to get one element of the temperature array that describes the behavior above the tropopause */
      double getTx(int n){return v_tx_[n];}
      //@} 
      
    protected:
      /** Default constructor */
      AtmType();
      
      Atmospheretype type_;
      double         prLimit_;
      vector<double> v_hx_;
      vector<double> v_px_;
      vector<double> v_tx_;
      
    private:
      
      static Atmospheretype type_default_;
    };
  
  // Atmospheretype AtmType::type_default_(tropical);
  
}

#define ATM_TYPE_H
#endif  
