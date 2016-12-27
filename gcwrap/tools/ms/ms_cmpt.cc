// -*- C++ -*-
//# Framework independent implementation file for ms..
//# Copyright (C) 2006-2007-2008
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
//# @author
//# @version
//////////////////////////////////////////////////////////////////////////////

#include <casa/Arrays/ArrayMath.h>
#include <casa/OS/DOos.h>
#include <casa/OS/File.h>
#include <casa/Quanta/MVTime.h>
#include <fits/FITS/FITSReader.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSHistoryHandler.h>
#include <ms/MeasurementSets/MSRange.h>
#include <ms/MSOper/MSConcat.h>
#include <ms/MSOper/MSLister.h>
#include <ms/MSOper/MSSummary.h>
#include <ms/MSSel/MSSelectionTools.h>
#include <ms/MSSel/MSSelUtil2.h>
#include <msfits/MSFits/MSFitsInput.h>
#include <msfits/MSFits/MSFitsOutput.h>
#include <msfits/MSFits/MSFitsIDI.h>
#include <tables/Tables/ConcatTable.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/TableCopy.h>

#include <asdmstman/AsdmStMan.h>
#include <mstransform/TVI/ChannelAverageTVI.h>
#include <msvis/MSVis/AveragingVi2Factory.h>
#include <msvis/MSVis/LayeredVi2Factory.h>
#include <msvis/MSVis/MSChecker.h>
#include <msvis/MSVis/MSContinuumSubtractor.h>
#include <msvis/MSVis/Partition.h>
#include <msvis/MSVis/Reweighter.h>
#include <msvis/MSVis/SubMS.h>
#include <msvis/MSVis/VisibilityIterator.h>
#include <msvis/MSVis/VisibilityIterator2.h>
#include <msvis/MSVis/VisBuffer.h>
#include <msvis/MSVis/VisBuffer2.h>
#include <msvis/MSVis/VisIterator.h>
#include <msvis/MSVis/VisSet.h>
#include <msvis/MSVis/VisSetUtil.h>
#include <msvis/MSVis/ViFrequencySelection.h>

#include <msvis/MSVis/statistics/Vi2ChunkStatisticsIteratee.h>
#include <msvis/MSVis/statistics/Vi2ChunkDataProvider.h>
#include <msvis/MSVis/statistics/Vi2ChunkVisAmplitudeProvider.h>
#include <msvis/MSVis/statistics/Vi2ChunkVisPhaseProvider.h>
#include <msvis/MSVis/statistics/Vi2ChunkVisRealProvider.h>
#include <msvis/MSVis/statistics/Vi2ChunkVisImaginaryProvider.h>
#include <msvis/MSVis/statistics/Vi2ChunkFloatVisDataProvider.h>
#include <msvis/MSVis/statistics/Vi2ChunkUVRangeDataProvider.h>
#include <msvis/MSVis/statistics/Vi2ChunkFlagCubeDataProvider.h>
#include <msvis/MSVis/statistics/Vi2ChunkAntennaDataProvider.h>
#include <msvis/MSVis/statistics/Vi2ChunkFeedDataProvider.h>
#include <msvis/MSVis/statistics/Vi2ChunkFieldIdDataProvider.h>
#include <msvis/MSVis/statistics/Vi2ChunkArrayIdDataProvider.h>
#include <msvis/MSVis/statistics/Vi2ChunkDataDescriptionIdsDataProvider.h>
#include <msvis/MSVis/statistics/Vi2ChunkFlagRowDataProvider.h>
#include <msvis/MSVis/statistics/Vi2ChunkIntervalDataProvider.h>
#include <msvis/MSVis/statistics/Vi2ChunkScanDataProvider.h>
#include <msvis/MSVis/statistics/Vi2ChunkTimeDataProvider.h>
#include <msvis/MSVis/statistics/Vi2ChunkWeightSpectrumDataProvider.h>

#include <ms_cmpt.h>
#include <msmetadata_cmpt.h>
#include <tools/ms/Statistics.h>

#include <casa/namespace.h>
#include <cassert>

