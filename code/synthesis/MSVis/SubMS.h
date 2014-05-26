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
#include <ms/MeasurementSets/MSMainEnums.h>
//#include <synthesis/MSVis/VisIterator.h>
#include <synthesis/MSVis/VisBufferComponents.h>
#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
//#include <casa/Utilities/CountedPtr.h>
#include <map>
#include <set>
#include <vector>
#include <scimath/Mathematics/InterpolateArray1D.h>

#include <mstransform/MSTransform/MSTransformManager.h>


#ifndef MSVIS_SUBMS_H
namespace casa { //# NAMESPACE CASA - BEGIN

#define MSVIS_SUBMS_H

namespace subms {
// Returns wt**-0.5 or -1, depending on whether wt is positive.
// NOT a member function, so it can be easily passed to other functions
// (i.e. arrayTransformInPlace).
Double wtToSigma(Double wt);
Double sigToWeight(Double sig);
}

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
class MSSelection; // #include <ms/MeasurementSets/MSSelection.h>
class VBRemapper;

  // // These typedefs are necessary because a<b::c> doesn't work.
  // typedef std::vector<uInt> uivector;
  // struct uIntCmp 
  // {
  //   bool operator()(const uInt i1, const uInt i2) const 
  //   {
  //     return i1 < i2;
  //   }
  // };
  // typedef std::map<const uInt, uivector, uIntCmp> ui2vmap;

template<class T> class ROArrayColumn;
  Bool isAllColumns(const Vector<MS::PredefinedColumns>& colNames);

class SubMS
{

 // jagonzal: Allow TransformVisDataHandler to access protected methods and members of this class
 friend class MSTransformManager;

 public:

  enum RegriddingAlternatives {
    useFFTShift = -100,   // needs input and output grid to have the same number of channels and be equidistant in freq.
    useLinIntThenFFTShift // for the case the input grid is not equidistant in frequency but the output grid is
  };

  enum asdmStManUseAlternatives {
    DONT,
    USE_FOR_DATA,
    USE_FOR_DATA_WEIGHT_SIGMA_FLAG
  };

  SubMS(String& theMS, Table::TableOption option = Table::Old);
  
  // construct from an MS
  SubMS(MeasurementSet& ms);

  virtual ~SubMS();
  
  // Change or Set the MS this MSSelector refers to.
  void setMS(MeasurementSet& ms);

  // Returns the set (possibly empty) of spectral windows that are in spwv but
  // not listed in ms's DATA_DESCRIPTION subtable.  (This happens with certain
  // calibration/hardware setups.)
  static std::set<Int> findBadSpws(MeasurementSet& ms, Vector<Int> spwv);

  // Select spw and channels for each spw.
  // This is the version used by split.  It returns true on success and false
  // on failure.
  Bool selectSpw(const String& spwstr, const Vector<Int>& steps);

  // This older version is used by the older version of setmsselect().
  void selectSpw(Vector<Int> spw, Vector<Int> nchan, Vector<Int> start, 
                 Vector<Int> step);
  
  // Setup polarization selection (for now, only from available correlations -
  // no Stokes transformations.)
  Bool selectCorrelations(const String& corrstr);

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
		   const String& subarray="", const String& correlation="",
                   const String& intent="", const String& obs="");

  // This older version does not return a success value, and does need nchan,
  // start, and step.  It is used elsewhere (i.e. ImagerMultiMS).
  void setmsselect(const String& spw,        const String& field, 
                   const String& baseline,   const String& scan,
                   const String& obs,        const String& uvrange,
                   const String& taql,       const Vector<Int>& nchan,
                   const Vector<Int>& start, const Vector<Int>& step,
                   const String& subarray, const String& intent="");

  // Select source or field
  Bool selectSource(const Vector<Int>& fieldid);
  
  // Select Antennas to split out  
  void selectAntenna(const Vector<Int>& antennaids,
		     const Vector<String>& antennaSel)
  {
    antennaSel_p = pickAntennas(antennaId_p, antennaSelStr_p,
				antennaids, antennaSel);
  }
  static Bool pickAntennas(Vector<Int>& selected_antennaids,
			   Vector<String>& selected_antenna_strs,
			   const Vector<Int>& antennaids,
			   const Vector<String>& antennaSel);
  
