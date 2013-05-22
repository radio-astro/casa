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

//# Includes
#include <components/SpectralComponents/GaussianSpectralElement.h>

#include <casa/BasicSL/Constants.h>

#include <casa/iostream.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//# Constants
const Double GaussianSpectralElement::SigmaToFWHM = sqrt(8.0*C::ln2);

GaussianSpectralElement::GaussianSpectralElement()
: PCFSpectralElement(SpectralElement::GAUSSIAN, 1, 0, 2*sqrt(C::ln2)/C::pi) {}

GaussianSpectralElement::GaussianSpectralElement(
	const Double ampl, const Double center, const Double sigma
) : PCFSpectralElement(SpectralElement::GAUSSIAN, ampl, center, sigma) {}

GaussianSpectralElement::GaussianSpectralElement(
	const Vector<Double> &param
) : PCFSpectralElement(SpectralElement::GAUSSIAN, param) {}

GaussianSpectralElement::GaussianSpectralElement(
	const GaussianSpectralElement &other
) : PCFSpectralElement(other) {}

GaussianSpectralElement::~GaussianSpectralElement() {}


SpectralElement* GaussianSpectralElement::clone() const {
	return new GaussianSpectralElement(*this);
}

Double GaussianSpectralElement::operator()(const Double x) const {
	Vector<Double> p = get();
    return p(0)*exp(-0.5 * (x-p(1))*(x-p(1)) / p(2)/p(2));
}

Double GaussianSpectralElement::getSigma() const {
	return get()[2];
}

Double GaussianSpectralElement::getFWHM() const {
	return sigmaToFWHM(get()[2]);
}

Double GaussianSpectralElement::getSigmaErr() const {
	return getError()[2];
}

Double GaussianSpectralElement::getFWHMErr() const {
	return sigmaToFWHM(getError()[2]);
}

void GaussianSpectralElement::setSigma(Double sigma) {
	if (sigma < 0) {
		sigma = -sigma;
	}
	Vector<Double> p = get();
	p[2] = sigma;
	_set(p);
	Vector<Double> err = getError();
	err(2) = 0;
	setError(err);
}

void GaussianSpectralElement::setFWHM(Double fwhm) {
	setSigma(sigmaFromFWHM(fwhm));
}

void GaussianSpectralElement::fixSigma(const Bool isFixed) {
	Vector<Bool> myFixed = fixed();
	myFixed[2] = isFixed;
	fix(myFixed);
}

Bool GaussianSpectralElement::fixedSigma() const {
	return fixed()[2];
}

Double GaussianSpectralElement::getIntegral() const {
	return sqrt(C::pi_4/C::ln2) * getAmpl() * getFWHM();
}

Bool GaussianSpectralElement::toRecord(
	RecordInterface& out
) const {
	PCFSpectralElement::toRecord(out);
	Vector<Double> p = out.asArrayDouble("parameters");
	Vector<Double> e = out.asArrayDouble("errors");
	p[2] = getFWHM();
	out.define("parameters", p);
	e[2] = getFWHMErr();
	out.define("errors", e);
	return True;
}

ostream &operator<<(ostream &os, const GaussianSpectralElement &elem) {
	os << SpectralElement::fromType((elem.getType())) << " element: " << endl;
    os << "  Amplitude: " << elem.getAmpl() << ", " << elem.getAmplErr();
    if (elem.fixedAmpl()) os << " (fixed)";
    os << endl << "  Center:    " << elem.getCenter() << ", " << elem.getCenterErr();
    if (elem.fixedCenter()) os << " (fixed)";
    os << endl << "  Sigma:     " << elem.getSigma() << ", " << elem.getSigmaErr();
    if (elem.fixedSigma()) os << " (fixed)";
    os << endl;
    return os;
}

Double GaussianSpectralElement::sigmaToFWHM (const Double sigma) {
   return SigmaToFWHM * sigma;
}

Double GaussianSpectralElement::sigmaFromFWHM(const Double fwhm) {
  return fwhm / SigmaToFWHM;
}

} //# NAMESPACE CASA - END

