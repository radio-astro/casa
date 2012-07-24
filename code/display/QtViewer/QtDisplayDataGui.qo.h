//# QtDisplayDataGui.qo.h: Qt options widget for single DisplayData
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

#ifndef QTDisplayDataGui_H
#define QTDisplayDataGui_H

#include <casa/aips.h>
#include <display/QtAutoGui/QtAutoGui.qo.h>
#include <display/QtViewer/QtDisplayData.qo.h>
#include <casa/Containers/Record.h>

#include <graphics/X11/X_enter.h>
#  include <QDir>
#  include <QColor>
#  include <QHash>
#include <graphics/X11/X_exit.h>

 
namespace casa { //# NAMESPACE CASA - BEGIN

class QtViewer;

// <summary>
// Qt options widget for single DisplayData.
// </summary>

// <synopsis>
// QtDisplayDataGui is a thin layer over a QtAutoGui base, to
// provide the options widget for a single QtDisplayData.  It is normally
// in a tab of a QtDataOptionsPanel.  Unlike QtAutoGui, which is
// intended to be generic and reusable for other purposes, QtDisplayDataGui   
// knows about a QtDisplayData; QtDisplayDataGui makes the communication
// connections between its QtDisplayData and QtAutoGui.
// </synopsis>

//class QtDisplayDataGui : protected QtAutoGui {
class QtDisplayDataGui : public QtAutoGui {
  
  Q_OBJECT	//# Allows slot/signal definition.  Must only occur in
		//# implement/.../*.h files; also, makefile must include
		//# name of this file in 'mocs' section.
   
 public:
  
  // Always pass a valid QtDisplayData* when creating QtDisplayDataGui,
  // and use it only as long as the QtDisplayData* remains valid
  QtDisplayDataGui(QtDisplayData* qdd) : QtAutoGui(), qdd_(qdd) {
  
    setFileName((qdd_->name()+".opts").c_str());
    
    loadRecord(qdd_->getOptions());	//# populate gui
    
    //# set up communication between gui and qdd_.
    
/*     
    QtAutoGui* thisAutoGui = this;
      
    connect( thisAutoGui, SIGNAL(setOptions(Record)),
             qdd_,        SLOT(setOptions(Record)) );
    connect( qdd_,         SIGNAL(optionsChanged(Record)),
             thisAutoGui,  SLOT(changeOptions(Record)) );  }
*/  
    
   
    connect( this, SIGNAL(setOptions(Record)),
             qdd_,   SLOT(setOptions(Record)) );
    //cerr<<" QDDG cn: this:"<<this<<" qdd:"<<qdd_<<endl;	//#diag

    connect( qdd_, SIGNAL(optionsChanged(Record)),
                     SLOT(changeOptions(Record)) );  }
 

    
  ~QtDisplayDataGui() {  }

   
 protected:
  
  QtDisplayData* qdd_;
	//# (QtDisplayDataGui is not responsible for destroying qdd_).
  
  // Not to be used; QtDisplayDataGui must be created with a valid
  // QtDisplayData*.
  QtDisplayDataGui() {  }
  
};

} //# NAMESPACE CASA - END

#endif
