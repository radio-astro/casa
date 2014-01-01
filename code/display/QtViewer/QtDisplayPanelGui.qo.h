//# QtDisplayPanelGui.qo.h: Qt implementation of main viewer display window.
//# with surrounding Gui functionality
//# Copyright (C) 2005,2009
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
//# $Id: QtDisplayPanelGui.qo.h,v 1.7 2006/10/10 21:42:05 dking Exp $

#ifndef QTDISPLAYPANELGUI_H
#define QTDISPLAYPANELGUI_H

#include <casa/aips.h>
#include <graphics/X11/X_enter.h>
#  include <QtCore>
#  include <QtGui>
#include <QTextEdit>
#include <QHash>
#include <QFont>

//#dk Be careful to put *.ui.h within X_enter/exit bracket too,
//#   because they'll have Qt includes.
//#   E.g. <QApplication> needs the X11 definition of 'Display'
#include <graphics/X11/X_exit.h>
#include <casaqt/QtUtilities/QtPanelBase.qo.h>
#include <display/QtViewer/QtDisplayPanel.qo.h>
#include <display/QtViewer/DisplayDataHolder.h>
#include <display/region/QtRegionDock.qo.h>
#include <display/Utilities/Lowlevel.h>
#include <display/DisplayDatas/DisplayDataOptions.h>
#include <display/Utilities/ImageProperties.h>
#include <display/Utilities/StatusSink.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	namespace viewer {
		class Preferences;
		class CleanGui;
	}

	class String;
	class QtViewer;
	class QtViewerPrintGui;
	class QtMouseToolBar;
	class QtCanvasManager;
	class QtAnnotatorGui;
	class MakeMask;
	class FileBox;
	class MakeRegion;
	class QtProfile;
	class QtDisplayData;
	class TrackBox;
	class QtRegionManager;
	class QtRegionShapeManager;
	class QtDataManager;
	class QtExportManager;
	class QtDataOptionsPanel;
	class AnimatorHolder;
	class BinPlotWidget;
	class HistogramMain;
	class Fit2DTool;
	class SlicerMainWindow;
	class ColorHistogram;
	class ImageManagerDialog;
	class QtDisplayPanelGui;
    class CursorTrackingHolder;

	template <class T> class ImageInterface;

	class LinkedCursorEH : public QObject, public WCRefreshEH {
		Q_OBJECT
	public:
		LinkedCursorEH( QtDisplayPanelGui *dpg );
		virtual ~LinkedCursorEH( );
		void operator ()(const WCRefreshEvent &ev);
		void addSource( QtDisplayPanelGui *src, QColor color );
		void removeSource( QtDisplayPanelGui *src );
	private slots:
		void boundary(QtDisplayPanel::CursorBoundaryCondition);
		void position(viewer::Position);
	private:
		struct cursor_info_t {
			cursor_info_t(QColor c) : color(c) { }
			QColor color;
			viewer::Position pos;
		};
		typedef std::map<QtDisplayPanelGui*,cursor_info_t> sources_list_t;
		sources_list_t cursor_sources;
		QtDisplayPanelGui *dpg_;
	};


