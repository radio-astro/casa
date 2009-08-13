//# QtCleanPanelGui.qo.h:  interactive clean display panel
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

#ifndef QTCLEANPANELGUI_H_
#define QTCLEANPANELGUI_H_

#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/Record.h>
#include <display/QtViewer/QtDisplayPanelGui.qo.h>
#include <casa/Arrays/Vector.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>

#include <graphics/X11/X_enter.h>
#  include <QObject>
#  include <QGroupBox>
#  include <QRadioButton>
#  include <QPushButton>
#  include <QHBoxLayout>
#  include <QVBoxLayout>
#  include <QLineEdit>
#include <graphics/X11/X_exit.h>


namespace casa {

class QtViewer;
class QtDisplayPanel;
class QtDisplayData;
class ImageRegion;
class WorldCanvasHolder;

// <synopsis>
// Demo class to encapsulate 'serial' running of qtviewer into callable
// methods of a class; this example also applies it to the task of
// interactive selection of CLEAN boxes.
// </synopsis>
class QtCleanPanelGui: public QtDisplayPanelGui {

  Q_OBJECT	//# Allows slot/signal definition.  Must only occur in
		//# implement/.../*.h files; also, makefile must include
		//# name of this file in 'mocs' section.
  
 
 public: 
 
  QtCleanPanelGui( QtViewer *v, QWidget *parent=0 );
  
  ~QtCleanPanelGui();
  
  // prototype: the CleanBoxes type will probably change
  typedef Vector<String> CleanBoxes;

#if 0 
  // returns True if it was able to find and display the image.
  virtual Bool loadImage(String imgname, String maskname);
  
  virtual Bool imageLoaded() { return imagedd_!=0;  }
  
  // start viewer display; return when viewer windows are closed.
  // Return value indicates event loop (exec) return status..
  virtual Int go(Int& niter, Int& ncycle, String& threshold);

  virtual Int go();

  //return/get the state of buttons on the viewer
  virtual void getButtonState(Record& state);
  virtual void setButtonState(const Record& state);


  virtual CleanBoxes cleanBoxes() { return boxes_;  }
  
 
 
 public slots: 
 
  // delete old dd (and clean boxes), if any.
  virtual void clearImage();

  // delete accumulated clean boxes, if any.
  virtual void clearCleanBoxes() { boxes_.resize(0);  }
  
  
 
 protected slots:
 
  // Connected to the rectangle region mouse tools new rectangle signal.
  // Accumulates [/ displays] selected boxes.
  virtual void newMouseRegion_(Record mouseRegion, WorldCanvasHolder* wch);

  virtual void exitLoop();
  virtual void exitStop();
  virtual void exitDone();
  virtual void exitNoMore();
 
 protected:

 void getPlaneBoxRegion(const Vector<Double>& blc, const Vector<Double>& trc, 
			ImageRegion& planeReg);

 void writeRegionText(const ImageRegion& imageReg, const String& filename, Float value);
#endif
 protected: 
  CoordinateSystem csys_p;  
  DirectionCoordinate dirCoord_p;
  String imgname_;
  CleanBoxes boxes_;	// accumulated clean boxes.
  
  QtDisplayData* imagedd_;
  QtDisplayData* maskdd_;	// later: to display clean region.

  Int niter_p, ncycles_p;
  String thresh_p;

 
  QGroupBox* clnGB_;
  QGroupBox* chanGB_;
  QGroupBox* maskGB_;
  QGroupBox* stopGB_;
  QGroupBox* niterGB_;
  QRadioButton* addRB_;
  QRadioButton* eraseRB_;
  QRadioButton* allChanRB_;
  QRadioButton* thisPlaneRB_;
  QPushButton* maskNoMorePB_;
  QPushButton* maskDonePB_;
  QPushButton* stopPB_;
  QLineEdit* niterED_;
  QLineEdit* ncyclesED_;
  QLineEdit* threshED_;
  Record buttonState_;

  Int retVal_p;

};


} //# NAMESPACE CASA - END

#endif


