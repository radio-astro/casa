//# QtDisplayPanelGui.cc: Qt implementation of main viewer display window.
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
//# $Id: QtDisplayPanelGui.cc,v 1.12 2006/10/10 21:59:19 dking Exp $

#include <algorithm>
#include <string>
#include <QSet>
#include <casa/BasicSL/String.h>
#include <display/Utilities/StringUtil.h>
#include <display/QtViewer/QtDisplayPanelGui.qo.h>
#include <display/QtViewer/QtViewerPrintGui.qo.h>
#include <display/QtViewer/QtCanvasManager.qo.h>
#include <display/QtViewer/QtRegionManager.qo.h>
#include <display/QtViewer/MakeMask.qo.h>
#include <display/QtViewer/FileBox.qo.h>
#include <display/QtViewer/MakeRegion.qo.h>
#include <display/region/Region.qo.h>
#include <display/QtViewer/QtDisplayData.qo.h>
#include <display/QtViewer/QtMouseToolBar.qo.h>
#include <display/QtViewer/QtViewer.qo.h>
#include <display/QtPlotter/QtProfile.qo.h>
#include <display/QtPlotter/Util.h>
#include <display/QtViewer/QtDisplayData.qo.h>
#include <display/QtViewer/QtDataManager.qo.h>
#include <display/QtViewer/QtExportManager.qo.h>
#include <display/QtViewer/QtDataOptionsPanel.qo.h>
#include <display/RegionShapes/QtRegionShapeManager.qo.h>
#include <display/QtViewer/AnimatorHolder.qo.h>
#include <display/QtViewer/CursorTrackingHolder.qo.h>
#include <display/QtViewer/QtWCBox.h>
#include <display/QtViewer/Preferences.qo.h>
#include <display/QtViewer/ColorHistogram.qo.h>
#include <display/QtViewer/ImageManager/ImageManagerDialog.qo.h>
#include <display/Fit/Fit2DTool.qo.h>
#include <display/Slicer/SlicerMainWindow.qo.h>
#include <display/region/QtRegionSource.qo.h>
#include <display/region/Polyline.qo.h>
#include <display/RegionShapes/RegionShapes.h>
#include <guitools/Histogram/HistogramMain.qo.h>
#include <display/Clean/CleanGui.qo.h>
#include <display/DisplayErrors.h>
#include <display/DisplayDatas/PrincipalAxesDD.h>
#include <display/DisplayDatas/LatticeAsRaster.h>

#include <display/QtViewer/InActiveDock.qo.h>

#include <tr1/memory>

namespace casa { //# NAMESPACE CASA - BEGIN

namespace viewer {
namespace hidden {
void display_panel_gui_status::enterEvent( QEvent * ) {
	emit enter( );
}
void display_panel_gui_status::leaveEvent( QEvent * ) {
	emit leave( );
}
}
}

LinkedCursorEH::LinkedCursorEH( QtDisplayPanelGui *d ) : WCRefreshEH( ), dpg_(d) {
	ConstListIter<WorldCanvas*>& wcs = *(dpg_->displayPanel( )->panelDisplay( )->myWCLI);
	for ( wcs.toStart( ); ! wcs.atEnd( ); ++wcs )
		wcs.getRight( )->addRefreshEventHandler(*this);
}

LinkedCursorEH::~LinkedCursorEH( ) {
	ConstListIter<WorldCanvas*>& wcs = *(dpg_->displayPanel( )->panelDisplay( )->myWCLI);
	for ( wcs.toStart( ); ! wcs.atEnd( ); ++wcs )
		wcs.getRight( )->removeRefreshEventHandler(*this);
}

void LinkedCursorEH::operator()(const WCRefreshEvent & ev) {
	WorldCanvas *wc = ev.worldCanvas( );
	if ( wc == 0 ) return;
	PixelCanvas *pc = wc->pixelCanvas();
	if ( pc == 0 ) return;
	QtPixelCanvas* qpc = dynamic_cast<QtPixelCanvas*>(pc);
	if ( qpc == 0 ) return;
	Vector<String> axes = wc->worldAxisNames( );
	Vector<String> units = wc->worldAxisUnits( );
	if ( axes.size( ) < 2 ) return;
	for ( sources_list_t::iterator iter = cursor_sources.begin( ); iter != cursor_sources.end( ); ++iter ) {
		Vector<String> pos_axes = iter->second.pos.csys( ).worldAxisNames( );
		if ( pos_axes.size( ) < 2 ) continue;
		int xindex, yindex;
		if ( axes(0) == pos_axes(0) && axes(1) == pos_axes(1) ) {
			xindex = 0;
			yindex = 1;
		} else if ( axes(0) == pos_axes(1) && axes(1) == pos_axes(0) ) {
			xindex = 1;
			yindex = 0;
		} else continue;

		Vector<Quantity> pvpos(iter->second.pos.coord( ));
		if ( pvpos.size( ) < 2 ) continue;

		if ( pvpos(xindex).isConform(units(0)) == false || pvpos(yindex).isConform(units(1)) == false )
			continue;

		double wx = pvpos(xindex).getValue(units(0));
		double wy = pvpos(yindex).getValue(units(1));
		int scr_x, scr_y;

		try {
			viewer::world_to_screen( wc, wx, wy, scr_x, scr_y );
		} catch(...) {
			continue;
		}

		if ( wc->inDrawArea(scr_x,scr_y) ) {
			QColor saved_color = qpc->getQtPenColor( );
			qpc->setQtPenColor(iter->second.color);
			qpc->drawEllipse( scr_x, scr_y, 5, 5, 0.0 );
			const int x_off=6;
			qpc->drawLine( scr_x - x_off, scr_y - x_off, scr_x + x_off, scr_y + x_off );
			qpc->drawLine( scr_x - x_off, scr_y + x_off, scr_x + x_off, scr_y - x_off );
			qpc->setQtPenColor(saved_color);
			dpg_->updateCursorInfo( wc, pvpos(xindex), pvpos(yindex) );
		}
	}
}

void LinkedCursorEH::addSource( QtDisplayPanelGui *src, QColor color ) {
	sources_list_t::iterator iter = cursor_sources.find(src);
	if ( iter != cursor_sources.end( ) ) {
		iter->second.color = color;
	} else {
		cursor_sources.insert(sources_list_t::value_type(src,cursor_info_t(color)));
		void cursorBoundary( QtDisplayPanel::CursorBoundaryCondition );
		void cursorPosition( viewer::Position );
		connect( src, SIGNAL(cursorBoundary(QtDisplayPanel::CursorBoundaryCondition)), this, SLOT(boundary(QtDisplayPanel::CursorBoundaryCondition)) );
		connect( src, SIGNAL(cursorPosition(viewer::Position)), this, SLOT(position(viewer::Position)) );
	}
}

void LinkedCursorEH::removeSource( QtDisplayPanelGui *src ) {
	sources_list_t::iterator iter = cursor_sources.find(src);
	if ( iter != cursor_sources.end( ) ) {
		// disconnect all objects from the source dpg to this object...
		disconnect( iter->first, 0, this, 0 );
		cursor_sources.erase(iter);
	}
}

void LinkedCursorEH::boundary( QtDisplayPanel::CursorBoundaryCondition ) { }
void LinkedCursorEH::position( viewer::Position position ) {
	sources_list_t::iterator iter = cursor_sources.find(dynamic_cast<QtDisplayPanelGui*>(QObject::sender( )));
	if ( iter != cursor_sources.end( ) ) {
		iter->second.pos = position;
		dpg_->displayPanel( )->refresh( );
	}
}

bool QtDisplayPanelGui::logger_did_region_warning = false;

// also in casadbus/utilities/BusAccess.cc (generate_proxy_suffix)
static char *base62( int len ) {
	char *suffix = 0;
	static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	suffix = new char[len+1];
	for (int i = 0; i < len; ++i) {
		suffix[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
	}
	suffix[len] = 0;
	return suffix;
}

std::string QtDisplayPanelGui::idGen( ) {
	static std::list<std::string> used;
	for ( unsigned int i=0; i < 9999; ++i ) {
		std::string identifier(base62(2));
		if ( std::find(used.begin(),used.end(),identifier) == used.end( ) ) {
			used.push_back(identifier);
			return identifier;
		}
	}
	throw viewer::internal_error("QtDisplayPanelGui id overflow (?)");
	return "";
}

QtDisplayPanelGui::QtDisplayPanelGui(QtViewer* v, QWidget *parent, std::string rcstr, const std::list<std::string> &args ) :
				QtPanelBase(parent), logger(LogOrigin("CASA", "Viewer")), qdm_(0),qem_(0),qdo_(0),
				colorBarsVertical_(True), v_(v), qdp_(0), qpm_(0), qcm_(0), qap_(0), qfb_(0), qmr_(0), qrm_(0),
				qsm_(0), qst_(0),
				profile_(0), savedTool_(QtMouseToolNames::NONE),
				profileDD_(0),
				annotAct_(0), mkRgnAct_(0), fboxAct_(0), cleanAct_(0), rgnMgrAct_(0), shpMgrAct_(0),
				rc(viewer::getrc()), rcid_(rcstr), use_new_regions(true),
				showdataoptionspanel_enter_count(0),
				/*controlling_dd(0),*/ preferences(0), animationHolder( NULL ),
				adjust_channel_animator(true), adjust_image_animator(true),
				histogrammer( NULL ), colorHistogram( NULL ),
				fitTool( NULL ), sliceTool( NULL ), imageManagerDialog(NULL),
				clean_tool(0), regionDock_(0),
				status_bar_timer(new QTimer( )),
				linkedCursorHandler(0), id_(idGen( )), autoDDOptionsShow(True) {

	// initialize the "pix" unit, et al...
	QtWCBox::unitInit( );

	setWindowTitle(QString("Viewer Display Panel (") + QString::fromStdString(id_) + QString(")"));

	use_new_regions = std::find(args.begin(),args.end(),"--oldregions") == args.end();
	const char default_dock_location[] = "right";

	std::string apos = rc.get("viewer." + rcid() + ".position.animator");
	std::transform(apos.begin(), apos.end(), apos.begin(), ::tolower);
	if ( apos != "bottom" && apos != "right" && apos != "left" && apos != "top" ) {
		rc.put( "viewer." + rcid() + ".position.animator", default_dock_location );
	}
	std::string tpos = rc.get("viewer." + rcid() + ".position.cursor_tracking");
	std::transform(tpos.begin(), tpos.end(), tpos.begin(), ::tolower);
	if ( tpos != "bottom" && tpos != "right" && tpos != "left" && tpos != "top" ) {
		rc.put( "viewer." + rcid() + ".position.cursor_tracking", default_dock_location );
	}
	std::string rpos = rc.get("viewer." + rcid() + ".position.regions");
	std::transform(rpos.begin(), rpos.end(), rpos.begin(), ::tolower);
	if ( rpos != "bottom" && rpos != "right" && rpos != "left" && rpos != "top" ) {
		rc.put( "viewer." + rcid() + ".position.regions", default_dock_location );
	}

	qdp_ = new QtDisplayPanel(this,0,args);
	displayDataHolder = new DisplayDataHolder();



	if ( use_new_regions ) {
		// -----
		// This must be created here, because the process of (a) constructing a QtDisplayPanel,
		// (b) creates a QtRegionCreatorSource, which (c) uses the constructed QtDisplayPanel, to
		// (d) retrieve the QToolBox which is part of this QtRegionDock... should fix... <drs>
		regionDock_  = new viewer::QtRegionDock(this, qdp_);
		connect( regionDock_, SIGNAL(regionChange(viewer::Region*,std::string)), SIGNAL(regionChange(viewer::Region*,std::string)));
		connect( regionDock_, SIGNAL(loadRegions(const QString&, const QString &)), SLOT(loadRegions(const QString&, const QString &)) );
		connect( this, SIGNAL(axisToolUpdate(QtDisplayData*)), regionDock_, SLOT(updateRegionState(QtDisplayData*)) );
		std::string shown = getrc("visible.regions");
		std::transform(shown.begin(), shown.end(), shown.begin(), ::tolower);
		if ( shown == "false" ) regionDock_->dismiss( );
	}

	QDockWidget* displayDock = new InActiveDock( this );
	displayDock->setWidget( qdp_);
	displayDock->setObjectName( QString::fromUtf8("Image Display"));
	displayDock->setWindowTitle(QApplication::translate("Display", "Display", 0, QApplication::UnicodeUTF8));
	std::string displayLocation = rc.get("viewer." + rcid() + ".position.display");
	addDockWidget( displayLocation == "right" ? Qt::RightDockWidgetArea :
			displayLocation == "bottom" ? Qt::BottomDockWidgetArea :
					displayLocation == "top" ? Qt::TopDockWidgetArea :
							Qt::LeftDockWidgetArea, displayDock, Qt::Vertical );
	string displayShown = getrc("visible.display");
	if ( displayShown == "false" ){
		displayDock->close( );
	}


	addDockWidget( Qt::LeftDockWidgetArea, displayDock );
	//setCentralWidget(qdp_);

	setFocusProxy(qdp_);	// Shifts panel kbd focus to qdp_, which
	// in turn shifts it to PixelCanvas.
	// PC/WC respond to some keystrokes
	// (e.g. arrow keys, esc, space...).
	// Note: this will not steal focus from,
	// e.g., a text widget on the panel.


	qrm_ = new QtRegionManager(qdp_);

	qsm_ = new QtRegionShapeManager(qdp_);
	qsm_->setVisible(false);

	qdp_->setShapeManager(qsm_);

	// SURROUNDING GUI LAYOUT

	// Create the widgets (plus a little parenting and properties)
	ddMenu_       = menuBar()->addMenu("&Data");
	ddOpenAct_    = ddMenu_->addAction("&Open...");
	manageImagesAct_ = ddMenu_->addAction( "&Manage Images...");
	ddAdjAct_     = ddMenu_->addAction("&Adjust Data Display...");
	ddSaveAct_    = ddMenu_->addAction("Sa&ve as...");
	ddMenu_->addSeparator();
	printAct_     = ddMenu_->addAction("&Print...");
	ddMenu_->addSeparator();
	dpSaveAct_    = ddMenu_->addAction("&Save Panel State...");
	dpRstrAct_    = ddMenu_->addAction("Restore Panel State...");
	ddMenu_->addSeparator();
	ddPreferencesAct_ = ddMenu_->addAction("Preferences...");
	ddMenu_->addSeparator();
	dpCloseAct_   = ddMenu_->addAction("&Close Panel");
	ddCloseMenu_ = new QMenu;
	//ddCloseAct_->setMenu(ddCloseMenu_);
	dpQuitAct_    = ddMenu_->addAction("&Quit Viewer");

	dpMenu_       = menuBar()->addMenu("D&isplay Panel");
	dpNewAct_     = dpMenu_->addAction("&New Panel");
	dpOptsAct_    = dpMenu_->addAction("Panel &Options...");
	dpMenu_->addAction(dpSaveAct_);
	dpMenu_->addAction(dpRstrAct_);
	dpMenu_->addAction(printAct_);
	dpMenu_->addSeparator();
	dpMenu_->addAction(dpCloseAct_);

	tlMenu_       = menuBar()->addMenu("&Tools");
	if ( ! use_new_regions ) {
		fboxAct_      = tlMenu_->addAction("&Box in File");
		annotAct_     = tlMenu_->addAction("Region in &File");
		mkRgnAct_     = tlMenu_->addAction("Region in &Image");
		//                  annotAct_->setEnabled(False);
		connect(fboxAct_,    SIGNAL(triggered()),  SLOT(showFileBoxPanel()));
		connect(annotAct_,   SIGNAL(triggered()),  SLOT(showAnnotatorPanel()));
		connect(mkRgnAct_,   SIGNAL(triggered()),  SLOT(showMakeRegionPanel()));
	}

	profileAct_   = tlMenu_->addAction("Spectral Profi&le...");
	// rgnMgrAct_    = new QAction("Region Manager...", 0);
	// rgnMgrAct_->setEnabled(False);

	if ( ! use_new_regions ) {
		shpMgrAct_    = tlMenu_->addAction("Shape Manager...");
		connect(shpMgrAct_,  SIGNAL(triggered()),  SLOT(showShapeManager()));
	}
	momentsCollapseAct_ = tlMenu_->addAction("Collapse/Moments...");
	histogramAct_ = tlMenu_->addAction( "Histogram...");
	fitAct_ = tlMenu_->addAction( "Fit...");
	cleanAct_ = tlMenu_->addAction( "Interactive Clean..." );

	vwMenu_       = menuBar()->addMenu("&View");
	// (populated after creation of toolbars/dockwidgets).

	mainToolBar_  = addToolBar("Main Toolbar");
	mainToolBar_->setObjectName("Main Toolbar");
	mainToolBar_->addAction(ddOpenAct_);
	mainToolBar_->addAction(ddAdjAct_);
	ddRegBtn_     = new QToolButton(mainToolBar_);
	mainToolBar_->addWidget(ddRegBtn_);
	connect( ddRegBtn_, SIGNAL(clicked()), this, SLOT(showImageManager()));
	ddCloseBtn_   = new QToolButton(mainToolBar_);
	mainToolBar_->addWidget(ddCloseBtn_);
	ddCloseBtn_->setMenu(ddCloseMenu_);
	//connect( ddCloseBtn_, SIGNAL(clicked()))
	mainToolBar_->addAction(ddSaveAct_);
	mainToolBar_->addSeparator();
	mainToolBar_->addAction(dpNewAct_);
	mainToolBar_->addAction(dpOptsAct_);
	mainToolBar_->addAction(dpSaveAct_);
	mainToolBar_->addAction(dpRstrAct_);
	mainToolBar_->addSeparator();
	mainToolBar_->addAction(profileAct_);
	mainToolBar_->addAction(momentsCollapseAct_);
	mainToolBar_->addAction(histogramAct_);
	mainToolBar_->addAction(fitAct_);
	//		    mainToolBar_->addAction(rgnMgrAct_);
	mainToolBar_->addSeparator();
	mainToolBar_->addAction(printAct_);
	mainToolBar_->addSeparator();
	unzoomAct_    = mainToolBar_->addAction("Un&zoom");
	zoomInAct_    = mainToolBar_->addAction("Zoom &In");
	zoomOutAct_   = mainToolBar_->addAction("Zoom O&ut");


	std::string mbpos = rc.get("viewer." + rcid() + ".position.mousetools");
	std::transform(mbpos.begin(), mbpos.end(), mbpos.begin(), ::tolower);
	mouseToolBar_ = new QtMouseToolBar(v_->mouseBtns(), qdp_);
	mouseToolBar_->setObjectName("Mouse Toolbar");
	mouseToolBar_->setAllowedAreas( (Qt::ToolBarArea) ( Qt::LeftToolBarArea | Qt::TopToolBarArea |
			Qt::RightToolBarArea | Qt::BottomToolBarArea ) );
	addToolBarBreak();
	addToolBar( mbpos == "left" ? Qt::LeftToolBarArea :
			mbpos == "right" ? Qt::RightToolBarArea :
					mbpos == "bottom" ? Qt::BottomToolBarArea :
							Qt::TopToolBarArea, mouseToolBar_);

	// This tool bar is _public_; programmer can add custom interface to it.
	customToolBar    = addToolBar("Custom Toolbar");
	customToolBar->setObjectName("Custom Toolbar");
	customToolBar->hide();	// (hidden by default).
	customToolBar->toggleViewAction()->setVisible(False);
	// This can also be reversed: controls visibility
	// of this toolbar in the 'View' (or rt.-click) menu.

	// Another public tool bar; This one in a separate row, will fit more.
	addToolBarBreak();
	customToolBar2   = addToolBar("Custom Toolbar 2");
	customToolBar2->setObjectName("Custom Toolbar 2");
	customToolBar2->hide();
	customToolBar2->toggleViewAction()->setVisible(False);

	//Animation
	initAnimationHolder();
	string animloc = addAnimationDockWidget();

	std::string shown = getrc("visible.animator");
	std::transform(shown.begin(), shown.end(), shown.begin(), ::tolower);
	if ( shown == "false" ) animationHolder->dismiss( );

	initFit2DTool();

	std::string trackloc = rc.get("viewer." + rcid() + ".position.cursor_tracking");
	std::transform(trackloc.begin(), trackloc.end(), trackloc.begin(), ::tolower);
	trkgDockWidget_  = new CursorTrackingHolder( this );
	addDockWidget( trackloc == "right" ? Qt::RightDockWidgetArea :
			trackloc == "left" ? Qt::LeftDockWidgetArea :
					trackloc == "top" ? Qt::TopDockWidgetArea :
							Qt::BottomDockWidgetArea, trkgDockWidget_, Qt::Vertical );
	shown = getrc("visible.cursor_tracking");
	std::transform(shown.begin(), shown.end(), shown.begin(), ::tolower);
	if ( shown == "false" ) trkgDockWidget_->dismiss( );

	//  ------------------------------------------------------------------------------------------
	if ( regionDock_ ) {
		std::string rgnloc = rc.get("viewer." + rcid() + ".position.regions");
		std::transform(rgnloc.begin(), rgnloc.end(), rgnloc.begin(), ::tolower);
		addDockWidget( rgnloc == "right" ? Qt::RightDockWidgetArea :
				rgnloc == "left" ? Qt::LeftDockWidgetArea :
						rgnloc == "top" ? Qt::TopDockWidgetArea :
								Qt::BottomDockWidgetArea, regionDock_, Qt::Vertical );
	}

#if 0
	if ( trackloc == "right" && animloc == "right" && rc.get("viewer." + rcid() + ".rightdock") == "tabbed" ) {
		tabifyDockWidget( animationHolder, trkgDockWidget_ );
	} else if ( trackloc == "left" && animloc == "left" && rc.get("viewer." + rcid() + ".leftdock") == "tabbed" ) {
		tabifyDockWidget( animationHolder, trkgDockWidget_ );
	} else if ( trackloc == "top" && animloc == "top" && rc.get("viewer." + rcid() + ".topdock") == "tabbed" ) {
		tabifyDockWidget( animationHolder, trkgDockWidget_ );
	} else if ( trackloc == "bottom" && animloc == "bottom" && rc.get("viewer." + rcid() + ".bottomdock") == "tabbed" ) {
		tabifyDockWidget( animationHolder, trkgDockWidget_ );
	}
#endif

	//  ------------------------------------------------------------------------------------------

	// (This was going to be used for tracking....  May still be useful for
	// additions, or perhaps they should be in central widget, above QtPC).
	// bottomToolBar_   = new QToolBar();
	// 		         addToolBar(Qt::BottomToolBarArea, bottomToolBar_);
	//  bottomWidget_   = new QWidget;
	// 		         bottomToolBar_->addWidget(bottomWidget_);
	//   new QVBoxLayout(bottomWidget_);





	// REST OF THE PROPERTY SETTING


	// The 'View' menu contains hide/show checkboxes for the tool bars
	// and dock widgets.  Thus it is populated here, after those tool
	// bars et. al. have been created.

	QMenu* rawVwMenu = createPopupMenu();

	if(rawVwMenu!=0) {
		// (Beware: createPopupMenu() returns 0 rather than an empty
		//  menu (<--as it should) if the window has no toolbars/dockwidgets.
		//  For safety only: the menu shouldn't be empty in this case...).

		QList<QAction*> vwActions = rawVwMenu->actions();

		// createPopupMenu() puts the dock widgets ahead of the tool bars by
		// default, with a separator in between.  Reversing this is more
		// intuitive for this application, since it puts them in the same order
		// as the widgets appear on the display panel itself.  (All this could
		// be replaced with the following if you didn't care about order):
		//
		//   vwMenu_->addActions(createPopupMenu()->actions());

		Int nActions = vwActions.size();
		Int sep;	// Index of the separator 'action'.
		for(sep=0; sep<nActions; sep++) if(vwActions[sep]->isSeparator()) break;

		for(Int i=sep+1; i<nActions; i++) vwMenu_->addAction(vwActions[i]);
		if (0<sep && sep<nActions-1)      vwMenu_->addSeparator();
		for(Int i=0; i<sep; i++)          vwMenu_->addAction(vwActions[i]);

		delete rawVwMenu;
	}


	if(vwMenu_->actions().size()==0)
		vwMenu_->hide();


	// Setup display of linked cursor tracking (from other QtDisplayPanelGui)...
	linkedCursorHandler = new LinkedCursorEH(this);

	//######################################
	//## Animation
	//######################################

	// menus / toolbars

	//  mainToolBar_->setIconSize(QSize(22,22));
	setIconSize(QSize(22,22));

	mainToolBar_->setMovable(False);

	ddOpenAct_ ->setIcon(QIcon(":/icons/File_Open.png"));
	ddSaveAct_ ->setIcon(QIcon(":/icons/Save_Img.png"));
	manageImagesAct_->setIcon(QIcon(":/icons/DD_Register.png"));
	//ddRegAct_  ->setIcon(QIcon(":/icons/DD_Register.png"));
	ddRegBtn_  ->setIcon(QIcon(":/icons/DD_Register.png"));
	//ddCloseAct_->setIcon(QIcon(":/icons/File_Close.png"));
	ddCloseBtn_->setIcon(QIcon(":/icons/File_Close.png"));
	ddAdjAct_  ->setIcon(QIcon(":/icons/DD_Adjust.png"));
	dpNewAct_  ->setIcon(QIcon(":/icons/DP_New.png"));
	dpOptsAct_ ->setIcon(QIcon(":/icons/DP_Options.png"));
	dpSaveAct_ ->setIcon(QIcon(":/icons/Save_Panel.png"));
	dpRstrAct_ ->setIcon(QIcon(":/icons/Restore_Panel.png"));
	profileAct_->setIcon(QIcon(":/icons/Spec_Prof.png"));
	momentsCollapseAct_->setIcon(QIcon(":/icons/profileMomentCollapse.png"));
	histogramAct_->setIcon( QIcon(":/icons/hist.png"));
	fitAct_->setIcon( QIcon(":/icons/gaussian.png"));
	// rgnMgrAct_ ->setIcon(QIcon(":/icons/Region_Save.png"));
	printAct_  ->setIcon(QIcon(":/icons/File_Print.png"));
	unzoomAct_ ->setIcon(QIcon(":/icons/Zoom0_OutExt.png"));
	zoomInAct_ ->setIcon(QIcon(":/icons/Zoom1_In.png"));
	zoomOutAct_->setIcon(QIcon(":/icons/Zoom2_Out.png"));
	dpCloseAct_->setIcon(QIcon(":/icons/File_Close.png"));
	dpQuitAct_ ->setIcon(QIcon(":/icons/File_Quit.png"));

	ddOpenAct_ ->setToolTip("Open Data...");
	ddRegBtn_  ->setToolTip(/*"[Un]register Data"*/"Manage Images");
	ddCloseBtn_->setToolTip("Close Data");
	ddAdjAct_  ->setToolTip("Data Display Options");
	ddSaveAct_ ->setToolTip("Save as...");
	dpNewAct_  ->setToolTip("New Display Panel");
	dpOptsAct_ ->setToolTip("Panel Display Options");
	dpSaveAct_ ->setToolTip("Save Display Panel State to File");
	profileAct_->setToolTip("Open the Spectral Profile Tool");
	momentsCollapseAct_->setToolTip("Calculate Moments/Collapse the Image Cube along the Spectral Axis.");
	histogramAct_->setToolTip("Histogram Statistical Analysis Tool");
	fitAct_->setToolTip( "Interactive 2D Fitting");
	dpRstrAct_ ->setToolTip("Restore Display Panel State from File");
	// rgnMgrAct_ ->setToolTip("Save/Control Regions");
	if ( shpMgrAct_ ) shpMgrAct_ ->setToolTip("Load/Control Region Shapes");
	printAct_  ->setToolTip("Print...");
	unzoomAct_ ->setToolTip("Zoom Out to Entire Image");
	zoomInAct_ ->setToolTip("Zoom In");
	zoomOutAct_->setToolTip("Zoom Out");
	dpQuitAct_ ->setToolTip("Close All Windows and Exit");


	ddRegBtn_  ->setPopupMode(QToolButton::InstantPopup);
	ddRegBtn_  ->setAutoRaise(True);
	ddRegBtn_  ->setIconSize(QSize(22,22));

	ddCloseBtn_->setPopupMode(QToolButton::InstantPopup);
	ddCloseBtn_->setAutoRaise(True);
	ddCloseBtn_->setIconSize(QSize(22,22));

	//  bottomToolBar_->setMovable(False);
	//  bottomToolBar_->hide();
	//  (disabled unless/until something for it to contain).


	//######################################
	//## CONNECTIONS
	//######################################


	//## Direct reactions to user interface.

	connect(ddOpenAct_,  SIGNAL(triggered()),  SLOT(showDataManager()));
	connect(ddSaveAct_,  SIGNAL(triggered()),  SLOT(showExportManager()));
	connect(ddPreferencesAct_, SIGNAL(triggered()), SLOT(showPreferences()));
	// connect(dpNewAct_,   SIGNAL(triggered()),  v_, SLOT(createDPG()));
	connect(dpNewAct_,   SIGNAL(triggered()),  SLOT(createNewPanel()));
	connect(dpOptsAct_,  SIGNAL(triggered()),  SLOT(showCanvasManager()));
	connect(dpSaveAct_,  SIGNAL(triggered()),  SLOT(savePanelState_()));
	connect(dpRstrAct_,  SIGNAL(triggered()),  SLOT(restorePanelState_()));
	connect(dpCloseAct_, SIGNAL(triggered()),  SLOT(close()));
	connect(dpQuitAct_,  SIGNAL(triggered()),  SLOT(quit()));
	connect(profileAct_, SIGNAL(triggered()),  SLOT(showSpecFitImageProfile()));
	connect(momentsCollapseAct_, SIGNAL(triggered()), SLOT(showMomentsCollapseImageProfile()));
	connect(histogramAct_, SIGNAL(triggered()), SLOT(showHistogram()));
	connect(fitAct_, SIGNAL(triggered()), SLOT(showFitInteractive()));
	connect(manageImagesAct_, SIGNAL(triggered()), SLOT(showImageManager()));

	if ( cleanAct_ ) connect(cleanAct_, SIGNAL(triggered()), SLOT(showCleanTool( )));

	// connect(rgnMgrAct_,  SIGNAL(triggered()),  SLOT(showRegionManager()));
	connect(ddAdjAct_,   SIGNAL(triggered()),  SLOT(showDataOptionsPanel()));
	connect(printAct_,   SIGNAL(triggered()),  SLOT(showPrintManager()));
	connect(unzoomAct_,  SIGNAL(triggered()),  qdp_, SLOT(unzoom()));
	connect(zoomInAct_,  SIGNAL(triggered()),  qdp_, SLOT(zoomIn()));
	connect(zoomOutAct_, SIGNAL(triggered()),  qdp_, SLOT(zoomOut()));
	// connect(animAuxButton_, SIGNAL(clicked()),    SLOT(toggleAnimExtras_()));

	//## docking changes
	connect( trkgDockWidget_, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), SLOT(trackingMoved(Qt::DockWidgetArea)) );
	connect( animationHolder, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), SLOT(animatorMoved(Qt::DockWidgetArea)) );

