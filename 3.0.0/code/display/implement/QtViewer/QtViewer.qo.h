//# QtViewer.qo.h: Qt implementation of main viewer supervisory object
//#		 -- Gui level.
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

#ifndef QTVIEWER_H
#define QTVIEWER_H
#include <display/QtViewer/QtViewerBase.qo.h>

#include <graphics/X11/X_enter.h>
#  include <QObject>
#include <graphics/X11/X_exit.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class QtDataManager;
class QtDataOptionsPanel;
class QtDBusViewerAdaptor;
class QtDisplayPanelGui;

// <summary>
// Qt implementation of main viewer supervisory object -- Gui level.
// </summary>

// <synopsis>
// A QtViewer app should use just one QtViewer object, in all
// Probability; the rest of the objects will coordinate through it.
// 
// The viewer is structured with a functional layer and a gui layer.
// In principle the former can operate without the latter.
// This class contains and manages various _gui_ objects associated with
// the viewer.  Its base manages the corresponding functional objects.
//
//   Functional objects     Related Gui objects
//   ------------------     -------------------
//      QtViewerBase        QtViewer, QtDataManager, print dialog
//      QtDisplayData       QtDataOptionsPanel
//      QtDisplayPanel      QtDisplayPanelGui, panel options dialog
//
//      QtDisplayPanel _is_ also a display widget, but minimal,
//      without surrounding graphical interface (animator, menus, etc.)
//      to operate it.
//
// </synopsis>

class QtViewer : public QtViewerBase {

  Q_OBJECT	//# Allows slot/signal definition.  Must only occur in
		//# implement/.../*.h files; also, makefile must include
		//# name of this file in 'mocs' section.

 public:
  
  QtViewer( bool is_server=false );
  ~QtViewer();
  
  QtDataManager* dataMgr() { return qdm_;  }

  // name used to initialize connection to dbus
  static const QString &name( );

 public slots:
 
  virtual void showDataManager();
  virtual void hideDataManager();
 
  virtual void showDataOptionsPanel();
  virtual void hideDataOptionsPanel();
  
  virtual void hideAllSubwindows();
  
  // create a main display panel Gui 
  virtual QtDisplayPanelGui *createDPG();
   
  // Exits Qt loop.  (Note that the loop can be restarted (and is, in
  // interactive clean, e.g.), with existing widgets intact.  This
  // does not in itself delete objects or exit the process, although
  // the driver program might do that).  Also, some of the panels may
  // have WA_DeleteOnClose set, which would cause their deletion (see,
  // e.g., createDPG()).
  virtual void quit();
 
 
 signals:
 
  
 protected:
 
  QtDataManager* qdm_;		//# The window for loading data.
  QtDataOptionsPanel* qdo_;	//# The window for controlling data display.

  QtDBusViewerAdaptor* dbus_;
 
 public:
 
  // True by default.  Set False to disable auto-raise of the Data
  // Options panel whenever the first DD is created.
  //# Users want to see this panel automatically when there are DDs
  //# to tweak.  (Apps like clean can turn v_->autoOptionsRaise off,
  //# if desired (yes, is is (gasp!) public data)).
  Bool autoDDOptionsShow;
 
 private:
  static QString name_;
    
};



} //# NAMESPACE CASA - END

#endif
