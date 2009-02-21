using namespace std;
#include <strings.h>
#include <iostream>

#include <atmosphere/ATM/ATMAtmosphereType.h>

#include <atmosphere/ATM/ATMTypeName.h>

#if     defined(_TESTBED_)




using namespace atm;

int main()
{
  
  Atmospheretype myType;

  myType=TROPICAL;

  //cout << AtmosphereType::name(1) << endl;
  cout << AtmosphereType::name(tropical) << endl;
  cout << AtmosphereType::name(myType) << endl;

  cout << "TESTBED done " << endl;

  return 0;
}

#endif
