	
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

#include <components/ComponentModels/SkyComponentFactory.h>

#include <casa/Quanta/QLogical.h>
#include <components/ComponentModels/Flux.h>
#include <components/ComponentModels/GaussianShape.h>
#include <components/ComponentModels/SkyComponent.h>
#include <components/ComponentModels/ConstantSpectrum.h>
#include <scimath/Mathematics/GaussianBeam.h>

using namespace casa;

int main() {
	LogIO log;
	try {
		{
			cout << "*** test deconvolve sky component" << endl;
			LogIO mylog;
			GaussianShape gauss(
					MDirection(), Quantity(10, "arcsec"),
					Quantity(5, "arcsec"), Quantity(20, "deg")
			);
			Flux<Double> flux(1.0);
			SkyComponent convolved(flux, gauss, ConstantSpectrum());
			GaussianBeam beam(
					Quantity(8, "arcsec"), Quantity(4, "arcsec"),
					Quantity(20, "deg")
			);

			SkyComponent deconvolved = SkyComponentFactory::deconvolveSkyComponent(
				mylog, convolved, beam
			);

			GaussianShape got = dynamic_cast<GaussianShape&>(deconvolved.shape());
			AlwaysAssert(got.majorAxis() == Quantity(6, "arcsec"), AipsError);
			AlwaysAssert(near(got.minorAxis(), Quantity(3, "arcsec"), 1e-7), AipsError);
			AlwaysAssert(
					nearAbs(
							got.positionAngle(), Quantity(20, "deg"), Quantity(1, "uas")
					),
					AipsError
			);
			gauss = GaussianShape(
					MDirection(), Quantity(4, "arcsec"),
					Quantity(3, "arcsec"),
					Quantity(20, "deg")
			);
			convolved = SkyComponent(flux, gauss, ConstantSpectrum());
			beam = GaussianBeam(
					Quantity(3, "arcsec"), Quantity(2, "arcsec"),
					Quantity(50, "deg")
			);
			deconvolved = SkyComponentFactory::deconvolveSkyComponent(
					mylog, convolved, beam
			);

			got = dynamic_cast<GaussianShape&>(deconvolved.shape());
			AlwaysAssert(near(got.majorAxis(), Quantity(3.0203474964295665, "arcsec"), 1e-7), AipsError);
			AlwaysAssert(near(got.minorAxis(), Quantity(1.6963198403637358, "arcsec"), 1e-7), AipsError);
			cout << "** pa " << got.positionAngle() << endl;
			AlwaysAssert(
					nearAbs(
							got.positionAngle(), Quantity(-1.9489431240069859 + 180, "deg"), Quantity(1, "uas")
					),
					AipsError
			);


		}

	}
	catch (const AipsError& x) {
		cout << "Caught error " << x.getMesg() << endl;
		return 1;
	}
	cout << "OK" << endl;
	return 0;
}
