using namespace std;
#include <string>
#include <vector>
#include <iostream>


#include "ATMAtmosphereType.h"
#include "ATMType.h"
#include "ATMTypeName.h"       // require
using namespace atm;

int main()
{  
  cout << endl;
  cout << " AtmTypeTest: Test for type tropical:" << endl;
  Atmospheretype monType=tropical;
  AtmType        myAtmType1(monType);

  for( int i=0; i<myAtmType1.getArraySize(); i++){
    cout << " AtmTypeTest: " << i 
	 << " hx: " << myAtmType1.getHx(i) 
	 << " px: " << myAtmType1.getPx(i)
	 << " tx: " << myAtmType1.getTx(i) << endl;  
  }

  cout << endl;
  cout << " AtmTypeTest: Test for type TROPICAL:" << endl;
  monType=TROPICAL;
  AtmType        myAtmType(monType);

  for( int i=0; i<myAtmType.getArraySize(); i++){
    cout << " AtmTypeTest: " << i 
	 << " hx: " << myAtmType.getHx(i) 
	 << " px: " << myAtmType.getPx(i)
	 << " tx: " << myAtmType.getTx(i) << endl;  
  }

  cout << endl;
  AtmType        myAtmType2(midlatWinter);
  cout << " AtmTypeTest: Test for type "<< myAtmType2.getAtmosphereType() << endl;
  for( int i=0; i<myAtmType2.getArraySize(); i++){
    cout << " AtmTypeTest: " << i 
	 << " hx: " << myAtmType2.getHx(i) 
	 << " px: " << myAtmType2.getPx(i)
	 << " tx: " << myAtmType2.getTx(i) << endl;  
  }

  cout << endl;
  AtmType*        myAtmType3;
  myAtmType3 = new AtmType(subarcticWinter);
  cout << " AtmTypeTest: Test for type "<< myAtmType3->getAtmosphereType() << endl;
  for( int i=0; i<myAtmType3->getArraySize(); i++){
    cout << " AtmTypeTest: " << i 
	 << " hx: " << myAtmType3->getHx(i) 
	 << " px: " << myAtmType3->getPx(i)
	 << " tx: " << myAtmType3->getTx(i) << endl;  
  }

  cout << " AtmTypeTest: TESTBED done " << endl;

  return 0;
}