  // Select array IDs to use.
  void selectArray(const String& subarray) {arrayExpr_p = subarray;}

  //select time parameters
  void selectTime(Double timeBin=-1.0, String timerng="");

  //void selectSource(Vector<String> sourceid);

  //Method to set if a phase Center rotation is needed
  //void setPhaseCenter(Int fieldid, MDirection& newPhaseCenter);

  // Sets the polynomial order for continuum fitting to fitorder.
  // If < 0, continuum subtraction is not done.
  void setFitOrder(Int fitorder, Bool advise=true);
  // Set the selection string for line-free channels.
  void setFitSpw(const String& fitspw) {fitspw_p = fitspw;}
  // Selection string for output channels if doing continuum subtraction.
  void setFitOutSpw(const String& fitoutspw) {fitoutspw_p = fitoutspw;}

  //Method to make the subMS
  //
  //TileShape of size 1 can have 2 values [0], and [1] ...these are used in to
  //determine the tileshape by using MSTileLayout. Otherwise it has to be a
  //vector size 3 e.g [4, 15, 351] => a tile shape of 4 stokes, 15 channels 351
  //rows.
  //
  // combine sets combine_p.  (Columns to ignore while time averaging.)
  //
  Bool makeSubMS(String& submsname, String& whichDataCol,
                 const Vector<Int>& tileShape=Vector<Int>(1, 0),
                 const String& combine="");

  //Method to make a scratch subMS and even in memory if posssible
  //Useful if temporary subselection/averaging is necessary
  // It'll be in memory if the basic output ms is less than half of 
  // memory reported by HostInfo unless forced to by user...
  virtual MeasurementSet* makeScratchSubMS(const Vector<MS::PredefinedColumns>& whichDataCols, 
				   const Bool forceInMemory=False);
  // In this form whichDataCol gets passed to parseColumnNames().
  virtual MeasurementSet* makeScratchSubMS(const String& whichDataCol, 
				   const Bool forceInMemory=False);

  // This sets up a default new ms
  // Declared static as it can be (and is) called directly outside of SubMS.
  // Therefore it is not dependent on any member variable.
  static MeasurementSet* setupMS(const String& msname, const Int nchan,
                                 const Int npol, const String& telescop,
                                 const Vector<MS::PredefinedColumns>& colNamesTok,
				 const Int obstype=0,
                                 const Bool compress=False,
				 const asdmStManUseAlternatives asdmStManUse=DONT);

  // Same as above except allowing manual tileshapes
  static MeasurementSet* setupMS(const String& msname, const Int nchan,
                                 const Int npol,
                                 const Vector<MS::PredefinedColumns>& colNamesTok,
				 const Vector<Int>& tileShape=Vector<Int>(1,0),
                                 const Bool compress=False,
				 const asdmStManUseAlternatives asdmStManUse=DONT);

  
  // Add optional columns to outTab if present in inTab and possColNames.
  // beLazy should only be true if outTab is in its default state.
  // Returns the number of added columns.
  static uInt addOptionalColumns(const Table& inTab, Table& outTab,
                                 const Bool beLazy=false);

  // Like TableCopy::copyRows, but by column.
  static Bool copyCols(Table& out, const Table& in, const Bool flush=True);

  // A customized version of MS::createDefaultSubtables().
  static void createSubtables(MeasurementSet& ms, Table::TableOption option);

  // Declared static because it's used in setupMS().  Therefore it can't use
  // any member variables.  It is also used in MSFixvis.cc.
  // colNameList is internally upcased, so it is not const or passed by reference.
  static const Vector<MS::PredefinedColumns>& parseColumnNames(String colNameList);
  // This version uses the MeasurementSet to check what columns are present,
  // i.e. it makes col=="all" smarter, and it is not necessary to call
  // verifyColumns() after calling this.  Unlike the other version, it knows
  // about FLOAT_DATA and LAG_DATA.  It throws an exception if a
  // _specifically_ requested column is absent.
  static const Vector<MS::PredefinedColumns>& parseColumnNames(String colNameList,
                                                    const MeasurementSet& ms);

