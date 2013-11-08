//# QtDisplayPanel.qo.h: Qt implementation of viewer display Widget.
//# Copyright (C) 2005
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

#ifndef QTDISPLAYPANEL_H
#define QTDISPLAYPANEL_H

#include <casa/aips.h>
#include <display/Display/Colormap.h>
#include <casa/Containers/List.h>
#include <display/QtViewer/QtPixelCanvas.qo.h>
#include <display/QtViewer/QtMouseTools.qo.h>
#include <display/QtViewer/QtOldMouseTools.qo.h>
#include <display/QtViewer/DisplayDataHolder.h>
#include <display/QtViewer/ImageManager/ImageTracker.h>
#include <display/Display/PanelDisplay.h>
#include <images/Regions/ImageRegion.h>
#include <display/DisplayEvents/MWCRTZoomer.h>
#include <display/DisplayEvents/PCPositionEH.h>
#include <display/QtPlotter/QtMWCTools.qo.h>
#include <display/QtPlotter/QtOldMWCTools.qo.h>
#include <display/region/QtRegionSourceFactory.h>
#include <display/Display/Position.h>

#include <graphics/X11/X_enter.h>
#  include <QtCore>
#  include <QtGui>
#  include <QTimer>
#  include <QDebug>
#  include <QtXml>
#include <graphics/X11/X_exit.h>


namespace casa { //# NAMESPACE CASA - BEGIN


	class QtDisplayPanelGui;
	class QtDisplayData;
	class DisplayData;
	class PCITFiddler;
//class MWCCrosshairTool;
	class MWCPannerTool;
	class MWCPolylineTool;
	class MWCRulerlineTool;
	class MWCPositionVelocityTool;
//class MWCPTRegion;
	class DParameterChoice;
	class RegionShape;
	class QtRegionShapeManager;

	namespace viewer {
		class RegionToolManager;
		class Region;
	}