// <summary>
// The main display window for the Qt version of the viewer.
// </summary>

	class QtDisplayPanelGui : public QtPanelBase, public viewer::StatusSink {

		Q_OBJECT;	//# Allows slot/signal definition.  Must only occur in
		//# implement/.../*.h files; also, makefile must include
		//# name of this file in 'mocs' section.

	protected:
		friend class QtViewer;
		QtDisplayPanelGui( QtViewer *v, QWidget *parent=0, std::string rcstr="dpg",
		                   const std::list<std::string> &args = std::list<std::string>( ) );
		QtDisplayPanelGui( const QtDisplayPanelGui *other, QWidget *parent=0, std::string rcstr="dpg",
		                   const std::list<std::string> &args = std::list<std::string>( ) );

	public:
		enum SCRIPTING_OPTION { INTERACT, SETOPTIONS };

		~QtDisplayPanelGui();

		// access to our viewer
		QtViewer *viewer( ) const {
			return v_;
		}
		int buttonToolState(const std::string &tool) const;

		// access our logger...
		LogIO &logIO( ) {
			return logger;
		}

		// enter status information...
		void status( const std::string &s, const std::string &type="info" );

		// access to graphics panel 'base'....
		QtDisplayPanel* displayPanel() { return qdp_; }
		const QtDisplayPanel* displayPanel() const { return qdp_; }

		typedef std::list<viewer::Region*> region_list_t;
		region_list_t regions( ) {
			return regionDock_ ? regionDock_->regions( ) : std::list<viewer::Region*>( );
		}
		// region coupling between QtRegionDock and QtRegionSource(s)...
		void revokeRegion( viewer::Region *r ) {
			qdp_->revokeRegion(r);
		}


		// public toolbars, for inserting custom buttons.
		QToolBar* customToolBar;	//# limited room
		QToolBar* customToolBar2;	//# wider -- in its own row.

		virtual void setStatsPrint(Bool printStats=True) {
			qdp_->printStats = printStats;
		}

		virtual void setUseRegion(Bool useRegion=True) {
			qdp_->useRegion = useRegion;
		}

		virtual bool supports( SCRIPTING_OPTION option ) const;
		virtual QVariant start_interact( const QVariant &input, int id );
		virtual QVariant setoptions( const QMap<QString,QVariant> &input, int id);

		// At least for now, colorbars can only be placed horizontally or
		// vertically, identically for all display panels.
		// This returns the current value.
		Bool colorBarsVertical() {
			return colorBarsVertical_;
		}

		//# ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
		//# DisplayData functionality brought down from QtViewerBase
		//# ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
		// Create a new QtDD from given parameters, and add to internal DD list.
		// (For now) QtViewerBase retains 'ownership' of the QtDisplayData; call
		// removeDD(qdd) to delete it.
		// Unless autoregister is set False, all open DisplayPanels will
		// register the DD for display.
		// Check return value for 0, or connect to the createDDFailed()
		// signal, to handle failure.
		QtDisplayData* createDD( String path, String dataType, String displayType,
				Bool autoRegister=True, int insertPosition = -1,
				bool masterCoordinate = false, bool masterSaturation = false,
				bool masterHue = false,
		        const viewer::DisplayDataOptions &ddo=viewer::DisplayDataOptions(),
		        const viewer::ImageProperties &props=viewer::ImageProperties( ));

		// Removes the QDD from the list and deletes it (if it existed --
		// Return value: whether qdd was in the list in the first place).
		virtual Bool removeDD(QtDisplayData*& qdd);

		// retrieve a copy of the current DD list.
		//List<QtDisplayData*> dds() { return qdds_;  }
		DisplayDataHolder::DisplayDataIterator beginDD() const;
		DisplayDataHolder::DisplayDataIterator endDD() const;
		Bool isEmptyDD() const;
		// return the number of user DDs.
		//Int nDDs() { return qdds_.len();  }

		// return a list of DDs that are registered on some panel.
		//List<QtDisplayData*> registeredDDs();

		// return a list of DDs that exist but are not registered on any panel.
		List<QtDisplayData*> unregisteredDDs();

		// retrieve a DD with given name (0 if none).
		QtDisplayData* dd(const std::string& name);
		// retrieve the controlling DD...
		QtDisplayData* dd( );

		// Check that a given DD is on the list.  Use qdd pointer or its name.
		//<group>
		Bool ddExists(QtDisplayData* qdd);
		//Bool ddExists(const String& name) { return dd(name)!=0;  }
		//</group>

		// Latest error (in createDD, etc.)
		virtual String errMsg() {
			return errMsg_;
		}

		//# ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
		//# DisplayData functionality brought down from QtViewerBase
		//# ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----

		// the QtDBusViewerAdaptor can handle loading & registering data itself,
		// but to connect up extra functionality, the upper-level QtDisplayPanelGui
		// (or in the current case, the derived QtCleanPanelGui) would have to be
		// notified that data has been added. This will allow it to set up the
		// callbacks for drawing regions...
		virtual void addedData( QString type, QtDisplayData * );

		QtDataManager* dataMgr() {
			return qdm_;
		}

		// return the id for viewer state for this type of panel
		virtual std::string rcid( ) const {
			return rcid_;
		}

		viewer::QtRegionDock *regionDock( ) {
			return regionDock_;
		}
		int numFrames( );

		// load casa (or DS9?) region files...
		void loadRegions( const std::string &path, const std::string &datatype );
		// zero length string indicates OK!
		std::string outputRegions( std::list<viewer::QtRegionState*> regions, std::string file, std::string format, std::string ds9_csys="pixel" );

		bool useNewRegions( ) const {
			return use_new_regions;
		}

		// called to indicate application activation state... true -> activated, false -> deactivated
		void activate( bool );

		// display cursor information for the specified point (in world coordinates)
		void updateCursorInfo( WorldCanvas *wc, Quantity x, Quantity y );
		typedef std::pair<QString, std::tr1::shared_ptr<ImageInterface<float> > > OverplotInterface;

	public slots:

		// At least for now, colorbars can only be placed horizontally or vertically,
		// identically for all display panels.  This sets that state for everyone.
		// Sends out colorBarOrientationChange signal when the state changes.
		virtual void setColorBarOrientation(Bool vertical);

		virtual QtDisplayPanelGui *createNewPanel( );

		virtual void showDataManager();
		virtual void hideDataManager();

		virtual void showExportManager();
		virtual void hideExportManager();

		virtual void showDataOptionsPanel();
		virtual void hideDataOptionsPanel();

		virtual void showPreferences( );

		virtual void removeAllDDs();

		// Show/hide display panel's auxiliary windows.
		//<group>
		virtual void showPrintManager();
		virtual void hidePrintManager();

		virtual void showCanvasManager();
		virtual void hideCanvasManager();

		virtual void showRegionManager();
		virtual void hideRegionManager();

		virtual void showShapeManager();
		virtual void hideShapeManager();

		virtual void showAnnotatorPanel();
		virtual void hideAnnotatorPanel();

		virtual void showFileBoxPanel();
		virtual void hideFileBoxPanel();

		virtual void showMakeRegionPanel();
		virtual void hideMakeRegionPanel();

		virtual void showImageProfile();
		virtual void hideImageProfile();
		virtual void refreshImageProfile();

		virtual void hideAllSubwindows();
		virtual void hideImageMenus();

		//Increments the channel in the images from the start channel
		//to the end channel.
		void movieChannels( int startChannel, int endChannel );
		void movieChannels( int channel, bool forward, int stepSize, int channelStart, int channelEnd  );
		void movieStop();
		void registerAllDDs();
		void unregisterAllDDs();
		virtual void showStats(const String&);
		virtual void hideStats();
		//</group>

		// add a new DD
		virtual QtDisplayData* addDD(String path, String dataType, String displayType, Bool autoRegister=True, Bool tmpDtata=False, std::tr1::shared_ptr<ImageInterface<Float> > img = std::tr1::shared_ptr<ImageInterface<Float> >());
		// go to a specifc channel
		virtual void doSelectChannel(int channelIndex);

		// (Attempts to) restore panel state from named file.
		virtual Bool restorePanelState(String filename);

		virtual void trackingMoved(Qt::DockWidgetArea);
		virtual void animatorMoved(Qt::DockWidgetArea);
		virtual void regionMoved(Qt::DockWidgetArea);
		virtual void mousetoolbarMoved(bool);

		// note that 'key' is prefixed with something like "viewer.dpg."...
		// for both get and put...
		std::string getrc( const std::string &key );
		void putrc( const std::string &key, const std::string &val );
		void showMomentsCollapseImageProfile();
		void showSpecFitImageProfile();
		void disconnectHistogram();
		void ddClose( QtDisplayData*& removeDD);
		void ddOpen( const String& path, const String& dataType,
				const String& displayType, int insertPosition = -1,
				bool register = true, bool masterCoordinate = false,
				bool masterSaturation = false, bool masterHue = false);

		// retrieve the identifier string for this QtDisplayPanelGui...
		std::string id( ) const {
			return id_;
		}

		void unlinkCursorTracking(QtDisplayPanelGui*);
		void linkCursorTracking(QtDisplayPanelGui*,QColor);

	signals:

		void regionChange( viewer::Region *, std::string );

		void axisToolUpdate( QtDisplayData *controlling_dd );

		void colorBarOrientationChange();

		//Notification that the frame has changed.
		void frameChanged( int );

		//Profile Overplots
		void overlay(QList<OverplotInterface>);

		void createDDFailed(String errMsg, String path, String dataType,
		                    String displayType);

		// The DD now exists, and is on QtViewerBase's list.
		// autoregister tells DPs whether they are to register the DD.
		// ***** dd is added to the world canvas holder during *****
		// ***** the processing of this event...               *****
		void ddCreated(QtDisplayData*, Bool autoRegister, int insertPosition, Bool csMaster);

		// The DD is no longer on QtViewerBase's list, but is not
		// destroyed until after the signal.
		// ***** dd is removed from the world canvas holder    *****
		// ***** during the processing of this event...        *****
		//void ddRemoved(QtDisplayData*);

		void closed( const QtDisplayPanelGui * );

		void cursorBoundary( QtDisplayPanel::CursorBoundaryCondition );
		void cursorPosition( viewer::Position );

	protected slots:

		virtual void close( );

		virtual void quit( );

		//# overrides of base QMainWindow slots

		void hideEvent(QHideEvent* ev) {
			// Note: If the display panel is iconified or the user changes
			// desktops, isVisible() will remain true here (surprising, but
			// useful -- see isVisible() doc).  Otherwise, in this context,
			// [I believe] we can assume the display panel has been _closed_;
			// in that case, we want to 'close' the auxiliary windows as well.
			// (Note that there is no 'closeEvent' per se).
			if(!isVisible()) hideAllSubwindows();

			QMainWindow::hideEvent(ev);
		}


		//# purely internal slots

		/* virtual void toggleAnimExtras_(); */
		/* virtual void setAnimExtrasVisibility_();   */

		//# slots reacting to signals from the basic QtDisplayPanel.
		//# Protected, connected by this object itself.

		// Respond to QDP::registrationChange() signal
		virtual void ddRegChange_();

		// Respond to registration/close menu clicks.
		//<group>
		virtual void ddRegClicked_();
		virtual void ddUnregClicked_();
		virtual void ddCloseClicked_();
		//</group>

		// Reflect animator state [changes] in gui.
		virtual void updateAnimUi_();

		// These react to fwd/reverse Play buttons.  They allow a single
		// play button to be used to toggle between play and stop.
		//<group>
		virtual void fwdPlayChannelMovie_( ) {
			if(qdp_->animating()>0) qdp_->stopChannelMovie( );
			else qdp_->fwdPlayChannelMovie( );
		}
		virtual void fwdPlayImageMovie_( ) {
			if(qdp_->animating()>0) qdp_->stopImageMovie( );
			else qdp_->fwdPlayImageMovie( );
		}

		virtual void revPlayChannelMovie_( ) {
			if(qdp_->animating()<0) qdp_->stopChannelMovie( );
			else qdp_->revPlayChannelMovie( );
		}
		virtual void revPlayImageMovie_() {
			if(qdp_->animating()<0) qdp_->stopImageMovie( );
			else qdp_->revPlayImageMovie();
		}
		//</group>


		// Display tracking data gathered by underlying panel.
		virtual void displayTrackingData_(Record trackingRec);

		// Reacts to QDP registration change signal.  If necessary, changes
		// the set of cursor position tracking boxes being displayed in
		// trkgWidget_ (creating new TrackBoxes as necessary).  A TrackBox
		// will be shown for each qdd in qdp_->registeredDDs() where
		// qdd->usesTracking() (in registration order).
		virtual void arrangeTrackBoxes_();

		// Deletes the TrackBox for the given QDD, if it exists.  (Deletion
		// automatically removes it from the trkgWidget_ and its layout).
		// Connected to the ddRemoved() signal of QtViewerBase.
		virtual void deleteTrackBox_(QtDisplayData* qdd);



		// Brings up dialog for saving display panel state: reg'd DDs, their
		// options, etc. Triggered by dpSaveAct_.
		virtual void savePanelState_();

		// Brings up dialog for restore file, attempts restore.
		// Triggered by dpRstrAct_.
		virtual void restorePanelState_();


		// Responds to qdp_->creatingRstrDoc(QDomDocument*) signal.
		// (Recall that qdp_ is unaware of this gui).
		// Adds gui state to the QDomDocument qdp has created.
		virtual void addGuiState_(QDomDocument*);

		// Responds to qdp_->restoring(QDomDocument*) signal.
		// Sets gui-specific state (most notably, overall window size).
		virtual void restoreGuiState_(QDomDocument*);

	protected:

		LogIO logger;
		static bool logger_did_region_warning;

		// Existing user-visible QDDs
		//List<QtDisplayData*> qdds_;
		DisplayDataHolder* displayDataHolder;
		String errMsg_;


		QtDataManager* qdm_;		//# The window for loading data.
		QtExportManager* qem_;    //# The window for exporting images.
		QtDataOptionsPanel* qdo_;	//# The window for controlling data display.

		// Keeps current data directory in sync between
		// DataManager window and save-restore dialogs.
		virtual Bool syncDataDir_(String filename);

		virtual void updateDDMenus_(Bool doCloseMenu = True);


		// scripted (via dbus) panels should override the closeEvent( ) and hide the gui
		// instead of deleting it when it was created via a dbus script...
		void closeEvent(QCloseEvent *event);

		//# ----------------------------DATA----------------------------------

		// At least for now, colorbars can only be placed horizontally or vertically,
		// identically for all display panels.  Here is where that state is kept for
		// everyone.
		Bool colorBarsVertical_;

		QtViewer* v_;		 	//# (Same viewer as qdp_'s)
		QtDisplayPanel* qdp_;  	//# Central Widget this window operates.
		QtViewerPrintGui* qpm_;	//# Print dialog for this display panel.
		QtCanvasManager* qcm_;	//# display panel options window.
		//QtAnnotatorGui* qap_;
		MakeMask* qap_;
		FileBox* qfb_;
		MakeRegion* qmr_;
		QtRegionManager* qrm_;      //# Region manager window.
		QtRegionShapeManager* qsm_; //# Region shape manager window.
		QTextEdit* qst_;


		QtProfile* profile_;		//# Profile window
		String savedTool_;		//# (for restoring left button)
		QtDisplayData* profileDD_;    //# QDD currently being profiled
		//# (0 if profiler is not showing).

		//# GUI LAYOUT

		QMenu *dpMenu_, *ddMenu_, /**ddRegMenu_,*/ *ddCloseMenu_, *tlMenu_, *vwMenu_;

		QAction *dpNewAct_, *printAct_, *dpOptsAct_, *dpCloseAct_, *dpQuitAct_,
		        *ddOpenAct_, *ddSaveAct_, *ddAdjAct_,/* *ddRegAct_, *ddCloseAct_,*/ *unzoomAct_,
		        *zoomInAct_, *zoomOutAct_, *annotAct_, *mkRgnAct_, *fboxAct_, *ddPreferencesAct_,
		        *profileAct_, *momentsCollapseAct_, *histogramAct_, *fitAct_,
		        *cleanAct_, *rgnMgrAct_, *shpMgrAct_, *dpSaveAct_, *dpRstrAct_, *manageImagesAct_;

		QToolBar* mainToolBar_;
		QToolButton *ddRegBtn_, *ddCloseBtn_;

		QtMouseToolBar* mouseToolBar_;

		// connection to rc file
		Casarc &rc;
		// rc id for this panel type
		std::string rcid_;

	private:
		/**
		 ** portion of construction shared by multiple constructor functions...
		 */
		void construct_( QtDisplayPanel *newpanel, const std::list<std::string> &args );

		void animationModeChanged( bool modeZ);

		bool use_new_regions;

		//Animating the channel
		int movieChannel;
		int movieChannelEnd;
		int movieLast;
		int movieStart;
		int movieStep;

		QTimer movieTimer;
		void setAnimationRate();
		int getBoundedChannel( int channelNumber ) const;
		void updateViewedImage();
		void profiledImageChange();
		void clearTools();
		unsigned int showdataoptionspanel_enter_count;
		QtDisplayPanelGui() : rc(viewer::getrc()), linkedCursorHandler(0) {  }		// (not intended for use)
		QtDisplayData* processDD( String path, String dataType,
				String displayType, Bool autoRegister, int insertPosition,
				bool masterCoordinate, bool masterSaturation, bool masterHue,
		        QtDisplayData* qdd, const viewer::DisplayDataOptions &ddo=viewer::DisplayDataOptions() );
		void connectRegionSignals(PanelDisplay* ppd);
		void notifyDDRemoval( QtDisplayData* qdd );
		//Management of the controlling DD
		QtDisplayData* lookForExistingController();
		//void setControllingDD( QtDisplayData* controlDD );


		//Methods for letting the animator know whether it should display
		//the image/channel animator(s) based on the number of images and
		//and the number of channels in the images.
		void updateFrameInformation();
		void updateFrameInformationChannel();
		void updateFrameInformationImage();

		void updateSliceCorners( int id, const QList<double>& worldX,
		                         const QList<double>& worldY );
		void initAnimationHolder();
		void initHistogramHolder();
		void hideHistogram();
		void initFit2DTool();
		void hideFit2DTool();
		void resetHistogram( viewer::Region* qtRegion );
		viewer::Region* findRegion( int id );

		viewer::Preferences *preferences;

		AnimatorHolder* animationHolder;
		bool adjust_channel_animator;
		bool adjust_image_animator;
		int animationImageIndex;

		HistogramMain* histogrammer;
		ColorHistogram* colorHistogram;
		Fit2DTool* fitTool;
		SlicerMainWindow* sliceTool;
		ImageManagerDialog* imageManagerDialog;

		// interactive clean...
		void initCleanTool( );
		viewer::CleanGui *clean_tool;


		//Docking/Dock Widgets
		string addAnimationDockWidget();

		QDockWidget*  histogramDockWidget_;
		viewer::QtRegionDock  *regionDock_;
		CursorTrackingHolder *trkgDockWidget_;

		QTimer *status_bar_timer;
		QString status_bar_state;
		QString status_bar_stylesheet;

		LinkedCursorEH *linkedCursorHandler;

		std::string id_;
		static std::string idGen( );

	private slots:
		void to_image_mode( ) { animationModeChanged(true); }
		void to_channel_mode( ) { animationModeChanged(false); }
		void loadRegions( const QString &path, const QString &type );
		void incrementMovieChannel();
		void clear_status_bar( );
		void reset_status_bar( );
		void controlling_dd_axis_change(String, String, String, std::vector<int> );
		//void controlling_dd_update(QtDisplayData*);
		void showHistogram();
		void showSlicer();
		void resetListenerImage();
		void histogramRegionChange( int, viewer::region::RegionChanges change = viewer::region::RegionChangeLabel );
		void showFitInteractive();
		void showCleanTool( );
		void refreshFit();
		void addSkyComponentOverlay(String path, const QString& colorName);
		void removeSkyComponentOverlay( String path );
		void add2DFitOverlay( QList<RegionShape*> fitMarkers );
		void remove2DFitOverlay( QList<RegionShape*> fitMarkers );
		void addResidualFitImage( String path );
		virtual void addDDSlot(String path, String dataType, String displayType,
				Bool autoRegister=True, Bool tmpData=False,
				std::tr1::shared_ptr<ImageInterface<Float> > img = std::tr1::shared_ptr<ImageInterface<Float> >());
		void sliceChanged( int regionId, viewer::region::RegionChanges change,
		                   const QList<double> & worldX, const QList<double> & worldY,
		                   const QList<int> &pixelX, const QList<int> & pixelY );
		void addSlice( int id, const QString& shape, const QString&, const QList<double>& worldX,
		               const QList<double>& worldY, const QList<int>& pixelX, const QList<int>& pixelY,
		               const QString&, const QString&, const QString&, int, int);
		void showColorHistogram(QtDisplayData* displayData);
		void globalColorSettingsChanged( bool global );
		void globalOptionsChanged( QtDisplayData* originator, Record opts );
		void updateColorHistogram( const QString& ddName );
		void showImageManager();
		void sliceMarkerVisibilityChanged(int regionId, bool visible);
		void sliceMarkerPositionChanged(int regionId, int segmentIndex, float percentage);
		void updateMultiSpectralFitLocation( Record trackingRec);
		/**
		 * Written in response to CAS-5101. When multiple images are loaded,
		 * some with many channels and one with only one channel, the available
		 * frames in the channel animator needs to change based on the mode.  Assume
		 * the single channel image is the one that will be animated in normal
		 * mode.  In this case, the channel animator should register a single
		 * frame and not allow animation.  However, if we are in "channel images mode", a form of blink mode, the
		 * number of available channels should be the maximum of the channel count
		 * in all the images. Channel animation should then take place.  Method
		 * sets the mode, and then updates the channel count accordingly.
		 */
		void animationImageChanged( int index );

		void registerDD( QtDisplayData* dd, int position );
		void unregisterDD( QtDisplayData* dd );
		// used to manage generation of the updateAxes( ) signal...
		void replaceControllingDD( QtDisplayData* oldControllingDD, QtDisplayData* newControllingDD);

		//Change the image that is being viewed in the blink
		//animator.
		void setAnimatedImage( int index );
		void createRGBImage( QtDisplayData* coordinateMaster,
						QtDisplayData* redImage, QtDisplayData* greenImage,
						QtDisplayData* blueImage );


	public:

		// True by default.  Set False to disable auto-raise of the Data
		// Options panel whenever the first DD is created.
		//# Users want to see this panel automatically when there are DDs
		//# to tweak.  (Apps like clean can turn v_->autoOptionsRaise off,
		//# if desired (yes, is is (gasp!) public data)).
		Bool autoDDOptionsShow;

		// Intended for use only by QtDataManager (or other data dialogs such as for
		// save-restore), to inform QtDisplayPanel of the directory currently
		// selected for data retrieval, if any ("" if none).
		String selectedDMDir;

	};


	namespace viewer {
		namespace hidden {
			// Qt's meta object features not supported for nested classes...
			// so this class cannot be nested in QtDisplayPanelGui...
			class display_panel_gui_status : public QStatusBar {
				Q_OBJECT;
			public:
				display_panel_gui_status( QWidget *parent = 0 ) : QStatusBar(parent) { }
				~display_panel_gui_status( ) { }
			signals:
				void enter( );
				void leave( );

			protected:
				void enterEvent( QEvent* );
				void leaveEvent( QEvent* );
			};
		}
	}


} //# NAMESPACE CASA - END

#endif
