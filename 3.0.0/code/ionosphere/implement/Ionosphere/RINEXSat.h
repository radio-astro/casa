#ifndef IONOSPHERE_RINEXSAT_H
#define IONOSPHERE_RINEXSAT_H

#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/Block.h>
#include <ionosphere/Ionosphere/GPS.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// -----------------------------------------------------------------------
// GPSDomain
// <summary>
// Computes and stores TEC data for a single GPS domain.
// </summary>
// -----------------------------------------------------------------------
class GPSDomain 
{
  private:
    uInt          iep0_,iep1_;
    Double        delph_,sigdel_,sigdel30_;
    
  public:
    GPSDomain (): iep0_(0),iep1_(0) {};
    
    // Constructor. Initializes the domain with data. "diffph" is a vector of
    // L1-L2 for this domain.
    GPSDomain ( uInt i0,uInt i1,Double d,Double sd,Double sdm )
    { 
      iep0_=i0; iep1_=i1; 
      delph_=d; 
      sigdel_=sd*Rnx_stec; 
      sigdel30_=sdm*Rnx_stec; 
    }
    
    // Data accessors
    uInt iep0 () const      { return iep0_; }
    uInt iep1 () const      { return iep1_; }
    Double delph   () const { return delph_; }
    Double sigma   () const { return sigdel_; }
    Double sigma30 () const { return sigdel30_; }
};

// -----------------------------------------------------------------------
// class RINEXSat
// <summary>
//    Stores RINEX data for one sattelite
// </summary>
// -----------------------------------------------------------------------
class RINEXSat {
  private:
    uInt          closeDomain ( uInt iep0,uInt iep1);
      
    uInt          svn_;         // SVN
    Double        tsamp_;       // nominal sampling time of Rinex data
    Double        tgd_;         // the group delay for this SVN

// Epochs (from original RINEX file). This is set up as a reference
// to the "master" RINEX::mjd array, so that all RINEXSats use uniform epoch
// indexing. The iep_ vector, below, is used to index into the mjd_master vector
// for samples pertaining to this particular SVN. I.e., if we have N
// observations for this SVN, then their epochs will be 
// mjd_master(iep_(0)) ... mjd_master(iep_(N-1)).
    Vector<Double> mjd_master;
     
    Vector<uInt>   iep_;         // index into mjd_master for each data record (below)
    Matrix<Double> d_;           // delay data 
    Matrix<uChar>  lli_,ssi_;    // Loss-of-lock and signal-strength data  
    
    Vector<Double> tec_;         // TEC data (valid only after computeDomains)
    
    // count of valid TECs computed (-1 while not computed)
    Int           numSamples_;
    
    // domains of valid TEC data
    Block<GPSDomain> domains_;  
    uInt             ndom_;
    Vector<Int>      idom_;     // # of domain for each sample; -1 if none

    // domain selection criteria
    Double dom_tmin,dom_sdmax,dom_sdmmax,dom_jumpmax; 
    
  public:
    enum ENUM_RINEX_COLUMNS;  
      
    static Int debug_level;

    uInt svn () const                   { return svn_; }
// returns the current number of observations
    uInt nelements ()                   { return iep_.nelements(); }
// resizes the arrays to the given number of observations
    void resize    ( uInt n,Bool copyData=False );
// sets up SVN, number of observations, and nominal sampling time    
    void set       ( uInt svn,uInt n,Double tsamp );
// sets up vector of epochs
    void setEpochs ( Vector<Double> &mjd ) { mjd_master.reference(mjd); }
// sets the group delay for this sattelite
    void setTGD    ( Double tgd )       { tgd_=tgd; }
// Finds domains of contigious data, and sets up the tec vector 
// with the resulting TEC samples. Returns number of valid samples.
    uInt computeDomains ();

// accessor to epoch indexes
    Vector<uInt> & iep ()       { return iep_; }
    uInt &   iep (uInt i)        { return iep_(i); }
    Double   mjd (uInt i) const   { return mjd_master(iep_(i)); }
// raw RINEX data accessors
    Matrix<Double> & d   ()  { return d_; }
    Matrix<uChar>  & lli ()  { return lli_; }
    Matrix<uChar>  & ssi     ()  { return ssi_; }
    Double & d   (uInt i,uInt j)  { return d_(i,j); }
    uChar  & lli (uInt i,uInt j)  { return lli_(i,j); }
    uChar  & ssi (uInt i,uInt j)  { return ssi_(i,j); }
// accessor to domain index (domain number of each record). TEC samples
// are valid only where idom(i) >= 0.
    const Vector<Int> & idom () const { return idom_; }
    Int idom (uInt i) const          { return idom_(i); }
// vector of tec values for each record
    const Vector<Double> & tec () const  { return tec_; }
    Double tec (uInt i) const           { return tec_(i); }
// returns the TEC value for record i, as well as sigma(TEC), and
// min. running 30-minute sigma(TEC)    
    Double getTEC (Double &sig,Double &sig30,uInt i) const;
// returns count of valid TEC samples (<0 if not yet computed)
    Int   numSamples () const                  { return numSamples_; }
// returns the number of domains computed
    uInt  ndom() const                        { return ndom_; } 
// returns domain number 'i'
    const GPSDomain & domain( uInt i ) const   { return domains_[i]; }
// constructors    
    RINEXSat();
    RINEXSat( uInt svn,uInt nobs,Double tsamp );
};        


} //# NAMESPACE CASA - END

#endif
