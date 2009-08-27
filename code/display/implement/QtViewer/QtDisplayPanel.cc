//# QtDisplayPanel.cc: Qt implementation of viewer display Widget.
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

#include <casa/BasicSL/String.h>
#include <display/QtViewer/QtViewerBase.qo.h>
#include <display/QtViewer/QtDisplayPanel.qo.h>
#include <display/QtViewer/QtDisplayData.qo.h>
#include <display/DisplayDatas/DisplayData.h>
#include <display/DisplayEvents/PCITFiddler.h>
#include <display/DisplayEvents/MWCPTRegion.h>
#include <display/DisplayEvents/MWCPolylineTool.h>
#include <display/DisplayEvents/MWCCrosshairTool.h>
#include <display/DisplayEvents/MWCPannerTool.h>
#include <display/Display/AttributeBuffer.h>
#include <display/Display/WorldCanvas.h>
#include <display/QtViewer/QtMouseToolState.qo.h>
#include <display/DisplayDatas/WedgeDD.h>
#include <casa/BasicMath/Math.h>
#include <display/Display/DParameterChoice.h>
#include <casa/IO/AipsIO.h>
#include <tables/Tables/TableRecord.h>
#include <display/QtAutoGui/QtXmlRecord.h>
#include <display/RegionShapes/RegionShape.h>
#include <display/RegionShapes/RegionShapes.h>
#include <display/RegionShapes/QtRegionShapeManager.qo.h>
#include <images/Regions/RegionManager.h>
#include <images/Regions/RegionHandler.h>
#include <images/Images/ImageInterface.h>

namespace casa { //# NAMESPACE CASA - BEGIN


QtDisplayPanel::QtDisplayPanel(QtViewerBase* v, QWidget *parent) : 
		QWidget(parent),

