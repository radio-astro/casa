#if     !defined(ATM_PERCENT_H)

#include <string>

namespace atm {
  /*! \brief Defines a class for quantities that may be expressed in percent
   *
   *   This class is aimed at quantities that may be defined in percent. Default is proposed with no units 
   *   (relative to one), and an alternative is considered for units in %. 
   */
  class Percent
    {
    public:
      /** Default constructor */
      Percent();
      /** A full constructor: Amount in default units (relative to 1) */
      Percent(double percent);
      /** A full constructor: Amount + units. Valid units are: % */
      Percent(double percent, string units);
      
      ~Percent();
      
      /* Accessor to get the value relative to one */
      double get()const;
      /* Accessor to get the value in % (the only implemented alternative). */
      double get(string units)const;
      
      
      Percent& operator=  (const Percent &);
      Percent& operator=  (const double &);
      Percent  operator+  (const Percent &);
      Percent  operator-  (const Percent &);
      Percent  operator*  (const double &);
      Percent  operator*  (const float  &);
      Percent  operator*  (const int    &); // rhs scale factor 
      Percent  operator/  (const double &);
      Percent  operator/  (const float  &);
      Percent  operator/  (const int    &); // rhs scale factor 
      
      bool    operator<  (const Percent & rhs);
      bool    operator>  (const Percent & rhs);
      bool    operator<= (const Percent & rhs);
      bool    operator>= (const Percent & rhs);
      bool    operator== (const Percent & rhs);
      bool    operator!= (const Percent & rhs);
      
    private:
      double valueIS_;
    };
}

#define ATM_PERCENT_H

#endif
