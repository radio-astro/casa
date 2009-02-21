using namespace std;
#include <string>
#include <atmosphere/ATM/ATMOpacity.h>

namespace atm {
  
  // Constructors
  
  Opacity::Opacity(){valueIS_=0.0;}
  Opacity::Opacity(double opacity){valueIS_=opacity;}
  Opacity::Opacity(double opacity, string units){valueIS_ = sput(opacity,units);}
  Opacity::Opacity(const Opacity & a){valueIS_ = a.valueIS_;}
  Opacity::~Opacity(){}

  // Methodes de conversions
  double Opacity::sget(double value, string units){
    if(units=="db"||units=="DB"){
      return value*4.34294482;
    }else{
      return value;
    }
  }

  double Opacity::sput(double value, string units){
    if(units=="db"||units=="DB"){
      return value/4.34294482;
    }else{
      return value;
    }
  }


  
  // Accessors

  string Opacity::get(string form, string units)const{
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

  double Opacity::get(string units)const{      
    return sget( valueIS_, units);
  }

  // Operators

  
  Opacity& Opacity::operator= (const Opacity & rhs){valueIS_=rhs.valueIS_; return *this;}
  Opacity& Opacity::operator= (const double & rhs){valueIS_=rhs;          return *this;}
  
  Opacity  Opacity::operator+ (const Opacity & rhs){return Opacity(valueIS_+rhs.get());}
  Opacity  Opacity::operator- (const Opacity & rhs){return Opacity(valueIS_-rhs.get());}
  Opacity  Opacity::operator* (const double & scf){return Opacity(valueIS_*scf);}
  Opacity  Opacity::operator* (const float  & scf){return Opacity(valueIS_*(double)scf);}
  Opacity  Opacity::operator* (const int    & scf){return Opacity(valueIS_*(double)scf);}
  Opacity  Opacity::operator/ (const double & scf){return Opacity(valueIS_/scf);}
  Opacity  Opacity::operator/ (const float  & scf){return Opacity(valueIS_/(double)scf);}
  Opacity  Opacity::operator/ (const int    & scf){return Opacity(valueIS_/(double)scf);}
  Opacity  Opacity::operator/ (const unsigned int    & scf){return Opacity(valueIS_/(double)scf);}
  
  
  bool    Opacity::operator< (const Opacity & rhs)const {return (valueIS_<rhs.get());}
  bool    Opacity::operator> (const Opacity & rhs)const {return (valueIS_>rhs.get());}
  bool    Opacity::operator<=(const Opacity & rhs)const {return (valueIS_<=rhs.get());}
  bool    Opacity::operator>=(const Opacity & rhs)const {return (valueIS_>=rhs.get());}
  bool    Opacity::operator==(const Opacity & rhs)const {return (valueIS_==rhs.get());}
  bool    Opacity::operator!=(const Opacity & rhs)const {return (valueIS_!=rhs.get());}
  
}

