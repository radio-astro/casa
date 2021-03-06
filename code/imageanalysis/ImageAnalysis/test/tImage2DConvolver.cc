	
//# tImage2DConvolver.cc: Miscellaneous information related to an image
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

#include <imageanalysis/ImageAnalysis/Image2DConvolver.h>

#include <casa/Arrays/ArrayMath.h>
#include <casa/OS/EnvVar.h>
#include <casa/Exceptions/Error.h>
#include <images/Images/FITSImage.h>
#include <images/Regions/WCBox.h>
#include <casa/Utilities/Assert.h>
#include <casa/namespace.h>
#include <imageanalysis/ImageAnalysis/SubImageFactory.h>

using namespace casa;

int main() {
	LogIO log;
	try {
		String casapath = EnvironmentVariable::get("CASAPATH");
		if (casapath.empty()) {
			cerr << "CASAPATH env variable not defined. Can't find fixtures. Did you source the casainit.(c)sh file?" << endl;
			return 1;
		}

		String *parts = new String[2];
		split(casapath, parts, 2, String(" "));
		String datadir = parts[0] + "/data/regression/unittest/imageanalysis/ImageAnalysis/";
		delete [] parts;

		{
			SPCIIF in(new FITSImage(datadir + "test_image2dconvolver.fits"));
			Vector<Quantity> gauss(3);
			gauss[0] = Quantity(10, "arcmin");
			gauss[1] = Quantity(8, "arcmin");
			gauss[2] = Quantity(80, "deg");
			Image2DConvolver<Float> c(in, nullptr, "", "", false);
			c.setKernel("gauss", gauss[0], gauss[1], gauss[2]);
			c.setAxes(make_pair(0, 1));
			c.setScale(-1);
			c.setTargetRes(false);
			auto got = c.convolve();
			FITSImage exp(datadir + "test_image2dconvolver_convolved.fits");
            cout << "max diff " << max(abs(got->get() - exp.get())) << endl;
			AlwaysAssert(max(abs(got->get() - exp.get())) < 7e-5, AipsError);
			GaussianBeam gotBeam = got->imageInfo().restoringBeam();
			GaussianBeam expBeam = exp.imageInfo().restoringBeam();
			AlwaysAssert (near(expBeam, gotBeam, 1e-6, Quantity(1e-5, "deg")), AipsError);
		}
		{
			SPCIIF in(
				new PagedImage<Float>(
					datadir + "test_image2dconvolver_multibeam.im"
				)
			);
			Vector<Quantity> gauss(3);
			gauss[0] = Quantity(10, "arcmin");
			gauss[1] = Quantity(8, "arcmin");
			gauss[2] = Quantity(80, "deg");
			Image2DConvolver<Float> c(in, nullptr, "", "", false);
			c.setKernel("gauss", gauss[0], gauss[1], gauss[2]);
			c.setAxes(make_pair(0, 1));
			c.setScale(-1);
			c.setTargetRes(false);
			auto got = c.convolve();
			IPosition shape = in->shape();
			IPosition start(in->ndim(), 0);
			IPosition end = shape - 1;
			for (Int i=0; i<shape[2]; i++) {
				start[2] = i;
				end[2] = i;
				Slicer slice(start, end, Slicer::endIsLast);
				SPCIIF expIn(new SubImage<Float>(*in, slice));
				Image2DConvolver<Float> c2(expIn, nullptr, "", "", false);
				c2.setKernel("gauss", gauss[0], gauss[1], gauss[2]);
				c2.setAxes(make_pair(0, 1));
				c2.setScale(-1);
				c2.setTargetRes(false);
				auto exp = c2.convolve();
				GaussianBeam gotBeam = got->imageInfo().restoringBeam(i, -1);
				GaussianBeam expBeam = exp->imageInfo().restoringBeam();
				AlwaysAssert(near(expBeam, gotBeam, 1e-7, Quantity(1e-4, "deg")), AipsError);
				Array<Float> gotData = got->get()(slice);
				Array<Float> expData = exp->get();
				AlwaysAssert(
					max(fabs(gotData - expData)) < 3e-5,
					AipsError
				);
			}
		}
		{
			const Double fac = 4*std::log(2);
			cout << "*** Test targetres parameter for single beam image" << endl;
			CoordinateSystem csys = CoordinateUtil::defaultCoords2D();
			csys.setWorldAxisUnits(Vector<String>(2, "arcsec"));
			csys.setIncrement(Vector<Double>(2, 1.0));
			SPIIF tim(new TempImage<Float>(TiledShape(IPosition(2, 100, 100)), csys));
			tim->setUnits("Jy/beam");
			ImageInfo info = tim->imageInfo();
			info.setRestoringBeam(
				GaussianBeam(
					Quantity(6.0,"arcsec"), Quantity(3.0, "arcsec"),
					Quantity(0, "deg")
				)
			);
			tim->setImageInfo(info);
			Array<Float> values(tim->shape());
			Array<Float> expected(tim->shape());
			for (Int i=0; i<tim->shape()[0]; i++) {
				Double x = (Int)tim->shape()[0]/2 - i;
				for (Int j=0; j<tim->shape()[1]; j++) {
					Double y = (Int)tim->shape()[1]/2 - j;
					values(IPosition(2, i, j)) = exp(-(x*x*fac/9 + y*y*fac/36));
					expected(IPosition(2, i, j)) = exp(-(x*x*fac/25 + y*y*fac/100));
                }
			}
			tim->put(values);
			Vector<Quantity> gauss(3);
			gauss[0] = Quantity(8, "arcsec");
			gauss[1] = Quantity(4, "arcsec");
			gauss[2] = Quantity(0, "deg");
			Bool targetres = false;

			Image2DConvolver<Float> c(tim, nullptr, "", "", false);
			c.setKernel("gauss", gauss[0], gauss[1], gauss[2]);
			c.setAxes(make_pair(0, 1));
			c.setScale(-1);
			c.setTargetRes(targetres);
			auto got = c.convolve();
			AlwaysAssert(got->shape() == expected.shape(), AipsError);
			GaussianBeam expBeam(
				Quantity(10, "arcsec"), Quantity(5, "arcsec"),
				Quantity(0, "deg")
			);
			AlwaysAssert(near(got->imageInfo().restoringBeam(), expBeam, 1e-6, Quantity(1e-5, "deg")), AipsError);
			cout << "max diff " << max(abs(got->get() - expected)) << endl;
			cout << "got " << got->get()(IPosition(2,50,50)) << endl;
			cout << "expected " << expected(IPosition(2, 50, 50)) << endl;
	        cout << "got " << got->get()(IPosition(2,51,50)) << endl;
			cout << "expected " << expected(IPosition(2, 51, 50)) << endl;
			AlwaysAssert(allNearAbs(got->get(), expected, 3e-5), AipsError);
			gauss[0] = Quantity(10, "arcsec");
			gauss[1] = Quantity(5, "arcsec");
			gauss[2] = Quantity(0, "deg");
			c.setKernel("gauss", gauss[0], gauss[1], gauss[2]);
			targetres = true;
			c.setTargetRes(targetres);
			got = c.convolve();
			cout << "got " << got->imageInfo().restoringBeam() << endl;
			cout << "exp " << expBeam << endl;
			AlwaysAssert(near(got->imageInfo().restoringBeam(), expBeam, 1e-6, Quantity(1e-5, "deg")), AipsError);
			cout << "max diff " << max(abs(got->get() - expected)) << endl;
			cout << "got " << got->get()(IPosition(2,50,50)) << endl;
			cout << "expected " << expected(IPosition(2, 50, 50)) << endl;
			AlwaysAssert(allNearAbs(got->get(), expected, 1e-6), AipsError);

			cout << "*** Test targetres parameter for multibeam image" << endl;
			csys.addCoordinate(SpectralCoordinate());
			tim.reset(new TempImage<Float>(TiledShape(IPosition(3, 100, 100, 2)), csys));
			tim->setUnits("Jy/beam");
			got = tim;
			info = tim->imageInfo();
			info.setAllBeams(tim->shape()[2], 0, GaussianBeam());
			info.setBeam(
				0, -1, GaussianBeam(
					Quantity(6.0,"arcsec"), Quantity(3.0, "arcsec"),
					Quantity(0, "deg")
				)
			);
			info.setBeam(
				1, -1, GaussianBeam(
					Quantity(4.0,"arcsec"), Quantity(2.0, "arcsec"),
					Quantity(0, "deg")
				)
			);
			tim->setImageInfo(info);
			values.assign(Array<Float>(tim->shape()));
			expected.assign(Array<Float>(tim->shape()));
			IPosition pos(3);
			for (Int i=0; i<tim->shape()[0]; i++) {
				pos[0] = i;
				Double x = tim->shape()[0]/2 - i;
				for (Int j=0; j<tim->shape()[1]; j++) {
					pos[1] = j;
					Double y = tim->shape()[1]/2 - j;
					for (Int k=0; k<tim->shape()[2]; k++) {
						pos[2] = k;
						Double sx = (k == 0) ? 9 : 4;
						Double sy = (k == 0) ? 36 : 16;
						values(pos) = exp(-(x*x*fac/sx + y*y*fac/sy));
					}
				}
			}
			tim->put(values);
			targetres = false;
			Vector<GaussianBeam> eBeam(2);
			for (uInt trial=0; trial<2; trial++) {
				targetres = trial == 1;
				gauss[0] = Quantity(targetres ? 10 : 8, "arcsec");
				gauss[1] = Quantity(targetres ? 5 : 4, "arcsec");
				gauss[2] = Quantity(0, "deg");
				for (Int k=0; k<tim->shape()[2]; k++) {
					IPosition testPos(3, 48, 48, k);
					IPosition start(3, 0, 0, k);
					IPosition end(3, tim->shape()[0]-1, tim->shape()[1]-1, k);
					WCBox box(
						LCBox(start, end,  tim->shape()), csys
					);
					auto inChannelIm = SubImageFactory<Float>::createSubImageRO(
						*tim, box.toRecord(""), "", &log
					);

					IPosition inPos = testPos;
					inPos[2] = 0;
					Image2DConvolver<Float> c3(inChannelIm, nullptr, "", "", false);
					c3.setKernel("gauss", gauss[0], gauss[1], gauss[2]);
					c3.setAxes(make_pair(0, 1));
					c3.setScale(-1);
					c3.setTargetRes(targetres);
					auto outChannelIm = c3.convolve();
					expected(start, end) = outChannelIm->get();
					eBeam[k] = outChannelIm->imageInfo().restoringBeam();
					if (targetres) {
						AlwaysAssert(
							near(
								eBeam[k],
								GaussianBeam(gauss),
								2e-5, Quantity(1e-5, "deg")
							), AipsError
						);
					}
				}
				Image2DConvolver<Float> c4(tim, nullptr, "", "", false);
				c4.setKernel("gauss", gauss[0], gauss[1], gauss[2]);
				c4.setAxes(make_pair(0, 1));
				c4.setScale(-1);
				c4.setTargetRes(targetres);
				auto got = c4.convolve();
				if (targetres) {
					AlwaysAssert(
						got->imageInfo().restoringBeam() == GaussianBeam(gauss),
						AipsError
					);
				}
				else {
					for (uInt k=0; k<2; k++) {
						AlwaysAssert(
							near(
								got->imageInfo().restoringBeam(k, -1),
								eBeam[k], 1e-6, Quantity(1e-5, "deg")
							), AipsError
						);
					}
				}
				AlwaysAssert(allNearAbs(got->get(), expected, 3e-7), AipsError);
			}
		}
	}
	catch (const AipsError& x) {
		cout << "Caught error " << x.getMesg() << endl;
		return 1;
	}
    cout << "OK" << endl;
    return 0;
}
