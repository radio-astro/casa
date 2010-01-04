//# IonosphModelPIM.cc: 
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

#include <stdio.h>
#include <ctype.h>
#include <setjmp.h>
#include <signal.h>
#include <casa/System/Aipsrc.h>    
#include <casa/Exceptions/Error.h>
#include <measures/Measures/MCPosition.h>
#include <measures/Measures/MCDirection.h>
#include <measures/Measures.h>
#include <measures/Measures/MEpoch.h>
#include <casa/System/ProgressMeter.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/RefRows.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/LogiVector.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Slice.h>

#include <ionosphere/Ionosphere/IonosphModelPIM.h>
#include <casa/iostream.h>

namespace casa { //# NAMESPACE CASA - BEGIN

const char * PIM_ParameterNames[] = { "F10.7","Ap","IMF Bz" };

 LogIO IonosphModelPIM::os( LogOrigin("PIM") );
        
// -----------------------------------------------------------------------
// FORTRAN declarations (PIM)
// These are the wrapper subroutines providing us with a PIM
// interface.
// All strings (that is, pathnames) are passed indirectly via common 
// blocks, since this seems to be more portable (across FORTRAN 
// compilers) than passing them as subroutine parameters.
// -----------------------------------------------------------------------
extern "C" 
{
  void faptokp_(const float *,float *);
  void fpiminit_(void);
  void fpimsetalt_(const float*,const int*);
  void fpimpreload_();
  void fpimload_(int*,int*,int*,double*,const double*,const double*);
  void fslantedp_(const int*,const int*,const double*,const float*,const float*,
                  const float*,const float*,int*,
                  Float*,float*,Float*,Float*);
  extern struct { char pusu[80], pmid[80], plow[80], plme[80], paws[80]; } dpath_;
  extern struct { char pkpf[80], pimf[80], pgdb[80]; } pimpaths_;
};

// -----------------------------------------------------------------------
// FORTRAN error handling
// Ugly, ugly, ugly... FUCKTRAN poisons everything it touches.
// We need to intercept FORTRAN STOP statements and I/O errors, 
// because otherwise PIM cheerfully terminates the process the instant
// something goes wrong.
// The bad news is that all this is highly unportable. The good news:
// if it doesn't work for a specific compiler, the only consequence
// is dumb FORTRAN-style "print message and die" error handling.
// 
// Implementation details for specific compilers:
// 
// g77: STOP calls s_stop, and fatal errors ultimately call sig_die,
//      so I provide custom versions here.
//
// Sun's f77: STOP calls __s_stop. Fatal errors: yet to figure out.
//
// Other compilers: yet to figure out.
// -----------------------------------------------------------------------
static String fortranModule,fortranErr;
static jmp_buf fortran_jmp;

// reportFortranError
// This is the common error handler.
static void reportFortranError ( const String &message = String("") )
{
  String err("FORTRAN module");
  if( fortranModule.length() )
  {
    err+=" [";
    err+=fortranModule;
    err+="]";
  }
  err+=": ";
  err+=message;
// It would be nice to throw an exception at this point, but apparently
// exceptions don't work after FORTRAN (that is, if C++ calls FORTRAN, 
// and FORTRAN does a STOP, which calls this procedure, then exceptions thrown
// here are not caught). So instead of a nice and clean: 
//    throw( AipsError(err) );
// we have to do the ugly:
  fortranErr = err;
  longjmp(fortran_jmp,1);
}

// Error handler for the replaced stop statements in the FORTRAN PIM code

extern "C" {
void sig_die ( char *str,int kill ) // fatal error
{
  fprintf(stderr,"Caught fatal error in FORTRAN [%s]: %s\n",fortranModule.chars(),str);
  if( kill )
  {
    char s[256];
    sprintf(s,"FATAL ERROR (%s)",str);
    reportFortranError(s);
  }
}

} // extern "C"

// -----------------------------------------------------------------------
// fortranAbortHandler
// This handler is called when a SIGABRT is raised inside FORTRAN 
// code. Note that g77 overrides this signal, so this routine 
// remains idle. I've left it in for the sake of other compilers.
// -----------------------------------------------------------------------
void fortranAbortHandler( int )
{
  fprintf(stderr,"Caught SIGABRT in FORTRAN [%s]\n",fortranModule.chars());
  reportFortranError("aborted");
}

// Define a sigaction structure for the signal handler
static struct sigaction sigact_save,sigact_abrt;

// -----------------------------------------------------------------------
// preFortran
// Prepares for a call into FORTRAN:
// Saves the module name, and saves the position for a longjmp()
// (in case of a FORTRAN STOP/abort -- see routines above). Throws an 
// exception if a the longjmp() occurs.
// This has to be a macro, otherwise setjmp()/longjmp() won't work.
// We also set up a signal handler for SIGABRT, though this is totally
// unnecessary for g77 (since we override sig_die instead).
// -----------------------------------------------------------------------
#define preFortran(module_name) { \
  fortranModule = module_name; \
  if( setjmp(fortran_jmp) ) \
    os<<fortranErr<<LogIO::EXCEPTION; \
  sigaction(SIGABRT,&sigact_abrt,&sigact_save); \
  }