	if ( regionDock_ ) connect( regionDock_, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), SLOT(regionMoved(Qt::DockWidgetArea)) );
#if QT_VERSION >= 0x040600
	connect( mouseToolBar_, SIGNAL(topLevelChanged(bool)), SLOT(mousetoolbarMoved(bool)) );
#endif




	// Reaction to signals from the basic graphics panel, qdp_.
	// (qdp_ doesn't know about, and needn't necessarily use, this gui).

	// For tracking

	connect( qdp_, SIGNAL(trackingInfo(Record)),
			SLOT(displayTrackingData_(Record)) );
	connect( qdp_, SIGNAL(trackingInfo(Record)),
			SLOT(updateMultiSpectralFitLocation( Record)));
	/*connect( this, SIGNAL(ddRemoved(QtDisplayData*)),
		         SLOT(deleteTrackBox_(QtDisplayData*)) );
	 */

	// From animator

	connect( qdp_, SIGNAL(animatorChange()),  SLOT(updateAnimUi_()) );


	// From registration
	connect( qdp_, SIGNAL(registrationChange()),  SLOT(ddRegChange_()) );

	// From save-restore
	connect( qdp_, SIGNAL(creatingRstrDoc(QDomDocument*)),
			SLOT(addGuiState_(QDomDocument*)) );
	// Adds gui state to QDomDocument qdp has created.
	// (Recall that qdp_ is unaware of this gui).

	connect( qdp_, SIGNAL(restoring(QDomDocument*)),
			SLOT(restoreGuiState_(QDomDocument*)) );
	// Sets gui state from QDomDocument (window size, esp.)

	// FINAL INITIALIZATIONS

	//connect( qdp_, SIGNAL(newRegisteredDD(QtDisplayData*)), SLOT(controlling_dd_update(QtDisplayData*)) );
	//connect( qdp_, SIGNAL(oldDDUnregistered(QtDisplayData*)), SLOT(controlling_dd_update(QtDisplayData*)) );
	//connect( qdp_, SIGNAL(oldDDRegistered(QtDisplayData*)), SLOT(controlling_dd_update(QtDisplayData*)) );
	//connect( qdp_, SIGNAL(RegisteredDDRemoved(QtDisplayData*)), SLOT(controlling_dd_update(QtDisplayData*)) );

	connect(qdp_,  SIGNAL(registrationChange()),
			SLOT(hideImageMenus()));

	// parallel cursor tracking signals...
	connect( qdp_, SIGNAL(cursorBoundary(QtDisplayPanel::CursorBoundaryCondition)),
			SIGNAL(cursorBoundary(QtDisplayPanel::CursorBoundaryCondition)) );
	connect( qdp_, SIGNAL(cursorPosition(viewer::Position)),
			SIGNAL(cursorPosition(viewer::Position)) );

	updateDDMenus_();


	bool dimension_set = false;
	std::string dim = rc.get("viewer." + rcid() + ".dimensions");
	if ( dim != "" ) {
		std::stringstream ins(dim);
		int width, height;
		ins >> width >> height;
		if ( ins.fail() ) {
			fprintf( stderr, "setting dimension failed...\n" );
		} else if ( width > 2400 || height > 2400 ) {
			fprintf( stderr, "unreasonably large dimensions...\n" );
		} else {
			dimension_set = true;
			resize( QSize( width, height ).expandedTo(minimumSizeHint()) );
		}
	}

	if ( dimension_set == false ) {
		if ( animloc == "right" || trackloc == "right" ||
				animloc == "left" || animloc == "left" ) {
			if ( trackloc != "right" && trackloc != "left" ) {
				resize( QSize(1000, 700).expandedTo(minimumSizeHint()) );
			} else if ( animloc != "right" && animloc != "left" ) {
				resize( QSize(1000, 760).expandedTo(minimumSizeHint()) );
			} else {
				resize( QSize(1000, 640).expandedTo(minimumSizeHint()) );
			}
		} else {
			resize( QSize(600, 770).expandedTo(minimumSizeHint()) );
		}
		// To do: use a QSetting, to preserve size.
	}


	connect( &movieTimer, SIGNAL(timeout()), this, SLOT(incrementMovieChannel()));

	viewer::hidden::display_panel_gui_status *status_bar = new viewer::hidden::display_panel_gui_status( );
	setStatusBar( status_bar );
	connect( status_bar, SIGNAL(enter( )), this, SLOT(reset_status_bar( )) );
	connect( status_bar, SIGNAL(leave( )), this, SLOT(clear_status_bar( )) );

	status_bar_timer->setSingleShot( true );
	status_bar_timer->setInterval( 15000 );
	connect( status_bar_timer, SIGNAL(timeout()), this, SLOT(clear_status_bar( )) );

	//Slice1D Tool
	PanelDisplay* panelDisplay = qdp_->panelDisplay();
	std::tr1::shared_ptr<QtPolylineTool> pos = std::tr1::dynamic_pointer_cast<QtPolylineTool>(panelDisplay->getTool(QtMouseToolNames::POLYLINE));
	if (pos) {
		std::tr1::shared_ptr<viewer::QtRegionSourceKernel> qrs = std::tr1::dynamic_pointer_cast<viewer::QtRegionSourceKernel>(pos->getRegionSource( )->kernel( ));
		if ( qrs ) {
			connect( qrs.get(), SIGNAL(show1DSliceTool()), this, SLOT(showSlicer()));

		}
	}
}

