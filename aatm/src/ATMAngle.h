#if     !defined(ATM_ANGLE_H)


namespace atm {
  
  
#include <string>
  /*! \brief Defines a Class for those parameters being angles.
   *
   *   Defines a Class for those physical parameters having units of angle. For 
   *   example the Total Phase Delay can be expressed as an angle (but also as a length). 
   */
  class Angle
  {
  public:
      /** Default constructor */
      Angle();
      /** A full constructor: Angle value in default units (SI: radians) */
      Angle(double angle);
      /** A full constructor: Angle value + units. Implemented units are: deg [DEG], rad [RAD] [Rad]*/
      Angle(double angle, string units);
      
      ~Angle();
      
      /** Accessor to get the angle value in SI units (rad) */
      double get()const;
      /** Accessor to the angle value in specified units. Implemented units are: deg [DEG], rad [RAD] [Rad]. 
       *  If none of these implented units is given, the SI value will be returned. */
      double get(string units)const;
      
      Angle& operator=  (const Angle &);
      Angle& operator=  (const double &);
      Angle  operator+  (const Angle &);
      Angle  operator-  (const Angle &);
      Angle  operator*  (const double &);
      Angle  operator*  (const float  &);
      Angle  operator*  (const int    &); // rhs scale factor 
      Angle  operator/  (const double &);
      Angle  operator/  (const float  &);
      Angle  operator/  (const int    &); // rhs scale factor 
      
      bool    operator<  (const Angle & rhs);
      bool    operator>  (const Angle & rhs);
      bool    operator<= (const Angle & rhs);
      bool    operator>= (const Angle & rhs);
      bool    operator== (const Angle & rhs);
      bool    operator!= (const Angle & rhs);
      
    private:
      double valueIS_;
    };
  
}

#define ATM_ANGLE_H

#endif
