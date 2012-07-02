#ifndef IONOSPHERE_IONOSPHDATAGPS_H
#define IONOSPHERE_IONOSPHDATAGPS_H
    
#include "Ionosphere.h"
    
namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Implements corrections via GPS data
// </summary>
class IonosphDataGPS : public IonosphData
{
  protected:
    Vector <Float>   gps_tec;  
    
  public:
    IonosphDataGPS ();
    virtual ~IonosphDataGPS ();  
      
    virtual Bool fitsTEC     () { return True; }
    virtual Bool fitsProfile () { return True; }
    
    virtual Float chiSquare (Vector<Float> &JChi,int isl,
                            Float TEC,const Vector<Float> &JT);

    virtual Float chiSquare (Vector<Float> &JChi,int isl,
                            const EDProfile &edp,
                            const Matrix<Float> &JD);
};




} //# NAMESPACE CASA - END

#endif