string QtDisplayPanelGui::addAnimationDockWidget() {
	std::string animloc = rc.get("viewer." + rcid() + ".position.animator");
	std::transform(animloc.begin(), animloc.end(), animloc.begin(), ::tolower);
	addDockWidget( animloc == "right" ? Qt::RightDockWidgetArea :
			animloc == "left" ? Qt::LeftDockWidgetArea :
					animloc == "top" ? Qt::TopDockWidgetArea :
							Qt::BottomDockWidgetArea, animationHolder, Qt::Vertical );

	return animloc;
}



void QtDisplayPanelGui::initAnimationHolder() {
	if ( animationHolder == NULL ) {
		animationImageIndex = -1;

		animationHolder = new AnimatorHolder( this, this );
		connect(animationHolder, SIGNAL(revPlayChannelMovie()), SLOT(revPlayChannelMovie_()));
		connect(animationHolder, SIGNAL(revPlayImageMovie()), SLOT(revPlayImageMovie_()));
		connect(animationHolder, SIGNAL(fwdPlayChannelMovie()), SLOT(fwdPlayChannelMovie_()));
		connect(animationHolder, SIGNAL(fwdPlayImageMovie()), SLOT(fwdPlayImageMovie_()));
		connect(animationHolder, SIGNAL(setChannelMode( )), this, SLOT(to_channel_mode( )));
		connect(animationHolder, SIGNAL(setImageMode( )), this, SLOT(to_image_mode( )));
		connect(animationHolder, SIGNAL(selectChannel(int)), this, SLOT(doSelectChannel(int)));
		connect(animationHolder, SIGNAL(channelMovieState(int,bool,int,int,int)), this, SLOT(movieChannels(int,bool,int,int,int)));
		connect(animationHolder, SIGNAL(stopImageMovie()), this, SLOT(movieStop()));
		connect(animationHolder, SIGNAL(stopChannelMovie()), this, SLOT(movieStop()));
		connect(animationHolder, SIGNAL(animationImageChanged(int)), this, SLOT(animationImageChanged(int)));

		// Set interface according to the initial state of underlying animator.
		updateAnimUi_();
	}
}

void QtDisplayPanelGui::animationModeChanged( bool modeZ){
	qdp_->setMode( modeZ );
	updateFrameInformationChannel();
}

void QtDisplayPanelGui::animationImageChanged( int index ){
	animationImageIndex = index;
	updateFrameInformationChannel();
	if ( regionDock_ != NULL ){
		regionDock_->updateStackOrder( animationImageIndex );
	}
}

void QtDisplayPanelGui::globalColorSettingsChanged( bool global ) {
	QtDisplayData::setGlobalColorOptions( global );
}

void QtDisplayPanelGui::globalOptionsChanged( QtDisplayData* originator, Record opts ) {
	DisplayDataHolder::DisplayDataIterator iter = displayDataHolder->beginDD();
	while ( iter != displayDataHolder->endDD()) {
		if ( originator != (*iter)) {
			(*iter)->setOptions( opts, true );
		}
		iter++;
	}
}



//---------------------------------------------------------------------------------
//                                      Histogram
//---------------------------------------------------------------------------------

void QtDisplayPanelGui::showColorHistogram(QtDisplayData* displayData ) {
	if ( colorHistogram == NULL ) {
		colorHistogram = new ColorHistogram( this );
		connect( qdo_, SIGNAL(dataOptionsTabChanged(const QString&)),
				this, SLOT( updateColorHistogram(const QString&)));
	}
	colorHistogram->showNormal();	// (Magic formula to bring a window up,
	colorHistogram->raise();
	colorHistogram->setDisplayData( displayData );
}

void QtDisplayPanelGui::updateColorHistogram( const QString& ddName ) {
	QtDisplayData* targetDD = qdp_->getDD( ddName.toStdString() );
	colorHistogram->setDisplayData( targetDD );
}

void QtDisplayPanelGui::showHistogram() {
	if ( histogrammer == NULL ) {
		this->initHistogramHolder();
	}
	//Send it the latest image
	resetListenerImage();

	//Image updates
	connect( qdp_, SIGNAL(registrationChange()), this, SLOT(resetListenerImage()), Qt::UniqueConnection );
	//Region updates
	PanelDisplay* panelDisplay = qdp_->panelDisplay();
	std::tr1::shared_ptr<QtCrossTool> pos = std::tr1::dynamic_pointer_cast<QtCrossTool>(panelDisplay->getTool(QtMouseToolNames::POINT));
	if (pos) {
		std::tr1::shared_ptr<viewer::QtRegionSourceKernel> qrs = std::tr1::dynamic_pointer_cast<viewer::QtRegionSourceKernel>(pos->getRegionSource( )->kernel( ));
		if ( qrs ) {
			connect( qrs.get( ), SIGNAL( regionUpdate( int, viewer::region::RegionChanges, const QList<double> &, const QList<double> &,
					const QList<int> &, const QList<int> & ) ),
					this, SLOT( histogramRegionChange( int, viewer::region::RegionChanges) ));
			//So the histogram has the latest in regions
			histogramRegionChange( -1, viewer::region::RegionChangeUpdate );
		}
	}

	histogrammer->showNormal();	// (Magic formula to bring a window up,
	histogrammer->raise();
}

void QtDisplayPanelGui::hideHistogram() {
	if ( histogrammer != NULL ) {
		histogrammer->hide();
	}
}

void QtDisplayPanelGui::disconnectHistogram() {
	//Disconnect all the signals and slots
	//Region updates
	PanelDisplay* panelDisplay = qdp_->panelDisplay();
	std::tr1::shared_ptr<QtCrossTool> pos = std::tr1::dynamic_pointer_cast<QtCrossTool>(panelDisplay->getTool(QtMouseToolNames::POINT));
	if (pos) {
		std::tr1::shared_ptr<viewer::QtRegionSourceKernel> qrs = std::tr1::dynamic_pointer_cast<viewer::QtRegionSourceKernel>(pos->getRegionSource( )->kernel( ));
		if ( qrs ) {
			disconnect( qrs.get( ), SIGNAL( regionUpdate( int, viewer::region::RegionChanges, const QList<double> &, const QList<double> &,
					const QList<int> &, const QList<int> & ) ),
					this, SLOT( histogramRegionChange( int, viewer::region::RegionChanges) ));
		}
	}
}

void QtDisplayPanelGui::resetListenerImage() {
	QtDisplayData* controllingDD = dd();
	if ( controllingDD != NULL ) {
		std::tr1::shared_ptr<ImageInterface<float> > img = /*pdd*/controllingDD->imageInterface();
		if ( sliceTool != NULL ) {
			sliceTool->setImage( img );
		}

		if ( histogrammer != NULL ) {
			histogrammer->setImage( img );
			const viewer::ImageProperties & imgProperties = /*pdd*/controllingDD->imageProperties( );
			if ( imgProperties.hasSpectralAxis() ) {
				int spectralAxisNum = imgProperties.spectralAxisNumber();
				const Vector<int> imgShape = imgProperties.shape();
				int channelCount = imgShape[spectralAxisNum];
				histogrammer->setChannelCount( channelCount );
			} else {
				histogrammer->setChannelCount( 1 );
			}
		}

	} else {
		if ( histogrammer != NULL ) {
			histogrammer->setImage( std::tr1::shared_ptr<ImageInterface<Float> >() );
		}

	}
}

void QtDisplayPanelGui::initHistogramHolder() {
	if ( histogrammer == NULL ) {
		histogrammer = new HistogramMain(false,true,true,true,this);
		histogrammer->setDisplayPlotTitle( true );
		histogrammer->setDisplayAxisTitles( true );
		//So we can disconnect the signals and not chew up bandwidth
		connect( histogrammer, SIGNAL(closing()), this, SLOT(disconnectHistogram()));
	}
}

viewer::Region* QtDisplayPanelGui::findRegion( int id ) {
	std::list<viewer::Region*> regionList = regions();
	std::list<viewer::Region*>::iterator regionIterator = regionList.begin();
	viewer::Region* targetRegion = NULL;
	while( regionIterator != regionList.end() ) {
		viewer::Region* region = (*regionIterator);
		int regionId = region->getId();
		if ( regionId == id ) {
			targetRegion = region;
			break;
		}
		regionIterator++;
	}
	return targetRegion;
}


void QtDisplayPanelGui::histogramRegionChange( int id, viewer::region::RegionChanges change) {
	viewer::Region* region = NULL;
	if ( id != -1 ) {
		region = findRegion( id );
		if ( region != NULL ) {
			if ( region->type() == viewer::region::PolylineRegion ) {
				return;
			}
		}
	}

	if ( change == viewer::region::RegionChangeCreate ||
			change == viewer::region::RegionChangeUpdate ||
			change == viewer::region::RegionChangeModified ) {
		if ( id != -1 ) {
			if ( region != NULL ) {
				resetHistogram( region );
			}
		} else {
			//Update all the histograms because we haven't been listening for awhile
			std::list<viewer::Region*> regionList = regions();
			std::list<viewer::Region*>::iterator regionIterator = regionList.begin();
			while( regionIterator != regionList.end() ) {
				viewer::Region* region = (*regionIterator);
				resetHistogram( region );
				regionIterator++;
			}
		}
	} else if ( change == viewer::region::RegionChangeDelete ) {
		histogrammer->deleteImageRegion( id );
	} else if ( change == viewer::region::RegionChangeSelected ) {
		histogrammer->imageRegionSelected( id );
	}
}


void QtDisplayPanelGui::resetHistogram( viewer::Region* region ) {
	if ( region != NULL && histogrammer != NULL ) {
		QtDisplayData* controllingDD = dd();
		if ( controllingDD != NULL ) {
			ImageRegion* imageRegion = region->getImageRegion(controllingDD->dd());
			if ( imageRegion != NULL ) {
				int regionId = region->getId();
				histogrammer->setImageRegion( imageRegion, regionId );
			}
		}
	} else {
		qDebug() << "Update region getting a null region";
	}
}

void QtDisplayPanelGui::refreshFit() {
	if ( fitTool != NULL ) {
		QtDisplayData* controllingDD = dd();
		if ( controllingDD != NULL ) {
			std::tr1::shared_ptr<const ImageInterface<Float> > img = controllingDD->imageInterface();
			fitTool->setImage( img );
		}
		else {
			std::tr1::shared_ptr<const ImageInterface<Float> > p;
			fitTool->setImage( p);
		}
	}
}



void QtDisplayPanelGui::initFit2DTool() {
	PanelDisplay* panelDisplay = qdp_->panelDisplay();
	fitTool = new Fit2DTool( this );

	connect( qdp_, SIGNAL(registrationChange()), SLOT(refreshFit()));
	connect( fitTool, SIGNAL(showOverlay(String, const QString&)),
			this, SLOT(addSkyComponentOverlay(String, const QString&)));
	connect( fitTool, SIGNAL(addResidualFitImage(String)), this, SLOT(addResidualFitImage(String)));
	connect( fitTool, SIGNAL(removeOverlay(String)),this, SLOT(removeSkyComponentOverlay(String)));
	connect( fitTool, SIGNAL(remove2DFitOverlay( QList<RegionShape*>)),this, SLOT( remove2DFitOverlay(QList<RegionShape*>)));
	connect( fitTool, SIGNAL(add2DFitOverlay( QList<RegionShape*> )),this, SLOT( add2DFitOverlay(QList<RegionShape*>)));

	//Update the channel for the fit.
	connect( this, SIGNAL(frameChanged(int)), fitTool, SLOT(frameChanged(int)));
	refreshFit();

	//Connect drawing tools so that regions are updated for the fit.
	std::tr1::shared_ptr<QtRectTool> rect = std::tr1::dynamic_pointer_cast<QtRectTool>(panelDisplay->getTool(QtMouseToolNames::RECTANGLE));
	// one region source is shared among all of the tools...
	// so there is no need to connect these signals for all of the tools...
	if ( rect.get( ) != 0 ) {
		std::tr1::shared_ptr<viewer::QtRegionSourceKernel> qrs = std::tr1::dynamic_pointer_cast<viewer::QtRegionSourceKernel>(rect->getRegionSource( )->kernel( ));
		if ( qrs ) {
			connect( qrs.get( ), SIGNAL( regionCreated( int, const QString &, const QString &, const QList<double> &,
					const QList<double> &, const QList<int> &, const QList<int> &,
					const QString &, const QString &, const QString &, int, int ) ),
					fitTool, SLOT( newRegion( int, const QString &, const QString &, const QList<double> &,
							const QList<double> &, const QList<int> &, const QList<int> &,
							const QString &, const QString &, const QString &, int, int ) ) );
			connect( qrs.get( ), SIGNAL( regionUpdate( int, viewer::region::RegionChanges, const QList<double> &, const QList<double> &,
					const QList<int> &, const QList<int> & ) ),
					fitTool, SLOT( updateRegion( int, viewer::region::RegionChanges, const QList<double> &, const QList<double> &,
							const QList<int> &, const QList<int> & ) ) );
			connect( qrs.get( ), SIGNAL( regionUpdateResponse( int, const QString &, const QString &, const QList<double> &,
					const QList<double> &, const QList<int> &, const QList<int> &,
					const QString &, const QString &, const QString &, int, int ) ),
					fitTool, SLOT( newRegion( int, const QString &, const QString &, const QList<double> &,
							const QList<double> &, const QList<int> &, const QList<int> &,
							const QString &, const QString &, const QString &, int, int ) ) );
			qrs->generateExistingRegionUpdates( );
		}
	}
}


void QtDisplayPanelGui::showFitInteractive() {
	fitTool->showNormal();	// (Magic formula to bring a window up,
	fitTool->raise();
}

void QtDisplayPanelGui::hideFit2DTool() {
	fitTool->hide();
}


void QtDisplayPanelGui::addResidualFitImage( String path ) {
	//If there is already a dd with this name, remove it and create another
	//one
	QString pathStr( path.c_str());
	int fileIndex = pathStr.lastIndexOf( QDir::separator() );
	if ( fileIndex >= 0 ) {
		QString fileName  =pathStr.mid( fileIndex+1 );
		QtDisplayData* oldResidualDD = dd( fileName.toStdString() );
		if ( oldResidualDD != NULL ) {
			removeDD( oldResidualDD );
		}
	}
	QtDisplayData* dd = createDD( path, "image", "raster" );
	if ( dd == NULL ) {
		qDebug() << "Could not add residual image to viewer: "<<path.c_str();
	}
}