// -----------------------------------------------------------------------
// postFortran
// Cleans up after a FORTRAN call:
// Restores the original SIGABRT behaviour.
// -----------------------------------------------------------------------
static void postFortran ()
{
  sigaction(SIGABRT,&sigact_save,NULL);
}
    

// -----------------------------------------------------------------------
// timeToPIM ()
// Helper function to convert a MJD into a YYDDD decimal 
// representation (two-digit year+day of year), and a UT, in hours.
// Returns the YYDDD integer, and stores the hours into "ut".
// (This ugly YYDDD cruft is relied on throughout PIM...)
//
// TODO: check the time handling...
// -----------------------------------------------------------------------
static int timeToPIM (Double &ut,Double mjd)
{
  MVTime tm(mjd);
  ut=(tm.day()-floor(tm.day()))*24;
  return (tm.year()%100)*1000+tm.yearday();
}

// -----------------------------------------------------------------------
// setPath
// helper function to concatenate two C strings into one FORTRAN string
// -----------------------------------------------------------------------
static void setPath (char *fstr,size_t fstr_size,
                    const char *base,const char *dbpath)
{
  strcpy(fstr,base);
  strcat(fstr,dbpath);
  for( size_t i=strlen(fstr); i<fstr_size; i++ )
    fstr[i]=' ';
}



// constants for option names
const String 
    OPT_SLANT_SEP("slant_sep"),
    OPT_IMF_STRAT("imf_strategy"),
    OPT_F107("f107"),
    OPT_AP("ap"),
    OPT_BY("by"),
    OPT_BZ("bz");
// fallback strategies for missing IMF data
const char 
      IMF_INTERPOLATE = 'I',
      IMF_FIX_NORTH = 'F',
      IMF_RUN_TWICE = 'R',
      IMF_ABORT = 'A';

    
const Float defaultSlantSep = .1;    
    

