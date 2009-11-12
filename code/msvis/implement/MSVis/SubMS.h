//# SubMS.h: this defines SubMS which creates a subset of an MS with some
//# transformation
//# Copyright (C) 1997,1998,1999,2000,2001,2003
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
#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <map>
#include <vector>
//#include <ms/MeasurementSets/MSColumns.h>
#include <scimath/Mathematics/InterpolateArray1D.h>


#ifndef MSVIS_SUBMS_H
namespace casa { //# NAMESPACE CASA - BEGIN

#define MSVIS_SUBMS_H

// <summary>
// SubMS provides functionalities to make a subset of an existing MS
// </summary>

// <visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> MeasurementSet
// </prerequisite>
//
// <etymology>
// SubMS ...from the SUBset of an MS
// </etymology>
//
// <synopsis>
// The order of operations (as in ms::split()) is:
//      ctor
//      setmsselect
//      selectTime
//      makeSubMS
// </synopsis>

// These forward declarations are so the corresponding .h files don't have to
// be included in this .h file, but it's only worth it if a lot of other files
// include this file.
class MSColumns;
class ROMSColumns;

  // These typedefs are necessary because a<b::c> doesn't work.
  typedef std::vector<uInt> uivector;
  struct uIntCmp 
  {
    bool operator()(const uInt i1, const uInt i2) const 
    {
      return i1 < i2;
    }
  };
  typedef std::map<const uInt, uivector, uIntCmp> ui2vmap;

template<class T> class ROArrayColumn;
  Bool isAllColumns(const Vector<String>& colNames);

class SubMS
{

 public:

  SubMS(String& theMS);
  
  // construct from an MS
  SubMS(MeasurementSet& ms);

  

  ~SubMS();
  
  // Change or Set the MS this MSSelector refers to.
  void setMS(MeasurementSet& ms);

  // Select spw and channels for each spw.
  // If averchan is true, chanStep_p will be used as a width for channel
  // averaging.  Otherwise it will be used as a step for channel skipping.
  // This is the version used by split.  It returns true on success and false
  // on failure.
  Bool selectSpw(const String& spwstr, const Vector<Int>& steps,
                 const Bool averchan=true);

  // This older version is used by the older version of setmsselect().
  void selectSpw(Vector<Int> spw, Vector<Int> nchan, Vector<Int> start, 
                 Vector<Int> step, const Bool averchan);
  
  //select Time and time averaging or regridding
  //void selectTime();

  //select stuff using msselection syntax ...time is left out
  // call it separately with timebin
  // This version returns a success value, and does not need nchan, start, and
  // step.  It is used by split.
  Bool setmsselect(const String& spw="", const String& field="", 
		   const String& baseline="", const String& scan="",
                   const String& uvrange="", const String& taql="", 
		   const Vector<Int>& step=Vector<Int> (1,1),
		   const Bool averchan=True, const String& subarray="");

  // This older version does not return a success value, and does need nchan,
  // start, and step.  It is used elsewhere.
  void setmsselect(const String& spw,        const String& field, 
                   const String& baseline,   const String& scan,
                   const String& uvrange,    const String& taql,
                   const Vector<Int>& nchan, const Vector<Int>& start,
                   const Vector<Int>& step,  const Bool averchan,
                   const String& subarray);

  // Select source or field
  Bool selectSource(const Vector<Int>& fieldid);
  
  // Select Antennas to split out  
  void selectAntenna(Vector<Int>& antennaids, Vector<String>& antennaSel);

  // Select array IDs to use.
  void selectArray(const String& subarray);

  //select time parameters
  void selectTime(Double timeBin=-1.0, String timerng="");

  //void selectSource(Vector<String> sourceid);

  //Method to set if a phase Center rotation is needed
  //void setPhaseCenter(Int fieldid, MDirection& newPhaseCenter);


  //Method to make the subMS

