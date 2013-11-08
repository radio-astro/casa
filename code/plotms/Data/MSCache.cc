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
#include <casa/OS/HostInfo.h>
#include <casa/OS/Memory.h>
#include <casa/Quanta/MVTime.h>
#include <casa/System/Aipsrc.h>
#include <casa/Utilities/Sort.h>
#include <lattices/Lattices/ArrayLattice.h>
#include <lattices/Lattices/LatticeFFT.h>
#include <scimath/Mathematics/FFTServer.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <ms/MeasurementSets/MSSelection.h>
#include <synthesis/MSVis/VisSet.h>
#include <synthesis/MSVis/VisBuffer2.h>
#include <synthesis/MSVis/VisibilityIterator2.h>
#include <mstransform/MSTransform/MSTransformIteratorFactory.h>
#include <synthesis/MSVis/VisBufferUtil.h>
#include <plotms/Data/PlotMSVBAverager.h>
#include <plotms/Data/MSCacheVolMeter.h>
#include <plotms/PlotMS/PlotMS.h>
#include <tables/Tables/Table.h>
#include <measures/Measures/Stokes.h>

namespace casa {

using namespace vi;

MSCache::MSCache(PlotMSApp* parent):
		  PlotMSCacheBase(parent)
{}

MSCache::~MSCache() {}

String MSCache::polname(Int ipol) {
	return Stokes::name(Stokes::type(ipol));
}


//*********************************
// protected method implementations

void MSCache::loadIt(vector<PMS::Axis>& loadAxes,
		vector<PMS::DataColumn>& loadData,
		ThreadCommunication* thread) {

	{
		// Check if scr cols present
		Bool corcolOk(False);
		const ColumnDescSet cds=Table(filename_).tableDesc().columnDescSet();
		corcolOk=cds.isDefined("CORRECTED_DATA");
		if (!corcolOk) {
			for (uInt i=0;i<loadData.size();++i) {
				switch (loadData[i]) {
				case PMS::CORRECTED:
				case PMS::CORRMODEL: {
					//Exception was removed - see CAS-5214
					loadData[i] = PMS::DATA;
					logWarn( "loadIt", "CORRECTED_DATA column not present; will use DATA instead.");
					//throw(AipsError("CORRECTED_DATA not present, please use DATA"));
					break;
				}
				default:
					break;
				}
			}
		}
	}

	// Get various names
	{
		MeasurementSet ms(filename_);
		ROMSColumns msCol(ms);
		antnames_.resize();
		stanames_.resize();
		antstanames_.resize();
		fldnames_.resize();
		antnames_=msCol.antenna().name().getColumn();
		stanames_=msCol.antenna().station().getColumn();
		fldnames_=msCol.field().name().getColumn();

		antstanames_=antnames_+String("@")+stanames_;

	}

	setUpVisIter(filename_,selection_,True,True,True);

    // TBF: Is this needed for flagging?
// 	if (averaging_.channel())
// 		vi_p->setChanAveBounds(averaging_.channelValue(),chanAveBounds_p);
// 	else
// 		vi_psetChanAveBounds(-1.0,chanAveBounds_p);

	// Remember how many antennas there are
	//   (should remove this)
	nAnt_ = vi_p->getVisBuffer()->nAntennas();

	// TBD: Consolidate count/loadChunks methods?
    Vector<Int> nIterPerAve;
    if(//(averaging_.time() && averaging_.timeValue() > 0.0) ||
       averaging_.baseline() ||
       averaging_.antenna() ||
       averaging_.spw() ) {
        countChunks(*vi_p, nIterPerAve, averaging_, thread);
        trapExcessVolume(pendingLoadAxes_);
        loadChunks(*vi_p, averaging_, nIterPerAve,
                   loadAxes, loadData, thread);
    }
    else {
        // supports other averaging
 		countChunks(*vi_p, thread);
        trapExcessVolume(pendingLoadAxes_);
        loadChunks(*vi_p, loadAxes, loadData, averaging_, thread);
    }

	// Remember # of VBs per Average
	nVBPerAve_.resize();
	if (nIterPerAve.nelements() > 0)
		nVBPerAve_ = nIterPerAve;
	else {
		nVBPerAve_.resize(nChunk_);
		nVBPerAve_.set(1);
	}

	if(vi_p)
		delete vi_p;
	vi_p = NULL;
}

void MSCache::setUpVisIter(const String& msname,
                           const PlotMSSelection& selection,
                           Bool readonly,
                           Bool /*chanselect*/,
                           Bool /*corrselect*/) {
    Record configuration;
    // Selection
    configuration.define("inputms", msname);
    configuration.define("field", selection.field());
    configuration.define("spw", selection.spw());
    configuration.define("timerange", selection.timerange());
    configuration.define("uvrange", selection.uvrange());
    configuration.define("antenna", selection.antenna());
    configuration.define("scan", selection.scan());
    configuration.define("correlation", selection.corr());
    configuration.define("array", selection.array());
    configuration.define("observation", selection.observation());
    // TBF: does the configuration support 'msselect' queries?
    //configuration.define("???", selection_.msselect());
    // Averaging
    if(averaging_.channel()) {
        configuration.define("chanaverage", true);
        configuration.define("chanbin", int(averaging_.channelValue()));
    }
    if(averaging_.time()) {
        configuration.define("timeaverage", true);
        configuration.define("timebin", averaging_.timeStr()+"s");
        String timespan = "";
        if(averaging_.scan()) {
            timespan += "scan";
        }
        // TBF: NYI
        //if(averaging_.field()) {
        //    if(averaging_.scan()) timespan += ",";
        //   timespan += "field";
        //}
        configuration.define("timespan", timespan);
    }
    // TBF: Inconceivably, this doesn't mean what I thought it meant
    //configuration.define("combinespws", averaging_.spw());
    // Transformations
    configuration.define("outframe", transformations_.frameStr());
    // TBF: does the configuration support "TRUE" velocity definition?
    configuration.define("veltype", transformations_.veldefStr());
    configuration.define("restfreq",
                         String::toString(transformations_.restFreq())+"MHz");
    // TBF: the configuration needs an absolute phase center, not an offset from
    //      the current phase center
    //configuration.define("phasecenter", ???);

    MSTransformIteratorFactory factory(configuration);
    vi_p = new vi::VisibilityIterator2 (factory);
    vi_p->originChunks();
    vi_p->origin();

    {
        Table::TableOption tabopt(Table::Update);
        if (readonly) tabopt = Table::Old;
        MeasurementSet ms(msname, TableLock(TableLock::AutoLocking), tabopt);
        MeasurementSet selms;
        // Apply selection
        Vector<Vector<Slice> > chansel;
        Vector<Vector<Slice> > corrsel;
        selection.apply(ms,selms,chansel,corrsel);
        // setup the volume meter
        vm_.reset();
        vm_ = MSCacheVolMeter(ms,averaging_,chansel,corrsel);
    }
}

void MSCache::countChunks(VisibilityIterator2& vi,
                          ThreadCommunication* thread ) {
	if (thread!=NULL) {
		thread->setStatus("Establishing cache size.  Please wait...");
		thread->setAllowedOperations(false,false,false);
	}

	// This is the old way, with no averaging over chunks.

	VisBuffer2 *vb = vi.getVisBuffer();

	vi.originChunks();
	vi.origin();
	//  refTime_p=86400.0*floor(vb->time()(0)/86400.0);

	// Count number of chunks.
	int chunk = 0;
	for(vi.originChunks(); vi.moreChunks(); vi.nextChunk()) {

		if (thread!=NULL) {
			if (thread->wasCanceled()) {
				dataLoaded_=false;
				return;
			}
			else{
				thread->setProgress(2);
			}
		}

		for (vi.origin(); vi.more(); vi.next()) {
			++chunk;
			vm_.add(*vb);
		}
	}
	if(chunk != nChunk_) increaseChunks(chunk);

	//  cout << "Found " << nChunk_ << " " << chunk << " chunks." << endl;

}


void MSCache::countChunks(VisibilityIterator2& vi, Vector<Int>& nIterPerAve,
                          const PlotMSAveraging& averaging,
                          ThreadCommunication* thread) {
	if (thread!=NULL) {
		thread->setStatus("Establishing cache size.  Please wait...");
		thread->setAllowedOperations(false,false,false);
	}

	Bool verby(False);

	Bool combscan(averaging_.scan());
	Bool combfld(averaging_.field());
	Bool combspw(averaging_.spw());

	vi.originChunks();
	vi.origin();

	VisBuffer2 *vb = vi.getVisBuffer();

	nIterPerAve.resize(100);
	nIterPerAve=0;

	Double time0(86400.0*floor(vb->time()(0)/86400.0));
	//  refTime_p=time0;
	Double time1(0.0),time(0.0);

	Int thisscan(-1),lastscan(-1);
	Int thisfld(-1),lastfld(-1);
	Int thisspw(-1),lastspw(-1);
	Int thisddid(-1),lastddid(-1);
	Int maxAveNRows(0);
	Int chunk(0);
	Int ave(-1);
	Double interval(0.0);
	if (averaging.time())
		interval= averaging.timeValue();
	Double avetime1(-1.0);
	stringstream ss;
	for (vi.originChunks(); vi.moreChunks(); vi.nextChunk(),chunk++) {
		if (thread!=NULL) {
			if (thread->wasCanceled()) {
				dataLoaded_=false;
				return;
			}
			else {
				thread->setProgress(2);
			}
		}

		Int iter(0);
		for (vi.origin(); vi.more(); vi.next(),iter++) {

			time1=vb->time()(0);  // first time in this vb
			thisscan = vb->scan()(0);
			thisfld = vb->fieldId()(0);
			thisspw = vb->spectralWindows()(0);
			thisddid = vb->dataDescriptionIds()(0);

			// New chunk means new ave interval, IF....
			if ( // (!combfld && !combspw) ||                // not combing fld nor spw, OR
                ((time1-avetime1)>interval) ||         // (combing fld and/or spw) and solint exceeded, OR
                ((time1-avetime1)<0.0) ||                // a negative time step occurs, OR
                (!combscan && (thisscan!=lastscan)) ||   // not combing scans, and new scan encountered OR
                (!combspw && (thisspw!=lastspw)) ||      // not combing spws, and new spw encountered  OR
                (!combfld && (thisfld!=lastfld)) ||      // not combing fields, and new field encountered OR
                (ave==-1))  {                            // this is the first interval

				if (verby) {
					ss << "--------------------------------\n";
					ss << boolalpha << interval << " "
							<< ((time1-avetime1)>interval)  << " "
							<< ((time1-avetime1)<0.0) << " "
							<< (!combscan && (thisscan!=lastscan)) << " "
							<< (!combspw && (thisspw!=lastspw)) << " "
							<< (!combfld && (thisfld!=lastfld)) << " "
							<< (ave==-1) << "\n";
				}

				// If we have accumulated enough info, poke the volume meter,
				//  with the _previous_ info, and reset the ave'd row counter
				if (ave>-1) {
					vm_.add(lastddid,maxAveNRows);
					maxAveNRows=0;
				}


				avetime1=time1;  // for next go
				ave++;

				if (verby) ss << "ave = " << ave << "\n";


				// increase size of nIterPerAve array, if needed
				if (nIterPerAve.nelements()<uInt(ave+1))
					nIterPerAve.resize(nIterPerAve.nelements()+100,True);
				nIterPerAve(ave)=0;
			}

			// Keep track of the maximum # of rows that might get averaged
			maxAveNRows=max(maxAveNRows,vb->nRows());


			// Increment chunk-per-sol count for current solution
			nIterPerAve(ave)++;

			if (verby) {
				ss << "          ck=" << chunk << " " << avetime1-time0 << "\n";
				time=vb->time()(0);
				ss  << "                 " << "vb=" << iter << " ";
				ss << "ar=" << vb->arrayId()(0) << " ";
				ss << "sc=" << vb->scan()(0) << " ";
				if (!combfld) ss << "fl=" << vb->fieldId()(0) << " ";
				if (!combspw) ss << "sp=" << vb->spectralWindows()(0) << " ";
				ss << "t=" << floor(time-time0)  << " (" << floor(time-avetime1) << ") ";
				if (combfld) ss << "fl=" << vb->fieldId()(0) << " ";
				if (combspw) ss << "sp=" << vb->spectralWindows()(0) << " ";
				ss << "\n";

			}

			lastscan=thisscan;
			lastfld=thisfld;
			lastspw=thisspw;
			lastddid=thisddid;
		}
	}
	// Add in the last iteration
	vm_.add(lastddid,maxAveNRows);

	Int nAve(ave+1);
	nIterPerAve.resize(nAve,True);

	if (verby)  ss << "nIterPerAve = " << nIterPerAve;
	if (verby) logInfo("count_chunks", ss.str());


	if (nChunk_ != nAve) increaseChunks(nAve);

}

void MSCache::trapExcessVolume(map<PMS::Axis,Bool> pendingLoadAxes) {
	try {
		String s;
		s=vm_.evalVolume(pendingLoadAxes,netAxesMask(currentX_,currentY_));
		logLoad(s);
	} catch(AipsError& log) {
		// catch detected volume excess, clear the existing cache, and rethrow
		logLoad(log.getMesg());
		clear();
		stringstream ss;
		ss << "Please try selecting less data or averaging and/or" << endl
				<< " 'force reload' (to clear unneeded cache items) and/or" << endl
				<< " letting other memory-intensive processes finish.";
		throw(AipsError(ss.str()));
	}
}


void MSCache::loadChunks(VisibilityIterator2& vi,
                         const vector<PMS::Axis>& loadAxes,
                         const vector<PMS::DataColumn>& loadData,
                         const PlotMSAveraging& averaging,
                         ThreadCommunication* thread) {

	// permit cancel in progress meter:
	if(thread != NULL)
		thread->setAllowedOperations(false,false,true);

	logLoad("Loading chunks......");
	VisBuffer2 *vb = vi.getVisBuffer();

	// Initialize the freq/vel calculator (in case we use it)
	//vbu_=VisBufferUtil(*vb);

	Int chunk = 0;
	chshapes_.resize(4,nChunk_);
	goodChunk_.resize(nChunk_);
	goodChunk_.set(False);
	double progress;
	for(vi.originChunks(); vi.moreChunks(); vi.nextChunk()) {
		for(vi.origin(); vi.more(); vi.next()) {
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


			// Force data I/O so that shifting averaging will work, if nec.
			forceVBread(*vb, loadAxes, loadData);

			// Adjust the visibility phase by phase-center shift
			//vb->phaseCenterShift(transformations_.xpcOffset(),
            //                     transformations_.ypcOffset());


			// Do channel averaging, if required
			if (averaging.channel() && averaging.channelValue()>0.0) {
				// Delegate actual averaging to the VisBuffer:
				//vb->channelAve(chanAveBounds_p(vb->spectralWindows()(0)));
			}

			// Cache the data shapes
			chshapes_(0,chunk) = vb->nCorrelations();
			chshapes_(1,chunk) = vb->nChannels();
			chshapes_(2,chunk) = vb->nRows();
			chshapes_(3,chunk) = vb->nAntennas();
			goodChunk_(chunk)=True;

			/*  not yet!
      // Make "U vs V"-like plots half-plane
      switch (currentX_) {
      case PMS::U:
      case PMS::V:
      case PMS::W:
      case PMS::UWAVE:
      case PMS::VWAVE:
      case PMS::WWAVE: {
	switch (currentY_) {
	case PMS::U:
	case PMS::V:
	case PMS::W:
	case PMS::UWAVE:
	case PMS::VWAVE:
	case PMS::WWAVE: {
	  // Invert sign when U<0
	  Vector<Double> uM(vb->uvwMat().row(0));
	  for (uInt iu=0;iu<uM.nelements();++iu)
	    if (uM(iu)<0.0) {
	      Vector<Double> uvw(vb->uvwMat().column(iu));
	      uvw*=-1.0;
	    }
	  break;
	}
	default: 
	  break;
	}
      }
      default: 
	break;
      }
			 */
			for(unsigned int i = 0; i < loadAxes.size(); i++) {
				//	cout << PMS::axis(loadAxes[i]) << " ";
				loadAxis(*vb, chunk, loadAxes[i], loadData[i]);
			}
			//cout << endl;
			chunk++;

			// If a thread is given, update it.
			if(thread != NULL && (nChunk_ <= (int)THREAD_SEGMENT ||
					chunk % THREAD_SEGMENT == 0)) {
				progress = ((double)chunk+1) / nChunk_;
				thread->setProgress((unsigned int)((progress * 100) + 0.5));
			}
		}
	}
}

void MSCache::loadChunks(VisibilityIterator2& vi,
                         const PlotMSAveraging& averaging,
                         const Vector<Int>& nIterPerAve,
                         const vector<PMS::Axis>& loadAxes,
                         const vector<PMS::DataColumn>& loadData,
                         ThreadCommunication* thread) {

	// permit cancel in progress meter:
	if(thread != NULL)
		thread->setAllowedOperations(false,false,true);

	logLoad("Loading chunks with averaging.....");

	Bool verby(False);

    vi.originChunks();
    vi.origin();
	VisBuffer2 *vb = vi.getVisBuffer();

	// Initialize the freq/vel calculator (in case we use it)
	//vbu_=VisBufferUtil(*vb);

	chshapes_.resize(4,nChunk_);
	goodChunk_.resize(nChunk_);
	goodChunk_.set(False);
	double progress;
	vi.originChunks();
	vi.origin();


	Double time0=86400.0*floor(vb->time()(0)/86400.0);
	for (Int chunk=0;chunk<nChunk_;++chunk) {

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

		// Arrange to accumulate many VBs into one
		PlotMSVBAverager pmsvba(vb->nAntennas(), vi.weightSpectrumExists());

		// Tell averager if we are averaging baselines together
		pmsvba.setBlnAveraging(averaging.baseline());
		pmsvba.setAntAveraging(averaging.antenna());

		// Turn on scalar averaging, if set (else Vector Ave will be done)
		pmsvba.setScalarAve(averaging.scalarAve());

		// Sort out which data to read
		discernData(loadAxes,loadData,pmsvba);

		stringstream ss;
		if (verby) ss << chunk << "----------------------------------\n";

		for (Int iter=0;iter<nIterPerAve(chunk);++iter) {

			// Force read on required stuff
			forceVBread(*vb,loadAxes,loadData);

			if (verby) {
				ss << "ck=" << chunk << " vb=" << iter << " (" << nIterPerAve(chunk) << ");  "
                   << "sc=" << vb->scan()(0) << " "
                   << "time=" << vb->time()(0)-time0 << " "
                   << "fl=" << vb->fieldId()(0) << " "
                   << "sp=" << vb->spectralWindows()(0) << " ";
			}

			// Adjust the visibility phase by phase-center shift
			//vb->phaseCenterShift(transformations_.xpcOffset(),
            //                     transformations_.ypcOffset());

			// Do channel averaging, if required
			if (averaging.channel() && averaging.channelValue()>0.0) {
				// Delegate actual averaging to the VisBuffer:
				//vb->channelAve(chanAveBounds_p(vb->spectralWindows()(0)));
			}

			// Accumulate into the averager
			pmsvba.accumulate(*vb);

			// Advance to next VB
			vi.next();

			if (verby) ss << " next VB ";


			if (!vi.more() && vi.moreChunks()) {
				// go to first vb in next chunk
				if (verby) ss << "  stepping VI";
				vi.nextChunk();
				vi.origin();
			}
			if (verby) ss << "\n";
		}

		if (verby) logLoad(ss.str());


		// Finalize the averaging
		pmsvba.finalizeAverage();

		// The averaged VisBuffer
		VisBuffer2& avb = pmsvba.aveVisBuff();

		// Only if the average yielded some data
		if (avb.nRows()>0) {

			// Form Stokes parameters, if requested
			//if (transformations_.formStokes())
			//	avb.formStokes();

			// Cache the data shapes
			chshapes_(0,chunk)=avb.nCorrelations();
			chshapes_(1,chunk)=avb.nChannels();
			chshapes_(2,chunk)=avb.nRows();
			chshapes_(3,chunk)=vi.getVisBuffer()->nAntennas();
			goodChunk_(chunk)=True;

			/*  not yet
      // Make "U vs V"-like plots half-plane
      switch (currentX_) {
      case PMS::U:
      case PMS::V:
      case PMS::W:
      case PMS::UWAVE:
      case PMS::VWAVE:
      case PMS::WWAVE: {
	switch (currentY_) {
	case PMS::U:
	case PMS::V:
	case PMS::W:
	case PMS::UWAVE:
	case PMS::VWAVE:
	case PMS::WWAVE: {
	  // Invert sign when U<0
	  Vector<Double> uM(avb.uvw().row(0));
	  for (uInt iu=0;iu<uM.nelements();++iu)
	    if (uM(iu)<0.0) {
	      Vector<Double> uvw(avb.uvw().column(iu));
	      uvw*=-1.0;
	    }
	  break;
	}
	default: 
	  break;
	}
      }
      default: 
	break;
      }
			 */

			for(unsigned int i = 0; i < loadAxes.size(); i++) {
				loadAxis(avb, chunk, loadAxes[i], loadData[i]);

			}
		}
		else {
			// no points in this chunk
			goodChunk_(chunk)=False;
			chshapes_.column(chunk)=0;
		}

		// If a thread is given, update it.
		if(thread != NULL && (nChunk_ <= (int)THREAD_SEGMENT ||
				chunk % THREAD_SEGMENT == 0)) {
			progress = ((double)chunk+1) / nChunk_;
			thread->setProgress((unsigned int)((progress * 100) + 0.5));
		}
	}

	//  cout << boolalpha << "goodChunk_ = " << goodChunk_ << endl;
}

void MSCache::forceVBread(VisBuffer2& vb,
                          const vector<PMS::Axis> &loadAxes,
                          const vector<PMS::DataColumn> &loadData) {

	// pre-load requisite pieces of VisBuffer for averaging
	for(unsigned int i = 0; i < loadAxes.size(); i++) {
		switch (loadAxes[i]) {
		case PMS::AMP:
		case PMS::PHASE:
		case PMS::REAL:
		case PMS::IMAG: {
			switch(loadData[i]) {
			case PMS::DATA: {
				vb.visCube();
				break;
			}
			case PMS::MODEL: {
				vb.visCubeModel();
				break;
			}
			case PMS::CORRECTED: {
				vb.visCubeCorrected();
				break;
			}
			case PMS::CORRMODEL: {
				vb.visCubeCorrected();
				vb.visCubeModel();
				break;
			}
			case PMS::DATAMODEL: {
				vb.visCube();
				vb.visCubeModel();
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
	vb.flagRow();
	vb.flagCube();

}

void MSCache::discernData(const vector<PMS::Axis>& loadAxes,
                          const vector<PMS::DataColumn>& loadData,
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




void MSCache::loadAxis(VisBuffer2& vb, Int vbnum, PMS::Axis axis,
                       PMS::DataColumn data) {

	switch(axis) {

	case PMS::SCAN: // assumes scan unique in VB
		scan_(vbnum) = vb.scan()(0);
		break;

	case PMS::FIELD:
		field_(vbnum) = vb.fieldId()(0);
		break;

	case PMS::TIME: // assumes time unique in VB
		time_(vbnum) = vb.time()(0);
		break;

	case PMS::TIME_INTERVAL: // assumes timeInterval unique in VB
		timeIntr_(vbnum) = vb.timeInterval()(0);
		break;

	case PMS::SPW:
		spw_(vbnum) = vb.spectralWindows()(0);
		break;

	case PMS::CHANNEL:
        chan_[vbnum]->resize(vb.nChannels());
		*chan_[vbnum] = vb.getChannelNumbers(0);
		break;

	case PMS::FREQUENCY: {
		//      cout << "Loading FREQUENCY" << endl;
        *freq_[vbnum] = vb.getFrequencies(0/*, transformations_.frame()*/) / 1.0e9;
		break;
	}
	case PMS::VELOCITY: {
		// Convert freq in the vb to velocity
// 		vbu_.toVelocity(*vel_[vbnum],
//                         vb,
//                         transformations_.frame(),
//                         MVFrequency(transformations_.restFreqHz()),
//                         transformations_.veldef());
		(*vel_[vbnum])/=1.0e3;  // in km/s
		break;
	}

	case PMS::CORR:
		*corr_[vbnum] = vb.getCorrelationTypes();
		break;

	case PMS::ANTENNA1:
		*antenna1_[vbnum] = vb.antenna1();
		break;
	case PMS::ANTENNA2:
		*antenna2_[vbnum] = vb.antenna2();
		break;
	case PMS::BASELINE: {
		Vector<Int> a1(vb.antenna1());
		Vector<Int> a2(vb.antenna2());
		baseline_[vbnum]->resize(vb.nRows());
		Vector<Int> bl(*baseline_[vbnum]);
		for (Int irow=0;irow<vb.nRows();++irow) {
			if (a1(irow)<0) a1(irow)=chshapes_(3,0);
			if (a2(irow)<0) a2(irow)=chshapes_(3,0);
			bl(irow)=(chshapes_(3,0)+1)*a1(irow) - (a1(irow) * (a1(irow) + 1)) / 2 + a2(irow);
		}
		break;
	}
	case PMS::UVDIST: {
		Array<Double> u(vb.uvw().row(0));
		Array<Double> v(vb.uvw().row(1));
		*uvdist_[vbnum] = sqrt(u*u+v*v);
		break;
	}
	case PMS::U:
		*u_[vbnum] = vb.uvw().row(0);
		break;
	case PMS::V:
		*v_[vbnum] = vb.uvw().row(1);
		break;
	case PMS::W:
		*w_[vbnum] = vb.uvw().row(2);
		break;
	case PMS::UVDIST_L: {
		Array<Double> u(vb.uvw().row(0));
		Array<Double> v(vb.uvw().row(1));
		Vector<Double> uvdistM = sqrt(u*u+v*v);
		uvdistM /=C::c;
		uvdistL_[vbnum]->resize(vb.nChannels(),vb.nRows());
		Vector<Double> uvrow;
		for (Int irow=0;irow<vb.nRows();++irow) {
			uvrow.reference(uvdistL_[vbnum]->column(irow));
			uvrow.set(uvdistM(irow));
			uvrow *= vb.getFrequencies(0);
		}
		break;
	}

	case PMS::UWAVE: {
		Vector<Double> uM(vb.uvw().row(0));
		uM/=C::c;
		uwave_[vbnum]->resize(vb.nChannels(),vb.nRows());
		Vector<Double> urow;
		Vector<Double> freq(vb.getFrequencies(0));
		for (Int irow=0;irow<vb.nRows();++irow) {
			urow.reference(uwave_[vbnum]->column(irow));
			urow.set(uM(irow));
			urow*=freq;
		}
		break;
	}
	case PMS::VWAVE: {
		Vector<Double> vM(vb.uvw().row(1));
		vM/=C::c;
		vwave_[vbnum]->resize(vb.nChannels(),vb.nRows());
		Vector<Double> vrow;
		Vector<Double> freq(vb.getFrequencies(0));
		for (Int irow=0;irow<vb.nRows();++irow) {
			vrow.reference(vwave_[vbnum]->column(irow));
			vrow.set(vM(irow));
			vrow*=freq;
		}
		break;
	}
	case PMS::WWAVE: {
		Vector<Double> wM(vb.uvw().row(2));
		wM/=C::c;
		wwave_[vbnum]->resize(vb.nChannels(),vb.nRows());
		Vector<Double> wrow;
		Vector<Double> freq(vb.getFrequencies(0));
		for (Int irow=0;irow<vb.nRows();++irow) {
			wrow.reference(wwave_[vbnum]->column(irow));
			wrow.set(wM(irow));
			wrow*=freq;
		}
		break;
	}
	case PMS::AMP: {
		switch(data) {
		case PMS::DATA: {
			MeasurementSet ms( filename_);
			//Please see CAS-5730.  For single dish data, absolute value of
			//points should not be plotted.
			if ( ms.isColumn( MS::FLOAT_DATA ) ){
				*amp_[vbnum]=real(vb.visCube());
			}
			else {
				*amp_[vbnum] = amplitude(vb.visCube());
			}
			// TEST fft on freq axis to get delay
			if (False) {

				// Only transform frequency axis
				//   (Should avoid cross-hand data, too?)
				Vector<Bool> ax(3,False);
				ax(1)=True;

				// Support padding for higher delay resolution
				Int fact(4);
				IPosition ip=vb.visCube().shape();
				Int nch=ip(1);
				ip(1)*=fact;

				Slicer sl(Slice(),Slice(nch*(fact-1)/2,nch,1),Slice());

				Array<Complex> vpad(ip);
				vpad.set(Complex(0.0));
				vpad(sl)=vb.visCube();


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
			*amp_[vbnum] = amplitude(vb.visCubeModel());
			break;
		}
		case PMS::CORRECTED: {
			*amp_[vbnum] = amplitude(vb.visCubeCorrected());
			break;
		}
		case PMS::CORRMODEL: {
			*amp_[vbnum] = amplitude(vb.visCubeCorrected() - vb.visCubeModel());
			break;
		}
		case PMS::DATAMODEL: {
			*amp_[vbnum] = amplitude(vb.visCube() - vb.visCubeModel());
			break;
		}
		}
		break;
	}
	case PMS::PHASE: {
		switch(data) {
		case PMS::DATA: {
			*pha_[vbnum] = phase(vb.visCube()) * 180.0 / C::pi;
			break;
		}
		case PMS::MODEL: {
			*pha_[vbnum] = phase(vb.visCubeModel()) * 180.0 / C::pi;
			break;
		}
		case PMS::CORRECTED: {
			*pha_[vbnum] = phase(vb.visCubeCorrected()) * 180.0 / C::pi;
			break;
		}
		case PMS::CORRMODEL: {
			*pha_[vbnum] = phase(vb.visCubeCorrected() - vb.visCubeModel()) *
					180.0 / C::pi;
			break;
		}
		case PMS::DATAMODEL: {
			*pha_[vbnum] = phase(vb.visCube() - vb.visCubeModel()) * 180 / C::pi;
			break;
		}
		}
		break;
	}

	case PMS::REAL: {
		switch(data) {
		case PMS::DATA: {
			*real_[vbnum] = real(vb.visCube());
			break;
		}
		case PMS::MODEL: {
			*real_[vbnum] = real(vb.visCubeModel());
			break;
		}
		case PMS::CORRECTED: {
			*real_[vbnum] = real(vb.visCubeCorrected());
			break;
		}
		case PMS::CORRMODEL: {
			*real_[vbnum] = real(vb.visCubeCorrected()) - real(vb.visCubeModel());
			break;
		}
		case PMS::DATAMODEL: {
			*real_[vbnum] = real(vb.visCube()) - real(vb.visCubeModel());
			break;
		}
		}
		break;
	}
	case PMS::IMAG: {
		switch(data) {
		case PMS::DATA: {
			*imag_[vbnum] = imag(vb.visCube());
			break;
		}
		case PMS::MODEL: {
			*imag_[vbnum] = imag(vb.visCubeModel());
			break;
		}
		case PMS::CORRECTED: {
			*imag_[vbnum] = imag(vb.visCubeCorrected());
			break;
		}
		case PMS::CORRMODEL: {
			*imag_[vbnum] = imag(vb.visCubeCorrected()) - imag(vb.visCubeModel());
			break;
		}
		case PMS::DATAMODEL: {
			*imag_[vbnum] = imag(vb.visCube()) - imag(vb.visCubeModel());
			break;
		}
		}
		break;
	}

	case PMS::FLAG:
		*flag_[vbnum] = vb.flagCube();
		break;
	case PMS::FLAG_ROW:
		*flagrow_[vbnum] = vb.flagRow();
		break;

	case PMS::WT: {
		*wt_[vbnum] = vb.weight();
		break;
	}
	case PMS::WTxAMP: {
		switch(data) {
		case PMS::DATA:
			*wtxamp_[vbnum] = amplitude(vb.visCube());
			break;
		case PMS::MODEL:
			*wtxamp_[vbnum] = amplitude(vb.visCubeModel());
			break;
		case PMS::CORRECTED:
			*wtxamp_[vbnum] = amplitude(vb.visCubeCorrected());
			break;
		case PMS::CORRMODEL:
			*wtxamp_[vbnum] = amplitude(vb.visCubeCorrected() - vb.visCube());
			break;
		case PMS::DATAMODEL:
			*wtxamp_[vbnum] = amplitude(vb.visCube() - vb.visCubeModel());
			break;
		}
		uInt nchannels = vb.nChannels();
		Cube<Float> wtA(*wtxamp_[vbnum]);
		for(uInt c = 0; c < nchannels; ++c) {
			wtA.xzPlane(c) = wtA.xzPlane(c) * vb.weight();
		}
	}
	case PMS::AZ0:
	case PMS::EL0: {
		MDirection azelMeas = vb.azel0(vb.time()(0));
        Vector<double> azel;
        azel.resize(2);
        azel = azelMeas.getAngle("deg").getValue();
		az0_(vbnum) = azel(0);
		el0_(vbnum) = azel(1);
		break;
	}

	case PMS::HA0:
		ha0_(vbnum) = vb.hourang(vb.time()(0))*12/C::pi;  // in hours
		break;
	case PMS::PA0: {
		pa0_(vbnum) = vb.parang0(vb.time()(0))*180.0/C::pi; // in degrees
		if (pa0_(vbnum)<0.0) pa0_(vbnum)+=360.0;
		break;
	}
	case PMS::ANTENNA: {
		antenna_[vbnum]->resize(nAnt_);
		indgen(*antenna_[vbnum]);
		break;
	}
	case PMS::AZIMUTH:
	case PMS::ELEVATION: {
        Vector<MDirection> azelMeas = vb.azel(vb.time()(0));
		Matrix<Double> azel;
        azel.resize(2, azelMeas.nelements());
        for(size_t iant = 0; iant < azelMeas.nelements(); ++iant) {
            azel.column(iant) = azelMeas(iant).getAngle("deg").getValue();
        }
		*az_[vbnum] = azel.row(0);
		*el_[vbnum] = azel.row(1);
		break;
	}
	case PMS::RADIAL_VELOCITY: {
		Int fieldId = vb.fieldId()(0);
		const ROMSFieldColumns& fieldColumns = vi_p->subtableColumns().field();
		MRadialVelocity radVelocity = fieldColumns.radVelMeas(fieldId, vb.time()(0));
		radialVelocity_(vbnum) = radVelocity.get("AU/d").getValue( "km/s");
		break;
	}
	case PMS::RHO:{
		Int fieldId = vb.fieldId()(0);
		const ROMSFieldColumns& fieldColumns = vi_p->subtableColumns().field();
		Quantity rhoQuantity = fieldColumns.rho(fieldId, vb.time()(0));
		rho_(vbnum ) = rhoQuantity.getValue( "km");
		break;
	}
	case PMS::PARANG:
		*parang_[vbnum] = vb.feedPa(vb.time()(0))*(180.0/C::pi);  // in degrees
		break;

	case PMS::ROW:
		*row_[vbnum] = vb.rowIds();
		break;

	default:
		throw(AipsError("Axis choice not supported for MS"));
		break;
	}
}

bool MSCache::isEphemeris(){
	setUpVisIter(filename_,selection_,True,True,True);
	VisBuffer2 *vb = vi_p->getVisBuffer();
	Int fieldId = vb->fieldId()(0);
	const ROMSFieldColumns& fieldColumns = vi_p->subtableColumns().field();
	String ephemerisExists = fieldColumns.ephemPath( fieldId );
	bool ephemerisAvailable = true;
	if ( ephemerisExists.empty()){
		ephemerisAvailable = false;
	}
	return ephemerisAvailable;
}


void MSCache::flagToDisk(const PlotMSFlagging& flagging,
		Vector<Int>& flchunks, Vector<Int>& flrelids,
		Bool flag, PlotMSIndexer* indexer) {

	// Sort the flags by chunk:
	Sort sorter;
	sorter.sortKey(flchunks.data(),TpInt);
	sorter.sortKey(flrelids.data(),TpInt);
	Vector<uInt> order;
	uInt nflag;
	nflag = sorter.sort(order,flchunks.nelements());

	stringstream ss;

	// Make the VisIterator writable, with selection revised as appropriate
	Bool selectchan(netAxesMask_(1) && !flagging.channel());
	Bool selectcorr(netAxesMask_(0) && !flagging.corrAll());

	// Establish a scope in which the VisBuffer is properly created/destroyed
	{
		setUpVisIter(filename_,selection_,False,selectchan,selectcorr);
		VisBuffer2 *vb = vi_p->getVisBuffer();

		vi_p->originChunks();
		vi_p->origin();

		Int iflag(0);
		for (Int ichk=0;ichk<nChunk_;++ichk) {

			if (ichk!=flchunks(order[iflag])) {
				// Step over current chunk
				for (Int i=0;i<nVBPerAve_(ichk);++i) {
					vi_p->next();
					if (!vi_p->more() && vi_p->moreChunks()) {
						vi_p->nextChunk();
						vi_p->origin();
					}
				}
			}
			else {

				// This chunk requires flag-setting

				// For each VB in this cache chunk
				Int ifl(iflag);
				for (Int i=0;i<nVBPerAve_(ichk);++i) {

					// Refer to VB pieces we need
					Cube<Bool> vbflag(vb->flagCube());
					Vector<Bool> vbflagrow(vb->flagRow());
					Vector<Int> corrType(vb->getCorrelationTypes());
					Vector<Int> channel(vb->getChannelNumbers(0));
					Vector<Int> a1(vb->antenna1());
					Vector<Int> a2(vb->antenna2());
					Int ncorr=corrType.nelements();
					Int nchan=channel.nelements();
					Int nrow=vb->nRows();

					if (False) {
						Int currChunk=flchunks(order[iflag]);
						Double time=getTime(currChunk,0);
						Int spw=Int(getSpw(currChunk,0));
						Int field=Int(getField(currChunk,0));
						ss << "Time diff: " << time-vb->time()(0) << " "  << time << " " << vb->time()(0) << "\n";
						ss << "Spw diff:  " << spw-vb->spectralWindows()(0) << " " << spw << " " << vb->spectralWindows()(0) << "\n";
						ss << "Field diff:  " << field-vb->fieldId()(0) << " " << field << " " << vb->fieldId()(0) << "\n";
					}

					// Apply all flags in this chunk to this VB
					ifl=iflag;
					while (ifl<Int(nflag) && flchunks(order[ifl])==ichk) {

						Int currChunk=flchunks(order[ifl]);
						Int irel=flrelids(order[ifl]);

						Slice corr,chan,bsln;

						// Set flag range on correlation axis:
						if (netAxesMask_(0) && !flagging.corrAll()) {
							// A specific single correlation
							Int icorr=indexer->getIndex1000(currChunk,irel);
							corr=Slice(icorr,1,1);
						}
						else
							corr=Slice(0,ncorr,1);

						// Set Flag range on channel axis:
						if (netAxesMask_(1) && !flagging.channel()) {
							Int ichan=indexer->getIndex0100(currChunk,irel);
							if (averaging_.channel() && averaging_.channelValue()>0) {
								Int start=chanAveBounds_p(vb->spectralWindows()(0))(ichan,2);
								Int n=chanAveBounds_p(vb->spectralWindows()(0))(ichan,3)-start+1;
								chan=Slice(start,n,1);
							}
							else
								// A single specific channel
								chan=Slice(ichan,1,1);
						}
						else
							// Extend to all channels
							chan=Slice(0,nchan,1);

						// Set Flags on the baseline axis:
						Int thisA1=Int(getAnt1(currChunk,indexer->getIndex0010(currChunk,irel)));
						Int thisA2=Int(getAnt2(currChunk,indexer->getIndex0010(currChunk,irel)));
						if (netAxesMask_(2) &&
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
										vbflag(corr,chan,Slice(irow,1,1))=flag;
										if (!flag) vbflagrow(irow)=False;   // unset flag_row when unflagging

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
										vbflag(corr,chan,Slice(irow,1,1))=flag;
										if (!flag) vbflagrow(irow)=False;   // unset flag_row when unflagging
									}
								}
							}
						}
						else {
							// Set flags for all baselines, because the plot
							//  is ordinarily implicit in baseline, we've turned on baseline
							//  extension, or we've avaraged over all baselines
							bsln=Slice(0,nrow,1);
							vbflag(corr,chan,bsln)=flag;
							if (!flag) vbflagrow(bsln)=False;   // unset flag_row when unflagging
						}

						++ifl;
					}

					// Put the flags back into the MS
					vi_p->writeFlag(vbflag);
					if (!flag) vi_p->writeFlagRow(vbflagrow);

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

			} // flagable VB


		} // ichk

		// Close the scope that holds the VisBuffer used above
	}

	// Delete the VisIter so lock is released
	if (vi_p)
		delete vi_p;
	vi_p=NULL;

	logFlag(ss.str());
}

}