		v_(v),
		pd_(0), pc_(0),
		qdds_(),
		zoom_(0), panner_(0), crosshair_(0), rtregion_(0),
		ptregion_(0), polyline_(0),  snsFidd_(0), bncFidd_(0),
		mouseToolNames_(),
		tracking_(True),
		modeZ_(True),
		zLen_(1), bLen_(1),
		zIndex_(0), bIndex_(0),
		zStart_(0), zEnd_(1), zStep_(1),
		bStart_(0), bEnd_(1), bStep_(1),
		animRate_(10), minRate_(1), maxRate_(50), animating_(0),
		blankCBPanel_(0), mainPanelSize_(1.),
		hasRgn_(False), rgnExtent_(0), qsm_(0),
		lastMotionEvent_(0), bkgdClrOpt_(0), printStats(True),
                extChan_(""), extPol_("")  {
    
  setWindowTitle("Viewer Display Panel");
  
  //pc_  = new QtPixelCanvas(this);
  pc_ = new QtPixelCanvas();
    
  // QDP's own widget just contains the pc_.
  
  QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget(pc_);
  setLayout(layout);
  
  pd_ = new PanelDisplay(pc_,1,1);	// (PD default is 3 by 2...)
 
  // Increase margins...
  
  Record margins, chgdopts;
  margins.define("leftmarginspacepg", 12);
  margins.define("bottommarginspacepg", 9);
  pd_->setOptions(margins, chgdopts);

  
  setupMouseTools_();
 

  // (or parsing background color user selection via
  // getOptions() / setOptions() / Options gui.
  Vector<String> blkWht(2);   blkWht[0]="black"; blkWht[1]="white";
  bkgdClrOpt_ = new DParameterChoice("bkgdclr", "Background Color",
                "Color assigned to 'background' in other color choices.\n"
		"Affects esp. margins surrounding the image.  Black\n"
		"background implies white 'foreground' (default for\n"
		"lettering, etc.), and vice versa.  White background is\n"
		"usually the preferred setting before printing.",
                 blkWht, "black", "black", "");

    
  
  connect( v_, SIGNAL(ddCreated(QtDisplayData*, Bool)),
                 SLOT(ddCreated_(QtDisplayData*, Bool)) );
  
  connect( v_, SIGNAL(ddRemoved(QtDisplayData*)),
                 SLOT(ddRemoved_(QtDisplayData*)) );

  connect( v_, SIGNAL(colorBarOrientationChange()),
                 SLOT(reorientColorBars_()) );

  
  // Connect detailed to general registration change signal.
  
  connect(this, SIGNAL(oldDDRegistered(QtDisplayData*)),
                SIGNAL(registrationChange()));
  
  connect(this, SIGNAL(oldDDUnregistered(QtDisplayData*)),
                SIGNAL(registrationChange()));
  
  connect(this, SIGNAL(newRegisteredDD(QtDisplayData*)),
                SIGNAL(registrationChange()));
  
  connect(this, SIGNAL(newUnregisteredDD(QtDisplayData*)),
                SIGNAL(registrationChange()));
  
  connect(this, SIGNAL(RegisteredDDRemoved(QtDisplayData*)),
                SIGNAL(registrationChange()));
  
  connect(this, SIGNAL(UnregisteredDDRemoved(QtDisplayData*)),
                SIGNAL(registrationChange()));
  
  connect(this, SIGNAL(allDDsRegistered()),   SIGNAL(registrationChange()));
  
  connect(this, SIGNAL(allDDsUnregistered()), SIGNAL(registrationChange()));


  // connect(this, SIGNAL(registrationChange()), SLOT(resetTools()));
  //#dk (1/08: too strong; tools now reset only when CS master DD unreg'd).

  connect(this, SIGNAL(registrationChange()), SLOT(checkColorBars_()));

  connect(pc_, SIGNAL(resizing(QResizeEvent*)), SLOT(pcResizing_()));


  // Animation
  
  connect(&tmr_, SIGNAL(timeout()),  SLOT(playStep_()));
  setRate(animRate_);


  // Old-style 'connections' (registry of callbacks) with underlying canvases.
  installEventHandlers_();

  setFocusProxy(pc_);	// Shifts display panel focus to the pc.
  
  v_->dpCreated(this);	// Inform supervisory object of DP creation.

}



QtDisplayPanel::~QtDisplayPanel() { 
  removeEventHandlers_();
  unregisterAll();	// Also removes/deletes color bar panels.
  
  delete pd_;		// Also deletes MWCTools that were added to it...
  
  delete snsFidd_;	// (not the case with PCTools...)
  delete bncFidd_;
  delete bkgdClrOpt_;
  if(lastMotionEvent_!=0) delete lastMotionEvent_;
  delete pc_;  }


  

// MOUSE TOOL METHODS / EVENT HANDLERS.  POSITION TRACKING
    

void QtDisplayPanel::setupMouseTools_() {
 
  using namespace QtMouseToolNames;	// (See QtMouseToolState.qo.h)
  
  mouseToolNames_.resize(8);
  mouseToolNames_[0] = ZOOM;
  mouseToolNames_[1] = PAN;
  mouseToolNames_[2] = SHIFTSLOPE;
  mouseToolNames_[3] = BRIGHTCONTRAST;
  mouseToolNames_[4] = POSITION;
  mouseToolNames_[5] = RECTANGLE;
  mouseToolNames_[6] = POLYGON;
  mouseToolNames_[7] = POLYLINE;
	// The canonical text-names of the mouse tools on this panel.
	// These happen to be in QtMouseToolNames::toolIndex order,
	// but that is not a requirement.  This order is returned by
	// mouseToolNames() as a suggestion for the order on a gui which
	// would operate the mouse tools on this type of panel.


  // Create the actual mouse tools.
  
  zoom_      = new MWCRTZoomer;       pd_->addTool(ZOOM, zoom_);
  panner_    = new MWCPannerTool;     pd_->addTool(PAN, panner_);
  //crosshair_ = new MWCCrosshairTool;  pd_->addTool(POSITION, crosshair_);
  crosshair_ = new QtCrossTool;  pd_->addTool(POSITION, crosshair_);
  //ptregion_  = new MWCPTRegion;       pd_->addTool(POLYGON, ptregion_);
  ptregion_  = new QtPolyTool(pd_);   pd_->addTool(POLYGON, ptregion_);
  //rtregion_  = new QtRTRegion(pd_);   pd_->addTool(RECTANGLE, rtregion_);
  rtregion_  = new QtRectTool(pd_);   pd_->addTool(RECTANGLE, rtregion_);

  polyline_  = new MWCPolylineTool;   pd_->addTool(POLYLINE, polyline_);
  
  snsFidd_ = new PCITFiddler(pc_, PCITFiddler::StretchAndShift,
				  Display::K_None);
  bncFidd_ = new PCITFiddler(pc_, PCITFiddler::BrightnessAndContrast,
                                  Display::K_None);
	// NB: The above two 'colormap fiddling tools' are 'PCTools';
	// they should really share a common (DisplayTool) base with the
	// others (which are MWCTools), but do not, at present.
	// PCTools are attached to the PixelCanvas and treat it as a
	// whole, whereas MWCTools are attached to the PanelDisplay
	// and are sensitive to the individual WC they're operating
	// over within the PC.

  
  connect( rtregion_, SIGNAL(mouseRegionReady(Record, WorldCanvasHolder*)),
		        SLOT(mouseRegionReady_(Record, WorldCanvasHolder*)) );
  
  connect( ptregion_, SIGNAL(mouseRegionReady(Record, WorldCanvasHolder*)),
		        SLOT(mouseRegionReady_(Record, WorldCanvasHolder*)) );
  
  connect( rtregion_, SIGNAL(echoClicked(Record)),
		        SLOT(clicked(Record)) );
  
  connect( ptregion_, SIGNAL(echoClicked(Record)),
		        SLOT(clicked(Record)) );
  
  QtMouseToolState* mBtns = v_->mouseBtns();
	// Central storage for current active mouse button of each tool.
  
  connect( mBtns, SIGNAL(mouseBtnChg(String, Int)),
                    SLOT(chgMouseBtn_(String, Int)) );
    
  mBtns->emitBtns();  }
	// (Causes mBtns to communicate current mouse button settings
	//  to the actual mouse tools (above), via the connection above).  



void QtDisplayPanel::chgMouseBtn_(String tool, Int button) {
  // Command to set/change the button currently assigned to a mouse tool.
  // The central place for this information is QtMouseToolState, which
  // invokes this routine.  This sets the active button onto the internal
  // (non-Qt) display library tool.
  //
  // button              Corresp. internal library value
  // ---------------     -------------------------------
  // 0:  <no button>     Display::K_None  
  // 1:  LeftButton      Display::K_Pointer_Button1
  // 2:  MidButton       Display::K_Pointer_Button2
  // 3:  RightButton     Display::K_Pointer_Button3
  
  if(button<0 || button>=4) return;	// (safety; shouldn't happen). 
  
  static const Display::KeySym dlBtns[4] = {
    Display::K_None, 
    Display::K_Pointer_Button1,
    Display::K_Pointer_Button2,
    Display::K_Pointer_Button3 };
  
  Display::KeySym dlbtn=dlBtns[button];
  
  using namespace QtMouseToolNames;	// (See QtMouseToolState.qo.h).
  
  if      (tool == SHIFTSLOPE)     snsFidd_->setKey(dlbtn);
  else if (tool == BRIGHTCONTRAST) bncFidd_->setKey(dlbtn);
  else                             pd_->setToolKey(tool, dlbtn);  }

 
   
  
void QtDisplayPanel::mouseRegionReady_(Record mouseRegion, 
                                       WorldCanvasHolder* wch) {
  // Connected to corresp. signals from 'region' mouse tools.  Emits that
  // signal verbatum, but also processes it through the registered DDs
  // and receives higher-level 'Image Regions' from those DDs which can
  // create one from the 'mouse region' record.  
  // At present it does two things with those returned ImageRegions:
  // 1) Prints image statistics for the regions.
  // 2) Saves the first-returned region internally (as  lastRgn_) and
  //    emits the newRegion(imgFilename) signal.  lastRgn_ can be
  //    serialized and saved as a file with saveLastRegion().

  emit mouseRegionReady(mouseRegion, wch);  // echo mouseTool signal.
  
  Bool rgnSaved = False;
    
  for(ListIter<QtDisplayData*> qdds(qdds_); 
        !qdds.atEnd(); qdds++) {
    QtDisplayData* qdd = qdds.getRight();

    //cout << "extChan_=" << extChan_
    //     << " extPol_=" << extPol_ << endl;
    ImageRegion* imReg = qdd->mouseToImageRegion(
        mouseRegion, wch, extChan_, extPol_);

    //ImageRegion* imReg = qdd->mouseToImageRegion(
    //    mouseRegion, wch, rgnExtent_>0, rgnExtent_>1);
	// rgnExtent_ controls region extent on non-display axes.
	// (N.B.: We're responsible for deleting imReg).
        
	//(Disabled -- for now, let mouseRegionReady connectors do their
        //own qdd->mouseToImageRegion()s (with allChannels/Axes as desired).
        // if(imReg!=0) emit imageRegionReady(*imReg, qdd->name());
	    // (*imReg is passed to slots by value (copied), so the
	    // delete below should safe (untested)).
    
    if(imReg==0) continue;
     
    if(printStats) qdd->printRegionStats(*imReg);	// stats.
    
    
    //if(!rgnSaved) {
    
      // First DD to respond with a region -- save it.
      
      lastRgn_ = *imReg;
      rgnImgPath_ = qdd->path();
      hasRgn_ = True;
      emit newRegion(rgnImgPath_);
	// rgnImgPath_: pathname of the active image used to make the region.
	// Will be transformed into regionPathname() if saved to disk.
      //   rgnSaved = True;  }
     resetRTRegion();


     //this reset is necessary but cannot do
     //because the bug that polygon tool prematurely
     //emit 'reagion ready'.
     //enable this after that bug cas-1393 fix
     resetPTRegion();
    
    
    delete imReg;  }  }  

    

void QtDisplayPanel::resetRTRegion()  { rtregion_->reset();  }

void QtDisplayPanel::resetPTRegion()  { ptregion_->reset();  }

void QtDisplayPanel::resetZoomer()    { zoom_->reset();  }

void QtDisplayPanel::resetCrosshair() { crosshair_->reset();  }

void QtDisplayPanel::resetPolyline()  { polyline_->reset();  }

void QtDisplayPanel::resetPanner()    { panner_->reset();  }
  

void QtDisplayPanel::resetRegionTools() {
  resetRTRegion();  resetPTRegion();  }

void QtDisplayPanel::resetSelectionTools() {
    resetRegionTools();  resetCrosshair();  resetPolyline();  }
  

void QtDisplayPanel::resetTool(String toolname) {
    // (NB: no effect on PCTools (e.g. SHIFTSLOPE, BRIGHTCONTRAST).
    MultiWCTool* tool = pd_->getTool(toolname);
    if(tool!=0) tool->reset();  }

  
void QtDisplayPanel::resetTools() {
    for(Int i=0; i<Int(mouseToolNames_.nelements()); i++) {
      resetTool(mouseToolNames_[i]);  }  }
  


void QtDisplayPanel::installEventHandlers_() {
  pc_->addPositionEventHandler(*this);
  ConstListIter<WorldCanvas*>& wcs = *(pd_->myWCLI);
  for(wcs.toStart(); !wcs.atEnd(); wcs++) {
    wcs.getRight()->addMotionEventHandler(*this);  }  }

void QtDisplayPanel::removeEventHandlers_() {
  pc_->removePositionEventHandler(*this);
  ConstListIter<WorldCanvas*>& wcs = *(pd_->myWCLI);
  for(wcs.toStart(); !wcs.atEnd(); wcs++) {
    wcs.getRight()->removeMotionEventHandler(*this);  }  }

  
    
      
void QtDisplayPanel::operator()(const WCMotionEvent& ev) {
  // Overrides base WCMotionEH operator, to forward cursor position
  // events from any of the panel's WorldCanvases as POSITION TRACKING
  // Qt signals, via any/all of the Panel's registered QtDisplayDatas.
  
  if(!tracking_) return;

  WorldCanvas* wc = ev.worldCanvas(); 
  if(!myWC_(wc)) return;	// (safety)
  
  
  if(!wc->inDrawArea(ev.pixX(), ev.pixY())) return;
	// Don't track motion off draw area (must explicitly test this now).
  
  if(lastMotionEvent_!=0) delete lastMotionEvent_;
  lastMotionEvent_ = new WCMotionEvent(ev);
	// Save last mouse position (used by refreshTracking_(), below,
	// in case tracking data changes without mouse motion.)

  Record trackingRec;
  
  for(ListIter<QtDisplayData*> qdds(qdds_); !qdds.atEnd(); qdds++) {
    
    QtDisplayData* qdd = qdds.getRight();
    DisplayData*    dd = qdd->dd();
    
    if(dd->classType()==Display::Annotation ||
       dd->classType()==Display::CanvasAnnotation) continue;
	// (Tracking information is not provided for these dd types).

    trackingRec.define(qdd->name(), qdd->trackingInfo(ev));  }
  
  
  if(trackingRec.nfields()>0u) emit trackingInfo(trackingRec);  }
  

 
  
void  QtDisplayPanel::refreshTracking_(QtDisplayData* qdd) {
  // Similar to above, but this is a Qt slot triggered by an internal
  // dd change (not mouse movement; see operator()(WCMotionEvent),
  // above, for that.)
  
  if(lastMotionEvent_==0) return;
  WCMotionEvent& ev = *lastMotionEvent_;	// (shorthand)
  
  if(qdd==0) { operator()(ev); return;  }	// (general tracking refresh)
  
  // Else, if qdd is specified, tracking data is refreshed only for that dd.
  
  if(!isRegistered(qdd)) return;
  
  WorldCanvas* wc = ev.worldCanvas(); 
  if(!myWC_(wc)) return;	// (safety; ev could be obsolete)
  if(!wc->inDrawArea(ev.pixX(), ev.pixY())) return;
  
  DisplayData* dd = qdd->dd();
  if(dd->classType()==Display::Annotation ||
     dd->classType()==Display::CanvasAnnotation) return;  // (unlikely)

  Record trackingRec;
  trackingRec.define(qdd->name(), qdd->trackingInfo(ev));
  
  if(trackingRec.nfields()>0u) emit trackingInfo(trackingRec);  }

  
    
 
Bool QtDisplayPanel::myWC_(const WorldCanvas* wc) {
  // Utility function for above: is the given wc one belonging
  // to the main pd_?
  
  ConstListIter<WorldCanvas*>& wcs = *(pd_->myWCLI);
	// I don't like using shared iterators (It's not safe), but
	// it's the only mechanism provided to access the list (no
	// time to create new PanelDisplay interface at the moment...).
  
  for(wcs.toStart(); !wcs.atEnd(); wcs++) {
    if (wc==wcs.getRight()) return True;  }
    
  return False;  }

  
   
     
void QtDisplayPanel::resizeEvent(QResizeEvent* ev) {
  
  hold();
  
  QWidget::resizeEvent(ev);
  
  QSize panelSize = ev->size(),
       canvasSize = pc_->size();
  emit resized(panelSize, canvasSize);
  
  release();  }
  

    

// REGISTRATION METHODS / SLOTS

  
void QtDisplayPanel::ddCreated_(QtDisplayData* qdd, Bool autoRegister) {
  // DP actions to take when viewer signals new DD creation.
  
  if(autoRegister) {
    registerDD_(qdd);
    emit newRegisteredDD(qdd);  }
  
  else emit newUnregisteredDD(qdd); 

 }



void QtDisplayPanel::ddRemoved_(QtDisplayData* qdd) {
  // DP actions to take when viewer signals DD removal.
  
  if(isRegistered(qdd)) {
    unregisterDD_(qdd);
    emit RegisteredDDRemoved(qdd);  }
  else emit UnregisteredDDRemoved(qdd);  }

  

void QtDisplayPanel::registerDD(QtDisplayData* qdd) {
  // Called externally (by gui, e.g.) to register pre-existing DDs.

  if(!isUnregistered(qdd)) return;  //  Nothing to do.
  registerDD_(qdd);
  emit oldDDRegistered(qdd);  }


  
void QtDisplayPanel::registerDD_(QtDisplayData* qdd) {
  // Internal method, called by public register method above,
  // or in reaction to new DD creation (ddCreated_() slot).
  // Precondition: isUnregistered(qdd) should be True before this is called.
  
  ListIter<QtDisplayData*> qdds(qdds_);
  qdds.toEnd();
  qdds.addRight(qdd);
  
  DisplayData* dd = qdd->dd();
    
  
  hold();
  
  Int preferredZIndex;
  Bool ddHasPreferredZIndex = dd->zIndexHint(preferredZIndex);
	// (preferredZIndex is recorded prior to adding DD to underlying
	// pd_, for obscure reasons: sometimes a frame setting may be
	// used from another Panel where dd is registered).

  
  pd_->addDisplayData(*dd);
	// Maintain registration relation between the
	// wrapped classes.
  

  // Reset animator in accordance with new set of registered DDs
  // (This code comes mostly from GTkPD::add()).

  Record animrec;

  if(pd_->isCSmaster(dd) && ddHasPreferredZIndex) {
    // New dd has become CS master: pass along its opinions
    // on animator frame number setting, if any.
    animrec.define("zindex", preferredZIndex);  }

  // Blink index or length may also change when DD added.

  if(pd_->isBlinkDD(dd)) {
    animrec.define("blength", pd_->bLength());
    animrec.define("bindex",  pd_->bIndex());  }

        
  setAnimator_(animrec);

  
  connect( qdd, SIGNAL(optionsChanged(Record)),
                  SLOT(setAnimatorOptions_(Record)) );
	// (Allows dd to change animator settings itself, e.g.,
	// after a user-requested change to its animation axis).

  connect( qdd, SIGNAL(colorBarChange()),  SLOT(checkColorBars_()) );
	// Triggers color bar rearrangement if necessary.

  connect( qdd, SIGNAL(trackingChange(QtDisplayData*)),
                  SLOT(refreshTracking_(QtDisplayData*)) );
	// Triggers (non-mouse-event) tracking refresh for signaling dd.

  // have the name of the latest handy
  rgnImgPath_ = qdd->path();
  qdd->registerNotice(this);	// Let QDD know.
    
  release();  }
  


  
void QtDisplayPanel::unregisterDD(QtDisplayData* qdd) {
  // Called externally (by gui, e.g.) to unregister pre-existing DDs.
  
  if(!isRegistered(qdd)) return;  //  Nothing to do.
  unregisterDD_(qdd);
  emit oldDDUnregistered(qdd);  }
  

void QtDisplayPanel::unregisterDD_(QtDisplayData* qdd) {
  for(ListIter<QtDisplayData*> qdds(qdds_); !qdds.atEnd(); qdds++) {
    if(qdd == qdds.getRight()) {
  
      qdd->unregisterNotice(this);	// Let QDD know.
    
      qdds.removeRight();
      DisplayData* dd = qdd->dd();
      
      hold();
      
      
      if(pd_->isCSmaster(dd)) resetTools();
	// CS master change means it's better to erase mouse tools;
	// their internal coordinates may not be relevant anymore.
      
      pd_->removeDisplayData(*dd);

      
      // Signal animator to reset number of (Z) frames according to
      // remaining DDs.  Current frame should remain unchanged if
      // still in range.  Blink index or length may also change when
      // DD is removed.  (This code comes mostly from GTkPD::remove()).
      
      Record animrec;

      if(pd_->isBlinkDD(dd)) {
        animrec.define("blength", pd_->bLength());
        animrec.define("bindex",  pd_->bIndex());  }

      setAnimator_(animrec);

      // Ignore further animation change-request signals from dd,
      // since it is no longer registered.
      disconnect( qdd, SIGNAL(optionsChanged(Record)),
                  this,  SLOT(setAnimatorOptions_(Record)) );  

      // Likewise, colorbar rearrangement via this signal won't be
      // necessary if qdd is not registered.
      disconnect( qdd, SIGNAL(colorBarChange()),
                  this,  SLOT(checkColorBars_()) );
  
      // ditto dd tracking refresh from signal.
      disconnect( qdd, SIGNAL(trackingChange(QtDisplayData*)),
                  this,  SLOT(refreshTracking_(QtDisplayData*)) );

            
      release();
      
      break;  } 
  } 
}




void QtDisplayPanel::registerAll() {
  // Called externally (by gui, e.g.) to register all DDs created
  // by user through QtViewer.
  
  List<QtDisplayData*> unregdDDs(unregisteredDDs());
  if(unregdDDs.len()==0) return;
  
  hold();
  
  for(ListIter<QtDisplayData*> udds(unregdDDs); !udds.atEnd(); udds++) {
    QtDisplayData* dd = udds.getRight();
    registerDD_(dd);  }

  emit allDDsRegistered();  
    //# do animator resetting, ala GTkPD
      
  release();  }

  
  
void QtDisplayPanel::unregisterAll() {
  // Called externally (by gui, e.g.) to unregister all DDs.
  List<QtDisplayData*> regdDDs(registeredDDs());
  if(regdDDs.len()==0) return;
  
  hold();
  
  for(ListIter<QtDisplayData*> rdds(regdDDs); !rdds.atEnd(); rdds++) {
    QtDisplayData* dd = rdds.getRight();
    unregisterDD_(dd);  }
  
  if(qsm_!=0) qsm_->deleteAll();

  emit allDDsUnregistered();  
    //# do animator resetting, ala GTkPD.
      
  release();  }

  

Bool QtDisplayPanel::isRegistered(QtDisplayData* qdd) {
  for(ListIter<QtDisplayData*> qdds(qdds_); !qdds.atEnd(); qdds++) {
    if(qdd == qdds.getRight()) return True;  }
  return False;  }
    
Bool QtDisplayPanel::isRegistered(String ddname) {
  for(ListIter<QtDisplayData*> qdds(qdds_); !qdds.atEnd(); qdds++) {
    if(ddname == qdds.getRight()->name()) return True;  }
  return False;  }
    
Bool QtDisplayPanel::isUnregistered(QtDisplayData* qdd) {
  return !isRegistered(qdd) && v_->ddExists(qdd);  }

Bool QtDisplayPanel::isUnregistered(String ddname) {
  return !isRegistered(ddname) && v_->ddExists(ddname);  }



List<QtDisplayData*> QtDisplayPanel::unregisteredDDs() {
  // retrieve an (ordered) list of DDs (created on QtViewer) which
  // are _not_ currently registered.
  
  List<QtDisplayData*> unregdDDs(v_->dds());
  
  for(ListIter<QtDisplayData*> udds(unregdDDs); !udds.atEnd(); ) {
    if(isRegistered(udds.getRight())) udds.removeRight();
    else udds++;  }
  
  return unregdDDs;  }




void QtDisplayPanel::registerRegionShape(RegionShape* rs) {
    ListIter<RegionShape*> rshapes(rshapes_);
    rshapes.toEnd();
    rshapes.addRight(rs);
    hold();

    pd_->addDisplayData(*rs);

    release();


}

void QtDisplayPanel::unregisterRegionShape(RegionShape* rs) {
    for(ListIter<RegionShape*> rrss(rshapes_); !rrss.atEnd(); rrss++) {
      if(rs == rrss.getRight()) {
        rrss.removeRight();
        hold();
        pd_->removeDisplayData(*rs);
        release();
        break;
      }
    }
}

Bool QtDisplayPanel::isRegistered(RegionShape* rs) {
    for(ListIter<RegionShape*> qrss(rshapes_); !qrss.atEnd(); qrss++) {
      if(rs == qrss.getRight()) return True;  }
    return False;  }




// REGION MANAGEMENT METHODS



String QtDisplayPanel::regionPathname(String origPath) {
  // [static] Transform origPath to a plausible pathname for an image region.
  // Assures the filename ends in '.rgn' and that no such file already exists.
  
  QString nm = QString(origPath.chars()).trimmed();  // (whitespace off ends).
  
  Int id=1; QString idstr="";	// I.D. to make names unique, if needed.
  
  if(nm=="" || nm.endsWith("/")) { 
    nm+= "region";
    idstr="-1";  }
    
  QString name;
  while(id<10000) {
    name = nm+idstr+".rgn";
    if(!QFileInfo(name).exists()) break;  // Found a name not in filesystem.
    id++;	// bump up unique I.D. suffix.
    idstr = "-" + QString::number(id);  }

  return name.toStdString();  }
  
  
  
