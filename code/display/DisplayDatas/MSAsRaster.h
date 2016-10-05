//# MSAsRaster.h: DisplayData (drawing layer) for raster displays of an MS
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
//#
//# $Id$

#ifndef TRIALDISPLAY_MSASRASTER_H
#define TRIALDISPLAY_MSASRASTER_H


#include <casa/aips.h>
#include <display/DisplayDatas/ActiveCaching2dDD.h>
#include <ms/MeasurementSets.h>
#include <images/Images/PagedImage.h>
#include <casa/Arrays/Vector.h>
#include <casa/Quanta/Unit.h>
#include <casa/BasicSL/String.h>
#include <display/Display/DParameterChoice.h>
#include <display/Display/DParameterRange.h>
#include <display/Display/DParameterString.h>
#include <display/Display/DParameterButton.h>
#include <display/DisplayCanvas/WCPowerScaleHandler.h>
#include <msvis/MSVis/VisibilityIterator.h>
#include <display/DisplayDatas/WorldAxesDD.h>
#include <display/Display/DisplayEnums.h>
#include <display/Display/WorldCanvasHolder.h>
#include <display/DisplayDatas/CachingDisplayMethod.h>
#include <display/DisplayDatas/DisplayDataOptions.h>
#include <display/Display/Colormap.h>
#include <display/region/Region.qo.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	class MSAsRasterDM;


// <summary>
// Class for displaying data within an casacore::MS as a raster (gridded) image.
// </summary>

// <prerequisite>
//   <li> CachingDisplayData
//   <li> WorldCanvas[Holder]
//   <li> MeasurementSet
//   <li> CoordinateSystem
//   <li> Array
// </prerequisite>

// <etymology>
// "MSAsRaster" is a implementation of a <linkto class=ActiveCaching2dDD>
// ActiveCaching2dDD </linkto> which provides for the display of Measurement
// Set data (visibilities) as a raster image.
// </etymology>

// <synopsis>
// MSAsRaster displays Measurement Set data which can be presented as a
// regular grid, on a WorldCanvas (data which is sparse and
// irregular on its display axes would typically be plotted using MSAsXY
// instead).  Display or iteration axes include time, baseline,
// channels and polarizations.  Visibilities of the selected slice are drawn
// as a Raster image.
//
// Unlike many DDs, MSAsRaster has _two_ levels of underlying data: the source
// casacore::MS, and a hypercube of visibility data, which is extracted from the casacore::MS and
// gridded onto 5 main axes: time, baseline, spectral window, frequency and
// polarization.  Extracting the data is a more time-consuming operation
// than display of various portions of gridded data, and is triggered only
// through the extract_() method.  casacore::Data selection options do not take effect
// until triggered in this way.  In contrast, many options affecting display
// of the extracted data (such as zooms, and colormapping) affect
// the display immediately, as with most DDs.  Setting new display axes will
// trigger extract_() if the current extracted hypercube does not fill them.
//
// </synopsis>

// <example>
// <srcblock>
//    MSAsRaster *mar = new MSAsRaster("filename_of.ms");
//    wcHolder->addDisplayData(mar);
// </srcblock>
// </example>
//
// <motivation>
// To move compute-intensive tasks of data handling
// for Measurement Set display out of glish into C++
// </motivation>

