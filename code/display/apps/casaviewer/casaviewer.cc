//# qtviewer.cc:  main program for standalone Qt viewer
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

#include <casa/aips.h>
#include <casa/iostream.h>
#include <casa/Inputs/Input.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/Record.h>
#include <casa/Exceptions/Error.h>
#include <signal.h>
#include <display/Display/StandAloneDisplayApp.h>
	// (Configures pgplot for stand-alone Display Library apps).

#include <display/QtViewer/QtDisplayData.qo.h>
#include <display/QtViewer/QtDisplayPanelGui.qo.h>
#include <display/QtViewer/QtViewer.qo.h>
#include <display/QtViewer/QtApp.h>

/*
#include <graphics/X11/X_enter.h>
#include   <QApplication>
#include <graphics/X11/X_exit.h>
*/


#include <casa/namespace.h>


int main( int argc, char **argv ) {

 // don't let ^C [from casapy] kill the viewer...
 signal(SIGINT,SIG_IGN);
	
	INITIALIZE_PGPLOT
 
 try {
  
  QApplication qapp(argc, argv, true); 
    
  String	   filename    = "",
		   displaytype = "",
		   datatype    = "",
		   arg2        = "",
		   arg3        = "";      
  

  Int narg;

#ifndef AIPS_DARWIN
  narg = qapp.argc();
  if(narg>1) filename = qapp.argv()[1];
  if(narg>2) arg2     = qapp.argv()[2];
  if(narg>3) arg3     = qapp.argv()[3];
#else
  narg = argc;
  if(narg>1) filename = argv[1];
  if(narg>2) arg2     = argv[2];
  if(narg>3) arg3     = argv[3];
#endif
  
  if(filename==".") filename="";
	// Workaround for python task's "empty parameter" disability....

  QtViewer* v = new QtViewer;
  
  QtDisplayPanelGui* dpg = new QtDisplayPanelGui(v);
  
  QtDisplayData* qdd = 0;

  // Data files are now typed automatically (see v_->filetype(filename),
  // below; e.g.: "image" or "ms").  arg2 need be used only to specify a
  // displaytype, and then only when it is not the default displaytype
  // for the datatype (e.g.  viewer "my.im", "contour" ).
  //
  // The user can enter an lel expression in place of filename, but such
  // an expression _cannot_ be automatically typed.  In this case the user
  // must have "lel" in arg2 (or in arg3: the only case where arg3 is vaguely
  // useful is something like:
  //
  //   casaviewer "'my.im'-'other.im'"  contour  lel 
  //
  // arg3 is not even offered in the viewer casapy task).
  //
  // The logic below allows displaytypes or datatypes to be entered in
  // either order, and for old datatypes to be used other than "lel" (these
  // are simply ignored).  This allows old (deprecated) parameter usage in
  // scripts (such as viewer("my.ms", "ms")) to continue to be understood.
  //
  // However, the canonical 'allowed' parameter set (per user documentation)
  // is now just:
  //
  //   viewer [filename [displaytype]]
  //
  
  if(filename!="") {
  
    Bool tryDDcreate = True;
    
    if(arg3=="lel" || arg2=="lel") {
      
      // (this means that first ('filename') parameter is supposed to
      // contain a valid lel (image expression) string; this is advanced
      // (and undocumented) parameter usage).
      
      datatype = "lel";
      displaytype = (arg3=="lel")? arg2 : arg3;
      v->dataDisplaysAs(datatype, displaytype);  }
      
    else {
      
      datatype = v->filetype(filename);
      
      
      if(datatype=="restore") {
	
	// filename is a restore file.
        
	tryDDcreate = False;
        
        dpg->restorePanelState(filename);  }
      
      else {
	
	if(datatype=="nonexistent") {
	  cerr << "***Can't find  " << filename << "***" << endl;
	  tryDDcreate = False;  }
      
	if(datatype=="unknown") {
	  cerr << "***Unknown file type for  " << filename << "***" << endl;
	  tryDDcreate = False;  }
      
	// filename names a normal data file.  If user has passed a valid
	// displaytype in either arg2 or arg3, use it; otherwise, the
	// default displaytype for datatype will be inserted.
        
	displaytype = arg2;
        if(!v->dataDisplaysAs(datatype, displaytype)) {
          displaytype = arg3;
          v->dataDisplaysAs(datatype, displaytype);  }  }  }
    
    
    if(tryDDcreate) {

          
      qdd = v->createDD(filename, datatype, displaytype);
  
      
      if(qdd==0)  cerr << v->errMsg() << endl;  }  }
    
      
  
  dpg->show();
  
  if(v->nDDs()==0) v->showDataManager();
  
  

  Int stat = QtApp::exec();
  
  //delete dpg;		// Used to lead to crash (double-deletion
  			// of MWCTools); should work now.
  
  delete v;
  
  // cerr<<"Normal exit -- status: "<<stat<<endl;	//#diag
  
  return stat;  }
  

    
 catch (const casa::AipsError& err) { cerr<<"**"<<err.getMesg()<<endl;  }
 catch (...) { cerr<<"**non-AipsError exception**"<<endl;  }

}