  void verifyColumns(const MeasurementSet& ms, const Vector<MS::PredefinedColumns>& colNames);

  // The output MS must have (at least?) 1 of DATA, FLOAT_DATA, or LAG_DATA.
  // MODEL_DATA or CORRECTED_DATA will be converted to DATA if necessary.
  static Bool mustConvertToData(const uInt nTok,
                                const Vector<MS::PredefinedColumns>& datacols)
  {
    return (nTok == 1) && (datacols[0] != MS::FLOAT_DATA) &&
      (datacols[0] != MS::LAG_DATA);
  }

  static Bool sepFloat(const Vector<MS::PredefinedColumns>& anyDataCols,
                       Vector<MS::PredefinedColumns>& complexDataCols);

  // Fills outToIn[pol] with a map from output correlation index to input
  // correlation index, for each input polID pol.
  // It does not yet check the appropriateness of the correlation selection
  // string, so ignore the return value for now.  outToIn[pol] defaults to
  // an empty Vector if no correlations are selected for pol.
  // That is not the same as the default "select everything in ms".
  static Bool getCorrMaps(MSSelection& mssel, const MeasurementSet& ms,
			  Vector<Vector<Int> >& outToIn,
			  const Bool areSelecting=false);
  
  // Replaces col[i] with mapper[col[i]] for each element of col.
  // Does NOT check whether mapper[col[i]] is defined, but it does return
  // right away (i.e. a no-op) if mapper is empty.
  static void remap(Vector<Int>& col, const Vector<Int>& mapper);
  static void remap(Vector<Int>& col, const std::map<Int, Int>& mapper);

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
		const Bool doHanningSmooth=False,
		const Int phaseCenterFieldId=-2, // -2 = use pahse center from field table
		MDirection phaseCenter=MDirection(), // this direction is used if phaseCenterFieldId==-1
		const Bool centerIsStart=False, // if true, the parameter regridCenter specifies the start
		const Bool startIsEnd=False, // if true, and centerIsStart is true, regridCenter specifies the upper end in frequency
		const Int nchan=0, // if >0: used instead of regridBandwidth, ==
		const Int width=0, // if >0 and regridQuant=="freq": used instead of regridChanWidth
		const Int start=-1 // if >=0 and regridQuant=="freq": used instead of regridCenter
		);

  // the following inline convenience methods for regridSpw bypass the whole CASA measure system
  // because when they are used, they can assume that the frame stays the same and the units are OK
  static lDouble vrad(const lDouble freq, const lDouble rest){ return (C::c * (1. - freq/rest)); };
  static lDouble vopt(const lDouble freq, const lDouble rest){ return (C::c *(rest/freq - 1.)); };
  static lDouble lambda(const lDouble freq){ return (C::c/freq); };
  static lDouble freq_from_vrad(const lDouble vrad, const lDouble rest){ return (rest * (1. - vrad/C::c)); };
  static lDouble freq_from_vopt(const lDouble vopt, const lDouble rest){ return (rest / (1. + vopt/C::c)); };
  static lDouble freq_from_lambda(const lDouble lambda){ return (C::c/lambda); };
  
  // Support method for regridSpw():
  // results in the column oldName being renamed to newName, and a new column
  // which is an empty copy of oldName being created together with a
  // TileShapeStMan data manager and hypercolumn (name copied from the old
  // hypercolumn) with given dimension, the old hypercolumn of name
  // hypercolumnName is renamed to name+"B"
  Bool createPartnerColumn(TableDesc& modMSTD, const String& oldName,
                           const String& newName, const Int& hypercolumnDim,
                           const IPosition& tileShape);

  // Support method for regridSpw():
  // calculate the final new channel boundaries from the regridding parameters
  // and the old channel boundaries (already transformed to the desired
  // reference frame); returns False if input paramters were invalid and no
  // useful boundaries could be created
  static Bool regridChanBounds(Vector<Double>& newChanLoBound, 
			       Vector<Double>& newChanHiBound,
			       const Double regridCenter, 
			       const Double regridBandwidth,
			       const Double regridChanWidth,
			       const Double regridVeloRestfrq, 
			       const String regridQuant,
			       const Vector<Double>& transNewXin, 
			       const Vector<Double>& transCHAN_WIDTH,
			       String& message, // message to the user, epsecially in case of error 
			       const Bool centerIsStart=False, // if true, the parameter regridCenter specifies the start
			       const Bool startIsEnd=False, // if true, and centerIsStart is true, regridCenter specifies the upper end in frequency
			       const Int nchan=0, // if != 0 : used instead of regridBandwidth, -1 means use all channels
			       const Int width=0, // if >0 and regridQuant=="freq": used instead of regridChanWidth
			       const Int start=-1 // if >=0 and regridQuant=="freq": used instead of regridCenter
			       );

  // a helper function for handling the gridding parameter user input
  static Bool convertGridPars(LogIO& os,
			      const String& mode, 
			      const int nchan, 
			      const String& start, 
			      const String& width,
			      const String& interp, 
			      const String& restfreq, 
			      const String& outframe,
			      const String& veltype,
			      String& t_mode,
			      String& t_outframe,
			      String& t_regridQuantity,
			      Double& t_restfreq,
			      String& t_regridInterpMeth,
			      Double& t_cstart, 
			      Double& t_bandwidth,
			      Double& t_cwidth,
			      Bool& t_centerIsStart, 
			      Bool& t_startIsEnd,			      
			      Int& t_nchan,
			      Int& t_width,
			      Int& t_start);

  // A wrapper for SubMS::regridChanBounds() which takes the user interface type gridding parameters
  // The ready-made grid is returned in newCHAN_FREQ and newCHAN_WIDTH
  static Bool calcChanFreqs(LogIO& os,
			    // output
			    Vector<Double>& newCHAN_FREQ,
			    Vector<Double>& newCHAN_WIDTH,
			    // input
			    const Vector<Double>& oldCHAN_FREQ, // the original grid
			    const Vector<Double>& oldCHAN_WIDTH, 
			    // the gridding parameters
			    const MDirection  phaseCenter,
			    const MFrequency::Types theOldRefFrame,
			    const MEpoch theObsTime,
			    const MPosition mObsPos,
			    const String& mode, 
			    const int nchan, 
			    const String& start, 
			    const String& width,
			    const String& restfreq, 
			    const String& outframe,
			    const String& veltype,
			    const Bool verbose=False,
			    const MRadialVelocity mRV=MRadialVelocity() // additional radial velo shift to apply, 
                                                                        // used e.g. when outframe=="SOURCE"
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
			   vector<Bool>& transform,
			   vector<MDirection>& theFieldDirV,
			   vector<MPosition>& mObsPosV,
			   vector<MFrequency::Types>& fromFrameTypeV,
			   vector<MFrequency::Ref>& outFrameV,
			   vector<Double>& weightScaleV,
			   vector< Vector<Double> >& xold, 
			   vector< Vector<Double> >& xout, 
			   vector< Vector<Double> >& xin, 
			   vector< Int >& method, // interpolation method cast to Int
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
			   String& regridMessage,
			   const Bool centerIsStart=False, // if true, the parameter regridCenter specifies the start
			   const Bool startIsEnd=False, // if true, and centerIsStart is true, regridCenter specifies the upper end in frequency
			   const Int nchan=0, // if >0: used instead of regridBandwidth
			   const Int width=0, // if >0 and regridQuant=="freq": used instead of regridChanWidth
			   const Int start=-1 // if >=0 and regridQuant=="freq": used instead of regridCenter
			   );

  // combineSpws():
  // make one spectral window from all spws given by the spwids vector
  Bool combineSpws(const Vector<Int>& spwids,  // Vector<Int>(1,-1) means: use all SPWs
		   const Bool noModify,   // if True, the MS will not be modified
		   Vector<Double>& newCHAN_FREQ, // will return the grid of the resulting SPW
		   Vector<Double>& newCHAN_WIDTH,
		   Bool verbose=False
		   );

  Bool combineSpws(const Vector<Int>& spwids = Vector<Int>(1,-1)){  // Vector<Int>(1,-1) means: use all SPWs
    Vector<Double> temp1; 
    Vector<Double> temp2;
    return combineSpws(spwids, False, temp1, temp2, True);
  }

  // Fills mapper[ntok] with a map from dataColumn indices to ArrayColumns in
  // the output.  mapper must have ntok slots!
  static void getDataColMap(MSColumns* msc, ArrayColumn<Complex>* mapper,
			    uInt ntok,
			    const Vector<MS::PredefinedColumns>& colEnums); 

 protected:

  //method that returns the selected ms (?! - but it's Boolean - RR)
  Bool makeSelection();

  // (Sub)table fillers.
  Bool fillAllTables(const Vector<MS::PredefinedColumns>& colNames);
  Bool fillDDTables();		// Includes spw and pol.
  Bool fillFieldTable();
  
  // Used to be fillMainTable(colnames), but what distinguishes it from
  // writeSomeMainRows(colnames) is that it is for cases where there is
  // a 1-1 match between rows in mssel_p and msOut_p (including order).
  Bool writeAllMainRows(const Vector<MS::PredefinedColumns>& colNames);

  // Used to be fillAverMainTable(colnames), but what distinguishes it from
  // writeAllMainRows(colnames) is that it is for cases where there is not
  // necessarily a 1-1 match between rows in mssel_p and msOut_p.
  Bool writeSomeMainRows(const Vector<MS::PredefinedColumns>& colNames);

  Bool copyAntenna();
  Bool copyFeed();
  Bool copyFlag_Cmd();
  Bool copyHistory();
  Bool copyObservation();
  Bool copyPointing();
  Bool copyProcessor();
  Bool copySource();
  Bool copyState();
  Bool copySyscal();
  Bool copyWeather();
  Bool copyGenericSubtables();
  void copyMainTableKeywords(TableRecord& outKeys,const TableRecord& inKeys);

  // This falls between copyGenericSubtables() and the copiers for standard
  // subtables like copyFeed().  It is for optional subtables like CALDEVICE
  // and SYSPOWER which can be watched for by name and may need their
  // ANTENNA_ID and SPECTRAL_WINDOW_ID columns remapped.  (Technically FEED_ID,
  // too, if split ever starts remapping feeds.)
  //
  // It must be called BEFORE copyGenericSubtables()!
  //
  Bool filterOptSubtable(const String& subtabname);

  //  Bool writeDiffSpwShape(const Vector<MS::PredefinedColumns>& colNames);
  Bool fillAccessoryMainCols();

  // *** Private member functions ***

  // Adds and copies inTab to msOut_p without any filtering.
  // tabName is the table "type", i.e. POINTING or SYSPOWER without the
  // preceding path.
  //
  // If noRows is True, the structure will be setup but no rows will be
  // copied.  (Useful for filtering)
  void copySubtable(const String& tabName, const Table& inTab,
                    const Bool noRows=False);

  Bool getDataColumn(ROArrayColumn<Complex>& data,
                     const MS::PredefinedColumns colName);
  Bool getDataColumn(ROArrayColumn<Float>& data,
                     const MS::PredefinedColumns colName);
  Bool putDataColumn(MSColumns& msc, ROArrayColumn<Complex>& data,
                     const MS::PredefinedColumns datacol,
                     const Bool writeToDataCol=False);
  Bool putDataColumn(MSColumns& msc, ROArrayColumn<Float>& data,
                     const MS::PredefinedColumns datacol,
                     const Bool writeToDataCol=False);

  // This method uses VisIter for efficient copy mode data transfer
  Bool copyDataFlagsWtSp(const Vector<MS::PredefinedColumns>& colNames,
                         const Bool writeToDataCol);

  // Like doTimeAver(), but it subtracts the continuum instead of time
  // averaging.  Unlike doTimeAver(), it infers writeToDataCol from
  // colNames.nelements() (subtractContinuum is intentionally not as general as
  // copyDataFlagsWtSp), and writes according to fitoutspw_p instead of spw_p.
  Bool subtractContinuum(const Vector<MS::PredefinedColumns>& colNames,
                         const VBRemapper& remapper);
  
  // Helper function for parseColumnNames().  Converts col to a list of
  // MS::PredefinedColumnss, and returns the # of recognized data columns.
  // static because parseColumnNames() is static.
  static uInt dataColStrToEnums(const String& col,
                                Vector<MS::PredefinedColumns>& colvec);
    
  Bool doChannelMods(const Vector<MS::PredefinedColumns>& colNames);

  void fill_vbmaps(std::map<VisBufferComponents::EnumType,
                            std::map<Int, Int> >& vbmaps);

  // return the number of unique antennas selected
  //Int numOfBaselines(Vector<Int>& ant1, Vector<Int>& ant2,
  //                   Bool includeAutoCorr=False);

  // Used in a couple of places to estimate how much memory to grab.
  Double n_bytes() {return mssel_p.nrow() * nchan_p[0] * ncorr_p[0] *
                           sizeof(Complex);}

  // Picks a reference to DATA, MODEL_DATA, CORRECTED_DATA, or LAG_DATA out
  // of ms_p.  FLOAT_DATA is not included because it is not natively complex. 
  const ROArrayColumn<Complex>& right_column(const ROMSColumns *ms_p,
                                             const MS::PredefinedColumns datacol);

  // The writable version of the above.
  ArrayColumn<Complex>& right_column(MSColumns *msclala,
				     const MS::PredefinedColumns col,
				     const Bool writeToDataCol);

  // Figures out the number, maximum, and index of the selected antennas.
  uInt fillAntIndexer(std::map<Int, Int>& antIndexer, const ROMSColumns *msc);

  // Read the input, time average it to timeBin_p, and write the output.
  // The first version uses VisibilityIterator (much faster), but the second
  // supports correlation selection using VisIterator.  VisIterator should be
  // sped up soon!
  Bool doTimeAver(const Vector<MS::PredefinedColumns>& dataColNames,
                  const VBRemapper& remapper);
  Bool doTimeAverVisIterator(const Vector<MS::PredefinedColumns>& dataColNames,
                  const VBRemapper& remapper);

  void getDataColMap(ArrayColumn<Complex>* mapper, uInt ntok,
                     const Vector<MS::PredefinedColumns>& colEnums)
  {
    getDataColMap(msc_p, mapper, ntok, colEnums);
  }

  // Returns whether or not the numbers of correlations and channels
  // are independent of DATA_DESCRIPTION_ID, for both the input and output
  // MSes.
  Bool areDataShapesConstant();

  // Returns whether or not the input MS has a valid FLAG_CATEGORY, and its
  // first row has the right number of correlations and channels.
  Bool existsFlagCategory() const;

  // Sets up sourceRelabel_p for mapping input SourceIDs (if any) to output
  // ones.  Must be called after fieldid_p is set and before calling
  // fillFieldTable() or copySource().
  void relabelSources();

  void relabelIDs();

  // Sets outcol to the indices of selvals that correspond to incol, i.e.
  // outcol(i) = j s.t. selvals[j] == incol(i).
  //
  // Ideally selvals is set up so that outcol's values will be row numbers in
  // the corresponding subtable of its ms.
  //
  // Throws an exception if incol and outcol do not have the same # of rows, or
  // incol has a value that is not in selvals.
  void remapColumn(ScalarColumn<Int>& outcol, const ROScalarColumn<Int>& incol,
                   const Vector<Int>& selvals);

  // Equivalent to but slightly more efficient than
  // remapColumn(outcol, incol, incol.getColumn()).
  void remapColumn(ScalarColumn<Int>& outcol, const ROScalarColumn<Int>& incol);

  //static void make_map(const Vector<Int>& mscol, Vector<Int>& mapper);

  // Sets mapper to to a map from the distinct values of inv, in increasing
  // order, to 0, 1, 2, ..., mapper.size() - 1.
  static void make_map(std::map<Int, Int>& mapper, const Vector<Int>& inv);
  // Sets mapper to form a map from inv whose elements are mapped values. It skips
  // to store in mapper if the value is -1
  static void make_map2(std::map<Int, Int>& mapper, const Vector<Int>& inv);

  uInt remapped(const Int ov, const Vector<Int>& mapper, uInt i);

  // Sets up the stub of a POINTING, enough to create an MSColumns.
  void setupNewPointing();

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

  // *** Member variables ***

  // Initialized* by ctors.  (Maintain order both here and in ctors.)
  //  * not necessarily to anything useful.
  MeasurementSet ms_p, mssel_p;
  MSColumns * msc_p;		// columns of msOut_p
  ROMSColumns * mscIn_p;
  Bool keepShape_p,      	// Iff true, each output array has the
				// same shape as the corresponding input one.
       // sameShape_p,             // Iff true, the shapes of the arrays do not
       //  			// vary with row number.
       antennaSel_p;		// Selecting by antenna?
  Double timeBin_p;
  String scanString_p,          // Selects scans by #number#.  Historically named.
         intentString_p,        // Selects scans by string.  scanString_p was taken.
         obsString_p,           // String for observationID selection.
         uvrangeString_p,
         taqlString_p;
  String timeRange_p, arrayExpr_p, corrString_p;
  String combine_p;          // Should time averaging not split bins by
                             // scan #, observation, and/or state ID?
                             // Must be lowercase at all times.
  Int fitorder_p;               // The polynomial order for continuum fitting.
                                // If < 0 (default), continuum subtraction is
                                // not done.
  String fitspw_p;           // Selection string for line-free channels.
  String fitoutspw_p;        // Selection string for output channels if doing
                             // continuum subtraction.

  // jagonzal: Allow main table to be left empty, so that it can be filled by another layer.
  Bool fillMainTable_p;

  // Uninitialized by ctors.
  MeasurementSet msOut_p;
  Vector<Int> spw_p,      // The input spw corresponding to each output spw.
              spw_uniq_p, // Uniquified version of spw_p.
              spwind_to_min_spwind_p,
              nchan_p,    // The # of output channels for each range.
              totnchan_p, // The # of output channels for each output spw.
              chanStart_p, // 1st input channel index in a selection.
              chanEnd_p,   // last input channel index in a selection.
              chanStep_p, // Increment between input chans, i.e. if 3, only every third
                          // input channel will be used. 
              widths_p,   // # of input chans per output chan for each range.
              ncorr_p,    // The # of output correlations for each DDID.
              inNumChan_p,    // The # of input channels for each spw.
              inNumCorr_p;    // The # of input correlations for each DDID.
  Vector<Int> fieldid_p;
  Vector<Int> spwRelabel_p, fieldRelabel_p, sourceRelabel_p;
  Vector<Int> oldDDSpwMatch_p;
  Vector<String> antennaSelStr_p;
  Vector<Int> antennaId_p;
  Vector<Int> antIndexer_p;
  Vector<Int> antNewIndex_p;

  Vector<Int> selObsId_p;  // List of selected OBSERVATION_IDs.
  Vector<Int> polID_p;	       // Map from input DDID to input polID, filled in fillDDTables(). 
  Vector<uInt> spw2ddid_p;

  // inCorrInd = outPolCorrToInCorrMap_p[polID_p[ddID]][outCorrInd]
  Vector<Vector<Int> > inPolOutCorrToInCorrMap_p;

  std::map<Int, Int> stateRemapper_p; 

  Vector<Vector<Slice> > chanSlices_p;  // Used by VisIterator::selectChannel()
  Vector<Slice> corrSlice_p;
  Vector<Vector<Slice> > corrSlices_p;  // Used by VisIterator::selectCorrelation()
  Matrix<Double> selTimeRanges_p;
};

} //# NAMESPACE CASA - END

#endif

