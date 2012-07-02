
#include <casa/stdio.h>
    
#include <ionosphere/Ionosphere/RINEXSat.h>
#include <casa/BasicSL/Constants.h>    
#include <casa/Quanta/MVTime.h>    
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/Slice.h>
#include <casa/Exceptions/Error.h>
    
namespace casa { //# NAMESPACE CASA - BEGIN

Int RINEXSat::debug_level =0;


// -----------------------------------------------------------------------
// Constructors
// -----------------------------------------------------------------------
RINEXSat::RINEXSat ()
{
  set(0,0,0);
}

RINEXSat::RINEXSat( uInt svn,uInt nobs,Double tsamp )
{
  set(svn,nobs,tsamp); 
}
    
// -----------------------------------------------------------------------
// set
// initializes, and sets up the data structures to a given size
// -----------------------------------------------------------------------
void RINEXSat::set ( uInt svn,uInt n,Double tsamp ) 
{ 
  resize(n); 
  
  svn_=svn; 
  tsamp_=tsamp; 
  idom_=-1;
  tec_=-1;

  numSamples_=-1;
  tgd_=C::dbl_min;
  dom_tmin    = 1/48.;          // default min. domain length: 30 minutes        
  dom_sdmax   = 60 / Rnx_stec;  // default max. stddev 
  dom_sdmmax  = 60 / Rnx_stec;  // default max. 30-min-min stddev.
  dom_jumpmax = 1 / Rnx_stec;   
}

static void resizeMatrix ( Matrix<Double> &m,Int x1,Int y1 )
{
  Int x,y;
  m.shape(x,y);
  Matrix<Double> m1(x1,y1);
  x=min(x,x1);
  y=min(y,y1);
  Slice sx(0,x),sy(0,y);
  if( x && y )
    m1(sx,sy)=m(sx,sy);
  m.reference(m1);
}

static void resizeMatrix ( Matrix<uChar> &m,Int x1,Int y1 )
{
  Int x,y;
  m.shape(x,y);
  Matrix<uChar> m1(x1,y1);
  x=min(x,x1);
  y=min(y,y1);
  Slice sx(0,x),sy(0,y);
  if( x && y )
    m1(sx,sy)=m(sx,sy);
  m.reference(m1);
}

// -----------------------------------------------------------------------
// resize
// Resizes the data arrays to the given size
// -----------------------------------------------------------------------
void RINEXSat::resize( uInt n,Bool copy )
{ 
  iep_.resize(n,copy);
  idom_.resize(n,copy);
  tec_.resize(n,copy);
  
  if( copy ) 
  {
    resizeMatrix(d_,n,5);
    resizeMatrix(lli_,n,5);
    resizeMatrix(ssi_,n,5);
  } 
  else
  {
    d_.resize(n,5); 
    lli_.resize(n,5); 
    ssi_.resize(n,5); 
  }
}


// -----------------------------------------------------------------------
// closeDomain
// Closes domain, adds it to domains array if good enough.
// Returns # of samples, or 0 if domain failed to meet criteria and 
// was discarded.
// -----------------------------------------------------------------------
uInt RINEXSat::closeDomain ( uInt irec0,uInt irec1 )
{
  uInt nrec=irec1-irec0;
  if( debug_level>1 ) 
    fprintf(stderr,"RINEXSat %d: domain of %d samples starting at %s: ",
        svn_,nrec,MVTime(mjd(irec0)).string(6).chars());
  if( mjd(irec1-1)-mjd(irec0) < dom_tmin ) // ignore short domains
  {
    if( debug_level>1 ) 
      fprintf(stderr,"too short\n");
    return 0;
  }
  
// diffph_fit.f incorporated here. Only place it's used, apparently
//BC:   Subroutine to connect the differential carrier phase to the 
//BC:     differential code range for a given domain, compute the overall
//BC:     st.dev. for the fit, and also track the minimum 30-min running
//BC:     st.dev.
//BC:   All variables are in [s] here, conversion back to TECU happens
//BC:     back in the calling program if desired
//BC: 

//BC: Conversion between RINEX SigStr & SNR (using arithmetic
//BC: mean of the SNR ranges for each SigStr value)
  const Double sigsnr[] = { -9.e0, -9.e0, 5.e-1, 2.e0, 4.e0,
                           7.5e0,  3.e1, 7.5e1, 3.e2, 7.5e2 };

//BC:  Loop over all samples for this domain; build arrays of differential
//BC:    Carrier Phase & Code Ranges & associated uncertainties
//BC:    Incorporate Tgd into the Diff GrDel
//BC:  Calculate LS-fitted parameters to these arrays
  Slice S(irec0,nrec);
  Vector<Double> diffph( d().column(L1)(S) - d().column(L2)(S) );
  Vector<Double> diffgr( d().column(P2)(S) - d().column(P1)(S) - tgd_ );
  Vector<uChar> sigstr( ::casa::min( static_cast<Array<uChar> >(ssi().column(L1)(S)),
                                     static_cast<Array<uChar> >(ssi().column(L2)(S)) ) );
  Vector<Double> snrsq(nrec);
  for( uInt i=0; i<nrec; i++ )
    snrsq(i)=pow(sigsnr[sigstr(i)-1],2.0);
  Double sum_snrsq = sum(snrsq);
  
// compute phase delta is weighted average
  Double delph = sum( (diffgr-diffph)*snrsq ) / sum_snrsq;
  
//BC:  Compute standard-deviation of the residual from connecting the
//BC:    PhDel to the GrDel
  Vector<Double> resid( diffgr - ( diffph + delph ) );
  Double sd = stddev(resid);
  
// stddev does not meet criteria, so discard this domain
  if( sd > dom_sdmax )
  {
    if( debug_level>1 ) 
      fprintf(stderr,"SD=%g too high\n",sd*Rnx_stec);
    return 0;
  }
// compute sigma of difference
// assume sd is the real sigma^2 of each individual point,
// and compute the sigma of the weighted mean accordingly
  Double sigma = sd*sqrt(sum(snrsq*snrsq))/sum_snrsq;

//BC:  Compute minimum 30-min running St.Dev.
  uInt n30 = (uInt)( (1/48.0) / (mjd(irec0+1) - mjd(irec0)) ) + 1;
  Double sdm = 10*sd;
  for( uInt i=0; i<=nrec-n30; i++ )
  {
    Double runsd = stddev( resid( Slice(i,n30) ) );
    if( runsd<sdm )
      sdm=runsd;
  }
// end of diffph_fit.f conversion

// now, recompute the error   
  
// min. 30-min running stddev does not meet criteria, so discard this domain
  if( sdm > dom_sdmmax )
  {
    if( debug_level>1 ) 
      fprintf(stderr,"MSD30=%g too high\n",sdm*Rnx_stec);
    return 0;
  }
  
// now that the domain meets all the criteria, add it to our arrays
  if( ndom_ >= domains_.nelements() )
    domains_.resize( domains_.nelements()+20 ); // grow the domains array, if needed
  idom_(S) = ndom_; // mark domain # for all samples
  domains_[ndom_++] = GPSDomain(irec0,irec1,delph,sigma,sdm);

// compute and store the TEC estimates
  tec_(S) = (diffph+delph)*Rnx_stec;
  
  if( debug_level>1 ) 
    fprintf(stderr,"OK\nResult: %4i%15.10f%15.10f%15.10f%15.10f%15.10f\n",
        nrec,tgd_,delph*Rnx_stec,sd*Rnx_stec,sigma*Rnx_stec,sdm*Rnx_stec);
  
  return nrec;
}
        
// -----------------------------------------------------------------------
// computeDomains
// Computes consecustive domains of RINEX data
// -----------------------------------------------------------------------
uInt RINEXSat::computeDomains ()
{
  domains_.resize(ndom_=0);
  if( !nelements() )  // no data for this sattelite
    return 0;
  
// check that TGD is defined
  if( tgd_ == C::dbl_min )
    throw( AipsError("TGD not defined") );
  
// normalize the RINEX columns
  if( numSamples_<0 )
  {
    Vector<Double> l1(d().column(L1)); l1/=Rnx_nu1;
    Vector<Double> l2(d().column(L2)); l2/=Rnx_nu2;
    Vector<Double> p1(d().column(P1)); p1/=Rnx_c;
    Vector<Double> p2(d().column(P2)); p2/=Rnx_c;
    Vector<Double> ca(d().column(CA)); ca/=Rnx_c;
  }
  
  idom_=-1;
  Bool indom=False;
  Double dph0 = d(0,L1) - d(0,L2);
//  Loop over records, testing for starting/stopping a consecutive run
//    of Rinex data, and whether any such found qualify as a "domain"
  uInt irec0=0,irec,nsamp=0;
  for( irec=0; irec<nelements(); irec++ )
  {
// Check for non-consecutive epochs, or a time jump (allowing for 1 leap second)
    if( indom && ( iep_(irec)-iep_(irec-1) > 1 ||
                  mjd(irec)-mjd(irec-1) > tsamp_/(24*3600)+1 ) )
    {
      nsamp+=closeDomain(irec0,irec);
      indom=False;
    }
    
    Bool dataGood=True; // flag: this data sample is good

// Test whether L2 & P2 data is completely missing (as in some
// "Euro-RINEX"), break domain if it is
    if( !lli(irec,L2) && !lli(irec,P2) && !ssi(irec,L2) && !ssi(irec,P2) )
      dataGood=False; // skip this epoch and break off domain, I guess
    else
    {
//  Do the Euro-Rinex stuff here... originally from readrnx2.f
//BC:  Determine whether this is a "Euro-RINEX" file:
//BC:     CA has no LLI, STR
//BC:     L1 has no LLI except when <>0 (then =LLI-4):  (LLI only on FREQ_2)
//BC:     L2 -- all okay
//BC:     P2 has no STR  :         (STR only on L_x)
//BC:     P1 completely missing (for A-S on files)
//BC:   & sometimes
//BC:       L2 also has no STR  :     (STR only on L_1)
//BC:

      Bool euroRinex = ( !lli(irec,CA) && !ssi(irec,CA) &&  !ssi(irec,P2) );

      if( euroRinex ) // that decadent Euro-RINEX again
      {
        lli().row(irec)=lli(irec,L2); // Load LLI elements based on L2
        ssi(irec,P1)=ssi(irec,L1);    // Load STR elements for FREQ_1 based on L1
        if( ssi(irec,L2)>0 )         // Test whether SSI on L2 exists, use L1 if not
          ssi(irec,P2)=ssi(irec,L2);
        else
          ssi(irec,P2)=ssi(irec,L2)=ssi(irec,L1)-2;
        if( lli(irec,L2) >= 4 )      // Determine whethere to use CA for GrDel1
          d(irec,P1)=d(irec,CA);
      }
      else // good ole' American RINEX
      {
        // determine whether to use CA for GrDel1
        if( lli(irec,CA) >= 4 )
        {
          d(irec,P1)=d(irec,CA);
          lli(irec,P1)=lli(irec,CA);
          ssi(irec,P1)=ssi(irec,CA);
        }
      }
      // check if any data is still missing  
      if( !d(irec,L1) || !d(irec,L2) || !d(irec,P1) || !d(irec,P2) )
        dataGood=False;
    }
// finally, if data is good, check for some remaining pathological cases
    if( dataGood )
    {
      if( !allEQ( lli().row(irec) , lli(irec,0) ) )
        throw( AipsError("differing LLIs in same sample") );
      if( ssi(irec,L1) != ssi(irec,P1) || ssi(irec,L2) != ssi(irec,P2) )
        throw( AipsError("differing SSI on L1/P1 or L2/P2 in same sample") );
    }

//   Compute consecutive-time-sample Diff PhDel jump; reset DPH0 for
//     comparison at the next IEP
    Double dph = d(irec,L1) - d(irec,L2);
    Double ddph = abs(dph-dph0);
    dph0 = dph;

// there is data now    
    if( dataGood && !(lli(irec,L1)&1) && ssi(irec,L2)>1 && ddph<=dom_jumpmax ) 
    {
      if( !indom )
      {
        indom=True;
        irec0=irec;
      }
    }
    else // no good data anymore, so close domain, if any
    {
      if( indom )
      {
        indom=False;
        nsamp+=closeDomain(irec0,irec);
      }
    }
  } // for(irec)
  
// at end of data fo this SVN, so close domain, if any
  if( indom )
    nsamp+=closeDomain(irec0,irec);
  
  if( ndom_ < domains_.nelements() )
    domains_.resize(ndom_);
  
  if( debug_level>0 ) 
    fprintf(stderr,"RINEXSat %d: %d domains, %d samples\n",svn_,ndom_,nsamp);

  return numSamples_=nsamp;
}

Double RINEXSat::getTEC (Double &sig,Double &sig30,uInt i) const
{
  uInt d=idom(i);
  if( d<0 )
    return -1;
  sig = domains_[d].sigma();
  sig30 = domains_[d].sigma30();
  return tec(i);
}

} //# NAMESPACE CASA - END