// <todo asof="2001/02/01">
//
// The current user interface for triggering extract_ is the 'Apply' button,
// which is detected internally when setOptions() is called with more than
// one user option field to be updated.  This is not entirely satisfactory;
// User interface could use improvement.
//
// Major items among other things left to to:
//
// * casacore::MS Selection and averaging.
//
// * World coordinates in axis labelling, position tracking and slice
//   selection
//
// * Alternative orderings, especially along baseline and time axes
//
// </todo>

	class MSAsRaster: public ActiveCaching2dDD {

	public:

		typedef casacore::Int Axis;

		// constructor
		// from the filename of an MS
		MSAsRaster( const casacore::String msname, const viewer::DisplayDataOptions &ddo );

		//# from an casacore::MS object
		//# MSAsRaster(casacore::MeasurementSet* ms);
		//#unneeded?--prefer to control permissions...

		// Destructor
		virtual ~MSAsRaster();


		// Apply option values stored in <src>rec</src> to the DisplayData.  A
		// return value of <src>true</src> means a refresh is needed.
		// <src>recOut</src> contains any fields which were implicitly
		// changed as a result of the call to this function.
		// Parameters from the 'adjust' gui are sent through here,
		// controlling and triggering many of
		// MSAsRaster's actions.  (Other input is via the mouse on the
		// canvas, which affects the object through the event handlers).
		virtual casacore::Bool setOptions(casacore::Record &rec, casacore::Record &recOut);

		// Retrieve the current options.  In addition to the values themselves, the
		// returned record contains meta-information (prompts, help text,
		// choices, defaults, etc.) useful in constructing gui elements to
		// control the DD settings.
		// Note: If the user interface is to be zero-based, you should call
		// setUIBase(0) _before_ using this routine -- see setUIBase().
		virtual casacore::Record getOptions( bool scrub=false ) const;

		// set all options to default values (unused so far; incomplete
		// support in base classes...incomplete here as well).
		virtual void setDefaultOptions();


		// Return the current options of this DisplayData as a 'restrictions'
		// AttributeBuffer (only options that affect the way the image would
		// be drawn are returned).  It is used to determine which (if any) of
		// the cached drawings can be used to satisfy the current draw request.
		virtual AttributeBuffer optionsAsAttributes();


		// Determine whether DD is compatible with the WC[H]'s current
		// world coordinates.  MSAsRaster DDs must be in charge (the CS master),
		// Otherwise they will not respond.  Multiple MSARs on one canvas
		// (blinking, etc.) is not supported, because of possible confusion
		// about which one(s) should respond to flagging edits.
		virtual casacore::Bool conformsToCS(const WorldCanvas& wc) {
			csConformed_ = wc.isCSmaster(this);
			return csConformed_;
		}

		// casacore::Format the data value at the given world position.
		// Call setActiveImage(zindex) with the desired animator position
		// before calling this routine.
		virtual casacore::String showValue(const casacore::Vector<casacore::Double> &world);

		// casacore::Format the position of the cursor.  Also requires previous call to
		// setActiveImage(zindex); also used for position tracking.
		virtual casacore::String showPosition(const casacore::Vector<casacore::Double> &world,
		                            const casacore::Bool &displayAxesOnly = false);
		bool showPosition( viewer::RegionInfo::stats_t &stat_list, const casacore::Vector<casacore::Double> &world,
		                   const casacore::Bool& displayAxesOnly = false);

		// get the casacore::Unit for displayed data values (visibilities)
		virtual const casacore::Unit dataUnit() const;
		const casacore::IPosition dataShape() const {
			return casacore::IPosition( );
		}
		casacore::uInt dataDim() const {
			return 0;
		}
		std::vector<int> displayAxes( ) const {
			return std::vector<int>( );
		}


		// Return the type of this DisplayData.
		virtual Display::DisplayDataType classType() {
			return Display::Raster;
		}
		// Pure virtual function from DisplayData...
		casacore::String dataType() const {
			return "ms";
		}


		// return the size of the animation axis.
		// <group>
		virtual casacore::uInt nelements() const {
			return msShape_[axisOn_(Z)];
		}
		virtual casacore::uInt nelements(const WorldCanvasHolder &) const {
			return msShape_[axisOn_(Z)];
		}
		// </group>

		// Handle axis labelling.
		virtual casacore::Bool labelAxes(const WCRefreshEvent &ev);
		virtual casacore::Bool canLabelAxes() const;

		// Needed to enable or destroy drawlists and colormaps on corresp. canvas.
		// (Only to be called by the relevant WCH).
		// <group>
		virtual void notifyRegister(WorldCanvasHolder *wch) ;
		virtual void notifyUnregister(WorldCanvasHolder& wch,
		                              casacore::Bool ignoreRefresh = false) ;
		// </group>

		// handle flagging region selection events, via new-style (1/02)
		// interface.
		virtual void handleEvent(DisplayEvent& ev);

		// Return the animator position setting preferred if this DD
		// about to be registered on a new DisplayPanel.
		virtual casacore::Bool zIndexHint(casacore::Int& preferredZIndex) const;

		// Empty cache completely.
		virtual void purgeCache() {
			ActiveCaching2dDD::purgeCache();
			itsAxisLabeller.purgeCache();
		}		//# (include our labeller)

		// Empty cache of all DMs for a given WCH.
		virtual void purgeCache(const WorldCanvasHolder& wch) {
			ActiveCaching2dDD::purgeCache(wch);
			itsAxisLabeller.purgeCache(wch);
		}		//# (include our labeller)

		// DD 'Absolute Pixel Coordinates', e.g. channel numbers, are internally
		// 0-based (they begin numbering at 0), but certain external user-interface
		// methods (e.g. showPosition(), used for position tracking) have
		// produced 1-based output traditionally for the glish-based viewer.
		// uiBase_, and related methods uiBase() and setUIBase(), allow newer
		// (python/Qt-based) code to cause external ui functions like showValue()
		// to report 0-based values instead.  Unless setUIBase(0) is called, the
		// traditional 1-based reporting behavior is retained by default.
		// For this DD, in addition to tracking, this setting affects labelling,
		// slider-based position setting, and casacore::MS selection on Field or Sp. Window.
		//
		// If you are using 0-basing in the user interface, you should call
		// setUIBase(0) right after constructing this DisplayData, before other
		// user interface operations such as getOptions(); the method has not
		// been tested other than for a one-time setUIBase(0) call directly after
		// DD construction.
		virtual void setUIBase(casacore::Int uibase) {
			casacore::Int oldUIBase = uiBase();
			ActiveCaching2dDD::setUIBase(uibase);
			if(oldUIBase != uiBase()) {
				setCS_();		// fixes axis labelling, among other things...
				purgeCache();
			}
		}
		// In case it helps; the method should probably be called
		// only before we've had a chance to cache anything though....



		virtual const casacore::String &name( ) const {
			return msName_;
		}

		// added to allow flagging control from mouse tools... <drs>
		bool flag( WorldCanvas *wc, double blc_x, double blc_y, double trc_x, double trc_y );

		std::string errorMessage( ) const { return ""; }

	protected:

		// This routine is called to inform the DD of the current canvas's
		// animator index.  Used by confromsTo() and related methods.
		// Return value indicates whether the index is within the data's range.
		virtual casacore::Bool setActiveZIndex_(casacore::Int zindex) {
			activeZIndex_ = zindex;
			// activeZIndex_ and zIndexConformed_ protected on the DD
			// base level; they are intended to be set only by these
			// [protected] setActiveZIndex_() methods, which are in turn
			// called by DD::conformsToZIndex().
			zIndexConformed_ = ( activeZIndex_>=0 && activeZIndex_<casacore::Int(nelements()) );
			return zIndexConformed_;
		}

		// Construct and destroy the user option DisplayParameters.
		// To be used by constructors/destructor only.
		// <group>
		void constructParameters_();
		void deleteParameters_();
		// </group>

		// return a new MSAsRasterDM for the given WorldCanvas.
		virtual CachingDisplayMethod *newDisplayMethod(WorldCanvas *worldCanvas,
		        AttributeBuffer *wchAttributes,
		        AttributeBuffer *ddAttributes,
		        CachingDisplayData *dd);

		// Helper functions.
		//# gcc-3.4.2 cannot distinguish between the 2 functions in complex.
		// <group>
		static casacore::Float real (const casacore::Complex& val) {
			return val.real();
		}
		static casacore::Float imag (const casacore::Complex& val) {
			return val.imag();
		}
		// </group>

	private:


		// Constructs position information for non-deviation display
		// state averaging...
		casacore::String avgPos( const casacore::String &dim, int v );

		// The (multiple) DMs which this DD creates just hold drawlist handles.
		// They send the actual drawing chores back to MSAsRaster::draw_().
		// The friend designation is so that draw_() can be made private.
		friend class MSAsRasterDM;

		// Default and copy constructors, and the assignment operator, are
		// non-functional and should not be used.  Do not make copies of
		// DisplayData objects, or pass them by value;
		// use references or pointers instead.
		// <group>
		MSAsRaster(): mspos_(this) {  }
		MSAsRaster(const MSAsRaster &other): ActiveCaching2dDD(other), mspos_(this) {  }
		MSAsRaster& operator=(const MSAsRaster &/*other*/) {
			return *this;
		}
		// </group>

		// Initialization common to all useful constructors
		void initMSAR_( const viewer::DisplayDataOptions &ddo );

		// set/restore default option values on this level only.  (Not implemented).
		void setDefaultMSAROptions_();


		//#-------------------------------------------------------------------
		//# The Workhorses--steps in producing the display from the casacore::MS and the
		//# the user input settings.  (Most of the control logic is elsewhere).

		// prepare the selection casacore::MS and its VisSet.
		void selectVS_( const viewer::DisplayDataOptions &ddo=viewer::DisplayDataOptions( ) );

		// find the ranges of the casacore::MS selection (VisSet) for the 5 hypercube axes
		void findRanges_();

		// update/set the (2d--canvas) coordinate system from the current MS
		// selection and display axes.
		DisplayCoordinateSystem setCS_();

		// Extract the hypercube buffer of visibilities for the requested
		// casacore::MS selection and axis settings (the most time-consuming operation).
		void extract_();

		// retrieve (2D) slice data casacore::Matrix, and corresponding mask/flag
		// matrices, to send to the display canvas.
		void createDisplaySlice_();

		// Actually do the drawing.
		// The return value indicates whether the DD was able to draw.
		casacore::Bool draw_(Display::RefreshReason reason,
		           WorldCanvasHolder &wch, WorldCanvas &wc);



		//#------baseline reordering routines-------------------------------------

		// Called from findRanges_(), computes translation matrices between
		// antenna1,antenna2 and baseline index  (a1A_, a2A_, a1L_, a2L_,
		// bslA_, bslL_).
		void computeBaselineSorts_();


		// Set the baseline index translation Arrays a1_, a2_ and bsl_ by
		// copying as appropriate according to the current sort.  (Source Arrays
		// should already have been created by computeBaselineSorts_(), above).
		// also sets total number of baselines, nbsl_, which becomes
		// msShape_[BASELN] except in single dish case.
		void setBslSort_();


		// Shuffle vis_ into new baseline order, per user request for sort change.
		// (Information needed to do this should already have been set up by
		// the previous two routines).
		void reSortVis_();



		//#-----small helper routines.-----------------------------------------

		// Return baseline index (for the ant1-ant2 sort _only_) from
		// antenna numbers.  casacore::Input must have 0 <= a1 <= a2 < nAnt_.
		// Mapping leaves room for autocorrelations plus a 1-pixel gap between
		// successive antenna1 groups.
		//
		//# (Crude--needs improvement:  *Wasteful if some antennas have ANTENNA
		//# table entries but don't appear in the visibility data.  *The gaps
		//# really should appear only in the final display casacore::Matrix, when needed.
		//# *Slots for autocorrs should also be inserted only if the selected
		//# data includes them.  *Other baseline ordering options should exist).
		casacore::Int bsln_(casacore::Int a1, casacore::Int a2) const {
			return a1*(nAnt_+2) - a1*(a1+1)/2 + a2-a1;
		}

		// A corresponding inverse, this handles non-integer 'baseline indices'.
		// Set abase to 1 to number ant1 and ant2 from 1.  (The baseline index
		// bsl is always numbered from 0, internally).  The version with the
		// casacore::Double return value returns a1 + a2/a1mult_() (for labelling --
		// example: for baseline index corresp. to 13-24, returns 13.024).
		// <group>
		void a1a2_(casacore::Double& a1, casacore::Double& a2,  casacore::Double bsl, casacore::Int abase=0) const;
		casacore::Double a1a2_(casacore::Double bsl, casacore::Int abase=0);
		// </group>

		// Subsidiary routine for above, determines (in effect) how many decimal
		// places are needed for an antenna number (including one leading zero).
		// Returns 1000 for 10-99 antennas, 10000 for 100-999 antennas, etc.
		casacore::Double a1mult_() {
			casacore::Double m=10.;
			while(m <= nAnt_-1+uiBase()) m*=10.;
			return 10*m;
		}


		// A small routine to return the label for the "Visibility Memory"
		// slider widget.  The label includes feedback on the selected casacore::MS's
		// total size, and is updated when that changes.
		casacore::String visMbLabel_();


		// Reset casacore::Block of relevant Spectral window IDs, and return
		// correponding Vectors of channel frequencies.  freq_ is used for display
		// of frequencies in position tracking.  spwId_ translates spw 'index'
		// into the actual spectral window ID.
		void resetFreq_() {
			for(casacore::uInt s=0; s<freq_.nelements(); s++) {
				delete static_cast<casacore::Vector<casacore::Double>*>(freq_[s]);
			}
			freq_.resize(0, true);
			spwId_.resize(0, true);
		}

		// Translate actual spectral window ID into the 'spw' index (zero-based
		// pixel coordinate) along the spectral window axis.  Because the
		// user can select specific spectral windows, these two may not be the same.
		// Returns -1 if the spectral window ID is not in the selected casacore::MS data.
		casacore::Int spw_(casacore::Int spwid) {
			casacore::Int nspw=spwId_.nelements();
			for(casacore::Int spw=0; spw<nspw; spw++) if(spwId_[spw]==spwid) return spw;
			return -1;
		}

		// Compute vis_ array dimensions which fit into allowed memory.
		// visShpA is the actual shape to be allocated to vis_; visShp is the
		// portion which will actually be used at present; it may be smaller on the
		// BASELN axis if baselines are currently sorted by length (no 'gaps').
		void computeVisShape_(casacore::Block<casacore::Int>& visShp, casacore::Block<casacore::Int>& visShpA);

		// Return how many of the given (sorted) animation frames
		// can be displayed from a given window (strtfrm, nfrms) on that axis.
		// In cases where nframes>0, margin will be the minimum padding on either
		// side, from the edges of the interval to the frames of interest.
		casacore::Int nframes_(const casacore::Block<casacore::Int>& frames, casacore::Int strtfrm, casacore::Int nfrms, casacore::Int& margin);

		// Return the maximum number of the given (sorted) animation frames that
		// can be displayed from a window or interval of a given size (nfrms).
		// also returns where that interval should start (strtfrm).
		casacore::Int maxframes_(const casacore::Block<casacore::Int>& frames, casacore::Int& strtfrm, casacore::Int nfrms);

		// Reset data scaling DParameters to newly-computed data ranges.
		void resetMinMax_();


        // represent channels with their frequency?
        bool freqAxis( Axis ) const;

		//#------flagging routines----------------------------------------------

		// Return or set a flag within the bitmapped flags_ vector, as if it were
		// a 5-axis casacore::Array<casacore::Bool> corresponding in dimensions to vis_.
		// (Note: flags_ is an internal array corresponding to the current state of
		// flags in the casacore::MS, but these routines in themselves do not read or write
		// any flags to disk).
		// <group>
		casacore::Bool flag_(casacore::IPosition& slot);
		void setFlag_(casacore::IPosition& slot, casacore::Bool flag);
		// </group>

		// Add the edit request that just came in (from the mouse, via handleEvent)
		// to the flagEdits_ casacore::List, then cause the display to be updated.
		void addEdit_(WorldCanvas* wc, casacore::Int xStart, casacore::Int xShape,
		              casacore::Int yStart, casacore::Int yShape);

		// Assure that the display matrices are up-to-date with the flagging edits
		// list, to provide visual feedback of the edits.
		void postEditsToDisp_();

		// Undo unsaved edits.  Return value indicates whether there were any
		// edits to undo.  extent=="all" means undo all, else just the last one.
		// If feedback==true, a warning message will be printed about discarded
		// edits.
		casacore::Bool undoEdits_(casacore::String extent="all", casacore::Bool feedback=false);

		// Save all edits permanently to the MS.  The return value indicates
		// whether there were any edits to save.
		casacore::Bool saveEdits_();


		//#------visibility deviation (difference, RMS) routines----------------

		// compute the lsTime_ and leTime_ vectors, which define the
		// 'local neighborhoods' around each given time slot, for
		// computing running averages.
		void computeTimeBoxcars_();

		// Return a single visibility point from vis_ or disp_, as a function
		// of time slot only.  The row of times and mode must be predetermined by
		// setting useVis_, dPos_, axlTm_ and flgdDev_ (below).  goodData_ is set
		// true by this routine if the data exists, is loaded and is not flagged
		// (false otherwise).
		casacore::Float v_(casacore::Int t);

		// Phase deviations are calculated both for the original phases
		// (in [-180,180]), and for the phases 180 degrees opposite
		// (also expressed within [-180,180]); the minimum result is displayed.
		// This is so that phases clustered around +-180 do not show
		// artificially high deviations.  (remainder(x,360) (from math.h)
		// is always in [-180,180] -- wierd, but usable for this purpose).
		casacore::Float vAlt_(casacore::Float v) {
			return remainder(v-180., 360.);
		}

		// Return the visibility deviation for the time slot t.  visDev_
		// determines whether this is an RMS deviation or absolute
		// difference from the running mean.  Maintains state from the
		// prior calculation to speed things up in some cases.  Must be
		// initialized as for v_() above, and sT_ set to -1, at the
		// beginning of a new row of times.
		casacore::Float dev_(casacore::Int t);

		// Calculate deviations throughout range of vis_.  Used to set
		// data scaling sliders when visibility deviations are to be displayed.
		void computeDevRange_();

		// Create dispDev_ casacore::Matrix for displaying deviations.  Both this
		// routine and the one above initialize and use dev_() to calculate
		// individual deviations.
		void createDevSlice_();




//===================    casacore::Data    ======================= (mostly) ========



		//------Main enums (and their conversions to strings)-------------------

		// (or, rather, they _used_ to be enums, until the compiler started
		// whining and moaning about their use as Ints, casacore::Vector/casacore::Array/Block
		// indices, etc.  (Strong typing: grrr...)

		static const casacore::Int INVALID;	// (==-1)  (fairly general purpose).

		// The visibility hypercube (vis_) has 5 axes, in this order.
		static const Axis TIME=0, BASELN=1, CHAN=2, POL=3, SP_W=4,
		                  NAXES=5,  INVALID_AXIS=-1;

		// Each axis can placed on the canvas display (X or Y), the animator (Z),
		// or on one of 2 auxiliary slider controls (SL0, SL1)
		typedef casacore::Int AxisLoc;
		static const AxisLoc X=0, Y=1, Z=2, SL0=3, SL1=4,   NLOCS=5;

		typedef casacore::Int VisType;
		static const VisType OBSERVED=VisibilityIterator::Observed,
		                     CORRECTED=VisibilityIterator::Corrected,
		                     MODEL=VisibilityIterator::Model,
		                     RESIDUAL=3,  // RATIO=4,
		                     NTYPES=4,   INVALID_VT=-1;

		typedef casacore::Int VisComp;
		static const VisComp AMPLITUDE=0, PHASE=1, REAL=2, IMAGINARY=3,
		                     NCOMPS=4,	// # of actual components (above)
		                     AMPDIFF=4, AMPRMS=5, PHDIFF=6, PHRMS=7,
		                     NCOMPNAMES=8,	// Number of choices in the
		                     // GUI choice box (itsVisComp).  itsVisComp is split into visComp_
		                     // (which must be one of the first four values above) and visDev_.
		                     // (see visDev_ below, and also setOptions()).
		                     INVALID_VC=-1;

		typedef casacore::Int VisDev;
		static const VisDev  NORMAL=0, DIFF=1, RMS=2,  INVALID_VD=-1;

		// Generic string-to-index converter...
		static casacore::Int ind_(const casacore::String& name, const casacore::Vector<casacore::String>& names) {
			for(casacore::uInt i=0; i<names.nelements(); i++) if(names(i)==name) return i;
			return -1;
		}

		// ...applied to 3 of the 'enums' used internally.
		// <group>
		Axis axisNum_(const casacore::String& axisName) const {
			return Axis(ind_(axisName, axisName_));
		}

		VisType visTypeNum_(const casacore::String& visTypeName) const {
			return VisType(ind_(visTypeName, visTypeName_));
		}

		VisComp visCompNum_(const casacore::String& visCompName) const {
			return VisComp(ind_(visCompName, visCompName_));
		}
		// </group>


//-----more constants and statics ----------------------------------------

		// These should be static const, but there's no way to initialize them.
		// Print strings, corresponding to Axis, VisType and VisComp enums above.
		casacore::Vector<casacore::String> axisName_;
		casacore::Vector<casacore::String> visTypeName_;
		casacore::Vector<casacore::String> visCompName_;

		static const casacore::Float NO_DATA;		// Arbitrary value commandeered
		// to stand for 'no data in the selected casacore::MS at this position in
		// the visibility cube'.  vis_ is initialized to this value before
		// data is extracted into it from the selected MS.  After extract_,
		// elements left with this value will be masked out during display.
		static const casacore::Float NOT_LOADED;
		// Very similar, but used only in the disp_ array, for data
		// which is not loaded into memory (vis_) at present.
		static const casacore::Float INSUF_DATA;
		// Also similar.  Returned by dev_() when there are less than 2
		// values in the local neighborhood from which to compute a
		// meaningful deviation, or if the data is flagged.

		// NB: The values above are large negative numbers which will not
		// correspond to legitimate data values in the data arrays
		// where they appear.  The dispFlags_ overlay matrix uses different
		// values (NODATA, NOTLOAED, below) to indicate these same two
		// conditions--small enum-like sequential integers which can be mapped to
		// definite colors easily.

		// dispFlags_ and the flagCM_ custom colormap use the following coding:

		static const casacore::Float NOTLOADED,  // not in vis_ memory buffer (grey)
		       NODATA,  // no data in the casacore::MS selection (black)
		       OLDFLAG,  // old flag, from the casacore::MS file (medium blue)
		       NEWFLAG;  // newly edited, unsaved flag (lighter blue)
		static const casacore::Int   NCOLORS=4;	 // number of colors above.

		//-----primary user input data-------------------------------------------

		//  Passed in through constructors and setOpts parameters.

		casacore::String msName_;		// casacore::MS filename.

		// User option DisplayParameters specific to MSAsRaster


		// Maximum memory to use for vis_ buffer.
		DParameterRange<casacore::Int> *itsVisMb;

		// Which axes will be on X, Y, Animator.  Chosen from
		// time, baseline, channel, polarization and spectral window.
		DParameterChoice *itsXAxis;
		DParameterChoice *itsYAxis;
		DParameterChoice *itsZAxis;

		// Which slice to display, for the other two axes.
		// The _label_ of the sliders (and the axes they control) vary.
		// They control the slice position of the axes not selected above.
		// Animator position is determined externally by WCH restriction 'zIndex'.
		DParameterRange<casacore::Int> *itsSL0Pos;
		DParameterRange<casacore::Int> *itsSL1Pos;

		// Baseline sort (antenna1-antenna2 vs. baseline length).
		DParameterChoice *itsBslnSort;

		// Sent to scale handler for scaling data to color within these limits.
		DParameterRange<casacore::Float> *itsDataMin;
		DParameterRange<casacore::Float> *itsDataMax;

		// Axis Labelling? (Yes/No).
		DParameterChoice *itsAxisLabelling;
        // Units of display for Channels
        DParameterChoice *itsParamSpectralUnit;

		// changes to these options require data extraction:

		//#  // the casacore::MS selections record:
		//#  DParameterRecord (or DParameterMSSelect) *itsSelections;

		// Observed, Corrected, Model, etc.
		DParameterChoice *itsVisType;

		//Which real visibiliy component to display: Real, Imag, Amp, Phase.
		DParameterChoice *itsVisComp;

		// how many values to use (ideally) in moving averages.
		DParameterRange<casacore::Int> *itsNAvg;


		//----derived from above: what is now requested-----------------------

		casacore::Vector<casacore::Int> axisOn_;	// the axis on each location.  This will be any
		// permutation of (TIME, BASELN, CHAN, POL, SP_W), corresponding to the
		// axis to display or control on X, Y, Z, SL0, and SL1, in that order.

		casacore::Block<casacore::Int> pos_;	// The position setting on the animator and sliders.
		// Zero-based (although the user sees 1-based values).
		// pos_ is indexed by Axis (TIME, BASELN, etc).  For axes being
		// displayed, the position is not immediately relevant, but serves
		// as a memory of the last setting and is restored to a slice control
		// whenever the axis moves off the display.  Corresponds somewhat to
		// itsFixedPosition in PrincipalAxesDD.

		VisType visType_;	// enums corresp. to strings in itsVisType/Comp
		VisComp visComp_;	// and in sync with them.
		VisDev  visDev_;	// (12/02) itsVisComp now maps into _two_ 'enums':
		// visComp_ and visDev_.
		// visComp_ is _only_ AMP, PHASE, REAL or IMAGINARY
		// (above is what is stored in large vis_ casacore::Array)
		// and visDev_ tells whether to display the
		// straight vis component (NORMAL), a difference
		// from a running average (DIFF) or a running
		// RMS (RMS).

		casacore::Int nDAvg_;		// RMS/Diff average value in itsNAvg (above).
		casacore::Int nPAvg_;		// number of planes to average

		casacore::Vector<casacore::Int> fieldIds_;	// user-selected field IDs and
		casacore::Vector<casacore::Int> spwIds_;		// spectral window IDs (0-based).


		//----current state of main internal data: what is already computed----

		// (These are set (successively) in initMSAR_, selectVS_, findRanges_,
		//  extract_, createDisplaySlice_, and createDevSlice_).


		// The original, unselected MS

		casacore::MeasurementSet *itsMS;		// The (unselected) casacore::MS to be displayed.
		VisibilityIterator* vs_;		// VisSet of (unselected) itsMS
		casacore::Bool msValid_;			// valid, writable, non-null (unselected) casacore::MS?
		// (should be set true (permanently) during
		// construction, or this object will be useless,
		// and do nothing).
		casacore::ROMSColumns* msCols_;			// utility object for (unselected) itsMS.
		casacore::Int nFieldIds_;			// Total number of fields, spectral windows,
		casacore::Int nSpwIds_;				// in the (unselected) MS.
		casacore::Vector<casacore::Int> nChan_;			// Number of channels, by Spectral window ID.
        casacore::Matrix<casacore::Double> chanFreq_;   // frequency mapping of channels, per Spectral window ID
		casacore::Bool dish_;				// true if the casacore::MS is 'single-dish' (has FLOAT_DATA,
		// which will then be used instead of the DATA column).


		// The selected MS.

		casacore::MS* mssel_;				// the selected casacore::MS and its VisSet--kept in sync
		VisibilityIterator* wvi_p;		// with user input (itsMS, fieldIds_, spwIds_).
		casacore::Bool msselValid_;			// mssel_ and vssel_ are valid and non-null.  We will
		// not draw until this is set true (in selectVS_).

		casacore::Int nAnt_;				// # rows in antenna table (for now).  Later: size
		// of the set of antennas appearing in mssel_
		// main data rows.
		// NB: nAnt_==1 is tested, rather than dish_ (which
		// should be equivalent), to determine whether feeds
		// are displayed instead of baselines.

		casacore::Block<casacore::Int> msShape_;			// shape of visibilites of the whole selected MS
		// = {nTime, nBsln, nChan, nPol, nSpw}.
		// msShape_[BASELN] reflects the size requirement for
		// the baseline sort in use (see antSort_, below).
		casacore::Block<casacore::Int> msShapeA_;			// Identical to msShape_, except possibly on BASELN
		// axis when baselines are sorted by length
		// (antSort==false).  In that case, msShapeA_[BASELN]
		// is the number of baseline slots that _would_ be
		// required if the antenna sort were used, including
		// 1-element 'gaps' where antenna1 changes.
		// msShape_[BASELN] is the number of actual baselines
		// needed; the gaps are not needed when displaying the
		// sort by baseline length.  However, when able to fit
		// into memory, vis_ is sized according to the larger
		// msShapeA_[BASELN] in any case, so that switching
		// between sorts does not require resizing or
		// reloading vis_, but only reshuffling of the
		// baseline planes in memory.

		casacore::Bool antSort_;			// true (the default) means baselines are (to be)
		// sorted by antenna1-antenna2.  false means sorting
		// by (unprojected, uvw) baseline length.

		casacore::Matrix<casacore::Double> bLen_;			// (Unprojected) baseline lengths, indexed by antenna
		// numbers (symmetric, 0 along diagonal).  Used to
		// order baselines by length on request.

		casacore::Vector<casacore::Int> a1_, a2_,			// These Arrays provide quick conversions between
		       a1A_, a2A_,		// (antenna1,antenna2) and baseline index.  Of course
		       a1L_, a2L_,		// arguments ( a1_(bsl), a2_(bsl),  bsl_(a1, a2) )
		       len2ant_, ant2len_;			// must be Ints within range; their values are
		casacore::Matrix<casacore::Int> bsl_,			// according to the _current_ baseline sort
		       bslA_, bslL_;			// (antSort_).  (In contrast, methods a1a2_() and
		casacore::Int         nbsl_,			// bsln_() may take casacore::Float arguments, but convert
		            nbslA_, nbslL_;			// _only_ according to the Antenna1-Antenna2 sort).
		// The A and L versions are for antenna and length
		// sorts, respectively; they are copied into a1_, a2_,
		// and bsl_ in accordance with the current sort.
		// len2ant_ and ant2len_ provide conversion between
		// baseline indices for the two sorts.  nbsl* give
		// the number of baselines for the applicable case
		// (if sgl dish, they will be 1, but irrelevant;
		// msShape_[BASELN] will be set to number of feeds
		// instead).


		casacore::Vector<casacore::Double> time_;			// sorted vector of actual times in mssel_
		// only first msShape_[TIME] ( <= time_.shape() )
		// are valid.
		casacore::Vector<casacore::Int> field_;			// vector for field ids corresponding to time slots;
		// indexed as time_ is, above.  For now, field id
		// is assumed to be unique for a given time.  Used
		// to avoid computing running averages across
		// field boundaries.
		casacore::Vector<casacore::Int> scan_;			// same as above, for scan numbers.
		casacore::Vector<casacore::String> fieldName_;		// Names corresponding to field_ above.

		casacore::Block<casacore::Int> spwId_;			// Spectral window index-to-ID translation.
		// The user can select the spectral windows to
		// view.  This casacore::Block holds the (sorted) spectral
		// windows actually found in the selected casacore::MS--usually
		// it will be identical to the user selection (spwIds_,
		// above).  Its length is the size of the Spectral
		// Window axis (msShape_[SP_W]).
		// Note that throughout the code, the variable 'spw'
		// refers to the _index_ into this block, not the
		// Spectral window ID itself.  Actual IDs will have
		// 'Id' in the variable name.
		casacore::Block<void*> freq_;			// *freq_[spw] is really a casacore::Vector<casacore::Double>.
		// (*freq_[spw])[chan] holds the CHAN_FREQ for
		// the given spw index and channel (in Hz).

		// The following translate pol ID and the polarization index within a cell
		// of data in the casacore::MS, to the 'pol' index within the internal visibility
		// cube.  There is no 'polID axis' separate from the pol axis internally or
		// for the display; it is flattened to a single pol axis, using these
		// tables.  They are set up in findRanges_.
		casacore::Int nPolIds_;				// Number of rows in the POLARIZATION subtable
		// (and the size of the next two vectors).
		casacore::Vector<casacore::Int> nPolsIn_;			// number of correlations in each polId.
		casacore::Vector<casacore::Int> pidBase_;			// difference between a visibility's 'pol' index
		// within msShape_[POL] and its index within
		// the casacore::MS table's visibility data cell, for given
		// polId (or -1, if the polID is not in the
		// selected data).
		// The following 2 vectors will have sizes = msShape_[POL], and
		// are indexed according to the internal polarization 'data pixel
		// number' (generally referred to as 'pol').
		casacore::Vector<casacore::Int> polId_;			// polarizationId for given pol.
		casacore::Vector<casacore::String> polName_;		// name of the pol.


		// The visibility hypercube.

		casacore::Array<casacore::Float> vis_;			// the (large) memory buffer: 5-axis hypercube of
		// gridded casacore::MS visibilities (for t, bsl, chan, pol, spw, in that
		// order).  Conceptually, this is a (contiguous, hyper-rectangular)
		// 'window' or 'cursor' into the whole gridded ms as characterized
		// by msShape_ above.  It _will_ be the whole thing, if it fits
		// into memory; in any case, the two display axes will be full size.
		casacore::Bool visValid_;			// Is vis_ valid for current selected casacore::MS?
		casacore::Block<casacore::Int> visShape_;			// shape of extracted vis_ casacore::Array (used*) and
		casacore::Block<casacore::Int> visStart_;			// start of extracted vis_ casacore::Array, within msShape_

		casacore::Block<casacore::Int> visShapeA_;		// *Identical to visShape_ except possibly on the
		// BASELN axis, and then only when computeVisShape_()
		// determines that the entire msShapeA_[BASELN]
		// will fit into memory, and the length sort
		// is also in effect.  In that case,
		// visShapeA_[BASELN] == msShapeA_[BASELN] and
		// visShape_[BASELN] == msShape_[BASELN] (which is
		// msShapeA_[BASELN] - (nAnt_-1) ).
		// vis_ is actually sized according to visShapeA_.

		VisType curVisType_;
		VisComp curVisComp_;			// type and component of current vis_.

		casacore::Float dataRngMin_, dataRngMax_;	// The 'data ranges' for vis_.
		// Used (only) for scaling data values to colors; they are
		// too expensive to compute except during extract_().
		// Not the actual min/max of the data in general, since they
		// may be sampled and/or clipped to 3-sigma limits.
		casacore::Float devRngMin_, devRngMax_;		// same thing, for the case when
		// visibility deviations are being displayed.  Both types
		// of ranges are kept, in case the user switches from one
		// type ot display to the other.  devRngMin_ is set to
		// NO_DATA if these haven't been computed yet.
		// computeDevRange_() is called from extract_ or
		// setOptions to compute these when needed.


		// The display matrices which are drawn on the canvas.

		casacore::Matrix<casacore::Float> disp_; 			// casacore::Matrix of data values actually passed to the
		// display canvas.
		Axis dispX_, dispY_;			// displayed axes that disp_ represents.
		casacore::Block<casacore::Int> dispPos_;			// non-display axis ('slice') positions disp_
		// represents, by Axis.  Settings for display axes,
		// (i.e. dispPos_[dispX_] and dispPos_[dispY_]),
		// are irrelevant).
		casacore::Bool dispValid_;			// Has disp_ been created since extract_ was
		// called?
		casacore::Bool dispNotLoaded_;			// Is the entire disp_ casacore::Matrix set to the
		// NOT_LOADED value?

		casacore::Matrix<casacore::Float> dispDev_;		// Similar to disp_, but for visibility deviation
		// data displays.  (Filled by createDevSlice_()).
		casacore::Bool dispDevValid_;			// Is dispDev_ valid for current disp_ and
		// (if necessary) the state of flag edits?
		VisDev dispDevType_;			// Type of deviation (RMS or DIFF) that dispDev_
		// represents.
		casacore::Int dispDevNAvg_;			// Nominal number of values in running averages
		// in effect when dispDev_ was last computed.




		//---------------additional control state----------------------------------
		//  ...for communication of control logic between various methods (mainly
		//  setOptions, draw_ and extract_)

		casacore::Bool visDataChg_;	// means that casacore::MS selection, visType_ or visComp_
		// do not reflect current state of vis_.  Set by
		// setOptions.  Indicates to extract_ that it should
		// completely recalculate data ranges (and set
		// the actual dataMin/Max_ sent to the scale handler
		// accordingly).  setOptions also uses it to help
		// determine whether it should call extract_.

		casacore::Bool postDataRng_;	// A kind of reply to the above; set true after
		// extract_ has recalculated data ranges from
		// scratch (as opposed to merely expanding them).
		// When true, setOptions will return these new
		// ranges unaltered to the gui, via recOut.


		//----translation between casacore::MS values and internal hypercube indices---------

		struct MSpos_;
		friend struct MSpos_;
		struct MSpos_ {

			// An MSpos_ holds information about a given position of interest within
			// the main casacore::MS visibility Table.  Its data members are the values at
			// that position as actually stored in the casacore::MS (times, antennas, etc.);
			// these can be freely set and queried by MSAsRaster, the only user of
			// this struct.  It provides translation between these values and the
			// hypercube indices (pos_) used internally.  It was time to quit
			// performing these translations ad hoc and consolidate them here (3/04).
			// (Still to do: use this instead of old ad hoc methods in, e.g.,
			// showPosition()).

			// When casacore::MS selection changes, MSAsRaster::findRanges_() computes several
			// tables (time_, spwId_, freq_, polId_, nAnt_, etc.) which [re-]define
			// the correspondence of casacore::MS data to hypercube positions.  An MSpos_
			// set up prior to this change can be queried afterward as to the data's
			// position (if any) in the new hypercube.  It is also useful in
			// determining whether flagging edits (which are stored in terms of
			// hypercube positions) apply to casacore::MS data beyond what is currently
			// selected.

			// Below is the real content of an MSpos_: data for a position within
			// an casacore::MS in terms of the values actually stored there.
			// Note that, in its current version, MSAsRaster assumes that this
			// data uniquely determines a visibility-and-flag within the casacore::MS (and
			// feed is not considered either, when there is more than one antenna).

			casacore::Double time;
			casacore::Int ant1, ant2, feed, spwId, polId, chan;
			casacore::String polName;

			// IMO, a _nested_ struct/class should have visibility (and access
			// permission(?)) into the nesting class, but it doesn't (except, rather
			// oddly, for the static consts of MSAsRaster: TIME, NAXES, etc.,
			// which are directly in scope and accessible here).   Hence the
			// need to have a pointer to the nesting object and for it to declare
			// this class a friend.  The nesting class's 'this' should be
			// available implicitly to this class instead.  As is, it's
			// rather clumsy to implement classes/structs that are purely in support
			// of another class....

			const MSAsRaster* m;	// parent dd, needed for access to the
			//  MSAsRaster tables which it set up in
			// findRanges_: nAnt_, time_, spwId_, freq_,
			// polId_, etc.

			casacore::Int ts, te;		// Ancillary output of t() (in terms of hypercube
			// time-slot indices).  When t() returns INVALID (time
			// not found) they are the bracketing time slots, i.e.
			// ts+1==te and (where ts or te are in timeslot range)
			// time_[ts] < time < time_[te].


			// Construct MSpos_ with no valid values set (yet).

			MSpos_(const MSAsRaster* msar):
				time(INVALID), ant1(INVALID), ant2(INVALID), feed(INVALID),
				spwId(INVALID), polId(INVALID), chan(INVALID), polName("Invalid"),
				m(msar), ts(INVALID), te(INVALID) {   }


			// Construct MSpos_ according to 5-element casacore::Block corresponding to a
			// position within the internal hypercube.

			MSpos_(const MSAsRaster* msar, const casacore::Block<casacore::Int>& pos): m(msar) {
				set(pos);
			}


			// Translation methods (for times, baselines, channels,
			// spectral windows, and polarizations).  All but the last seven
			// set MSpos_ state to the appropriate values as stored in the MS.

			// hypercube indices to casacore::MS values

			void set(const casacore::Block<casacore::Int>& pos) {
				sett(pos[TIME]);
				setb(pos[BASELN]);
				sets(pos[SP_W]);
				setc(pos[CHAN]);
				setp(pos[POL]);
			}

			void sett(casacore::Int t);
			void setb(casacore::Int b);
			void sets(casacore::Int s);
			void setc(casacore::Int c);
			void setp(casacore::Int p);

			// casacore::MS values to hypercube indices.

			casacore::Int t(casacore::Double tm) {
				time=tm;
				return t();
			}
			casacore::Int b(casacore::Int a1, casacore::Int a2) {
				ant1=a1;
				ant2=a2;
				feed=INVALID;
				return b();
			}
			casacore::Int b(casacore::Int fd) {
				feed=fd;
				ant1=ant2=INVALID;
				return b();
			}
			casacore::Int s(casacore::Int sid) {
				spwId=sid;
				return s();
			}
			casacore::Int c(casacore::Int ch) {
				chan=ch;
				return c();
			}
			casacore::Int c(casacore::Int sid, casacore::Int ch) {
				spwId=sid;
				chan=ch;
				return c();
			}
			casacore::Int p(casacore::Int pid, casacore::Int pnm) {
				polId=pid;
				polName=pnm;
				return p();
			}
			casacore::Int p0(casacore::Int pid) {
				polId=pid;
				return p0();
			}

			casacore::Int p0() {
				if(polId<0 || polId>=casacore::Int(m->pidBase_.nelements())) return INVALID;
				return m->pidBase_[polId];
			}

			casacore::Int operator[](Axis ax) {
				switch(ax) {
				case   TIME:
					return t();
				case BASELN:
					return b();
				case   SP_W:
					return s();
				case   CHAN:
					return c();
				case    POL:
					return p();
				default:
					return INVALID;
				}
			}

			casacore::Int t();
			casacore::Int b();
			casacore::Int s();
			casacore::Int c();
			casacore::Int p();
		};



		MSpos_ mspos_;	// used by findRanges_() to maintain the casacore::MS 'slice'
		// position being viewed as much as possible when
		// casacore::MS selection is changed--e.g., to remain on the
		// same time (if it is still in the selected casacore::MS),
		// even though its time-slot index (slice position)
		// may have changed.


		//----flagging state-------------------------------------------------------

		DParameterChoice *itsFlagColor;
		casacore::Bool flagsInClr_;	// equivalent to  itsFlagColor->value()=="In Color"

		DParameterChoice *itsUnflag;
		casacore::Bool unflag_;	   	// equivalent to  itsUnflag->value()=="Unflag"

		casacore::Block<casacore::Bool> flagAll_;    // from UI checkboxes for each axis (there is
		// no checkbox-type DParameter yet)..

		DParameterChoice *itsEntireAnt;
		// choice box for applying edits to entire antenna.
		casacore::Bool entireAnt_;	  // equivalent to  itsEntireAnt->value()=="Yes"

		DParameterButton *itsUndoOne;
		// defines the button for undoing one edit.
		DParameterButton *itsUndoAll;
		// defines the button for undoing all edits.
		DParameterChoice *itsEditEntireMS;
		// choice box for using entire casacore::MS (vs. selected
		// casacore::MS only) when saving edits.
		DParameterButton *itsSaveEdits;
		// defines the button for saving all edits.

		casacore::Vector<casacore::uInt> flags_;	   // Bitmapped storage for flags retrieved
		// from the casacore::MS (only); not for new, unsaved flags.
		// Virtually, this has the same shape as vis_.

		casacore::Matrix<casacore::Bool> dispMask_;  // Mask for disp_ or dispFlags_.  Distinguishes
		// good data from flagged/missing data.  Which
		// is true and which false depends on flagsInClr_.
		casacore::Matrix<casacore::Float> dispFlags_;  // For the (slower) 'color flags' option, this
		// will overlay the main data array to show
		// flags in color.
		casacore::Bool dispFlagsInClr_;	     // Are the current display matrices set up to
		// draw flags in color?
		casacore::uInt dispNEdits_;	     // Number of flag edits already reflected
		// in display matrices and masks.



		// FlagEdit_ holds information about a single
		// (new, unsaved) flagging edit command.

		struct FlagEdit_;
		friend struct FlagEdit_;
		struct FlagEdit_ {


			MSAsRaster* msar;	// parent dd, needed for access to its members.

			// All 3 of these Blocks are indexed by Axis, and have NAXES elements.
			casacore::Block<casacore::Bool> all;	// all[axis]==T: apply edit to entire axis.
			casacore::Block<casacore::Int> start;   // all[axis]==F: apply edit from start[axis] up to
			casacore::Block<casacore::Int> shape;   // (but not including) start[axis]+shape[axis].

			// Note that all[axis] will imply start[axis]==0 &&
			// shape[axis]==msShape_[axis], but _not vice-versa_.
			// all[axis] has the _additional_ meaning that the
			// edit should extend beyond hypercube (selected
			// casacore::MS) boundaries, to the _entire_ casacore::MS, when saving
			// flags (if itsEditEntireMS is turned on).


			casacore::Bool unflag;	// T=unflag  F=flag

			casacore::Bool entireAnt;	// T=apply to all baselines with Antenna1 of selection.

			// The following are set by appliesToChunk(spw, nChan, pol0, nPol) at
			// the beginning of each chunk, within saveEdits_().
			// A given chunk has a fixed polId and spwId, which imply a given
			// shape (nPol, nChan) of visibilities and flags as actually stored in
			// the MS.

			casacore::Bool appChunk;	// Does the edit apply to the spectral window and
			// at least some of the pols and channels implied by
			// the chunk's data description ID?
			casacore::Int sPol,ePol, sChan,eChan;		// If appChunk is true, (and the
			// edit applies to the row's time and baseline -- tested
			// elsewhere), then the visibilities in the casacore::MS row where
			// the edit applies have pol and channel index ranges
			// [sPol,ePol) and [sChan,eChan).  (Note that these are
			// _different_ index ranges from those determined by
			// start & shape, above, which are relative to the
			// _internal hypercube_ shape, msShape_).

			FlagEdit_(MSAsRaster* m):		// constructor.
				msar(m),
				all(NAXES, false), start(NAXES), shape(NAXES, 1),
				unflag(false), entireAnt(false),
				appChunk(false), sPol(0),ePol(0), sChan(0),eChan(0) {   }

			// Return the range (half-open interval) over which the edit definitely
			// applies*, on the given axis.  This is the range selected with the
			// mouse or (when edit.all[ax]==true) the entire axis range.
			//
			// * Exception: when antenna-based editing is on (entireAnt==true),
			// baselines of only one antenna will be edited regardless -- see
			// appliesTo(casacore::Int bsln).  The interval returned is still as stated
			// above, however).
			void getSureRange(Axis ax,  casacore::Int& strt, casacore::Int& fin) {
				strt=start[ax];
				fin=strt+shape[ax];
			}

			// Is the given position within the range above, on the given axis?
			casacore::Bool inSureRange(Axis ax, casacore::Int pos) {
				return start[ax]<=pos && pos<start[ax]+shape[ax];
			}

			// Retrieve ranges for loops.  Same as getSureRange(), except for
			// antenna-based baseline testing, when the entire baseline range
			// of the selected casacore::MS is returned.  In that case, applicability
			// of the edit to the individual baselines must still be tested
			// within the loop, with edit.appliesTo(bsln).
			void getLoopRange(casacore::Int ax,  casacore::Int& strt, casacore::Int& fin) {
				if(entireAnt && ax==BASELN) {
					strt=0;
					fin=msar->msShape_[ax];
				} else getSureRange(ax, strt,fin);
			}

			// Is the given position in the 'loop range' on the given axis?
			casacore::Bool inLoopRange(casacore::Int ax, casacore::Int pos) {
				casacore::Int strt, fin;
				getLoopRange(ax, strt,fin);
				return (strt<=pos && pos<fin);
			}

			// Does edit apply to given hypercube position (relative to
			// entire casacore::MS)?  (pos must be NAXES in size).
			casacore::Bool appliesTo(casacore::IPosition pos) {
				for(Axis ax=0; ax<NAXES; ax++) if(!appliesTo(ax, pos(ax))) return false;
				return true;
			}

			// Does edit apply to given position on given axis?
			casacore::Bool appliesTo(Axis ax, casacore::Int pos) {
				return inLoopRange(ax, pos) && applies2(ax, pos);
			}

			// Same as above, but for use (only) where inLoopRange(ax, pos)
			// is already known to be true.  This one is used within loops,
			// for efficiency.
			casacore::Bool applies2(Axis ax, casacore::Int pos) {
				return ax!=BASELN || !entireAnt || appliesTo(pos);
			}

			// Does edit apply to the given baseline?  (Also for use only where
			// inLoopRange(BASELN, bsln) is already known to be true).  This is
			// used mainly for testing baselines against antenna-based edits.
			casacore::Bool appliesTo(casacore::Int bsln);

			// Does the edit apply to the current casacore::MS iteration chunk?
			casacore::Bool appliesToChunk(casacore::Int pol0, casacore::Int nPol, casacore::Int spw, casacore::Int nChan);

			// Does the edit apply to a given (raw) time?
			casacore::Bool appliesTo(casacore::Double time);

			casacore::Bool operator==(FlagEdit_& other);
		};




		casacore::List<void*> flagEdits_; 	// casacore::List of all the (so-far-unsaved) edits.
		// (<void*> rather than <FlagEdit_*> just to avoid the extra templates)


		//---state for computing [RMS] deviation from local visibility average---
		// (Not very object-oriented--more like Fortran common.  Sorry, pressed
		// for time.  To be reworked into object(s) sometime, if it can be
		// done efficiently).

		casacore::Vector<casacore::Int> lsTime_, leTime_,  lsvTime_, levTime_;
		// Beginning and (1 beyond) ending time index defining the 'boxcar'
		// or local neighborhood over which averages are computed, for given
		// time slot.  (Note that, for now, these are assumed to be only a
		// function of time.  Later, these may become Matrices, indexed
		// also by casacore::Array ID or baseline).  Recomputed (computeTimeBoxcars_)
		// when nAvg_ changes, and after extract_.
		// lsv, lev versions are for vis_, i.e relative to visStart_[TIME],
		// and of length visShape_[TIME].

		casacore::Bool useVis_;
		casacore::IPosition dPos_;
		AxisLoc axlTm_;
		casacore::Bool flgdDev_;
		// Initializing input to dev_(t) and its subsidiary routine v_(t).
		// These create something similar to an casacore::ArrayAccessor, in that
		// vis_ (or disp_, depending on useVis_) can be accessed by giving
		// only the value along the time axis.  dPos_ fixes the row of times
		// to use within vis_ or disp_; axlTm_ gives the location of the
		// time axis within dPos_.  The value of the index along that
		// axis (only) will be varied as needed, by dev_() and v_().

		// flgdDev_ should usually be set false, causing dev_ simply to
		// return INSUF_DATA for flagged points.  In one obscure case, it
		// is set true to request dev_ to calculate deviations even for
		// the flagged points.

		casacore::Bool goodData_;
		// This really should be a return value from v_(); placed here for
		// 'efficiency'.  Set after each call to v_(): true iff the data
		// existed, was loaded, and was not flagged.

		casacore::Int sT_, eT_;
		casacore::Double sumv_, sumv2_, sumva_, sumv2a_;
		casacore::Int nValid_;
		casacore::Float d_;
		// Saved state from the last call to dev_().  When moving along
		// a time axis, it sometimes saves time to have these previous
		// results handy.  d_ is the most recently computed deviation.
		// sT_ (boxcar start time slot) must be set to -1 when starting to
		// compute deviations on a new line of times, indicating that
		// none of this 'saved state' is valid yet.




		//----------helper objects and their control data-----------

		// This does data scaling for WorldCanvas
		WCPowerScaleHandler itsPowerScaleHandler;

		// This labels the axes.  An actual CachingDisplayData itself, contained
		// within MSAsRaster and controlled through it.  MSAsRaster propagates
		// getOptions and setOptions calls and draw commands to it, and sets the
		// WC DisplayCoordinateSystem which it uses.
		WorldAxesDD itsAxisLabeller;

		// A private internal colormap for showing colors for various conditions
		// (flagged, no data, data not loaded).  This is not the colormap set
		// by the user for mapping data values.  It has rigid single colors
		// for the different conditions.
		Colormap flagCM_;

	private:

		bool adjustAvgRange( VisDev newstate, casacore::Record &outrec, bool force=false );

	};



