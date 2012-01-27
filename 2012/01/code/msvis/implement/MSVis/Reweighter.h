//# Reweighter.h: this defines Reweighter which reweights a selected part of an
//# MS based on the properties of a (possibly different) selection of it.
//# Copyright (C) 2011
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
//#
//# $Id$
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <ms/MeasurementSets/MSMainEnums.h>
//#include <msvis/MSVis/VisIterator.h>
#include <msvis/MSVis/SubMS.h>
#include <msvis/MSVis/VisBufferComponents.h>
#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
//#include <casa/Utilities/CountedPtr.h>
#include <map>
#include <set>
#include <vector>


#ifndef MSVIS_REWEIGHTER_H
namespace casa { //# NAMESPACE CASA - BEGIN

#define MSVIS_REWEIGHTER_H

// <summary>
//   Reweighter reweights a selected part of an MS based on the properties of a
//   (possibly different) selection of it.
// </summary>

// <visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> MeasurementSet
// </prerequisite>
//
// <etymology>
// Reweighter ...it reweights.
// </etymology>
//
// <synopsis>
// The order of operations (as in ms::split()) is:
//      ctor
//      setmsselect
//      selectTime
//      makeReweighter
// </synopsis>

// These forward declarations are so the corresponding .h files don't have to
// be included in this .h file, but it's only worth it if a lot of other files
// include this file.
class MSSelection; // #include <ms/MeasurementSets/MSSelection.h>

template<class T> class ROArrayColumn;

class Reweighter
{
public:
  Reweighter(const String& theMS,
             const Bool dorms=false,           // Reweight by rms or stddev?
             const uInt minsamp=1);            // Minimum # of visibilities for measuring a scatter.
  
  virtual ~Reweighter();
  
  // Select spw and channels for each spw in spwstr.
  // Returns true on success and false on failure.
  Bool selectSpw(std::set<Int>& spwset, Vector<Int>& chanStartv,
                 Vector<Int>& chanEndv, Vector<Int>& chanStepv,
                 const String& spwstr);

  // Setup polarization selection (for now, only from available correlations -
  // no Stokes transformations.)
  Bool selectCorrelations(const String& corrstr);

  //select stuff using msselection syntax ...time is left out
  // call it separately with timebin
  // Returns a success value.
  Bool setmsselect(const String& fitspw="", const String& outspw="",
                   const String& field="",  const String& baseline="",
                   const String& scan="",
		   const String& subarray="", const String& correlation="",
                   const String& intent="", const String& obs="");

  // Select source or field
  Bool selectSource(const Vector<Int>& fieldid);
  
  void selectAntenna(const Vector<Int>& antennaids,
		     const Vector<String>& antennaSel)
  {
    antennaSel_p = SubMS::pickAntennas(antennaId_p, antennaSelStr_p,
                                       antennaids, antennaSel);
  }
  
  // Select array IDs to use.
  void selectArray(const String& subarray) {arrayExpr_p = subarray;}

  //select time parameters
  void selectTime(Double timeBin=-1.0, String timerng="");

  //void selectSource(Vector<String> sourceid);

  // Set the selection string for line-free channels.
  void setFitSpw(const String& fitspw) {fitspw_p = fitspw;}
  // Set the selection string for output spws (not channels!).
  void setOutSpw(const String& outspw) {outspw_p = outspw;}
  
  // Do the reweighting!
  Bool reweight(String& colname, const String& combine);

  //void verifyColumns(const MeasurementSet& ms, const Vector<MS::PredefinedColumns>& colNames);

  // Fills polIDs with a map from DDID to polID, and corrTypes with a map from
  // polID to corrTypes.
  // The return value is meaningless for now.
  static Bool getCorrTypes(Vector<Int>& polIDs,
                           Vector<Vector<Int> >& corrTypes,
                           const MSColumns& msc);
  
 protected:

  //method that returns the selected ms (?! - but it's Boolean - RR)
  Bool makeSelection();

  // *** Private member functions ***

  // Picks a reference to DATA, MODEL_DATA, CORRECTED_DATA, or LAG_DATA out
  // of ms_p.  FLOAT_DATA is not included because it is not natively complex. 
  const ROArrayColumn<Complex>& right_column(const ROMSColumns *ms_p,
                                             const MS::PredefinedColumns datacol);

  // Sets sort to a Block of columns that a VisibilityIterator should sort by,
  // according to combine_p.  Columns that should never be combined in the
  // calling function, i.e. spw for time averaging, should be listed in
  // uncombinable.
  //
  // verbose: log a message on error.
  //
  // Returns whether or not there were any conflicts between combine_p and
  // uncombinable.
  Bool setSortOrder(Block<Int>& sort, const String& uncombinable="",
                    const Bool verbose=true) const;

  // Returns whether col is (not in combine_p) || in uncombinable.
  // Columns that should never be combined in the
  // calling function, i.e. spw for time averaging, should be listed in
  // uncombinable.
  //
  // verbose: log a message on error.
  //
  // conflict is set to true if there is a conflict between combine_p and
  // uncombinable.
  Bool shouldWatch(Bool& conflict, const String& col,
                   const String& uncombinable="",
                   const Bool verbose=true) const;

  void makeUnionSpw();

  // Defaults to DATA.
  MS::PredefinedColumns dataColStrToEnum(const String& col);

  // *** Member variables ***

  // Initialized* by ctors.  (Maintain order both here and in ctors.)
  //  * not necessarily to anything useful.
  MeasurementSet ms_p, mssel_p;
  Bool dorms_p;                 // Reweight by rms or stddev?
  uInt minsamp_p;               // Minimum # of visibilities for measuring a scatter.
  MSColumns *msc_p;
  Bool antennaSel_p;		// Selecting by antenna?
  Double timeBin_p;
  String scanString_p,          // Selects scans by #number#.  Historically named.
         intentString_p,        // Selects scans by string.  scanString_p was taken.
         obsString_p;            // String for observationID selection.
  String timeRange_p, arrayExpr_p, corrString_p;
  String combine_p;          // Should time averaging not split bins by
                             // scan #, observation, and/or state ID?
                             // Must be lowercase at all times.
  String fitspw_p;           // Selection string for line-free channels.
  String outspw_p;           // Selection string for ddids to reweight.
  Vector<Int> unionspw_p;    // All the spws in fitspw_p or outspw_p.

  // Uninitialized by ctors.
  Vector<String> antennaSelStr_p;
  Vector<Int> antennaId_p;

  Vector<Int> fieldId_p;

  // Map from DDID to polID, filled in getCorrTypes().
  Vector<Int> polIDs_p;

  // Map from polID to corrTypes, filled in getCorrTypes().
  Vector<Vector<Int> > corrTypes_p;

  Vector<Vector<Slice> > chanSlices_p;  // Used by VisIterator::selectChannel()
  Vector<Slice> corrSlice_p;
  Vector<Vector<Slice> > corrSlices_p;  // Used by VisIterator::selectCorrelation()
  Matrix<Double> selTimeRanges_p;

  std::set<Int> fitspwset_p, outspwset_p;
  Vector<Int> fitStart_p, fitEnd_p, fitStep_p;
};

} //# NAMESPACE CASA - END

#endif