  //TileShape of size 1 can have 2 values [0], and [1] ...these are used in to determine the tileshape
  //by using MSTileLayout 
  //Otherwise it has to be a vector size 3  e.g [4, 15, 351] => a tile shape of 4 stokes, 15 channels 351 rows.
  Bool makeSubMS(String& submsname, String& whichDataCol, const Vector<Int>& tileShape = Vector<Int> (1, 0));

  //Method to make a scratch  subMS and even in memory if posssible
  //Useful if temporary subselection/averaging is necessary
  // It'll be in memory if the basic output ms is less than half of 
  // memory reported by HostInfo unless forced to by user...
  MeasurementSet* makeScratchSubMS(String& whichDataCol, 
				   Bool forceInMemory=False);

  // This setup a default new ms
  // Declared static as it can be (and is) called directly outside of SubMS.
  // Therefore it is not dependent on any member variable.
  static MeasurementSet* setupMS(String msname, Int nchan, Int npol, 
				 String telescop, String colName="DATA",
				 Int obstype=0);

  // Same as above except allowing manual tileshapes
  static MeasurementSet* setupMS(String msname, Int nchan, Int npol, 
				 String colName="DATA",
				 const Vector<Int>& tileShape=Vector<Int>(1,0));

  
  // Add optional columns to outTab if present in inTab and possColNames.
  // M must be derived from a Table.
  // beLazy should only be true if outTab is in its default state.
  // Returns the number of added columns.
  template<class M>
  static uInt addOptionalColumns(const M& inTab, M& outTab,
				 const Vector<String>& possColNames,
				 const Bool beLazy=false);
  
  // Declared static because it's used in setupMS().  Therefore it can't use
  // any member variables.  It is also used in MSFixvis.cc.
  static const Vector<String>& parseColumnNames(const String colNameList);

  void verifyColumns(const MeasurementSet& ms, const Vector<String>& colNames);
  Bool doWriteImagingWeight(const ROMSColumns& msc, const Vector<String>& colNames);

  // Transform spectral data to different reference frame,
  // optionally regrid the frequency channels 
  // return values: -1 = MS not modified, 1 = MS modified and OK, 
  // 0 = MS modified but not OK (i.e. MS is probably damaged) 
  Int regridSpw(String& message, // returns the MS history entry 
		const String& outframe="", // default = "keep the same"
		const String& regridQuant="chan",
		const Double regridVeloRestfrq=-3E30, // default = "not set" 
		const String& regridInterpMeth="LINEAR",
		const Double regridCenter=-3E30, // default = "not set" 
		const Double regridBandwidth=-1., // default = "not set" 
		const Double regridChanWidth=-1., // default = "not set" 
		const Int phaseCenterFieldId=-2, // -2 = use pahse center from field table
		MDirection phaseCenter=MDirection() // this direction is used if phaseCenterFieldId==-1
		);

  // the following inline convenience methods for regridSpw bypass the whole CASA measure system
  // because when they are used, they can assume that the frame stays the same and the units are OK
  Double vrad(const Double freq, const Double rest){ return (C::c * (1. - freq/rest)); };
  Double vopt(const Double freq, const Double rest){ return (C::c *(rest/freq - 1.)); };
  Double lambda(const Double freq){ return (C::c/freq); };
  Double freq_from_vrad(const Double vrad, const Double rest){ return (rest * (1. - vrad/C::c)); };
  Double freq_from_vopt(const Double vopt, const Double rest){ return (rest / (1. + vopt/C::c)); };
  Double freq_from_lambda(const Double lambda){ return (C::c/lambda); };
  
  // Support method for regridSpw():
  // results in the column oldName being renamed to newName, and a new column which is an empty copy of 
  // oldName being created together with a TileShapeStMan data manager and hypercolumn (name copied from 
  // the old hypercolumn) with given dimension, the old hypercolumn of name hypercolumnName is renamed 
  // to  name+"B"
  Bool createPartnerColumn(TableDesc& modMSTD, const String& oldName, const String& newName,
			   const Int& hypercolumnDim, const IPosition& tileShape);

