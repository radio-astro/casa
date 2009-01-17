#ifndef IONOSPHERE_GPSEPHEMERIS_H
#define IONOSPHERE_GPSEPHEMERIS_H

#include <casa/stdio.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Quanta/MVPosition.h>
#include <casa/Quanta/MVDirection.h>
#include <scimath/Functionals/ScalarSampledFunctional.h>
    
#include <ionosphere/Ionosphere/GPS.h>
    
namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Stores a chunk of GPS Ephemeris data
// </summary>
class GPSEphemeris
{
protected:
// reads one line from input file
  const char * readLine ( FILE *f );

  Int lineNum; // line number in input file
    
// sampling interval
  Double interval;
// vectors of SVN-valid flags, and their orbital accuracy exponents
// Note that SVNs are 1-based
  Vector<Bool> svn_valid;
  Vector<Int>  orbSig;

// vector of epochs (as MJD)
  Vector<Double> mjd;
// SSF representation of MJDs (for interpolation)
  ScalarSampledFunctional<Double> ssf_mjd;
  
// Each sattellite's ephemeris is represented by a (nep,3) Matrix.
// The first index is the epoch, the second is EX/EY/EZ.
  Matrix<Double> eph[NUM_GPS];

public:
  enum { EX=0,EY=1,EZ=2 } Ephemeris;

// default constructor
  GPSEphemeris ();
// constructor. Loads IGS/SP3 file    
  GPSEphemeris( const char *filename );  

// Loads IGS/SP3 file
  Int importIGS (const char *filename);

// do we have ephemeris for this SVN?
  Bool svnValid ( uInt svn )        
      { return svn<=svn_valid.nelements() && svn_valid(svn-1); }

// returns the orbital accuracy indicator (=exponent of sigma) for SVN
  Bool orbSigma ( uInt svn )        
      { return orbSig(svn-1); }
  
// returns vector of epochs 
  const Vector<Double> & getEpochs ()     
      { return mjd; }
// returns ephemeris of given SVN
  const Matrix<Double> & getEph (uInt svn) 
      { return eph[svn-1]; }

// Splines the ephemeris of sattelite SVN to time grid MJD1,
// and returns the result as a (nep,3) matrix of ephemeris.
  Matrix<Double> splineEph (uInt svn,const Vector<Double> &mjd1 ) const;

// Splines the ephemeris of sattelite SVN to new time grid MJD1,
// and returns the result as a (nep) vector of MVDirections, corresponding
// to Az/El from the observation point given by POS (ITRF).
// Tension is the spline tension, 0 for default (1e-3).
  Vector<MVDirection> splineAzEl (uInt svn,const Vector<Double> &mjd1,
                                 const MVPosition &pos) const;

// diagnostics level (for debugging messages)
  static Int debug_level;
};


} //# NAMESPACE CASA - END

#endif
