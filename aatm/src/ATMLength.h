#if     !defined(ATM_LENGTH_H)

#include <string>

using namespace std;

namespace atm {
  /*! \brief Class for those physical parameters having dimensions of Length [L]
   *
   *   This class is defined for those physical parameters that have units of Length, for example
   *   the water vapor scale height. Default units are m (International System).
   */
  class Length
    {
    public:
      /** Default constructor: Length value set to 0 m */
      Length();
      /** A full constructor: value in default units (m) */
      Length(double length);
      /** A full constructor: value + units. Valid units are:  km [KM], m [M], mm [MM], micron [MICRON], nm [NM]. */
      Length(double length, string units);
      /** Copy constructor */
      Length (const Length &);
      
      ~Length();
      

      //@{ 
      /** Accessor to get the numerical value of length (in International System units: m) */
      double get()const{return valueIS_;}
      /** Accessor to the length value in specified units. Implemented units are km [KM], m [M], mm [MM], micron [MICRON], nm [NM].
       *  If none of these implented units is given, the SI value will be returned. */
      double get(string units)const;
      /** Accessor to the length in specified units as a formatted string.
          Implemented units are km [KM], m [M], mm [MM], micron [MICRON], nm [NM].
          If none of these implented units is given, the SI value will be returned. */
      string get(string form, string units)const;
      //@} 

      
      /** Operator "equal to a Length" in the class Length */
      Length& operator=  (const Length &);
      /** Operator "equal to a double converted to Length in m" in the class Length */
      Length& operator=  (const double &);
      /** Operator "addition of lengths" in the class Length */
      Length  operator+  (const Length &);
      /** Operator "substraction of lengths" in the class Length */
      Length  operator-  (const Length &);
      /** Operator "multiplication of a length by a double" in the class Length */
      Length  operator*  (const double &);
      /** Operator "multiplication of a length by a float" in the class Length */
      Length  operator*  (const float  &);
      /** Operator "multiplication of a length by an int" in the class Length */
      Length  operator*  (const int    &); // rhs scale factor 
      /** Operator "division of a length by a double" in the class Length */
      Length  operator/  (const double &);
      /** Operator "division of a length by a float" in the class Length */
      Length  operator/  (const float  &);
      /** Operator "division of a length by an int" in the class Length */
      Length  operator/  (const int    &); // rhs scale factor 
      /** Operator "division of a length by an unsigned int" in the class Length */
      Length  operator/  (const unsigned int    &); // rhs scale factor 
      
      /** Operator "comparator < for two lengths" in the class Length */
      bool    operator<  (const Length & rhs)const;
      /** Operator "comparator > for two lengths" in the class Length */
      bool    operator>  (const Length & rhs)const;
      /** Operator "comparator <= for two lengths" in the class Length */
      bool    operator<= (const Length & rhs)const;
      /** Operator "comparator >= for two lengths" in the class Length */
      bool    operator>= (const Length & rhs)const;
      /** Operator "comparator == for two lengths" in the class Length */
      bool    operator== (const Length & rhs)const;
      /** Operator "comparator != for two lengths" in the class Length */
      bool    operator!= (const Length & rhs)const;
      
    private:
      double valueIS_;
      static double sget(double value, string units);  
      static double sput(double value, string units);  
    };
  
}


#define ATM_LENGTH_H

#endif
