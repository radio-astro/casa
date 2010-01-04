using namespace std;
#include <string>
#include "ATMPercent.h"

namespace atm {
  
  // Constructors
  
  Percent::Percent(){valueIS_=0.0;}
  Percent::Percent(double percent){valueIS_=percent;}
  Percent::Percent(double percent, string units){
    if(units=="%"){
      valueIS_=percent/100.0; 
    }else{
      valueIS_=percent; 
    }
  }
  
  Percent::~Percent(){}
  
  
  // Accessors
  double Percent::get()const{return valueIS_;}
  double Percent::get(string units)const{  
    if(units=="%"){
      return valueIS_*100.0;
    }else{
      return valueIS_;
    }
  }
  
  Percent& Percent::operator= (const Percent & rhs){valueIS_=rhs.valueIS_; return *this;}
  Percent& Percent::operator= (const double & rhs){valueIS_=rhs;           return *this;}
  
  Percent  Percent::operator+ (const Percent & rhs){return Percent(valueIS_+rhs.get());}
  Percent  Percent::operator- (const Percent & rhs){return Percent(valueIS_-rhs.get());}
  Percent  Percent::operator* (const double & scf){return Percent(valueIS_*scf);}
  Percent  Percent::operator* (const float  & scf){return Percent(valueIS_*(double)scf);}
  Percent  Percent::operator* (const int    & scf){return Percent(valueIS_*(double)scf);}
  Percent  Percent::operator/ (const double & scf){return Percent(valueIS_/scf);}
  Percent  Percent::operator/ (const float  & scf){return Percent(valueIS_/(double)scf);}
  Percent  Percent::operator/ (const int    & scf){return Percent(valueIS_/(double)scf);}
  
  
  bool    Percent::operator< (const Percent & rhs){return (valueIS_<rhs.get());}
  bool    Percent::operator> (const Percent & rhs){return (valueIS_>rhs.get());}
  bool    Percent::operator<=(const Percent & rhs){return (valueIS_<=rhs.get());}
  bool    Percent::operator>=(const Percent & rhs){return (valueIS_>=rhs.get());}
  bool    Percent::operator==(const Percent & rhs){return (valueIS_==rhs.get());}
  bool    Percent::operator!=(const Percent & rhs){return (valueIS_!=rhs.get());}
  
}