  Bool QtDisplayPanel::saveLastRegion(String path) {
    // Save the last region which was created (via an image DD) with the mouse.
    // (i.e. the one stored in lastRgn_).  Return value indicates success.
    // (The ImageRegion is transformed to a TableRecord, then saved via AipsIO).
    
    if(!hasRgn_) return False;	// (No region ever created here).
    
    try{
      
      AipsIO os(path, ByteIO::NewNoReplace);
      
      os << lastRgn_.toRecord(path+".tbl");  }
    // (I don't believe the 'tablename' parameter to toRecord() is
    // actually used in most cases.  Just guessing at a plausible
    // value to pass to it...).
    
    catch(...) { return False;  }
    
    return True;  }
  String QtDisplayPanel::saveRegionInImage(String regname, 
					   const ImageRegion& imreg) {
    // (The ImageRegion is stored in the image/table as a keyworrd).
    
    String retval="";
    //if(!hasRgn_) return retval;	// (No region ever created here)
    
    try{
      
      //RegionManager regMan;
      //retval=regMan.imageRegionToTable(rgnImgPath_, imreg, regname);
      ListIter<QtDisplayData*> qdds(qdds_);
      qdds.toEnd();
      qdds--;
      QtDisplayData* qdd = qdds.getRight();
      if( (qdd->imageInterface()) && ((qdd->imageInterface())->canDefineRegion())){
	
	(qdd->imageInterface())->defineRegion(regname, imreg, RegionHandler::Regions, True);
      }

 
    }
    catch(...) { return String("");  }
    
    return retval; 
  }

  void QtDisplayPanel::removeRegionInImage(String regname) {
    // (The ImageRegion is stored in the image/table as a keyworrd).
    
    try{
      
      
      //RegionManager regMan;
      //retval=regMan.imageRegionToTable(rgnImgPath_, imreg, regname);
      ListIter<QtDisplayData*> qdds(qdds_);
      qdds.toEnd();
      qdds--;
      QtDisplayData* qdd = qdds.getRight();
      if( (qdd->imageInterface()) && ((qdd->imageInterface())->canDefineRegion())){
	
	(qdd->imageInterface())->removeRegion(regname, RegionHandler::Any, False);
      }

 
    }
    catch(...) { return;  }
    
  }


  String QtDisplayPanel::listRegions() { 
    String retval="";
    try{
      
      RegionManager regMan;
      Vector<String> regs=regMan.namesInTable(rgnImgPath_);
      if(regs.nelements() > 0){
	retval=regs[0];
	for (uInt k=1; k < regs.nelements(); ++k){
	  retval=retval+", "+regs[k];
	}
      }
    }
    catch(...) { return String("");  }
    
    return retval; 
  }
  Vector<String> QtDisplayPanel::listRegionsInImage(){
    Vector<String> retval(0);
    try{
      
      
      retval.resize();
      ListIter<QtDisplayData*> qdds(qdds_);
      qdds.toEnd();
      qdds--;
      QtDisplayData* qdd = qdds.getRight();
      if( qdd->imageInterface()){

	//retval=regMan.namesInTable(rgnImgPath_);
	retval=(qdd->imageInterface())->regionNames();
      if(retval.shape()(0)==0)
	retval=Vector<String>(0);
      }
    }
    
    catch(...) 
      { return Vector<String>(0); 
      }


    return retval; 
  }

