using namespace std;
#include <string>
#include "ATMHumidity.h"

namespace atm {
  
  // Constructors
  
  Humidity::Humidity(){valueIS_=0.0;}
  Humidity::Humidity(double humidity){valueIS_=humidity;}
  Humidity::Humidity(double humidity, string units){
    if(units=="%"){
      valueIS_=humidity; 
    }else{
      valueIS_=humidity; 
    }
  }
  
  Humidity::~Humidity(){}
  
  
  // Accessors
  double Humidity::get()const{return valueIS_;}
  double Humidity::get(string units)const{  
    if(units=="%"){
      return valueIS_;
    }else{
      return valueIS_;
    }
  }
  
  Humidity& Humidity::operator=  (const Humidity & rhs){valueIS_=rhs.valueIS_; return *this;}
  Humidity& Humidity::operator=  (const double   & rhs){valueIS_=rhs;          return *this;}
  
  Humidity Humidity::operator+  (const Humidity & rhs){return Humidity(valueIS_+rhs.get());}
  Humidity Humidity::operator-  (const Humidity & rhs){return Humidity(valueIS_-rhs.get());}
  Humidity Humidity::operator*  (const double   & scf){return Humidity(valueIS_*scf);}
  Humidity Humidity::operator*  (const float    & scf){return Humidity(valueIS_*(double)scf);}
  Humidity Humidity::operator*  (const int      & scf){return Humidity(valueIS_*(double)scf);}
  Humidity Humidity::operator/  (const double   & scf){return Humidity(valueIS_/scf);}
  Humidity Humidity::operator/  (const float    & scf){return Humidity(valueIS_/(double)scf);}
  Humidity Humidity::operator/  (const int      & scf){return Humidity(valueIS_/(double)scf);}
  
  
  bool      Humidity::operator<  (const Humidity & rhs){return (valueIS_<rhs.get());}
  bool      Humidity::operator>  (const Humidity & rhs){return (valueIS_>rhs.get());}
  bool      Humidity::operator<= (const Humidity & rhs){return (valueIS_<=rhs.get());}
  bool      Humidity::operator>= (const Humidity & rhs){return (valueIS_>=rhs.get());}
  bool      Humidity::operator== (const Humidity & rhs){return (valueIS_==rhs.get());}
  bool      Humidity::operator!= (const Humidity & rhs){return (valueIS_!=rhs.get());}
  
}
