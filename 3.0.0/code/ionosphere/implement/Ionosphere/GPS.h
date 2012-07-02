#ifndef IONOSPHERE_GPS_H
#define IONOSPHERE_GPS_H

#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/Containers/Block.h>
    
    
namespace casa { //# NAMESPACE CASA - BEGIN

const uInt MAX_GPS_SVN = 32;            // highest sattelite number in GPS constellation
const uInt NUM_GPS     = MAX_GPS_SVN; // # of sats in GPS constellation
                      
    
// Some RINEX-related constants
const Double 
    Rnx_c = 2.99792458e+08,
    Rnx_nu1 = 1.57542e+09,
    Rnx_nu2 = 1.2276e+09,
    Rnx_kp = 80.616369539,
    Rnx_stec = 2.*Rnx_c*(Rnx_nu1*Rnx_nu1)*(Rnx_nu2*Rnx_nu2) /
                      (1e+16*Rnx_kp*(Rnx_nu1*Rnx_nu1-Rnx_nu2*Rnx_nu2));


// macro to define the column-index enums in the RINEX-related classes
#define ENUM_RINEX_COLUMNS Columns {L1=0,L2=1,P1=2,P2=3,C1=4,CA=4  };


// function to convert an Array to a Block
// (to use with InterpolateArray1D...)
template<class T> void toBlock ( Block<T> &blk,const Array<T> &arr );


} //# NAMESPACE CASA - END

#endif