void QtDisplayPanelGui::initCleanTool( ) {
	try {
		clean_tool = new viewer::CleanGui( );
		clean_tool->hide( );
	} catch(...) {
		clean_tool = 0;
		logger << LogIO::WARN
				<< "no dbus session available..."
				<< LogIO::POST;
	}
}

void QtDisplayPanelGui::showCleanTool( ) {
	if ( clean_tool == 0 ) initCleanTool( );
	if ( clean_tool ) clean_tool->show( );
}

void QtDisplayPanelGui::addSkyComponentOverlay( String path, const QString& colorName ) {
	QtDisplayData* dd = createDD( path, "sky cat", "skycatalog" );
	Record opts = dd->getOptions();
	//cout << opts << endl;
	const String MARKER_COLOR( "markercolor");
	const String LABEL_CHAR_COLOR( "labelcharcolor");
	Record markerColorRecord = opts.subRecord( MARKER_COLOR );
	markerColorRecord.define("value", colorName.toStdString());
	opts.defineRecord( MARKER_COLOR, markerColorRecord);
	Record labelColorRecord = opts.subRecord( LABEL_CHAR_COLOR );
	labelColorRecord.define("value", colorName.toStdString());
	opts.defineRecord( LABEL_CHAR_COLOR, labelColorRecord);
	dd->setOptions( opts );
}

void QtDisplayPanelGui::removeSkyComponentOverlay( String path ) {
	QtDisplayData* displayDataToRemove = displayDataHolder->getDD( path.c_str());
	if ( displayDataToRemove != NULL ) {
		removeDD( displayDataToRemove );
	}
}

void QtDisplayPanelGui::add2DFitOverlay( QList<RegionShape*> fitMarkers ) {
	for ( int i = 0; i < fitMarkers.size(); i++ ) {
		qdp_->registerRegionShape(fitMarkers[i]);
	}
}

void QtDisplayPanelGui::remove2DFitOverlay( QList<RegionShape*> fitMarkers ) {
	for ( int i = 0; i < fitMarkers.size(); i++ ) {
		qdp_->unregisterRegionShape( fitMarkers[i]);
	}
}



QtDisplayPanelGui::~QtDisplayPanelGui() {

	v_->dpgDeleted(this);
	delete linkedCursorHandler;
	linkedCursorHandler = NULL;

	delete qpm_;
	qpm_= NULL;
	delete qrm_;
	qrm_ = NULL;
	delete qdm_;
	qdm_ = NULL;
	delete qdo_;
	qdo_ = NULL;
	delete qdp_;	// (probably unnecessary because of Qt parenting...)
	// (possibly wrong, for same reason?...).
	// (indeed was wrong as the last deletion [at least] because the display panel also reference the qsm_)
	qdp_= NULL;
	delete qsm_;
	qsm_ = NULL;

	removeAllDDs();
	delete imageManagerDialog;
	imageManagerDialog = NULL;
	delete displayDataHolder;
}

int QtDisplayPanelGui::buttonToolState(const std::string &tool) const {
	return v_->mouseBtns( )->getButtonState(tool);
}

void QtDisplayPanelGui::status( const std::string &s, const std::string &type ) {
	status_bar_state = QString::fromStdString(s);
	status_bar_stylesheet = (type == "error" ? "QStatusBar { color: rgb(255, 53, 43) }" : "QStatusBar { }");
	reset_status_bar( );
}

void QtDisplayPanelGui::clear_status_bar( ) {
	statusBar( )->clearMessage( );
}

void QtDisplayPanelGui::reset_status_bar( ) {
	statusBar( )->setStyleSheet( status_bar_stylesheet );
	statusBar( )->showMessage( status_bar_state );
	QCoreApplication::processEvents( );
	status_bar_timer->start( );
}

bool QtDisplayPanelGui::supports( SCRIPTING_OPTION ) const {
	return false;
}

QVariant QtDisplayPanelGui::start_interact( const QVariant &, int ) {
	return QVariant(QString("*error* unimplemented (by design)"));
}
QVariant QtDisplayPanelGui::setoptions( const QMap<QString,QVariant> &, int ) {



	return QVariant(QString("*error* nothing implemented yet"));
}

//# ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
//# DisplayData functionality brought down from QtViewerBase
//# ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----

QtDisplayData* QtDisplayPanelGui::createDD( String path, String dataType,
		String displayType, Bool autoRegister, int insertPosition,
		bool masterCoordinate, bool masterSaturation, bool masterHue,
		const viewer::DisplayDataOptions &ddo,
		const viewer::ImageProperties &props ) {
	adjust_channel_animator = true;
	adjust_image_animator = true;
	QtDisplayData* qdd = new QtDisplayData( this, path, dataType, displayType, ddo, props );
	return processDD( path, dataType, displayType, autoRegister,
			insertPosition, masterCoordinate, masterSaturation, masterHue, qdd, ddo  );
}
QtDisplayData* QtDisplayPanelGui::processDD( String path, String dataType, String displayType,
		Bool autoRegister, int insertPosition,
		bool masterCoordinate, bool masterSaturation, bool masterHue,
		QtDisplayData* qdd, const viewer::DisplayDataOptions& /*ddo*/) {
	if(qdd->isEmpty()) {
		errMsg_ = qdd->errMsg();
		status( "display data creation failed: " + errMsg_ );
		emit createDDFailed(errMsg_, path, dataType, displayType);
		return 0;
	}

	// Be sure name is unique by adding numerical suffix if necessary.
	String name=qdd->name();
	for(Int i=2; dd(name)!=0; i++) {
		name=qdd->name() + " <" + viewer::to_string( i ) + ">";
	}
	qdd->setName(name);
	qdd->setPlotTitle();
	status( "loaded: " + qdd->path( ) );

	//By default if there are no images,
	//we make it the master coordinate image, provided
	//we don't already have a master coordinate image.
	QtDisplayData* controllingData = displayDataHolder->getDDControlling();
	if ( controllingData == NULL && autoRegister ){
		int imageCount = displayDataHolder->getCount();
		if (imageCount == 0 ){
			masterCoordinate = true;
		}
	}
	displayDataHolder->addDD( qdd, insertPosition, autoRegister,
			masterCoordinate, masterSaturation,	 masterHue );

	updateDDMenus_( true );
	emit ddCreated(qdd, autoRegister, insertPosition, masterCoordinate);
	updateFrameInformation();
	if ( regionDock_ ) {
		regionDock_->updateRegionStats( );
	}

	connect( qdd, SIGNAL(showColorHistogram(QtDisplayData*)), this, SLOT(showColorHistogram(QtDisplayData*)));

	//Allows dds to synchronize options that are global.
	connect( qdd, SIGNAL(globalOptionsChanged(QtDisplayData*, Record)),
			this,	SLOT(globalOptionsChanged(QtDisplayData*, Record)));
	return qdd;
}

void QtDisplayPanelGui::createRGBImage( QtDisplayData* coordinateMaster,
		QtDisplayData* redImage, QtDisplayData* greenImage,
		QtDisplayData* blueImage ){
	String redName(" ");
	if ( redImage != NULL ){
		redName = Util::mainImageName( redImage->name());
	}
	String greenName( " ");
	if ( greenImage != NULL ){
		greenName = Util::mainImageName( greenImage->name());
	}
	String blueName( " ");
	if ( blueImage != NULL ){
		blueName = Util::mainImageName( blueImage->name());
	}
	String imageName = "RGB("+redName+","+greenName+","+blueName+").image";
	cout << "Image name="<<imageName<<endl;

	std::tr1::shared_ptr<ImageInterface<Float> > imgPtr = coordinateMaster->imageInterface();
	/*********************************************************
	 * SML NOTE:  Need to get image and set in red, blue, green
	 * before it gets painted - then add it to panel.
	 *
	 *
	 *
	 */

	QtDisplayData* rgbImage = addDD( imageName, "image", "raster", true, true, imgPtr );
	if ( rgbImage != NULL ){

		//Set the data that will determine the amount of red, green, blue
		DisplayData* rgbDD = rgbImage->dd();
		if ( redImage != NULL ){
			cout << "Set red image"<<endl;
			rgbDD->setDisplayDataRed( redImage->dd() );
		}
		if ( greenImage != NULL ){
			rgbDD->setDisplayDataGreen( greenImage->dd() );
		}
		if ( blueImage != NULL ){
			cout << "Set blue image"<<endl;
			rgbDD->setDisplayDataBlue( blueImage->dd() );
		}
	}
	else {
		cout << "Could not add RGB DD"<<endl;
	}
}


void QtDisplayPanelGui::updateFrameInformationImage(){
	if ( qdp_ == NULL || animationHolder == NULL ){
		return;
	}
	//Determine whether we should show the image animator.
	QSet<QString> uniqueImages;
	DisplayDataHolder::DisplayDataIterator iter = qdp_->beginRegistered();
	while ( iter != qdp_->endRegistered()) {
		QtDisplayData* rdd = (*iter);
		const viewer::ImageProperties & imgProperties = rdd->imageProperties( );
		const string imagePath = imgProperties.path();
		//In the image animator, don't cycle over contours, markers,
		//or vectors.
		QString imagePathStr( imagePath.c_str());
		if ( !rdd->isSkyCatalog()) {
			uniqueImages.insert( imagePathStr );
		}
		iter++;
	}
	animationHolder->setModeEnabled( uniqueImages.size() );
	//Update the animator to reflect the current axis state.
	if ( adjust_image_animator ) {
		adjust_image_animator = false;
		if ( animationHolder->getImageCount( ) <= 1 ) {
			animationHolder->foldImage( );
		} else {
			animationHolder->unfoldImage( );
		}
	}
}

int QtDisplayPanelGui::numFrames( ) {
	int frameCount = -1;
	if ( animationHolder != NULL ){
		frameCount = animationHolder->getChannelCount();
	}
	else {
		frameCount = qdp_->nFrames( );
	}
	return frameCount;
}

void QtDisplayPanelGui::updateFrameInformationChannel(){
	if ( qdp_ == NULL || animationHolder == NULL ){
		return;
	}
	//This figure is the maximum number of channels in any image.
	//We should enable the channel animator if there is at least one
	//image with more than one channel.
	int maxChannels = qdp_->nZFrames();
	int actualChannels = maxChannels;
	if ( maxChannels > 1 ) {
		//To find the actual number of channels, we rely on the
		//the image that is currently on target for channeling.
		QtDisplayData* channelMaster = qdp_->getChannelDD(animationImageIndex);
		if ( channelMaster != NULL ){
			DisplayData* dd = channelMaster->dd();
			if ( dd != NULL ){
				actualChannels = dd->nelements();
			}
		}
	}

	//If we are in channel mode we use the actual number of channels.
	int count = actualChannels;
	if ( qdp_->modeZ()){
		animationHolder->setChannelModeEnabled( actualChannels );
	}
	//Use the maximum number of channels, but since we aren't in channel
	//mode, we don't want it to come up selected.
	else {
		animationHolder->setChannelModeEnabled( maxChannels, false);
		count = maxChannels;
	}
	if ( regionDock_ != NULL ){
		regionDock_->updateFrameCount( count );
	}
}

void QtDisplayPanelGui::updateFrameInformation() {
	updateFrameInformationChannel();
	updateFrameInformationImage();
}

int QtDisplayPanelGui::getBoundedChannel( int channelNumber ) const {
	int boundedChannel = channelNumber;
	int lowerBoundChannel = animationHolder->getLowerBoundChannel();
	if ( boundedChannel < lowerBoundChannel ) {
		boundedChannel = lowerBoundChannel;
	}
	int upperBoundChannel = animationHolder->getUpperBoundChannel();
	if ( boundedChannel > upperBoundChannel ) {
		boundedChannel = upperBoundChannel;
	}
	return boundedChannel;
}
void QtDisplayPanelGui::addDDSlot(String path, String dataType, String displayType,
		Bool autoRegister, Bool tmpData, std::tr1::shared_ptr<ImageInterface<Float> > img) {
	//std::tr1::shared_ptr<ImageInterface<Float> > imgPtr(img);
	addDD( path, dataType, displayType, autoRegister, tmpData, img );

}

QtDisplayData* QtDisplayPanelGui::addDD(String path, String dataType, String displayType, Bool autoRegister, Bool tmpData,
		std::tr1::shared_ptr<ImageInterface<Float> > img) {
	// create a new DD
	QtDisplayData* dd = NULL;
	if ( ! img ) {
		dd = createDD(path, dataType, displayType, autoRegister);
	} else {
		dd =new QtDisplayData( this, path, dataType, displayType);
		dd->setImage( img );
		dd->initImage();
		dd->init();
		dd = processDD( path, dataType, displayType, autoRegister,
				-1, false, false, false, dd );
	}

	// set flagg if requested
	if (tmpData && dd != NULL ) {
		dd->setDelTmpData(True);
	}
	return dd;
}

void QtDisplayPanelGui::setAnimatedImage( int index ){
	if ( qdp_->modeZ() ){
		qdp_->setMode( false );
	}
	qdp_->goTo( index );
}

void QtDisplayPanelGui::doSelectChannel( int channelNumber ) {
	//Make sure the channel number is not outside the min/max bounds
	//of the animator.
	int boundedChannel = getBoundedChannel( channelNumber );
	qdp_->goTo( boundedChannel, true );

	qdp_->goTo( boundedChannel, true );
	int frameCount = qdp_->nFrames();
	animationHolder->setFrameInformation( AnimatorHolder::NORMAL_MODE, channelNumber, frameCount );
	emit frameChanged( boundedChannel );
}

void QtDisplayPanelGui::incrementMovieChannel() {

	//Increment/Decrement the channel
	movieChannel = movieChannel + movieStep;

	//Take care of wrap around in either direction.
	if ( movieChannel > movieLast ) {
		movieChannel = movieStart;
	}
	if ( movieChannel < movieStart ) {
		movieChannel = movieLast;
	}

	//Check to see if we should stop or continue playing
	if ( movieChannel == movieChannelEnd ) {
		movieTimer.stop();
	} else {
		doSelectChannel( movieChannel );
	}
}

void QtDisplayPanelGui::movieChannels( int startChannel, int endChannel ) {
	//Make sure it is not currently playing
	//before we start a new one.
	movieTimer.stop();

	//Make sure the range of the move falls withen the animation range.
	int boundedChannelStart = getBoundedChannel( startChannel );
	int boundedChannelEnd = getBoundedChannel( endChannel );
	movieLast = boundedChannelEnd + 1;
	movieStart = boundedChannelStart - 1;
	if ( boundedChannelStart < boundedChannelEnd ) {
		movieStep = 1;
	} else {
		movieStep = -1;
	}
	//Movie last must be larger than movie start so we can use them for wrap
	//around and stopping criteria.
	if ( movieLast < movieStart ){
		int tmp = movieLast;
		movieLast = movieStart;
		movieStart = tmp;
	}

	//Start a new movie.
	setAnimationRate();

	movieChannel = boundedChannelStart;
	movieChannelEnd = boundedChannelEnd + 1;
	movieTimer.start();
}

void QtDisplayPanelGui::setAnimationRate() {
	int animationRate = animationHolder->getRate( AnimatorHolder::NORMAL_MODE );
	movieTimer.setInterval( 1000/ animationRate );
}

void QtDisplayPanelGui::movieChannels( int startChannel, bool forward,
		int stepSize, int channelMin, int channelMax ) {
	movieTimer.stop();

	movieLast = channelMax;
	movieChannelEnd = -1;
	movieStart = channelMin;

	if ( forward ) {
		movieStep = stepSize;
	} else {
		movieStep = -1 * stepSize;
	}
	movieChannel = startChannel;
	setAnimationRate();
	movieTimer.start();
}

void QtDisplayPanelGui::movieStop() {
	movieTimer.stop();
}

void QtDisplayPanelGui::removeAllDDs() {
	if ( qdp_ != NULL ){

		qdp_->setControllingDD( NULL );
		qdp_->unregisterAll();
	}

	//Order is important.
	//First get a list of everything going down.
	DisplayDataHolder::DisplayDataIterator iter = displayDataHolder->beginDD();
	QList<QtDisplayData*>  removeDDs;
	while ( iter != displayDataHolder->endDD()) {
		QtDisplayData* qdd = (*iter);
		removeDDs.append( qdd );
		iter++;
	}

	//Clean up for each one.
	for ( int i = 0; i < removeDDs.size(); i++ ) {
		removeDD( removeDDs[i] );
	}

	//Remove them all from the master list
	displayDataHolder->removeDDAll();
	updateFrameInformation();
}

void QtDisplayPanelGui::clearTools() {
	//Tell other widgets to update.
	if ( regionDock_ ) {
		regionDock_->updateRegionStats( );
	}
	if ( fitTool != NULL ) {
		std::tr1::shared_ptr<const ImageInterface<Float> > p;
		fitTool->setImage( p );
	}
	if ( histogrammer != NULL ) {
		std::tr1::shared_ptr< ImageInterface<Float> > p;
		histogrammer->setImage( p );
	}
}

Bool QtDisplayPanelGui::isEmptyDD() const {
	return displayDataHolder->isEmpty();
}