// -----------------------------------------------------------------------
// Constructor
// Initializes the PIM modeller
// -----------------------------------------------------------------------
IonosphModelPIM::IonosphModelPIM ()
{
// setup the abort sigaction (do only once)
  static Bool sigaction_setup = False;
  if( !sigaction_setup )
  {
    sigaction_setup = True;
    sigact_abrt.sa_handler = fortranAbortHandler;
    sigemptyset(&sigact_abrt.sa_mask);
    sigact_abrt.sa_flags = SA_RESETHAND;
  }
// setup default options and option description record
  option_desc = Record(RecordInterface::Variable);
  option_desc.define("slant_sep",defaultSlantSep); 
  option_desc.define("imf_strategy","Interpolate");
  options = option_desc;
  option_desc.define("f107","none");
  option_desc.define("ap","none");
  option_desc.define("by","none");
  option_desc.define("bz","none");
  option_desc.setComment("slant_sep","Minimum unique slant separation, in degrees");
  option_desc.setComment("imf_strategy","Strategy for handling missing IMF data: (I)nterpolate, (F)ix north, (R)un twice");
  option_desc.setComment("f107","Fixed F10.7 value (only for next calculation) [float]");
  option_desc.setComment("ap","Fixed Ap value (only for next calculation) [float]");
  option_desc.setComment("by","Fixed IMF By value (only for next calculation) [+/-1]");
  option_desc.setComment("bz","Fixed IMF Bz value (only for next calculation) [+/-1]");

// get the PIM DB path. Default is $AIPSROOT/$AIPSARCH/PIMDB
  String dbpath;
  if( !Aipsrc::find(dbpath,"ionosphere.pim.path") )
  {
    dbpath+=Aipsrc::aipsArch();
    dbpath+="/PIMDB";
  }
  os<<"PIM database path is "<<dbpath<<LogIO::POST;
// Set paths in common blocks.
#define fsetpath(v,path) setPath(v,sizeof(v),dbpath.chars(),path)
  fsetpath(pimpaths_.pkpf,"/noaa_dat/");
  fsetpath(pimpaths_.pimf,"/noaa_dat/IMF24.dat");
  fsetpath(pimpaths_.pgdb,"/cgmdb/");
  fsetpath(dpath_.pusu,"/usudb/");
  fsetpath(dpath_.plow,"/llfdb/");
  fsetpath(dpath_.pmid,"/mlfdb/");
  fsetpath(dpath_.plme,"/lmedb/");
  fsetpath(dpath_.paws,"/ursidb/");
#undef fsetpath
// do FORTRAN initialization stuff (w/datafile names)
  preFortran("PIM:init");
  fpiminit_();
  postFortran();
}

// -----------------------------------------------------------------------
// IonosphModelPIM::~IonosphModelPIM ()
// Destructor
// -----------------------------------------------------------------------
IonosphModelPIM::~IonosphModelPIM ()
{
}

// -----------------------------------------------------------------------
// setAlt ()
// Stores the altitude grid, and also passes it into FORTRAN
// -----------------------------------------------------------------------
void IonosphModelPIM::setAlt( const Float *alt,uInt nalt )
{
  IonosphModel::setAlt(alt,nalt);
  preFortran("PIM:setalt");
  fpimsetalt_(alt,(int*)&nalt); 
  postFortran();
}

void IonosphModelPIM::setAlt( const Vector<Float> &alt )
{
  Bool deleteIt;
  const Float *ptr=alt.getStorage(deleteIt);
  setAlt(ptr,alt.nelements());
  alt.freeStorage(ptr,deleteIt);
}

// -----------------------------------------------------------------------
// readApF107 
// Reads the Kp/Ap/F107 table, computes the 27-day F10.7 average,
// and the 24h-averaged Ap (3 hour sampling)
// Inputs: 
//   mjd1,mjd2   range of dates for which data is required
//
// Outputs:
//   sf107       the 27-day F10.7 average, starting with mjd1-1, and
//               ending with mjd2+1.
//               (ndays+2 elements)
//   sap         the 24h running 3-hourly Ap average, starting with
//               00-03UT mjd1-1, and ending with 21-24UT mjd2+1.
//               ((ndays+2)*8 elements)
//
// The pointer version will assume arrays are big enough to hold
// all the data (use sizeF107() and sizeAp() to find out).
// The Vector version will resize the vectors as needed.
// -----------------------------------------------------------------------

void IonosphModelPIM::readApF107( Double *p_sf107,Double *p_sap,Int mjd1,Int mjd2 )
{
  Int n107 = sizeF107(mjd1,mjd2),
      nap = sizeAp(mjd1,mjd2);
  Vector<Double> sf107(IPosition(1,n107),p_sf107,SHARE),
                sap(IPosition(1,nap),p_sap,SHARE);
  readApF107(sf107,sap,mjd1,mjd2);
}

