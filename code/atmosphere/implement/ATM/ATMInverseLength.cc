using namespace std;
#include <string>
#include <atmosphere/ATM/ATMInverseLength.h>

namespace atm {
  
  // Constructors
  
  InverseLength::InverseLength(){valueIS_=0.0;}
  InverseLength::InverseLength(double inverseLength){valueIS_=inverseLength;}
  InverseLength::InverseLength(double inverseLength, string units){valueIS_ = sput(inverseLength,units);}
  InverseLength::InverseLength(const InverseLength & a){valueIS_ = a.valueIS_;}
  InverseLength::~InverseLength(){}

  // Methodes de conversions
  double InverseLength::sget(double value, string units){
    if(units=="km-1"||units=="KM-"){
      return 1.0E+3*value;
    }else if(units=="m-1"||units=="M-1"){
      return value;
    }else if(units=="mm-1"||units=="MM-1"){
      return 1.0E-3*value;
    }else if(units=="micron-1"||units=="MICRON-1"){
      return 1.0E-6*value;
    }else if(units=="nm-1"||units=="NM-1"){
      return 1.0E-9*value;
    }else{
      return value;
    }
  }

  double InverseLength::sput(double value, string units){
    if(units=="km-1"||units=="KM-1"){
      return 1.0E-3*value;
    }else if(units=="m-1"||units=="M-1"){
      return value;
    }else if(units=="mm-1"||units=="MM-1"){
      return 1.0E+3*value;
    }else if(units=="micron-1"||units=="MICRON-1"){
      return 1.0E+6*value;
    }else if(units=="nm-1"||units=="NM-1"){
      return 1.0E+9*value;
    }else{
      return value;
    }
  }


  
  // Accessors

  string InverseLength::get(string form, string units)const{
    char myString[18];
    //   if(form.length()==0)
    sprintf( myString,"%f %s",
	     get(units),
	     units.c_str()); 
    //     else
    //       sprintf( myString,"%form %s",
    // 	       get(units),
    // 	       units);
    
    return string(myString);
  } 

  double InverseLength::get(string units)const{      
    return sget( valueIS_, units);
  }

  // Operators

  
  InverseLength& InverseLength::operator= (const InverseLength & rhs){valueIS_=rhs.valueIS_; return *this;}
  InverseLength& InverseLength::operator= (const double & rhs){valueIS_=rhs;          return *this;}
  
  InverseLength  InverseLength::operator+ (const InverseLength & rhs){return InverseLength(valueIS_+rhs.get());}
  InverseLength  InverseLength::operator- (const InverseLength & rhs){return InverseLength(valueIS_-rhs.get());}
  InverseLength  InverseLength::operator* (const double & scf){return InverseLength(valueIS_*scf);}
  InverseLength  InverseLength::operator* (const float  & scf){return InverseLength(valueIS_*(double)scf);}
  InverseLength  InverseLength::operator* (const int    & scf){return InverseLength(valueIS_*(double)scf);}
  InverseLength  InverseLength::operator/ (const double & scf){return InverseLength(valueIS_/scf);}
  InverseLength  InverseLength::operator/ (const float  & scf){return InverseLength(valueIS_/(double)scf);}
  InverseLength  InverseLength::operator/ (const int    & scf){return InverseLength(valueIS_/(double)scf);}
  InverseLength  InverseLength::operator/ (const unsigned int    & scf){return InverseLength(valueIS_/(double)scf);}
  
  
  bool    InverseLength::operator< (const InverseLength & rhs)const {return (valueIS_<rhs.get());}
  bool    InverseLength::operator> (const InverseLength & rhs)const {return (valueIS_>rhs.get());}
  bool    InverseLength::operator<=(const InverseLength & rhs)const {return (valueIS_<=rhs.get());}
  bool    InverseLength::operator>=(const InverseLength & rhs)const {return (valueIS_>=rhs.get());}
  bool    InverseLength::operator==(const InverseLength & rhs)const {return (valueIS_==rhs.get());}
  bool    InverseLength::operator!=(const InverseLength & rhs)const {return (valueIS_!=rhs.get());}
  
}

