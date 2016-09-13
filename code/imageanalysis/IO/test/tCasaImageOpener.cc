//# tCasaImageOpener.cc: This program tests the CasaImageOpener class
//# Copyright (C) 2015
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$
#include <casa/iostream.h>
#include <casa/aips.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <images/Images/ImageConcat.h>
#include <images/Images/PagedImage.h>
#include <images/Images/TempImage.h>
#include <imageanalysis/IO/CasaImageOpener.h>
#include <casa/OS/Directory.h>
#include <casa/namespace.h>
using namespace std;
using namespace casa;
Bool copyImages(const String& dirname, Vector<String>& images){
  
  {
    File elfil(dirname);
    if(elfil.exists()){
      String command = "rm -rf "+dirname;  
      system(command.c_str());

    }
  }
  Directory eldir(dirname);
  eldir.create();
  String rootdir=eldir.path().absoluteName();
  for (uInt k=0; k< images.nelements(); ++k){
    Path elpath(images[k]);
    String newimage=rootdir+"/"+elpath.baseName();
    system(String("cp -rf "+images[k]+ " "+ newimage).c_str());
    
  }

  return True;

}

Bool testImage(const String& image){
  LatticeBase * latt=nullptr;
  latt=CasaImageOpener::openImage(image);
  AlwaysAssertExit(latt);
  DataType dtype = TpOther;
  dtype=latt->dataType();
  AlwaysAssertExit(dtype==TpFloat);
  AlwaysAssertExit((latt->shape()[3])==2);

  return True;
}
int main() {
  try {

// Make some Arrays

    IPosition shape(4,5,10, 1,1);
    Array<Float> a1(shape);
    Array<Float> a2(shape);
      Int i, j;
      for (i=0; i<shape(0); i++) {
	for (j=0; j<shape(1); j++) {
	  a1(IPosition(4,i,j,0,0)) = i + j;
	  a2(IPosition(4,i,j,0,0)) = -i - j;
        }
      }

// Make some PagedImages and give them a mask
      {
	PagedImage<Float> im1(shape, CoordinateUtil::defaultCoords4D(),
			      "tImageConcat_tmp1.img");
	PagedImage<Float> im2(shape, CoordinateUtil::defaultCoords4D(),
			      "tImageConcat_tmp2.img");
	im1.makeMask("mask0", True, True, True, True); 
	im2.makeMask("mask0", True, True, True, False);
	im1.put(a1); 
	im2.put(a2);

	// Make a MaskedLattice as well
	
	ArrayLattice<Float> al1(a1);
	SubLattice<Float> ml1(al1);
	
//
      {
	cout << "Axis 0, PagedImages (masks)" << endl;
	
	// Concatenate along Spectral axis

	ImageConcat<Float> lc (3, True);
	lc.setImage(im1, True);
	lc.setImage(im2, True);
	
	// Find output shape

	lc.save("virtualConcat1.image");
	
	// Make output
	IPosition outShape=lc.shape();
	PagedImage<Float> ml3(outShape, CoordinateUtil::defaultCoords4D(),
			      "tImageConcat_tmp3.img");
	ml3.makeMask("mask0", True, True, True, True);
	
	// Copy to output
	
	ml3.copyData(lc);
	ml3.pixelMask().put(lc.getMask());
      }
      }
      Vector<String> images(3);
      images[0]="tImageConcat_tmp1.img";
      images[1]="tImageConcat_tmp2.img";
      images[2]="virtualConcat1.image";
      copyImages("virtualConcat2.image", images);
      AlwaysAssertExit(CasaImageOpener::imageType("virtualConcat1.image")==ImageOpener::IMAGECONCAT);
      testImage("virtualConcat1.image");
      AlwaysAssertExit(CasaImageOpener::imageType("virtualConcat2.image")==ImageOpener::IMAGECONCAT);
      testImage("virtualConcat2.image");
      AlwaysAssertExit(CasaImageOpener::imageType("tImageConcat_tmp3.img")==ImageOpener::AIPSPP);
      testImage("tImageConcat_tmp3.img");
       
      // 

         

      

  }
  catch(const AipsError& x) {
    cerr << x.getMesg() << endl;
    return 1;
  } 
  cout << "OK" << endl;
  return 0;
}
