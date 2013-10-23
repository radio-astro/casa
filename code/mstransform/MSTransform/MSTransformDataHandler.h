//# MSTransformDataHandler.h: This file contains the interface definition of the MSTransformDataHandler class.
//#
//#  CASA - Common Astronomy Software Applications (http://casa.nrao.edu/)
//#  Copyright (C) Associated Universities, Inc. Washington DC, USA 2011, All rights reserved.
//#  Copyright (C) European Southern Observatory, 2011, All rights reserved.
//#
//#  This library is free software; you can redistribute it and/or
//#  modify it under the terms of the GNU Lesser General Public
//#  License as published by the Free software Foundation; either
//#  version 2.1 of the License, or (at your option) any later version.
//#
//#  This library is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY, without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//#  Lesser General Public License for more details.
//#
//#  You should have received a copy of the GNU Lesser General Public
//#  License along with this library; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
//#  MA 02111-1307  USA
//# $Id: $

#ifndef MSTransformDataHandler_H_
#define MSTransformDataHandler_H_

// Measurement Set
#include <ms/MeasurementSets.h>

// Measurement Set Selection
#include <ms/MeasurementSets/MSSelection.h>
#include <ms/MeasurementSets/MSSelectionTools.h>

// Needed by setupMS
#include <tables/Tables.h>
#include <ms/MeasurementSets/MSTileLayout.h>
#include <asdmstman/AsdmStMan.h>

// Needed by copyTable
#include <tables/Tables/PlainTable.h>

// OS methods needed by fillSubTables
#include <casa/OS/Timer.h>
#include <casa/OS/Path.h>
#include <casa/OS/Directory.h>


namespace casa { //# NAMESPACE CASA - BEGIN

//  MSTransformDataHandler definition
class MSTransformDataHandler
{

public:

	enum asdmStManUseAlternatives
	{
		DONT,
		USE_FOR_DATA,
		USE_FOR_DATA_WEIGHT_SIGMA_FLAG
	};

	MSTransformDataHandler(String& theMS, Table::TableOption option);
	MSTransformDataHandler(MeasurementSet& ms);
	~MSTransformDataHandler();

	// Declared static because it's used in setupMS().
	// colNameList is internally up-cased, so it is not const or passed by reference.
	static const Vector<MS::PredefinedColumns>& parseColumnNames(String colNameList);

	// This version uses the MeasurementSet to check what columns are present,
	// i.e. it makes col=="all" smarter, and it is not necessary to call
	// verifyColumns() after calling this.  Unlike the other version, it knows
	// about FLOAT_DATA and LAG_DATA.  It throws an exception if a
	// _specifically_ requested column is absent.
	static const Vector<MS::PredefinedColumns>& parseColumnNames(String colNameList, const MeasurementSet& ms);

	// Helper function for parseColumnNames().  Converts col to a list of
	// MS::PredefinedColumnss, and returns the # of recognized data columns.
	// static because parseColumnNames() is static.
	static uInt dataColStrToEnums(const String& col,Vector<MS::PredefinedColumns>& colvec);

	// Selection method using msselection syntax. Time is not handled by this method.
	Bool setmsselect(	const String& spw = "", const String& field = "",
						const String& baseline = "", const String& scan = "",
						const String& uvrange = "", const String& taql = "",
						const Vector<Int>& step = Vector<Int> (1, 1),
						const String& subarray = "", const String& correlation = "",
						const String& intent = "", const String& obs = "");

	// Select source or field
	Bool selectSource(const Vector<Int>& fieldid);

	// Select spw and channels for each spw.
	Bool selectSpw(const String& spwstr, const Vector<Int>& steps);

	// Returns the set (possibly empty) of spectral windows that are
	// in SPW but not listed in ms's DATA_DESCRIPTION sub-table.
	// (This happens with certain calibration/hardware setups.)
	static std::set<Int> findBadSpws(MeasurementSet& ms, Vector<Int> spwv);

	// Select Antennas to split out
	void selectAntenna(const Vector<Int>& antennaids,const Vector<String>& antennaSel);

	// Helper function for selectAntenna()
	static Bool pickAntennas(	Vector<Int>& selected_antennaids,
								Vector<String>& selected_antenna_strs,
								const Vector<Int>& antennaids,
								const Vector<String>& antennaSel);

	// Select array IDs to use.
	void selectArray(const String& subarray);

	// Setup polarization selection (for now, only from available correlations - no Stokes transformations.)
	Bool selectCorrelations(const String& corrstr);

