using namespace std;
#include <string>
#include <atmosphere/ATM/ATMLength.h>

namespace atm {
  
  // Constructors
  
  Length::Length(){valueIS_=0.0;}
  Length::Length(double length){valueIS_=length;}
  Length::Length(double length, string units){valueIS_ = sput(length,units);}
  Length::Length(const Length & a){valueIS_ = a.valueIS_;}
  Length::~Length(){}

  // Methodes de conversions
  double Length::sget(double value, string units){
    if(units=="km"||units=="KM"){
      return 1.0E-3*value;
    }else if(units=="m"||units=="M"){
      return value;
    }else if(units=="mm"||units=="MM"){
      return 1.0E+3*value;
    }else if(units=="micron"||units=="MICRON"){
      return 1.0E+6*value;
    }else if(units=="nm"||units=="NM"){
      return 1.0E+9*value;
    }else{
      return value;
    }
  }

  double Length::sput(double value, string units){
    if(units=="km"||units=="KM"){
      return 1.0E+3*value;
    }else if(units=="m"||units=="M"){
      return value;
    }else if(units=="mm"||units=="MM"){
      return 1.0E-3*value;
    }else if(units=="micron"||units=="MICRON"){
      return 1.0E-6*value;
    }else if(units=="nm"||units=="NM"){
      return 1.0E-9*value;
    }else{
      return value;
    }
  }


  
  // Accessors

  string Length::get(string form, string units)const{
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

  double Length::get(string units)const{      
    return sget( valueIS_, units);
  }

  // Operators

  
  Length& Length::operator= (const Length & rhs){valueIS_=rhs.valueIS_; return *this;}
  Length& Length::operator= (const double & rhs){valueIS_=rhs;          return *this;}
  
  Length  Length::operator+ (const Length & rhs){return Length(valueIS_+rhs.get());}
  Length  Length::operator- (const Length & rhs){return Length(valueIS_-rhs.get());}
  Length  Length::operator* (const double & scf){return Length(valueIS_*scf);}
  Length  Length::operator* (const float  & scf){return Length(valueIS_*(double)scf);}
  Length  Length::operator* (const int    & scf){return Length(valueIS_*(double)scf);}
  Length  Length::operator/ (const double & scf){return Length(valueIS_/scf);}
  Length  Length::operator/ (const float  & scf){return Length(valueIS_/(double)scf);}
  Length  Length::operator/ (const int    & scf){return Length(valueIS_/(double)scf);}
  Length  Length::operator/ (const unsigned int    & scf){return Length(valueIS_/(double)scf);}
  
  
  bool    Length::operator< (const Length & rhs)const {return (valueIS_<rhs.get());}
  bool    Length::operator> (const Length & rhs)const {return (valueIS_>rhs.get());}
  bool    Length::operator<=(const Length & rhs)const {return (valueIS_<=rhs.get());}
  bool    Length::operator>=(const Length & rhs)const {return (valueIS_>=rhs.get());}
  bool    Length::operator==(const Length & rhs)const {return (valueIS_==rhs.get());}
  bool    Length::operator!=(const Length & rhs)const {return (valueIS_!=rhs.get());}
  
}

#if     defined(_TESTBED_)

#include <iostream>
int main()
{
  Length longueur(10); 
  cout << "longueur=" << longueur.get("km") << "km" << endl;
  cout << "longueur=" << longueur.get() << "m" << endl;
  
  Length h(10,"km"), dh(10,"m"), hsum, hdif;
  cout << h.get()+dh.get() << endl;
  hsum = h+dh;
  cout << hsum.get() << endl;
  cout << hsum.get("KM") << "KM" << endl;
  cout << hsum.get("km") << "km" << endl;
  Length z;
  z = 20;
  Length dz(20,"mm");
  hdif = z-dz; 
  cout << hdif.get() << " SI units" << endl;
  cout << hdif.get("mm") << "mm" <<  endl;
  
  Length hscaled;
  int scf=3;
  
  hscaled = h*scf;
  cout <<h.get("km")<<"km * "<< scf <<" = "<< hscaled.get() << "m" << endl;
  
  
  Length P(10); cout<< "Length P(10) ==> P="<<P.get() << endl;
  
  Length p = P; cout<< "Length p=P; ==> p="<<p.get() << endl;
  cout << "Test for the relational operators:" << endl;
  if(p>=P)cout << "p>=P" << endl;
  if(p<=P)cout << "p<=P" << endl;
  if(p==P)cout << "p==P" << endl;
  cout << "Arithmetic operations:" << endl;
  p = P+p; cout<< "p=P+p ==> p=" << p.get() << endl;
  cout << "P=" <<P.get() << " must not change of value!" << endl;
  p = p+P; cout<< "p=p+P ==> p=" << p.get() << endl;
  cout << "P=" <<P.get() << " must not change of value!" << endl;
  Length p0; cout << "Let declare p0 be of type Length using the default constructor: Length p0;" << endl;
  p0 = p+P; cout <<"p0 = p+P with p="<<p.get()<<" and P="<<P.get()<<" ==> p0=p+P="<<p0.get()<<endl; 
  Length p1; cout << "Let declare p0 be of type Length using the default constructor: Length p1;" << endl;
  cout << "With no assignement its value must be 0: p1="<<p1.get() << endl;
  if(P<p){
    cout << "Test relational operator P>p: OK" << endl;
  }
  if(p!=P){
    cout << "Test relational operator p!=P: OK" << endl;
  } 
  if(p<P)cout << "p<P" << endl;
  cout << "Some more tests for arithmetic expressions " << endl;
  p1 = P; cout << "Assign the value of P to p1: p1=P ==> p1=" << p1.get() << endl;
  p1 = p1+p+P;cout << "Arithmetic sum: p1+p+P=" << p1.get() << endl;
  p1 = p1-p-P;cout << "Arithmetic difference: p1-p-P=" << p1.get() << endl;
  p1 = p1*2.0;cout << "Product by a scalar p1=p1*2.0=" << p1.get("mm") << "mm" << endl;
  cout <<"p0="<<p0.get()<<endl;
  p1 = p0*2.0;cout << "p1=p0*2.0=" << p1.get("mm") << "mm" << endl;
  cout <<"--------------------------------------------------------------------------" << endl;
  cout <<"| Note that the commutativity of the oprator * has not been implemented! |" << endl;
  cout <<"--------------------------------------------------------------------------" << endl;
  cout <<"p0="<<p0.get()<<" must have not changed of value" << endl;
  cout <<"p1="<<p1.get()<<endl;
  p1 = p1/2.0e0;cout << "p1=p1/2.0=" << p1.get("mm") << "mm" << endl;
  p1 = p0/2.0;cout << "p1=p0/2.0=" << p1.get("mm") << "mm" << endl;
  cout <<"-----------------------------------------------------------------------" << endl;
  cout <<"| Note that dividing by a physical quantity has not been implemented! |" << endl;
  cout <<"-----------------------------------------------------------------------" << endl;
  cout << endl;
  cout << "Some more tests for arithmetic expressions " << endl;
  cout <<"p0="<<p0.get()<<"  must have not changed of value" << endl;
  cout << "P=" << P.get() << endl;
  p1 = p0*2.0e0+P;
  cout <<"p1=p0*2.0E0+P="<<p1.get()<<endl;
  cout << endl;
  cout << "TESTBED done " << endl;
  
  return 0;
}

#endif
