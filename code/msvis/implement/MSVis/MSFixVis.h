//# MSFixVis.h: Does for MSes various fixes which do not involve calibrating.
//# Copyright (C) 2008
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$
//#
#ifndef MS_MSFIXVIS_H
#define MS_MSFIXVIS_H

#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <msvis/MSVis/MSUVWGenerator.h>

// UVWMachine Does rotation, including B1950 <-> J2000, refocusing, and maybe
// even SIN <-> (-)NCP reprojection of existing UVWs, but it does not generate
// UVWs from an antenna table.
// FTMachine::rotateUVW(Matrix<Double>& uvw, Vector<Double>& dphase,
//                      const VisBuffer& vb)
#include <synthesis/MeasurementComponents/FTMachine.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class MeasurementSet;
class LogIO;

// <summary>Performs for MeasurementSets various fixes which do not involve calibrating.
// This includes (in order of implementation):
//   1. Generating and inserting (u, v, w)s into a MS that may or may not
//      already have them.  Includes antenna offsets when known.
//   2. Correcting for differential aberration.
//   3. Changing the phase tracking center.
//   4. Changing the equinox (B1950 -> J2000).
//   5. (maybe never) Changing the projection, i.e. SIN <-> (-)NCP.
// </summary>
// <use visibility=export>
// 
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
// 
// <prerequisite>
//   <li> <linkto class=MeasurementSet>MeasurementSet</linkto>
// </prerequisite>
//
// <etymology>
// It is a variation on the UVFIX task of AIPS.
// </etymology>
//
// <synopsis>
// (u, v, w)s are needed for imaging, but some data sets may not come with them
// included.
//
// FixVis can also be used to shift the tangent point, and correct for
// differential aberration.
// </synopsis>
//
// <example>
// <srcBlock>
//     MS inMS(fileName);
//     MSFixVis uvwfixer(inMS);
//     uvwfixer.setDataDescriptionIds(ddIds);
//     uvwfixer.setFields(fieldIds);
//     uvwfixer.fixuvw();
// </srcBlock>
// A <src>MSFixVis</src> object is constructed 
// and the baselines are calculated and stored in inMS.
// </example>
//
// <motivation>
// Currently (10/30/2008), ASDMs from either the ALMA Test Facility or the
// EVLA do not come with (u, v, w)s, and need to be processed with the UVFIX
// task in AIPS.  It would be preferable to process them entirely inside CASA.
// </motivation>
//
// <todo asof="">
// </todo>
 
class MSFixVis
{
public:
// Constructor
  MSFixVis (MeasurementSet& ms);

// Assignment (only copies reference to MS, need to reset selection etc)
  MSFixVis& operator=(MSFixVis& other);

// Destructor
  ~MSFixVis();

// Set the required field Ids and return the # of selected fields.
  Int setField(const String& field);
  Int setFields(const Vector<Int>& fieldIds);

  // Specifies the new phase tracking centers for each selected field.
  void setPhaseDirs(const Vector<MDirection>& phaseDirs);

// Calculate the (u, v, w)s and store them in ms_p.
  Bool calc_uvw(const String refcode);

  // For things like rotation, differential aberration correction, etc., when
  // there already is a UVW column, using FTMachine.
  Bool fixvis(const String refcode);

private:
  // Interpret field indices (MSSelection)
  Vector<Int> getFieldIdx(const String& fields);
  
  // Returns the number of selected fields, or -1 if a nonnegative entry in
  // FieldIds_p does not match its spot.
  Int check_fields();

  // Log functions and variables
  LogIO sink_p;
  LogIO& logSink();

  MeasurementSet* ms_p;			// Pointer to MS
  Int             nsel_p;		// Number of selected fields.
  Vector<Int> DDIds_p;			// DataDescription Ids to process
  Vector<Int> FieldIds_p;		// Field Ids to process
  Vector<MDirection> phaseDirs_p;	// new phase centers for each selected field
  Int nSpw_p;				// Number of spws
};
  
} //# NAMESPACE CASA - END

#endif
