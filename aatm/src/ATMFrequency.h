#if     !defined(ATM_FREQUENCY_H)

#include <string>

using namespace std;

namespace atm {


/*! \brief Defines a frequency value with units
 *
 *   Objects of type Frequency has a value in Hz (International Systen Units). The 
 *   class Frequency allows clients to instantiate or retrieve Frequency objects with
 *   values specified in any units within a restricted set of predefined units, namely 
 *   in THz, GHz, MHz, kHz or Hz. Hence units are handled only at the interface.
 *   If the units is not specified at this interface it is assumed to be Hz.
 */

  class Frequency
    {
    public:
      /** Default constructor: Frequency value set to 0 Hz */
      Frequency();
      /** A full constructor: Frequency value + units. Valid units are THz [THZ] [thz], GHz [GHZ] [ghz], MHz [MHZ] [mhz], Hz [HZ] [hz] */
      Frequency(double frequency, string units);  
      /** A full constructor: Frequency value in default units (International System: Hz) */
      Frequency(double frequency);
      /** Copy constructor */
      Frequency (const Frequency &);

      ~Frequency();
      
      //@{ 
      /** Accessor to get the numerical value of frequency (in International System units: Hz) */
      double get()const{return valueIS_;}
      /** Accessor to the frequency value in specified units. Implemented units are THz [THZ] [thz], GHz [GHZ] [ghz], MHz [MHZ] [mhz], Hz [HZ] [hz].
       *  If none of these implented units is given, the SI value will be returned. */
      double get(string units)const;
      /** Accessor to the frequency in specified units as a formatted string.
          Implemented units are THz [THZ] [thz], GHz [GHZ] [ghz], MHz [MHZ] [mhz], Hz [HZ] [hz].
          If none of these implented units is given, the SI value will be returned. */
      string get(string form, string units)const;
      //@} 

      /** Operator "equal to a Frequency" in the class Frequency */
      Frequency& operator=(const Frequency &);
      /** Operator "equal to a double converted to Frequency in Hz" in the class Frequency */
      Frequency& operator=(const double &);
      /** Operator "addition of frequencies" in the class Frequency */
      Frequency  operator+(const Frequency &);
      /** Operator "substraction of frequencies" in the class Frequency */
      Frequency  operator-(const Frequency &);
      /** Operator "multiplication of a frequency by a double" in the class Frequency */
      Frequency  operator*(const double &);
      /** Operator "multiplication of a frequency by a float" in the class Frequency */
      Frequency  operator*(const float  &);
      /** Operator "multiplication of a frequency by an int" in the class Frequency */
      Frequency  operator*(const int    &); // rhs scale factor 
      /** Operator "division of a frequency by a double" in the class Frequency */
      Frequency  operator/(const double &);
      /** Operator "division of a frequency by a float" in the class Frequency */
      Frequency  operator/(const float  &);
      /** Operator "division of a frequency by an int" in the class Frequency */
      Frequency  operator/(const int    &); // rhs scale factor 
      /** Operator "division of a frequency by an unsigned int" in the class Frequency */
      Frequency  operator/(const unsigned int    &); // rhs scale factor 
    
      /** Operator "comparator < for two frequencies" in the class Frequency */
      bool    operator<  (const Frequency & rhs)const;
      /** Operator "comparator > for two frequencies" in the class Frequency */
      bool    operator>  (const Frequency & rhs)const;
      /** Operator "comparator <= for two frequencies" in the class Frequency */
      bool    operator<= (const Frequency & rhs)const;
      /** Operator "comparator >= for two frequencies" in the class Frequency */
      bool    operator>= (const Frequency & rhs)const;
      /** Operator "comparator == for two frequencies" in the class Frequency */
      bool    operator== (const Frequency & rhs)const;
      /** Operator "comparator != for two frequencies" in the class Frequency */
      bool    operator!= (const Frequency & rhs)const;
      
    private:
      double valueIS_;
      static double sget(double value, string units);  
      static double sput(double value, string units);  
    };
}

#define ATM_FREQUENCY_H

#endif
