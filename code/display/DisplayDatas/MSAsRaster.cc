//# MSAsRaster.cc:  DisplayData (drawing layer) for raster displays of an MS
//# Copyright (C) 2000,2001,2002,2003
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

#include <display/DisplayDatas/MSAsRaster.h>

#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <casa/sstream.h>
#include <casa/iomanip.h>
#include <casa/BasicMath/Math.h>
#include <msvis/MSVis/VisBuffer.h>
#include <msvis/MSVis/VisibilityIterator.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <ms/MSSel/MSSelectionTools.h>
#include <coordinates/Coordinates/LinearCoordinate.h>
#include <display/Display/DisplayCoordinateSystem.h>
#include <coordinates/Coordinates/LinearCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/TabularCoordinate.h>
#include <casa/Arrays/ArrayMath.h>
#include <display/Display/Attribute.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/PixelCanvas.h>
#include <display/DisplayCanvas/WCResampleHandler.h>
#include <casa/OS/Timer.h>
#include <display/DisplayEvents/CrosshairEvent.h>
#include <display/DisplayEvents/RectRegionEvent.h>
#include <display/DisplayEvents/MWCEvents.h>
#include <display/Display/ColormapDefinition.h>
#include <casa/Utilities/Regex.h>
#include <casa/System/Aipsrc.h>
#include <casa/Quanta/MVTime.h>
#include <casa/BasicSL/Constants.h>
#include <casa/OS/HostInfo.h>
#include <casa/BasicMath/Random.h>
#include <ms/MSSel/MSSelection.h>
#include <casa/Arrays/ArrayAccessor.h>
#include <cstdarg>

namespace casa { //# NAMESPACE CASA - BEGIN

    IPosition ipos( int num, ... ) {
        std::vector<int> pos(num);
        va_list vl;
        va_start(vl,num);
        for ( int i=0; i < num; ++i )
            pos[i] = va_arg(vl,int);
        va_end(vl);
        return IPosition(pos);
    }

//---------------------------------------------------------------------
	MSAsRaster::MSAsRaster( const String msname, const viewer::DisplayDataOptions &ddo ):

		ActiveCaching2dDD(),		// base class

		axisName_(NAXES), visTypeName_(NTYPES),
		visCompName_(NCOMPNAMES),
		msName_(msname),
		itsVisMb(0),
		itsXAxis(0), itsYAxis(0), itsZAxis(0),
		itsSL0Pos(0), itsSL1Pos(0),
		itsBslnSort(0),
		itsDataMin(0), itsDataMax(0),
		// itsAlign(0),
		itsAxisLabelling(0),
        itsParamSpectralUnit(0),
		// itsSelections(0),
		itsVisType(0), itsVisComp(0),
		itsNAvg(0),

		axisOn_(NLOCS,0),	  // (For true initial setting see initMSAR_).
		pos_(NAXES,0),
		visType_(OBSERVED),
		visComp_(AMPLITUDE),  // (reset to REAL for single-dish)
		visDev_(NORMAL),
		nDAvg_(3), nPAvg_(1),
		fieldIds_(),
		spwIds_(),

		itsMS(0), vs_(0),
		msValid_(False),
		msCols_(0),
		nFieldIds_(), nSpwIds_(),
		nChan_(),
		dish_(False),

		mssel_(0), wvi_p(0),
		msselValid_(False),
		nAnt_(1),
		msShape_(NAXES,1),
		msShapeA_(NAXES,1),
		antSort_(True),
		bLen_(),
		a1_(), a2_(), a1A_(), a2A_(), a1L_(), a2L_(),
		len2ant_(), ant2len_(),
		bsl_(), bslA_(), bslL_(),
		nbsl_(1), nbslA_(1), nbslL_(1),
		time_(1,0.), field_(1,0), scan_(1,0), fieldName_(1,""),
		spwId_(1,0), freq_(),
		nPolIds_(), nPolsIn_(), pidBase_(),
		polId_(), polName_(),

		vis_(),
		visValid_(False),
		visShape_(NAXES,0),
		visStart_(NAXES,0),
		visShapeA_(NAXES,0),
		curVisType_(INVALID_VT),
		curVisComp_(INVALID_VC),
		dataRngMin_(0.), dataRngMax_(1.),
		devRngMin_(NO_DATA), devRngMax_(),

		disp_(),
		dispX_(INVALID_AXIS), dispY_(INVALID_AXIS),
		dispPos_(NAXES,0),
		dispValid_(False),
		dispNotLoaded_(),

		dispDev_(),
		dispDevValid_(False),
		dispDevType_(INVALID_VD),
		dispDevNAvg_(),

		visDataChg_(True),
		postDataRng_(False),

		mspos_(this),

		itsFlagColor(0),
		flagsInClr_(True),
		itsUnflag(0),
		unflag_(False),
		flagAll_(NAXES,False),
		itsEntireAnt(0),
		entireAnt_(False),
		itsUndoOne(0),
		itsUndoAll(0),
		itsEditEntireMS(0),
		itsSaveEdits(0),
		flags_(),
		dispMask_(),
		dispFlags_(),
		dispFlagsInClr_(),
		dispNEdits_(),
		flagEdits_(),

		lsTime_(), leTime_(),  lsvTime_(), levTime_(),
		useVis_(), dPos_(), axlTm_(), flgdDev_(), goodData_(),
		sT_(-1), eT_(), sumv_(), sumv2_(), nValid_(), d_(),

