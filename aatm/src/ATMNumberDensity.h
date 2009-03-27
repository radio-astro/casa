#if     !defined(ATM_NUMBERDENSITY_H)

#include <string>

namespace atm {
  /*! \brief Number Density value with units
   *
   *   This class defines physical parameters having units of Number Density.
   *   For example, the profiles of minor gases, such as O3, are in number density.
   *   Default is m**-3 (International System).
   */
  class NumberDensity
    {
    public:
      /** Default constructor */
      NumberDensity(); 
      /** A full constructor: Number Density value assumed by default to be in m**-3 (International System) */
      NumberDensity(double numberdensity);
      /** A full constructor: Number Density value + unit. Valid units are m**-3 [M**-3], cm**-3 [CM**-3].  
       *  If none of these implented units is given, the SI value will be returned. */
      NumberDensity(double numberdensity, string units);
      ~NumberDensity();
      
      //@{ 
      /** Accessor to the numberdensity value in International System units (K) */
      double get()const;
      /** Accessor to the numberdensity value in specified units. Valid units are K [k], mK [mk], and C [c] */
      double get(string units)const;
      //@} 
      
      NumberDensity& operator=  (const NumberDensity &);
      NumberDensity& operator=  (const double &);
      NumberDensity  operator+  (const NumberDensity &);
      NumberDensity  operator-  (const NumberDensity &);
      NumberDensity  operator*  (const double &);
      NumberDensity  operator*  (const float  &);
      NumberDensity  operator*  (const int    &); // rhs scale factor 
      NumberDensity  operator/  (const double &);
      NumberDensity  operator/  (const float  &);
      NumberDensity  operator/  (const int    &); // rhs scale factor 
      
      bool    operator<  (const NumberDensity & rhs);
      bool    operator>  (const NumberDensity & rhs);
      bool    operator<= (const NumberDensity & rhs);
      bool    operator>= (const NumberDensity & rhs);
      bool    operator== (const NumberDensity & rhs);
      bool    operator!= (const NumberDensity & rhs);
      
    private:
      double valueIS_;
    };
}

#define ATM_NUMBERDENSITY_H

#endif
