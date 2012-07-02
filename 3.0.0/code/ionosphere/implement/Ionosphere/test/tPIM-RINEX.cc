#include <stdio.h>
    
#include <ionosphere/Ionosphere/Ionosphere.h>
#include <ionosphere/Ionosphere/IonosphModelPIM.h>
#include <ionosphere/Ionosphere/RINEX.h>
#include <ionosphere/Ionosphere/GPSEphemeris.h>                                      
#include <ionosphere/Ionosphere/GPSDCB.h>                                      
#include <casa/Quanta/MVTime.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MeasConvert.h>
#include <casa/Arrays/Slice.h>
#include <casa/Arrays/MaskedArray.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/LogiVector.h>
#include <casa/Exceptions/Error.h>
    
#include <casa/namespace.h>

int main (void)
{
  RINEX::debug_level=1;
  RINEXSat::debug_level=1;
  GPSEphemeris::debug_level=1;
  Ionosphere::debug_level=2;  
  
  try {
// read files
    GPSDCB tgd("jpl_tgd.dta");
    GPSEphemeris eph("test.orb");                                               
    RINEX rinex("test.rnx");
// compute TEC samples
    Vector<Double> mjd,tec,stec,stec30;
    Vector<Int> sat,domain;
    rinex.getTEC(mjd,sat,tec,stec,stec30,domain,tgd);
// compute corresponding az/els
    Vector<MVDirection> dir(mjd.nelements());
    for( Int s=0; s<=(Int)NUM_GPS; s++ )
    {
      LogicalArray wh(sat==s);                // find slots corresponding to this sat
      if( wh.nelements() )
        dir(wh) = eph.splineAzEl(s,mjd(wh).getArray(),rinex.rcvPos());   // spline it orbit to these time slots
    }
// build up slants array for ionosphere
    MVPosition pos( rinex.header().pos );
    const uInt nrnx = mjd.nelements(),
//              nsl = nrnx;
              nsl = 50;
    Slice S(0,nsl,30);
    Vector<Double> mjd1(mjd(S)),tec1(tec(S)),stec1(stec(S)),stec301(stec30(S));
    Vector<MVDirection> dir1(dir(S));
    Vector<uInt> sat1(sat(S));

    SlantSet sl_set(nsl);     
    for( uInt i=0; i<nsl; i++ )
      sl_set[i].set(mjd1(i),dir1(i),pos);  // really should pass in some ID...
// init PIM    
    IonosphModelPIM pim;
    Ionosphere iono(&pim);
    iono.addTargetSlants(sl_set);
// perform computations, and get rotation measure
    LogicalVector isUniq;
    const Block<EDProfile> &edp( iono.compute(isUniq) );
    Vector<Double> pimtec,rmi;
    iono.getTecRot(pimtec,rmi,edp);
    rmi*=(0.677458*C::degree); // use the Bob Campbell constant... 
// print results
    Vector<Double> dtec = tec1-pimtec,
        chitec = dtec/stec1,
        abstec = abs(dtec)/tec1;
        
    printf("TIME SVN OBSTEC OBSSIG PIMTEC CHI DELTA RMI AZ EL\n"); 
    for( uInt i=0; i<nsl; i++ ) 
      printf("%s %02d %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f\n",
        MVTime(mjd1(i)).string().chars(),sat1(i),
          tec1(i),stec1(i),pimtec(i),chitec(i),abstec(i),rmi(i),
          (dir1(i).getAngle().getValue("deg"))(0),
          (dir1(i).getAngle().getValue("deg"))(1));
    
//    cerr<<"======================================== Computed TECs\n";
//    for( uInt i=0; i<mjd.nelements(); i++ ) 
//       cerr<<MVTime(mjd(i))<<" "<<sat(i)<<" "<<tec(i)<<" "<<stec(i)
//          <<" "<<stec30(i)<<" "<<dir(i).getAngle(Unit("deg"))<<endl;
  }
  
  catch( AipsError x )
  {
    cerr << "AipsError: " << x.getMesg() << endl;
    return 1;
  }
  return 0;
}
