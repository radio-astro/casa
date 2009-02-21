#if     !defined(ATM_MASSDENSITY_H)

#include <string>

namespace atm {
  /*! \brief Mass Density value with units
   *
   *   This class defines physical parameters having units of Mass Density.
   *   For example, the profiles of minor gases, such as O3, are in mass density.
   *   Default is m**-3 (International System).
   */
  class MassDensity
    {
    public:
      /** Default constructor */
      MassDensity(); 
      /** A full constructor: Mass Density value assumed by default to be in kgm**-3 (International System) */
      MassDensity(double massdensity);
      /** A full constructor: Mass Density value + unit. 
       *  Valid units are kgm**-3 [kg m**-3, KGM**-3, KG M**-3], gcm**-3 [g cm**-3, GCM**-3, G CM**-3].  
       *  If none of these implented units is given, the SI value will be returned. */
      MassDensity(double massdensity, string units);
      ~MassDensity();
      
      //@{ 
      /** Accessor to the massdensity value in International System units (kgm**-3) */
      double get()const;
      /** Accessor to the massdensity value in specified units. 
       *  Valid units are kgm**-3 [kg m**-3, KGM**-3, KG M**-3], gcm**-3 [g cm**-3, GCM**-3, G CM**-3].  
       */
      double get(string units)const;
      //@} 
      
      MassDensity& operator=  (const MassDensity &);
      MassDensity& operator=  (const double &);
      MassDensity  operator+  (const MassDensity &);
      MassDensity  operator-  (const MassDensity &);
      MassDensity  operator*  (const double &);
      MassDensity  operator*  (const float  &);
      MassDensity  operator*  (const int    &); // rhs scale factor 
      MassDensity  operator/  (const double &);
      MassDensity  operator/  (const float  &);
      MassDensity  operator/  (const int    &); // rhs scale factor 
      
      bool    operator<  (const MassDensity & rhs);
      bool    operator>  (const MassDensity & rhs);
      bool    operator<= (const MassDensity & rhs);
      bool    operator>= (const MassDensity & rhs);
      bool    operator== (const MassDensity & rhs);
      bool    operator!= (const MassDensity & rhs);
      
      
    private:
      double valueIS_;
    };
}

#define ATM_MASSDENSITY_H

#endif
