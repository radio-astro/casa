//# MSTransformDataHandler.cc: This file contains the implementation of the MSTransformDataHandler class.
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

#include <mstransform/MSTransform/MSTransformDataHandler.h>
#include <tables/Tables/TableProxy.h>
#include <tables/Tables/TableParse.h>

namespace casa { //# NAMESPACE CASA - BEGIN

/////////////////////////////////////////////
/// MSTransformDataHandler implementation ///
/////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
MSTransformDataHandler::MSTransformDataHandler(String& theMS, Table::TableOption option, Bool realmodelcol) :
		  ms_p(MeasurementSet(theMS, option)),
		  mssel_p(ms_p),
		  msc_p(NULL),
		  mscIn_p(NULL),
		  keepShape_p(True),
		  antennaSel_p(False),
		  timeBin_p(-1.0),
		  scanString_p(""),
		  intentString_p(""),
		  obsString_p(""),
		  uvrangeString_p(""),
		  taqlString_p(""),
		  timeRange_p(""),
		  arrayExpr_p(""),
		  combine_p(""),
		  fitorder_p(-1),
		  fitspw_p("*"),
		  fitoutspw_p("*"),
		  realmodelcol_p(realmodelcol)
{
	// CAS-5348 (jagonzal): Check if model parameters are defined.
	if ((realmodelcol_p) and (not ms_p.source().isColumn(MSSource::SOURCE_MODEL)))
	{
		LogIO os(LogOrigin("MSTransformDataHandler", __FUNCTION__));
		os << LogIO::WARN 	<< "Requested to make virtual MODEL_DATA column real but "
							<< "SOURCE_MODEL column is not present in SOURCE sub-table"
							<< LogIO::POST;
		realmodelcol_p = False;
	}

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
MSTransformDataHandler::MSTransformDataHandler(MeasurementSet& ms, Bool realmodelcol) :
		   ms_p(ms),
		   mssel_p(ms_p),
		   msc_p(NULL),
		   mscIn_p(NULL),
		   keepShape_p(True),
		   antennaSel_p(False),
		   timeBin_p(-1.0),
		   scanString_p(""),
		   intentString_p(""),
		   obsString_p(""),
		   uvrangeString_p(""),
		   taqlString_p(""),
		   timeRange_p(""),
		   arrayExpr_p(""),
		   combine_p(""),
		   fitorder_p(-1),
		   fitspw_p("*"),
		   fitoutspw_p("*"),
		   realmodelcol_p(realmodelcol)
{
	// CAS-5348 (jagonzal): Check if model parameters are defined.
	if ((realmodelcol_p) and (not ms_p.source().isColumn(MSSource::SOURCE_MODEL)))
	{
		LogIO os(LogOrigin("MSTransformDataHandler", __FUNCTION__));
		os << LogIO::WARN 	<< "Requested to make virtual MODEL_DATA column real but "
							<< "SOURCE_MODEL column is not present in SOURCE sub-table"
							<< LogIO::POST;
		realmodelcol_p = False;
	}

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
MSTransformDataHandler::~MSTransformDataHandler()
{
	if(!msOut_p.isNull()) msOut_p.flush();

	if (msc_p) delete msc_p;
	msc_p = NULL;

	if (mscIn_p) delete mscIn_p;
	mscIn_p = NULL;

	msOut_p=MeasurementSet();

	// parseColumnNames unavoidably has a static String and Vector<MS::PredefinedColumns>.
	// Collapse them down to free most of that memory.
	parseColumnNames("None");

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
const Vector<MS::PredefinedColumns>& MSTransformDataHandler::parseColumnNames(String col)
{
	// Memorize both for efficiency and so that the info
	// message at the bottom isn't unnecessarily repeated.
	static String my_colNameStr = "";
	static Vector<MS::PredefinedColumns> my_colNameVect;

	col.upcase();
	if(col == my_colNameStr && col != "")
	{
		return my_colNameVect;
	}
	else if(col == "None")
	{
		my_colNameStr = "";
		my_colNameVect.resize(0);
		return my_colNameVect;
	}

	uInt nNames;

	if(col.contains("ALL"))
	{
		nNames = 3;
		my_colNameVect.resize(nNames);
		my_colNameVect[0] = MS::DATA;
		my_colNameVect[1] = MS::MODEL_DATA;
		my_colNameVect[2] = MS::CORRECTED_DATA;
	}
	else
	{
		nNames = dataColStrToEnums(col, my_colNameVect);
	}

	// Whether or not the MS has the columns is checked by verifyColumns().
	// Unfortunately it cannot be done here because this is a static method.


	/*
	 * jagonzal: Redundant logging message (this info is already provided by MSTransformManager)
	 *
	LogIO os(LogOrigin("MSTransformDataHandler", __FUNCTION__));
	// "NONE" is used by the destructor
	if(col != "NONE")
	{
		os << LogIO::NORMAL << "Using ";
		for(uInt i = 0; i < nNames; ++i)
		{
			os << MS::columnName(my_colNameVect[i]) << " ";
		}

		os << " column" << (my_colNameVect.nelements() > 1 ? "s." : ".") << LogIO::POST;
	}
	*/

	my_colNameStr = col;
	return my_colNameVect;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
const Vector<MS::PredefinedColumns>& MSTransformDataHandler::parseColumnNames(String col,const MeasurementSet& msref, Bool realmodelcol)
{
	// Memorize both for efficiency and so that the info
	// message at the bottom isn't unnecessarily repeated.
	static String my_colNameStr = "";
	static Vector<MS::PredefinedColumns> my_colNameVect;

	// Data columns to pick up if present.
	Vector<MS::PredefinedColumns> wanted;

	col.upcase();

	// This version of parseColumnNames does not reuse results of previous calls
	// but always checks the given columns because it cannot be certain that msref
	// refers to the same MS with every call.

	if (col == "None")
	{
		my_colNameStr = "";
		my_colNameVect.resize(0);
		return my_colNameVect;
	}

	LogIO os(LogOrigin("MSTransformDataHandler", __FUNCTION__));

	// Are we choosy?
	const Bool doAny = col.contains("ALL") || col.contains("ANY");

	uInt nPoss;
	if (doAny)
	{
		nPoss = 5;
		wanted.resize(nPoss);
		wanted[0] = MS::DATA;
		wanted[1] = MS::MODEL_DATA;
		wanted[2] = MS::CORRECTED_DATA;
		wanted[3] = MS::FLOAT_DATA;
		wanted[4] = MS::LAG_DATA;
	}
	// split name string into individual names
	else
	{
		nPoss = dataColStrToEnums(col, wanted);
	}

	uInt nFound = 0;
	my_colNameVect.resize(0);
	for (uInt i = 0; i < nPoss; ++i)
	{
		if (msref.tableDesc().isColumn(MS::columnName(wanted[i])))
		{
			++nFound;
			my_colNameVect.resize(nFound, true);
			my_colNameVect[nFound - 1] = wanted[i];
		}
		// CAS-5348 (jagonzal): Model parameters check is done at construction time
		else if (wanted[i] == MS::MODEL_DATA and realmodelcol)
		{
			++nFound;
			my_colNameVect.resize(nFound, true);
			my_colNameVect[nFound - 1] = wanted[i];
			os << LogIO::NORMAL 	<< "Virtual MODEL_DATA column found will be written to output MS " << LogIO::POST;
		}
		else if (!doAny)
		{
			ostringstream ostr;
			ostr 	<< "Desired column (" << MS::columnName(wanted[i])
					<< ") not found in the input MS (" << msref.tableName()
					<< ").";
			throw(AipsError(ostr.str()));
		}
	}
	if (nFound == 0) throw(AipsError("Did not find and select any data columns."));

	/*
	 * jagonzal: Redundant logging message (this info is already provided by MSTransformManager)
	 *
	os << LogIO::NORMAL << "Using ";

	for (uInt i = 0; i < nFound; ++i)
	{
		os << MS::columnName(my_colNameVect[i]) << " ";
	}

	os << "column" << (nFound > 1 ? "s." : ".") << LogIO::POST;
	*/

	my_colNameStr = col;

	return my_colNameVect;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
uInt MSTransformDataHandler::dataColStrToEnums(const String& col, Vector<MS::PredefinedColumns>& colvec)
{
	LogIO os(LogOrigin("MSTransformDataHandler", __FUNCTION__));
	String tmpNames(col);
	Vector<String> tokens;
	tmpNames.upcase();

	// split name string into individual names
	char * pch;
	Int i = 0;
	pch = strtok((char*) tmpNames.c_str(), " ,");
	while (pch != NULL)
	{
		tokens.resize(i + 1, True);
		tokens[i] = String(pch);
		++i;
		pch = strtok(NULL, " ,");
	}

	uInt nNames = tokens.nelements();

	uInt nFound = 0;
	for (uInt i = 0; i < nNames; ++i)
	{
		colvec.resize(nFound + 1, True);
		colvec[nFound] = MS::UNDEFINED_COLUMN;

		if (	tokens[i] == "OBSERVED"
				|| tokens[i] == "DATA"
				|| tokens[i] == MS::columnName(MS::DATA))
		{
			colvec[nFound++] = MS::DATA;
		}
		else if (	tokens[i] == "FLOAT"
					|| tokens[i] == "FLOAT_DATA"
					|| tokens[i] == MS::columnName(MS::FLOAT_DATA))
		{
			colvec[nFound++] = MS::FLOAT_DATA;
		}
		else if (	tokens[i] == "LAG"
					|| tokens[i] == "LAG_DATA"
					|| tokens[i] == MS::columnName(MS::LAG_DATA))
		{
			colvec[nFound++] = MS::LAG_DATA;
		}
		else if (	tokens[i] == "MODEL"
					|| tokens[i] == "MODEL_DATA"
					|| tokens[i] == MS::columnName(MS::MODEL_DATA))
		{
			colvec[nFound++] = MS::MODEL_DATA;
		}
		else if (	tokens[i] == "CORRECTED"
					|| tokens[i] == "CORRECTED_DATA"
					|| tokens[i] == MS::columnName(MS::CORRECTED_DATA))
		{
			colvec[nFound++] = MS::CORRECTED_DATA;
		}
		// "NONE" is used by the destructor
		else if (tmpNames != "NONE")
		{
			os << LogIO::SEVERE;

			if (nFound == 0)
			{
				colvec[0] = MS::DATA;
				os << "Unrecognized data column " << tokens[i] << "...trying DATA.";
			}
			else
			{
				os << "Skipping unrecognized data column " << tokens[i];
			}

			os << LogIO::POST;
		}
	}
	return nFound;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Bool MSTransformDataHandler::setmsselect(	const String& spw, const String& field,
                        					const String& baseline, const String& scan,
                        					const String& uvrange, const String& taql,
                        					const Vector<Int>& step, const String& subarray,
                        					const String& correlation, const String& intent,
                        					const String& obs)
{
	LogIO os(LogOrigin("MSTransformDataHandler", __FUNCTION__));
	Bool ok;

	String myspwstr(spw == "" ? "*" : spw);
	Record selrec = ms_p.msseltoindex(myspwstr, field);

	ok = selectSource(selrec.asArrayInt("field"));

	// All of the requested selection functions will be tried, even if an
	// earlier one has indicated its failure.  This allows all of the selection
	// strings to be tested, yielding more complete feedback for the user
	// (fewer retries).  This is a matter of taste, though.  If the selections
	// turn out to be slow, this function should return on the first false.

	if (!selectSpw(myspwstr, step))
	{
		os << LogIO::SEVERE << "No channels selected." << LogIO::POST;
		ok = false;
	}

	if (baseline != "")
	{
		Vector<Int> antid(0);
		Vector<String> antstr(1, baseline);
		selectAntenna(antid, antstr);
	}

	scanString_p = scan;
	intentString_p = intent;
	obsString_p = obs;
	uvrangeString_p = uvrange;
	taqlString_p = taql;

	if (subarray != "") selectArray(subarray);

	if (!selectCorrelations(correlation))
	{
		os << LogIO::SEVERE << "No correlations selected." << LogIO::POST;
		ok = false;
	}

	return ok;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Bool MSTransformDataHandler::selectSource(const Vector<Int>& fieldid)
{
	LogIO os(LogOrigin("MSTransformDataHandler", __FUNCTION__));

	Bool cando = true;

	if (fieldid.nelements() < 1)
	{
		fieldid_p = Vector<Int> (1, -1);
	}
	else if (fieldid.nelements() > ms_p.field().nrow())
	{
		os << LogIO::SEVERE << "More fields were requested than are in the input MS." << LogIO::POST;
		cando = false;
	}
	else if (max(fieldid) >= static_cast<Int> (ms_p.field().nrow()))
	{
		// Arriving here is very unlikely since if fieldid came from MSSelection
		// bad fields were presumably already quietly dropped.
		os << LogIO::SEVERE << "At least 1 field was requested that is not in the input MS." << LogIO::POST;
		cando = false;
	}
	else
	{
		fieldid_p = fieldid;
	}

	if (fieldid_p.nelements() == 1 && fieldid_p[0] < 0)
	{
		fieldid_p.resize(ms_p.field().nrow());
		indgen(fieldid_p);
	}

	return cando;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Bool MSTransformDataHandler::selectSpw(const String& spwstr,const Vector<Int>& steps)
{
	LogIO os(LogOrigin("MSTransformDataHandler", __FUNCTION__));

	MSSelection mssel;
	String myspwstr(spwstr == "" ? "*" : spwstr);

	mssel.setSpwExpr(myspwstr);

	widths_p = steps.copy();
	if (widths_p.nelements() < 1)
	{
		widths_p.resize(1);
		widths_p[0] = 1;
	}
	else
	{
		for (uInt k = 0; k < widths_p.nelements(); ++k)
		{
			if (widths_p[k] == 0)
			{
				os << LogIO::WARN << "0 cannot be used for channel width...using 1 instead." << LogIO::POST;
				widths_p[k] = 1;
			}
		}
	}

	// Each row should have spw, start, stop, step
	// A single width is a default, but multiple widths should be used literally.
	Matrix<Int> chansel = mssel.getChanList(&ms_p, 1);

	if (chansel.nrow() > 0)
	{
		// Use myspwstr if it selected anything...
		spw_p = chansel.column(0);
		chanStart_p = chansel.column(1);
		chanEnd_p = chansel.column(2);
		chanStep_p = chansel.column(3);

		uInt nspw = chanEnd_p.nelements();
		nchan_p.resize(nspw);

		// A single width is a default, but multiple widths should be used literally.
		if (widths_p.nelements() > 1 && widths_p.nelements() != spw_p.nelements())
		{
			os 		<< LogIO::SEVERE
					<< "Mismatch between the # of widths specified by width and the # of spws."
					<< LogIO::POST;
			return false;
		}

		// Copy the default width to all spws.
		if (widths_p.nelements() < nspw)
		{
			widths_p.resize(nspw, True);
			for (uInt k = 1; k < nspw; ++k)
			{
				widths_p[k] = widths_p[0];
			}
		}

		for (uInt k = 0; k < nspw; ++k)
		{
			// CAS-2224, triggered by spw='0:2' (as opposed to '0:2~2').
			if (chanStep_p[k] == 0) chanStep_p[k] = 1;

			nchan_p[k] = 1 + (chanEnd_p[k] - chanStart_p[k]) / (chanStep_p[k]* widths_p[k]);
			if (nchan_p[k] < 1) nchan_p[k] = 1;
		}
	}
	else
	{
		// Select everything and rely on widths.
		ROMSSpWindowColumns mySpwTab(ms_p.spectralWindow());
		uInt nspw = mySpwTab.nrow();

		nchan_p = mySpwTab.numChan().getColumn();

		spw_p.resize(nspw);
		indgen(spw_p);

		chanStart_p.resize(nspw);
		chanStep_p.resize(nspw);
		for (uInt k = 0; k < nspw; ++k)
		{
			chanStart_p[k] = 0;
			chanEnd_p[k] = nchan_p[k] - 1;
			chanStep_p[k] = 1;
		}

		if (widths_p.nelements() != spw_p.nelements())
		{
			if (widths_p.nelements() == 1)
			{
				widths_p.resize(spw_p.nelements(), True);
				for (uInt k = 1; k < spw_p.nelements(); ++k)
				{
					widths_p[k] = widths_p[0];
				}

			}
			else
			{
				os 		<< LogIO::SEVERE
						<< "Mismatch between the # of widths specified by width and the # of spws."
						<< LogIO::POST;
				return false;
			}
		}

		for (uInt k = 0; k < nspw; ++k)
		{
			nchan_p[k] = 1 + (nchan_p[k] - 1) / widths_p[k];
		}
	}

	// Check for and filter out selected spws that aren't included in DATA_DESCRIPTION.
	// (See CAS-1673 for an example.)
	std::set<Int> badSelSpwSlots(MSTransformDataHandler::findBadSpws(ms_p, spw_p));
	uInt nbadSelSpwSlots = badSelSpwSlots.size();
	if (nbadSelSpwSlots > 0)
	{
		os << LogIO::WARN << "Selected input spw(s)\n";
		for (std::set<Int>::iterator bbit = badSelSpwSlots.begin(); bbit != badSelSpwSlots.end(); ++bbit)
		{
			os << spw_p[*bbit] << " ";
		}
		os << "\nwere not found in DATA_DESCRIPTION and are being excluded." << LogIO::POST;

		uInt nSelSpw = spw_p.nelements();
		uInt ngoodSelSpwSlots = nSelSpw - nbadSelSpwSlots;
		Vector<Int> spwc(ngoodSelSpwSlots);
		Vector<Int> chanStartc(ngoodSelSpwSlots);
		Vector<Int> chanEndc(ngoodSelSpwSlots);
		Vector<Int> nchanc(ngoodSelSpwSlots);
		Vector<Int> chanStepc(ngoodSelSpwSlots);
		std::set<Int>::iterator bsend = badSelSpwSlots.end();

		uInt j = 0;
		for (uInt k = 0; k < nSelSpw; ++k)
		{
			if (badSelSpwSlots.find(k) == bsend)
			{
				spwc[j] = spw_p[k];
				chanStartc[j] = chanStart_p[k];
				chanEndc[j] = chanEnd_p[k];
				nchanc[j] = nchan_p[k];
				chanStepc[j] = chanStep_p[k];
				++j;
			}
		}
		spw_p.resize(ngoodSelSpwSlots);
		spw_p = spwc;
		chanStart_p.resize(ngoodSelSpwSlots);
		chanStart_p = chanStartc;
		chanEnd_p.resize(ngoodSelSpwSlots);
		chanEnd_p = chanEndc;
		nchan_p.resize(ngoodSelSpwSlots);
		nchan_p = nchanc;
		chanStep_p.resize(ngoodSelSpwSlots);
		chanStep_p = chanStepc;
	}

	mssel.getChanSlices(chanSlices_p, &ms_p, 1);
	return true;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
std::set<Int> MSTransformDataHandler::findBadSpws(MeasurementSet& ms,Vector<Int> spwv)
{
	ROScalarColumn<Int> spws_in_dd(	ms.dataDescription(),
									MSDataDescription::columnName(MSDataDescription::SPECTRAL_WINDOW_ID));
	std::set<Int> uniqSpwsInDD;
	uInt nspwsInDD = spws_in_dd.nrow();

	for (uInt ddrow = 0; ddrow < nspwsInDD; ++ddrow)
	{
		uniqSpwsInDD.insert(spws_in_dd(ddrow));
	}

	std::set<Int> badSelSpwSlots;
	std::set<Int>::iterator ddend = uniqSpwsInDD.end();
	for (uInt k = 0; k < spwv.nelements(); ++k)
	{
		if (uniqSpwsInDD.find(spwv[k]) == ddend)
		{
			badSelSpwSlots.insert(k);
		}
	}

	return badSelSpwSlots;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformDataHandler::selectAntenna(const Vector<Int>& antennaids,const Vector<String>& antennaSel)
{
	antennaSel_p = MSTransformDataHandler::pickAntennas(antennaId_p, antennaSelStr_p, antennaids,antennaSel);
	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Bool MSTransformDataHandler::pickAntennas(	Vector<Int>& selected_antennaids,
			 	 	 	 	 	 	 	 	Vector<String>& selected_antenna_strs,
			 	 	 	 	 	 	 	 	const Vector<Int>& antennaids,
			 	 	 	 	 	 	 	 	const Vector<String>& antennaSel)
{
	Bool didSelect = true;

	if ((antennaids.nelements() == 1) && (antennaids[0] == -1))
	{
		if (antennaSel[0] == "")
		{
			didSelect = false;
		}
		else
		{
			selected_antennaids.resize();
		}
	}
	else
	{
		selected_antennaids = antennaids;
	}

	selected_antenna_strs = antennaSel;

	return didSelect;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformDataHandler::selectArray(const String& subarray)
{
	arrayExpr_p = subarray;
	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Bool MSTransformDataHandler::selectCorrelations(const String& corrstr)
{
	LogIO os(LogOrigin("MSTransformDataHandler", __FUNCTION__));

	MSSelection mssel;
	const Bool areSelecting = corrstr != "" && corrstr != "*";

	if (areSelecting) mssel.setPolnExpr(corrstr);

	corrString_p = corrstr;
	mssel.getCorrSlices(corrSlices_p, &ms_p);
	return MSTransformDataHandler::getCorrMaps(mssel, ms_p, inPolOutCorrToInCorrMap_p, areSelecting);
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Bool MSTransformDataHandler::getCorrMaps(	MSSelection& mssel,
											const MeasurementSet& ms,
											Vector<Vector<Int> >& outToIn,
											const Bool areSelecting)
{

	// ?? This always returns true!!!?!!
	Bool cando = true;

	// The total number of polids
	uInt npol = ms.polarization().nrow();

	// Nominally empty selection for all polids
	outToIn.resize(npol);
	outToIn.set(Vector<Int> ());
	if (areSelecting)
	{
		// Get the corr indices as an ordered map
		OrderedMap<Int, Vector<Vector<Int> > > corrmap(mssel.getCorrMap(&ms));

		// Iterate over the ordered map to fill the vector maps
		ConstMapIter<Int, Vector<Vector<Int> > > mi(corrmap);
		for (mi.toStart(); !mi.atEnd(); ++mi)
		{
			Int pol = mi.getKey();
			outToIn[pol] = mi.getVal()[0];
		}
	}
	else
	{ 	// Make outToIn an identity map.
		ROScalarColumn<Int> numCorr(ms.polarization(),MSPolarization::columnName(MSPolarization::NUM_CORR));

		for (uInt polid = 0; polid < npol; ++polid)
		{
			uInt ncorr = numCorr(polid);
			outToIn[polid].resize(ncorr);
			for (uInt cid = 0; cid < ncorr; ++cid)
			{
				outToIn[polid][cid] = cid;
			}
		}
	}

	return cando;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformDataHandler::selectTime(Double timeBin, String timerng)
{
	timeBin_p   = timeBin;
	timeRange_p = timerng;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Bool MSTransformDataHandler::makeMSBasicStructure(	String& msname,
													String& colname,
													const Vector<Int>& tileShape,
													const String& combine,
													Table::TableOption)
{
	LogIO os(LogOrigin("MSTransformDataHandler", __FUNCTION__));

	if ((spw_p.nelements() > 0) && (max(spw_p) >= Int(ms_p.spectralWindow().nrow())))
	{
		os << LogIO::SEVERE << "SpectralWindow selection contains elements that do not exist in this MS" << LogIO::POST;
		ms_p = MeasurementSet();
		return False;
	}

	// Watch out!  This throws an AipsError if ms_p doesn't have the requested columns.
	const Vector<MS::PredefinedColumns> colNamesTok = parseColumnNames(colname,ms_p,realmodelcol_p);

	if (!makeSelection())
	{
		ms_p = MeasurementSet();
		return False;
	}

	mscIn_p = new ROMSColumns(mssel_p);

	// Note again the parseColumnNames() a few lines back that stops setupMS()
	// from being called if the MS doesn't have the requested columns.
	MeasurementSet* outpointer = 0;

	if (tileShape.nelements() == 3)
	{
		outpointer = setupMS(msname, nchan_p[0], ncorr_p[0], colNamesTok,tileShape);
	}

	// the following calls MSTileLayout...  disabled for now because it
	// forces tiles to be the full spw bandwidth in width (gmoellen, 2010/11/07)
	else if ((tileShape.nelements() == 1) && (tileShape[0] == 0 || tileShape[0]== 1))
	{
		outpointer = setupMS(	msname,
								nchan_p[0],
								ncorr_p[0],
								mscIn_p->observation().telescopeName()(0),
								colNamesTok,
								tileShape[0]);
	}
	else {
		// Sweep all other cases of bad tileshape to a default one.
		// (this probably never happens)
		outpointer = setupMS(	msname,
								nchan_p[0],
								ncorr_p[0],
								mscIn_p->observation().telescopeName()(0),
								colNamesTok,
								0);
	}

	combine_p = combine;

	msOut_p = *outpointer;

	if (!fillSubTables(colNamesTok))
	{
		delete outpointer;
		os << LogIO::WARN << msname << " left unfinished." << LogIO::POST;
		ms_p = MeasurementSet();
		return False;
	}

	//Detaching the selected part
	ms_p = MeasurementSet();


	// If all columns are in the new MS, set the CHANNEL_SELECTION
	// keyword for the MODEL_DATA column.  This is apparently used
	// in at least imager to decide if MODEL_DATA and CORRECTED_DATA
	// columns should be initialized or not.
	if (isAllColumns(colNamesTok))
	{
		MSSpWindowColumns msSpW(msOut_p.spectralWindow());
		Int nSpw = msOut_p.spectralWindow().nrow();
		if (nSpw == 0) nSpw = 1;
		Matrix<Int> selection(2, nSpw);
		selection.row(0) = 0; //start
		selection.row(1) = msSpW.numChan().getColumn();
		ArrayColumn<Complex> mcd(msOut_p, MS::columnName(MS::MODEL_DATA));
		mcd.rwKeywordSet().define("CHANNEL_SELECTION", selection);
	}

	delete outpointer;
	return True;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Bool MSTransformDataHandler::isAllColumns(const Vector<MS::PredefinedColumns>& colNames)
{
	Bool dCol = False, mCol = False, cCol = False;
	for (uInt i = 0; i < colNames.nelements(); i++)
	{
		if (colNames[i] == MS::DATA) dCol = True;
		else if (colNames[i] == MS::MODEL_DATA) mCol = True;
		else if (colNames[i] == MS::CORRECTED_DATA) cCol = True;
		// else turn off all?
	}

	return (dCol && mCol && cCol);
}

// -----------------------------------------------------------------------
// Modified version of makeSelection that uses the new getter methods
// MSS::getSPWDDIDList() and MSS::getDDIDList()
// -----------------------------------------------------------------------
Bool MSTransformDataHandler::makeSelection()
{

	LogIO os(LogOrigin("MSTransformDataHandler", __FUNCTION__));

	// VisSet/MSIter will check if the SORTED exists
	// jagonzal (CAS-5327): Commenting this out, since this implies all sorts of memory leaks
	// Block<Int> sort;
	// ROVisibilityIterator(ms_p, sort);

	const MeasurementSet *elms;
	elms = &ms_p;
	MeasurementSet sorted;
	if (ms_p.keywordSet().isDefined("SORTED_TABLE"))
	{
		sorted = ms_p.keywordSet().asTable("SORTED_TABLE");

		//If ms is not writable and sort is a subselection...use original ms
		if (ms_p.nrow() == sorted.nrow()) elms = &sorted;
	}

	MSSelection thisSelection;
	if (fieldid_p.nelements() > 0)
	{
		thisSelection.setFieldExpr(MSSelection::indexExprStr(fieldid_p));
	}

	if (spw_p.nelements() > 0)
	{
		thisSelection.setSpwExpr(MSSelection::indexExprStr(spw_p));
	}

	if (antennaSel_p)
	{
		if (antennaId_p.nelements() > 0)
		{
			thisSelection.setAntennaExpr(MSSelection::indexExprStr(antennaId_p));
		}
		if (antennaSelStr_p[0] != "")
		{
			thisSelection.setAntennaExpr(MSSelection::nameExprStr(antennaSelStr_p));
		}

	}

	if (timeRange_p != "")
	{
		thisSelection.setTimeExpr(timeRange_p);
	}


	thisSelection.setUvDistExpr(uvrangeString_p);
	thisSelection.setScanExpr(scanString_p);
	thisSelection.setStateExpr(intentString_p);
	thisSelection.setObservationExpr(obsString_p);

	if (arrayExpr_p != "")
	{
		thisSelection.setArrayExpr(arrayExpr_p);
	}

	if (corrString_p != "")
	{
		thisSelection.setPolnExpr(corrString_p);
	}

	thisSelection.setTaQLExpr(taqlString_p);

	TableExprNode exprNode = thisSelection.toTableExprNode(elms);
	selTimeRanges_p = thisSelection.getTimeList();
	selObsId_p = thisSelection.getObservationList();

	// Get the list of DDI for the selected spws
	spw2ddid_p = thisSelection.getSPWDDIDList(elms);

	const MSDataDescription ddtable = elms->dataDescription();
	ROScalarColumn<Int> polId(ddtable,MSDataDescription::columnName(MSDataDescription::POLARIZATION_ID));
	const MSPolarization poltable = elms->polarization();
	ROArrayColumn<Int> pols(poltable,MSPolarization::columnName(MSPolarization::CORR_TYPE));

	// Get the list of DDI for the selected polarizations
	Vector<Int> polDDIList = thisSelection.getDDIDList(elms);

	// When polDDIList is empty, do not do an intersection
	Bool doIntersection = true;

	if (polDDIList.size() == 0){
		doIntersection = false;
	}

	// intersection between selected DDI from spw selection and
	// selected DDI from polarization selection
	if (doIntersection) {
		Vector<Int> intersectedDDI = set_intersection(spw2ddid_p, polDDIList);
		uInt nddids = intersectedDDI.size();
		if (nddids > 0){
			spw2ddid_p.resize(nddids);
			for (uInt ii = 0; ii < nddids; ++ii){
				spw2ddid_p[ii] = intersectedDDI[ii];
			}
		}
		else {
			os 	<< LogIO::SEVERE << "None of the selected correlations are in spectral window "
				<< LogIO::POST;
		}
	}


	// This is actually the number of selected DDI
	uInt nDDIs = spw2ddid_p.size();

	inNumCorr_p.resize(nDDIs);
	ncorr_p.resize(nDDIs);

	// Map the correlations from input selected DDI to output
	for (uInt k = 0; k < nDDIs; ++k)
	{
		Int ddid = spw2ddid_p[k];

		// Number of input correlations for each DDI
		// It reads the nelements of the CORR_TYPE column cell
		inNumCorr_p[k] = pols(polId(ddid)).nelements();

		// Corresponding number of output correlations for each DDI
		ncorr_p[k] = inPolOutCorrToInCorrMap_p[polId(ddid)].nelements();
		if (ncorr_p[k] == 0)
		{
			os 		<< LogIO::SEVERE
					<< "None of the selected correlations are in spectral window "
					<< spw_p[k] << LogIO::POST;
			return false;
		}
	}


	// Now remake the selected ms
	if (!(exprNode.isNull()))
	{
		mssel_p = MeasurementSet((*elms)(exprNode));
	}
	else
	{
		// Null take all the ms ...setdata() blank means that
		mssel_p = MeasurementSet((*elms));
	}

	if (mssel_p.nrow() == 0) return False;

	// Setup antNewIndex_p now that mssel_p is ready.
	if (antennaSel_p)
	{
		// Watch out! getAntenna*List() and getBaselineList() return negative numbers for negated antennas!
		ROScalarColumn<Int> ant1c(mssel_p, MS::columnName(MS::ANTENNA1));
		ROScalarColumn<Int> ant2c(mssel_p, MS::columnName(MS::ANTENNA2));
		Vector<Int> selAnts(ant1c.getColumn());
		uInt nAnts = selAnts.nelements();

		selAnts.resize(2 * nAnts, True);
		selAnts(Slice(nAnts, nAnts)) = ant2c.getColumn();
		nAnts = GenSort<Int>::sort(selAnts, Sort::Ascending,Sort::NoDuplicates);
		selAnts.resize(nAnts, True);
		Int maxAnt = max(selAnts);

		if (maxAnt < 0)
		{
			os 	<< LogIO::SEVERE
				<< "The maximum selected antenna number, " << maxAnt
				<< ", seems to be < 0."
				<< LogIO::POST;
			return False;
		}

		antNewIndex_p.resize(maxAnt + 1);
		//So if you see -1 in the main, feed, or pointing tables, fix it
		antNewIndex_p.set(-1);

		Bool trivial = true;
		for (uInt k = 0; k < nAnts; ++k)
		{
			trivial &= (selAnts[k] == static_cast<Int> (k));
			antNewIndex_p[selAnts[k]] = k;
		}
		// It is possible to exclude baselines without excluding any antennas.
		antennaSel_p = !trivial;
	}
	// This still gets tripped up by VLA:OUT.
	else
	{
		// Make a default antNewIndex_p.
		antNewIndex_p.resize(mssel_p.antenna().nrow());
		indgen(antNewIndex_p);
	}

	if (mssel_p.nrow() < ms_p.nrow())
	{
		os 		<< LogIO::NORMAL
				<< mssel_p.nrow() << " out of " << ms_p.nrow()
				<< " rows are going to be considered due to the selection criteria."
				<< LogIO::POST;
	}

	return True;
}


// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
MeasurementSet* MSTransformDataHandler::setupMS(	const String& MSFileName, const Int nchan,
                                					const Int nCorr, const String& telescop,
                                					const Vector<MS::PredefinedColumns>& colNames,
                                					const Int obstype,const Bool compress,
                                					const asdmStManUseAlternatives asdmStManUse,
                                					Table::TableOption option)
 {
	//Choose an appropriate tileshape
	IPosition dataShape(2, nCorr, nchan);
	IPosition tileShape = MSTileLayout::tileShape(dataShape, obstype, telescop);
	return setupMS(MSFileName, nchan, nCorr, colNames, tileShape.asVector(),compress, asdmStManUse,option);
 }

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
 MeasurementSet* MSTransformDataHandler::setupMS(	const String& MSFileName, const Int nchan,
                                					const Int nCorr,
                                					const Vector<MS::PredefinedColumns>& colNamesTok,
                                					const Vector<Int>& tshape, const Bool compress,
                                					const asdmStManUseAlternatives asdmStManUse,
                                					Table::TableOption option)
 {
	if (tshape.nelements() != 3) throw(AipsError("TileShape has to have 3 elements "));

	// This is more to shush a compiler warning than to warn users.
	LogIO os(LogOrigin("MSTransformDataHandler", __FUNCTION__));
	if (tshape[0] != nCorr)
		os << LogIO::DEBUG1 << "Warning: using " << tshape[0]
				<< " from the tileshape instead of " << nCorr
				<< " for the number of correlations." << LogIO::POST;
	if (tshape[1] != nchan)
		os << LogIO::DEBUG1 << "Warning: using " << tshape[1]
				<< " from the tileshape instead of " << nchan
				<< " for the number of channels." << LogIO::POST;

	// Choose an appropriate tileshape //////////////////

	IPosition tileShape(tshape);

	// Make the MS table
	TableDesc td = MS::requiredTableDesc();
	Vector<String> tiledDataNames;

	// Even though we know the data is going to be the same shape throughout I'll
	// still create a column that has a variable shape as this will permit MS's
	// with other shapes to be appended.
	uInt ncols = colNamesTok.nelements();
	const Bool mustWriteOnlyToData = mustConvertToData(ncols, colNamesTok);

	if (mustWriteOnlyToData)
	{
		MS::addColumnToDesc(td, MS::DATA, 2);
		if (asdmStManUse == DONT)
		{
			if (compress) MS::addColumnCompression(td, MS::DATA, true);
			String hcolName = String("Tiled") + String("DATA");
			td.defineHypercolumn(hcolName, 3, stringToVector("DATA"));
			tiledDataNames.resize(1);
			tiledDataNames[0] = hcolName;
		}
	}
	else
	{
		tiledDataNames.resize(ncols);
		for (uInt i = 0; i < ncols; ++i)
		{
			// Unfortunately MS::PredefinedColumns aren't ordered so that
			// I can just check if colNamesTok[i] is in the "data range".
			if (	colNamesTok[i] == MS::DATA
					|| colNamesTok[i] == MS::MODEL_DATA
					|| colNamesTok[i] == MS::CORRECTED_DATA
					|| colNamesTok[i] == MS::FLOAT_DATA
					|| colNamesTok[i] == MS::LAG_DATA)
			{
				if (asdmStManUse == DONT || colNamesTok[i] != MS::DATA)
				{
					MS::addColumnToDesc(td, colNamesTok[i], 2);
					if (compress) MS::addColumnCompression(td, colNamesTok[i], true);
				}
			}
			else
			{
				throw(AipsError( MS::columnName(colNamesTok[i]) + " is not a recognized data column "));
			}
			if (asdmStManUse == DONT || colNamesTok[i] != MS::DATA)
			{
				String hcolName = String("Tiled") + MS::columnName(colNamesTok[i]);
				td.defineHypercolumn(hcolName, 3,stringToVector(MS::columnName(colNamesTok[i])));
				tiledDataNames[i] = hcolName;
			}
		}
	}

	//other cols for compression
	if (compress && asdmStManUse != USE_FOR_DATA_WEIGHT_SIGMA_FLAG)
	{
		MS::addColumnCompression(td, MS::WEIGHT, true);
		MS::addColumnCompression(td, MS::SIGMA, true);
	}

	// Add this optional column because random group fits has a weight per visibility
	MS::addColumnToDesc(td, MS::WEIGHT_SPECTRUM, 2);

	td.defineHypercolumn("TiledFlagCategory", 4,stringToVector(MS::columnName(MS::FLAG_CATEGORY)));
	td.defineHypercolumn("TiledWgtSpectrum", 3,stringToVector(MS::columnName(MS::WEIGHT_SPECTRUM)));
	td.defineHypercolumn("TiledUVW", 2, stringToVector(MS::columnName(MS::UVW)));

	if (asdmStManUse != USE_FOR_DATA_WEIGHT_SIGMA_FLAG)
	{
		td.defineHypercolumn("TiledFlag", 3,stringToVector(MS::columnName(MS::FLAG)));
		td.defineHypercolumn("TiledWgt", 2,stringToVector(MS::columnName(MS::WEIGHT)));
		td.defineHypercolumn("TiledSigma", 2,stringToVector(MS::columnName(MS::SIGMA)));
	}

	SetupNewTable newtab(MSFileName, td, option);

	uInt cache_val = 32768;

	// Set the default Storage Manager to be the Incr one
	IncrementalStMan incrStMan("ISMData", cache_val);
	newtab.bindAll(incrStMan, True);

	//Override the binding for specific columns
	IncrementalStMan incrStMan0("Array_ID", cache_val);
	newtab.bindColumn(MS::columnName(MS::ARRAY_ID), incrStMan0);
	IncrementalStMan incrStMan1("EXPOSURE", cache_val);
	newtab.bindColumn(MS::columnName(MS::EXPOSURE), incrStMan1);
	IncrementalStMan incrStMan2("FEED1", cache_val);
	newtab.bindColumn(MS::columnName(MS::FEED1), incrStMan2);
	IncrementalStMan incrStMan3("FEED2", cache_val);
	newtab.bindColumn(MS::columnName(MS::FEED2), incrStMan3);
	IncrementalStMan incrStMan4("FIELD_ID", cache_val);
	newtab.bindColumn(MS::columnName(MS::FIELD_ID), incrStMan4);
	IncrementalStMan incrStMan5("FLAG_ROW", cache_val / 4);
	newtab.bindColumn(MS::columnName(MS::FLAG_ROW), incrStMan5);
	IncrementalStMan incrStMan6("INTERVAL", cache_val);
	newtab.bindColumn(MS::columnName(MS::INTERVAL), incrStMan6);
	IncrementalStMan incrStMan7("OBSERVATION_ID", cache_val);
	newtab.bindColumn(MS::columnName(MS::OBSERVATION_ID), incrStMan7);
	IncrementalStMan incrStMan8("PROCESSOR_ID", cache_val);
	newtab.bindColumn(MS::columnName(MS::PROCESSOR_ID), incrStMan8);
	IncrementalStMan incrStMan9("SCAN_NUMBER", cache_val);
	newtab.bindColumn(MS::columnName(MS::SCAN_NUMBER), incrStMan9);
	IncrementalStMan incrStMan10("STATE_ID", cache_val);
	newtab.bindColumn(MS::columnName(MS::STATE_ID), incrStMan10);
	IncrementalStMan incrStMan11("TIME", cache_val);
	newtab.bindColumn(MS::columnName(MS::TIME), incrStMan11);
	IncrementalStMan incrStMan12("TIME_CENTROID", cache_val);
	newtab.bindColumn(MS::columnName(MS::TIME_CENTROID), incrStMan12);

	// Bind ANTENNA1, ANTENNA2 and DATA_DESC_ID to the standardStMan
	// as they may change sufficiently frequently to make the
	// incremental storage manager inefficient for these columns.
	StandardStMan aipsStMan0("ANTENNA1", cache_val);
	newtab.bindColumn(MS::columnName(MS::ANTENNA1), aipsStMan0);
	StandardStMan aipsStMan1("ANTENNA2", cache_val);
	newtab.bindColumn(MS::columnName(MS::ANTENNA2), aipsStMan1);
	StandardStMan aipsStMan2("DATA_DESC_ID", cache_val);
	newtab.bindColumn(MS::columnName(MS::DATA_DESC_ID), aipsStMan2);

	TiledShapeStMan tiledStMan1f("TiledFlag", tileShape);
	TiledShapeStMan tiledStMan1fc("TiledFlagCategory",IPosition(4, tileShape(0), tileShape(1), 1, tileShape(2)));
	TiledShapeStMan tiledStMan2("TiledWgtSpectrum", tileShape);
	TiledColumnStMan tiledStMan3("TiledUVW",IPosition(2, 3, (tileShape(0) * tileShape(1) * tileShape(2)) / 3));
	TiledShapeStMan tiledStMan4("TiledWgt",IPosition(2, tileShape(0), tileShape(1) * tileShape(2)));
	TiledShapeStMan tiledStMan5("TiledSigma",IPosition(2, tileShape(0), tileShape(1) * tileShape(2)));

	// Bind the DATA, FLAG & WEIGHT_SPECTRUM columns to the tiled stman or asdmStMan
	AsdmStMan sm;

	if (mustWriteOnlyToData)
	{
		if (asdmStManUse == DONT)
		{
			TiledShapeStMan tiledStMan1Data("TiledDATA", tileShape);
			newtab.bindColumn(MS::columnName(MS::DATA), tiledStMan1Data);
		}
		else
		{
			newtab.bindColumn(MS::columnName(MS::DATA), sm);
		}
	}
	else
	{
		for (uInt i = 0; i < ncols; ++i)
		{
			TiledShapeStMan tiledStMan1Data(tiledDataNames[i], tileShape);
			newtab.bindColumn(MS::columnName(colNamesTok[i]), tiledStMan1Data);
		}
		if (asdmStManUse != DONT)
		{
			newtab.bindColumn(MS::columnName(MS::DATA), sm);
		}
	}
	newtab.bindColumn(MS::columnName(MS::FLAG_CATEGORY), tiledStMan1fc);
	newtab.bindColumn(MS::columnName(MS::WEIGHT_SPECTRUM), tiledStMan2);

	newtab.bindColumn(MS::columnName(MS::UVW), tiledStMan3);
	if (asdmStManUse == USE_FOR_DATA_WEIGHT_SIGMA_FLAG)
	{
		newtab.bindColumn(MS::columnName(MS::FLAG), sm);
		newtab.bindColumn(MS::columnName(MS::WEIGHT), sm);
		newtab.bindColumn(MS::columnName(MS::SIGMA), sm);
	}
	else
	{
		newtab.bindColumn(MS::columnName(MS::FLAG), tiledStMan1f);
		newtab.bindColumn(MS::columnName(MS::WEIGHT), tiledStMan4);
		newtab.bindColumn(MS::columnName(MS::SIGMA), tiledStMan5);
	}

	// Avoid lock overheads by locking the table permanently
	TableLock lock(TableLock::AutoLocking);
	MeasurementSet *ms = new MeasurementSet(newtab, lock);

	// Set up the sub-tables for the UVFITS MS (we make new tables with 0 rows)
	// Table::TableOption option = Table::New;
	createSubtables(*ms, option);

	// Set the TableInfo
	TableInfo& info(ms->tableInfo());
	info.setType(TableInfo::type(TableInfo::MEASUREMENTSET));
	info.setSubType(String("UVFITS"));
	info.readmeAddLine("This is a measurement set Table holding astronomical observations");

	return ms;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformDataHandler::createSubtables(MeasurementSet& ms, Table::TableOption option)
{
	SetupNewTable antennaSetup(ms.antennaTableName(),MSAntenna::requiredTableDesc(), option);
	ms.rwKeywordSet().defineTable(MS::keywordName(MS::ANTENNA),Table(antennaSetup));
	SetupNewTable dataDescSetup(ms.dataDescriptionTableName(),MSDataDescription::requiredTableDesc(), option);
	ms.rwKeywordSet().defineTable(MS::keywordName(MS::DATA_DESCRIPTION),Table(dataDescSetup));
	SetupNewTable feedSetup(ms.feedTableName(), MSFeed::requiredTableDesc(),option);
	ms.rwKeywordSet().defineTable(MS::keywordName(MS::FEED), Table(feedSetup));
	SetupNewTable flagCmdSetup(ms.flagCmdTableName(),MSFlagCmd::requiredTableDesc(), option);
	ms.rwKeywordSet().defineTable(MS::keywordName(MS::FLAG_CMD),Table(flagCmdSetup));
	SetupNewTable fieldSetup(ms.fieldTableName(), MSField::requiredTableDesc(),option);
	ms.rwKeywordSet().defineTable(MS::keywordName(MS::FIELD), Table(fieldSetup));
	SetupNewTable historySetup(ms.historyTableName(),MSHistory::requiredTableDesc(), option);
	ms.rwKeywordSet().defineTable(MS::keywordName(MS::HISTORY),Table(historySetup));
	SetupNewTable observationSetup(ms.observationTableName(),MSObservation::requiredTableDesc(), option);
	ms.rwKeywordSet().defineTable(MS::keywordName(MS::OBSERVATION),Table(observationSetup));
	SetupNewTable polarizationSetup(ms.polarizationTableName(),MSPolarization::requiredTableDesc(), option);
	ms.rwKeywordSet().defineTable(MS::keywordName(MS::POLARIZATION),Table(polarizationSetup));
	SetupNewTable processorSetup(ms.processorTableName(),MSProcessor::requiredTableDesc(), option);
	ms.rwKeywordSet().defineTable(MS::keywordName(MS::PROCESSOR),Table(processorSetup));
	SetupNewTable spectralWindowSetup(ms.spectralWindowTableName(),MSSpectralWindow::requiredTableDesc(), option);
	ms.rwKeywordSet().defineTable(MS::keywordName(MS::SPECTRAL_WINDOW),Table(spectralWindowSetup));
	SetupNewTable stateSetup(ms.stateTableName(), MSState::requiredTableDesc(),option);
	ms.rwKeywordSet().defineTable(MS::keywordName(MS::STATE), Table(stateSetup));

	// Add the optional Source sub table to allow for specification of the rest frequency
	SetupNewTable sourceSetup(ms.sourceTableName(),MSSource::requiredTableDesc(), option);
	ms.rwKeywordSet().defineTable(MS::keywordName(MS::SOURCE),Table(sourceSetup, 0));

	// Update the references to the sub-table keywords
	ms.initRefs();

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Bool MSTransformDataHandler::fillSubTables(const Vector<MS::PredefinedColumns>&)
{
	LogIO os(LogOrigin("MSTransformDataHandler", __FUNCTION__));
	Bool success = true;

	// Copy the sub-tables before doing anything with the main table.
	// Otherwise MSColumns won't work.

	// fill or update
	Timer timer;

	timer.mark();
	success &= copyPointing();
	os << LogIO::DEBUG1 << "copyPointing took " << timer.real() << "s." << LogIO::POST;

	// Optional columns should be set up before msc_p.
	addOptionalColumns(mssel_p.spectralWindow(), msOut_p.spectralWindow(), true);

	// Force the Measures frames for all the time type columns to
	// have the same reference as the TIME column of the main table.
	// Disable the empty table check (with false) because some of the
	// sub-tables (like POINTING) might already have been written.
	// However, empty tables are still empty after setting up the reference codes here.
	msc_p = new MSColumns(msOut_p);
	msc_p->setEpochRef(MEpoch::castType(mscIn_p->timeMeas().getMeasRef().getType()), False);

	// UVW is the only other Measures column in the main table.
	msc_p->uvwMeas().setDescRefCode(Muvw::castType(mscIn_p->uvwMeas().getMeasRef().getType()));

	if (!mscIn_p->flagCategory().isNull() && mscIn_p->flagCategory().isDefined(0))
	{
		msc_p->setFlagCategories(mscIn_p->flagCategories());
	}


	timer.mark();
	if (!fillDDTables()) return False;
	os << LogIO::DEBUG1 << "fillDDTables took " << timer.real() << "s." << LogIO::POST;

	// SourceIDs need to be re-mapped around here
	// (It cannot not be done in selectSource() because mssel_p is not set up yet)
	timer.mark();
	relabelSources();
	os << LogIO::DEBUG1 << "relabelSources took " << timer.real() << "s." << LogIO::POST;

	success &= fillFieldTable();
	success &= copySource();

	success &= copyAntenna();
	// Feed table writing has to be after antenna
	if (!copyFeed()) return false;

	success &= copyFlag_Cmd();
	success &= copyHistory();
	success &= copyObservation();
	success &= copyProcessor();
	success &= copyState();

	timer.mark();
	success &= copySyscal();
	os << LogIO::DEBUG1 << "copySyscal took " << timer.real() << "s." << LogIO::POST;

	timer.mark();
	success &= copyWeather();
	os << LogIO::DEBUG1 << "copyWeather took " << timer.real() << "s." << LogIO::POST;

	timer.mark();
	success &= filterOptSubtable("CALDEVICE");
	os << LogIO::DEBUG1 << "CALDEVICE took " << timer.real() << "s." << LogIO::POST;

	timer.mark();
	success &= filterOptSubtable("SYSPOWER");
	os << LogIO::DEBUG1 << "SYSPOWER took " << timer.real() << "s." << LogIO::POST;

	// Run this after running the other copy*()s.
	// Maybe there should be an option to *not* run it.
	success &= copyGenericSubtables();

	return success;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Bool MSTransformDataHandler::fillFieldTable()
{
	LogIO os(LogOrigin("MSTransformDataHandler", __FUNCTION__));

	uInt nAddedCols = addOptionalColumns(mssel_p.field(), msOut_p.field(), true);

	MSFieldColumns msField(msOut_p.field());

	const ROMSFieldColumns& fieldIn = mscIn_p->field();
	ROScalarColumn<String> code(fieldIn.code());
	ROArrayColumn<Double> delayDir(fieldIn.delayDir());
	ROScalarColumn<Bool> flagRow(fieldIn.flagRow());
	ROScalarColumn<String> name(fieldIn.name());
	ROScalarColumn<Int> numPoly(fieldIn.numPoly());
	ROArrayColumn<Double> phaseDir(fieldIn.phaseDir());
	ROArrayColumn<Double> refDir(fieldIn.referenceDir());
	ROScalarColumn<Int> sourceId(fieldIn.sourceId());
	ROScalarColumn<Double> time(fieldIn.time());

	String refstr;
	String nameVarRefColDelayDir, nameVarRefColPhaseDir, nameVarRefColRefDir;

	// Need to correctly define the direction measures.

	// DelayDir
	if (delayDir.keywordSet().asRecord("MEASINFO").isDefined("Ref"))
	{
		delayDir.keywordSet().asRecord("MEASINFO").get("Ref", refstr);
		msField.delayDir().rwKeywordSet().asrwRecord("MEASINFO").define("Ref",refstr);
	}

	// it's a variable ref. column
	if (delayDir.keywordSet().asRecord("MEASINFO").isDefined("VarRefCol"))
	{
		delayDir.keywordSet().asRecord("MEASINFO").get("VarRefCol", refstr);
		msField.delayDir().rwKeywordSet().asrwRecord("MEASINFO").define("VarRefCol", refstr);
		nameVarRefColDelayDir = refstr;

		Vector<String> refTypeV;
		delayDir.keywordSet().asRecord("MEASINFO").get("TabRefTypes", refTypeV);
		msField.delayDir().rwKeywordSet().asrwRecord("MEASINFO").define("TabRefTypes", refTypeV);

		Vector<uInt> refCodeV;
		delayDir.keywordSet().asRecord("MEASINFO").get("TabRefCodes", refCodeV);
		msField.delayDir().rwKeywordSet().asrwRecord("MEASINFO").define("TabRefCodes", refCodeV);
		Int refid = msField.delayDir().keywordSet().asRecord("MEASINFO").fieldNumber("Ref");

		// Erase the redundant Ref keyword
		if (refid >= 0)
		{
			msField.delayDir().rwKeywordSet().asrwRecord("MEASINFO").removeField(RecordFieldId(refid));
		}
	}

	// PhaseDir
	if (phaseDir.keywordSet().asRecord("MEASINFO").isDefined("Ref"))
	{
		phaseDir.keywordSet().asRecord("MEASINFO").get("Ref", refstr);
		msField.phaseDir().rwKeywordSet().asrwRecord("MEASINFO").define("Ref",refstr);
	}

	// It's a variable ref. column
	if (phaseDir.keywordSet().asRecord("MEASINFO").isDefined("VarRefCol"))
	{
		phaseDir.keywordSet().asRecord("MEASINFO").get("VarRefCol", refstr);
		msField.phaseDir().rwKeywordSet().asrwRecord("MEASINFO").define("VarRefCol", refstr);
		nameVarRefColPhaseDir = refstr;

		Vector<String> refTypeV;
		phaseDir.keywordSet().asRecord("MEASINFO").get("TabRefTypes", refTypeV);
		msField.phaseDir().rwKeywordSet().asrwRecord("MEASINFO").define("TabRefTypes", refTypeV);

		Vector<uInt> refCodeV;
		phaseDir.keywordSet().asRecord("MEASINFO").get("TabRefCodes", refCodeV);
		msField.phaseDir().rwKeywordSet().asrwRecord("MEASINFO").define("TabRefCodes", refCodeV);

		Int refid = msField.phaseDir().keywordSet().asRecord("MEASINFO").fieldNumber("Ref");
		if (refid >= 0)
		{
			msField.phaseDir().rwKeywordSet().asrwRecord("MEASINFO").removeField(RecordFieldId(refid));
		}
	}

	// ReferenceDir
	if (refDir.keywordSet().asRecord("MEASINFO").isDefined("Ref"))
	{
		refDir.keywordSet().asRecord("MEASINFO").get("Ref", refstr);
		msField.referenceDir().rwKeywordSet().asrwRecord("MEASINFO").define(
				"Ref", refstr);
	}

	// It's a variable ref. column
	if (refDir.keywordSet().asRecord("MEASINFO").isDefined("VarRefCol"))
	{
		refDir.keywordSet().asRecord("MEASINFO").get("VarRefCol", refstr);
		msField.referenceDir().rwKeywordSet().asrwRecord("MEASINFO").define("VarRefCol", refstr);
		nameVarRefColRefDir = refstr;

		Vector<String> refTypeV;
		refDir.keywordSet().asRecord("MEASINFO").get("TabRefTypes", refTypeV);
		msField.referenceDir().rwKeywordSet().asrwRecord("MEASINFO").define("TabRefTypes", refTypeV);

		Vector<uInt> refCodeV;
		refDir.keywordSet().asRecord("MEASINFO").get("TabRefCodes", refCodeV);
		msField.referenceDir().rwKeywordSet().asrwRecord("MEASINFO").define("TabRefCodes", refCodeV);

		Int refid = msField.referenceDir().keywordSet().asRecord("MEASINFO").fieldNumber("Ref");
		if (refid >= 0)
		{
			msField.referenceDir().rwKeywordSet().asrwRecord("MEASINFO").removeField(RecordFieldId(refid));
		}
	}

	// ...and the time measure...
	time.keywordSet().asRecord("MEASINFO").get("Ref", refstr);
	msField.time().rwKeywordSet().asrwRecord("MEASINFO").define("Ref", refstr);

	// fieldRelabel_p size: nrow of a input MS, -1 for unselected field ids
	fieldRelabel_p.resize(mscIn_p->field().nrow());
	fieldRelabel_p.set(-1);

	os 	<< LogIO::DEBUG1 << fieldid_p.nelements()
		<< " fields selected out of " << mscIn_p->field().nrow()
		<< LogIO::POST;

	try {

		msOut_p.field().addRow(fieldid_p.nelements());

		for (uInt k = 0; k < fieldid_p.nelements(); ++k)
		{
			fieldRelabel_p[fieldid_p[k]] = k;

			msField.code().put(k, code(fieldid_p[k]));
			msField.delayDir().put(k, delayDir(fieldid_p[k]));
			msField.flagRow().put(k, flagRow(fieldid_p[k]));
			msField.name().put(k, name(fieldid_p[k]));
			msField.numPoly().put(k, numPoly(fieldid_p[k]));
			msField.phaseDir().put(k, phaseDir(fieldid_p[k]));
			msField.referenceDir().put(k, refDir(fieldid_p[k]));
			msField.time().put(k, time(fieldid_p[k]));

			Int inSrcID = sourceId(fieldid_p[k]);
			if (inSrcID < 0)
			{
				msField.sourceId().put(k, -1);
			}
			else
			{
				msField.sourceId().put(k, sourceRelabel_p[inSrcID]);
			}
		}

		if (nAddedCols > 0)
		{
			ROScalarColumn<Int> eID(fieldIn.ephemerisId());

			if (eID.hasContent())
			{
				String destPathName = Path(msOut_p.field().tableName()).absoluteName();

				for (uInt k = 0; k < fieldid_p.nelements(); ++k)
				{

					Int theEphId = eID(fieldid_p[k]);

					// There is an ephemeris attached to this field
					if (theEphId > -1)
					{
						Path ephPath = Path(fieldIn.ephemPath(fieldid_p[k]));

						// Copy the ephemeris table over to the output FIELD table
						if (ephPath.length() > 0)
						{
							Directory origEphemDir(ephPath);
							origEphemDir.copy(destPathName + "/" + ephPath.baseName());

							os 	<< LogIO::NORMAL
								<< "Transferring ephemeris " << ephPath.baseName()
								<< " for output field " << name(fieldid_p[k])
								<< LogIO::POST;
						}
					}

					msField.ephemerisId().put(k, theEphId);
				}
			}

			// need to copy the reference column
			if (!nameVarRefColDelayDir.empty())
			{
				ROScalarColumn<Int> dM(mssel_p.field(), nameVarRefColDelayDir);
				ScalarColumn<Int> cdMDirRef(msOut_p.field(),nameVarRefColDelayDir);
				for (uInt k = 0; k < fieldid_p.nelements(); ++k)
				{
					cdMDirRef.put(k, dM(fieldid_p[k]));
				}
			}

			// need to copy the reference column
			if (!nameVarRefColPhaseDir.empty())
			{
				ROScalarColumn<Int> dM(mssel_p.field(), nameVarRefColPhaseDir);
				ScalarColumn<Int> cdMDirRef(msOut_p.field(),nameVarRefColPhaseDir);
				for (uInt k = 0; k < fieldid_p.nelements(); ++k)
				{
					cdMDirRef.put(k, dM(fieldid_p[k]));
				}
			}

			// need to copy the reference column
			if (!nameVarRefColRefDir.empty())
			{
				ROScalarColumn<Int> dM(mssel_p.field(), nameVarRefColRefDir);
				ScalarColumn<Int> cdMDirRef(msOut_p.field(),nameVarRefColRefDir);
				for (uInt k = 0; k < fieldid_p.nelements(); ++k)
				{
					cdMDirRef.put(k, dM(fieldid_p[k]));
				}
			}
		}

	}
	catch (AipsError x)
	{
		os 	<< LogIO::EXCEPTION << "Error " << x.getMesg() << " setting up the output FIELD table." << LogIO::POST;
	}
	catch (...)
	{
		throw(AipsError("Unknown exception caught and released in fillFieldTable()"));
	}

	return True;
}

// -----------------------------------------------------------------------
//  Modified version of fillDDTables
// -----------------------------------------------------------------------
Bool MSTransformDataHandler::fillDDTables()
{
	LogIO os(LogOrigin("MSTransformDataHandler", __FUNCTION__));

	// Selected input MS SPW Table Columns (read-only)
	ROMSSpWindowColumns inSpWCols(mssel_p.spectralWindow());

	// SPW Table Columns of output MS (read-write)
	MSSpWindowColumns& msSpW(msc_p->spectralWindow());

	// Detect which optional columns of SPECTRAL_WINDOW are present.
	// inSpWCols and msSpW should agree because addOptionalColumns() was done for
	// SPECTRAL_WINDOW in fillAllTables() before making msc_p or calling fillDDTables
	Bool haveSpwAN = inSpWCols.assocNature().hasContent();
	Bool haveSpwASI = inSpWCols.assocSpwId().hasContent();
	Bool haveSpwBN = inSpWCols.bbcNo().hasContent();
	Bool haveSpwBS = inSpWCols.bbcSideband().hasContent();
	Bool haveSpwDI = inSpWCols.dopplerId().hasContent();

	// DATA_DESCRIPTION Table columns of output MS
	MSDataDescColumns& msDD(msc_p->dataDescription());

	// POLARIZATION Table columns of output MS
	MSPolarizationColumns& msPol(msc_p->polarization());

	//DATA_DESCRIPTION table of selected MS
	const MSDataDescription ddtable = mssel_p.dataDescription();

	// POLARIZATION_ID column of DD in selected MS
	ROScalarColumn<Int> polId(ddtable,
			MSDataDescription::columnName(MSDataDescription::POLARIZATION_ID));

	// SPECTRAL_WINDOW_ID of selected SPWS
	ROScalarColumn<Int> spwIds(ddtable,
			MSDataDescription::columnName(MSDataDescription::SPECTRAL_WINDOW_ID));

	// From the selected DDI, get the SPW_IDs for the output MS
	Vector<Int> selectedSpwIds;
	uInt nddid = spw2ddid_p.size();
	selectedSpwIds.resize(nddid);
	for (uInt row=0; row<spw2ddid_p.size(); ++row){
		// These are the selected SPW_IDs based on the DDI
		// They need to be re-indexed later
		selectedSpwIds[row] = spwIds(spw2ddid_p[row]);
	}

	// Re-label the SPW_IDs (get new indices)
	spwRelabel_p.resize(mscIn_p->spectralWindow().nrow());
	spwRelabel_p.set(-1);

	// Make map from input to output spws.
	// spwRelabel_p has to be the SPECTRAL_WINDOW_ID not DDI
	Bool dum;
	Sort sortSpws(spw_p.getStorage(dum), sizeof(Int));
	sortSpws.sortKey((uInt) 0, TpInt);
	Vector<uInt> spwsortindex, spwuniqinds;
	sortSpws.sort(spwsortindex, spw_p.nelements());
	uInt nuniqSpws = sortSpws.unique(spwuniqinds, spwsortindex);
	spw_uniq_p.resize(nuniqSpws);

	for (uInt k = 0; k < nuniqSpws; ++k)
	{
		spw_uniq_p[k] = spw_p[spwuniqinds[k]];
		spwRelabel_p[spw_uniq_p[k]] = k;
	}
	if (nuniqSpws < spw_p.nelements())
	{
		os 	<< LogIO::WARN
			<< "Multiple channel ranges within an spw may not work.  SOME DATA MAY BE OMITTED!"
			<< "\nConsider splitting them individually and optionally combining the output MSes with concat."
			<< "\nEven then, expect problems if exporting to uvfits."
			<< LogIO::POST;
	}

	// Make map from input to output spws.
	// SPW_ID must NOT be unique, as it can repeat sometimes
	Sort sortSpws1(selectedSpwIds.getStorage(dum), sizeof(Int));
	sortSpws1.sortKey((uInt) 0, TpInt);
	Vector<uInt> spwsortindex1;
	sortSpws1.sort(spwsortindex1, selectedSpwIds.nelements());
	uInt nSortedSpws = spwsortindex1.nelements();

	// This table is only used to fill the DD output table at
	// the end of this function
	Vector<Int> newDDtable;
	newDDtable.resize(mscIn_p->dataDescription().nrow());
	newDDtable.set(-1);

	// Re-index the SPW IDs for the output DD table
	uInt spwindex = 0;
	for (uInt k=0; k < nSortedSpws; ++k){
		// k --> row number of output DD table
		if (k == 0){
			// do it only the first time
			newDDtable[k] = spwindex;
			++spwindex;
			continue;
		}

		if (selectedSpwIds[k] == selectedSpwIds[k-1]){
			newDDtable[k] = spwindex - 1;
		}
		else {
			newDDtable[k] = spwindex;
			++spwindex;
		}
	}

	const MSPolarization poltable = mssel_p.polarization();
	ROScalarColumn<Int> numCorr(poltable,MSPolarization::columnName(MSPolarization::NUM_CORR));
	ROArrayColumn<Int> corrType(poltable,MSPolarization::columnName(MSPolarization::CORR_TYPE));
	ROArrayColumn<Int> corrProd(poltable,MSPolarization::columnName(MSPolarization::CORR_PRODUCT));
	ROScalarColumn<Bool> polFlagRow(poltable,MSPolarization::columnName(MSPolarization::FLAG_ROW));

	inNumChan_p.resize(spw_p.nelements());

	// Vector of POLARIZATION_ID column from selected MS
	polID_p = polId.getColumn();
	uInt nPol = polID_p.size();

	// Map from output POLARIZATION_ID to input POLARIZATION_ID.
	Vector<Int> selectedPolId(nPol);
	for (uInt k = 0; k < nPol; ++k)
	{
		selectedPolId[k] = polID_p[k];
	}

	Vector<Int> newPolId(nddid);
	for (uInt k = 0; k < nddid; ++k)
	{
		Bool found = false;

		for (uInt j = 0; j < nPol; ++j)
		{
			// Get the POLARIZATION_ID of the intersected DDI
			// from spw and polarization selections (spw2ddid_p)
			if (selectedPolId[j] == polID_p[spw2ddid_p[k]])
			{
				// These should go to the POLARIZATION_ID column of the output MS
				newPolId[k] = j;
				found = true;
				break;
			}
		}
		if (!found)
		{
			os	<< LogIO::SEVERE
				<< "No polarization ID found for output polarization setup " << k
				<< LogIO::POST;
			return false;
		}
	}


	// Get a unique sorted list of the POLARIZATION_ID
	// of the selected DDI
	Sort sort(selectedPolId.getStorage(dum), sizeof(Int));
	sort.sortKey((uInt) 0, TpInt);
	Vector<uInt> index, uniq;
	sort.sort(index, selectedPolId.nelements());
	nPol = sort.unique(uniq, index);

	// Write to output MS POLARIZATION table
	// This table should not be resized!!!
	corrSlice_p.resize(nPol);
	for (uInt outpid = 0; outpid < nPol; ++outpid)
	{
		uInt inpid = selectedPolId[outpid];
		uInt ncorr = inPolOutCorrToInCorrMap_p[inpid].nelements();
		const Vector<Int> inCT(corrType(inpid));

		// ncorr will be 0 if none of the selected spws have this pid.
		if (ncorr > 0 && ncorr < inCT.nelements())
		{
			keepShape_p = false;

			// Check whether the requested correlations can be accessed by slicing.
			// That means there must be a constant stride.  The most likely (only?)
			// way to violate that is to ask for 3 out of 4 correlations.
			if (ncorr > 2)
			{
				os 	<< LogIO::SEVERE
					<< "Sorry, the requested correlation selection is not unsupported.\n"
					<< "Try selecting fewer or all of the correlations."
					<< LogIO::POST;
				return false;
			}

			size_t increment = 2;
			if (ncorr > 1)
			{
				increment = inPolOutCorrToInCorrMap_p[inpid][1] - inPolOutCorrToInCorrMap_p[inpid][0];
			}
			corrSlice_p[outpid] = Slice(inPolOutCorrToInCorrMap_p[inpid][0],ncorr,increment);
		}
		else
		{
			corrSlice_p[outpid] = Slice(0, ncorr);
		}


		// Add rows to POLARIZATION table of output MS
		msOut_p.polarization().addRow();
		msPol.numCorr().put(outpid, ncorr);
		msPol.flagRow().put(outpid, polFlagRow(inpid));

		Vector<Int> outCT;
		const Matrix<Int> inCP(corrProd(inpid));
		Matrix<Int> outCP;
		outCT.resize(ncorr);
		outCP.resize(2, ncorr);
		for (uInt k = 0; k < ncorr; ++k)
		{
			Int inCorrInd = inPolOutCorrToInCorrMap_p[inpid][k];

			outCT[k] = inCT[inCorrInd];
			for (uInt feedind = 0; feedind < 2; ++feedind)
			{
				outCP(feedind, k) = inCP(feedind, inCorrInd);
			}

		}
		msPol.corrType().put(outpid, outCT);
		msPol.corrProduct().put(outpid, outCP);
	}

	// This sets the number of input channels for each spw. But
	// it considers that a SPW ID contains only one set of channels.
	// I hope this is true!!
	// Write to SPECTRAL_WINDOW table
	for (uInt k = 0; k < spw_p.nelements(); ++k)
	{
		inNumChan_p[k] = inSpWCols.numChan()(spw_p[k]);
	}


	Vector<Vector<Int> > spwinds_of_uniq_spws(nuniqSpws);
	totnchan_p.resize(nuniqSpws);
	for (uInt k = 0; k < nuniqSpws; ++k)
	{
		Int maxchan = 0;
		uInt j = 0;

		// Add rows to output MS SPW table
		msOut_p.spectralWindow().addRow();

		totnchan_p[k] = 0;
		spwinds_of_uniq_spws[k].resize();
		for (uInt spwind = 0; spwind < spw_p.nelements(); ++spwind)
		{
			if (spw_p[spwind] == spw_uniq_p[k])
			{
				Int highchan = nchan_p[spwind] * chanStep_p[spwind] + chanStart_p[spwind];

				if (highchan > maxchan) maxchan = highchan;

				totnchan_p[k] += nchan_p[spwind];

				// The true is necessary to avoid scrambling previously assigned values.
				spwinds_of_uniq_spws[k].resize(j + 1, true);

				// Warning!  spwinds_of_uniq_spws[k][j] will compile without warning, but dump core at runtime.
				(spwinds_of_uniq_spws[k])[j] = spwind;

				++j;
			}
		}
		if (maxchan > inSpWCols.numChan()(spw_uniq_p[k]))
		{
			os 	<< LogIO::SEVERE
				<< " Channel settings wrong; exceeding number of channels in spw "
				<< spw_uniq_p[k] << LogIO::POST;
			return False;
		}
	}

	// min_k is an index for getting an spw index via spw_uniq_p[min_k].
	// k is an index for getting an spw index via spw_p[k].
	for (uInt min_k = 0; min_k < nuniqSpws; ++min_k)
	{
		uInt k = spwinds_of_uniq_spws[min_k][0];

		if (spwinds_of_uniq_spws[min_k].nelements() > 1 || nchan_p[k] != inSpWCols.numChan()(spw_p[k]))
		{
			Vector<Double> effBWIn = inSpWCols.effectiveBW()(spw_uniq_p[min_k]);
			Int nOutChan = totnchan_p[min_k];
			Vector<Double> chanFreqOut(nOutChan);
			Vector<Double> chanFreqIn = inSpWCols.chanFreq()(spw_uniq_p[min_k]);
			Vector<Double> chanWidthOut(nOutChan);
			Vector<Double> chanWidthIn = inSpWCols.chanWidth()(spw_uniq_p[min_k]);
			Vector<Double> spwResolOut(nOutChan);
			Vector<Double> spwResolIn = inSpWCols.resolution()(spw_uniq_p[min_k]);
			Vector<Double> effBWOut(nOutChan);
			Int outChan = 0;

			keepShape_p = false;

			// The sign of CHAN_WIDTH defaults to +.  Its determination assumes that
			// chanFreqIn is monotonic, but not that the sign of the chanWidthIn is correct.
			Bool neginc = chanFreqIn[chanFreqIn.nelements() - 1] < chanFreqIn[0];

			effBWOut.set(0.0);
			Double totalBW = 0.0;
			for (uInt rangeNum = 0; rangeNum < spwinds_of_uniq_spws[min_k].nelements(); ++rangeNum)
			{
				k = spwinds_of_uniq_spws[min_k][rangeNum];

				Int span = chanStep_p[k] * widths_p[k];

				for (Int j = 0; j < nchan_p[k]; ++j)
				{
					Int inpChan = chanStart_p[k] + j * span;

					if (span > 1)
					{
						Int lastChan = inpChan + span - 1;

						if (lastChan > chanEnd_p[k])
						{
							// The averaging width is not a factor of the number of
							// selected input channels, so the last output bin receives
							// fewer input channels than the other bins.
							lastChan = chanEnd_p[k];

							Int nchan = lastChan - inpChan + 1;
							os 	<< LogIO::NORMAL
								<< "The last output channel of spw "
								<< spw_p[k] << " has only " << nchan
								<< " input channel";
							if (nchan > 1) os << "s.";
							os << LogIO::POST;

						}

						chanFreqOut[outChan] = (chanFreqIn[inpChan]
								+ chanFreqIn[lastChan]) / 2;

						Double sep = chanFreqIn[lastChan] - chanFreqIn[inpChan];

						if (neginc) sep = -sep;

						// The internal abs is necessary because the sign of chanWidthIn may be wrong.
						chanWidthOut[outChan] = sep + 0.5 * abs(chanWidthIn[inpChan] + chanWidthIn[lastChan]);
						if (neginc) chanWidthOut[outChan] = -chanWidthOut[outChan];

						spwResolOut[outChan] = 0.5 * (spwResolIn[inpChan] + spwResolIn[lastChan]) + sep;

						for (Int avgChan = inpChan; avgChan <= lastChan; avgChan += chanStep_p[k])
						{
							effBWOut[outChan] += effBWIn[avgChan];
						}

					}
					else
					{
						chanFreqOut[outChan] = chanFreqIn[inpChan];
						spwResolOut[outChan] = spwResolIn[inpChan];
						chanWidthOut[outChan] = chanWidthIn[inpChan];
						effBWOut[outChan] = effBWIn[inpChan];
					}
					totalBW += effBWOut[outChan];
					++outChan;
				}
			}
			--outChan;

			msSpW.chanFreq().put(min_k, chanFreqOut);
			msSpW.refFrequency().put(min_k,min(chanFreqOut[0], chanFreqOut[chanFreqOut.size() - 1]));
			msSpW.resolution().put(min_k, spwResolOut);
			msSpW.numChan().put(min_k, nOutChan);
			msSpW.chanWidth().put(min_k, chanWidthOut);
			msSpW.effectiveBW().put(min_k, spwResolOut);
			msSpW.totalBandwidth().put(min_k, totalBW);
		}
		else
		{
			msSpW.chanFreq().put(min_k, inSpWCols.chanFreq()(spw_p[k]));
			msSpW.refFrequency().put(min_k, inSpWCols.refFrequency()(spw_p[k]));
			msSpW.resolution().put(min_k, inSpWCols.resolution()(spw_p[k]));
			msSpW.numChan().put(min_k, inSpWCols.numChan()(spw_p[k]));
			msSpW.chanWidth().put(min_k, inSpWCols.chanWidth()(spw_p[k]));
			msSpW.effectiveBW().put(min_k, inSpWCols.effectiveBW()(spw_p[k]));
			msSpW.totalBandwidth().put(min_k,inSpWCols.totalBandwidth()(spw_p[k]));
		}

		msSpW.flagRow().put(min_k, inSpWCols.flagRow()(spw_p[k]));
		msSpW.freqGroup().put(min_k, inSpWCols.freqGroup()(spw_p[k]));
		msSpW.freqGroupName().put(min_k, inSpWCols.freqGroupName()(spw_p[k]));
		msSpW.ifConvChain().put(min_k, inSpWCols.ifConvChain()(spw_p[k]));
		msSpW.measFreqRef().put(min_k, inSpWCols.measFreqRef()(spw_p[k]));
		msSpW.name().put(min_k, inSpWCols.name()(spw_p[k]));
		msSpW.netSideband().put(min_k, inSpWCols.netSideband()(spw_p[k]));
		if (haveSpwAN) msSpW.assocNature().put(min_k, inSpWCols.assocNature()(spw_p[k]));
		if (haveSpwASI) msSpW.assocSpwId().put(min_k, inSpWCols.assocSpwId()(spw_p[k]));
		if (haveSpwBN) msSpW.bbcNo().put(min_k, inSpWCols.bbcNo()(spw_p[k]));
		if (haveSpwBS) msSpW.bbcSideband().put(min_k, inSpWCols.bbcSideband()(spw_p[k]));
		if (haveSpwDI) msSpW.dopplerId().put(min_k, inSpWCols.dopplerId()(spw_p[k]));

	}

	// Write to the DATA_DESCRIPTION table of output MS
	for (uInt dd=0; dd<nddid; ++dd){
		msOut_p.dataDescription().addRow();
		msDD.flagRow().put(dd, False);
		msDD.polarizationId().put(dd, newPolId[dd]);
		msDD.spectralWindowId().put(dd, newDDtable[dd]);
	}


	return true;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
uInt MSTransformDataHandler::addOptionalColumns(const Table& inTab, Table& outTab,const Bool beLazy)
{
	uInt nAdded = 0;
	const TableDesc& inTD = inTab.actualTableDesc();

	// Only rely on the # of columns if you are sure that inTab and outTab
	// can't have the same # of columns without having _different_ columns,
	// i.e. use beLazy if outTab.actualTableDesc() is in its default state.
	uInt nInCol = inTD.ncolumn();
	if (!beLazy || nInCol > outTab.actualTableDesc().ncolumn())
	{
		LogIO os(LogOrigin("MSTransformDataHandler", __FUNCTION__));

		Vector<String> oldColNames = inTD.columnNames();

		for (uInt k = 0; k < nInCol; ++k)
		{
			if (!outTab.actualTableDesc().isColumn(oldColNames[k]))
			{
				try
				{
					outTab.addColumn(inTD.columnDesc(k), false);
					++nAdded;
				}
				// NOT AipsError x
				catch (...)
				{
					os 	<< LogIO::WARN << "Could not add column "
						<< oldColNames[k] << " to " << outTab.tableName()
						<< LogIO::POST;
				}
			}
		}
	}

	return nAdded;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformDataHandler::relabelSources()
{
	LogIO os(LogOrigin("MSTransformDataHandler", __FUNCTION__));

	//Source is an optional table, so it may not exist
	if (Table::isReadable(mssel_p.sourceTableName()))
	{
		// Note that mscIn_p->field().sourceId() has ALL of the
		// sourceIDs in the input MS, not just the selected ones.
		const Vector<Int>& inSrcIDs = mscIn_p->field().sourceId().getColumn();

		Int highestInpSrc = max(inSrcIDs);

		// Ensure space for -1.
		if (highestInpSrc < 0) highestInpSrc = 0;

		sourceRelabel_p.resize(highestInpSrc + 1);
		// Default to "any".
		sourceRelabel_p.set(-1);

		// Enable sourceIDs that are actually referred
		// by selected fields, and remap them using j.
		uInt j = 0;
		for (uInt k = 0; k < fieldid_p.nelements(); ++k)
		{
			Int fldInSrcID = inSrcIDs[fieldid_p[k]];

			if (fldInSrcID > -1)
			{
				// Multiple fields can use the same
				if (sourceRelabel_p[fldInSrcID] == -1)
				{
					// source in a mosaic.
					sourceRelabel_p[fldInSrcID] = j;
					++j;
				}
			}
		}
	}
	else
	{
		// Default to "any".
		os 	<< LogIO::NORMAL
			<< "The input MS does not have the optional SOURCE table.\n"
			<< "-1 will be used as a generic source ID." << LogIO::POST;
		sourceRelabel_p.resize(1);
		sourceRelabel_p.set(-1);
	}

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformDataHandler::copySubtable(const String& tabName, const Table& inTab,const Bool doFilter)
{
	String outName(msOut_p.tableName() + '/' + tabName);

	if (PlainTable::tableCache()(outName)) PlainTable::tableCache().remove(outName);
	inTab.deepCopy(outName, Table::New, False, Table::AipsrcEndian, doFilter);
	Table outTab(outName, Table::Update);
	msOut_p.rwKeywordSet().defineTable(tabName, outTab);
	msOut_p.initRefs();

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformDataHandler::make_map(std::map<Int, Int>& mapper, const Vector<Int>& inv)
{
  std::set<Int> valSet;

	// Strange, but slightly more efficient than going forward.
	for (Int i = inv.nelements(); i--;)
	{
		valSet.insert(inv[i]);
	}

	uInt remaval = 0;
	for (std::set<Int>::const_iterator vs_iter = valSet.begin(); vs_iter != valSet.end(); ++vs_iter)
	{
		mapper[*vs_iter] = remaval;
		++remaval;
	}

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Bool MSTransformDataHandler::copyPointing()
{
	LogIO os(LogOrigin("MSTransformDataHandler", __FUNCTION__));

	//Pointing is allowed to not exist
	if (Table::isReadable(mssel_p.pointingTableName()))
	{
		const MSPointing& oldPoint = mssel_p.pointing();

		if (!antennaSel_p && timeRange_p == "")
		{
			copySubtable(MS::keywordName(MS::POINTING), oldPoint);
		}
		else
		{
			setupNewPointing();

			if (oldPoint.nrow() > 0)
			{
				// Could be declared as Table&
				MSPointing& newPoint = msOut_p.pointing();

				// Add optional columns if present in oldPoint.
				uInt nAddedCols = addOptionalColumns(oldPoint, newPoint, true);
				os << LogIO::DEBUG1 << "POINTING has " << nAddedCols << " optional columns." << LogIO::POST;

				const ROMSPointingColumns oldPCs(oldPoint);
				MSPointingColumns newPCs(newPoint);
				newPCs.setEpochRef(MEpoch::castType(oldPCs.timeMeas().getMeasRef().getType()));
				newPCs.setDirectionRef(MDirection::castType(oldPCs.directionMeasCol().getMeasRef().getType()));
				newPCs.setEncoderDirectionRef(MDirection::castType(oldPCs.encoderMeas().getMeasRef().getType()));

				const ROScalarColumn<Int>& antIds = oldPCs.antennaId();
				const ROScalarColumn<Double>& time = oldPCs.time();
				ScalarColumn<Int>& outants = newPCs.antennaId();

				uInt nTRanges = selTimeRanges_p.ncolumn();

				uInt outRow = 0;

				// Int for comparison
				Int maxSelAntp1 = antNewIndex_p.nelements();
				// with newAntInd.
				for (uInt inRow = 0; inRow < antIds.nrow(); ++inRow)
				{
					Int newAntInd = antIds(inRow);
					if (antennaSel_p)
					{
						newAntInd = newAntInd < maxSelAntp1 ? antNewIndex_p[newAntInd] : -1;
					}

					Double t = time(inRow);

					if (newAntInd > -1)
					{
						Bool matchT = false;
						for (uInt tr = 0; tr < nTRanges; ++tr)
						{
							if (t >= selTimeRanges_p(0, tr) && t <= selTimeRanges_p(1, tr))
							{
								matchT = true;
								break;
							}
						}

						if (matchT)
						{
							TableCopy::copyRows(newPoint, oldPoint, outRow,inRow, 1, false);
							outants.put(outRow, newAntInd);
							++outRow;
						}
					}
				}
				newPoint.flush();
			}
		}
	}
	else
	{
		// Make an empty stub for MSColumns.
		setupNewPointing();
	}


	return True;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformDataHandler::setupNewPointing()
{
	SetupNewTable pointingSetup(msOut_p.pointingTableName(),
	MSPointing::requiredTableDesc(), Table::New);

	// POINTING can be large, set some sensible defaults for storageMgrs
	IncrementalStMan ismPointing("ISMPointing");
	StandardStMan ssmPointing("SSMPointing", 32768);
	pointingSetup.bindAll(ismPointing, True);
	pointingSetup.bindColumn(MSPointing::columnName(MSPointing::DIRECTION),ssmPointing);
	pointingSetup.bindColumn(MSPointing::columnName(MSPointing::TARGET),ssmPointing);
	pointingSetup.bindColumn(MSPointing::columnName(MSPointing::TIME),ssmPointing);
	msOut_p.rwKeywordSet().defineTable(MS::keywordName(MS::POINTING),Table(pointingSetup));
	msOut_p.initRefs();

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Bool MSTransformDataHandler::copySource()
{
	//Source is an optional table, so it may not exist
	if (Table::isReadable(mssel_p.sourceTableName()))
	{
		LogIO os(LogOrigin("MSTransformDataHandler", __FUNCTION__));

		const MSSource& oldSource = mssel_p.source();
		MSSource& newSource = msOut_p.source();

		// Add optional columns if present in oldSource.
		uInt nAddedCols = addOptionalColumns(oldSource, newSource, true);
		os << LogIO::DEBUG1 << "SOURCE has " << nAddedCols << " optional columns." << LogIO::POST;

		const ROMSSourceColumns incols(oldSource);
		MSSourceColumns outcols(newSource);

		// Copy the Measures frame info.  This has to be done before filling the rows.
		outcols.setEpochRef(MEpoch::castType(incols.timeMeas().getMeasRef().getType()));
		outcols.setDirectionRef(MDirection::castType(incols.directionMeas().getMeasRef().getType()));
		outcols.setPositionRef(MPosition::castType(incols.positionMeas().getMeasRef().getType()));
		outcols.setFrequencyRef(MFrequency::castType(incols.restFrequencyMeas().getMeasRef().getType()));
		outcols.setRadialVelocityRef(MRadialVelocity::castType(incols.sysvelMeas().getMeasRef().getType()));

		const ROScalarColumn<Int>& inSId = incols.sourceId();
		ScalarColumn<Int>& outSId = outcols.sourceId();
		const ROScalarColumn<Int>& inSPW = incols.spectralWindowId();
		ScalarColumn<Int>& outSPW = outcols.spectralWindowId();

		// row number in output.
		uInt outrn = 0;
		uInt nInputRows = inSId.nrow();
		// inSidVal is Int.
		Int maxSId = sourceRelabel_p.nelements();
		Int maxSPWId = spwRelabel_p.nelements();
		for (uInt inrn = 0; inrn < nInputRows; ++inrn)
		{
			Int inSidVal = inSId(inrn);
			// -1 means the source is valid for any SPW.
			Int inSPWVal = inSPW(inrn);
			if (inSidVal >= maxSId)
			{
				os << LogIO::WARN << "Invalid SOURCE ID in SOURCE table row " << inrn << LogIO::POST;
			}
			if (inSPWVal >= maxSPWId)
			{
				os << LogIO::WARN << "Invalid SPW ID in SOURCE table row " << inrn << LogIO::POST;
			}

			if (	(inSidVal > -1) &&
					(inSidVal < maxSId) &&
					(sourceRelabel_p[inSidVal] > -1) &&
					((inSPWVal == -1) || (inSPWVal < maxSPWId && spwRelabel_p[inSPWVal] > -1)))
			{
				// Copy inrn to outrn.
				TableCopy::copyRows(newSource, oldSource, outrn, inrn, 1);
				outSId.put(outrn, sourceRelabel_p[inSidVal]);
				outSPW.put(outrn, inSPWVal > -1 ? spwRelabel_p[inSPWVal] : -1);
				++outrn;
			}
		}

		return True;
	}

	return False;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Bool MSTransformDataHandler::copyAntenna()
{
	const MSAntenna& oldAnt = mssel_p.antenna();
	MSAntenna& newAnt = msOut_p.antenna();
	const ROMSAntennaColumns incols(oldAnt);
	MSAntennaColumns outcols(newAnt);
	Bool retval = False;

	outcols.setOffsetRef(MPosition::castType(incols.offsetMeas().getMeasRef().getType()));
	outcols.setPositionRef(MPosition::castType(incols.positionMeas().getMeasRef().getType()));

	if (!antennaSel_p)
	{
		TableCopy::copyRows(newAnt, oldAnt);
		retval = True;
	}
	else
	{
		uInt nAnt = antNewIndex_p.nelements();
		// Don't use min() here, it's too overloaded.
		if (nAnt > oldAnt.nrow()) nAnt = oldAnt.nrow();

		for (uInt k = 0; k < nAnt; ++k)
		{
			if (antNewIndex_p[k] > -1) TableCopy::copyRows(newAnt, oldAnt, antNewIndex_p[k], k, 1, false);
		}
		newAnt.flush();
		retval = True;
	}
	return retval;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Bool MSTransformDataHandler::copyFeed()
{
	LogIO os(LogOrigin("MSTransformDataHandler", __FUNCTION__));

	const MSFeed& oldFeed = mssel_p.feed();
	MSFeed& newFeed = msOut_p.feed();
	const ROMSFeedColumns incols(oldFeed);
	MSFeedColumns outcols(newFeed);

	outcols.setDirectionRef(MDirection::castType(incols.beamOffsetMeas().getMeasRef().getType()));
	outcols.setEpochRef(MEpoch::castType(incols.timeMeas().getMeasRef().getType()));
	outcols.setPositionRef(MPosition::castType(incols.positionMeas().getMeasRef().getType()));

	if (!antennaSel_p && allEQ(spwRelabel_p, spw_p))
	{
		TableCopy::copyRows(newFeed, oldFeed);
	}
	else
	{
		const Vector<Int>& antIds = incols.antennaId().getColumn();
		const Vector<Int>& spwIds = incols.spectralWindowId().getColumn();

		// Copy selected rows.
		uInt totNFeeds = antIds.nelements();
		uInt totalSelFeeds = 0;
		Int maxSelAntp1 = antNewIndex_p.nelements();
		for (uInt k = 0; k < totNFeeds; ++k)
		{
			// antenna must be selected, and spwId must be -1 (any) or selected.
			if (	antIds[k] < maxSelAntp1 &&
					antNewIndex_p[antIds[k]] > -1 &&
					(spwIds[k] < 0 || spwRelabel_p[spwIds[k]] > -1)
				)
			{
				TableCopy::copyRows(newFeed, oldFeed, totalSelFeeds, k, 1,false);
				++totalSelFeeds;
			}
		}
		newFeed.flush();

		// Remap antenna and spw #s.
		ScalarColumn<Int>& antCol = outcols.antennaId();
		ScalarColumn<Int>& spwCol = outcols.spectralWindowId();

		for (uInt k = 0; k < totalSelFeeds; ++k)
		{
			antCol.put(k, antNewIndex_p[antCol(k)]);
			if (spwCol(k) > -1) spwCol.put(k, spwRelabel_p[spwCol(k)]);
		}
	}

	// Check if selected spw is WVR data. WVR data does not have FEED data
	// so it is not a failure if newFeed.nrow == 0
	if (newFeed.nrow() < 1 and spw_p.size() == 1){
		const MSSpectralWindow oldSpw = mssel_p.spectralWindow();
		const ROMSSpWindowColumns spwcols(oldSpw);
		const ROScalarColumn<String>& spwNames = spwcols.name();

		String wvrName = spwNames.asString(spw_p[0]);
		os << LogIO::DEBUG1 << "spw name is "<< wvrName << LogIO::POST;;
		if (wvrName.compare("WVR#NOMINAL") == 0)
			return true;

		// NOTE: for older MSs that do not have NAME set in the SPW table,
		// an alternative solution will be used to get the PROCESSOR_TYPE instead.
		// spw2ddid_p[0] will give the DDID of spw=0
		// use TaQL to get the PROCESSOR_ID from this DDID
		// get the PROCESSOR_TYPE from this PROCESSOR_ID... maybe also using TaQL?

	}

	if (newFeed.nrow() < 1)
	{
//		LogIO os(LogOrigin("MSTransformDataHandler", __FUNCTION__));
		os << LogIO::SEVERE << "No feeds were selected." << LogIO::POST;
		return false;
	}

	return True;
}

// Get the processorId corresponding to a given DDI
Int MSTransformDataHandler::getProcessorId(Int dataDescriptionId, String msname)
{
    ostringstream taql;
    taql << "SELECT PROCESSOR_ID from " << msname;
    taql << " WHERE DATA_DESC_ID==" << dataDescriptionId;
    taql << " LIMIT 1";

    casa::TableProxy *firstSelectedRow = new TableProxy(tableCommand(taql.str()));
    Record colWrapper = firstSelectedRow->getVarColumn(String('PROCESSOR_ID'),0,1,0);
    casa::Vector<Int> processorId = colWrapper.asArrayInt("r1");

    return processorId[0];
}

// Get the processor Type corresponding to a given Processor ID
String MSTransformDataHandler::getProcessorType(Int processorId, String msname)
{
	// How to get row??
    ostringstream taql;
    String procname = msname+"/PROCESSOR";
    taql << "SELECT TYPE from " << procname;
    taql << " WHERE DATA_DESC_ID==" << processorId;
    taql << " LIMIT 1";

    casa::TableProxy *firstSelectedRow = new TableProxy(tableCommand(taql.str()));
    Record colWrapper = firstSelectedRow->getVarColumn(String('PROCESSOR_ID'),0,1,0);
    casa::Vector<Int> processorId = colWrapper.asArrayInt("r1");

    return processorId[0];
};


// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Bool MSTransformDataHandler::copyFlag_Cmd()
{

	// Like POINTING, FLAG_CMD is supposed to exist but is allowed not to.
	if (Table::isReadable(mssel_p.flagCmdTableName()))
	{
		const MSFlagCmd& oldFlag_Cmd = mssel_p.flagCmd();

		if (oldFlag_Cmd.nrow() > 0)
		{

			// Could be declared as Table&
			MSFlagCmd& newFlag_Cmd = msOut_p.flagCmd();

			LogIO os(LogOrigin("MSTransformDataHandler", __FUNCTION__));

			// Add optional columns if present in oldFlag_Cmd.
			uInt nAddedCols = addOptionalColumns(oldFlag_Cmd, newFlag_Cmd, true);
			os << LogIO::DEBUG1 << "FLAG_CMD has " << nAddedCols << " optional columns." << LogIO::POST;

			const ROMSFlagCmdColumns oldFCs(oldFlag_Cmd);
			MSFlagCmdColumns newFCs(newFlag_Cmd);
			newFCs.setEpochRef(MEpoch::castType(oldFCs.timeMeas().getMeasRef().getType()));

			TableCopy::copyRows(newFlag_Cmd, oldFlag_Cmd);

		}
	}

	return True;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Bool MSTransformDataHandler::copyHistory()
{
	LogIO os(LogOrigin("MSTransformDataHandler", __FUNCTION__));

	const MSHistory& oldHistory = mssel_p.history();

	// Could be declared as Table&
	MSHistory& newHistory = msOut_p.history();

	// Add optional columns if present in oldHistory.
	uInt nAddedCols = addOptionalColumns(oldHistory, newHistory, true);
	os << LogIO::DEBUG1 << "HISTORY has " << nAddedCols << " optional columns." << LogIO::POST;

	const ROMSHistoryColumns oldHCs(oldHistory);
	MSHistoryColumns newHCs(newHistory);
	newHCs.setEpochRef(MEpoch::castType(oldHCs.timeMeas().getMeasRef().getType()));

	TableCopy::copyRows(newHistory, oldHistory);

	return True;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Bool MSTransformDataHandler::copyObservation()
{
	const MSObservation& oldObs = mssel_p.observation();
	MSObservation& newObs = msOut_p.observation();
	const ROMSObservationColumns oldObsCols(oldObs);
	MSObservationColumns newObsCols(newObs);
	newObsCols.setEpochRef(MEpoch::castType(oldObsCols.releaseDateMeas().getMeasRef().getType()));

	uInt nObs = selObsId_p.nelements();
	if (nObs > 0)
	{
		for (uInt outrn = 0; outrn < nObs; ++outrn)
		{
			TableCopy::copyRows(newObs, oldObs, outrn, selObsId_p[outrn], 1);
		}

	}
	else
	{
		TableCopy::copyRows(newObs, oldObs);
	}

	return True;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Bool MSTransformDataHandler::copyProcessor()
{
	const MSProcessor& oldProc = mssel_p.processor();
	MSProcessor& newProc = msOut_p.processor();
	TableCopy::copyRows(newProc, oldProc);

	return True;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Bool MSTransformDataHandler::copyState()
{
	// STATE is allowed to not exist, even though it is not optional in
	// the MS def'n.  For one thing, split dropped it for quite a while.
	if (Table::isReadable(mssel_p.stateTableName()))
	{
		LogIO os(LogOrigin("MSTransformDataHandler", __FUNCTION__));
		const MSState& oldState = mssel_p.state();

		if (oldState.nrow() > 0)
		{
			if (!intentString_p.empty())
			{
				MSState& newState = msOut_p.state();
				const ROMSStateColumns oldStateCols(oldState);
				MSStateColumns newStateCols(newState);

				// Initialize stateRemapper_p if necessary.
				// if (stateRemapper_p.size() < 1) make_map(stateRemapper_p, mscIn_p->stateId().getColumn());

				// jagonzal (CAS-6351): Do not apply implicit re-indexing //////////////////////////////////////////////////
				//
				// Get list of selected scan intent indexes
				MSSelection mssel;
				mssel.setStateExpr(intentString_p);
				Vector<Int> scanIntentList = mssel.getStateObsModeList(getInputMS());
				//
				// Populate state re-mapper using all selected indexes (not only the implicit ones)
				stateRemapper_p.clear();
				for (uInt index=0; index < scanIntentList.size(); index++)
				{
					stateRemapper_p[scanIntentList(index)] = index;
				}
				///////////////////////////////////////////////////////////////////////////////////////////////////////////

				uInt nStates = stateRemapper_p.size();

				// stateRemapper_p goes from input to output, as is wanted in most
				// places.  Here we need a map going the other way, so make one.
				Vector<Int> outStateToInState(nStates);
				std::map<Int, Int>::iterator mit;

				for (mit = stateRemapper_p.begin(); mit != stateRemapper_p.end(); ++mit)
				{
					outStateToInState[(*mit).second] = (*mit).first;
				}


				for (uInt outrn = 0; outrn < nStates; ++outrn)
				{
					TableCopy::copyRows(newState, oldState, outrn,outStateToInState[outrn], 1);
				}
			}
			// jagonzal (CAS-6351): Do not apply implicit re-indexing
			// Therefore just copy the input state sub-table to the output state sub-table
			else
			{
				const MSState& oldState = mssel_p.state();
				MSState& newState = msOut_p.state();
				TableCopy::copyRows(newState, oldState);
			}

		}
	}
	return True;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Bool MSTransformDataHandler::copySyscal()
{
	// SYSCAL is allowed to not exist.
	if (Table::isReadable(mssel_p.sysCalTableName()))
	{
		const MSSysCal& oldSysc = mssel_p.sysCal();

		if (oldSysc.nrow() > 0)
		{
			// Add a SYSCAL subtable to msOut_p with 0 rows for now.
			Table::TableOption option = Table::New;
			TableDesc sysCalTD = MSSysCal::requiredTableDesc();
			SetupNewTable sysCalSetup(msOut_p.sysCalTableName(), sysCalTD,option);
			msOut_p.rwKeywordSet().defineTable(MS::keywordName(MS::SYSCAL),Table(sysCalSetup, 0));

			// update the references to the subtable keywords
			msOut_p.initRefs();

			// Could be declared as Table&.
			MSSysCal& newSysc = msOut_p.sysCal();

			LogIO os(LogOrigin("MSTransformDataHandler", __FUNCTION__));

			uInt nAddedCols = addOptionalColumns(oldSysc, newSysc, true);
			os << LogIO::DEBUG1 << "SYSCAL has " << nAddedCols << " optional columns." << LogIO::POST;

			const ROMSSysCalColumns incols(oldSysc);
			MSSysCalColumns outcols(newSysc);
			outcols.setEpochRef(MEpoch::castType(incols.timeMeas().getMeasRef().getType()));

			if (!antennaSel_p && allEQ(spwRelabel_p, spw_p))
			{
				TableCopy::copyRows(newSysc, oldSysc);
			}
			else
			{
				const Vector<Int>& antIds = incols.antennaId().getColumn();
				const Vector<Int>& spwIds = incols.spectralWindowId().getColumn();

				// Copy selected rows.
				uInt totNSyscals = antIds.nelements();
				uInt totalSelSyscals = 0;
				Int maxSelAntp1 = antNewIndex_p.nelements(); // Int for comparison with antIds.
				for (uInt k = 0; k < totNSyscals; ++k)
				{
					// antenna must be selected, and spwId must be -1 (any) or selected.
					if (	antIds[k] < maxSelAntp1 &&
							antNewIndex_p[antIds[k]] > -1 &&
							(spwIds[k] < 0 || spwRelabel_p[spwIds[k]] > -1)
						)
					{
						TableCopy::copyRows(newSysc, oldSysc, totalSelSyscals,k, 1, false);
						++totalSelSyscals;
					}
				}
				newSysc.flush();

				// Remap antenna and spw #s.
				ScalarColumn<Int>& antCol = outcols.antennaId();
				ScalarColumn<Int>& spwCol = outcols.spectralWindowId();

				for (uInt k = 0; k < totalSelSyscals; ++k)
				{
					antCol.put(k, antNewIndex_p[antCol(k)]);
					if (spwCol(k) > -1) spwCol.put(k, spwRelabel_p[spwCol(k)]);
				}
			}
		}
	}

	return True;
}

Bool MSTransformDataHandler::copyWeather()
{
	// Weather is allowed to not exist.
	if (Table::isReadable(mssel_p.weatherTableName()))
	{
		const MSWeather& oldWeath = mssel_p.weather();

		if (oldWeath.nrow() > 0)
		{
			// Add a WEATHER subtable to msOut_p with 0 rows for now.
			Table::TableOption option = Table::New;
			TableDesc weatherTD = MSWeather::requiredTableDesc();
			SetupNewTable weatherSetup(msOut_p.weatherTableName(), weatherTD,option);
			msOut_p.rwKeywordSet().defineTable(MS::keywordName(MS::WEATHER),Table(weatherSetup, 0));

			// update the references to the subtable keywords
			msOut_p.initRefs();

			MSWeather& newWeath = msOut_p.weather(); // Could be declared as Table&

			LogIO os(LogOrigin("MSTransformDataHandler", __FUNCTION__));

			uInt nAddedCols = addOptionalColumns(oldWeath, newWeath, true);
			os << LogIO::DEBUG1 << "WEATHER has " << nAddedCols << " optional columns." << LogIO::POST;

			const ROMSWeatherColumns oldWCs(oldWeath);
			MSWeatherColumns newWCs(newWeath);
			newWCs.setEpochRef(MEpoch::castType(oldWCs.timeMeas().getMeasRef().getType()));

			if (!antennaSel_p)
			{
				TableCopy::copyRows(newWeath, oldWeath);
			}
			else
			{
				const Vector<Int>& antIds(oldWCs.antennaId().getColumn());
				ScalarColumn<Int>& outants = newWCs.antennaId();

				uInt selRow = 0;
				Int maxSelAntp1 = antNewIndex_p.nelements();

				for (uInt k = 0; k < antIds.nelements(); ++k)
				{
					// Weather station is on antenna?
					if (antIds[k] > -1)
					{
						// remap ant num
						if (antIds[k] < maxSelAntp1)
						{
							Int newAntInd = antNewIndex_p[antIds[k]];

							// Ant selected?
							if (newAntInd > -1)
							{
								TableCopy::copyRows(newWeath, oldWeath, selRow,k, 1);
								outants.put(selRow, newAntInd);
								++selRow;
							}
						}
					}
					else
					{
						// means valid for all antennas.
						TableCopy::copyRows(newWeath, oldWeath, selRow, k, 1);
						outants.put(selRow, -1);
						++selRow;
					}
				}
			}
		}
	}

	return True;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Bool MSTransformDataHandler::filterOptSubtable(const String& subtabname)
{
	LogIO os(LogOrigin("MSTransformDataHandler", __FUNCTION__));

	// subtabname is allowed to not exist.  Use ms_p instead of mssel_p, because
	// the latter has a random name which does NOT necessarily lead to
	// subtabname.  (And if selection took care of subtables, we probably
	// wouldn't have to do it here.)
	if (Table::isReadable(ms_p.tableName() + '/' + subtabname))
	{
		const Table intab(ms_p.tableName() + '/' + subtabname);

		if (intab.nrow() > 0) {

			// Add feed if selecting by it is ever added.
			Bool doFilter = antennaSel_p || !allEQ(spwRelabel_p, spw_p);

			copySubtable(subtabname, intab, doFilter);

			if (doFilter) {

				// At this point msOut_p has subtab with 0 rows.
				Table outtab(msOut_p.tableName() + '/' + subtabname,Table::Update);
				ROScalarColumn<Int> inAntIdCol(intab, "ANTENNA_ID");
				ROScalarColumn<Int> inSpwIdCol(intab, "SPECTRAL_WINDOW_ID");
				const Vector<Int>& antIds = inAntIdCol.getColumn();
				const Vector<Int>& spwIds = inSpwIdCol.getColumn();

				// Copy selected rows.
				uInt totNOuttabs = antIds.nelements();
				uInt totalSelOuttabs = 0;

				// Int for comparison with antIds.
				Int maxSelAntp1 = antNewIndex_p.nelements();

				Bool haveRemappingProblem = False;
				for (uInt inrow = 0; inrow < totNOuttabs; ++inrow)
				{
					// antenna must be selected, and spwId must be -1 (any) or selected.
					// Extra care must be taken because for a while MSes had CALDEVICE
					// and SYSPOWER, but the ANTENNA_ID and SPECTRAL_WINDOW_ID of those
					// sub-tables were not being re-mapped by split.
					if (antIds[inrow] < maxSelAntp1 && antNewIndex_p[antIds[inrow]] > -1)
					{

						if (spwIds[inrow] < 0 ||
								(spwIds[inrow] < static_cast<Int> (spwRelabel_p.nelements()) &&
										spwRelabel_p[spwIds[inrow]] > -1))
						{
							TableCopy::copyRows(outtab, intab, totalSelOuttabs,inrow, 1, false);
							++totalSelOuttabs;
						}

						// Ideally we'd like to catch antenna errors too.  They are
						// avoided, but because of the way antNewIndex_p is made,
						// antIds[inrow] >= maxSelAntp1
						// is not necessarily an error.  It's not even possible to catch
						// all the spw errors, so we settle for catching the ones we can
						// and reliably avoiding segfaults.
						else if (spwIds[inrow] >= static_cast<Int> (spwRelabel_p.nelements()))
						{
							haveRemappingProblem = True;
						}

					}
				}

				outtab.flush();
				if (haveRemappingProblem)
				{
					os 	<< LogIO::WARN << "At least one row of "
						<< intab.tableName()
						<< " has an antenna or spw mismatch.\n"
						<< "(Presumably from an older split, sorry.)\n"
						<< "If " << subtabname
						<< " is important, it should be fixed with tb or browsetable,\n"
						<< "or by redoing the split that made "
						<< ms_p.tableName() << " (check its history)."
						<< LogIO::POST;
				}


				// Remap antenna and spw #s.
				ScalarColumn<Int> outAntCol(outtab, "ANTENNA_ID");
				ScalarColumn<Int> outSpwCol(outtab, "SPECTRAL_WINDOW_ID");

				for (uInt k = 0; k < totalSelOuttabs; ++k)
				{
					outAntCol.put(k, antNewIndex_p[outAntCol(k)]);
					if (outSpwCol(k) > -1) outSpwCol.put(k, spwRelabel_p[outSpwCol(k)]);
				}
			}
		}
	}

	return True;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Bool MSTransformDataHandler::copyGenericSubtables()
{
	LogIO os(LogOrigin("MSTransformDataHandler", __FUNCTION__));

	// Already handled subtables will be removed from this,
	// so a modifiable copy is needed.
	TableRecord inkws(mssel_p.keywordSet());

	// Some of the standard subtables need special handling,
	// e.g. DATA_DESCRIPTION, SPECTRAL_WINDOW, and ANTENNA, so they are already
	// defined in msOut_p.  Several more (e.g. FLAG_CMD) were also already
	// created by MS::createDefaultSubtables().  Don't try to write over them - a
	// locking error will result.
	const TableRecord& outkws = msOut_p.keywordSet();
	for (uInt i = 0; i < outkws.nfields(); ++i)
	{
		if (outkws.type(i) == TpTable && inkws.isDefined(outkws.name(i)))
		{
			inkws.removeField(outkws.name(i));
		}
	}

	// msOut_p.rwKeywordSet() (pass a reference, not a copy) will put a lock on msOut_p.
	TableCopy::copySubTables(msOut_p.rwKeywordSet(), inkws, msOut_p.tableName(), msOut_p.tableType(), mssel_p);

	// TableCopy::copySubTables(Table, Table, Bool) includes this other code,
	// which seems to be copying subtables at one level deeper, but not recursively?
	const TableDesc& inDesc = mssel_p.tableDesc();
	const TableDesc& outDesc = msOut_p.tableDesc();
	for (uInt i = 0; i < outDesc.ncolumn(); ++i)
	{
		// Only writable cols can have keywords (and thus subtables) defined.
		if (msOut_p.isColumnWritable(i))
		{
			const String& name = outDesc[i].name();

			if (inDesc.isColumn(name))
			{
			    TableColumn outCol(msOut_p, name);
			    ROTableColumn inCol(mssel_p, name);

			    TableCopy::copySubTables(outCol.rwKeywordSet(),
						     inCol.keywordSet(),
						     msOut_p.tableName(),
						     msOut_p.tableType(),
						     mssel_p);
			    // Copy the keywords if column is FLOAT_DATA
			    if (name == "FLOAT_DATA")
				copyMainTableKeywords(outCol.rwKeywordSet(), inCol.keywordSet());

			}
		}
	}

	msOut_p.flush();

	return true;
}

// -----------------------------------------------------------------------
// Method to merge SPW sub-tables from SubMSs to create the MMS level SPW sub-table
// -----------------------------------------------------------------------
Bool MSTransformDataHandler::mergeSpwSubTables(Vector<String> filenames)
{
	String filename_0 = filenames(0);
	MeasurementSet ms_0(filename_0,Table::Update);
	MSSpectralWindow spwTable_0 = ms_0.spectralWindow();
	MSSpWindowColumns spwCols_0(spwTable_0);

	uInt rowIndex = spwTable_0.nrow();

	for (uInt subms_index=1;subms_index < filenames.size();subms_index++)
	{
		String filename_i = filenames(subms_index);
		MeasurementSet ms_i(filename_i);
		MSSpectralWindow spwTable_i = ms_i.spectralWindow();
		MSSpWindowColumns spwCols_i(spwTable_i);

		spwTable_0.addRow(spwTable_i.nrow());

		for (uInt subms_row_index=0;subms_row_index<spwTable_i.nrow();subms_row_index++)
		{
			spwCols_0.measFreqRef().put(rowIndex,spwCols_i.measFreqRef()(subms_row_index));
			spwCols_0.chanFreq().put(rowIndex,spwCols_i.chanFreq()(subms_row_index));
			spwCols_0.refFrequency().put(rowIndex,spwCols_i.refFrequency()(subms_row_index));
			spwCols_0.chanWidth().put(rowIndex,spwCols_i.chanWidth()(subms_row_index));
			spwCols_0.effectiveBW().put(rowIndex,spwCols_i.effectiveBW()(subms_row_index));
			spwCols_0.resolution().put(rowIndex,spwCols_i.resolution()(subms_row_index));
			spwCols_0.flagRow().put(rowIndex,spwCols_i.flagRow()(subms_row_index));
			spwCols_0.freqGroup().put(rowIndex,spwCols_i.freqGroup()(subms_row_index));
			spwCols_0.freqGroupName().put(rowIndex,spwCols_i.freqGroupName()(subms_row_index));
			spwCols_0.ifConvChain().put(rowIndex,spwCols_i.ifConvChain()(subms_row_index));
			spwCols_0.name().put(rowIndex,spwCols_i.name()(subms_row_index));
			spwCols_0.netSideband().put(rowIndex,spwCols_i.netSideband()(subms_row_index));
			spwCols_0.numChan().put(rowIndex,spwCols_i.numChan()(subms_row_index));
			spwCols_0.totalBandwidth().put(rowIndex,spwCols_i.totalBandwidth()(subms_row_index));

			// Optional columns (BBC_NO, ASSOC_SPW_ID, ASSOC_NATURE)
			if (spwCols_i.bbcNo().isNull()==false and spwCols_i.bbcNo().hasContent()==true)
				spwCols_0.bbcNo().put(rowIndex,spwCols_i.bbcNo()(subms_row_index));

			if (spwCols_i.assocSpwId().isNull()==false and spwCols_i.assocSpwId().hasContent()==true)
				spwCols_0.assocSpwId().put(rowIndex,spwCols_i.assocSpwId()(subms_row_index));

			if(spwCols_i.assocNature().isNull()==false and spwCols_i.assocNature().hasContent()==true)
				spwCols_0.assocNature().put(rowIndex,spwCols_i.assocNature()(subms_row_index));

			rowIndex += 1;
		}
	}

	ms_0.flush(True);

	mergeDDISubTables(filenames);

	return True;
}

// -----------------------------------------------------------------------
// Method to merge DDI sub-tables from SubMSs to create the MMS-level DDI sub-table
// -----------------------------------------------------------------------
Bool MSTransformDataHandler::mergeDDISubTables(Vector<String> filenames)
{
	String filename_0 = filenames(0);
	MeasurementSet ms_0(filename_0,Table::Update);
	MSDataDescription ddiTable_0 = ms_0.dataDescription();
	MSDataDescColumns ddiCols_0(ddiTable_0);

	uInt rowIndex = ddiTable_0.nrow();

	for (uInt subms_index=1;subms_index < filenames.size();subms_index++)
	{
		String filename_i = filenames(subms_index);
		MeasurementSet ms_i(filename_i);
		MSDataDescription dditable_i = ms_i.dataDescription();
		MSDataDescColumns ddicols_i(dditable_i);

		ddiTable_0.addRow(dditable_i.nrow());


		for (uInt subms_row_index=0;subms_row_index<dditable_i.nrow();subms_row_index++)
		{
			// get the last spw id entered in the 0th DD table
			uInt spwid = ddiCols_0.spectralWindowId().get(rowIndex-1);

			ddiCols_0.flagRow().put(rowIndex,ddicols_i.flagRow()(subms_row_index));
			ddiCols_0.polarizationId().put(rowIndex,ddicols_i.polarizationId()(subms_row_index));
			ddiCols_0.spectralWindowId().put(rowIndex,spwid+1);
			rowIndex += 1;
		}
	}

	ms_0.flush(True);

	return True;
}

// -----------------------------------------------------------------------
// Work-around to copy the keywords of the FLOAT_DATA column to the output MS
// -----------------------------------------------------------------------
void MSTransformDataHandler::copyMainTableKeywords (TableRecord& outKeys,
		const TableRecord& inKeys)
{
	for (uInt i=0; i<inKeys.nfields(); i++) {
		if (inKeys.type(i) == TpString) {
			// Add keywords for MAIN table columns such as FLOAT_DATA
			String ikey = inKeys.name(i);
			if (!outKeys.isDefined (ikey)) {
				String keyval;
				inKeys.get(ikey, keyval);
				outKeys.define(ikey,keyval);
			}

		}

	}
}


} //# NAMESPACE CASA - END
