//# MSCache.cc: Specialized PlotMSCache for filling MSs
//# Copyright (C) 2009
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
//# $Id: $
#include <plotms/Data/MSCache.h>
#include <plotms/Data/PlotMSIndexer.h>

#include <casa/OS/Timer.h>
#include <casa/OS/Memory.h>
#include <casa/Quanta/MVTime.h>
#include <casa/System/Aipsrc.h>
#include <casa/Utilities/Sort.h>
#include <tables/Tables/ScalarColumn.h>
#include <lattices/Lattices/ArrayLattice.h>
#include <lattices/LatticeMath/LatticeFFT.h>
#include <scimath/Mathematics/FFTServer.h>
#include <ms/MeasurementSets/MSColumns.h> 	 
#include <msvis/MSVis/VisSet.h>
#include <msvis/MSVis/VisBuffer2.h>
#include <mstransform/MSTransform/MSTransformIteratorFactory.h>
#include <plotms/Data/PlotMSVBAverager.h>
#include <plotms/Data/MSCacheVolMeter.h>
#include <plotms/PlotMS/PlotMS.h>
#include <tables/Tables/Table.h>
#include <measures/Measures/Stokes.h>

#include <ms/MeasurementSets/MeasurementSet.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MFrequency.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <ms/MeasurementSets/MSAntennaColumns.h>

#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogFilter.h>

#include <ctime>