		itsPowerScaleHandler(),		// helper objects
		itsAxisLabeller(),
		flagCM_() {

		setCS_();			// Dummy default CS (doing initialization work that
		// ActiveCaching2dDD() should have done for itself...).
		// It will be overridden in initMSAR_(), below (if we
		// get that far).

		itsMS = new MeasurementSet( msname,  TableLock(TableLock::AutoNoReadLocking), Table::Update );
		if ( itsMS == 0 )
			throw AipsError("Cannot open Measurement Set.");
		if ( itsMS->nrow() == 0u )
			throw AipsError("Measurement Set contains no data.");

		initMSAR_( ddo );
	}



//---------------------------------------------------------------------
	void MSAsRaster::initMSAR_( const viewer::DisplayDataOptions &ddo ) {
		// common constructor initialization

		// Initialize private internal colormap with colors for flagged data, etc.

		msValid_ = True;		// (DD will do nothing unless this is true).

		// Initialize private colormap (for flags, etc.).

		Vector<Float> r(NCOLORS), g(NCOLORS), b(NCOLORS);

		uInt nl=uInt(NOTLOADED), nd=uInt(NODATA),
		     of=uInt(OLDFLAG),   nf=uInt(NEWFLAG);
		// (DK: to stop a new compiler whine....  Historical note: this
		//      kind of crap went out in 1963 or so, with the automatic type
		//      conversions of FORTRAN IV.  Now the 'strong-typing' gang
		//      has brought it back for us all to enjoy again...).

		r[nl] =	  g[nl] =       b[nl] = .6f;	// not loaded (grey)
		r[nd] =	  g[nd] =       b[nd] = 0.f;	// 'no data' color (black)
		r[of] = .23f;
		g[of] = .33f;
		b[of] = .8f;	// (medium blue)
		r[nf] = .56f;
		g[nf] = .76f;
		b[nf] = .9f;	// (lighter blue)
		flagCM_.definition()->setValues(r, g, b);
		flagCM_.setRigid(True);
		flagCM_.setRigidSize(r.nelements());

		// Create MSColumns object on (unselected) itsMS, for utility purposes.
		// Save total number of field and spectral window IDs in the
		// (as-yet-unselected) MS, and provide "all fields and SpwIds"
		// as selection defaults.  Also save a Vector of the number of
		// channels in each spectral window.

		msCols_ = new ROMSColumns(*itsMS);
		nFieldIds_ = msCols_->field().nrow();
		fieldIds_.resize(nFieldIds_);
		indgen(fieldIds_);
		String chanerr = "MSAsRaster: channel conformance error in MS";
		nSpwIds_ = msCols_->dataDescription().nrow();
		if(nSpwIds_<=0) throw AipsError(chanerr);
		spwIds_.resize(nSpwIds_);
		indgen(spwIds_);

		nChan_ = msCols_->spectralWindow().numChan().getColumn();
		//Taken out because of CAS-7522
		/*if ( Int(nChan_.nelements()) < nSpwIds_ )
			throw AipsError(chanerr);*/
		if ( Int(nChan_.nelements()) > nSpwIds_ )
			nChan_.resize(nSpwIds_, True);

        chanFreq_ = msCols_->spectralWindow( ).chanFreq( ).getColumn( );
        if ( chanFreq_.shape( ).size( ) != 2 )
             throw AipsError( "frequency matrix conformance error" );
        if ( chanFreq_.shape( )(0) != max(nChan_) )
             throw AipsError( "channel frequency mismatch" );
        if ( (size_t) chanFreq_.shape( )(1) < nChan_.size( ) )
             throw AipsError( "frequency spectral window mismatch" );
        if ( (size_t) chanFreq_.shape( )(1) > nChan_.size( ) )
             chanFreq_.resize(IPosition(chanFreq_.shape( )(0),nChan_.size( )),True);

        // at some point we may need the rest refrequencies for the
        // misc sources...
        //
        //ROMSSourceColumns src(itsMS->source( ));
        //cout << "#######>>>> source ids:\n" << src.sourceId( ).getColumn( ) << endl;
        //cout << "#######>>>> rest frequencies per id for each spectral window:\n" << src.restFrequency( ).getColumn( ) << endl;
        //
        //... which produces output like:
        //
        //   #######>>>> source ids:
        //   [0, 1, 2, 3, 4, 5]
        //   #######>>>> rest frequencies per id for each spectral window:
        //   Axis Lengths: [2, 6]  (NB: Matrix in Row/Column order)
        //   [ 0, 0, 2.36945e+10, 2.36945e+10, 2.36945e+10, 2.36945e+10
        //     0, 0, 2.37226e+10, 2.37226e+10, 2.37226e+10, 2.37226e+10 ]
        //
        // where each row is the rest frequencies for each spectral window
        // and each column is the rest frequencies for a particular source
        // ---------------------------------------------------------------------------------------------
        // This table is a bit complex...
        //
        // The Source table also has a SPECTRAL_WINDOW_ID column (which is a key)
        // ..so this tells you for which SPW it is valid...a -1 there means it is
        // valid for all SPW that the source has been observed with
        //
        // There may be several rows with the same SOURCE_ID...but then SPW_ID
        // will have to be different or TIME.
        //
        // In your case i assume i guess that
        // SOURCE 0 and 1  have 2 rest frequencies associated with them and both are 0 Hz
        //
        // source 2 to 6 each have 2 specific lines rest frequencies (2.369e10
        // Hzand 2.372e10 Hz) associated with them
        // ---------------------------------------------------------------------------------------------
		const ColumnDescSet& cds = itsMS->tableDesc().columnDescSet();
		dish_ = (cds.isDefined(MS::columnName(MS::FLOAT_DATA)));
		// Set the dish_ variable according to whether we have
		// single-dish or interferometric data.


		// First-guess setting of which axes to put on display, animator, sliders.
		// Order below is for X and Y display axes, animator, sliders 0 and 1.

		Axis  dishAxes[NLOCS] = {CHAN, TIME, SP_W, POL, BASELN};   // single-dish
		Axis arrayAxes[NLOCS] = {BASELN, TIME, CHAN, POL, SP_W};   // array

		for(AxisLoc loc=0; loc<NLOCS; loc++) {
			if(dish_) axisOn_[loc] = dishAxes[loc];
			else      axisOn_[loc] = arrayAxes[loc];
		}


		if(dish_) visComp_ = REAL;	// Any 'Amplitude' user choices are silently
		// converted to REAL internally, for
		// single-dish data.  (Occasionally, you do
		// see negative values (!)).  REAL should be
		// the initial choice in this case.


		// We always create the VisSet and gather valid ranges for
		// the whole MS (and set the CS accordingly) on startup.

		selectVS_( ddo );
		vs_ = wvi_p;
		findRanges_();


		// Move degenerate axes to sliders: try to assure that display and
		// animation axes all have length greater than one.

		for(AxisLoc loc=X; loc<=SL0; loc++) {
			Axis dax = axisOn_[loc];
			if(msShape_[dax]==1) {
				//dax is degenerate
				Bool replacementFound=False;
				for(AxisLoc rloc=max(loc+1,Z); rloc<NLOCS; rloc++) {
					Axis ndax = axisOn_[rloc];
					if(msShape_[ndax]>1) {
						// ndax is non-degenerate.
						replacementFound=True;
						axisOn_[loc] = ndax;	// replace with non-degenerate axis.
						for(AxisLoc sloc=rloc+1; sloc<NLOCS; sloc++) {
							axisOn_[sloc-1] = axisOn_[sloc];
						}  // shift others up, and...
						axisOn_[NLOCS-1] = dax;	// put degen. axis on last slider.
						break;
					}
				}
				if(!replacementFound) break;
			}
		}
		// No more non-degenerate axes: no more can be done.


		constructParameters_();
		setCS_();

		// Actually extracting the data requires the user to press the 'Apply'
		// button if the MS is so large it won't all fit in memory.  This is
		// to avoid wasting the user's time extracting the wrong section of
		// the data or the wrong visibility component in cases where the
		// extraction can take on the order of minutes.

		if(msselValid_) {
			Double msSize = sizeof(Float);
			for(Axis ax=0; ax<NAXES; ax++) msSize *= msShapeA_[ax];
			Double visMemAvail = max(20, itsVisMb->value()) * 1024. * 1024.;
			if(msSize <= visMemAvail) {

				// Go ahead and retrieve the data if the whole ms's
				// visibilities will fit in available memory.
				extract_();
			}
		}

		itsMS->relinquishAutoLocks(True);	// (just to be sure).
	}



//--------------------------------------------------------------------------
	void MSAsRaster::selectVS_( const viewer::DisplayDataOptions &ddo ) {
		// Prepare the selection MS and its VisSet.
		// Can take 5-20 sec. if sorted selected reference MS does not
		// already exist.

		// Input: valid non-null itsMS (writable)
		//        MS selections (at present, fieldIds_ and spwIds_)

		Bool printwarning=True;
		undoEdits_("all", printwarning);
		// Any unsaved flagging edits will be meaningless
		// when ranges/selections change, and must be discarded.

		purgeCache();				// Also invalidate old drawings,...
		dispValid_=False;			// old display matrices,...
		visValid_ =False;			// and old vis_, For new selection.

		if ( wvi_p != 0 && wvi_p != vs_ )
			delete wvi_p;
		wvi_p = 0;				// delete old selected VisibilityIterator and

		if(mssel_!=0 && mssel_!=itsMS) {
			mssel_->relinquishAutoLocks(True);
			delete mssel_;			// ...its selected MS.
		}
		mssel_=0;

		cerr<<endl<<"Sorting... "<<flush;	// progress feedback.  (Actually,
		// we don't know whether it will have to sort or not...).


		if ( ddo.size( ) > 0 ) {
			// user selection from GUI or arguments when opening...
			mssel_ = new MeasurementSet( *itsMS );
			Vector<Vector<Slice> > chansel;
			Vector<Vector<Slice> > corrsel;
			/*bool res =*/
			mssSetData( *itsMS, *mssel_, chansel, corrsel, "",
			            ddo["time"], ddo["antenna"], ddo["field"], ddo["spw"],
			            ddo["uvrange"], ddo["msexpr"], ddo["corr"], ddo["scan"], ddo["array"] );

		} else if( Int(fieldIds_.nelements()) == nFieldIds_ &&
		           // If the entire MS has been selected,
		           // use the original MS [and VisSet] as the selected ones.

		           Int(spwIds_.nelements()) == nSpwIds_ ) {
			mssel_ = itsMS;
			if( vs_ != 0 )
				wvi_p = vs_;			// unselected vs_ already created--use that.

		} else {

			// Otherwise, create the selected MS using the vectors of selected
			// field and spectral window IDs.  (Uses the relatively-newer
			// MSSelection class).

			MSSelection msseln;

			msseln.setFieldExpr(MSSelection::indexExprStr(fieldIds_));
			msseln.setSpwExpr(MSSelection::indexExprStr(spwIds_));


			mssel_ = new MS( (*itsMS)(msseln.toTableExprNode(itsMS)) );
		}


		msselValid_ = (mssel_!=0 && mssel_->nrow()!=0u);

		if (!msselValid_) {
			cerr<<endl<<endl<<"   ***MS Selection contains no data.***"<<endl<<endl;
			vis_.resize(IPosition(5, 0));
			visStart_=0;
			visShape_=0;
			visShapeA_=0;
			// best to release any old vis_ storage.
			// Nothing will draw until selection is revised.
			return;
		}


		if( wvi_p == 0 ) {		// (True unless we're using the existing
			// unselected vs_ as vssel_).

			// Create VisSet (vssel_) for selected MS (mssel_).

			Block<int> sort(4);

			sort[0] = MS::ARRAY_ID;	// Use the 'traditional' order:
			sort[1] = MS::FIELD_ID;	// results in larger chunks, leading to
			sort[2] = MS::DATA_DESC_ID;	// more efficient data retrieval...
			sort[3] = MS::TIME;		// (4/03).

			wvi_p = new VisibilityIterator( *mssel_, sort );

			mssel_->relinquishAutoLocks(True);
		}	      // (just to be sure).

		cerr<<"Done."<<endl;
	}		// progress feedback


//--------------------------------------------------------------------------
	void MSAsRaster::findRanges_() {
		// Find the axis ranges of the (selected) MS, including msShape_ and
		// the complete sorted Vector of distinct times (time_), with corresponding
		// fields (field_), field names (fieldName_), and scan numbers (scan_).
		// Also collects information on the polIds and spwIds encountered, providing
		// translation from internal pol and spw axis indices to and from data
		// positions within the MS (nPolIds_, nPolsIn_, pidBase_, polId_, polName_;
		// spwId_, freq_).
		// Slice positions (pos_) may also be adjusted in accordance with
		// the new ranges.
		// Finding visibility _data_ ranges is much more time-consuming in general,
		// and is done only in extract_ (at some inconvenience).
		// This routine can take up to 20 sec. or so, for a million-row MS.

		// input: valid, non-null vssel_ (or, if !msselValid_, output will be set
		// to 'null' values that will only be of use for ensuring internal
		// consistency).
		// antSort_ should also be in sync with current user request before this
		// routine is called.


		// Before resetting msShape_, record whether it was all 1's previously.
		// This is a (rather sloppy) indicator that this call to findRanges_
		// is the initial one, or that the prior MS selection was empty.  This
		// affects adjustments to the pos_ variable at the end of the routine.

		Bool wasEmpty=True;
		for(Axis ax=0; ax<NAXES; ax++) if(msShape_[ax]>1) wasEmpty = False;

		if(!wasEmpty) mspos_.set(pos_);
		// Record the current slice position, in terms of actual MS
		// values (time, antennas, etc.).  This position will be restored
		// as much as possible with respect to the hypercube for the new
		// selected MS.


		if(!msselValid_) {

			// Invalid selected MS (most likely, selection contains no data).
			// It is still useful to set the normal output data of this routine
			// to values that ensure that other routines (getOptions, setOptions,
			// constructParameters_, setCS_, ...) can function properly.

			msShape_=1;		// Some routines expect the visibility shape
			msShapeA_=1;	// to be at least one pixel wide on each axis.
			pos_=0;
			if(itsVisMb!=0) itsVisMb->setDescription(visMbLabel_());
			nAnt_=1;
			computeBaselineSorts_();	// (Sets them to trivial initial values...).
			time_.resize(1);
			time_=0.;
			field_.resize(1);
			field_=0;
			scan_.resize(1);
			scan_=0;
			fieldName_.resize(1);
			fieldName_="";
			resetFreq_();
			spwId_.resize(1, True);
			spwId_=0;
			freq_.resize(1, True);
			freq_[0] = new Vector<Double>(1, 0.);
			nPolIds_=1;
			nPolsIn_.resize(nPolIds_);
			nPolsIn_=1;
			pidBase_.resize(nPolIds_);
			pidBase_=0;
			polId_.resize(1);
			polId_=0;
			polName_.resize(1);
			polName_="";

			return;
		}


		Timer tmr;
		Double pctDone = 100./max(1., Double(mssel_->nrow()));
		Bool prgShown=False;
		// For progress feedback.

		VisibilityIterator &vi(*wvi_p);

		// Retrieving many rows per iteration can save a lot of time.
		// This routine uses VisBuffer's internal table column vectors for
		// time and scan number only (12 bytes per row).  Set row blocking so
		// that up to about 1% of memory is used for these vectors.  In most
		// cases the entire chunk (all data with the same dd_ID, field and arrayID)
		// will be returned in VisIter vi's minor iteration.

		Double iterMemory = HostInfo::memoryTotal() * (.01 * 1024.);
		Int nBlkRows = Int(max(80000., min(5.e8,	// (between these values)
		                                   iterMemory/(sizeof(Double)+sizeof(Int)) )));
		vi.setRowBlocking(nBlkRows);

		VisBuffer vb(vi);


		nAnt_ = max(1u, msCols_->antenna().nrow());


		// In the single-antenna case, the 'baseline' axis is used for feeds
		// instead.  Determine its size from the feedId column of the Feed table.

		Int nFeed=1;
		Vector<Int> feedIds;
		msCols_->feed().feedId().getColumn(feedIds);
		for(uInt i=0; i<feedIds.nelements(); i++) nFeed=max(nFeed, feedIds[i]+1);


		// Set up some polID-to-pol translation data.

		String polerr = "MSAsRaster: polarization conformance error in MS";
		String chanerr = "MSAsRaster: channel conformance error in MS";
		msCols_->polarization().numCorr().getColumn(nPolsIn_, True);
		// retrieve nPolsIn_ vector -- the number of correlations in
		// each of the polarization setups (polIds).

		nPolIds_ = msCols_->polarization().nrow();
		if(Int(nPolsIn_.nelements()) != nPolIds_) throw AipsError(polerr);

		pidBase_.resize(nPolIds_);
		pidBase_ = INVALID;
		// Here, INVALID means 'No data found for this polId (or spwId) [yet]'.

		Int nPol = 0;		// Total number of pols in all polIds found
		// in the data--will become size of pol axis.

		Int nSpw=0;
		resetFreq_();
		Int maxnChan=1;

		Int szTime=1000;	// actual size of time_, field_, scan_, fieldName_...
		time_.resize(szTime);
		field_.resize(szTime);
		fieldName_.resize(szTime);
		scan_.resize(szTime);

		Int nTime=0;		// ...but only first nTime values are valid.

		Int iRow=0, iIter=0, iChunk=0;
		Double intvl=1.;		// For progress feedback.


		for (vi.originChunks(); vi.moreChunks(); vi.nextChunk() ) {

			// PolID, Sp.Win, and channel frequencies are all functions of
			// DataDescription ID, which is constant within the 'chunk'
			// (single MSIter iteration).

			Int polId = vi.polarizationId();
			if(polId<0 || polId>=nPolIds_) throw AipsError(polerr);
			Int nPolsInDataCell=vi.visibilityShape()(0);
			if(nPolsIn_[polId] != nPolsInDataCell) throw AipsError(polerr);
			if(pidBase_[polId]==INVALID) {

				// First time this polId has been encountered in the data:
				// Add its correlations to the end of our polarization axis.

				Vector<Int> stokes;
				vi.corrType(stokes);	// retrieve Stokes enums for this polId
				if(nPolsIn_[polId] != Int(stokes.nelements()))
					throw AipsError(polerr);
				pidBase_[polId] = nPol;
				// Starting point for this polId along the pol axis.

				nPol += nPolsIn_[polId];
				// increase length of our polarization axis by the number
				// of pols in the newly-encountered polId.

				polId_.resize(nPol,True);
				polName_.resize(nPol,True);
				for(Int polInId=0; polInId < nPolsIn_[polId]; polInId++) {
					Int pol = pidBase_[polId] + polInId;
					polId_[pol] = polId;
					polName_[pol] = Stokes::name(Stokes::StokesTypes(
					                                 stokes[polInId]));
				}
			}


			Int nChan=vi.visibilityShape()(1);
			maxnChan = max(nChan, maxnChan);


			Int spwid = vi.spectralWindow();
			if(spw_(spwid)==INVALID) {

				// Sp. Win. ID encountered for the first time.
				// Insert (in ID order) into table of spwIDs and their
				// corresponding channel frequencies.

				if(spwid<0 || spwid>=nSpwIds_) throw AipsError(chanerr);
				nSpw++;
				freq_.resize(nSpw, True);
				spwId_.resize(nSpw, True);

				Int spw=nSpw-1;
				for(Int sprev=spw-1;  sprev>=0;  spw--,sprev--) {
					if(spwId_[sprev]<spwid) break;	  // (correct spw found).
					freq_[spw] = freq_[sprev];	  // Move existing entries
					spwId_[spw] = spwId_[sprev];
				}   // larger than spwid up one
				// slot, to make room.

				// spw is index for new spwid.
				Vector<Double>* fr = new Vector<Double>;
				vi.frequency(*fr);	// vector of frequencies for spw/spwID
				if(Int(fr->nelements())!=nChan_[spwid]) throw AipsError(chanerr);
				freq_[spw] = fr;
				spwId_[spw] = spwid;
			}


			// Times are sorted (only) within each chunk now.
			// Reset time_ vector's search index back to the beginning
			// before gathering time information within this new chunk.

			Double oldTime=-1.e15;
			Int iTime=-1;


			for (vi.origin(); vi.more(); vi++) {

				Int nRow=vb.nRow();
				for (Int row=0; row<nRow; row++) {

					// progress feedback

					if(tmr.real()>intvl) {
						if(!prgShown) {
							cerr<<endl<<"Gathering MS Ranges: "<<flush;
							prgShown=True;
						}
						cerr<<Int((iRow+row)*pctDone)<<"%  "<<flush;
						tmr.mark();
						intvl++;
					}


					// Because of row blocking, time may now vary within the minor
					// VisIter iteration.  Therefore time slot information is
					// gathered row by row.

					Double time=vb.time()[row];
					while(oldTime<time) {
						iTime++;
						if(iTime>=nTime) break;
						oldTime=time_[iTime];
					}


					if(oldTime!=time) {

						// New time value, to be inserted at time slot iTime in the
						// sorted Vector of times (time_).

						nTime++;	//  Actual number of valid time slots found so far.

						if (nTime>szTime) {

							// Out of room: need to expand timeslot-indexed Vectors.

							szTime=Int(szTime*1.618);
							time_.resize(szTime, True);
							field_.resize(szTime, True);
							fieldName_.resize(szTime, True);
							scan_.resize(szTime, True);
						}

						// Push later times down to make room for this entry.
						// (Inefficient, but the sizes of these Vectors should make
						// a more sophisticated sort/merge unnecessary).

						for(Int i=nTime-1; i>iTime; i--) {
							time_[i]=time_[i-1];
							scan_[i]=scan_[i-1];
							field_[i]=field_[i-1];
							fieldName_[i]=fieldName_[i-1];
						}

						// Fill in the new information for this time slot.

						time_[iTime] = time;
						scan_[iTime]=vb.scan()[row];
						field_[iTime]=vb.fieldId();
						fieldName_[iTime]=vi.fieldName();

						oldTime = time;
					}
				}


				iRow+=nRow;
				iIter++;
			}
			iChunk++;
		}

		mssel_->relinquishAutoLocks(True);	 // (just to be sure).


		// Gather information for either baseline sort, including baseline lengths.

		computeBaselineSorts_();


		// Reset msShape_ according to new ranges.

		msShape_[TIME]   = nTime;
		msShape_[BASELN] = (nAnt_==1)?
		                   nFeed :	// single antenna case
		                   nbsl_;	// interferometer case (set by
		// computeBaselineSorts_()).
		msShape_[CHAN] = maxnChan;
		msShape_[POL] =  nPol;
		msShape_[SP_W] = nSpw;

		msShapeA_ = msShape_;
		if(nAnt_>1) msShapeA_[BASELN] = nbslA_;
		// Even when using length sort, msShapeA_ gives the size on the BASELN
		// axis if antenna sort _were_ being used.  vis_ is sized to the
		// latter value if possible, regardless of sort.  msShape_ and
		// msShapeA_ are always identical on the other axes (as are visShape_
		// and visShapeA_).


		if(prgShown) cerr<<"Done."<<endl;	// progress feedback.


		// Display ranges.

		cerr<<endl;
		if(msName_!="") cerr<<msName_<<endl;
		cerr<<"  Selected MS:  Time slots: "<<msShape_[TIME];

		if(nAnt_==1)   cerr<<"  Feeds: ";
		else {
			if(antSort_) cerr<<"  Baselines (incl. gaps): ";
			else         cerr<<"  Baselines: ";
		}
		cerr<<msShape_[BASELN]<<endl;

		cerr<<"  Correlations: "<<msShape_[POL]<<
		    "  Channels: "<<msShape_[CHAN]<<
		    "  Spectral Windows: "<<msShape_[SP_W]<<endl;


		// Update total size of visibilities for the memory slider's label.
		// (On the initial call to this routine from initMSAR_, itsVisMb does not
		// exist yet, and constructParameters_ initializes the label instead).

		if(itsVisMb!=0) itsVisMb->setDescription(visMbLabel_());



		// Reset current slice positions, in accordance with saved mspos_ and
		// the new ranges (real-MS positions are preserved when possible).
		// Position changes here will get sent to animators and sliders,
		// where applicable.

		pos_[TIME] = max(mspos_[TIME], 0);
		pos_[POL]  = max(mspos_[POL], 0);
		pos_[SP_W] = max(mspos_[SP_W], 0);
		// All but channel and baseline are reset to position 0
		// if the old position is no longer valid.

		// Channel number is [re]set to middle channel
		// of the current spectral window, when needed.

		mspos_.sets(pos_[SP_W]);	// (Be sure mspos_ sees the sp.win. we are
		// using; validity of channel depends on it).
		if((pos_[CHAN] = mspos_[CHAN]) == INVALID) {
			pos_[CHAN] = (nChan_[mspos_.spwId]-1)/2;
		}

		// Baseline is more usefully reset to the first non-autocorr baseline
		// (rather than 0-0), for interferometric data.

		if((pos_[BASELN] = mspos_[BASELN]) == INVALID) {
			pos_[BASELN] = (nAnt_==1 || msShape_[BASELN]<=1)? 0 :
			               antSort_? 1 : min(nAnt_, msShape_[BASELN]-1);
		}
	}
	// (dk note: this code is repeated twice more -- time to factor
	// it to a defaultPos_(Axis) method).



//---------------------------------------------------------------------
	void MSAsRaster::constructParameters_() {

		axisName_[TIME] = "Time";
		axisName_[BASELN] = (nAnt_==1)? "Feed" : "Baseline";
		//'Baseline' axis becomes Feed axis instead, when nAnt_==1 (although
		// still usually refered to in the code as BASELN, bsln, bsl, etc.).
		axisName_[CHAN] = "Channel";
		axisName_[POL]  = "Correlation";
		// 3/08: "Correlation" is a more accurate term than "Polarization" in
		// this context.  In accordance with efforts at CASA-wide uniformity
		// of terms, MSAR's external labels now use "Correlation".  Note that
		// internal variable names (and even comments) still normally use
		// 'polarization', 'POL', 'p', etc.
		axisName_[SP_W] = "Spectral Window";
		// (these are essentially static const strings...).

		Vector<String> cols = itsMS->tableDesc().columnNames();
		bool has_model = false, has_corrected = false;
		for ( size_t i = 0; i < cols.size( ); ++i ) {
			if ( cols[i] == "CORRECTED_DATA" ) has_corrected = true;
			if ( cols[i] == "MODEL_DATA" ) has_model = true;
		}
		visTypeName_(OBSERVED)="Observed";
		if ( has_model ) {
			visTypeName_(MODEL)="Model";
		}
		if ( has_corrected ) {
			visTypeName_(CORRECTED)="Corrected";
		}
		if ( has_model && has_corrected) {
			visTypeName_(RESIDUAL)="Residual";
		}
		// (later)  visTypeName_(RATIO)="Ratio";

		visCompName_(AMPLITUDE)="Amplitude";
		visCompName_(PHASE)="Phase";
		visCompName_(REAL)="Real";
		visCompName_(IMAGINARY)="Imaginary";
		visCompName_(AMPDIFF)="Amp Diff";
		visCompName_(AMPRMS)="Amp RMS";
		visCompName_(PHDIFF)="Phase Diff";
		visCompName_(PHRMS)="Phase RMS";

		itsVisType = new DParameterChoice("vistype", "visibility type",
		                                  "The visibility process stage you wish to view",
		                                  visTypeName_, visTypeName_(visType_), visTypeName_(visType_),
		                                  "ms_and_visibility_selection");

		itsVisComp = new DParameterChoice("viscomp", "visibility component",
		                                  "Real portion of the visibilities to display as color",
		                                  visCompName_, visCompName_(visComp_), visCompName_(visComp_),
		                                  "ms_and_visibility_selection");

		itsNAvg  =  new DParameterRange<Int>("navg", "average size",
		                                     "Number of cube planes to include in the average",
		                                     1, msShape_[axisOn_(Z)], 1, nPAvg_, nPAvg_, "ms_and_visibility_selection",
		                                     true, false, true );

//   itsNAvg  =  new DParameterRange<Int>("navg", "RMS/Diff Moving Average Size",
//               "The number of time slots in moving\n"
//          "averages, for visibility difference\n"
//          "or RMS displays",
//               2, 15, 1, nDAvg_, nDAvg_, "ms_and_visibility_selection");

		itsXAxis =  new DParameterChoice("xaxis", "x axis",
		                                 "Axis to show horizontally in the display",
		                                 axisName_, axisName_(axisOn_(X)), axisName_(axisOn_(X)),
		                                 "display_axes");
		itsYAxis =  new DParameterChoice("yaxis", "y axis",
		                                 "Axis to show vertically in the display",
		                                 axisName_, axisName_(axisOn_(Y)), axisName_(axisOn_(Y)),
		                                 "display_axes");
		itsZAxis =  new DParameterChoice("zaxis", "animation axis",
		                                 "Coordinate which varies during animation",
		                                 axisName_, axisName_(axisOn_(Z)), axisName_(axisOn_(Z)),
		                                 "display_axes");

		itsSL0Pos =  new DParameterRange<Int>("sl0pos", axisName_(axisOn_(SL0)),
		                                      "Varies the position (frame number) on this axis",
		                                      uiBase(), msShape_[axisOn_[SL0]]-1+uiBase(), 1,
		                                      uiBase(), pos_[axisOn_[SL0]]+uiBase(),  "display_axes");
		itsSL1Pos =  new DParameterRange<Int>("sl1pos", axisName_(axisOn_(SL1)),
		                                      "Varies the position (frame number) on this axis",
		                                      uiBase(), msShape_[axisOn_[SL1]]-1+uiBase(), 1,
		                                      uiBase(), pos_[axisOn_[SL1]]+uiBase(),  "display_axes");

		Vector<String> antLen(2);
		antLen[0]="Antenna";
		antLen[1]="Baseline Length";
		itsBslnSort = new DParameterChoice("antvslen", "baseline sort",
		                                   "Sort baselines by Antenna1-Antenna2 or by (unprojected) "
		                                   "baseline length?\n(You must save or discard pending edits "
		                                   "before changing baseline order).",
		                                   antLen, antLen[0], antLen[0],  "display_axes");


		// Default maximum vis_ size: 30% of (physical memory-60mb)
		// If physical memory cannot be retrieved from the system, get it
		// from the aipsrc parameter (system.resources.memory).
		Int minMb=5;		// Allow vis_ at least this much.
		Int physMb = HostInfo::memoryTotal()/1024;
		Int maxMb=max(minMb, physMb-60);
		Int mb =  max(minMb, Int(.3*maxMb));

		itsVisMb  =  new DParameterRange<Int>("vismb", visMbLabel_(),
		                                      "Varies the maximum memory used to hold\n"
		                                      "MS visibility data (in Megabytes).",
		                                      minMb, maxMb, 1, mb, mb, "advanced");


		// These really belong with the Scale Handler; they would have
		// to be moved from LatticeAsRaster as well...
		itsDataMin = new DParameterRange<Float>(DisplayData::DATA_MIN, "data minimum",
		                                        "Colors will be mapped between this value and Data max.",
		                                        dataRngMin_, dataRngMax_, 0.001f, dataRngMin_, dataRngMin_,
		                                        "", True, True);

		itsDataMax = new DParameterRange<Float>(DisplayData::DATA_MAX, "data maximum",
		                                        "Colors will be mapped between this value and Data min.",
		                                        dataRngMin_, dataRngMax_, 0.001f, dataRngMax_, dataRngMax_,
		                                        "", True, True);


		String aipsrcval, defaultval;
		Aipsrc::find(aipsrcval,"display.axislabels","on");
		defaultval = aipsrcval.matches(Regex(
		                                   "[ \t]*(([nN]o)|([oO]ff)|([fF](alse)*))[ \t\n]*"   ))?
		             "No" : "Yes";
		Vector<String> yesNo(2);
		yesNo[0]="No";
		yesNo[1]="Yes";
		itsAxisLabelling = new DParameterChoice("axislabelling",
		                                        "draw and label axes?",
		                                        "Should axes, axis labels and/or titles be shown?",
		                                        yesNo, defaultval, defaultval, "axis_drawing_and_labels");

		Vector<String> spectral_options;
		// spectral preferences
        spectral_options.resize(4);
        spectral_options(0) = "channel";
		spectral_options(1) = "GHz";
		spectral_options(2) = "MHz";
		spectral_options(3) = "Hz";
		itsParamSpectralUnit = new DParameterChoice( "spectralunit", "spectral unit", "", spectral_options,
                                                     spectral_options(0), spectral_options(0), "axis_units" );


		Vector<String> maskClr(2);
		maskClr(0)="Masked to Background";
		maskClr(1)="In Color";
		Int initOpt = (flagsInClr_)? 1:0;
		itsFlagColor = new DParameterChoice("flagcolor",
		                                    "show flagged regions...",
		                                    "Show flags as background (slightly faster) or highlighted with color?",
		                                    maskClr, maskClr(initOpt), maskClr(initOpt), "flagging_options");

		Vector<String> flgUnflg(2);
		flgUnflg(0)="Flag";
		flgUnflg(1)="Unflag";
		itsUnflag = new DParameterChoice("unflg",
		                                 "should new edits flag or unflag?",
		                                 "This determines whether edits you make with the mouse, "
		                                 "_after_ setting this, will flag or unflag.",
		                                 flgUnflg, flgUnflg(0), flgUnflg(0), "flagging_options");

		itsEntireAnt = new DParameterChoice("entireant",
		                                    "flag/unflag entire antenna?",
		                                    "Should subsequent edits apply to all baselines "
		                                    "which include the selected antenna[s]?",
		                                    yesNo, yesNo[0], yesNo[0], "flagging_options");

		itsUndoOne = new DParameterButton("undoone",
		                                  "undo last unsaved edit (if any)", "Undo One",
		                                  "Undo the last unsaved Flag/Unflag edit (if any).  "
		                                  "Saved edits cannot be undone, except by making the "
		                                  "reverse edits manually.", "flagging_options");

		itsUndoAll = new DParameterButton("undoall",
		                                  "undo all unsaved edits (if any)", "Undo All",
		                                  "Undo all unsaved Flag/Unflag edits (if any).  "
		                                  "Saved edits cannot be undone, except by making the "
		                                  "reverse edits manually.", "flagging_options");

		itsEditEntireMS = new DParameterChoice("entirems",
		                                       "use entire ms when saving edits?",
		                                       "Should edits apply to the enitre MS when saved?\n"
		                                       "If \"Yes\", unselected fields within an edit's\n"
		                                       "time range will be edited; unselected spectral\n"
		                                       "windows will only be edited if \"Flag/Unflag All\"\n"
		                                       "is also checked for them.",
		                                       yesNo, yesNo[1], yesNo[1], "flagging_options");

		itsSaveEdits = new DParameterButton("saveedits",
		                                    "save edits to disk", "Save Edits",
		                                    "Save Flag/Unflag edits permanently to the MS.  "
		                                    "You must save your edits or they will be discarded.  "
		                                    "After saving, the only way to undo is to make the "
		                                    "reverse edits manually.", "flagging_options");
	}


//--------------------------------------------------------------------------
	DisplayCoordinateSystem MSAsRaster::setCS_() {
		// Update/set the (2d--canvas) coordinate system and extents.
		// This gets set onto the WC before drawing
		// (via ActiveCaching2dDD::sizeControl) to define the CS of the
		// WorldCanvas.  Causes a zoom to the new axis extents, too.

		// We want nice world coordinates (for axis drawing/labelling/tracking)
		// eventually.  For now, world coordinates are simply the same as
		// the data pixel coordinates.  I.e., the CS is an identity
		// LinearCoordinate transformation (with translation from 0- to 1-basing
		// when desired).
		// 'World Coordinates' for baselines are an exception to this.  A
		// TabularCoordinate is used there so that, e.g, the baseline between
		// antennas 11 and 23 might have world coordinate '11023'.  (The axis
		// labeller may in turn rather lamely label this as '1.1023*10^4).
		// That scheme was inherited from msplot, but should probably be
		// pitched, since the resulting labels are generally just confusing
		// (the tracking display is clearer; it doesn't make use of the CS).

        std::vector<Axis> ax(2);
        ax[0] = axisOn_(X);
        ax[1] = axisOn_(Y);
		DisplayCoordinateSystem cs;

		Vector<Double> linblc(2), lintrc(2);  // new max zoom extents.

		// Data pixel indices go from 0 to msShape-1, and as coordinates
		// these indices are identified with the data pixel centers.  Total
		// extent of the data is considered to be the coordinates of the
		// boundary pixels' outer edges, i.e., .5 pixels beyond their centers.

		linblc=-.5;
		lintrc(X) = msShape_[ax[0]]-1 + .5;
		lintrc(Y) = msShape_[ax[1]]-1 + .5;

		// For now, there is little axis labelling in world terms.
		// Except for baselines, only the pixel indices of the the data
		// are shown on the axis scales.
		// Internally, these coordinates are zero-based (i.e., they begin
		// numbering at zero).  Position tracking and axis labelling must
		// display pixel coordinates as either 1-based or 0-based for the
		// user, depending on the value of uiBase().  Hence, refpix 0 is
		// identified with refvalue (world coordinate value) equal to uiBase(),
		// in the simple linear coordinates below.

		Int nlc = ( nAnt_ != 1 && (ax[0] == BASELN || ax[1] == BASELN) || 
                    freqAxis(ax[0]) || freqAxis(ax[1]) ) ? 1 : 2;
		// The Linear coordinate will cover just one axis if
		// a Tabular coordinate (below) is used for baseline numbering
		// on the other axis; otherwise the Linear Coordinate will
		// be used for both axes.

		LinearCoordinate lc(nlc);
		Vector<Double> refval(nlc);
		refval=Double(uiBase());
		lc.setReferenceValue(refval);

        
		if( nAnt_ == 1 )
            cs.addCoordinate(lc);	// (no baselines, just feeds).

		else {

            for ( int x=0; x < 2; ++x ) {
                if ( freqAxis(ax[x]) ) {
                    SpectralCoordinate spec( MFrequency::DEFAULT, Vector<double>(chanFreq_(ipos(2,0,mspos_.spwId),ipos(2,chanFreq_.shape( )(0)-1,mspos_.spwId))) );
                    cs.addCoordinate(spec);

                } else if( ax[x] == BASELN ) {

                    Vector<Double>pix, wld;

                    if(antSort_) {

                          // Add Tab Coord for baseline-index-to-antenna-number conversion.
                          // There are interpolation points just inside both ends of
                          // the 'shelf' corresponding to each value of ant1. (plus one extra
                          // 'shelf' on the left, for good luck).

                          pix.resize(2*nAnt_ + 3), wld.resize(2*nAnt_ + 2);
                          for(Int a1=-1; a1<nAnt_+1; a1++) pix[2*a1+2] = bsln_(a1,a1)-.5;
                          for(Int a1=-1; a1<nAnt_; a1++)   pix[2*a1+3] = pix[2*a1+4]-.0001;
                          pix.resize(2*nAnt_+2, True);
                          for(Int i=0; i<2*nAnt_+2; i++) wld[i]=a1a2_(pix[i], uiBase());
                    }
                    // (uiBase() indicates whether antenna numbers (in this case),
                    // channels, baseline indices, etc. should be displayed to the
                    // user numbered from 0 or from 1.  They are always numbered
                    // from zero 'internally').

                    else {

                        // This TabularCoordinate will show baseline lengths in meters.

                        pix.resize(nbsl_), wld.resize(nbsl_);
                        for(Int b=0; b<nbsl_; b++) {
                            pix(b)=b;
                            wld(b) = bLen_(a1_(b), a2_(b));
                            if(b>0 && wld(b)<=wld(b-1)) wld(b) = wld(b-1) + 1.e-6;
                        }
                    }
                    // TabCoords require monotonically increasing[/decreasing]
                    // conversions.  (Unfortunately, it is not possible at present
                    // to produce ideal baseline labelling (for either sort) with
                    // the current labelling and coordinate capabilities).

                    TabularCoordinate tc(pix,wld,"","");
                    cs.addCoordinate(tc);

                } else if ( cs.nWorldAxes( ) < 2 ) {
                     cs.addCoordinate(lc);
                }

            }
        }

		Vector<String> daxisnames(2);
		daxisnames(X) = axisName_(ax[0]);
		daxisnames(Y) = axisName_(ax[1]);

		if(!antSort_) {
			if(ax[0]==BASELN) daxisnames(X) += " (m)";	  // (Unit is meters
			if(ax[1]==BASELN) daxisnames(Y) += " (m)";
		}   //  in this case).

		cs.setWorldAxisNames(daxisnames);

		// ActiveCachingDD will propagate this CS to the WC.  (itsAxisLabeller
		// finds it there for use in axis labelling).
		setCoordinateSystem(cs, linblc, lintrc);

		// This causes a zoom-out to the extent of the data on the new display axes,
		// on all WCs where this DD is active, upon their next refresh.
		Attribute zoomToExtent("resetCoordinates", True);
		setAttributeOnPrimaryWCHs(zoomToExtent);

		if(!msValid_) return cs;	// In this case, the object is useless.
		// Protect public methods from crashes.

		// These help ensure that the axis labeller uses only the correct
		// cached axis drawings.
		AttributeBuffer restrctns;
		Vector<String> axname = itsCoordinateSystem.worldAxisNames();
		restrctns.set("msar xaxisname", axname(X));
		restrctns.set("msar yaxisname", axname(Y));
		restrctns.set("msar msname", itsMS->tableName());
        restrctns.set("msar spectralunit", itsParamSpectralUnit->value( ));
		itsMS->relinquishAutoLocks(True);	 // (just to be sure).
		itsAxisLabeller.setRestrictions(restrctns);
        return cs;
	}


//---------------------------------------------------------------------
	Record MSAsRaster::getOptions( bool scrub ) const {
		// getOptions() returns a record of the user-adjustable display
		// parameters for this object.  It includes initial values, ranges,
		// interface element types, help and label texts; in short, all the
		// data and meta-data needed to construct a user interface window to
		// control the object.  The corresponding setOptions method can be
		// called directly or via a user interface window to change these
		// control parameters.  Many DL objects (not just DDs) use this
		// scheme as their main control interface.  Carefully examining the
		// format of the record returned by this method (and comparing it to
		// the corresponding 'adjust' or 'options' window, if available) is the
		// easiest way of understanding its format.
		//
		// Note: If the user interface is to be zero-based, you should call
		// setUIBase(0) _before_ using this routine -- see setUIBase().

		// get base class options.
		Record rec = ActiveCaching2dDD::getOptions(scrub);

		// Remove a few unneeded ActiveCaching2dDD options.
		// This will prevent their showing up in Adjust gui's.
		// The defaults (absolute whole-pixel coordinates) will do nicely.
		// Both pixel and world coordinates are tracked now.
		// We can leave Aspect ratio on 'flexible', too.
		if(rec.isDefined("absrel")) rec.removeField("absrel");
		if(rec.isDefined("fracpix")) rec.removeField("fracpix");
		if(rec.isDefined("aspect")) rec.removeField("aspect");
		if(rec.isDefined("pixelworld")) rec.removeField("pixelworld");

		if(!msValid_) return rec;	// In this case, the object is useless.
		// Protect public methods from crashes.

		// MSAsRaster options



		// 'Advanced' rollup.

		itsVisMb->toRecord(rec);



		// 'MS and Visibility Selection' rollup

		itsVisType->toRecord(rec);
		itsVisComp->toRecord(rec);
		itsNAvg->toRecord(rec);

		// (no DParameters for numeric array user entry boxes yet--do it
		// the old-fashioned longhand way for now).

		// Field IDs entry box.

		Record fieldids;
		fieldids.define("dlformat", "fieldids");
		fieldids.define("listname", "field ids");
		fieldids.define("help", "Enter a set of (integer) field IDs,\n"
		                "to select data only for those fields.\n"
		                "Separate with spaces or commas.  Leave\n"
		                "unset (wrench menu) to select all fields.\n"
		                "Press 'Apply' to retrieve selected data." );
		fieldids.define("ptype", "array");
		Vector<Int> allFieldIds(nFieldIds_);
		indgen(allFieldIds, uiBase());
		fieldids.define("default",allFieldIds);
		fieldids.define("value", Vector<Int>(fieldIds_+uiBase()));
		fieldids.define("allowunset", True);
		fieldids.define("context", "ms_and_visibility_selection");
		rec.defineRecord("fieldids", fieldids);

		// Spectral Window IDs entry box.

		Record spwids;
		spwids.define("dlformat", "spwids");
		spwids.define("listname", "spectral windows");
		spwids.define("help", "Enter a set of (integer) spectral\n"
		              "window ID numbers, to select data only\n"
		              "for those spectral windows.  Separate\n"
		              "with spaces or commas.  Leave unset\n"
		              "(wrench menu) to select all spectral windows.\n"
		              "Press 'Apply' to retrieve selected data." );
		spwids.define("ptype", "array");
		Vector<Int> allSpwIds(nSpwIds_);
		indgen(allSpwIds, uiBase());
		spwids.define("default",allSpwIds);
		spwids.define("value", Vector<Int>(spwIds_+uiBase()));
		spwids.define("allowunset", True);
		spwids.define("context", "ms_and_visibility_selection");
		rec.defineRecord("spwids", spwids);



		// 'Display Axes' rollup.

		itsXAxis->toRecord(rec);
		itsYAxis->toRecord(rec);
		itsZAxis->toRecord(rec);

		// Be sure slider axis position interface reflects latest uiBase() value.
		// uiBase() determines external numbering: from either 0 or 1; internal
		// numbering is always from 0.

		itsSL0Pos->setMinimum(uiBase());
		itsSL0Pos->setMaximum(msShape_[axisOn_[SL0]] - 1 + uiBase());
		itsSL0Pos->setDefaultValue(uiBase());
		itsSL0Pos->setValue(pos_[axisOn_[SL0]] + uiBase());

		itsSL1Pos->setMinimum(uiBase());
		itsSL1Pos->setMaximum(msShape_[axisOn_[SL1]] - 1 + uiBase());
		itsSL1Pos->setDefaultValue(uiBase());
		itsSL1Pos->setValue(pos_[axisOn_[SL1]] + uiBase());

		itsSL0Pos->toRecord(rec);
		itsSL1Pos->toRecord(rec);

		if(nAnt_>1) itsBslnSort->toRecord(rec);
		// (Irrelevant for single dish -- won't show in that case).



		// 'Flagging Options' rollup.

		itsFlagColor->toRecord(rec);
		itsUnflag->toRecord(rec);

		// Flagging extent checkboxes (also to be replaced by a DParameter).

		Vector<String> axisNames(NAXES);
		for(Axis ax=0; ax<NAXES; ax++) axisNames(ax) = axisName_(ax)+String("s");
		// (form plurals of axis names for this use).
		Record flgall;
		flgall.define("dlformat", "flgall");
		flgall.define("listname", "flag/unflag all...");
		flgall.define("help", "Check these boxes to extend edits over\n"
		              "the entire range of the corresponding axes.\n"
		              "(These will apply (only) to edits made\n"
		              "after the checkboxes are set)." );
		flgall.define("ptype", "check");
		flgall.define("default", "");
		flgall.define("nperline", 2);
		flgall.define("popt", axisNames);

		// (send out current flag checkbox values, in the form needed).

		Int nChkd=0;
		for(Axis ax=0; ax<NAXES; ax++) if(flagAll_[ax]) nChkd++;
		Vector<String> chkdAxes(nChkd);
		Int iChkd=0;
		for(Axis ax=0; ax<NAXES; ax++) {
			if(flagAll_[ax]) chkdAxes[iChkd++] = axisNames[ax];
		}
		flgall.define("value", chkdAxes);


		flgall.define("allowunset", False);
		flgall.define("context", "flagging_options");
		rec.defineRecord("flgall", flgall);


		itsEntireAnt->toRecord(rec);
		itsUndoOne->toRecord(rec);
		itsUndoAll->toRecord(rec);
		itsEditEntireMS->toRecord(rec);
		itsSaveEdits->toRecord(rec);



		// 'Basic settings' rollup.
		((MSAsRaster*)this)->postDataRng_ = False;    // this seems like a hack that should go away...
		                                              // <drs:Thu Dec 19 13:27:39 EST 2013>

		// postDataRng_ is a signal from extract_ to send itsDataMin/Max
		// out to the GUI.  This is being done now.
		itsDataMin->toRecord(rec);
		itsDataMax->toRecord(rec);

		// from attached data scaler

		rec.merge(itsPowerScaleHandler.getOptions(scrub));
		if(rec.isDefined("histoequalisation")) rec.removeField("histoequalisation");
		// (not fond of this option either...)



		// 'Axis Drawing and Labels' rollup--from attached axis labeller

		itsAxisLabelling->toRecord(rec);
		rec.merge(itsAxisLabeller.getOptions(), Record::SkipDuplicates);
		// SkipDuplicates, to tie AxisLabeller's CDD Params (such as Cache
		// Size) to those of MSAsRaster (with single gui control for both).
		if(rec.isDefined("spectralunit")) rec.removeField("spectralunit");
        itsParamSpectralUnit->toRecord(rec);

		if(rec.isDefined("velocitytype")) rec.removeField("velocitytype");
		// Flexible labelling of frequency axis is not available in
		// MSAsRaster yet.  Prevent these user options from appearing
		// in the gui.
		return rec;
	}


//---------------------------------------------------------------------------
	Bool MSAsRaster::setOptions(Record &rec, Record &recOut) {
		// Process record of current user options.
		// All MSAsRaster activity in response to 'Adjust' panel input is triggered
		// through this routine.  Hence much of the control logic is here too:
		// comparing user input changes with what is currently computed/displayed,
		// and requesting new computation/display as needed.

		// The return value indicates whether refresh is needed.  A change to almost
		// any option will require refresh (but not quite all; e.g., flagging
		// extent checkboxes and cache size do not affect drawing).
		// recOut contains options which MSAsRaster itself wishes to update on
		// the adjust panel or animator, e.g. axes or data ranges.  (Currently
		// only the values, ranges and labels for certain controls can be
		// changed, and the only time they can be changed is on exit from
		// this routine).  Most options (i.e., settings for the DD) are stored
		// internally as DisplayParameters (which package both the value and
		// related gui information).

		// Most of the MSAsRaster control logic is split between this routine and
		// draw_().

		if(!msValid_) return False;	// In this case, the object is useless.
		// Protect public methods from crashes.

		static const Bool wholeRecord=True, overwrite=True;
		// (settings for update of gui from Parameters via recOut)

		// Fetch this parameter before sending out possible changes to its
		// description.  Changes to this one do not require refresh.
		itsVisMb->fromRecord(rec);


		Bool needsRefresh=False;


		// Do any requested AXIS CHANGES first.

		// This code sets the (important) internal axisOn_ Vector, in sync with the
		// input options settings from the its{X,Y,Z}Axis DParameters.  axisOn_
		// records which data axes are displayed, which put on the animator or
		// sliders.

		// In the usual case, the user will change only one axis at a time to
		// trigger this setOptions call.  The code below automatically swaps
		// with the old location of the requested axis, guaranteeing that
		// axisOn_ is always a permutation of (TIME, BASELN, CHAN, POL, SP_W),
		// corresponding to the axis to display or control on the axis locations
		// (X, Y, Z, SL0, and SL1, in that order).


		// save old axisOn_ state, and control axis ranges and positions,
		// in order to determine and respond to changes to them.
		// After axis and range setting, recOut will be returned to notify the
		// user interface elements of additional changes needed there. if any.

		Axis oldAxX = axisOn_[X],   oldAxY = axisOn_[Y],   oldAxZ=axisOn_[Z],
		     oldAxSL0=axisOn_[SL0], oldAxSL1=axisOn_[SL1];

		Int  oldPosZ    = pos_[oldAxZ],
		     oldPosSL0  = pos_[oldAxSL0],
		     oldPosSL1  = pos_[oldAxSL1];

		Int  oldRngZ    = msShape_[oldAxZ],
		     oldRngSL0  = msShape_[oldAxSL0],
		     oldRngSL1  = msShape_[oldAxSL1];


		Block<Int> uiAxisOn(3);
		Block<Bool> swapAxes(3, False);

		Bool axisChg = False;

		itsXAxis->fromRecord(rec);
		itsYAxis->fromRecord(rec);
		itsZAxis->fromRecord(rec);
		uiAxisOn[X]=axisNum_(itsXAxis->value());	// uiAxisOn are the axes
		uiAxisOn[Y]=axisNum_(itsYAxis->value());	// currently set on the user
		uiAxisOn[Z]=axisNum_(itsZAxis->value());	// interface choice boxes.
		// These may require further changes as a result of
		// swapping axes.

		for(AxisLoc loc=X; loc<=Z; loc++) {
			Axis oldax=axisOn_[loc], uiax=uiAxisOn[loc];
			if(uiax!=oldax) {

				// An axis has changed.

				swapAxes[loc] = axisChg = True;
				for(AxisLoc prevloc=X; prevloc<loc; prevloc++) {
					if(swapAxes[prevloc] && uiAxisOn[prevloc]==uiax) {
						swapAxes[loc] = False;
						break;
					}
				}
			}
		}
		// swapAxes will be True where user has requested a change
		// to an axis (unless he has tried to change two locations
		// to hold the same axis--unlikely).

		if(axisChg) {
			needsRefresh = True;

			for(AxisLoc newloc=X; newloc<=Z; newloc++) if(swapAxes[newloc]) {
					Axis oldax=axisOn_[newloc], newax=uiAxisOn[newloc];
					AxisLoc oldloc=X;
					while(axisOn_[oldloc]!=newax) oldloc++;
					axisOn_[oldloc]=oldax;
					axisOn_[newloc]=newax;
				}
			// Swap axes between new and old locations.

			if(uiAxisOn[X]!=axisOn_[X]) {
				itsXAxis->setValue(axisName_[axisOn_[X]]);
				itsXAxis->toRecord(recOut, wholeRecord, overwrite);
			}
			if(uiAxisOn[Y]!=axisOn_[Y]) {
				itsYAxis->setValue(axisName_[axisOn_[Y]]);
				itsYAxis->toRecord(recOut, wholeRecord, overwrite);
			}
			if(uiAxisOn[Z]!=axisOn_[Z]) {
				itsZAxis->setValue(axisName_[axisOn_[Z]]);
				itsZAxis->toRecord(recOut, wholeRecord, overwrite);
			}
		}
		// Communicate any extra changes back to user interface.


		Axis axX = axisOn_[X],   axY = axisOn_[Y],   axZ=axisOn_[Z],
		     axSL0=axisOn_[SL0], axSL1=axisOn_[SL1];
		// shorthand for the new axes on display/controls.


		Bool dspAxisChg = (axX!=oldAxX || axY!=oldAxY),
		     axZChg     = (axZ != oldAxZ),
		     axSL0Chg   = (axSL0 != oldAxSL0),
		     axSL1Chg   = (axSL1 != oldAxSL1);
		// control variables based on axis changes.




		// MS DATA SELECTION

		Bool newRanges = False;	// will indicate MS selection change.
		Bool notfound;
		Bool fieldIdsChg = False,  spwIdsChg = False;
		// indicates user _desired_ a selection change.


		// This is just minor massaging of the field IDs user input vector
		// (such as removing dups or invalid Ids, and substituting all IDs
		// for an unset entry.  Sure takes a lot of code, for some reason...).

		Vector<Int> newFieldIds(nFieldIds_);
		indgen(newFieldIds, uiBase());
		// Default new field selection ('all fields', used for 'unset').

		Vector<Int> inputFieldIds;
		readOptionRecord(inputFieldIds, notfound, rec, "fieldids");

		if(notfound) {	// must check separately for single ID (dumb).
			Int inputFieldId;
			readOptionRecord(inputFieldId, notfound, rec, "fieldids");
			if(!notfound) {
				inputFieldIds.resize(1);
				inputFieldIds = inputFieldId;
			}
		}

		if(!notfound) {

			// Remove invalid and duplicate field IDs from input and sort them
			// (i.e., just check all possible valid Ids in order, to see which are
			// included in the input vector).

			Int nInputFlds = inputFieldIds.nelements();
			Int nNewFlds=0;	// number of distinct valid field IDs found in input.

			for(Int f = uiBase(); f < nFieldIds_+uiBase(); f++) {
				for(Int i=0; i<nInputFlds; i++) if(f==inputFieldIds[i]) {
						newFieldIds[nNewFlds++] = f;
						break;
					}
			}

			if(nNewFlds==0) {	// no (valid) fields entered -- reset to all fields.
				newFieldIds.resize(nFieldIds_);
				indgen(newFieldIds, uiBase());
			}

			else newFieldIds.resize(nNewFlds, True);
		}

		else {

			// Must also check separately for fieldids 'unset'  (also dumb).
			// (if it is unset, notfound will be set False; all fields
			// are selected in that case).

			String dummy;
			Bool unset;
			readOptionRecord(dummy, unset, notfound, rec, "fieldids");
		}


		if(!notfound) {

			// 'fieldids' field was found in the setOptions record.
			// newFieldIds now contains the desired new field values, in order.

			Int nNewFlds = newFieldIds.nelements();
			fieldIdsChg = nNewFlds!=Int(fieldIds_.nelements());
			if(!fieldIdsChg) {
				for(Int i=0; i<nNewFlds; i++) {
					if(newFieldIds[i] != fieldIds_[i] + uiBase()) {
						// newFieldIds (user input) are numbered from uiBase (0 or 1);
						// fieldIds_ are numbered from 0.
						fieldIdsChg=True;
						break;
					}
				}
			}

			if(fieldIdsChg) {

				// There has been a change to desired field IDs.

				// Changes to MS selections are not allowed until pending edits
				// have been saved (or undone).  This is because the edits are
				// stored in terms of indices relative to msShape_.  The correspondence
				// between, e.g., time slot index and times as stored in the MS
				// is set up in findRanges_, and changes when the MS selection does.

				if(flagEdits_.len()==0u) {

					// OK to change MS selections.

					fieldIds_.assign(newFieldIds - uiBase());
					// (user input may be 0- or 1-based, fieldIds_ is 0-based).
					newRanges = True;
				}

				else newFieldIds.assign(fieldIds_+uiBase());
			}
			// change not allowed: revert to old selection
			// (user is warned below).

			Record fieldids;
			fieldids.define("value", newFieldIds);
			fieldids.define("ptype", "array");
			recOut.defineRecord("fieldids", fieldids);
		}
		// gui feedback, to make sure user sees massaged entry.




		// Go through same rigamarole for spectral window ID selection.

		Vector<Int> newSpwIds(nSpwIds_);
		indgen(newSpwIds, uiBase());

		Vector<Int> inputSpwIds;
		readOptionRecord(inputSpwIds, notfound, rec, "spwids");

		if(notfound) {
			Int inputSpwId;
			readOptionRecord(inputSpwId, notfound, rec, "spwids");
			if(!notfound) {
				inputSpwIds.resize(1);
				inputSpwIds = inputSpwId;
			}
		}

		if(!notfound) {
			Int nInputSpws = inputSpwIds.nelements();
			Int nNewSpws=0;

			for(Int s=uiBase(); s<nSpwIds_+uiBase(); s++) {
				for(Int i=0; i<nInputSpws; i++) if(s==inputSpwIds[i]) {
						newSpwIds[nNewSpws++] = s;
						break;
					}
			}

			if(nNewSpws==0) {
				newSpwIds.resize(nSpwIds_);
				indgen(newSpwIds, uiBase());
			} else newSpwIds.resize(nNewSpws, True);
		}

		else {
			String dummy;
			Bool unset;
			readOptionRecord(dummy, unset, notfound, rec, "spwids");
		}


		if(!notfound) {

			Int nNewSpws = newSpwIds.nelements();
			spwIdsChg = nNewSpws!=Int(spwIds_.nelements());
			if(!spwIdsChg) {
				for(Int i=0; i<nNewSpws; i++) {
					if(newSpwIds[i] != spwIds_[i]+uiBase()) {
						spwIdsChg=True;
						break;
					}
				}
			}

			if(spwIdsChg) {

				if(flagEdits_.len()==0u) {

					spwIds_.assign(newSpwIds - uiBase());
					newRanges = True;
				}

				else newSpwIds.assign(spwIds_+uiBase());
			}

			Record spwids;
			spwids.define("value", newSpwIds);
			spwids.define("ptype", "array");
			recOut.defineRecord("spwids", spwids);
		}


		if((fieldIdsChg || spwIdsChg) && !newRanges) {
			cerr<<endl<<"  ***You must save (or undo) edits before changing "
			    "MS selections.***"<<endl<<endl;
		}
		// Warn user if their MS selection changes were rejected.



		// Determine whether a CHANGE IN BASELINE SORT is requested and
		// allowed (bslSortChg).  If so, antSort_ is set accordingly.  New
		// antenna-to-baseline-index translation Arrays (a1_, a2_, bsl_) and
		// related state (msShape_[BASELN]) will be set below (by
		// setBslSort_(), either directly or via findRanges_()).
		// Changing the sort will also require either resorting vis_ (via
		// reSortVis_()) or retrieving its visibilities afresh from disk (via
		// extract_()).

		Bool bslSortChg = False;	// change both requested _and_ allowed.
		String oldsort = itsBslnSort->value();

		itsBslnSort->fromRecord(rec);
		Bool newsort = (itsBslnSort->value() == "Antenna");

		if(newsort != antSort_) {

			// The user wants to change the sort.

			if(flagEdits_.len()>0u || nAnt_==1) {	// Change not allowed.

				if(nAnt_==1) cerr<<endl<<"  ***Baseline sort change is not allowed "
					                 "(or useful) for single dish data.***"<<endl<<endl;
				else        cerr<<endl<<"  ***You must save (or undo) edits before "
					                "changing baseline sort.***"<<endl<<endl;
				itsBslnSort->setValue(oldsort);
				itsBslnSort->toRecord(recOut, wholeRecord, overwrite);
			}

			else {
				antSort_ = newsort;	// Register change (will be acted on below).
				bslSortChg = True;
			}
		}




		// RETRIEVE any new SLIDER VALUES from the user interface (although
		// they may be overridden below during range gathering).

		// The sliders and animator can move through the entire selected
		// MS range.  However, the display area will be grey if they are
		// moved beyond the range of vis_, until 'Apply' is pressed.  This
		// allows the controls to be moved all the way to the desired location
		// before any lengthy data extraction is performed.

		// (These positions are for the axes the sliders controlled _at the
		// start of the call_ (i.e., as the user saw them), in case (unlikely)
		// the axis controlled by a slider has also changed during this call).

		itsSL0Pos->fromRecord(rec);
		pos_[oldAxSL0] = itsSL0Pos->value() - uiBase();
		// pos_ is 0-based; slider is numbered either from 0 or 1 (uiBase()).
		itsSL1Pos->fromRecord(rec);
		pos_[oldAxSL1] = itsSL1Pos->value() - uiBase();

		// Also RETRIEVE the 'true, original' ANIMATOR POSITION, when necessary:
		// either when moving the axis off the animator (possibly to a slider--note
		// use of the axis which _used to be_ on Z), or to decide whether animator
		// resetting is necessary after a change to MS selection or baseline sort.
		// (Conceptually simpler, perhaps better, would have been simply to do this
		// unconditionally above, where oldPosZ was originally initialized.  It was
		// avoided on (somewhat bogus) grounds that it might be a bit expensive).
		// Recall that _normal_ animator operation by the user to change
		// frames does _not_ pass through this setOptions() routine.
		if(axZChg || newRanges || oldAxZ==BASELN && bslSortChg) {
			getFirstZIndex(pos_[oldAxZ], oldRngZ);
			oldPosZ = pos_[oldAxZ];
		}



		// Changes to the MS data selections require us to
		// REBUILD THE VISSET and RE-GATHER THE AXIS RANGES.
		// This work (which can take a little time on a large MS) is done here,
		// so that any new axis ranges may be returned to the user interface.

		if(newRanges) {


			selectVS_();
			findRanges_();


			// This posts total MS visibility size to this slider's description.
			itsVisMb->toRecord(recOut, wholeRecord, overwrite);
			needsRefresh = True;
		}


		else if(bslSortChg) {

			// Baseline sort changed, but no need to recompute msShape_ completely.
			// Just adjust the new baseline order information, and msShape_ and pos_
			// on the baseline axis.

			needsRefresh = True;

			mspos_.setb(pos_[BASELN]);	// Save current baseline position (still
			// in terms of old sort) in mspos_
			// (i.e., as antenna numbers).

			setBslSort_();	// This establishes the new order mapping between
			// ant1-ant2 and baseline position indices.

			if(nAnt_>1) msShape_[BASELN] = nbsl_;

			pos_[BASELN] = mspos_[BASELN];	// Restore current baseline position
			// index (under new sort order).

			if(pos_[BASELN] == INVALID) {	// (We were probably on a 'gap').
				// Baseline position is more usefully reset in this case to the first
				// non-autocorr baseline (rather than 0-0), for interferometric data.

				pos_[BASELN] = (nAnt_==1 || msShape_[BASELN]<=1)? 0 :
				               antSort_? 1 : min(nAnt_, msShape_[BASELN]-1);
			}
		}



		pos_[axZ]   = max( 0, min(msShape_[axZ]-1,    pos_[axZ]) );
		pos_[axSL0] = max( 0, min(msShape_[axSL0]-1,  pos_[axSL0]) );
		pos_[axSL1] = max( 0, min(msShape_[axSL1]-1,  pos_[axSL1]) );
		// Just be sure final slice control positions
		// are in range (they should be, already).


		// msShape_, baseline sort state, and pos_ are up-to-date for
		// user's desired new settings at this point.


		Bool rngZChg   = (msShape_[axZ]   != oldRngZ);
		Bool rngSL0Chg = (msShape_[axSL0] != oldRngSL0);
		Bool rngSL1Chg = (msShape_[axSL1] != oldRngSL1);
		// More control variables, based on range changes.



		// Changes to X or Y axes or to the axis ranges (or to the baseline order,
		// when baseline is on display) require  RESETTING THE COORDINATE SYSTEM.

		if( dspAxisChg || newRanges ||
		        (bslSortChg && (axX==BASELN || axY==BASELN)) ) setCS_();




		// SEND UPDATED SLICE POSITIONS AND RANGES OUT TO ANIMATOR AND SLIDER
		// CONTROLS, synchronizing the interface with final values of the pos_
		// and msShape_ variables.

		// Changes to the animation axis, or (because of changes to MS selections)
		// to its range/position, require resetting of the animator[s]

		if( isCSmaster() &&
		        (axZChg || rngZChg || pos_[axZ]!=oldPosZ &&
		         (newRanges || bslSortChg)) ) {
			Record setanimrec;
			setanimrec.define("zindex", pos_[axZ]);
			recOut.defineRecord("setanimator", setanimrec);
		}
		// Note: this is not a change to an autogui parameter, but a request
		// to set the animator position.  It will also result in the number
		// of animator frames being set to nelements() (==msShape_[axZ]).


		if(pos_[axSL0]!=oldPosSL0 || pos_[axSL1]!=oldPosSL1) needsRefresh = True;
		// Refresh if pos_ differs from its original value on slider axes.

		// If the final values for the slider's label, range or value differ
		// from that currently on the user interface, send out updates.
		// (Slightly different from line above: testing for changes in
		// values on controls, rather than positions on axes...).

		Bool posSL0Chg = (pos_[axSL0]+uiBase() != itsSL0Pos->value());
		if(axSL0Chg)  itsSL0Pos->setDescription(axisName_(axSL0));
		if(rngSL0Chg) itsSL0Pos->setMinimum(uiBase());
		if(rngSL0Chg) itsSL0Pos->setMaximum(msShape_[axSL0]-1+uiBase());
		if(posSL0Chg) itsSL0Pos->setValue(pos_[axSL0] + uiBase());
		if(axSL0Chg || rngSL0Chg || posSL0Chg) {
			itsSL0Pos->toRecord(recOut, wholeRecord, overwrite);
		}

		Bool posSL1Chg = (pos_[axSL1]+uiBase() != itsSL1Pos->value());
		if(axSL1Chg)  itsSL1Pos->setDescription(axisName_(axSL1));
		if(rngSL1Chg) itsSL1Pos->setMinimum(uiBase());
		if(rngSL1Chg) itsSL1Pos->setMaximum(msShape_[axSL1]-1+uiBase());
		if(posSL1Chg) itsSL1Pos->setValue(pos_[axSL1] + uiBase());
		if(axSL1Chg || rngSL1Chg || posSL1Chg) {
			itsSL1Pos->toRecord(recOut, wholeRecord, overwrite);
		}




		// VISIBILITY TYPE, COMPONENT AND DEVIATION MODE.

		// Record whether we were in a deviation display mode prior to
		// this setOptions call.

		Bool wasDev = (visDev_>NORMAL);

		needsRefresh = itsVisType->fromRecord(rec) || needsRefresh;
		needsRefresh = itsVisComp->fromRecord(rec) || needsRefresh;
		visType_ = visTypeNum_(itsVisType->value());  // (Keep enums in sync)
		visComp_ = visCompNum_(itsVisComp->value());

		// Parse GUI choice box (itsVisComp) into visComp_ and visDev_.

		bool avgrf = False;
		switch(visComp_) {
		case AMPDIFF: {
			visComp_ = AMPLITUDE;
			avgrf = adjustAvgRange( DIFF, recOut );
			visDev_ = DIFF;
			break;
		}
		case AMPRMS:  {
			visComp_ = AMPLITUDE;
			avgrf = adjustAvgRange( RMS, recOut );
			visDev_ = RMS;
			break;
		}
		case PHDIFF:  {
			visComp_ = PHASE;
			avgrf = adjustAvgRange( DIFF, recOut );
			visDev_ = DIFF;
			break;
		}
		case PHRMS:   {
			visComp_ = PHASE;
			avgrf = adjustAvgRange( RMS, recOut );
			visDev_ = RMS;
			break;
		}
		default:      {
			avgrf = adjustAvgRange( NORMAL, recOut );
			visDev_ = NORMAL;
			break;
		}
		}

		needsRefresh = avgrf || needsRefresh;

		// Use REAL component for single-dish, whenever 'Amplitude' is
		// advertized to the user (a minor deception..).

		if(dish_ && visComp_==AMPLITUDE) visComp_ = REAL;

		Bool isDev = (visDev_>NORMAL);
		// whether we're [now] in deviation display mode.

		// nDAvg_ is the nominal size of the 'boxcar', or interval of times
		// for moving averages.  The actual boxcar boundaries for each time
		// are recomputed whenever nDAvg_ changes (or extract_ is called).

		if ( itsNAvg->fromRecord(rec) || avgrf ) {
			if ( visDev_ > NORMAL ) {
				nDAvg_ = max(2, itsNAvg->value());	// (min. of 2 just for safety)
				if(visValid_) {
					computeTimeBoxcars_();
					needsRefresh = True;
				}
				if ( ! wasDev && nPAvg_ > 1 ) {
					computeTimeBoxcars_();
					needsRefresh = True;
					dispValid_ = False;
					purgeCache( );
				}
			} else {
				nPAvg_ = itsNAvg->value( );
				needsRefresh = True;
				dispValid_ = False;
				purgeCache( );
			}
		}


		// DECIDE WHETHER DATA NEEDS TO BE [RE]LOADED (via extract_).

		Bool applyPressed = (rec.nfields()>1);  // More than one option was
		// passed; we assume this means the apply button was pressed.
		// Data retrieval is sometimes deferred until 'Apply' is pressed.
		// To do: create a separate button specifically for loading
		// the data (unconditionally).

		if(applyPressed) needsRefresh = True;
		// be sure of refresh on 'apply'--some option
		// changes might have occurred in prior setOptions calls which were
		// not drawn, because of invalid vis_ (and no 'apply').

		// visDataChg_ means the user's MS selection or real visibility component
		// (i.e., the complete set of desired visibilities) is not yet reflected
		// in the memory buffer (vis_).  Signals the need to recalculate
		// visibility data ranges as well as create a new vis_ (via extract_).
		// (visValid_ means there is _some_ data in vis_ (though it may no longer
		// be the currently-desired data)).
		//
		// The last condition below means that the user has changed the baseline
		// sort, but vis_ cannot accommodate the entire baseline range (at least not
		// under the antenna sort).  In this (rather rare) case, no vis_ re-sort is
		// attempted in memory (loaded baselines would no longer be contiguous) --
		// we'll throw in the towel and reload from scratch.

		visDataChg_ = !visValid_ ||
		              visType_!=curVisType_ ||
		              visComp_!=curVisComp_ ||
		              newRanges ||
		              (bslSortChg && visShapeA_[BASELN]!=msShapeA_[BASELN]);


		Bool shouldExtract = msselValid_ &&
		                     ( applyPressed ||
		                       (visDataChg_ && curVisComp_!=INVALID_VC) );
		// Reload of data from disk will automatically occur if
		// desired vis_ data is not loaded, _except_ for the _first time_
		// on a _really large_ MS (curVisComp_!=INVALID_VC indicates that
		// vis_ has _never_ been loaded yet).
		// 'Apply' must be pressed in other cases, e.g. after selecting
		// an unloaded slice with the animator.

		if(shouldExtract) {	// <-- not done with this variable yet: still
			// determining whether extract_ is really needed.
			// (It usually is, but not if all the following
			// conditions hold...).

			if(!visDataChg_) {
				if(visShape_[axX]==msShape_[axX] && visShape_[axY]==msShape_[axY]) {
					if(pos_[axSL0]>=visStart_[axSL0] &&
					        pos_[axSL0]< visStart_[axSL0] + visShape_[axSL0] &&
					        pos_[axSL1]>=visStart_[axSL1] &&
					        pos_[axSL1]< visStart_[axSL1] + visShape_[axSL1]) {

						// So far, no extract is needed--the current vis_ applies in
						// most respects: there has been no change to the set of
						// visibilities, and vis_ covers the entire range of the display
						// axes and the selected slice for the slider axes.
						// It remains only to check whether vis_ covers displayed
						// animation frames.  This is a bit trickier, because the DD may
						// be registered on 0, 1 or more canvases, each with its own
						// animator position.  (Depending on memory size constraints,
						// it may not actually be possible to display all of them).
						// We retrieve the current animator positions on all of our
						// canvases and call extract_ if necessary, in order to display
						// on as many as possible.

						Block<Int> visShp(NAXES), visShpA(NAXES);
						computeVisShape_(visShp, visShpA);
						// visShp: a 'tentative' visShape_: what visShape_
						// will be if we have to call extract_.
						Block<Int> zInds = allZIndices();
						// a sorted block of animator positions on current canvases.

						// (BTW, this code assumes there was no Z axis change on
						// this setOptions call.  Otherwise, a poor
						// choice of Z axis data may be extracted.  The user would
						// have to try hard to shoot himself in the foot in this
						// way, though....
						// In general it's a real pain to have to assume that
						// 'auto-apply' may have been turned off somewhere and
						// that there may be more than one change to deal with
						// at once--not worth it, IMO).

						Int mrg, stfrm;	// (returned below, but unused here).


						if(maxframes_(zInds, stfrm,          visShp[axZ])   <=
						        nframes_(zInds, visStart_[axZ], visShape_[axZ], mrg) ) {

							// current vis_ already shows as many frames as possible--
							// no need to call extract_.

							shouldExtract = False;
						}
					}
				}
			}
		}



		// extract_ is only called here and during initialization.

		if(shouldExtract)    extract_();


		else if(bslSortChg && visValid_) reSortVis_();
		// Normal action when just changing sort: re-sort the vis_ buffer
		// (and corresp. flags_).
		//
		// Note that vis_ will extend over all baselines (including gaps) in
		// this case, i.e., visShapeA_[BASELN]==msShapeA_[BASELN], implying
		// that visStart_[BASELN]==0 (proof left as an exercise.
		// Hint: show visValid_ ==> msselValid_).





		// Communicate any changes in DATAMIN/MAX RANGES to/from the gui.
		// (They may be changed by the user, and/or computed in extract_).

		// dataRngMin/Max_:	latest computed data range.  May not be based on the
		//			whole selected MS--just what we've seen of it in
		//			vis_ so far.  (It is also clipped to 3-sigma limits,
		//			in order not to waste the color range on infrequent
		//			data).
		// itsDataMin/Max->value/minimum/maximum(): from/to user/gui.  Generally
		//			kept in sync to above--the user is allowed to expand
		//			slider ranges  beyond the range of the data, but
		//			they are reset to the computed ranges whenever
		//			there is a change in visType/Comp or MS selection.

		if(visValid_ && isDev && devRngMin_==NO_DATA) computeDevRange_();
		// deviation-mode data ranges still needed calculation in this case.

		if(visValid_ && (postDataRng_ || wasDev!=isDev)) {

			// extract_ has recalculated data ranges, or the deviation
			// display mode has changed.  Both ranges and values on the sliders
			// must be completely reset in either case.  The calculated ranges
			// override any any new user settings (unlikely in this case anyway).
			// They are placed in the DParameters (itsDataMin/Max) and sent out
			// to the gui.

			resetMinMax_();  // Assure that itsDataMin/Max have the computed ranges
			itsDataMin->toRecord(recOut, wholeRecord, overwrite);  // ...and send
			itsDataMax->toRecord(recOut, wholeRecord, overwrite);  // them to GUI.
			recOut.define("dataunit", dataUnit().getName());
			// ("dataunit" is not part of the UI; this assures that (esp.)
			// the color bar DD gets the latest data unit for its labelling).
			needsRefresh = True;
			postDataRng_ = False;
		}	// (msg. flag has been acted on)

		else {

			// Retrieve slider values, and just expand slider ranges if
			// necessary (may be due to user entry or to additional data
			// loading in extract_).  Post any expanded ranges to the
			// DParameters (and the gui elements themselves), but leave
			// the values as the user has set them.

			needsRefresh = itsDataMin->fromRecord(rec) || needsRefresh;
			needsRefresh = itsDataMax->fromRecord(rec) || needsRefresh;

			// fromRecord() only retrieves the slider values (it really
			// ought to retrieve ranges as well...).  We compute the
			// ranges actually on the sliders here.
			Float dminmin=min(itsDataMin->minimum(), itsDataMin->value());
			Float dminmax=max(itsDataMin->maximum(), itsDataMin->value());
			Float dmaxmin=min(itsDataMax->minimum(), itsDataMax->value());
			Float dmaxmax=max(itsDataMax->maximum(), itsDataMax->value());

			Float newrgmin = min(dminmin, dmaxmin);	// expansion due to
			Float newrgmax = max(dminmax, dmaxmax);	// any user entry.
			if(visValid_) {
				Float dmin=dataRngMin_, dmax=dataRngMax_;
				if(isDev) {
					dmin=devRngMin_;
					dmax=devRngMax_;
				}
				newrgmin = min(newrgmin, dmin);		// expansion due to
				newrgmax = max(newrgmax, dmax);
			}	// computed data ranges.

			// newrngmin/max now have final ranges to use (for both sliders)

			if(newrgmin!=dminmin || newrgmax!= dminmax) {
				itsDataMin->setMinimum(newrgmin);
				itsDataMin->setMaximum(newrgmax);
				itsDataMin->toRecord(recOut, wholeRecord, overwrite);
			}

			if(newrgmin!=dmaxmin || newrgmax!= dmaxmax) {
				itsDataMax->setMinimum(newrgmin);
				itsDataMax->setMaximum(newrgmax);
				itsDataMax->toRecord(recOut, wholeRecord, overwrite);
			}
		}




		// FLAGGING OPTIONS.

		if(itsFlagColor->fromRecord(rec)) {
			flagsInClr_ = itsFlagColor->value()=="In Color";
			needsRefresh = True;
		}

		if(itsUnflag->fromRecord(rec)) unflag_ = itsUnflag->value()=="Unflag";
		if(itsEntireAnt->fromRecord(rec)) entireAnt_ = itsEntireAnt->value()=="Yes";

		if(itsUndoOne->fromRecord(rec)) if(undoEdits_("one")) needsRefresh = True;
		if(itsUndoAll->fromRecord(rec)) if(undoEdits_("all")) needsRefresh = True;
		itsEditEntireMS->fromRecord(rec);	// (no refresh effect).
		if(itsSaveEdits->fromRecord(rec)) if(saveEdits_()) needsRefresh = True;

		// Flagging extent checkboxes.  The return value (inconveniently)
		// may be either a String or a Vector of Strings--handle both cases.
		// (To Do: implement a DParameter for these checkboxes instead).

		Vector<String> axisNames(NAXES);
		for(Axis ax=0; ax<NAXES; ax++) axisNames(ax) = axisName_(ax)+String("s");
		// (form plurals of axis names for this use).
		String checkedAxis;
		readOptionRecord(checkedAxis, notfound, rec, "flgall");    // single String
		if(!notfound) {
			for(Axis ax=0; ax<NAXES; ax++) {
				flagAll_[ax] = (checkedAxis==axisNames[ax]);
			}
		} else {
			Vector<String> checkedAxes;
			readOptionRecord(checkedAxes, notfound, rec, "flgall");  // Vector<String>
			if(!notfound) {
				for(Axis ax=0; ax<NAXES; ax++) {
					flagAll_[ax] = (ind_(axisNames[ax], checkedAxes) != -1);
				}
			}
		}

        bool spec_change = itsParamSpectralUnit->fromRecord(rec);
		needsRefresh =  spec_change || needsRefresh;
        if ( rec.isDefined("spectralunit") ) rec.removeField("spectralunit");


		// set OPTIONS ON BASE CLASS levels
		needsRefresh = ActiveCaching2dDD::setOptions(rec,recOut) || needsRefresh;
        if ( spec_change ) {
            recOut.define( "spectralunit", itsParamSpectralUnit->value( ) );
            if ( axisOn_(X) == CHAN || axisOn_(Y) == CHAN ) {
                setCS_( );
                if ( itsParamSpectralUnit->value( ) != "channel" ) {
                    Record in,out;
                    in.define("spectralunit",itsParamSpectralUnit->value( ));
                    itsAxisLabeller.setOptions(in,out);
                }
            }
        }


		// set HELPER CLASS OPTIONS
		needsRefresh = itsPowerScaleHandler.setOptions(rec,recOut) || needsRefresh;

		needsRefresh = itsAxisLabelling->fromRecord(rec) || needsRefresh;
		needsRefresh = itsAxisLabeller.setOptions(rec,recOut) || needsRefresh;

		// A True return value means caller should call refresh() on this DD,
		// which eventually leads to a redraw.
		return needsRefresh;
	}



//--------------------------------------------------------------------------
	Bool MSAsRaster::draw_(Display::RefreshReason /*reason*/,
	                       WorldCanvasHolder &wch, WorldCanvas &wc)  {
		// (!)
		// The MSAsRasterDM only holds the drawlist handle; all the drawing data
		// is in the DD--it makes sense to draw here in the DD too.  _This_ is the
		// routine that does the WC drawing.

		// This routine should only be called by the draw() method of an
		// MSAsRasterDM which in turn is owned by this DD.

		// Input:
		// If msselValid_ is False, we cannot draw (no data).  Otherwise,
		// at this point the VisSet (vssel_), ranges (msShape_), coordinate system,
		// axis settings (axisOn_) and slice positions (pos_) are set up according
		// to user options (except that pos_[axisOn_(Z)], the animator position,
		// must still be retrieved from the 'zIndex' restriction on the WCH).
		// visType_ and visComp_ also reflect user settings.

		// No 'Passive' behavior is implemented yet: we assume the WC CS and
		// related 'canvas-wide' restrictions (axis codes) were set (by this DD
		// and its helpers) in agreement with the above.

		// We know there is no valid cached drawlist, or it would have been used
		// instead of calling this routine.

		// the current visibility array (vis_) may or may not contain the
		// requested data; the same goes for the slice Matrix (disp_) that is
		// actually passed to WC::drawImage().  vis_ may contain some, none or
		// all of the slice to be drawn on canvas.  Portions of the data which
		// are not loaded will be masked or displayed as dark grey.  The user
		// can cause the missing data to be loaded by pressing 'Apply'.

		// The return value indicates whether the DD was able to draw (False if
		// the chosen position was out of range of the selected MS--can happen
		// in the final frames of multipanel display, e.g.).

		if(!msselValid_) return False;	// No data in [selected] MS.

		// The color wedge is no longer displayed for DisplayDatas whose
		// state is something other than DISPLAYED...
		setDisplayState( DisplayData::DISPLAYED );

		Axis axX=axisOn_[X], axY=axisOn_[Y], axZ=axisOn_[Z];

		// Retrieve animator axis position.  NB: this is the only parameter
		// affecting the suitibility of the current vis_ which is not
		// determined by the DD or its 'Adjust' input.  It may vary with the
		// world canvas, which is an input parameter to this
		// draw_ routine.  Recall that this DD may draw on more than one canvas.
		// Animator position is essentially a WorldCanvas property (unlike the
		// positions on the axis sliders, e.g.).
		// Presumably, conformsTo() was checked before entering this routine,
		// so that we could just use activeZIndex_ instead of retrieving it
		// here explicitly..

		Int zIndex;
		Bool zIndexExists;
		const AttributeBuffer *wchRestrs = wch.restrictionBuffer();
		zIndexExists = wchRestrs->getValue("zIndex",zIndex);
        if(zIndexExists) {
             pos_[axZ] = zIndex;
             if ( axZ == SP_W ) {
                  mspos_.sets(pos_[SP_W]);
                  DisplayCoordinateSystem newcs = setCS_( );
                  wc.setCoordinateSystem(newcs);
             }
        }
		pos_[axZ] = max(0, min(msShape_[axZ]-1,  pos_[axZ]  ));
		if(zIndexExists && zIndex != pos_[axZ] ) return False;
		// Do not draw if the animator position value
		// on the WCH is out of range.
		// Trim pos_[axZ] anyway for the sake of robustness:
		// extract_ and createDisplaySlice_ expect pos_ values to be in range;
		// slider positions are also trimmed, in setOptions.


		// Are the display Matrices (disp_, dispMask [, dispFlags_])
		// already up to date?

		Bool dispOK = dispValid_ && dispX_==axX && dispY_==axY;
		// (For simplicity) they must have been created since last extract_
		// and last MS selection / range gathering.  Also, they can be used
		// for this drawing request only if they were created for the
		// current display axes.

		if(dispOK) {

			//  (Getting here implies createDisplaySlice_() has been executed
			//  at some time already, so that all the disp*_ below have also been
			//  initialized by this time.)

			for(AxisLoc loc=Z; loc<=SL1; loc++) {
				Axis ax=axisOn_(loc);
				if(pos_[ax]!=dispPos_[ax]) dispOK=False;
			}
			// Disp_ must also have been created for the position settings
			// on other axes.

			if(dispNEdits_ > flagEdits_.len()) dispOK=False;
			// Undo of an edit: easiest to re-create
			// display matrices from scratch here too...

			if(!dispFlagsInClr_ && flagsInClr_) dispOK=False;
		}
		// ...and here, to create the color flag matrix (dispFlags_).


		if(!dispOK) createDisplaySlice_();


		if(dispFlagsInClr_ && !flagsInClr_) {
			// This means flagging in color was switched off, but otherwise disp_ and
			// friends did not need recreation (no call to createDisplaySlice_).
			// We must simply invert dispMask_ to make it suitable for non-color
			// flag display.
			Int nx=dispMask_.shape()(X);
			Int ny=dispMask_.shape()(Y);
			for(Int ix=0; ix<nx; ix++) {
				for(Int iy=0; iy<ny; iy++) {
					dispMask_(ix,iy) = !dispMask_(ix,iy);
				}
			}
			dispFlagsInClr_ = False;
		}

		postEditsToDisp_();	// Make sure all flagging edits are also
		// reflected in the display matrices.

		if(visDev_==DIFF || visDev_==RMS) {

			// A visibility deviation display mode is in effect: we will display
			// the dispDev_ Matrix instead of disp_.  Make sure dispDev_ is
			// up-to-date.

			if(!dispDevValid_ || dispDevType_!=visDev_ || dispDevNAvg_!=nDAvg_) {

				createDevSlice_();
			}
		}

		// Set data scale.  Must set min/max onto WC, then register scale
		// handler with WC.  WC will then set min/max onto scale handler.
		// Min/max really should be scale handler user options,
		// and go directly from the gui to it, without DD
		// involvement at all, except for setting their range....

		wc.setDataScaleHandler(&itsPowerScaleHandler);
		wc.setDataMinMax(itsDataMin->value(), itsDataMax->value());

		WCResampleHandler *useDefault=0;	// be sure resampling is set to
		wc.setResampleHandler(useDefault);	// default (nearest neighbor, i.e.
		// single color for each data pixel).


		// Finally, we are ready to draw the display matrix onto the WC.

		Vector<Double> lblc(2, -.5);	// WC::drawImage needs to know where to
		Vector<Double> ltrc(2);	// place the array passed to it, i.e., WC
		ltrc(X)=disp_.shape()(X)-1 + .5;	// coordinates for the array's
		ltrc(Y)=disp_.shape()(Y)-1 + .5;	// corner pixels' outer edges.

		// Some things shouldn't be in world coordinates.  The corners of an image,
		// which is rectangular only in the WC's linear system, and for which linear
		// coordinates are more immediately available, is one of them.  World
		// coordinates might not even exist at the corners of an image.  But
		// the method wants world coordinates (which it promptly converts back
		// to pixel/linear coordinates), so we fetch them here...
		Vector<Double> wblc(2), wtrc(2);
		linToWorld(wblc, lblc);
		linToWorld(wtrc, ltrc);

		Bool edge=True; 	// We pass the coordinates of the corner pixel
		// outer edges to drawImage.

		if(!flagsInClr_) {


			if(visDev_==NORMAL) wc.drawImage(wblc, wtrc, disp_,    dispMask_, edge);
			else	        wc.drawImage(wblc, wtrc, dispDev_, dispMask_, edge);
			// simple masked (non-color) flag drawing.


			wc.setDataScaleHandler(0);
		}
		// (Don't leave our scale handler on the wc:
		// we might go away before the wc does).

		else {

			// Color flag drawing: we won't mask the data itself; instead
			// we draw flags as a separate top layer, masked
			// in the opposite sense, to show the good data underneath.

			// bottom (data) layer.

			if(visDev_==NORMAL) wc.drawImage(wblc, wtrc, disp_,    edge);
			else	        wc.drawImage(wblc, wtrc, dispDev_, edge);


			// Color flag drawing uses a custom colormap (flagCM_).
			// dispFlags_ and flagCM_ use the following code:
			//
			// 0--data not loaded (grey)
			// 1--no data in MS selection (black)
			// 2--old flag (medium blue)
			// 3--newly edited, unsaved flag (lighter blue)
			//
			// for good data, disp_flags_ is masked out, to show the data value
			// from disp_ in the layer below.

			wc.setDataScaleHandler(0);
			// use default here, for linear scaling of flag codes to colors.
			wc.setDataMinMax(NOTLOADED , NEWFLAG);
			// range of colormapped codes above, mapped to flagCM_
			// colormap, below.

			Colormap* cmsave = wc.colormap();	      // (will be restored below)
			wc.setColormap(&flagCM_);	// Color cells for this custom-coded colormap
			// were allocated in notifyRegister().


			wc.drawImage(wblc, wtrc, dispFlags_, dispMask_, edge);   // flag layer.


			wc.setColormap(cmsave);
		}
		// Restore the user's colormap--the fiddler mouse tools and color
		// wedge apply to the map last set onto the wc (pc).


		return True;
	}



//--------------------------------------------------------------------------
	void MSAsRaster::extract_() {
		// Iterate through the MS and extract the hypercube buffer
		// of visibilities (vis_) for the existing axis settings.
		// This is the most time-consuming work of MSAR--it can take
		// several minutes for a large MS.  Caller should first determine
		// that vis_ is not already up-to-date.

		// Input: valid, non-null VisSet vssel_ and its ranges (msShape_).
		//       Which axes are on display and on slice controls (axisOn_).
		//       Slice to display (pos_[axisOn_[Z*, Sl0, and SL1]]),
		//		 with 0 <= pos_ < msShape_ on these axes.
		//       Requested visType_ and real visComp_.
		//	   visDataChg_ ( == 'data ranges should be completely recalculated').
		//
		// Output: vis_ (and corresp. vis_ state data: visValid_, curVisType_,
		//	     curVisComp_, visShape_, visShapeA_, visStart_), which has the
		//	     requested visType_ and visComp_, and includes the requested
		//         display slice(s).
		//	     dataRngMin/Max_ -- actual min/max of the data encountered.
		//	     postDataRng_ is returned True if gui data ranges should be
		//	     reset to dataRngMin/Max_.

		// (10/02) *The canvases themselves are now polled for the slices
		// they wish to display on the Z axis, and the vis_ 'window' on
		// this axis is positioned so as to include as many of them as possible.
		// pos_[axisOn_[Z]] is now used only as a default in the case where
		// the DD is not currently registered on any canvas.


		// compute array dimensions which fit into allowed memory (visShape_,
		// visShapeA_).
		//
		// visShapeA_ will be identical to visShape_, except that it will be
		// larger on the BASELN axis iff: the MS is not single dish (nAnt_>1),
		// the baseline length sort is to be used (!antSort_), and
		// computeVisShape_() determines there is enough memory for vis_ to
		// span the entire baseline axis, even if resorted by antenna
		// (visShapeA_{BASELN] will be == msShapeA_{BASELN]).
		// In that case, visShape_[BASELN] will be only the total number of
		// 'real' baselines including autocorrs (i.e. nAnt_*(nAnt_+1)/2), but
		// visShapeA_[BASELN] will add to that nAnt_-1 'gap' baselines.  Under the
		// length sort the gaps reside (rather uselessly) at the end of the axis,
		// and are not displayed; however, they are dispersed throughout the axis
		// under the antenna sort, and are displayed in that case.
		// vis_ is actually sized to visShapeA_, but vis_ is treated as if its size
		// is only visShape_ by the display code.

		computeVisShape_(visShape_, visShapeA_);

		Int vnTime=visShape_[TIME], vnBsln=visShape_[BASELN],
		    vnBslnA=visShapeA_[BASELN],
		    vnChan=visShape_[CHAN], vnPol=visShape_[POL], vnSpw=visShape_[SP_W];


		// For progress feedback.

		Timer tmr;
		Double pctDone = 100./max( 1.,  Double(mssel_->nrow()) *
		                           Double(visShape_[SP_W]) /
		                           max(1., Double(msShape_[SP_W])) );
		// chunks not in sp.win. range won't need to be read.
		// Inaccurate progress values may still result, however,
		// when the various sp.wins have uneven amounts of data...
		Bool prgShown=False;
		Int iRow=0, iIter=0, iChunk=0;
		Double intvl=3.;


		vis_.resize(IPosition(5,   vnTime, vnBslnA, vnChan, vnPol, vnSpw));
		vis_ = NO_DATA;	// (A value unused by real data).

		Double visSize = vnTime*vnBslnA*vnChan*vnPol*vnSpw;
		static const Int uIntBits = sizeof(uInt)*8;
		uInt flagsSize = uInt(ceil(visSize/uIntBits));
		flags_.resize(flagsSize);	// a bit for every element of vis_...
		flags_ = 0u;			// ...all initialized to 0 (false).

		// vis_ will hold the block of data that includes requested slice and
		// starts at a multiple of visShape_ on each axis (relative to the
		// whole ms), except when there's no exact fit at the end of an axis.

		for(Axis ax=0; ax<NAXES; ax++) {
			if(visShape_[ax] == msShape_[ax]) visStart_[ax] = 0;
			else visStart_[ax] = min( pos_[ax] - (pos_[ax] % visShape_[ax]) ,
				                          msShape_[ax] - visShape_[ax] );
		}

		// (10/02) The determination of the vis_ 'window' positioning on the Z
		// axis is slightly more complicated.  We look at all the animator
		// positions on canvases where this DD is registered (if any) and
		// select an interval for vis_ on this axis (visStart_[axZ]) which
		// includes as many of them as possible.

		Block<Int> zInds = allZIndices();
		// All the animator frame numbers on all registered canvases.
		if(zInds.nelements()>0) {
			Axis axZ=axisOn_[Z];

			maxframes_(zInds, visStart_[axZ], visShape_[axZ]);
		}
		// Sets visStart_[axZ] (the starting point for the window or
		// interval to load along the Z axis).  visShape_[axZ] (the size
		// of the interval) was already set above, in computeVisShape_().

		Int vsTime=visStart_[TIME], vsBsln=visStart_[BASELN],
		    vsChan=visStart_[CHAN], vsPol=visStart_[POL], vsSpw=visStart_[SP_W];

		// Endpoints (just _beyond_ axis ranges of vis_).  As with visStart_, these
		// are indices relative to whole selected MS (as characterized by msShape_).

		Int veTime=vsTime+vnTime, veBsln=vsBsln+vnBsln, veChan=vsChan+vnChan,
		    vePol=vsPol+vnPol, veSpw=vsSpw+vnSpw;



		// If the set of visibilities has changed, recalculate the data ranges
		// completely (as opposed to just allowing them to expand).

		static const Float radToDeg = 180.f / 3.14159265f;
		Bool resid = (visType_ >= RESIDUAL),
		     phase = (visComp_ == PHASE);

		if(phase) {
			dataRngMin_=-180.;
			dataRngMax_=180.;
		}
		// phase range is hard-coded (is this a good idea?...)
		else if(visDataChg_) {
			dataRngMin_=0;    // reset ranges.
			dataRngMax_=0;
		}
		Double sumv=0., sumv2=0.;
		Int nvis=0;
		// For 3-sigma trim of data scale slider ranges.


		typedef Float CompFn(const Complex&);
		CompFn* compFn[NCOMPS] = { &abs, &arg,
		                           &MSAsRaster::real, &MSAsRaster::imag
		                         };
		// compFn contains pointers to the standard library
		// complex-to-float conversion routine, corresponding to visComp_.
		CompFn& component = *(compFn[visComp_]);
		// component(complexVisibility)  will return the correct
		// real component of complexVisibility, when used below.


		String polerr = "MSAsRaster: polarization conformance error in MS";


		IPosition slot(NAXES);  // index into vis_ array

		Cube<Complex> vc, vc0;
		Cube<Bool> flg;
		Vector<Bool>flgRow;



		// ---Iterate the selected MS to fill the vis_ Array---

		VisibilityIterator &vi(*wvi_p);
		VisBuffer vb(vi);

		for (vi.originChunks(); vi.moreChunks(); vi.nextChunk()) {

			Int iRowChunk=iRow;		// # rows processed prior to this chunk
			Bool doneChunk=False;	// signals that we're finished (early)
			// with this chunk.

			Int spw = spw_(vi.spectralWindow());	  // Sp. Win. axis index.
			if(spw<vsSpw || spw>=veSpw) doneChunk=True;   // not in spw range of vis_.
			else slot(SP_W)=spw-vsSpw;

			Int polId = vi.polarizationId();
			if(polId<0 || polId>=nPolIds_) throw AipsError(polerr);
			Int nPolsInDataCell=vi.visibilityShape()(0);
			if(nPolsIn_[polId] != nPolsInDataCell) throw AipsError(polerr);
			Int pidBase = pidBase_[polId];
			if(pidBase==INVALID) throw AipsError(polerr);

			Int frstPol = max(vsPol, pidBase);
			Int lstPol =  min(vePol, pidBase + nPolsInDataCell);
			if(frstPol>=lstPol) doneChunk=True;		// not in pol. range.

			Int nChan=vi.visibilityShape()(1);
			Int frstChan=   vsChan;
			Int lstChan=min(veChan,nChan);
			if(frstChan>=lstChan) doneChunk=True;	// not in channel range.


			if(doneChunk) {
				iRow = iRowChunk;
				if(spw>=vsSpw && spw<veSpw) iRow += vi.nRowChunk();
				iChunk++;
				continue;
			}	// ... to next chunk.

			// Set row blocking to retrieve as many rows as possible consistent
			// with modest-sized table column Arrays.  First compute the amount
			// of data to be retrieved in each row, which depends particularly
			// on the shape of the visibility cells.
			// Here we're retrieving time, flagrow, feed1 or ant1,ant2,  flags,
			// and either 1 or 2 visibility cubes.  (Add as needed).

			Int rowBytes = sizeof(Double) + sizeof(Bool)   +
			               ((nAnt_==1)? 1 : 2)*sizeof(Int) +
			               nPolsInDataCell*nChan*
			               (sizeof(Bool) + (resid? 2:1)*sizeof(Complex));
			Double iterMemory = max(1024., .01*HostInfo::memoryTotal()) * 1024.;

			// Allow use of up to 1% of memory (or at least 1Mb) for these MS
			// table buffers (whole chunks will sometimes require even less).

			Int nBlkRows = Int(min(1.e8, max(50., iterMemory/rowBytes )));
			vi.setRowBlocking(nBlkRows);


			// Times are sorted (only) within each chunk now.
			// Reset time_ vector's search index to the beginning of vis_'s
			// retrieval range on the time axis.

			Int iTime = vsTime;
			Double searchTime = time_[iTime];
			Int tsOffset=0;
			Bool newTime=True;


			for (vi.origin(); vi.more(); vi++) {

				Int nRow=vb.nRow();


				// Retrieve the visibility cube(s) for this iteration.  extract_
				// spends most of its time in the if statement below (mostly in
				// disk I/O (?)).
				// extract_ for RESIDUAL takes about 60% longer than for others.

				if(resid) {
					vi.visibility( vc, VisibilityIterator::Corrected );
					vi.visibility(vc0, VisibilityIterator::Model);
				}
				// need both corrected and model vis cubes for residual.
				else
					vi.visibility(vc, (VisibilityIterator::DataColumn)visType_);
				// Only single cube of correct type needed.

				vi.flag(flg);	// Retrieve corresponding flags.
				vi.flagRow(flgRow);


				// Transfer the data to vis_ and flags_ arrays.

				for (Int row=0; row<nRow; row++) {

					if(tmr.real()>intvl) {
						if(!prgShown) {
							cerr<<endl<<"Loading MS vis. data:  "<<flush;
							prgShown=True;
						}
						Int pct = Int((iRow+row)*pctDone);
						if(pct>0 && pct<100) cerr<<pct<<"%  "<<flush;
						tmr.mark();
						intvl++;
					}
					// progress feedback (can't use progress meter...  )-;

					Double time=vb.time()[row];

					if(time<searchTime) continue;  // not yet in time range.

					while(time!=searchTime) {	// we should find an exact match...
						iTime++;
						if(iTime>=veTime) {		// ... unless we're beyond time
							doneChunk=True;		// range of vis_, in which case
							break;
						}			// we're done with the whole chunk.
						newTime=True;		// record first encounter of time in chunk.
						searchTime=time_[iTime];
					}

					if(doneChunk) break;

					// time found in time_ vector at slot iTime.

					slot(TIME)=iTime-vsTime;

					// For reasons of efficiency, the offset into the 1-D flags_
					// vector is hand-computed in the loops below, given the 5 axis
					// values.  This is the code dealing with 'Offset', 'fslot' and
					// 'bit'.  The same array index arithmetic appears in a compact
					// form in setFlag_(slot), which could have been used instead.
					if(newTime) {
						tsOffset = vnBslnA*(vnSpw*(iTime-vsTime) + spw-vsSpw);
						newTime=False;
					}


					Int bsl;
					Bool calcRange=True;

					if(nAnt_==1) bsl=vb.feed1()(row);
					// single antenna: 'bsl' is feed number.
					else {
						Int ant1=vb.antenna1()(row);
						if(ant1<0) continue;				// sanity check
						Int ant2=vb.antenna2()(row);
						if(ant2<ant1 || ant2>=nAnt_) continue;	// sanity check
						calcRange = (ant1!=ant2);
						// Do not include autocorr's in data ranges
						bsl=bsl_(ant1, ant2);
					}

					if(bsl<vsBsln || bsl>=veBsln) continue;  // not in Baseline range.

					slot(BASELN)=bsl-vsBsln;
					Int bOffset = vnPol*(tsOffset + bsl-vsBsln);


					for (Int pol=frstPol; pol<lstPol; pol++) {
						slot(POL)=pol-vsPol;
						Int pOffset = vnChan*(bOffset + pol-vsPol);
						Int vcpol = pol-pidBase;
						for (Int chn=frstChan; chn<lstChan; chn++) {
							slot(CHAN)=chn-vsChan;

							Float v  =  (resid)?
							            component( vc(vcpol,chn,row) - vc0(vcpol,chn,row) ) :
							            component( vc(vcpol,chn,row) );

							if(phase) v *= radToDeg;		// show phases in degrees.

							vis_(slot) = v;			// store visibility.

							if(flgRow(row) || flg(vcpol,chn,row)) {	// store flags
								Int offset = pOffset + chn-vsChan;
								Int fslot = offset / uIntBits;
								Int bit = offset % uIntBits;
								flags_(fslot) = flags_(fslot) | 1u<<bit;
							}

							else if(!phase && calcRange) {
								// data range calculations (over unflagged data only)
								if(v<dataRngMin_) dataRngMin_=v;
								if(v>dataRngMax_) dataRngMax_=v;
								sumv += v;
								sumv2 += Double(v)*v;
								nvis++;
							}
						}
					}
				}


				iIter++;
				if(doneChunk) break;	// (from vi minor iterations).
				iRow+=nRow;
			}		// for(vi minor iterations)

			iRow = iRowChunk + vi.nRowChunk();
			iChunk++;
		}		// for(vi Chunk iterations)


		mssel_->relinquishAutoLocks(True);	 	// (just to be sure).

		visValid_=True;		// validate vis_, and set its current
		curVisType_=visType_;		// type and component to reflect the extract_
		curVisComp_=visComp_;		// just completed according to user input.

		// Clip data color scaling ranges to 3-sigma limits, so that wild
		// data doesn't cause ridiculously large ranges on the sliders.
		if(!phase && nvis>=2) {
			Double vbar = sumv/nvis;
			Double sumv2dev = sumv2 - sumv*sumv/nvis;
			if(sumv2dev>0) {
				Double vsig = sqrt(sumv2dev/(nvis-1));
				dataRngMin_ = max(Float(vbar - 3*vsig), dataRngMin_);
				dataRngMax_ = min(Float(vbar + 3*vsig), dataRngMax_);
			}
		}


		// For visibility deviation displays.  (The range gathering is
		// similar, but separate from the above).

		computeTimeBoxcars_();
		// Compute the time intervals to use for each running average.
		if(visDev_>NORMAL) computeDevRange_();
		// Compute the range of all deviations (for color scaling), or...
		else devRngMin_=NO_DATA;
		// ...indicate that it hasn't been computed yet (it can take a
		// little time, and might not be necessary).


		if(prgShown) cerr<<"Done."<<endl<<endl;	// progress feedback.

		if(visDataChg_) {
			resetMinMax_();	// Set newly-computed data range onto the DParams.
			postDataRng_=True;
		}
		// Signals getOptions or setOptions (whichever is called first)
		// to pass data range on to the gui unaltered.

		visDataChg_=False;	// vis_ now in sync with user data selection.
		dispValid_=False;	// Forces call to createDisplaySlice_ in draw_.
		purgeCache();
	}	// Assures that displayed data corresponds to
	// data in memory, to avoid confusion.


//--------------------------------------------------------------------------
	void MSAsRaster::createDisplaySlice_() {
		// create (2D) slice Matrices to draw on the WC, from vis_ and flags.
		// Input: axisOn_ and pos_ describing what slice of selected ms to display.
		//	    valid vis_ that conforms to user input
		//	    ([MS selection], visType/Comp_).
		//	    flags_ (read from the MS, corresp to vis_)
		//        flagEdits_--the List of new flagging commands.
		//        flag display options (flagsInClr_[, itsShowFlags])
		// Output: disp_, dispMask_
		//	     dispFlags_  (for the flags 'In Color' user option only)
		//         metadata describing the current state of the Matrices above
		//	     (these member variables also all begin with 'disp')
		// Note: the output matrices will reflect saved flags from the MS as well
		//	   as 'no data' or 'not loaded' areas, but _not_ new flagging edits.
		//	   postEditsToDisp_ is called (by draw_) to add these, prior
		//	   to sending the matrices to WC::drawImage.

		// (10/02) vis_ is no longer required to contain all the data on the
		// display axes in order to draw...

		Axis dispZ_=axisOn_(Z);
		dispX_=axisOn_(X), dispY_=axisOn_(Y);	 // data axes on display.
		Int nx=msShape_[dispX_], ny=msShape_[dispY_];

		disp_.resize(nx,ny);
		dispMask_.resize(nx,ny);
		if(flagsInClr_) dispFlags_.resize(nx,ny);


		dispNotLoaded_=False;

		if(!visValid_) dispNotLoaded_=True;
		// (may occur during init, if vis_ is not loaded immediately).
		else for(AxisLoc loc=Z; loc<=SL1; loc++) {
				Axis ax=axisOn_(loc);
				if(pos_[ax]<visStart_[ax] || pos_[ax]>=visStart_[ax]+visShape_[ax]) {
					dispNotLoaded_=True;
					break;
				}
			}

		if(dispNotLoaded_) {

			// Simple case in which the whole slice is out of range of vis_.

			disp_ = NOT_LOADED;
			if(flagsInClr_) {
				dispMask_ = True;
				dispFlags_ = NOTLOADED;
			} else dispMask_ = False;
		}

		else {

			// vis_ (and flags_) contain at least some of the display slice;
			// retrieve into the display matrices.

			dispMask_ = !flagsInClr_;	// Initialized to the 'good data' value.

			IPosition slot(NAXES);	// index into vis_ array

			for(AxisLoc control=Z; control<=SL1; control++) {
				Axis ctlAx=axisOn_(control);
				slot(ctlAx) = pos_[ctlAx] - visStart_[ctlAx];
			}
			// Set the (fixed) animator/slider slice position within vis.

			// fill display Matrices along display axes.

			// The user can choose to display flags in color or simply masked
			// to background.  In the latter case, dispMask_ is set to False for
			// flagged data so that the data pixel is masked out.  In the color case,
			// the data matrix is drawn first without a mask, then the color-coded
			// and masked flag matrix is drawn on top of that. In that case, the
			// mask matrix has the opposite 'polarity'--i.e. False to show good data
			// underneath.
			//
			// dispFlags_ (and flagCM_) are used for the flagsInClr_ option (only).
			// They use the following code:
			//
			// 0--data NOTLOADED, i.e. not in range of vis_ (dark grey)
			// 1--NODATA in MS selection (black)
			//    (NB: these are different from the NO_DATA and NOT_LOADED
			//    values, which indicate the same thing in vis_ and disp_).
			// 2--OLDFLAG (medium blue)
			// 3--NEWFLAG (lighter blue)--newly edited, unsaved flag
			//
			//    (For loaded, unflagged, valid data, the value of dispFlags_
			//    is irrelevant; it will be masked out).


			Int vsX = visStart_[dispX_],     vsY = visStart_[dispY_];
			Int veX = vsX+visShape_[dispX_], veY = vsY+visShape_[dispY_];

			for(Int ix=0; ix<nx; ix++) {  // fill Martices along display axes.
				if(ix<vsX || ix>=veX) {

					// cases for the 'frame' around the vis_ boundaries, where
					// data is not loaded.

					for(Int iy=0; iy<ny; iy++) {
						disp_(ix,iy) = NOT_LOADED;
						if(flagsInClr_) {
							dispMask_(ix,iy) = True;
							dispFlags_(ix,iy) = NOTLOADED;
						} else dispMask_(ix,iy) = False;
					}
				}

				else {
					for(Int iy=0;  iy<vsY; iy++) {
						disp_(ix,iy) = NOT_LOADED;
						if(flagsInClr_) {
							dispMask_(ix,iy) = True;
							dispFlags_(ix,iy) = NOTLOADED;
						} else dispMask_(ix,iy) = False;
					}

					for(Int iy=veY; iy<ny; iy++) {
						disp_(ix,iy) = NOT_LOADED;
						if(flagsInClr_) {
							dispMask_(ix,iy) = True;
							dispFlags_(ix,iy) = NOTLOADED;
						} else dispMask_(ix,iy) = False;
					}

					// 'Core' rectangle where data has been loaded into vis_:
					// [vsX,veX) x [vsY,veY)

					slot(dispX_) = ix-vsX;		// (adjust for visStart_ offset).
					for(Int iy=vsY; iy<veY; iy++) {
						slot(dispY_) = iy-vsY;

						Float d = NO_DATA;
						if ( nPAvg_ > 1 && visDev_ == NORMAL ) {
							IPosition slotb = slot;
							slotb(dispZ_) = min( (Int) (slotb(dispZ_)+nPAvg_-1), (Int) (vis_.shape( )(dispZ_)-1) ) ;
							const Array<Float> avga = vis_(slot,slotb);
							Bool delstor = False;
							const Float *stor = avga.getStorage(delstor);
							int num = 0;
							Double sum = 0;
							for ( size_t x=0; x < avga.nelements(); ++x ) {
								if ( stor[x] != NO_DATA ) {
									sum += stor[x];
									++num;
								}
							}
							avga.freeStorage( stor, delstor );
							d = disp_(ix,iy) = (num == 0 ? NO_DATA : (Float) (sum / (Double) num));
						} else {
							d = disp_(ix,iy) = vis_(slot);	// disp_: the data display Matrix
						}

						if(d == NO_DATA) {
							if(flagsInClr_) {
								dispMask_(ix,iy) = True;
								dispFlags_(ix,iy) = NODATA;
							} else dispMask_(ix,iy) = False;
						}

						else if(flag_(slot)) {
							if(flagsInClr_) {
								dispMask_(ix,iy) = True;
								dispFlags_(ix,iy) = OLDFLAG;
							} else dispMask_(ix,iy) = False;
						}
					}
				}
			}
		}


		dispFlagsInClr_ = flagsInClr_;	// save current flag display style.
		dispNEdits_ = 0;	// None of the new flagging edits (if any)
		// are reflected in dispFlags_ or dispMask_ yet.
		// (postEditsToDisp_ will be called to do that).
		dispPos_ = pos_;	// Save record of which slice disp_ represents.
		dispValid_ = True;	// Record that disp_ was created since last extract_.
		dispDevValid_ = False;
	}	// deviation Matrix is _not_ yet up-to-date.



//--------------------------------------------------------------------------
	AttributeBuffer MSAsRaster::optionsAsAttributes() {
		// Return option values which affect drawing appearance, as an
		// AttributeBuffer.  These 'restrictions' define what will be drawn.
		// CachingDD compares this buffer to its cache of drawings to determine
		// whether any of them can be displayed in lieu of drawing from sctatch.

		// (needs work--depends on some basic design issues, e.g. canvas-wide
		// restrictions/state, who controls canvas coordinates, better
		// world coordinates for MSAR,...)


		//Base class.
		AttributeBuffer restrctns = ActiveCaching2dDD::optionsAsAttributes();

		if(!msValid_) return restrctns;	// In this case, the object is useless.
		// Protect public methods from crashes.

		// These first restrictions (and perhaps axis lengths as well) should
		// really be incorporated into the axis codes, so that all DDs on the
		// canvas assure conformity to them.  They have to do with the current
		// real world-coordinate meaning of the canvas.  MSAR's current 'axiscodes'
		// do not define this adequately (but current DDs will not match them
		// anyway).  Whenever draw_ is called MSAR assumes the following reflect
		// the current state of the entire canvas.  It might not behave well if it
		// were not in charge of canvas coordinate state.  This is why it
		// returns False to conformsToCS() if it is not CS master, which effectively
		// disables it from responding to canvas events (drawing, labelling,
		// tracking, flagging) in this case.
		// In summary: if this DD (and its axis labeller) are drawing on the
		// canvas, nothing else will be (that's the intention, anyway).

		// I could get these values from axisOn_, but prefer something a little
		// closer to the canvas CS itself.
		Vector<String> axname = itsCoordinateSystem.worldAxisNames();
		restrctns.set("msar xaxisname", axname(X));
		restrctns.set("msar yaxisname", axname(Y));

		// Since Z position ('zIndex') is a canvas-wide restriction, its meaning
		// should also really be the same on all canvas drawing layers.
		restrctns.set("msar zaxisname", axisName_(axisOn_(Z)) );

		// The world meaning of all MSAR axes depends on the MS and selection.
		// (including types of averaging, etc.)  Rather than define the meaning
		// of each timeslot, channel number, etc., the DM cache is
		// simply purged when these are changed.  MS name is encoded into the
		// restrictions anyway, but it's not really necessary at present...

		restrctns.set("msar msname", itsMS->tableName());
		itsMS->relinquishAutoLocks(True);	 // (just to be sure of unlock).



		// The rest could properly be considered state just for this DD,
		// that its DMs must match in order to be reused.

		// Positions of the axes on sliders.
		restrctns.set("msar " + axisName_(axisOn_(SL0))+" pos",
		              pos_[axisOn_(SL0)] );
		restrctns.set("msar " + axisName_(axisOn_(SL1))+" pos",
		              pos_[axisOn_(SL1)] );
		restrctns.set("msar bslsort", antSort_);

		restrctns.set("msar vistype", visType_);
		restrctns.set("msar viscomp", visComp_);
		restrctns.set("msar visdev", visDev_);
		if(visDev_!=NORMAL) restrctns.set("msar navg", nDAvg_);


		restrctns.set("msar datamin", itsDataMin->value());
		restrctns.set("msar datamax", itsDataMax->value());

		restrctns.set("msar flagcolor", itsFlagColor->value());

		//# optionsAsAttributes() really should be implemented on helper classes
		// like WCDataScaleHandler....

		restrctns.set("msar powerscale", itsPowerScaleHandler.cycles());

        restrctns.set("msar spectralunit", itsParamSpectralUnit->value( ));

		//# investigate colormap restriction handling in LAR.  Colormap state
		// should enter into these restrictions too--how?
		// Currently, we just empty cache in CDD:refEH() (as in PADD::refEH()),
		// on any ColorTableChange event, which is probably overkill.

		// String cmname="default";
		// if(colormap()!=0) cmname=colormap()->name();
		// restrctns.set("msar colormap", cmname);

		return restrctns;
	}


//--------------------------------------------------------------------------
// MSAsRaster::handleEvent(DisplayEvent&) could (probably should) be
// implemented in terms of this function... <drs>
//--------------------------------------------------------------------------
	bool MSAsRaster::flag( WorldCanvas *wc, double blc_x, double blc_y, double trc_x, double trc_y ) {
		conformsTo(wc);
		if(!rstrsConformed_ || !csConformed_) return false;
		if(!msselValid_) return false;	// No data--don't attempt to flag.

		Int ix0 = max( 0, ifloor(blc_x+.5) );
		Int ix1 = min( msShape_[axisOn_[X]], ifloor(trc_x+.5)+1 );
		Int iy0 = max( 0, ifloor(blc_y+.5) );
		Int iy1 = min( msShape_[axisOn_[Y]], ifloor(trc_y+.5)+1 );

		addEdit_(wc, ix0, (ix0 == ix1 ? 1 : ix1-ix0), iy0, (iy0 == iy1 ? 1 : iy1-iy0));
		return true;
	}

//--------------------------------------------------------------------------
	void MSAsRaster::handleEvent(DisplayEvent& ev) {
		// Process user selection of flagging region.  These (currently)
		// come from the crosshair or rectangular region mouse tools
		// (MWCCrosshairTool or MWCRTRegion).

		ActiveCaching2dDD::handleEvent(ev);	// Give base class[es] a chance
		// to handle the event too.



		CrosshairEvent* chev = dynamic_cast<CrosshairEvent*>(&ev);
		if(chev != 0) {

			WorldCanvas* wc = chev->worldCanvas();
			conformsTo(wc);
			if(!rstrsConformed_ || !csConformed_) return;
			// test applicability to canvas.  If we're not in charge of it,
			// don't respond to event.  (We may still take certain actions
			// like mouse tool erase/refresh if zIndex is out of range).

			if(chev->evtype() == "up") {
				// ignore the button release events, except to erase crosshair.
				ResetCrosshairEvent ev;
				wc->handleEvent(ev);
				return;
			}

			if(!msselValid_) return;	// No data--don't attempt to flag.

			// press or motion of crosshair: flag point selected

			Int nx=msShape_[axisOn_[X]], ny=msShape_[axisOn_[Y]];

			Vector<Double> pix(2), lin(2);
			pix(0) = chev->pixX();
			pix(1) = chev->pixY();
			chev->worldCanvas()->pixToLin(lin, pix);
			Int ix = ifloor(lin(0)+.5), iy = ifloor(lin(1)+.5);

			if(ix>=0 && ix<nx  &&  iy>=0 && iy<ny) {

				addEdit_(wc, ix,1, iy,1);
			}

			return;
		}



		RectRegionEvent* rev = dynamic_cast<RectRegionEvent*>(&ev);
		if(rev != 0) {

			// (pseudo)region selected by rectangle mouse tool.

			WorldCanvas* wc = rev->worldCanvas();
			conformsTo(wc);
			if(!rstrsConformed_ || !csConformed_) return;
			// test applicability to canvas.  If we're not in charge of it,
			// don't respond to event.  (We may still take certain actions
			// like mouse tool erase/refresh if zIndex is out of range).

			if(!msselValid_) {			// No data--don't attempt to flag
				ResetRTRegionEvent ev;
				wc->handleEvent(ev);		// (erase rectangle).
				return;
			}

			// massage event coordinates into proper form/type
			// (and clip to msShape_ for safety...)

			Vector<Double> pixb(2), pixt(2), linb(2), lint(2);
			for(Int i=0; i<2; i++) {
				pixb(i)=rev->pixBlc()(i);
				pixt(i)=rev->pixTrc()(i);
			}
			wc->pixToLin(linb, pixb);
			wc->pixToLin(lint, pixt);
			Int ix0 = max( 0, ifloor(linb(0)+.5) );
			Int ix1 = min( msShape_[axisOn_[X]], ifloor(lint(0)+.5)+1 );
			Int iy0 = max( 0, ifloor(linb(1)+.5) );
			Int iy1 = min( msShape_[axisOn_[Y]], ifloor(lint(1)+.5)+1 );

			if(ix0<ix1 && iy0<iy1) {
				ResetRTRegionEvent ev = ResetRTRegionEvent(True);
				wc->handleEvent(ev);	// (erase rectangle; I'll refresh).

				addEdit_(wc, ix0,ix1-ix0, iy0,iy1-iy0);
			}

			else {
				ResetRTRegionEvent ev;
				wc->handleEvent(ev);
			};	// (erase rectangle, and you refresh).

			return;
		}
	}


//--------------------------------------------------------------------------
	void MSAsRaster::addEdit_(WorldCanvas* wc,
	                          Int xStart, Int xShape,
	                          Int yStart, Int yShape) {
		// Add the edit request that came in from the rectangle or crosshair
		// mouse tool to the flagEdits_ List.
		// Then cause the necessary display matrices to be updated and displayed.


		// Fetch position on animator axis for the wc of the event (could have
		// used activeZIndex_, instead of getting it from the wch again).

		WorldCanvasHolder *wch = findHolder(wc);
		if(wch==0) return;			//shouldn't happen.
		Int zStart;
		Bool zStartExists;
		const AttributeBuffer *wchRestrs = wch->restrictionBuffer();
		zStartExists = wchRestrs->getValue("zIndex",zStart);

		if(!zStartExists || zStart<0 || zStart>=msShape_[axisOn_[Z]]) {
			PixelCanvas *pc = wc->pixelCanvas();
			pc->copyBackBufferToFrontBuffer();
			pc->setDrawBuffer(Display::FrontBuffer);
			pc->callRefreshEventHandlers(Display::BackCopiedToFront);
			return;
		}
		// (should happen only on multipanels, off the end of the
		// 'filmstrip'....  Need pc front-buffer refresh here too,
		// just to erase the mouse tool).

		// Store parameters for this edit and add to end of edit list.

		FlagEdit_* edit = new FlagEdit_(this);
		edit->start[axisOn_[X]] = xStart;
		edit->shape[axisOn_[X]] = xShape;
		edit->start[axisOn_[Y]] = yStart;
		edit->shape[axisOn_[Y]] = yShape;
		edit->start[axisOn_[Z]] = zStart;
		edit->start[axisOn_[SL0]] = pos_[axisOn_[SL0]];
		edit->start[axisOn_[SL1]] = pos_[axisOn_[SL1]];

		edit->unflag = unflag_;
		edit->all = flagAll_;
		edit->entireAnt = entireAnt_;
		for(Axis ax=0; ax<NAXES; ax++) if(flagAll_[ax]) {
				edit->start[ax] = 0;
				edit->shape[ax] = msShape_[ax];
			}

		ListIter<void*> edits(flagEdits_);
		edits.toEnd();

		// Check that this edit doesn't exactly repeat the last one.
		// This better supports the crosshair flagger tool,
		// which can be dragged to flag everything in its path.

		if(!edits.atStart()) {
			edits--;
			FlagEdit_* lastedit = static_cast<FlagEdit_*>(edits.getRight());
			if(*lastedit==*edit) {
				delete edit;

				// we're ignoring this edit, but may still need to erase the mouse tool
				// (which is already reset but not redrawn) via front-buffer refresh.
				// (all the following should be what pc->refresh(FrontBuffer) does....).
				PixelCanvas *pc = wc->pixelCanvas();
				pc->copyBackBufferToFrontBuffer();
				pc->setDrawBuffer(Display::FrontBuffer);
				pc->callRefreshEventHandlers(Display::BackCopiedToFront);

				return;
			}

			edits.toEnd();
		}

		// Go ahead and add new edit to the list.

		edits.addRight(edit);

		purgeCache();		// (not worth keeping old drawings and trying to
		// determine if any apply to current edit state).

		// Safest (although occasionally inefficient) to refresh all canvases
		// where this DD is registered.

		refresh();
	}


//--------------------------------------------------------------------------
	void MSAsRaster::postEditsToDisp_() {
		// Post new user flag commands to the display matrices.

		// Old flags retrieved from the MS are stored in a bitmapped array
		// corresponding to the shape and size of vis_.  These are copied
		// (in createDisplaySlice_) directly to the the display matrices which
		// are drawn on the WC.
		// In contrast, new user flag commands (as yet unsaved to the MS)
		// are stored as a list of the regions chosen and flagging range options
		// in effect for each (see addEdit_() and struct FlagEdit_).
		// dispNEdits_ contains the number of edit commands already reflected
		// in the display matrices.  This routine interprets any edits
		// beyond that point and updates the display matrices accordingly.
		// In other respects, the display matrices should already be in sync with
		// user input on entry to this routine.

		// NB: This routine is for display of the new edits only.  saveEdits_()
		// is called to save the edits to the MS on disk.

		if(dispNEdits_ >= flagEdits_.len()) return;
		// up-to-date: nothing to do (should actually be == in this case).

		ListIter<void*> edits(flagEdits_);

		uInt i=0;
		for(edits.toStart(); !edits.atEnd(); i++, edits++) {
			if(i<dispNEdits_) continue;	   // skip past previously posted edits.
			FlagEdit_& edit = *static_cast<FlagEdit_*>(edits.getRight());

			// First check whether the edit applies to the current display slice
			// (chosen via animator and sliders).

			Bool applies=True;
			for(Axis ax=0; ax<NAXES; ax++) {
				if(ax==dispX_ || ax==dispY_) continue;	// (don't check display axes)
				if(!edit.appliesTo(ax, dispPos_[ax])) {
					applies=False;
					break;
				}
			}  // edit doesn't apply to this display
			if(!applies) continue;	     // slice--go on to next edit.

			// Edit applies to this slice.
			// determine region (box) of slice to [un]flag.

			Int startX, endX, startY, endY;

			edit.getLoopRange(dispX_, startX,endX);
			edit.getLoopRange(dispY_, startY,endY);

			startX=max(0,min(msShape_[dispX_], startX));   // trim to shape
			endX=  max(0,min(msShape_[dispX_], endX));	   // (just for insurance--
			startY=max(0,min(msShape_[dispY_], startY));   // should already be
			endY=  max(0,min(msShape_[dispY_], endY));     // within bounds).

			// [un]flag the relevant regions of display matrices.

			// (ifs are outside the loops for efficiency).

			// The user can choose to display flags in color or simply masked
			// to background.  In the latter case, dispMask_ is set to False for
			// flagged data so that the data pixel is masked out.  In the color case,
			// the data matrix is drawn first without a mask, then the color-coded
			// (and masked) flag matrix is drawn on top of that. In that case, the
			// mask matrix has the opposite 'polarity'--False to show the good data
			// underneath.

			if(!edit.unflag) {      			// flag the region:
				if(!dispFlagsInClr_) {
					for(Int ix=startX; ix<endX; ix++) if(edit.applies2(dispX_, ix)) {
							for(Int iy=startY; iy<endY; iy++) if(edit.applies2(dispY_, iy)) {
									dispMask_(ix,iy) = False;
								}
						}
				}	// mask out the pixel...
				else {
					for(Int ix=startX; ix<endX; ix++) if(edit.applies2(dispX_, ix)) {
							for(Int iy=startY; iy<endY; iy++) if(edit.applies2(dispY_, iy)) {
									if (disp_(ix,iy)!=NO_DATA) {
										dispFlags_(ix,iy) = NEWFLAG;	// or show in 'newflag' color
										dispMask_(ix,iy) = True;
									}
								}
						}
				}
			}

			else {	// unflag--revert to either 'good data', 'no data',
				// or 'not loaded' pixels, depending on the state of
				// underlying data display matrix (disp_).
				if(!dispFlagsInClr_) {
					for(Int ix=startX; ix<endX; ix++) if(edit.applies2(dispX_, ix)) {
							for(Int iy=startY; iy<endY; iy++) if(edit.applies2(dispY_, iy)) {
									Float d = disp_(ix,iy);
									dispMask_(ix,iy) = (d!=NO_DATA && d!=NOT_LOADED);
								}
						}
				} else {
					for(Int ix=startX; ix<endX; ix++) if(edit.applies2(dispX_, ix)) {
							for(Int iy=startY; iy<endY; iy++) if(edit.applies2(dispY_, iy)) {
									Float d = disp_(ix,iy);
									if (d==NO_DATA) {
										dispFlags_(ix,iy) = NODATA;
										dispMask_(ix,iy) = True;
									} else if (d==NOT_LOADED) {
										dispFlags_(ix,iy) = NOTLOADED;
										dispMask_(ix,iy) = True;
									} else {
										dispMask_(ix,iy) = False;
									}
								}
						}
				}
			}
		}

		if(dispX_==TIME || dispY_==TIME) dispDevValid_=False;
		// New edits invalidate the deviation Matrix if Time is on display.

		dispNEdits_ = flagEdits_.len();
	}  // disp_ is up-to-date with edits now.



//--------------------------------------------------------------------------
	Bool MSAsRaster::saveEdits_() {
		// Save new edits permanently to the MS.  The return value indicates
		// whether there were any edits to save.


		ListIter<void*> edits(flagEdits_);
		Int nEdits=edits.len();
		if(nEdits==0 || !msselValid_) return False;


		Bool entireMS = (itsEditEntireMS->value() == "Yes" && mssel_!=itsMS);
		// indicates need for additional logic to extend edits
		// beyond the selected portion of the MS.


		// For progress feedback.

		Timer tmr;
		cerr<<endl<<"Saving "<<nEdits<<" flagging edit";
		if(nEdits>1) cerr<<"s";
		cerr<<"  "<<flush;
		Double msRows = (entireMS)? itsMS->nrow() : mssel_->nrow();
		Double pctDone = 100./max(1., msRows);
		Double intvl=4.;
		Int iRow=0;


		// Collect a Vector of the time slots (and Matrix of time-baseline
		// combinations) actually involved in some edit, in an effort
		// to save time iterating through the (time-ordered) chunks.

		Int nTime=msShape_[TIME], nBsl=msShape_[BASELN];
		Vector<Bool> tused(nTime, False);
		Matrix<Bool> tbused(nTime, nBsl, False);
		Bool alltused = False;	// whether some edit applies to all times.
		Vector<Bool> usedallt(nBsl, False);
		// indicates baselines applicable to some edit in which
		// _all_ times apply.

		for(edits.toStart(); !edits.atEnd(); edits++) {
			FlagEdit_& edit = *static_cast<FlagEdit_*>(edits.getRight());
			if(edit.all[TIME]) {
				if(!alltused) tused=True;  // Set tused all True just for consistency
				alltused=True;		 // (it won't be needed in this case).
				Int bs, be;
				edit.getLoopRange(BASELN, bs,be);
				for(Int b=bs; b<be; b++) if(edit.appliesTo(b)) usedallt[b]=True;
			}

			else {
				Int ts, te;
				edit.getSureRange(TIME, ts,te);
				if(!alltused) for(Int t=ts; t<te; t++) tused[t] = True;

				Int bs, be;
				edit.getLoopRange(BASELN, bs,be);
				for(Int b=bs; b<be; b++) if(edit.appliesTo(b)) {
						for(Int t=ts; t<te; t++) tbused(t,b)=True;
					}
			}
		}

		Double maxTime = INVALID;	// last time which is involved in any edit.
		if(!alltused) {		// (maxTime will be used only in this case).
			Int tmax;
			for(tmax=nTime-1; tmax>=0; tmax--) if(tused[tmax]) break;
			if(tmax<0) {
				undoEdits_("all");    // (shouldn't happen)
				return False;
			}
			maxTime=time_[tmax];
		}



		// Iterate the MS, storing the new flag/unflag edits.

		VisibilityIterator &vi( *((entireMS)? vs_ : wvi_p) );	// Iterate either entire MS or

		VisBuffer vb(vi);
		Cube<Bool> flg;
		Vector<Bool> flgRow;
		MSpos_ mpos(this);	// Used for translating MS values (time, antennas,
		// etc.) to the hypercube indices (time slot,
		// baseline index, etc.) used by the edits.

		for (vi.originChunks(); vi.moreChunks(); vi.nextChunk() ) {

			Int iRowChunk=iRow;		// # rows processed prior to this chunk

			// Before iterating within chunk, check that at least some edit
			// applies to the spectral window and range of pols and channels
			// implied by this chunk's dataDescription_ID.  Also, save some state
			// in the edits indicating which do apply to this chunk, and to which
			// pol and channel ranges within the MS's FLAG column cells.

			Bool doneChunk=True;	// signals that we're finished (early)
			// with this chunk.

			Int pol0  = mpos.p0(vi.polarizationId());
			Int nPol  = vi.visibilityShape()(0);
			Int spw   = mpos.s(vi.spectralWindow());
			Int nChan = vi.visibilityShape()(1);

			for(edits.toStart();  !edits.atEnd();  edits++) {
				FlagEdit_& edit = *static_cast<FlagEdit_*>(edits.getRight());

				if(edit.appliesToChunk(pol0,nPol, spw,nChan)) doneChunk=False;
			}
			// This edit will apply within the chunk, to any rows with
			// appropriate times/baselines; it applies to the chunk's
			// spectral window, and to at least some of its pols and
			// channels.

			if(doneChunk) {	// No edits apply to chunk.
				iRow = iRowChunk + vi.nRowChunk();
				continue;
			}	// ... to next chunk.



			// Row blocking for flags is similar to that for data in extract_(),
			// but will use less memory.  This will retrieve approximately as
			// many rows, up to the chunk limit.  Because flagging is scattered,
			// this may also avoid retrieving some unneeded flags.  We're
			// retrieving time, flagrow, feed1 or ant1,ant2, and flags.


			Int rowBytes = sizeof(Double) + sizeof(Bool)   +
			               ((nAnt_==1)? 1 : 2)*sizeof(Int) +
			               nPol*nChan*sizeof(Bool);
			Double iterMemory = max(102., .001*HostInfo::memoryTotal()) * 1024.;
			// Allow use of up to 0.1% of memory or 0.1Mb here.

			Int nBlkRows = Int(min(1.e6, max(100., iterMemory/rowBytes )));
			vi.setRowBlocking(nBlkRows);


			// Times are sorted (only) within each chunk.
			// Reset search index into time_ Vector.

			mpos.time = INVALID;	// Last time found in chunk.
			Int tm = INVALID;		// Corresponding index into hypercube.
			Int ts = INVALID,		// Bracketing timeslots (for use when
			    te = INVALID;		// time is not in selected MS).

			for (vi.origin(); vi.more(); vi++) {

				Int nRow=vb.nRow();
				iRow += nRow;

				Bool flgsRetrieved=False, writeFlgs=False;


				for (Int row=0; row<nRow; row++) {

					if(tmr.real()>intvl) {
						Int pct = Int((iRow+row)*pctDone);
						if(pct>0) cerr<<pct<<"%  "<<flush;
						tmr.mark();
						intvl++;
					}		// progress feedback


					// Fetch hypercube indices for time (tm) and baseline (bsl) of row.
					// (At present, bsl should always be valid, although tm may not be).
					// Skip out if we are sure no edit applies to tm and bsl.
					// (Rather complex logic -- take care if you alter this).

					Double time = vb.time()[row];
					if(!alltused && time>maxTime) {
						doneChunk=True;
						break;
					}
					// Beyond times used in any edit: break out of
					// row iterations (to flag writing, if needed).

					if(mpos.time!=time) {	// new time.
						tm = mpos.t(time);	// (sets mpos.time, mpos.ts and mpos.te well).
						ts = mpos.ts;
						te = mpos.te;
					}
					// bracketing timeslots  (An INVALID time (not in
					// selected ms) may still be edited, if it is
					// within the time bracket of some edit).

					if( !alltused &&
					        (tm!=INVALID?	   !tused[tm] :
					         ts<0 || !tused[ts] ||
					         te>=msShape_[TIME] || !tused[te]) ) continue;
					// no edits on this time slot (or even on the
					// bracketing slots of an unselected time).


					Int bsl = (nAnt_==1)? mpos.b(vb.feed1()[row]) :
					          mpos.b(vb.antenna1()[row], vb.antenna2()[row]);
					// single ant: 'bsl' index is feed number,
					// otherwise derived from antennas.
					if(bsl==INVALID) continue;	// (shouldn't happen).

					if( !usedallt[bsl] &&
					        (tm!=INVALID?	   !tbused(tm,bsl) :
					         ts<0 || !tbused(ts,bsl) ||
					         te>=msShape_[TIME] || !tbused(te,bsl)) ) continue;
					// no edits on this time-baseline combination.


					for(edits.toStart();  !edits.atEnd();  edits++) {

						FlagEdit_& edit = *static_cast<FlagEdit_*>(edits.getRight());
						if(!edit.appChunk) continue;

						// edit applies to current Sp.Win. and at least some of the
						// row's pols and channels.

						if(!edit.appliesTo(time)) continue;		// time N/A to edit
						if(!edit.appliesTo(BASELN, bsl)) continue;	// baseln N/A to edit

						// We have an applicable edit-- retrieve the flags if not done yet.

						if(!flgsRetrieved) {
							vi.flag(flg);
							vi.flagRow(flgRow);
							flgsRetrieved = True;
						}

						// Write any flag changes into flag cube.  (Note: pol and chan
						// edit ranges are for the _MS row_, and were pre-calculated at
						// the start of the chunk for each edit, in edit.appliesToChunk()).

						Bool newflag = !edit.unflag;
						for (Int pol=edit.sPol; pol<edit.ePol; pol++) {
							for (Int chan=edit.sChan; chan<edit.eChan; chan++) {
								if(newflag != flg(pol,chan,row)) {   // flag has changed.
									writeFlgs = True;

									flg(pol,chan,row) = newflag;
								}
							}
						}
					}
				}


				if(writeFlgs) {

					// There were some flag changes within this vi minor iteration:
					// write them out to the MS.

					vi.setFlag(flg);

					// update FLAG_ROW too (logical AND of all flags in the MS row).

					for (Int row=0; row<nRow; row++) {

						Bool fr=True;
						for (Int pol=0; pol<nPol; pol++) {
							for (Int chan=0; chan<nChan; chan++) {
								if(!flg(pol,chan,row)) {
									fr=False;
									break;
								}
							}
							if(!fr) break;
						}
						flgRow(row) = fr;
					}

					vi.setFlagRow(flgRow);
				}


				if(doneChunk) break;
			}		// ( ...out of vi minor iterations).

			iRow = iRowChunk + vi.nRowChunk();
		}	// for(vi Chunk iterations)


//vs->flush();				// VisibilityIterator/VisBuffer have no flush( )
		mssel_->relinquishAutoLocks(True);	// (just to be sure).
		itsMS->relinquishAutoLocks(True);	// (just to be sure).

		// Post new edits to internal Flags_ too--avoids having
		// to call extract_ again.

		IPosition slot(NAXES);  // index into vis_ and flag arrays.
		Block<Int> s(NAXES), e(NAXES);
		for(edits.toStart(); !edits.atEnd(); edits++) {
			FlagEdit_& edit = *static_cast<FlagEdit_*>(edits.getRight());
			for(Axis ax=0; ax<NAXES; ax++) {
				edit.getLoopRange(ax, s[ax],e[ax]);
				s[ax] = max(s[ax]-visStart_[ax], 0);
				e[ax] = min(e[ax]-visStart_[ax], visShape_[ax]);
			}
			// Don't try to post edits beyond current vis_ limits.
			// (and adjust loop ranges relative to visStart_)

			Bool newflag = !edit.unflag;

			for(Int bsl=s[BASELN]; bsl<e[BASELN]; bsl++)
				if(edit.appliesTo(bsl+visStart_[BASELN])) {
					slot(BASELN)=bsl;

					if(tmr.real()>1.) {
						cerr<<"."<<flush;
						tmr.mark();
					}		// progress feedback

					for(Int tm=s[TIME]; tm<e[TIME]; tm++) {
						slot(TIME)=tm;
						for(Int spw=s[SP_W]; spw<e[SP_W]; spw++) {
							slot(SP_W)=spw;
							for(Int pol=s[POL]; pol<e[POL]; pol++) {
								slot(POL)=pol;
								for(Int chn=s[CHAN]; chn<e[CHAN]; chn++) {
									slot(CHAN)=chn;
									if(vis_(slot) != NO_DATA) {

										setFlag_(slot, newflag);
									}
								}
							}
						}
					}
				}
		}

		// Discard (saved) edits and old display images.

		undoEdits_("all");

		cerr<<"  Done."<<endl<<endl;	// progress feedback.

		// return value advises caller to refresh all canvases on which this DD
		// is active, in order to show all flags in the 'saved' color.

		return True;
	}



//--------------------------------------------------------------------------
	void MSAsRaster::computeTimeBoxcars_() {
		// compute the lsTime_ and leTime_ vectors, which define the
		// 'local neighborhood' around each time slot, for running averages.
		// The neighborhood for slot t will be the loop values (tint) in
		//   "for (tint=lsTime_[t]; tint<leTime_[t]; tint++)"
		// Normally this is just an interval of nDAvg_ slots centered around slot t,
		// i.e.  leTime_[t] = t + (nDAvg_+1)/2  and  lsTime_[t] = leTime_[t] - nDAvg_.
		// However, adjustments are made for boundaries between scans or fields,
		// and for beginning and ending time slots (for example, lsTime_[0] and
		// lsTime_[1] will both be 0).

		Int nTimes = msShape_[TIME];
		lsTime_.resize(nTimes);
		leTime_.resize(nTimes);
		Int n = (nDAvg_-1)/2;

		for(Int t=0; t<nTimes; t++) {
			Int fld=field_[t];
			Int scn=scan_[t];

			Int et=t+1;
			while(et<nTimes && et<=t+n && field_[et]==fld && scan_[et]==scn) et++;

			Int st=t-1;
			while(st>=0 && st>=et-nDAvg_ && field_[st]==fld && scan_[st]==scn) st--;
			st++;

			while(et<nTimes && et<st+nDAvg_ && field_[et]==fld && scan_[et]==scn) et++;

			lsTime_[t]=st;
			leTime_[t]=et;
		}

		// lsvTime_ and levTime_ are essentially the same as lsTime_
		// and leTime_, but are relative to time-slot indexing in vis_
		// (i.e., relative to visStart_[TIME]) and are only as long as
		// the vis_ time axis (visShape_[TIME]).

		Int nvTimes = visShape_[TIME], vsTime=visStart_[TIME];
		lsvTime_.resize(nvTimes);
		levTime_.resize(nvTimes);
		for(Int t=0; t<nvTimes; t++) {
			lsvTime_[t] = max(0,       lsTime_[t+vsTime]-vsTime);
			levTime_[t] = min(nvTimes, leTime_[t+vsTime]-vsTime);
		}
	}



//--------------------------------------------------------------------------
	Float MSAsRaster::v_(Int t) {
		// return a single visibility point from vis_ or disp_, as a function
		// of time slot only.  The row of times was predetermined by setting
		// useVis_, dPos_ and axlTm_.  goodData_ is set True if the data exists,
		// is loaded and is not flagged (set False otherwise).

		dPos_[axlTm_] = t;

		if(useVis_) {
			Float v = vis_(dPos_);
			goodData_ = (v>NO_DATA && !flag_(dPos_));
			return v;
		}

		goodData_ = (dispMask_(dPos_)!=flagsInClr_);
		return disp_(dPos_);
	}



//--------------------------------------------------------------------------
	Float MSAsRaster::dev_(Int t) {
		// return the visibility deviation for the time slot t.  visDev_
		// determines whether this is an RMS deviation or simple absolute
		// difference from the running mean.  Maintains state from the
		// prior calculation to speed things up in some cases.

		Float vt = v_(t);
		if(vt<=NO_DATA) return vt;
		if(!goodData_ && !flgdDev_) return INSUF_DATA;
		// The (usual) !flgdDev_ option means 'Don't bother computing
		// deviation for a flagged point--just return INSUF_DATA'.

		Int st, et;		// new neighborhood bounds.  Bounds from last
		// computation are [sT_,eT_)  (if sT_>=0).
		if(useVis_) {
			st = lsvTime_[t];
			if(st==sT_ && visDev_==RMS) return d_;
			et = levTime_[t];
		} else {
			st = lsTime_[t];
			if(st==sT_ && visDev_==RMS) return d_;
			et = leTime_[t];
		}


		// Phase deviations are calculated two ways; the minimum of these
		// is the returned value.  This is so that phases clustered around
		// +-180 degrees do not show artificially high deviations.

		Float vta=0., va;
		Bool phase = (visComp_==PHASE);
		if(phase) vta = vAlt_(vt);


		if(sT_>=0 && eT_-st > st-sT_ + 1) {

			// Worth it to recycle old sums (subtracting terms for t in
			// [sT_,st), rather than re-summing for t in [st,eT_) ).
			// (Note that we always have st>=sT_ && et>=eT_ here, because
			// this routine is to be called with increasing t, after sT_ is
			// initialized to -1).
			// Such reuse occurs only when flgdDev_==False; no flagged
			// points will be included in these incrementally-modified sums.

			for(Int it=sT_; it<st; it++) {
				Float v = v_(it);
				if(goodData_) {
					nValid_--;
					sumv_-=v;
					if(visDev_==RMS) sumv2_-=Double(v)*v;
					if(phase) {
						va = vAlt_(v);
						sumva_-=va;
						if(visDev_==RMS) sumv2a_-=Double(va)*va;
					}
				}
			}

			// (We must add in the new terms (for t in [eT_,et)) in all cases).

			for(Int it=eT_; it<et; it++) {
				Float v = v_(it);
				if(goodData_) {
					nValid_++;
					sumv_+=v;
					if(visDev_==RMS) sumv2_+=Double(v)*v;
					if(phase) {
						va = vAlt_(v);
						sumva_+=va;
						if(visDev_==RMS) sumv2a_+=Double(va)*va;
					}
				}
			}
		}

		else {

			// Recompute sums from scratch instead, discarding old ones.
			// The sums are always initialized with vt (even though vt itself
			// _might_ be flagged, if flgdDev_ is True);

			nValid_ = 1;
			sumv_=vt;
			sumva_=vta;
			sumv2_=Double(vt)*vt;
			sumv2a_=Double(vta)*vta;

			for(Int it=st; it<et; it++) if(it!=t) {
					Float v = v_(it);
					if(goodData_) {	// (All _other_ points in sum _will_ be unflagged).
						nValid_++;
						sumv_+=v;
						if(visDev_==RMS) sumv2_+=Double(v)*v;
						if(phase) {
							va = vAlt_(v);
							sumva_+=va;
							if(visDev_==RMS) sumv2a_+=Double(va)*va;
						}
					}
				}
		}

		if(!flgdDev_) {
			sT_=st;
			eT_=et;
		}
		// Save latest neighborhood bounds [st,et).

		if(nValid_<2) d_ = INSUF_DATA;	// not enough valid data in nbhd.

		else if(visDev_!=RMS) {
			d_ = abs(vt - (sumv_/nValid_));
			// absolute deviation from running mean.
			if(phase) d_ = min( d_, abs(vta - (sumva_/nValid_)) );
		}

		else {

			// RMS deviation

			Double sumdev2 = sumv2_ - sumv_*sumv_/nValid_;

			if(sumdev2<0) {
				d_ = INSUF_DATA;		// Computation got funky--discard
				sT_ = -1;
			}		// (shouldn't happen, except perhaps
			// through roundoff error).

			else if(phase) {
				Double sumdev2a = sumv2a_ - sumva_*sumva_/nValid_;
				if(sumdev2a<sumdev2 && sumdev2a>=0) sumdev2 = sumdev2a;
			}
			// if alternate computation smaller, use it instead.

			d_ = sqrt(sumdev2/(nValid_-1));
		}

		return d_;
	}



//--------------------------------------------------------------------------
	void MSAsRaster::computeDevRange_() {
		// Calculate the range of the deviations, over all of vis.  Used to set
		// data scaling sliders when visibility deviations are to be displayed.
		// Input: valid vis_ (& corresp. visShape_)
		// Output: devRngMin_, devRngMax_  (a lot of calculation for 2 numbers...)

		if(visDev_<=NORMAL) return;	 // (probably won't be called in this
		// case anyway...)
		devRngMin_ = devRngMax_ = 0.;  // deviations will be non-negative.

		Double sumdev=0., sumdev2=0.;
		Int ndev=0;
		// For 3-sigma trim of the computed ranges (see extract_).
		// (Not to be confused with computation of the RMS values
		// themselves, although this also does a calculation
		// of std. deviation; this makes sure the _range_ of
		// RMS's (or abs. deviations) which is set on the
		// sliders aren't too wild).


		// Initialize the input to the dev_() routine, which calculates the
		// individual deviations.

		useVis_ = True;
		flgdDev_ = False;
		dPos_.resize(NAXES);
		axlTm_ = TIME;

		Int vnTime=visShape_[TIME], vnBsln=visShape_[BASELN],
		    vnChan=visShape_[CHAN], vnPol=visShape_[POL], vnSpw=visShape_[SP_W];

		Double visSize = vnTime*vnBsln*vnChan*vnPol*vnSpw;
		if(visSize<100000.) {

			// Small enough to compute quickly using all of vis_.

			for(Int bsl=0; bsl<vnBsln; bsl++) {

				if(nAnt_!=1) {
					Int b = bsl+visStart_[BASELN];	// ('actual' bsln index..)
					Int ant1=a1_(b), ant2=a2_(b);

					if(ant2>=nAnt_ || (ant1==ant2 && !dish_))  continue;
				}
				// skips baseline 'gaps' and autocorrs, if not single-dish.

				dPos_[BASELN]=bsl;
				for(Int chn=0; chn<vnChan; chn++) {
					dPos_[CHAN]=chn;
					for(Int pol=0; pol<vnPol; pol++) {
						dPos_[POL]=pol;
						for(Int spw=0; spw<vnSpw; spw++) {
							dPos_[SP_W]=spw;

							sT_=-1;	// signal dev_ to discard previous calculations
							// at start of new row of times.

							for(Int t=0; t<vnTime; t++) {

								Float dev = dev_(t);

								if(dev>NO_DATA) {		// valid deviation data

									if(dev>devRngMax_) devRngMax_=dev;
									sumdev += dev;
									sumdev2 += dev*dev;
									ndev++;
								}
							}
						}
					}
				}
			}
		}

		else {

			// vis_ is so big it's not worth reading it all just to compute one
			// number.  Random sample it instead.

			MLCG m;
			DiscreteUniform randT(&m, 0, vnTime-1), randB(&m, 0, vnBsln-1),
			                randC(&m, 0, vnChan-1), randP(&m, 0, vnPol-1),
			                randS(&m, 0, vnSpw-1);

			for(Int i=0; i<15000; i++) {

				Int bsl=randB.asInt();
				if(nAnt_!=1) {
					Int b = bsl+visStart_[BASELN];	// ('actual' bsln index..)
					Int ant1=a1_(b), ant2=a2_(b);

					if(ant2>=nAnt_ || (ant1==ant2 && !dish_))  continue;
				}
				// skips baseline 'gaps' and autocorrs, if not single-dish.

				dPos_[BASELN]=bsl;
				dPos_[CHAN]=randC.asInt();
				dPos_[POL]=randP.asInt();
				dPos_[SP_W]=randS.asInt();
				Int t=randT.asInt();
				sT_=-1;

				Float dev = dev_(t);

				if(dev>NO_DATA) {		// valid deviation data

					if(dev>devRngMax_) devRngMax_=dev;
					sumdev += dev;
					sumdev2 += dev*dev;
					ndev++;
				}
			}
		}


		if(ndev>=2) {
			Double devbar = sumdev/ndev;
			Double sumdev2dev = sumdev2 - sumdev*sumdev/ndev;
			if(sumdev2dev>0) {
				Double devsig = sqrt(sumdev2dev/(ndev-1));
				devRngMax_ = min(Float(devbar + 3*devsig), devRngMax_);
			}
		}
	}



//--------------------------------------------------------------------------
	void MSAsRaster::createDevSlice_() {
		// Create/fill dispDev_ Matrix for displaying deviations.  Both this
		// routine and the one above initialize and use dev_() to calculate
		// individual deviations.
		// Input: valid Disp_, DispMask_ and associated meta-information,
		//        up to date with user input.  This is achieved via
		//        calls to createDisplaySlice_ and postEditsToDisp_ prior
		//	    to calling this routine.

		Int nx=msShape_[dispX_], ny=msShape_[dispY_];
		dispDev_.resize(nx,ny);

		if(dispNotLoaded_) dispDev_ = NOT_LOADED;
		// Easy case where none of the current slice is loaded into vis_.

		else {

			Int vsX = visStart_[dispX_],     vsY = visStart_[dispY_];
			Int veX = vsX+visShape_[dispX_], veY = vsY+visShape_[dispY_];

			if(vsX>0 || vsY>0 || veX<nx || veY<ny) dispDev_ = NOT_LOADED;
			// initialize dispDev_, when at least _some_ of it should
			// have the NOT_LOADED value (unnecessary otherwise).

			// These are 2 major cases here (distinguished by useVis_).

			useVis_ = (dispX_ != TIME && dispY_ != TIME);

			if(!useVis_) {

				// Case in which the time axis is on display--all the data needed
				// to do the deviation calculations are in disp_ and dispMask_.

				dPos_.resize(2);	// dPos_ will index 2D disp_ Matrix in this case.
				flgdDev_=False;	// No need to calculate flagged values

				AxisLoc axlOther;
				Int vsTime,veTime, vsOth,veOth;
				if(dispX_ == TIME) {
					axlTm_=X;
					vsTime=vsX;
					veTime=veX;	// time axis is on X
					axlOther=Y;
					vsOth=vsY;
					veOth=veY;
				}	// (other display axis is
				else {					//  on Y)
					axlTm_ = Y;
					vsTime=vsY;
					veTime=veY;	// time axis is on Y
					axlOther = X;
					vsOth=vsX;
					veOth=veX;
				}

				for(Int oth=vsOth; oth<veOth; oth++) {
					dPos_[axlOther] = oth;
					sT_=-1;	// signal dev_ to discard previous calculations
					// at start of new row of times.

					for(Int t=vsTime; t<veTime; t++) {

						Float dev = dev_(t);
						dPos_[axlTm_] = t;
						dispDev_(dPos_) = dev;
					}
				}
			}
			// (dPos_ can serve to index the destination Array too).


			else {

				// In this case (not likely to be used often), time is not on a
				// display axis.  We must go to vis_ for the data necessary for
				// the deviation calculations.  This is slower, and also has the
				// drawback that new, unsaved edits cannot be taken into account,
				// because they are never computed for the entire vis_ Array.

				dPos_.resize(NAXES);
				flgdDev_ = True;	// This keeps the color of a newly-unflagged
				// point in synch with its position-tracking
				// value even before the edit is saved, by
				// assuring that its deviation is calculated.

				for(AxisLoc control=Z; control<=SL1; control++) {
					Axis ctlAx=axisOn_(control);
					dPos_(ctlAx) = pos_[ctlAx] - visStart_[ctlAx];
				}
				// Set the (fixed) animator/slider slice position within vis.

				Int t = dPos_(TIME);	// (TIME was one of ctlAx's above--the
				// particular time slot is fixed here).
				axlTm_ = TIME;

				Int ix=vsX;
				for(dPos_[dispX_]=0; ix<veX; dPos_[dispX_]++,ix++) {
					Int iy=vsY;
					for(dPos_[dispY_]=0; iy<veY; dPos_[dispY_]++,iy++) {
						sT_=-1;	// (Must completely recalculate sums on every call).

						dispDev_(ix,iy) = dev_(t);
					}
				}
			}
		}


		dispDevValid_ = True;		// Indicate that dispDev_ is valid, for
		dispDevType_ = visDev_;	// the currently requested deviation type
		dispDevNAvg_ = nDAvg_;
	}	// and size of moving average boxcars.



//--------------------------------------------------------------------------
	String MSAsRaster::showValue(const Vector<Double> &world) {
		// for position tracking--formatted data value at given coordinate.

		if(!msselValid_) return "";	// no data (disabled).

		if(!rstrsConformed_ || !csConformed_) return "";
		// Doesn't match wch restrictions or CS state.  Probably means
		// this DD isn't the one in charge of the canvas (csMaster).
		// Blank the tracking message area in this case.

		// we need _linear_ coords for display _and_ animator axes.
		// So we convert back to lin...
		Vector<Double> lin(2);
		if(!worldToLin(lin, world)) return "Invalid ";

		IPosition pos(NAXES), vpos(NAXES);
		// The index numbering's a bit confusing:
		// * pos is relative to start of entire (selected) MS hypercube;
		//   needed to compute new flag state.  These are the standard
		//   (0-based) 'linear' or data pixel coordinates (rounded to integers).
		// * vpos is relative to start of vis_; needed for retrieving
		//   visibilities and old flags,

		for(Axis ax=0; ax<NAXES; ax++) pos(ax)=pos_[ax];
		for(AxisLoc disploc=X; disploc<=Y; disploc++) {
			Axis ax=axisOn_(disploc);
			Double lcoord=lin(disploc);
			pos(ax) = ifloor(lcoord+.5);
			// 'close the upper display edge'...(round down in this case)
			if(near(lcoord, msShape_[ax]-.5)) pos(ax)=ifloor(lcoord);
		}

		pos[axisOn_[Z]] = activeZIndex_;	// Insert latest canvas animator
		// position.  This position (or the WC[H]) should really be a parameter
		// to the method instead.  pos_[axisOn_[Z]] is not necessarily already
		// up-to-date, because the event giving rise to this call may have
		// occurred on a different canvas from the one which last set
		// pos_[axisOn_(Z)].  setActiveZIndex_(zindex) must be called prior
		// to calling this routine (as it is via the conformsTo(WCH&) call
		// in the GtkDD motion EH), so that activeZIndex_ contains the
		// correct animator position for the canvas.

		for(Axis ax=0; ax<NAXES; ax++) {
			if(pos(ax)<0 || pos(ax)>=msShape_[ax]) return "Invalid ";
		}

		String loadData = "press 'Apply' on Adjust panel to load data ";
		if(!visValid_) return loadData;
		for(Axis ax=0; ax<NAXES; ax++) {
			vpos[ax] = pos[ax] - visStart_[ax]; // (vpos is rel. to start of vis_)
			if(vpos(ax)<0 || vpos(ax)>=visShape_[ax]) return loadData;
		}
		// data not currently loaded into memory buffer (vis_).

		Float v = NO_DATA;
		if ( nPAvg_ > 1 && visDev_ == NORMAL ) {
			IPosition vpos_end(vpos);
			vpos_end[axisOn_[Z]] = min( (Int) (vpos[axisOn_[Z]] + nPAvg_-1), (Int) (vis_.shape( )(axisOn_(Z))-1) );
			const Array<Float> avga = vis_(vpos,vpos_end);
			Bool delstor = False;
			const Float *stor = avga.getStorage(delstor);
			int num = 0;
			Double sum = 0;
			for ( size_t x=0; x < avga.nelements(); ++x ) {
				if ( stor[x] != NO_DATA ) {
					sum += stor[x];
					++num;
				}
			}
			avga.freeStorage( stor, delstor );
			v = (num == 0 ? NO_DATA : (Float) (sum / (Double) num));
		} else {
			v = vis_(vpos);
		}

		if(v == NO_DATA) return "No Data ";

		String f = flag_(vpos)? "Flagged (" : "";
		ListIter<void*> edits(flagEdits_);
		for(edits.toStart(); !edits.atEnd(); edits++) {
			FlagEdit_& edit = *static_cast<FlagEdit_*>(edits.getRight());
			if(edit.appliesTo(pos)) f = edit.unflag? "" : "flagged (";
		}
		String f2 = (f=="")? "" : ")";
		// Retrieve flagging state for position, and format it.

		if(visDev_>NORMAL) {

			// We're in a visibility deviation display mode.  It turns out to
			// be easier to recompute the deviation here from scratch, rather
			// than try to recycle the code in dev_() (sigh).

			Int t = vpos(TIME);
			Int st=lsvTime_[t], et=levTime_[t];
			if(et-st<2 || t<st || t>=et) return f+"Insufficient Data"+f2+" ";

			// The visibility at t will be used in the computation _regardless_
			// of flag state (so the tracking shows what the value would have
			// been anyway).  But other flagged data will _not_ be included in
			// the local running mean.

			Double sumv=v, sumv2=Double(v)*v;
			Int nvalid=1;


			// Phase deviations are calculated two ways; the minimum of these
			// is the returned value.  This is so that phases clustered around
			// +-180 degrees do not show artificially high deviations.

			Float va=0.;
			Double sumva=0., sumv2a=0.;
			Bool phase = (visComp_==PHASE);
			if(phase) {
				va = vAlt_(v);
				sumva=va;
				sumv2a=Double(va)*va;
			}



			for(Int tbox=st; tbox<et; tbox++) if(tbox!=t) {
					vpos[TIME] = tbox;
					Float vbox = vis_(vpos);
					Bool nodata = (vbox==NO_DATA);
					Bool fbox =  nodata || flag_(vpos);

					if(!nodata && (dispX_==TIME || dispY_==TIME)) {

						// Because of computational expense (and somewhat inconsistently),
						// _new_ flag edits are not considered in deviation displays
						// if Time is not a displayed axis.  In order to keep display
						// and tracking consistent (at least), we also take new flag edits
						// into account here only when the time axis is on display (the
						// usual case).

						pos[TIME] = tbox + visStart_[TIME];

						for(edits.toStart(); !edits.atEnd(); edits++) {
							FlagEdit_& edit = *static_cast<FlagEdit_*>(edits.getRight());
							if(edit.appliesTo(pos)) fbox = !edit.unflag;
						}
					}

					if(!fbox) {	// unflagged, valid data for the running average.
						nvalid++;
						sumv+=vbox;
						sumv2+=Double(vbox)*vbox;

						if(phase) {
							Float vboxa = vAlt_(vbox);
							sumva+=vboxa;
							sumv2a+=Double(vboxa)*vboxa;
						}
					}
				}


			if(nvalid<2) return f+"Insufficient Data"+f2+" ";
			// Not enough data to compute a meaningful deviation.

			if(visDev_==DIFF) {

				v = abs(v - sumv/nvalid);  // absolute difference

				if(phase) v = min(v, abs(va - sumva/nvalid));
			}

			else {
				Double sumdev2 = sumv2 - sumv*sumv/nvalid;
				if(sumdev2<0) return f+"Invalid"+f2;	// (shouldn't happen).

				v = sqrt(sumdev2/(nvalid-1));	 // RMS deviation

				if(phase) {
					Double sumdev2a = sumv2a - sumva*sumva/nvalid;
					if(sumdev2a>=0) v = min(v, sqrt(sumdev2a/(nvalid-1)));
				}
			}
		}


		// Format output value (v) into string.

		ostringstream os;
		os << setprecision(4);
		os << f << v << " " << dataUnit().getName() << f2 << " ";

		return String(os);
	}



//--------------------------------------------------------------------------
	String MSAsRaster::avgPos( const String &dim, int v) {
		char buf[2048];
		if ( visDev_ > NORMAL || nPAvg_ == 1 || dim != axisName_(axisOn_(Z)) ) {
			sprintf( buf, "%d", v );
			return String(buf);
		} else {
			sprintf( buf, "avg(%d:%d)", v, min( (Int) (v + nPAvg_-1), (Int) (vis_.shape( )(axisOn_(Z))-1) ) );
			return String(buf);
		}
	}

//--------------------------------------------------------------------------
	String MSAsRaster::showPosition(const Vector<Double> &world,
	                                const Bool& /*displayAxesOnly*/) {
		// for position tracking--formatted data value at given coordinate.
		// #dk (Still to do: convert this to use of MSpos_. Also, add MSpos_
		//      method to return frequency GHz).

		if(!msValid_) return "\n\n\n";	// In this case, the object is useless.
		// Protect public methods from crashes.

		if(!msselValid_) return "MS selection contains no data.\n"
			                        "Change MS selections and press 'Apply'.\n\n";

		if(!rstrsConformed_ || !csConformed_) return "\n\n\n";
		// Doesn't match wch restrictions or CS state.  Probably means
		// this DD isn't the one in charge of the canvas (csMaster).
		// Return 4 blank lines anyway, to prevent gratuitous resizes
		// of the tracking message area.

		// we need _linear_ coords for display _and_ animator axes.
		// So we convert back to lin...
		Vector<Double> lin(2);
		if(!worldToLin(lin, world)) return "\n(Invalid Coordinates)\n\n";
		Block<Int> pos(pos_);
		for(AxisLoc disploc=X; disploc<=Y; disploc++) {
			Axis ax=axisOn_[disploc];
			Double lcoord=lin(disploc);
			pos[ax] = ifloor(lcoord+.5);
			// 'close the upper display edge'...(round down in this case)
			if(near(lcoord, msShape_[ax]-.5)) pos[ax]=ifloor(lcoord);
		}

		pos[axisOn_[Z]] = activeZIndex_;	// Insert latest canvas animator
		// position.  This position (or the WC[H]) should really be a parameter
		// to the method instead.  pos_[axisOn_[Z]] is not necessarily already
		// up-to-date, because the event giving rise to this call may have
		// occurred on a different canvas from the one which last set
		// pos_[axisOn_(Z)].  setActiveZIndex_(zindex) must be called prior
		// to calling this routine (as it is via the conformsTo(WCH&) call
		// in the GtkDD motion EH), so that activeZIndex_ contains the
		// correct animator position for the canvas.

		ostringstream os;
		os << setprecision(5);

		// The auxiliary data used for this world-coordinate tracking output
		// (time_, nAnt_, freq_, spwId_, scan_, field_, fieldName_, polName_)
		// was gathered during findRanges_().

		// Ideally, it would be collected into the DD's DisplayCoordinateSystem and
		// used by the WADD (itsAxisLabeller) as well, but neither of these
		// has the full generality needed yet.  E.g., the baseline-to-antennas
		// translation _is_ in the CS already, but the lame results in axis
		// labelling are manifest.... (Axis labelling also fails for Image DDs
		// when a Stokes coordinate is put onto a display axis).  Needs work...

		Int pt=pos[TIME],   szt=msShape_[TIME];
		Bool validt=(pt>=0 && pt<szt);
		Int pb=pos[BASELN], szb=msShape_[BASELN];
		Bool validb=(pb>=0 && pb<szb);
		Int pc=pos[CHAN],   szc=msShape_[CHAN];
		Bool validc=(pc>=0 && pc<szc);
		Int pp=pos[POL],    szp=msShape_[POL];
		Bool validp=(pp>=0 && pp<szp);
		Int ps=pos[SP_W],   szs=msShape_[SP_W];
		Bool valids=(ps>=0 && ps<szs &&
		             ps<Int(freq_.nelements()));

		os<<endl;
		Int startp=os.tellp();


		// Time, Scan

		if(validt) {
			MVTime mtm(time_[pt]/C::day);	// (convert seconds to days)
			if(mtm.year()<1000 || mtm.year()>=10000) validt=False;  // (Y10K bug :-)
			else {
				String tstr = mtm.string(MVTime::YMD);
				os<<tstr.substr(8,2)<<"-"<<mtm.monthName()<<"-"<<
				  tstr.substr(0,4)<<" "<<tstr.substr(11,8);
			}
		}
		if(!validt) os<<"Invalid";
		os<<" (t "<<avgPos(axisName_[TIME],pt+uiBase())<<")";
		// (see def of DisplayData::uiBase() for explanation...).
		if(validt) {
			os<<String(max( 0, 33+startp-Int(os.tellp()) ), ' ');
			// tab to position 33 in the line.
			// (_such_ a pain compared to FORTRAN formatting: "T33"....)
			os<<"Scan "<<scan_[pt];
		}  // (scan #s always reported as set in MS).
		os<<endl;
		startp=os.tellp();


		// Field

		if(validt) os<<setw(8)<<left<<fieldName_[pt]<<" (Field "<<
			             field_[pt]+uiBase()<<") ";


		// Baseline/Feed

		if(szb>1) {

			os<<String(max( 0, 30+startp-Int(os.tellp()) ), ' ');

			if(nAnt_==1) {

				// Feed case

				os<<"   Feed "<<pb+uiBase();
				if(!validb) os<<" (Invalid)";
			}

			else {

				// Baseline case

				Int a1=0, a1u=0, a2=0, a2u=0;
				if(!validb) os<<"Invalid ";
				else {
					a1 = a1_(pb);
					a1u = a1 + uiBase();
					a2 = a2_(pb);
					a2u = a2 + uiBase();
					os<<setw(4)<<right<<a1u;
					if(a2>nAnt_-1) os<<"|"<<setw(3)<<left<<a1u+1<<"         ";
					// "Dividing line" (gap) between antennas (no data here)
					else {	// Real baseline
						os<<"-"<<setw(3)<<left<<a2u<<" ";
						if(a1==a2) os<<"        ";
						else os<<right<<setw(6)<<setprecision(4)<<bLen_(a1,a2)<<"m ";
					}
				}

				os<<"(b "<<setw(1)<<left<<avgPos(axisName_[BASELN],pb+uiBase())<<")";
			}
		}

		os<<endl;
		startp=os.tellp();


		// Spectral window, Channel, Frequency

		if(valids) os<<"Sp Win "<<setw(1)<<avgPos(axisName_[SP_W],spwId_[ps]+uiBase())<<" ";
		os<<"(s "<<setw(1)<<ps+uiBase()<<") ";
		if(valids) {
			Vector<Double>& fr = *static_cast<Vector<Double>*>(freq_[ps]);
			if( validc && pc<Int(fr.nelements()) ) {
				os<<setw(9)<<right<<setprecision(7)<<  fr[pc]/1.e9<<" GHz"  <<
				  setprecision(5);
			} else validc=False;
		}
		if(!valids || !validc) os<<" Invalid ";
		os<<" (ch "<<setw(1)<<avgPos(axisName_[CHAN],pc+uiBase())<<")  ";



		// Polarization

		os<<String(max( 0, 41+startp-Int(os.tellp()) ), ' ');
		if(validp) os<<setw(1)<<polName_[pp];
		else os<<"Invalid";
		os<<" (cor "<<avgPos(axisName_[POL],pp+uiBase())<<")";

		return String(os);
	}