	class QtDisplayPanel : public QWidget,
		public WCMotionEH,  public PCPositionEH, public ImageDisplayer {

		Q_OBJECT	//# Allows slot/signal definition.  Must only occur in
		//# implement/.../*.h files; also, makefile must include
		//# name of this file in 'mocs' section.

	public:

		// included in cursor boundary signals...
		enum CursorBoundaryCondition { ENTER, LEAVE };

		class panel_state {
		public:

			class colormap_state {
			public:
				colormap_state( const std::string &n, const Vector<Float> &s, const Vector<Float> &b ) : name_(n), shift_(s), brightness_(b) { }
				colormap_state( const colormap_state &other ) : name_(other.name_), shift_(other.shift_), brightness_(other.brightness_) { }
				const std::string &name( ) const {
					return name_;
				}
				const Vector<Float> &shift( ) const {
					return shift_;
				}
				const Vector<Float> &brightness( ) const {
					return brightness_;
				}
				const std::string &colormap( ) const {
					return name_;
				}
			private:
				std::string name_;
				Vector<Float> shift_;
				Vector<Float> brightness_;
			};

			typedef std::pair<Vector<Double>,Vector<Double> > zoom_state;
			typedef std::map<std::string,colormap_state> colormap_map;

			panel_state( const panel_state &other ) : zoom_(other.zoom_) { }
			const Vector<Double> &blc( ) const {
				return zoom_.first;
			}
			const Vector<Double> &trc( ) const {
				return zoom_.second;
			}
			const colormap_state *colormap( const std::string &s ) const;

		private:
			panel_state( const zoom_state &z_, const colormap_map &cm ) : zoom_(z_), colormaps_(cm) { }

			zoom_state zoom_;
			colormap_map colormaps_;

			friend class QtDisplayPanel;
		};


		QtDisplayPanel( QtDisplayPanelGui* panel, QWidget* parent=0,
		                const std::list<std::string> &args = std::list<std::string>( ) );
		~QtDisplayPanel();

		Bool isEmptyRegistered() const;
		int getRegisteredCount() const;
		//Manipulation of the QtDisplayData's.
		DisplayDataHolder::DisplayDataIterator beginRegistered() const;
		DisplayDataHolder::DisplayDataIterator endRegistered() const;
		QtDisplayData* getDD( const std::string& name ) const;
		QtDisplayData* getDD( const DisplayData *dd ) const;
		DisplayDataHolder* getDataHolder() const;
		void setControllingDD( QtDisplayData* controllingDD );
		// True if DD is on our list.  (It may _not_ be on viewer's list
		// any longer, in particular when reacting to ddRemoved signal).
		// Either qdd pointer or its name can be given.
		//<group>
		virtual Bool isRegistered(QtDisplayData*);
		//</group>


		//Returns the DD that will be animating the channels
		//in normal mode.
		QtDisplayData* getChannelDD( int index ) const;

		//Reorders panels in response to a change in the registration order.
		void registrationOrderChanged();

		// Return Options record (of margins and no.-of-panels settings, e.g.)
		// The form of the record is suitable for automatically creating
		// controlling user interface.  These options are set with the
		// corresponding setOptions() slot (below).
		virtual Record getOptions();


		/*   QtViewerBase* viewer() { return v_;  } */

		// Return a QPixmap* with a copy of currently-displayed widget contents.
		// Caller is responsible for deleting.
		virtual QPixmap* contents() {
			return pc_->contents();
		}
		virtual QPixmap* getBackBuffer() {
			return pc_->getBackBuffer();
		}

		// Return underlying library PanelDisplay (used by Annotations, e.g.)
		// (You better know what you're doing if you reach underneath the Qt layer.
		// You are responsible for consistency between the layers, etc...).
		virtual PanelDisplay *panelDisplay() {
			return pd_;
		}

		// Hold and release of refresh.  In order to draw, every call to hold()
		// must be accompanied by a subsequent call to release() (so don't
		// neglect: beware of exceptions, e.g.).  Calls can nest (they are
		// counted).  Panel may be deleted in a held state.  Also, excess calls
		// to release() will have no effect.  The calls are propagated to the main
		// PanelDisplay as well as to those used for color bars (and thence to
		// their WorldCanvases).
		//<group>
		virtual void hold();
		virtual void release();
		//</group>

		// Return names of resident mouse tools (order is a suggestion
		// for order in gui).
		virtual Vector<String> mouseToolNames() {
			return mouseToolNames_;
		}


		// This callback method is automatically registered with the
		// appropriate objects, and is not intended for general use (it
		// has a similar role to a protected, self-connected slot).
		// Overrides (null) base WCMotionEH operator to forward cursor
		// position tracking events from any of the panel's WorldCanvases
		// as Qt signals.
		virtual void operator()(const WCMotionEvent &ev);

		// This callback method is automatically registered with the
		// appropriate PC object, and is not intended for general use (it
		// has a similar role to a protected, self-connected slot).
		void operator()(const PCPositionEvent &pev) {
			//# (Its only current role is to toggle a
			//# freeze of cursor tracking via the space bar).
			if(pev.key()==Display::K_space) tracking_ = !tracking_;
		}


		// Return total number of (main) subpanels (e.g. 12 for 3x4-panel display).
		Int nPanels() {
			return pd_->nWCHs();
		}


		//# animation

		//# ( Updates to user interface will likely need only these:
		//#   modeZ() (or mode()),
		//#   nFrames(), frame(),
		//#   startFrame(), lastFrame(), step(),
		//#   animRate(), minRate(), maxRate(), animating()  ).

		virtual Bool modeZ()  {
			return modeZ_;
		}
		virtual String mode() {
			return modeZ()?  "Normal" : "Blink";
		}

		virtual Int nFrames()  {
			return modeZ()?  nZFrames() : nBFrames();
		}
		virtual Int nZFrames() {
			return zLen_;
		}
		virtual Int nBFrames() {
			return bLen_;
		}

		virtual Int frame()  {
			return index();
		}
		virtual Int index()  {
			return modeZ()?  zIndex() : bIndex();
		}
		virtual Int zIndex() {
			return zIndex_;
		}
		virtual Int bIndex() {
			return bIndex_;
		}

		virtual Int startFrame()  {
			return modeZ()?  zStart() : bStart();
		}
		virtual Int zStart() {
			return zStart_;
		}
		virtual Int bStart() {
			return bStart_;
		}

		virtual Int lastFrame() {
			return endFrame()-1;
		}
		virtual Int endFrame()  {
			return modeZ()?  zEnd() : bEnd();
		}
		virtual Int zEnd() {
			return zEnd_;
		}
		virtual Int bEnd() {
			return bEnd_;
		}
		//# NB: frame() <  endFrame()  (<--used internally)
		//# but frame() <= lastFrame() (<--shown in ui)

		virtual Int step()  {
			return modeZ()?  zStep() : bStep();
		}
		virtual Int zStep() {
			return zStep_;
		}
		virtual Int bStep() {
			return bStep_;
		}

		virtual Int animRate()  {
			return animRate_;
		}
		virtual Int minRate()   {
			return minRate_;
		}
		virtual Int maxRate()   {
			return maxRate_;
		}
		virtual Int animating() {
			return animating_;
		}




		//# region management


		// Transform origPath to a plausible pathname for a new image region file.
		// Assures the filename ends in '.rgn' and that no such file already exists.
		static String regionPathname(String origPath);

		// Save the last region created (via an image DD) with the mouse.
		// Return value indicates success.
		// (The ImageRegion is transformed to a TableRecord then saved via AipsIO).
		Bool saveLastRegion(String path);

		// same as above except it is saved in the last registered image as a keyword

		String saveRegionInImage(String regname, const ImageRegion& imreg);

		// Delete the region regname from last registered image
		// does not care if its RegionHandler::Masks or  RegionHandler::Regions
		void removeRegionInImage(String regname);

		// give the regions already stored in last dd image
		// a coma separated list
		String listRegions();
		// a vector list
		Vector<String> listRegionsInImage();
		// get an ImageRegion from active Image
		ImageRegion getRegion(const String& name);

		// Set current extension policy to use to for (subsequent) image region
		// creation (and for printed region statistics):
		// 0: Region should apply to viewed plane only.
		// 1: Extend to all channels (iff spectral axis not on display).
		// 2: Extend along all non-displayed axes.
		// (The mouse polygon/rectangle always determines the region on the
		// displayed axes).
		//
		// The default policy is 0.
		void setRegionExtent(Int ext) {
			rgnExtent_ = max(0, min(2,  ext  ));
		}

		// Return the last region created (via an image DD) with the mouse.
		ImageRegion lastRegion() {
			return lastRgn_;
		}

		Bool hasRegion() {
			return hasRgn_;
		}




		//# 'region' Shapes (unrelated to mouse regions / ImageRegions;
		//# more like annotations.)


		// Sets the region shape manager from QtDisplayPanelGui.
		void setShapeManager(QtRegionShapeManager* manager);

		// Register the given RegionShape on this display panel.
		void registerRegionShape(RegionShape* rs);

		// Unregister the given RegionShape on this display panel.
		void unregisterRegionShape(RegionShape* rs);

		// Returns true if the given RegionShape is registered on this display panel,
		// false otherwise.
		Bool isRegistered(RegionShape* rs);




		//# save-restore


		// Suggest name for restore file (based on first-registered DD).
		String suggestedRestoreFilename();

		// Returns an xml String of displayPanel state.  This includes registered
		// DDs and their options, panel options, animation and zoom state, etc.
		// (QtDisplayPanelGui adds some gui/window state to this, and has
		// file-saving interface).
		// If a restorefilename where you intend to store the state is given, it
		// will be set as attribute 'restorefile' of the root node.  This allows
		// data files to be moved and still restored later, relative to the restore
		// file location.
		String dpState(String restorefilename="");

		// Save panel state to a file (as xml).  State is also stored to an
		// internal lastSavedState_ String (only there, if filename=="").
		// By default, the file will be overwritten if necessary.
		virtual Bool savePanelState(String filename="", Bool overwrite=True);

		// Restore panel state from a file or from lastSavedState_ if filename=="".
		virtual Bool restorePanelState(String filename="");

		// set panel state from xml String (lastSavedState_, by default).
		virtual Bool setPanelState(String xmlState="");

		// [re]set panel state from a QDomDocument.
		virtual Bool setPanelState(QDomDocument& restoredoc,
		                           QString restorefiledir="");

		panel_state getPanelState( ) const;
		void setPanelState( const panel_state & );

		// *new* Region code  --  load casa (or DS9?) region files...
		void loadRegions( const std::string &path, const std::string &type );
		// *new* Region code  --  revoke region from region source...
		void revokeRegion( viewer::Region *r );

		// called to indicate application activation state... true -> activated, false -> deactivated
		void activate( bool );


	public slots:

		//respond to region manager to set ImageRegion extension
		void extendRegion(String, String);

		// Register / unregister [all] DDs created by user through QtViewer.
		//<group>
		virtual void registerDD(QtDisplayData*, int postion = -1);
		virtual void unregisterDD(QtDisplayData*);
		virtual void unregisterAll();
		virtual void registerAll(List<QtDisplayData*> registerDatas);
		//</group>

		// Set display panel options such as margins or number of panels.  The
		// form of the record (along with current settings) is returned by
		// getOptions().  (These Records are an older form of 'parameter sets'
		// which are used in various places within the display library).
		// Set emitAll = True if the call was not initiated by the options gui
		// itself (e.g. via scripting or save-restore); that will assure that
		// the options gui does receive all option updates (via the optionsChanged
		// signal) and updates its user interface accordingly.
		virtual void setOptions(Record opts, Bool emitAll=False);



		//# animation

		virtual void setMode(bool modez, bool channelCubes = false);
		virtual void setMode(String mode) {
			setMode(downcase(mode)=="normal");
		}

		virtual void toStart() {
			goTo(startFrame());
		}
		virtual void toEnd()   {
			goTo(lastFrame());
		}
		virtual void revStep() {
			stop_();
			prev_();
		}
		virtual void fwdStep() {
			stop_();
			next_();
		}
		virtual void revPlay();
		virtual void stop();		// slots corresp. to tapedeck buttons.
		virtual void fwdPlay();
		virtual void setRate(int rate);
		void lowerBoundAnimatorImageChanged( int );
		void upperBoundAnimatorImageChanged(int);
		void stepSizeAnimatorImageChanged(int);
		void lowerBoundAnimatorChannelChanged( int );
		void upperBoundAnimatorChannelChanged(int);
		void stepSizeAnimatorChannelChanged(int);


		virtual void goTo(int frm, bool channelFrame=false) {
			if(modeZ() || channelFrame ) goToZ(frm);
			else goToB(frm);
		}
		//# Note: connected to std Qt signal which takes 'int'.
		//# As of Qt4.1.3, declaring goTo(Int frm) will no longer
		//# do (which is a bit of a pain...).  (Actually, though,
		//# it is very unclear to me that having casa Ints, Floats,
		//# et. al. buys us anything at all...).
		virtual void goToZ(int frm);
		virtual void goToB(int frm);

		virtual void emitAnimState() {
			emit animatorChange();
		}

		//# mouse tools

		// (Will remove mouse-tools' own visual feedback from screen;
		// usually called after selection has been processed).

		//<group>
		virtual void resetRTRegion();
		virtual void resetPTRegion();
		virtual void resetETRegion();
		virtual void resetZoomer();	//# (NB: != unzoom(); no zoom effect).
		virtual void resetCrosshair();
		virtual void resetPolyline();
		virtual void resetPanner();

		virtual void resetRegionTools();
		virtual void resetSelectionTools();

		virtual void resetTool(String toolname);
		virtual void resetTools();
		//</group>


		// Expose Zoomer functions: zoom in/out, zoom out to whole image.
		//<group>
		virtual void zoomIn (Double fctr=2.) {
			if(zoom_!=0) zoom_->zoomIn (fctr);
		}
		virtual void zoomOut(Double fctr=2.) {
			if(zoom_!=0) zoom_->zoomOut(fctr);
		}
		virtual void zoom (const Vector<Double> &blc, const Vector<Double> &trc ) {
			if ( zoom_ != 0 ) zoom_->zoom( blc, trc );
		}
		virtual void unzoom() {
			if(zoom_!=0) zoom_->unzoom();
		}
		//</group>


		Bool worldToLin(Vector<Double> &lin, const Vector<Double> &world);

		// Returns the pixel canvas's current size.
		QSize canvasSize() {
			return pc_->size();
		}



		// Refresh everything: main panel and colorbar panels (if any).
		virtual void refresh() {
			pd_->refresh();
			refreshCBPanels_();
		}
		virtual void setBlen_(Int len);

		viewer::RegionToolManager *toolMgr( ) { return toolmgr; }

	signals:



		void animatorChange();
		//# (Updating entire animator gui when any animator
		//# change occurs is plenty fast, it turns out...).


		// signals from registration methods.
		//
		// If you connect to any of these registrationChange signals
		// and/or to ddCreated or ddRemoved signals (from QtViewerBase),
		// the following are true when the slot is invoked:
		// * Any removed QDD still exists until after the signal's
		//   _directly_ connected slots have returned.
		// * Any new QDD has been created before the corresp. signal is emitted.
		// * The list of existing QDDs (QtViewerBase::dds()) and [un]registered
		//   QDDS (registeredDDs(), unregisteredDDs()) reflect the new state.
		//
		// However, if you connect to a registrationChange signal _and_ to
		// ddCreated or ddRemoved, it is not certain which slot will be
		// invoked first.
		//<group>

		//#dk (These signals are the ones most suitable for the regMenu and
		//#dk  other parts of the gui to react to....  They are emitted
		//#dk _after_ respective action has taken place, but QDD still exists).
		void oldDDRegistered(QtDisplayData*);		//# reg. status change
		void oldDDUnregistered(QtDisplayData*);	//# on pre-existing DDs.
		void newRegisteredDD(QtDisplayData*);		//# new DD creation, with
		void newUnregisteredDD(QtDisplayData*);	//# new reg. status.
		void RegisteredDDRemoved(QtDisplayData*);	//# DD removal from viewer,
		void UnregisteredDDRemoved(QtDisplayData*);	//# with former reg. status.
		void allDDsRegistered();	//# (may be emitted _instead_ of
		void allDDsUnregistered();	//#  oldDD[Un]registered, above).

		void registrationChange();	//# Any of above occurred; usually
		//# simplest just to connect to this one.
		//</group>


		//# signals arising from 'region' mouse tools.

		// This raw mouse region signal is probably less useful to connect
		// to than the ones processed through the DDs (below), which contain
		// true 'image regions'.
		void mouseRegionReady(Record mouseRegion, WorldCanvasHolder*);


		// Higher-level (ImageRegion) signal.  ddName is the name() of a
		// registered [Lattice] QDD which conforms to the panel's current
		// state (see DD:conformsTo()) and can compute true-region information
		// from the latest 'mouse tool region event'.  The ImageRegion
		// parameter has the dimensions [and CS] of the DD's Image
		// (currently, only Image PADDs are supported).  It will correspond
		// to the 2-D mouse region on its display axes. On 'animator' and
		// 'hidden' axes (if any), the region will be 1 element wide, positioned
		// at the current 'slice' for each of these axes.
		void imageRegionReady(ImageRegion imgRegion, String ddName);




		// signal from cursor position tracking.  The field names are the
		// names of registered QDDs with tracking info; corresponding values
		// are Strings with the formatted tracking information.
		void trackingInfo(Record trackingRec);



		// signal from option setting (probably unused at present).
		void optionsChanged(Record chgOpt);

		// Emitted when this QDP is hidden (closed).
		void dpHidden(QtDisplayPanel* dp);


		// Emitted when a new ImageRegion has been stored internally (in lastRgn_).
		// It can be saved to disk with saveLastRegion().
		void newRegion(String imagePath);


		// Emitted when the panel has been resized.
		void resized(QSize panelSize, QSize canvasSize);


		// Emitted when creating a restore xml doc.  QtDisplayPanelGui (in
		// particular) can process by adding intormation to restoredoc.
		void creatingRstrDoc(QDomDocument* restoredoc);

		// Emitted when restoring panel state from an xml doc.  QtDisplayPanelGui
		// (in particular) can respond by setting its own state from certain
		// elements of restoredoc.
		void restoring(QDomDocument* restoredoc);

		void activate(Record);

		// signal cursor boundary transitions
		void cursorBoundary( QtDisplayPanel::CursorBoundaryCondition );
		void cursorPosition( viewer::Position );

	protected slots:

		//# I.e., only this class creates the connections to these,
		//# (though the signals _may_ come from outside...).

		//# Protected counterparts to public slots/routines generally are
		//# 'workhorse' parts of the public routines.  They do the indicated
		//# task, but without assuring consistency with other state/interface,
		//# or sending signals for that purpose.  Those jobs are left to the
		//# public parts, mostly.


		//# registration

		// reacts to similar-named signals from QtViewer
		// <group>
		virtual void ddCreated_(QtDisplayData*, Bool autoRegister, int position=-1, Bool csMaster = False);
		virtual void ddRemoved_(QtDisplayData*);
		// </group>


		//# animation

		virtual void setAnimatorOptions_(Record opts);
		virtual void setAnimator_(Record sarec);

		virtual void setLen_(Int len) {	//# (probably unneeded).
			if(modeZ()) setZlen_(len);
			else        setBlen_(len);
		}
		virtual void setZlen_(Int len);


		virtual void stop_();
		virtual void goTo_(Int frm) {
			if(modeZ()) goToZ_(frm);
			else goToB_(frm);
		}
		virtual void goToZ_(Int frm);
		virtual void goToB_(Int frm);

		virtual void playStep_() {
			if(animating_<0) prev_();
			else if(animating_>0) next_();
		}
		virtual void prev_();	//# (Like fwdStep, revStep, but these don't stop
		virtual void next_();	//#  the animation; they are _used_ by animation)



		//# mouse tools

		// Connected to QtMouseToolState::mouseBtnChg() signal: changes
		// button assignment for a mouse tool.
		virtual void chgMouseBtn_(std::string tool, Int button);

		// Connected to corresp. signals from 'region' mouse tools.  Emits that
		// signal verbatum, but also processes it through the registered DDs,
		// and emits higher-level 'Image Regions' from those DDs which can
		// create one from the 'mouse region' record.
		virtual void mouseRegionReady_(Record mouseRegion, WorldCanvasHolder*);



		//# color bar maintenance.

		virtual void reorientColorBars_() {
			arrangeColorBars_(True);
		}
		virtual void checkColorBars_()    {
			arrangeColorBars_(False);
		}
		virtual void pcResizing_()        {
			arrangeColorBars_(False, True);
		}



		//# position tracking

		// triggered by internal dd change (not mouse movement;
		// see operator()(WCMotionEvent) for that.)
		virtual void refreshTracking_(QtDisplayData* qdd=0);

		void clicked(Record);

	protected:

		// Called during construction.
		virtual void setupMouseTools_( bool new_region_tools );

		// The workhorse part of [un]registering; these do not send the
		// highest-level signals.
		// Called internally when the DD is new or being removed, or from
		// corresponding public methods.
		// <group>
		virtual void registerDD_(QtDisplayData* qdd, int position = -1);
		virtual void unregisterDD_(QtDisplayData* qdd);
		// </group>

		// Maintain monitors of events on underlying canvases.  Called during
		// construction/destruction, or when the set of canvases might change.
		// <group>
		virtual void installEventHandlers_();
		virtual void removeEventHandlers_();
		// </group>

		// Utility function: is the given wc one belonging to the main pd_?
		virtual Bool myWC_(const WorldCanvas* wc);


		//# color bar maintenance.

		// This routine corresponds to the old viewerdisplaypanel.g routine called
		// 'arrangewedgerequirements'.  It responds to events which may require
		// change to the relative placement or number of colorbars and their panels.
		// 'reorient' is True if color bars are changing from horizontal to vertical
		// (or vise versa); it helps determine how much action (if any) this routine
		// needs to take.
		// Only the pcResizing_() slot should set resizing=True; in this case,
		// arrangeColorBars_ lets the PC take care of refresh.
		virtual void arrangeColorBars_(Bool reorient=False, Bool resizing=False);


		// Update Lists of registered DDs which have color bar display
		// activated (allColorBarDDs_ and colorBarDDsToDisplay_).
		// Used by arrangeColorBars_().
		virtual void updateColorBarDDLists_();


		// Used by arrangeColorBars_().
		// Return the margin to give to dd's colorbar panel on the side where
		// colorbar labelling is done (right margin for vertical bars, e.g.).
		// shrink will usually be 1.; in rare cases where many colorbars are
		// crowding the main panel and each other, it may be less (and then
		// the returned margin allowance may also be less than ideal...).
		virtual Int marginb_(QtDisplayData* dd, Float shrink=1.);


		// Used by arrangeColorBars_().
		// Return the nominal proportion of the PixelCanvas to use for dd's
		// colorbar panel.  (This is allocated along the direction of the
		// colorbar's thickness; in the direction its length the panel uses
		// the entire PC size).
		virtual Float cbPanelSpace_(QtDisplayData* dd);


		// Refresh (only) the colorbar panels (if any).  (An attempt to reduce
		// flashing during blink animation).
		virtual void refreshCBPanels_();


		//# save-restore

		// Tries to find existing DD (or create one from a file) that matches
		// the given path.  Used for restoring DDs from a restore file.
		// Several directories are checked, not just the one in path, to
		// provide flexibility.  If dd is non-zero, it is tested for suitability
		// to path, datatype and displaytype.  If dd is zero, the filesystem
		// is searched and an attempt is made to create dd from a matching file
		// (if any); if creation succeeds, dd will contain the new DD.
		// Note: the DD will _not_ be registered automatically.
		// origrestorefile and (current) restoredir aid in locating data files
		// relative to the current restore file location.
		virtual Bool ddFileMatch_( const std::string &path, const std::string &dataType, const std::string &displayType,
		                           QtDisplayData*& dd, QString origrestorefile, QString restoredir);



		//# Qt Widget event handlers

		void hideEvent(QHideEvent* ev) {
			QWidget::hideEvent(ev);
			emit dpHidden(this);
		}

		// (emits Qt signal with QDP and PC sizes.  (You probably really want
		// to take action on QPC resizeEvents (based on QPC size) instead...))
		void resizeEvent(QResizeEvent* ev);


		// central (unique) viewer object: for viewer-global state / methods.
		QtDisplayPanelGui *panel_;

		// 'Main' panel of pc_, where qdds_ draw themselves.
		PanelDisplay* pd_;

		// The entire display canvas for this object
		//(the area with black (default) background).
		QtPixelCanvas* pc_;	//# QtDisplayPanel is basically just enhanced
		//# state and functional interface on top of
		//# this PixelCanvas.  Its own QWidget
		//# is just a container for the PC's.

		// QDDs registered on this QDP, in registration order.
		// (List of _all_ (user-loaded) QDDs is v_->dds()).
		//List<QtDisplayData*> qdds_;
		DisplayDataHolder* displayDataHolder;

		//# mouse tools.
		viewer::RegionToolManager *toolmgr;

		//<group>
		MWCRTZoomer* zoom_;
		MWCPannerTool* panner_;

		QtOldCrossTool* ocrosshair_;
		QtOldRectTool* ortregion_;
		QtOldEllipseTool *oelregion_;
		QtOldPolyTool* optregion_;

		viewer::QtRegionSourceFactory *region_source_factory;

		MWCPolylineTool* polyline_;

		MWCRulerlineTool* rulerline_;
		MWCPositionVelocityTool *pvtool_;

		PCITFiddler* snsFidd_;
		PCITFiddler* bncFidd_;
		//</group>

		Vector<String> mouseToolNames_;



		QTimer tmr_;

		Bool tracking_;	//# Won't send out tracking signals when False.


		//# animation state

		Bool modeZ_;			//# True (default) == normal mode; else blink.
		Int zLen_, bLen_;		//# total number of frames for each mode.
		Int zIndex_, bIndex_;		//# current frame (0-based).

		Int animRate_;  		//# frames / sec. for play.
		Int minRate_, maxRate_;	//# limits to above:
		//# 1 <= minRate_ <= animRate_ <= maxRate_.
		Int animating_;		//# -1: reverse play  0: stopped  1: fwd. play



		//# color bar state

		// Registered QDDs which have color bar display activated (in registration
		// order).  See arrangeColorBars_() and updateColorBarDDLists_().
		List<QtDisplayData*> allColorBarDDs_;

		// Subset of the list above: those which should actually display now
		// (which would exclude, e.g., a DD which is 'off blink').  These DDs
		// are ordered so that in multipanel blink displays the order of images
		// in panels is the same as the display order of their corresponding
		// colorbars.
		List<QtDisplayData*> colorBarDDsToDisplay_;

		// Corresponding List of PanelDisplays in which color bars are placed
		// -- one for each QDD in colorBarDDsToDisplay_.  Assignment of QDDs
		// to panels is maintained in the order of list above.
		List<PanelDisplay*> colorBarPanels_;

		// A panel that takes unused PC space in the colorbar area, if any.
		// just to see that it's cleared when appropriate.
		PanelDisplay* blankCBPanel_;

		// Proportion of the pc_ that each color bar panel will use along the
		// direction of colorbar thickness (e.g., along the horizontal axis for
		// a vertical bar).  arrangeColorBars_() gets nominal sizes for colorbar
		// panels from cbPanelSpace_(), and that is usually what they get.
		Vector<Float> colorBarPanelSizes_;

		// The portion of the PixelCanvas allocated to the main (image display)
		// PanelDisplay (pd_);
		Float mainPanelSize_;

		// The following are used (only) by arrangeColorBars_() and its
		// auxiliary routines, and are set for current conditions every time
		// arrangeColorBars_() is called. The margins (in 'pgp chars') are
		// set onto the colorbar panels. PixelCanvas current sizes are in
		// screen pixels.  marginUnit_ is 1/65 of the PC's minimum dimansion.
		// 'ln' and 'th' refer to the directions along the colorbar's length
		// and thickness, respectively; 'ln' corresponds to vertical / horizontal
		// according to the current orientation of color bars.  mrgna_, mrgnb_
		// are margins in the colorbar thickness direction; mrgnb_ is where
		// labelling occurs.
		Int pcthsz_,  pclnsz_;
		Float marginUnit_;
		Int mrgna_, mrgnb_,  lnmrgna_, lnmrgnb_;



		//# region management state

		// The last region created with a mouse tool.
		ImageRegion lastRgn_;

		// path (if any) to image of DD that produced the region (needed?)
		String rgnImgPath_;

		// Has any such region (ever) been saved?
		Bool hasRgn_;

		// Extension policy to use to for saved region (and for statistics:
		// 0: Region applies to viewed plane only.
		// 1: Extend to all channels (iff spectral axis not on display).
		// 2: Extend along all non-displayed axes.
		// (The mouse polygon/rectangle always determines the region on the
		// displayed axes).
		Int rgnExtent_;



		//# 'Region Shape' state (unrelated to mouse region / ImageRegion state
		//# above; more like annotations.)

		// RegionShapes registered on this display panel, in registration order.
		List<RegionShape*> rshapes_;

		// Pointer to shape manager.
		// (within this class, please do not assume qsm_ has been set.)
		QtRegionShapeManager* qsm_;



		//# save-restore state

		// last saved state for this panel (this session -- "" if none).
		// Used for fast 'clipboard' save-restore.
		// (Note: No user interface for this yet...).
		String lastSavedState_;



		//# position tracking state

		// last valid mouse motion event; for use by refreshTracking_() in
		// case something besides mouse motion dictates that tracking
		// data should be refreshed.
		WCMotionEvent* lastMotionEvent_;



		//# misc.

		// User interface parsing object for background/foreground color.
		// (Used in get/setOptions(), shows up in QDPG's 'Panel Display Options'
		// window).
		DParameterChoice* bkgdClrOpt_;



		//image region extension flags
		String extChan_;
		String extPol_;

		// keep track or where the (USER) cursor currently is...
		enum CursorBoundaryState { INSIDE_PLOT, OUTSIDE_PLOT };
		CursorBoundaryState cursorBoundaryState;

	public:

		//# This (public) bool is probably temporary.  True by default.
		//# Set it False to disable stats printing.
		Bool printStats;

		Bool useRegion;

		//# Printing utilities

		void setAllowBackToFront(Bool allowed=True) {
			if (pc_) pc_->setAllowBackToFront(allowed);
		}

		void setUpdateAllowed(Bool allowed=True) {
			if (pc_) pc_->setUpdateAllowed(allowed);
		}

		void setLineWidthPS(Float &w);
		void setBackgroundPS(String &w, String &c);

		void beginLabelAndAxisCaching( ) {
			pc_->beginLabelAndAxisCaching( );
		}
		void endLabelAndAxisCaching( QPainter &qp ) {
			pc_->endLabelAndAxisCaching( qp );
		}


	private:

		/**
		 * Sends a signal containing information about the data pinpointed by the mouse
		 * & displays flyover information about the data.
		 */
		void processTracking( const Record& trackingRec, const WCMotionEvent& ev );

		/**
		 * Estimates appropriate margins for the color bars in the long direction.
		 * @param vertical True if the long side of the color bar is in the vertical direction.
		 * @param plotPercentage a float giving the approximate percentage of the screen real estate
		 *        that will be reserved for plots (as opposed to the color bars).
		 * @param cbp the PanelDisplay containing the color bar
		 * @param resizing a boolean that will be true if the calculation was triggered by the
		 * 	screen size changing.
		 */
		void setColorBarMargins( bool vertical, float plotPercentage, PanelDisplay* cbp, bool resizing );

		/**
		 * Stores the x-offset and side length of the passed in panel.
		 * @param pd the PanelDisplay whose geometry will be set.
		 * @param orgn the offset of the panel
		 * @param siz the side length of the panel.
		 */
		void setPanelGeometry( PanelDisplay* pd, float orgn, float siz );

		/**
		 * Sets the margins of the passed in panel to the indicated amounts.
		 * @param marginA in PGP plot units.
		 * @param marginB in PGP plot units.
		 * @param lengthMarginA in PGP plot units.
		 * @param lengthMarginB in PGP plot units.
		 */
		void setPanelMargins( PanelDisplay* pd, int marginA =LEFT_MARGIN_SPACE_DEFAULT,
		                      int marginB = RIGHT_MARGIN_SPACE_DEFAULT,
		                      int lengthMarginA = BOTTOM_MARGIN_SPACE_DEFAULT,
		                      int lengthMarginB = TOP_MARGIN_SPACE_DEFAULT);
		/*
		 * Attempts to set an appropriate font size for plot axis
		 * labels based on the number of plots that are displayed.
		 */
		void setLabelFontSize(  );

		/**
		 * Attempts to set appropriate plot margins based on the number of
		 * plots that are displayed.
		 */
		void setMarginSize(  );

		//# DATA
		//An estimate for the number of pixels that correspond to one
		//PGP plot unit.
		const int PGP_MARGIN_UNIT;
		enum ColorBarIndex {ORIGIN, SIZE, LENGTH_SIDE_ORIGIN, LENGTH_SIDE_SIZE, MARGIN_A,
		                    MARGIN_B, LENGTH_SIDE_MARGIN_A, LENGTH_SIDE_MARGIN_B
		                   };

		std::map<ColorBarIndex,String> settingsMap;

		/**
		 * Initializes the settings map based on whether the color bar is vertical
		 * or horizontal.
		 */
		void initializeSettingsMap( bool vertical );

		/**
		 * Fixes a bug where the color bar disappears if the number of
		 * plots that are displayed decreases.
		 */
		void plotCountChangeAdjustment();

		//Used for coming up with estimates for the new font and margin
		//sizes.
		float oldPlotPercentage;
		int oldPixelCanvasHeight;
		int oldPixelCanvasWidth;
		int oldRowCount;
		int oldColumnCount;

		//Default sizes for the margins
		static const int LEFT_MARGIN_SPACE_DEFAULT;
		static const int BOTTOM_MARGIN_SPACE_DEFAULT;
		static const int TOP_MARGIN_SPACE_DEFAULT;
		static const int RIGHT_MARGIN_SPACE_DEFAULT;

		//# Start_, End_ are current user-defined animator limits;
		//# The user can now set her/her own desired limits.
		//# 0 <= Start_ <=Index_ < End_ <=Len_   and  1 <= Step_ <= Len_.
		Int zStart_, zEnd_, zStep_;
		Int bStart_, bEnd_, bStep_;

	};



} //# NAMESPACE CASA - END

#endif
