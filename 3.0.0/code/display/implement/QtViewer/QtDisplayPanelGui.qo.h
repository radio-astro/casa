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
   //#dk Be careful to put *.ui.h within X_enter/exit bracket too,
   //#   because they'll have Qt includes.
   //#   E.g. <QApplication> needs the X11 definition of 'Display'
#  include <display/QtViewer/QtAnimatorGui.ui.h>
#include <graphics/X11/X_exit.h>
#include <casaqt/QtUtilities/QtPanelBase.qo.h>
#include <display/QtViewer/QtDisplayPanel.qo.h>


namespace casa { //# NAMESPACE CASA - BEGIN

class String;
class QtViewer;
class QtViewerPrintGui;
class QtMouseToolBar;
class QtCanvasManager;
class QtAnnotatorGui;
class MakeMask;
class MakeRegion;
class QtProfile;
class QtDisplayData;
class TrackBox;
class QtRegionManager;
class QtRegionShapeManager;

// <summary>
// The main display window for the Qt version of the viewer.
// </summary>

class QtDisplayPanelGui : public QtPanelBase,
		          protected Ui::QtAnimatorGui {

  Q_OBJECT;	//# Allows slot/signal definition.  Must only occur in
		//# implement/.../*.h files; also, makefile must include
		//# name of this file in 'mocs' section.

 public:
  enum SCRIPTING_OPTION { INTERACT, SETOPTIONS };

  QtDisplayPanelGui(QtViewer* v, QWidget* parent=0);
  ~QtDisplayPanelGui();
  
  // access to graphics panel 'base'....
  QtDisplayPanel* displayPanel() { return qdp_;  }
  
  // public toolbars, for inserting custom buttons.
  QToolBar* customToolBar;	//# limited room
  QToolBar* customToolBar2;	//# wider -- in its own row.

  virtual void setStatsPrint(Bool printStats=True) {
    qdp_->printStats = printStats;  }

  virtual bool supports( SCRIPTING_OPTION option ) const;
  virtual QVariant start_interact( const QVariant &input, int id );
  virtual QVariant setoptions( const QMap<QString,QVariant> &input, int id);

  // the QtDBusViewerAdaptor can handle loading & registering data itself,
  // but to connect up extra functionality, the upper-level QtDisplayPanelGui
  // (or in the current case, the derived QtCleanPanelGui) would have to be
  // notified that data has been added. This will allow it to set up the
  // callbacks for drawing regions...
  virtual void addedData( QString type, QtDisplayData * );
 
 public slots:
 
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

  virtual void showMakeRegionPanel();
  virtual void hideMakeRegionPanel();
 
  virtual void showImageProfile();
  virtual void hideImageProfile();
  
  virtual void hideAllSubwindows();
  //</group>
 
 
  // (Attempts to) restore panel state from named file.
  virtual Bool restorePanelState(String filename);
 
 
 protected slots:
  

  //# overrides of base QMainWindow slots
  
  void hideEvent(QHideEvent* ev) {
    // Note: If the display panel is iconified or the user changes
    // desktops, isVisible() will remain true here (surprising, but
    // useful -- see isVisible() doc).  Otherwise, in this context,
    // [I believe] we can assume the display panel has been _closed_;
    // in that case, we want to 'close' the auxiliary windows as well.
    // (Note that there is no 'closeEvent' per se).
    if(!isVisible()) hideAllSubwindows();
    
    QMainWindow::hideEvent(ev);  }
 
  
  //# purely internal slots
 
  virtual void toggleAnimExtras_();
  virtual void setAnimExtrasVisibility_();  
  
  virtual void frameNumberEdited_();
  
  
  //# slots reacting to signals from the basic QtDisplayPanel.
  //# Protected, connected by this object itself.
  
  // Respond to QDP::registrationChange() signal
  virtual void ddRegChange_() {
    updateDDMenus_();
    arrangeTrackBoxes_();  }

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
  virtual void fwdPlay_() {
    if(qdp_->animating()>0) qdp_->stop(); else qdp_->fwdPlay();  }
  
  virtual void revPlay_() {
    if(qdp_->animating()<0) qdp_->stop(); else qdp_->revPlay();  }
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
    
  // Keeps current data directory in sync between
  // DataManager window and save-restore dialogs.
  virtual Bool syncDataDir_(String filename);
  
  // return the tracking box for a QDD (given pointer or name).  0 if none.
  //<group>
  virtual TrackBox* trkBox_(QtDisplayData* qdd);
  virtual TrackBox* trkBox_(String ddname);
  //</group>
  
  
  // Does the display panel have a TrackBox for this panel (yet)?
  virtual Bool hasTrackBox_(QtDisplayData* qdd) { return trkBox_(qdd)!=0;  }
  
  
  // If qdd->usesTracking(), this method assures that a TrackBox for qdd
  // is visible in the trkgWidget_'s layout (creating the TrackBox if it
  // didn't exist).  Used by arrangeTrackBoxes_().  Returns the TrackBox
  // (or 0 if none, i.e., if !qdd->usesTracking()).
  virtual TrackBox* showTrackBox_(QtDisplayData* qdd);
 


  
  virtual void updateDDMenus_(Bool doCloseMenu = True);
  
  
  
  
  //# ----------------------------DATA----------------------------------
  
  
  QtViewer* v_;		 	//# (Same viewer as qdp_'s)
  QtDisplayPanel* qdp_;  	//# Central Widget this window operates.
  QtViewerPrintGui* qpm_;	//# Print dialog for this display panel.
  QtCanvasManager* qcm_;	//# display panel options window.
  //QtAnnotatorGui* qap_;
  MakeMask* qap_;
  MakeRegion* qmr_;
  QtRegionManager* qrm_;      //# Region manager window.
  QtRegionShapeManager* qsm_; //# Region shape manager window.
  
  QtProfile* profile_;		//# Profile window
  String savedTool_;		//# (for restoring left button)
  QtDisplayData* profileDD_;    //# QDD currently being profiled
                                //# (0 if profiler is not showing).

  //# GUI LAYOUT  

  QMenu *dpMenu_, *ddMenu_, *ddRegMenu_, *ddCloseMenu_, *tlMenu_, *vwMenu_;
  
  QAction *dpNewAct_, *printAct_, *dpOptsAct_, *dpCloseAct_, *dpQuitAct_,
	  *ddOpenAct_, *ddAdjAct_, *ddRegAct_, *ddCloseAct_, *unzoomAct_, 
	  *zoomInAct_, *zoomOutAct_, *annotAct_, *mkRgnAct_,
          *profileAct_, *rgnMgrAct_, *shpMgrAct_, *dpSaveAct_, *dpRstrAct_; 
  
  QToolBar* mainToolBar_;
  QToolButton *ddRegBtn_, *ddCloseBtn_;

  QtMouseToolBar* mouseToolBar_;
  
  QDockWidget*  animDockWidget_;
  QFrame*       animWidget_;  // Ui::QtAnimatorGui populates this.
  
  QDockWidget*  trkgDockWidget_;
  QWidget*    trkgWidget_;
  
     
 private:
  
  QtDisplayPanelGui() {  }		// (not intended for use)  
    
};




// <summary>
// Helper class for QtDisplayPanelGui, for display of tracking information.
// </summary>

// <synopsis>
// TrackBox is the widget for the position the tracking information of
// a single QtDisplayData within a QtDisplayPanelGui.  trkgWidget_ will
// show a TrackBox for each registered QDD capable of displaying tracking
// information, in registration order.  TrackBox is simply a QGroupBox with
// a QTextEdit inside it.  The QGroupBox displays the QDD's name and has
// a checkbox that can be used to hide the tracking text area to save
// space.  TrackBox is intended to be used exclusively by QtDisplayPanelGui.
// </synopsis>
  
class TrackBox : public QGroupBox {
   
 Q_OBJECT;
   
 public:
   
  TrackBox(QtDisplayData* qdd, QWidget* parent=0);
  
  void setText(String trkgString);
  void clear() { trkgEdit_->clear();  }
  QtDisplayData* dd() { return qdd_;  }
  String name() { return objectName().toStdString();  }
  
 protected:
   
  // Attempts automatic adjustment of tracking display height
  // according to contents.
  void setTrackingHeight_();
    
  QtDisplayData* qdd_;	// (the QDD whose tracking info it displays).
  QTextEdit*    trkgEdit_;	// (the box's tracking info display area).
    
 private:
  
  TrackBox() {  }		// (not intended for use)  

};

  


} //# NAMESPACE CASA - END

#endif
