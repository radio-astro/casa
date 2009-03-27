#if     !defined(ATM_INVERSELENGTH_H)

#include <string>

using namespace std;

namespace atm {
  /*! \brief Class for those physical parameters having dimensions of Inverse Length [L^-1]
   *
   *   This class is defined for those physical parameters that have units of Inverse Length, for example
   *   the absorption coefficients. Default units are m^-1 (International System).
   */
  class InverseLength
    {
    public:
      /** Default constructor: Length value set to 0 m^-1 */
      InverseLength();
      /** A full constructor: value in default units (m^-1) */
      InverseLength(double inverseLength);
      /** A full constructor: value + units. Valid units are:  km-1 [KM-1], m-1 [M-1], mm-1 [MM-1], micron-1 [MICRON-1], nm-1 [NM-1]. */
      InverseLength(double inverseLength, string units);
      /** Copy constructor */
      InverseLength (const InverseLength &);
      
      ~InverseLength();
      

      //@{ 
      /** Accessor to get the numerical value of inverse length (in International System units: m^-1) */
      double get()const{return valueIS_;}
      /** Accessor to the inverse length value in specified units. Implemented units are km-1 [KM-1], m-1 [M-1], mm-1 [MM-1], micron-1 [MICRON-1], nm-1 [NM-1].
       *  If none of these implented units is given, the SI value will be returned. */
      double get(string units)const;
      /** Accessor to the inverse length in specified units as a formatted string.
          Implemented units are km-1 [KM-1], m-1 [M-1], mm-1 [MM-1], micron-1 [MICRON-1], nm-1 [NM-1].
          If none of these implented units is given, the SI value will be returned. */
      string get(string form, string units)const;
      //@} 

      
      /** Operator "equal to a InverseLength" in the class InverseLength */
      InverseLength& operator=  (const InverseLength &);
      /** Operator "equal to a double converted to InverseLength in m-1" in the class InverseLength */
      InverseLength& operator=  (const double &);
      /** Operator "addition of inverse lengths" in the class InverseLength */
      InverseLength  operator+  (const InverseLength &);
      /** Operator "substraction of inverse lengths" in the class InverseLength */
      InverseLength  operator-  (const InverseLength &);
      /** Operator "multiplication of a inverse length by a double" in the class InverseLength */
      InverseLength  operator*  (const double &);
      /** Operator "multiplication of a inverse length by a float" in the class InverseLength */
      InverseLength  operator*  (const float  &);
      /** Operator "multiplication of a inverse length by an int" in the class InverseLength */
      InverseLength  operator*  (const int    &); // rhs scale factor 
      /** Operator "division of a inverse length by a double" in the class InverseLength */
      InverseLength  operator/  (const double &);
      /** Operator "division of a inverse length by a float" in the class InverseLength */
      InverseLength  operator/  (const float  &);
      /** Operator "division of a inverse length by an int" in the class InverseLength */
      InverseLength  operator/  (const int    &); // rhs scale factor 
      /** Operator "division of a inverse length by an unsigned int" in the class InverseLength */
      InverseLength  operator/  (const unsigned int    &); // rhs scale factor 
      
      /** Operator "comparator < for two inverse lengths" in the class InverseLength */
      bool    operator<  (const InverseLength & rhs)const;
      /** Operator "comparator > for two inverse lengths" in the class InverseLength */
      bool    operator>  (const InverseLength & rhs)const;
      /** Operator "comparator <= for two inverse lengths" in the class InverseLength */
      bool    operator<= (const InverseLength & rhs)const;
      /** Operator "comparator >= for two inverse lengths" in the class InverseLength */
      bool    operator>= (const InverseLength & rhs)const;
      /** Operator "comparator == for two inverse lengths" in the class InverseLength */
      bool    operator== (const InverseLength & rhs)const;
      /** Operator "comparator != for two inverse lengths" in the class InverseLength */
      bool    operator!= (const InverseLength & rhs)const;
      
    private:
      double valueIS_;
      static double sget(double value, string units);  
      static double sput(double value, string units);  
    };
  
}


#define ATM_INVERSELENGTH_H

#endif