DisplayDataHolder::DisplayDataIterator QtDisplayPanelGui::beginDD() const {
	return displayDataHolder->beginDD();
}

DisplayDataHolder::DisplayDataIterator QtDisplayPanelGui::endDD() const {
	return displayDataHolder->endDD();
}
void QtDisplayPanelGui::notifyDDRemoval( QtDisplayData* qdd ){
	if ( qdp_ != NULL ){
		qdp_->unregisterDD(qdd );
	}
	//emit ddRemoved(qdd);
	if ( qdo_ != NULL ){
		qdo_->removeDD(qdd);
	}
	if (trkgDockWidget_ != NULL ){
		trkgDockWidget_->removeTrackBox( qdd );
	}
	if ( regionDock_ ){
		regionDock_->updateRegionStats( );
	}
	if ( qdm_ != NULL ){
		qdm_->updateDisplayDatas(qdd);
	}
	if ( imageManagerDialog != NULL ){
		imageManagerDialog->closeImageView( qdd );
	}
	updateDDMenus_( true );
}


Bool QtDisplayPanelGui::removeDD(QtDisplayData*& qdd) {
	bool removed = displayDataHolder->removeDD( qdd );
	//In case we are removing the coordinate master.
	if ( displayDataHolder->isCoordinateMaster( qdd) && qdp_!= NULL ){
		qdp_->setControllingDD( NULL );
	}
	if ( removed ) {
		int imageCount = displayDataHolder->getCount();
		if ( imageCount == 0 ){
			if ( qdp_ != NULL ){
				qdp_->setControllingDD( NULL );
			}
			displayDataHolder->setDDControlling( NULL );
			clearTools();
			if ( profile_ != NULL ) {
				profile_->clearPlots();
			}
			if ( regionDock_ != NULL ){
				regionDock_->delete_all_regions( true );
			}
		}

		notifyDDRemoval( qdd );
		delete qdd;
		qdd = NULL;
		updateFrameInformation();
	}
	return removed;
}



Bool QtDisplayPanelGui::ddExists(QtDisplayData* qdd) {
	return displayDataHolder->exists( qdd );
}

void QtDisplayPanelGui::loadRegions( const QString &path, const QString &type ) {
	loadRegions( path.toStdString( ), type.toStdString( ) );
}
void QtDisplayPanelGui::loadRegions( const std::string &path, const std::string &type ) {
	if ( logger_did_region_warning == false ) {
		logger << LogIO::NORMAL
				<< "currently only supports rectangle, ellipse, symbol (somewhat), and polygon region shapes"
				<< LogIO::POST;
		logger_did_region_warning = true;
	}

	qdp_->loadRegions( path, type );
}

std::string QtDisplayPanelGui::outputRegions( std::list<viewer::QtRegionState*> regions,
		std::string file, std::string format, std::string ds9_csys ) {
	std::transform(format.begin(), format.end(), format.begin(), ::tolower);
	if ( format != "ds9" && format != "crtf" ) {
		return "invalid output format '" + format + "'";
	}
	return regionDock_->outputRegions( regions, file, format, ds9_csys );
}

void QtDisplayPanelGui::activate( bool state ) {
	qdp_->activate(state);
}

void QtDisplayPanelGui::updateCursorInfo( WorldCanvas *wc, Quantity x, Quantity y ) {
	Vector<String> units = wc->worldAxisUnits( );
	if ( x.isConform(units(0)) == false || y.isConform(units(1)) == false )
		return;

	std::vector<double> wpt(2);
	wpt[0] = x.getValue(units(0));
	wpt[1] = y.getValue(units(1));

	for ( DisplayDataHolder::DisplayDataIterator iter = beginDD( ); iter != endDD( ); ++iter ) {
		trkgDockWidget_->cursorUpdate( wpt, *iter );
	}
}

QtDisplayData* QtDisplayPanelGui::dd(const std::string& name) {
	// retrieve DD with given name (0 if none).
	QtDisplayData* qdd = displayDataHolder->getDD(name);
	return qdd;
}

QtDisplayData* QtDisplayPanelGui::lookForExistingController() {
	QtDisplayData *ctrld = 0;
	DisplayDataHolder::DisplayDataIterator iter = qdp_->endRegistered();
	while( iter != qdp_->beginRegistered()) {
		iter--;
		QtDisplayData* pdd = (*iter);
		if ( pdd != 0 && pdd->isImage() ) {
			std::tr1::shared_ptr<ImageInterface<float> > img = pdd->imageInterface( );
			PanelDisplay* ppd = qdp_->panelDisplay( );
			if ( ppd != 0 && ppd->isCSmaster(pdd->dd()) && img ) {
				ctrld = pdd;
				break;
			}
		}
	}
	return ctrld;
}


QtDisplayData* QtDisplayPanelGui::dd( ) {
	// retrieve the "controlling" DD...
	QtDisplayData* controlling_dd = displayDataHolder->getDDControlling();
	if ( controlling_dd == 0 ) {
		controlling_dd = lookForExistingController();
		/*if ( controlling_dd != 0 ) {
				emit axisToolUpdate( controlling_dd );
				//The connection below represents a threading problem.  Arrays should
				//not be passed between threads because they are not deep copied. To
				//reproduce the threading problem, uncomment the code, set the coordinate
				//system master to NULL and then try to bring up the histogram tool.
				//connect( controlling_dd, SIGNAL(axisChanged(String, String, String, std::vector<int>)),
				  //       SLOT(controlling_dd_axis_change(String, String, String, std::vector<int> )) );
			}*/
	}
	return controlling_dd;
}


List<QtDisplayData*> QtDisplayPanelGui::unregisteredDDs() {
	// return a list of DDs that exist but are not registered on any panel.
	std::list<QtDisplayPanelGui*> dps(viewer( )->openDPs());
	List<QtDisplayData*> uDDs;
	ListIter<QtDisplayData*> uDDsIter( uDDs );
	DisplayDataHolder::DisplayDataIterator iter = displayDataHolder->beginDD();
	while ( iter != displayDataHolder->endDD()) {
		QtDisplayData* dd = (*iter);
		iter++;
		Bool regd = False;
		for ( std::list<QtDisplayPanelGui*>::iterator iter = dps.begin( ); iter != dps.end( ); ++iter ) {
			if((*iter)->displayPanel( )->isRegistered(dd)) {
				regd = True;
				break;
			}
		}

		if ( !regd ) {
			uDDsIter.addRight( dd );
		}
	}
	return uDDs;
}

//# ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
//# DisplayData functionality brought down from QtViewerBase
//# ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----


void QtDisplayPanelGui:: addedData( QString /*type*/, QtDisplayData * ) { }

// Animation slots.

void QtDisplayPanelGui::updateAnimUi_() {

	// Current animator state.
	Int  frm   = qdp_->frame();
	Int len  = qdp_->nFrames();
	qdp_->startFrame();
	qdp_->lastFrame();
	qdp_->step();
	Int     rate = qdp_->animRate();
	Int     minr  = qdp_->minRate();
	Int maxr = qdp_->maxRate();
	//Int play  = qdp_->animating();
	Bool modez = qdp_->modeZ();

	emit frameChanged( frm );

	if ( animationHolder != NULL ) {
		animationHolder->setFrameInformation( modez, frm, len );
		animationHolder->setRateInformation( modez, minr, maxr, rate );
		//animationHolder->setPlaying( modez, play );
	}
	if ( histogrammer != NULL ) {
		if ( modez ) {
			histogrammer->setChannelValue( frm );
		}
	}

	updateViewedImage();
}


void QtDisplayPanelGui::updateViewedImage(){
	Bool modez = qdp_->modeZ();
	Int  frm   = qdp_->frame();

	//Tell the image manager what is being viewed now.
	if ( imageManagerDialog != NULL ){
		if ( !modez ){
			imageManagerDialog->setViewedImage( frm );
		}
		else {
			imageManagerDialog->setViewedImage( -1 );
		}
	}

	//Update the title to the 'Channel' Animator if the axis changes type.
	int lookUpIndex = -1;
	if ( qdp_ != NULL && !qdp_->modeZ() ){
		lookUpIndex = animationImageIndex;
	}

	QtDisplayData* newViewedImage = qdp_->getChannelDD( lookUpIndex );
	if ( newViewedImage != NULL ){

		//Disconnect listening to the previous 'channel' DD.
		for (DisplayDataHolder::DisplayDataIterator iter = qdp_->beginRegistered();
			iter != qdp_->endRegistered(); iter++) {
				disconnect( (*iter), SIGNAL(axisChanged(String, String, String, std::vector<int>)),
										this, SLOT(controlling_dd_axis_change(String, String, String, std::vector<int> )) );
		}

		//Hook up the new dd so we get axis change updates.
		connect( newViewedImage, SIGNAL(axisChanged(String, String, String, std::vector<int>)),
				this, SLOT(controlling_dd_axis_change(String, String, String, std::vector<int> )),
				Qt::UniqueConnection );

		//Update the animator to reflect the current axis state.
		if ( animationHolder != NULL ){
			String zAxisName = newViewedImage->getZAxisName();
			animationHolder->setChannelZAxis( zAxisName.c_str());
			if ( adjust_channel_animator ) {
				adjust_channel_animator = false;
				if ( animationHolder->getChannelCount( ) <= 1 ) {
					animationHolder->foldChannel( );
				} else {
					animationHolder->unfoldChannel( );
				}
			}
		}
	}
}

// Public slots: may be safely operated programmatically (i.e.,
// scripted, when available), or via gui actions.

void QtDisplayPanelGui::hideImageMenus() {

	DisplayDataHolder::DisplayDataIterator iter = qdp_->beginRegistered();
	while ( iter != qdp_->endRegistered()) {
		QtDisplayData* pdd = (*iter);

		iter++;
		if(pdd != 0) {
			std::tr1::shared_ptr<ImageInterface<float> > img = pdd->imageInterface();

			PanelDisplay* ppd = qdp_->panelDisplay();
			//cout << "ppd->isCSmaster="
			//      << ppd->isCSmaster(pdd->dd()) << endl;
			if (ppd != 0 && ppd->isCSmaster(pdd->dd())) {
				if (pdd->isImage() && img ) {
					if ( fboxAct_ ) fboxAct_->setEnabled(True);
					if ( mkRgnAct_ ) mkRgnAct_->setEnabled(True);
					if ( annotAct_ ) annotAct_->setEnabled(True);
					profileAct_->setEnabled(True);
					momentsCollapseAct_->setEnabled(True);
					histogramAct_->setEnabled(True);
					fitAct_->setEnabled( True );
					if ( shpMgrAct_ ) shpMgrAct_->setEnabled(True);
					setUseRegion(False);
					break;
				}
				if (pdd->isMS() || img ==0) {

					hideRegionManager();
					hideAnnotatorPanel();
					hideFileBoxPanel();
					hideMakeRegionPanel();
					hideImageProfile();
					hideFit2DTool();
					hideHistogram();
					hideShapeManager();
					hideStats();

					if ( fboxAct_ ) fboxAct_->setEnabled(False);
					if ( mkRgnAct_ ) mkRgnAct_->setEnabled(False);
					if ( annotAct_ ) annotAct_->setEnabled(False);
					profileAct_->setEnabled(False);
					momentsCollapseAct_->setEnabled( False );
					histogramAct_->setEnabled( False );
					fitAct_->setEnabled( False );
					if ( shpMgrAct_ ) shpMgrAct_->setEnabled(False);
					setUseRegion(False);
					//cout << "hide image menus" << endl;
					break;
				}
			}
		}
	}
}

void QtDisplayPanelGui::hideAllSubwindows() {
	hidePrintManager();
	hideCanvasManager();
	hideRegionManager();
	hideAnnotatorPanel();
	hideFileBoxPanel();
	hideMakeRegionPanel();
	hideImageProfile();
	hideFit2DTool();
	hideHistogram();
	hideDataManager();
	hideExportManager();
	hideDataOptionsPanel();
	hideStats();
}

QtDisplayPanelGui *QtDisplayPanelGui::createNewPanel( ) {
	QtDisplayPanelGui *new_panel = v_->createDPG( );
	new_panel->show( );
	return new_panel;
}

void QtDisplayPanelGui::showDataManager() {
	if(qdm_==0) {
		qdm_ = new QtDataManager(this);
		/*connect( this, SIGNAL(ddRemoved(QtDisplayData*)),
					qdm_, SLOT(updateDisplayDatas(QtDisplayData*)));*/
		connect( this, SIGNAL(ddCreated(QtDisplayData*, Bool, int, Bool)),
				qdm_, SLOT(updateDisplayDatas(QtDisplayData*, Bool)));
	}
	qdm_->showNormal();
	qdm_->raise();
}

void QtDisplayPanelGui::hideDataManager() {
	if(qdm_==0) return;
	qdm_->hide();
}

void QtDisplayPanelGui::showExportManager() {
	showDataManager( );
	qdm_->showTab("save image");
}

void QtDisplayPanelGui::hideExportManager() {
	hideDataManager( );
}

void QtDisplayPanelGui::showPreferences( ) {
	if ( preferences == 0 )
		preferences = new viewer::Preferences( );
	preferences->showNormal( );
	preferences->raise( );
}
void QtDisplayPanelGui::updateMultiSpectralFitLocation( Record trackingRec) {
	//The profiler needs to know the location of the mouse so it can update
	//the multi-pixel spectral fit if it has one.
	if ( profile_!= NULL && profile_->isVisible() ) {
		for(uInt i=0; i<trackingRec.nfields(); i++) {
			profile_->processTrackRecord( trackingRec.name(i), trackingRec.asString(i) );
		}
	}
}


void QtDisplayPanelGui::showDataOptionsPanel() {
	//  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---
	// prevent infinate recursion, due to loop:
	//		#1321 0x00e37264 in casa::QtDataOptionsPanel::createDDTab_ ()
	//		#1322 0x00e375b9 in casa::QtDataOptionsPanel::QtDataOptionsPanel ()
	//		#1323 0x00eda8bf in casa::QtDisplayPanelGui::showDataOptionsPanel ()
	//		#1324 0x00e37264 in casa::QtDataOptionsPanel::createDDTab_ ()
	//		#1325 0x00e375b9 in casa::QtDataOptionsPanel::QtDataOptionsPanel ()
	//		#1326 0x00eda8bf in casa::QtDisplayPanelGui::showDataOptionsPanel ()
	//  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---

	if ( showdataoptionspanel_enter_count == 0 ) {
		++showdataoptionspanel_enter_count;
		if(qdo_==0) {
			qdo_ = new QtDataOptionsPanel(this);
			connect( qdo_, SIGNAL( globalColorSettingsChanged(bool)), this, SLOT(globalColorSettingsChanged( bool )));
		}
		if(qdo_!=0) {  // (should be True, barring exceptions above).
			qdo_->showNormal();
			qdo_->raise();
		}
		--showdataoptionspanel_enter_count;
	}
}


void QtDisplayPanelGui::hideDataOptionsPanel() {
	if(qdo_==0) return;
	qdo_->hide();
}


void QtDisplayPanelGui::showPrintManager() {
	if(qpm_==0) qpm_ = new QtViewerPrintGui(qdp_);
	qpm_->showNormal();	// (Magic formula to bring a window up,
	qpm_->raise();
}	// normal size, whether 'closed' (hidden),
// iconified, or merely obscured by other
// windows.  (Found through trial-and-error).

void QtDisplayPanelGui::hidePrintManager() {
	if(qpm_==0) return;
	qpm_->hide();
}



void QtDisplayPanelGui::showCanvasManager() {
	if(qcm_==0) qcm_ = new QtCanvasManager(this);
	qcm_->showNormal();
	qcm_->raise();
}

void QtDisplayPanelGui::hideCanvasManager() {
	if(qcm_==0) return;
	qcm_->hide();
}

void QtDisplayPanelGui::showStats(const String& stats) {
	cout << stats << endl;

	/* this print stats on to a panel
		QFont font("Monospace");
		if(qst_==0) {
		 qst_ = new QTextEdit;
		 qst_->setWindowTitle(QObject::tr("Image Statistics"));
		 //qst_->setLineWrapMode(QPlainTextEdit::WidgetWidth);
		 font.setStyleHint(QFont::TypeWriter);
		 qst_->setCurrentFont(font);
		 qst_->setReadOnly(1);
		 qst_->setMinimumWidth(600);
		}
		QString s;
		s=stats.c_str();
		QFontMetrics fm(font);
		int len=s.length();
		int last = s.lastIndexOf('\n', len - 3);
		//cout << "length=" << len << " last=" << last << endl;
		QString lastLine=s.right(max(len - last,0));
		//cout << "lastLine=" << lastLine.toStdString()
		//     << "<<<==========\n" << endl;
		int width=fm.width(lastLine);
		//cout << "width=" << width << endl;
		qst_->resize(width, qst_->size().height());
		qst_->append(s);
		qst_->showNormal();
		qst_->raise();
	 */
}

