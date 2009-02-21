#if     !defined(ATM_OPACITY_H)

#include <string>

using namespace std;

namespace atm {
  /*! \brief Class for opacities [no dimensions]
   *
   *   This class is defined for opacities. 
   *   Default units are np (Transmission=exp(-opacity)).
   */
  class Opacity
    {
    public:
      /** Default constructor: Opacity value set to 0 np */
      Opacity();
      /** A full constructor: value in default units (np) */
      Opacity(double opacity);
      /** A full constructor: value + units. Valid units are:  np [neper] [NP] [NEPER], db [DB]. */
      Opacity(double opacity, string units);
      /** Copy constructor */
      Opacity (const Opacity &);
      
      ~Opacity();
      

      //@{ 
      /** Accessor to get the numerical value of opacity (in np) */
      double get()const{return valueIS_;}
      /** Accessor to the opacity value in specified units. Implemented units are np [neper] [NP] [NEPER], db [DB].
       *  If none of these implented units is given, the value in neper will be returned. */
      double get(string units)const;
      /** Accessor to the opacity in specified units as a formatted string.
          Implemented units are np [neper] [NP] [NEPER], db [DB].
          If none of these implented units is given, the value in neper will be returned. */
      string get(string form, string units)const;
      //@} 

      
      /** Operator "equal to a Opacity" in the class Opacity */
      Opacity& operator=  (const Opacity &);
      /** Operator "equal to a double converted to Opacity in m" in the class Opacity */
      Opacity& operator=  (const double &);
      /** Operator "addition of opacities" in the class Opacity */
      Opacity  operator+  (const Opacity &);
      /** Operator "substraction of opacities" in the class Opacity */
      Opacity  operator-  (const Opacity &);
      /** Operator "multiplication of an opacity by a double" in the class Opacity */
      Opacity  operator*  (const double &);
      /** Operator "multiplication of an opacity by a float" in the class Opacity */
      Opacity  operator*  (const float  &);
      /** Operator "multiplication of an opacity by an int" in the class Opacity */
      Opacity  operator*  (const int    &); // rhs scale factor 
      /** Operator "division of a opacity by an int" in the class Opacity */
      Opacity  operator/  (const double &);
      /** Operator "division of a opacity by a float" in the class Opacity */
      Opacity  operator/  (const float  &);
      /** Operator "division of a opacity by an int" in the class Opacity */
      Opacity  operator/  (const int    &); // rhs scale factor 
      /** Operator "division of a opacity by an unsigned int" in the class Opacity */
      Opacity  operator/  (const unsigned int    &); // rhs scale factor 
      
      /** Operator "comparator < for two opacities" in the class Opacity */
      bool    operator<  (const Opacity & rhs)const;
      /** Operator "comparator > for two opacities" in the class Opacity */
      bool    operator>  (const Opacity & rhs)const;
      /** Operator "comparator <= for two opacities" in the class Opacity */
      bool    operator<= (const Opacity & rhs)const;
      /** Operator "comparator >= for two opacities" in the class Opacity */
      bool    operator>= (const Opacity & rhs)const;
      /** Operator "comparator == for two opacities" in the class Opacity */
      bool    operator== (const Opacity & rhs)const;
      /** Operator "comparator != for two opacities" in the class Opacity */
      bool    operator!= (const Opacity & rhs)const;
      
    private:
      double valueIS_;
      static double sget(double value, string units);  
      static double sput(double value, string units);  
    };
  
}


#define ATM_OPACITY_H

#endif
