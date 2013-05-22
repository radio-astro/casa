//# SpectralElement.cc: Describes (a set of related) spectral lines
//# Copyright (C) 2001,2004
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
//# $Id: SpectralElement.cc 21024 2011-03-01 11:46:18Z gervandiepen $

#include <components/SpectralComponents/LogTransformedPolynomialSpectralElement.h>

#include <casa/iostream.h>

#define _ORIGIN  String("LogTransformedPolynomialSpectralElement::") + __FUNCTION__ + ":" + String::toString(__LINE__) + ": "


namespace casa { //# NAMESPACE CASA - BEGIN


LogTransformedPolynomialSpectralElement::LogTransformedPolynomialSpectralElement(
	uInt n
) : CompiledSpectralElement(SpectralElement::LOGTRANSPOLY, n) {
	if (n == 0) {
		throw AipsError(_ORIGIN + "n must be greater than zero.");
	}
	_makeFunction();
}

LogTransformedPolynomialSpectralElement::LogTransformedPolynomialSpectralElement(
	const Vector<Double>& param
) : CompiledSpectralElement(SpectralElement::LOGTRANSPOLY, param) {
	_makeFunction();
}

void LogTransformedPolynomialSpectralElement::_makeFunction() {
	ostringstream function;
	function << "ln(p0)";
	uInt n = get().size();
	for (uInt i=1; i<n; i++) {
		if (i == 1) {
			function << "+ p1*ln(x)";
		}
		else {
			function << " + p" << i << "*ln(x)^" << i;
		}
	}
	_setFunction(function.str());
}

LogTransformedPolynomialSpectralElement::LogTransformedPolynomialSpectralElement(
	const LogTransformedPolynomialSpectralElement &other
) : CompiledSpectralElement(other) {}

LogTransformedPolynomialSpectralElement::~LogTransformedPolynomialSpectralElement() {}

LogTransformedPolynomialSpectralElement& LogTransformedPolynomialSpectralElement::operator=(
	const LogTransformedPolynomialSpectralElement& other
) {
	if (this != &other) {
		CompiledSpectralElement::operator=(other);
	}
	return *this;
}

SpectralElement* LogTransformedPolynomialSpectralElement::clone() const {
	return new LogTransformedPolynomialSpectralElement(*this);
}

} //# NAMESPACE CASA - END

