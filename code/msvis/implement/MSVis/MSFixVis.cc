#include <msvis/MSVis/MSFixVis.h>
#include <msvis/MSVis/MSUVWGenerator.h>
#include <casa/Logging/LogIO.h>
#include <casa/Exceptions/Error.h>
#include <ms/MeasurementSets/MSSelection.h>
#include <ms/MeasurementSets/MSSelectionTools.h>

namespace casa {

MSFixVis::MSFixVis(MeasurementSet& ms) :
  ms_p(&ms),
  nsel_p(-1)
{
  logSink() << LogOrigin("MSFixVis","") << LogIO::NORMAL3;
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

// Interpret field indices (MSSelection)
Vector<Int> MSFixVis::getFieldIdx(const String& fields)
{
  MSSelection mssel;

  mssel.setFieldExpr(fields);
  return mssel.getFieldList(ms_p);
}

// Set the required field Ids
Int MSFixVis::setField(const String& field)
{
  Vector<Int> fldIndices(getFieldIdx(field));
  
  FieldIds_p = getFieldIdx(field);
  nsel_p = check_fields();
  logSink() << LogOrigin("MSFixVis", "setField")
	    << LogIO::NORMAL
	    << FieldIds_p << " -> " << nsel_p << " selected fields." 
	    << LogIO::POST;
  return nsel_p;
}
  
Int MSFixVis::setFields(const Vector<Int>& fieldIds)
{
  FieldIds_p = fieldIds;
  nsel_p = check_fields();
  logSink() << LogOrigin("MSFixVis", "setFields")
	    << LogIO::NORMAL
	    << "Selected " << nsel_p << " fields: " << FieldIds_p
	    << LogIO::POST;
  return nsel_p;
}

void MSFixVis::setPhaseDirs(const Vector<MDirection>& phaseDirs)
{
  phaseDirs_p = phaseDirs;

  //TODO
  // Update the FIELD table with the new phase directions (and DELAY_DIR?)
  // Do not change REFERENCE_DIR since it is supposed to be the original
  // PHASE_DIR.  In particular, if the POINTING table is invalid (i.e. VLA),
  // the REFERENCE_DIR seems like the best substitute for the pointing direction.
}

Int MSFixVis::check_fields()
{
  // Make sure FieldIds_p.nelements() == # fields in *ms_p.

  // Go through FieldIds_p, make sure the nonnegative entries are in the right
  // spots, and count them.
  uInt nsel = 0;
  for(uInt i = 0; i < FieldIds_p.nelements(); ++i){
    if(FieldIds_p[i] > -1){
      if(static_cast<uInt>(FieldIds_p[i]) != i)
	return -1;
      ++nsel;
    }
  }
  return nsel;
}

LogIO& MSFixVis::logSink() {return sink_p;};

// Calculate the (u, v, w)s and store them in ms_p.
Bool MSFixVis::calc_uvw(const String refcode)
{
  // Make sure FieldIds_p has a Field ID for each selected field, and -1 for
  // everything else!
  if(nsel_p > 0 // && static_cast<uInt>(nsel_p) == phaseDirs_p.nelements()
     ){
    // This is just an enum!  Why can't Muvw just return it instead of
    // filling out a reference?
    Muvw::Types uvwtype;
    MBaseline::Types bltype;
    
    try{
      MBaseline::getType(bltype, refcode);
      Muvw::getType(uvwtype, refcode);
    }
    catch(AipsError x){
      logSink() << LogOrigin("MSFixVis", "calc_uvw")
		<< LogIO::SEVERE
		<< "refcode \"" << refcode << "\" is not valid for baselines."
		<< LogIO::POST;
    }
    
    MSUVWGenerator uvwgen(*ms_p, bltype, uvwtype);
  
    return uvwgen.make_uvws(FieldIds_p);
  }
  else{
    logSink() << LogOrigin("MSFixVis", "calc_uvw") << LogIO::NORMAL3;
    logSink() << LogIO::SEVERE
	      << "There is a problem with the selected field IDs and phase tracking centers."
	      << LogIO::POST;
    return false;
  }
}

// Calculate the (u, v, w)s and store them in ms_p.
Bool MSFixVis::fixvis(const String refcode)
{
  if(nsel_p > 0){
    if(phaseDirs_p.nelements() == static_cast<uInt>(nsel_p)){
      //**** Adjust the phase tracking centers. ****
      // First calculate new UVWs for the selected fields.
      calc_uvw(refcode);
    }
    else if(phaseDirs_p.nelements() > 0){
      logSink() << LogIO::WARN 
		<< "There is a problem with the selected field IDs and phase tracking centers.\n"
		<< "No adjustments of phase tracking centers will be done."
		<< LogIO::POST;
    }
    
    // Correct differential aberration?
  }
  else{
    logSink() << LogIO::SEVERE << "No fields are selected." << LogIO::POST;
    return false;
  }  
  return true;
}

}  // End of casa namespace.

