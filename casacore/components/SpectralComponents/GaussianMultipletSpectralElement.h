//# SpectralElement.h: Describes (a set of related) spectral lines
//# Copyright (C) 2001,2003,2004
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
//# $Id: SpectralElement.h 20652 2009-07-06 05:04:32Z Malte.Marquarding $

#ifndef COMPONENTS_GAUSSIANMULTIPLETSPECTRALELEMENT_H
#define COMPONENTS_GAUSSIANMULTIPLETSPECTRALELEMENT_H

#include <casa/Arrays/Matrix.h>
#include <components/SpectralComponents/CompiledSpectralElement.h>
#include <components/SpectralComponents/GaussianSpectralElement.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Describes a multiplet of Gaussian shaped spectral lines
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="tSpectralFit" demos="">
// </reviewed>

// <prerequisite>
//   <li> <linkto module=SpectralElement>SpectralElement</linkto> module
//   <li> <linkto module=CompiledSpectralElement>CompiledSpectralElement</linkto> module
// </prerequisite>
//
// <etymology>
// From Gaussian and multiplet and spectral line and element
// </etymology>
//
// <synopsis>
// The GaussianMultipletSpectralElement class describes a multiplet
// of Gaussian shaped spectral lines for describing spectral profile.
//
// Relationships between Gaussians in the multiplet must be specified.
// Any combination of one or more of fixed relationships between Gaussian
// centers, Gaussian amplitudes, and/or Gaussian widths. The constructor
// takes a Vector of GaussianSpectralElements which describe the estimates
// of the n Gaussians in the multiplet. These objects themselves are not
// used in the fitting but just to create the function that describes the
// multiplet. It takes a string, describing which, if any, of the parameters
// of the zeroth (reference) Gaussian should be fixed. It takes an n-1 x 3
// Matrix<Double> describing these fixed relationships. Each ith row describes
// the fixed relationship(s) between the ith and the zeroth (reference) Gaussian.
// The first element of each row describes the ratio of amplitudes between the
// ith and reference Gaussian, the second element describes the difference between
// the centers of the ith and reference Gaussian, and the third represents the
// ratios of the FWHM of the ith and reference Gaussian. A value of 0 for any of
// these indicates there is no fixed relationship for that parameter. At least one
// and possibly all three values must be non-zero for each row.
// </synopsis>
//
// <example>
// This is how to specify a doublet in which the only constraint is that the amplitudes
// between the lines must be fixed, the first Gaussian must have an ampliute of 0.6 times
// the zeroth (reference) Gaussian. Other than that, all other parameters can
// vary during the fit:

// GaussianSpectralElement reference(5, 25, 16);
// // The amplitude value of first is ignored because of the relationship to the
////  reference, but it still must be specified in the constructor
// GaussianSpectralElement first(1, 40, 17);
// Vector<GaussianSpectralElement> pair(2);
// pair[0] = reference;
// pair[1] = first;
// Matrix<Double> fixedRel(1, 3);
// fixedRel[0][0] = 0.6
// GaussianMultipletSpectralElement doublet(pair,"",fixedRel);
// </example>
//
// <motivation>
// To allow specifying constraints between different Gaussian spectral
// lines for fitting
// </motivation>

class GaussianMultipletSpectralElement: public CompiledSpectralElement {
public:

	// Construct a Gaussian multiplet. The values of non-reference (non-zeroth)
	// estimates will be automatically adjusted if there is a fixed relationship
	// between a non-reference Gaussian parameter and the corresponding reference
	// Gaussian parameter.
	GaussianMultipletSpectralElement(
		const Vector<GaussianSpectralElement>& estimates,
		const Matrix<Double>& fixedRelationships
	);

	// copy semantics
	GaussianMultipletSpectralElement(
		const GaussianMultipletSpectralElement& other
	);

	~GaussianMultipletSpectralElement();

	SpectralElement* clone() const;

	// copy semantics
	GaussianMultipletSpectralElement &operator=(
		const GaussianMultipletSpectralElement& other
	);

	Bool operator==(
		const GaussianMultipletSpectralElement& other
	) const;

	// get the gaussians
	const Vector<GaussianSpectralElement>& getGaussians() const;

	const Matrix<Double>& getConstraints() const;

	void set(const Vector<Double>& param);

	void setError(const Vector<Double> &err);

	void fix(const Vector<Bool>& fix);

	// Save to a record.
	Bool toRecord(RecordInterface& out) const;

private:
	Vector<GaussianSpectralElement> _gaussians;
	Matrix<Double> _constraints;
	Matrix<uInt> _paramIndices;
};

ostream &operator<<(ostream &os, const GaussianMultipletSpectralElement &elem);

} //# NAMESPACE CASA - END

#endif

