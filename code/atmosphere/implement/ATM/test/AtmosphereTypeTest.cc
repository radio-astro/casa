using namespace std;
#include <strings.h>
#include <iostream>

#include "ATMAtmosphereType.h"
#include "ATMTypeName.h"
using namespace atm;

int main()
{

  Atmospheretype myType;

  myType=TROPICAL;

  //cout << AtmosphereType::name(1) << endl;
  cout << " AtmosphereTypeTest: " << AtmosphereType::name(tropical) << endl;
  cout << " AtmosphereTypeTest: " << AtmosphereType::name(myType) << endl;

  cout << " AtmosphereTypeTest: TESTBED done " << endl;

  return 0;
}


