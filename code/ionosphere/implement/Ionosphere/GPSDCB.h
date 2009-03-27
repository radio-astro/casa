#ifndef IONOSPHERE_GPSDCB_H
#define IONOSPHERE_GPSDCB_H

#include <casa/stdio.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>
#include <ionosphere/Ionosphere/GPS.h>
#include <casa/Logging/LogIO.h>    
#include <casa/Containers/Record.h>    
    

namespace casa { //# NAMESPACE CASA - BEGIN

// 30 days is normal spacing for DCB estimates
const Float DCB_SPACING = 30;
    
// <summary>
// Holds GPS group delays. Deprecated, use GPSDCB instead.
// </summary>

class GPSDCB
{
protected:
  Float missing;                 // missing value
  uInt nrec,nsvn;
  Vector<Int>   mjds;             // vector of NREC MJDs of data samples
  Vector<Float> fmjd;             // same, but in float version
  Cube<Float>   dcb;              // NSVNxNMJDxNTYPE cube of sattelite DCBs
  Cube<Float>   dcbrms;           // NSVNxNMJDxNTYPE cube of sattelite DCB RMSs

  uInt nsta;
  Vector<String>  stids;           // NSTA IGS IDs
  Record         stnums;         // record maps IDs back to numbers
  Matrix<Float>   stadcb;         // NSTAxNMJD matrix of station P1-P2 DCBs
  Matrix<Float>   starms;         // NSTAxNMJD matrix of station P1-P2 DCB RMSs

  LogIO os;
  
public:
  typedef enum { P1_P2,P1_C1 } DCBType;
  
// constructor. Loads specified table, or default system-wide table    
  GPSDCB ( const char *tablename=NULL );  
  
  uInt numSVN ()
  { return nsvn; }
      

// do we have a DCB value for this SVN & date?
//  Bool svnValid ( DCBType type,Float mjd,uInt svn ) const;
//  Vector<Bool> svnValid ( DCBType type,Float mjd ) const;

// TGD is a synonym for the P1-P2 DCB  
//  Bool tgdValid ( Float mjd,uInt svn ) const 
//      { return svnValid(P1_P2,mjd,svn); };
//  Vector<Bool> tgdValid ( Float mjd ) const 
//      { return svnValid(P1_P2,mjd); };

// interpolates DCB data to given MJDs, returns interpolated values
// and stores RMS into rms. If warn is True, issues warnings if
// data is being extrapolated or interpolated within significant gaps. 
// Note: all units are ns.
  Vector<Float> getDcb ( Vector<Float> &rms,DCBType type,
                        const Vector<Float> &mjd,uInt svn,Bool warn=True );
  
// TGD is a synonym for the P1-P2 DCB. Units: ns.  
  Vector<Float> getTgd ( Vector<Float> &rms,const Vector<Float> &mjd,uInt svn,Bool warn=True )
      { return getDcb(rms,P1_P2,mjd,svn,warn); };
  
// return vector of available station IDs
  const Vector<String> & stations () const;
// converts station ID to number (throws exception for none)
  Int stationNum ( const String &id ) const;
  
// do we have a TGD value for this station and date?
  Bool stationTgdValid ( Float mjd,Int stnum ) const;
  Bool stationTgdValid ( Float mjd,const String &id ) const
          { return stationTgdValid(mjd,stationNum(id)); }
  
  Float getStationTgd ( Float mjd,Int stnum ) const;
  Float getStationTgd ( Float mjd,const String &id ) const
          { return getStationTgd(mjd,stationNum(id)); }
  
// accessors to raw data records
  const Vector<Int> &    rawMjd      () const;
  const Vector<Float> &  rawFmjd      () const;
  const Cube<Float> &    rawDcb      () const;
  const Cube<Float> &    rawDcbRms   () const;
  const Vector<String> & rawStIDs    () const;
  const Matrix<Float> &  rawStDcb    () const;
  const Matrix<Float> &  rawStDcbRms () const;
};

inline const Vector<String> & GPSDCB::stations () const
{
  return stids;
}

inline Int GPSDCB::stationNum ( const String &id ) const 
{
  return stnums.asInt(id); 
}

inline const Vector<Int> & GPSDCB::rawMjd    () const 
{ return mjds; }

inline const Vector<Float> & GPSDCB::rawFmjd    () const 
{ return fmjd; }

inline const Cube<Float> & GPSDCB::rawDcb      () const
{ return dcb; }

inline const Cube<Float> & GPSDCB::rawDcbRms   () const
{ return dcbrms; }

inline const Vector<String> & GPSDCB::rawStIDs () const
{ return stids; }

inline const Matrix<Float> & GPSDCB::rawStDcb () const
{ return stadcb; }

inline const Matrix<Float> & GPSDCB::rawStDcbRms () const
{ return starms; }


} //# NAMESPACE CASA - END

#ifndef AIPS_NO_TEMPLATE_SRC
#include <ionosphere/Ionosphere/GPSDCB.tcc>
#endif //# AIPS_NO_TEMPLATE_SRC
#endif
