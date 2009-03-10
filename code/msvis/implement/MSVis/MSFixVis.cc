#include <msvis/MSVis/MSFixVis.h>

namespace casa {

MSFixVis::MSFixVis(MeasurementSet& ms) :
  ms_p(&ms)
{
}
  
// // Assignment (only copies reference to MS, need to reset selection etc)
// MSFixVis::MSFixVis& operator=(MSFixVis::MSFixVis& other)
// {
//   ms_p = other.ms_p;
//   DDIds_p = other.DDIds_p;
//   FieldIds_p = other.FieldIds_p;
//   nSpw_p = other.nSpw_p;
// }
  
// Destructor
MSFixVis::~MSFixVis()
{
}
  
// Set the required field Ids
void MSFixVis::setField(const String& field)
{
}
  
void MSFixVis::setFields(const Vector<Int>& fieldIds)
{
  FieldIds_p = fieldIds;
}
  
// Calculate the (u, v, w)s and store them in ms_p.
Bool MSFixVis::calc_uvw()
{
  return true;
}

}  // End of casa namespace.

