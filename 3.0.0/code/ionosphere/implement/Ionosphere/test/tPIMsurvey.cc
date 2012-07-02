#include <stdio.h>
    
#include <ionosphere/Ionosphere/Ionosphere.h>
#include <ionosphere/Ionosphere/IonosphModelPIM.h>
#include <ionosphere/Ionosphere/RINEX.h>
#include <ionosphere/Ionosphere/GPSEphemeris.h>                                      
#include <ionosphere/Ionosphere/GPSGroupDelay.h>                                      
#include <casa/Quanta/MVTime.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MeasTable.h>

#include <casa/Arrays/Slice.h>
#include <casa/Arrays/LogiVector.h>
    
#include <casa/namespace.h>

int main (int argc,char *argv[])
{
  RINEX::debug_level=1;
  RINEXSat::debug_level=1;
  GPSEphemeris::debug_level=1;
  Ionosphere::debug_level=2;  
  
  try {
// frequency
    const Double freq=75*1e+6; // 75 MHz
// setup observatory position, in ITRF
    Vector<MVPosition> pos(2);
    MPosition obs;
    char *observatory = "WSRT";
    if( argc>1 ) 
      observatory = argv[1];
    int yy=1998,mm=7,dd=21;
    if( argc>4 )
    {
      yy=atoi(argv[2]);
      mm=atoi(argv[3]);
      dd=atoi(argv[4]);
    }
    if( !MeasTable::Observatory(obs,observatory) ) 
      throw( AipsError(String("Observatory ")+observatory+" not found") );
    pos(0) = MPosition::Convert(obs,MPosition::WGS84)().getValue();
// setup another position, offset by 30 km along longtitude
    MVPosition pos_itrf( MPosition::Convert(obs,MPosition::ITRF)().getValue() );
    pos(1) = MPosition::Convert( 
                  MPosition(MVPosition( pos_itrf.getLength(),
                             pos_itrf.getLong()+(300/6300.),
                             pos_itrf.getLat()),MPosition::ITRF),
                  MPosition::WGS84)().getValue();
// setup epochs of interest. We want local midnight and local midday.
    Vector<Double> mjd(2);
    mjd(0)=MVTime(yy,mm,dd,0).day();       // UTC midnight
    mjd(1)=MVTime(yy,mm,dd,0.5).day();     // UTC midday  
// apply approximate offset to get local midnight/midday
    mjd -= pos(0).getLong()/C::_2pi;
// setup grid of azimuths and elevantions
    const Double az[]={ 90 },
                el[]={ 20 };
//    const Double az[]={ 90,135,180,225,270 },
//                el[]={ 20,40,60,80 };
    const Int    naz=1,nel=1,
                ndirs=naz*nel; 
    const Int    npos=pos.nelements(),
                nep=mjd.nelements(),
                nsl=ndirs*npos*nep*2;
    SlantSet sl_set(nsl);     
    Int isl=0;
    for( Int iep=0; iep<nep; iep++ )
      for( Int iel=0; iel<nel; iel++ )
        for( Int iaz=0; iaz<naz; iaz++ )
        {
          // compute two slants at +/- 5 degrees of elevation
          Quantity qaz(az[iaz],"deg"),
                   qel1(el[iel]+5,"deg"),
                   qel2(el[iel]-5,"deg");
          MVDirection dir1(qaz,qel1),dir2(qaz,qel2);
          for( Int ipos=0; ipos<npos; ipos++ )
          {
            sl_set[isl++].set(mjd(iep),dir1,pos(ipos));
            sl_set[isl++].set(mjd(iep),dir2,pos(ipos));
          }
        }
// init PIM    
    IonosphModelPIM pim;
    Ionosphere iono(&pim);
    iono.addTargetSlants(sl_set);
// perform computations, and get rotation measure
    LogicalVector isUniq;
    const Block<EDProfile> &edp( iono.compute(isUniq) );
    Vector<Double> pimtec,rmi;
    iono.getTecRot(pimtec,rmi,edp);
    rmi/=(pow(freq/1e+9,2.0));                // recompute rotation at desired freq
// compute phase delay
    const Double Kp2=80.61636539;
    Vector<Double> phdel( Kp2/(2*C::c*freq)*pimtec*1e+16 );
    
// now, compute various interesting values
    printf("Location (%s): %8.4f%8.4f, %8.4f%8.4f\n",observatory,
        pos(0).getLong()/C::degree,pos(0).getLat()/C::degree, 
        pos(1).getLong()/C::degree,pos(1).getLat()/C::degree); 
    printf("Beam: 10 deg       Freq: %f0.0 MHz\n",freq/1e+6);
    isl=0;
    for( Int iep=0; iep<nep; iep++ )
    {
      char *label[] = {"midnight","midday"};
      printf("Local %s (%s UTC)\n",label[iep],MVTime(mjd(iep)).string(MVTime::YMD).chars()); 
      printf("%5s%5s%7s%7s%7s%7s%7s%7s%7s%7s%7s\n",
          "EL","AZ","min FR","max FR","DFR","DFRb","DFRp","min PD","max PD","DPDb","DPDp");
      for( Int iel=0; iel<nel; iel++ )
        for( Int iaz=0; iaz<naz; iaz++ )
        {
          Slice S(isl,npos*2);
// compute FR
          Vector<Double> r(rmi(S)); 
          // min and max rotation measure across all slants
          Double rmin,rmax;
          minMax(rmin,rmax,r);
          // find max variation across each beam
          Double beamvar = max(
              abs( r(Slice(0,npos,2)) - r(Slice(1,npos,2)) ) );
          // find max variation w/position
          Double posvar = max(
              abs( r(Slice(0,2)) - r(Slice(2,2)) ) );
// compute phase delays
          Vector<Double> pd(phdel(S));
          Double pdmin,pdmax;
          minMax(pdmin,pdmax,pd);
          // find max variation across each beam
          Double dpdbeam = max(
              abs( pd(Slice(0,npos,2)) - pd(Slice(1,npos,2)) ) );
          // find max variation w/position
          Double dpdpos = max(
              abs( pd(Slice(0,2)) - pd(Slice(2,2)) ) );
          
          // print results
          printf("%5.0f%5.0f%7.2f%7.2f%7.2f%7.2f%7.2f%7.1f%7.1f%7.2f%7.2f\n",
              el[iel],az[iaz],rmin,rmax,rmax-rmin,beamvar,posvar,pdmin,pdmax,dpdbeam,dpdpos);
          
          isl+=npos*2;
        }
    }
  }
  
  catch( AipsError x )
  {
    cerr << "AipsError: " << x.getMesg() << endl;
    return 1;
  }
  return 0;
}