void QtDisplayPanelGui::trackingMoved(Qt::DockWidgetArea area) {
	if ( area == Qt::RightDockWidgetArea ) {
		rc.put( "viewer." + rcid() + ".position.cursor_tracking", "right" );
	} else if ( area == Qt::BottomDockWidgetArea ) {
		rc.put( "viewer." + rcid() + ".position.cursor_tracking", "bottom" );
	} else if ( area == Qt::LeftDockWidgetArea ) {
		rc.put( "viewer." + rcid() + ".position.cursor_tracking", "left" );
	} else if ( area == Qt::TopDockWidgetArea ) {
		rc.put( "viewer." + rcid() + ".position.cursor_tracking", "top" );
	}
}

void QtDisplayPanelGui::animatorMoved(Qt::DockWidgetArea area) {
	if ( area == Qt::RightDockWidgetArea ) {
		rc.put( "viewer." + rcid() + ".position.animator", "right" );
	} else if ( area == Qt::BottomDockWidgetArea ) {
		rc.put( "viewer." + rcid() + ".position.animator", "bottom" );
	} else if ( area == Qt::LeftDockWidgetArea ) {
		rc.put( "viewer." + rcid() + ".position.animator", "left" );
	} else if ( area == Qt::TopDockWidgetArea ) {
		rc.put( "viewer." + rcid() + ".position.animator", "top" );
	}
}

void QtDisplayPanelGui::regionMoved(Qt::DockWidgetArea area) {
	if ( area == Qt::RightDockWidgetArea ) {
		rc.put( "viewer." + rcid() + ".position.regions", "right" );
	} else if ( area == Qt::BottomDockWidgetArea ) {
		rc.put( "viewer." + rcid() + ".position.regions", "bottom" );
	} else if ( area == Qt::LeftDockWidgetArea ) {
		rc.put( "viewer." + rcid() + ".position.regions", "left" );
	} else if ( area == Qt::TopDockWidgetArea ) {
		rc.put( "viewer." + rcid() + ".position.regions", "top" );
	}
}

void QtDisplayPanelGui::mousetoolbarMoved(bool) {
	if ( toolBarArea(mouseToolBar_) == Qt::TopToolBarArea ) {
		rc.put( "viewer." + rcid() + ".position.mousetools", "top" );
	} else if ( toolBarArea(mouseToolBar_) == Qt::LeftToolBarArea ) {
		rc.put( "viewer." + rcid() + ".position.mousetools", "left" );
	} else if ( toolBarArea(mouseToolBar_) == Qt::RightToolBarArea ) {
		rc.put( "viewer." + rcid() + ".position.mousetools", "right" );
	} else if ( toolBarArea(mouseToolBar_) == Qt::BottomToolBarArea ) {
		rc.put( "viewer." + rcid() + ".position.mousetools", "bottom" );
	}
}

std::string QtDisplayPanelGui::getrc( const std::string &key ) {
	return rc.get( "viewer." + rcid() + "." + key );
}
void QtDisplayPanelGui::putrc( const std::string &key, const std::string &val ) {
	rc.put( "viewer." + rcid() + "." + key, val );
}


void QtDisplayPanelGui::hideStats() {
	if(qst_==0) return;
	qst_->hide();
}

void QtDisplayPanelGui::showRegionManager() {
	if(qrm_==0) return;
	/*List<QtDisplayData*> rdds = qdp_->registeredDDs();
		for (ListIter<QtDisplayData*> qdds(&rdds); !qdds.atEnd(); qdds++) {
			QtDisplayData* pdd = qdds.getRight();
			if(pdd != 0 && pdd->dataType() == "image") {

				ImageInterface<float>* img = pdd->imageInterface();
				PanelDisplay* ppd = qdp_->panelDisplay();
				if (ppd != 0 && ppd->isCSmaster(pdd->dd()) && img != 0) {*/
	QtDisplayData* pdd = dd();
	if ( pdd != NULL ) {
		connect(pdd, SIGNAL(axisChanged(String, String, String, std::vector<int> )),
				qrm_, SLOT(changeAxis(String, String, String, std::vector<int> )));
		//	}
	//	}
	}
	qrm_->showNormal();
	qrm_->raise();
}

void QtDisplayPanelGui::hideRegionManager() {
	if(qrm_==0) return;
	qrm_->hide();
}



void QtDisplayPanelGui::showShapeManager() {
	if(qsm_==0) qsm_ = new QtRegionShapeManager(qdp_);
	/*List<QtDisplayData*> rdds = qdp_->registeredDDs();
		for (ListIter<QtDisplayData*> qdds(&rdds); !qdds.atEnd(); qdds++) {
			QtDisplayData* pdd = qdds.getRight();
			if(pdd != 0 && pdd->dataType() == "image") {

				ImageInterface<float>* img = pdd->imageInterface();
				PanelDisplay* ppd = qdp_->panelDisplay();
				if (ppd != 0 && ppd->isCSmaster(pdd->dd()) && img != 0) {*/
	QtDisplayData* ppd = dd();
	if ( ppd != NULL ) {
		qsm_->showNormal();
		qsm_->raise();
	}
}

void QtDisplayPanelGui::hideShapeManager() {
	if(qsm_==0) return;
	qsm_->hide();
}

void QtDisplayPanelGui::showFileBoxPanel() {

	if (qfb_ == 0)
		qfb_ = new FileBox(qdp_);

	/*List<QtDisplayData*> rdds = qdp_->registeredDDs();
		for (ListIter<QtDisplayData*> qdds(&rdds); !qdds.atEnd(); qdds++) {
			QtDisplayData* pdd = qdds.getRight();
			if(pdd != 0 && pdd->dataType() == "image") {

				ImageInterface<float>* img = pdd->imageInterface();
				PanelDisplay* ppd = qdp_->panelDisplay();
				if (ppd != 0 && ppd->isCSmaster(pdd->dd()) && img != 0) {*/
	QtDisplayData* ppd = dd();
	if ( ppd != NULL ) {
		connect(qfb_,  SIGNAL(hideFileBox()),
				SLOT(hideFileBoxPanel()));
		connect(ppd, SIGNAL(axisChanged(String, String, String, std::vector<int> )),
				qfb_, SLOT(changeAxis(String, String, String, std::vector<int> )));
	}
	qfb_->showNormal();
	qfb_->raise();
	if ( annotAct_ ) annotAct_->setEnabled(False);
	if ( mkRgnAct_ ) mkRgnAct_->setEnabled(False);
	setUseRegion(True);
}

void QtDisplayPanelGui::hideFileBoxPanel() {
	if (qfb_==0)
		return;
	qfb_->hide();
	if ( annotAct_ ) annotAct_->setEnabled(True);
	if ( mkRgnAct_ ) mkRgnAct_->setEnabled(True);
	setUseRegion(False);
}

void QtDisplayPanelGui::showAnnotatorPanel() {

	if (qap_ == 0)
		qap_ = new MakeMask(qdp_);

	/*List<QtDisplayData*> rdds = qdp_->registeredDDs();
		for (ListIter<QtDisplayData*> qdds(&rdds); !qdds.atEnd(); qdds++) {
			QtDisplayData* pdd = qdds.getRight();
			if(pdd != 0 && pdd->dataType() == "image") {

				ImageInterface<float>* img = pdd->imageInterface();
				PanelDisplay* ppd = qdp_->panelDisplay();
				if (ppd != 0 && ppd->isCSmaster(pdd->dd()) && img != 0) {*/
	QtDisplayData* pdd = dd();
	if ( pdd != NULL ) {
		connect(qap_,  SIGNAL(hideRegionInFile()), SLOT(hideAnnotatorPanel()));
		connect(pdd, SIGNAL(axisChanged(String, String, String, std::vector<int> )),
				qap_, SLOT(changeAxis(String, String, String, std::vector<int> )));
	}
	qap_->showNormal();
	qap_->raise();
	if ( fboxAct_ ) fboxAct_->setEnabled(False);
	if ( mkRgnAct_ ) mkRgnAct_->setEnabled(False);
	setUseRegion(True);
}

void QtDisplayPanelGui::hideAnnotatorPanel() {
	//cout << "hide--------region in image" << endl;
	if (qap_==0)
		return;
	qap_->hide();
	if ( fboxAct_ ) fboxAct_->setEnabled(True);
	if ( mkRgnAct_ ) mkRgnAct_->setEnabled(True);
	setUseRegion(False);
}

void QtDisplayPanelGui::showMakeRegionPanel() {

	if (qmr_ == 0)
		qmr_ = new MakeRegion(qdp_);

	/*List<QtDisplayData*> rdds = qdp_->registeredDDs();
		for (ListIter<QtDisplayData*> qdds(&rdds); !qdds.atEnd(); qdds++) {
			QtDisplayData* pdd = qdds.getRight();*/
	DisplayDataHolder::DisplayDataIterator iter = qdp_->beginRegistered();
	while ( iter != qdp_->endRegistered()) {
		QtDisplayData* pdd = (*iter);
		iter++;
		if(pdd != 0 && pdd->dataType() == "image") {

			std::tr1::shared_ptr<ImageInterface<float> > img = pdd->imageInterface();
			PanelDisplay* ppd = qdp_->panelDisplay();
			if (ppd != 0 && ppd->isCSmaster(pdd->dd()) && img ) {
				connect(qmr_,  SIGNAL(hideRegionInImage()),
						SLOT(hideMakeRegionPanel()));
				qmr_->showNormal();
				qmr_->raise();
				break;
			}
		}
	}
	if ( fboxAct_ ) fboxAct_->setEnabled(False);
	if ( annotAct_ ) annotAct_->setEnabled(False);
	setUseRegion(True);

}

void QtDisplayPanelGui::hideMakeRegionPanel() {
	//cout << "hide--------region in file" << endl;
	if (qmr_==0)
		return;
	qmr_->hide();
	if ( fboxAct_ ) fboxAct_->setEnabled(True);
	if ( annotAct_ ) annotAct_->setEnabled(True);
	setUseRegion(False);
}

void QtDisplayPanelGui::showImageProfile() {

	QList<OverplotInterface> overlays;
	bool profileVisible = false;
	if ( profile_ && profile_->isVisible()) {
		profileVisible = true;
	}

	//The image that will be channeled is the last registered one in the list.
	//This is the one that should be sent to the profiler.
	int i = 0;
	int lastRegistered = qdp_->getRegisteredCount() - 1;
	bool profiledDDChange = false;
	for ( DisplayDataHolder::DisplayDataIterator iter = qdp_->beginRegistered();
			iter != qdp_->endRegistered(); iter++ ){
		QtDisplayData* pdd = (*iter);
		if(pdd != 0 && pdd->isImage()) {

			std::tr1::shared_ptr<ImageInterface<float> > img = pdd->imageInterface();
			PanelDisplay* ppd = qdp_->panelDisplay();
			if (ppd != 0 && img ) {

				if (/*ppd->isCSmaster(pdd->dd())*/ i == lastRegistered ) {
					// pdd is a suitable QDD for profiling.
					if (!profile_) {
						// Set up profiler for first time.

						profile_ = new QtProfile(img, pdd->name().c_str());
						profile_->setPath(QString(pdd->path().c_str()) );
						connect( profile_, SIGNAL(hideProfile()), SLOT(hideImageProfile()));
						connect( qdp_, SIGNAL(registrationChange()), SLOT(refreshImageProfile()));
						connect( pdd, SIGNAL(axisChangedProfile(String, String, String, std::vector<int> )),
								profile_, SLOT(changeAxis(String, String, String, std::vector<int> )));
						connect( pdd, SIGNAL(spectrumChanged(String, String, String )),
								profile_, SLOT(changeSpectrum(String, String, String )));
						connect(profile_, SIGNAL(showCollapsedImg(String, String, String, Bool, Bool, std::tr1::shared_ptr<ImageInterface<Float> > )),
								this, SLOT(addDDSlot(String, String, String, Bool, Bool, std::tr1::shared_ptr<ImageInterface<Float> >)));
						connect(profile_, SIGNAL(channelSelect(int)), this, SLOT(doSelectChannel(int)));
						connect( this, SIGNAL(frameChanged(int)), profile_, SLOT(frameChanged(int)));
						connect( profile_, SIGNAL(movieChannel(int,int)), this, SLOT(movieChannels(int, int)));
						connectRegionSignals(ppd);
					} else {
						if (profileDD_ != pdd) {
							profiledDDChange = true;
							// [Re-]orient pre-existing profiler to pdd
							profile_->resetProfile(img, pdd->name().c_str());
							disconnect( profileDD_, SIGNAL(axisChangedProfile(String, String, String, std::vector<int> )),
									profile_, SLOT(changeAxis(String, String, String, std::vector<int> )));
							disconnect( profileDD_, SIGNAL(spectrumChanged(String, String, String )),
									profile_, SLOT(changeSpectrum(String, String, String )));
							profileDD_ = pdd;
							connect( profileDD_, SIGNAL(axisChangedProfile(String, String, String, std::vector<int> )),
									profile_, SLOT(changeAxis(String, String, String, std::vector<int> )));
							connect( profileDD_, SIGNAL(spectrumChanged(String, String, String )),
									profile_, SLOT(changeSpectrum(String, String, String )));
						} else {
							pdd->checkAxis();
						}
					}
					if (pdd->getAxisIndex(String("Spectral")) == -1 &&
						Util::getTabularFrequencyAxisIndex( img) == -1 ) {
					//if (pdd->getAxisIndex(String("Spectral")) == -1) {
						profileDD_ = 0;
						hideImageProfile();
						QMessageBox::warning( this, "Channel Image Problem", "The z-axis of the channel image is not frequency.");
					} else {
						profileDD_ = pdd;
						profile_->showNormal();
						profile_->raise();
						pdd->checkAxis();
					}
				} else {
					if (pdd->getAxisIndex(String("Spectral")) != -1 ||
							pdd->getAxisIndex(String("Tabular")) != -1){
						OverplotInterface overlap( pdd->name().c_str(), img );
						overlays.append( overlap );
					}
				}
				i++;
			}
		}
	}


	if (profile_) {

		connect( this, SIGNAL(overlay(QList<OverplotInterface>)),
				profile_, SLOT(overplot(QList<OverplotInterface>)));
		emit overlay(overlays);
	}

	PanelDisplay* ppd = qdp_->panelDisplay();
	std::tr1::shared_ptr<QtRectTool> rect = std::tr1::dynamic_pointer_cast<QtRectTool>(ppd->getTool(QtMouseToolNames::RECTANGLE));
	if ( (rect.get( ) != 0 && ! profileVisible) ||
			(profiledDDChange && profileVisible) ) {
		// this is the *new* region implementation... all events come from region source...
		std::tr1::shared_ptr<viewer::QtRegionSourceKernel> qrs = std::tr1::dynamic_pointer_cast<viewer::QtRegionSourceKernel>(rect->getRegionSource( )->kernel( ));
		qrs->generateExistingRegionUpdates( );
	}

	//Let the profiler know about the current frame.
	if ( profile_ ) {
		int frameIndex = qdp_->frame();
		profile_->frameChanged( frameIndex );
	}
}


