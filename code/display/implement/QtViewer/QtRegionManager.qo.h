//# QtRegionManager.qo.h: Qt implementation of viewer region manager window.
//# (This is the gui part only; it connects to region functions within
//# QtDisplayPanel).
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

#ifndef QTREGIONMANAGER_H
#define QTREGIONMANAGER_H

#include <casa/aips.h>
#include <casa/Containers/List.h>
#include <display/QtViewer/QtDisplayPanel.qo.h>

#include <graphics/X11/X_enter.h>
#  include <QtCore>
#  include <QtGui>
   //#dk Be careful to put *.ui.h within X_enter/exit bracket too,
   //#   because they'll have Qt includes.
   //#   E.g. <QApplication> needs the X11 definition of 'Display'
#  include <display/QtViewer/QtRegionMgr.ui.h>
#include <graphics/X11/X_exit.h>

 
namespace casa { //# NAMESPACE CASA - BEGIN

class QtViewer;
class ImageRegion;
class Record;
template <class T> class PtrBlock;


class QtRegionManager : public QWidget, protected Ui::QtRegionMgr {

  Q_OBJECT

 
 public:
  
  // This gui receives signals from, and sends commands to qdp;
  // qdp must be valid.   parent is usually 0 though; QtRegionManager
  // is a stand-alone auxiliary window to QtDisplayPanelGui.
  QtRegionManager(QtDisplayPanel* qdp, QWidget* parent=0);
  
  ~QtRegionManager() {  }
  

 protected:

  
  
  // This gui receives signals from, and sends commands to this display panel.
  QtDisplayPanel* qdp_;
  
  // (Usually 0 -- standalone window).
  QWidget* parent_;
 
 
 protected slots:

  //update the regionames in the menu
  void updateNames();
  //remove region from image
  void removeRegion();
  // React to new region creation in display panel.
  void newRegion_(String imgFilename);
  
  // Save last-created region (reacts to Save button).
  void saveRegion_();
  void saveRegionInImage();
  // Reacts to extent button selection -- sets extent state in display panel.
  void setExtent_();

  // Reacts to change in pathname (just to re-enable Save button).
  void pathnameChg_();

  // Cleanup on destruction
  void cleanup();
  // Set up stuff to extend or not plane along channels and pol
  void singlePlane();
  void planeExtension();
  // Load region from ds9 or aipsbox or rgn file
  void loadRegionsFromFile();
  //draw region on viewer
  void drawRegion(Record mousereg, WorldCanvasHolder *wch);

 private:
  PtrBlock<const ImageRegion * >  unionRegions_p;
  List<RegionShape*> regShapes_p;
  
};

} //# NAMESPACE CASA - END

#endif