	// Fills outToIn[pol] with a map from output correlation index to input
	// correlation index, for each input polID pol.
	// It does not yet check the appropriateness of the correlation selection
	// string, so ignore the return value for now.  outToIn[pol] defaults to
	// an empty Vector if no correlations are selected for pol.
	// That is not the same as the default "select everything in ms".
	static Bool getCorrMaps(	MSSelection& mssel,
								const MeasurementSet& ms,
								Vector<Vector<Int> >& outToIn,
								const Bool areSelecting = false);

	// Select time parameters
	void selectTime(Double timeBin=-1.0, String timerng="");

	//Method to make the basic structure of the MS
	//
	//TileShape of size 1 can have 2 values [0], and [1] ...these are used in to
	//determine the tileshape by using MSTileLayout. Otherwise it has to be a
	//vector size 3 e.g [4, 15, 351] => a tile shape of 4 stokes, 15 channels 351
	//rows.
	//
	// combine sets combine_p.  (Columns to ignore while time averaging.)
	//
	Bool makeMSBasicStructure(	String& msname,
								String& whichDataCol,
								const Vector<Int>& tileShape = Vector<Int> (1, 0),
								const String& combine = "");

	Bool isAllColumns(const Vector<MS::PredefinedColumns>& colNames);

	// Method that returns the selected ms (?! - but it's Boolean - RR)
	Bool makeSelection();

	// This sets up a default new ms
	// Declared static as it can be (and is) called directly from outside
	// Therefore it is not dependent on any member variable.
	static MeasurementSet* setupMS(	const String& msname, const Int nchan,
									const Int npol, const String& telescop,
									const Vector<MS::PredefinedColumns>& colNamesTok,
									const Int obstype = 0, const Bool compress = False,
									const asdmStManUseAlternatives asdmStManUse = DONT);

	// Same as above except allowing manual tileshapes
	static MeasurementSet* setupMS(	const String& msname, const Int nchan,
									const Int npol, const Vector<MS::PredefinedColumns>& colNamesTok,
									const Vector<Int>& tileShape = Vector<Int> (1, 0),
									const Bool compress = False,
									const asdmStManUseAlternatives asdmStManUse = DONT);


	// The output MS must have (at least?) 1 of DATA, FLOAT_DATA, or LAG_DATA.
	// MODEL_DATA or CORRECTED_DATA will be converted to DATA if necessary.
	// jagonzal (CAS-5327): The implementation has to go here because a member function cannot have static linkage
	static Bool mustConvertToData(	const uInt nTok,const Vector<MS::PredefinedColumns>& datacols)
	{
		return (nTok == 1) && (datacols[0] != MS::FLOAT_DATA) && (datacols[0] != MS::LAG_DATA);
	}

	// A customized version of MS::createDefaultSubtables().
	static void createSubtables(MeasurementSet& ms, Table::TableOption option);

	// Sub-table fillers.
	Bool fillSubTables(const Vector<MS::PredefinedColumns>& colNames);
	Bool fillFieldTable();
	Bool fillDDTables(); // Includes spw and pol

	// Add optional columns to outTab if present in inTab and possColNames.
	// beLazy should only be true if outTab is in its default state.
	// Returns the number of added columns.
	static uInt addOptionalColumns(const Table& inTab, Table& outTab,const Bool beLazy=false);

	// Sets up sourceRelabel_p for mapping input SourceIDs (if any) to output
	// ones.  Must be called after fieldid_p is set and before calling
	// fillFieldTable() or copySource().
	void relabelSources();

	// Adds and copies inTab to msOut_p without any filtering.
	// tabName is the table "type", i.e. POINTING or SYSPOWER
	// without the preceding path.
	//
	// If noRows is True, the structure will be setup but no
	// rows will be copied  (useful for filtering).
	void copySubtable(const String& tabName, const Table& inTab,const Bool noRows = False);

	// Sets mapper to to a map from the distinct values of inv,
	// in increasing order, to 0, 1, 2, ..., mapper.size() - 1.
	static void make_map(std::map<Int, Int>& mapper, const Vector<Int>& inv);

	Bool copyPointing();
	// Sets up the stub of a POINTING, enough to create an MSColumns.
	void setupNewPointing();

	Bool copySource();
	Bool copyAntenna();
	Bool copyFeed();
	Bool copyFlag_Cmd();
	Bool copyHistory();
	Bool copyObservation();
	Bool copyProcessor();
	Bool copyState();
	Bool copySyscal();
	Bool copyWeather();

