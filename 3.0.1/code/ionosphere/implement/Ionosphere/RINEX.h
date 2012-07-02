#ifndef IONOSPHERE_RINEX_H
#define IONOSPHERE_RINEX_H

#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/BasicSL/String.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MEpoch.h>
#include <casa/Containers/Block.h>
#include <ionosphere/Ionosphere/GPS.h>
#include <ionosphere/Ionosphere/RINEXSat.h>
#include <ionosphere/Ionosphere/GPSEphemeris.h>
#include <ionosphere/Ionosphere/GPSDCB.h>


namespace casa { //# NAMESPACE CASA - BEGIN

//
// Note on SVNs.
// All SVNs are 1-based.
//

// -----------------------------------------------------------------------
// class RINEXHeader
// <summary>
// Holds information from the header of a RINEX file
// </summary>
// -----------------------------------------------------------------------
class RINEXHeader {
public:
  String header;            // full header
  Float  tsmp;              // sampling interval 
  MVPosition marker;         // marker position (as given in file)
  MVPosition offset;         // antenna offset  (as given in file)
  MVPosition pos;            // receiver position (marker+offset)  
  Double ep_first,ep_last;   // first/last epochs

  uInt    nep,nsat,ntyp;
  Vector<uInt> svn;
  Matrix<uInt> nobs;
  
  RINEXHeader() :
      header(""),tsmp(0),marker(0,0,0),offset(0,0,0),pos(),
      ep_first(0),ep_last(0),
      nep(0),nsat(0),ntyp(0),
      svn(0),nobs(0,0) {}
};

// -----------------------------------------------------------------------
// class RINEX
// <summary>
// Holds information from one RINEX file
// </summary>
// -----------------------------------------------------------------------
class RINEX
{
protected:
  RINEXHeader    hdr_;
  RINEXSat       sat_[NUM_GPS];
// vector of epochs. This vector is global to all the RINEXSats, who
// index into it
  Vector<Double>  mjd_;
// conversion engine for IAT->UTC conversion
  static MEpoch::Convert iat2utc;
  

public:
// Enumerator for interesting RINEX columns
  enum ENUM_RINEX_COLUMNS;  

// default constructor and destructor
  RINEX();
  ~RINEX();
  
// static methods for converting GPS time to MJD
  static Double GPS2MJD ( Double gpsday );
  static Double GPS2MJD ( int iyr,int imo,int idom,int ihr,int imin,Float sec );

// constructor. Imports RINEX2 file.
  RINEX( const char *filename );  
  
// Imports a RINEX2 file.  
  Int import ( const char *filename );
  
// Computes TEC samples for all SVNs, sorted by epoch. 
// Returns total number of samples.
  uInt getTEC ( Vector<Double> &mjd,          // output: epochs
               Vector<Int>   &svn,           // output: SVNs
               Vector<Double> &tec,           // output: TEC
               Vector<Double> &sigTec,        // output: sigma TEC
               Vector<Double> &sigTec30,      // output: 30-minute min. sigma TEC
               Vector<Int>    &domain,        // output: domain number
               GPSDCB &dcb );              // input: group delays
// Returns per-sattelite data as a RINEXSat object
  RINEXSat & sdata (uInt svn);
// Returns vector of defined epochs
  const Vector<Double> & epochs ()  { return mjd_; }
  Double epoch ( uInt i )          { return mjd_(i); }
// Returns header
  const RINEXHeader & header   ()  { return hdr_; }
// Returns receiver position
  const MVPosition  & rcvPos   ()  { return hdr_.pos; }

// diagnostics level  
  static int debug_level;
};

inline RINEXSat & RINEX::sdata ( uInt isat ) 
{
  return sat_[isat];
}
  


} //# NAMESPACE CASA - END

#endif
