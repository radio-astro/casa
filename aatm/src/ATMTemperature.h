#if     !defined(ATM_TEMPERATURE_H)


#include <string>

namespace atm {
  /*! \brief Temperature value with units
   *
   *   This class defines physical parameters having units of Temperature.
   *   For example, the Radiance can be converted into Brightness Temperature.   
   *   Default is K (International System), also used by the ATM library. 
   */
    
  class Temperature
    {
    public:
      /** Default constructor */
      Temperature(); 
      /** A full constructor: Temperature value assumed by default to be in K (International System) */
      Temperature(double temperature);
      /** A full constructor: Temperature value + unit. Valid units are K [k], mK [mk], and C [c].  
       *  If none of these implented units is given, the SI value will be returned. */
      Temperature(double temperature, string units);
      ~Temperature();
      
      //@{ 
      /** Accessor to the temperature value in International System units (K) */
      double get()const;
      /** Accessor to the temperature value in specified units. Valid units are K [k], mK [mk], and C [c] */
      double get(string units)const;
      //@} 
      
      Temperature&  operator=(const Temperature &);
      Temperature&  operator=(const double &);
      Temperature   operator+(const Temperature &);
      Temperature   operator-(const Temperature &);
      Temperature   operator*(const double   &); // rhs scale factor 
      Temperature   operator*(const float    &); // rhs scale factor 
      Temperature   operator*(const int      &); // rhs scale factor
      Temperature   operator/(const double   &); // rhs scale factor 
      Temperature   operator/(const float    &); // rhs scale factor 
      Temperature   operator/(const int      &); // rhs scale factor
      
      
      bool      operator<  (const Temperature & rhs)const;
      bool      operator>  (const Temperature & rhs)const;
      bool      operator<= (const Temperature & rhs)const;
      bool      operator>= (const Temperature & rhs)const;
      bool      operator== (const Temperature & rhs)const;
      bool      operator!= (const Temperature & rhs)const;
      
    private:
      double valueIS_;
    };
  
}

#define ATM_TEMPERATURE_H

#endif