	bool MSAsRaster::showPosition( viewer::RegionInfo::stats_t &stat_list, const Vector<Double> &world,
	                               const Bool& /*displayAxesOnly*/ ) {
		// for position tracking--formatted data value at given coordinate.
		// #dk (Still to do: convert this to use of MSpos_. Also, add MSpos_
		//      method to return frequency GHz).

		if(!msValid_) return false;		// In this case, the object is useless.
		// Protect public methods from crashes.

		if(!msselValid_) return false;

		if(!rstrsConformed_ || !csConformed_) return false;
		// Doesn't match wch restrictions or CS state.  Probably means
		// this DD isn't the one in charge of the canvas (csMaster).
		// Return 4 blank lines anyway, to prevent gratuitous resizes
		// of the tracking message area.

		// we need _linear_ coords for display _and_ animator axes.
		// So we convert back to lin...
		Vector<Double> lin(2);
		if(!worldToLin(lin, world)) return false;

		Block<Int> pos(pos_);
		for(AxisLoc disploc=X; disploc<=Y; disploc++) {
			Axis ax=axisOn_[disploc];
			Double lcoord=lin(disploc);
			pos[ax] = ifloor(lcoord+.5);
			// 'close the upper display edge'...(round down in this case)
			if(near(lcoord, msShape_[ax]-.5)) pos[ax]=ifloor(lcoord);
		}

		pos[axisOn_[Z]] = activeZIndex_;	// Insert latest canvas animator
		// position.  This position (or the WC[H]) should really be a parameter
		// to the method instead.  pos_[axisOn_[Z]] is not necessarily already
		// up-to-date, because the event giving rise to this call may have
		// occurred on a different canvas from the one which last set
		// pos_[axisOn_(Z)].  setActiveZIndex_(zindex) must be called prior
		// to calling this routine (as it is via the conformsTo(WCH&) call
		// in the GtkDD motion EH), so that activeZIndex_ contains the
		// correct animator position for the canvas.

		// The auxiliary data used for this world-coordinate tracking output
		// (time_, nAnt_, freq_, spwId_, scan_, field_, fieldName_, polName_)
		// was gathered during findRanges_().

		// Ideally, it would be collected into the DD's DisplayCoordinateSystem and
		// used by the WADD (itsAxisLabeller) as well, but neither of these
		// has the full generality needed yet.  E.g., the baseline-to-antennas
		// translation _is_ in the CS already, but the lame results in axis
		// labelling are manifest.... (Axis labelling also fails for Image DDs
		// when a Stokes coordinate is put onto a display axis).  Needs work...

		Int pt=pos[TIME],   szt=msShape_[TIME];
		Bool validt=(pt>=0 && pt<szt);
		Int pb=pos[BASELN], szb=msShape_[BASELN];
		Bool validb=(pb>=0 && pb<szb);
		Int pc=pos[CHAN],   szc=msShape_[CHAN];
		Bool validc=(pc>=0 && pc<szc);
		Int pp=pos[POL],    szp=msShape_[POL];
		Bool validp=(pp>=0 && pp<szp);
		Int ps=pos[SP_W],   szs=msShape_[SP_W];
		Bool valids=(ps>=0 && ps<szs &&
		             ps<Int(freq_.nelements()));


		// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
		// Time, Scan
		// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
		if(validt) {
			MVTime mtm(time_[pt]/C::day);		// (convert seconds to days)
			if(mtm.year()<1000 || mtm.year()>=10000) {
				stat_list.push_back(viewer::RegionInfo::stats_t::value_type("time","invalid"));
				validt=False;  // (Y10K bug :-)
			} else {
				String tstr = mtm.string(MVTime::YMD);
				String time_val = tstr.substr(8,2) + "-" + mtm.monthName() + "-" + tstr.substr(0,4) + " " + tstr.substr(11,8);
				stat_list.push_back(viewer::RegionInfo::stats_t::value_type("time",time_val));
			}
		}


		stat_list.push_back(viewer::RegionInfo::stats_t::value_type("time index",avgPos(axisName_[TIME],pt+uiBase())));
		// (see def of DisplayData::uiBase() for explanation...).

		char buf[1024];

		if(validt) {
			// os<<String(max( 0, 33+startp-Int(os.tellp()) ), ' ');
			// 	// tab to position 33 in the line.
			// 	// (_such_ a pain compared to FORTRAN formatting: "T33"....)
			sprintf( buf, "%d", scan_[pt] );
			stat_list.push_back(viewer::RegionInfo::stats_t::value_type("scan",buf));
			// (scan #s always reported as set in MS).
		}

		// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
		// Field
		// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----

		if ( validt ) {
			sprintf( buf, "%s (%d)", fieldName_[pt].c_str( ), field_[pt]+uiBase() );
			stat_list.push_back(viewer::RegionInfo::stats_t::value_type("field",buf));
		}

		// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
		// Baseline/Feed
		// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
		if ( szb > 1 ) {

			if(nAnt_==1) {

				// Feed case
				char *ptr = buf;
				*ptr = '\0';

				sprintf( ptr, "%d", pb+uiBase() );
				ptr += strlen(ptr);
				if ( ! validb ) sprintf( ptr, " (invalid)" );
				stat_list.push_back(viewer::RegionInfo::stats_t::value_type("feed", buf));

			} else {

				// Baseline case

				char *ptr = buf;
				*ptr = '\0';
				std::string baseline_val;
				Int a1=0, a1u=0, a2=0, a2u=0;
				if(!validb) {
					sprintf( ptr, "(invalid)" );
					ptr += strlen(ptr);
				} else {
					a1 = a1_(pb);
					a1u = a1 + uiBase();
					a2 = a2_(pb);
					a2u = a2 + uiBase();
					sprintf( ptr, "%d", a1u );
					ptr += strlen(ptr);
					if ( a2 > nAnt_-1 ) {
						sprintf( ptr, "|%d", a1u+1 );
						ptr += strlen(ptr);
						// "Dividing line" (gap) between antennas (no data here)
					} else {
						// Real baseline
						sprintf( ptr, "-%d", a2u );
						ptr += strlen(ptr);
						if( a1 != a2 ) {
							sprintf( ptr, " %.0fm", bLen_(a1,a2) );
							ptr += strlen(ptr);
						}
					}
				}

				sprintf( ptr, " (b %s)", avgPos(axisName_[BASELN],pb+uiBase()).c_str( ) );
				ptr += strlen(ptr);
				stat_list.push_back(viewer::RegionInfo::stats_t::value_type("baseline",buf));
			}
		}

		// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
		// Spectral window, Channel, Frequency
		// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----

		if(valids) {

			char *ptr = buf;
			*ptr = '\0';

			sprintf( ptr, "%s (s %d)", avgPos(axisName_[SP_W],spwId_[ps]+uiBase()).c_str( ), ps+uiBase());
			ptr += strlen(ptr);

			if(valids) {
				Vector<Double>& fr = *static_cast<Vector<Double>*>(freq_[ps]);
				if( validc && pc<Int(fr.nelements()) ) {
					sprintf( ptr, " %.5g GHz", fr[pc]/1.e9 );
					ptr += strlen(ptr);
				} else {
					validc=False;
				}
			}
		}

		if ( ! valids || ! validc ) sprintf( buf, "(invalid)" );

		stat_list.push_back(viewer::RegionInfo::stats_t::value_type("sp win",buf));

		stat_list.push_back(viewer::RegionInfo::stats_t::value_type("channel",avgPos(axisName_[CHAN],pc+uiBase())));


		// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
		// Polarization
		// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----

		String pol_val;
		if ( validp )
			pol_val = polName_[pp];
		else
			pol_val = "(invalid)";

		pol_val = pol_val + " (cor " + avgPos(axisName_[POL],pp+uiBase()) + ")";

		stat_list.push_back(viewer::RegionInfo::stats_t::value_type("polarization",pol_val));

		return true;
	}

//===================misc. small stuff======================================

// Note: These constants are static, and it should be possible to
// initialize them in the .h file, but some compilers have a problem
// with that (though not with static consts of a typedef (!) ).

