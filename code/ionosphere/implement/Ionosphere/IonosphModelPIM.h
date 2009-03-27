#ifndef IONOSPHERE_IONOSPHMODELPIM_H
#define IONOSPHERE_IONOSPHMODELPIM_H

#include <casa/Arrays/LogiVector.h>    
#include "Ionosphere.h"

namespace casa { //# NAMESPACE CASA - BEGIN

const Float pimDefaultAltGrid[]=
      { 90., 95., 100., 105., 110., 115., 120., 125., 130.,
        135., 140., 145., 150., 155., 160., 165., 170., 175., 180., 
        190., 200., 210., 220., 230., 240., 250., 260., 270., 280.,
        290., 300., 310., 320., 330., 340., 350., 360., 370., 380.,
        390., 400., 420., 440., 460., 480., 500., 520., 540., 560.,
        580., 600., 620., 640., 660., 680., 700., 720., 740., 770.,
        800., 850., 900., 950., 1000., 1100., 1200., 1300., 1400., 
        1500., 1600., 1700., 1800., 1900., 2000., 2100., 2200., 2300.,
        2400., 2500., 3000., 3500., 4000., 4500., 5000., 5500., 6000.,
        6500., 7000., 7500., 8000., 8500., 9000., 10000., 11000.,
        12000., 13000., 14000., 15000., 17500., 20182. };
    
       
// -----------------------------------------------------------------------
// IonosphModelPIM
// <summary>
// Implements the PIM ionospheric model
// </summary>
// (wrapper around PIM and some of ased on Bob Campbell's FORTRAN code)
// -----------------------------------------------------------------------
class IonosphModelPIM : public IonosphModel
{
  public:
// fallback strategies for missing IMF data
    enum IMF_Fallback { 
          IMF_INTERPOLATE,
          IMF_FIX_NORTH,
          IMF_RUN_TWICE,
          IMF_ABORT
    };
    typedef WHATEVER_SUN_TYPEDEF(IonosphModelPIM) IMF_Fallback IMF_Fallback;
// types of external data
    enum PIM_Parameter { 
          PIM_F107   =0,
          PIM_AP     =1,
          PIM_IMF_BZ =2,
              
          PIM_NPARAM =3
    };
    typedef WHATEVER_SUN_TYPEDEF(IonosphModelPIM) PIM_Parameter PIM_Parameter;
      
// default contructor and destructor
    IonosphModelPIM ();
    virtual ~IonosphModelPIM ();  
      
// sets up the altitude grid
    virtual void setAlt( const Float *alt,uInt nalt );
    virtual void setAlt( const Vector<Float> &alt );
    
// sets up a fallback strategy for missing IMF datums
//    IMF_Fallback getFallbackStrategy () const;
//    void setFallbackStrategy ( IMF_Fallback strategy );
    
// sets up the slant separation criteria
//    void     setMinSlantSep ( Double sep_radians );
//    void     setMinSlantSep ( Quantity sep );
// Returns the slant separation criteria. Slants closer than this
// will be computed only once
//    Double   getMinSlantSep () const;              // in radians
//    Quantity getMinSlantSep ( Unit unit ) const;  // in specified units

// Returns sizes of Ap and F10.7 arrays for a given range of dates
    uInt sizeAp   (Int mjd1,Int mjd2) { return mjd2-mjd1+3; };
    uInt sizeF107 (Int mjd1,Int mjd2) { return (mjd2-mjd1+3)*8; };
// Read in Ap and F10.7 data from global tables
// the first version reads into a regular array (must be big enough,
// as indicated by sizeAp() and sizeF107()). The second version stores
// it into vectors, resizing as appropriate.
    void  readApF107 (Double *sf107,Double *sap,Int mjd1,Int mjd2);
    void  readApF107 (Vector<Double> &sf107,Vector<Double> &sap,Int mjd1,Int mjd2);

// Obtains the IMF By and Bz values in the middle of the specified
// time interval.
// If IMF data is missing, uses the strategy parameter to figure out
// what to do. Throws an exception if unable to handle. Returns
// True if valid By/Bz values are returned, or False if the RUN_TWICE
// strategy was selected and data is missing.
    Bool  readIMF ( Float &by,Float &bysig,Float &bz,Float &bzsig,
                    Double mjd1,Double mjd2 );
    
// Performs the actual model computations for a given slant set, 
// and returns the corresponding ED profile estimates.
// isUniq is a vector of flags; if false, then the corresponding slant
// was not actually computed, but rather copied from the nearest neighbour. 
    virtual Block<EDProfile> getED ( LogicalVector &isUniq,
                                    const SlantSet &sl_set,
                                    const Vector<uInt> &sidx,
                                    const Vector<uInt> &suniq );

// Normally, tables in /aips++/data will be read to determine the Kp/Ap, 
// F10.7 and IMF parameters for the given dates. If you want to use your
// own values (i.e., for model computations, or if data is not available),
// call this method before a getED(). 
// Fixed parameters remian in effect for a single getED() call. 
// Note that when IMF Bz is >0 (north), Ap no longer matters.
    void fixParameter       ( PIM_Parameter type,Float val);

// log sink
    static LogIO os;    

//  private:
// clears all fixed parameters
//    void clearFixes ();
      
//    IMF_Fallback imf_strategy;  // missing-IMF handling
//    Double  slant_sep_rad;       // slant separation criteria, in radians
    
//    Bool  fix_parm[PIM_NPARAM];      // flags: parameter is fixed (PIM_Parameter)
//    Float fix_parm_val[PIM_NPARAM];     // fixed value of parameter
};

// inline IonosphModelPIM::IMF_Fallback IonosphModelPIM::getFallbackStrategy () const
// { 
//   return imf_strategy; 
// }
// 
// inline void IonosphModelPIM::setFallbackStrategy( IonosphModelPIM::IMF_Fallback strategy ) 
// { 
//   imf_strategy  = strategy; 
// }
// 
// inline void IonosphModelPIM::setMinSlantSep ( Double sep_radians )
// {
//   slant_sep_rad = sep_radians;
// }
// 
// inline void IonosphModelPIM::setMinSlantSep ( Quantity sep )
// {
//   sep.assure( UnitVal::ANGLE );
//   setMinSlantSep( sep.getBaseValue() );
// }
// 
// inline Double IonosphModelPIM::getMinSlantSep () const
// {
//     return slant_sep_rad;
// }
// 
// inline Quantity IonosphModelPIM::getMinSlantSep ( Unit unit ) const
// {
//   Quantity q( slant_sep_rad,"rad" );
//   return q.get(unit);
// }
// 
// inline void IonosphModelPIM::clearFixes () 
// {
//   for( Int i=0; i<PIM_NPARAM; i++ )
//     fix_parm[i] = False;
// }
// 

} //# NAMESPACE CASA - END

#endif
