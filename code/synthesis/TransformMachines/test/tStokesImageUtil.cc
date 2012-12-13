//# Copyright (C) 2011
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

#include <casa/OS/EnvVar.h>
#include <images/Images/FITSImage.h>
#include <images/Images/TempImage.h>

#include <synthesis/TransformMachines/StokesImageUtil.h>

#include <casa/namespace.h>

int main() {
	try {
		String casapath = EnvironmentVariable::get("CASAPATH");
		String *parts = new String[2];
		split(casapath, parts, 2, String(" "));
		String datadir = parts[0] + "/data/regression/unittest/synthesis/";
		delete [] parts;
		FITSImage gaussianModel(datadir + "gaussian_model.fits");
		Vector<Float> beam(3, 0);
		AlwaysAssert(
			StokesImageUtil::FitGaussianPSF(gaussianModel, beam),
			AipsError
		);
		AlwaysAssert(beam[0] == 2.5, AipsError);
		AlwaysAssert(beam[1] == 1.25, AipsError);
		AlwaysAssert(near(beam[2], 57.2958), AipsError);

		GaussianBeam gbeam;
		AlwaysAssert(
			StokesImageUtil::FitGaussianPSF(gaussianModel, gbeam),
			AipsError
		);
		AlwaysAssert(gbeam.getMajor("arcsec") == 2.5, AipsError);
		AlwaysAssert(gbeam.getMinor("arcsec") == 1.25, AipsError);
		AlwaysAssert(near(gbeam.getPA(Unit("deg")), 57.2958, 1e-6), AipsError);

		TempImage<Float> t(
			TiledShape(gaussianModel.shape()),
			gaussianModel.coordinates()
		);
		t.put(gaussianModel.get());

		Bool b = True;
		Quantity bmaj(5, "arcsec");
		Quantity bmin(2.5, "arcsec");
		Quantity bpa(20, "deg");
		gbeam = GaussianBeam(bmaj, bmin, bpa);
		StokesImageUtil::Convolve(t, gbeam, b);
		FITSImage exp(datadir + "expected_convolved.fits");

		AlwaysAssert(allEQ(t.get(), exp.get()), AipsError);
	}
	catch (AipsError x) {
		cout << x.getMesg() << endl;
		cout << "FAIL" << endl;
		return 1;
	}
	cout << "OK" << endl;
	return 0;



}