	const Int MSAsRaster::INVALID=-1;
	// Fairly general purpose, usually for out-of-bounds or
	// non-existent hypercube indices.

	const Float MSAsRaster::NO_DATA=-6.283e18f;
	// Arbitrary value commandeered to stand for 'no data' in vis_.
	// NB: the dispFlags_ Matrix uses a different value for this (below).

	const Float MSAsRaster::NOT_LOADED=-7.283e18f;
	// Very similar, but used only in the disp_ array, for data
	// which is not loaded into memory (vis_) at present.

	const Float MSAsRaster::INSUF_DATA=-8.283e18f;
	// Also similar.  Used in dispDev_ when there are less than 2
	// values in the local neighborhood from which to compute a
	// meaningful deviation.

// NB: the color dispFlags_ matrix uses a different value to indicate
// these two conditions (NODATA, NOTLOAED, below).

// dispFlags_ and the flagCM_ custom colormap
// use the following coding:
	const Float MSAsRaster::NOTLOADED=0.f;
	const Float MSAsRaster::NODATA=1.f;
	const Float MSAsRaster::OLDFLAG=2.f;
	const Float MSAsRaster::NEWFLAG=3.f;



	void MSAsRaster::computeVisShape_(Block<Int>& visShp, Block<Int>& visShpA) {
		// Compute vis_ array dimensions which fit into allowed memory.
		// vis_ is a 'cursor' or 'window' into the whole selected, gridded ms
		// whose dimensions are given by msShapeA_.  It _will_ be the entire
		// hypercube if it fits into memory; in any case, the two display
		// axes will be full size.
		// visShpA is the actual shape to be allocated to vis_; visShp is the
		// portion actually to be used at present; it may be smaller on the
		// BASELN axis if baselines are currently sorted by length (no 'gaps').
		// (Whereas the suffix 'A' in other baseline sort state variables stands
		// for 'under the Antenna sort', for visShpA (visShapeA_) it more
		// accurately stands for the space actually allocated for vis_).


		Double kbAvail=itsVisMb->value() * 1024.;	// memory allowed for vis_,
		kbAvail = max(kbAvail, 5.*1024.);		// from user interface.

		// Trim the size of the memory buffer axes as necessary, to keep vis_
		// within specified memory size limits, if possible.  The entire range of
		// the selected MS on the display axes is always stored, however.
		// Trim slider axes first, then the animator axis.  To display
		// data beyond the range of the vis_ buffer will require another extract_.
		// On the sliders, give up Times first, then Baselines, then Spw's,
		// then Channels, then Polarizations.

		static const Axis axisToTrim[5] = {TIME, BASELN, SP_W, CHAN, POL};

		visShpA = msShapeA_;	// Try trimming from here first.

		while(True) {		// (Will execute either once or twice).

			Double kbNeeded=sizeof(Float)/1024.;
			for(Axis ax=0; ax<NAXES; ax++) kbNeeded *= visShpA[ax];

			Double fctr = kbNeeded/kbAvail;
			// Ratio of memory required by whole (later: trimmed) MS selection,
			// to memory available for the vis_ buffer.

			Axis axSL0=axisOn_[SL0], axSL1=axisOn_[SL1];
			for(Int i=0; i<NAXES && fctr>1.; i++) {
				Axis tax = axisToTrim[i];
				if(tax!=axSL0 && tax!=axSL1) continue;	// trim slider axes first.
				Int newsz = Int(max(1.,visShpA[tax]/fctr));
				fctr = fctr*newsz/visShpA[tax];
				visShpA[tax] = newsz;
			}
			if(fctr>1.) visShpA[axisOn_[Z]] = Int(max(1.,visShpA[axisOn_[Z]]/fctr));
			// finally, trim off animation frames, if necessary.

			if(visShpA[BASELN]==msShapeA_[BASELN] ||
			        visShpA[BASELN]<=msShape_[BASELN] ) break;

			// Can't accommodate all of msShapeA_[BASELN]; trim starting from
			// msShape_[BASELN] instead (wasteful to use more than msShape_[BASELN]).

			visShpA = msShape_;
		}


		visShp = visShpA;

		visShp[BASELN] = min(visShp[BASELN], msShape_[BASELN]);
	}
	// visShp: portion of vis_ currently to be used for data.  No
	// more than msShape_, which may be smaller on BASELN axis when
	// length sort is used (no gap elements).



