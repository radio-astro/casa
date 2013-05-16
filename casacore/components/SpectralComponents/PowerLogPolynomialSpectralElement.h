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

#ifndef COMPONENTS_POWERLOGPOLYNOMIALSPECTRALELEMENT_H
#define COMPONENTS_POWERLOGPOLYNOMIALSPECTRALELEMENT_H

#include <components/SpectralComponents/CompiledSpectralElement.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Describes the function most often used for determining spectral index plus higher order terms:
// S_nu = c_0 * nu ** ( c_1 + c_2 * log( nu ) + c_3 * log( nu )**2 + ... )
// where c_1 is the traditional spectral index (alpha).
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="tSpectralFit" demos="">
// </reviewed>

// <prerequisite>
//   <li> <linkto module=SpectralElement>SpectralElement</linkto> module
// </prerequisite>
//
// <etymology>
// From power law, logarithm, and polynomial and spectral line and element
// </etymology>
//
// <synopsis>
// Describes a function that can be used to fit for spectral index and higher order terms

// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// To have a spectral element representing a spectral index function.
// </motivation>


class PowerLogPolynomialSpectralElement: public CompiledSpectralElement {
public:

	// Constructor. The n coefficients c_i to be solved for are
	// c_0 * nu**(c_1 + c_2*log(x) + c_3*log(x)**2 + ... + c_(n-1)*log(x)**(n - 2))
	// where x = nu/nu0
	explicit PowerLogPolynomialSpectralElement(uInt n, Double nu0);

	// Construct with the given parameters. See above constructor for order in which the parameters should
	// be supplied.
	PowerLogPolynomialSpectralElement(const Vector<Double> &param, Double n0);

	// Copy constructor (deep copy)
	PowerLogPolynomialSpectralElement(const PowerLogPolynomialSpectralElement &other);

	~PowerLogPolynomialSpectralElement();

	PowerLogPolynomialSpectralElement &operator=(
		const PowerLogPolynomialSpectralElement& other
	);

	SpectralElement* clone() const;

	Bool toRecord(RecordInterface& out) const;

private:
	Double _nu0;

	void _makeFunction();

};

ostream &operator<<(ostream &os, const PowerLogPolynomialSpectralElement &elem);

} //# NAMESPACE CASA - END

#endif