void IonosphModelPIM::readApF107( Vector<Double> &sf107,Vector<Double> &sap,Int mjd1,Int mjd2 )
{
// get the KpApF107 DB path. Default is $AIPSROOT/data/geodetic/KpApF107
  String dbpath;
  if( !Aipsrc::find(dbpath,"ionosphere.kpapf107.path") )
    dbpath=Aipsrc::aipsRoot() + "/data/geodetic/KpApF107";
// open the table  
  Table tab(dbpath);
// get MJD of first row
  Int tab_mjd0 = tab.keywordSet().asInt("MJD0");
  if( !tab_mjd0 ) 
    os<<"The Kp/Ap/F107 table is invalid"<<LogIO::EXCEPTION;
// compute row indices
  Int ndays = mjd2-mjd1+1,
      row1 = mjd1-tab_mjd0, row2 = mjd2-tab_mjd0;  
  if( row1<27 || row2>=(Int)tab.nrow()-1 )
    os<<"No Kp/Ap/F107 data available for given range of dates"<<LogIO::EXCEPTION;
// attach to required columns
  ROScalarColumn<Int>   mjd_col(tab,"MJD"),fqual_col(tab,"FQUAL");
  ROScalarColumn<Float> f107_col(tab,"F107");
// check that data for all days in range is available  
  RefRows r(row1-27,row2+1);
  Vector<Int> mjd( mjd_col.getColumnCells(r) );
  if( anyEQ(mjd,0) ) 
    os<<"Missing some Kp/Ap/F107 data for given dates"<<LogIO::EXCEPTION;
// check that MJDs are correct
  Vector<Int> mjdref(ndays+28);
  indgen(mjdref,mjd1-27,1);
  if( anyNE(mjd,mjdref) )
    os<<"Kp/Ap/F10.7 table is corrupt (MJD mismatch)"<<LogIO::EXCEPTION;
// compute 27-day F10.7 average
  Vector<Float> f107( f107_col.getColumnCells(r) );
  Vector<Int>   fqual( fqual_col.getColumnCells(r) );
  if( anyNE(fqual,0) )
    os<<LogIO::WARN<<"Non-zero F10.7 flag in data"<<LogIO::POST<<LogIO::NORMAL;
  if( sf107.nelements() != (uInt)ndays+2 )
    sf107.resize(ndays+2);
  for( Int i=0; i<ndays+2; i++ )
    sf107(i) = sum( f107( Slice(i,27) ) )/27.0;
  os<<"Loaded F10.7; range: "<<min(sf107)<<" to "<<max(sf107)<<endl;
// now, compute 24-hour running Ap average
  ROArrayColumn<Int>    ap3_col(tab,"Ap_3hr");
  r = RefRows(row1-2,row2+1);
  Array<Int> ap3_ar( ap3_col.getColumnCells(r) );
  // reform into vector 
  Vector<Int> ap3( ap3_ar.reform( IPosition(1,(ndays+3)*8) ));
// compute 24h running average
  if( sap.nelements() != (uInt)(ndays+2)*8 )
    sap.resize((ndays+2)*8);
  Double s0 = sum( ap3( Slice(0,8) ) );
  for( Int i=0; i<(ndays+2)*8; i++ ) 
  {
    s0 += ap3(i+8) - ap3(i);
    sap(i) = s0/8;
  }
  Float minsap=min(sap),maxsap=max(sap);
  os<<"Loaded 3-hr Ap; range: "<<minsap<<" to "<<maxsap<<endl;
  Float minkp,maxkp;
  faptokp_(&minsap,&minkp);  
  faptokp_(&maxsap,&maxkp);  
  os<<"Corresponding 3-hr Kp: "<<minkp<<" to "<<maxkp<<LogIO::POST;
}


