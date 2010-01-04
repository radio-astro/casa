using namespace std;
#include <string>
#include "ATMMassDensity.h"

namespace atm {
  
  // Constructors
  
  MassDensity::MassDensity():valueIS_(0.0){}
  MassDensity::MassDensity(double massdensity):valueIS_(massdensity){}
  MassDensity::MassDensity(double massdensity, string units){
    if(units=="gcm**-3"||units=="g cm**-3"||units=="GCM**-3"||units=="G CM**-3"){
      valueIS_=1.0E+3*massdensity;
    }else if(units=="gm**-3"||units=="g m**-3"||units=="GM**-3"||units=="G M**-3"){
      valueIS_=1.0E-3*massdensity;
    }else if(units=="kgm**-3"||units=="kg m**-3"||units=="KGM**-3"||units=="KG M**-3"){
      valueIS_=massdensity;
    }else{
      valueIS_=massdensity;
    }
  }
  
  MassDensity::~MassDensity(){}
  
  // Accessors
  double MassDensity::get()const{return valueIS_;}
  double MassDensity::get(string units)const{  
    if(units=="gcm**-3"||units=="g cm**-3"||units=="GCM**-3"||units=="G CM**-3"){
      return 1.0E-3*valueIS_;
    }else if(units=="gm**-3"||units=="g m**-3"||units=="GM**-3"||units=="G M**-3"){
      return 1.0E+3*valueIS_;
    }else if(units=="kgm**-3"||units=="kg m**-3"||units=="KGM**-3"||units=="KG M**-3"){
      return valueIS_;
    }else{
      return valueIS_;
    }
  }
  
  MassDensity& MassDensity::operator= (const MassDensity & rhs){valueIS_=rhs.valueIS_; return *this;}
  MassDensity& MassDensity::operator= (const double & rhs){valueIS_=rhs;               return *this;}
  
  MassDensity  MassDensity::operator+ (const MassDensity & rhs){return MassDensity(valueIS_+rhs.get());}
  MassDensity  MassDensity::operator- (const MassDensity & rhs){return MassDensity(valueIS_-rhs.get());}
  MassDensity  MassDensity::operator* (const double & scf){return MassDensity(valueIS_*scf);}
  MassDensity  MassDensity::operator* (const float  & scf){return MassDensity(valueIS_*(double)scf);}
  MassDensity  MassDensity::operator* (const int    & scf){return MassDensity(valueIS_*(double)scf);}
  MassDensity  MassDensity::operator/ (const double & scf){return MassDensity(valueIS_/scf);}
  MassDensity  MassDensity::operator/ (const float  & scf){return MassDensity(valueIS_/(double)scf);}
  MassDensity  MassDensity::operator/ (const int    & scf){return MassDensity(valueIS_/(double)scf);}
  
  
  bool    MassDensity::operator< (const MassDensity & rhs){return (valueIS_<rhs.get());}
  bool    MassDensity::operator> (const MassDensity & rhs){return (valueIS_>rhs.get());}
  bool    MassDensity::operator<=(const MassDensity & rhs){return (valueIS_<=rhs.get());}
  bool    MassDensity::operator>=(const MassDensity & rhs){return (valueIS_>=rhs.get());}
  bool    MassDensity::operator==(const MassDensity & rhs){return (valueIS_==rhs.get());}
  bool    MassDensity::operator!=(const MassDensity & rhs){return (valueIS_!=rhs.get());}
  
}
