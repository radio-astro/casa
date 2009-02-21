using namespace std;
#include <string>
#include <atmosphere/ATM/ATMPressure.h>

namespace atm {
  
  // Constructors
  
  Pressure::Pressure(){valueIS_=0.0;}
  Pressure::Pressure(double pressure){valueIS_=pressure;}
  
  Pressure::Pressure(double pressure, string units){
    if(units=="Pa"||units=="PA"){
      valueIS_ = pressure;
    }else if(units=="hPa"||units=="HPA"){
      valueIS_ = pressure*100.0;
    }else if(units=="bar"||units=="BAR"){
      valueIS_ = 1.0E+5*pressure;
    }else if(units=="mb"||units=="MB"){
      valueIS_ = 1.0E+2*pressure;
    }else if(units=="mbar"||units=="NBAR"){
      valueIS_ = 1.0E+2*pressure;
    }else if(units=="atm"||units=="ATM"){
      valueIS_ = pressure*101325.;
    }else{
      valueIS_ = pressure;
    }
    
  }
  
  Pressure& Pressure::operator=  (const Pressure & rhs){valueIS_=rhs.valueIS_; return *this;}
  Pressure& Pressure::operator=  (const double   & rhs){valueIS_=rhs;          return *this;}
  
  Pressure Pressure::operator+  (const Pressure & rhs){return Pressure(valueIS_+rhs.get());}
  Pressure Pressure::operator-  (const Pressure & rhs){return Pressure(valueIS_-rhs.get());}
  Pressure Pressure::operator*  (const double   & scf){return Pressure(valueIS_*scf);}
  Pressure Pressure::operator*  (const float    & scf){return Pressure(valueIS_*(double)scf);}
  Pressure Pressure::operator*  (const int      & scf){return Pressure(valueIS_*(double)scf);}
  Pressure Pressure::operator/  (const double   & scf){return Pressure(valueIS_/scf);}
  Pressure Pressure::operator/  (const float    & scf){return Pressure(valueIS_/(double)scf);}
  Pressure Pressure::operator/  (const int      & scf){return Pressure(valueIS_/(double)scf);}
  //Pressure& Pressure::operator/  (const double   & scf){valueIS_=valueIS_/scf;           return *this;}
  //Pressure& Pressure::operator/  (const int      & scf){valueIS_=valueIS_/(double)scf;   return *this;}
  
  
  bool      Pressure::operator<  (const Pressure & rhs){return (valueIS_<rhs.get());}
  bool      Pressure::operator>  (const Pressure & rhs){return (valueIS_>rhs.get());}
  bool      Pressure::operator<= (const Pressure & rhs){return (valueIS_<=rhs.get());}
  bool      Pressure::operator>= (const Pressure & rhs){return (valueIS_>=rhs.get());}
  bool      Pressure::operator== (const Pressure & rhs){return (valueIS_==rhs.get());}
  bool      Pressure::operator!= (const Pressure & rhs){return (valueIS_!=rhs.get());}
  
  Pressure::~Pressure(){}
  
  // Accessors
  double Pressure::get()const{return valueIS_;}
  double Pressure::get(string units)const{  
    if(units=="Pa"||units=="PA"){
      return valueIS_;
    }else if(units=="hPa"||units=="HPA"||units=="hpa"){
      return 1.0E-2*valueIS_;
    }else if(units=="bar"||units=="BAR"){
      return 1.0E-5*valueIS_;
    }else if(units=="mb"||units=="MB"){
      return 1.0E-2*valueIS_;
    }else if(units=="mbar"||units=="MBAR"){
      return 1.0E-2*valueIS_;
    }else if(units=="atm"||units=="ATM"){
      return valueIS_/101325.;
    }else{
      return valueIS_;
    }
    
  }
  
}