  ImageRegion QtDisplayPanel::getRegion(const String& name){


    ImageRegion reg;
    
    try{


      ListIter<QtDisplayData*> qdds(qdds_);
      qdds.toEnd();
      qdds--;
      QtDisplayData* qdd = qdds.getRight();
      if( qdd->imageInterface()){
	reg=(qdd->imageInterface())->getRegion(name); 
      }
    }
    catch(...) 
      { return ImageRegion(); 
      }

    return reg;

  }
// HOLD AND RELEASE OF REFRESH.  

// In order to draw, every call to hold()
// must be accompanied by a subsequent call to release() (so don't
// neglect: beware of exceptions, e.g.).  Calls can nest (they are 
// counted).  Panel may be deleted in a held state.  Also, excess calls
// to release() will have no effect.  The calls are propagated to the main
// PanelDisplay as well as to those used for color bars (and thence to
// their WorldCanvases).

void QtDisplayPanel::hold() {
  
  pd_->hold();
   
  for(ListIter<PanelDisplay*> cbps(colorBarPanels_); !cbps.atEnd(); cbps++) {
    cbps.getRight()->hold();  }
  
  if(blankCBPanel_!=0) blankCBPanel_->hold();   }
   

     
void QtDisplayPanel::release() {
  
  if(blankCBPanel_!=0) blankCBPanel_->release();
  
  for(ListIter<PanelDisplay*> cbps(colorBarPanels_); !cbps.atEnd(); cbps++) {
    cbps.getRight()->release();  }
    
  pd_->release();  }

  
 
   
// DISPLAY PANEL OPTIONS METHODS    
    
        
Record QtDisplayPanel::getOptions() { 
  // Return Options record (of margins and no.-of-panels settings, e.g.)
  // The form of the record is suitable for automatically creating
  // controlling user interface.  These options are set with the
  // corresponding setOptions() slot (below).

  Record opts = pd_->getOptions(); 
  
  bkgdClrOpt_->toRecord(opts, True, True);
  
  return opts;  }    

  
        
void QtDisplayPanel::setOptions(Record opts, Bool emitAll) {
  // Set display panel options such as margins or number of panels.  The
  // form of the record (along with current settings) is returned by
  // getOptions().  (These Records are an older form of 'parameter sets';
  // they are used in various places within the display library).
  // Set emitAll = True if the call was not initiated by the options gui
  // itself (e.g. via scripting or save-restore); that will assure that
  // the options gui does receive all option updates (via the optionsChanged
  // signal) and updates its user interface accordingly.
  
  Record chgdOpts;
  
  try {
    
    hold();
    
    removeEventHandlers_();
	// These are torn down and then rebuilt because the setOptions
	// call below may change the set of underlying canvases.
    
    
    Bool needsRefresh = pd_->setOptions(opts, chgdOpts);
    
        
    installEventHandlers_();
    
    
    setAnimator_(Record());
	// Assures animation state is set onto new canvases,
	// if any (and causes refresh).
    
    
    checkColorBars_();
	// to realign color bar (length) margins if necessary.
       
        
    // Background / foreground color setting.
    
    if(bkgdClrOpt_->fromRecord(opts)){
      String bgd = bkgdClrOpt_->value();
      String fgd = (bgd=="white")? "black" : "white";
      pc_->setDeviceBackgroundColor(bgd);
      pc_->setDeviceForegroundColor(fgd);
      pc_->refresh();  }

    
    if(emitAll) chgdOpts.merge(opts, Record::SkipDuplicates);
	// When emitAll==True this assures that the options gui
	// receives all option updates via the optionsChanged
	// signal, not just internally-generated ones.  For use
	// when the gui itself didn't initiate the setOptions call.
	// Note that the internal chgdOpts are a response to the
	// original opts sent into this method and are more recent;
	// they overwrite the original opts if the same option
	// is present in both.
    
    
    if(chgdOpts.nfields()!=0) emit optionsChanged(chgdOpts);

    if(needsRefresh) pd_->refresh();
    
    release();  }

  catch (...) {  }  }


  
  
// COLOR BAR MANAGEMENT METHODS

    
void QtDisplayPanel::updateColorBarDDLists_() {
  // Update the List of registered DDs (in registration order) which
  // have color bar display activated and would draw if 'blink' selection
  // were correct (member variable allColorBarDDs_). That list is then
  // further restricted to (member list) colorBarDDsToDisplay_, the ones
  // whose blink restriction (if any) allows them to display on some
  // [sub]panel right now.  Those are the ones whose color bars will
  // actually be displayed now; they will be ordered according to the
  // (main) subpanel on which they appear (and then by registration order).
  // However, they are then reversed for horizontal colorbars, in order
  // to display top-to-bottom.
  // A helper routine, called (only) by arrangeColorBars_().
  
  
  // Create new allColorBarDDs_ List (paring down from all registered DDs).  
  
  allColorBarDDs_ = registeredDDs();
  
  for(ListIter<QtDisplayData*> acbdds(allColorBarDDs_); !acbdds.atEnd(); ) {
    
    QtDisplayData* cbdd = acbdds.getRight();
    
    if(cbdd->wouldDisplayColorBar() &&
       pd_->conforms(cbdd->dd(), False, True, False)) acbdds++;	 // (keep).
    else  acbdds.removeRight();  }				 // (remove).
	// (Note: behavior is still somewhat anomalous if some raster DDs
	// have different axes set from CSMaster DD (uncommon, no big deal.))
  
  // Pare allColorBarDDs_ down further, into colorBarDDsToDisplay_ (the
  // latter will be in sub-panel order).
  
  List<QtDisplayData*>     ccbDDs = allColorBarDDs_;
  ListIter<QtDisplayData*> ccbdds(ccbDDs);
	// 'Candidate List' of cbDDs to display; they will be removed
	// from this temporary List if/when added to colorBarDDsToDisplay_.
  
  colorBarDDsToDisplay_ = List<QtDisplayData*>();
	// Start from scratch by clearing away old list
	// (NB: there is no List[Iter]::clear() method (!?)).

  ListIter<QtDisplayData*> cbdds(colorBarDDsToDisplay_);
  
  // We iterate first over subpanels so that in multipanel 'blink' displays
  // colorbars are shown in the same order as the images themselves.
  for(Int panel_i=0; panel_i<nPanels() ; panel_i++) {
    
    // Will any of the remaining candidate DDs display on this subpanel?
    
    for(ccbdds.toStart(); !ccbdds.atEnd(); ) {
      
      QtDisplayData* ccbdd = ccbdds.getRight();
      
      // 'True, False, False' == 'does DD conform to this sub-panel's
      // blink restriction (if any)?'  (dd's Coordinate compatibility
      // has already been tested farther above).
      if(pd_->conforms(ccbdd->dd(), True, False, False, panel_i)) {
      
        // dd will display.  Move it off the candidate list, onto the
	// end of the list of DDs whose colorbars should also display.
	
        ccbdds.removeRight();
	cbdds.addRight(ccbdd);
	if(v_->colorBarsVertical()) cbdds.toEnd();
	else                        cbdds.toStart();  }
		// In the horizontal case, colorbars are added to the start
		// of colorBarDDsToDisplay_ rather than to the end, reversing
		// this list from 'proper display order'.  Colorbar panels
		// are created left-to-right or bottom-to-top, whereas
		// colorbars are to display 'in proper order' either
		// left-to-right or _top-to-bottom_.

      else ccbdds++;  }  }  }
		// (ccbdd won't display on this subpanel; pass on it for now)



Int QtDisplayPanel::marginb_(QtDisplayData* dd, Float shrink) {
  // Return the margin to give to dd's colorbar panel on the side where
  // colorbar labelling is done (right margin for vertical bars, e.g.).
  // shrink will usually be 1.; in rare cases where many colorbars are
  // crowding the main panel and each other, it may be less (and then
  // the returned margin allowance may also be less than ideal...).
  // A helper routine, called (only) by arrangeColorBars_().
  
  Bool vertical = v_->colorBarsVertical();
  Float charsz = dd->colorBarLabelSpaceAdj();
	// 'pgp character size' times a user-settable adjustment factor.
    
  return max(0, Int(ceil(
         (mrgna_+(vertical?11:6)*charsz) * shrink - mrgna_   )));  }


    
Float QtDisplayPanel::cbPanelSpace_(QtDisplayData* dd) {
  // A helper routine, called (only) by arrangeColorBars_().
  // Return the proportion of the PixelCanvas to use (whenever possible)
  // for dd's colorbar panel.  (This is allocated along the direction of
  // the colorbar's thickness; the panel uses the entire PC size along the
  // direction of the colorbar's length).  
  
  Float cblen = pclnsz_ - (lnmrgna_+lnmrgnb_)*marginUnit_;
	// (Approx.) length in screen pixels that the colorbar will have.
	// (All colorbars are given the same margins in their 'length'
	// direction as the main image panel(s)), in an attempt (not always
	// entirely successful) to align the colorbar with its image).

  Float th4ln = .03;
	// An important parameter -- adjust if necessary:
	// the nominal thickness-to-length ratio for a colorbar.

  th4ln *= dd->colorBarSizeAdj();
	// Manual user option: an adjustment factor to the above.  
  
  Float cbth = max(0., th4ln*cblen / pcthsz_);
	// Thickness to give the colorbar itself, as a proportion of
	// PC size in the thickness direction.

  Float cbmrg = (mrgna_ + marginb_(dd))*marginUnit_ / pcthsz_;
	// Proportion of PC size (again, in the thickness direction)
	// to use for margins.

  return  cbth + cbmrg;  }
  

  

void QtDisplayPanel::arrangeColorBars_(Bool reorient, Bool resizing) {
  // This routine corresponds to the old viewerdisplaypanel.g routine called
  // 'arrangewedgerequirements'.  It responds to events which may require a
  // change to the relative placement or number of colorbars and their panels.
  // 'reorient' means color bars are changing from horizontal to vertical or
  // vice versa.  That helps determine how much action (if any) this routine
  // needs to take.
  // Only the pcResizing_() slot should set resizing=True; in this case,
  // arrangeColorBars_ lets the PC take care of refresh.

  
  List<QtDisplayData*>  oldCBDDs = colorBarDDsToDisplay_;
  	// Store copy of the old List, for comparison.
  
  updateColorBarDDLists_();
	// Update colorBarDDsToDisplay_ (as well as allColorBarDDs_).
  
  List<QtDisplayData*>& newCBDDs = colorBarDDsToDisplay_;
		// Alternate name for the newly-updated (definitive)
		// member List (just for style).

  Int totcbdds  = allColorBarDDs_.len();
  Int nOld      = oldCBDDs.len();  // (Also == (old) colorBarPanels_.len().)
  Int nNew      = newCBDDs.len();  // (Will become new colorBarPanels_.len().)

  ListIter<QtDisplayData*> oldcbdds(oldCBDDs);
  ListIter<QtDisplayData*> newcbdds(newCBDDs);
  ListIter<QtDisplayData*> allcbdds(allColorBarDDs_);
  


  // Prepare field names and values for geometry/margin setting on the
  // color bar panels (according to whether colorbars are being placed
  // vertically or horizontally).
  
  Bool vertical = v_->colorBarsVertical();

  // field names for vertical bars  
  
  String origin="xorigin", size="xsize",
         lengthsideorigin="yorigin", lengthsidesize="ysize",
         margina = "leftmarginspacepg",
         marginb = "rightmarginspacepg",
         lengthsidemargina = "bottommarginspacepg",  
         lengthsidemarginb = "topmarginspacepg";
  
  // field names for horizontal bars
  
  if(!vertical) {
         origin="yorigin"; size="ysize";
         lengthsideorigin="xorigin"; lengthsidesize="xsize";
         margina = "bottommarginspacepg";
         marginb = "topmarginspacepg";
         lengthsidemargina = "leftmarginspacepg";  
         lengthsidemarginb = "rightmarginspacepg";  }

  
  // Set up (preliminary) panel margin sizes.
  
  // (Experiment shows one 'pgp margin unit' (horizontal _or_ vertical) to
  // be the width of a pgplot letter "X" of 'size 1', which is also about
  // 1/65  of the shortest side the (_entire_) pc_ (not 1/40, as I thought
  // some doc somewhere had said...).
  // Also, the code in WorldCanvasHolder::executeSizeControl() which
  // operationally defines this 'pgp margin unit' seems to be retrieving
  // the _height_ rather than width of a letter "X".  (I do not understand
  // that, since space for its width is actually what gets allocated....))
  
  Int pcw = max(Int(pc_->width()),  1);  
  Int pch = max(Int(pc_->height()), 1);  
  
  marginUnit_ = min(pcw, pch)/65.;
	// (Approx.) screen pixels in one 'margin unit': 1/65 of
	// the PixelCanvas's minimum dimension.
  
  mrgna_ = 1,	// (fixed; others likely to be refined below).
  mrgnb_ = 11, lnmrgna_ = 7, lnmrgnb_ = 4;
	// (mrgnb_ is reset farther below, according
	// to each color bar's individual font size).

  Bool notFound;
  Record mainPanelOpts = pd_->getOptions();
  pd_->readOptionRecord(lnmrgna_, notFound, mainPanelOpts, lengthsidemargina);
  pd_->readOptionRecord(lnmrgnb_, notFound, mainPanelOpts, lengthsidemarginb);
	// Set margins which constrain the colorbar's long-side to those of
	// the main Panel/WC.
	// (Could be improved, because WC draw area, which ideally the color
	// bar aligns with on its long side, often does not fill the
	// WC area within the margins due to fixed aspect ratio.  Also,
	// there may be more than one WC within the main panel).
  
  if(vertical) { pcthsz_ = pcw,  pclnsz_ = pch;  }
  else         { pcthsz_ = pch,  pclnsz_ = pcw;  }
	// PC size in pixels along the direction of the colorbar's
	// thickness (pcthsz_) and length (pclnsz_).  The latter direction is
	// called the _colorbar's_ 'length side'; 'lengthsidemargin's
	// constrain the colorbar's length.  But note that this may or may
	// not be the _PC's_ longer side.
  
  
  
  // How much space should be allowed for colorbars?  We want the area
  // allocated for them to remain unchanged during an animation (in either
  // blink or normal mode) so that the area for image display doesn't
  // fluctuate.  We first find the maximum number of colorbars that may
  // conceivably display at one time (nMax), and then the size requests of
  // the nMax thickest colorbars.  In the blink case nMax may be less than
  // total CBDDs yet more than the CBDDs to display at the moment. 

    
  // In "Normal" animation mode it's simple: all color bar DDs get a panel
  // to display their colorbar.
  
  Int nMax;
  
  if(mode()=="Normal") nMax = totcbdds;
  
  else {
  
    // In blink mode, it's more complicated: DDs which are always on anyway
    // in blink mode and which display a colorbar always get a panel.  (At
    // present no DDs fit this category, but colormapped contour DDs, e.g.,
    // might do so if implemented).  Of the remaining CBDDs, up to one per
    // main image subpanel can display.
    
    Int nb=0;
    for(allcbdds.toStart();  !allcbdds.atEnd();  allcbdds++) {
      if( pd_->isBlinkDD(allcbdds.getRight()->dd()) ) nb++;  }
    
    Int nnb = totcbdds - nb;	// nnb, nb: number of non-blinking and
				// blinking CBDDs.
  
    nMax = nnb + min(nPanels(), nb);  }
  
  
  // [Try to] allocate space for the nMax largest colorbar panels, over
  // all colorbars that might display during an animation.  (Often, though
  // not always, all panels will have the same size).
    
  // cbpszs: panel sizes in descending order.
  Vector<Float> cbpszs(totcbdds, 0.);
  Int i=0;
  for(allcbdds.toStart();  !allcbdds.atEnd();  allcbdds++, i++) {
    Float cbpsz = cbPanelSpace_(allcbdds.getRight());
    Int j=i;
    for(; j>0 && cbpszs[j-1]<cbpsz ; j--) cbpszs[j] = cbpszs[j-1];
    cbpszs[j] = cbpsz;  }
    
  // totcbpsz: desired total PC proportion for cb panels.
  Float totcbpsz = 0.;  for(Int i=0; i<nMax; i++) totcbpsz += cbpszs[i];
  
  // newmainpanelsz and totcbpsz are the proportions that will actually
  // be allocated for the main image display area and colorbar panels
  // respectively.  We'll always allocate at least 35% for main image
  // display area, but totcbpsz will be less than 65% anyway in the
  // majority of cases.
  //
  // It is possible in certain cases that the allocated colorbar area will
  // not be completely filled (the simplest case is in blinking 2 images,
  // one with colorbar on, one off; the space for one colorbar will remain
  // in the display whichever image is showing).
  
  Float  oldmainpanelsz = mainPanelSize_;	// (copy of old)
  Float& newmainpanelsz = mainPanelSize_;
  
  totcbpsz = min(.65, totcbpsz);
  newmainpanelsz = 1. - totcbpsz;
  
  
  
  // Determine relative size that each new colorbar panel should finally
  // have. Only colorbars actually displayed at present need to be tested to
  // see if they all fit within totcbpsz.  (If not, they will all have to be
  // reduced by applying a shrinkfctr less than 1; this should occur rarely
  //, though).
  
  Vector<Float>  oldcbpszs;
                 oldcbpszs = colorBarPanelSizes_;	// (copy of old)
  Vector<Float>& newcbpszs = colorBarPanelSizes_;
  newcbpszs.resize(nNew);	// Prepare to recalculate the new
				// (member) Vector of panel sizes.
  Float requestedsz = 0.;
	// Total proportion of pc_ requested for colorbar panels 
	// displaying now (to check for for crowding of main pd_).
  
  newcbdds.toStart();
  for(Int i=0;   i<nNew;   newcbdds++, i++) {
    QtDisplayData* dd = newcbdds.getRight();
    newcbpszs[i] = cbPanelSpace_(dd);
    requestedsz += newcbpszs[i];  }
      
  Float shrinkfctr = 1.;
  if(requestedsz > totcbpsz) {
    // Colorbars must be reduced in size so that they fit within
    // totcbpsz (won't happen very often).
    shrinkfctr = totcbpsz/requestedsz;
    for(Int i=0; i<nNew; i++) newcbpszs[i] *= shrinkfctr;  }
    
  
  // Determine whether the number or relative sizes of color bar panels and
  // main panel will change.  If 'placementChange' remains False, no change
  // will be needed to the relative placement ('geometry') of panels within
  // pc_.  (The colorbar panels' margins will be updated anyway, though,
  // just in case -- not an expensive operation if there's no change).
  
  Bool mainPlacementChange = oldmainpanelsz!=newmainpanelsz || reorient;
	// Whether the main panel itself will require resizing.
  
  Bool placementChange = mainPlacementChange || nNew!=nOld;
  if(!placementChange) {
    for(Int i=0; i<nNew; i++)  if(oldcbpszs[i]!=newcbpszs[i]) {
      placementChange = True;  break;  }  }
	// whether colorbar panels will be resized.



  // All necessary information has now been gathered;
  // do the actual adjustments.
  
    
  hold();	// (suspend refresh of main pd_ and colorbar
		// panels until all adjustments finished).
      
  
  // Create or delete panels (PanelDisplays) for the colorbars, if
  // the required number of them has changed (nNewPanels!=nOldPanels).
  
  ListIter<PanelDisplay*> cbps(colorBarPanels_);
  cbps.toEnd();
  
  for(Int i=nNew;  i<nOld;  i++) {
    cbps--;
    delete cbps.getRight();	// Delete excess colorbar PanelDisplays...
    cbps.removeRight();  }	// (old colorbar automatically unregistered)
    
  for(Int i=nOld;  i<nNew;  i++, cbps++) {    // ...or add needed new ones.
    PanelDisplay* cbp = new PanelDisplay(pc_, 1,1);
    cbp->hold();   // (consistent with hold() above -- uses same release()).   
    cbps.addRight(cbp);  }
  
  
  // Place main data DisplayPanel (pd_).
  
  Record geom;
  Float orgn = 0.,  siz = newmainpanelsz;

	// Current relative position (and size) for panel placement;
	// ranges from 0 (left edge of pc) to 1 (right edge)
	// for vertical = True (otherwise 0 = bottom, 1 = top).

  if(mainPlacementChange) {
    pd_->getGeometry(geom);
    geom.define(origin, orgn);  geom.define(lengthsideorigin, 0.f);
    geom.define(size,   siz);   geom.define(lengthsidesize,   1.f);
    pd_->setGeometry(geom);  }
  
  
  // For each color bar [panel] in the new list.
  
  cbps.toStart();  newcbdds.toStart();  oldcbdds.toStart();
  
  for(Int i=0;    i<nNew;     i++, cbps++, newcbdds++) {
      
    // Assure that correct color bar DD is registered on each panel.
    
    PanelDisplay*  cbp     = cbps.getRight();
    QtDisplayData* newdd   = newcbdds.getRight();
    WedgeDD*       newcb   = newdd->colorBar();
    
    if(i<nOld) {
      
      QtDisplayData* olddd = oldcbdds.getRight();
      WedgeDD*       oldcb = olddd->colorBar();
      
      if(olddd != newdd) {
        cbp->removeDisplayData(*oldcb);
        cbp->addDisplayData(*newcb);  }		// Replace panel's color bar.
    
      oldcbdds++;  }
      
    else cbp->addDisplayData(*newcb);		// Add new panel's color bar.
    
  
    // Place/size the colorbar panels
    // (NB: QtViewerBase sets orientation onto the colorbars themselves).
    
    if(placementChange) {
      orgn += siz;			 // Move origin past previous panel
      siz = min(newcbpszs[i], 1.-orgn);	 // and retrieve new panel's size.
    
      cbp->getGeometry(geom);
      geom.define(origin, orgn);  geom.define(lengthsideorigin, 0.f);
      geom.define(size,   siz);   geom.define(lengthsidesize,   1.f);
      cbp->setGeometry(geom);  }
    
    
    // Set margins of color bar panels.
    
    // 'Margin b' is where most color bar labelling occurs (to the right of
    // vertical colorbars, or above horizontal ones).  marginb_() tries to
    // allocate enough margin so that the labels fit, taking label character
    // size, color bar orientation and possible manual user adjustment into
    // account.
    
    mrgnb_ = marginb_(newdd, shrinkfctr);
    
    Record margins, chgdOpts;
    
    margins.define(margina, mrgna_);
    margins.define(marginb, mrgnb_);
    margins.define(lengthsidemargina, lnmrgna_); 
    margins.define(lengthsidemarginb, lnmrgnb_); 
    
    cbp->setOptions(margins, chgdOpts);  }	// (chgOpts ignored here)

  
  // Set or remove blank colorbar panel, as needed.  (Its sole purpose
  // is to assure that unused colorbar space (if any) is cleared).
  
  if(placementChange) {
    orgn += siz;
    if(orgn>=1.) {
      if(blankCBPanel_!=0) { delete blankCBPanel_; blankCBPanel_=0;  }  }
    else {
      if(blankCBPanel_==0) { 
        blankCBPanel_ = new PanelDisplay(pc_, 1,1);
        // No margins for blank panel.
        Record margins, chgdOpts;
        margins.define(margina, 0);
        margins.define(marginb, 0);
        margins.define(lengthsidemargina, 0); 
        margins.define(lengthsidemarginb, 0); 
        blankCBPanel_->setOptions(margins, chgdOpts);  }
      
      siz = 1.-orgn;
      blankCBPanel_->getGeometry(geom);
      geom.define(origin, orgn);  geom.define(lengthsideorigin, 0.f);
      geom.define(size,   siz);   geom.define(lengthsidesize,   1.f);
      blankCBPanel_->setGeometry(geom);  }  }
      
      
  
  if(!resizing) {	// (on resize, PC takes care of refresh).
    if(mainPlacementChange) refresh();
    else                    refreshCBPanels_();  }

  release();  }





void QtDisplayPanel::refreshCBPanels_() {  
  // Refresh (only) the colorbar panels (if any).  (An attempt to reduce
  // flashing during blink animation).
  
  for(ListIter<PanelDisplay*> cbps(colorBarPanels_); !cbps.atEnd(); cbps++) {
     cbps.getRight()->refresh();  } 
  
  if(blankCBPanel_!=0) blankCBPanel_->refresh();  }
  



// ANIMATION METHODS/SLOTS


void QtDisplayPanel::setAnimatorOptions_(Record opts) {
  if(opts.isDefined("setanimator") && 
     opts.dataType ("setanimator")==TpRecord) {
     
    Record sarec = opts.asRecord("setanimator");
    
    setAnimator_(sarec);  }  }



void QtDisplayPanel::setAnimator_(Record sarec) {
  // sarec can contain "zindex", "zlength" (cube mode settings),
  // "bindex" and/or "blength" fields (blink mode settings).
  // Either may be updated, regardless of current animator mode.
  //
  // The current zlength value (polled from DDs) is always
  // [re]set onto the animator during this call, even when it
  // does not appear explicitly in sarec.  setAnimator_ can simply
  // be called with an empty sarec (and often is) when the number of
  // animation frames has changed.  (However, blink mode settings are
  // specified explicitly if they need to be changed).
  
   
  // Z-MODE ("Normal") SETTINGS
  
  Int len = pd_->zLength();
	// pd_->zLength() polls the active DDs' nelements(), taking their
	// maximum.
  
  if(len<1) len=1;
	// (Even an empty display panel is considered to have animation
	// length 1 in the Qt version of the animator).
  
  if(sarec.isDefined("zlength") && sarec.dataType("zlength")==TpInt) {
    len = max(len, sarec.asInt("zlength"));  }
	// (This statement shouldn't be necessary; the previous ones
	// should suffice.  Probably best if DDs don't send an explicit
	// "zlength" field, and just implement nelements() properly
	// instead; that gets polled in the prior statement.  This was
	// inserted just in case ScrollingRasterDD might need it).


  setZlen_(len);
  
  // If the old frame number is now out of new range, reset it to zero
  // (otherwise, leave it alone).  However, if a new one was suggested
  // in the Record, use that instead.
  Int frm = zIndex();
  if(frm >= nZFrames()) frm = 0;
  if(sarec.isDefined("zindex") && sarec.dataType("zindex")==TpInt) {
    frm = sarec.asInt("zindex");  }

    
  goToZ_(frm);


  // B-MODE ("Blink") SETTINGS
  
  if(sarec.isDefined("blength") && sarec.dataType("blength")==TpInt) {
    setBlen_(sarec.asInt("blength"));  }
  
  if(sarec.isDefined("bindex") && sarec.dataType("bindex")==TpInt) {
    goToB_(sarec.asInt("bindex"));  }
  else goToB_(bIndex());
	// (Assures blink restriction is set onto any new panels, if nec.).

        
  emit animatorChange();  }




void QtDisplayPanel::setZlen_(Int len) {
  // Only used by setAnimator_ at present, in turn resulting from
  // DD signals or DD registration activity.  setAnimator_() normally
  // determines this animation length by polling active DDs.
  // Caller still needs to signal animatorChange, and assure that
  // zIndex_ is in range (best to use goToZ_ for that).
  
  len = max(1, len);
  if(len==zLen_) return;
  
  stop_();
  
  zLen_=len;
  
  zStart_=0;	 // Change in total number of frames always
  zEnd_=zLen_;	 // resets 'playback range' to 'all frames'....
  zStep_=1;
  
  // emit animatorChange();   // (Caller should do this instead).
    
}
 

void QtDisplayPanel::setBlen_(Int len) {
  len = max(1, len);
  if(len==bLen_) return;
  
  stop_();
  
  bLen_=len;
  
  bStart_=0;
  bEnd_=bLen_;
  bStep_=1;  }
  


  
void QtDisplayPanel::goToZ(int frm) {
  // Connected from text box and slider; also usable by scripts.
  stop_();
  goToZ_(frm);
  emit animatorChange();  }


  
void QtDisplayPanel::goToZ_(Int frm) {
  // Internal part: doesn't send signals, but does set the restrictions
  // (which requests refresh).

  frm = max(0, min(nZFrames()-1,  frm));
	// Assure value is in range, if caller didn't do this himself.
  
  zStart_ = min(zStart_, frm);	// Calling this 'automatically' expands
  zEnd_ =   max(zEnd_, frm+1);	// animation limits to include selected frame.
  
  zIndex_ = frm;
  
  AttributeBuffer zInd, zIncr;
  zInd.set("zIndex", zIndex_); 
  
  zIncr.set("zIndex", modeZ()? 1:0);
        // Set the 'multipanel increment':
	// In blink mode all 'panels' (WCs) of the DisplayPanel display
	// the same plane (of various images) -- in normal mode, multiple
	// panels display successive planes (of the same image).
	// NB: 'step()' does not refer to this increment between panels
	// (which is fixed, here), but to the number of planes moved
	// (by _all_ panels) when an animation (tapedeck) step occurs.

  pd_->setLinearRestrictions(zInd, zIncr);  }
	// (I think we want to do this regardless of previous
	// zIndex_: new canvases, init, etc. (?))
    

 

         
void QtDisplayPanel::goToB(int frm) {
  // Connected from text box, or usable by scripts.
  
  stop_();
  goToB_(frm);
  emit animatorChange();  }
 
  

void QtDisplayPanel::goToB_(Int frm) {
  
  frm = max(0, min(nBFrames()-1, frm));
	// Assure within range.  If changing number of frames also,
	// do that first.
  
  bStart_ = min(bStart_, frm);
  bEnd_ =   max(bEnd_, frm+1);
  
  Int oldbIndex = bIndex_;
  
  bIndex_ = frm;	// blink state is always maintained.  However,...

  if(!modeZ()) {	// ...actual blink restriction is set onto
			// canvases only during blink mode.
    AttributeBuffer bInd, bIncr;
    bInd.set("bIndex", bIndex_); 
    bIncr.set("bIndex", 1);
    pd_->setLinearRestrictions(bInd, bIncr);
    
    if(oldbIndex!=bIndex_) checkColorBars_();  }  }
 

         

void QtDisplayPanel::setMode(bool modez) {
  // True: "Normal" ("Z") mode.  False: "Blink" ("B") mode.
  // (NB: small 'b' bool for a reason -- see declataion of goTo(int)).

  stop_();
  
  if(modeZ_!=modez) {	// (already there otherwise).
    
    modeZ_ = modez;
  
    hold();
  
    goToZ_(zIndex());	// (Sets proper multi-panel zIndex increment
			//  in accordance with new mode, primarily).
  
    if(mode()=="Blink")  goToB_(bIndex());
		// (Sets 'Blink restrictions').    
    else  pd_->removeRestriction("bIndex");
		// (Those restrictions shouldn't exist in "Normal" mode).

    checkColorBars_();
    
    release();  }
  
  
  emit animatorChange();  }



void QtDisplayPanel::prev_() { 
  Int newframe = frame() - step();
  if(newframe<startFrame()) newframe = lastFrame();
  goTo_(newframe);
  emit animatorChange();  }

void QtDisplayPanel::next_() {
  Int newframe = frame() + step();
  if(newframe>lastFrame()) newframe = startFrame();
  goTo_(newframe);
  emit animatorChange();  }


//#dk Limiting animation range not really supported yet.
//    (Remember, these should call goToX_(), if necessary
//     to put current frame within new range).
void QtDisplayPanel::setEndZFrame(Int frm) {  }
void QtDisplayPanel::setEndBFrame(Int frm) {  }



void QtDisplayPanel::revPlay() { 
  animating_ = -1;
  tmr_.start();
  emit animatorChange();  }

void QtDisplayPanel::stop() { stop_();  emit animatorChange();  }

void QtDisplayPanel::stop_() { animating_ = 0; tmr_.stop();  }


void QtDisplayPanel::fwdPlay() {
  animating_ = 1;
  tmr_.start();
  emit animatorChange();  }

void QtDisplayPanel::setRate(int rate) {
  animRate_ = max(minRate(), min(maxRate(),  rate  ));
  tmr_.setInterval(1000/animRate_);
  emit animatorChange();  }
  
  


  
// SAVE - RESTORE METHODS/SLOTS



String QtDisplayPanel::dpState(String restorefilename) {
  // Returns an xml String of display panel state.  This includes registered
  // DDs and their options, panel options, animation and zoom state, etc.
  // (QtDisplayPanelGui adds some gui/window state to this, and has
  // file-saving interface).
  // If a restorefilename where you intend to store the state is given, it
  // will be set as attribute 'original-path' of the root element.  This
  // allows data files to be restored relative to the restore file location.
  
  QDomDocument restoredoc;
  
  QtXmlRecord xr;	// (silly: QtXmlRecord should be a static class,
			//  and doesn't need to inherit from Record...)
  
  
  QDomElement restoreElem = restoredoc.createElement(v_->cvRestoreID.chars());
  restoredoc.appendChild(restoreElem);
  restoreElem.setAttribute("version", "0");
  if(restorefilename!="") restoreElem.setAttribute("original-path",
						   restorefilename.chars());
  
  
  
  //DD OPTIONS
  
  QDomElement ddopts = restoredoc.createElement("dd-options");
  restoreElem.appendChild(ddopts);
  
  // For each registered dd...
  
  List<QtDisplayData*> DDs = registeredDDs();
  for(ListIter<QtDisplayData*> dds(DDs); !dds.atEnd(); dds++) {
    QtDisplayData* dd = dds.getRight();
    
    // ...Create element for the dd's options (ddelem - tagName "dd")
    
    Record ddrec = dd->getOptions();
    
    if(ddrec.isDefined("region")) ddrec.removeField("region");
    if(ddrec.isDefined("mask")) ddrec.removeField("mask");
	// (region and mask interface elements not supported
	// at present -- don't propagate them...)

    
    QDomDocument dddoc;
    
    xr.recordToDom(&ddrec, dddoc);
    
    QDomElement ddelem = dddoc.documentElement();
    
    ddopts.appendChild(ddelem);
    ddelem.setTagName("dd");
    
    ddelem.setAttribute("path", dd->path().chars());
    ddelem.setAttribute("dataType", dd->dataType().chars());
    ddelem.setAttribute("displayType", dd->displayType().chars());     
    
    for (QDomElement optgrp = ddelem.firstChildElement(); !optgrp.isNull(); 
                     optgrp = optgrp.nextSiblingElement()) {
      for (QDomElement  opt = optgrp.firstChildElement(); !opt.isNull(); 
                        opt = opt.nextSiblingElement()) {
      
        // Store current color adjustments to the dd's colormap, if any.
	if(opt.tagName()=="colormap" && dd->hasColormap()) {
	  Vector<Float> shiftSlope, brtCont;
	  dd->getCMShiftSlope(shiftSlope);
	  dd->getCMBrtCont(brtCont);
	  opt.setAttribute("shiftslope", v_->toString(shiftSlope).chars());
	  opt.setAttribute("brtcont", v_->toString(brtCont).chars());  }

        
	// These option fields are not processed during restore at present;
        // they are removed here simply to reduce clutter when reading
        // the xml.  They should be able to be reinstated without harm if
        // found useful in the future.  (Also, there may still be more
        // fields that can be removed -- but use caution, and check first).
      
        opt.removeAttribute("dependency_list");
        opt.removeAttribute("dependency_type");
        opt.removeAttribute("dependency_group");
        opt.removeAttribute("allowunset");
        opt.removeAttribute("editable");
        opt.removeAttribute("provideentry");
        opt.removeAttribute("help");
        opt.removeAttribute("nperline");  }  }  }  
  
  
  
  // DISPLAY PANEL OPTIONS -- (Margins, number of panels,
  // background color, etc.)
  
  Record dpOptsRec = getOptions();
    
  QDomDocument dpOptsDoc;
  xr.recordToDom(&dpOptsRec, dpOptsDoc);
  QDomElement dpOptsElem = dpOptsDoc.documentElement();
  dpOptsElem.setTagName ("dp-options");
  
  restoreElem.appendChild(dpOptsElem);

  
  
  // SAVE DISPLAY PANEL SETTINGS -- (not from an options record:
  // animation, zoom, etc.)

  QDomElement dpSettings = restoredoc.createElement("dp-settings");
  restoreElem.appendChild(dpSettings);
  
  
  // Zoom state.  Preferable to get it from wc [0] than zoomer (God
  // knows what obsolete rubbishy state that may be in).  (Zoomer will
  // be used to _restore_ zoom state though...).
  
  ListIter<WorldCanvas* >* wcs = pd_->wcs();
	// (Who exposed this?  Convenient
	// here, certainly, (but wtf?...)).
  wcs->toStart();
  if(!wcs->atEnd()) {	// (Shouldn't be at end...).
    
    WorldCanvas* wc = wcs->getRight();
    
    QDomElement zoom = restoredoc.createElement("zoom");
    dpSettings.appendChild(zoom);
    
    Vector<Double> blc(2), trc(2);
    blc[0] = wc->linXMin(); blc[1] = wc->linYMin();
    trc[0] = wc->linXMax(); trc[1] = wc->linYMax();

    zoom.setAttribute("blc", v_->toString(blc).chars());
    zoom.setAttribute("trc", v_->toString(trc).chars());  }

  
  // Animation state.  Mode and current frame numbers, animation rate.
  // (To do later, when implemented: start, step, end frames).
  
  QDomElement anim = restoredoc.createElement("animator");
  dpSettings.appendChild(anim);
  
  anim.setAttribute("mode", mode().chars());
  anim.setAttribute("zindex", zIndex());
  anim.setAttribute("bindex", bIndex());
  anim.setAttribute("animrate", animRate());
    
  
  
  // Signal gui to add gui-specific settings -- the gui can process
  // this signal by adding more to restoredoc.
  
  emit creatingRstrDoc(&restoredoc);
  
  
  return restoredoc.toString().toStdString();  }
  
  
  


Bool QtDisplayPanel::setPanelState(QDomDocument& restoredoc,
				   QString restoredir) {
  // [re]set panel state from a QDomDocument.
  // Set restoredir to the directory of the restore file if available,
  // to enable data files to be found by restorefile-relative location.

  QDomElement restoreElem = 
              restoredoc.firstChildElement(v_->cvRestoreID.chars());
  if(restoreElem.isNull()) return False;
  
  QString origrestorefile = restoreElem.attribute("original-path");
  
  QtXmlRecord xr;
  
  hold();

  
  
  // restore DDs and their options
  
  QDomElement ddopts = restoreElem.firstChildElement("dd-options");
  
  if(!ddopts.isNull()) {
  
    // Begin by unregistering all existing DDs; only those prescribed
    // by the restoredoc are subsequently registered (in correct order).
    
    unregisterAll();
    
    // exDDs is a List of existing DDs which may be suitable for registering
    // and reusing in this restore.  Only those not currently registered on
    // _any_ panel (not just this one) are eligible.
    // (That is the distinction between ViewerBase::unregisteredDDs() vs.
    // DisplayPanel::unregisteredDDs(), btw).
    List<QtDisplayData*> exDDs = v_->unregisteredDDs();
    ListIter<QtDisplayData*> exdds(exDDs);
      
    
    // For each dd in the restore doc...
    for (QDomElement  ddelem = ddopts.firstChildElement(); !ddelem.isNull(); 
                      ddelem = ddelem.nextSiblingElement()) {
     
      String     path = ddelem.attribute("path",        "#").toStdString(),
             dataType = ddelem.attribute("dataType",    "#").toStdString(),
          displayType = ddelem.attribute("displayType", "#").toStdString();
    
    
      if(path=="#" || dataType=="#" || displayType=="#") continue;
				// nonexistent attribute (shouldn't happen)



      // Try to find the appropriate image/dd for this ddelem.
      
      QtDisplayData *dd=0;
      
      // First, see if the existing DD candidates are appropriate,
      for(exdds.toStart(); !exdds.atEnd(); exdds++) {
        QtDisplayData *exdd = exdds.getRight();
	if(exdd->dataType()   != dataType ||
	   exdd->displayType()!= displayType) continue;
			// (type mismatch -- try next existing dd).
	
	if(ddFileMatch_(path, dataType, displayType,  exdd,
			origrestorefile, restoredir)) {
	  dd = exdd;		// match found - reuse existing dd.
	  exdds.removeRight();	// (do not use exdd for further searches).
	  break;  }  }

      
      if(dd==0) {
        
	// No existing dd will do.  Try to find/create dd from the filesystem.
      
	if(!ddFileMatch_(path, dataType, displayType,  dd,
			 origrestorefile, restoredir)) {

	  cerr<<"**Unable to restore "<<path<<"**"<<endl;

	  continue;  }  }	// (...to next ddelem in rstr file -- 
				//  NB: _outer_ for loop).
        
      
      // An existing dd was found, or a new one was successfully created.
      // It is not registered yet, though; register it here (only).

      registerDD(dd);
      

      
      // Restore option settings to this dd.

      Record opts;
      
      try {

	xr.optsElemToRecord(ddelem, opts);

	dd->setOptions(opts, True);
		// 'True' assures that all options in opts
		// are emitted to update the gui as well.


	// Set stored color adjustments onto dd's colormap, if any.
	
	if(dd->hasColormap()) {
	  QDomElement cmopt = ddelem.elementsByTagName("colormap")
				    .item(0).toElement();
	  String shSl = cmopt.attribute("shiftslope", "#").toStdString(),
	        brCnt = cmopt.attribute("brtcont",    "#").toStdString();

	  if(shSl!="#")  dd->setCMShiftSlope(v_->toVectorF(shSl));
	  if(brCnt!="#") dd->setCMBrtCont(v_->toVectorF(brCnt));  }  }

      catch(...) {  }   }  }
  
  
  
  
  // RESTORE PANEL OPTIONS
  
  QDomElement dpOptsElem = restoreElem.firstChildElement("dp-options");
  
  if(!dpOptsElem.isNull()) {
      
    try {

      Record dpOptsRec;
      xr.optsElemToRecord(dpOptsElem, dpOptsRec);
      setOptions(dpOptsRec, True);  }
  
      catch(...) {  }  }
  
  
  
  
  // RESTORE (NON-GUI-OPTION) PANEL SETTINGS (zoom, animation...)

  QDomElement dpSettings = restoreElem.firstChildElement("dp-settings");
  
  
  // Restore Zoom
  
  QDomElement zoom = dpSettings.firstChildElement("zoom");
  
  String blcstr = zoom.attribute("blc").toStdString(),
         trcstr = zoom.attribute("trc").toStdString();
  
  Bool b_ok = False, t_ok=False;
  Vector<Double> blc = v_->toVectorD(blcstr, &b_ok),
		 trc = v_->toVectorD(trcstr, &t_ok);
  
  if(b_ok && t_ok  &&  blc.size()==2u && trc.size()==2u) {
    
    zoom_->zoom(blc, trc);  }

    
  // Restore Animation state
  
  QDomElement anim = dpSettings.firstChildElement("animator");
  
  QString md = anim.attribute("mode", "#"),
          zi = anim.attribute("zindex", "#"),
          bi = anim.attribute("bindex", "#"),
	  rt = anim.attribute("animrate", "#");
  
  Bool ok = False;
  
  Int zind = zi.toInt(&ok);  if(ok) goToZ(zind);  
  Int bind = bi.toInt(&ok);  if(ok) goToB(bind);  
  Int rate = rt.toInt(&ok);  if(ok) setRate(rate);  
  
  if(md!="#") setMode(md.toStdString());
  
  
  // mouse tool state (button assignments, at least) (?)
  
    // (to implement)  
  
  
  // Signal Gui, which can process relevant elements of restoredoc to
  // restore its own state (e.g. window size, dockWidget status, etc.).
  
  emit restoring(&restoredoc);
  
  
  
  release();
    
  return True;  }

  

 
Bool QtDisplayPanel::savePanelState(String filename, Bool overwrite) {
  // Save panel state to a file (as xml).  State is also stored to an
  // internal lastSavedState_ String (only there, if filename=="").
  // When overwrite is True (default) the file is overwritten if it exists
  // (and is writable).
  
  lastSavedState_ = dpState(filename);	// Save to 'clipboard' in any case.
  if(filename=="") return True;
  
  QFileInfo fi(filename.chars());
  if( fi.exists() && (!overwrite || !fi.isFile()) ) return False;

  QFile fl(filename.chars());
  
  if(!fl.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
    return False;  }
  
  QString state = lastSavedState_.chars();
  QTextStream os(&fl);
  os<<state; os.flush();
  
  fl.close();

  return True;  }


  
  
Bool QtDisplayPanel::setPanelState(String xmlState) {
  // [re]set panel state from xml String.

  if(xmlState=="") xmlState = lastSavedState_;
  QDomDocument restoredoc;
  if(!restoredoc.setContent(QString(xmlState.chars()))) return False;
  
  return setPanelState(restoredoc);  }


  
Bool QtDisplayPanel::restorePanelState(String filename) {
  // Restore panel state from a file,
  // or from lastSavedState_ if filename=="" (default).
  
  if(filename=="") return setPanelState();	// from 'clipboard'
  
  QDomDocument restoredoc;
  
  if(!v_->isRestoreFile(filename, restoredoc)) return False;
  
  QFileInfo fi(filename.chars());
  QString restoredir = fi.dir().path();
  
  return setPanelState(restoredoc, restoredir);  }
  



Bool QtDisplayPanel::ddFileMatch_(String path, String dataType,
                           String displayType,  QtDisplayData*& dd,
			   QString origrestorefile, QString restoredir) {
  // Tries to find existing DD (or create one from a file) that matches
  // the given path.  Used for restoring DDs from a restore file.
  // Several directories are checked, not just the one in path, to
  // provide flexibility.  If dd is non-zero, it is tested for suitability
  // to path, datatype and displaytype.  If dd is zero, the filesystem
  // is searched and an attempt is made to create dd from a matching file
  // (if any); if creation succeeds, dd will contain the new DD.
  // (Note: the DD will not be registered automatically).
  // origrestorefile and (current) restoredir aid in locating data files
  // relative to the current restore file location.

  
  
  QFileInfo ddfi;	// (for existing dd case)
  
  if(dd!=0) {			// (existing dd)
    
    if(dataType!=dd->dataType() || displayType!=dd->displayType()) {
      return False;  }		// type mismatch.
    
    if(dataType=="lel")  return path==dd->path();
	// Reuse lel dd iff lel strings match exactly; no directory
	// search is feasible at present.
    
    ddfi.setFile(dd->path().chars());  }
	// for file comparisons below.
    
  
  // path as passed in from restore file will be tested as-is, then
  // altered to check 4 alternative locations.  For an existing dd,
  // the test is whether the two paths refer to the same file.  If
  // no dd was passed in, the test is whether the file exists and can
  // be successfully used to create the desired type of dd.
  
  Bool noAutoReg = False;	// No automatic registration of
				// any newly-created dd.
  
  if(dataType=="lel") {
    dd = v_->createDD(path, dataType, displayType, noAutoReg);
    return (dd!=0);  }	// We must find all data files in the exact location
			// specified in lel string to reconstruct an lel dd.

  QFileInfo pathfi(path.chars());
  QString filename = pathfi.fileName();
  QString dmDir = v_->selectedDMDir.chars();
  
  QFileInfo testfi;	// (will hold representation of the 5
			//  filepaths below, in succession).
  
  // We will try to find/match data in the following locations, in order:
  // 1) the original path, exactly as stored.
  // 2) the path's filename, in the cwd.
  // 3) the path's filename, in the DataManager's current directory (if any).
  // 4) the path's filename, in the directory of the restore file.
  // 5) a path that will match if both data and restore file have been
  //    moved while maintaining their relative positions.
  
  for(Int loc=1; loc<=5; loc++) {
    
    Bool testok = True;
    
    switch(loc) {
      
      case 1: {
        testfi = pathfi;
        break;  }
      
      case 2: {
        testfi.setFile(QDir::current(), filename);
        break;  }
      
      case 3: {
        if(dmDir=="") { testok = False; break;  }
        testfi.setFile(QDir(dmDir), filename);
        break;  }
      
      case 4: {
        if(restoredir=="") { testok = False; break;  }
        testfi.setFile(restoredir, filename);
        break;  }
      
      case 5: {
        if(origrestorefile=="" || restoredir=="") { testok = False; break;  }
        QDir origdir(QFileInfo(origrestorefile).dir());
        QString relpath = origdir.relativeFilePath(pathfi.filePath());
		// original data file path relative to original restore dir...
	testfi.setFile(QDir(restoredir), relpath);
		// ...above appended to current restoredir.
        break;  }  }
          
    
    if(!testok) continue;
    
    
    if(dd!=0) {				// Existing dd
    
      if(ddfi==testfi) return True;  }	// existing dd matches test path.
      
    
    else if(testfi.exists()) {
      
      // No existing dd, but candidate file found:
      // see if we can create DD from it.

      String testpath = QDir::cleanPath(testfi.filePath()).toStdString();
      
      dd = v_->createDD(testpath, dataType, displayType, noAutoReg);
      
      if(dd!=0) return True;  }  }

        
  return False;  }	// No matching file or dd.
  
  
  

String QtDisplayPanel::suggestedRestoreFilename() {
  // Suggest name for restore file (based on first-registered DD).
  
  String filename = "viewer";
  List<QtDisplayData*> DDs(registeredDDs());
  if(DDs.len()>0) {
    ListIter<QtDisplayData*> dds(DDs);
    QtDisplayData* dd = dds.getRight();
    if(dd->dataType()=="lel") filename = "lel";
    else {
      QFileInfo fi(dd->path().chars());
      String nm = fi.fileName().toStdString();
      if(nm!="") filename = nm;  }  }
  
  return filename+"."+v_->cvRestoreFileExt;  }
  

  

// METHODS USED IN PRINTING


void QtDisplayPanel::setLineWidthPS(Float &w) {

    for (ListIter<QtDisplayData*> qdds(&colorBarDDsToDisplay_); 
          !qdds.atEnd(); qdds++) {
         QtDisplayData* pdd = qdds.getRight();
         if (pdd != 0) { 
            Record wl, chgdOpts;
            wl.define("wedgelabellinewidth", w);
            pdd->colorBar()->setOptions(wl, chgdOpts);
            break;
         }
    }
    List<QtDisplayData*> rdds = registeredDDs();
    for (ListIter<QtDisplayData*> qdds(&rdds); !qdds.atEnd(); qdds++) {
         QtDisplayData* pdd = qdds.getRight();
         PanelDisplay* ppd = panelDisplay();
         if (ppd != 0 && pdd != 0 && ppd->isCSmaster(pdd->dd())) { 
            Float lw = 2.;
            Record rec = pdd->getOptions();
            try {
               lw = rec.subRecord("labellinewidth").asFloat("value");
            }
            catch(...) {
            }
            if (w < 0.1) w = 0.1; 
            Record pl, chgdOpts;
            pl.define("labellinewidth", w);
            pdd->dd()->setOptions(pl, chgdOpts);
            w = lw;
            break; 
         }
    }
}



void QtDisplayPanel::setBackgroundPS(String &bg, String &fg) {
    String bs = pc_->deviceBackgroundColor();
    String fs = pc_->deviceForegroundColor(); 
    pc_->setDeviceBackgroundColor(bg);
    pc_->setDeviceForegroundColor(fg);
    bg = bs;
    fg = fs;
}




// MISC.


void QtDisplayPanel::setShapeManager(QtRegionShapeManager* manager) {
  // (best not to assume this routine has been called though, 
  //  i.e., that qsm_ has been set....)
  qsm_ = manager;  }

void QtDisplayPanel::extendRegion(
     String chans, String pols){
  extChan_ = chans;
  extPol_ = pols;
}

void QtDisplayPanel::clicked(Record rec) {
  emit activate(rec);
}


} //# NAMESPACE CASA - END

