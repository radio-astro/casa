using namespace std;
#include <string>
#include "ATMTemperature.h"

namespace atm {
  
  // Constructors
  
  Temperature::Temperature():valueIS_(0.0){}
  Temperature::Temperature(double temperature):valueIS_(temperature){}
  Temperature::Temperature(double temperature, string units){
    if(units=="mK"||units=="mk"){
      valueIS_=1.0E-3*temperature;
    }else if(units=="K"||units=="k"){
      valueIS_=temperature;
    }else if(units=="C"||units=="c"){
      valueIS_=temperature+273.16;
    }else{
      valueIS_=temperature;
    }
  }
  Temperature::~Temperature(){}
  
  // Accessors
  double Temperature::get()const{return valueIS_;}
  double Temperature::get(string units)const{  
    if(units=="mK"){
      return 1.0E3*valueIS_;
    }else if(units=="K"){
      return valueIS_;
    }else if(units=="C"||units=="c"){
      return valueIS_-273.16;
    }else{
      return valueIS_;
    }
  }
  
  
  Temperature& Temperature::operator=  (const Temperature & rhs){valueIS_=rhs.valueIS_; return *this;}
  Temperature& Temperature::operator=  (const double   & rhs){valueIS_=rhs;             return *this;}
  Temperature Temperature::operator+  (const Temperature & rhs){return Temperature(valueIS_+rhs.get());}
  Temperature Temperature::operator-  (const Temperature & rhs){return Temperature(valueIS_-rhs.get());}
  Temperature Temperature::operator*  (const double   & scf){return Temperature(valueIS_*scf);}
  Temperature Temperature::operator*  (const float    & scf){return Temperature(valueIS_*(double)scf);}
  Temperature Temperature::operator*  (const int      & scf){return Temperature(valueIS_*(double)scf);}
  Temperature Temperature::operator/  (const double   & scf){return Temperature(valueIS_/scf);}
  Temperature Temperature::operator/  (const float    & scf){return Temperature(valueIS_/(double)scf);}
  Temperature Temperature::operator/  (const int      & scf){return Temperature(valueIS_/(double)scf);}
  
  bool      Temperature::operator<  (const Temperature & rhs)const {return (valueIS_<rhs.get());}
  bool      Temperature::operator>  (const Temperature & rhs)const {return (valueIS_>rhs.get());}
  bool      Temperature::operator<= (const Temperature & rhs)const {return (valueIS_<=rhs.get());}
  bool      Temperature::operator>= (const Temperature & rhs)const {return (valueIS_>=rhs.get());}
  bool      Temperature::operator== (const Temperature & rhs)const {return (valueIS_==rhs.get());}
  bool      Temperature::operator!= (const Temperature & rhs)const {return (valueIS_!=rhs.get());}
  
}
