#include <casa/stdio.h>
#include <errno.h>
#include <casa/string.h>
#include <ionosphere/Ionosphere/GPS.h>
#include <ionosphere/Ionosphere/GPSGroupDelay.h>
#include <casa/Exceptions/Error.h>
    
namespace casa { //# NAMESPACE CASA - BEGIN

// constructor. Loads TGD file    
GPSGroupDelay::GPSGroupDelay( const char *filename ) :
    mjd1_(0),mjd2_(0),
    svn_valid(NUM_GPS,False),
    tgd_(NUM_GPS,0.0)
{
  import(filename);
}

// Loads TGD file
Int GPSGroupDelay::import (const char *filename) 
{
  String errHead=String("TGD import ")+filename+": ";

// read group delay data (JPL format -- provided by Bob)
  FILE *f=fopen(filename,"rt");
  if( !f )
    throw( AipsError(errHead+strerror(errno)) );
  
  int count=0,svn; float t1,t2;
  while( fscanf(f,"%d %f %f",&svn,&t1,&t2)==3 )
  {
    svn_valid(svn) = True;
    tgd_(svn) = t1/1e9;
    count++;
  }

  fclose(f);
  
  return count;
}


} //# NAMESPACE CASA - END

