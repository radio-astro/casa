// dRegionFile.cc  3/31/08 dlk

// Trivial demonstration of region file reading.  Print its contents; if a
// corresponding image file can be found by dropping '.rgn' from the end of
// the region filename, print statistics on the SubImage formed by applying
// the region to the image.

//# Copyright (C) 2008
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

#include <casa/aips.h>
#include <casa/iostream.h>
#include <casa/IO/AipsIO.h>
#include <tables/Tables/TableRecord.h>
#include <images/Regions/ImageRegion.h>
#include <casa/Inputs/Input.h>
#include <casa/Exceptions/Error.h>
#include <display/QtViewer/QtViewerBase.qo.h>
#include <display/QtViewer/QtDisplayData.qo.h>

#include <casa/namespace.h>

int main(int argc, char** argv) {
  
  try {
 
    if(argc < 2){
      cout << "Usage: dRgn <regionfilename> " << endl;
      return -1;  }
    
    String path = argv[1];
    
    
    //Extract the ImageRegion from the file.
    
    AipsIO os(path);
    TableRecord trec;
    os >> trec;
    ImageRegion* ir = ImageRegion::fromRecord(trec, path+".tbl");
  
    
    // Print the region from its 'TableRecord' form.
    
    cout<<endl<<"rec:"<<endl<<endl << trec <<endl;
    
    
    // Do something 'useful' with the restored ImageRegion.
    // (Note: printRegionStats() was handy; it happens to be a viewer
    // method, but doesn't require the viewer in principle; internally
    // it only needs an ImageRegion and the image it applies to.
    // See internals of printRegionStats() to see how it creates
    // a SubImage and orders up statistics on that).
    
    Int rgnpos = path.length()-4;
    if(rgnpos>0 && path.from(rgnpos)==".rgn") {
      String imgName = path.before(rgnpos);
      QtViewerBase v;
      QtDisplayData dd(&v, imgName, "image", "raster");
      if(dd.imageInterface()!=0) {

	cout<<endl<<"Region stats for "<<imgName<<":"<<endl;
        dd.printRegionStats(*ir);  }  }
      
    
    delete ir;
    return 0;  }
  
  
  catch (const casa::AipsError& err) { 
    cerr<<"**"<<err.getMesg()<<endl; return -1;  }
  catch (...) { cerr<<"**non-AipsError exception**"<<endl; return -1;  }  }