// -----------------------------------------------------------------------
// readIMF
// Obtains the IMF By and Bz values in the middle of the specified
// time interval.
// If IMF data is missing, uses the strategy parameter to figure out
// what to do. Throws an exception if unable to handle. Returns
// True if valid By/Bz values are returned, or False if the RUN_TWICE
// strategy was selected and data is missing.
// -----------------------------------------------------------------------
Bool IonosphModelPIM::readIMF( Float &by,Float &bysig,Float &bz,Float &bzsig,
                  Double mjd1,Double mjd2 )
{
// get the KpApF107 DB path. Default is $AIPSROOT/data/geodetic/KpApF107
  String dbpath;
  if( !Aipsrc::find(dbpath,"ionosphere.imf.path") )
    dbpath=Aipsrc::aipsRoot() + "/data/geodetic/IMF";
// open the table  
  Table tab(dbpath);
// setup fallback strategy
  String strat; // default strategy is interpolate
  char imf_strategy = 0;
  if( options.dataType(OPT_IMF_STRAT) == TpString ) {
    options.get(OPT_IMF_STRAT,strat);
    imf_strategy = toupper( strat.chars()[0] );
  }
// get MJD of first row
  Int tab_mjd0 = tab.keywordSet().asInt("MJD0");
  if( !tab_mjd0 ) 
    os<<"The IMF table is invalid"<<LogIO::EXCEPTION;
// compute midpoint
  Double mjd_mid = (mjd1+mjd2)/2;
  String mjd_str = MVTime(mjd_mid).string(MVTime::YMD_ONLY);
  Int irec = (Int)mjd_mid-tab_mjd0;
// If date is out of range, see which fallback strategy to employ.
  if( irec<0 || irec>=(Int)tab.nrow() ) 
  {
    os<<LogIO::WARN<<mjd_str<<" is outside range of available IMF data.\n";
    switch( imf_strategy ) {
      case 'A': //IMF_ABORT:
        os<<"No fallback strategy selected, aborting PIM."<<LogIO::EXCEPTION;
        return False;
        
      case 'F': //IMF_FIX_NORTH: 
        os<<"Selected fallback strategy is to assume Bz North."<<LogIO::NORMAL;
        by = bz = 1;
        bysig = bzsig = 0;
        return True;
        
      case 'R':
        os<<"Selected fallback strategy is to run PIM twice for Bz North/South."<<LogIO::NORMAL;
        return False;

      case 'I': // the default assumes IMF_INTERPOLATE
        os<<"Cannot interpolate when date is out of range.\n"
            "Try setting a different IMF fallback strategy."<<LogIO::EXCEPTION;
        return False;
        
      default:
        os<<"Unknown missing-IMF strategy \""<<strat<<"\""<<LogIO::EXCEPTION;
    }
  }
// get the data for this date from the table
  ROScalarColumn<Int>   mjd_col(tab,"MJD");
  ROScalarColumn<Float>  by_col(tab,"By"),bysig_col(tab,"sBy"),
                        bz_col(tab,"Bz"),bzsig_col(tab,"sBz");
// if MJD in row is non-0, then data is available. Load it and return.
  if( mjd_col(irec) ) 
  {
    if( mjd_col(irec) != (Int)mjd_mid ) // sanity check
      os<<"IMF table is corrupt (MJD mismatch)"<<LogIO::EXCEPTION;
    by = by_col(irec);
    bz = bz_col(irec);
    bysig = bysig_col(irec);
    bzsig = bzsig_col(irec);
  }
  else 
  {
    os<<LogIO::WARN<<"Missing IMF data for "<<mjd_str<<endl;
// At this point, we have a missing-data situation. Handle it according
// to selected strategy. First the two easy ones:
    if( imf_strategy == 'A'  ) {
      os<<"No fallback strategy selected, aborting PIM."<<LogIO::EXCEPTION;
      return False;
    } else if( imf_strategy == 'F' ) {
      os<<"Selected fallback strategy is to assume Bz North."<<LogIO::NORMAL;
      by = bz = 1;
      bysig = bzsig = 0;
    } else if( imf_strategy == 'R' ) {
      os<<"Selected fallback strategy is to run PIM twice for Bz North/South."<<LogIO::NORMAL;
      return False;
    } else if( imf_strategy == 'I' ) { // Else interpolate is assumed. Find nearest dates that actually have data
      os<<"Selected fallback strategy is to interpolate."<<endl;
      Vector<Int> mjd_vec( mjd_col.getColumn() );
      Int i1,i2;
      for( i1=irec-1; i1>=0; i1-- )
        if( mjd_vec(i1) )
          break;
      for( i2=irec+1; i2<(Int)tab.nrow(); i2++ )
        if( mjd_vec(i2) )
          break;
      // failed to find?
      if( i1<=0 || i2>=(Int)tab.nrow() ) 
      {
        os<<"Can't interpolate: no data."<<LogIO::EXCEPTION;
        return False;
      }
      os<<"Interpolating between "<<
          MVTime(tab_mjd0+i1).string(MVTime::YMD_ONLY)<<" and "<<
          MVTime(tab_mjd0+i2).string(MVTime::YMD_ONLY)<<LogIO::NORMAL;
      // interpolate
      Double x1 = i1-irec, x2=i2-irec,
            x = mjd_mid-(Int)mjd_mid;
      by    = by_col(i1)+(by_col(i2)-by_col(i1))*(x-x1)/(x2-x1);
      bz    = bz_col(i1)+(bz_col(i2)-bz_col(i1))*(x-x1)/(x2-x1);
      bysig = bysig_col(i1)+(bysig_col(i2)-bysig_col(i1))*(x-x1)/(x2-x1);
      bzsig = bzsig_col(i1)+(bzsig_col(i2)-bzsig_col(i1))*(x-x1)/(x2-x1);
    }
    else {  // unknown strategy
      os<<"Unknown missing-IMF strategy \""<<strat<<"\""<<LogIO::EXCEPTION;
    }
  }
// Log the By/Bz values
  os<<"IMF By = "<<by<<" +/- "<<bysig<<
      ";  Bz = "<<bz<<" +/- "<<bzsig<<LogIO::POST;
  return True;
}