using namespace casacore;
namespace casac {

ms::ms()
{
	try {
		itsMS = new MeasurementSet(); // working MS
		itsOriginalMS = new MeasurementSet();  // before transformation
		itsSelectedMS = new MeasurementSet();  // accumulate selections
        itsSel = new MSSelector();
		itsLog = new LogIO();
        itsMSS = new MSSelection();
		itsVI = NULL;
		itsVI2 = NULL;
        doingIterations_p=false;
        doingAveraging_p=false;
        polnExpr_p = "";
        wantedpol_p.resize();
        ifrnumbers_p.resize();
        chansel_p.clear();
        chanselExpr_p = "";
        initSel_p = False;
        maxrows_p = False;
        nAnt1_p = 0;
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
}

ms::~ms()
{
	try {
		if(itsMS)           {delete itsMS;itsMS=NULL;}
		if(itsOriginalMS)   {delete itsOriginalMS;itsOriginalMS=NULL;}
		if(itsSelectedMS)   {delete itsSelectedMS;itsSelectedMS=NULL;}
        if(itsSel)          {delete itsSel; itsSel=NULL;}
		if(itsLog)          {delete itsLog; itsLog=NULL;}
		if(itsMSS)          {delete itsMSS; itsMSS=NULL;}
		if (itsVI)          {delete itsVI; itsVI=NULL;}
		if (itsVI2)         {delete itsVI2; itsVI2=NULL;}
		doingIterations_p=false;
		doingAveraging_p=false;
        polnExpr_p = "";
        wantedpol_p.resize();
        ifrnumbers_p.resize();
        chansel_p.clear();
        chanselExpr_p = "";
        initSel_p = False;
        maxrows_p = False;
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
}

/////// Internal helper functions ///////

// Takes a variant a returns a casa String, converting -1 to "" along the way.
inline static String m1toBlankCStr_(const ::casac::variant& v)
{
	String cs(toCasaString(v));
	return cs == String("-1") ? "" : cs;
}

// Returns whether or not the MS pointed to by itsMS can be written to,
// avoiding a weird exception from itsMS->isWritable() if no ms is attached.
inline Bool ms::ready2write_()
{
	return (!detached() && itsMS->isWritable());
}
////// End of helper functions //////
bool
ms::listfits(const std::string &fitsfile)
{
	FITSReader fr;
	fr.listFits(fitsfile.c_str());
	return 1;
}


bool
ms::createmultims(const std::string &outputTableName,
                  const std::vector<std::string> &tableNames,
                  const std::vector<std::string> &subtableNames,
                  const bool nomodify,
                  const bool lock,
                  const bool copysubtables,
                  const std::vector<std::string> &omitSubtableNames)
{
	*itsLog << LogOrigin("ms", "createmultims");

	try {
		Block<String> tableNameVector(tableNames.size());
		Block<String> subtableVector(subtableNames.size());
		Block<String> omitSubtables(omitSubtableNames.size());

		/* Copy the input vectors into Block */
		for (uInt idx=0; idx<tableNameVector.nelements(); idx++)
			tableNameVector[idx] = tableNames[idx];

		for (uInt idx=0; idx<subtableVector.nelements(); idx++)
			subtableVector[idx] = subtableNames[idx];

		for (uInt idx=0; idx<omitSubtables.nelements(); idx++)
			omitSubtables[idx] = omitSubtableNames[idx];

		if((tableNameVector.nelements()>1) && copysubtables){
			*itsLog << LogIO::NORMAL << "Copying subtables from " << tableNameVector[0]
			        << " to the other MMS members." << LogIO::POST;
			Table firstTab(tableNameVector[0]);
			for(uInt idx=1; idx<tableNameVector.nelements(); idx++){
				Table otherTab(tableNameVector[idx], Table::Update);
				TableCopy::copySubTables (otherTab, firstTab,
				                          false, // noRows==false, i.e. subtables are copied
				                          omitSubtables);
			}
		}

		TableLock tlock(TableLock::AutoNoReadLocking);

		{
			ConcatTable concatTable(tableNameVector,
			                        subtableVector,
			                        "SUBMSS", // move all member tables into subdirectory SUBMSS
			                        Table::New,
			                        tlock,
			                        TSMOption::Default);
			concatTable.tableInfo().setSubType("CONCATENATED");
			concatTable.rename(outputTableName, Table::New);
		}


	} catch (AipsError ex) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << ex.getMesg()
		        << LogIO::POST;
		return false;
	}
	/* Now open our new MS so it is referred to by the tool */
	return open(outputTableName, nomodify, lock);
}

bool ms::ismultims()
{
	bool rstat(false);
	try {
		if(!detached()){
			rstat = (itsMS->tableInfo().subType() == "CONCATENATED");
		}
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return rstat;
}


std::vector<std::string> ms::getreferencedtables()
{

	std::vector<std::string> rvalue(0);
	try {
		if (!detached()) {
			Block<String> refTables = itsMS->getPartNames();
			rvalue.resize(refTables.nelements());

			/* Copy the return block to an output vector */
			for (uInt idx=0; idx<rvalue.size(); idx++)
				rvalue[idx] = refTables[idx];

		}
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return rvalue;
}

int
ms::nrow(const bool selected)
{
	Int rstat(0);
	try {
		if(!detached()){
			if(!selected)
				rstat = itsMS->nrow();
			else
				rstat = itsSel->nrow();
		}
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(True);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(True);
	return rstat;
}

int
ms::nrow2(const bool selected)
{
	*itsLog << LogOrigin("ms", "nrow");
	Int rstat(0);
    try {
		if(!detached()){
			if(!selected)
				rstat = itsOriginalMS->nrow();
			else
				rstat = itsSelectedMS->nrow();
		}
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return rstat;
}

bool
ms::iswritable()
{
	Bool rstat(false);
	try {
		rstat = ready2write_();
	}
	catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return rstat;
}

bool
ms::open(const std::string& thems, bool nomodify, bool lock, bool check)
{
	try {
		*itsLog << LogOrigin("ms", "open");
		const Table::TableOption openOption = nomodify ? Table::Old : Table::Update;
		TableLock tl;
		if (lock) tl = TableLock(TableLock::PermanentLocking);
		//
		//  A little book-keeping here, to make sure locks are not held and to noleak some ms's
		//
		if(!itsMS->isNull()){
			close();
		}
		*itsMS = MeasurementSet(thems, tl, openOption);
		if (check) {
			MSChecker msChecker(*itsMS);
			try {
				msChecker.checkReferentialIntegrity();
			}
			catch(const AipsError& x) {
				close();
				RETHROW(x);
			}
		}
		*itsOriginalMS = MeasurementSet(*itsMS);
		*itsSelectedMS = MeasurementSet(*itsMS);
        if(itsSel){
			delete itsSel;
			itsSel = new MSSelector();
		}
        itsSel->setMS(*itsMS);
        if (itsMSS)
		{
			delete itsMSS;
			itsMSS = new MSSelection();
			itsMSS->resetMS(*itsMS);
		}
		doingIterations_p=false;
		doingAveraging_p=false;
        polnExpr_p = "";
        wantedpol_p.resize();
        ifrnumbers_p.resize();
        chansel_p.clear();
        chanselExpr_p = "";
        initSel_p = False;
	} catch (const AipsError& x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	return true;
}

bool
ms::reset()
{
	try {
		*itsLog << LogOrigin("ms", "reset");
		// Set itsMS to the original MS, and re-make the various objects
		// that hold the pointer to working MS
		*itsMS = MeasurementSet(*itsOriginalMS);
		*itsSelectedMS = MeasurementSet(*itsMS);
        if(itsSel)  {delete itsSel;}  itsSel = new MSSelector();
        if (itsMSS) {delete itsMSS;}  itsMSS = new MSSelection();
        itsMSS->resetMS(*itsMS);
        itsSel->setMS(*itsMS);
		doingIterations_p=false;
		doingAveraging_p=false;
        polnExpr_p = "";
        wantedpol_p.resize();
        ifrnumbers_p.resize();
        chansel_p.clear();
        chanselExpr_p = "";
        initSel_p = False;
        maxrows_p = False;
	}
	catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: "
		        << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	return true;
}

bool
ms::fromfits(const std::string& msfile, const std::string &fitsfile,
             const bool nomodify, const bool lock,
             const int obstype, const std::string &,//host,
             bool, //forcenewserver,
             const std::string& antnamescheme)
{
	try {
		*itsLog << LogOrigin("ms", "fromfits")
		        << LogIO::NORMAL3 << "Opening fits file " << fitsfile << LogIO::POST;
		String namescheme(antnamescheme);
		namescheme.downcase();
		MSFitsInput msfitsin(String(msfile), String(fitsfile), (namescheme=="new"));
		msfitsin.readFitsFile(obstype);
		*itsLog << LogOrigin("ms", "fromfits")
		        << LogIO::NORMAL3 << "Flushing MS " << msfile
		        << " to disk" << LogIO::POST;


		open(msfile, nomodify, lock);
	}
	catch (const AipsError& x) {
		*itsLog << LogOrigin("ms", "fromfits")
		        << LogIO::SEVERE << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return true;
}

bool
ms::fromfitsidi(const std::string& msfile, const std::string &fitsidifile, const bool nomodify, const bool lock,
                const int obstype)
{
	try {
		*itsLog << LogIO::NORMAL3 << "Opening FITS-IDI file " << fitsidifile << LogIO::POST;

		{
			MSFitsIDI msfitsidi(String(fitsidifile), String(msfile), !nomodify, obstype); // i.e. overwrite == !nomodify
			msfitsidi.fillMS();
		} // let the msfitsidi go out of scope in order to close the new MS

		*itsLog << LogIO::NORMAL3 << "Flushing MS " << msfile << " to disk" << LogIO::POST;

		open(msfile, nomodify, lock);
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return true;
}

bool
ms::close()
{
	Bool rstat(false);
	try {
		if(!detached()){
			*itsLog << LogOrigin("ms", "close");
			*itsLog << LogIO::NORMAL3;
			if (itsMS->isWritable()) {
				*itsLog << "Flushing data to disk and detaching from file.";
			}
			else {
				*itsLog << "Readonly measurement set: just detaching from file.";
			}
			*itsLog << LogIO::POST;
			delete itsMS;          itsMS = new MeasurementSet();
			delete itsOriginalMS;  itsOriginalMS = new MeasurementSet();
			delete itsSelectedMS;  itsSelectedMS = new MeasurementSet();
            itsSel->setMS(*itsMS);
            if (itsMSS) {delete itsMSS;  itsMSS = new MSSelection();};
			if (itsVI) {delete itsVI; itsVI=NULL;}
			if (itsVI2) {delete itsVI2; itsVI2=NULL;}
			doingIterations_p=false;
		    doingAveraging_p=false;
            polnExpr_p = "";
            wantedpol_p.resize();
            ifrnumbers_p.resize();
            chansel_p.clear();
            initSel_p = False;
            maxrows_p = False;
			rstat = true;
		}
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return rstat;
}

std::string
ms::name()
{
	std::string rstat("none");
	try {
		if(!detached()){
			rstat = itsOriginalMS->tableName();
		}
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return rstat;
}

/*
  ::casac::record*
  ms::command(const std::string& msfile, const std::string& command, const bool nomodify)
  {

  *itsLog << LogOrigin("ms", "command");
  *itsLog << "not implemented"<<LogIO::POST;
  Table::relinquishAutoLocks(true);
  return 0;
  }
*/

bool ms::tofits(
	const std::string& fitsfile, const std::string& column,
	const casac::variant& field, const casac::variant& spw,
	int width,
	const ::casac::variant& baseline, const std::string& time,
	const ::casac::variant& scan, const ::casac::variant& uvrange,
	const std::string& taql, bool writesyscal,
	bool multisource, bool combinespw,
	bool writestation, bool padwithflags, bool overwrite
) {
	Bool rstat(true);
	try {
		if(!detached()) {
			unique_ptr<MeasurementSet> mssel(new MeasurementSet());
			Bool subselect=false;
			String fieldS(m1toBlankCStr_(field));
			String spwS(m1toBlankCStr_(spw));
			String baselineS=toCasaString(baseline);
			String timeS=toCasaString(time);
			String scanS=toCasaString(scan);
			String uvrangeS=toCasaString(uvrange);
			String taqlS=toCasaString(taql);
			Int inchan=1;
			Int istart=0;
			Int istep=1;
            if (width != 1) {
                *itsLog << LogIO::WARN << LogOrigin("ms", __func__)
                    << "The width parameter has been deprecated. Run mstransform "
                    << "prior to running ms.tofits() to select and average data."
                    << LogIO::POST;
            }
			Int iwidth = 1;
			if (spwS==String("")) {
				spwS="*";
			}
			Record selrec;
			try {
				selrec=itsMS->msseltoindex(spwS, fieldS);
			}
			catch (const AipsError& x) {
				Table::relinquishAutoLocks(true);
				*itsLog << LogOrigin("ms", "tofits")
				        << LogIO::SEVERE << x.getMesg() << LogIO::POST;
				RETHROW(x);
			}
			Vector<Int> fldids = selrec.asArrayInt("field");
			ThrowIf(
				! multisource && fldids.size() > 1,
				"If multisource is false, no more than one field should be specified"
				);
			Int fieldID = 0;
			if (! multisource && fldids.size() == 1) {
				fieldID = fldids[0];
			}
			Vector<Int> spwids = selrec.asArrayInt("spw");
			Matrix<Int> chansel=selrec.asArrayInt("channel");
			if (chansel.nelements() !=0) {
				istep=chansel.row(0)(3);
				if(istep < 1) {
					istep=1;
				}
				istart=chansel.row(0)(1);
				inchan=(chansel.row(0)(2)-istart+1)/istep;
				if (inchan<1) {
					inchan=1;
					istep=1;
				}
			}
			subselect = mssSetData(
				*itsMS, *mssel, "", timeS, baselineS, fieldS,
				spwS, uvrangeS, taqlS, "", scanS
				);

			if(subselect && mssel->nrow() < itsMS->nrow()) {
				if(mssel->nrow() == 0) {
					*itsLog << LogIO::WARN << LogOrigin("ms", __func__)
					        << "No data for selection: will convert full MeasurementSet"
					        << LogIO::POST;
					mssel.reset(new MeasurementSet(*itsMS));
				}
				else {
					*itsLog << LogOrigin("ms", __func__)
					        << "By selection " << itsMS->nrow()
					        <<  " rows to be converted are reduced to "
					        << mssel->nrow() << LogIO::POST;
				}
			}
			else {
				mssel.reset(new MeasurementSet(*itsMS));
			}
			MeasurementSet selms(*mssel);
			if (
				! MSFitsOutput::writeFitsFile(
					fitsfile, selms, column, istart, inchan,
					istep, writesyscal, multisource, combinespw,
					writestation, 1.0, padwithflags, iwidth,
					fieldID, overwrite
					)
				) {
				*itsLog << LogOrigin("ms", __func__)
				        << LogIO::SEVERE << "Conversion to FITS failed"<< LogIO::POST;
				rstat = false;
			}
		}
	}
	catch (const AipsError& x) {
		*itsLog << LogOrigin("ms", __func__)
		        << LogIO::SEVERE << "Exception Reported: "
		        << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return rstat;
}

msmetadata* ms::metadata(const float cachesize)
{
	try {
		if (detached()) {
			return 0;
		}
		return new msmetadata(itsMS, cachesize);
	}
	catch(const AipsError& x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
}

record* ms::summary(
	bool verbose, const string& listfile, bool listunfl,
	double cachesize, bool overwrite, bool wantreturn
) {
	if (detached()) {
		return 0;
	}
	record *header = 0;
	try {
		*itsLog << LogOrigin("ms", __func__);
		// pass the original MS name to the constructor
		// so that it is correctly printed in the output
		MSSummary mss(itsMS, itsOriginalMS->tableName(), cachesize);
		mss.setListUnflaggedRowCount(listunfl);
		casacore::Record outRec;
		if (! listfile.empty()){
			File diskfile(listfile);
			ThrowIf(
				diskfile.exists() && ! overwrite,
				"File: " + listfile + " already exists; delete "
				"it, choose another name, or set overwrite=true."
				);

			*itsLog << LogIO::NORMAL << "Writing output to file: "
			        << listfile << LogIO::POST;

			/* First, save output to a string so that LogMessages
			   and time stamps can be removed from it */
			ostringstream ostr;
			streambuf *osbuf, *backup;

			// Backup original cout buffer
			backup = cout.rdbuf();

			// Redirect cout's buffer to string
			osbuf = ostr.rdbuf();
			cout.rdbuf(osbuf);
			// Sink the messages locally to a string
			LogSink sink(LogMessage::NORMAL, &ostr, false);
			LogIO os(sink);
			// Call the listing routines
			mss.list(os, outRec, verbose, wantreturn);
            if (wantreturn) {
			    header = fromRecord(outRec);
            }
			// Restore cout's buffer
			cout.rdbuf(backup);
			String str(ostr.str());
            Int count = str.freq('\n') + 1;
            String *s = new String[count];
            static const Regex regx(".*\tINFO\t[+]?\t");
            casacore::split(str, s, count, "\n");
			ofstream file;
			file.open(listfile.data());
            for (Int i=0; i<count; ++i) {
			    // Remove the extra fields (time, severity) from the output string
			    // Write output string to a file
                file << s[i].after(regx);
                if (i < count - 1) {
                    file << "\n";
                }
            }
            delete [] s;
			file.close();
		}
		else {
			mss.list(*itsLog, outRec, verbose, wantreturn);
            if (wantreturn) {
			    header = fromRecord(outRec);
            }
		}
	}
	catch (const AipsError& x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return header;
}

::casac::record*
ms::getscansummary()
{
	::casac::record *scansummary = 0;
	try {
		if(!detached()){
			MSSummary mss(itsMS);
			casacore::Record outRec;
			mss.getScanSummary(outRec);
			scansummary=fromRecord(outRec);
		}
	}
	catch (const AipsError& x) {
		*itsLog << LogOrigin("ms", "getscansummary");
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return scansummary;
}

::casac::record*
ms::getspectralwindowinfo()
{
	::casac::record *spwSummary = nullptr;
	try {
		if(!detached()){
			//*itsLog << LogOrigin("ms", "summary");
			MSSummary mss(itsMS);
			casacore::Record outRec;
			mss.getSpectralWindowInfo(outRec);
			spwSummary=fromRecord(outRec);
		}
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return spwSummary;
}

variant*
ms::getfielddirmeas(
	const std::string& dircolname, int fieldid,
	double time, const string& format)
{
	variant *retval = 0;
	try{
		if(!detached()){
			*itsLog << LogOrigin("ms", "getfielddirmeas");
			String error;
			String colname(dircolname);
			colname.upcase();

			casacore::MSFieldColumns msfc(itsMS->field());
			casacore::MDirection d;
			if(colname=="DELAY_DIR"){
				d = msfc.delayDirMeas(fieldid, time);
			}
			else if(colname=="PHASE_DIR"){
				d = msfc.phaseDirMeas(fieldid, time);
			}
			else if(colname=="REFERENCE_DIR"){
				d = msfc.referenceDirMeas(fieldid, time);
			}
			else{
				*itsLog << LogIO::SEVERE
				        << "Illegal FIELD direction column name: " << dircolname
				        << LogIO::POST;
			}
			String f(format);
			f.downcase();
			if (f.startsWith("s")) {
				return new variant(String::toString(d));
			}
			MeasureHolder out(d);

			Record outRec;
			if (out.toRecord(error, outRec)) {
				retval = new variant(fromRecord(outRec));
			} else {
				error += String("Failed to generate direction return value.\n");
				*itsLog << LogIO::SEVERE << error << LogIO::POST;
			};
		}
	} catch (AipsError(x)) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	}

	return retval;
}

bool
ms::listhistory()
{
	Bool rstat(false);
	try {
		if(!detached()){
			*itsLog << LogOrigin("ms", "listhistory");
			MSSummary mss(*itsMS);
			mss.listHistory(*itsLog);
			rstat = true;
		}
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return rstat;
}

bool
ms::writehistory(const std::string& message, const std::string& parms, const std::string& origin, const std::string& msname, const std::string& app)
{
	Bool rstat(false);
	try {
		if (message.length() > 0 || parms.length() > 0) {
			MeasurementSet outMS;
			if (msname.length() > 0) {
				outMS = MeasurementSet(msname,TableLock::AutoLocking,Table::Update);
			} else {
				outMS = MeasurementSet(ms::name(),
				                       TableLock::AutoLocking,Table::Update);
			}
			// make sure the MS has a HISTORY table
			if(!(Table::isReadable(outMS.historyTableName()))){
				TableRecord &kws = outMS.rwKeywordSet();
				SetupNewTable historySetup(outMS.historyTableName(),
				                           MSHistory::requiredTableDesc(),Table::New);
				kws.defineTable(MS::keywordName(MS::HISTORY), Table(historySetup));
			}
			MSHistoryHandler::addMessage(outMS, message, app, parms, origin);
			rstat = true;
		}
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return rstat;
}

::casac::record*
ms::range(const std::vector<std::string>& items, const bool useflags, const int blocksize)
{
	::casac::record *retval(0);
	try {
		if(!detached()){
			MSRange msrange(*itsSel);
			msrange.setBlockSize(blocksize);
			retval = fromRecord(msrange.range(toVectorString(items), useflags, False));
		}
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(True);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(True);
	return retval;
}

::casac::record*
ms::range2(const std::vector<std::string>& items, const bool useflags, const int blocksize)
{
	*itsLog << LogOrigin("ms", "range");
	::casac::record *retval(0);
	try {
		if(!detached()){
			MSRange msrange(*itsSelectedMS);
			msrange.setBlockSize(blocksize);
			retval = fromRecord(msrange.range(toVectorString(items), useflags, false));
		}
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return retval;
}

template <typename T>
static void
append(Array<T> &data, unsigned &current_length,
       unsigned nrow,
       const Array<T> &data_chunk,
       const string &column)
{
	unsigned dimension = data_chunk.shape().nelements();

	if (data.nelements() == 0) {
		/* Initialize.
		   Allocate the full buffer at once,
		   because there does not seem to exist an efficient way
		   to expand an Array<T> chunk by chunk.
		   (like std::vector<>::push_back()).
		   Note that Array<T>::resize() takes linear time, it is inefficent
		   to use that in every iteration
		*/
		IPosition shape = data_chunk.shape();
		shape(dimension - 1) = nrow;
		data.resize(shape);
		current_length = 0;
	}

	if (dimension != data.shape().nelements()) {

		stringstream ss;
		ss << "Dimension of " << column << " values changed from " <<
			data.shape().nelements() << " to " << dimension;
		throw AipsError(ss.str());

	}

	/* Accumulate */
	if (dimension == 3) {
		for (unsigned i = 0; i < (unsigned) data_chunk.shape()(0); i++) {
			for (unsigned j = 0; j < (unsigned) data_chunk.shape()(1); j++) {
				for (unsigned k = 0; k < (unsigned) data_chunk.shape()(2); k++) {
					static_cast<Cube<T> >(data)(i, j, current_length+k) =
						static_cast<Cube<T> >(data_chunk)(i, j, k);
				}
			}
		}
	}
	else if (dimension == 2) {
		for (unsigned i = 0; i < (unsigned) data_chunk.shape()(0); i++) {
			for (unsigned j = 0; j < (unsigned) data_chunk.shape()(1); j++) {
				static_cast<Matrix<T> >(data)(i, current_length+j) =
					static_cast<Matrix<T> >(data_chunk)(i, j);
			}
		}
	}
	else if (dimension == 1) {
		for (unsigned i = 0; i < (unsigned) data_chunk.shape()(0); i++) {
			static_cast<Vector<T> >(data)(current_length+i) =
				static_cast<Vector<T> >(data_chunk)(i);
		}
	}
	else {
		stringstream ss;
		ss << "Unsupported dimension of " << column << ": " << dimension;
		throw AipsError(ss.str());
	}

	current_length += data_chunk.shape()(dimension - 1);
}


::casac::record*
ms::statistics(const std::string& column,
               const std::string& complex_value,
               const bool useflags,
               const std::string& spw,
               const std::string& field,
               const std::string& baseline,
               const std::string& uvrange,
               const std::string& time,
               const std::string& correlation,
               const std::string& scan,
               const std::string& array,
               const std::string& obs)
{
	*itsLog << LogOrigin("ms", "statistics");

	::casac::record *retval(0);
	try {
		if(!detached()){

			/* This tools built-in itsSel is of type
			   MSSelector.
			   That is something completely different than
			   MSSelection.
			*/
			const String dummyExpr = String("");

			if (0) cerr << "selection: " << endl <<
				       "time = " << time << endl <<
				       "baseline = " << baseline << endl <<
				       "field = " << field << endl <<
				       "spw = " << spw << endl <<
				       "uvrange = " << uvrange << endl <<
				       "correlation = " << correlation << endl <<
				       "scan = " << scan << endl <<
				       "array = " << array <<
				       "obs = " << obs << endl;

			MSSelection mssel(*itsMS,
			                  MSSelection::PARSE_NOW,
			                  time,
			                  baseline,
			                  field,
			                  spw,
			                  uvrange,
			                  dummyExpr,   // taqlExpr
			                  correlation,
			                  scan,
			                  array,
			                  "",       // stateExpr
			                  obs);

			MeasurementSet *sel_p;
			MeasurementSet sel;
			if (mssel.getSelectedMS(sel)) {
				/* It is undocumented, but
				   getSelectedMS() seems to return true
				   if there's a non-trivial selection.
				   If it returns false, the output MS is null.
				*/

				sel_p = &sel;
				if (0) cout << "Got the subset MS!" << endl;
			}
			else {
				sel_p = itsMS;
			}

			// Be more flexible about column, i.e.
			// "MODEL_DATA"}
			// "model"     } => "MODEL",
			// "antenna2"    => "ANTENNA2"
			String mycolumn(upcase(column).before("_DATA"));

			*itsLog << "Use " << itsMS->tableName() <<
				", useflags = " << useflags << LogIO::POST;
			*itsLog << "Compute statistics on " << mycolumn;

			if (complex_value != "") {
				*itsLog << ", use " << complex_value;
			}
			*itsLog << "..." << LogIO::POST;



			Block<Int> sortColumns;
			ROVisIterator vi(*sel_p, sortColumns, 0.0);

			unsigned nrow = sel_p->nrow();

			VisBuffer vb(vi);

			/* Apply selection */
			Vector<Vector<Slice> > chanSlices;
			Vector<Vector<Slice> > corrSlices;
			mssel.getChanSlices(chanSlices, itsMS);
			mssel.getCorrSlices(corrSlices, itsMS);
			vi.selectChannel(chanSlices);
			vi.selectCorrelation(corrSlices);

			/* Now loop over the column, collect the data and compute statistics.
			   This is somewhat involved because:
			   - each column has its own accessor function
			   - each column has its own type
			   - each column has its own dimension

			   In the end, all collected values are linearized and
			   converted to float.
			*/
			Array<Complex> data_complex;
			Array<Double> data_double;
			Array<Float> data_float;
			Array<Bool> data_bool;
			Array<Int> data_int;
			unsigned length;  // logical length of data array

			Vector<Bool> flagrows;
			Cube<Bool> flags;
			unsigned flagrows_length;
			unsigned flags_length;

			for (vi.originChunks();
			     vi.moreChunks();
			     vi.nextChunk()) {

				for (vi.origin();
				     vi.more();
				     vi++) {

					Array<Complex> data_complex_chunk;
					Array<Double> data_double_chunk;
					Array<Float> data_float_chunk;
					Array<Bool> data_bool_chunk;
					Array<Int> data_int_chunk;

					if (useflags) {
						Cube<Bool> flag_chunk;
						vi.flag(static_cast<Cube<Bool>&>(flag_chunk));

						Vector<Bool> flagrow_chunk;
						vi.flagRow(static_cast<Vector<Bool>&>(flagrow_chunk));

						/* If FLAG_ROW is set, update flags */
						for (unsigned i = 0; i < flagrow_chunk.nelements(); i++) {
							if (flagrow_chunk(i))
								flag_chunk.xyPlane(i).set(true);
						}

						append<Bool>(flags, flags_length, nrow, flag_chunk, "FLAG");
						append<Bool>(flagrows, flagrows_length, nrow, flagrow_chunk,
						             "FLAG_ROW");
					}

					if (mycolumn == "DATA" || mycolumn == "CORRECTED" || mycolumn == "MODEL") {
						ROVisibilityIterator::DataColumn dc;
						if(mycolumn == "DATA"){
							dc = ROVisibilityIterator::Observed;
							if(vi.msColumns().data().isNull()){
								throw(AipsError("Data column is not present"));
							}
						}
						else if(mycolumn == "CORRECTED"){
							dc = ROVisibilityIterator::Corrected;
							if(vi.msColumns().correctedData().isNull()){
								throw(AipsError("Corrected Data column is not present"));
							}
						}
						else{
							dc = ROVisibilityIterator::Model;
							if(vi.msColumns().modelData().isNull()){
								throw(AipsError("Model Data column is not present"));
							}
						}
						vi.visibility(static_cast<Cube<Complex>&>(data_complex_chunk),
						              dc);

						append<Complex>(data_complex, length, nrow, data_complex_chunk,
						                mycolumn);
					}
					else if (mycolumn == "UVW") {
						Vector<RigidVector<Double, 3> > uvw;
						vi.uvw(uvw);

						data_double_chunk.resize(IPosition(2, 3, uvw.nelements()));
						for (unsigned i = 0; i < uvw.nelements(); i++) {
							static_cast<Matrix<Double> >(data_double_chunk)(0, i) = uvw(i)(0);
							static_cast<Matrix<Double> >(data_double_chunk)(1, i) = uvw(i)(1);
							static_cast<Matrix<Double> >(data_double_chunk)(2, i) = uvw(i)(2);
						}
						append<Double>(data_double, length, nrow, data_double_chunk, mycolumn);
					}
					else if (mycolumn == "UVRANGE") {
						Vector<RigidVector<Double, 3> > uvw;
						vi.uvw(uvw);

						data_double_chunk.resize(IPosition(1, uvw.nelements()));
						for (unsigned i = 0; i < uvw.nelements(); i++) {
							static_cast<Vector<Double> >(data_double_chunk)(i) =
								sqrt( uvw(i)(0)*uvw(i)(0) + uvw(i)(1)*uvw(i)(1) );
						}
						append<Double>(data_double, length, nrow, data_double_chunk, mycolumn);
					}
					else if (mycolumn == "FLAG") {
						vi.flag(static_cast<Cube<Bool>&>(data_bool_chunk));
						append<Bool>(data_bool, length, nrow, data_bool_chunk, mycolumn);
					}
					else if (mycolumn == "WEIGHT") {
						vi.weightMat(static_cast<Matrix<Float>&>(data_float_chunk));
						append<Float>(data_float, length, nrow, data_float_chunk, mycolumn);
					}
					else if (mycolumn == "SIGMA") {
						vi.sigmaMat(static_cast<Matrix<Float>&>(data_float_chunk));
						append<Float>(data_float, length, nrow, data_float_chunk, mycolumn);
					}
					else if (mycolumn == "ANTENNA1") {
						vi.antenna1(static_cast<Vector<Int>&>(data_int_chunk));
						append<Int>(data_int, length, nrow, data_int_chunk, mycolumn);
					}
					else if (mycolumn == "ANTENNA2") {
						vi.antenna2(static_cast<Vector<Int>&>(data_int_chunk));
						append<Int>(data_int, length, nrow, data_int_chunk, mycolumn);
					}
					else if (mycolumn == "FEED1") {
						vi.feed1(static_cast<Vector<Int>&>(data_int_chunk));
						append<Int>(data_int, length, nrow, data_int_chunk, mycolumn);
					}
					else if (mycolumn == "FEED2") {
						vi.feed2(static_cast<Vector<Int>&>(data_int_chunk));
						append<Int>(data_int, length, nrow, data_int_chunk, mycolumn);
					}
					else if (mycolumn == "FIELD_ID") {
						data_int_chunk.resize(IPosition(1, 1));
						data_int_chunk(IPosition(1, 0)) = vi.fieldId();
						append<Int>(data_int, length, nrow, data_int_chunk, mycolumn);
					}
					else if (mycolumn == "ARRAY_ID") {
						data_int_chunk.resize(IPosition(1, 1));
						data_int_chunk(IPosition(1, 0)) = vi.arrayId();
						append<Int>(data_int, length, nrow, data_int_chunk, mycolumn);
					}
					else if (mycolumn == "DATA_DESC_ID") {
						data_int_chunk.resize(IPosition(1, 1));
						data_int_chunk(IPosition(1, 0)) = vi.dataDescriptionId();
						append<Int>(data_int, length, nrow, data_int_chunk, mycolumn);
					}
					else if (mycolumn == "FLAG_ROW") {
						vi.flagRow(static_cast<Vector<Bool>&>(data_bool_chunk));
						append<Bool>(data_bool, length, nrow, data_bool_chunk, mycolumn);
					}
					else if (mycolumn == "INTERVAL") {
						vi.timeInterval(static_cast<Vector<Double>&>(data_double_chunk));
						append<Double>(data_double, length, nrow, data_double_chunk, mycolumn);
					}
					else if (mycolumn == "SCAN_NUMBER" || mycolumn == "SCAN") {
						vi.scan(static_cast<Vector<Int>&>(data_int_chunk));
						append<Int>(data_int, length, nrow, data_int_chunk, mycolumn);
					}
					else if (mycolumn == "TIME") {
						vi.time(static_cast<Vector<Double>&>(data_double_chunk));
						append<Double>(data_double, length, nrow, data_double_chunk, mycolumn);
					}
					else if (mycolumn == "WEIGHT_SPECTRUM") {
						vi.weightSpectrum(static_cast<Cube<Float>&>(data_float_chunk));
						append<Float>(data_float, length, nrow, data_float_chunk, mycolumn);
					}
					else {
						stringstream ss;
						ss << "Unsupported column name: " << column;
						throw AipsError(ss.str());
					}

				}
			}


			unsigned dimension;
			unsigned n;
			if (data_complex.nelements() > 0) {
				dimension = data_complex.shape().nelements();
				n = data_complex.shape().product();
			}
			else if (data_double.nelements() > 0) {
				dimension = data_double.shape().nelements();
				n = data_double.shape().product();
			}
			else if (data_float.nelements() > 0) {
				dimension = data_float.shape().nelements();
				n = data_float.shape().product();
			}
			else if (data_bool.nelements() > 0) {
				dimension = data_bool.shape().nelements();
				n = data_bool.shape().product();
			}
			else if (data_int.nelements() > 0) {
				dimension = data_int.shape().nelements();
				n = data_int.shape().product();
			}
			else {
				throw AipsError("No data could be found!");
			}

			Vector<Bool> f;
			if (useflags) {
				if (dimension == 1) {
					f = flagrows;
				}
				else if (dimension == 2) {
					f = flagrows;
				}
				else if (dimension == 3) {
					f = flags.reform(IPosition(1, n));
				}
				else {
					stringstream ss;
					ss << "Unsupported column name: " << column;
					throw AipsError(ss.str());
				}
			}
			else {
				f = Vector<Bool>(n, false);
			}

			bool supported = true;
			if (data_complex.nelements() > 0) {

				Vector<Complex> v(data_complex.reform(IPosition(1, data_complex.shape().product())));
				retval = fromRecord(Statistics<Complex>::get_stats_complex(v, f,
				                                                           mycolumn,
				                                                           supported,
				                                                           complex_value));
			}
			else if (data_double.nelements() > 0) {
				if (dimension == 2) {
					f.resize(0);
					f = Vector<Bool>(data_double.shape()(1), false);
					retval = fromRecord(Statistics<Double>::get_stats_array(static_cast<Matrix<Double> >(data_double),
					                                                        f,
					                                                        mycolumn,
					                                                        supported));

				}
				else {
					retval = fromRecord(Statistics<Double>::get_stats(data_double.reform(IPosition(1, data_double.shape().product())),
					                                                  f,
					                                                  mycolumn,
					                                                  supported));
				}
			}
			else if (data_bool.nelements() > 0) {
				retval = fromRecord(Statistics<Bool>::get_stats(data_bool.reform(IPosition(1, data_bool.shape().product())),
				                                                f,
				                                                mycolumn,
				                                                supported));
			}
			else if (data_float.nelements() > 0) {
				if (dimension == 2) {
					f.resize(0);
					f = Vector<Bool>(data_float.shape()(1), false);
					retval = fromRecord(Statistics<Float>::get_stats_array(static_cast<Matrix<Float> >(data_float),
					                                                       f,
					                                                       mycolumn,
					                                                       supported));
				}
				else {
					retval = fromRecord(Statistics<Float>::get_stats(data_float.reform(IPosition(1, data_float.shape().product())),
					                                                 f,
					                                                 mycolumn,
					                                                 supported));
				}
			}
			else if (data_int.nelements() > 0) {
				retval = fromRecord(Statistics<Int>::get_stats(data_int.reform(IPosition(1, data_int.shape().product())),
				                                               f,
				                                               mycolumn,
				                                               supported));
			}
			else {
				throw AipsError("No data could be found!");
			}
		} // end if !detached
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return retval;
}

// Create key value for statistics reporting axis
//
static string
mkKey(int id, const vi::VisBuffer2 *vb)
{
	string result;

	switch (id) {
	case MSMainEnums::PredefinedColumns::ARRAY_ID:
		result = "ARRAY_ID=" + to_string(vb->arrayId()[0]);
		break;
	case MSMainEnums::PredefinedColumns::FIELD_ID:
		result = "FIELD_ID=" + to_string(vb->fieldId()[0]);
		break;
	case MSMainEnums::PredefinedColumns::DATA_DESC_ID:
		result = "DATA_DESC_ID=" + to_string(vb->dataDescriptionIds()[0]);
		break;
	case MSMainEnums::PredefinedColumns::TIME:
		result = "TIME=" + to_string(vb->time()[0] - vb->timeInterval()[0] / 2);
		break;
	case MSMainEnums::PredefinedColumns::SCAN_NUMBER:
		result = "SCAN_NUMBER=" + to_string(vb->scan()[0]);
		break;
	case MSMainEnums::PredefinedColumns::STATE_ID:
		result = "STATE_ID=" + to_string(vb->stateId()[0]);
		break;
	default:
		assert(false);
		break;
	}
	return result;
}

// Class used by doStatistics to accumulate the statistics for each chunk
// provided by a VisibilityIterator2 instance (through a Vi2ChunkDataProvider
// instance.)
template <class A, class D, class W, class M>
class ChunkStatisticsAccumulator
	: public Vi2ChunkStatisticsIteratee<D,W,M>
{
	std::map<double, A> quantileToValue;
	const double quartile = 0.75;
	std::set<double> quantiles = {quartile};

	Record &acc;
	vector<Int> &sortColumnIds;
	bool hideTimeAxis;

public:
	ChunkStatisticsAccumulator(Record &acc, vector<Int> &sortColumnIds, 
	                           bool hideTimeAxis)
		: acc(acc)
		, sortColumnIds(sortColumnIds)
		, hideTimeAxis(hideTimeAxis) {};

	void nextChunk(StatisticsAlgorithm<A,D,M,W> &statistics, const vi::VisBuffer2 *vb) {
		string keyvals;
		string delim;
		for (auto const & id : sortColumnIds) {
			if (!(id == MSMainEnums::PredefinedColumns::TIME
			      && hideTimeAxis)) {
				keyvals += delim + mkKey(id, vb);
				delim = ",";
			}
		}
		Record stats = toRecord(statistics.getStatistics());

		// Compute some quantiles
		quantileToValue.clear();
		A median = statistics.getMedianAndQuantiles(quantileToValue, quantiles);
		stats.define("median", median);
		stats.define("quartile", quantileToValue[quartile]);
		A medianAbsDevMed = statistics.getMedianAbsDevMed();
		stats.define("medabsdevmed", medianAbsDevMed);

		// Record statistics, associated with key
		acc.defineRecord(keyvals, stats);
	}
};

// Compute statistics using a given DataProvider, using iteration over vi2
// chunks to implement reporting axes. The Statistics template parameter may be
// any StatisticsAlgorithm class, although statistics2 always uses
// ClassicalStatistics.
//
// Note that the format of the returned record has not been finalized, and may
// change.
template <class DataProvider,
          template <class A, class D, class M, class W> class Statistics>
static ::casac::record *
doStatistics(
	vector<Int> &sortColumnIds,
	bool hideTimeAxis,
	DataProvider *dataProvider)
{
	Record result;
	std::unique_ptr<DataProvider> dp(dataProvider);
	Statistics<typename DataProvider::AccumType,
	           typename DataProvider::DataIteratorType,
	           typename DataProvider::MaskIteratorType,
	           typename DataProvider::WeightsIteratorType>
		statistics;

	ChunkStatisticsAccumulator<typename DataProvider::AccumType,
	                           typename DataProvider::DataIteratorType,
	                           typename DataProvider::WeightsIteratorType,
	                           typename DataProvider::MaskIteratorType>
		accumulateChunkStatistics(result, sortColumnIds, hideTimeAxis);

	dp->foreachChunk(statistics, accumulateChunkStatistics);
	return fromRecord(result);
}

// Thin wrapper over doStatistics, provided because statistics2 requires
// ClassicalStatistics.
template <class DataProvider>
static ::casac::record *
doClassicalStatistics(
	vector<Int> &sortColumnIds,
	bool hideTimeAxis,
	DataProvider *dataProvider)
{
	return doStatistics<DataProvider,ClassicalStatistics>(
		sortColumnIds, hideTimeAxis, dataProvider);
}

// Convert string provided as a statistics "reporting axis" to MS column id.
static Int
reportingAxisId(const string &axis)
{
	if (axis == "ddid") return MSMainEnums::PredefinedColumns::DATA_DESC_ID;
	if (axis == "field") return MSMainEnums::PredefinedColumns::FIELD_ID;
	if (axis == "integration") return MSMainEnums::PredefinedColumns::TIME;
	if (axis == "array") return MSMainEnums::PredefinedColumns::ARRAY_ID;
	if (axis == "scan") return MSMainEnums::PredefinedColumns::SCAN_NUMBER;
	if (axis == "subscan") return MSMainEnums::PredefinedColumns::STATE_ID;
	return -1;
}

// Convert string of comma-separated values provided as statistics "reporting
// axes" to vector of MS column ids.
static vector<Int>
reportingAxisIds(const string &s)
{
	// convert string s to vector of sort column ids
	vector<Int> result;
	size_t init = 0;
	size_t sep = s.find(',', init);
	while (sep != string::npos) {
		Int colId = reportingAxisId(s.substr(init, sep - init));
		if (colId >= 0)
			result.push_back(colId);
		init = sep + 1;
		sep = s.find(',', init);
	}
	Int colId = reportingAxisId(s.substr(init, s.length() - init));
	if (colId >= 0)
		result.push_back(colId);
	return result;
}

// Parse "timespan" string provided to statistics2 to determine whether
// statistics should span scans or subscans. The input string is expected to be
// composed of the tokens "scan" or "state", separated by commas.
static void
timespanBoundaries(const string &s, bool &spanScan, bool &spanSubscan)
{
	// search for 'scan' and 'subscan' in string s
	spanScan = false;
	spanSubscan = false;
	size_t init = 0;
	size_t sep = s.find(',', init);
	while (sep != string::npos) {
		string token = s.substr(init, sep - init);
		if (token == "scan") spanScan = true;
		else if (token == "state") spanSubscan = true;
		init = sep + 1;
		sep = s.find(',', init);
	}
	string token = s.substr(init, s.length() - init);
	if (token == "scan") spanScan = true;
	else if (token == "state") spanSubscan = true;
}

//
// Compute statistics on values derived from an MS column. Many options are
// supported, controlled by the following arguments:
//
// * column: the MS column name string
// * complex_value: string to select value derived from a complex visibility on
//                  which to compute statistics, "amplitude", "amp", "phase",
//                  "real", "imaginary", "imag" (does not apply to any data but
//                  visibilities)
// * useflags: whether to omit flagged data from the sample used to compute
//             statistics
// * useweights: whether to do weighted statistics, using visibility weights
//               (does not apply to any data but visibilities)
// * spw: spectral window selection
// * field: field selection
// * baseline: baseline selection
// * uvrange: uvrange selection
// * time: time selection
// * correlation: correlation selection
// * scan: scan selection
// * array: array selection
// * obs: obs selection
// * reportingaxes: comma separated string to select axes along which statistics
//                  are reported
// * timeaverage: whether to do time averaging
// * timebin: time averaging interval
// * timespan: whether time averaging crosses span or subscan boundaries; value
//             is a string consisting of "scan" and/or "state", separated by
//             commas
// * maxuvwdistance: Maximum separation of start-to-end baselines that can be
//                   included in an average
//
// TODO: how to handle WEIGHT, SIGMA and UVW columns?
//
::casac::record*
ms::statistics2(const std::string& column,
                const std::string& complex_value,
                bool useflags,
                bool useweights,
                const std::string& spw,
                const std::string& field,
//                const std::string& feed,
                const std::string& baseline,
                const std::string& uvrange,
                const std::string& time,
                const std::string& correlation,
                const std::string& scan,
                const std::string& intent,
                const std::string& array,
                const std::string& obs,
                const std::string& reportingaxes,
                bool timeaverage,
                const std::string& timebin,
                const std::string& timespan,
                double maxuvwdistance)
{

	// const std::array<Int,6> validSortColumnIds = {
	//  MSMainEnums::PredefinedColumns::ARRAY_ID,
	//  MSMainEnums::PredefinedColumns::FIELD_ID,
	//  MSMainEnums::PredefinedColumns::DATA_DESC_ID,
	//  MSMainEnums::PredefinedColumns::SCAN_NUMBER,
	//  MSMainEnums::PredefinedColumns::STATE_ID,
	//  MSMainEnums::PredefinedColumns::TIME
	// };

	*itsLog << LogOrigin("ms", "statistics2");

	::casac::record *retval(0);
	try {
		if (!detached()) {

			/* This tools built-in itsSel is of type
			   MSSelector.
			   That is something completely different than
			   MSSelection.
			*/
			const String dummyExpr = String("");

			if (0) cerr << "selection: " << endl <<
				       "time = " << time << endl <<
				       "baseline = " << baseline << endl <<
				       "field = " << field << endl <<
//				       "feed= " << feed << endl <<
				       "spw = " << spw << endl <<
				       "uvrange = " << uvrange << endl <<
				       "correlation = " << correlation << endl <<
				       "scan = " << scan << endl <<
				       "intent= " << intent << endl <<
				       "array = " << array << endl <<
				       "obs = " << obs << endl <<
				       "reportingaxes =" << reportingaxes << endl <<
				       "timeaverage = " << timeaverage << endl <<
				       "timebin = " << timebin << endl <<
				       "timespan = " << timespan << endl;

			MSSelection mssel(*itsMS,
			                  MSSelection::PARSE_NOW,
			                  time,
			                  baseline,
			                  field,
			                  spw,
			                  uvrange,
			                  dummyExpr,   // taqlExpr
			                  correlation,
			                  scan,
			                  array,
			                  intent,
			                  obs);
				         // , feed);

			MeasurementSet *sel_p;
			MeasurementSet sel;
			if (mssel.getSelectedMS(sel)) {
				/* It is undocumented, but
				   getSelectedMS() seems to return true
				   if there's a non-trivial selection.
				   If it returns false, the output MS is null.
				*/

				sel_p = &sel;
				if (0) cout << "Got the subset MS!" << endl;
			}
			else {
				sel_p = itsMS;
			}

			// Be more flexible about column, i.e.
			// "MODEL_DATA"}
			// "model"     } => "MODEL",
			// "antenna2"    => "ANTENNA2"
			String mycolumn(upcase(column).before("_DATA"));

			*itsLog << "Use " << itsMS->tableName() <<
				", useflags = " << useflags <<
				", useweights = " << useweights << LogIO::POST;
			*itsLog << "Compute statistics on " << mycolumn;

			if (complex_value != "") {
				*itsLog << ", use " << complex_value;
			}
			*itsLog << "..." << LogIO::POST;

			// NB: The effect of the "timeInterval" argument to the
			// VisibilityIterator2 constructor is complex, and the
			// VisibilityIterator2 source code documentation is inconsistent. It
			// appears that timeInterval=0 groups all times into one chunk, not
			// each time into its own iterator chunk. For this reason, when the
			// user specifies "integration" as a statistics reporting axis, we
			// will use a small strictly positive value for timeInterval to
			// force every integration into its own chunk.
			const Double positiveButShorterThanEveryIntegrationSec = 1.0e-4;
			const Double allTimesInOneChunkSec = 0;
			Double chunkInterval;
			Double averagingInterval = 0;
			vector<Int> sortColumnIds = reportingAxisIds(reportingaxes);
			bool hideTimeAxis = false;

			// Set chunkInterval and modify sortColumnIds to support the call to
			// doStatistics.
			vector<Int>::const_iterator endIter = sortColumnIds.cend();
			if (find(sortColumnIds.cbegin(), endIter,
			         MSMainEnums::PredefinedColumns::TIME) == endIter) {
				chunkInterval = allTimesInOneChunkSec;
				sortColumnIds.push_back(
					MSMainEnums::PredefinedColumns::TIME);
				// user didn't ask for time axis, so we don't want it presented
				// in the results
				hideTimeAxis = true;
			} else {
				chunkInterval = positiveButShorterThanEveryIntegrationSec;
			}
			if (timeaverage) {
				hideTimeAxis = false;
				averagingInterval =
					casaQuantity(timebin).get("s").getValue();
				// remove TIME from sortColumnIds and determine chunkInterval
				auto endIter = sortColumnIds.end();
				auto timeColIter = find(
					sortColumnIds.begin(),
					endIter,
					MSMainEnums::PredefinedColumns::TIME);
				if (timeColIter != endIter) {
					sortColumnIds.erase(timeColIter);
					chunkInterval = averagingInterval;
				} else {
					chunkInterval = allTimesInOneChunkSec;
				}

				bool spanScan;
				bool spanSubscan;
				timespanBoundaries(timespan, spanScan, spanSubscan);

				if (!spanScan) {
					// append SCAN_NUMBER to sort columns, if not present
					endIter = sortColumnIds.end();
					auto scanColIter = find(
						sortColumnIds.begin(),
						endIter,
						MSMainEnums::PredefinedColumns::SCAN_NUMBER);
					if (scanColIter == endIter)
						sortColumnIds.push_back(
							MSMainEnums::PredefinedColumns::SCAN_NUMBER);
				}

				if (!spanSubscan) {
					// append STATE_ID to sort columns, if not present
					endIter = sortColumnIds.end();
					auto stateColIter = find(
						sortColumnIds.begin(),
						endIter,
						MSMainEnums::PredefinedColumns::STATE_ID);
					if (stateColIter == endIter)
						sortColumnIds.push_back(
							MSMainEnums::PredefinedColumns::STATE_ID);
				}
				// append TIME to sortColumnIds
				sortColumnIds.push_back(MSMainEnums::PredefinedColumns::TIME);
			}

			// create the vi2 instance
			auto sortColumnIdsData = sortColumnIds.data();
			Block<Int> sortColumnsBlock(
				sortColumnIds.size(), sortColumnIdsData, false);
			vi::SortColumns sortColumns(sortColumnsBlock, false);
			vi::VisibilityIterator2 *vi2;

			if (!timeaverage) {
				vi2 = new vi::VisibilityIterator2(
					*sel_p, sortColumns, false, 0, chunkInterval);
			} else if (!(mycolumn == "DATA" || mycolumn == "CORRECTED" ||
			             mycolumn == "MODEL" || mycolumn == "FLOAT")) {
				stringstream ss;
				ss << "Time averaging of '" << mycolumn
				   << "' is not supported";
				throw AipsError(ss.str());
			} else {
				// To use AveragingVi2Factory, we must decide how to apply
				// weights and flags upon construction. After doing that, we set
				// the useweights and useflags variables to false, as the
				// statistics framework classes should not need to handle
				// weights and flags.
				int options = vi::AveragingOptions::Nothing;
				if (mycolumn == "DATA") {
					options = vi::AveragingOptions::AverageObserved;
					if (useweights) {
						if (useflags)
							options |= vi::AveragingOptions::ObservedFlagWeightAvgFromSIGMA;
						else
							options |= vi::AveragingOptions::ObservedWeightAvgFromSIGMA;
					} else {
						if (useflags)
							options |= vi::AveragingOptions::ObservedFlagAvg;
						else
							options |= vi::AveragingOptions::ObservedPlainAvg;
					}
					useweights = false;
					useflags = false;
				} else if (mycolumn == "CORRECTED") {
					options = vi::AveragingOptions::AverageCorrected;
					if (useweights) {
						if (useflags)
							options |= vi::AveragingOptions::CorrectedFlagWeightAvgFromWEIGHT;
						else
							options |= vi::AveragingOptions::CorrectedWeightAvgFromWEIGHT;
					} else {
						if (useflags)
							options |= vi::AveragingOptions::CorrectedFlagAvg;
						else
							options |= vi::AveragingOptions::CorrectedPlainAvg;
					}
					useweights = false;
					useflags = false;
				} else if (mycolumn == "MODEL") {
					options = vi::AveragingOptions::AverageModel;
					if (useweights) {
						bool hasCorrected = sel_p->isColumn(
							MSMainEnums::PredefinedColumns::CORRECTED_DATA);
						bool hasObserved = sel_p->isColumn(
							MSMainEnums::PredefinedColumns::DATA);
						if (useflags) {
							if (hasCorrected)
								options |= vi::AveragingOptions::ModelFlagWeightAvgFromWEIGHT;
							else if (hasObserved)
								options |= vi::AveragingOptions::ModelFlagWeightAvgFromSIGMA;
							else
								options |= vi::AveragingOptions::ModelFlagAvg;
						} else {
							if (hasCorrected)
								options |= vi::AveragingOptions::ModelWeightAvgFromWEIGHT;
							else if (hasObserved)
								options |= vi::AveragingOptions::ModelWeightAvgFromSIGMA;
							else
								options |= vi::AveragingOptions::ModelPlainAvg;
						}
					} else {
						if (useflags)
							options |= vi::AveragingOptions::ModelFlagAvg;
						else
							options |= vi::AveragingOptions::ModelPlainAvg;
					}
					useweights = false;
					useflags = false;
				} else if (mycolumn == "FLOAT") {
					options = vi::AveragingOptions::AverageFloat;
				}
				vi::AveragingParameters params(averagingInterval, chunkInterval,
				                               sortColumns, options,
				                               maxuvwdistance);
				vi::AveragingVi2Factory factory(params, sel_p);
				vi2 = new vi::VisibilityIterator2(factory);
			}

			/* Apply selection */
			vi::FrequencySelectionUsingChannels freqSelection;
			freqSelection.add(mssel, itsMS);
			vi2->setFrequencySelection(freqSelection);

			// Construct an instance of a data provider for the requested
			// column, and call doStatistics() with it.
			//
			// Most of the remaining code in this function effectively acts as a
			// lookup table to get an instance of the appropriate sub-class of
			// Vi2ChunkDataProvider for the requested MS column (and data
			// transformation for visibilities), followed by a call to
			// doStatistics().
			if (mycolumn == "DATA") {
				if (complex_value == "amplitude" || complex_value == "amp")
					retval = doClassicalStatistics(
						sortColumnIds,
						hideTimeAxis,
						new Vi2ChunkObservedVisAmplitudeProvider(
							vi2, useflags, useweights));

				else if (complex_value == "phase")
					retval = doClassicalStatistics(
						sortColumnIds,
						hideTimeAxis,
						new Vi2ChunkObservedVisPhaseProvider(
							vi2, useflags, useweights));

				else if (complex_value == "imaginary" || complex_value == "imag")
					retval = doClassicalStatistics(
						sortColumnIds,
						hideTimeAxis,
						new Vi2ChunkObservedVisImaginaryProvider(
							vi2, useflags, useweights));

				else if (complex_value == "real")
					retval = doClassicalStatistics(
						sortColumnIds,
						hideTimeAxis,
						new Vi2ChunkObservedVisRealProvider(
							vi2, useflags, useweights));

			} else if (mycolumn == "CORRECTED") {
				if (complex_value == "amplitude" || complex_value == "amp")
					retval = doClassicalStatistics(
						sortColumnIds,
						hideTimeAxis,
						new Vi2ChunkCorrectedVisAmplitudeProvider(
							vi2, useflags, useweights));

				else if (complex_value == "phase")
					retval = doClassicalStatistics(
						sortColumnIds,
						hideTimeAxis,
						new Vi2ChunkCorrectedVisPhaseProvider(
							vi2, useflags, useweights));

				else if (complex_value == "imaginary" || complex_value == "imag")
					retval = doClassicalStatistics(
						sortColumnIds,
						hideTimeAxis,
						new Vi2ChunkCorrectedVisImaginaryProvider(
							vi2, useflags, useweights));

				else if (complex_value == "real")
					retval = doClassicalStatistics(
						sortColumnIds,
						hideTimeAxis,
						new Vi2ChunkCorrectedVisRealProvider(
							vi2, useflags, useweights));

			} else if (mycolumn == "MODEL") {
				if (complex_value == "amplitude" || complex_value == "amp")
					retval = doClassicalStatistics(
						sortColumnIds,
						hideTimeAxis,
						new Vi2ChunkModelVisAmplitudeProvider(
							vi2, useflags, useweights));

				else if (complex_value == "phase")
					retval = doClassicalStatistics(
						sortColumnIds,
						hideTimeAxis,
						new Vi2ChunkModelVisPhaseProvider(
							vi2, useflags, useweights));

				else if (complex_value == "imaginary" || complex_value == "imag")
					retval = doClassicalStatistics(
						sortColumnIds,
						hideTimeAxis,
						new Vi2ChunkModelVisImaginaryProvider(
							vi2, useflags, useweights));

				else if (complex_value == "real")
					retval = doClassicalStatistics(
						sortColumnIds,
						hideTimeAxis,
						new Vi2ChunkModelVisRealProvider(
							vi2, useflags, useweights));

			} else if (mycolumn == "FLOAT") {
				retval = doClassicalStatistics(
					sortColumnIds,
					hideTimeAxis,
					new Vi2ChunkFloatVisDataProvider(
						vi2, useflags, useweights));
				// } else if (mycolumn == "UVW") {

			} else if (mycolumn == "UVRANGE") {
				retval = doClassicalStatistics(
					sortColumnIds,
					hideTimeAxis,
					new Vi2ChunkUVRangeDataProvider(vi2, useflags));

			} else if (mycolumn == "FLAG") {
				retval = doClassicalStatistics(
					sortColumnIds,
					hideTimeAxis,
					new Vi2ChunkFlagCubeDataProvider(vi2, useflags));
				// } else if (mycolumn == "WEIGHT") {
				// } else if (mycolumn == "SIGMA") {

			} else if (mycolumn == "ANTENNA1") {
				retval = doClassicalStatistics(
					sortColumnIds,
					hideTimeAxis,
					new Vi2ChunkAntenna1DataProvider(vi2, useflags));

			} else if (mycolumn == "ANTENNA2") {
				retval = doClassicalStatistics(
					sortColumnIds,
					hideTimeAxis,
					new Vi2ChunkAntenna2DataProvider(vi2, useflags));

			} else if (mycolumn == "FEED1") {
				retval = doClassicalStatistics(
					sortColumnIds,
					hideTimeAxis,
					new Vi2ChunkFeed1DataProvider(vi2, useflags));

			} else if (mycolumn == "FEED2") {
				retval = doClassicalStatistics(
					sortColumnIds,
					hideTimeAxis,
					new Vi2ChunkFeed2DataProvider(vi2, useflags));

			} else if (mycolumn == "FIELD_ID") {
				retval = doClassicalStatistics(
					sortColumnIds,
					hideTimeAxis,
					new Vi2ChunkFieldIdDataProvider(vi2, useflags));

			} else if (mycolumn == "ARRAY_ID") {
				retval = doClassicalStatistics(
					sortColumnIds,
					hideTimeAxis,
					new Vi2ChunkArrayIdDataProvider(vi2, useflags));

			} else if (mycolumn == "DATA_DESC_ID") {
				retval = doClassicalStatistics(
					sortColumnIds,
					hideTimeAxis,
					new Vi2ChunkDataDescriptionIdsDataProvider(vi2, useflags));

			} else if (mycolumn == "FLAG_ROW") {
				retval = doClassicalStatistics(
					sortColumnIds,
					hideTimeAxis,
					new Vi2ChunkFlagRowDataProvider(vi2, useflags));

			} else if (mycolumn == "INTERVAL") {
				retval = doClassicalStatistics(
					sortColumnIds,
					hideTimeAxis,
					new Vi2ChunkIntervalDataProvider(vi2, useflags));

			} else if (mycolumn == "SCAN_NUMBER" || mycolumn == "SCAN") {
				retval = doClassicalStatistics(
					sortColumnIds,
					hideTimeAxis,
					new Vi2ChunkScanDataProvider(vi2, useflags));

			} else if (mycolumn == "TIME") {
				retval = doClassicalStatistics(
					sortColumnIds,
					hideTimeAxis,
					new Vi2ChunkTimeDataProvider(vi2, useflags));

			} else if (mycolumn == "WEIGHT_SPECTRUM") {
				retval = doClassicalStatistics(
					sortColumnIds,
					hideTimeAxis,
					new Vi2ChunkWeightSpectrumDataProvider(vi2, useflags));

			} else {
				stringstream ss;
				ss << "Unsupported column name: " << column;
				throw AipsError(ss.str());
			}

		} // end if !detached
	} catch (AipsError x) {
		*itsLog <<
			LogIO::SEVERE <<
			"Exception Reported: " <<
			x.getMesg() <<
			LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return retval;
}

bool
ms::lister(const std::string& options,
           const std::string& datacolumn,
           const std::string& field,
           const std::string& spw,
           const std::string& antenna,
           const std::string& timerange,
           const std::string& correlation,
           const std::string& scan,
           const std::string& feed,
           const std::string& array,
           const std::string& observation,
           const std::string& uvrange,
           const std::string& average,
           const bool         showflags,
           const std::string& msselect,
           const int          pagerows,
           const std::string& listfile)
{
	Bool rstat(false);
	try {
		if(detached()) return false;
		*itsLog << LogOrigin("ms", "lister");

		MSLister msl(*itsMS, *itsLog);
		msl.list(options, datacolumn, field, spw, antenna, timerange,
		         correlation, scan, feed, array, observation, uvrange, average,
		         showflags, msselect, pagerows, listfile);
		rstat = true;
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return rstat;
}

Bool ms::checkinit() {
    if (initSel_p) return True;
    // No DDIDs selected, check if data shapes same
    // else select DDID 0
    initSel_p = True;;
    vi::VisibilityIterator2* vi2 = new vi::VisibilityIterator2(*itsSelectedMS);
    vi::VisBuffer2* vb2 = vi2->getVisBuffer();
    vi2->originChunks();
    vi2->origin();
    IPosition firstShape = vb2->getShape();
    Bool shapesConform = True;
    for (vi2->originChunks(); vi2->moreChunks(); vi2->nextChunk()) {
        for (vi2->origin(); vi2->more(); vi2->next()) {
            IPosition thisShape = vb2->getShape();
            if (thisShape(0) != firstShape(0) ||
                    thisShape(1) != firstShape(1)) {
                shapesConform = False;
                break;
            }
        }
    }
    initSel_p = shapesConform;
    if (!shapesConform) {
	    *itsLog << LogOrigin("ms", "checkinit");
		*itsLog << LogIO::WARN << "Data shape varies, selecting first data desc id only" << LogIO::POST;
        initSel_p = selectinit2(0);
    }
    delete vi2;
    return initSel_p;
}

bool
ms::selectinit(const int datadescid, const bool reset)
{
	Bool retval = False;
	try {
		Vector<Int> ddId(1, datadescid);
		if(!detached()){
			Int n=ddId.nelements();
			if (n>0 && min(ddId)<0 && !reset) {
				*itsLog << "The data description id must be a list of "
					"positive integers" << LogIO::EXCEPTION;
			}
			if (n>0) {
				Vector<Int> tmp(ddId.nelements());
				tmp=ddId;
				retval = itsSel->initSelection(tmp, reset);
			} else {
				retval = itsSel->initSelection(ddId, reset);
			}
		}
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(True);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(True);
	return retval;
}

bool
ms::selectinit2(const int datadescid, const bool resetsel)
{
	*itsLog << LogOrigin("ms", "selectinit2");
	Bool retval = false;
	try {
		Vector<Int> ddId(1, datadescid);
		if(!detached()){
			Int n=ddId.nelements();
			if (n>0 && min(ddId)<0 && !resetsel) {
				*itsLog << "The data description id must be a list of "
					"positive integers" << LogIO::EXCEPTION;
			}
            if (resetsel) {
                retval = reset();
            } else {
                String taQLExpr = "DATA_DESC_ID IN [" + String::toString(datadescid) + "]";
                retval = selecttaql2(taQLExpr);
                initSel_p = retval;
            }
		}
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return retval;
}

bool
ms::select(const ::casac::record& items)
{
	Bool retval = false;
	try {
		if(!detached()){
			Record *myTmp = toRecord(items);
			retval = itsSel->select(*myTmp, false);
			delete myTmp;
		}
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(True);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(True);
	return retval;
}

bool
ms::select2(const ::casac::record& items)
{
	*itsLog << LogOrigin("ms", "select");
    // Use selecttaql and msselect for these selections
	Bool retval = true;
	try {
		if(!detached()){
            if (checkinit()) {
	          *itsLog << LogOrigin("ms", "select");
			  Record* selRecord = toRecord(items);
              for (uInt field=0; field < selRecord->nfields(); ++field) {
                String fieldStr = selRecord->name(field);
                fieldStr.upcase();
                if (fieldStr=="ANTENNA1" || 
                    fieldStr=="ANTENNA2" || 
                    fieldStr=="ARRAY_ID" || 
                    fieldStr=="FEED1" ||
                    fieldStr=="FEED2" ||
                    fieldStr=="FIELD_ID" ||
                    fieldStr=="SCAN_NUMBER" ||
                    fieldStr=="DATA_DESC_ID" ||
                    fieldStr=="ROWS") {
                  String taqlStr = fieldStr;
                  if (fieldStr=="ROWS") taqlStr = "ROWID()";
                  taqlStr += " in [";
                  taqlStr += MSSelection::indexExprStr(selRecord->asArrayInt(RecordFieldId(field)));
                  taqlStr += "]";
			      retval = retval & selecttaql2(taqlStr);
                }
                else if (fieldStr=="IFR_NUMBER") {
                    Vector<Int> ifrNums = selRecord->asArrayInt(RecordFieldId(field));
                    String antennaExpr("");
                    Record antSelRec(Record::Variable);
                    for (uInt nums=0; nums<ifrNums.nelements(); ++nums) {
                        Int ant1 = ifrNums(nums)/1000;
                        Int ant2 = ifrNums(nums)%1000;
                        String bslnExpr = String::toString(ant1)+"&"+String::toString(ant2);
                        antennaExpr += bslnExpr + ";";
                     }
                     antennaExpr.rtrim(';'); // remove trailing ';'
                     antSelRec.define("baseline", antennaExpr);
                     ::casac::record* casacRec = fromRecord(antSelRec);
                     retval = retval & msselect(*casacRec);
                }
                else if (fieldStr=="TIME") {
                    Vector<Double> times = selRecord->asArrayDouble(RecordFieldId(field));
                    if (times.nelements()==2) {
                        Record timeSelRec(Record::Variable);
                        MVTime startTime = MVTime(times[0]/86400.0);
                        MVTime stopTime = MVTime(times[1]/86400.0);
                        String timeExpr = startTime.string(MVTime::YMD) + "~" + stopTime.string(MVTime::YMD);
                        timeSelRec.define("time", timeExpr);
                        ::casac::record* casacRec = fromRecord(timeSelRec);
                        retval = retval & msselect(*casacRec);
                    } else {
                        *itsLog << LogIO::WARN << "Illegal value for time range: two element numeric vector [start,stop] required" << LogIO::POST;
                        retval = false;
                    }
                }
                else if (fieldStr == "U" ||
                         fieldStr == "V" ||
                         fieldStr == "W") {
                    Vector<Double> uvw = selRecord->asArrayDouble(RecordFieldId(field));
                    if (uvw.nelements()==2) {
                        String uvwStr = "UVW[";
                        if (fieldStr=="U")      uvwStr += "1]";
                        else if (fieldStr=="V") uvwStr += "2]";
                        else                    uvwStr += "3]";
                        String taqlStr = uvwStr + ">=" + String::toString(uvw[0]);
                        taqlStr += " && ";
                        taqlStr += uvwStr + "<=" + String::toString(uvw[1]);
			            retval = retval & selecttaql2(taqlStr);
                    } else {
                        *itsLog << LogIO::WARN << "Illegal value for uvdist range selection: two element numeric vector required" << LogIO::POST;
                        retval = false;
                    }       
                }
                else if (fieldStr == "UVDIST") {
                    Vector<Double> uvdist = selRecord->asArrayDouble(RecordFieldId(field));
                    if (uvdist.nelements()==2) {
                        Record uvdistSelRec(Record::Variable);
                        String uvdistExpr = String::toString(uvdist[0]) + "~" + String::toString(uvdist[1]);
                        uvdistSelRec.define("uvdist", uvdistExpr);
                        ::casac::record* casacRec = fromRecord(uvdistSelRec);
                        retval = retval & msselect(*casacRec);
                    } else {
                        *itsLog << LogIO::WARN << "Illegal value for uvdist range selection: two element numeric vector required" << LogIO::POST;
                        retval = false;
                    }
                }
                else
                  *itsLog << LogIO::WARN << "Unrecognized field in input ignored: "+fieldStr << LogIO::POST;
                     
              }
			  delete selRecord;
		  }
        }
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return retval;
}

bool
ms::selecttaql(const std::string& msselect)
{
	Bool retval(False);
	try {
		if(!detached())
			retval = itsSel->select(msselect);
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(True);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(True);
	return retval;
}

bool
ms::selecttaql2(const std::string& taqlstr)
{
    *itsLog << LogOrigin("ms", "selecttaql");
	Bool retval(false);
	try {
		if(!detached()) {
            Record taqlSelRec(Record::Variable);
            String taqlExpr = String::toString(taqlstr);
            taqlSelRec.define("taql", taqlExpr);
            ::casac::record* casacRec = fromRecord(taqlSelRec);
            retval = msselect(*casacRec);
        }
    } catch (MSSelectionNullSelection x) {
		*itsLog << LogIO::WARN << "Selected table has zero rows" << LogIO::POST;
        retval = true;
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return retval;
}

bool
ms::selectchannel(const int nchan, const int start, const int width, const int inc)
{
	Bool retval(false);
	try {
		if(!detached())
			retval = itsSel->selectChannel(nchan, start, width, inc);
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(True);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(True);
	return retval;
}

String ms::getSpwExpr() {
    // Get spw selection
    String spwExpr;
    Vector<Int> selSpws = getspectralwindows();
    // if using all spws, use "*"
    uInt totalSpws = itsSelectedMS->dataDescription().nrow();
    if(selSpws.size() == totalSpws) {
        spwExpr = "* : ";
    } else {
        for (uInt spwIdx=0; spwIdx<selSpws.size(); ++spwIdx) {
            // join spw sels with ';'
            if (spwIdx > 0) spwExpr += ";";
            spwExpr += String::toString(selSpws(spwIdx));
        }
        spwExpr += " : ";
    }
    return spwExpr;
}

bool
ms::selectchannel2(const int nchan, const int start, const int width, const int inc)
{
	*itsLog << LogOrigin("ms", "selectchannel2");
	Bool retval(false);
	try {
		if(!detached()) {
            if (checkinit()) {
	            *itsLog << LogOrigin("ms", "selectchannel2");
                // No averaging = width of 1
                Int chanbin = ( width>0 ? width : 1 );
                Bool ok = (nchan>0 && start>=0 && width>0 && inc>0);
                if (!ok) {
                    *itsLog << LogIO::SEVERE << "Illegal channel selection"<<LogIO::POST;
                    return False;
                }

                // Make channel selection string
                int firstchan = start;
                Vector<int> selChans(chanbin);
                String chanSelStr;
                for (int outchan=0; outchan<nchan; ++outchan) {
                    int chan = firstchan;
                    // Get list of channels
                    for (int j=0; j<chanbin; ++j)
                        selChans(j) = chan++;
                    // join chan sels with ';'
                    if (outchan > 0) chanSelStr += ";";
                    // convert chan vector to chan range string
                    chanSelStr += String::toString(selChans(0));
                    if (selChans.size() > 1)
                        chanSelStr += "~" + String::toString(selChans(selChans.size()-1));
                    firstchan += inc;
                }
                int end = selChans(selChans.size()-1);
                Vector<Int> spws = getspectralwindows();
                ROMSColumns msc(*itsSelectedMS);
                Int numchans = msc.spectralWindow().numChan().getColumn()(spws(0));
                if (start>numchans || end>numchans) {
                    *itsLog << LogIO::SEVERE << "Illegal channel selection"<<LogIO::POST;
                    return False;
                }
                
                // Check spw selection;
                String spwExpr = getSpwExpr();
                spwExpr += chanSelStr;
                // If spw is set you can't set it again with channel ranges
                // so make a new selMS
                MeasurementSet newSelectedMS;
                retval = mssSetData(*itsSelectedMS, newSelectedMS,
                        "","","","",spwExpr);
                if (retval) {
                    chansel_p.resize(4);
                    chansel_p = {nchan, start, chanbin, inc};
                    chanselExpr_p = chanSelStr;  // use in ChanAvgTVI Record
                }
            }
        }
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return retval;
}

bool
ms::selectpolarization(const std::vector<std::string>& wantedpol)
{
	Bool retval(False);
	try {
        if(!detached()) {
            itsSel->initSelection(); // seg fault if not initialized
            retval = itsSel->selectPolarization(toVectorString(wantedpol));
        }
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(True);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(True);
	return retval;
}

bool
ms::selectpolarization2(const std::vector<std::string>& wantedpol)
{
	*itsLog << LogOrigin("ms", "selectpolarization");
	Bool retval(false);
	try {
        if(!detached()) {
            if (checkinit()) {
	            *itsLog << LogOrigin("ms", "selectpolarization");
                Record polnSelRec(Record::Variable);
                String polnExpr = MSSelection::nameExprStr(wantedpol);
                polnSelRec.define("polarization", polnExpr);
                ::casac::record* casacRec = fromRecord(polnSelRec);
                retval = msselect(*casacRec);
                if (retval) {
                    polnExpr_p = polnExpr;
                    wantedpol_p.resize();
                }
            }
        }
	} catch (AipsError x) {
        if (x.getMesg().find("No match") != std::string::npos) {
            // either invalid poln selection or need conversion
            try {
                wantedpol_p.resize(wantedpol.size());
                for (uInt i=0; i<wantedpol.size(); ++i) {
                    Stokes::StokesTypes type = Stokes::type(wantedpol[i]);
                    if (type == Stokes::Undefined) {
                        *itsLog << LogIO::SEVERE << "Unrecognized polarization: "<< wantedpol[i] << LogIO::POST;
                        wantedpol_p.resize();
	                    Table::relinquishAutoLocks(true);
                        return false;
                    } else {
                        wantedpol_p[i] = type;
                    }
                }
                polnExpr_p = "";
                itsMSS->setPolnExpr(polnExpr_p);
                retval = true;
            } catch (AipsError x) {
		        *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		        Table::relinquishAutoLocks(true);
		        RETHROW(x);
            }
        } else {
		    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		    Table::relinquishAutoLocks(true);
		    RETHROW(x);
        }
	}
	Table::relinquishAutoLocks(true);
	return retval;
}

bool
ms::regridspw(const std::string& outframe,
              const std::string& regrid_quantity,
              const double regrid_velo_restfrq,
              const std::string& regrid_interp_meth,
              const double regrid_start,
              const double regrid_center,
              const double regrid_bandwidth,
              const double regrid_chan_width,
              const bool hanning)
{
	Bool rstat(false);
	try {
		*itsLog << LogOrigin("ms", "regridspw");
		if(!ready2write_()){
			*itsLog << LogIO::SEVERE
			        << "Please open ms with parameter nomodify=false. Write access to ms is needed."
			        << LogIO::POST;

			return false;
		}

		double center = regrid_center;
		if(center > -1E30 && regrid_start > -1E30){
			Bool agree(false);
			if( regrid_quantity == "chan" ){
				agree = (center == floor(regrid_bandwidth/2. + regrid_start));
			}
			else{
				agree = (center == regrid_bandwidth/2. + regrid_start);
			}
			if(!agree){ // start and center don't agree
				*itsLog << LogIO::SEVERE
				        << "Please give only the start (lower edge) or the center of the new spectral window, not both."
				        << LogIO::POST;
				return false;
			}
		}
		else if(regrid_start > -1E30){ // only start given, need to calculate center
			if( regrid_quantity == "chan" ){
				center = floor(regrid_bandwidth/2. + regrid_start);
			}
			else{
				center = regrid_bandwidth/2. + regrid_start;
			}
		}

		SubMS *subms = new SubMS(*itsMS);
		*itsLog << LogIO::NORMAL << "Starting spectral frame transformation / regridding ..." << LogIO::POST;
		String t_outframe=toCasaString(outframe);
		String t_regridQuantity=toCasaString(regrid_quantity);
		String t_regridInterpMeth=toCasaString(regrid_interp_meth);
		Int rval;
		String regridMessage;


		if((rval = subms->regridSpw(regridMessage,
		                            t_outframe,
		                            t_regridQuantity,
		                            Double(regrid_velo_restfrq),
		                            t_regridInterpMeth,
		                            Double(center),
		                            Double(regrid_bandwidth),
		                            Double(regrid_chan_width),
		                            hanning
			    )
			   )==1){ // successful modification of the MS took place
			*itsLog << LogIO::NORMAL << "Spectral frame transformation/regridding completed." << LogIO::POST;

			// Update HISTORY table of modfied MS
			String message= "Transformed/regridded with regridspw";
			writehistory(message, regridMessage, "ms::regridspw()", "", "ms"); // empty name writes to itsMS
			rstat = true;
		}
		else if(rval==0) { // an unsuccessful modification of the MS took place
			String message= "Frame transformation to " + t_outframe + " failed. MS probably invalid.";
			*itsLog << LogIO::WARN << message << LogIO::POST;
			// Update HISTORY table of the unsuccessfully modfied MS
			ostringstream param;
			param << "Original input parameters: outframe=" << t_outframe << " mode= " <<  t_regridQuantity
			      << " center= " << center << " bandwidth=" << regrid_bandwidth
			      << " chanwidth= " << regrid_chan_width << " restfreq= " << regrid_velo_restfrq
			      << " interpolation= " << t_regridInterpMeth;
			String paramstr=param.str();
			writehistory(message,paramstr,"ms::regridspw()", "", "ms"); // empty name writes to itsMS
		}
		else {
			*itsLog << LogIO::NORMAL << "MS not modified." << LogIO::POST;
			rstat = true;
		}

		delete subms;

	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return rstat;
}


bool
ms::cvel(const std::string& mode,
         const int nchan,
         const ::casac::variant& start, const ::casac::variant& width,
         const std::string& interp,
         const ::casac::variant& phasec,
         const ::casac::variant& restfreq,
         const std::string& outframe,
         const std::string& veltype,
         const bool hanning)
{
	Bool rstat(false);

	Bool hanningDone(false);
	Bool needToClearModel(false);
	SubMS *sms = 0;

	try {

		*itsLog << LogOrigin("ms", "cvel");

		Bool t_doHanning = hanning;

		String t_phasec = toCasaString(phasec);

		String t_mode;
		String t_outframe;
		String t_regridQuantity;
		Double t_restfreq;
		String t_regridInterpMeth;
		Double t_cstart;
		Double t_bandwidth;
		Double t_cwidth;
		Bool t_centerIsStart;
		Bool t_startIsEnd;
		Int t_nchan;
		Int t_width;
		Int t_start;

		casacore::MDirection  t_phaseCenter;
		Int t_phasec_fieldid = -1;

		//If phasecenter is a simple numeric value then it's taken as a fieldid
		//otherwise its converted to a MDirection
		if(phasec.type()==::casac::variant::DOUBLEVEC
		   || phasec.type()==::casac::variant::DOUBLE
		   || phasec.type()==::casac::variant::INTVEC
		   || phasec.type()==::casac::variant::INT){
			t_phasec_fieldid = phasec.toInt();
			if(t_phasec_fieldid >= (Int)itsMS->field().nrow() || t_phasec_fieldid < 0){
				*itsLog << LogIO::SEVERE << "Field id " << t_phasec_fieldid
				        << " selected to be used as phasecenter does not exist." << LogIO::POST;
				return false;
			}
		}
		else{
			if(t_phasec.empty()){
				t_phasec_fieldid = 0;
			}
			else{
				if(!casaMDirection(phasec, t_phaseCenter)){
					*itsLog << LogIO::SEVERE << "Could not interprete phasecenter parameter "
					        << t_phasec << LogIO::POST;
					return false;
				}
				*itsLog << LogIO::NORMAL << "Using user-provided phase center." << LogIO::POST;
			}
		}

		// go over the remaining grid parameters and consolidate them

		if(!SubMS::convertGridPars(*itsLog,
		                           mode,
		                           nchan,
		                           start.toString(),
		                           width.toString(),
		                           interp,
		                           restfreq.toString(),
		                           outframe,
		                           veltype,
		                           ////// output ////
		                           t_mode,
		                           t_outframe,
		                           t_regridQuantity,
		                           t_restfreq,
		                           t_regridInterpMeth,
		                           t_cstart,
		                           t_bandwidth,
		                           t_cwidth,
		                           t_centerIsStart,
		                           t_startIsEnd,
		                           t_nchan,
		                           t_width,
		                           t_start
			   )
			){
			// an error occured
			return false;
		}

		// end prepare regridding parameters

		*itsLog << LogOrigin("ms", "cvel");

		String originalName = itsMS->tableName();

		// test parameters of input SPWs
		Bool foundInconsistentSPW = false;

		{
			Table spwtable(originalName+"/SPECTRAL_WINDOW");
			ROArrayColumn<Double> chanwidths(spwtable, "CHAN_WIDTH");
			ROArrayColumn<Double> chanfreqs(spwtable, "CHAN_FREQ");

			if(spwtable.nrow()>1){
				needToClearModel = true;
			}

			for(uInt ii=0; ii<spwtable.nrow(); ii++){
				Vector<Double> cw(chanwidths(ii));
				Vector<Double> cf(chanfreqs(ii));
				Int totNumChan = cw.size();

				Bool isEquidistant = true;
				for(Int i=0; i<totNumChan; i++){
					cw(i) = abs(cw(i)); // ignore sign of width
					if(abs(cw(i)-cw(0))>0.1){
						isEquidistant = false;
					}
				}
				Double minWidth = min(cw);
				Double maxWidth = max(cw);

				ostringstream oss;

				if(isEquidistant){
					oss <<  "Input spectral window " << ii << " has " << totNumChan
					    << " channels of width " << scientific << setprecision(6) << setw(6) << cw(0) << " Hz";
				}
				else{
					oss << "Input spectral window " << ii << " has " << totNumChan
					    << " channels of varying width: minimum width = " << scientific << setprecision(6) << setw(6) << minWidth
					    << " Hz, maximum width = " << scientific << setprecision(6) << setw(6) << maxWidth << " Hz";
				}
				oss << endl;
				if(totNumChan > 1){
					oss << "   First channel center = " << setprecision(9) << setw(9) << cf(0)
					    << " Hz, last channel center = " << setprecision(9) << setw(9) << cf(totNumChan-1) << " Hz";
				}
				else{
					oss << "   Channel center = " << setprecision(9) << setw(9) << cf(0) << " Hz";
				}

				Double chanOrderSign = 1.;
				if((cf(0)-cf(totNumChan-1))>0.){ // i.e. channel order is descending
					chanOrderSign = -1.;
					oss << "\n  i.e. channels are in order of decreasing frequency.\n";
				}

				for(Int i=0; i<totNumChan-2; i++){
					if( abs((cf(i)+cw(i)/2. * chanOrderSign) - (cf(i+1)-cw(i+1)/2. * chanOrderSign))>1.0 ){
						oss << "\n   Internal ERROR: Center of channel " << i <<  " is off nominal center by "
						    << ((cf(i)+cw(i)/2.* chanOrderSign) - (cf(i+1)-cw(i+1)/2.* chanOrderSign)) << " Hz\n"
						    << "   Distance between channels " << i << " and " << i+1 << " ("
						    << scientific << setprecision(6) << setw(6) << cf(i+1)-cf(i) << " Hz) is not equal to what is"
						    << " expected\n   from their channel widths which would be "
						    << scientific << setprecision(6) << setw(6) << + chanOrderSign * (cw(i)/2.+cw(i+1)/2.) << " Hz.\n"
						    << "   Will skip other channels in this SPW.";
						foundInconsistentSPW = true;
						break;
					}
				}
				*itsLog << LogIO::NORMAL  << oss.str() << LogIO::POST;
			}
		}
		if(foundInconsistentSPW){
			throw(AipsError("Inconsistent SPECTRAL_WINDOW table in input MS."));
		}

		// check disk space: need at least twice the size of the original for safety
		if (2 * DOos::totalSize(itsMS->tableName(), true) >
		    DOos::freeSpace(Vector<String>(1, itsMS->tableName()), true)(0)) {
			*itsLog << "Not enough disk space. To be on the safe side, need at least "
			        << 2 * DOos::totalSize(itsMS->tableName(), true)/1E6
			        << " MBytes on the filesystem containing " << itsMS->tableName()
			        << " for the SPW combination and regridding to succeed." << LogIO::EXCEPTION;
		}

		// need exclusive rights to this MS, will re-open it after combineSpws
		itsMS->flush();
		close();

		*itsLog << LogOrigin("ms", "cvel");

		sms = new SubMS(originalName, Table::Update);

		*itsLog << LogIO::NORMAL << "Starting combination of spectral windows ..." << LogIO::POST;

		// combine Spws
		if(!sms->combineSpws()){
			*itsLog << LogIO::SEVERE << "Error combining spectral windows." << LogIO::POST;
			delete sms;
			open(originalName,  Table::Update, false);
			return false;
		}

		*itsLog << LogIO::NORMAL << " " << LogIO::POST;

		//    cout << "trq " << t_regridQuantity << " ts " << t_start << " tcs " << t_cstart << " tb "
		//       << t_bandwidth << " tcw " << t_cwidth << " tw " << t_width << " tn " << t_nchan << endl;

		// Regrid

		*itsLog << LogIO::NORMAL << "Testing if spectral frame transformation/regridding is needed ..." << LogIO::POST;

		Int rval;
		String regridMessage;

		if((rval = sms->regridSpw(regridMessage,
		                          t_outframe,
		                          t_regridQuantity,
		                          t_restfreq,
		                          t_regridInterpMeth,
		                          t_cstart,
		                          t_bandwidth,
		                          t_cwidth,
		                          t_doHanning,
		                          t_phasec_fieldid, // == -1 if t_phaseCenter is valid
		                          t_phaseCenter,
		                          true, // use "center is start" mode
		                          t_startIsEnd,
		                          t_nchan,
		                          t_width,
		                          t_start
			    )
			   )==1){ // successful modification of the MS took place
			hanningDone = t_doHanning;

			*itsLog << LogIO::NORMAL << "Spectral frame transformation/regridding completed." << LogIO::POST;
			if(hanningDone){
				*itsLog << LogIO::NORMAL << "Hanning smoothing was applied." << LogIO::POST;
			}



			// Update HISTORY table of modfied MS
			String message = "Transformed/regridded with cvel";
			writehistory(message, regridMessage, "ms::cvel()", originalName, "ms");
			rstat = true;
		}
		else if(rval==0) { // an unsuccessful modification of the MS took place
			String message= "Frame transformation to " + t_outframe + " failed. MS probably invalid.";
			*itsLog << LogIO::WARN << message << LogIO::POST;
			// Update HISTORY table of the unsuccessfully modfied MS
			ostringstream param;
			param << "Original input parameters: outframe=" << t_outframe << " mode= " <<  t_regridQuantity
			      << " start= " << t_start << " bandwidth=" << t_bandwidth
			      << " chanwidth= " << t_width << " restfreq= " << t_restfreq
			      << " interpolation= " << t_regridInterpMeth;
			String paramstr=param.str();
			writehistory(message,paramstr,"ms::cvel()", originalName, "ms");
			rstat = false;
		}
		else { // there was no need to regrid
			*itsLog << LogIO::NORMAL << "SubMS not modified by regridding." << LogIO::POST;
			rstat = true;
		}

		delete sms;
		sms = 0;

		if(rstat){
			// print parameters of final SPW
			Table spwtable(originalName+"/SPECTRAL_WINDOW");
			ROArrayColumn<Double> chanwidths(spwtable, "CHAN_WIDTH");
			ROArrayColumn<Double> chanfreqs(spwtable, "CHAN_FREQ");

			Vector<Double> cw(chanwidths(0));
			Vector<Double> cf(chanfreqs(0));
			Int totNumChan = cw.size();

			Bool isEquidistant = true;
			for(Int i=0; i<totNumChan; i++){
				if(abs(cw(i)-cw(0))>0.1){
					isEquidistant = false;
				}
			}
			Double minWidth = min(cw);
			Double maxWidth = max(cw);

			ostringstream oss;

			if(isEquidistant){
				oss <<  "Final spectral window has " << totNumChan
				    << " channels of width " << scientific << setprecision(6) << setw(6) << cw(0) << " Hz";
			}
			else{
				oss << "Final spectral window has " << totNumChan
				    << " channels of varying width: minimum width = " << scientific << setprecision(6) << setw(6) << minWidth
				    << " Hz, maximum width = " << scientific << setprecision(6) << setw(6) << maxWidth << " Hz";
			}
			oss << endl;
			if(totNumChan > 1){
				oss << "First channel center = " << setprecision(9) << setw(9) << cf(0)
				    << " Hz, last channel center = " << setprecision(9) << setw(9) << cf(totNumChan-1) << " Hz";
			}
			else{
				oss << "Channel center = " << setprecision(9) << setw(9) << cf(0) << " Hz";
			}

			Double chanOrderSign = 1.;
			if((cf(0)-cf(totNumChan-1))>0.){ // i.e. channel order is descending
				chanOrderSign = -1.;
				oss << "\n  i.e. channels are in order of decreasing frequency.\n";
			}

			for(Int i=0; i<totNumChan-2; i++){
				if( abs((cf(i)+cw(i)/2. * chanOrderSign) - (cf(i+1)-cw(i+1)/2. * chanOrderSign))>1.0 ){
					oss << "\n   Internal Error: Center of channel " << i <<  " is off nominal center by "
					    << ((cf(i)+cw(i)/2.* chanOrderSign) - (cf(i+1)-cw(i+1)/2.* chanOrderSign)) << " Hz\n"
					    << "     Will not test subsequent channels.";
					break;
				}
			}
			*itsLog << LogIO::NORMAL  << oss.str() << LogIO::POST;
		}

		if(rstat){ // re-open MS for writing, unlocked
			open(originalName,  false, false);
			*itsLog << LogOrigin("ms", "cvel");
			if(!hanningDone && t_doHanning){
				// still need to Hanning Smooth
				hanningsmooth("all");
			}

			if(needToClearModel){
				*itsLog << LogIO::NORMAL  << "NOTE: any virtual model data will be cleared." << LogIO::POST;
				CountedPtr<VisModelDataI> visModelData = VisModelDataI::create();
				visModelData->clearModelI(*itsMS);
			}

		}

	} catch (AipsError x) {
		if(sms){
			delete sms;
		}
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return rstat;
}

std::vector<double>
ms::cvelfreqs(const std::vector<int>& spwids,
              const std::vector<int>& fieldids,
              const std::string& obstime,
              const std::string& mode,
              const int nchan,
              const ::casac::variant& start,
              const ::casac::variant& width,
              const ::casac::variant& phasec,
              const ::casac::variant& restfreq,
              const std::string& outframe,
              const std::string& veltype,
              const bool verbose)
{
	std::vector<double> rval(0); // the new channel centers

	try {

		if(!detached()){

			*itsLog << LogOrigin("ms", "cvelfreqs");

			if(verbose){
				*itsLog << LogIO::NORMAL << "Calculating grid ..." << LogIO::POST;
			}

			Vector<Double> newCHAN_FREQ;
			Vector<Double> newCHAN_WIDTH;


			ROMSMainColumns mainCols(*itsMS);
			ROScalarColumn<Double> timeCol(mainCols.time());
			ROScalarColumn<Int> ddCol(mainCols.dataDescId());
			ROScalarColumn<Int> fieldCol(mainCols.fieldId());

			ROMSDataDescColumns DDCols(itsMS->dataDescription());
			ROScalarColumn<Int> spwidCol(DDCols.spectralWindowId());

			ROMSSpWindowColumns SPWCols(itsMS->spectralWindow());
			ROMSFieldColumns FIELDCols(itsMS->field());
			ROMSAntennaColumns ANTCols(itsMS->antenna());

			// extract grid from SPW given by spwids
			Vector<Double> oldCHAN_FREQ;
			Vector<Double> oldCHAN_WIDTH;

			if(spwids.size() == 1){ // only one spw selected
				oldCHAN_FREQ.assign(SPWCols.chanFreq()(spwids[0]));
				oldCHAN_WIDTH.assign(SPWCols.chanWidth()(spwids[0]));
			}
			else if(spwids.size() > 1){ // several ids selected
				SubMS theMS(*itsMS);
				Vector<Int> spwidv(spwids);
				if(!theMS.combineSpws(spwidv,
				                      true, // dont't modify the MS
				                      oldCHAN_FREQ,
				                      oldCHAN_WIDTH,
				                      verbose
					   )){
					*itsLog << LogIO::SEVERE << "Error combining SPWs." << LogIO::POST;
					return rval;
				}
			}
			else{
				*itsLog << LogIO::NORMAL << "Zero SPWs selected." << LogIO::POST;
				return rval;
			}

			// determine old reference frame
			casacore::MFrequency::Types theOldRefFrame = MFrequency::castType(SPWCols.measFreqRef()(spwids[0]));

			// determine observation epoch
			casacore::MEpoch theObsTime;
			String t_obstime = toCasaString(obstime);
			if (obstime.length()>0) {
				Quantum<Double> qt;
				if (MVTime::read(qt,obstime)) {
					casacore::MVEpoch mv(qt);
					theObsTime = casacore::MEpoch(mv, MEpoch::UTC);
				} else {
					*itsLog << LogIO::SEVERE << "Invalid time format: "
					        << obstime << LogIO::POST;
					return rval;
				}
			} else {
				// take the smallest obstime in the MS given the spw and field selection

				// determine the relevant DD Ids
				vector<Int> ddids;
				for(uInt irow=0; irow<DDCols.nrow(); irow++){ // loop over DD table
					for(uInt i=0; i<spwids.size(); i++){
						Int theSPWId = spwidCol(irow);
						if(theSPWId==spwids[i]){ // SPWId match found
							ddids.push_back(irow);
						}
					} // end for
				} // end for
				//cout << "DD IDs selected " << Vector<Int>(ddids) << endl;
				uInt minTimeRow = 0;
				Double minTime = 1E42;
				Bool doField = (fieldids.size()!=0);

				for(uInt irow=0;irow<itsMS->nrow(); irow++){ // loop over main table
					if(timeCol(irow)<minTime){

						Int theDDId = ddCol(irow);
						for(uInt i=0; i<ddids.size(); i++){
							if(theDDId==ddids[i]){ // DD match found
								if(doField){
									Int theFieldId = fieldCol(irow);
									for(uInt i=0; i<fieldids.size(); i++){
										if(theFieldId==fieldids[i]){ // field match found
											minTime=timeCol(irow);
											minTimeRow = irow;
											break;
										}
									}
								}
								else{ // all fields selected
									minTime=timeCol(irow);
									minTimeRow = irow;
								}
								break;
							} // end if DD matches
						} // end for

					}
				} // end for
				theObsTime = mainCols.timeMeas()(minTimeRow);
				if(verbose){
					*itsLog << LogIO::NORMAL << "Using observation time from earliest row of the MS given the SPW and FIELD selection:" << LogIO::POST;
					*itsLog << LogIO::NORMAL << "    " << MVTime(theObsTime.getValue().getTime()).string(MVTime::YMD)
					        << " (" << theObsTime.getRefString() << ")" << LogIO::POST;
				}
			}

			// determine phase center
			casacore::MDirection phaseCenter;
			casacore::MRadialVelocity mRV; // needed when using outframe "SOURCE"
			Int phasec_fieldid = -1;
			String t_phasec = toCasaString(phasec);

			//If phasecenter is a simple numeric value then it's taken as a fieldid
			//otherwise its converted to a MDirection
			if(phasec.type()==::casac::variant::DOUBLEVEC
			   || phasec.type()==::casac::variant::DOUBLE
			   || phasec.type()==::casac::variant::INTVEC
			   || phasec.type()==::casac::variant::INT){
				phasec_fieldid = phasec.toInt();
				if(phasec_fieldid >= (Int)itsMS->field().nrow() || phasec_fieldid < 0){
					*itsLog << LogIO::SEVERE << "Field id " << phasec_fieldid
					        << " selected to be used as phasecenter does not exist." << LogIO::POST;
					return rval;
				}
				else{
					phaseCenter = FIELDCols.phaseDirMeas(phasec_fieldid, theObsTime.get("s").getValue());
					mRV = FIELDCols.radVelMeas(phasec_fieldid, theObsTime.get("s").getValue());
				}
			}
			else{
				if(t_phasec.empty()){
					phasec_fieldid = 0;
					if(fieldids.size()!=0){
						phasec_fieldid = fieldids[0];
					}
					phaseCenter = FIELDCols.phaseDirMeas(phasec_fieldid, theObsTime.get("s").getValue());
					mRV = FIELDCols.radVelMeas(phasec_fieldid, theObsTime.get("s").getValue());
				}
				else{
					if(!casaMDirection(phasec, phaseCenter)){
						*itsLog << LogIO::SEVERE << "Could not interprete phasecenter parameter "
						        << t_phasec << LogIO::POST;
						return rval;
					}
					else{
						if(verbose){
							*itsLog << LogIO::NORMAL << "Using user-provided phase center." << LogIO::POST;
						}
					}
				}
			}


			// determine observatory position
			// use a tabulated version if available
			casacore::MPosition mObsPos;
			{
				casacore::MPosition Xpos;
				String Xobservatory;
				ROMSObservationColumns XObsCols(itsMS->observation());
				if (itsMS->observation().nrow() > 0) {
					Xobservatory = XObsCols.telescopeName()(mainCols.observationId()(0));
				}
				if (Xobservatory.length() == 0 ||
				    !casacore::MeasTable::Observatory(Xpos,Xobservatory)) {
					// unknown observatory
					if(verbose){
						*itsLog << LogIO::WARN << "Unknown observatory: \"" << Xobservatory
						        << "\". Determining observatory position from antenna 0." << LogIO::POST;
					}
					Xpos=casacore::MPosition::Convert(ANTCols.positionMeas()(0), casacore::MPosition::ITRF)();
				}
				else{
					if(verbose){
						*itsLog << LogIO::NORMAL << "Using tabulated observatory position for " << Xobservatory << ":"
						        << LogIO::POST;
					}
					Xpos=casacore::MPosition::Convert(Xpos, casacore::MPosition::ITRF)();
				}
				if(verbose){
					mObsPos = Xpos;
					ostringstream oss;
					oss <<  "   " << mObsPos << " (ITRF)";
					*itsLog << LogIO::NORMAL << oss.str() << LogIO::POST;
				}
			}

			// calculate new grid
			SubMS::calcChanFreqs(*itsLog,
			                     newCHAN_FREQ,
			                     newCHAN_WIDTH,
			                     oldCHAN_FREQ,
			                     oldCHAN_WIDTH,
			                     phaseCenter,
			                     theOldRefFrame,
			                     theObsTime,
			                     mObsPos,
			                     mode,
			                     nchan,
			                     start.toString(),
			                     width.toString(),
			                     restfreq.toString(),
			                     outframe,
			                     veltype,
			                     verbose,
			                     mRV
				);

			newCHAN_FREQ.tovector(rval);

		} // end if !detached

	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}

	return rval;
}

Vector<Int> ms::convertCorrToInt(vi::VisBuffer2* vb2) {
    Vector<Stokes::StokesTypes> types = vb2->getCorrelationTypesSelected();
    size_t nPol = types.size();
    Vector<Int> inputPol(nPol);
    for (uInt i=0; i<nPol; ++i)
        inputPol(i) = static_cast<Int>(types(i));
    return inputPol;
}

Vector<Int> ms::addgaps(Vector<Int> ifrnums, Int gap) {
    Int gapRows = (nAnt1_p-1) * gap;
    Vector<Int> ifrgap(ifrnums.size() + gapRows);
    uInt gapIdx=0;
    for (uInt i=0; i<ifrnums.size(); ++i) {
        // look for ant1 change:
        if (i>0 && (ifrnums(i)/1000 != ifrnums(i-1)/1000)) {
            // add gap(s)
            for (Int ngap=0; ngap<gap; ++ngap) {
               ifrgap(gapIdx++) = -1;
            }
        }
        ifrgap(gapIdx++) = ifrnums(i);
    }
    return ifrgap;
}

Vector<Int> ms::getifrnumbers() {
    // Get vector of ifr numbers in MS
    ROMSColumns msc(*itsSelectedMS);
    const Sort::Order order = Sort::Ascending;
    const Int option = Sort::HeapSort | Sort::NoDuplicates;
    // form ifr numbers
    Vector<Int> ant1 = msc.antenna1().getColumn();
    Vector<Int> ant2 = msc.antenna2().getColumn();
    Vector<Int> ifrnums = ant1*1000 + ant2;  // ifrnums for all rows of MS
    // now sort and unique (need nAnt1_p for gap)
    nAnt1_p = GenSort<Int>::sort(ant1, order, option);
    Int nIfr = GenSort<Int>::sort(ifrnums, order, option);
    return ifrnums(Slice(0,nIfr)); 
}

Vector<Int> ms::getbaselines(vi::VisBuffer2* vb2) {
    // Get ifr numbers in current visbuffer 
    Vector<Int> ant1 = vb2->antenna1();
    Vector<Int> ant2 = vb2->antenna2();
    Vector<Int> baselines = ant1*1000 + ant2; // form ifrnums vector
    return baselines;
}

template <typename T>
void ms::ifrToArray(Array<T>& ifrarray, vi::VisBuffer2* vb2) {
    // resizes per vb and reorders per MS ifr
    IPosition ifrshape = ifrarray.shape();
    ifrshape.setLast(IPosition(1, vb2->nRows()));
    Int ndim = ifrshape.size();
    Array<T> outputarray(ifrshape);
    outputarray.set(0);
    Vector<Int> baselines = getbaselines(vb2);
    Slicer outslicer, ifrslicer;
    for (uInt i=0; i<baselines.size(); ++i) {
        for (uInt j=0; j<ifrnumbers_p.size(); ++j) {
            if (baselines(i) == ifrnumbers_p(j)) {
                switch (ndim) {
                    case 1: {
                        outputarray[i] = ifrarray[j];
                        }
                        break;
                    case 2: {
                        outslicer = Slicer(Slice(), i);
                        ifrslicer = Slicer(Slice(), j);
                        outputarray(outslicer) = ifrarray(ifrslicer);
                        }
                        break;
                    case 3: {
                        outslicer = Slicer(Slice(), Slice(), i);
                        ifrslicer = Slicer(Slice(), Slice(), j);
                        outputarray(outslicer) = ifrarray(ifrslicer);
                        }
                        break;
                    default:
                        break;
                }
            }
        }
    }
    ifrarray.resize(ifrshape);
    ifrarray.reference(outputarray);
}

template <typename T>
void ms::getIfrArray(Array<T>& inputarray, vi::VisBuffer2* vb2) {
    // resizes and reorders input array by ifr 
    IPosition inshape = inputarray.shape();
    inshape.setLast(IPosition(1, ifrnumbers_p.size()));
    Array<T> ifrarray(inshape);
    ifrarray.set(0);
    Int ndim = ifrarray.ndim();
    Slicer inslicer, ifrslicer;
    Vector<Int> baselines = getbaselines(vb2);
    for (uInt i=0; i<baselines.size(); ++i) {
        for (uInt j=0; j<ifrnumbers_p.size(); ++j) {
            if (baselines(i) == ifrnumbers_p(j)) {
                switch (ndim) {
                    case 1: {
                        ifrarray[j] = inputarray[i];
                        }
                        break;
                    case 2: {
                        inslicer = Slicer(Slice(), i);
                        ifrslicer = Slicer(Slice(), j);
                        ifrarray(ifrslicer) = inputarray(inslicer);
                        }
                        break;
                    case 3: {
                        inslicer = Slicer(Slice(), Slice(), i);
                        ifrslicer = Slicer(Slice(), Slice(), j);
                        ifrarray(ifrslicer) = inputarray(inslicer);
                        }
                        break;
                    default:
                        break;
                }
            }
        }
    }
    inputarray.resize(inshape);
    inputarray.reference(ifrarray);
}

::casac::record*
ms::getdata(const std::vector<std::string>& items, const bool ifraxis, const int ifraxisgap, const int increment, const bool average)
{

	::casac::record *retval(0);
	try {
		/*
		  Matrix<Int> m(10u,10u);
		  m=0;
		  for(int i=1;i<11;i++){
		  for(int j=1;j<11;j++){
		  m(i-1,j-1) = i*10+j;
		  }
		  }
		  std::cerr << m << std::endl;
		  Int *storage;
		  Bool deleteIt(false);
		  storage = m.getStorage(deleteIt);
		  for(int k=0;k<10;k++){
		  for(int l=0;l<10;l++){
		  std::cerr << *(storage + k*10+l) << " ";
		  }
		  std::cerr << std::endl;
		  }
		*/
		if(!detached())
			retval = fromRecord(itsSel->getData(toVectorString(items), ifraxis, ifraxisgap, increment, average, false));
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(True);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(True);
	return retval;
}

::casac::record*
ms::getdata2(const std::vector<std::string>& items, const bool ifraxis, const int ifraxisgap, const int increment, const bool average)
{
	*itsLog << LogOrigin("ms", "getdata2");

	::casac::record *retval(0);
	try {
		if(!detached()) {
            Record out(RecordInterface::Variable);
            Vector<String> itemnames(items);
            Int axisgap = ifraxisgap;
		    doingAveraging_p = average;

            uInt nrows = itsSelectedMS->nrow();
            if (nrows == 0) {
                *itsLog << LogIO::WARN << "Selected Table is empty - use selectinit" << LogIO::POST;
		        return retval;
	        }

            if (axisgap>0 && ifraxis==false) {
                *itsLog << LogIO::WARN << "ifraxis not requested, ignoring ifraxisgap argument" << LogIO::POST;
                axisgap = 0;
            }

            // Keep table before increment selection, restore later
            MeasurementSet origSelMS = *itsSelectedMS;
            if ((increment>1) && (uInt(increment)<=nrows)) {
                Vector<uInt> rows(nrows/increment);
                indgen(rows, uInt(0), uInt(increment));
                Table selTable = (*itsSelectedMS)(rows);
                *itsSelectedMS = selTable;
            }

            if (ifraxis) 
                ifrnumbers_p = getifrnumbers();
            if (axisgap>0) {
                Vector<Int> ifrWithGaps = addgaps(ifrnumbers_p, axisgap);
                ifrnumbers_p.resize(ifrWithGaps.size());
                ifrnumbers_p = ifrWithGaps;
            }

            // Check for flag_sum, done after get flags
            // Check for ha, last, ut -- included in AXIS_INFO
            // Check data columns: empty array if does not exist
            // (issue warning once!)
            ROMSColumns msc(*itsSelectedMS);
            Bool noCorrectedCol = msc.correctedData().isNull();
            Bool noModelCol = msc.modelData().isNull();
            Bool noFloatCol = msc.floatData().isNull();
            Bool do_flag_sum(False), do_axis_info(False), do_info_options(False),
                 do_time(False), do_field(False), do_flag(False), do_weight(False);
            Vector<Bool> info_options(3); // [ha, last, ut]
            for (uInt it=0; it<itemnames.size(); ++it) {
                String name = downcase(itemnames(it));
                if (name=="flag_sum") 
                    do_flag_sum = True; // added later
                if (name=="axis_info") 
                    do_axis_info = True;
                if (name=="time") 
                    do_time = True; // needed for axis_info
                if (name=="field_id") 
                    do_field = True; // needed for info options
                if (average) {
                    if (name=="flag") do_flag = True;
                    if (name=="weight") do_weight = True;
                }
                if (name=="ha" && ifraxis) {
                    do_info_options = True;
                    info_options(0) = True;
                    out.define(itemnames(it), info_options);
                    itemnames(it)="";
                }
                if (name=="last" && ifraxis) {
                    do_info_options = True;
                    info_options(1) = True;
                    out.define(itemnames(it), info_options);
                    itemnames(it)="";
                }
                if (name=="ut" && ifraxis) {
                    do_info_options = True;
                    info_options(2) = True;
                    out.define(itemnames(it), info_options);
                    itemnames(it)="";
                }

                // check data columns
                // model
                if (noModelCol &&
                    ((name.find("model")!=string::npos) ||
                     (name.find("ratio")!=string::npos) ||
                     (name.find("residual")!=string::npos))) { 
		                *itsLog << LogIO::WARN << "Requested column doesn't exist: " + itemnames(it) <<  LogIO::POST;
                        if (name.find("data")!=string::npos) {
                            out.define(itemnames(it), Array<Complex>());
                        } else {
                            out.define(itemnames(it), Array<Float>());
                        }
                        itemnames(it)="";
                }
                // corrected
                if (noCorrectedCol &&
                    ((name.find("corrected")!=string::npos) ||
                     (name.find("ratio")!=string::npos) ||
                     (name.find("residual")!=string::npos))) {
		                *itsLog << LogIO::WARN << "Requested column doesn't exist: " + itemnames(it) <<  LogIO::POST;
                        if (name.find("data")!=string::npos) {
                            out.define(itemnames(it), Array<Complex>());
                        } else {
                            out.define(itemnames(it), Array<Float>());
                        }
                        itemnames(it)="";
                }
                // float
                if (noFloatCol &&
                    name.find("float")!=string::npos) {
		                *itsLog << LogIO::WARN << "Requested column doesn't exist: " + itemnames(it) <<  LogIO::POST;
                        out.define(itemnames(it), Array<Float>());
                        itemnames(it)="";
                }
            } // for loop (itemnames)

            if (ifraxis && do_axis_info && !do_time) {
                // need time for time_axis
                size_t itemsSize = itemnames.size();
                itemnames.resize(itemsSize+1, True);
                itemnames(itemsSize) = "time";
            }
            if (ifraxis && do_axis_info && do_info_options && !do_field) {
                // need field for options
                size_t itemsSize = itemnames.size();
                itemnames.resize(itemsSize+1, True);
                itemnames(itemsSize) = "field_id";
            }
            if (average && !do_flag) {
                size_t itemsSize = itemnames.size();
                itemnames.resize(itemsSize+1, True);
                itemnames(itemsSize) = "flag";
            }
            if (average && !do_weight) {
                size_t itemsSize = itemnames.size();
                itemnames.resize(itemsSize+1, True);
                itemnames(itemsSize) = "weight";
            }


            // iterate to next chunk or subchunk (if maxrows) and get items
            if (itsVI2) {
                vi::VisBuffer2* vb2 = itsVI2->getVisBuffer();
                if (maxrows_p) {
                    // this is per visbuffer!
                    // Iteration adjusted in iternext2()
                    for (uInt it=0; it<itemnames.size(); ++it) {
                        if (!itemnames(it).empty()) 
                            getitem(itemnames(it), vb2, out, ifraxis);
                    }
                } else {
                    for (itsVI2->origin(); itsVI2->more(); itsVI2->next()) {
                        for (uInt it=0; it<itemnames.size(); ++it) {
                            if (!itemnames(it).empty()) 
                                getitem(itemnames(it), vb2, out, ifraxis);
                        }
                    }
                }
            } else {   
                // Set up iterator and go through all chunks
                // Note: iter methods change LogOrigin, change back!
                if (checkinit()) {
                    *itsLog << LogOrigin("ms", "getdata2");
                    // init iterator, do not sort columns
                    std::vector<std::string> columns = {""};
                    if (iterinit2(columns, 0.0, 0, false)) {
                        iterorigin2();
                        vi::VisBuffer2* vb2 = itsVI2->getVisBuffer();
                        *itsLog << LogOrigin("ms", "getdata2");
                        // handle first chunk
                        for (itsVI2->origin(); itsVI2->more(); itsVI2->next()) {
                            for (uInt it=0; it<itemnames.size(); ++it) {
                                if (!itemnames(it).empty()) {
                                    getitem(itemnames(it), vb2, out, ifraxis);
                                }
                            }
                        }
                        // continue iteration
                        while (iternext2()) {
                            *itsLog << LogOrigin("ms", "getdata2");
                            for (itsVI2->origin(); itsVI2->more(); itsVI2->next()) {
                                for (uInt it=0; it<itemnames.size(); ++it) {
                                    if (!itemnames(it).empty())
                                        getitem(itemnames(it), vb2, out, ifraxis);
                                }
                            }
                        }
                        iterend2();
                        *itsLog << LogOrigin("ms", "getdata2");
                    } // iterinit
                } // checkinit 
            } // else (!VI2)

            if (average) {
                getAveragedValues(itemnames, out);
                // remove or redefine flag field
                if (!do_flag) out.removeField("flag");
                else {
                    if (out.isDefined("dataflag"))
                        out.renameField("dataflag", "flag");
                }
                // remove or redefine weight field
                if (!do_weight) out.removeField("weight");
                else {
                    Array<Float> weights = out.asArrayFloat("weight");
                    out.removeField("weight");
                    getWeightSum(weights);  // redefines weights array
                    out.define("weight", weights); 
                }
            }
            if (do_flag_sum) {
                Array<Bool> flagarray = out.asArrayBool("flag_sum");
                out.removeField("flag_sum");
                Array<Int> flagsum = getFlagCount(flagarray, ifraxis);
                out.define("flag_sum", flagsum);
            }
            if (do_axis_info && ifraxis) {
                // copy time field and remove if not requested by user
                addTimeAxis(out);
                if (!do_time) out.removeField("time");
                if (do_info_options) {
                    getInfoOptions(info_options, out);
                    // remove field_id field if not requested by user
                    if (!do_field) out.removeField("field_id");
                }
            }
            // restore original selected table
            *itsSelectedMS = origSelMS;
            retval = casa::fromRecord(out);
        } // !detached
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return retval;
}

void ms::getAveragedValues(Vector<String> fieldnames, Record& rec) {
    for (uInt it=0; it<fieldnames.size(); ++it) {
        String field = fieldnames(it);
        MSS::Field fld = MSS::field(field);
        switch(fld) {
            case MSS::AMPLITUDE:
            case MSS::CORRECTED_AMPLITUDE:
            case MSS::MODEL_AMPLITUDE:
            case MSS::RATIO_AMPLITUDE:
            case MSS::RESIDUAL_AMPLITUDE:
            case MSS::OBS_RESIDUAL_AMPLITUDE:
            case MSS::IMAGINARY:
            case MSS::CORRECTED_IMAGINARY:
            case MSS::MODEL_IMAGINARY:
            case MSS::RATIO_IMAGINARY:
            case MSS::RESIDUAL_IMAGINARY:
            case MSS::OBS_RESIDUAL_IMAGINARY:
            case MSS::PHASE:
            case MSS::CORRECTED_PHASE:
            case MSS::MODEL_PHASE:
            case MSS::RATIO_PHASE:
            case MSS::RESIDUAL_PHASE:
            case MSS::OBS_RESIDUAL_PHASE:
            case MSS::REAL:
            case MSS::CORRECTED_REAL:
            case MSS::MODEL_REAL:
            case MSS::RATIO_REAL:
            case MSS::RESIDUAL_REAL:
            case MSS::OBS_RESIDUAL_REAL:
            case MSS::FLOAT_DATA: {
                Array<Bool> flags = rec.asArrayBool("flag");
                Array<Float> weight = rec.asArrayFloat("weight");
                Array<Float> data = rec.asArrayFloat(field);
                Array<Bool> dataflag;
                MSSelUtil2<Float>::timeAverage(dataflag, data, flags, weight);
                rec.removeField(field);
                rec.define(field, data);
                rec.define("dataflag", dataflag);
                }
                break; 
            case MSS::DATA:
            case MSS::CORRECTED_DATA:
            case MSS::MODEL_DATA:
            case MSS::RATIO_DATA:
            case MSS::RESIDUAL_DATA:
            case MSS::OBS_RESIDUAL_DATA: {
                Array<Bool> flags = rec.asArrayBool("flag");
                Array<Float> weight = rec.asArrayFloat("weight");
                Array<Complex> data = rec.asArrayComplex(field);
                Array<Bool> dataflag;
                MSSelUtil2<Complex>::timeAverage(dataflag, data, flags, weight);
                rec.removeField(field);
                rec.define(field, data);
                rec.define("dataflag", dataflag);
                }
                break;
            case MSS::ANTENNA1:
            case MSS::ANTENNA2:
            case MSS::ARRAY_ID:
            case MSS::DATA_DESC_ID:
            case MSS::FEED1:
            case MSS::FEED2:
            case MSS::FIELD_ID:
            case MSS::IFR_NUMBER:
            case MSS::SCAN_NUMBER: {
                // Use common value if allEQ, else -1
                Vector<Int> intvec = rec.asArrayInt(field);
                if (intvec.nelements()>1) {
                    Int first = intvec(0);
                    if (!allEQ(intvec, first)) first = -1;
                    intvec.resize(1);
                    intvec(0) = first;
                }
                rec.removeField(field);
                rec.define(field, intvec);
                }
                break;
            case MSS::SIGMA: {
                Array<Float> sigma = rec.asArrayFloat(field);
                rec.removeField(field);
                getAvgSigma(sigma); // redefines sigma
                rec.define(field, sigma);
                }
                break; 
            case MSS::TIME: {
                Vector<Double> times = rec.asArrayDouble(field);
                Int ntimes = times.nelements();
                Double avgtime(0);
                for (Int i=0; i<ntimes; ++i) avgtime+=times(i);
                avgtime /= ntimes;
                times.resize(1);
                times = avgtime;
                rec.removeField(field);
                rec.define(field, times);
                }
                break;
            default:
                break;
        }
    }
}

void ms::getWeightSum(Array<Float>& weight) {
    // sum of weights when averaging
    IPosition arrayshape = weight.shape();
    uInt nCorr(arrayshape(0)), nIfr(0), nRow(0);
    if (arrayshape.size()==2) {  // Matrix: no ifraxis
        nRow = arrayshape(1);
        Vector<Float> sumwt(nCorr);
        sumwt = 0.0;
        for (uInt i=0; i<nRow; i++) {
            for (uInt j=0; j<nCorr; j++) {
                sumwt(j) += weight(IPosition(2,j,i));
            }
        }
        weight.resize(sumwt.shape());
        weight.reference(sumwt);
    } else { // Cube: ifraxis
        nIfr = arrayshape(1);
        nRow = arrayshape(2);
        Matrix<Float> sumwt(IPosition(2, nCorr, nIfr));
        sumwt = 0.0;
        for (uInt i=0; i<nIfr; i++) {
            for (uInt j=0; j<nCorr; j++) {
                for (uInt k=0; k<nRow; k++) {
                    sumwt(j,i) += weight(IPosition(3,j,i,k));
                }
            }
        }
        weight.resize(sumwt.shape());
        weight.reference(sumwt);
    }
}

void ms::getAvgSigma(Array<Float>& sigma) {
    IPosition arrayshape = sigma.shape();
    uInt nCorr(arrayshape(0)), nIfr(0), nRow(0);
    if (arrayshape.size()==2) {  // Matrix: no ifraxis
        nRow = arrayshape(1);
        Vector<Float> sumsquares(nCorr);
        sumsquares = 0.0;
        for (uInt j=0; j<nCorr; j++) {
            for (uInt i=0; i<nRow; ++i) {
                sumsquares(j) += square(sigma(IPosition(2,j,i)));
            }
            sumsquares(j) = sqrt(sumsquares(j));
        }
        sigma.resize(sumsquares.shape());
        sigma.reference(sumsquares);
    } else { // Cube: ifraxis
        nIfr = arrayshape(1);
        nRow = arrayshape(2);
        Matrix<Float> sumsquares(IPosition(2, nCorr, nIfr));
        sumsquares = 0.0;
        for (uInt i=0; i<nIfr; i++) {
            for (uInt j=0; j<nCorr; j++) {
                for (uInt k=0; k<nRow; k++) {
                    sumsquares(j,i) += square(sigma(IPosition(3,j,i,k)));
                }
                sumsquares(j,i) = sqrt(sumsquares(j,i));
            }
        }
        sigma.resize(sumsquares.shape());
        sigma.reference(sumsquares);
    }
}

void ms::convertPoln(Cube<Complex>& inputcube, vi::VisBuffer2* vb) {
    Cube<Complex> scData;
    Vector<Int> inputPol = convertCorrToInt(vb);
    StokesConverter* sc = new StokesConverter(wantedpol_p, inputPol, True);
    if (sc) sc->convert(scData, inputcube);
    inputcube.reference(scData);
    if (sc) delete sc;
}

template <typename T>
void ms::addArrayToRec(Array<T>& inputarray, Record& rec, 
        String field, Bool ifraxis) {
    Array<T> fieldArray;
    try {
        rec.get(field, fieldArray);
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
    rec.removeField(field); // free up fieldArray for resizing
    IPosition fieldArrayShape = fieldArray.shape();
    uInt ndim = fieldArrayShape.size();
    if (ifraxis) {
        if (ndim == inputarray.ndim()) {
            // add time axis
            fieldArrayShape.append(IPosition(1,1));
        }
        ssize_t timeAxisSize = fieldArrayShape.last();
        fieldArrayShape.setLast(IPosition(1, timeAxisSize + 1));
        fieldArray.resize(fieldArrayShape, True);
        // add inputarray to last axis
        fieldArray[timeAxisSize] = inputarray;
    } else {
        size_t startrows = fieldArrayShape.last();
        size_t addrows = inputarray.shape().last();
        IPosition start(ndim, 0);
        start.setLast(IPosition(1, startrows));
        IPosition length = fieldArrayShape.getFirst(ndim-1);
        length.append(IPosition(1,addrows));
        // add new rows to last axis, then add cube
        fieldArrayShape.setLast(IPosition(1,startrows+addrows));
        fieldArray.adjustLastAxis(fieldArrayShape);
        fieldArray(Slicer(start, length)) = inputarray;
    }
    rec.define(field, fieldArray);
}

void ms::completeMissingIfrs(Vector<Int>& inputvec, Record& rec, String field) {
    // missing antennas result in -1; try to fill in
    Vector<Int> recvec = rec.asArrayInt(field);
    rec.removeField(field);
    for (uInt i=0; i<recvec.size(); ++i) 
        if (recvec(i) == -1) recvec(i) = inputvec(i);
    rec.define(field, recvec);
}

Array<Int> ms::getFlagCount(Array<Bool> flagarray, Bool ifraxis) {
    IPosition flagshape = flagarray.shape();
    uInt nPol(flagshape(0)), nChan(flagshape(1)), nIfr(flagshape(2)), nRow(0);
    Array<Int> flagsum;
    if (ifraxis) {
        nRow = flagshape(3);
        flagsum.resize(IPosition(3, nPol, nChan, nIfr));
        for (uInt j=0; j<nPol; j++) {
            for (uInt k=0; k<nChan; k++) {
                for (uInt l=0; l<nIfr; l++) {
                    Int count = 0;
                    for (uInt m=0; m<nRow; m++) {
                        if (flagarray(IPosition(4,j,k,l,m))) count++;
                    }
                    flagsum(IPosition(3,j,k,l)) = count;
                }
            }
        }
    } else {
        flagsum.resize(IPosition(2, nPol, nChan));
        for (uInt j=0; j<nPol; j++) {
            for (uInt k=0; k<nChan; k++) {
                Int count=0;
                for (uInt l=0; l<nIfr; l++) {
                    if (flagarray(IPosition(3,j,k,l))) count++;
                }
                flagsum(IPosition(2,j,k)) = count;
            }
        }
    }
    return flagsum;
}

void ms::getInfoOptions(Vector<Bool> info_options, Record& outputRec) {
    // get [axis_info][time_axis]["MJDseconds"]
    // previously obtained with getdata(["time"])
    String infoName = "axis_info";
    if (!outputRec.isDefined(infoName)) infoName="AXIS_INFO";
    Record axisInfoRec = outputRec.asRecord(infoName);
    outputRec.removeField(infoName);
    Record timeAxisRec = axisInfoRec.asRecord("time_axis");
    axisInfoRec.removeField("time_axis");
    Bool do_ha(info_options(0)), do_last(info_options(1)), do_ut(info_options(2));
    Vector<Double> times = timeAxisRec.asArrayDouble("MJDseconds");

    // set up arrays for values
    IPosition arraySize = IPosition(1,times.size());
    Vector<Double>haArray, lastArray, utArray;

    if (do_ha || do_last) {
        if (do_ha) haArray.resize(arraySize);
        if (do_last) lastArray.resize(arraySize);
        // these depend on field array
        Vector<Int> fields = outputRec.asArrayInt("field_id");
        // set up MSDerivedValues
        MSDerivedValues msd;
        ROMSColumns msCol(*itsSelectedMS);
        msd.setAntennas(msCol.antenna());
        MEpoch ep=msCol.timeMeas()(0);
        // iterate through fields
        for (uInt field=0; field<fields.size(); ++field) {
            Int fieldId = fields(field);
            if (msCol.field().numPoly()(fieldId) == 0)
                msd.setFieldCenter(msCol.field().phaseDirMeas(fieldId));
            else if (msCol.field().numPoly()(fieldId) > 0)
                msd.setFieldCenter(msCol.field().phaseDirMeas(fieldId, times(field)));
            ep.set(MVEpoch(times(field)/C::day));
            msd.setEpoch(ep);
            if (do_ha) haArray(field) = msd.hourAngle()/C::_2pi*C::day;
            if (do_last) lastArray(field) = msd.last().getValue().get();
        }
    }
    if (do_ha) timeAxisRec.define("HA", haArray);
    if (do_last) timeAxisRec.define("LAST", lastArray);

    if (do_ut) {
        utArray.resize(arraySize);
        // this depends on time array
        Double firstTime = times(IPosition(1,1));
        Double startOfDay = C::day* int(firstTime/C::day);
        Array<Double> utArray = times - startOfDay;
        timeAxisRec.define("UT", utArray);
    }

            /*
            // get values for HA field and remove it
            String haName = "ha";
            if (!outputRec.isDefined(haName)) haName="HA";
            Array<Double> haArray = outputRec.asArrayDouble(haName);
            outputRec.removeField(haName);
            // now add it to time_axis Record
            timeAxisRec.define("HA", haArray);
            */

    // Now put Records back into container Records
    axisInfoRec.defineRecord("time_axis", timeAxisRec);
    outputRec.defineRecord(infoName, axisInfoRec);
}

Vector<String> ms::getCorrAxis(vi::VisBuffer2* vb2) {
    Vector<Stokes::StokesTypes> types;
    uInt polSize = wantedpol_p.size();
    if (polSize > 0) {
        // convert Int to StokesTypes
        types.resize(polSize);
        for (uInt i=0; i<polSize; ++i) 
            types(i) = Stokes::type(wantedpol_p(i));
    } else {
        types = vb2->getCorrelationTypesSelected();
    }
    Vector<String> corrNames(types.size());
    for (uInt i=0; i<types.size(); ++i) {
        corrNames(i) = Stokes::name(types(i));
    }
    return corrNames;
}

Record ms::getFreqAxis() {
    ROMSColumns msCol(*itsSelectedMS);
    // get columns from SPECTRAL_WINDOW table
    Vector<Int> spws = getspectralwindows();
    int nSpw = spws.nelements();
    Vector<uInt> uSpws(nSpw); // need uInt for RefRows
    for (int i=0; i<nSpw; ++i) { uSpws(i) = spws(i); }
    Matrix<Double> chanfreqs = msCol.spectralWindow().chanFreq().getColumnCells(RefRows(uSpws));
    Matrix<Double> resolution = msCol.spectralWindow().resolution().getColumnCells(RefRows(uSpws));
    if (chansel_p.size() > 0) {
        // get channel-selected elements
        int nChan = chansel_p[0];
        int start = chansel_p[1];
        int width = chansel_p[2];
        int inc   = chansel_p[3];
        Matrix<Double> selChanFreqs(nChan, nSpw);
        Matrix<Double> selResolution(nChan, nSpw);
        // get values for selected spws and chans
        for (int spw=0; spw<nSpw; ++spw) {
            start = chansel_p[1];
            for (int chan=0; chan<nChan; ++chan) {
                selChanFreqs(chan,spw) = 0.0;
                selResolution(chan,spw) = 0.0;
                // accum channels in width then get average
                for (int n=0; n<width; ++n) {
                    selChanFreqs(chan,spw) += chanfreqs(start+n, spw);
                    selResolution(chan,spw) += resolution(start+n, spw);
                }
                selChanFreqs(chan,spw) /= width;
                selResolution(chan,spw) /= width;
                start += inc;
            }
        }
        chanfreqs.resize();
        chanfreqs = selChanFreqs;
        resolution.resize();
        resolution = selResolution;
    }
    Record freqaxis;
    freqaxis.define("chan_freq", chanfreqs);
    freqaxis.define("resolution", resolution);
    return freqaxis;
}

Record ms::getIfrAxis() {
    Record ifrAxisRec;
    ifrAxisRec.define("ifr_number", ifrnumbers_p);
    // storage
    Vector<String> ifrnames(ifrnumbers_p.size());
    Vector<String> ifrshortnames(ifrnumbers_p.size());
    Vector<Double> baselines(ifrnumbers_p.size(), 0.0);
    // read columns from MS
    ROMSColumns msCol(*itsSelectedMS);
    Vector<String> antnames = msCol.antenna().name().getColumn();
    Array<Double> antpos = msCol.antenna().position().getColumn();

    Int ant1, ant2;
    String name1, name2, shortname1, shortname2;
    for (uInt i=0; i<ifrnumbers_p.size(); ++i) {
        ant1 = ifrnumbers_p(i)/1000;
        ant2 = ifrnumbers_p(i)%1000;
        // name
        name1 = antnames(ant1);
        name2 = antnames(ant2);
        ifrnames(i) = name1 + "-" + name2;
        // shortname
        string::size_type name1size = name1.size(); 
        if (name1size>2) shortname1 = name1.from(name1size-2);
        else shortname1=name1;
        string::size_type name2size = name2.size(); 
        if (name2size>2) shortname2 = name2.from(name2size-2);
        else shortname2=name2;
        ifrshortnames(i) = shortname1 + "-" + shortname2;
        // baseline
        Vector<Double> ant1pos = antpos[ant1];
        Vector<Double> ant2pos = antpos[ant2];
        baselines(i) = sqrt(pow((ant1pos[0] - ant2pos[0]), 2) + 
                         pow((ant1pos[1] - ant2pos[1]), 2) +
                         pow((ant1pos[2] - ant2pos[2]), 2));
    }
    ifrAxisRec.define("ifr_name", ifrnames);
    ifrAxisRec.define("ifr_shortname", ifrshortnames);
    ifrAxisRec.define("baseline", baselines);
    return ifrAxisRec;
}

void ms::addTimeAxis(Record& out) {
    // copy "time" field to ["axis_info"]["time_axis"]["MJDseconds"]
    Array<Double> times = out.asArrayDouble("time");
    Record timeAxisRec;
    timeAxisRec.define("MJDseconds", times);
    String fieldname = "axis_info";
    if (!out.isDefined(fieldname)) fieldname = "AXIS_INFO";
    Record axisInfoRec = out.asRecord(fieldname);
    out.removeField(fieldname);
    axisInfoRec.defineRecord("time_axis", timeAxisRec);
    out.defineRecord(fieldname, axisInfoRec);
}

void ms::getitem(String item, vi::VisBuffer2* vb2, Record& outputRec, bool ifraxis) {
    String itemname = downcase(item);
    Bool fieldExists = outputRec.isDefined(item);
    Record intermediateValue(RecordInterface::Variable);
    MSS::Field fld = MSS::field(itemname);
    switch(fld) {
		case MSS::AMPLITUDE: {
            getitem("data", vb2, intermediateValue, ifraxis);
            Cube<Complex> viscube = intermediateValue.asArrayComplex("data");
            Cube<Float> amp = amplitude(viscube);
            if (fieldExists) addArrayToRec(amp, outputRec, itemname, ifraxis);
            else outputRec.define(itemname, amp);
            }
            break; 
		case MSS::CORRECTED_AMPLITUDE: {
            getitem("corrected_data", vb2, intermediateValue, ifraxis);
            Cube<Complex> corrcube = intermediateValue.asArrayComplex("corrected_data");
            Cube<Float> corramp = amplitude(corrcube);
            if (fieldExists) addArrayToRec(corramp, outputRec, itemname, ifraxis);
            else outputRec.define(itemname, corramp);
            }
            break; 
		case MSS::MODEL_AMPLITUDE: {
            getitem("model_data", vb2, intermediateValue, ifraxis);
            Cube<Complex> modelcube = intermediateValue.asArrayComplex("model_data");
            Cube<Float> modelamp = amplitude(modelcube);
            if (fieldExists) addArrayToRec(modelamp, outputRec, itemname, ifraxis);
            else outputRec.define(itemname, modelamp);
            }
            break; 
		case MSS::RATIO_AMPLITUDE: {
            getitem("ratio_data", vb2, intermediateValue, ifraxis);
            Cube<Complex> ratiocube = intermediateValue.asArrayComplex("ratio_data");
            Cube<Float> ratioamp = amplitude(ratiocube);
            if (fieldExists) addArrayToRec(ratioamp, outputRec, itemname, ifraxis);
            else outputRec.define(itemname, ratioamp);
            }
            break; 
		case MSS::RESIDUAL_AMPLITUDE: {
            getitem("residual_data", vb2, intermediateValue, ifraxis);
            Cube<Complex> rescube = intermediateValue.asArrayComplex("residual_data");
            Cube<Float> resamp = amplitude(rescube);
            if (fieldExists) addArrayToRec(resamp, outputRec, itemname, ifraxis);
            else outputRec.define(itemname, resamp);
                                      }
            break; 
		case MSS::OBS_RESIDUAL_AMPLITUDE: {
            getitem("obs_residual_data", vb2, intermediateValue, ifraxis);
            Cube<Complex> obsrescube = intermediateValue.asArrayComplex("obs_residual_data");
            Cube<Float> obsresamp = amplitude(obsrescube);
            if (fieldExists) addArrayToRec(obsresamp, outputRec, itemname, ifraxis);
            else outputRec.define(itemname, obsresamp);
            }
            break; 
		case MSS::ANTENNA1: {
            Vector<Int> ant1 = vb2->antenna1();
            if (ifraxis) {
                getIfrArray(ant1, vb2);
                if (fieldExists) completeMissingIfrs(ant1, outputRec, itemname);
                else outputRec.define(itemname, ant1);
            } else { 
                if (fieldExists) addArrayToRec(ant1, outputRec, itemname);
                else outputRec.define(itemname, ant1);
            }
            }
            break; 
		case MSS::ANTENNA2: {
            Vector<Int> ant2 = vb2->antenna2();
            if (ifraxis) {
                getIfrArray(ant2, vb2);
                if (fieldExists) completeMissingIfrs(ant2, outputRec, itemname);
                else outputRec.define(itemname, ant2);
            } else {
                if (fieldExists) addArrayToRec(ant2, outputRec, itemname);
                else outputRec.define(itemname, ant2);
            }
                            }
            break; 
		case MSS::ARRAY_ID: {
            Vector<Int> arrayIds = vb2->arrayId();
            if (ifraxis) arrayIds.resize(IPosition(1,1), True);
            if (fieldExists) addArrayToRec(arrayIds, outputRec, itemname);
            else outputRec.define(itemname, arrayIds);
            }
            break; 
		case MSS::DATA: {
            Cube<Complex> viscube = vb2->visCube();
            if (wantedpol_p.size() > 0) convertPoln(viscube, vb2);
            if (ifraxis) getIfrArray(viscube, vb2);
            if (fieldExists) addArrayToRec(viscube, outputRec, itemname, ifraxis);
            else outputRec.define(itemname, viscube);
                        }
            break;
		case MSS::CORRECTED_DATA: {
            Cube<Complex> corrcube = vb2->visCubeCorrected();
            if (wantedpol_p.size() > 0) convertPoln(corrcube, vb2);
            if (ifraxis) getIfrArray(corrcube, vb2);
            if (fieldExists) addArrayToRec(corrcube, outputRec, itemname, ifraxis);
            else outputRec.define(itemname, corrcube);
                                  }
            break; 
		case MSS::MODEL_DATA: {
            Cube<Complex> modelcube = vb2->visCubeModel();
            if (wantedpol_p.size() > 0) convertPoln(modelcube, vb2);
            if (ifraxis) getIfrArray(modelcube, vb2);
            if (fieldExists) addArrayToRec(modelcube, outputRec, itemname, ifraxis);
            else outputRec.define(itemname, modelcube);
            }
            break; 
		case MSS::RATIO_DATA: {
            Cube<Complex> ratiocube = vb2->visCubeCorrected() / vb2->visCubeModel();
            if (wantedpol_p.size() > 0) convertPoln(ratiocube, vb2);
            if (ifraxis) getIfrArray(ratiocube, vb2);
            if (fieldExists) addArrayToRec(ratiocube, outputRec, itemname, ifraxis);
            else outputRec.define(itemname, ratiocube);
            }
            break; 
		case MSS::RESIDUAL_DATA: {
            Cube<Complex> residcube = vb2->visCubeCorrected() - vb2->visCubeModel();
            if (wantedpol_p.size() > 0) convertPoln(residcube, vb2);
            if (ifraxis) getIfrArray(residcube, vb2);
            if (fieldExists) addArrayToRec(residcube, outputRec, itemname, ifraxis);
            else outputRec.define(itemname, residcube);
            }
            break; 
		case MSS::OBS_RESIDUAL_DATA: {
            Cube<Complex> obsrescube = vb2->visCube() - vb2->visCubeModel();
            if (wantedpol_p.size() > 0) convertPoln(obsrescube, vb2);
            if (ifraxis) getIfrArray(obsrescube, vb2);
            if (fieldExists) addArrayToRec(obsrescube, outputRec, itemname, ifraxis);
            else outputRec.define(itemname, obsrescube);
            }
            break; 
        case MSS::DATA_DESC_ID: {
            Vector<Int> ddids = vb2->dataDescriptionIds();
            if (ifraxis) ddids.resize(IPosition(1,1), True);
            if (fieldExists) addArrayToRec(ddids, outputRec, itemname);
            else outputRec.define(itemname, ddids);
            }
            break; 
		case MSS::FEED1: {
            Vector<Int> feed1 = vb2->feed1();
            if (ifraxis) feed1.resize(IPosition(1,1), True);
            if (fieldExists) addArrayToRec(feed1, outputRec, itemname);
            else outputRec.define(itemname, feed1);
            }
            break; 
		case MSS::FEED2: {
            Vector<Int> feed2 = vb2->feed2();
            if (ifraxis) feed2.resize(IPosition(1,1), True);
            if (fieldExists) addArrayToRec(feed2, outputRec, itemname);
            else outputRec.define(itemname, feed2);
            }
            break;
		case MSS::FIELD_ID: {
            Vector<Int> fieldIds = vb2->fieldId();
            if (ifraxis) fieldIds.resize(IPosition(1,1), True);
            if (fieldExists) addArrayToRec(fieldIds, outputRec, itemname);
            else outputRec.define(itemname, fieldIds);
            }
            break; 
		case MSS::FLAG: 
		case MSS::FLAG_SUM: {
            Cube<Bool> flagcube = vb2->flagCube();
            if (ifraxis) getIfrArray(flagcube, vb2);
            if (fieldExists) addArrayToRec(flagcube, outputRec, itemname, ifraxis);
            else outputRec.define(itemname, flagcube);
            }
            break; 
		case MSS::FLAG_ROW: {
            Vector<Bool> flagrow = vb2->flagRow();
            if (ifraxis) getIfrArray(flagrow, vb2);
            if (fieldExists) addArrayToRec(flagrow, outputRec, itemname, ifraxis);
            else outputRec.define(itemname, flagrow);
            }
            break; 
		case MSS::FLOAT_DATA: {
            Cube<Float> floatcube = vb2->visCubeFloat();
            if (ifraxis) getIfrArray(floatcube, vb2);
            if (fieldExists) addArrayToRec(floatcube, outputRec, itemname, ifraxis);
            else outputRec.define(itemname, floatcube);
            }
            break;
		case MSS::IFR_NUMBER: {
            if (ifraxis) {
                if (!fieldExists) outputRec.define(itemname, ifrnumbers_p);
            } else {
                Vector<Int> ifrs = getbaselines(vb2);
                if (fieldExists) addArrayToRec(ifrs, outputRec, itemname);
                else outputRec.define(itemname, ifrs);
            }
            }
            break;
		case MSS::IMAGINARY: {
            getitem("data", vb2, intermediateValue, ifraxis);
            Cube<Complex> viscube = intermediateValue.asArrayComplex("data");
            Cube<Float> imagnry = imag(viscube);
            if (fieldExists) addArrayToRec(imagnry, outputRec, itemname, ifraxis);
            else outputRec.define(itemname, imagnry);
            }
            break; 
		case MSS::CORRECTED_IMAGINARY: {
            getitem("corrected_data", vb2, intermediateValue, ifraxis);
            Cube<Complex> corrcube = intermediateValue.asArrayComplex("corrected_data");
            Cube<Float> corrimag = imag(corrcube);
            if (fieldExists) addArrayToRec(corrimag, outputRec, itemname, ifraxis);
            else outputRec.define(itemname, corrimag);
            }
            break; 
		case MSS::MODEL_IMAGINARY: {
            getitem("model_data", vb2, intermediateValue, ifraxis);
            Cube<Complex> modelcube = intermediateValue.asArrayComplex("model_data");
            Cube<Float> modelimag = imag(modelcube);
            if (fieldExists) addArrayToRec(modelimag, outputRec, itemname, ifraxis);
            else outputRec.define(itemname, modelimag);
                                   }
            break; 
		case MSS::RATIO_IMAGINARY: {
            getitem("ratio_data", vb2, intermediateValue, ifraxis);
            Cube<Complex> ratiocube = intermediateValue.asArrayComplex("ratio_data");
            Cube<Float> ratioimag = imag(ratiocube);
            if (fieldExists) addArrayToRec(ratioimag, outputRec, itemname, ifraxis);
            else outputRec.define(itemname, ratioimag);
            }
            break; 
		case MSS::RESIDUAL_IMAGINARY: {
            getitem("residual_data", vb2, intermediateValue, ifraxis);
            Cube<Complex> rescube = intermediateValue.asArrayComplex("residual_data");
            Cube<Float> resimag = imag(rescube);
            if (fieldExists) addArrayToRec(resimag, outputRec, itemname, ifraxis);
            else outputRec.define(itemname, resimag);
            }
            break; 
		case MSS::OBS_RESIDUAL_IMAGINARY: {
            getitem("obs_residual_data", vb2, intermediateValue, ifraxis);
            Cube<Complex> obsrescube = intermediateValue.asArrayComplex("obs_residual_data");
            Cube<Float> obsresimag = imag(obsrescube);
            if (fieldExists) addArrayToRec(obsresimag, outputRec, itemname, ifraxis);
            else outputRec.define(itemname, obsresimag);
            }
            break; 
		case MSS::PHASE: {
            getitem("data", vb2, intermediateValue, ifraxis);
            Cube<Complex> viscube = intermediateValue.asArrayComplex("data");
            Cube<Float> visphase = phase(viscube);
            if (fieldExists) addArrayToRec(visphase, outputRec, itemname, ifraxis);
            else outputRec.define(itemname, visphase);
            }
            break; 
		case MSS::CORRECTED_PHASE: {
            getitem("corrected_data", vb2, intermediateValue, ifraxis);
            Cube<Complex> corrcube = intermediateValue.asArrayComplex("corrected_data");
            Cube<Float> corrphase = phase(corrcube);
            if (fieldExists) addArrayToRec(corrphase, outputRec, itemname, ifraxis);
            else outputRec.define(itemname, corrphase);
                                   }
            break; 
		case MSS::MODEL_PHASE: {
            getitem("model_data", vb2, intermediateValue, ifraxis);
            Cube<Complex> modelcube = intermediateValue.asArrayComplex("model_data");
            Cube<Float> modelphase = phase(modelcube);
            if (fieldExists) addArrayToRec(modelphase, outputRec, itemname, ifraxis);
            else outputRec.define(itemname, modelphase);
            }
            break; 
		case MSS::RATIO_PHASE: {
            getitem("ratio_data", vb2, intermediateValue, ifraxis);
            Cube<Complex> ratiocube = intermediateValue.asArrayComplex("ratio_data");
            Cube<Float> ratiophase = phase(ratiocube);
            if (fieldExists) addArrayToRec(ratiophase, outputRec, itemname, ifraxis);
            else outputRec.define(itemname, ratiophase);
            }
            break; 
		case MSS::RESIDUAL_PHASE: {
            getitem("residual_data", vb2, intermediateValue, ifraxis);
            Cube<Complex> rescube = intermediateValue.asArrayComplex("residual_data");
            Cube<Float> resphase = phase(rescube);
            if (fieldExists) addArrayToRec(resphase, outputRec, itemname, ifraxis);
            else outputRec.define(itemname, resphase);
            }
            break; 
		case MSS::OBS_RESIDUAL_PHASE: {
            getitem("obs_residual_data", vb2, intermediateValue, ifraxis);
            Cube<Complex> obsrescube = intermediateValue.asArrayComplex("obs_residual_data");
            Cube<Float> obsresphase = phase(obsrescube);
            if (fieldExists) addArrayToRec(obsresphase, outputRec, itemname, ifraxis);
            else outputRec.define(itemname, obsresphase);
            }
            break; 
		case MSS::REAL: {
            getitem("data", vb2, intermediateValue, ifraxis);
            Cube<Complex> viscube = intermediateValue.asArrayComplex("data");
            Cube<Float> visreal = real(viscube);
            if (fieldExists) addArrayToRec(visreal, outputRec, itemname, ifraxis);
            else outputRec.define(itemname, visreal);
            }
            break; 
		case MSS::CORRECTED_REAL: {
            getitem("corrected_data", vb2, intermediateValue, ifraxis);
            Cube<Complex> corrcube = intermediateValue.asArrayComplex("corrected_data");
            Cube<Float> corrreal = real(corrcube);
            if (fieldExists) addArrayToRec(corrreal, outputRec, itemname, ifraxis);
            else outputRec.define(itemname, corrreal);
                                  }
            break; 
		case MSS::MODEL_REAL: {
            getitem("model_data", vb2, intermediateValue, ifraxis);
            Cube<Complex> modelcube = intermediateValue.asArrayComplex("model_data");
            Cube<Float> modelreal = real(modelcube);
            if (fieldExists) addArrayToRec(modelreal, outputRec, itemname, ifraxis);
            else outputRec.define(itemname, modelreal);
            }
            break; 
		case MSS::RATIO_REAL: {
            getitem("ratio_data", vb2, intermediateValue, ifraxis);
            Cube<Complex> ratiocube = intermediateValue.asArrayComplex("ratio_data");
            Cube<Float> ratioreal = real(ratiocube);
            if (fieldExists) addArrayToRec(ratioreal, outputRec, itemname, ifraxis);
            else outputRec.define(itemname, ratioreal);
            }
            break; 
		case MSS::RESIDUAL_REAL: {
            getitem("residual_data", vb2, intermediateValue, ifraxis);
            Cube<Complex> rescube = intermediateValue.asArrayComplex("residual_data");
            Cube<Float> resreal = real(rescube);
            if (fieldExists) addArrayToRec(resreal, outputRec, itemname, ifraxis);
            else outputRec.define(itemname, resreal);
            }
            break; 
		case MSS::OBS_RESIDUAL_REAL: {
            getitem("obs_residual_data", vb2, intermediateValue, ifraxis);
            Cube<Complex> obsrescube = intermediateValue.asArrayComplex("obs_residual_data");
            Cube<Float> obsresreal = real(obsrescube);
            if (fieldExists) addArrayToRec(obsresreal, outputRec, itemname, ifraxis);
            else outputRec.define(itemname, obsresreal);
            }
            break; 
		case MSS::SCAN_NUMBER: {
            Vector<Int> scans = vb2->scan();
            if (ifraxis) scans.resize(IPosition(1,1), True);
            if (fieldExists) addArrayToRec(scans, outputRec, itemname);
            else outputRec.define(itemname, scans);
            }
            break; 
		case MSS::SIGMA: {
            Matrix<Float> sigma = vb2->sigma();
            if (ifraxis) getIfrArray(sigma, vb2);
            if (fieldExists) addArrayToRec(sigma, outputRec, itemname, ifraxis);
            else outputRec.define(itemname, sigma);
            }
            break; 
		case MSS::TIME: {
            Vector<Double> times = vb2->time();
            if (ifraxis) times.resize(IPosition(1,1), True);
            if (fieldExists) addArrayToRec(times, outputRec, itemname);
            else outputRec.define(itemname, times);
            }
            break; 
		case MSS::UVW: {
            Matrix<Double> uvw = vb2->uvw();
            if (ifraxis) getIfrArray(uvw, vb2);
            if (fieldExists) addArrayToRec(uvw, outputRec, itemname, ifraxis);
            else outputRec.define(itemname, uvw);
            }
            break; 
		case MSS::U: {
            Vector<Double> u = vb2->uvw().row(0);
            if (ifraxis) getIfrArray(u, vb2);
            if (fieldExists) addArrayToRec(u, outputRec, itemname, ifraxis);
            else outputRec.define(itemname, u);
            }
            break; 
		case MSS::V: {
            Vector<Double> v = vb2->uvw().row(1);
            if (ifraxis) getIfrArray(v, vb2);
            if (fieldExists) addArrayToRec(v, outputRec, itemname, ifraxis);
            else outputRec.define(itemname, v);
            }
            break; 
		case MSS::W: {
            Vector<Double> w = vb2->uvw().row(2);
            if (ifraxis) getIfrArray(w, vb2);
            if (fieldExists) addArrayToRec(w, outputRec, itemname, ifraxis);
            else outputRec.define(itemname, w);
            }
            break; 
		case MSS::UVDIST: {
            Array<Double> u(vb2->uvw().row(0));
            Array<Double> v(vb2->uvw().row(1));
            Vector<Double> uvdist = sqrt(u*u+v*v);
            if (ifraxis) getIfrArray(uvdist, vb2);
            if (fieldExists) addArrayToRec(uvdist, outputRec, itemname, ifraxis);
            else outputRec.define(itemname, uvdist);
            }
            break; 
		case MSS::WEIGHT: {
            Matrix<Float> weight = vb2->weight();
            if (ifraxis) getIfrArray(weight, vb2);
            if (fieldExists) addArrayToRec(weight, outputRec, itemname, ifraxis);
            else outputRec.define(itemname, weight);
            }
            break; 
		case MSS::AXIS_INFO: {
            if (!fieldExists) {
                // corr_, freq_, ifr_axis same for all iterations!
                Record info(RecordInterface::Variable);
                Vector<String> corrAxis = getCorrAxis(vb2);
                // corr_axis
                info.define("corr_axis", corrAxis);
                // freq_axis
                Record freqAxis = getFreqAxis();
                info.defineRecord("freq_axis", freqAxis);
                if (ifraxis) {
                    // ifr_axis
                    Record ifrAxis = getIfrAxis();
                    info.defineRecord("ifr_axis", ifrAxis);
                }
                outputRec.defineRecord(itemname, info);
            } 
            }
            break;
		case MSS::UNDEFINED:
		default:
			*itsLog << LogIO::WARN << "Unrecognized field or field not implemented: "
                    << itemname << LogIO::POST;
			break;
    }
}

bool
ms::putdata(const ::casac::record& items)
{
	Bool rstat(False);
	try {
		if(!detached()){
			Record *myTmp = toRecord(items);
			rstat = itsSel->putData(*myTmp);
			delete myTmp;
		}
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(True);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(True);
	return rstat;
}

bool
ms::putdata2(const ::casac::record& items)
{
	*itsLog << LogOrigin("ms", "putdata2");
	bool rstat(false);
	try {
		if(!detached()){
            // run some checks!
            if (nrow2(True)==0) {
                *itsLog << LogIO::SEVERE << "Selected Table is empty - use selectinit"
				    << LogIO::POST;
		        return false;
	        }
	        *itsLog << LogOrigin("ms", "putdata2");
            if (!ready2write_()) {
                *itsLog << LogIO::SEVERE << "MeasurementSet is not writable; use open with nomodify=False" << LogIO::POST;
		        return false;
            }
            if (chansel_p.size()>0 && chansel_p[2]>0) {
                *itsLog << LogIO::SEVERE << "Channel averaging not supported when writing data" << LogIO::POST;
		        return false;
            }
            if (doingAveraging_p) {
                *itsLog << LogIO::SEVERE << "Averaging not supported when writing data, cannot change data shape" << LogIO::POST;
		        return false;
            }
            if (wantedpol_p.size() > 0) {
                *itsLog << LogIO::SEVERE << "Polarization conversion not supported when writing data" << LogIO::POST;
		        return false;
            }

			Record* putRecord = toRecord(items);
            // check for valid fields for putdata, issue warning once
            Vector<Bool> allowed(putRecord->nfields());
            for (uInt i=0; i<putRecord->nfields(); ++i) {
                String fieldname = downcase(putRecord->name(i));
                allowed(i) = allowPut(fieldname);
            }
            if (anyTrue(allowed)) {
                Int startrow(0), subchunk(0);
                if (itsVI2) {
                    vi::VisBuffer2* vb2 = itsVI2->getVisBuffer();
                    for (itsVI2->origin(); itsVI2->more(); itsVI2->next()) {
                        for (uInt i=0; i<putRecord->nfields(); ++i) {
                            if (allowed(i)) putitem(i, vb2, *putRecord,
                                startrow, subchunk);
                        }
                        startrow += vb2->nRows();
                        ++subchunk;
                    }
                } else {
                    if (checkinit()) {
                        *itsLog << LogOrigin("ms", "putdata2");
                        std::vector<std::string> columns = {""};
                        if (iterinit2(columns, 0.0, 0, false)) {
                            iterorigin2();
                            *itsLog << LogOrigin("ms", "putdata2");
                            vi::VisBuffer2* vb2 = itsVI2->getVisBuffer();
                            for (itsVI2->origin(); itsVI2->more(); itsVI2->next()) {
                                for (uInt i=0; i<putRecord->nfields(); ++i) {
                                    if (allowed(i)) putitem(i, vb2, *putRecord,
                                        startrow, subchunk);
                                }
                                startrow += vb2->nRows();
                                ++subchunk;
                            }
                            while(iternext2()) {
                                *itsLog << LogOrigin("ms", "putdata2");
                                for (itsVI2->origin(); itsVI2->more(); itsVI2->next()) {
                                    for (uInt i=0; i<putRecord->nfields(); ++i) {
                                        if (allowed(i)) putitem(i, vb2, *putRecord,
                                            startrow, subchunk);
                                    }
                                    startrow += vb2->nRows();
                                    ++subchunk;
                                }
                            }
                            iterend2();
                            *itsLog << LogOrigin("ms", "putdata2");
                        } // iterinit
                    } // checkinit
                } // else
                rstat = true;
            }
			delete putRecord;
		}
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return rstat;
}

bool ms::allowPut(String fieldname) {
    ROMSColumns msc(*itsSelectedMS);
    MSS::Field fld = MSS::field(fieldname);
    bool allow(true);
    switch(fld) {
        case MSS::CORRECTED_DATA:
            if (msc.correctedData().isNull()) {
                *itsLog << LogIO::WARN << "Cannot write " << fieldname << ", column does not exist" << LogIO::POST;
                allow = false;
            }
            break; 
        case MSS::MODEL_DATA:
            if (msc.modelData().isNull()) {
                *itsLog << LogIO::WARN << "Cannot write " << fieldname << ", column does not exist" << LogIO::POST;
                allow = false;
            }
            break; 
        case MSS::FLOAT_DATA:
            if (msc.floatData().isNull()) {
                *itsLog << LogIO::WARN << "Cannot write " << fieldname << ", column does not exist" << LogIO::POST;
                allow = false;
            }
            break; 
        case MSS::DATA:
        case MSS::FLAG:
        case MSS::FLAG_ROW:
        case MSS::SIGMA:
        case MSS::WEIGHT:
            break;
        case MSS::UNDEFINED:
        default:
            *itsLog << LogIO::WARN << "Invalid field in putdata ignored: " << fieldname << LogIO::POST;
            allow = false;
    }
    return allow;
}

void ms::putitem(uInt fieldId, vi::VisBuffer2* vb2, Record& inputRecord,
    Int startrow, Int subchunk) {
    String fieldname = downcase(inputRecord.name(fieldId));
    MSS::Field fld = MSS::field(fieldname);
    switch (fld) {
        case MSS::DATA:
        case MSS::CORRECTED_DATA:
        case MSS::MODEL_DATA: {
            Array<Complex> data = inputRecord.toArrayComplex(fieldname);
            IPosition shape = data.shape();
            // get dataToWrite cube from data array
            Cube<Complex> dataToWrite;
            if (shape.size()==3) {
                // Write nrows of array at a time
                IPosition start(3,0,0,startrow), 
                          length(3,shape(0), shape(1), vb2->nRows()), 
                          stride(3,1,1,1);
                Slicer slicer(start, length, stride);
                dataToWrite = data(slicer);
            } else if (shape.size()==4) { // ifraxis
                // Each ifraxis is a subchunk's worth of data
                dataToWrite = data[subchunk];
                // reorder/resize chunk according to MS ifr and vb shape
                ifrToArray(dataToWrite, vb2);
            }
            switch (fld) {
                case MSS::DATA:
                    itsVI2->getImpl()->writeVisObserved(dataToWrite);
                    break;
                case MSS::CORRECTED_DATA:
                    itsVI2->getImpl()->writeVisCorrected(dataToWrite);
                    break;
                case MSS::MODEL_DATA:
                    itsVI2->getImpl()->writeVisModel(dataToWrite);
                    break;
                default:
                    break;
            }
        }
        break; 
        case MSS::FLOAT_DATA: {
            Array<Float> data = inputRecord.toArrayFloat(fieldname);
            IPosition shape = data.shape();
            Cube<Float> dataToWrite;
            if (shape.size()==3) {
                IPosition start(3,0,0,startrow), 
                          length(3,shape(0), shape(1), vb2->nRows()), 
                          stride(3,1,1,1);
                Slicer slicer(start, length, stride);
                dataToWrite = data(slicer);
            } else if (shape.size()==4) { // ifraxis
                dataToWrite = data[subchunk];
                ifrToArray(dataToWrite, vb2);
            }
            vb2->setVisCubeFloat(dataToWrite);
            itsVI2->getImpl()->writeBackChanges(vb2);
        }
        break;
        case MSS::FLAG: {
            Array<Bool> flags = inputRecord.toArrayBool(fieldname);
            IPosition shape = flags.shape();
            Cube<Bool> dataToWrite;
            if (shape.size()==3) {
                IPosition start(3,0,0,startrow), 
                          length(3,shape(0), shape(1), vb2->nRows()), 
                          stride(3,1,1,1);
                Slicer slicer(start, length, stride);
                dataToWrite = flags(slicer);
            } else if (shape.size()==4) { // ifraxis
                dataToWrite = flags[subchunk];
                ifrToArray(dataToWrite, vb2);
            }
            itsVI2->getImpl()->writeFlag(dataToWrite);
        }
        break;
        case MSS::FLAG_ROW: {
            Array<Bool> flagrow = inputRecord.toArrayBool(fieldname);
            IPosition shape = flagrow.shape();
            Vector<Bool> dataToWrite;
            if (shape.size()==1) {
                IPosition start(1,startrow), 
                          length(1,vb2->nRows()), 
                          stride(1,1);
                Slicer slicer(start, length, stride);
                dataToWrite = flagrow(slicer);
            } else if (shape.size()==2) { // ifraxis
                dataToWrite = flagrow[subchunk];
                ifrToArray(dataToWrite, vb2);
            }
            itsVI2->getImpl()->writeFlagRow(dataToWrite);
        }
        break;
        case MSS::SIGMA:
        case MSS::WEIGHT: {
            Array<Float> data = inputRecord.toArrayFloat(fieldname);
            IPosition shape = data.shape();
            Matrix<Float> dataToWrite;
            if (shape.size()==2) {
                IPosition start(2,0,startrow), 
                          length(2,shape(0), vb2->nRows()), 
                          stride(2,1,1);
                Slicer slicer(start, length, stride);
                dataToWrite = data(slicer);
            } else if (shape.size()==3) { // ifraxis
                dataToWrite = data[subchunk];
                ifrToArray(dataToWrite, vb2);
            }
            switch (fld) {
                case MSS::SIGMA:
                    itsVI2->getImpl()->writeSigma(dataToWrite);
                    break;
                case MSS::WEIGHT:
                    itsVI2->getImpl()->writeWeight(dataToWrite);
                    break;
                default:
                    break;
            }
        }
        break;
        case MSS::UNDEFINED:
        default: {
            *itsLog << LogIO::WARN << "Unrecognized field in putdata ignored: " << fieldname << LogIO::POST;
        }
        break;
    }
}

bool
ms::concatenate(const std::string& msfile, const ::casac::variant& freqtol, const ::casac::variant& dirtol, const float weightscale,
                const int handling, const std::string& destmsfile, const bool respectname)
{
	Bool rstat(false);
	try {
		if(!detached()){
			*itsLog << LogOrigin("ms", "concatenate");

			if (!Table::isReadable(msfile)) {
				*itsLog << "Cannot read the measurement set called " << msfile
				        << LogIO::EXCEPTION;
			}
			if (DOos::totalSize(msfile, true) >
			    DOos::freeSpace(Vector<String>(1, itsMS->tableName()), true)(0)) {
				*itsLog << "There does not appear to be enough free disk space "
				        << "(on the filesystem containing " << itsMS->tableName()
				        << ") for the concatantion to succeed." << LogIO::EXCEPTION;
			}

			const MeasurementSet appendedMS(msfile);

			addephemcol(appendedMS); // add EPHEMERIS_ID column to FIELD table of itsMS if necessary

			MSConcat mscat(*itsMS);

			Quantum<Double> dirtolerance;
			Quantum<Double> freqtolerance;
			if(freqtol.toString().empty()){
				freqtolerance=casacore::Quantity(1.0,"Hz");
			}
			else{
				freqtolerance=casaQuantity(freqtol);
			}
			if(dirtol.toString().empty()){
				dirtolerance=casacore::Quantity(1.0, "mas");
			}
			else{
				dirtolerance=casaQuantity(dirtol);
			}

			mscat.setTolerance(freqtolerance, dirtolerance);
			mscat.setRespectForFieldName(respectname);
			mscat.setWeightScale(weightscale);
			mscat.concatenate(appendedMS, static_cast<uint>(handling), destmsfile);

			String message = String(msfile) + " appended to " + itsMS->tableName();
			ostringstream param;
			param << "msfile='" << msfile
			      << "' freqTol='" << casaQuantity(freqtol)
			      << "' dirTol='" << casaQuantity(dirtol)
			      << "' respectname='" << respectname
			      << "' handling= " << handling
			      << " destmsfile='" << destmsfile << "'";
			String paramstr=param.str();
			writehistory(std::string(message.data()), std::string(paramstr.data()),
			             std::string("ms::concatenate()"), msfile, "ms");
		}
		rstat = true;
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return rstat;
}

bool
ms::testconcatenate(const std::string& msfile, const ::casac::variant& freqtol, const ::casac::variant& dirtol,
                    const bool respectname)
{
	Bool rstat(false);
	try {
		if(!detached()){
			*itsLog << LogOrigin("ms", "testconcatenate");

			*itsLog << LogIO::NORMAL << "*** Note: this method does _not_ merge the Main and Pointing tables!"
			        << LogIO::POST;

			if (!Table::isReadable(msfile)) {
				*itsLog << "Cannot read the measurement set called " << msfile
				        << LogIO::EXCEPTION;
			}

			const MeasurementSet appendedMS(msfile);

			addephemcol(appendedMS); // add EPHEMERIS_ID column to FIELD table of itsMS if necessary

			MSConcat mscat(*itsMS);
			Quantum<Double> dirtolerance;
			Quantum<Double> freqtolerance;
			if(freqtol.toString().empty()){
				freqtolerance=casacore::Quantity(1.0,"Hz");
			}
			else{
				freqtolerance=casaQuantity(freqtol);
			}
			if(dirtol.toString().empty()){
				dirtolerance=casacore::Quantity(1.0, "mas");
			}
			else{
				dirtolerance=casaQuantity(dirtol);
			}

			mscat.setTolerance(freqtolerance, dirtolerance);
			mscat.setRespectForFieldName(respectname);
			mscat.concatenate(appendedMS, 3); // 3 meaning "don't concatenate Main and Pointing table"

			String message = "Subtables from "+String(msfile) + " appended to those from " + itsMS->tableName();
			ostringstream param;
			param << "msfile= " << msfile
			      << " freqTol='" << casaQuantity(freqtol) << "' dirTol='"
			      << casaQuantity(dirtol) << "'" << "' respectname='" << respectname;
			String paramstr=param.str();
			writehistory(std::string(message.data()), std::string(paramstr.data()),
			             std::string("ms::testconcatenate()"), msfile, "ms");
			itsMS->flush(true);
		}
		rstat = true;
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return rstat;
}

bool
ms::virtconcatenate(const std::string& msfile, const std::string& auxfile, const ::casac::variant& freqtol,
                    const ::casac::variant& dirtol, const float weightscale, const bool respectname)
{
	Bool rstat(false);
	try {
		if(!detached()){
			*itsLog << LogOrigin("ms", "virtconcatenate");

			if (!Table::isReadable(msfile)) {
				*itsLog << "Cannot read the measurement set called " << msfile
				        << LogIO::EXCEPTION;
			}

			MeasurementSet appendedMS(msfile, Table::Update);

			if (!appendedMS.isWritable()) {
				*itsLog << "Cannot write to the measurement set called " << msfile
				        << LogIO::EXCEPTION;
			}

			addephemcol(appendedMS); // add EPHEMERIS_ID to FIELD table of itsMS if necessary

			MSConcat mscat(*itsMS);
			Quantum<Double> dirtolerance;
			Quantum<Double> freqtolerance;
			if(freqtol.toString().empty()){
				freqtolerance=casacore::Quantity(1.0,"Hz");
			}
			else{
				freqtolerance=casaQuantity(freqtol);
			}
			if(dirtol.toString().empty()){
				dirtolerance=casacore::Quantity(1.0, "mas");
			}
			else{
				dirtolerance=casaQuantity(dirtol);
			}

			mscat.setTolerance(freqtolerance, dirtolerance);
			mscat.setRespectForFieldName(respectname);
			mscat.setWeightScale(weightscale);

			mscat.virtualconcat(appendedMS, true, casacore::String(auxfile));

			String message = String(msfile) + " virtually concatenated with " + itsMS->tableName();
			ostringstream param;
			param << "msfile='" << msfile
			      << "' freqTol='" << casaQuantity(freqtol)
			      << "' dirTol='" << casaQuantity(dirtol)
			      << "' respectname='" << respectname << "'";
			String paramstr=param.str();
			writehistory(std::string(message.data()), std::string(paramstr.data()),
			             std::string("ms::virtconcatenate()"), msfile, "ms");
		}
		rstat = true;
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return rstat;
}


bool
ms::timesort(const std::string& msname)
{
	Bool rstat(false);
	try {
		if(!detached()){
			*itsLog << LogOrigin("ms", "timesort");

			if (DOos::totalSize(itsMS->tableName(), true) >
			    DOos::freeSpace(Vector<String>(1, itsMS->tableName()), true)(0)) {
				*itsLog << "There does not appear to be enough free disk space "
				        << "(on the filesystem containing " << itsMS->tableName()
				        << ") for the sorting to succeed." << LogIO::EXCEPTION;
			}

			{
				String originalName = itsMS->tableName();
				itsMS->flush();
				MeasurementSet sortedMS = itsMS->sort("TIME");

				if (msname.length() == 0) { // no name given, sort and don't keep copy
					// make deep copy
					sortedMS.deepCopy(originalName+".sorted", Table::New);
					// close reference table
					sortedMS = MeasurementSet();
					// close original MS
					close();
					// rename copy to original name
					Table newMSmain(originalName+".sorted", Table::Update);
					newMSmain.rename(originalName, Table::New);    // will also delete original table
					newMSmain = Table();
					// reopen
					open(originalName,  Table::Update, false);
					*itsLog << LogOrigin("ms", "timesort");
					String message = "Sorted by TIME in ascending order.";
					writehistory(std::string(message.data()), "", std::string("ms::timesort()"), originalName, "ms");
					*itsLog << LogIO::NORMAL << "Sorted main table of " << originalName << " by TIME."
					        << LogIO::POST;
				}
				else { // sort into a new MS
					sortedMS.deepCopy(msname, Table::New);
					String message = "Generated from " + originalName + " by sorting by TIME in ascending order.";
					writehistory(std::string(message.data()), "", std::string("ms::timesort()"), msname, "ms");

					*itsLog << LogIO::NORMAL << "Sorted main table of " << originalName << " by TIME and stored it in "
					        << msname << " ."<< LogIO::POST;

				}
			}

			rstat = true;
		}

	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return rstat;
}

bool
ms::sort(const std::string& msname,const std::vector<std::string>& columns)
{
	Bool rstat(false);
	try {
		if(!detached()){
			*itsLog << LogOrigin("ms", "sort");

			if (DOos::totalSize(itsMS->tableName(), true) >
			    DOos::freeSpace(Vector<String>(1, itsMS->tableName()), true)(0)) {
				*itsLog << "There does not appear to be enough free disk space "
				        << "(on the filesystem containing " << itsMS->tableName()
				        << ") for the sorting to succeed." << LogIO::EXCEPTION;
			}

			{

				// Prepare columns for Table::sort method
				Block<String> cols(columns.size());
				for (uInt col=0;col<columns.size();col++)
				{
					cols[col] = columns.at(col);
				}

				// Prepare columns for loggin info
				ostringstream oss;
				oss.clear();
				oss << columns;
				String strCols(oss.str());

				String originalName = itsMS->tableName();
				itsMS->flush();

				MeasurementSet sortedMS = itsMS->sort(cols);

				if (msname.length() == 0) { // no name given, sort and don't keep copy
					// make deep copy
					sortedMS.deepCopy(originalName+".sorted", Table::New);
					// close reference table
					sortedMS = MeasurementSet();
					// close original MS
					close();
					// rename copy to original name
					Table newMSmain(originalName+".sorted", Table::Update);
					newMSmain.rename(originalName, Table::New);    // will also delete original table
					newMSmain = Table();
					// reopen
					open(originalName,  Table::Update, false);
					*itsLog << LogOrigin("ms", "sort");
					String message = "Sorted by " + oss.str() + " in ascending order.";
					writehistory(std::string(message.data()), "", std::string("ms::sort()"), originalName, "ms");
					*itsLog << LogIO::NORMAL << "Sorted main table of " << originalName << " by  " + strCols + " ."
					        << LogIO::POST;
				}
				else { // sort into a new MS
					sortedMS.deepCopy(msname, Table::New);
					String message = "Generated from " + originalName + " by sorting by  " + strCols + "  in ascending order.";
					writehistory(std::string(message.data()), "", std::string("ms::sort()"), msname, "ms");

					*itsLog << LogIO::NORMAL << "Sorted main table of " << originalName << " by  " + strCols + "  and stored it in "
					        << msname << " ."<< LogIO::POST;

				}
			}

			rstat = true;
		}

	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return rstat;
}

bool ms::contsub(const std::string& outputms,    const ::casac::variant& fitspw,
                 const int fitorder,             const std::string& combine,
                 const ::casac::variant& spw,    const ::casac::variant& unionspw,
                 const ::casac::variant& field,  const ::casac::variant& scan,
                 const std::string&      intent, const std::string& correlation,
                 const std::string&      obs,    const std::string& whichcol)
{
	Bool rstat(false);

	try{
		*itsLog << LogOrigin("ms", "contsub");

		SubMS subtractor(*itsMS);
		*itsLog << LogIO::NORMAL2 << "Sub MS created" << LogIO::POST;
		String t_field(m1toBlankCStr_(field));
		String t_fitspw(m1toBlankCStr_(fitspw));
		String t_spw(m1toBlankCStr_(spw));
		String t_unionspw(m1toBlankCStr_(unionspw));
		if(t_spw == "")   // MSSelection doesn't respond well to "", and setting it
			t_spw = "*";    // at the XML level does not work.
		if(t_spw != "*" && t_spw != t_unionspw){
			*itsLog << LogIO::WARN
			        << "The spws in the output will be remapped according to "
			        << t_unionspw << ", not " << t_spw
			        << LogIO::POST;
			*itsLog << LogIO::WARN
			        << "This only affects the numbering of the spws, not their validity or frequencies."
			        << LogIO::POST;
		}
		String t_scan    = toCasaString(scan);
		String t_intent  = toCasaString(intent);
		String t_obs     = toCasaString(obs);
		String t_correlation = upcase(correlation);

		if(!subtractor.setmsselect(t_unionspw, t_field,
		                           "",                      // antenna
		                           t_scan,
		                           "",                      // uvrange
		                           "",                      // taql
		                           Vector<Int>(1, 1),       // step
		                           "",                      // subarray
		                           t_correlation,
		                           t_intent, t_obs)){
			*itsLog << LogIO::SEVERE
			        << "Error selecting data."
			        << LogIO::POST;
			return false;
		}

		String t_outputms(outputms);
		String t_whichcol(whichcol);
		Vector<Int> t_tileshape(1, 0);
		const String t_combine = downcase(combine);

		subtractor.setFitOrder(fitorder);
		subtractor.setFitSpw(t_fitspw);
		subtractor.setFitOutSpw(t_spw);
		subtractor.setWantCont(False);

		if(!subtractor.makeSubMS(t_outputms, t_whichcol, t_tileshape, t_combine)){
			*itsLog << LogIO::SEVERE
			        << "Error subtracting from " << itsMS->tableName() << " to "
			        << t_outputms
			        << LogIO::POST;
			return false;
		}

		*itsLog << LogIO::NORMAL2 << "Continuum subtracted" << LogIO::POST;

		{// Update HISTORY table of newly created MS
			String message = "Continuum subtracted from " + itsMS->tableName();
			ostringstream param;
			param << "fieldids=" << t_field << " spwids=" << t_spw
			      << " whichcol='" << whichcol << "'";
			String paramstr=param.str();
			writehistory(message, paramstr, "ms::contsub()", outputms, "ms");
		}

		rstat = true;
	}
	catch (AipsError x){
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return rstat;
}

bool ms::statwt(const bool dorms,                const bool /*byantenna*/,
                const bool /*sepacs*/,               const ::casac::variant& fitspw,
                const ::casac::variant& /*fitcorr*/, const std::string& combine,
                const ::casac::variant& timebin, const int minsamp,
                const ::casac::variant& field,   const ::casac::variant& spw,
                const ::casac::variant& baseline, const std::string& timerange,
                const ::casac::variant& scan,    const std::string&      intent,
                const ::casac::variant& subarray,const std::string& correlation,
                const std::string&      obs,     const std::string& datacol)
{
	Bool rstat(false);

	try {
		*itsLog << LogOrigin("ms", "statwt");

		Reweighter reweighter(itsMS->tableName(), dorms, minsamp);

		*itsLog << LogIO::NORMAL2 << "Reweighter created" << LogIO::POST;
		String t_field(m1toBlankCStr_(field));
		String t_fitspw(m1toBlankCStr_(fitspw));
		String t_spw(m1toBlankCStr_(spw));
		if(t_spw == "")   // MSSelection doesn't respond well to "", and setting it
			t_spw = "*";    // at the XML level does not work.

		String t_baseline = toCasaString(baseline);
		String t_scan    = toCasaString(scan);
		String t_intent  = toCasaString(intent);
		String t_obs     = toCasaString(obs);
		String t_subarray    = toCasaString(subarray);
		String t_correlation = upcase(correlation);

		if(!reweighter.setmsselect(t_fitspw, t_spw,
		                           t_field,
		                           t_baseline,                      // antenna
		                           t_scan,
		                           t_subarray,                      // subarray
		                           t_correlation,
		                           t_intent, t_obs)){
			*itsLog << LogIO::SEVERE
			        << "Error selecting data."
			        << LogIO::POST;
			return false;
		}
		Double timeInSec=casaQuantity(timebin).get("s").getValue();
		reweighter.selectTime(timeInSec, String(timerange));

		String t_whichcol(datacol);
		const String t_combine = downcase(combine);

		reweighter.setFitSpw(t_fitspw);
		reweighter.setOutSpw(t_spw);
		if(!reweighter.reweight(t_whichcol, t_combine)){
			*itsLog << LogIO::SEVERE
			        << "Error reweighting " << itsMS->tableName()
			        << LogIO::POST;
			return false;
		}

		*itsLog << LogIO::NORMAL2
		        << itsMS->tableName() << " reweighted"
		        << LogIO::POST;
		rstat = true;
	}
	catch(AipsError x){
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return rstat;
}

bool
ms::split(const std::string&      outputms,  const ::casac::variant& field,
          const ::casac::variant& spw,       const std::vector<int>& step,
          const ::casac::variant& antenna,   const ::casac::variant& timebin,
          const std::string&      timerange, const ::casac::variant& scan,
          const ::casac::variant& uvrange,   const std::string&      taql,
          const std::string&      whichcol,  const ::casac::variant& tileShape,
          const ::casac::variant& subarray,  const std::string&      combine,
          const std::string& correlation,    const std::string&      intent,
          const std::string&      obs)
{
	Bool rstat(false);
	SubMS *splitter = NULL;
	try {
		*itsLog << LogOrigin("ms", "split");
		splitter = new SubMS(*itsMS);
		*itsLog << LogIO::NORMAL2 << "Sub MS created" << LogIO::POST;
		String t_field(m1toBlankCStr_(field));
		String t_spw(m1toBlankCStr_(spw));
		if(t_spw == "")   // MSSelection doesn't respond well to "", and setting it
			t_spw = "*";    // at the XML level does not work.

		String t_antenna = toCasaString(antenna);
		String t_scan    = toCasaString(scan);
		String t_intent  = toCasaString(intent);
		String t_obs     = toCasaString(obs);
		String t_uvrange = toCasaString(uvrange);
		String t_taql(taql);
		const String t_subarray = toCasaString(subarray);
		String t_correlation = upcase(correlation);
		//if(t_correlation == "")
		//  t_correlation = "*";   // * doesn't work.

		if(!splitter->setmsselect(t_spw, t_field, t_antenna, t_scan, t_uvrange,
		                          t_taql, Vector<Int>(step), t_subarray, t_correlation,
		                          t_intent, t_obs)){
			*itsLog << LogIO::SEVERE
			        << "Error selecting data."
			        << LogIO::POST;
			delete splitter;
			return false;
		}

		Double timeInSec=casaQuantity(timebin).get("s").getValue();
		splitter->selectTime(timeInSec, String(timerange));
		String t_outputms(outputms);
		String t_whichcol(whichcol);
		Vector<Int> t_tileshape(1, 0);
		if(toCasaString(tileShape) != String("")){
			t_tileshape.resize();
			t_tileshape=tileShape.toIntVec();
		}
		const String t_combine = downcase(combine);

		if(!splitter->makeSubMS(t_outputms, t_whichcol, t_tileshape, t_combine)){
			//      *itsLog << LogIO::WARN
			//        << "Splitting " << itsMS->tableName() << " to "
			//        << t_outputms << " failed."
			//        << LogIO::POST;
			delete splitter;
			return false;
		}

		*itsLog << LogIO::NORMAL2 << "SubMS made" << LogIO::POST;
		delete splitter;
		splitter = NULL;

		{// Update HISTORY table of newly created MS
			String message= toCasaString(outputms) + " split from " + itsMS->tableName();
			ostringstream param;
			param << "fieldids=" << t_field << " spwids=" << t_spw
			      << " step=" << Vector<Int>(step) << " which='" << whichcol << "'";
			String paramstr=param.str();
			writehistory(message, paramstr, "ms::split()", outputms, "ms");
		}

		rstat = true;
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		if(splitter){
			delete splitter;
		}
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return rstat;
}

bool
ms::partition(const std::string&      outputms,   const ::casac::variant& field,
              const ::casac::variant& spw,        const ::casac::variant& antenna,
              const ::casac::variant& timebin,    const std::string&      timerange,
              const ::casac::variant& scan,       const ::casac::variant& uvrange,
              const std::string&      taql,       const std::string&      whichcol,
              const ::casac::variant& tileShape,  const ::casac::variant& subarray,
              const std::string&      combine,    const std::string&      intent,
              const std::string&      obs)
{
	Bool rstat(false);
	try {
		*itsLog << LogOrigin("ms", "partition");
		Partition *partitioner = new Partition(*itsMS);
		*itsLog << LogIO::NORMAL2 << "Sub MS created" << LogIO::POST;
		String t_field(m1toBlankCStr_(field));
		String t_spw(m1toBlankCStr_(spw));
		if(t_spw == "")   // MSSelection doesn't respond well to "", and setting it
			t_spw = "*";    // at the XML level does not work.

		String t_antenna = toCasaString(antenna);
		String t_scan    = toCasaString(scan);
		String t_intent  = toCasaString(intent);
		String t_obs     = toCasaString(obs);
		String t_uvrange = toCasaString(uvrange);
		String t_taql(taql);
		const String t_subarray = toCasaString(subarray);

		if(!partitioner->setmsselect(t_spw, t_field, t_antenna, t_scan, t_uvrange,
		                             t_taql, t_subarray, t_intent, t_obs)){
			*itsLog << LogIO::SEVERE
			        << "Error selecting data."
			        << LogIO::POST;
			delete partitioner;
			return false;
		}

		Double timeInSec=casaQuantity(timebin).get("s").getValue();
		partitioner->selectTime(timeInSec, String(timerange));
		String t_outputms(outputms);
		String t_whichcol(whichcol);
		Vector<Int> t_tileshape(1,0);
		if(toCasaString(tileShape) != String("")){
			t_tileshape.resize();
			t_tileshape=tileShape.toIntVec();
		}
		const String t_combine = downcase(combine);

		if(!partitioner->makePartition(t_outputms, t_whichcol, t_tileshape, t_combine)){
			*itsLog << LogIO::SEVERE
			        << "Error partitioning " << itsMS->tableName() << " to "
			        << t_outputms
			        << LogIO::POST;
			delete partitioner;
			return false;
		}

		*itsLog << LogIO::NORMAL2 << "Partition made" << LogIO::POST;
		delete partitioner;

		rstat = true;
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return rstat;
}

Vector<Int> ms::getspectralwindows() {
    // Get list of selected SPWs from DDID selection
    ROMSColumns msc(*itsSelectedMS);
    Vector<Int> allDDIDs = msc.dataDescId().getColumn();
    Vector<Int> allSPWs = msc.dataDescription().spectralWindowId().getColumn();
    Int n = GenSort<Int>::sort(allDDIDs, Sort::Ascending, Sort::NoDuplicates);
    Vector<Int> selDDIDs = allDDIDs(Slice(0,n));
    Vector<Int> selSPWs(selDDIDs.size());
    for (uInt i=0; i<selDDIDs.size(); ++i) {
        selSPWs(i) = allSPWs(selDDIDs(i));
    }
    return selSPWs;
}

bool
ms::iterinit(const std::vector<std::string>& columns, const double interval,
             const int maxrows, const bool adddefaultsortcolumns)
{
	Bool rstat(false);
	try {
		if(!detached()){
			Vector<String> cols=toVectorString(columns);
			if(cols.nelements()==1 && (cols[0]==String("")))
				cols.resize();
			rstat = itsSel->iterInit(cols, interval, maxrows,
			                         adddefaultsortcolumns);
		}
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(True);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(True);
	return rstat;
}

bool
ms::iterinit2(const std::vector<std::string>& columns, const double interval,
             const int maxrows, const bool adddefaultsortcolumns)
{
	*itsLog << LogOrigin("ms", "iterinit");
	Bool rstat(false);
	try {
		if (!detached()) {
            // make sure we start fresh
            if (itsVI2)         {delete itsVI2; itsVI2=NULL;}

            Bool polnSelection = !polnExpr_p.empty();
            Bool chanSelection = !chanselExpr_p.empty();
            Bool chanAverage = ((chansel_p.size() > 0) && (chansel_p[2] > 1));

            // Iterating parameters for first layer (basic VIImpl2)
            // Process given columns
            Block<Int> columnIds = Block<Int>();
            Vector<String> colNames = casa::toVectorString(columns);
            Int n = colNames.nelements();
            if (n>0 && colNames(0)!="") {
                columnIds.resize(n);
                for (Int i=0; i<n; ++i) {
                    columnIds[i] = MS::columnType(colNames(i));
                    if (columnIds[i]==MS::UNDEFINED_COLUMN) {
                        *itsLog << LogIO::SEVERE << "Iteration initialization failed: unrecognized column name " << colNames(i) << LogIO::POST;
                        return false;
                    }
                }
            }
            // Make sort columns
            vi::SortColumns sortcols = vi::SortColumns(columnIds, adddefaultsortcolumns);
            vi::IteratingParameters iterpar(interval, sortcols);
            // Make VI2 basic layer
            bool writable = true;  // for putdata
            vi::VisIterImpl2LayerFactory viilayer(itsSelectedMS, iterpar, writable);
            Vector<vi::ViiLayerFactory*> layers(1);
            layers[0] = &viilayer;

            // Add channel-averaging layer if requested in selectchannel2 
            if (chanAverage) {
                Record config;
                config.define("chanbin", chansel_p[2]);
                if (chanSelection) {
                    String spwExpr = getSpwExpr() + chanselExpr_p;
                    config.define("spw", spwExpr);
                }
                vi::ChannelAverageTVILayerFactory* chanavglayer =
                    new vi::ChannelAverageTVILayerFactory(config);
                layers.resize(layers.size()+1, True);
                layers[1] = chanavglayer;
            }

            // Create VI2
            itsVI2 = new vi::VisibilityIterator2(layers);
            if (itsVI2 != NULL) {
                // Apply max rows
                if (maxrows>0) {
                    maxrows_p = True;
                    itsVI2->setRowBlocking(maxrows-1);
                }

                // Apply in-row selections with FrequencySelection class
                // If chanavg, chan selection handled in chanavg layer;
                if (polnSelection || (chanSelection && !chanAverage)) {
                    vi::FrequencySelectionUsingChannels freqSel = vi::FrequencySelectionUsingChannels();
                    if (polnSelection) {
                        Vector<Vector<Slice>> corrslices, chanslices;
                        mssSetData(*itsSelectedMS, *itsSelectedMS, 
                            chanslices, corrslices, "", "", "", "", "", "", 
                            "", polnExpr_p, "", "", "", "", 1, itsMSS);
                        freqSel.addCorrelationSlices(corrslices);
                    }

                    if (chanSelection) {
                        Int nchan(chansel_p[0]), start(chansel_p[1]), width(chansel_p[2]), inc(chansel_p[3]);
                        Vector<Int> spws = getspectralwindows();
                        for (Int chanbin=0; chanbin<nchan; ++chanbin) {
                            for (uInt spwIdx=0; spwIdx<spws.size(); ++spwIdx) 
                                freqSel.add(spws(spwIdx), start, width, 1);
                            start += inc;
                        }
                    }
                    itsVI2->setFrequencySelection(freqSel);
                }
                rstat = true;
            }
		}
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return rstat;
}

bool
ms::iterorigin()
{
	Bool rstat(False);
	try {
		if(!detached())
			rstat =  itsSel->iterOrigin();
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(True);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(True);
	return rstat;
}

bool
ms::iterorigin2()
{
	*itsLog << LogOrigin("ms", "iterorigin");
	Bool rstat(false);
	try {
		if(!detached()) {
            if (itsVI2) {
			    itsVI2->originChunks();
			    itsVI2->origin();
			    rstat = true;
            } else {
                *itsLog << LogIO::SEVERE << "Iteration failed: must call iterinit first" << LogIO::POST;
            }
        }
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return rstat;
}

bool
ms::iternext()
{
	Bool rstat(false);
	try {
		if(!detached())
			rstat =  itsSel->iterNext();
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(True);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(True);
	return rstat;
}

bool
ms::iternext2()
{
	*itsLog << LogOrigin("ms", "iternext");
	Bool rstat(false);
	try {
		if(!detached()) {
            if (itsVI2) {
                if (maxrows_p) {  // doing subchunks
                    itsVI2->next();
                    if (!itsVI2->more()) {
                        itsVI2->nextChunk();
                        if (!itsVI2->moreChunks())
                            rstat = False;
                        else {
                            itsVI2->origin();
                            rstat = itsVI2->more();
                        }
                    } else {
                        rstat = true;
                    }
                } else { // doing chunks
			        itsVI2->nextChunk();
                    rstat = itsVI2->moreChunks();
                }
            } else {
                *itsLog << LogIO::SEVERE << "Iteration failed: must call iterinit first" << LogIO::POST;
            }
        }
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return rstat;
}

bool
ms::iterend()
{
	Bool rstat(False);
	try {
		if(!detached())
			rstat =  itsSel->iterEnd();
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(True);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(True);
	return rstat;
}

bool
ms::iterend2()
{
	*itsLog << LogOrigin("ms", "iterend");
	Bool rstat(false);
	try {
		if(!detached())
		    if (itsVI2) {
                delete itsVI2;
                itsVI2 = NULL;
            }
			rstat = true;
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return rstat;
}

/*
  bool
  ms::tosdfits(const std::string& fitsfile)
  {

  *itsLog << LogOrigin("ms", "tosdfits");
  *itsLog << "not implemented"<<LogIO::POST;
  Table::relinquishAutoLocks(true);
  return false;
  }
*/

std::string
ms::asdmref(const std::string& abspath)
{
	std::string retval;
	*itsLog << LogOrigin("ms", "asdmref");
	try {
		if(!detached()){
			// get the data manager of the DATA column
			TableDesc mSTD(itsMS->actualTableDesc());
			ColumnDesc myColDesc(mSTD.columnDesc("DATA"));
			String hcName(myColDesc.dataManagerGroup());
			DataManager* myDM = itsMS->findDataManager(hcName);
			String dataManName(myDM->dataManagerName());

			if(dataManName != "AsdmStMan"){
				*itsLog << LogIO::NORMAL << "MS does not reference an ASDM." << LogIO::POST;
			}
			else{

				AsdmStMan* myASTMan = static_cast<AsdmStMan*>(itsMS->findDataManager(hcName));

				Block<String> bDFNames;
				myASTMan->getBDFNames(bDFNames);

				if(bDFNames.size()!=0){
					// from name 0 determine path
					Path tmpPath(bDFNames[0]);
					tmpPath = Path(tmpPath.dirName()); // remove BLOB name
					String binDir(tmpPath.baseName()); // memorize the ASDMBinary dir name
					String presentPath(tmpPath.dirName()); // remove ASDMBinary dir name
					*itsLog << LogIO::NORMAL << "Present ASDM reference path:\n"
					        << presentPath << LogIO::POST;
					if(abspath == ""){
						retval = presentPath;
					}
					else{
						if(abspath=="/"){
							*itsLog << LogIO::SEVERE << "Choosing abspath==\"/\" is not a good idea ..." << LogIO::POST;
							retval = presentPath;
						}
						else{
							String absPath(abspath);
							if(absPath.lastchar()!='/'){
								absPath += "/";
							}
							if(!File(absPath).isDirectory()){
								*itsLog << LogIO::WARN << "\""+absPath+"\" is presently not a valid path ..." << LogIO::POST;
							}

							// modify the bDFNames and write them back
							for(uInt i=0; i<bDFNames.size(); i++){
								bDFNames[i] = absPath+binDir+"/"+Path(bDFNames[i]).baseName();
							}
							myASTMan->setBDFNames(bDFNames);
							myASTMan->writeIndex();

							retval = abspath;
							*itsLog << LogIO::NORMAL << "New ASDM reference path:\n"
							        << retval << LogIO::POST;
						}
					}
				}
			}
		}
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return retval;

}

bool ms::continuumsub(const ::casac::variant& field,
                      const ::casac::variant& fitspw,
                      const ::casac::variant& spw,
                      const ::casac::variant& solint,
                      const int fitorder,
                      const std::string& mode)
{
	Bool rstat(false);
	try {
		*itsLog << LogOrigin("ms", "continuumsub");
		*itsLog << LogIO::NORMAL2 << "continuumsub starting" << LogIO::POST;

		MSContinuumSubtractor sub(*itsMS);
		sub.setField(toCasaString(field));
		sub.setFitSpw(toCasaString(fitspw));
		sub.setSubSpw(toCasaString(spw));
		sub.setSolutionInterval(toCasaString(solint));
		sub.setOrder(fitorder);
		sub.setMode(mode);
		sub.subtract();
		*itsLog << LogIO::NORMAL2 << "continuumsub finished" << LogIO::POST;
		rstat = true;
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return rstat;
}

bool ms::continuumsub2(const ::casac::variant& field,
                      const ::casac::variant& fitspw,
                      const ::casac::variant& spw,
                      const ::casac::variant& solint,
                      const int fitorder,
                      const std::string& mode)
{
	Bool rstat(False);
	try {
		*itsLog << LogOrigin("ms", "continuumsub");
		*itsLog << LogIO::NORMAL2 << "continuumsub starting" << LogIO::POST;

		MSContinuumSubtractor sub(*itsMS);
		sub.setField(toCasaString(field));
		sub.setFitSpw(toCasaString(fitspw));
		sub.setSubSpw(toCasaString(spw));
		sub.setSolutionInterval(toCasaString(solint));
		sub.setOrder(fitorder);
		sub.setMode(mode);
		sub.subtract2();
		*itsLog << LogIO::NORMAL2 << "continuumsub finished" << LogIO::POST;
		rstat = True;
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(True);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(True);
	return rstat;
}

/*
  bool
  ms::ptsrc(const std::vector<int>& fldid, const std::vector<int>& spwid)
  {

  *itsLog << LogOrigin("ms", "ptsrc");
  *itsLog << "not implemented"<<LogIO::POST;
  Table::relinquishAutoLocks(true);
  return false;
  }
*/

bool
ms::done()
{
	*itsLog << LogOrigin("ms", "done");
	Table::relinquishAutoLocks(true);
	return close();
}

bool
ms::detached()
{
	Bool rstat(false);
	try {
		if (itsMS->isNull()) {
			*itsLog << LogOrigin("ms", __func__);
			*itsLog << LogIO::SEVERE
			        << "ms is not attached to a file - cannot perform operation.\n"
			        << "Call ms.open('filename') to reattach." << LogIO::POST;
			rstat = true;
		}
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return rstat;
}

::casac::record*
ms::msseltoindex(const std::string& vis, const ::casac::variant& spw,
                 const ::casac::variant& field,
                 const ::casac::variant& baseline,
                 const ::casac::variant& time,
                 const ::casac::variant& scan, const ::casac::variant& uvrange,
                 const ::casac::variant& observation,
                 const ::casac::variant& polarization,
                 const std::string& taql)
{
	casac::record* rstat(0);
	try {


		MeasurementSet thisms(vis);
		Record selected=thisms.msseltoindex(toCasaString(spw), toCasaString(field),
		                                    toCasaString(baseline),
		                                    toCasaString(time),toCasaString(scan),
		                                    toCasaString(uvrange),
		                                    toCasaString(observation),
		                                    toCasaString(polarization),
		                                    String(taql));
		rstat = fromRecord(selected);


	}catch (AipsError x){
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}

	Table::relinquishAutoLocks(true);
	return rstat;

}

bool
ms::hanningsmooth(const std::string& datacolumn)
{
	Bool rstat(false);
	try {
		*itsLog << LogOrigin("ms", "hanningsmooth");
		if(!ready2write_()){
			*itsLog << LogIO::SEVERE
			        << "Please open ms with parameter nomodify=false so "
			        << "smoothed channel data can be stored."
			        << LogIO::POST;
			return rstat;
		}

		Block<int> noSort;
		Matrix<Int> allChannels;
		Double intrvl = 0;
		String dcol(datacolumn);
		dcol.downcase();
		Bool addScratch = !(dcol=="data") && !(dcol=="all"); // don't add scratch columns if they don't exist already
		// and the requested output column is == "data" or "all"
		VisSet vs(*itsMS, noSort, allChannels, addScratch, intrvl, false);
		if(dcol=="all"){
			MSMainColumns mainCols(*itsMS);
			if(!mainCols.correctedData().isNull()){ // there are scratch columns
				*itsLog << LogIO::NORMAL << "Smoothing MS Main Table column CORRECTED_DATA ... " << LogIO::POST;
				VisSetUtil::HanningSmooth(vs, "corrected", false); // false, i.e. don't change flags and weights here, will be done below
			}
			*itsLog << LogIO::NORMAL << "Smoothing MS Main Table column DATA ... " << LogIO::POST;
			VisSetUtil::HanningSmooth(vs, "data", true);
		}
		else{
			*itsLog << LogIO::NORMAL << "Smoothing MS Main Table column \'"
			        << datacolumn << "\'" << LogIO::POST;
			VisSetUtil::HanningSmooth(vs, datacolumn, true);
		}

		rstat = true;
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return rstat;
}

bool
ms::uvsub(Bool reverse)
{
	Bool rstat(false);
	try {
		*itsLog << LogOrigin("ms", "uvsub");
		if(!ready2write_()){
			*itsLog << LogIO::SEVERE
			        << "Please open ms with parameter nomodify=false. "
			        << "Write access to ms is needed by uvsub."
			        << LogIO::POST;
			return rstat;
		}

		// Ensure CORRECTED_DATA exists and is initialized
		//   (no-op if CORRECTED_DATA already present)
		VisSetUtil::addScrCols(*itsMS,false,true,true,false);

		// Open VisSet w/out triggering scr cols
		//  because CORRECTED_DATA has already been added above and
		//  MODEL_DATA is either present or automatic in VSU::UVSub
		Block<int> noSort;
		Matrix<Int> allChannels;
		Double intrvl = 0;

		VisSet vs(*itsMS, noSort, allChannels,false, intrvl, false,false);
		VisSetUtil::UVSub(vs, reverse);

		rstat = true;
	}
	catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return rstat;
}

bool ms::msselect(const ::casac::record& exprs, const bool onlyparse)
{
	Bool retVal=false;
	try
	{
		*itsLog << LogOrigin("ms", "msselect");
		Record *casaRec = toRecord(exprs);
		String spwExpr, timeExpr, fieldExpr, baselineExpr, scanExpr, scanIntentExpr,
			polnExpr, uvDistExpr, obsExpr, arrayExpr, taQLExpr;
		Int nFields = casaRec->nfields();
		for (Int i=0; i<nFields; i++)
		{
			if (casaRec->name(i) == "spw")           {spwExpr        = casaRec->asString(RecordFieldId(i));}
			if (casaRec->name(i) == "time")          {timeExpr       = casaRec->asString(RecordFieldId(i));}
			if (casaRec->name(i) == "field")         {fieldExpr      = casaRec->asString(RecordFieldId(i));}
			if (casaRec->name(i) == "baseline")      {baselineExpr   = casaRec->asString(RecordFieldId(i));}
			if (casaRec->name(i) == "antenna")       {baselineExpr   = casaRec->asString(RecordFieldId(i));}
			if (casaRec->name(i) == "scan")          {scanExpr       = casaRec->asString(RecordFieldId(i));}
			if (casaRec->name(i) == "scanintent")    {scanIntentExpr = casaRec->asString(RecordFieldId(i));}
			if (casaRec->name(i) == "state")         {scanIntentExpr = casaRec->asString(RecordFieldId(i));}
			if (casaRec->name(i) == "polarization")  {polnExpr       = casaRec->asString(RecordFieldId(i));}
			if (casaRec->name(i) == "uvdist")        {uvDistExpr     = casaRec->asString(RecordFieldId(i));}
			if (casaRec->name(i) == "observation")   {obsExpr        = casaRec->asString(RecordFieldId(i));}
			if (casaRec->name(i) == "array")         {arrayExpr      = casaRec->asString(RecordFieldId(i));}
			if (casaRec->name(i) == "taql")          {taQLExpr       = casaRec->asString(RecordFieldId(i));}
		}
		// if (itsSelectedMS) delete itsSelectedMS;
		// itsSelectedMS = new MeasurementSet();

		//
		// If only parsing is requested, just set up the itsMSS object.
		// This is much faster if one is only interested in the indices
		// and not the actual selected MS.
		//
		if (onlyparse)
		{
			itsMSS->reset(*itsMS, MSSelection::PARSE_NOW,timeExpr,baselineExpr,fieldExpr,spwExpr,uvDistExpr,
			              taQLExpr,polnExpr,scanExpr,arrayExpr,scanIntentExpr,obsExpr);
			retVal=(itsMSS->getTEN(itsMS).isNull() == false);
		} else {
		    retVal = mssSetData(*itsSelectedMS, *itsSelectedMS, "",/*outMSName*/
		        timeExpr, baselineExpr, fieldExpr, spwExpr, uvDistExpr,
			    taQLExpr, polnExpr, scanExpr,
			    arrayExpr, scanIntentExpr, obsExpr, itsMSS);
		    *itsMS = MeasurementSet(*itsSelectedMS);
        }
        if (itsSel) itsSel->setMS(*itsMS);
		return retVal;
	}
	catch (AipsError x)
	{
		RETHROW(x);
	}
	Table::relinquishAutoLocks(true);
	return retVal;
}

::casac::record*
ms::msselectedindices()
{
	casac::record *selectedIndices(0);
	try
	{
		*itsLog << LogOrigin("ms", "msselectedindices");
		Record tmp =  mssSelectedIndices(*itsMSS, itsMS);
		selectedIndices = fromRecord(tmp);
	}
	catch (AipsError x)
	{
		Table::relinquishAutoLocks(true);
		RETHROW(x);
	}
	return selectedIndices;
}

bool
ms::addephemeris(const int id,
                 const std::string& ephemerisname,
                 const std::string& comment,
                 const ::casac::variant& field)
{
	Bool rstat(false);
	try {
		*itsLog << LogOrigin("ms", "addephemeris");

		String t_field(m1toBlankCStr_(field));
		String t_name     = toCasaString(ephemerisname);
		String t_comment = toCasaString(comment);
		Record selrec;
		Vector<Int> fieldids;

		if(detached()){
			return false;
		}

		if(t_field.size()>0){
			try {
				selrec=itsMS->msseltoindex("*", t_field);
			}
			catch (AipsError x) {
				*itsLog << LogOrigin("ms", "addephemeris")
				        << LogIO::SEVERE << x.getMesg() << LogIO::POST;
				RETHROW(x);
			}
			fieldids=selrec.asArrayInt("field");
		}

		Double startTime;

		Array<Double> timeRanges = MSObservationColumns(itsMS->observation()).timeRange().getColumn();

		try{
			MeasComet mc(t_name);
			startTime = casacore::Quantity(mc.getStart(),"d").getValue("s");
			Double endTime = casacore::Quantity(mc.getEnd(),"d").getValue("s");
			if(startTime>min(timeRanges)){
				*itsLog << LogOrigin("ms", "addephemeris")
				        << LogIO::WARN << "Ephemeris validity time range starts after start of observation." << LogIO::POST;
			}
			if(endTime<max(timeRanges)){
				*itsLog << LogOrigin("ms", "addephemeris")
				        << LogIO::WARN << "Ephemeris validity time range ends before end of observation." << LogIO::POST;
			}
		}
		catch (AipsError x){
			*itsLog << LogOrigin("ms", "addephemeris")
			        << LogIO::SEVERE << "Error reading input ephemeris table. No changes made to MS." << endl
			        << x.getMesg() << LogIO::POST;
			RETHROW(x);

		}

		uInt theId=0;
		if(id>=0){
			theId = id;
		}
		else{ // if the given id is invalid, determine the next free id
			String ephIDName = MSField::columnName(MSField::EPHEMERIS_ID);
			if(itsMS->field().actualTableDesc().isColumn(ephIDName)){
				ScalarColumn<Int> ephid(itsMS->field(), ephIDName);
				Vector<Int> ids = ephid.getColumn();
				for(uInt i=0; i<ids.size(); i++){
					for(uInt j=0; j<fieldids.size(); j++){
						if((Int)i==fieldids[j] && ids[i]>=0){ // these are the ids to be overwritten
							ids[i] = -1; // exclude them from the search
						}
					}
				}
				theId = max(ids)+1;
			}
		}

		if(!itsMS->field().addEphemeris(theId, t_name, t_comment)){
			*itsLog << LogOrigin("ms", "addephemeris")
			        << LogIO::SEVERE << "Error adding ephemeris to MS." << LogIO::POST;
			return false;
		}

		MSFieldColumns msfc(itsMS->field());

		for(uInt i=0; i<fieldids.size(); i++){
			Double presentStartTime = msfc.time()(fieldids(i));
			if(presentStartTime<min(timeRanges) || presentStartTime>max(timeRanges)){
				// present start time is inconsistent with values of observation table
				*itsLog << LogOrigin("ms", "addephemeris")
				        << LogIO::WARN << "The TIME column entry for field " << fieldids(i)
				        << "is outside the observation time range given by the OBSERVATION table." << LogIO::POST;
				if(min(timeRanges)<=startTime || startTime<=max(timeRanges)){
					// start time of ephemeris is OK, use it as new time column entry
					msfc.time().put(fieldids(i), startTime);
					*itsLog << LogIO::WARN << "   Will replace it by the start time of the added ephemeris." << LogIO::POST;
				}
			}
			msfc.ephemerisId().put(fieldids(i), theId);
		}

		{// Update HISTORY table of newly created MS
			String message= "Added ephemeris to FIELD table.";
			ostringstream param;
			param << "field=" << t_field << " id=" << id
			      << " name='" << t_name << "' coment='" << comment << "'";
			String paramstr=param.str();
			writehistory(message, paramstr, "ms::addephemeris()", itsMS->tableName(), "ms");
		}

		rstat = true;
	} catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}

	return rstat;
}

void
ms::addephemcol(const casacore::MeasurementSet& appendedMS)
{
	if(!itsMS->field().actualTableDesc().isColumn(MSField::columnName(MSField::EPHEMERIS_ID))){
		// if not, test if the other MS uses ephem objects
		Bool usesEphems = false;
		const ROMSFieldColumns otherFldCol(appendedMS.field());
		for(uInt i=0; i<otherFldCol.nrow(); i++){
			if(!otherFldCol.ephemPath(i).empty()){
				usesEphems = true;
				break;
			}
		}
		if(usesEphems){ // if yes, the ephID column needs to be added to this MS FIELD table
			String thisMSName = Path(itsMS->tableName()).absoluteName();
			*itsLog << LogIO::NORMAL << "Adding the EPHEMERIS_ID column to the FIELD table of first MS. "
			        << LogIO::POST;
			if(!itsMS->field().addEphemeris(0,"","")){
				*itsLog << "Cannot add the EPHEMERIS_ID column to the FIELD table of MS " << thisMSName
				        << LogIO::EXCEPTION;
			}
			// reopen this MS
			close();
			open(thisMSName, false, false);
		}
	}
}
//
// New iteration control interface.  Uses the VI for iterations and
// getting the requested data out.
//
bool
ms::niterinit(const std::vector<std::string>& /*columns*/, const double interval,
              const int maxrows, const bool adddefaultsortcolumns)
{
	Bool rstat(false);
	Block<Int> sort(1);
	sort[0]=MS::TIME;
	try
	{
		if (itsVI == NULL)
			itsVI = new VisibilityIterator(*itsMS, sort, adddefaultsortcolumns, interval);
		else
			*itsVI = VisibilityIterator(*itsMS, sort, adddefaultsortcolumns, interval);
		if (interval <= 0) itsVI->setRowBlocking(itsMS->nrow());
		if (maxrows > 0) itsVI->setRowBlocking(maxrows);
		//       *itsVB = VisBuffer(*itsVI);
		rstat=true;
	}
	catch (AipsError x)
	{
		RETHROW(x);
	}

	niterorigin();
	doingIterations_p=true;
	return rstat;
}

bool
ms::niterorigin()
{
	Bool rstat(false);
	if (!detached())
	{
		try
		{
			if (itsVI)
			{
				itsVI->originChunks();
				rstat=true;
			}
			else
				*itsLog << "ms::niterorigin:  Please call niterinit() first." << LogIO::EXCEPTION;
		}
		catch (AipsError x)
		{
			RETHROW(x);
		}
	}
	return rstat;
}

bool
ms::niterend()
{
	Bool rstat(false);
	if (!detached())
	{
		try
		{
			rstat = !itsVI->moreChunks();
		}
		catch (AipsError x)
		{
			RETHROW(x);
		}
	}
	return rstat;
}

bool
ms::niternext()
{
	Bool rstat(false);
	if (!detached())
	{
		try
		{
			if (!niterend())
			{
				itsVI->nextChunk();
				rstat=true;
			}
		}
		catch (AipsError x)
		{
			RETHROW(x);
		}
	}

	return rstat;
}

::casac::record*
ms::ngetdata(const std::vector<std::string>& items, const bool /*ifraxis*/, const int /*ifraxisgap*/, const int /*increment*/, const bool /*average*/)
{
	try
	{
		if (itsVI == NULL)
			niterinit(items,0.0,0,false);
		// if (doingIterations_p == false)
		//  niterorigin();

		casacore::Record rec;
		Int nItems = items.size();
		for (Int i=0; i<nItems; i++)
		{
			String item(items[i]);
			item.downcase();
			MSS::Field fld=MSS::field(item);
			switch (fld)
			{
			case MSS::DATA:
			{
				Cube<Complex> vis;
				itsVI->visibility(vis,VisibilityIterator::Observed);
				rec.define(item,vis);
				break;
			}
			case MSS::MODEL_DATA:
			{
				Cube<Complex> vis;
				itsVI->visibility(vis,VisibilityIterator::Model);
				rec.define(item,vis);
				break;
			}
			case MSS::CORRECTED_DATA:
			{
				Cube<Complex> vis;
				itsVI->visibility(vis,VisibilityIterator::Corrected);
				rec.define(item,vis);
				break;
			}
			case MSS::ANTENNA1:
			{
				Vector<Int> a1;
				a1 = itsVI->antenna1(a1);
				rec.define(item,a1);
				break;
			}
			case MSS::ANTENNA2:
			{
				Vector<Int> a;
				a = itsVI->antenna1(a);
				rec.define(item,a);
				break;
			}
			case MSS::FLAG:
			{
				Cube<Bool> flag;
				flag = itsVI->flag(flag);
				rec.define(item,flag);
				break;
			}
			case MSS::TIME:
			{
				Vector<Double> time;
				time = itsVI->time(time);
				rec.define(item,time);
				break;
			}
			case MSS::ROWS:
			{
				Vector<uInt> rowIds;
				rowIds = itsVI->rowIds(rowIds);
				Vector<Int> tmp(rowIds.shape());
				for (Int ii=0;ii<(Int)tmp.nelements(); ii++)
					tmp(ii)=rowIds(ii);
				rec.define(item,tmp);
				break;
			}
			case MSS::WEIGHT:
			{
				Vector<Float> weight;
				weight = itsVI->weight(weight);
				rec.define(item,weight);
				break;
			}
			// case MSS::UVW:
			//   {
			//  Vector<RigidVector<double, 3> > uvw;
			//  uvw = itsVI->uvw(uvw);
			//  rec.define(item,uvw);
			//  break;
			//   }

			default:
			{
				*itsLog  << "ngetdata: Unsupported item requrested (" << items[i] << ")" <<LogIO::EXCEPTION;
			}
			}
		}
		return fromRecord(rec);
	}
	catch (AipsError x)
	{
		RETHROW(x);
	}
}

} // casac namespace
