//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002
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

#include <components/ComponentModels/GaussianDeconvolver.h>

#include <scimath/Mathematics/GaussianBeam.h>

using namespace casa;

int main() {
	typedef GaussianBeam Angular2DGaussian;
	const String rad = "rad";
	try {
		{
			// Easy test 1 - P.A. = 0
			Angular2DGaussian source(
					Quantity(20, "arcsec"),
					Quantity(10, "arcsec"),
					Quantity(0, "deg")
			);
			GaussianBeam beam(
					Quantity(15, "arcsec"),
					Quantity(5, "arcsec"),
					Quantity(0, "deg")
			);
			Angular2DGaussian model;
			Double maj = sqrt(
					square(source.getMajor(rad))
					- square(beam.getMajor(rad))
			);
			Quantity majQ(maj, rad);
			majQ.convert(source.getMajor().getFullUnit());

			Double min = sqrt(square(source.getMinor(rad)) -
					square(beam.getMinor(rad)));
			Quantity minQ(min, rad);
			minQ.convert(source.getMinor().getFullUnit());

			Quantity paQ(0.0,source.getPA().getFullUnit());
			Angular2DGaussian expected(majQ, minQ, paQ);

			Bool isPoint = GaussianDeconvolver::deconvolve(model, source, beam);
			cout << "Source   = " << source << endl;
			cout << "Beam     = " << beam << endl;
			cout << "Model    = " << model << endl;
			cout << "Expected = " << expected << endl;
			cout << "isPoint  = " << isPoint << endl << endl;
			AlwaysAssert(!isPoint, AipsError);
			AlwaysAssert(near(expected, model,1e-6, Quantity(0.001, "arcsec")), AipsError);
		}
		{
			// Easy test 2 - P.A. aligned
			Angular2DGaussian source(
					Quantity(10, "arcsec"),
					Quantity(5, "arcsec"),
					Quantity(20, "deg")
			);
			GaussianBeam beam(
					Quantity(8, "arcsec"),
					Quantity(4, "arcsec"),
					Quantity(20, "deg")
			);

			Double maj = sqrt(
					square(
							source.getMajor(rad))
							- square(beam.getMajor(rad)
							)
			);
			Quantity majQ(maj, rad);
			majQ.convert(source.getMajor().getFullUnit());

			Double min = sqrt(
					square(source.getMinor(rad))
					- square(beam.getMinor(rad))
			);
			Quantity minQ(min, rad);
			minQ.convert(source.getMinor().getFullUnit());
			Quantity paQ(20, source.getPA().getFullUnit());

			Angular2DGaussian expected(majQ, minQ, paQ);
			Angular2DGaussian model;
			model.convert("arcsec", "arcsec", "deg");
			Bool isPoint = GaussianDeconvolver::deconvolve(model, source, beam);
			cout << "Source   = " << source << endl;
			cout << "Beam     = " << beam << endl;
			cout << "Model    = " << model << endl;
			cout << "Expected = " << expected << endl;
			cout << "isPoint  = " << isPoint << endl << endl;
			AlwaysAssert(!isPoint, AipsError);
			AlwaysAssert(near(expected, model,1e-6, Quantity(1, "mas")), AipsError);
		}
		{
			// Easy test 3 - beam and source the same
			Angular2DGaussian source(
					Quantity(20, "arcsec"),
					Quantity(10, "arcsec"),
					Quantity(45, "deg")
			);
			GaussianBeam beam(
					Quantity(20.00001, "arcsec"),
					Quantity(10.00001, "arcsec"),
					Quantity(45, "deg")
			);
			Angular2DGaussian model;
			Bool isPoint = GaussianDeconvolver::deconvolve(model, source, beam);
			cout << "Source   = " << source << endl;
			cout << "Beam     = " << beam << endl;
			cout << "Model    = " << model << endl;
			cout << "Expected = " << beam << endl;
			cout << "isPoint  = " << isPoint << endl << endl;
			AlwaysAssert(isPoint, AipsError);
			AlwaysAssert(near(beam, model,1e-6, Quantity(1, "mas")), AipsError);
		}
		{
			// Not so easy test 4
			Angular2DGaussian source(
					Quantity(4, "arcsec"),
					Quantity(3, "arcsec"),
					Quantity(20, "deg")
			);
			GaussianBeam beam(
					Quantity(3, "arcsec"),
					Quantity(2, "arcsec"),
					Quantity(50, "deg")
			);
			Angular2DGaussian expected(
					Quantity(3.0203474964313628, "arcsec"),
					Quantity(1.6963198403605391, "arcsec"),
					Quantity(-1.9489431240069859, "deg")
			);
			Angular2DGaussian model;
			model.convert("arcsec", "arcsec", "deg");
			Bool isPoint = GaussianDeconvolver::deconvolve(model, source, beam);
			cout << "Source   = " << source << endl;
			cout << "Beam     = " << beam << endl;
			cout << "Model    = " << model << endl;
			cout << "Expected = " << expected << endl;
			cout << "isPoint  = " << isPoint << endl << endl;
			AlwaysAssert(!isPoint, AipsError);
			AlwaysAssert(near(expected, model,1e-6, Quantity(1, "mas")), AipsError);
		}

	}
	catch (const AipsError& err) {
		cout << err.getMesg() << endl;
		cout << "Unexpected failure!" << endl;
		return 1;
	}

	cout << "OK" << endl;

	return 0;
}
