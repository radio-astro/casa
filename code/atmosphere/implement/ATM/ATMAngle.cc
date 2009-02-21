using namespace std;
#include <string>
#include <atmosphere/ATM/ATMAngle.h>


namespace atm {
  
  // Constructors
  
  Angle::Angle(){valueIS_=0.0;}
  Angle::Angle(double angle){valueIS_=angle;}
  Angle::Angle(double angle, string units){
    if(units=="Rad"||units=="RAD"||units=="rad"){
      valueIS_ = angle;
    }else if(units=="deg"||units=="DEG"){
      valueIS_ = (angle/360.0)*6.2831852;
    }else{
      valueIS_ = angle;
    }
  }
  
  Angle::~Angle(){}
  
  // Accessors
  double Angle::get()const{return valueIS_;}
  double Angle::get(string units)const{  
    if(units=="Rad"||units=="RAD"||units=="rad"){
      return valueIS_;
    }else if(units=="deg"||units=="DEG"){
      return 360.0*(valueIS_/6.2831852);
    }else{
      return valueIS_;
    }
  }
  
  Angle& Angle::operator= (const Angle & rhs){valueIS_=rhs.valueIS_; return *this;}
  Angle& Angle::operator= (const double & rhs){valueIS_=rhs;          return *this;}
  
  Angle  Angle::operator+ (const Angle & rhs){return Angle(valueIS_+rhs.get());}
  Angle  Angle::operator- (const Angle & rhs){return Angle(valueIS_-rhs.get());}
  Angle  Angle::operator* (const double & scf){return Angle(valueIS_*scf);}
  Angle  Angle::operator* (const float  & scf){return Angle(valueIS_*(double)scf);}
  Angle  Angle::operator* (const int    & scf){return Angle(valueIS_*(double)scf);}
  Angle  Angle::operator/ (const double & scf){return Angle(valueIS_/scf);}
  Angle  Angle::operator/ (const float  & scf){return Angle(valueIS_/(double)scf);}
  Angle  Angle::operator/ (const int    & scf){return Angle(valueIS_/(double)scf);}
  
  
  bool    Angle::operator< (const Angle & rhs){return (valueIS_<rhs.get());}
  bool    Angle::operator> (const Angle & rhs){return (valueIS_>rhs.get());}
  bool    Angle::operator<=(const Angle & rhs){return (valueIS_<=rhs.get());}
  bool    Angle::operator>=(const Angle & rhs){return (valueIS_>=rhs.get());}
  bool    Angle::operator==(const Angle & rhs){return (valueIS_==rhs.get());}
  bool    Angle::operator!=(const Angle & rhs){return (valueIS_!=rhs.get());}
  
}