	Bool MSAsRaster::undoEdits_(String extent, Bool feedback) {
		// Remove edit[s] from end of list.  Return value indicates whether
		// there were any edits to undo.  extent=="all" means undo all, else
		// just the last one.  If feedback==True, print a warning message
		// about discarded edits.

		Int ndeleted=flagEdits_.len();
		if(ndeleted==0) return False;

		ListIter<void*> edits(flagEdits_);
		edits.toEnd();
		while(!edits.atStart()) {
			edits--;
			delete static_cast<FlagEdit_*>(edits.getRight());
			edits.removeRight();
			if(extent!="all") {
				ndeleted=1;
				break;
			}
		}

		purgeCache();		// needed for the display to redraw properly,
		// because we don't try to distinguish the
		// flagging state of the cached drawings.

		if(feedback==True) {
			cerr<<"(Discarded "<<ndeleted<<" unsaved flagging edit";
			if(ndeleted>1) cerr<<"s";
			cerr<<")"<<endl;
		}

		return True;
	}




	Bool MSAsRaster::zIndexHint(Int& preferredZIndex) const {
		// Return the animator position setting preferred if this DD is about
		// to be registered on a new DisplayPanel.  It uses the minimum
		// setting on the DD's other WCHs (if any), or a default initial value.

		if(!msselValid_) return False;	// No data.

		if(!getFirstZIndex(preferredZIndex)) {
			// use existing animator setting (unless there aren't any)...

			preferredZIndex = 0;		// ...else: use 0 in most cases...

			if(axisOn_[Z]==CHAN) {
				MSpos_ mpos(this);
				mpos.sets(pos_[SP_W]);
				if(mpos.spwId!=INVALID) {
					preferredZIndex = (nChan_[mpos.spwId]-1)/2;
				}
			}
			// if CHAN is the current Z axis, pick the middle
			// channel of the current spectral window.
			if(axisOn_[Z]==BASELN && nAnt_>1 && msShape_[BASELN]>1) {
				preferredZIndex = antSort_? 1 : min(nAnt_, msShape_[BASELN]-1);
			}
		}
		// First non-autocorrelation is a better baseline choice
		// for interoferometric data.

		if(preferredZIndex >= Int(nelements())) return False;  // (unlikely)
		return True;
	}



