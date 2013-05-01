//# tSubImage.cc: Test program for class SubImage
//# Copyright (C) 1998,1999,2000,2001,2003
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
//# $Id: tSubImage.cc 20567 2009-04-09 23:12:39Z gervandiepen $

#include <imageanalysis/ImageAnalysis/ImageHeader.h>

#include <coordinates/Coordinates/CoordinateUtil.h>
#include <images/Images/TempImage.h>

#include <casa/namespace.h>


int main () {
	try {
		cout << "*** Test constructor" << endl;
		std::tr1::shared_ptr<TempImage<Float> >
		x(new TempImage<Float>(
			TiledShape(IPosition(4, 30, 30, 4, 30)),
			CoordinateUtil::defaultCoords4D()
		));
		ImageInfo info = x->imageInfo();
		GaussianBeam beam(
			Quantity(4, "arcsec"), Quantity(2, "arcsec"),
			Quantity(10, "deg")
		);
		info.setRestoringBeam(beam);
		x->setImageInfo(info);
		x->putAt(-20.5, IPosition(4, 1, 3, 2, 20));
		x->putAt(92.6, IPosition(4, 1, 4, 2, 6));
		ImageHeader<Float> header(x);
		Record headRec = header.toRecord(True);
		cout << "header looks like:" << endl;
		headRec.print(cout);



  }
  catch (const AipsError& x) {
    cerr << "Caught exception: " << x.getMesg() << endl;
    cout << "FAIL" << endl;
    return 1;
  } 

  cout << "OK" << endl;
  return 0;
}
