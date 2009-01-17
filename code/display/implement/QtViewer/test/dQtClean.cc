//# dQtClean.cc:  Demo driver program for prototype qtviewer-based
//# interactive clean object (QtClean).
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
#include <display/QtViewer/QtClean.qo.h>
#include <casa/Exceptions/Error.h>

#include <display/Display/StandAloneDisplayApp.h>
	// (Configures pgplot for stand-alone Display Library apps).


#include <casa/namespace.h>


int main( int argc, char **argv ) {
 

  try {

    if(argc < 3){
      cout << "Usage: dQtClean imagefile maskfile " << endl;
      return -1;
    }

    cout<<endl
        <<"***Qt is not running yet***"<<endl
        <<"Type 'c' to continue."<<endl
	<<endl<<endl;
        
    String dummy;
    cin>>dummy;
   
    //Needed (I think) before any QObject (e.g. QtClean) can be created.
    QtApp::init(argc, argv); 
  
  
    QtClean vwrCln;
  
    String filename = "";
    String maskname= "";
    if(argc>1) filename = argv[1];
    if(argc>2) maskname = argv[2];
  
    while(True) {
  
      while(!vwrCln.loadImage(filename, maskname)) {
        
	cout<<"(This is console input)"<<endl
	    <<"Image File Name (q to quit, b to browse): "<<flush;
        cin>>filename;
        if(filename=="q" || filename=="Q") {

	  cout<<endl<<endl<<endl;
	  return 0;  }			// ***Demo Program Exit***.

        if(filename=="b" || filename=="B") break;  }

      cout<<endl<<endl<<endl
          <<"***Entering qtviewer event loop (casapy/console input frozen)***"
          <<endl<<"   Rectangles being defined by mouse tool in the viewer."   
          <<endl<<endl;    
      
      vwrCln.go();	// Enter viewer loop.
      
      cout<<endl<<"***Viewer closed: Qt loop exit -- casapy/console"
	    " is responsive again***"<<endl;    
    
      // At this point (viewer window closed), clean boxes are ready.
      // This is where actual clean logic using the boxes belongs.
    
      QtClean::CleanBoxes boxes = vwrCln.cleanBoxes();
      uInt nBoxes = boxes.nelements();
      if(nBoxes==0) cout<<endl<<endl<<"No clean boxes selected";
      else {
        cout<<endl<<endl<<endl
	    <<"Returned clean boxes (casapy app can clean"
	      " using them here):"<<endl;
        for (uInt i=0; i<nBoxes; i++) cout<<"  "<<boxes[i]<<endl;  }
      cout<<endl<<endl;
      filename="";  }  }
  

    
  catch (const casa::AipsError& err) { cerr<<"**"<<err.getMesg()<<endl;  }
  catch (...) { cerr<<"**non-AipsError exception**"<<endl;  }

}

