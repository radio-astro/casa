#include <msvis/MSVis/MSFixVis.h>
#include <msvis/MSVis/MSUVWGenerator.h>

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

void MSFixVis::setPhaseDirs(const Vector<MDirection>& phaseDirs)
{
  phaseDirs_p = phaseDirs;
}

Bool MSFixVis::fields_are_OK()
{
  return false;  // TODO, obviously.
}

// Calculate the (u, v, w)s and store them in ms_p.
Bool MSFixVis::calc_uvw()
{
  MSUVWGenerator uvwgen(*ms_p);
  
  // Make sure FieldIds_p has a Field ID for each selected field, and -1 for
  // everything else!
  if(fields_are_OK())
    return uvwgen.make_uvws(FieldIds_p, phaseDirs_p);
  else
    return false;
}

// Calculate the (u, v, w)s and store them in ms_p.
Bool MSFixVis::fixvis()
{
  return true;
}

}  // End of casa namespace.

