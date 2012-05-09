 //# tImageInfo.cc: Miscellaneous information related to an image
//# Copyright (C) 1998,1999,2000,2002
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
//#
//# $Id: tImageInfo.cc 20491 2009-01-16 08:33:56Z gervandiepen $

#include <images/Images/Image2DConvolver.h>

#include <casa/Exceptions/Error.h>
#include <images/Images/FITSImage.h>
#include <casa/Utilities/Assert.h>
/*
#include <casa/BasicMath/Math.h>
#include <casa/Containers/Record.h>
#include <casa/Containers/RecordField.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/QLogical.h>
#include <casa/Arrays/Vector.h>
#include <casa/iostream.h>
*/
#include <casa/namespace.h>

int main() {
	try {
		FITSImage in("test_image2dconvolver.fits");
		TempImage<Float> got(in.shape(), in.coordinates());
		Image2DConvolver<Float> ic;
		Vector<Quantity> gauss(3);
		gauss[0] = Quantity(10, "arcmin");
		gauss[1] = Quantity(8, "arcmin");
		gauss[2] = Quantity(80, "deg");
		LogIO log;
		ic.convolve(
			log, got, in, VectorKernel::GAUSSIAN,
			IPosition(2, 0, 1), gauss, True, -1.0, True
		);
		FITSImage exp("test_image2dconvolver_convolved.fits");
		cout << "*** max " << max(abs(got.get() - exp.get())) << endl;
		AlwaysAssert(max(abs(got.get() - exp.get())) < 1e-8, AipsError);
		Vector<Quantity> gotBeam = got.imageInfo().restoringBeam();
		Vector<Quantity> expBeam = exp.imageInfo().restoringBeam();
		for (uInt i=0; i<3; i++) {
			AlwaysAssert(
				gotBeam[i].getUnit() == expBeam[i].getUnit(),
				AipsError
			);
			AlwaysAssert(
				abs(gotBeam[i].getValue() - expBeam[i].getValue()) < 1e-8,
				AipsError
			);
		}
	}
	catch (AipsError x) {
		cout << "Caught error " << x.getMesg() << endl;
		return 1;
	}
    cout << "OK" << endl;
    return 0;
}