	Int MSAsRaster::nframes_(const Block<Int>& frames, Int strtfrm, Int nfrms,
	                         Int& margin) {
		// Return how many of the given (sorted) animation frames
		// can be displayed from a given interval (strtfrm, nfrms) on that axis.
		// If the return value is >0, margin will be the minimum padding on either
		// side, from the edges of the interval to the frames of interest.
		margin=1000000;
		Int nframes=0,  tfrms=frames.nelements(),  endfrm=strtfrm+nfrms;
		for(Int i=0; i<tfrms; i++) {
			Int frm=frames[i];
			if(frm>=endfrm) break;
			if(frm>=strtfrm) {
				nframes++;
				Int marg=min(frm-strtfrm, endfrm-1-frm);
				if(nframes==1 || marg<margin) margin=marg;
			}
		}
		return nframes;
	}



	Int MSAsRaster::maxframes_(const Block<Int>& frames,
	                           Int& strtfrm, Int nfrms) {
		// Return the maximum number of the given (sorted) animation frames that
		// can be displayed from an interval of a given number of frames (nfrms).
		// Also returns where that interval should start (strtfrm).
		strtfrm=0;
		Int tfrms=frames.nelements();
		if(tfrms==0 || nfrms<1) return 0;

		strtfrm=-1;
		Int maxframes=-1;  // (Will be immediately overridden...)
		Bool multiple=False;
		Int margin=-1;
		Int lstpfrm=max(0,Int(nelements())-nfrms);
		// last _possible_ interval start frame,
		// given interval size and axis size.

		int frstfrm=max(0, frames[0]+1 - nfrms);
		Int lstfrm=max(0, min(lstpfrm, frames[tfrms-1]));
		// first and last _feasible_ starting frames for the interval:
		// no point in checking intervals beyond the first or last frames
		// of interest.

		for(Int trialfrm=frstfrm; trialfrm<=lstfrm; trialfrm++) {

			// Heuristic for interval selection among those containing an
			// equal number of the frames of interest: favor start frames
			// which are a multiple of window size (or the last possible
			// interval).  This minimizes data loading requirements when
			// going serially through a very large dataset.
			// Other things being equal, choose the interval leaving the largest
			// number of frames on both sides around the frames of interest.

			Bool mult = trialfrm % nfrms==0  ||  trialfrm==lstpfrm;
			Int marg;
			Int nframes = nframes_(frames, trialfrm, nfrms, marg);
			if(nframes  > maxframes ||
			        nframes == maxframes && !multiple && (mult || marg>margin) ) {

				// trialfrm is the optimal candidate so far.

				maxframes=nframes;
				strtfrm=trialfrm;
				multiple=mult;
				margin=marg;
			}
		}

		return maxframes;
	}