  // Support method for regridSpw():
  // calculate the final new channel boundaries from the regridding parameters
  // and the old channel boundaries (already transformed to the desired reference frame);
  // returns False if input paramters were invalid and no useful boundaries could be created
  Bool regridChanBounds(Vector<Double>& newChanLoBound, 
			Vector<Double>& newChanHiBound,
			const Double regridCenter, 
			const Double regridBandwidth,
			const Double regridChanWidth,
			const Double regridVeloRestfrq, 
			const String regridQuant,
			const Vector<Double>& transNewXin, 
			const Vector<Double>& transCHAN_WIDTH,
			String& message // message to the user, epsecially in case of error 
			);

  // Support method for regridSpw():
  // if writeTables is False, the (const) input parameters are only verified, nothing is written;
  // return value is True if the parameters are OK.
  // if writeTables is True, the vectors are filled and the SPW, DD, and SOURCE tables are modified;
  // the return value in this case is True only if a successful modification (or none) took place
  Bool setRegridParameters(vector<Int>& oldSpwId,
			   vector<Int>& oldFieldId,
			   vector<Int>& newDataDescId,
			   vector<Bool>& regrid,
			   vector< Vector<Double> >& xout, 
			   vector< Vector<Double> >& xin, 
			   // This is a temporary fix until InterpolateArray1D<Double, Complex>& works.
			   vector< InterpolateArray1D<Float,Complex>::InterpolationMethod >& method,
			   vector< InterpolateArray1D<Double,Float>::InterpolationMethod >& methodF,
			   Bool& msMod,
			   const String& outframe,
			   const String& regridQuant,
			   const Double regridVeloRestfrq,
			   const String& regridInterpMeth,
			   const Double regridCenter, 
			   const Double regridBandwidth, 
			   const Double regridChanWidth,
			   const Int regridPhaseCenterFieldId, // -2 = take from field table, -1 = use 
			   const MDirection regridPhaseCenter, //    <- this value, >-1 = take from this field
			   const Bool writeTables,
			   LogIO& os,
			   String& regridMessage
			   );

  // combineSpws():
  // make one spectral window from all spws given by the spwids vector, 
  // Vector<Int>(1,-1) means: use all SPWs
  Bool combineSpws(const Vector<Int>& spwids = Vector<Int>(1,-1));

 private:
  // *** Private member functions ***

  Bool putDataColumn(MSColumns& msc, ROArrayColumn<Complex>& data, 
		     const String& colName, const Bool writeToDataCol=False);
  Bool putDataColumn(MSColumns& msc, Cube<Complex>& data, 
		     const String& colName, const Bool writeToDataCol=False);
  Bool getDataColumn(ROArrayColumn<Complex>& data, 
		     const String& colName);

  //method that returns the selected ms (?! - but it's Boolean - RR)
  Bool makeSelection();
  Bool fillAllTables(const String& colname);
  Bool fillDDTables();
  Bool fillFieldTable();
  Bool fillMainTable(const Vector<String>& colNames);
  Bool fillAverMainTable(const Vector<String>& colNames);
  Bool copyAntenna();
  Bool copyFeed();
  Bool copySource();
  Bool copyObservation();
  Bool copyPointing();
  Bool writeDiffSpwShape(const Vector<String>& colNames);
  Bool writeSimilarSpwShape(const Vector<String>& colNames);
  // return the number of unique antennas selected
  //Int numOfBaselines(Vector<Int>& ant1, Vector<Int>& ant2, Bool includeAutoCorr=False);
  // Number of time bins to average into from selected data
  Int numOfTimeBins(const Double timeBin);

  // Used in a couple of places to estimate how much memory to grab.
  Double n_bytes() {return mssel_p.nrow() * nchan_p[0] * npol_p[0] *
                           sizeof(Complex);}

