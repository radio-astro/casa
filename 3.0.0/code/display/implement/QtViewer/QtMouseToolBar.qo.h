//# QtMouseToolBar.qo.h: 'mouse-tool' toolbar for qtviewer display panel.
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


#ifndef QTMOUSETOOLBAR_H
#define QTMOUSETOOLBAR_H

#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <casa/Arrays/Vector.h>
#include <display/QtViewer/QtMouseToolState.qo.h>


#include <graphics/X11/X_enter.h>
#  include <QMouseEvent>
#  include <QContextMenuEvent>
#  include <QToolBar>
#  include <QToolButton>
#include <graphics/X11/X_exit.h>

namespace casa {

class QtDisplayPanel;



class QtMouseToolButton: public QToolButton {
  
  Q_OBJECT	//# Allows slot/signal definition.  Must only occur in
		//# implement/.../*.h files; also, makefile must include
		//# name of this file in 'mocs' section.

 public: 
 
  QtMouseToolButton(QWidget* parent = 0) : QToolButton(parent) {  }
  ~QtMouseToolButton() {  }
  
 protected:
  
  void mousePressEvent(QMouseEvent *event) {
    Int btn = (event->button() == Qt::LeftButton)?   1 :
              (event->button() == Qt::MidButton)?    2 :
              (event->button() == Qt::RightButton)?  3 :   0;
    if(btn!=0) emit mouseToolBtnPress(text().toStdString(), btn);  }

  void mouseMoveEvent   (QMouseEvent *event) {  }
  void mouseReleaseEvent(QMouseEvent *event) {  }
  
  // (Prevents interference with right mouse button).
  void contextMenuEvent(QContextMenuEvent *event) { event->accept();  }
  
 signals:
 
  void mouseToolBtnPress(String tool, Int btn);

};







class QtMouseToolBar: public QToolBar {
  
  Q_OBJECT	//# Allows slot/signal definition.  Must only occur in
		//# implement/.../*.h files; also, makefile must include
		//# name of this file in 'mocs' section.


 public: 
 
  QtMouseToolBar(QtMouseToolState* msbtns, QtDisplayPanel* qdp = 0,
		 QWidget* parent = 0);
  ~QtMouseToolBar() {  }


 
 protected slots:
  
  // Connected to the QtMouseToolState::mouseBtnChg() signal.  Changes the
  // tool button's (QAction's) state (icon, whether checked), to reflect
  // the [new] mouse button assignment for a given mouse tool.
  virtual void chgMouseBtn_(String tool, Int button);
  
  // Overridden from QToolBar, responding to clicks from any mouse button.
  // Relays that mouse button to central registry for assignment to 
  // desired tool.
  //void mouseReleaseEvent(QMouseEvent *event);

 
 protected:
 
  // Holds button state for all mouse-tool bars and display panels.
  QtMouseToolState* msbtns_;
  
  // Names of tools within this toolbar (in order)
  Vector<String> tools_;
  
};


} //# NAMESPACE CASA - END

#endif

