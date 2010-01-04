#include <ionosphere/Ionosphere/IonosphDataGPS.h>


namespace casa { //# NAMESPACE CASA - BEGIN

// -----------------------------------------------------------------------
// Constructor
// -----------------------------------------------------------------------
IonosphDataGPS::IonosphDataGPS ()
{
}    

// -----------------------------------------------------------------------
// Destructor
// -----------------------------------------------------------------------
IonosphDataGPS::~IonosphDataGPS ()
{
}    


// -----------------------------------------------------------------------
// chiSquare
// Compute chi-square and gradients for an ED profile
// -----------------------------------------------------------------------
Float IonosphDataGPS::chiSquare ( Vector<Float> &JChi,int isl,
                                  const EDProfile &edp,const Matrix<Float> &JD ) 
{
  Float d=edp.tec()-gps_tec(isl);
  
  int ncol=JChi.nelements();
  for( int j=0; j<ncol; j++ )
    JChi(j)=sum(JD.column(j));
  JChi*=2*d;
  
  return d*d;
}

// -----------------------------------------------------------------------
// chiSquare
// Computes chi-square and gradients for a TEC
// -----------------------------------------------------------------------
Float IonosphDataGPS::chiSquare ( Vector<Float> &JChi,int isl,
                                  Float TEC,const Vector<Float> &JT )
{
  Float d=TEC-gps_tec(isl);
  JChi=2*d*JT;
  
  return d*d;
}


} //# NAMESPACE CASA - END

