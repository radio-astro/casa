//# RINEX.cc: 
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

#include <casa/stdio.h>
#include <errno.h>
#include <casa/string.h>
#include <ctype.h>
#include <casa/Exceptions.h>
#include <measures/Measures.h>
#include <measures/Measures/MCPosition.h>
#include <measures/Measures/MCEpoch.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MEpoch.h>
#include <casa/Quanta/MVTime.h>
#include <ionosphere/Ionosphere/RINEX.h>
#include <ionosphere/Ionosphere/GPSDCB.h>
#include <casa/iostream.h>

namespace casa { //# NAMESPACE CASA - BEGIN

Int RINEX::debug_level=0;
MEpoch::Convert RINEX::iat2utc(MEpoch::IAT,MEpoch::UTC);

// small helepr functions
// chops trailing whitespace from string
static String & chop ( String &str )
{
  Int len=str.length(),i;
  for( i=len; i>0 && isspace(str[i-1]); i-- );
  if( i!=len )
    str.del( i,len-i );
  return str;
}
static char * chop ( char *str )
{
  Int len=strlen(str),i;
  for( i=len; i>0 && isspace(str[i-1]); i-- );
  str[i]=0;
  return str;
}

RINEX::RINEX ()
{
}

RINEX::~RINEX ()
{
}

RINEX::RINEX( const char *filename ) 
{
  import(filename);
}

// -----------------------------------------------------------------------
// GPS2MJD
//   Converts GPS time to a UTC MJD
//   First version accepts GPS time specified in days (i.e. a "GPS MJD")
//   Second version accepts broken-down time (Y M D H M S)
// -----------------------------------------------------------------------
Double RINEX::GPS2MJD( Double gpsday )
{
// this gives us IAT time (GPS+19s)
  gpsday += 19./(24*(double)3600);
// convert to UTC
  return iat2utc(gpsday).getValue().get();
}

Double RINEX::GPS2MJD( int iyr,int imo,int idom,int ihr,int imin,Float sec )
{
  if( iyr<100 ) 
    iyr+= iyr<80 ? 2000 : 1900; // obvious century bug in 2080...
  return GPS2MJD( (Double)MVTime(iyr,imo,idom,(ihr+imin/60.+sec/3600.)/24.) );
}

// -----------------------------------------------------------------------
// import
// Import data from a RINEX file
// -----------------------------------------------------------------------
Int RINEX::import ( const char *filename )
{
  String errHead=String("RINEX import ")+filename+": ";
  const char *errhead=errHead.chars();

  RINEXHeader h;
  
  FILE *f=fopen(filename,"rt");
  if( !f )
    throw( AipsError(errHead+strerror(errno)) );

  Vector<uInt> iObs(NUM_GPS);   // number of observations per each sattelite
  iObs=0;
  uInt iSat=0,          // number of sattelites (from header)
       totObs=0,        // total number of observations (from header)
       lineNum=0,       // current line number in input file
       iSlots=0;        // number of unique time slots
  Bool knownSize=False; // flag: is iObs, totObs specified?
  
  
  try {
  Int iwv1,iwv2,iwv3;
  Vector<Int> ind(5); // vector of column indeces
  char str[256];
  Bool pos_set=False;
  
  if( debug_level>0 )
    cerr<<errhead<<"processing header\n";
// read RINEX header  
  Bool endHeader=False;
  while( !endHeader )
  {
    str[0]=0; lineNum++;
    if( fgets(str,sizeof(str),f)==NULL )
      throw( AipsError("unexpected EOF") );
    if( strlen(str)<61 )
      throw( AipsError("malformed header") );
    
    String data( str,60 );   // header data 
    String label( str+60 );  // header label
    chop( data );            // chop of trailing whitespace
    chop( label );

    if( debug_level>1 )
      cerr<<"Headerline "<<label<<": ";

    // process header data depending on label
    if ( label == "RINEX VERSION / TYPE" ) {
      Int version;
      sscanf(data.chars(),"%d",&version);
      if( version!=1 && version!=2 )
        throw( AipsError("unsupported RINEX version") );
      if( data.chars()[20]!='O' )
        throw( AipsError("not an \"Observation Data\" RINEX") );
      if( data.chars()[40]!='G' ) 
        throw( AipsError("not a GPS RINEX") );
    }
    else if ( label == "APPROX POSITION XYZ" ) {
      Double x,y,z;
      pos_set=True;
      if( sscanf(data.chars(),"%lf %lf %lf",&x,&y,&z)<3 )
        throw( AipsError("bad position") );
      h.marker = MVPosition(x,y,z);
      if( debug_level>1 ) cerr<<"[ "<<h.marker<<" ]\n";
    } else if ( label == "ANTENNA: DELTA H/E/N" ) {
      Double dh,de,dn;
      if( sscanf(data.chars(),"%lf %lf %lf",&dh,&de,&dn)<3 )
        throw( AipsError("bad antenna delta") );
      h.offset = MVPosition(de,dn,dh);
      if( debug_level>1 ) cerr<<"[ "<<h.offset<<" ]\n";
    } else if ( label == "INTERVAL" ) {
      if( sscanf(data.chars(),"%f",&h.tsmp)<1 )
        throw( AipsError("bad sampling interval") );
      if( debug_level>1 ) cerr<<"[ "<<h.tsmp<<" ]\n";
    } else if ( label == "TIME OF FIRST OBS" ) {
      int iyr,imo,idom,ihr,imin; Float sec;
      if( sscanf( str,"%d %d %d %d %d %f",
            &iyr,&imo,&idom,&ihr,&imin,&sec)!=6 )
        throw( AipsError("bad time specification") );
      h.ep_first=GPS2MJD(iyr,imo,idom,ihr,imin,sec);
      if( debug_level>1 ) cerr<<"[ "<<MVTime(h.ep_first)<<" ]\n";
    } else if ( label == "TIME OF LAST OBS" ) {
      int iyr,imo,idom,ihr,imin; Float sec;
      if( sscanf( str,"%d %d %d %d %d %f",
            &iyr,&imo,&idom,&ihr,&imin,&sec)!=6 )
        throw( AipsError("bad time specification") );
      h.ep_last=GPS2MJD(iyr,imo,idom,ihr,imin,sec);
      if( debug_level>1 ) cerr<<"[ "<<MVTime(h.ep_last)<<" ]\n";
    } else if ( label == "WAVELENGTH FACT L1/2" ) {
      iwv2=iwv3=0;
      sscanf(data.chars(),"%d %d %d",&iwv1,&iwv2,&iwv3);
      if( debug_level>1 ) cerr<<"[ "<<iwv1<<" "<<iwv2<<" "<<iwv3<<" ]\n";
      if( iwv2 == 0 )
        throw( AipsError("not a dual-frequency RINEX file") );
      if( iwv3 != 0 )
        throw( AipsError("wavelength factors do not apply to all SVN") );
    } else if ( label == "# / TYPES OF OBSERV" ) { // column definitions
//      String tok[20];
//      Int ncol = split( data,tok,sizeof(tok)/sizeof(tok[0]),Regex("[:space:]+") );
      Int ncol,ntyp;
      char colId[10][8];
      ncol = sscanf( data.chars(),"%d %s %s %s %s %s %s %s %s %s %s",&ntyp,
                      colId[0],colId[1],colId[2],colId[3],colId[4],
                      colId[5],colId[6],colId[7],colId[8],colId[9] );
      if( ncol<6 ) // we need at least five columns (one count, plus five types)
        throw( AipsError("missing L1/L2/P1/P2 and/or C1 column") );
      if( ntyp < ncol-1 )
        throw( AipsError("malformed TYPES OF OBSERV line") );
      h.ntyp = ntyp;
      ind = -1;
      for( Int i=0; i<ncol-1; i++ )
      {
        if( !strcasecmp( colId[i],"L1" ) )       ind(L1)=i;
        else if ( !strcasecmp( colId[i],"L2" ) )  ind(L2)=i;
        else if ( !strcasecmp( colId[i],"P1" ) )  ind(P1)=i;
        else if ( !strcasecmp( colId[i],"P2" ) )  ind(P2)=i;
        else if ( !strcasecmp( colId[i],"C1" ) )  ind(CA)=i;
      }
      if( min(ind)<0 )
        throw( AipsError("missing L1/L2/P1/P2 and/or C1 column") );
      if( debug_level>1 ) fprintf(stderr,"[ L1:%d L2:%d P1:%d P2:%d C1:%d ]\n",
                                  ind(L1),ind(L2),ind(P1),ind(P2),ind(CA));
    } else if ( label == "# OF SATELLITES" ) { // number of sattelites in file
      sscanf(data.chars(),"%d",&h.nsat);
      if( h.nsat<1 )
        throw( AipsError("bad # OF SATTELITES line") );
      h.svn.resize(h.nsat);
      h.nobs.resize(h.nsat,5);
      iSat=0;
      totObs=0;
      if( debug_level>1 ) cerr<<"[ "<<h.nsat<<" ]\n";
    } else if ( label == "PRN / # OF OBS" ) { // number of observations per sattelite
      if( h.nsat<1 )
        throw( AipsError("bad or missing # OF SATELLITES line") );
      if( h.ntyp<1 )
        throw( AipsError("bad or missing TYPES OF OBSERV line") );
      if( iSat >= h.nsat )
        throw( AipsError("too many PRN / # OF OBS lines") );
      Int nx[10],svn;
      Int ncol = sscanf( data.chars(),"%d %d %d %d %d %d %d %d %d %d %d",
                            &svn,nx,nx+1,nx+2,nx+3,nx+4,nx+5,nx+6,nx+7,nx+8,nx+9 );
      if( ncol < 6 )
        throw( AipsError("malformed PRN / # OF OBS line") );
      if( svn<1 || svn>(Int)MAX_GPS_SVN )
      {
        char s[64]; sprintf(s,"illegal SVN %d",svn);
        throw( AipsError(s) );
      }
      h.svn(iSat)=svn;
      h.nobs(iSat,0)=nx[ind(L1)];
      h.nobs(iSat,1)=nx[ind(L2)];
      h.nobs(iSat,2)=nx[ind(P1)];
      h.nobs(iSat,3)=nx[ind(P2)];
      h.nobs(iSat,4)=nx[ind(CA)];
      Int m=max(h.nobs.row(iSat));
      totObs+=m;
// We can safely assume that there are m records for this SVN.
      iObs(svn-1)=m;
      if( debug_level>1 ) fprintf(stderr,"[ %d L1:%d L2:%d P1:%d P2:%d CA:%d ]\n",
                                h.svn(iSat),h.nobs(iSat,0),h.nobs(iSat,1),
                                h.nobs(iSat,2),h.nobs(iSat,3),h.nobs(iSat,4));
      iSat++;
    } else if ( label == "END OF HEADER" ) {
      endHeader=True;
      if( debug_level>1 ) cerr<<endl;
    } else {
      if( debug_level>1 ) cerr<<"ignored\n";
    }
    h.header+=str; // attach line to full header, for future reference
  } // end while( !endHeader )
  // check that all requisite data was found in header
  if( !pos_set )
    throw( AipsError("bad or missing APPROX POSITION XYZ line") );
  if( iwv1 <= 0 )
    throw( AipsError("bad or missing WAVELENGTH FACT L1/2 line") );
  if( h.tsmp < 0 )
    throw( AipsError("bad or missing INTERVAL line") );
      
  // compute true receiver position, applying antenna offsets
  {
  const Vector<Double> pos( h.marker.get() ),
                     off( h.offset.get() );
  Double r = sqrt(pos(0)*pos(0)+pos(1)*pos(1)),
        rr = h.marker.getLength().getBaseValue();
  MVPosition offset( 
    pos(0)/rr*off(2) - pos(1)/r*off(0) - pos(0)*pos(2)/(r*rr)*off(1),
    pos(1)/rr*off(2) - pos(0)/r*off(0) - pos(1)*pos(2)/(r*rr)*off(1),
    pos(2)/rr*off(2) + r/rr*off(1) );
  h.pos = h.marker+offset;
  if( debug_level > 0 ) 
  {
    cerr<<errhead<<"receiver position is "<<h.pos<<"(ITRF)\n";
	  MPosition wgs = 
      MPosition::Convert(MPosition(h.pos,MPosition::ITRF),MPosition::WGS84)();
    cerr<<errhead<<"corresponding to "<<
        wgs.getValue().getAngle("deg")<<" "<<wgs.getValue().getLength("m")<<"(WGS84)\n";
  }
  }
  

  Bool guess=False,guess_tsmp=False;
  if( h.tsmp==0 ) 
  {
    guess=guess_tsmp=True;
    h.tsmp=30;
    fprintf(stderr,"%ssampling interval not specified, guessing %f s\n",errhead,h.tsmp);
  }

  uInt nep;
  if( h.ep_first>0 && h.ep_last>0 )
  { 
    nep=1+(uInt)((h.ep_last-h.ep_first)*(24*3600)/h.tsmp+.5);
    fprintf(stderr,"%sexpecting %d epochs\n",errhead,nep);
  }
  else
  {
    guess=True;
    nep=2880;    // guess a reasonable size - 1 day at 30 sec interval
  }
  mjd_.resize(nep);

// Allocate blocks to hold data
  if( h.nsat<1 || iSat<1 ) // Euro-RINEX, no NSAT/NOBS information
  {
    knownSize=False;
    if( debug_level>0 )
      cerr<<errhead<<"reading EuroRINEX; no prior NSAT/NOBS info\n";
    for( uInt s=0; s<NUM_GPS; s++ )
      sat_[s].set(s+1,1500,h.tsmp);
  }
  else  // full RINEX
  { 
    if( iSat < h.nsat )
      throw( AipsError("too few PRN / # OF OBS lines") );
    if( debug_level>0 )
      fprintf(stderr,"%sexpecting %d SVNs\n",errhead,h.nsat);
    knownSize=True;
    for( uInt s=0; s<NUM_GPS; s++ )
      sat_[s].set(s+1,iObs(s),h.tsmp);
  }
  iObs=0; // reset observation counts -- will now count actual data
  iSlots=0;
  
// set up conversion engine for IAT->UTC conversion
//  MEpoch::Convert iat2utc(MEpoch::IAT,MEpoch::UTC);
  
  Bool endData=False;
  while( !endData )
  {
    // read the current epoch record
    str[0]=0; lineNum++;
    if( fgets(str,sizeof(str),f)==NULL )
      break;  // end of file
    int iyr,imo,idom,ihr,imin,event;
    uInt nchan,svn[NUM_GPS]; 
    Float sec;
    // scan fixed part of epoch record (time, event, # sats)
    if( sscanf( str,"%3d%3d%3d%3d%3d%11f%3d%3d",
            &iyr,&imo,&idom,&ihr,&imin,&sec,&event,&nchan) < 8 )
      throw( AipsError("malformed epoch record") );
    if( nchan > NUM_GPS )
      throw( AipsError("too many sattelites specified by epoch record") );
    // check the event flag (namely, for EOF)
    if( event != 0 ) 
    {
      if( event == 4 ) // event==4 => EOF expected
      {
        str[0]=0; lineNum++;
        if( fgets(str,sizeof(str),f)==NULL ) // really EOF, but no terminating record
        {
          cerr<<errhead<<"warning: missing END OF FILE record\n";
          endData=True;
          continue;
        } else {
          const char eof[]="END OF FILE";      // check for term. record
          if( !strncmp(str,eof,sizeof(eof)-1) ) {
            endData=True;
            continue;
          }
        }
      }
      sprintf(str,"unexpected event flag %d",event);
      throw( AipsError(str) );
    }
    // read in additional line, if multi-line record is expected
    if( nchan > 12 ) {
      Int len=strlen(chop(str));
      lineNum++;
      if( fgets(str+len,sizeof(str)-len,f)==NULL )
        throw( AipsError("unexpected EOF") );
    }
    // scan in the SVNs
    if( strlen(str) < 32+3*nchan )
      throw( AipsError("malformed epoch record") );
    for( uInt i=0; i<nchan; i++ ) 
    {      
      char type;
      if( sscanf(str+32+3*i,"%c%2d",&type,&svn[i])<2 )
        throw( AipsError("malformed epoch record") );
      if( type!=' ' && type!='G' )
      {
        sprintf(str,"unexpected sattelite type <%c>",type);
        throw( AipsError(str) );
      }
    }
// convert to UTC
    Double mjd = GPS2MJD(iyr,imo,idom,ihr,imin,sec);
// check if we don't have more slots than expected...
    if( iSlots >= mjd_.nelements() )
    {
      mjd_.resize( iSlots+1000,True );
      if( h.ep_first>0 && h.ep_last>0 )
        fprintf(stderr,"%swarning: more epoch records than indicated by header\n",errhead);
      if( debug_level>0 )
        cerr<<errhead<<"resizing to "<<mjd_.nelements()<<" samples\n";
    };
    mjd_(iSlots)=mjd;
    
//** Note 2: there was a check for non-uniform sampling here,
//** but I don't see that we need it anymore

//BC:  Now loop through the NCHNL SVNs for this epoch:
//BC:    Read the data -- load data for all SVNs at this epoch first to
//BC:       allow some diagnostic tests required by the not-quite-standard
//BC:       "Euro-RINEX" format (not required by the JPL-processed files
//BC:       on which READ_RINEX was originally based)
//BC:    Find the proper satellite-index for the SVN from the ephemeris
//BC:    Assign the data to the appropriate phase/group delays
    Vector<Double> d(5);           // data columns
    Vector<Int> lli(5),ssi(5);     // loss-of-lock and signal strength indicators
    for( uInt ichan=0; ichan<nchan; ichan++ )
    {
      str[0]=0; lineNum++;
      if( fgets(str,sizeof(str),f)==NULL )
        throw( AipsError("unexpected EOF") );  // end of file
      if( h.ntyp>5 ) // multi-line record?
      {
        Int len=strlen(chop(str));
        lineNum++;
        if( fgets(str+len,sizeof(str)-len,f)==NULL )
          throw( AipsError("unexpected EOF") );
      }
// after all these years, I now learn that scanf() is unsuitable for scanning
// fixed length, stuck-together fields -- the dumbass skips leading whitespace, 
// so it isn't counted towards the width of the first field, so everything
// just fucks up after that. So this didn't work:
//  int res=sscanf( str,"%14lf%1d%1d...
// ...and I have to do the following UGLY kludge instead:
#define DATAW   14
#define LLIPOS  14
#define SSIPOS  15
#define FIELDW  (DATAW+2)
      chop(str);
      int len=strlen(str);
      d=0; lli=ssi=0;  // default values 
      for( uInt i=0; i<ind.nelements(); i++ )   // extract data columns
      {
        int field_pos = FIELDW*ind(i);         // start of field in line
        if( field_pos+LLIPOS < len-1 )
          lli(i)=str[field_pos+LLIPOS]-'0';    // LLI is character 14 in each column  
        if( field_pos+SSIPOS < len-1 )
          ssi(i)=str[field_pos+SSIPOS]-'0'; // SSI is character 15
        if( field_pos+DATAW < len )
        {
          str[field_pos+DATAW]=' ';           // put in space to isolate the float value...
          sscanf(str+field_pos,"%lf",&d(i));
        }
        if( lli(i)==(' '-'0') ) lli(i)=0;    
        if( ssi(i)==(' '-'0') ) ssi(i)=0;
        if( lli(i)<0 || lli(i)>9 || ssi(i)<0 || ssi(i)>9 )
          throw( AipsError("malformed data record") ); 
      };
      uInt s=svn[ichan]-1,irec=iObs(s);
      // resize array of records (if too many)
      if( irec >= sat_[s].nelements() )
      {
        sat_[s].resize( irec+1000,True );
        if( knownSize )
          fprintf(stderr,"%swarning: more records for SVN %d than indicated by header\n",errhead,s+1);
        if( debug_level>0 )
          cerr<<errhead<<"resizing to "<<sat_[s].nelements()<<" samples\n";
      };
// At this point, Bob does a bunch of checks for euro-Rinex and data validity.
// I've realized we ought to defer this until a later stage, so all this code
// is commented out (see commented blocks at end of file).
// Instead, we'll just read the data and stuff it into the table.
      sat_[s].iep(irec)=iSlots;
      for( Int i=0; i<5; i++ ) {
        sat_[s].d(irec,i)   = d(i);
        sat_[s].lli(irec,i) = lli(i);
        sat_[s].ssi(irec,i) = ssi(i);
      };
      iObs(s)++;
    }; // for( ichan )
    
    iSlots++;
  };
  
// everything caught here is a RINEX format error, so re-throw the exception,
// adding filename and line number information
  } catch( AipsError excp ) { 
    fclose(f);
    for( uInt s=0; s<NUM_GPS; s++ ) 
      sat_[s].resize(0);
    mjd_.resize(0);
    char str[32];
    sprintf(str," at line %d",lineNum);
    throw( AipsError( errHead+excp.getMesg()+str ) );
  }
  fclose(f);
// finished reading file, do some accounting
  if( debug_level>0 )
    fprintf(stderr,"%sread %d epochs\n",errhead,iSlots);
// shrink arrays, if needed
  if( iSlots < mjd_.nelements() )
    mjd_.resize( iSlots,True );
  for( uInt s=0; s<NUM_GPS; s++ )
  {
    if( debug_level>0 )
      fprintf(stderr,"%sSVN %d: read %d records\n",errhead,s+1,iObs(s));
    if( iObs(s) < sat_[s].nelements() ) 
      sat_[s].resize( iObs(s),True );
    sat_[s].setEpochs(mjd_);
  }

  h.nep=iSlots;
  if( h.ep_first != mjd_(0) )
  {
    if( h.ep_first )
      fprintf(stderr,"%swarning: time of first obs. differs from header\n",errhead);
    h.ep_first=mjd_(0);
  }
  if( h.ep_last != mjd_(iSlots-1) )
  {
    if( h.ep_last )
      fprintf(stderr,"%swarning: time of last obs. differs from header\n",errhead);
    h.ep_last=mjd_(iSlots-1);
  }
  hdr_=h;
  
  return iSlots;  
}

// -----------------------------------------------------------------------
// getTEC
// Returns all available TEC samples, sorted by time.
// -----------------------------------------------------------------------
uInt RINEX::getTEC ( Vector<Double> &mjd,         // epochs
                   Vector<Int>   &svn,         // SVNs
                   Vector<Double> &tec,        // TEC
                   Vector<Double> &sigTec,     // sigma TEC
                   Vector<Double> &sigTec30,   // 30-minute mean sigma TEC
                   Vector<Int>    &domain,     // domain number
                   GPSDCB &dcb )              // DCB accessor
{
  uInt tot_ep=mjd_.nelements();
//  Vector<MVDirection> dirsat[NUM_GPS];   // direction to each sat at each observation point
  Matrix<Int> xrec(NUM_GPS,tot_ep,-1);  // index, epoch -> rec. num
  Matrix<Int> xdir(NUM_GPS,tot_ep,-1);  // index, epoch -> direction num
  uInt nout=0;
  
  // Compute domains of GPS data, and spline the orbits to required time slots.
  // In the same loop, build up indices (epochs->records,ephemeris)
  for( uInt s=0; s<NUM_GPS; s++ ) 
  {
    RINEXSat &sat(sat_[s]);
    if( !sat.nelements() )
      continue;
    // get TGD for this SVN
    Vector<Float> rms;
    Vector<Float> tgd( dcb.getTgd(rms,Vector<Float>(1,(Float)hdr_.ep_first),s+1) );
    if( !tgd.nelements() )
    {
      char str[128];
      sprintf(str,"Missing P1-P2 DCB data for SVN %d",s+1);
      throw( AipsError(str) );
    }
    sat.setTGD( tgd(0)*1e-9 );
    // compute domains for this SVN
    Int nep = sat.numSamples();
    if( nep<0 ) 
    {
      nep = sat.computeDomains();
      if( !nep )
        continue;
    }
    // now, build up vector of valid epochs for this SVN
    Vector<Double> t1(nep);
    uInt j=0;
    for( uInt i=0; i<sat.nelements(); i++ )
    {
      if( sat.idom(i)>=0 )           
      {
        xrec(s,sat.iep(i))=i;   // set epoch->irec index
        xdir(s,sat.iep(i))=j;   // set epoch->ieph index
        t1(j++)=sat.mjd(i);      // add observed epoch
        nout++;
      }
    }
  }
// resize the output arrays
  mjd.resize(nout);    
  svn.resize(nout);    
  tec.resize(nout);    
  sigTec.resize(nout);    
  sigTec30.resize(nout);    
  domain.resize(nout);
//  dir.resize(nout);
  uInt iout=0; // current output record number
// fill the arrays with data from each of the sattelites
  for( uInt iep=0; iep<mjd_.nelements(); iep++ ) // loop over epochs
  {
    Double t=mjd_(iep);
    for( uInt s=0; s<NUM_GPS; s++ ) // loop over sattelites
    {
      if( xrec(s,iep) >=0  )     // is there a valid datum here?
      {
        mjd(iout) = t;
        svn(iout) = s+1;
        tec(iout) = sat_[s].getTEC(sigTec(iout),sigTec30(iout),xrec(s,iep));
        domain(iout) = sat_[s].idom(xrec(s,iep));
        if( debug_level>1 ) 
          cerr<<MVTime(t)<<" "<<s+1<<" "<<tec(iout)<<" "<<sigTec(iout)<<" "
              <<sigTec30(iout)<<endl;
        iout++;                
      }
    }
  }
  return nout;
}


} //# NAMESPACE CASA - END