	void MSAsRaster::setDefaultOptions() {
		// set the default options for the display data (all levels)

		ActiveCaching2dDD::setDefaultOptions();   // (base levels)
		setDefaultMSAROptions_();
	}              // (this level)



	void MSAsRaster::setDefaultMSAROptions_() {
		//# (setDefaultOptions is unused; implement if needed some day)...
		/* *itsXAxis = itsXAxis->defaultValue(); ... */
	}



	String MSAsRaster::visMbLabel_() {
		// This returns the label for the itsVisMb slider widget.  It includes
		// feedback on the total selected MS size, so is updated as appropriate.
		// Input: msShapeA_

		Float mbVis = sizeof(Float);
		for(Axis ax=0; ax<NAXES; ax++) mbVis *= msShapeA_[ax];
		mbVis = ceil(mbVis / 1024. / 1024.);
		ostringstream os;
		os << setprecision(6);
		os << "max vis memory (whole ms: "<<Int(mbVis)<< " mb)";
		return String(os);
	}



	const Unit MSAsRaster::dataUnit() const {
		// For position tracking and color bar labelling -- Unit of the
		// displayed data values
		switch(curVisComp_) {
		case PHASE:
			return "deg";
		case INVALID_VC:
			return "";
		default:
			return "Jy";
		}
	}
	// (akemball said Jy was 'the MS visibility unit'.  Is it ever
	// anything else (and if so, where is it stored)?...)



	Bool MSAsRaster::labelAxes(const WCRefreshEvent &ev) {
		Bool doLabelling = (msselValid_ && itsAxisLabelling->value() == "Yes");
		if(doLabelling) itsAxisLabeller.refreshEH(ev);
		// call on our WorldAxesDD to do the job
		return doLabelling;
	}

	Bool MSAsRaster::canLabelAxes() const {
		Bool doLabelling = (msselValid_ && itsAxisLabelling->value() == "Yes");
		return doLabelling;
	}

