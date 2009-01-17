#ifndef IONOSPHERE_IONOSPHMODEL_H
#define IONOSPHERE_IONOSPHMODEL_H

#include "Ionosphere.h"

namespace casa { //# NAMESPACE CASA - BEGIN

// -----------------------------------------------------------------------
// IonosphModel
// <summary>
// Abstract base class for apriori ionospheric models
// </summary>
// -----------------------------------------------------------------------
class IonosphModel
{
  protected:
    Vector <EDProfileAlt>  altgrid;
  
  public:
// Constructor. Sets up the model.
    IonosphModel () {};
    virtual ~IonosphModel() {}

// Sets up the altitude sampling grid      
    virtual void setAlt( const Vector <EDProfileAlt> &alt ) { altgrid=alt; }

// Performs the actual model computations for a number of slants, and 
// returns the corresponding ED profile estimates.
// Slants should be sorted by time, into unique time slots, with sidx and
// suniq containing the sorted and unique index vectors.
    virtual PtrBlock<EDProfile> getED ( const SlantSet &sl_set,
                                       const Vector<uInt> &sidx,
                                       const Vector<uInt> &suniq )=0;

// Version of getED for an unsorted time slot array. Default implementation
// does the sort, then calls getED() above.
    virtual PtrBlock<EDProfile> getED ( const SlantSet &sl_set ) 
    {
      Vector<uInt> sidx,suniq;
      sortSlants(sidx,suniq,sl_set);
      return getED(sl_set,sidx,suniq);
    };
    
};



} //# NAMESPACE CASA - END

#endif
