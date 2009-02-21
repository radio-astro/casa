using namespace std;

#include <iostream>
#include "ATMLength.h"
#include <string>

 
using namespace atm;

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

