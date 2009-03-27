#if     !defined(ATM_PRESSURE_H)

#include <string>

namespace atm {
  /*! \brief Defines pressure with units
   *
   *   This class defines physical parameters having units of Pressure. 
   *   Default is Pa (International System) but the ATM library uses mb (hPa). 
   */
  class Pressure
    {
    public:
      
      /** Default constructor */
      Pressure();
      /** A full constructor: Pressure value in default SI units (Pa) */
      Pressure(double pressure);
      /** A full constructor: Pressure value + units. Valid units are hPa [HPA] [hpa], bar [BAR], mb [MB], mbar [MBAR], atm [ATM]. */
      Pressure(double pressure, string units);
      
      ~Pressure();
      
      /** Accessor to get the value in SI units (Pa) */
      double get()const;
      /** Accessor to get the value in the following (implemented) units: hPa [HPA] [hpa], bar [BAR], mb [MB], mbar [MBAR], atm [ATM]. If none of these implented 
	  units is given, the SI value will be returned. */
      double get(string units)const;
      
      Pressure&  operator=(const Pressure &);
      Pressure&  operator=(const double &);
      Pressure   operator+(const Pressure &);
      Pressure   operator-(const Pressure &);
      Pressure   operator*(const double   &); // rhs scale factor 
      Pressure   operator*(const float    &); // rhs scale factor 
      Pressure   operator*(const int      &); // rhs scale factor
      Pressure   operator/(const double   &); // rhs scale factor 
      Pressure   operator/(const float    &); // rhs scale factor 
      Pressure   operator/(const int      &); // rhs scale factor
      
      
      bool      operator<  (const Pressure & rhs);
      bool      operator>  (const Pressure & rhs);
      bool      operator<= (const Pressure & rhs);
      bool      operator>= (const Pressure & rhs);
      bool      operator== (const Pressure & rhs);
      bool      operator!= (const Pressure & rhs);
      
      
    private:
      double valueIS_;
    };
}

#define ATM_PRESSURE_H

#endif
