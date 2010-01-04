//# GPSEphemeris.cc: 
//# Copyright (C) 2001,2002
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#include <errno.h>
#include <casa/string.h>
#include <casa/Exceptions.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Quanta/MVEpoch.h>
#include <measures/Measures/MCEpoch.h>
#include <measures/Measures/MCDirection.h>
#include <measures/Measures.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MPosition.h>
#include <scimath/Functionals/Interpolate1D.h>
#include <casa/iostream.h>
#include <ionosphere/Ionosphere/GPSEphemeris.h>

namespace casa { //# NAMESPACE CASA - BEGIN

Int GPSEphemeris::debug_level=0;

GPSEphemeris::GPSEphemeris() :
    svn_valid(NUM_GPS,False),
    orbSig(NUM_GPS,0)
{
}

GPSEphemeris::GPSEphemeris( const char *filename ) :
    svn_valid(NUM_GPS,False),
    orbSig(NUM_GPS,0)
{
  importIGS(filename);
}

const char * GPSEphemeris::readLine ( FILE *f )
{
  static char str[256];
  memset(str,0,sizeof(str));
  lineNum++;
  if( fgets(str,sizeof(str),f)==NULL )
    throw( AipsError("unexpected EOF") );
  return str;
}

Int GPSEphemeris::importIGS ( const char *filename )
{
  String errHead=String("IGS-SP3 import ")+filename+": ";
  const char *errhead=errHead.chars();

// open file
  FILE *f=fopen(filename,"rt");
  if( !f )
    throw( AipsError(errHead+strerror(errno)) );

// set up conversion engine for IAT->UTC conversion
  MEpoch::Convert iat2utc(MEpoch::IAT,MEpoch::UTC);
  
  int nsat,nep;

  try {
  if( debug_level>0 )
    cerr<<errhead<<"processing header\n";
  
// read IGS-SP3 header  
  Vector<String> header(22);
  for( uInt i=0; i<header.nelements(); i++ )
    header(i) = readLine(f);
  
// Line 1: check P/V flag (must be P), and get number of epochs in file
  Bool hasVel=False;
  switch( header(lineNum=0)[2] )
  {
    case 'v': case 'V': hasVel=True; break;
    case 'p': case 'P': hasVel=False; break;
    default:
      throw( AipsError( String("unknown Pos/Vel flag '")+header(0)[2]+"'" ) );
  }
  if( sscanf(header(0).chars(),"%*32c%d",&nep) < 1 )
    throw( AipsError("bad # epochs") );
  if( debug_level>1 )
    cerr<<errhead<<"expecting "<<nep<<" epochs\n";
// Line 2: sampling interval
  if( sscanf(header(lineNum=1).chars(),"%*24c%lf",&interval) < 1 )
    throw( AipsError("bad sampling interval") );
  if( debug_level>1 )
    cerr<<errhead<<"sampling interval: "<<interval<<endl;
// Lines 3, 4: NSAT and SVNs
  if( sscanf( header(lineNum=2).chars(),"%*4c%d",&nsat) < 1 )
    throw( AipsError("bad # satellites") );
  svn_valid=False;
  orbSig.resize(NUM_GPS);
  Vector<uInt> svn(nsat);
  Int tmp[17];
  if( sscanf(header(2).chars(),"%*10c%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
        tmp,tmp+1,tmp+2,tmp+3,tmp+4,tmp+5,tmp+6,tmp+7,tmp+8,tmp+9,tmp+10,
        tmp+11,tmp+12,tmp+13,tmp+14,tmp+15,tmp+16 ) < 17 )
    throw( AipsError("bad SVNs") );
  for( Int i=0; i<min(17,nsat); i++ ) 
  {
    if( tmp[i]<1 || tmp[i]>(Int)MAX_GPS_SVN )
      throw( AipsError("illegal SVN") );
    svn_valid((svn(i)=tmp[i])-1)=True;
  }
  if( nsat > 17 ) // scan more SVNs from line 4, if needed
  {
    if( sscanf(header(lineNum=3).chars(),"%*10c%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
          tmp,tmp+1,tmp+2,tmp+3,tmp+4,tmp+5,tmp+6,tmp+7,tmp+8,tmp+9,tmp+10,
          tmp+11,tmp+12,tmp+13,tmp+14,tmp+15,tmp+16 ) < 17 )
      throw( AipsError("bad SVNs") );
    for( Int i=17; i<nsat; i++ ) 
    {
      if( tmp[i-17]<1 || tmp[i-17]>(Int)MAX_GPS_SVN )
        throw( AipsError("illegal SVN") );
      svn_valid((svn(i)=tmp[i-17])-1)=True;
    }
  }
// Lines 8,9: orbital accuracies
  if( sscanf(header(lineNum=7).chars(),"%*10c%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
        tmp,tmp+1,tmp+2,tmp+3,tmp+4,tmp+5,tmp+6,tmp+7,tmp+8,tmp+9,tmp+10,
        tmp+11,tmp+12,tmp+13,tmp+14,tmp+15,tmp+16 ) < 17 )
    throw( AipsError("bad accuracies") );
  for( Int i=0; i<min(17,nsat); i++ ) 
    orbSig(svn(i)-1)=tmp[i];
  if( nsat > 17 ) // scan more from line 9, if needed
  {
    if( sscanf(header(lineNum=8).chars(),"%*10c%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
          tmp,tmp+1,tmp+2,tmp+3,tmp+4,tmp+5,tmp+6,tmp+7,tmp+8,tmp+9,tmp+10,
          tmp+11,tmp+12,tmp+13,tmp+14,tmp+15,tmp+16 ) < 17 )
      throw( AipsError("bad accuracies") );
    for( Int i=17; i<nsat; i++ ) 
      orbSig(svn(i)-1)=tmp[i-17];
  }
  if( debug_level>1 )
    cerr<<errhead<<"Orbital accuracies: "<<orbSig<<endl;

// Now, prepare storage and read the data records
  if( debug_level>0 )
    cerr<<errhead<<"reading data records\n";
  lineNum = header.nelements()-1;
  Int nrec = hasVel ? nsat*2 : nsat;   // number of records per epoch

  mjd.resize(nep);
  for( uInt i=0; i<NUM_GPS; i++ )
    if( svn_valid(i) )
      eph[i].resize(nep,3);
  
  for( Int iep=0; iep<nep; iep++ )  // expecting this many epochs
  {
    const char *str = readLine(f);  // read epoch record
    if( str[0] != '*' )
      throw( AipsError("epoch record ('*') expected") );
    int iyr,imo,idom,ihr,imin;
    Float sec;
    if( sscanf(str,"%*c %d %d %d %d %d %f",&iyr,&imo,&idom,&ihr,&imin,&sec) < 6 )
      throw( AipsError("malformed epoch record"));
// convert GPS time to UTC
// this gives us GPS time, in days
    Double t = (Double)MVTime(iyr,imo,idom,(ihr+imin/60.+sec/3600.)/24.);
// this gives us IAT = GPS + 19s
    t += 19./(24*(double)3600);
// convert to UTC
    mjd(iep) = iat2utc(t).getValue().get();
    
// Bob used to test for missing epochs or non-uniform sampling, but I think we
// can skip this part safely enough
    for( Int irec=0; irec<nrec; irec++ )
    {
//    Read position information for each satellite for this epoch
//      In case the order of SVNs listed in an epoch differs from
//      that in the header, search through SVN(i), starting with
//      I=ISAT, as it should be for matching order.
//    Write position info for the SVN into the appropriate
//      satellite index I
      str = readLine(f);
      if( str[0] != 'P' ) // not a position record?
      {
        if( hasVel && str[0] == 'V' ) // velocity record skipped...
          continue;
        // otherwise rwaise a stink
        throw( AipsError( String("unexpected record type '")+str[0]+"'" ) );
      }
      Int svn;
      Double x,y,z;
      if( sscanf(str,"%*c %d %lf %lf %lf",&svn,&x,&y,&z) < 4 )
        throw( AipsError( "malformed P-record" ) );
      if( svn<1 || svn>(Int)MAX_GPS_SVN || !svn_valid(svn-1) )
        throw( AipsError( "bad SVN" ) );
      eph[svn-1](iep,EX) = x*1e3;
      eph[svn-1](iep,EY) = y*1e3;
      eph[svn-1](iep,EZ) = z*1e3;
    }
  }
// As a last check, the only remaining line should be "EOF"
  if( strncmp(readLine(f),"EOF",3) )
    cerr<<errhead<<"warning: EOF record missing\n";
  if( debug_level>0 )
  {
    fprintf(stderr,"%sfinished reading %d epochs\n",errhead,nep);
    cerr<<errhead<<"first epoch is "<<MVTime(mjd(0))<<endl;
    cerr<<errhead<<"last epoch is "<<MVTime(mjd(nep-1))<<endl;
  }

// everything caught here is a file format error, so re-throw the exception,
// adding filename and line number information
  } catch( AipsError excp ) { 
    fclose(f);
    mjd.resize(0);
    for( uInt i=0; i<NUM_GPS; i++ )
      eph[i].resize(0,0);
    char str[32];
    sprintf(str,"\b%d: ",lineNum+1);
    throw( AipsError( errHead+str+excp.getMesg() ) );
  }

  fclose(f);
  
  ssf_mjd = ScalarSampledFunctional<Double>(mjd);
  return nep;
}

// -----------------------------------------------------------------------
// splineEph
// Splines the ephemeris Matrix of sattelite svn to new time grid mjd1.
// Returns (nep,3) matrix of splined ephemeris.
// -----------------------------------------------------------------------
Matrix<Double> GPSEphemeris::splineEph (uInt svn,const Vector<Double> &mjd1 ) const
{
  uInt isat = svn-1;
  if( !svn_valid(isat) )
    return Matrix<Double>();

  uInt nmjd1 = mjd1.nelements();
  Matrix<Double> eph1(mjd1.nelements(),3);
  
// init SSFs for interpolation
  for( uInt i=0; i<3; i++ )
  {
    Vector<Double> col( eph[isat].column(i) );
    ScalarSampledFunctional<Double> ssf(col);
    Interpolate1D<Double,Double> spline(ssf_mjd,ssf,True,True);
    spline.setMethod(Interpolate1D<Double,Double>::spline);
    for( uInt j=0; j<nmjd1; j++ )
      eph1(j,i) = spline(mjd1(j)); 
  }
  return eph1;
}

// // -----------------------------------------------------------------------
// // splineEph
// // Splines the ephemeris Matrix of sattelite svn to new time grid mjd1.
// // Sigma is the spline tension, 0 for default (1e-3).
// // Returns (nep,3) matrix of splined ephemeris.
// // -----------------------------------------------------------------------
// Matrix<Double> GPSEphemeris::splineEph (uInt svn,const Vector<Double> &mjd1,Double sigma ) const
// {
//   uInt isat = svn-1;
//   
//   typedef Vector<Double> Vect;
//   if( !svn_valid(isat) )
//     return Matrix<Double>();
// 
//   if( sigma<1e-3 ) 
//     sigma=1e-3;
// #define t0 mjd
// #define t1 mjd1
// // space-saving macro, returns row 'i' of eph matrix (this corresponds to
// // one {X,Y,Z} vector)
// #define r0(i) (eph[isat].row(i))
//   uInt i,j,
//     n0 = t0.nelements(),
//     n1 = t1.nelements(),
//     M  = 3;
//   
//   const Double een=1, hlf=0.5;
//   
//   
//   Double delx1 = t0(1) - t0(0);
//   Vect  dx1( (r0(1) - r0(0)) / delx1 );
//   Double        
//   delx2 = t0(2) - t0(1),
//   delx12 = t0(2) - t0(0),
//   c1 = -(delx12 + delx1) / (delx12*delx1),
//   c2 = delx12 / (delx1*delx2),
//   c3 = -delx1 / (delx12*delx2);
// 
//   Vect slpp1( c1*r0(0) + c2*r0(1) + c3*r0(2) );
//   Double
//   deln = t0(n0-1) - t0(n0-2),
//   delnm1 = t0(n0-2) - t0(n0-3),
//   delnn = t0(n0-1) - t0(n0-3);
//   
//   c1 = (delnn + deln) / (delnn*deln);
//   c2 = -delnn / (deln*delnm1);
//   c3 = deln / (delnn*delnm1);
//   
//   Vect slppn( c3*r0(n0-3) + c2*r0(n0-2) + c1*r0(n0-1) );
// 
//   Double 
//   sigmap = sigma * n0 / (t0(n0-1)-t0(0)),
//   dels = sigmap * delx1,
//   exps = exp(dels),
//   sinhs = hlf * (exps - een/exps),
//   sinhin = een / (delx1*sinhs),
//   diag1 = sinhin * (dels * hlf*(exps + een/exps) - sinhs),
//   spdiag = sinhin * (sinhs-dels);
//   Vect diagin(M);
//   diagin = een / diag1;
// 
//   Matrix<Double> yp_matrix(M,2*n0);
// #define yp(i) (yp_matrix.column(i))
//   yp(0) = diagin * (dx1 - slpp1);
//   yp(n0) = diagin * spdiag;
// 
//   Double diag2; Vect dx2;
//   
//   for( uInt i=1; i<n0-1; i++ ) 
//   {
//     delx2 = t0(i+1) - t0(i);
//     dx2 = r0(i+1) - r0(i);
//     dx2 /= delx2;
//     dels = sigmap * delx2;
//     exps = exp(dels);
//     sinhs = hlf * (exps - een/exps);
//     sinhin = een / (delx2*sinhs);
//     diag2 = sinhin * (dels * hlf*(exps + een/exps) - sinhs);
//     diagin = een / (diag1 + diag2 - spdiag*yp(n0+i-1));
//     yp(i) = diagin * (dx2 - dx1 - spdiag*yp(i-1) );
//     spdiag = sinhin * (sinhs-dels);
//     yp(n0+i) = diagin * spdiag;
//     dx1.reference( dx2 );
//     diag1 = diag2;
//   }
// 
//   diagin = een / (diag1 - spdiag*yp(2*n0-2)); 
//   yp(n0-1) = diagin * (slppn - dx2 - spdiag*yp(n0-2));
// 
//   for( Int i=n0-2; i>=0; i-- )
//     yp(i) = yp(i)-yp(i+n0)*yp(i+1);
//   
//   Vector<uInt> subs(n1);
//   subs = n0-1;
//   Double s = t0(n0-1) - t0(0); 
//   sigmap = sigma*n0 / s;
// 
//   for( i=1,j=0; i<n0 && j<n1; )
//     if( t0(i) > t1(j) )
//       subs(j++)=i;
//     else
//       i++;
// 
//   Matrix<Double> eph1(n1,M);
//   for( j=0; j<n1; j++ )
//   {
//     Double del1,del2,exps1,sinhd1,sinhd2;
//     uInt subs1 = subs(j) - 1;
//     del1 = t1(j) - t0(subs1);
//     del2 = t0(subs(j)) - t1(j);
//     dels = t0(subs(j)) - t0(subs1);
//     exps1 = exp(sigmap*del1);
//     sinhd1 = hlf * (exps1 - een/exps1);
//     exps = exp(sigmap*del2);
//     sinhd2 = hlf * (exps - een/exps);
//     exps = exps * exps1;
//     sinhs = hlf * (exps - een/exps);
//     eph1.row(j) = 
//       (yp(subs(j))*sinhd1 + yp(subs1)*sinhd2) / sinhs +
//       ( (r0(subs(j))  - yp(subs(j)))*del1 +
//         (r0(subs1)   - yp(subs1)  )*del2  ) / dels ;
//   }
//   
//   return eph1;
// }
// 

// -----------------------------------------------------------------------
// splineAzEl
// Splines the ephemeris of sattelite SVN to new time grid MJD1,
// and returns the result as a (nep) vector of MVDirections, corresponding
// to Az/El from the observation point given by POS (ITRF).
// Tension is the spline tension, 0 for default (1e-3).
// -----------------------------------------------------------------------
Vector<MVDirection> GPSEphemeris::splineAzEl (uInt svn,const Vector<Double> &mjd1,
                                  const MVPosition &pos ) const
{
  uInt nep=mjd1.nelements();
  Matrix<Double> eph1( splineEph(svn,mjd1) );
//  cerr<<eph1.column(EX)<<endl;
//  cerr<<eph1.column(EY)<<endl;
//  cerr<<eph1.column(EZ)<<endl;
//  cerr<<pos;

  Vector<MVDirection> azel(nep);
  
  const Vector<Double> posvec( pos.getValue() );
  cerr<<"Receiver position: "<<pos<<endl;
  fprintf(stderr,"Receiver is at %f %f %f\n",posvec(0),posvec(1),posvec(2));
  
  MeasFrame frame( MPosition(pos,MPosition::ITRF) ); //,MEpoch(MVEpoch(mjd1(0))) ); //,MEpoch(MVEpoch(mjd1(0))) );
  MDirection::Ref ref0(MDirection::ITRF,frame), 
                  ref1(MDirection::AZEL,frame); 
  MDirection::Convert converter(ref0,ref1);

  for( uInt i=0; i<nep; i++ ) 
  {
//    frame.resetEpoch( MVEpoch(mjd1(i)) );
    azel(i) = converter( eph1.row(i) - posvec ).getValue();
//    azel(i) = converter( eph1.row(i) ).getValue();
    
    MVTime mvt(mjd1(i));
//    fprintf(stderr,"%02d %s %f %f %f: %f %f\n",isat,
//            mvt.string().chars(),
//            eph1(i,EX),eph1(i,EY),eph1(i,EZ),
//            azel(i).getLong()/C::degree,azel(i).getLat()/C::degree);
  }
  
//   Vector<Double> xyz( pos.get() );
//   Double x0=pos(0),y0=pos(1),z0=pos(2),  
//         rr = sqrt( x0*x0+y0*y0 ),
//         ra = sqrt( x0*x0+y0*y0+z0*z0 ); 
// // convert to az/el
//   Vector<Double> 
//     xlos( eph1.column(EX) - x0 ),
//     ylos( eph1.column(EY) - y0 ),
//     zlos( eph1.column(EZ) - z0 ),
//     elos( (-xlos*y0 + ylos*x0)/rr ),
//     nlos( zlos*rr/ra - (xlos*z0*x0 + ylos*z0*y0)/(ra*rr) ),
// 	  ulos( (xlos*x0 + ylos*y0 + zlos*z0) / ra ),
//     az( atan2(elos, nlos) ),
//     el( atan2(ulos, sqrt(elos*elos + nlos*nlos)) );
// // store as MVDirections
//   Vector<MVDirection> dir(nep);
//   for( uInt i=0; i<nep; i++ )
//     dir(i) = MVDirection( az(i),el(i) );
  
  return azel;
}

} //# NAMESPACE CASA - END

