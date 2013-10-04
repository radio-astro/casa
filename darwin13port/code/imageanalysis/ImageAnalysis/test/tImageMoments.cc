//# tImageRegrid.cc: This program test Measure functionsimage regridding
//# Copyright (C) 2001,2002,2004
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
//# $Id: tImageRegrid.cc 20567 2009-04-09 23:12:39Z gervandiepen $

//# Includes
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <images/Images/TempImage.h>
#include <imageanalysis/ImageAnalysis/Image2DConvolver.h>
#include <imageanalysis/ImageAnalysis/ImageMoments.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Logging/LogIO.h>
#include <casa/Utilities/Assert.h>
#include <casa/iostream.h>

#include <casa/namespace.h>
int main () {

	try {
		CoordinateSystem csys = CoordinateUtil::defaultCoords3D();
		IPosition shape(3, 100, 100, 3);
		TempImage<Float> temp(TiledShape(shape), csys);
		TempImage<Float> expImage(TiledShape(shape), csys);
		TempImage<Float> multi(TiledShape(shape), csys);
		TempImage<Float> got(TiledShape(shape), csys);
		temp.set(0);
		temp.putAt(1.0, IPosition(3, 50, 50, 0));
		temp.putAt(1.0, IPosition(3, 50, 50, 1));
		temp.putAt(1.0, IPosition(3, 50, 50, 2));
		Quantity fwhm(10, "arcmin");
		Vector<Quantity> p(3);
		p(0) = fwhm;
		p(1) = fwhm;
		p(2) = Quantity(0, "deg");
		LogIO log;
		temp.setUnits("Jy/pixel");
		Image2DConvolver<Float>::convolve(
			log, expImage, temp, VectorKernel::GAUSSIAN,
			IPosition(2, 0, 1), p, True, -1.0, True, True
		);

		ImageMoments<Float> momentMaker(expImage, log, True, False);
		Vector<Int> whichMoments(5);
		whichMoments(0) = 0;
		whichMoments(1) = 1;
		whichMoments(2) = 2;
		whichMoments(3) = 3;
		whichMoments(4) = 4;
		if (!momentMaker.setMoments(whichMoments)) {
			throw AipsError("Cannot set moments");
		}
		Int axis = 2;
		momentMaker.setMomentAxis(axis);
		PtrBlock<MaskedLattice<Float>*> images;
		momentMaker.createMoments(images, False, "exp", False);
		shape[2] = 1;
		TempImage<Float> i6(TiledShape(shape), csys);
		TempImage<Float> io6(TiledShape(shape), csys);

		TempImage<Float> i8(TiledShape(shape), csys);
		TempImage<Float> io8(TiledShape(shape), csys);

		TempImage<Float> i10(TiledShape(shape), csys);
		TempImage<Float> io10(TiledShape(shape), csys);

		ImageBeamSet beamSet(3, 0);
		for (uInt i=0; i<3; i++) {
			fwhm.setValue(6 + i*2);
			p(0) = fwhm;
			p(1) = fwhm;
			if (i == 0) {
				i6.set(0);
				i6.setUnits("Jy/pixel");
				i6.putAt(1.0, IPosition(3, 50, 50, 0));
				Image2DConvolver<Float>::convolve(
					log, io6, i6, VectorKernel::GAUSSIAN,
					IPosition(2, 0, 1), p, True, -1.0, True, True
				);
			}
			if (i == 1) {
				i8.set(0);
				i8.setUnits("Jy/pixel");
				i8.putAt(1.0, IPosition(3, 50, 50, 0));
				Image2DConvolver<Float>::convolve(
					log, io8, i8, VectorKernel::GAUSSIAN,
					IPosition(2, 0, 1), p, True, -1.0
				);
			}
			if (i == 2) {
				i10.set(0);
				i10.setUnits("Jy/pixel");
				i10.putAt(1.0, IPosition(3, 50, 50, 0));
				Image2DConvolver<Float>::convolve(
					log, io10, i10, VectorKernel::GAUSSIAN,
					IPosition(2, 0, 1), p, True, -1.0
				);
			}
			beamSet.setBeam(i, -1, GaussianBeam(p));

		}
		multi.putSlice(io6.get(), IPosition(3, 0, 0, 0));
		multi.putSlice(io8.get(), IPosition(3, 0, 0, 1));
		multi.putSlice(io10.get(), IPosition(3, 0, 0, 2));
		ImageInfo info = multi.imageInfo();
		info.setBeams(beamSet);
		multi.setUnits("Jy/beam");
		multi.setImageInfo(info);
		ImageMoments<Float> gotMom(multi, log, True, False);
		if (!gotMom.setMoments(whichMoments)) {
			throw AipsError("Cannot set moments");
		}
		gotMom.setMomentAxis(axis);
		PtrBlock<MaskedLattice<Float>*> gotImages;
		gotMom.createMoments(gotImages, False, "got", False);
		for (uInt i=0; i<gotImages.size(); i++) {
			IPosition mshape = gotImages[i]->shape();
			IPosition axisPath = IPosition::makeAxisPath(mshape.size());
            ArrayPositionIterator iter(mshape, axisPath, False);
			while (! iter.pastEnd()) {
                const IPosition pos = iter.pos();
                if (expImage.getAt(pos) < 0.01) {
					gotImages[i]->putAt(-100, pos);
					images[i]->putAt(-100, pos);
				}
                iter.next();
			}
           	if (i == 0 || i == 1 || i == 3) {
				Float epsilon = i == 3 ? 3e-3 : 1e-5;
				cout << (max(abs(gotImages[i]->get() - images[i]->get()))) << endl;
				AlwaysAssert(
					allNearAbs(
						gotImages[i]->get(), images[i]->get(),
						epsilon
					),
					AipsError
				);
			}
			else {
				AlwaysAssert(
					allNear(
						gotImages[i]->get(), images[i]->get(),
						1e-5
					),
					AipsError
				);
			}
		}


	}
	catch (const AipsError& x) {
		cerr << "FAIL: " << x.getMesg() << endl;
		return 1;
	}
	cout << "OK" << endl;
	return 0;
}