void QtDisplayPanelGui::connectRegionSignals(PanelDisplay* ppd ) {
	std::tr1::shared_ptr<QtCrossTool> pos = std::tr1::dynamic_pointer_cast<QtCrossTool>(ppd->getTool(QtMouseToolNames::POINT));
	if ( pos.get( ) != 0 ) {
		connect( pos.get( ), SIGNAL(wcNotify( const String, const Vector<Double>, const Vector<Double>,
				const Vector<Double>, const Vector<Double>, const ProfileType)),
				profile_, SLOT(wcChanged( const String, const Vector<Double>, const Vector<Double>,
						const Vector<Double>, const Vector<Double>, const ProfileType)));
		connect( profile_, SIGNAL(coordinateChange(const String&)),
				pos.get( ), SLOT(setCoordType(const String&)));

		// one region source is shared among all of the tools...
		// so there is no need to connect these signals for all of the tools...
		std::tr1::shared_ptr<viewer::QtRegionSourceKernel> qrs = std::tr1::dynamic_pointer_cast<viewer::QtRegionSourceKernel>(pos->getRegionSource( )->kernel( ));

		if ( qrs ) {
			connect( profile_, SIGNAL(adjustPosition(double,double,double,double)),
					qrs.get(), SLOT(adjustPosition(double,double,double,double)));
			connect( qrs.get( ), SIGNAL( regionCreated( int, const QString &, const QString &, const QList<double> &,
					const QList<double> &, const QList<int> &, const QList<int> &,
					const QString &, const QString &, const QString &, int, int ) ),
					profile_, SLOT( newRegion( int, const QString &, const QString &, const QList<double> &,
							const QList<double> &, const QList<int> &, const QList<int> &,
							const QString &, const QString &, const QString &, int, int ) ) );
			connect( qrs.get( ), SIGNAL( regionUpdate( int, viewer::region::RegionChanges, const QList<double> &, const QList<double> &,
					const QList<int> &, const QList<int> & ) ),
					profile_, SLOT( updateRegion( int, viewer::region::RegionChanges, const QList<double> &, const QList<double> &,
							const QList<int> &, const QList<int> & ) ) );
			connect( qrs.get( ), SIGNAL( regionUpdateResponse( int, const QString &, const QString &, const QList<double> &,
					const QList<double> &, const QList<int> &, const QList<int> &,
					const QString &, const QString &, const QString &, int, int ) ),
					profile_, SLOT( newRegion( int, const QString &, const QString &, const QList<double> &,
							const QList<double> &, const QList<int> &, const QList<int> &,
							const QString &, const QString &, const QString &, int, int ) ) );

		}
	} else {
		std::tr1::shared_ptr<QtOldCrossTool> pos = std::tr1::dynamic_pointer_cast<QtOldCrossTool>(ppd->getTool(QtMouseToolNames::POINT));
		if ( pos.get( ) != 0 ) {
			connect( pos.get( ), SIGNAL(wcNotify( const String, const Vector<Double>, const Vector<Double>,
					const Vector<Double>, const Vector<Double>, const ProfileType)),
					profile_, SLOT(wcChanged( const String, const Vector<Double>, const Vector<Double>,
							const Vector<Double>, const Vector<Double>, const ProfileType)));
			connect( profile_, SIGNAL(coordinateChange(const String&)),
					pos.get( ), SLOT(setCoordType(const String&)));
		}
	}

	std::tr1::shared_ptr<QtRectTool> rect = std::tr1::dynamic_pointer_cast<QtRectTool>(ppd->getTool(QtMouseToolNames::RECTANGLE));
	if ( rect.get( ) != 0 ) {
		connect( rect.get( ), SIGNAL(wcNotify( const String, const Vector<Double>, const Vector<Double>,
				const Vector<Double>, const Vector<Double>, const ProfileType)),
				profile_, SLOT(wcChanged( const String, const Vector<Double>, const Vector<Double>,
						const Vector<Double>, const Vector<Double>, const ProfileType )));
		connect( profile_, SIGNAL(coordinateChange(const String&)),
				rect.get( ), SLOT(setCoordType(const String&)));
	} else {
		std::tr1::shared_ptr<QtOldRectTool> rect = std::tr1::dynamic_pointer_cast<QtOldRectTool>(ppd->getTool(QtMouseToolNames::RECTANGLE));
		if ( rect.get( ) != 0 ) {
			connect( rect.get( ), SIGNAL(wcNotify( const String, const Vector<Double>, const Vector<Double>,
					const Vector<Double>, const Vector<Double>, const ProfileType)),
					profile_, SLOT(wcChanged( const String, const Vector<Double>, const Vector<Double>,
							const Vector<Double>, const Vector<Double>, const ProfileType )));
			connect( profile_, SIGNAL(coordinateChange(const String&)),
					rect.get( ), SLOT(setCoordType(const String&)));
		}
	}

	std::tr1::shared_ptr<QtEllipseTool> ellipse = std::tr1::dynamic_pointer_cast<QtEllipseTool>(ppd->getTool(QtMouseToolNames::ELLIPSE));
	if ( ellipse.get( ) != 0 ) {
		connect( ellipse.get( ), SIGNAL(wcNotify( const String, const Vector<Double>, const Vector<Double>,
				const Vector<Double>, const Vector<Double>, const ProfileType )),
				profile_, SLOT(wcChanged( const String, const Vector<Double>, const Vector<Double>,
						const Vector<Double>, const Vector<Double>, const ProfileType )));
		connect( profile_, SIGNAL(coordinateChange(const String&)),
				ellipse.get( ), SLOT(setCoordType(const String&)));
	} else {
		std::tr1::shared_ptr<QtOldEllipseTool> ellipse = std::tr1::dynamic_pointer_cast<QtOldEllipseTool>(ppd->getTool(QtMouseToolNames::ELLIPSE));
		if ( ellipse.get( ) != 0 ) {
			connect( ellipse.get( ), SIGNAL(wcNotify( const String, const Vector<Double>, const Vector<Double>,
					const Vector<Double>, const Vector<Double>, const ProfileType )),
					profile_, SLOT(wcChanged( const String, const Vector<Double>, const Vector<Double>,
							const Vector<Double>, const Vector<Double>, const ProfileType )));
			connect( profile_, SIGNAL(coordinateChange(const String&)),
					ellipse.get( ), SLOT(setCoordType(const String&)));
		}
	}

	std::tr1::shared_ptr<QtPolyTool> poly = std::tr1::dynamic_pointer_cast<QtPolyTool>(ppd->getTool(QtMouseToolNames::POLYGON));
	if ( poly.get( ) != 0 ) {
		connect( poly.get( ), SIGNAL(wcNotify( const String, const Vector<Double>, const Vector<Double>,
				const Vector<Double>, const Vector<Double>, const ProfileType )),
				profile_, SLOT(wcChanged( const String, const Vector<Double>, const Vector<Double>,
						const Vector<Double>, const Vector<Double>, const ProfileType )));
		connect( profile_, SIGNAL(coordinateChange(const String&)),
				poly.get( ), SLOT(setCoordType(const String&)));
	} else {
		std::tr1::shared_ptr<QtOldPolyTool> poly = std::tr1::dynamic_pointer_cast<QtOldPolyTool>(ppd->getTool(QtMouseToolNames::POLYGON));
		if ( poly.get( ) != 0 ) {
			connect( poly.get( ), SIGNAL(wcNotify( const String, const Vector<Double>, const Vector<Double>,
					const Vector<Double>, const Vector<Double>, const ProfileType )),
					profile_, SLOT(wcChanged( const String, const Vector<Double>, const Vector<Double>,
							const Vector<Double>, const Vector<Double>, const ProfileType )));
			connect( profile_, SIGNAL(coordinateChange(const String&)),
					poly.get( ), SLOT(setCoordType(const String&)));
		}
	}
}


void QtDisplayPanelGui::hideImageProfile() {

	if(profile_) {
		profile_->hide();
		delete profile_;
		profile_ = 0;
	}
	profileDD_ = 0;

}


void QtDisplayPanelGui::refreshImageProfile() {
	if(profile_) {
		//List<QtDisplayData*> rdds = qdp_->registeredDDs();
		//if ( rdds.len() > 0 ) {
		if ( !qdp_->isEmptyRegistered()) {
			showImageProfile( );
			if ( profile_ ) profile_->redraw( );
		} else {
			profile_->hide();
			delete profile_;
			profile_ = 0;
			profileDD_ = 0;
		}
	}
}




// Internal slots and methods.


// Position Tracking


void QtDisplayPanelGui::arrangeTrackBoxes_() {
	trkgDockWidget_->arrangeTrackBoxes( );
}


void QtDisplayPanelGui::deleteTrackBox_(QtDisplayData* qdd) {
	trkgDockWidget_->removeTrackBox(qdd);
}




void QtDisplayPanelGui::displayTrackingData_(Record trackingRec) {
	trkgDockWidget_->display( trackingRec );
}


// Etc.

void QtDisplayPanelGui::close( ) {

	// shut down the window
	QtPanelBase::closeMainPanel();
}

void QtDisplayPanelGui::quit( ) {

	removeAllDDs();
	emit closed( this );
	delete imageManagerDialog;
	imageManagerDialog = NULL;
	if ( v_->server( ) ) {
		close( );
	} else {
		v_->quit( );
	}
}

// void QtDisplayPanelGui::toggleAnimExtras_() {

//   if(animAuxButton_->text()=="Full") animAuxButton_->setText("Compact");
//   else				     animAuxButton_->setText("Full");
//   setAnimExtrasVisibility_();  }



// void QtDisplayPanelGui::setAnimExtrasVisibility_() {

// //#dg  ...of failure of dockWidgets/areas to downsize when needed.
// // (also, of improper dependencies of szHints on event processing).
// // (Leave these until promised Qt fixes arrive...).

// /*  //#dg
// cerr<<"anMSzHb:"<<animWidget_->minimumSizeHint().width()
//           <<","<<animWidget_->minimumSizeHint().height()
//     <<"   SzHb:"<<animWidget_->sizeHint().width()
//           <<","<<animWidget_->sizeHint().height()<<endl;
// cerr<<"trMSzHb:"<<trkgWidget_->minimumSizeHint().width()
//           <<","<<trkgWidget_->minimumSizeHint().height()
//     <<"   SzHb:"<<trkgWidget_->sizeHint().width()
//           <<","<<trkgWidget_->sizeHint().height()<<endl<<endl;
// //*/  //#dg



//   if(animAuxButton_->text()=="Full") {
//     animAuxFrame_->hide(); modeGB_->hide();  }
//   else {
//     animAuxFrame_->show(); modeGB_->show();
//     animAuxButton_->setText("Compact");  }


/*  //#dg
	cerr<<"anMSzHa:"<<animWidget_->minimumSizeHint().width()
	          <<","<<animWidget_->minimumSizeHint().height()
	    <<"   SzHa:"<<animWidget_->sizeHint().width()
	          <<","<<animWidget_->sizeHint().height()<<endl;
	cerr<<"trMSzHa:"<<trkgWidget_->minimumSizeHint().width()
	          <<","<<trkgWidget_->minimumSizeHint().height()
	    <<"   SzHa:"<<trkgWidget_->sizeHint().width()
	          <<","<<trkgWidget_->sizeHint().height()<<endl<<endl;
	//*/  //#dg


/*  //#dg
	  repaint();    //#dg
	cerr<<"anMSzHr:"<<animWidget_->minimumSizeHint().width()
	          <<","<<animWidget_->minimumSizeHint().height()
	    <<"   SzHr:"<<animWidget_->sizeHint().width()
	          <<","<<animWidget_->sizeHint().height()<<endl;
	cerr<<"trMSzHr:"<<trkgWidget_->minimumSizeHint().width()
	          <<","<<trkgWidget_->minimumSizeHint().height()
	    <<"   SzHr:"<<trkgWidget_->sizeHint().width()
	          <<","<<trkgWidget_->sizeHint().height()<<endl<<endl;
	//*/  //#dg


/*  //#dg
	  update();    //#dg
	cerr<<"anMSzHu:"<<animWidget_->minimumSizeHint().width()
	          <<","<<animWidget_->minimumSizeHint().height()
	    <<"   SzHu:"<<animWidget_->sizeHint().width()
	          <<","<<animWidget_->sizeHint().height()<<endl;
	cerr<<"trMSzHu:"<<trkgWidget_->minimumSizeHint().width()
	          <<","<<trkgWidget_->minimumSizeHint().height()
	    <<"   SzHu:"<<trkgWidget_->sizeHint().width()
	          <<","<<trkgWidget_->sizeHint().height()<<endl<<endl;
	//*/  //#dg


/*  //#dg
	  QtApp::app()->processEvents();  //#dg
		// (NB: anSzH's don't usually decrease until this point(?!)...)
	cerr<<"anMSzHp:"<<animWidget_->minimumSizeHint().width()
	          <<","<<animWidget_->minimumSizeHint().height()
	    <<"   SzHp:"<<animWidget_->sizeHint().width()
	          <<","<<animWidget_->sizeHint().height()<<endl;
	cerr<<"trMSzHp:"<<trkgWidget_->minimumSizeHint().width()
	          <<","<<trkgWidget_->minimumSizeHint().height()
	    <<"   SzHp:"<<trkgWidget_->sizeHint().width()
	          <<","<<trkgWidget_->sizeHint().height()<<endl<<endl;
	//*/  //#dg


// }




// Save-Restore methods.



void QtDisplayPanelGui::savePanelState_() {
	// Brings up dialog for saving display panel state: reg'd DDs, their
	// options, etc.  Triggered by dpSaveAct_.

	QString         savedir = selectedDMDir.chars();
	if(savedir=="") savedir = QDir::currentPath();

	QString suggpath = savedir + "/" + qdp_->suggestedRestoreFilename().chars();

	String filename = QFileDialog::getSaveFileName(
			this, "Save State As", suggpath,
			( "viewer restore files (*."
					+ v_->cvRestoreFileExt
					+ ");;all files (*.*)" ).chars() ).toStdString();

	if(filename=="") return;

	if(!qdp_->savePanelState(filename)) {

		cerr<<endl<<"**Unable to save "<<filename<<endl
				<<"  (check permissions)**"<<endl<<endl;
	}

	syncDataDir_(filename);
}
// Keeps current data directory in sync between
// DataManager window and save-restore dialogs.




void QtDisplayPanelGui::restorePanelState_() {
	// Brings up dialog for restore file, attempts restore.
	// Triggered by dpRstrAct_.

	QString            restoredir = selectedDMDir.chars();
	if(restoredir=="") restoredir = QDir::currentPath();

	String filename = QFileDialog::getOpenFileName(
			this, "Select File for Viewer Restore", restoredir,
			( "viewer restore files (*."
					+v_->cvRestoreFileExt
					+");;all files (*.*)" ).chars() ).toStdString();

	if(filename=="") return;

	syncDataDir_(filename);
	// Keeps current data directory in sync between
	// DataManager window and save-restore dialogs.

	restorePanelState(filename);
}




Bool QtDisplayPanelGui::restorePanelState(String filename) {
	// (Attempts to) restore panel state from named file (or from
	// 'clipboard' if filename=="").

	return qdp_->restorePanelState(filename);
}




Bool QtDisplayPanelGui::syncDataDir_(String filename) {
	// Keeps current data directory in sync between
	// DataManager window and save-restore dialogs.

	QDir datadir = QFileInfo(filename.chars()).dir();
	if(!datadir.exists()) return False;

	QString datadirname = datadir.path();

	if(dataMgr()!=0) dataMgr()->updateDirectory(datadirname.toStdString( ));
	else selectedDMDir = datadirname.toStdString();
	return True;
}



void QtDisplayPanelGui::addGuiState_(QDomDocument* restoredoc) {
	// Responds to qdp_->creatingRstrDoc(QDomDocument*) signal.
	// (Recall that qdp_ is unaware of this gui).
	// Adds gui state to the QDomDocument qdp_ has created.

	QDomElement restoreElem =
			restoredoc->firstChildElement(v_->cvRestoreID.chars());
	if(restoreElem.isNull()) return;  // invalid rstr doc (shouldn't happen).

	QDomElement dpgSettings = restoredoc->createElement("gui-settings");
	restoreElem.appendChild(dpgSettings);


	// QMainWindow settings (dock status of toolbars and dockwidgets, etc.)

	QDomElement dockSettings = restoredoc->createElement("dock-settings");
	dpgSettings.appendChild(dockSettings);
	QByteArray docksettings = saveState().toBase64();

	dockSettings.setAttribute("data", docksettings.constData());


	// Overall window size.

	QDomElement winsize = restoredoc->createElement("window-size");
	dpgSettings.appendChild(winsize);

	winsize.setAttribute("width",  size().width());
	winsize.setAttribute("height", size().height());

	// RegionShapes
	qsm_->saveState(*restoredoc);
}




void QtDisplayPanelGui::restoreGuiState_(QDomDocument* restoredoc) {
	// Responds to qdp_->creatingRstrDoc(QDomDocument*) signal.
	// Sets gui-specific state (most notably, overall window size).

	QDomElement dpgSettings =
			restoredoc->firstChildElement(v_->cvRestoreID.chars())
			.firstChildElement("gui-settings");

	if(dpgSettings.isNull()) return;


	// QMainWindow settings (dock status of toolbars and dockwidgets, etc.)

	QDomElement dockSettings = dpgSettings.firstChildElement("dock-settings");
	QString dockstring = dockSettings.attribute("data");
	if(dockstring!="") {
		QByteArray docksettings;		// (Why is it so damn hard to convert
		docksettings.append(dockstring);	// between ascii QByteArrays and
		// QStrings?...)
		restoreState(QByteArray::fromBase64(docksettings));
	}


	// Overall window size

	QDomElement winsize = dpgSettings.firstChildElement("window-size");
	QString wd = winsize.attribute("width", "#"),
			ht = winsize.attribute("height", "#");
	Bool w_ok = False, h_ok = False;
	Int w = wd.toInt(&w_ok);
	Int h = ht.toInt(&h_ok);

	if(h_ok && w_ok) resize(w, h);


	// Other items (anim full/compact, chkboxes, trkgfontsize, etc...).

	// (to implement).


	// RegionShapes state

	qsm_->restoreState(*restoredoc);
}






void QtDisplayPanelGui::registerAllDDs() {
	DisplayDataHolder::DisplayDataIterator iter = displayDataHolder->beginDD();
	while ( iter != displayDataHolder->endDD()) {
		qdp_->registerDD( (*iter));
		iter++;
	}
	updateFrameInformation();
}

void QtDisplayPanelGui::unregisterAllDDs() {
	qdp_->unregisterAll();
	if ( qdm_ != NULL ){
		qdm_->clearRegionMap();
	}
	if ( regionDock_ != NULL ){
		regionDock_->delete_all_regions( true );

	}
	updateFrameInformation();
}


