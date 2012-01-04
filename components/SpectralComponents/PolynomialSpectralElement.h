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

#ifndef COMPONENTS_POLYNOMIALSPECTRALELEMENT_H
#define COMPONENTS_POLYNOMIALSPECTRALELEMENT_H

#include <components/SpectralComponents/SpectralElement.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Describes (a set of related) spectral lines
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="tSpectralFit" demos="">
// </reviewed>

// <prerequisite>
//   <li> <linkto module=Functionals>Functionals</linkto> module
//   <li> <linkto class=RecordInterface>RecordInterface</linkto> class
// </prerequisite>
//
// <etymology>
// From spectral line and element
// </etymology>
//
// <synopsis>
// The SpectralElement class is a container for a spectral line descriptor.
// It can contain a single line (like a Gaussian profile), or a set of
// related lines (like a doublet or so).
//
// The element can be used in the
// <linkto class=SpectralFit>SpectralFit</linkto> class and in the
// <linkto class=SpectralEstimate>SpectralEstimate</linkto> class.
//
// The default type is a Gaussian, defined as:
// <srcblock>
//	AMPL.exp[ -(x-CENTER)<sup>2</sup>/2 SIGMA<sup>2</sup>]
// </srcblock>
//
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// To have a container for fitting of spectral profiles to an observed spectrum
// </motivation>
//
// <todo asof="2001/02/04">
//   <li> add more profile types
// </todo>

class PolynomialSpectralElement: public SpectralElement {
public:

	// Construct an n-degree polynomial
	explicit PolynomialSpectralElement(const uInt n);

	// Construct the given tp with the given param
	// <thrown>
	//   <li> AipsError if incorrect number of parameters (e.g. not 3 for GAUSSIAN)
	//   <li> AipsError if sigma == 0.0
	// </thrown>
	PolynomialSpectralElement(const Vector<Double> &param);
	// Copy constructor (deep copy)
	// <thrown>
	//   <li> AipsError if sigma == 0.0
	// </thrown>
	PolynomialSpectralElement(const PolynomialSpectralElement &other);

	~PolynomialSpectralElement();

	SpectralElement* clone() const;

	PolynomialSpectralElement &operator=(const PolynomialSpectralElement& other);

	// Evaluate the value of the element at x
	Double operator()(const Double x) const;

	// Get the degree of polynomial
	uInt getDegree() const;

};

//# Global functions
// <summary> Global functions </summary>
// <group name=Output>
// Output declaration
ostream &operator<<(ostream &os, const SpectralElement &elem);
// </group>


} //# NAMESPACE CASA - END

#endif