namespace casa {

MSCache::MSCache(PlotMSApp* parent):
		  PlotMSCacheBase(parent)
{
	ephemerisAvailable = false;
	vi_p = NULL;
	vm_ = NULL;
}

MSCache::~MSCache() {}

String MSCache::polname(Int ipol) {
	return Stokes::name(Stokes::type(ipol));
}


void MSCache::loadIt(vector<PMS::Axis>& loadAxes,
		vector<PMS::DataColumn>& loadData,
		ThreadCommunication* thread) {

	// process selected columns			
	String dataColumn = checkDataColumn(loadAxes, loadData);

	// Apply selections to MS to create selection MS 
	// and channel/correlation selections
	Vector<Vector<Slice> > chansel;
	Vector<Vector<Slice> > corrsel;
	MeasurementSet* selMS = new MeasurementSet();
	Table::TableOption tabopt(Table::Old);
	MeasurementSet* inputMS = new MeasurementSet(filename_, TableLock(TableLock::AutoLocking), tabopt);
	getNamesFromMS(*inputMS);
	// improper selection can cause exception
	try {
		selection_.apply(*inputMS, *selMS, chansel, corrsel);
	} catch(AipsError& log) {
		delete inputMS;
		delete selMS;
		loadError(log.getMesg());
	}
	vm_ = new MSCacheVolMeter(*inputMS, averaging_, chansel, corrsel);
	delete inputMS;

	Vector<Int> nIterPerAve;
	visBufferShapes_.clear();

	// Note: MSTransformVI/VB handles channel and time averaging
	// It does not handle Antenna, Baseline, and SPW averaging yet
	// so for now we'll do it the old way
	if ( averaging_.baseline() || averaging_.antenna() || averaging_.spw() ) {
		// make a "counting VI" -- plain VI2 is faster than TransformingTvi2.
		// Use plotms code to determine nChunks and memory requirements
		vi::VisibilityIterator2* cvi = setUpVisIter(*selMS, chansel, corrsel);
		countChunks(*cvi, nIterPerAve, thread);
		delete cvi;
		delete selMS;  // close open tables
		try {
			trapExcessVolume(pendingLoadAxes_);
			// Now set up TransformingVi2 for averaging/loading data
			setUpVisIter(selection_, calibration_, dataColumn, False, False);
			loadChunks(*vi_p, averaging_, nIterPerAve,
			   loadAxes, loadData, thread);
		} catch(AipsError& log) {
			loadError(log.getMesg());	
		}	
	}
	else { 
		delete selMS;  // close open tables
		try {
			// setUpVisIter also gets the VB shapes and 
			// calls trapExcessVolume:
			setUpVisIter(selection_, calibration_, dataColumn, False, True);
			loadChunks(*vi_p, loadAxes, loadData, thread);
		} catch(AipsError& log) {
			loadError(log.getMesg());
		}	
	}
	// Remember # of VBs per Average
	nVBPerAve_.resize();
	if (nIterPerAve.nelements()>0)
		nVBPerAve_ = nIterPerAve;
	else {
		nVBPerAve_.resize(nChunk_);
		nVBPerAve_.set(1);
	}
	deleteVi(); // close any open tables
}

void MSCache::loadError(String mesg) {
	// catch load error, clear the existing cache, and rethrow
	logLoad(mesg);
	clear();
	deleteVi();  // close any open tables
	stringstream ss;
	ss << "Error loading cache";
	throw(AipsError(ss.str()));
}

void MSCache::deleteVi() {
	if (vi_p) delete vi_p;
	vi_p = NULL;
}

void MSCache::deleteVm() {
	if (vm_) delete vm_;
	vm_ = NULL;
}

String MSCache::checkDataColumn(vector<PMS::Axis>& loadAxes,
	vector<PMS::DataColumn>& loadData)
{	// Check data column choice and determine which column to pass to VisIter
 
	Bool corcolOk(false), floatcolOk(false), datacolOk(false), wtspcolOk(false);
	String dataColumn = "NONE";  // default is none - CAS-7506

	// Check if data, scratch, float, and wtsp cols present
	Table thisTable(filename_);
	const ColumnDescSet cds = thisTable.tableDesc().columnDescSet();
	datacolOk  = cds.isDefined("DATA");
	corcolOk   = cds.isDefined("CORRECTED_DATA");
	floatcolOk = cds.isDefined("FLOAT_DATA");
	wtspcolOk  = cds.isDefined("WEIGHT_SPECTRUM");

	for (uInt i=0; i<loadData.size(); ++i) {
		// Check data column
		switch (loadData[i]) {
			case PMS::CORRECTED:
			case PMS::CORRECTED_DIVIDE_MODEL:
			case PMS::CORRMODEL: {
			    // user asked for corrected data but no corrected column
			    if (!corcolOk && !calibration_.useCallib()) {
                    if (datacolOk) {
                        //Exception was removed - see CAS-5214
                        loadData[i] = PMS::DATA;
                        logWarn( "load_cache", 
                             "CORRECTED_DATA column not present and calibration library not set or enabled; will use DATA instead.");
                        //throw(AipsError("CORRECTED_DATA not present, please use DATA"));
                    } else {
                        // CAS-7761 - if no corrected/data cols use float for SD MS 
                        if (floatcolOk) {
                            loadData[i] = PMS::FLOAT_DATA;
				            logWarn( "load_cache", 
                             "CORRECTED_DATA column not present and calibration library not set or enabled; will use FLOAT_DATA instead.");
                        }
                    }
                }
			    break;
			}
			case PMS::FLOAT_DATA: {
			    if (!floatcolOk) {
                    // user asked for float data but no float column 
                    throw(AipsError("FLOAT_DATA not present, please use DATA"));
			    }
			    break;
			}
			case PMS::DATA: {
			    // CAS-7482 data requested (could be default) but no DATA column 
			    if (!datacolOk && floatcolOk) {
				    loadData[i] = PMS::FLOAT_DATA;
                    if (loadAxes[i] == PMS::AMP)
				        logWarn( "load_cache", "DATA column not present; will use FLOAT_DATA instead.");
			    }
			    break;
			}
			default:
				break;
		} // switch

		// Check load axes
		switch (loadAxes[i]) {
			case PMS::WTSP: {
				if (wtspcolOk) {
					ArrayColumn<Float> weightSpectrum;
					weightSpectrum.attach(thisTable,
                        MS::columnName(MS::WEIGHT_SPECTRUM));
         			if (!weightSpectrum.hasContent())  // CAS-7517
				    	logWarn("load_cache", "Plotting WEIGHT column, WEIGHT_SPECTRUM (WTSP) has not been initialized (this can be changed with initweights task)");
                        // Also send to console
				    	cout << "WARNING: Plotting WEIGHT column, WEIGHT_SPECTRUM (WTSP) has not been initialized (this can be changed with initweights task)" << endl;
				}
                break;
			}
            case PMS::PHASE:
			case PMS::IMAG: {
                if (loadData[i] == PMS::FLOAT_DATA) {
				    // user asked for float data for nonvalid axes
				    throw(AipsError("Chosen axis not valid for FLOAT_DATA, please use AMP or change Data Column"));
                } else {
			        dataColumn = getDataColumn(loadData[i]);
                }
				break;
            }
			case PMS::REAL:
            case PMS::AMP:
            case PMS::WTxAMP: {
                dataColumn = getDataColumn(loadData[i]);
                break;
            }
            default:
                break;
        } // switch
	} // for

    // If data was already loaded but an axis was changed,
    // only the new axis is in loadAxes and column will end up NONE.
    // Check for datacolumn-dependent axis being plotted
    if (dataColumn == "NONE") {
        dataColumn = checkAxesDatacolumns();
    }
        
	return dataColumn;
}

String MSCache::getDataColumn(PMS::DataColumn dataCol)
{
    String dataColumn = PMS::dataColumn(dataCol);
	// Convert datacolumn as needed for MSTransformManager
	if ((dataColumn == "corrected-model") || 
	    (dataColumn == "data-model") ||
	    (dataColumn == "data/model") || 
	    (dataColumn == "corrected/model")) {
			dataColumn = "ALL";
	} else if (dataColumn == "float") {
		dataColumn = "FLOAT_DATA";
	} else {	
		dataColumn.upcase();
	}
	return dataColumn;
}

String MSCache::checkAxesDatacolumns() {
    // Check data column of plotted axes
    String dataCol = "NONE";
    int axesCount = currentX_.size();
    for (int i=0; i<axesCount; ++i) {
        if (PMS::axisIsData(currentX_[i])) {
            dataCol = getDataColumn(loadedAxesData_[currentX_[i]]);
        }
        if (PMS::axisIsData(currentY_[i])) {
            dataCol = getDataColumn(loadedAxesData_[currentY_[i]]);
        }
    }
    return dataCol;
}

void MSCache::getNamesFromMS(MeasurementSet& ms)
{
    ROMSColumns msCol(ms);
    antnames_.resize();
    stanames_.resize();
    antstanames_.resize();
    fldnames_.resize();
    intentnames_.resize();
    positions_.resize();

    antnames_    = msCol.antenna().name().getColumn();
    stanames_    = msCol.antenna().station().getColumn();
    antstanames_ = antnames_+String("@")+stanames_;
    positions_   = msCol.antenna().position().getColumn();

    fldnames_    = msCol.field().name().getColumn();

    intentnames_ = msCol.state().obsMode().getColumn();
    mapIntentNamesToIds();  // eliminate duplicate intent names
}

void MSCache::setUpVisIter(PlotMSSelection& selection,
		PlotMSCalibration& calibration,
		String dataColumn, Bool interactive,
        Bool estimateMemory) {
	/* Create plain or averaging (time or channel) VI with 
           configuration Record and MSTransformIterator factory */

	// Create configuration:
	// Start with data selection; rename fields with expected keywords
	Record configuration = selection.toRecord();
	configuration.renameField("correlation", configuration.fieldNumber("corr"));

	// Add needed fields
	configuration.define("inputms", filename_);
	configuration.define("datacolumn", dataColumn);
	configuration.define("buffermode", True);
	configuration.define("reindex", False);
    configuration.define("interactive", interactive);

	// Add transformation selection with expected keywords and string value
	configuration.merge(transformations_.toRecord());
	Double restfreq = configuration.asDouble(configuration.fieldNumber("RestFreq"));
	configuration.removeField(configuration.fieldNumber("RestFreq"));
	configuration.define("restfreq", String::toString(restfreq));
	configuration.renameField("outframe", configuration.fieldNumber("Frame"));
	configuration.renameField("veltype", configuration.fieldNumber("Veldef"));

	// Add calibration library if set
	if (calibration.useCallib()) {
		configuration.define("callib", calibration.calLibrary());
	}	

	// Apply time and channel averaging
	if (averaging_.time()){
		configuration.define("timeaverage", True);
		configuration.define("timebin", averaging_.timeStr());
		String timespanStr = "state";
		if (averaging_.field())
			timespanStr += ",scan,field";
		else if (averaging_.scan())
			timespanStr += ",scan";
		configuration.define("timespan", timespanStr);
	}
	if (averaging_.channel()) {
		configuration.define("chanaverage", True);
		int chanVal = static_cast<int>(averaging_.channelValue());
		configuration.define("chanbin", chanVal);
	}

    LogFilter oldFilter(plotms_->getParameters().logPriority());
	MSTransformIteratorFactory* factory = NULL;
	try {
        // Filter out MSTransformManager setup messages
        LogFilter filter(LogMessage::WARN);
        LogSink().globalSink().filter(filter);

		factory = new MSTransformIteratorFactory(configuration);

		if (estimateMemory) {
			visBufferShapes_ = factory->getVisBufferStructure();
			Int chunks = visBufferShapes_.size();
			if(chunks != nChunk_) increaseChunks(chunks);
			trapExcessVolume(pendingLoadAxes_);
		}
		vi_p = new vi::VisibilityIterator2(*factory);
	} catch(AipsError& log) {
        // now put filter back
        LogSink().globalSink().filter(oldFilter);
		try {
			if (factory) delete factory;
		} catch(AipsError ae) {}
		throw(AipsError(log.getMesg()));
	}
    // now put filter back
    LogSink().globalSink().filter(oldFilter);
	if (factory) delete factory;
}

vi::VisibilityIterator2* MSCache::setUpVisIter(MeasurementSet& selectedMS,
	Vector<Vector<Slice> > chansel, Vector<Vector<Slice> > corrsel) {
	// Plain VI2 for chunk counting with baseline/antenna/spw averaging
	// (need to set SortColumns manually)
	Bool combscan(averaging_.scan());
	Bool combfld(averaging_.field());
	Bool combspw(averaging_.spw());

	// Set iterInterval
	Double iterInterval(0.0);
	if (averaging_.time()){
		iterInterval = averaging_.timeValue();
	}
	if (combspw || combfld) iterInterval = DBL_MIN;  // force per-timestamp chunks

	// Create SortColumns
	Int nsortcol(4 + Int(!combscan));  // include room for scan
	Block<Int> columns(nsortcol);
	Int i(0);
	columns[i++]                = MS::ARRAY_ID;
	if (!combscan) columns[i++] = MS::SCAN_NUMBER;  // force scan boundaries
	if (!combfld) columns[i++]  = MS::FIELD_ID;      // force field boundaries
	if (!combspw) columns[i++]  = MS::DATA_DESC_ID;  // force spw boundaries
	columns[i++]                = MS::TIME;
	if (combfld) columns[i++]   = MS::FIELD_ID;      // effectively ignore field boundaries
	if (combspw) columns[i++]   = MS::DATA_DESC_ID;  // effectively ignore spw boundaries
	vi::SortColumns sortcol(columns, False);

	vi::VisibilityIterator2* vi2 = new vi::VisibilityIterator2(selectedMS, sortcol, False, 0, iterInterval);
	vi::FrequencySelectionUsingChannels fs;
	setUpFrequencySelectionChannels(fs, chansel);
	fs.addCorrelationSlices(corrsel);
	// Add FrequencySelection to VI
	vi2->setFrequencySelection(fs);
	return vi2;
}

void MSCache::setUpFrequencySelectionChannels(vi::FrequencySelectionUsingChannels fs,
						Vector<Vector<Slice> > chansel) {
	/* For the plain VI2 for chunk counting */
	int nSpws = static_cast<int>(chansel.nelements());
	int nChansels;
	for (int spw=0; spw<nSpws; spw++) {
		nChansels = static_cast<int>(chansel[spw].nelements());
		// Add channel selections to FrequencySelection
		for ( int sel=0; sel<nChansels; sel++) {
			fs.add(spw, 
			       chansel[spw][sel].start(), 
			       chansel[spw][sel].length(),
			       chansel[spw][sel].inc());
		}
	}
}

void MSCache::countChunks(vi::VisibilityIterator2& vi,
		Vector<Int>& nIterPerAve, 
		ThreadCommunication* thread) {
	/* Let plotms count the chunks for memory estimation 
	   when baseline/antenna/spw averaging */
	if (thread != NULL) {
		thread->setStatus("Establishing cache size.  Please wait...");
		thread->setAllowedOperations(false,false,false);
	}

	Bool verby(false);
	stringstream ss;

	Bool combscan(averaging_.scan());
	Bool combfld(averaging_.field());
	Bool combspw(averaging_.spw());

	vi.originChunks();
	vi.origin();
	vi::VisBuffer2* vb = vi.getVisBuffer();

	Double time0(86400.0 * floor(vb->time()(0)/86400.0));
	Double time1(0.0), time(0.0);
	Int thisscan(-1),lastscan(-1);
	Int thisfld(-1), lastfld(-1);
	Int thisspw(-1),lastspw(-1);
	Int thisddid(-1),lastddid(-1);
	Int chunk(0);

	// Averaging stats
	Int maxAveNRows(0);
	nIterPerAve.resize(100);
	nIterPerAve = 0;	
	Int ave(-1);
	Double avetime1(-1.0);

	// Time averaging interval
	Double interval(0.0);
	if (averaging_.time())
		interval = averaging_.timeValue();

	for (vi.originChunks(); vi.moreChunks(); vi.nextChunk(), chunk++) {
		if (thread!=NULL) {
			if (thread->wasCanceled()) {
				dataLoaded_=false;
				return;
			}
			else 
				thread->setProgress(2);
		}

		for (vi.origin(); vi.more(); vi.next()) {
			time1 = vb->time()(0);  // first time in this vb
			thisscan = vb->scan()(0);
			thisfld = vb->fieldId()(0);
			thisspw = vb->spectralWindows()(0);
			thisddid = vb->dataDescriptionIds()(0);
			// New chunk means new ave interval, IF....
			if ( // (!combfld && !combspw) ||               // not combing fld nor spw, OR
			     ((time1 - avetime1) >= interval) ||         // (combing fld and/or spw) and solint exceeded, OR
		             ((time1 - avetime1) < 0.0) ||              // a negative time step occurs, OR
			     (!combscan && (thisscan != lastscan)) ||   // not combing scans, and new scan encountered OR
			     (!combspw && (thisspw != lastspw)) || // not combing spws, and new spw encountered  OR
			     (!combfld && (thisfld != lastfld)) ||        // not combing fields, and new field encountered OR
			     (ave == -1)) {                            // this is the first interval

				if (verby) {
					ss << "--------------------------------\n";
					ss << boolalpha << interval << " "
							<< ((time1-avetime1) > interval)  << " "
							<< ((time1-avetime1) < 0.0) << " "
							<< (!combscan && (thisscan!=lastscan)) << " "
							<< (!combspw && (thisspw!=lastspw)) << " "
							<< (!combfld && (thisfld!=lastfld)) << " "
							<< (ave == -1) << "\n";
				}

				// If we have accumulated enough info, poke the volume meter,
				//  with the _previous_ info, and reset the ave'd row counter
				if (ave > -1) {
					vm_->add(lastddid, maxAveNRows);
					maxAveNRows = 0;
				}

				avetime1 = time1;  // for next go
				ave++;
				if (verby) ss << "ave = " << ave << "\n";

				// increase size of nIterPerAve array, if needed
				if (nIterPerAve.nelements() < uInt(ave+1))
					nIterPerAve.resize(nIterPerAve.nelements()+100, True);
				nIterPerAve(ave) = 0;
			}

			// Keep track of the maximum # of rows that might get averaged
			maxAveNRows = max(maxAveNRows, vb->nRows());
			// Increment chunk-per-sol count for current solution
			nIterPerAve(ave)++;

			if (verby) {
				ss << "          chunk=" << chunk << " " << avetime1 - time0 << "\n";
				time = vb->time()(0);
				ss  << "                 " << "vb=" << vb->getSubchunk().toString() << " ";
				ss << "ar=" << vb->arrayId()(0) << " ";
				ss << "sc=" << vb->scan()(0) << " ";
				if (!combfld) ss << "fl=" << vb->fieldId()(0) << " ";
				if (!combspw) ss << "sp=" << vb->spectralWindows()(0) << " ";
				ss << "t=" << floor(time - time0)  << " (" << floor(time - avetime1) << ") ";
				if (combfld) ss << "fl=" << vb->fieldId()(0) << " ";
				if (combspw) ss << "sp=" << vb->spectralWindows()(0) << " ";
				ss << "\n";

			}

			lastscan = thisscan;
			lastfld  = thisfld;
			lastspw  = thisspw;
			lastddid = thisddid;
		}
	}
	// Add in the last iteration
	vm_->add(lastddid,maxAveNRows);

	Int nAve(ave+1);
	nIterPerAve.resize(nAve, True);
	if (nChunk_ != nAve) increaseChunks(nAve);

	if (verby) {
		ss << "nIterPerAve = " << nIterPerAve;
		logInfo("count_chunks", ss.str());
	}
	// cout << "Found " << nChunk_ << " chunks." << endl;
}

void MSCache::trapExcessVolume(map<PMS::Axis,Bool> pendingLoadAxes) {
	try {
		String s;
		if (visBufferShapes_.size() > 0) {
			s = vm_->evalVolume(visBufferShapes_, pendingLoadAxes); }
		else {
			Vector<Bool> mask(4, False);
			int dataCount = getDataCount();

			for ( int i = 0; i < dataCount; i++ ){
				Vector<Bool> subMask = netAxesMask( currentX_[i], currentY_[i]);
				mask = mask || subMask;
			}
			s = vm_->evalVolume(pendingLoadAxes, mask);
		}
		logLoad(s);

	} catch(AipsError& log) {
		// catch detected volume excess, clear the existing cache, and rethrow
		logLoad(log.getMesg());
		deleteVm();
		stringstream ss;
		ss << "Please try selecting less data or averaging and/or" << endl
		   << " 'force reload' (to clear unneeded cache items) and/or" << endl
		   << " letting other memory-intensive processes finish.";
		throw(AipsError(ss.str()));
	}
	deleteVm();
}

void MSCache::loadChunks(vi::VisibilityIterator2& vi,
		const vector<PMS::Axis> loadAxes,
		const vector<PMS::DataColumn> loadData,
		ThreadCommunication* thread) {

	// permit cancel in progress meter:
	if(thread != NULL)
		thread->setAllowedOperations(false,false,true);
	logLoad("Loading chunks......");

	// Initialize VI and get VB
	vi.originChunks();
	vi.origin();
	vi::VisBuffer2* vb = vi.getVisBuffer();

	nAnt_ = vb->nAntennas();  // needed to set up indexer
	// set frame; VB2 does not handle N_Types, just passes it along
	// and fails check in MFrequency so handle it here
	freqFrame_ = transformations_.frame();
	if (freqFrame_ == MFrequency::N_Types)
		freqFrame_ = static_cast<MFrequency::Types>(vi.getReportingFrameOfReference());

	Int chunk = 0;
	chshapes_.resize(4,nChunk_);
	goodChunk_.resize(nChunk_);
	goodChunk_.set(False);
	double progress;

	for(vi.originChunks(); vi.moreChunks(); vi.nextChunk()) {
		for(vi.origin(); vi.more(); vi.next()) {
            if (vb->nRows() > 0) {
                if (chunk >= nChunk_) {  // nChunk_ was just an estimate
                    increaseChunks(chunk-nChunk_+1);  // updates nChunk_
                    chshapes_.resize(4, nChunk_, True);
                    goodChunk_.resize(nChunk_, True);
                }

                // If a thread is given, check if the user canceled.
                if(thread != NULL && thread->wasCanceled()) {
                    dataLoaded_ = false;
                    return;
                }
                // If a thread is given, update its chunk number
                if(thread != NULL && (nChunk_ <= (int)THREAD_SEGMENT ||
                        chunk % THREAD_SEGMENT == 0))
                    thread->setStatus("Loading chunk " + String::toString(chunk) +
                            " / " + String::toString(nChunk_) + ".");

                // Cache the data shapes
                chshapes_(0,chunk) = vb->nCorrelations();
                chshapes_(1,chunk) = vb->nChannels();
                chshapes_(2,chunk) = vb->nRows();
                chshapes_(3,chunk) = vb->nAntennas();
                goodChunk_(chunk)  = True;
                for(unsigned int i = 0; i < loadAxes.size(); i++) {
                    loadAxis(vb, chunk, loadAxes[i], loadData[i]);
                }
                chunk++;

                // If a thread is given, update its progress bar
                if(thread != NULL && (nChunk_ <= (int)THREAD_SEGMENT ||
                        chunk % THREAD_SEGMENT == 0)) {
                    progress = ((double)chunk+1) / nChunk_;
                    thread->setProgress((unsigned int)((progress * 100) + 0.5));
                }
            }
		}
	}
}

void MSCache::loadChunks(vi::VisibilityIterator2& vi,
		const PlotMSAveraging& averaging,
		const Vector<Int>& nIterPerAve,
		const vector<PMS::Axis> loadAxes,
		const vector<PMS::DataColumn> loadData,
		ThreadCommunication* thread) {

	// permit cancel in progress meter:
	if(thread != NULL)
		thread->setAllowedOperations(false,false,true);

	logLoad("Loading chunks with averaging.....");
	Bool verby(false);
	stringstream ss;

	vi.originChunks();
	vi.origin();
	vi::VisBuffer2* vb = vi.getVisBuffer();
	nAnt_ = vb->nAntennas();  // needed to set up indexer

	// set frame; VB2 does not handle N_Types, just passes it along
	// and fails check in MFrequency so handle it here
	freqFrame_ = transformations_.frame();
	if (freqFrame_ == MFrequency::N_Types)
		freqFrame_ = static_cast<MFrequency::Types>(vi.getReportingFrameOfReference());

	Double time0 = 86400.0 * floor(vb->time()(0)/86400.0);
	chshapes_.resize(4, nChunk_);
	goodChunk_.resize(nChunk_);
	goodChunk_.set(False);
	double progress;
	Int nAnts;
	vi::VisBuffer2* vbToUse = NULL;

	for (Int chunk=0; chunk<nChunk_; ++chunk) {
		// If a thread is given, check if the user canceled.
		if(thread != NULL && thread->wasCanceled()) {
			dataLoaded_ = false;
			return;
		}

		// If a thread is given, update it.
		if(thread != NULL && (nChunk_ <= (int)THREAD_SEGMENT ||
				chunk % THREAD_SEGMENT == 0))
			thread->setStatus("Loading chunk " + String::toString(chunk) +
					" / " + String::toString(nChunk_) + ".");

		if (chunk >= nChunk_) {  // nChunk_ was just an estimate
			increaseChunks(chunk-nChunk_+1);  // updates nChunk_
			chshapes_.resize(4, nChunk_, True);
			goodChunk_.resize(nChunk_, True);
		}

		// Save some data from vb 
		// (averaged vb not attached to VI so cannot get this later)
		// (and no VB2 set method to do it in averager)
		nAnts = vb->nAntennas();

		// Arrange to accumulate many VBs into one
		PlotMSVBAverager pmsvba(nAnts);
		// Set averaging options in averager
		pmsvba.setBlnAveraging(averaging.baseline());
		pmsvba.setAntAveraging(averaging.antenna());
		pmsvba.setScalarAve(averaging.scalarAve());
		// Sort out which data to read
		discernData(loadAxes,loadData,pmsvba);
		stringstream ss;
		if (verby) {
			ss << chunk << "----------------------------------\n"
			   << "ck=" << chunk << " (" << nIterPerAve(chunk) << ");  "
			   << "sc=" << vb->scan()(0) << " "
			   << "time=" << vb->time()(0)-time0 << " "
			   << "fl=" << vb->fieldId()(0) << " "
			   << "sp=" << vb->spectralWindows()(0) << " " << endl;
		}

		// Accumulate into the averager
		pmsvba.accumulate(*vb);

		
		// Finalize the averaging
		pmsvba.finalizeAverage();
		// The averaged VisBuffer
		vi::VisBuffer2& avb(pmsvba.aveVisBuff());

		

		// Only if the average yielded some data
		if (avb.nRows() > 0) {
			// Cache the data shapes
			chshapes_(0,chunk) = avb.nCorrelations();
			chshapes_(1,chunk) = avb.nChannels();
			chshapes_(2,chunk) = avb.nRows();
			chshapes_(3,chunk) = nAnts;
			goodChunk_(chunk)  = True;

			for(unsigned int i = 0; i < loadAxes.size(); i++) {
				if (useAveragedVisBuffer(loadAxes[i])) 
					vbToUse = &avb;
                else
					vbToUse = vb;
				loadAxis(vbToUse, chunk, loadAxes[i], loadData[i]);
			}
		}
		else {
			// no points in this chunk
			goodChunk_(chunk) = False;
			chshapes_.column(chunk) = 0;
		}

        // Advance to next VB
		vi.next();
		if (verby) ss << " next VB ";

		if (!vi.more() && vi.moreChunks()) {
			// go to first vb in next chunk
			if (verby) ss << "  next chunk";
			vi.nextChunk();
			vi.origin();
		}

		if (verby) ss << "\n";
		if (verby) logLoad(ss.str());
		// If a thread is given, update it.
		if(thread != NULL && (nChunk_ <= (int)THREAD_SEGMENT ||
				chunk % THREAD_SEGMENT == 0)) {
			progress = ((double)chunk+1) / nChunk_;
			thread->setProgress((unsigned int)((progress * 100) + 0.5));
		}
	}

	//cout << boolalpha << "goodChunk_ = " << goodChunk_ << endl;
}



bool MSCache::useAveragedVisBuffer(PMS::Axis axis) {
	// Some axes should be obtained from the VB2 provided by the VI2
	// rather than the averaged VB2, which is not attached
	bool useAvg(True);
	switch(axis) {
	case PMS::CHANNEL:
	case PMS::FREQUENCY:
	case PMS::VELOCITY:
	case PMS::UVDIST_L:
	case PMS::UWAVE:
	case PMS::VWAVE:
	case PMS::WWAVE:
	case PMS::AZ0:
	case PMS::EL0:
	case PMS::HA0:
	case PMS::PA0:
	case PMS::ANTENNA:
	case PMS::AZIMUTH:
	case PMS::ELEVATION:
	case PMS::PARANG:
	case PMS::ROW: {
		useAvg = False;
		break;
	}
	default:
		break;
	}
	return useAvg;
}

void MSCache::forceVBread(vi::VisBuffer2* vb,
		vector<PMS::Axis> loadAxes,
		vector<PMS::DataColumn> loadData) {

	// pre-load requisite pieces of VisBuffer for averaging
	for(unsigned int i = 0; i < loadAxes.size(); i++) {
		switch (loadAxes[i]) {
		case PMS::AMP:
		case PMS::PHASE:
		case PMS::REAL:
		case PMS::IMAG: {
			switch(loadData[i]) {
			case PMS::DATA: {
				vb->visCube();
				break;
			}
			case PMS::MODEL: {
				vb->visCubeModel();
				break;
			}
			case PMS::CORRECTED: {
				vb->visCubeCorrected();
				break;
			}
			case PMS::CORRECTED_DIVIDE_MODEL:
			case PMS::CORRMODEL: {
				vb->visCubeCorrected();
				vb->visCubeModel();
				break;
			}
			case PMS::DATAMODEL: {
				vb->visCube();
				vb->visCubeModel();
				break;
			}
			case PMS::DATA_DIVIDE_MODEL: {
				vb->visCube();
				vb->visCubeModel();
				break;
			}
			case PMS::FLOAT_DATA: {
				vb->visCubeFloat();
				break;
			}
			default:
				break;
			}
			break;
		}
		default:
			break;
		}
	}

	// Always need flags
	vb->flagRow();
	vb->flagCube();

}

void MSCache::discernData(vector<PMS::Axis> loadAxes,
		vector<PMS::DataColumn> loadData,
		PlotMSVBAverager& vba) {

	// Turn off
	vba.setNoData();

	// Tell the averager which data column to read
	for(unsigned int i = 0; i < loadAxes.size(); i++) {
		switch (loadAxes[i]) {
		case PMS::AMP:
		case PMS::PHASE:
		case PMS::REAL:
		case PMS::IMAG: {
			switch(loadData[i]) {
			case PMS::DATA: {
				// cout << "Arranging to load VC." << endl;
				vba.setDoVC();
				break;
			}
			case PMS::MODEL: {
				// cout << "Arranging to load MVC." << endl;
				vba.setDoMVC();
				break;
			}
			case PMS::CORRECTED: {
				// cout << "Arranging to load CVC." << endl;
				vba.setDoCVC();
				break;
			}
			case PMS::CORRECTED_DIVIDE_MODEL:
			case PMS::CORRMODEL: {
				// cout << "Arranging to load CVC & MVC." << endl;
				vba.setDoCVC();
				vba.setDoMVC();
				break;
			}
			case PMS::DATAMODEL: {
				vba.setDoVC();
				vba.setDoMVC();
			}
			case PMS::DATA_DIVIDE_MODEL: {
				vba.setDoVC();
				vba.setDoMVC();
			}
			case PMS::FLOAT_DATA:
				vba.setDoFC();
				break;
			default:
				break;
			}
			break;
		}
		case PMS::UVDIST:
		case PMS::UVDIST_L:
		case PMS::U:
		case PMS::V:
		case PMS::W:
		case PMS::UWAVE:
		case PMS::VWAVE:
		case PMS::WWAVE: {
			//  cout << "Arranging to load UVW
			vba.setDoUVW();
		}
		default:
			break;
		}
	}

}




void MSCache::loadAxis(vi::VisBuffer2* vb, Int vbnum, PMS::Axis axis,
		PMS::DataColumn data) {

	switch(axis) {

	case PMS::SCAN: // assumes scan unique in VB
		scan_(vbnum) = vb->scan()(0);
		break;

	case PMS::FIELD: // assumes field unique in VB
		field_(vbnum) = vb->fieldId()(0);
		break;

	case PMS::TIME: // assumes time unique in VB
		time_(vbnum) = vb->time()(0);
		break;

	case PMS::TIME_INTERVAL: // assumes timeInterval unique in VB
		timeIntr_(vbnum) = vb->timeInterval()(0);
		break;

	case PMS::SPW:
		spw_(vbnum) = vb->spectralWindows()(0);
		break;

	case PMS::CHANNEL:
		*chan_[vbnum] = vb->getChannelNumbers(0);
		break;

	case PMS::FREQUENCY: {
		// Convert freq to desired frame
  		*freq_[vbnum] = vb->getFrequencies(0, freqFrame_);
		(*freq_[vbnum]) /= 1.0e9; // in GHz
		break;
	}
	case PMS::VELOCITY: {
		/*
		// Convert freq in the vb to velocity
		VisBufferUtil vbu = VisBufferUtil();
		vbu.toVelocity(*vel_[vbnum],
				*vb,
				freqFrame_,
				MVFrequency(transformations_.restFreqHz()),
				transformations_.veldef());
		(*vel_[vbnum]) /= 1.0e3;  // in km/s
		*/
		*vel_[vbnum] = calcVelocity(vb); 
		break;
	}

	case PMS::CORR: {
        Vector<Stokes::StokesTypes> corrTypes = vb->getCorrelationTypesSelected();
        Vector<Int> corrTypesInt;
        corrTypesInt.resize(corrTypes.size());
        for (uInt i=0; i<corrTypes.size(); ++i) {
            corrTypesInt[i] = static_cast<Int>(corrTypes[i]);
        }
		*corr_[vbnum] = corrTypesInt;
		break;
    }

	case PMS::ANTENNA1:
		*antenna1_[vbnum] = vb->antenna1();
		break;
	case PMS::ANTENNA2:
		*antenna2_[vbnum] = vb->antenna2();
		break;
	case PMS::BASELINE: {
		Vector<Int> a1(vb->antenna1());
		Vector<Int> a2(vb->antenna2());
		baseline_[vbnum]->resize(vb->nRows());
		Vector<Int> bl(*baseline_[vbnum]);
		for (Int irow = 0; irow < vb->nRows(); ++irow) {
			if (a1(irow)<0) a1(irow)=chshapes_(3,0);
			if (a2(irow)<0) a2(irow)=chshapes_(3,0);
			bl(irow)=(chshapes_(3,0)+1)*a1(irow) - (a1(irow) * (a1(irow) + 1)) / 2 + a2(irow);
		}
		break;
	}
	case PMS::UVDIST: {
		Array<Double> u(vb->uvw().row(0));
		Array<Double> v(vb->uvw().row(1));
		*uvdist_[vbnum] = sqrt(u*u+v*v);
		break;
	}
	case PMS::U:
		*u_[vbnum] = vb->uvw().row(0);
		break;
	case PMS::V:
		*v_[vbnum] = vb->uvw().row(1);
		break;
	case PMS::W:
		*w_[vbnum] = vb->uvw().row(2);
		break;
	case PMS::UVDIST_L: {
		Array<Double> u(vb->uvw().row(0));
		Array<Double> v(vb->uvw().row(1));
		Vector<Double> uvdistM = sqrt(u*u + v*v);
		uvdistM /=C::c;
		uvdistL_[vbnum]->resize(vb->nChannels(), vb->nRows());
		Vector<Double> uvrow;
		for (Int irow = 0; irow < vb->nRows(); ++irow) {
			uvrow.reference(uvdistL_[vbnum]->column(irow));
			uvrow.set(uvdistM(irow));
			uvrow *= vb->getFrequencies(irow, freqFrame_);
		}
		break;
	}

	case PMS::UWAVE: {
		Vector<Double> uM(vb->uvw().row(0));
		uM/=C::c;
		uwave_[vbnum]->resize(vb->nChannels(), vb->nRows());
		Vector<Double> urow;
		for (Int irow = 0; irow < vb->nRows(); ++irow) {
			urow.reference(uwave_[vbnum]->column(irow));
			urow.set(uM(irow));
			urow *= vb->getFrequencies(irow, freqFrame_);
		}
		break;
	}
	case PMS::VWAVE: {
		Vector<Double> vM(vb->uvw().row(1));
		vM/=C::c;
		vwave_[vbnum]->resize(vb->nChannels(), vb->nRows());
		Vector<Double> vrow;
		for (Int irow = 0; irow < vb->nRows(); ++irow) {
			vrow.reference(vwave_[vbnum]->column(irow));
			vrow.set(vM(irow));
			vrow *= vb->getFrequencies(irow, freqFrame_);
		}
		break;
	}
	case PMS::WWAVE: {
		Vector<Double> wM(vb->uvw().row(2));
		wM/=C::c;
		wwave_[vbnum]->resize(vb->nChannels(), vb->nRows());
		Vector<Double> wrow;
		for (Int irow = 0; irow < vb->nRows(); ++irow) {
			wrow.reference(wwave_[vbnum]->column(irow));
			wrow.set(wM(irow));
			wrow *= vb->getFrequencies(irow, freqFrame_);
		}
		break;
	}
	case PMS::AMP: {
		switch(data) {
		case PMS::DATA: {
			//CAS-5730.  For single dish data, absolute value of
			//points should not be plotted.
			MeasurementSet ms( filename_);
			if ( ms.isColumn( MS::FLOAT_DATA ) ){
				*amp_[vbnum]=real(vb->visCube());
			}
			else {
				*amp_[vbnum] = amplitude(vb->visCube());
			}
			// TEST fft on freq axis to get delay
			if (False) {

				// Only transform frequency axis
				//   (Should avoid cross-hand data, too?)
				Vector<Bool> ax(3,False);
				ax(1) = True;

				// Support padding for higher delay resolution
				Int fact(4);
				IPosition ip = vb->visCube().shape();
				Int nch = ip(1);
				ip(1) *= fact;

				Slicer sl(Slice(),Slice(nch*(fact-1)/2,nch,1),Slice());

				Array<Complex> vpad(ip);
				vpad.set(Complex(0.0));
				vpad(sl) = vb->visCube();


				cout << "vpad.shape() = " << vpad.shape() << endl;
				cout << "vpad(sl).shape() = " << vpad(sl).shape() << endl;

				Vector<Complex> testf(64,Complex(1.0));
				FFTServer<Float,Complex> ffts;
				cout << "FFTServer..." << flush;
				ffts.fft(testf,True);
				cout << "done." << endl;

				ArrayLattice<Complex> tf(testf);
				cout << "tf.isWritable() = " << boolalpha << tf.isWritable() << endl;

				LatticeFFT::cfft(tf,False);
				cout << "testf = " << testf << endl;


				cout << "Starting ffts..." << flush;

				ArrayLattice<Complex> c(vpad);
				cout << "c.shape() = " << c.shape() << endl;
				//	  LatticeFFT::cfft(c,ax);
				LatticeFFT::cfft2d(c,False);

				cout << "done." << endl;

				*amp_[vbnum] = amplitude(vpad(sl));
			}
			break;

		}
		case PMS::MODEL: {
			*amp_[vbnum] = amplitude(vb->visCubeModel());
			break;
		}
		case PMS::CORRECTED: {

			*amp_[vbnum] = amplitude(vb->visCubeCorrected());
			break;
		}
		case PMS::CORRMODEL: {
			*amp_[vbnum] = amplitude(vb->visCubeCorrected() - vb->visCubeModel());
			break;
		}
		case PMS::DATAMODEL: {
			*amp_[vbnum] = amplitude(vb->visCube() - vb->visCubeModel());
			break;
		}
		case PMS::DATA_DIVIDE_MODEL: {
			*amp_[vbnum] = amplitude( vb->visCube() / vb->visCubeModel());
			break;
		}
		case PMS::CORRECTED_DIVIDE_MODEL: {
			*amp_[vbnum] = amplitude( vb->visCubeCorrected() / vb->visCubeModel());
			break;
		}
		case PMS::FLOAT_DATA: {
			*amp_[vbnum] = vb->visCubeFloat();
			break;
		}
		}
		break;
	}
	case PMS::PHASE: {
		switch(data) {
		case PMS::DATA: {
			*pha_[vbnum] = phase(vb->visCube()) * 180.0 / C::pi;
			break;
		}
		case PMS::MODEL: {
			*pha_[vbnum] = phase(vb->visCubeModel()) * 180.0 / C::pi;
			break;
		}
		case PMS::CORRECTED: {
			*pha_[vbnum] = phase(vb->visCubeCorrected()) * 180.0 / C::pi;
			break;
		}

		case PMS::CORRMODEL: {
			*pha_[vbnum] = phase(vb->visCubeCorrected() - vb->visCubeModel()) *
					180.0 / C::pi;
			break;
		}
		case PMS::DATAMODEL: {
			*pha_[vbnum] = phase(vb->visCube() - vb->visCubeModel()) * 180 / C::pi;
			break;
		}
		case PMS::DATA_DIVIDE_MODEL: {
			*pha_[vbnum] = phase(vb->visCube() / vb->visCubeModel()) * 180 / C::pi;
			break;
		}
		case PMS::CORRECTED_DIVIDE_MODEL: {
			*pha_[vbnum] = phase(vb->visCubeCorrected() / vb->visCubeModel()) * 180 / C::pi;
			break;
		}
		case PMS::FLOAT_DATA:  // should have caught this already
			break;
		}
		break;
	}

	case PMS::REAL: {
		switch(data) {
		case PMS::DATA: {
			*real_[vbnum] = real(vb->visCube());
			break;
		}
		case PMS::MODEL: {
			*real_[vbnum] = real(vb->visCubeModel());
			break;
		}
		case PMS::CORRECTED: {
			*real_[vbnum] = real(vb->visCubeCorrected());
			break;
		}
		case PMS::CORRMODEL: {
			*real_[vbnum] = real(vb->visCubeCorrected()) - real(vb->visCubeModel());
			break;
		}
		case PMS::DATAMODEL: {
			*real_[vbnum] = real(vb->visCube()) - real(vb->visCubeModel());
			break;
		}
		case PMS::DATA_DIVIDE_MODEL: {
			*real_[vbnum] = real(vb->visCube()) / real(vb->visCubeModel());
			break;
		}
		case PMS::CORRECTED_DIVIDE_MODEL: {
			*real_[vbnum] = real(vb->visCubeCorrected()) / real(vb->visCubeModel());
			break;
		}
		case PMS::FLOAT_DATA:
            *real_[vbnum] = vb->visCubeFloat();  // float data is real
			break;
		}
		break;
	}
	case PMS::IMAG: {
		switch(data) {
		case PMS::DATA: {
			*imag_[vbnum] = imag(vb->visCube());
			break;
		}
		case PMS::MODEL: {
			*imag_[vbnum] = imag(vb->visCubeModel());
			break;
		}
		case PMS::CORRECTED: {
			*imag_[vbnum] = imag(vb->visCubeCorrected());
			break;
		}
		case PMS::CORRMODEL: {
			*imag_[vbnum] = imag(vb->visCubeCorrected()) - imag(vb->visCubeModel());
			break;
		}
		case PMS::DATAMODEL: {
			*imag_[vbnum] = imag(vb->visCube()) - imag(vb->visCubeModel());
			break;
		}
		case PMS::DATA_DIVIDE_MODEL: {
			*imag_[vbnum] = imag(vb->visCube()) / imag(vb->visCubeModel());
			break;
		}
		case PMS::CORRECTED_DIVIDE_MODEL: {
			*imag_[vbnum] = imag(vb->visCubeCorrected()) / imag(vb->visCubeModel());
			break;
		}
		case PMS::FLOAT_DATA:  // should have caught this already
			break;
		}
		break;
	}

	case PMS::FLAG:
		*flag_[vbnum] = vb->flagCube();
		break;
	case PMS::FLAG_ROW:
		*flagrow_[vbnum] = vb->flagRow();
		break;

	case PMS::WT: {
		*wt_[vbnum] = vb->weight();
		break;
	}

	case PMS::WTSP: {
	        if (vb->weightSpectrum().nelements()>0)
			*wtsp_[vbnum] = vb->weightSpectrum();
		else
	  		throw(AipsError("This MS does not have a valid WEIGHT_SPECTRUM column."));
	}

	case PMS::WTxAMP: {
		switch(data) {
		case PMS::DATA:
			*wtxamp_[vbnum] = amplitude(vb->visCube());
			break;
		case PMS::MODEL:
			*wtxamp_[vbnum] = amplitude(vb->visCubeModel());
			break;
		case PMS::CORRECTED:
			*wtxamp_[vbnum] = amplitude(vb->visCubeCorrected());
			break;
		case PMS::CORRMODEL:
			*wtxamp_[vbnum] = amplitude(vb->visCubeCorrected() - vb->visCube());
			break;
		case PMS::DATAMODEL:
			*wtxamp_[vbnum] = amplitude(vb->visCube() - vb->visCubeModel());
			break;
		case PMS::DATA_DIVIDE_MODEL:
			*wtxamp_[vbnum] = amplitude(vb->visCube() / vb->visCubeModel());
			break;
		case PMS::CORRECTED_DIVIDE_MODEL:
			*wtxamp_[vbnum] = amplitude(vb->visCubeCorrected() / vb->visCubeModel());
			break;
		case PMS::FLOAT_DATA: 
			*wtxamp_[vbnum] = vb->visCubeFloat();
			break;
		}
		uInt nchannels = vb->nChannels();
		Cube<Float> wtA(*wtxamp_[vbnum]);
		for(uInt c = 0; c < nchannels; ++c) {
			wtA.xzPlane(c) = wtA.xzPlane(c) * vb->weight();
		}
	}

	case PMS::SIGMA:
		*sigma_[vbnum] = vb->sigma();
		break;
	case PMS::SIGMASP:
		*sigmasp_[vbnum] = vb->sigmaSpectrum();
		break;

	case PMS::AZ0:
	case PMS::EL0: {
		MDirection azelMDir = vb->azel0(vb->time()(0));
		Vector<Double> azelVec = azelMDir.getAngle("deg").getValue();
		az0_(vbnum) = azelVec(0);
		el0_(vbnum) = azelVec(1);
		break;
	}

	case PMS::HA0:
		ha0_(vbnum) = vb->hourang(vb->time()(0))*12/C::pi;  // in hours
		break;
	case PMS::PA0: {
		pa0_(vbnum) = vb->parang0(vb->time()(0))*180.0/C::pi; // in degrees
		if (pa0_(vbnum)<0.0) pa0_(vbnum)+=360.0;
		break;
	}
	case PMS::ANTENNA: {
		antenna_[vbnum]->resize(vb->nAntennas());
		indgen(*antenna_[vbnum]);
		break;
	}
	case PMS::AZIMUTH:
	case PMS::ELEVATION: {
		Vector<MDirection> azelVec = vb->azel(vb->time()(0));
		Matrix<Double> azelMat;
                azelMat.resize(2, azelVec.nelements());
		for (uInt iant = 0; iant < azelVec.nelements(); ++iant) {
			azelMat.column(iant) = (azelVec(iant).getAngle("deg").getValue());
		}
		*az_[vbnum] = azelMat.row(0);
		*el_[vbnum] = azelMat.row(1);
		break;
	}
	case PMS::RADIAL_VELOCITY: {
		Int fieldId = vb->fieldId()(0);
		const ROMSFieldColumns& fieldColumns = vi_p->subtableColumns().field();
		MRadialVelocity radVelocity = fieldColumns.radVelMeas(fieldId, vb->time()(0));
		radialVelocity_(vbnum) = radVelocity.get("AU/d").getValue( "km/s");
		break;
	}
	case PMS::RHO:{
		Int fieldId = vb->fieldId()(0);
		const ROMSFieldColumns& fieldColumns = vi_p->subtableColumns().field();
		Quantity rhoQuantity = fieldColumns.rho(fieldId, vb->time()(0));
		rho_(vbnum ) = rhoQuantity.getValue( "km");
		break;
	}
	case PMS::PARANG:
		*parang_[vbnum] = vb->feedPa(vb->time()(0))*(180.0/C::pi);  // in degrees
		break;

	case PMS::ROW:
		*row_[vbnum] = vb->rowIds();
		break;

	case PMS::OBSERVATION:
		*obsid_[vbnum] = vb->observationId();
		break;

	case PMS::INTENT:{
		Vector<Int> states = vb->stateId();
		*intent_[vbnum] = assignIntentIds(states);
		break;
	}

	default:
		throw(AipsError("Axis choice not supported for MS"));
		break;
	}
}

bool MSCache::isEphemeris(){
	if ( !ephemerisInitialized ){
		Table::TableOption tabopt(Table::Old);
		MeasurementSet ms(filename_,TableLock(TableLock::AutoLocking), tabopt);
		ROMSColumns msc(ms);

        // Check the field subtable for ephemeris fields
		const ROMSFieldColumns& fieldColumns = msc.field();
        uInt nrow = fieldColumns.nrow();

		ephemerisAvailable = true;
        String ephemPath;
        for (uInt i=0; i<nrow; ++i) {
            ephemPath = fieldColumns.ephemPath(i);
		    if ( ephemPath.empty()) ephemerisAvailable = false;
		}
		ephemerisInitialized = true;
	}
	return ephemerisAvailable;
}


void MSCache::flagToDisk(const PlotMSFlagging& flagging,
		Vector<Int>& flchunks, Vector<Int>& flrelids,
		Bool setFlag, PlotMSIndexer* indexer, int dataIndex) {

	// Sort the flags by chunk:
	Sort sorter;
	sorter.sortKey(flchunks.data(),TpInt);
	sorter.sortKey(flrelids.data(),TpInt);
	Vector<uInt> order;
	uInt nflag;
	nflag = sorter.sort(order,flchunks.nelements());

	stringstream ss;

	// Establish a scope in which the VisBuffer is properly created/destroyed
	{
		setUpVisIter(selection_, calibration_, "DATA", True, False);

		vi_p->originChunks();
		vi_p->origin();
		vi::VisBuffer2* vb = vi_p->getVisBuffer();

		Int iflag(0);
		for (Int ichk=0; ichk<nChunk_; ++ichk) {

			if (ichk != flchunks(order[iflag])) {
				// Step over current chunk
				for (Int i=0;i<nVBPerAve_(ichk);++i) {
					vi_p->next();
					if (!vi_p->more() && vi_p->moreChunks()) {
						vi_p->nextChunk();
						vi_p->origin();
					}
				}
			}
			else if ( averaging_.baseline() ||
				  averaging_.antenna() ||
				  averaging_.spw() ) {

				// This chunk requires flag-setting

				// For each VB in this cache chunk
				Int ifl(iflag);
				for (Int i=0; i<nVBPerAve_(ichk); ++i) {

					// Refer to VB pieces we need
					Cube<Bool> vbflag(vb->flagCube());
					Vector<Bool> vbflagrow(vb->flagRow());
					Vector<Int> a1(vb->antenna1());
					Vector<Int> a2(vb->antenna2());
					Int ncorr = vb->nCorrelations();
					Int nchan = vb->nChannels();
					Int nrow  = vb->nRows();

					if (False) {
						Int currChunk = flchunks(order[iflag]);
						Double time = getTime(currChunk,0);
						Int spw = Int(getSpw(currChunk,0));
						Int field = Int(getField(currChunk,0));
						ss << "Time diff: " << time - vb->time()(0) << " "  << time << " " << vb->time()(0) << "\n";
						ss << "Spw diff:  " << spw - vb->spectralWindows()(0) << " " << spw << " " << vb->spectralWindows()(0) << "\n";
						ss << "Field diff:  " << field - vb->fieldId()(0) << " " << field << " " << vb->fieldId()(0) << "\n";
					}

					// Apply all flags in this chunk to this VB
					ifl=iflag;
					while (ifl<Int(nflag) && flchunks(order[ifl])==ichk) {

						Int currChunk=flchunks(order[ifl]);
						Int irel=flrelids(order[ifl]);

						Slice corr,chan,bsln;

						// Set flag range on correlation axis:
						if (netAxesMask_[dataIndex](0) && !flagging.corrAll()) {
							// A specific single correlation
							Int icorr=indexer->getIndex1000(currChunk,irel);
							corr=Slice(icorr,1,1);
						}
						else
							corr=Slice(0,ncorr,1);

						// Set Flag range on channel axis:
						if (netAxesMask_[dataIndex](1) && !flagging.channel()) {
							Int ichan=indexer->getIndex0100(currChunk,irel);
							// A single specific channel
							chan=Slice(ichan,1,1);
						}
						else
							// Extend to all channels
							chan=Slice(0,nchan,1);

						// Set Flags on the baseline axis:
						Int thisA1=Int(getAnt1(currChunk,indexer->getIndex0010(currChunk,irel)));
						Int thisA2=Int(getAnt2(currChunk,indexer->getIndex0010(currChunk,irel)));
						if (netAxesMask_[dataIndex](2) &&
								!flagging.antennaBaselinesBased() &&
								thisA1>-1 ) {
							// i.e., if baseline is an explicit data axis,
							//       full baseline extension is OFF
							//       and the first antenna in the selected point is > -1

							// Do some variety of detailed per-baseline flagging
							for (Int irow=0;irow<nrow;++irow) {

								if (thisA2>-1) {
									// match a baseline exactly
									if (a1(irow)==thisA1 &&
											a2(irow)==thisA2) {
										vbflag(corr,chan,Slice(irow,1,1)) = setFlag;
										if (!setFlag) vbflagrow(irow) = False;   // unset flag_row when unflagging

										break;  // found the one baseline, escape from for loop
									}
								}
								else {
									// either antenna matches the one specified antenna
									//  (don't break because there will be more than one)
									//  TBD: this doesn't get cross-hands quite right when
									//    averaging 'by antenna'...
									if (a1(irow)==thisA1 ||
											a2(irow)==thisA1) {
										vbflag(corr,chan,Slice(irow,1,1)) = setFlag;
										if (!setFlag) vbflagrow(irow) = False;   // unset flag_row when unflagging
									}
								}
							}
						}
						else {
							// Set flags for all baselines, because the plot
							//  is ordinarily implicit in baseline, we've turned on baseline
							//  extension, or we've avaraged over all baselines
							bsln=Slice(0,nrow,1);
							vbflag(corr,chan,bsln) = setFlag;
							if (!setFlag) vbflagrow(bsln) = False;   // unset flag_row when unflagging
						}

						++ifl;
					}

					// Put the flags back into the MS
					vi_p->writeFlag(vbflag);
					// This is now done by MSTransformIterator
					// when writeFlag is called
					//if (!setFlag) vi_p->writeFlagRow(vbflagrow);

					// Advance to the next vb
					vi_p->next();
					if (!vi_p->more() && vi_p->moreChunks()) {
						vi_p->nextChunk();
						vi_p->origin();
					}
				}  // VBs in this averaging chunk

				// step over the flags we've just done
				iflag=ifl;

				// Escape if we are already finished
				if (uInt(iflag)>=nflag) break;

			} // flaggable VB with averaging handled by plotms
			else {
				vi_p->writeFlag(flag(ichk));
				// This is now done by MSTransformIterator
				// when writeFlag is called
				//if (!setFlag) vi_p->writeFlagRow(flagrow(ichk));

				// Advance to the next vb
				vi_p->next();
				if (!vi_p->more() && vi_p->moreChunks()) {
					vi_p->nextChunk();
					vi_p->origin();
				}
				// Advance to the next flag
				++iflag;
				// Escape if we are already finished
				if (uInt(iflag) >= nflag) break;
			} // flaggable VB with averaging handled by MSTransformIterator

		} // ichk

		// Close the scope that holds the VisBuffer used above
	}

	// Delete the VisIter so lock is released
	deleteVi();

	logFlag(ss.str());

}

void MSCache::mapIntentNamesToIds() {
	intentIds_.clear();
	Int newId = 0;

	for (uInt i=0; i<intentnames_.size(); i++) {
		if (intentIds_.find(intentnames_[i]) == intentIds_.end()) {
			intentIds_[intentnames_[i]] = newId;
			newId++;
		}
	}
}

Vector<Int> MSCache::assignIntentIds(Vector<Int>& stateIds) {
	Vector<Int> intents;
	intents.resize(stateIds.size());

	for (uInt i=0; i<stateIds.size(); i++) {
		if ((intentnames_.size() > 0) && (stateIds[i] >= 0))
			intents[i] = intentIds_[intentnames_[stateIds[i]]];
		else
			intents[i] = stateIds[i];
	}

	return intents;
}

Vector<Double> MSCache::calcVelocity(vi::VisBuffer2* vb) {
	// Convert freq in the vb to velocity
	Vector<Double> outVel;
	VisBufferUtil vbu = VisBufferUtil();
	vbu.toVelocity(outVel,
			*vb,
			freqFrame_,
			MVFrequency(transformations_.restFreqHz()),
			transformations_.veldef());
	outVel /= 1.0e3;  // in km/s
	return outVel;
}

}