void QtDisplayPanelGui::closeEvent(QCloseEvent *event) {

	// save viewer dimensions
	QSize ending_size(size());
	char buf[256];
	sprintf( buf, "%d %d", ending_size.width(), ending_size.height() );
	rc.put( "viewer." + rcid() + ".dimensions", buf );

#if QT_VERSION >= 0x040500
	QList<QDockWidget*> tabbeddocks = tabifiedDockWidgets( trkgDockWidget_ );

	int rcnt = 0, lcnt = 0, tcnt = 0, bcnt = 0;
	for ( QList<QDockWidget*>::const_iterator i = tabbeddocks.begin(); i !=  tabbeddocks.end(); ++i ) {
		if ( dockWidgetArea(*i) == Qt::RightDockWidgetArea ) {
			rc.put( "viewer." + rcid() + ".rightdock", "tabbed" );
			++rcnt;
		}
		if ( dockWidgetArea(*i) == Qt::BottomDockWidgetArea ) {
			rc.put( "viewer." + rcid() + ".bottomdock", "tabbed" );
			++bcnt;
		}
		if ( dockWidgetArea(*i) == Qt::LeftDockWidgetArea ) {
			rc.put( "viewer." + rcid() + ".leftdock", "tabbed" );
			++lcnt;
		}
		if ( dockWidgetArea(*i) == Qt::TopDockWidgetArea ) {
			rc.put( "viewer." + rcid() + ".topdock", "tabbed" );
			++tcnt;
		}
	}

	if ( trkgDockWidget_->isVisible( ) && animationHolder->isVisible( ) &&
			tabbeddocks.indexOf(trkgDockWidget_) < 0 && tabbeddocks.indexOf(animationHolder) < 0 ) {
		// ... both right/bottom dock widgets are available & untabbed
		// ... if they are both on the same dock location it means that location is untabbed
		if ( dockWidgetArea(trkgDockWidget_) == Qt::RightDockWidgetArea &&
				dockWidgetArea(animationHolder) == Qt::RightDockWidgetArea ) {
			rc.put( "viewer." + rcid() + ".rightdock", "untabbed" );
		}
		if ( dockWidgetArea(trkgDockWidget_) == Qt::BottomDockWidgetArea &&
				dockWidgetArea(animationHolder) == Qt::BottomDockWidgetArea ) {
			rc.put( "viewer." + rcid() + ".bottomdock", "untabbed" );
		}
		if ( dockWidgetArea(trkgDockWidget_) == Qt::LeftDockWidgetArea &&
				dockWidgetArea(animationHolder) == Qt::LeftDockWidgetArea ) {
			rc.put( "viewer." + rcid() + ".leftdock", "untabbed" );
		}
		if ( dockWidgetArea(trkgDockWidget_) == Qt::TopDockWidgetArea &&
				dockWidgetArea(animationHolder) == Qt::TopDockWidgetArea ) {
			rc.put( "viewer." + rcid() + ".topdock", "untabbed" );
		}
	}
#endif

	delete imageManagerDialog;
	imageManagerDialog = NULL;

	QtPanelBase::closeEvent(event);
}

// Slots to respond to registration/close menu clicks.


void QtDisplayPanelGui::ddRegClicked_() {

	// Retrieve the dd associated with the signal.

	QAction* action = dynamic_cast<QAction*>(sender());
	if(action==0) return;		// (shouldn't happen).
	QtDisplayData* dd = action->data().value<QtDisplayData*>();
	registerDD( dd, -1 );
}

void QtDisplayPanelGui::registerDD( QtDisplayData* dd, int position ){
	qdp_->registerDD(dd, position );
	updateFrameInformation();
	if ( regionDock_ ){
		regionDock_->updateRegionStats( );
	}
	profiledImageChange();
}

void QtDisplayPanelGui::profiledImageChange(){
	//We may have a different image that is being channeled.
	if ( profile_ != NULL && profile_->isVisible() ){
		showImageProfile();
	}
}


void QtDisplayPanelGui::ddUnregClicked_() {
	QAction* action = dynamic_cast<QAction*>(sender());
	if(action==0) return;		// (shouldn't happen).
	QtDisplayData* dd = action->data().value<QtDisplayData*>();
	unregisterDD( dd );
}

void QtDisplayPanelGui::unregisterDD( QtDisplayData* dd ){
	qdp_->unregisterDD(dd);
	updateFrameInformation();
	if ( regionDock_ ){
		regionDock_->updateRegionStats( );
	}
	profiledImageChange();
}

void QtDisplayPanelGui::ddCloseClicked_() {
	QAction* action = dynamic_cast<QAction*>(sender());
	if(action==0) return;		// (shouldn't happen).
	QtDisplayData* dd = action->data().value<QtDisplayData*>();

	ddClose( dd );
}

void QtDisplayPanelGui::ddClose(QtDisplayData*& ddRemove ) {
	if ( ddRemove != NULL ) {
		emit removeDD(ddRemove);
	}
}

void QtDisplayPanelGui::ddOpen( const String& path, const String& dataType,
		const String& displayType, int insertPosition,
		bool registerImage, bool masterCoordinate, bool masterSaturation,
		bool masterHue) {
	createDD( path, dataType, displayType, registerImage, insertPosition,
			masterCoordinate, masterSaturation, masterHue);
}

void QtDisplayPanelGui::unlinkCursorTracking( QtDisplayPanelGui *other ) {
	linkedCursorHandler->removeSource(other);
}

void QtDisplayPanelGui::linkCursorTracking( QtDisplayPanelGui *other, QColor color ) {
	linkedCursorHandler->addSource(other,color);
}


void QtDisplayPanelGui::setColorBarOrientation(Bool vertical) {

	// At least for now, colorbars can only be placed horizontally or vertically,
	// identically for all display panels.  This sets that state for everyone.
	// Sends out colorBarOrientationChange signal when the state changes.

	if(colorBarsVertical_ == vertical) return;	// (already there).

	colorBarsVertical_ = vertical;

	// Tell QDPs and QDDs to rearrange color bars as necessary.

	v_->hold();	// (avoid unnecessary extra refreshes).
	emit colorBarOrientationChange();
	v_->release();
}



void QtDisplayPanelGui::replaceControllingDD( QtDisplayData* oldControllingDD, QtDisplayData* newControllingDD) {
	if ( newControllingDD != oldControllingDD ) {
		if ( oldControllingDD != NULL ) {

			disconnect( oldControllingDD, SIGNAL(axisChanged(String, String, String, std::vector<int>)),
					this, SLOT(controlling_dd_axis_change(String, String, String, std::vector<int> )) );
		}

		//Set the new controlling DD in the layers below this one.

		//qdp_->setControllingDD( newControllingDD );


		emit axisToolUpdate( newControllingDD );


		if ( newControllingDD != 0 ) {
			connect( newControllingDD, SIGNAL(axisChanged(String, String, String, std::vector<int>)),
					SLOT(controlling_dd_axis_change(String, String, String, std::vector<int> )) );
		}
	}
}


void QtDisplayPanelGui::controlling_dd_axis_change(String /*firstStr*/, String /*secondStr*/,
		String thirdStr, std::vector<int> /*axes*/) {
	if ( thirdStr.length() > 0 && animationHolder != NULL ){
		animationHolder->setChannelZAxis( thirdStr.c_str() );
	}
}

void QtDisplayPanelGui::showMomentsCollapseImageProfile() {
	showImageProfile();
	if ( profile_ != NULL ) {
		profile_->setPurpose(ProfileTaskMonitor::MOMENTS_COLLAPSE);
	}
}


void QtDisplayPanelGui::showSpecFitImageProfile() {
	showImageProfile();
	if ( profile_ != NULL ) {
		profile_->setPurpose(ProfileTaskMonitor::SPECTROSCOPY);
	}
}

void QtDisplayPanelGui::addSlice( int id, const QString& shape, const QString&, const QList<double>& worldX,
		const QList<double>& worldY, const QList<int>& pixelX, const QList<int>& pixelY,
		const QString& lineColor, const QString&, const QString&, int, int) {
	if ( shape == "polyline") {
		sliceTool->addPolyLine( id, viewer::region::RegionChangeCreate,
				worldX, worldY, pixelX, pixelY, lineColor );
		updateSliceCorners( id, worldX, worldY );
	}
}

void QtDisplayPanelGui::updateSliceCorners( int id, const QList<double>& worldX,
		const QList<double>& worldY ) {
	QtDisplayData* controllingDD = dd();
	if ( controllingDD != NULL && sliceTool != NULL ) {
		int cornerCount = worldX.size();
		QVector<String> positionInformation(cornerCount);
		for ( int i = 0; i < cornerCount; i++ ) {
			Vector<double> worldCoords(2);
			worldCoords[0] = worldX[i];
			worldCoords[1] = worldY[i];
			positionInformation[i] = controllingDD->getPositionInformation( worldCoords );
		}
		sliceTool->updatePositionInformation( id, positionInformation );
	}
}

void QtDisplayPanelGui::sliceMarkerPositionChanged(int regionId,int segmentIndex,float percentage) {
	if ( sliceTool != NULL ) {
		viewer::Region* region = findRegion( regionId );
		viewer::region::RegionTypes defaultType = viewer::region::PolylineRegion;
		if ( region != NULL ) {
			viewer::region::RegionTypes regionType = region->type();
			if ( regionType == defaultType ) {
				viewer::Polyline* polyline = dynamic_cast<viewer::Polyline*>(region);
				polyline->setMarkerPosition( regionId, segmentIndex,percentage);
			}
		}
	}
}

void QtDisplayPanelGui::sliceMarkerVisibilityChanged(int regionId,bool showMarker) {
	if ( sliceTool != NULL ) {
		viewer::Region* region = findRegion( regionId );
		viewer::region::RegionTypes defaultType = viewer::region::PolylineRegion;
		if ( region != NULL ) {
			viewer::region::RegionTypes regionType = region->type();
			if ( regionType == defaultType ) {
				viewer::Polyline* polyline = dynamic_cast<viewer::Polyline*>(region);
				polyline->setShowMarkerPosition( regionId, showMarker);
			}
		}
	}
}

void QtDisplayPanelGui::sliceChanged( int regionId, viewer::region::RegionChanges change,
		const QList<double> & worldX, const QList<double> & worldY,
		const QList<int> &pixelX, const QList<int> & pixelY ) {
	if ( sliceTool != NULL ) {
		viewer::Region* region = findRegion( regionId );
		viewer::region::RegionTypes defaultType = viewer::region::PolylineRegion;
		if ( region != NULL ) {
			viewer::region::RegionTypes regionType = region->type();
			if ( regionType == defaultType ) {
				if ( change == viewer::region::RegionChangeModified ) {
					std::string polyColor = region->lineColor();
					QString qPolyColor( polyColor.c_str());
					sliceTool->setCurveColor( regionId, qPolyColor );
				} else if ( change == viewer::region::RegionChangeNewChannel ) {
					int channelIndex = region->zIndex();
					sliceTool->updateChannel( channelIndex );
					sliceTool->updatePolyLine( regionId, change, worldX, worldY, pixelX, pixelY );
				} else if ( change == viewer::region::RegionChangeSelected ) {
					bool marked= region->marked();
					sliceTool->setRegionSelected( regionId, marked );
					sliceTool->updatePolyLine( regionId, change, worldX, worldY, pixelX, pixelY );
					updateSliceCorners( regionId, worldX, worldY );
				} else {
					sliceTool->updatePolyLine( regionId, change, worldX, worldY, pixelX, pixelY );
					updateSliceCorners( regionId, worldX, worldY);
				}
			}
		} else if ( change == viewer::region::RegionChangeDelete ) {
			sliceTool->updatePolyLine( regionId, change, worldX, worldY, pixelX, pixelY );
		}
	}
}



void QtDisplayPanelGui::showSlicer() {
	if ( sliceTool == NULL ) {
		sliceTool = new SlicerMainWindow( this );

		//Image updates
		connect( qdp_, SIGNAL(registrationChange()), this, SLOT(resetListenerImage()), Qt::UniqueConnection );
		resetListenerImage();

		//Update the polyline with the new slice position
		connect(sliceTool, SIGNAL(markerPositionChanged(int,int,float)), this, SLOT(sliceMarkerPositionChanged(int,int,float)));;
		connect(sliceTool, SIGNAL(markerVisibilityChanged(int,bool)), this, SLOT(sliceMarkerVisibilityChanged(int,bool)));

		//Region updates
		PanelDisplay* panelDisplay = qdp_->panelDisplay();
		std::tr1::shared_ptr<QtPolylineTool> pos = std::tr1::dynamic_pointer_cast<QtPolylineTool>(panelDisplay->getTool(QtMouseToolNames::POLYLINE));
		if (pos) {
			std::tr1::shared_ptr<viewer::QtRegionSourceKernel> qrs = std::tr1::dynamic_pointer_cast<viewer::QtRegionSourceKernel>(pos->getRegionSource( )->kernel( ));
			if ( qrs ) {
				connect( qrs.get( ), SIGNAL( regionUpdate( int, viewer::region::RegionChanges, const QList<double> &, const QList<double> &,
						const QList<int> &, const QList<int> & ) ),
						this, SLOT( sliceChanged( int, viewer::region::RegionChanges, const QList<double>&, const QList<double>&,
								const QList<int>&, const QList<int> &) ));
				//So that the slicer knows about regions that were generated
				//before it was created.
				connect( qrs.get(), SIGNAL(regionUpdateResponse( int, const QString &, const QString &,
						const QList<double> &, const QList<double> &, const QList<int> &, const QList<int> &,
						const QString &, const QString &, const QString &, int, int)),
						this, SLOT(addSlice( int, const QString&, const QString&, const QList<double>&,
								const QList<double>&, const QList<int>&, const QList<int>&,
								const QString&, const QString&, const QString&, int, int)));
				qrs->generateExistingRegionUpdates();

			}
		}


	}
	sliceTool->show();
}

void QtDisplayPanelGui::updateDDMenus_(Bool /*doCloseMenu*/) {

		ddCloseMenu_->clear();
		int count = displayDataHolder->getCount();
		Bool manydds = false;
		if ( count > 1 ) {
			manydds = true;
		}

		QAction* action = 0;

		// The following allows slots to distinguish the dd associated with
		// triggered actions (Qt actions and signals are somewhat deficient in
		// their ability to make distinctions of this sort, imo).
		// Also note the macro at the end of QtDisplayData.qo.h, which enables
		// QtDisplayData* to be a QVariant's value.
		QVariant ddv;		// QVariant wrapper for a QtDisplayData pointer.

		DisplayDataHolder::DisplayDataIterator iter = displayDataHolder->beginDD();
		while ( iter != displayDataHolder->endDD()) {

			QtDisplayData* dd = (*iter);
			iter++;
			ddv.setValue(dd);


			// 'Close' menu item.

			action = new QAction( ("Close "+dd->name()).c_str(), ddCloseMenu_ );
			action->setData(ddv);
			ddCloseMenu_->addAction(action);
			connect(action, SIGNAL(triggered()), SLOT(ddCloseClicked_()));

		}

		// '[Un]Register All' / 'Close All'  menu items.
		if(manydds) {
			ddCloseMenu_->addSeparator();

			action = new QAction("Close All", ddCloseMenu_);
			ddCloseMenu_->addAction(action);
			connect(action, SIGNAL(triggered()), SLOT(removeAllDDs()));
		}
	}


void QtDisplayPanelGui::showImageManager() {
	if ( imageManagerDialog == NULL ) {
		imageManagerDialog = new ImageManagerDialog( NULL );
		imageManagerDialog->setImageHolders( qdp_->getDataHolder(), displayDataHolder );
		connect( imageManagerDialog, SIGNAL(ddClosed(QtDisplayData*&)),
				this, SLOT(ddClose(QtDisplayData*&)));
		connect( imageManagerDialog, SIGNAL(ddOpened(const String&, const String&, const String&, int, bool, bool, bool, bool)),
				this, SLOT(ddOpen(const String&, const String&, const String&, int, bool, bool, bool, bool)));
		connect( imageManagerDialog, SIGNAL(registerAll()),
				this, SLOT(registerAllDDs()));
		connect( imageManagerDialog, SIGNAL(unregisterAll()),
				this, SLOT(unregisterAllDDs()));
		connect( imageManagerDialog, SIGNAL(registerDD(QtDisplayData*, int)),
				this, SLOT(registerDD(QtDisplayData*, int)));
		connect( imageManagerDialog, SIGNAL(unregisterDD(QtDisplayData*)),
				this, SLOT(unregisterDD(QtDisplayData*)));
		connect( imageManagerDialog, SIGNAL(masterCoordinateChanged(QtDisplayData*, QtDisplayData*)),
				this, SLOT(replaceControllingDD(QtDisplayData*, QtDisplayData*)));
		connect( imageManagerDialog, SIGNAL(animateToImage(int)),
				this, SLOT(setAnimatedImage(int)));
		connect( imageManagerDialog, SIGNAL(createRGBImage(QtDisplayData*, QtDisplayData*, QtDisplayData*, QtDisplayData*)),
				this, SLOT(createRGBImage(QtDisplayData*,QtDisplayData*,QtDisplayData*,QtDisplayData*)));
		updateViewedImage();

	}
	imageManagerDialog->showNormal();
	imageManagerDialog->raise();
}
}
