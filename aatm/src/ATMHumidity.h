#if     !defined(ATM_HUMIDITY_H)

#include <string>

namespace atm {
  /*! \brief Defines humidity with units
   *
   *   This class defines the humidity with units. Default is proposed to be relative humidity (%), 
   *   but many different ways are currently used. International system units would be kg/m**3 but 
   *   the conversion to relative humidity needs the temperature. 
   */
  
  class Humidity
    {
    public:
      /** Default constructor */
      Humidity();
      /** A full constructor: Humidity in default units (proposed to be rel.hum. in %) */
      Humidity(double humidity);
      /** A full constructor: Humidity + units. Valig units are: % */
      Humidity(double humidity, string units);
      
      ~Humidity();
      
      /* Accessor to get the value of relative humidity in % */
      double get()const;
      /* Accessor to get the value of relative humidity in % (the only implemented unit). If none of these implented units is given, the % value will be returned. */
      double get(string units)const;
      
      
      Humidity&  operator=(const Humidity &);
      Humidity&  operator=(const double &);
      Humidity   operator+(const Humidity &);
      Humidity   operator-(const Humidity &);
      Humidity   operator*(const double   &); // rhs scale factor 
      Humidity   operator*(const float    &); // rhs scale factor 
      Humidity   operator*(const int      &); // rhs scale factor
      Humidity   operator/(const double   &); // rhs scale factor 
      Humidity   operator/(const float    &); // rhs scale factor 
      Humidity   operator/(const int      &); // rhs scale factor
      
      
      bool      operator<  (const Humidity & rhs);
      bool      operator>  (const Humidity & rhs);
      bool      operator<= (const Humidity & rhs);
      bool      operator>= (const Humidity & rhs);
      bool      operator== (const Humidity & rhs);
      bool      operator!= (const Humidity & rhs);
      
      
    private:
      double valueIS_;
    };
  
}

#define ATM_HUMIDITY_H

#endif