//#----------MSAsRasterDM----------------------------------------------------

// <summary>
// (Minimal) DisplayMethod for MSAsRaster.
// </summary>

// <prerequisite>
//   <li> MSAsRaster
//   <li> CachingDisplayMethod
// </prerequisite>

// <etymology>
// "MSAsRasterDM" is a implementation of a <linkto class=CachingDisplayMethod>
// CachingDisplayMethod </linkto> for <linkto class=MSAsRaster>
// MSAsRaster </linkto>.
// </etymology>

// <synopsis>
// MSAsRasterDM a minimal skeleton; it is implemented
// in its entirety here.  Its only purpose is to hold the cached drawlist
// and use it when appropriate via the mechanism and data structure
// implemented on the CachingDisplayMethod level.  MSAsRasterDM just
// turns the draw request back over to MSAsRaster, since that's where
// the necessary data is.

// This 'minimal' CachingDisplayMethod could be adapted and reused by any
// other CachingDisplayData that wanted to do its own drawing.

// </synopsis>

	class MSAsRasterDM : public CachingDisplayMethod {

	public:

		// Constructor.  The parameters contain state that determines
		// what should be drawn.
		MSAsRasterDM(WorldCanvas *wc,	AttributeBuffer *wchAttrs,
		             AttributeBuffer *ddAttrs, CachingDisplayData *dd):
			CachingDisplayMethod(wc, wchAttrs, ddAttrs, dd) {  }

		// Destructor.
		virtual ~MSAsRasterDM() {  }

	protected:

		// The base CachingDM takes care of using any cached drawlist.
		// When this method is called, we know that no drawlist applies,
		// and that we must actually send drawing commands to the WC (though
		// technically, they might _not_ actually be going into a drawlist).
		// This skeleton DM just hands the drawing task back to the DD,
		// where all the data has been created and maintained anyway.

		virtual casacore::Bool drawIntoList(Display::RefreshReason reason,
		                          WorldCanvasHolder &wcHolder) {
			MSAsRaster *msar = dynamic_cast<MSAsRaster *>(parentDisplayData());
			if (!msar) throw(casacore::AipsError("invalid parent of MSAsRasterDM"));

			return msar->draw_(reason, wcHolder, *(worldCanvas()) );
		}


	private:

		// Default and copy constructors, and the assignment operator, are
		// mon-functional and should not be used.  Do not make copies of
		// DisplayMethod objects, or pass them by value;
		// use references or pointers instead.
		// <group>
		MSAsRasterDM() {  }
		MSAsRasterDM(const MSAsRasterDM &other) : CachingDisplayMethod(other) {  }
		MSAsRasterDM& operator=(const MSAsRasterDM &/*other*/) {
			return *this;
		}
		// </group>

	};


} //# NAMESPACE CASA - END

#endif