  // Picks a reference to DATA, MODEL_DATA, or CORRECTED_DATA out of ms_p.
  const ROArrayColumn<Complex>& right_column(const ROMSColumns *ms_p, const String& colName);

  // Figures out the number, maximum, and index of the selected antennas.
  uInt fillAntIndexer(const ROMSColumns *msc, Vector<Int>& antIndexer);

  //Bool fillAverAntTime();
  Bool fillTimeAverData(const Vector<String>& colNames);

  // Returns whether or not all the elements of inNumChan_p are the
  // same, AND whether all the elements of nchan_p are the same.
  Bool checkSpwShape();

  // Sets up sourceRelabel_p for mapping input SourceIDs (if any) to output
  // ones.  Must be called after fieldid_p is set and before calling
  // fillFieldTable() or copySource().
  void relabelSources();

  void relabelIDs();
  void remapColumn(const ROScalarColumn<Int>& incol, ScalarColumn<Int>& outcol);
  void make_map(const Vector<Int>& mscol, Vector<Int>& mapper);
  uInt remapped(const Int ov, const Vector<Int>& mapper, uInt i);

  // A "Slot" is a subBin, i.e. rows within the same time bin that have
  // different Data Descriptors, Field_IDs, Array_IDs, or States, and so should
  // not be averaged together.  This function returns the Slot number
  // corresponding to the Data Descriptor (dd), Field_ID, Array_ID, and State.
  uInt rowProps2slotKey(const Int ant1,  const Int ant2, const Int dd, 
			const Int field, const Int scan, const Int state,
                        const uInt array);

  // *** Member variables ***

  // Initialized* by ctors.  (Maintain order both here and in ctors.)
  //  * not necessarily to anything useful.
  MeasurementSet ms_p, mssel_p;
  MSColumns * msc_p;		// columns of msOut_p
  ROMSColumns * mscIn_p;
  Bool chanModification_p,      // Iff true, the input channels cannot simply
                                // be copied through to the output channels.
       antennaSel_p,
       sameShape_p;
  Double timeBin_p;
  uInt numOutRows_p;
  String scanString_p, uvrangeString_p, taqlString_p;
  String timeRange_p, arrayExpr_p;
  uInt nant_p;

  // Uninitialized by ctors.
  MeasurementSet msOut_p;
  Vector<Int> spw_p,      // The input spw corresponding to each output spw.
              spw_uniq_p, // Uniquified version of spw_p.
              spwind_to_min_spwind_p,
              nchan_p,    // The # of output channels for each range.
              totnchan_p, // The # of output channels for each output spw.
              chanStart_p,
              chanStep_p,
              npol_p,
              inNumChan_p;
  Vector<Int> fieldid_p;
  Bool averageChannel_p;
  Vector<Int> spwRelabel_p, fieldRelabel_p, sourceRelabel_p;
  Vector<Int> oldDDSpwMatch_p;
  Vector<String> antennaSelStr_p;
  Vector<Int> antennaId_p;
  Vector<Int> antIndexer_p;
  Vector<Int> antNewIndex_p;
  Vector<Int> feedNewIndex_p;
  Vector<Int> arrayId_p;
  Vector<Double> newTimeVal_p;
  Vector<uInt> tOI_p; //timeOrderIndex
  Vector<uInt> spw2ddid_p;

  Vector<Int> arrayRemapper_p, scanRemapper_p, stateRemapper_p; 

  // Each bin gets a map which maps its set of slot keys from
  // rowProps2slotKey() to lists of the row numbers in mscIn_p that belong to
  // the slot.
  Vector<ui2vmap> bin_slots_p;

  Bool doImgWts_p; // Use doWriteImagingWeight() to get this!  Cutting corners
                   // just leads to trouble...
};


} //# NAMESPACE CASA - END

#ifndef AIPS_NO_TEMPLATE_SRC
#include <msvis/MSVis/SubMS.tcc>
#endif //# AIPS_NO_TEMPLATE_SRC

#endif

