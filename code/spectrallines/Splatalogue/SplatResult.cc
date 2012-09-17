//# Copyright (C) 2004
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

#include "SplatResult.h"
#include <sstream>

namespace casa {

SplatResult::SplatResult(int speciesId, const string& species,
		const string& chemicalName,
		const string& quantumNumbers, const string& catalogueName,
		bool recommended, long molecularType, const pair<double,string>& frequency,
		const pair<double,string>& smu2, const pair<double,string>& el,
		const pair<double,string>& eu, double logA, double wavelength,
		double intensity) {
	this->_speciesId = speciesId;
	this->_species = species;
	this->_chemicalName = chemicalName;
	this->_quantumNumbers = quantumNumbers;
	this->_catalogueName = catalogueName;
	this->_recommended = recommended;
	this->_molecularType = molecularType;
	this->_frequency = frequency;
	this->_smu2 = smu2;
	this->_el = el;
	this->_eu = eu;
	this->_logA = logA;
	this->_wavelength = wavelength;
	this->_intensity = intensity;

}

int SplatResult::getSpeciesId() const {
	return _speciesId;
}

std::string SplatResult::getSpecies() const {
	return _species;
}
std::string SplatResult::getChemicalName() const {
	return _chemicalName;
}
std::string SplatResult::getQuantumNumbers() const {
	return _quantumNumbers;
}
std::string SplatResult::getCatalogueName() const {
	return _catalogueName;
}
bool SplatResult::isRecommended() const {
	return _recommended;
}
long SplatResult::getMolecularType() const {
	return _molecularType;
}
pair<double,std::string> SplatResult::getFrequency() const {
	return _frequency;
}
pair<double,std::string> SplatResult::getSmu2() const {
	return _smu2;
}
pair<double,std::string> SplatResult::getEL() const {
	return _el;
}
pair<double,std::string> SplatResult::getEU() const {
	return _eu;
}
double SplatResult::getLogA() const {
	return _logA;
}
double SplatResult::getWavelength() const {
	return _wavelength;
}
double SplatResult::getIntensity() const {
	return _intensity;
}
string SplatResult::toString() const {
	ostringstream os;
	os << "Species: "<< _species << "\n";
	os << "Recommended: "<< _recommended << "\n";
	os << "Chemical Name: "<< _chemicalName << "\n";
	os << "Frequency: "<< _frequency.first << "\n";
	os << "Resolved QNs: "<< _quantumNumbers << "\n";
	os << "Intensity: "<< _intensity << "\n";
	os << "SMU2: "<< _smu2.first << "\n";
	os << "LOGA: " << _logA << "\n";
	os << "EL: " << _el.first << "\n";
	os << "EU: " << _eu.first << "\n";
	os << "LINELIST: " << _catalogueName << "\n";
	return os.str();
}
SplatResult::~SplatResult() {
	// TODO Auto-generated destructor stub
}

} /* namespace casa */