	void MSAsRaster::a1a2_(Double& a1, Double& a2,  Double bsl, Int abase) const {
		// A inverse corresponding to bsln_(a1,a2), returns a1 and a2, given
		// bsl.  This handles non-integer baseline indices (i.e., bsl doesn't
		// have to be an integer, in which case a2 won't be either).  The output
		// is thus suitable for building a TabularCoordinate (see setCS_()).
		// The second version translates to 'world coordinates' in a form
		// like 11.017 for baseline 11-17.
		//
		// Note: a1a2_() and bsln_() routines are _only_ for the antenna sort;
		// the a1_, a2_ and bsl_ Arrays are used for the current sort (either by
		// antenna or baseline length).

		// (The bsln index corresponding to a1-a1 (i.e. bsln_(a1,a1))  is
		// a1*(nAnt_+2) - a1*(a1+1)/2, so that
		// a1^2 - (2*nAnt_ + 3)*a1 + 2*bsln = 0.  The code below essentially
		// solves this quadratic for a1.)

		Int bsln = ifloor(bsl+.5);
		Double b=2*nAnt_+3, ac4=8*bsln;
		Double b2=b*b;
		if(b2-ac4<0) a1 = nAnt_ - 1;	      // (shouldn't happen--next statement
		else a1=floor((b-sqrt(b2-ac4))/2);  // should always solve for a1 exactly).
		Int bsl0;
		while(True) {
			bsl0 = bsln_(Int(a1),Int(a1));
			if(bsl0<=bsln) break;
			a1--;
		}			      // (safety--shoudn't be needed).
		a2 = a1 + (bsl-bsl0);
		a1+=abase;
		a2+=abase;
	}



	Double MSAsRaster::a1a2_(Double bsl, Int abase) {
		Double a1,a2;
		a1a2_(a1,a2, bsl, abase);
		return a1 + a2/a1mult_();
	}



	void MSAsRaster::notifyRegister(WorldCanvasHolder *wch) {
		ActiveCaching2dDD::notifyRegister(wch);
		itsAxisLabeller.notifyRegister(wch);		// notify our AxesDD too.
		if(msValid_) wch->worldCanvas()->registerColormap(&flagCM_, 1.f);
	}
	// Be sure cells for flagging Colormap are allocated on this canvas.



	void MSAsRaster::notifyUnregister(WorldCanvasHolder &wch,
	                                  Bool ignoreRefresh) {
		ActiveCaching2dDD::notifyUnregister(wch, ignoreRefresh);
		itsAxisLabeller.notifyUnregister(wch, ignoreRefresh);
		// notify our AxesDD too.
		if(msValid_) wch.worldCanvas()->unregisterColormap(&flagCM_);
	}
	// Deallocate cells for flag colormap when through with the canvas.



	CachingDisplayMethod *MSAsRaster::newDisplayMethod( WorldCanvas *wc,
	        AttributeBuffer *wchAttrs, AttributeBuffer *ddAttrs,
	        CachingDisplayData *dd) {
		return new MSAsRasterDM(wc, wchAttrs, ddAttrs, dd);
	}



	Bool MSAsRaster::flag_(IPosition& slot) {
		Int offset = (((	      slot(TIME)
		                          * visShape_[SP_W]   + slot(SP_W))
		               * visShapeA_[BASELN] + slot(BASELN))
		              * visShape_[POL]    + slot(POL))
		             * visShape_[CHAN]   + slot(CHAN);
		// NB: the order of axes within IPos& slot is same as for
		// vis_ (i.e., TIME, BASELN, CHAN, POL, SP_W), but this is
		// not the order of least-to-most rapidly varying axis.  That order,
		// reflected in the statement above, was chosen to correspond to
		// extract_ loop nesting, for efficiency there.

		static const Int uIntBits = sizeof(uInt)*8;
		Int indx = offset / uIntBits;
		Int bit = offset % uIntBits;

		return ( (flags_(indx) & 1u<<bit) != 0 );
	}



	void MSAsRaster::setFlag_(IPosition& slot, Bool flag) {
		// This routine writes data into flags_, an internal array
		// corresponding to the current state of flags in the MS, but it
		// does not itself write any flags to disk.
		Int offset = (((	      slot(TIME)
		                          * visShape_[SP_W]   + slot(SP_W))
		               * visShapeA_[BASELN] + slot(BASELN))
		              * visShape_[POL]    + slot(POL))
		             * visShape_[CHAN]   + slot(CHAN);

		static const Int uIntBits = sizeof(uInt)*8;
		Int indx = offset / uIntBits;
		Int bit = offset % uIntBits;

		if(flag) flags_(indx) |=   1u<<bit;		// Set flag
		else     flags_(indx) &= ~(1u<<bit);
	}	// Reset flag



	Bool MSAsRaster::FlagEdit_::appliesTo(Int bsln) {
		// Does edit apply to given baseline?

		// the caller must assure that the passed baseline is within the
		// range returned by edit.getLoopRange(BASELN, strt, fin), i.e.,
		// strt<=bsln && bsln<fin.  This saves time calling this method
		// within loops.  In particular:

		if(!entireAnt) /* we can simply */ return True;
		// ...because we know edit.inSureRange is True for the bsln.

		if(msar->nAnt_==1) return True;
		// (edit applies to entire single antenna).

		// Otherwise, do antenna-based baseline testing.  Does the given baseline
		// include the edit's selected antenna?  In this type of editing, the
		// 'antenna to edit' is (only) the _ant1_ of the left-most baseline in
		// the region chosen with the mouse.  (In most cases, baselines will be
		// sorted by antenna, and the user will select a region entirely within
		// a single 'antenna1-bar' -- by clicking the crosshair somewhere within
		// it, e.g.  It doesn't make much sense to attempt antenna-based editing
		// in any other way).  Note that all baselines will be [un]flagged which
		// have the 'antenna to edit' as _either_ the baseline's ant1 or ant2.

		Int firstBsl, endBsl;
		getSureRange(BASELN, firstBsl, endBsl);
		Int antToEdit = msar->a1_(firstBsl);

		return  msar->a1_(bsln)==antToEdit || msar->a2_(bsln)==antToEdit;
	}



	Bool MSAsRaster::FlagEdit_::operator==(FlagEdit_& other) {
		// compare two flag edit commands for equality.
		uInt n=all.nelements();
		if(n!=other.all.nelements()) return False;
		for(uInt i=0; i<n; i++)
			if(all[i]!=other.all[i] || start[i]!=other.start[i] ||
			        shape[i]!=other.shape[i] ) return False;
		if(unflag!=other.unflag || entireAnt!=other.entireAnt) return False;
		return True;
	}



	Bool MSAsRaster::FlagEdit_::appliesToChunk(
	    Int pol0, Int nPol,  Int spw, Int nChan) {

		// Does the edit apply to the current MS chunk?  (A 'chunk' will have a
		// fixed polID, spwId and (hence) data shape on disk).  This routine
		// is called by MSAsRaster::saveEdits_().

		// Note: this routine adds some new complexity to the FlagEdit_ struct,
		// because it helps implement an innovation: applying and saving edits
		// _beyond_ the range of the selected MS and the internal hypercube.
		// Except for this routine and the state it sets, everything in the
		// struct had been expressed in terms of indices into the internal
		// hypercube.  Even here, the parameters pol0 and spw are still
		// in these terms: spw is the hypercube index for the chunk's spwId
		// (not always the same as the spwId).  Similarly, pol0 is the beginning
		// hypercube index for the pols of the chunk's polId.  Either parameter
		// may INVALID if the chunk lies outside the selected MS (we must be
		// applying edits to the entire MS, to have retrieved such a chunk).
		// nPol and nChan, however, refer to the shape, not of the internal
		// cube, but of the data and flags as stored on disk in each row of
		// the chunk.
		// The state set by this routine (sPol,ePol, sChan,eChan)
		// are also a translation into _disk_ indices, of the range of chans
		// and pols to which the edit applies.  These are calculated here at
		// the beginning of each chunk and stored with the edit, rather than
		// recalculating these same ranges for every row of the chunk.

		appChunk = False;
		sPol=ePol=sChan=eChan=0;
		// (default, until shown otherwise).

		if(all[POL]) {
			sPol=0;
			ePol=nPol;
		}	// Edit applies to all pols in MS row.
		else {
			if(pol0==INVALID) return False;	// polId N/A to edit.
			getSureRange(POL, sPol, ePol);
			sPol = max(0,    sPol-pol0);	// Adjust edit range for polId
			ePol = min(nPol, ePol-pol0);
		}	// and clip to MS row's pol shape.

		if(all[SP_W] || inSureRange(SP_W, spw)) {	// Edit applies to spwId.
			if(all[CHAN]) {				// Edit applies to all
				sChan=0;
				eChan=nChan;
			}			// channels in MS row.
			else {
				getSureRange(CHAN, sChan, eChan);  	// clip edit channels to
				eChan = min(nChan, eChan);
			}
		}   	// MS row's channel shape.

		appChunk = (sPol<ePol && sChan<eChan);

		return appChunk;
	}


	Bool MSAsRaster::FlagEdit_::appliesTo(Double time) {
		// Does the edit apply to a given (raw) time?
		// This is another new routine that deals with data as stored in the
		// MS, instead of hypercube indices.  When edits are extended to the
		// entire MS, we want to flag any MS time between the ones the user has
		// chosen, regardless of whether a timeslot index for it exists within the
		// currently-selected fields.  Hence the need to compare raw times here.

		if(all[TIME]) return True;
		Int st, et;
		getSureRange(TIME, st, et);
		// Edit bracket in terms of timeslot indices.
		return (time>=msar->time_[st] && time<=msar->time_[et-1]);
	}






// MSpos_: Translation methods:
// hypercube-to-MS

	void MSAsRaster::MSpos_::sett(Int t) {
		time = (t>=0 && t<m->msShape_[TIME])?  m->time_[t] : INVALID;
	}

	void MSAsRaster::MSpos_::setb(Int b) {
		ant1 = ant2 = feed = INVALID;
		if(b<0 || b>=m->msShape_[BASELN]) return;
		if(m->nAnt_==1) feed=b;
		else {
			ant1=m->a1_(b);
			ant2=m->a2_(b);
		}
	}

	void MSAsRaster::MSpos_::sets(Int s) {
		spwId = (s>=0 && s<Int(m->spwId_.nelements()))?  m->spwId_[s] : INVALID;
	}

	void MSAsRaster::MSpos_::setc(Int c) {
		chan = c;
	}
	// (validity tested elsewhere (in c()) against spWin size).

	void MSAsRaster::MSpos_::setp(Int p) {
		if(p>=0 && p<Int(m->polId_.nelements())) {
			polId   = m->polId_[p];
			polName = m->polName_[p];
		} else {
			polId=INVALID;
			polName="Invalid";
		}
	}


// MS-to-hypercube.

	Int MSAsRaster::MSpos_::t() {
		// Does a binary search on the sorted time_ Vector for the
		// matching timeslot matching 'time'.
		ts=0, te=m->msShape_[TIME];
		while(ts<te) {
			Int tmid = (ts+te)/2;
			Double tm = m->time_[tmid];
			if(time<tm) te=tmid;
			else if(time>tm) ts=tmid+1;

			else return tmid;
		}

		ts--;			// (not found: ts, te will be the
		return INVALID;
	}	// bracketing time slots in this case).


	Int MSAsRaster::MSpos_::b() {
		Int bsl;
		if(m->nAnt_==1) bsl = feed;
		else if(ant1<0 || ant1>ant2 || ant2>=m->nAnt_) bsl = INVALID;

		else bsl = m->bsl_(ant1, ant2);

		if(bsl<0 || bsl>=m->msShape_[BASELN]) bsl = INVALID;
		return bsl;
	}


	Int MSAsRaster::MSpos_::s() {
		if(spwId==INVALID) return INVALID;
		for(Int spw=0; spw<Int(m->spwId_.nelements()); spw++) {

			if(spwId==m->spwId_[spw]) return spw;
		}

		return INVALID;
	}


	Int MSAsRaster::MSpos_::c() {
		if(chan<0) return INVALID;
		if(s()==INVALID) return INVALID;

		if(chan < m->nChan_[spwId]) return chan;

		return INVALID;
	}


	Int MSAsRaster::MSpos_::p() {
		Int ps = p0();
		if(ps==INVALID) return INVALID;
		Int pe = ps + m->nPolsIn_[polId];
		for(Int pol=ps; pol<pe; pol++) {

			if(polName==m->polName_[pol]) return pol;
		}

		return INVALID;
	}




	void MSAsRaster::resetMinMax_() {
		// Reset data scaling DParameters to newly-computed data ranges.
		// Both ranges and values are reset, to the appropriate (normal or
		// deviation mode) ranges.
		// Caller must be sure *RngMin/Max_ have already been calculated,
		// and must arrange to send the DParameters on to the gui.

		Float dmin=dataRngMin_, dmax=dataRngMax_;
		if(visDev_>NORMAL) {
			dmin=devRngMin_;
			dmax=devRngMax_;
		}

		*itsDataMin = dmin;
		*itsDataMax = dmax;
		itsDataMin->setMinimum(dmin);
		itsDataMax->setMinimum(dmin);
		itsDataMin->setMaximum(dmax);
		itsDataMax->setMaximum(dmax);
	}


    bool MSAsRaster::freqAxis( Axis a ) const {
        if ( a != CHAN ) return false;
        if ( itsParamSpectralUnit->value( ) == "channel" ) return false;
        if ( mspos_.spwId == INVALID && mspos_.spwId >= chanFreq_.shape( )(1) )
            return false;
        return true;
    }



	void MSAsRaster::computeBaselineSorts_() {
		// Called from findRanges_(), computes translation matrices between
		// antenna1,antenna2 and baseline index  (a1A_, a2A_, a1L_, a2L_,
		// bslA_, bslL_).  Arrays for both sorts are computed here; the ones
		// for the current sort are copied into a1_, a2_ and bsl_.
		// Other related Arrays computed are: bLen_(a1,a2) (baseline lengths),
		// len2ant_ and ant2len_ (translate bsl indices from one sort to the other).


		// First, gather baseline lengths.

		bLen_.resize(nAnt_+1, nAnt_+1);
		// (These matrices are made symmetric, for no really good reason;
		// a1 will be <= a2 when used as indices to them).

		for(Int a1=0; a1<nAnt_; a1++) {
			bLen_(a1,a1)=0.;

			static Double gaplen = pow(2., 48);
			bLen_(a1,nAnt_) = bLen_(nAnt_,a1) = (a1+2.)*gaplen;
			// gaplen is an arbitrary value longer than any real baseline.
			// It is useful to sort 'gap' bsls (which are indicated by a2==nAnt_)
			// after legitimate bsls under the length sort; this provides a 1-to-1
			// corresp. between bsl indices under the two sorts, even for gaps.

			if(a1 == nAnt_-1) break;

			MPosition p1;
			msCols_->antenna().positionMeas().get(a1, p1);
			p1 = MPosition::Convert (p1, MPosition::ITRF)();
			MVPosition pv1 = p1.getValue();

			for(Int a2=a1+1; a2<nAnt_; a2++) {
				MPosition p2;
				msCols_->antenna().positionMeas().get(a2, p2);
				p2 = MPosition::Convert(p2, MPosition::ITRF)();
				MVPosition pv2 = p2.getValue();
				bLen_(a1,a2) = bLen_(a2,a1) = min(gaplen, (pv1-pv2).radius());
			}
		}
		// (pv1-pv2).radius() is the desired baseline length (the min with
		// gaplen is for safety only; sensible baselines will always be
		// shorter than gaplen, which is on the order of a light-year).


		// Set number of baselines for either sort: 1 more than the final index
		// when using the antenna sort.  For the length sort, the (nAnt_-1) gaps
		// are removed.  (In the single-dish case, both  will be 1, but they won't
		// be used; msShape[BASELN] will be set to the number of feeds instead).

		nbslA_ = bsln_(nAnt_-1, nAnt_-1) + 1;
		nbslL_ = nbslA_ - (nAnt_-1);


		// Translation matrices for the antenna sort are computed directly via
		// the older routines, a1a2_ and bsln_.  (This should also be more efficient
		// than the old practice of using those routines in major tight loops).

		a1A_.resize(nbslA_);
		a2A_.resize(nbslA_);
		for(Double bsl=0.; bsl<nbslA_; bsl++) {
			Double a1d, a2d;
			a1a2_(a1d,a2d, bsl);
			a1A_(int(bsl)) = Int(a1d);
			a2A_(int(bsl)) = Int(a2d);
		}

		bslA_.resize(nAnt_+1, nAnt_+1);
		for(Int a1=0; a1<nAnt_; a1++) {
			for(Int a2=a1; a2<nAnt_+1; a2++) {
				bslA_(a1, a2) = bslA_(a2, a1) = bsln_(a1, a2);
			}
		}


		// The lengths, for bsl index in a1-a2 order.  (See above for 'lengths'
		// set for 'gap' baselines).

		Vector<Double> blen(nbslA_);
		for(Int bsl=0; bsl<nbslA_; bsl++)  blen(bsl) = bLen_(a1A_(bsl), a2A_(bsl));


		// Create Vector len2ant_, which translates a len-order bsl into an
		// antenna-order one; i.e. a Vector of antenna-based bsl indices in
		// baseline length order.


		Sort sort;
		Bool deleteln, deletea1, deletea2;

		const Double* ln = blen.getStorage(deleteln);
		const Int*    a1 = a1A_.getStorage(deletea1);
		const Int*    a2 = a2A_.getStorage(deletea2);

		sort.sortKey(ln, TpDouble);	// Where lengths are identical (e.g. for
		sort.sortKey(a1, TpInt);	// 0-length autocorrs), order by antennas
		sort.sortKey(a2, TpInt);	// (there will be no 'dup keys').

		Vector<uInt> l2a;	// (More strong-typing B.S...)


		sort.sort(l2a, nbslA_);


		len2ant_.resize(l2a.size());
		for(uInt i=0; i<l2a.size(); i++) len2ant_[i] = Int(l2a[i]);
		// len2ant_ will contain antenna-based bsl indices in length order;
		// i.e., it translates from length- to antenna-based bsl indices.

		blen.freeStorage(ln, deleteln);
		a1A_.freeStorage(a1, deletea1);
		a2A_.freeStorage(a2, deletea2);


		// Create the inverse Vector, ant2Len_, which gives a length-based
		// bsl index for an antenna-based one.

		ant2len_.resize(nbslA_);
		for(Int bl=0; bl<nbslA_; bl++) ant2len_(len2ant_(bl)) = bl;


		// Create a1L_, a2L_, bslL_ -- the length-based translation Arrays between
		// antenna numbers and baseline indices.  Note that 'gap' baselines also
		// have a (unique) bsl index even under the length sort; they lie beyond
		// the bsls actually used for display (nbslL_ <= gapbsl < nbslA_).  In
		// antenna terms, gaps are indicated by ant2 (or, here, ant1) == nAnt_.
		// (bslL_(nAnt_,nAnt_) is unused and remains undefined though).

		a1L_.resize(nbslA_);
		a2L_.resize(nbslA_);
		bslL_.resize(nAnt_+1, nAnt_+1);

		for(Int bl=0; bl<nbslA_; bl++) {
			Int ba = len2ant_(bl);
			Int a1 = (a1L_(bl) = a1A_(ba));
			Int a2 = (a2L_(bl) = a2A_(ba));
			bslL_(a1, a2) = bslL_(a2, a1) = bl;
		}


		setBslSort_();
	}




	void MSAsRaster::setBslSort_() {
		// Set the baseline index translation Arrays a1_, a2_ and bsl_ by
		// copying as appropriate according to the current sort.  (Source Arrays
		// should already have been created by computeBaselineSorts_(), above).
		// also sets total number of baselines, nbsl_, which becomes
		// msShape_[BASELN] except in single-dish case.

		if(antSort_) {
			a1_.resize(a1A_.size());	// Soooo dumb to have to do this...
			a1_ = a1A_;		// ...before this (grr).
			a2_.resize(a2A_.size());
			a2_ = a2A_;
			bsl_.resize(bslA_.shape());
			bsl_ = bslA_;
			nbsl_ = nbslA_;
		} else {
			a1_.resize(a1L_.size());
			a1_ = a1L_;
			a2_.resize(a2L_.size());
			a2_ = a2L_;
			bsl_.resize(bslL_.shape());
			bsl_ = bslL_;
			nbsl_ = nbslL_;
		}
	}



	void MSAsRaster::reSortVis_() {

		// Shuffle vis_ (and flags_) into new baseline order,
		// per user request for sort change.
		//
		// Baseline sort and translation data, visShape_ (except on BASELN) and
		// related data should be up-to-date with new sort request before calling.
		// Also, this routine should not be used unless vis covers all baselines,
		// including gaps (i.e., visShapeA_[BASELN]==msShapeA_{BASELN] and thus
		// visStart_[BASELN]==0).

		Int nt=visShape_[TIME], nb=visShapeA_[BASELN], nc=visShape_[CHAN],
		    np=visShape_[POL], ns=visShape_[SP_W];

		// Adjust visShape_[BASELN] appropriately for the new sort.

		visShape_[BASELN] = msShape_[BASELN];

		Int nbl=visShape_[BASELN];
		// Note that flags and vis_ both have room for 'gaps' on the baseline
		// axis thoush the gaps are not displayed under the length sort
		// (their real axis length on that axis is visShapeA_[BASELN]).
		//
		// visShape_[BASELN] (or nbl) is the axis length actually used for
		// display.  When the new sort is by baseline length, this will be
		// shorter than visShapeA_[BASELN].

		Int ci = 1;		// Axis increments for efficiently
		Int pi = ci*nc;	// computing offsets into flags_.
		Int bi = pi*np;
		Int si = bi*nb;
		Int ti = si*ns;

		Vector<uInt> flgs(flags_.size(), 0u);
		// temporary destination flags cleared initially; same size as flags_

		Int o2n[nb];	// Translation vector from old to new baseline indices.
		for(Int b=0; b<nb; b++) o2n[b] = antSort_? len2ant_(b) : ant2len_(b);


		// A temporary vector of visibilities in new baseline order
		// for the current time-channel-pol-spWin.

		Float tv[nb];


		// For progress feedback.

		Timer tmr;
		Double pctDone = 100. / max(1., Double(nt*nc*np));
		Bool prgShown = False;
		Double intvl=2.;
		Double iPlanes=0.;


		// (NB: struct casa::Axis, defined in ArrayAccessor.h,
		// vs.  typedef Int Axis,  defined in MSAsRaster.h...).

		ArrayAccessor<Float, casa::Axis<TIME> >  t(vis_);
		for(Int to=0;   t!=t.end();   ++t, to+=ti) {
			ArrayAccessor<Float, casa::Axis<CHAN> >  c(t);
			for(Int co=to;   c!=c.end();   ++c, co+=ci) {
				ArrayAccessor<Float, casa::Axis<POL> >  p(c);
				for(Int po=co;   p!=p.end();   ++p, po+=pi) {
					ArrayAccessor<Float, casa::Axis<SP_W> >  s(p);
					for(Int so=po;   s!=s.end();   ++s, so+=si) {
						Int* o2np=o2n;
						ArrayAccessor<Float, casa::Axis<BASELN> >  b(s);
						for(Int bo=so;   b!=b.end();   ++b, bo+=bi, o2np++) {


							// Load temporary vector of visibilities in new baseline order.

							tv[*o2np]=*b;


							// Also set flag in new position if necessary.

							if(antSort_ && o2np-o2n>=nbl) continue;
							// (For efficiency: the (old) length sort will have not
							// have flags set beyond the first ('real') nbl baselines).

							static const Int uIntBits = sizeof(uInt)*8;
							Int indx = bo / uIntBits;
							Int bit = bo % uIntBits;
							if( (flags_(indx) & 1u<<bit) != 0 ) {

								// Flagged at (old) offset (bo).

								Int bon =  so  +  *o2np * bi;	// bon: offset for flag
								indx = bon / uIntBits;		// under new baseline order.
								bit = bon % uIntBits;

								// Set flag in its new position (in temp array).

								flgs(indx) |= 1u<<bit;
							}
						}



						Float* tvp=tv;
						for(b.reset(); b!=b.end(); ++b) {
							*b=*tvp++;
						}
					}
					// Store newly-ordered visibilities back into vis_


					// progress feedback

					iPlanes++;
					if(tmr.real()>intvl) {
						if(!prgShown) {
							cerr<<endl<<"Resorting MS vis. data:  "<<flush;
							prgShown=True;
						}
						Int pct = Int(iPlanes*pctDone +.5);
						if(pct>0 && pct<100) cerr<<pct<<"%  "<<flush;
						tmr.mark();
						intvl++;
					}
				}
			}
		}



		// Store reordered flags finally, in their official place.

		flags_ = flgs;


		dispValid_ = False;   // Forces call to createDisplaySlice_ in draw_.

		if(prgShown) cerr<<"Done."<<endl<<endl;
	}	// progress feedback.





	MSAsRaster::~MSAsRaster() {
		removeFromAllWCHs();			// disconnect from canvases/refresh events.

		if(msValid_)
			deleteParameters_();

		removeColormap();
		resetFreq_();

		Bool printwarning=True;
		undoEdits_("all", printwarning);		// delete any leftover flagging edits--user will lose them.


		if( wvi_p !=0 && wvi_p != vs_ )
			delete wvi_p;
		if( vs_!=0)					// delete old VisibilityIterators...
			delete vs_;

		if( mssel_ != itsMS && mssel_ != 0 ) {
			mssel_->relinquishAutoLocks(True);
			delete mssel_;				// selected MS,...
		}

		if ( msCols_!=0)
			delete msCols_;

		if(itsMS!=0) {
			itsMS->relinquishAutoLocks(True);
			delete itsMS;				// and original unselected MS.
		}
	}

	bool MSAsRaster::adjustAvgRange( VisDev newstate, Record &outrec, bool force ) {
		if ( (visDev_ == NORMAL && newstate > NORMAL) || (force && visDev_ > NORMAL) ) {
			// switch to difference (RMS/Diff) averaging
			const char *help = "The number of time slots in moving\n"
			                   "averages, for visibility difference\n"
			                   "or RMS displays";
			itsNAvg->setName( "navg" );
			itsNAvg->setDescription( help );
			itsNAvg->setMinimum( 2 );
			itsNAvg->setMaximum( 15 );
			itsNAvg->setDefaultValue( 3 );
			itsNAvg->setValue( 3 );
			Record navg;
			navg.define("value",nDAvg_);
			navg.define("pmin",2);
			navg.define("pmax",15);
			navg.define("listname","rms/diff moving average size");
			navg.define("help",help);
			outrec.defineRecord( "navg", navg );
			return true;
		} else if ( (visDev_ > NORMAL && newstate == NORMAL) || (force && visDev_ == NORMAL) ) {
			int zlen = msShape_[axisOn_(Z)];
			const char *help = "Number of cube planes to include in the average";
			itsNAvg->setName( "navg" );
			itsNAvg->setDescription( help );
			itsNAvg->setMinimum( 1 );
			itsNAvg->setMaximum( zlen );
			itsNAvg->setDefaultValue( 1 );
			itsNAvg->setValue( nPAvg_ );
			Record navg;
			navg.define("value",nPAvg_);
			navg.define("pmin",1);
			navg.define("pmax",zlen);
			navg.define("help",help);
			navg.define("listname","average size");
			outrec.defineRecord( "navg", navg );
			return true;
		}
		return false;
	}

	void MSAsRaster::deleteParameters_() {
		//#  delete itsSelections;
		delete itsVisType;
		delete itsXAxis;
		delete itsYAxis;
		delete itsZAxis;
		delete itsSL0Pos;
		delete itsSL1Pos;
		delete itsVisComp;
		delete itsNAvg;
		delete itsDataMin;
		delete itsDataMax;
		delete itsAxisLabelling;
        delete itsParamSpectralUnit;
		delete itsBslnSort;
		delete itsFlagColor;
		delete itsUnflag;
		delete itsEntireAnt;
		delete itsUndoOne;
		delete itsUndoAll;
		delete itsEditEntireMS;
		delete itsSaveEdits;
	}



//====================end==================================================

} //# NAMESPACE CASA - END