	// This falls between copyGenericSubtables() and the copiers for standard
	// sub-tables like copyFeed().  It is for optional sub-tables like CALDEVICE
	// and SYSPOWER which can be watched for by name and may need their
	// ANTENNA_ID and SPECTRAL_WINDOW_ID columns re-mapped.
	// (Technically FEED_ID, too, if split ever starts re-mapping feeds.)
	//
	// It must be called BEFORE copyGenericSubtables()!
	//
	Bool filterOptSubtable(const String& subtabname);

	Bool copyGenericSubtables();

	// To consolidate several SPW sub-tables
	static Bool mergeSpwSubTables(Vector<String> filenames);
	static Bool mergeDDISubTables(Vector<String> filenames);

	// Accesors for the MS objects
	MeasurementSet * getInputMS() {return &ms_p;};
	MeasurementSet * getSelectedInputMS() {return &mssel_p;};
	MeasurementSet * getOutputMS() {return &msOut_p;};
	ROMSColumns * getSelectedInputMSColumns() {return mscIn_p;};
	MSColumns * getOutputMSColumns() {return msc_p;};

	// Accesors for the Re-mapper objects
	map<Int, Int> & getStateRemapper() {return stateRemapper_p;};
	Vector<Int> & getAntennaRemapper() {return antNewIndex_p;};

protected:

	// Initialized* by ctors.  (Maintain order both here and in ctors.)
	//  * not necessarily to anything useful.
	MeasurementSet ms_p, mssel_p;
	MSColumns * msc_p; // columns of msOut_p
	ROMSColumns * mscIn_p;
	Bool keepShape_p, // Iff true, each output array has the
			// same shape as the corresponding input one.
			// sameShape_p,             // Iff true, the shapes of the arrays do not
			//  			// vary with row number.
			antennaSel_p; // Selecting by antenna?
	Double timeBin_p;
	String scanString_p, // Selects scans by #number#.  Historically named.
			intentString_p, // Selects scans by string.  scanString_p was taken.
			obsString_p, // String for observationID selection.
			uvrangeString_p, taqlString_p;
	String timeRange_p, arrayExpr_p, corrString_p;
	String combine_p; // Should time averaging not split bins by
	// scan #, observation, and/or state ID?
	// Must be lowercase at all times.
	Int fitorder_p; // The polynomial order for continuum fitting.
	// If < 0 (default), continuum subtraction is
	// not done.
	String fitspw_p; // Selection string for line-free channels.
	String fitoutspw_p; // Selection string for output channels if doing
	// continuum subtraction.

	// Uninitialized by ctors.
	MeasurementSet msOut_p;
	Vector<Int> spw_p, // The input spw corresponding to each output spw.
			spw_uniq_p, // Uniquified version of spw_p.
			 nchan_p, // The # of output channels for each range.
			totnchan_p, // The # of output channels for each output spw.
			chanStart_p, // 1st input channel index in a selection.
			chanEnd_p, // last input channel index in a selection.
			chanStep_p, // Increment between input chans, i.e. if 3, only every third
			// input channel will be used.
			widths_p, // # of input chans per output chan for each range.
			ncorr_p, // The # of output correlations for each DDID.
			inNumChan_p, // The # of input channels for each spw.
			inNumCorr_p; // The # of input correlations for each DDID.
	Vector<Int> fieldid_p;
	Vector<Int> spwRelabel_p, fieldRelabel_p, sourceRelabel_p;
	Vector<Int> oldDDSpwMatch_p;
	Vector<String> antennaSelStr_p;
	Vector<Int> antennaId_p;
	Vector<Int> antIndexer_p;
	Vector<Int> antNewIndex_p;

	Vector<Int> selObsId_p; // List of selected OBSERVATION_IDs.
	Vector<Int> polID_p; // Map from input DDID to input polID, filled in fillDDTables().
	Vector<Int> spw2ddid_p;

	// inCorrInd = outPolCorrToInCorrMap_p[polID_p[ddID]][outCorrInd]
	Vector<Vector<Int> > inPolOutCorrToInCorrMap_p;

	std::map<Int, Int> stateRemapper_p;

	Vector<Vector<Slice> > chanSlices_p; // Used by VisIterator::selectChannel()
	Vector<Slice> corrSlice_p;
	Vector<Vector<Slice> > corrSlices_p; // Used by VisIterator::selectCorrelation()
	Matrix<Double> selTimeRanges_p;

};

} //# NAMESPACE CASA - END

#endif /* MSTransformDataHandler_H_ */