// -----------------------------------------------------------------------
// getED
// Go off and do the computations, returning densities...
// -----------------------------------------------------------------------
Block <EDProfile> IonosphModelPIM::getED( LogicalVector &isUniq,
                                          const SlantSet &slants,
                                          const Vector<uInt> &sidx,
                                          const Vector<uInt> &suniq )
{
// PIM needs new databases to be loaded at each U.T., before profiles for 
// that time slot can be derived. Loading is expensive; deriving profiles is
// cheap. Therefore, we need the slants to be sorted into unique time slots, 
// by ascending U.T. Hence we rely on sidx and suniq.
  uInt nslants=slants.nelements(),nut=suniq.nelements();
  
  if( nslants!=sidx.nelements() )
    os<<"PIM confused: slants array not sorted"<<LogIO::EXCEPTION;

  uInt nsamples=altgrid.nelements();
  if( !nsamples )
    setAlt(pimDefaultAltGrid,
            nsamples=sizeof(pimDefaultAltGrid)/sizeof(pimDefaultAltGrid[0]));
  
  Block <EDProfile> edp(nslants);
  isUniq.resize(nslants);
  isUniq.set(False);
  
  Double mjd1=slants[sidx(0)].mjd(),
        mjd2=slants[sidx(nslants-1)].mjd();

  if( Ionosphere::debug_level )
    cerr<<"Options were: "<<options.description()<<endl;
// load IMF values
  Float by,bysig,bz,bzsig;
  Bool fix_by = options.isDefined(OPT_BY),
       fix_bz = options.isDefined(OPT_BZ);
  if( !fix_by || !fix_bz ) { // load if not fixed
    if( !readIMF(by,bysig,bz,bzsig,mjd1,mjd2) )
      os<<"IMF fallback strategy RUN_TWICE not yet supported"<<LogIO::EXCEPTION;
  }
  if( fix_by ) {  // fixed value provided?
    options.get(OPT_BY,by);
    bysig = 0;
    os<<"Using fixed IMF By = "<<by<<LogIO::POST;
  }
  if( fix_bz ) {  // fixed value provided?
    options.get(OPT_BZ,bz);
    bzsig = 0;
    os<<"Using fixed IMF Bz = "<<bz<<LogIO::POST;
  } 
// AP values are really only needed when Bz is south 
  Bool bz_south = (bz+bzsig/2)<=0;
  
  Vector<Double> sf107( sizeF107(mjd1,mjd2) ),
                sap ( sizeAp(mjd1,mjd2) );
  
// load Ap and F10.7 tables if ultimately required
  Bool fix_f107 = options.isDefined(OPT_F107),
      fix_ap = options.isDefined(OPT_AP);
  if( !fix_f107 || (!fix_ap && bz_south) )
    readApF107(sf107,sap,mjd1,mjd2);
// fixed F10.7 value?
  if( fix_f107 ) {
    Float val;
    options.get(OPT_F107,val);
    sf107 = val;
    os<<"Using fixed F10.7 = "<<val<<LogIO::POST;
  }
// figure out Ap values
  if( bz_south )    // Bz is south - Ap matters
  {
    if( fix_ap ) 
    {
      Float val,kp;
      options.get(OPT_AP,val);
      sap = val;
      faptokp_(&val,&kp);  
      os<<"Using fixed Ap = "<<val<<" (Kp = "<<kp<<")"<<LogIO::POST;
    }
  } 
  else // Bz is north - that predetermines Ap 
    {
    Float val=4.0,kp;
    sap = val;
    faptokp_(&val,&kp);  
    os<<"IMF Bz>0 implies Ap = "<<sap(0)<<" (Kp = "<<kp<<")"<<LogIO::POST;
  }
  
// clear one-time options (i.e. those that must be specified anew
// each run)
  
  const String transients[] = { OPT_BY,OPT_BZ,OPT_F107,OPT_AP };
  for( uInt i=0; i<sizeof(transients)/sizeof(transients[0]); i++ )
    if( options.isDefined(transients[i]) )
      options.removeField(transients[i]);
  
  if( Ionosphere::debug_level )
    cerr<<"Options now: "<<options.description()<<endl;

// preload global PIM datafiles (i.e., those loaded for the whole
// observational time span)
  Double utime,utime1;
  int yrdy=timeToPIM(utime,mjd1),yrdy1=timeToPIM(utime1,mjd2); // start/end times
  if( Ionosphere::debug_level )
    fprintf(stderr,"fpimpreload: %d %f - %d %f\n",
        yrdy,(double)utime,yrdy1,(double)utime1);
  preFortran("PIM:preload");
  fpimpreload_(); // call FORTRAN
  postFortran();
    
// To save time, we only compute significantly separated slants.
// Loop over slants here to figure out which will be truly computed.
// If slant i is close enough to a truly computed slant i0, set 
// copy_from(i) = i0. 
// Truly computed slants will have their copy_from index <0.
  Float sep_deg = defaultSlantSep;
  if( options.isDefined(OPT_SLANT_SEP) )
    options.get(OPT_SLANT_SEP,sep_deg);
  
  Vector<Int> copy_from(nslants,-1);
  uInt ncomputed=0;
  const Double sep_rad = sep_deg*C::degree,
              sep_mjd = sep_rad/2*C::pi;
  for( uInt ut=0; ut<nut; ut++ ) 
  {
    uInt sl_first=suniq(ut), // figure out first and last slant for this time slot
         sl_last=ut+1<nut ? suniq(ut+1)-1 : sidx.nelements()-1;
    for( uInt sl=sl_first; sl<=sl_last; sl++ )
    {
      uInt idx=sidx(sl),sl1;
      // is there a near-enough truly computed slot?
      for( sl1=0; sl1<sl; sl1++ )
      {
        uInt idx1=sidx(sl1);
        if( copy_from(idx1)<0 &&
            abs(slants[idx].mjd()-slants[idx1].mjd())   < sep_mjd &&
            slants[idx].dir().separation(slants[idx1].dir()) < sep_rad &&
            slants[idx].pos().separation(slants[idx1].pos()) < sep_rad )
        {
          copy_from(idx) = idx1;
          break;
        }
      }
      if( sl1>=sl )  // none, so do a real compute for this slant
      {
        isUniq(idx) = True;
        ncomputed++;
      }
    }
  }
// initialize a progress meter (if >1 slant requested)
  uInt icomputed=0;
  ProgressMeter prog_meter(1,max(ncomputed,2U),
      "Computing PIM profiles","PIM","1",String::toString(ncomputed));
  os<<"Minimal significant slant separation is "<<sep_deg<<" degrees.\n";
  os<<"Running PIM for "<<ncomputed<<" slants."<<LogIO::POST;
    
// outer loop over time slots
  for( uInt ut=0; ut<nut; ut++ ) 
  {
    Bool pim_loaded;
    pim_loaded = False;
    int year,day;
    
    uInt sl_first=suniq(ut), // figure out first and last slant for this time slot
         sl_last=ut+1<nut ? suniq(ut+1)-1 : sidx.nelements()-1;
// inner loop over slants within this time slot
    for( uInt sl=sl_first; sl<=sl_last; sl++ )
    {
      uInt idx=sidx(sl);
      Int idx1=copy_from(idx);
      edp[idx].set_slant( slants[idx] );
      // can be copied from an already-computed slant? Copy and go to next.
      if( idx1 >=0 )
      {
        edp[idx].copyData( edp[idx1] );
        if( Ionosphere::debug_level )
          fprintf(stderr,"slant %d %s will be copied from %d\n",
              sl,edp[idx].slant().string().chars(),idx1);
        continue;
      }
// Else it needs to be computed. Load PIM for this time slot then.
// load PIM for this time slot 
      if( !pim_loaded )
      {
        yrdy=timeToPIM(utime,slants[idx].mjd());
        preFortran("PIM:load");
        Bool dum; // for getStorage() - Ger says OK because arrays are guaranteed contigious.
        fpimload_(&yrdy,&year,&day,&utime,sf107.getStorage(dum),sap.getStorage(dum)); // call FORTRAN 
        postFortran();
        pim_loaded = True;
        if( Ionosphere::debug_level )
        {
          fprintf(stderr,"fpimload: %d %d %d %f; ",
                            yrdy,year,day,utime);
//          cerr<<sf107<<"; "<<sap<<endl;
        }
      }

      edp[idx].set_alt( altgrid );
// since PIM assumes that slants start at altitude 0, we need to
// correct the observatory position accordingly
      Double altitude = MPosition::Convert(slants[idx].mpos(),(MPosition::Ref(MPosition::WGS84)))().getValue().getLength("m").getValue();
      Double range = altitude/sin(slants[idx].dir().getLat());
// convert direction and position into ITRF
      MVDirection vdir( 
        (MDirection::Convert(slants[idx].mdir(),
            (MDirection::Ref(MDirection::ITRF)))()).getValue() );
// now, go back along this direction to reach altitude 0
      MVPosition vobs = MVPosition( slants[idx].pos().getValue()-range*vdir.getValue() );
      Vector <Double> lonlat=vobs.getAngle().getValue("deg");

      if( Ionosphere::debug_level>2 )
        cerr<<"altitude: "<<altitude<<endl<<
              "range: "<<range<<endl<<
              "New position: "<<vobs<<" "<<lonlat<<endl;

      Vector <Double> azel=slants[idx].azElDeg();
      
      Float saz=azel(0),sel=azel(1),slon=lonlat(0),slat=lonlat(1);
// get the ed/lat/lon vectors' storage pointers, to pass to FSLANTEDP
      edp[idx].resize(nsamples); 
      Float *ed=edp[idx].ed_storage(),
            *lat=edp[idx].lat_storage(),
            *lon=edp[idx].lon_storage(),
            *rng=edp[idx].rng_storage();
      if( Ionosphere::debug_level )
        fprintf(stderr,"fslantedp: slant %d %s\n",
            sl,slants[idx].string().chars());
  // call the FORTRAN wrapper to compute this slant's profile
      preFortran("PIM:slantedp");
      fslantedp_(&year,&day,&utime,
          &saz,&sel,&slat,&slon,
          (int*)&nsamples,ed,rng,lat,lon);
      if( Ionosphere::debug_level>1 )
        cerr<<"profile: "<<edp[idx].tec()<<" / "<<edp[idx].ed()<<endl;
      postFortran();

// update the progress meter      
      if( ncomputed>1 )
          prog_meter.update(++icomputed);
    }
  }
  
  return edp;
}


} //# NAMESPACE CASA - END

