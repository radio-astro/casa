using namespace std;
#include <string>
#include "ATMNumberDensity.h"

namespace atm {
  
  // Constructors
  NumberDensity::NumberDensity():valueIS_(0.0){}
  NumberDensity::NumberDensity(double numberdensity):valueIS_(numberdensity){}
  NumberDensity::NumberDensity(double numberdensity, string units){
    if(units=="cm**-3"||units=="CM**-3"){
      valueIS_=1.0E+6*numberdensity;
    }else if(units=="m**-3"||units=="M**-3"){
      valueIS_=numberdensity;
    }else{
      valueIS_=numberdensity;
    }
  }
  
  NumberDensity::~NumberDensity(){}
  
  // Accessors
  double NumberDensity::get()const{return valueIS_;}
  double NumberDensity::get(string units)const{  
    if(units=="cm**-3" || units=="CM**-3" ){
      return 1.0E-6*valueIS_;
    }else if(units=="m**-3" || units=="M**-3"){
      return valueIS_;
    }else{
      return valueIS_;
    }
  }
  
  NumberDensity& NumberDensity::operator= (const NumberDensity & rhs){valueIS_=rhs.valueIS_; return *this;}
  NumberDensity& NumberDensity::operator= (const double & rhs){valueIS_=rhs;          return *this;}
  
  NumberDensity  NumberDensity::operator+ (const NumberDensity & rhs){return NumberDensity(valueIS_+rhs.get());}
  NumberDensity  NumberDensity::operator- (const NumberDensity & rhs){return NumberDensity(valueIS_-rhs.get());}
  NumberDensity  NumberDensity::operator* (const double & scf){return NumberDensity(valueIS_*scf);}
  NumberDensity  NumberDensity::operator* (const float  & scf){return NumberDensity(valueIS_*(double)scf);}
  NumberDensity  NumberDensity::operator* (const int    & scf){return NumberDensity(valueIS_*(double)scf);}
  NumberDensity  NumberDensity::operator/ (const double & scf){return NumberDensity(valueIS_/scf);}
  NumberDensity  NumberDensity::operator/ (const float  & scf){return NumberDensity(valueIS_/(double)scf);}
  NumberDensity  NumberDensity::operator/ (const int    & scf){return NumberDensity(valueIS_/(double)scf);}
  
  
  bool    NumberDensity::operator< (const NumberDensity & rhs){return (valueIS_<rhs.get());}
  bool    NumberDensity::operator> (const NumberDensity & rhs){return (valueIS_>rhs.get());}
  bool    NumberDensity::operator<=(const NumberDensity & rhs){return (valueIS_<=rhs.get());}
  bool    NumberDensity::operator>=(const NumberDensity & rhs){return (valueIS_>=rhs.get());}
  bool    NumberDensity::operator==(const NumberDensity & rhs){return (valueIS_==rhs.get());}
  bool    NumberDensity::operator!=(const NumberDensity & rhs){return (valueIS_!=rhs.get());}
  
}
