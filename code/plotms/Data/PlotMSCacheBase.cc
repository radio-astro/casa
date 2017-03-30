//# PlotMSCache2.cc: Data cache for plotms.
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
#include <plotms/Data/PlotMSCacheBase.h>
#include <plotms/Data/PlotMSIndexer.h>
#include <plotms/Threads/ThreadCommunication.h>
#include <casa/OS/Timer.h>
#include <casa/OS/HostInfo.h>
#include <casa/OS/Memory.h>
#include <casa/Quanta/MVTime.h>
#include <casa/System/Aipsrc.h>
#include <casa/Utilities/Sort.h>
#include <lattices/Lattices/ArrayLattice.h>
#include <lattices/LatticeMath/LatticeFFT.h>
#include <scimath/Mathematics/FFTServer.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <measures/Measures/Stokes.h>
#include <msvis/MSVis/VisSet.h>
#include <msvis/MSVis/VisBuffer2.h>
#include <msvis/MSVis/VisBufferUtil.h>
#include <plotms/Data/PlotMSVBAverager.h>
#include <plotms/PlotMS/PlotMS.h>
#include <tables/Tables/Table.h>
#include <QDebug>

#include <unistd.h>

using namespace casacore;
namespace casa {

const unsigned int PlotMSCacheBase::N_METADATA = 13;

const PMS::Axis PlotMSCacheBase::METADATA[] =
{
		PMS::TIME,
		PMS::FIELD,
		PMS::SPW,
		PMS::CHANNEL,
		PMS::FREQUENCY,
		PMS::CORR,
		PMS::SCAN,
		PMS::ANTENNA1,
		PMS::ANTENNA2,
		PMS::BASELINE,
		PMS::FLAG,
		PMS::OBSERVATION,
		PMS::INTENT,
};

//      PMS::TIME_INTERVAL,
//      PMS::FLAG_ROW


bool PlotMSCacheBase::axisIsMetaData(PMS::Axis axis) {
	for(unsigned int i = 0; i < N_METADATA; i++)
		if(METADATA[i] == axis) return true;
	return false;
}

const unsigned int PlotMSCacheBase::THREAD_SEGMENT = 10;

PlotMSCacheBase::PlotMSCacheBase(PlotMSApp* parent):
		  plotms_(parent),
		  indexer0_(NULL),
		  indexer_(0),
		  nChunk_(0),
		  refTime_p(0.0),
		  nAnt_(0),
		  minX_(0),
		  maxX_(0),
		  minY_(0),
		  maxY_(0),
		  time_(),
		  timeIntr_(),
		  field_(),
		  spw_(),
		  scan_(),
		  dataLoaded_(false),
		  userCanceled_(false),
          xminG_(0),
          yminG_(0),
          xmaxG_(0),
          ymaxG_(0),
          calType_(""),
          polnRatio_(false)
{

	// Make the empty indexer0 object so we have and empty PlotData object
	int dataCount = 1;
	currentX_.resize(dataCount, PMS::DEFAULT_XAXIS);
	currentY_.resize(dataCount, PMS::DEFAULT_YAXIS);
	currentXData_.resize(dataCount, PMS::DEFAULT_DATACOLUMN);
	currentYData_.resize(dataCount, PMS::DEFAULT_DATACOLUMN);
	indexer0_ = new PlotMSIndexer();
	indexer_.resize(dataCount);
	netAxesMask_.resize( dataCount );
	plmask_.resize( dataCount );
	for ( int i = 0; i < dataCount; i++ ){
		netAxesMask_[i].resize(4,false);
		indexer_[i].set( NULL );
		plmask_[i].set( NULL  );
	}

	// Set up loaded axes to be initially empty, and set up data columns for
	// data-based axes.
	const vector<PMS::Axis>& axes = PMS::axes();
	for(unsigned int i = 0; i < axes.size(); i++) {
		loadedAxes_[axes[i]] = false;
		//if(PMS::axisIsData(axes[i]))
		//	loadedAxesData_[axes[i]]= PMS::DEFAULT_DATACOLUMN;
	}
    loadedAxesData_.clear();
	this->iterAxis = PMS::NONE;

    // Default frequency frame
    freqFrame_ = MFrequency::N_Types;
}

PlotMSCacheBase::~PlotMSCacheBase() {

	//  cout << "PMSCB::~PMSCB" << endl;
	
	delete indexer0_;

	// Deflate everything
	deleteIndexer();
	deletePlotMask();
	deleteCache();
}

Int PlotMSCacheBase::nIter( int dataIndex ) const {
	  int iterationCount = -1;
	  int indexerSize = indexer_.size();
	  if ( 0 <= dataIndex && dataIndex < indexerSize ){
		  if ( !indexer_[dataIndex].empty() ){
		  	  iterationCount = indexer_[dataIndex].nelements();
	  	  }
	  }
	  return iterationCount;
 };

vector<PMS::Axis> PlotMSCacheBase::loadedAxes() const {    
	// have to const-cast loaded axes because the [] operator is not const,
	// even though we're not changing it.
	map<PMS::Axis, bool>& la = const_cast<map<PMS::Axis, bool>& >(loadedAxes_);

	vector<PMS::Axis> v;
	const vector<PMS::Axis>& axes = PMS::axes();
	for(unsigned int i = 0; i < axes.size(); i++)
		if(la[axes[i]])
			v.push_back(axes[i]);

	return v;
}

Record PlotMSCacheBase::locateInfo(int plotIterIndex, const Vector<PlotRegion>& regions,
    		bool showUnflagged, bool showFlagged, bool selectAll ){
	Record record;
	int indexCount = indexer_.size();
	int dataCount = getDataCount();
	if ( 0 <= plotIterIndex && plotIterIndex < indexCount){
		for ( int i = 0; i < dataCount; i++ ){
			Record subRecord = indexer_[i][plotIterIndex]->locateInfo(regions, showUnflagged,
							showFlagged, selectAll);
			record.defineRecord( i, subRecord );
		}
	}
	return record;
}

PMS::Axis PlotMSCacheBase::getIterAxis() const {
	return iterAxis;
}

PlotLogMessage* PlotMSCacheBase::locateRange( int plotIterIndex, const Vector<PlotRegion> & regions,
     		bool showUnflagged, bool showFlagged){
	PlotLogMessage* mesg = NULL;
	String mesgContents;
	int dataCount = indexer_.size();
	if ( dataCount == 1 ){
		int indexCount = indexer_[0].size();
		if ( 0 <= plotIterIndex && plotIterIndex < indexCount){
			mesg = indexer_[0][plotIterIndex]->locateRange( regions, showUnflagged, showFlagged );
		}
	}
	else {
		String contents;
		PlotLogMessage* subMesg = NULL;
		for ( int i = 0; i < dataCount; i++ ){
			int indexCount = indexer_[i].size();
			if ( 0 <= plotIterIndex && plotIterIndex < indexCount){
				subMesg = indexer_[i][plotIterIndex]->locateRange( regions, showUnflagged, showFlagged );
				contents.append(String::toString(i+1));
				contents.append( ": ");
				contents.append( subMesg->message() );
				contents.append( "\n");
			}
		}
		mesg = subMesg;
		mesg->message( contents );
	}
	return mesg;
}

PlotLogMessage* PlotMSCacheBase::flagRange( int plotIterIndex, casa::PlotMSFlagging& flagging,
     		const Vector<PlotRegion>& regions, bool showFlagged){
	PlotLogMessage* mesg = NULL;
	int dataCount = indexer_.size();
	if ( dataCount == 0 ){
		int indexCount = indexer_[0].size();
		if ( 0 <= plotIterIndex && plotIterIndex < indexCount){
			mesg = indexer_[0][plotIterIndex]->flagRange( flagging, regions, showFlagged );
		}
	}
	else {
		String contents;
		PlotLogMessage* subMesg = NULL;
		for ( int i = 0; i < dataCount; i++ ){
			int indexCount = indexer_[i].size();
			if ( 0 <= plotIterIndex && plotIterIndex < indexCount){
				subMesg = indexer_[i][plotIterIndex]->flagRange( flagging, regions, showFlagged );
				contents.append(String::toString(i+1));
				contents.append( ": ");
				contents.append( subMesg->message() );
				contents.append( "\n");
			}
		}
		mesg = subMesg;
		mesg->message( contents );
	}
	return mesg;
}


void PlotMSCacheBase::load(const vector<PMS::Axis>& axes,
		const vector<PMS::DataColumn>& data,
		const String& filename,
		const PlotMSSelection& selection,
		const PlotMSAveraging& averaging,
		const PlotMSTransformations& transformations,
		const PlotMSCalibration& calibration,
		ThreadCommunication* thread) {

	// TBD:
	// o Should we have ONE PtrBlock to a list of Records, each of which
	//    we fill with desired data/info, rather than private data for
	//    every possible option?  (Keys become indices, etc., Eventual
	//    disk paging?)
	// o Partial appends? E.g., if we have to go back and get something
	//    that we didn't get the first time around.  Use of Records as above
	//    may make this easier, too.
	// o ***Handle failure better: mainly leave state sensible for prior
	//    plot, if one exists.

	// need a way to keep track of whether:
	// 1) we already have the metadata loaded
	// 2) the underlying MS has changed, requiring a reloading of metadata

    userCanceled_ = false;

    // Trap ratio plots, only for cal tables
    if ((selection.corr()=='/') && (cacheType()==PlotMSCacheBase::MS))
	    throw(AipsError("Polarization ratio plots not supported for measurement sets."));

	// Remember the axes that we will load for plotting:
	currentX_.clear();
	currentY_.clear();
	currentXData_.clear();
	currentYData_.clear();
	int dataCount = axes.size() / 2;
	for ( int i = 0; i < dataCount; i++ ){
		currentX_.push_back(axes[i]);
        currentXData_.push_back(data[i]);
		currentY_.push_back(axes[dataCount+i]);
        currentYData_.push_back(data[dataCount+i]);
	}

	// Maintain access to this msname, selection, & averager, because we'll
	// use it if/when we flag, etc.
	if ( filename_ != filename ){
		ephemerisInitialized = false;
	    const vector<PMS::Axis>& axes = PMS::axes();
	    for(unsigned int i = 0; i < axes.size(); i++) 
		    loadedAxes_[axes[i]] = false;
        loadedAxesData_.clear();
	}
	filename_ = filename;
	selection_ = selection;
	averaging_ = averaging;
	transformations_ = transformations;
	calibration_ = calibration;

	//logLoad(selection_.summary());
	logLoad(transformations_.summary());
	logLoad(averaging_.summary());
	logLoad(calibration_.summary());

	// Trap (currently) unsupported modes
	for ( int i = 0; i < dataCount; i++ ){

		// Forbid antenna-based/baseline-based combination plots, for now
		Vector<Bool> nAM=netAxesMask(currentX_[i],currentY_[i]);
		if (nAM(2)&&nAM(3)){
			throw(AipsError("Plots of antenna-based vs. baseline-based axes not supported ("+
					PMS::axis(currentX_[i])+" and "+PMS::axis(currentY_[i])+")"));
		}

        // Check averaging validity
		if (averaging_.time() && averaging_.timeValue() < 0.0) {
			logError("load", "Cannot average negative time value");
            throw(AipsError("Invalid avgtime"));
        }
        if (averaging_.channel() && averaging_.channelValue() < 0.0) {
			logError("load", "Cannot average negative number of channels");
            throw(AipsError("Invalid avgchannel"));
        }
		if ( averaging_.baseline() || averaging_.antenna() || averaging_.spw() || averaging_.scalarAve())
		{
			int axesCount = axes.size();
			for ( int j = 0; j < axesCount; j++ ){
		        // Can't plot averaged weights yet with plotms-averaging code
                if (PMS::axisIsWeight(axes[j])) {
					throw(AipsError("Selected averaging does not yet support Weight and Sigma axes."));
				}
                // Check axis/averaging compatibility
				if ( !axisIsValid(axes[j], averaging_) ) {
					throw(AipsError(PMS::axis(axes[j]) + " axis is not valid for selected averaging."));
				}
			}
		}

        // Check ephemeris validity
		bool ephemerisX = isEphemerisAxis( currentX_[i]);
		bool ephemerisY = isEphemerisAxis( currentY_[i]);
		if ( ephemerisX || ephemerisY ){
			bool ephemerisAvailable = isEphemeris();
			if ( !ephemerisAvailable ){
				String axisName;
				if ( ephemerisX ){
					axisName.append( PMS::axis( currentX_[i]));
				}
				if ( ephemerisY ){
					if ( ephemerisX ){
						axisName.append( " and ");
					}
					axisName.append( PMS::axis( currentY_[i]));
				}
				String errorMessage( "Not loading axis "+axisName+
						" because ephemeris data is not available for this ms.");
				logWarn( "load", errorMessage);
				if ( thread != NULL ){
					thread->setError( errorMessage );
				}
				throw AipsError(errorMessage );
			}
		}

	}

	stringstream ss;
	ss << "Caching for the new plot: ";
	for ( int i = 0; i < dataCount; i++ ){
		ss << PMS::axis(currentY_[i]) << "(" << currentY_[i] << ")";  
        if (PMS::axisIsData(currentY_[i]))
            ss << ":" << PMS::dataColumn(currentYData_[i]);
        ss << " vs. " << PMS::axis(currentX_[i]) << "(" << currentX_[i] << ")";
        if (PMS::axisIsData(currentX_[i]))
            ss << ":" << PMS::dataColumn(currentXData_[i]);
        ss << "...\n";
	}
	logLoad(ss.str());

	// Calculate which axes need to be loaded; those that have already been
	// loaded do NOT need to be reloaded (assuming that the rest of PlotMS has
	// done its job and cleared the cache if the underlying MS/selection has
	// changed).
	vector<PMS::Axis> loadAxes;
	vector<PMS::DataColumn> loadData;

	// A map that keeps track of all pending loaded axes
	//  this is a list of all axes that will be loaded, if everything
	//   works---it is used to pre-estimate memory requirements.
	pendingLoadAxes_.clear();

	// Check meta-data.
	for(Int i = 0; i < nmetadata(); ++i) {
		pendingLoadAxes_[metadata(i)]=true; // all meta data will be loaded
		if(!loadedAxes_[metadata(i)]) {
			loadAxes.push_back(metadata(i));
			loadData.push_back(PMS::DEFAULT_DATACOLUMN);
		}
	}

	// Ensure all _already-loaded_ axes are in the pending list
	for (Int i= 0;i<PMS::NONE;++i)
		if (loadedAxes_[PMS::Axis(i)]) pendingLoadAxes_[PMS::Axis(i)]=true;

	// Check given axes.  Should only be added to load list if: 1) not
	// already in load list, 2) not loaded, or 3) loaded but with different
	// data column (if applicable).
	bool found; PMS::Axis axis; PMS::DataColumn dc;

	for(unsigned int i = 0; i < axes.size(); i++) {
		found = false;
		axis = axes[i];

		// add to pending list
		pendingLoadAxes_[axis]=true;

		// if data vector is not the same length as axes vector, assume
		// default data column
		dc = PMS::DEFAULT_DATACOLUMN;
		if(i < data.size()) dc = data[i];

		// 1)  already in the load list? (loadAxes)
		for(unsigned int j = 0; !found && j < loadAxes.size(); j++)
			if(loadAxes[j]==axis && loadData[j]==dc) found = true;
		if(found) continue;

		//If ephemeris data is not available we should not load axes 
        //associated with ephemeris data.
		bool ephemerisAvailable = isEphemeris();
		if ( !ephemerisAvailable ){
			if ( axis == PMS::RADIAL_VELOCITY || axis == PMS::RHO ){
				continue;
			}
		}

		// 2)  not already loaded? (loadedAxes)
		if(!loadedAxes_[axis]) {
			loadAxes.push_back(axis);
			loadData.push_back(dc);
		}

		// 3)  data axis is loaded; check if data column loaded
		else if(PMS::axisIsData(axis)) {
            // see if datacol is loaded for axis
            std::set<PMS::DataColumn> datacols = loadedAxesData_[axis];
            if (datacols.find(dc) == datacols.end()) {
			    loadAxes.push_back(axis);
			    loadData.push_back(dc);
            }
        }
	}

	if (false) {
		{
			cout << "Already loaded axes: " << flush;
			Int nload(0);
			for (Int i= 0;i<PMS::NONE;++i)
				if (loadedAxes_[PMS::Axis(i)]) {
					++nload;
					cout << PMS::axis(PMS::Axis(i)) << " " << flush;
				}
			cout << " (" << nload << ")" <<  endl;
		}
		cout << "To be loaded axes (" << loadAxes.size() << "): " << flush;
		for (uInt i=0;i<loadAxes.size();++i)
			cout << PMS::axis(loadAxes[i]) << " " << flush;
		cout << endl;
	}

	// Now Load data if the user doesn't cancel.
	if(loadAxes.size() > 0) {

		// Call method that actually does the loading (MS- or Cal-specific)
		loadIt(loadAxes,loadData,thread);

		// Update loaded axes if not canceled.
        if (wasCanceled()) { 
            logLoad("Cache loading cancelled.");
            return;  // no need to continue
        } else {
            for(unsigned int i = 0; i < loadAxes.size(); i++) {
                axis = loadAxes[i];
                loadedAxes_[axis] = true;
                if(PMS::axisIsData(axis)) 
                    loadedAxesData_[axis].insert(loadData[i]);
            }
        }

		if (false) {
			{
				cout << "Finally loaded axes: " << flush;
				Int nload(0);
				for (Int i= 0;i<PMS::NONE;++i)
					if (loadedAxes_[PMS::Axis(i)]) {
						++nload;
						cout << PMS::axis(PMS::Axis(i)) << " " << flush;
					}
				cout << " (" << nload << ")" <<  endl;
			}
		}

	} // something to load

    if (wasCanceled()) { 
        logLoad("Cache loading cancelled.");
        return;  // no need to continue
    }

    // Setup/revis masks that we use to realize axes relationships
    netAxesMask_.resize( dataCount );
    for ( int i = 0; i < dataCount; i++ ){
        Vector<Bool> xmask(4,false);
        Vector<Bool> ymask(4,false);
        setAxesMask(currentX_[i],xmask);
        setAxesMask(currentY_[i],ymask);
        netAxesMask_[i]=(xmask || ymask);
    }
    /*
  cout << boolalpha;
  cout << "xmask = " << xmask << endl;
  cout << "ymask = " << ymask << endl;
  cout << "netAxesMask_ = " << netAxesMask_ << endl;
    */

    // Generate the plot mask from scratch
    deletePlotMask();
    plmask_.resize( dataCount );
    for ( int i = 0; i < dataCount; i++ ){
        setPlotMask( i );
    }

    // At this stage, data is loaded and ready for indexing then plotting....
    dataLoaded_ = true;

    // Calculate refTime (for plot labels)
    refTime_p=min(time_);
    refTime_p=86400.0*floor(refTime_p/86400.0);
    logLoad("refTime = "+MVTime(refTime_p/C::day).string(MVTime::YMD,7));
    QString timeMesg("refTime = ");
    timeMesg.append(MVTime(refTime_p/C::day).string(MVTime::YMD,7).c_str());
    logLoad("Finished loading.");
}

bool PlotMSCacheBase::axisIsValid(PMS::Axis axis, const PlotMSAveraging& averaging) {
	// Check if axis is valid for the type of averaging requested
	bool bslnValid(true), spwValid(true);
	if (averaging.baseline()) {
		switch(axis) {
		case PMS::UVDIST_L:
		case PMS::UWAVE:
		case PMS::VWAVE:
		case PMS::WWAVE:
		case PMS::ANTENNA:
		case PMS::AZIMUTH:
		case PMS::ELEVATION:
		case PMS::PARANG: {
			bslnValid = false;
			break;
		}
		default:
			break;
		}
	}
	if (averaging.spw()) {
		switch(axis) {
		case PMS::VELOCITY:
		case PMS::UVDIST_L:
		case PMS::UWAVE:
		case PMS::VWAVE:
		case PMS::WWAVE: {
			spwValid = false;
			break;
		}
		default:
			break;
		}
	}
	return bslnValid && spwValid;
}

void PlotMSCacheBase::clear() {
	logLoad("Clearing the existing plotms cache.");
	deleteIndexer();
	deletePlotMask();
	deleteCache();
	refTime_p=0.0;
	dataLoaded_=false;
}

#define PMSC_DELETE(VAR)                                                \
		{ \
	for(unsigned int j = 0; j < VAR.size(); j++) {	\
		if(VAR[j]) {						  \
			delete VAR[j];						  \
		}								  \
	} \
	VAR.resize(0,true);				\
		}


//    cout << VAR.size() << " " << j << " " << PMS::axis(axes[i]) << " " << VAR[j]->nrefs() << " " << VAR[j] << endl; \


void PlotMSCacheBase::release(const vector<PMS::Axis>& axes) {
		for(unsigned int i = 0; i < axes.size(); i++) {
			switch(axes[i]) {
			case PMS::SCAN: scan_.resize(0);
                break;
			case PMS::FIELD: field_.resize(0);
                break;
			case PMS::TIME: time_.resize(0);
                break;
			case PMS::TIME_INTERVAL: timeIntr_.resize(0);
                break;
			case PMS::SPW: spw_.resize(0);
                break;
			case PMS::CHANNEL: { 
                PMSC_DELETE(chan_)
                PMSC_DELETE(chansPerBin_)
                }
                break;
			case PMS::FREQUENCY: PMSC_DELETE(freq_)
                break;
			case PMS::VELOCITY: PMSC_DELETE(vel_)
                break;
			case PMS::CORR: PMSC_DELETE(corr_)
                break;
			case PMS::ANTENNA1: PMSC_DELETE(antenna1_)
                break;
			case PMS::ANTENNA2: PMSC_DELETE(antenna2_)
                break;
			case PMS::BASELINE: PMSC_DELETE(baseline_)
                break;
			case PMS::ROW: PMSC_DELETE(row_)
                break;
			case PMS::OBSERVATION: PMSC_DELETE(obsid_)
                break;
			case PMS::INTENT: PMSC_DELETE(intent_)
                break;
			case PMS::FEED1: PMSC_DELETE(feed1_)
                break;
			case PMS::FEED2: PMSC_DELETE(feed2_)
                break;
			case PMS::AMP:
			case PMS::GAMP: {
                PMSC_DELETE(amp_)
                PMSC_DELETE(ampCorr_)
                PMSC_DELETE(ampModel_)
                PMSC_DELETE(ampCorrModel_)
                PMSC_DELETE(ampDataModel_)
                PMSC_DELETE(ampDataDivModel_)
                PMSC_DELETE(ampCorrDivModel_)
                PMSC_DELETE(ampFloat_)
                }
                break;
			case PMS::PHASE:
			case PMS::GPHASE: {
                PMSC_DELETE(pha_)
                PMSC_DELETE(phaCorr_)
                PMSC_DELETE(phaModel_)
                PMSC_DELETE(phaCorrModel_)
                PMSC_DELETE(phaDataModel_)
                PMSC_DELETE(phaDataDivModel_)
                PMSC_DELETE(phaCorrDivModel_)
                }
                break;
			case PMS::REAL:
			case PMS::GREAL: {
                PMSC_DELETE(real_)
                PMSC_DELETE(realCorr_)
                PMSC_DELETE(realModel_)
                PMSC_DELETE(realCorrModel_)
                PMSC_DELETE(realDataModel_)
                PMSC_DELETE(realDataDivModel_)
                PMSC_DELETE(realCorrDivModel_)
                }
                break;
			case PMS::IMAG:
			case PMS::GIMAG: {
                PMSC_DELETE(imag_)
                PMSC_DELETE(imagCorr_)
                PMSC_DELETE(imagModel_)
                PMSC_DELETE(imagCorrModel_)
                PMSC_DELETE(imagDataModel_)
                PMSC_DELETE(imagDataDivModel_)
                PMSC_DELETE(imagCorrDivModel_)
                }
                break;
			case PMS::WTxAMP: {
                PMSC_DELETE(wtxamp_)
                PMSC_DELETE(wtxampCorr_)
                PMSC_DELETE(wtxampModel_)
                PMSC_DELETE(wtxampCorrModel_)
                PMSC_DELETE(wtxampDataModel_)
                PMSC_DELETE(wtxampDataDivModel_)
                PMSC_DELETE(wtxampCorrDivModel_)
                PMSC_DELETE(wtxampFloat_)
                }
                break;
			case PMS::WT: PMSC_DELETE(wt_)
                break;
			case PMS::WTSP: PMSC_DELETE(wtsp_)
                break;
			case PMS::SIGMA: PMSC_DELETE(sigma_)
                break;
			case PMS::SIGMASP: PMSC_DELETE(sigmasp_)
                break;
			case PMS::FLAG: PMSC_DELETE(flag_)
                break;
			case PMS::FLAG_ROW: PMSC_DELETE(flagrow_)
                break;
			case PMS::UVDIST: PMSC_DELETE(uvdist_)
                break;
			case PMS::UVDIST_L: PMSC_DELETE(uvdistL_)
                break;
			case PMS::U: PMSC_DELETE(u_)
                break;
			case PMS::V: PMSC_DELETE(v_)
                break;
			case PMS::W: PMSC_DELETE(w_)
                break;
			case PMS::UWAVE: PMSC_DELETE(uwave_)
                break;
			case PMS::VWAVE: PMSC_DELETE(vwave_)
                break;
			case PMS::WWAVE: PMSC_DELETE(wwave_)
                break;
			case PMS::AZ0: az0_.resize(0);
                break;
			case PMS::EL0: el0_.resize(0);
                break;
			case PMS::HA0: ha0_.resize(0);
                break;
			case PMS::PA0: pa0_.resize(0);
                break;
			case PMS::ANTENNA: PMSC_DELETE(antenna_)
                break;
			case PMS::AZIMUTH: PMSC_DELETE(az_)
                break;
			case PMS::ELEVATION: PMSC_DELETE(el_)
                break;
			case PMS::PARANG: PMSC_DELETE(parang_)
                break;
			case PMS::DELAY:
			case PMS::SWP:
			case PMS::TSYS:
			case PMS::OPAC:
			case PMS::TEC: PMSC_DELETE(par_)
                break;
			case PMS::SNR: PMSC_DELETE(snr_)
                break;
			case PMS::RADIAL_VELOCITY: radialVelocity_.resize(0);
                break;
			case PMS::RHO: rho_.resize(0);
                break;
			case PMS::NONE:
                break;
			}

			loadedAxes_[axes[i]] = false;

			if(dataLoaded_ && axisIsMetaData(axes[i])) dataLoaded_ = false;

			if(dataLoaded_ ){
				int plotDataCount = getDataCount();
				for ( int j = 0; j < plotDataCount; j++ ){
					if ( currentX_[j] == axes[i] || currentY_[j] == axes[i] ) {
						dataLoaded_ = false;
					}
				}
			}
		}
	//    uInt postmem=HostInfo::memoryFree();
	//    cout << "memoryFree = " << premem << " " << postmem << " " << premem-postmem << endl;
	if(!dataLoaded_) nChunk_ = 0;
}

bool PlotMSCacheBase::isEphemerisAxis( PMS::Axis axis ) const {
	bool ephemerisAxis = false;
	if ( axis == PMS::RADIAL_VELOCITY || axis == PMS::RHO ){
		ephemerisAxis = true;
	}
	return ephemerisAxis;
}

void PlotMSCacheBase::resizeIndexer( int size ){
	deleteIndexer();
	indexer_.resize( size );
	//plmask_.resize( size );
}

void PlotMSCacheBase::clearRanges(){
	xminG_=yminG_=xflminG_=yflminG_=DBL_MAX;
	xmaxG_=ymaxG_=xflmaxG_=yflmaxG_=-DBL_MAX;
}

String PlotMSCacheBase::getTimeBounds( int iterValue ){
	String formattedTime;
	if ( averaging_.time() ){
		int nIter = 1;
		if ( indexer_.size() > 0 ){
			nIter = indexer_[0].size();
		}
		int divisor = nChunk_/nIter;
		int lowBound = iterValue;
		int highBound = iterValue;
		double iterNorm = iterValue / divisor;
		for ( int i = 0; i < nChunk_; i++ ){
			if ( i / divisor == iterNorm ){
				if ( lowBound > i ){
					lowBound = i;
				}
				if ( highBound < i ){
					highBound = i;
				}
			}
		}

		double lowValue = time_(lowBound);
		double highValue = time_(highBound );
		if ( highBound < nChunk_ - 1){
			highValue = time_(highBound+1);
		}

		if ( lowBound == highBound ){
			formattedTime = Plotter::formattedDateString(Plotter::DEFAULT_RELATIVE_DATE_FORMAT,lowValue, DATE_MJ_SEC );
		}
		else {
			String lowTime = Plotter::formattedDateString(Plotter::DEFAULT_RELATIVE_DATE_FORMAT,lowValue, DATE_MJ_SEC );
			String highTime = Plotter::formattedDateString(Plotter::DEFAULT_RELATIVE_DATE_FORMAT,highValue, DATE_MJ_SEC );
			formattedTime = lowTime + " - " + highTime;
		}
	}
	else {
		double tValue = time_(iterValue);
		formattedTime =Plotter::formattedDateString(Plotter::DEFAULT_RELATIVE_DATE_FORMAT,tValue, DATE_MJ_SEC );
	}
	return formattedTime;
}

pair<Double,Double> PlotMSCacheBase::getTimeBounds() const {
	pair<Double,Double> timeBounds;
	timeBounds.first = timeBounds.second = 0.0;
	int dataCount = getDataCount();
	for ( int i = 0; i < dataCount; i++ ){
		if (PMS::axis(currentY_[i]) == "Time") {
			timeBounds.first = yminG_;
			timeBounds.second = ymaxG_;
			break;
		}
		else if (PMS::axis(currentX_[i]) == "Time") {
			timeBounds.first = xminG_;
			timeBounds.second = xmaxG_;
			break;
		}
	}
	return timeBounds;
}

pair<Double,Double> PlotMSCacheBase::getYAxisBounds() const {
    pair<Double,Double> axisBounds;
	axisBounds.first = yminG_;
	axisBounds.second = ymaxG_;
	return axisBounds;
}

pair<Double,Double> PlotMSCacheBase::getXAxisBounds() const {
    pair<Double,Double> axisBounds;
	axisBounds.first = xminG_;
	axisBounds.second = xmaxG_;
	return axisBounds;
}

bool PlotMSCacheBase::isIndexerInitialized( PMS::Axis iteraxis, Bool globalXRange,
		Bool globalYRange, int dataIndex ) const {
	bool initialized = true;
	if ( this->iterAxis != iteraxis ){
		initialized = false;
	}
	else {
		if ( static_cast<int>(indexer_.size())<= dataIndex ){
			initialized = false;
		}
		else {
			if ( indexer_[dataIndex].empty()){
				initialized = false;
			}
			else {
				if ( indexer_[dataIndex][0] == NULL ||
						indexer_[dataIndex][0]->isGlobalXRange() != globalXRange ||
					indexer_[dataIndex][0]->isGlobalYRange() != globalYRange ){
					initialized = false;
				}
			}
		}
	}
	return initialized;
}

void PlotMSCacheBase::setUpIndexer(PMS::Axis iteraxis, Bool globalXRange,
		Bool globalYRange, int dataIndex ) {

	logLoad("Setting up iteration indexing (if necessary), and calculating plot ranges.");

	//cout << "############ PlotMSCacheBase::setUpIndexer: " << PMS::axis(iteraxis)
	//     << " cacheReady() = " << boolalpha << cacheReady() << endl;
	Int nIter=0;
	Vector<Int> iterValues;

	// If the cache hasn't been filled, do nothing
	if (!cacheReady()) return;
	switch (iteraxis) {
	case PMS::SCAN: {
		iterValues=scan_(goodChunk_).getCompressedArray();
		nIter=genSort(iterValues,Sort::Ascending,(Sort::QuickSort | Sort::NoDuplicates));
		break;
	}
	case PMS::SPW: {
		iterValues=spw_(goodChunk_).getCompressedArray();
		nIter=genSort(iterValues,Sort::Ascending,(Sort::QuickSort | Sort::NoDuplicates));
		break;
	}
	case PMS::FIELD: {
		iterValues=field_(goodChunk_).getCompressedArray();
		nIter=genSort(iterValues,Sort::Ascending,(Sort::QuickSort | Sort::NoDuplicates));
		break;
	}
	case PMS::BASELINE: {

		// Revise axes mask, etc., to ensure baseline-dependence
		if (!netAxesMask_[dataIndex](2)) {
			netAxesMask_[dataIndex](2)=true;
			setPlotMask( dataIndex );
		}

		// Maximum possible baselines (includes ACs)
		Int nBslnMax((nAnt_+1)*(nAnt_+2)/2);

		// Find the limited list of _occurring_ baseline indices
		Vector<Int> bslnList(nBslnMax);
		Vector<Bool> bslnMask(nBslnMax,false);
		indgen(bslnList);
		/*
    cout << "*baseline_[0] = " << *baseline_[0] << endl;
    cout << "*antenna1_[0] = " << *antenna1_[0] << endl;
    cout << "*antenna2_[0] = " << *antenna2_[0] << endl;
    cout << "bslnList = " << bslnList << endl;
		 */

		for (Int ich=0;ich<nChunk_;++ich)
			if (goodChunk_(ich))
				for (Int ibl=0;ibl<chunkShapes()(2,ich);++ibl)
					bslnMask(*(baseline_[ich]->data()+ibl))=true;
		//    cout << "bslnMask = " << boolalpha << bslnMask << endl;

		// Remember only the occuring baseline indices
		iterValues=bslnList(bslnMask).getCompressedArray();
		nIter=iterValues.nelements();
		//    cout << "nIter = " << nIter << " iterValues = " << iterValues << endl;
		break;
	}
	case PMS::ANTENNA: {

		// Escape if (full) baseline averaging is on, since we won't find any iterations
		if (averaging_.baseline())
			throw(AipsError("Iteration over antenna not supported with full baseline averaging."));

		// Revise axes mask, etc., to ensure baseline-dependence
		if (!netAxesMask_[dataIndex](2)) {
			netAxesMask_[dataIndex](2)=true;
			setPlotMask( dataIndex );
		}
		// Find the limited list of _occuring_ antenna indices
		Vector<Int> antList(nAnt_);
		Vector<Bool> antMask(nAnt_,false);
		indgen(antList);
		bool selectionEmpty = selection_.isEmpty();
		Vector<Int> selAnts1;
		if ( !selectionEmpty ){
			selAnts1 = selection_.getSelectedAntennas1();
		}
		Vector<Int> selAnts2;
		if ( !selectionEmpty ){
			selAnts2 = selection_.getSelectedAntennas2();
		}
		for (Int ich=0;ich<nChunk_;++ich){
			if (goodChunk_(ich)){
				for (Int ibl=0;ibl<chunkShapes()(2,ich);++ibl) {
					Int a1 =*(antenna1_[ich]->data()+ibl);
					_updateAntennaMask( a1, antMask, selAnts1 );

					Int a2 =*(antenna2_[ich]->data()+ibl);
					_updateAntennaMask( a2, antMask, selAnts2 );
				}
			}
		}

		// Remember only the occuring antenna indices
		iterValues=antList(antMask).getCompressedArray();
		nIter=iterValues.nelements();
		break;
	}
	case PMS::TIME: {


		if (averaging_.time()){
			double timeInterval= averaging_.timeValue();
			if ( timeInterval <= 0 ){
				timeInterval = 1;
			}
			double baseTime = getTime( 0, 0 );
			double endTime = getTime( nChunk_-1, 0 );
			double totalTime = endTime - baseTime;
			double quotient = qRound(totalTime / timeInterval);
			nIter = static_cast<int>( quotient )+1;

		    //It does not make sense to have more iterations than number of chunks.
			//This could happen if averaging is set less than the time interval between chunks.
			if ( nIter > nChunk_){
				nIter = nChunk_;
			}
			const int UNITIALIZED = -1;
			Vector<Int> timeList(nIter, UNITIALIZED);

			int divisor = nChunk_ / nIter;
			for ( int j = 0; j < nIter; j++ ){
				int timeIndex = j * divisor;
				timeList[j] = timeIndex;
			}
			iterValues = timeList;
		}

		//We are not averaging time.  Store and count the unique time values.
		else {
			::QList<double> uniqueTimes;
			::QVector<int> timeList;
			for ( int i = 0; i < nChunk(); i++ ){
				double timeValue = getTime(i, 0);
				if ( !uniqueTimes.contains( timeValue )){
					uniqueTimes.append( timeValue );
					timeList.append( i );
				}
			}
			nIter = uniqueTimes.size();
			iterValues.resize( nIter );
			for ( int i = 0; i < nIter; i++ ){
				iterValues[i] = timeList[i];
			}
		}
		break;
	}
    case PMS::CORR: {
        // Revise axes mask, etc., to ensure correlation-dependence
        if (!netAxesMask_[dataIndex](0)) {
            netAxesMask_[dataIndex](0)=true;
            setPlotMask( dataIndex );
        }

        Int nCorrMax = Stokes::NumberOfTypes;
        Vector<Int> corrList(nCorrMax);
        Vector<Bool> corrMask(nCorrMax,false);
        indgen(corrList);
        for (Int ich=0;ich<nChunk_;++ich){
            if (goodChunk_(ich)){
                for (Int icorr=0; icorr<chunkShapes()(0,ich); ++icorr) {
                    corrMask(*(corr_[ich]->data()+icorr))=true;
                }
            }
        }
        iterValues=corrList(corrMask).getCompressedArray();
        nIter=iterValues.nelements();
        break;
    }
	case PMS::NONE: {
		nIter=1;
		iterValues.resize(1);
		iterValues(0)=0;
		break;
	}
	default:
		throw(AipsError("Unsupported iteration axis:"+PMS::axis(iteraxis)));
		break;
	}

	if (iteraxis!=PMS::NONE) {
		stringstream ss;
		ss << "Found " << nIter << " " << PMS::axis(iteraxis) << " iterations.";
		logLoad(ss.str());
	}

	// cout << "********nIter = " << nIter << " iterValues = " << iterValues(IPosition(1,0),IPosition(1,nIter-1)) << endl;


	indexer_[dataIndex].resize(nIter);
	indexer_[dataIndex].set( NULL );

	for (Int iter=0;iter<nIter;++iter){
		indexer_[dataIndex][iter] = new PlotMSIndexer(this, 
            currentX_[dataIndex], currentXData_[dataIndex], 
            currentY_[dataIndex], currentYData_[dataIndex],
            iteraxis, iterValues(iter), dataIndex);
	}
	// Extract global ranges from the indexers
	// Initialize limits

	Double ixmin,iymin,ixmax,iymax;
	for (Int iter=0;iter<nIter;++iter) {
		indexer_[dataIndex][iter]->unmaskedMinsMaxesRaw(ixmin,ixmax,iymin,iymax);
		xminG_=min(xminG_,ixmin);
		xmaxG_=max(xmaxG_,ixmax);
		yminG_=min(yminG_,iymin);
		ymaxG_=max(ymaxG_,iymax);

		indexer_[dataIndex][iter]->maskedMinsMaxesRaw(ixmin,ixmax,iymin,iymax);
		xflminG_=min(xflminG_,ixmin);
		xflmaxG_=max(xflmaxG_,ixmax);
		yflminG_=min(yflminG_,iymin);
		yflmaxG_=max(yflmaxG_,iymax);

		// set usage of globals
		indexer_[dataIndex][iter]->setGlobalMinMax(globalXRange,globalYRange);
	}

	//Store the iteration axis.
	this->iterAxis = iteraxis;

	{
		stringstream ss;
		ss << "Global ranges:" << endl
		   << PMS::axis(currentX_[dataIndex]);
        if (PMS::axisIsData(currentX_[dataIndex])) 
            ss << ":" << PMS::dataColumn(currentXData_[dataIndex]);
        ss << ": " << xminG_ << " to " << xmaxG_ << " (unflagged); ";
        if (xflminG_ == DBL_MAX)
            ss << "(no flagged data)" << endl;
        else 
		   ss << "; " << xflminG_ << " to " << xflmaxG_ << " (flagged)." << endl;
		ss << PMS::axis(currentY_[dataIndex]);
        if (PMS::axisIsData(currentY_[dataIndex])) 
            ss << ":" << PMS::dataColumn(currentYData_[dataIndex]);
        ss << ": " << yminG_ << " to " << ymaxG_ << " (unflagged); ";
        if (yflminG_ == DBL_MAX)
            ss << "(no flagged data)";
        else
		    ss << yflminG_ << " to " << yflmaxG_ << "(flagged).";
		logLoad(ss.str());
    
        if (indexer_[dataIndex][0]->plotConjugates()) {
            stringstream ss;
            ss << "For a UV plot, plotms will plot the conjugates of the points in the MS." << endl;
            ss << "However, the Locate and Flag functions will not work for these conjugate points!" << endl;
            ss << "The global ranges above do not include the conjugates.";
            logWarn("load_cache", ss.str());
        } 
	}
}

void PlotMSCacheBase::_updateAntennaMask( Int a, Vector<Bool>& antMask,
		const Vector<Int> selectedAntennas ){
	if (a>-1){
		bool selected = false;
		int selectedAntennaCount = selectedAntennas.size();
		if ( selectedAntennaCount == 0){
			selected = true;
		}
		else {
			for ( int i = 0; i < selectedAntennaCount; i++ ){
				if ( selectedAntennas[i] == a ){
					selected = true;
					break;
				}
			}
		}
		if ( selected ){
			antMask(a)=true;
		}
	}
}


//*********************************
// protected method implementations



// set the number of chunks we can store
void PlotMSCacheBase::setCache(Int newnChunk, 
    const vector<PMS::Axis>& loadAxes, 
    const vector<PMS::DataColumn>& loadData) {

    nChunk_ = newnChunk;

    // Resize axes we will load
    for (uInt i=0; i<loadAxes.size(); ++i) {
	    // Resize, copying existing contents
        switch(loadAxes[i]) {
            case PMS::SCAN: {
	            scan_.resize(nChunk_,true);
                }
                break;
            case PMS::FIELD: {
                field_.resize(nChunk_,true);
                }
                break;
            case PMS::TIME: {
                time_.resize(nChunk_,true);
                }
                break;
            case PMS::TIME_INTERVAL: {
                timeIntr_.resize(nChunk_,true);
                }
                break;
	        case PMS::SPW: {
                spw_.resize(nChunk_,true);
                }
                break;
            case PMS::CHANNEL: {
		        addVectors(chan_);
		        addArrays(chansPerBin_);
                }
                break;
            case PMS::FREQUENCY:
		        addVectors(freq_);
                break;
            case PMS::VELOCITY:
		        addVectors(vel_);
                break;
            case PMS::CORR:
		        addVectors(corr_);
                break;
	        case PMS::ANTENNA1:
		        addVectors(antenna1_);
                break;
            case PMS::ANTENNA2:
		        addVectors(antenna2_);
                break;
            case PMS::BASELINE:
		        addVectors(baseline_);
                break;
            case PMS::ROW:
                addVectors(row_);
                break;
            case PMS::OBSERVATION:
		        addVectors(obsid_);
                break;
            case PMS::INTENT:
		        addVectors(intent_);
                break;
            case PMS::FEED1:
		        addVectors(feed1_);
                break;
            case PMS::FEED2:
		        addVectors(feed2_);
                break;
	        case PMS::AMP: 
            case PMS::GAMP: {
                switch(loadData[i]) {
                    case PMS::DATA:
		                addArrays(amp_);
                        break;
                    case PMS::CORRECTED:
		                addArrays(ampCorr_);
                        break;
                    case PMS::MODEL:
		                addArrays(ampModel_);
                        break;
                    case PMS::CORRMODEL:
		                addArrays(ampCorrModel_);
                        break;
                    case PMS::DATAMODEL:
		                addArrays(ampDataModel_);
                        break;
                    case PMS::DATA_DIVIDE_MODEL:
		                addArrays(ampDataDivModel_);
                        break;
                    case PMS::CORRECTED_DIVIDE_MODEL:
		                addArrays(ampCorrDivModel_);
                        break;
                    case PMS::FLOAT_DATA:
		                addArrays(ampFloat_);
                        break;
                    }
                }
                break;
            case PMS::PHASE:
            case PMS::GPHASE: {
                switch(loadData[i]) {
                    case PMS::DATA:
		                addArrays(pha_);
                        break;
                    case PMS::CORRECTED:
		                addArrays(phaCorr_);
                        break;
                    case PMS::MODEL:
		                addArrays(phaModel_);
                        break;
                    case PMS::CORRMODEL:
		                addArrays(phaCorrModel_);
                        break;
                    case PMS::DATAMODEL:
		                addArrays(phaDataModel_);
                        break;
                    case PMS::DATA_DIVIDE_MODEL:
		                addArrays(phaDataDivModel_);
                        break;
                    case PMS::CORRECTED_DIVIDE_MODEL:
		                addArrays(phaCorrDivModel_);
                        break;
                    case PMS::FLOAT_DATA:
                        break;
                    }
                }
                break;
            case PMS::REAL:
            case PMS::GREAL: {
                switch(loadData[i]) {
                    case PMS::DATA:
		                addArrays(real_);
                        break;
                    case PMS::CORRECTED:
		                addArrays(realCorr_);
                        break;
                    case PMS::MODEL:
		                addArrays(realModel_);
                        break;
                    case PMS::CORRMODEL:
		                addArrays(realCorrModel_);
                        break;
                    case PMS::DATAMODEL:
		                addArrays(realDataModel_);
                        break;
                    case PMS::DATA_DIVIDE_MODEL:
		                addArrays(realDataDivModel_);
                        break;
                    case PMS::CORRECTED_DIVIDE_MODEL:
		                addArrays(realCorrDivModel_);
                        break;
                    case PMS::FLOAT_DATA:
		                addArrays(real_);
                        break;
                    }
                }
                break;
            case PMS::IMAG:
            case PMS::GIMAG: {
                switch(loadData[i]) {
                    case PMS::DATA:
		                addArrays(imag_);
                        break;
                    case PMS::CORRECTED:
		                addArrays(imagCorr_);
                        break;
                    case PMS::MODEL: 
		                addArrays(imagModel_);
                        break;
                    case PMS::CORRMODEL:
		                addArrays(imagCorrModel_);
                        break;
                    case PMS::DATAMODEL:
		                addArrays(imagDataModel_);
                        break;
                    case PMS::DATA_DIVIDE_MODEL: 
		                addArrays(imagDataDivModel_);
                        break;
                    case PMS::CORRECTED_DIVIDE_MODEL:
		                addArrays(imagCorrDivModel_);
                        break;
                    case PMS::FLOAT_DATA:
                        break;
                    }
                }
                break;
            case PMS::WTxAMP: {
                switch(loadData[i]) {
                    case PMS::DATA:
		                addArrays(wtxamp_);
                        break;
                    case PMS::CORRECTED:
		                addArrays(wtxampCorr_);
                        break;
                    case PMS::MODEL:
		                addArrays(wtxampModel_);
                        break;
                    case PMS::CORRMODEL:
		                addArrays(wtxampCorrModel_);
                        break;
                    case PMS::DATAMODEL:
		                addArrays(wtxampDataModel_);
                        break;
                    case PMS::DATA_DIVIDE_MODEL:
		                addArrays(wtxampDataDivModel_);
                        break;
                    case PMS::CORRECTED_DIVIDE_MODEL:
		                addArrays(wtxampCorrDivModel_);
                        break;
                    case PMS::FLOAT_DATA:
		                addArrays(wtxampFloat_);
                        break;
                    }
                }
                break;
            case PMS::WT:
		        addArrays(wt_);
                break;
            case PMS::WTSP:
		        addArrays(wtsp_);
                break;
            case PMS::SIGMA:
		        addArrays(sigma_);
                break;
            case PMS::SIGMASP:
		        addArrays(sigmasp_);
                break;
            case PMS::FLAG:
            case PMS::FLAG_ROW: {
		        addArrays(flag_);
		        addVectors(flagrow_);
                break;
            }
	        case PMS::UVDIST:
		        addVectors(uvdist_);
                break;
	        case PMS::UVDIST_L:
		        addMatrices(uvdistL_);
                break;
            case PMS::U:
		        addVectors(u_);
                break;
            case PMS::V:
		        addVectors(v_);
                break;
            case PMS::W:
		        addVectors(w_);
                break;
            case PMS::UWAVE:
		        addMatrices(uwave_);
                break;
            case PMS::VWAVE:
		        addMatrices(vwave_);
                break;
            case PMS::WWAVE:
		        addMatrices(wwave_);
                break;
	        case PMS::AZ0:
            case PMS::EL0: {
                az0_.resize(nChunk_,true);
                el0_.resize(nChunk_,true);
                break;
            }
            case PMS::HA0:
                ha0_.resize(nChunk_,true);
                break;
            case PMS::PA0:
                pa0_.resize(nChunk_,true);
                break;
	        case PMS::ANTENNA:
		        addVectors(antenna_);
                break;
            case PMS::AZIMUTH:
            case PMS::ELEVATION: {
		        addVectors(az_);
		        addVectors(el_);
                break;
            }
	        case PMS::PARANG:
		        addVectors(parang_);
                break;
	        case PMS::DELAY:
		        addArrays(par_);
                break;
            case PMS::SWP:
		        addArrays(par_);
                break;
            case PMS::TSYS:
		        addArrays(par_);
                break;
            case PMS::OPAC:
		        addArrays(par_);
                break;
            case PMS::SNR:
		        addArrays(snr_);
                break;
            case PMS::TEC:
		        addArrays(par_);
                break;
	        case PMS::RADIAL_VELOCITY: {
                radialVelocity_.resize(nChunk_,true);
                }
                break;
            case PMS::RHO: {
                rho_.resize(nChunk_,true);
                }
                break;
            case PMS::NONE:
                break;
        }
	}
}

template<typename T>
void PlotMSCacheBase::addArrays(PtrBlock<Array<T>*>& input) {
    Int oldsize = input.size();
    if (nChunk_ > oldsize) {
        input.resize(nChunk_, false, true);
	    // Construct (empty) pointed-to Vectors
	    for (Int ic=oldsize; ic<nChunk_; ++ic) 
            input[ic] = new Array<T>();
    } else {
        input.resize(nChunk_, true, false);
    }
}

template<typename T>
void PlotMSCacheBase::addMatrices(PtrBlock<Matrix<T>*>& input) {
    Int oldsize = input.size();
    if (nChunk_ > oldsize) {
        input.resize(nChunk_, false, true);
	    // Construct (empty) pointed-to Vectors
	    for (Int ic=oldsize; ic<nChunk_; ++ic) 
            input[ic] = new Matrix<T>();
    } else {
        input.resize(nChunk_, true, false);
    }
}

template<typename T>
void PlotMSCacheBase::addVectors(PtrBlock<Vector<T>*>& input) {
    Int oldsize = input.size();
    if (nChunk_ > oldsize) {
        input.resize(nChunk_, false, true);
	    // Construct (empty) pointed-to Vectors
	    for (Int ic=oldsize; ic<nChunk_; ++ic) 
            input[ic] = new Vector<T>();
    } else {
        input.resize(nChunk_, true, false);
    }
}

void PlotMSCacheBase::deleteCache() {
	// Release all axes.
	release(PMS::axes());

	// zero the meta-name containers
	antnames_.resize();
	stanames_.resize();
	antstanames_.resize();
	fldnames_.resize();

}
void PlotMSCacheBase::deleteIndexer() {
	int indexerCount = indexer_.size();
	for ( int j = 0; j < indexerCount; j++ ){
		for (uInt i=0;i<indexer_[j].nelements();++i){
			if (indexer_[j][i]){
				delete indexer_[j][i];
			}
		}
		indexer_[j].resize(0,true);
	}
	indexer_.clear();
}

void PlotMSCacheBase::setAxesMask(PMS::Axis axis,Vector<Bool>& axismask) {

	// Nominally all false
	axismask.set(false);

	switch(axis) {
	case PMS::AMP:
	case PMS::PHASE:
	case PMS::REAL:
	case PMS::IMAG:
	case PMS::GAMP:
	case PMS::GPHASE:
	case PMS::GREAL:
	case PMS::GIMAG:
	case PMS::DELAY:
	case PMS::SWP:
	case PMS::TSYS:
	case PMS::OPAC:
	case PMS::SNR:
	case PMS::TEC:
	case PMS::FLAG:
	case PMS::WTxAMP:
	case PMS::WTSP:
	case PMS::SIGMASP:
		axismask(Slice(0,3,1))=true;
		break;
	case PMS::CHANNEL:
	case PMS::FREQUENCY:
	case PMS::VELOCITY:
		axismask(1)=true;
		break;
	case PMS::CORR:
		axismask(0)=true;
		break;
	case PMS::ROW:
	case PMS::ANTENNA1:
	case PMS::ANTENNA2:
	case PMS::BASELINE:
	case PMS::UVDIST:
	case PMS::U:
	case PMS::V:
	case PMS::W:
	case PMS::FLAG_ROW:
		axismask(2)=true;
		break;
	case PMS::UVDIST_L:
	case PMS::UWAVE:
	case PMS::VWAVE:
	case PMS::WWAVE:
		axismask(1)=true;
		axismask(2)=true;
		break;
	case PMS::WT:
	case PMS::SIGMA:
		axismask(0)=true;
		axismask(2)=true;
		break;
	case PMS::ANTENNA:
	case PMS::AZIMUTH:
	case PMS::ELEVATION:
	case PMS::PARANG:
		axismask(3)=true;
		break;
	case PMS::TIME:
	case PMS::TIME_INTERVAL:
	case PMS::SCAN:
	case PMS::SPW:
	case PMS::FIELD:
	case PMS::AZ0:
	case PMS::EL0:
	case PMS::HA0:
	case PMS::PA0:
	case PMS::RADIAL_VELOCITY:
	case PMS::RHO:
	case PMS::OBSERVATION:
	case PMS::INTENT:
	case PMS::FEED1:
	case PMS::FEED2:
	case PMS::NONE:
		break;
	}

}

Vector<Bool> PlotMSCacheBase::netAxesMask(PMS::Axis xaxis,PMS::Axis yaxis) {

	if (xaxis==PMS::NONE || yaxis==PMS::NONE)
		throw(AipsError("Problem in PlotMSCacheBase::netAxesMask()."));

	Vector<Bool> xmask(4,false);
	setAxesMask(xaxis,xmask);
	Vector<Bool> ymask(4,false);
	setAxesMask(yaxis,ymask);

	return (xmask || ymask);

}


void PlotMSCacheBase::setPlotMask( int dataIndex ) {

	logLoad("Generating the plot mask.");

	// Generate the plot mask
	//deletePlotMask();
	plmask_[dataIndex].resize(nChunk());
	plmask_[dataIndex].set(NULL);

	for (Int ichk=0; ichk<nChunk(); ++ichk) {
		plmask_[dataIndex][ichk] = new Array<Bool>();
		// create a collapsed version of the flags for this chunk
		setPlotMask(dataIndex, ichk);
	}
}


void PlotMSCacheBase::setPlotMask(Int dataIndex, Int chunk) {

	// Do nothing if chunk empty
	if (!goodChunk_(chunk))
		return;

	IPosition nsh(3,1,1,1),csh;

	for (Int iax=0;iax<3;++iax) {
		if (netAxesMask_[dataIndex](iax))
			// non-trivial size for this axis
			nsh(iax)=chunkShapes()(iax,chunk);
		else
			// add this axis to collapse list
			csh.append(IPosition(1,iax));
	}

	if (netAxesMask_[dataIndex](3) && !netAxesMask_[dataIndex](2)) {
		nsh(2)=chunkShapes()(3,chunk);   // antenna axis length

		plmask_[dataIndex][chunk]->resize(nsh);
		// TBD: derive antenna flags from baseline flags
		plmask_[dataIndex][chunk]->set(true);
	}
	else {
		plmask_[dataIndex][chunk]->resize(nsh);
		(*plmask_[dataIndex][chunk]) = operator>(partialNFalse(*flag_[chunk],csh).reform(nsh),uInt(0));
	}

}

void PlotMSCacheBase::deletePlotMask() {
	int dataCount = plmask_.size();
	for ( int j = 0; j < dataCount; j++ ){
		for (uInt i=0;i<plmask_[j].nelements();++i){
			if (plmask_[j][i]) {
				delete plmask_[j][i];
			}
		}
		plmask_[j].resize(0,true);
	}
	plmask_.resize( 0 );

	// This indexer is no longer ready for plotting
	//dataLoaded_=false;

}



void PlotMSCacheBase::log(const String& method, const String& message,
		int eventType) {
	plotms_->getLogger()->postMessage(PMS::LOG_ORIGIN,method,message,eventType);
}

int PlotMSCacheBase::findColorIndex( int chunk, bool initialize ){
	if ( initialize || uniqueTimes.size() == 0 ){
		uniqueTimes.resize(0);
		for ( int j = 0; j <= nChunk_; j++ ){
			double chunkTime = getTime( j, 0 );
			if ( !uniqueTimes.contains( chunkTime)){
				uniqueTimes.append( chunkTime );
			}
		}
	}
	double timeChunk = getTime(chunk,0);
	int index = uniqueTimes.indexOf( timeChunk );
	return index;
}

}
