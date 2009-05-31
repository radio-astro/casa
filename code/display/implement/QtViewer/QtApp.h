//# QtApp.h: Management of the QApp object needed by any Qt application.
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

#ifndef QTAPP_H
#define QTAPP_H

#include <casa/aips.h>

#include <graphics/X11/X_enter.h>
#  include <QApplication>
#  include <QThread>
#include <graphics/X11/X_exit.h>

namespace casa { //# NAMESPACE CASA - BEGIN


// <summary>
// Management of the QApp object needed by any Qt application.
// </summary>

// <synopsis>
// This adds just a little to QApplication's [static] services.  (Actually,
// I wish all this _were_ there instead -- it could be...).  Casa
// applications which use Qt can just call QtApp::app() to retrieve
// 'the' (unique) QApplication object for the program; that routine
// will create it if it doesn't yet exist.  It is recommended that all
// access by casa to the QApplication object go through this routine, to
// assure only one is created.
//#
//# This class has utility for _any_ Qt casa app, not just qtviewer.
//# Ultimately, it probably belongs outside in a different directory.
// </synopsis>
class QtApp {

 public:
  
  QtApp() { init();  }   //# (You may not need to create one, though:
  ~QtApp() {  }		 //#  everything's static).
  
  
  // Return the program's [unique] QApplication object, creating it
  // if it doesn't yet exist.
  // Note: use QtApp::destroy() to delete the QApplication.
  static QApplication* app( ) {

    static Int argc = 1;
    static Char *argv[1];
    static Char name[] = "casa.qtapp";
    argv[0] = name;

    QCoreApplication* qcapp = QApplication::instance();
    if(QApplication::startingUp() || qcapp==0) {
      qcapp = new QApplication(argc, argv);  }
    
    QApplication* qapp = dynamic_cast<QApplication*>(qcapp);
    
    if(qapp==0) {      //# This probably should not happen....
      //# Someone created a QCoreApplication which was not a full-fledged
      //# (i.e. gui-capable) QApplication, before calling this.  We want a
      //# full-monty QApplication.  The following may be marginally better
      //# than throwing/crashing if the caller is truly done with the old
      //# QCoreApp (s/he shouldn't have called this routine otherwise).
      delete qcapp; 
      qapp = new QApplication(argc, argv);  }
      
    return qapp;  }
  
  
  // Another name for app() that may be clearer during initialization....
  static QApplication* init( ) {
    return app( );  }

  
  // Enter the QApp's event loop.
  static Int exec() { return app()->exec();  }

  
  // Exit the QApp's event loop.
  static void exit(Int returnCode=0) { app()->exit(returnCode);  }

  
  // Call when completely finished with Qt, if you're a stickler for cleanup.
  static void destroy() { 
    if(!QApplication::startingUp()) delete QApplication::instance();  }
    
  
  // If True, a full-fledged QApplication has been created (though it
  // may not necessarily be executing its event loop).
  static Bool exists() { 
    return !QApplication::startingUp() && 
            dynamic_cast<QApplication*>(QApplication::instance())!=0;  }

  
  // Is the QApp executing its event loop? 
  // (In many cases, caller probably ought to know this already...).
  static Bool isInLoop() {
    return exists() && app()->thread()->isRunning();  }
	//# (Gleaned from QCoreApplication::exec() in qtapplication.cpp)

  
    
};



} //# NAMESPACE CASA - END

#endif
