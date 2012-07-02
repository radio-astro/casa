#include <stdio.h>
    
#include <ionosphere/Ionosphere/GPSEphemeris.h>
#include <casa/Exceptions/Error.h>
    
#include <casa/namespace.h>
int main (void)
{
  GPSEphemeris::debug_level=2;
  
  try {
    GPSEphemeris eph("test.orb");
  }
  
  catch( AipsError& x )
  {
    cerr << "AipsError: " << x.getMesg() << endl;
    return 1;
  }
  return 0;
}
