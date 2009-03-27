using namespace std;
#include <string>
#include "ATMFrequency.h"

namespace atm {

  // Constructors
  Frequency::Frequency(){valueIS_=0.0;}
  Frequency::Frequency(double freq){valueIS_=freq;}
  Frequency::Frequency(double freq, string units){valueIS_ = sput(freq,units);}
  Frequency::Frequency(const Frequency & a){valueIS_ = a.valueIS_;}
  Frequency::~Frequency(){}
    
  // Methodes de conversions

    double Frequency::sget(double value, string units){  
      if(units=="THz"||units=="THZ"){
	return 1.0E-12*value;
      }else if(units=="GHz"||units=="GHz"||units=="ghz"){
	return 1.0E-9*value;
      }else if(units=="MHz"||units=="MHZ"||units=="mhz"){
	return 1.0E-6*value;
      }else if(units=="kHz"||units=="KHZ"||units=="khz"){
	return 1.0E-3*value;
      }else if(units=="Hz"||units=="HZ"||units=="hz"){
	return value;
      }else{
	return value;
      }
    } 
  
   double Frequency::sput(double freq, string units){
     if(units=="THz"||units=="THZ"){
       return 1.0E12*freq;
     }else if(units=="GHz"||units=="GHZ"||units=="ghz"){
       return 1.0E9*freq;
     }else if(units=="MHz"||units=="MHZ"||units=="mhz"){
       return 1.0E6*freq;
     }else if(units=="kHz"||units=="KHZ"||units=="khz"){
       return 1.0E3*freq;
     }else if(units=="Hz"||units=="HZ"||units=="hz"){
       return freq;
     }else{
       return freq;
     }
   } 

  // Accessors


  string Frequency::get(string form, string units)const{
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


  double Frequency::get(string units)const{
    return sget( valueIS_, units);
  }

  // Operators

  Frequency& Frequency::operator= (const Frequency & rhs){valueIS_=rhs.valueIS_; return *this;}
  Frequency& Frequency::operator= (const double & rhs){valueIS_=rhs;             return *this;}
  
  Frequency  Frequency::operator+ (const Frequency & rhs){return Frequency(valueIS_+rhs.get());}
  Frequency  Frequency::operator- (const Frequency & rhs){return Frequency(valueIS_-rhs.get());}
  Frequency  Frequency::operator* (const double & scf){return Frequency(valueIS_*scf);}
  Frequency  Frequency::operator* (const float  & scf){return Frequency(valueIS_*(double)scf);}
  Frequency  Frequency::operator* (const int    & scf){return Frequency(valueIS_*(double)scf);}
  Frequency  Frequency::operator/ (const double & scf){return Frequency(valueIS_/scf);}
  Frequency  Frequency::operator/ (const float  & scf){return Frequency(valueIS_/(double)scf);}
  Frequency  Frequency::operator/ (const int    & scf){return Frequency(valueIS_/(double)scf);}
  Frequency  Frequency::operator/ (const unsigned int    & scf){return Frequency(valueIS_/(double)scf);}
  
  bool    Frequency::operator< (const Frequency & rhs)const {return (valueIS_<rhs.get());}
  bool    Frequency::operator> (const Frequency & rhs)const {return (valueIS_>rhs.get());}
  bool    Frequency::operator<=(const Frequency & rhs)const {return (valueIS_<=rhs.get());}
  bool    Frequency::operator>=(const Frequency & rhs)const {return (valueIS_>=rhs.get());}
  bool    Frequency::operator==(const Frequency & rhs)const {return (valueIS_==rhs.get());}
  bool    Frequency::operator!=(const Frequency & rhs)const {return (valueIS_!=rhs.get());}
  
}


