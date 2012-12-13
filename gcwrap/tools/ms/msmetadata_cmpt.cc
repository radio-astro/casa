// -*- C++ -*-
//# Framework independent implementation file for ms..
//# Copyright (C) 2006-2007-2008
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
//# @author 
//# @version 
//////////////////////////////////////////////////////////////////////////////

#include <msmetadata_cmpt.h>

#include <tools/ms/msmetadata_forward.h>

#include <casa/Logging/LogIO.h>
#include <casacore/casa/OS/PrecTimer.h>
#include <ms/MeasurementSets/MSMetaData.h>

#include <casa/namespace.h>

#define _ORIGIN *_log << LogOrigin("msmetadata_cmpt.cc", __FUNCTION__, __LINE__);
// common method scaffold
#define _FUNC(BODY) \
	try { \
		_ORIGIN; \
		_isAttached(); \
		BODY \
	} \
	catch (const AipsError& x) { \
		_handleException(x); \
	}

#define _FUNC2(BODY) \
	try { \
		_ORIGIN; \
		BODY \
	} \
	catch (const AipsError& x) { \
		_handleException(x); \
	}

namespace casac {

msmetadata::msmetadata() : _log(new LogIO()) {}

msmetadata::~msmetadata() {}

int msmetadata::antennaid(const string& name) {
	_FUNC(
		return _msmd->getAntennaID(name);
	)
}

string msmetadata::antennaname(int antennaid) {
	_FUNC(
		if (antennaid < 0) {
			throw AipsError("Antenna ID must be nonnegative.");
		}
		return _msmd->getAntennaName(antennaid);
	)
}

vector<int> msmetadata::chanavgspws() {
	_FUNC (
		return _setUIntToVectorInt(_msmd->getChannelAvgSpw());
	)
}

bool msmetadata::done() {
	_FUNC2(
		_msmd.reset(0);
		return true;
	)
}

vector<int> msmetadata::fdmspws() {
	_FUNC(
		return _setUIntToVectorInt(_msmd->getFDMSpw());
	)
}

variant* msmetadata::fieldsforintent(const string& intent, const bool asnames) {
	_FUNC(
		std::set<uInt> ids = _msmd->getFieldsForIntent(intent);
		if (asnames) {
			return new variant(_fieldNames(ids));
		}
		else {
			return new variant(_setUIntToVectorInt(ids));
		}
	)
}

vector<int> msmetadata::fieldsforname(const string& name) {
	_FUNC(
		return _setUIntToVectorInt(_msmd->getFieldIDsForField(name));
	)
}

variant* msmetadata::fieldsforscan(const int scan, const bool asnames) {
	_FUNC(
		if (scan < 0) {
			throw AipsError("Scan number must be nonnegative.");
		}
		std::set<uInt> ids = _msmd->getFieldsForScan(scan);
		if (asnames) {
			return new variant(_fieldNames(ids));
		}
		else {
			return new variant(
				_setUIntToVectorInt(ids)
			);
		}
	)
}

variant* msmetadata::fieldsforscans(const vector<int>& scans, const bool asnames) {
	_FUNC(
		std::set<uInt> uscans;
		for (
			vector<int>::const_iterator scan=scans.begin();
			scan!=scans.end(); scan++
		) {
			if (*scan < 0) {
				throw AipsError("All scan numbers must be nonnegative.");
			}
			uscans.insert(*scan);
		}
		std::set<uInt> ids = _msmd->getFieldsForScans(uscans);
		if (asnames) {
			return new variant(_fieldNames(ids));
		}
		else {
			return new variant(
				_setUIntToVectorInt(ids)
			);
		}
	)
}

variant* msmetadata::fieldsforspw(const int spw, const bool asnames) {
	_FUNC(
		if (spw < 0) {
			throw AipsError("Spectral window ID must be nonnegative.");
		}
		if (asnames) {
			return new variant(
				_setStringToVectorString(_msmd->getFieldNamesForSpw(spw))
			);
		}
		else {
			return new variant(
				_setUIntToVectorInt(_msmd->getFieldIDsForSpw(spw))
			);
		}
	)
}

vector<int> msmetadata::fieldsfortimes(const double center, const double tol) {
	_FUNC(
		return _setUIntToVectorInt(_msmd->getFieldsForTimes(center, tol));
	)
}

vector<string> msmetadata::intents() {
	_FUNC(
		return _setStringToVectorString(_msmd->getIntents());
	)
}

vector<string> msmetadata::intentsforscan(int scan) {
	_FUNC(
		if (scan < 0) {
			throw AipsError("Scan number must be nonnegative.");
		}
		return _setStringToVectorString(_msmd->getIntentsForScan(scan));
	)
}

vector<string> msmetadata::intentsforspw(int spw) {
	_FUNC(
		if (spw < 0) {
			throw AipsError("Spectral window ID must be nonnegative.");
		}
		return _setStringToVectorString(_msmd->getIntentsForSpw(spw));
	)
}

string msmetadata::nameforfield(const int fieldid) {
	_FUNC(
		if (fieldid < 0) {
			throw AipsError("Field ID must be nonnegative.");
		}
		return _msmd->getFieldNameForFieldID(fieldid);
	)
}

int msmetadata::nantennas() {
	_FUNC(
		return _msmd->nAntennas();
	)
}

int msmetadata::nfields() {
	_FUNC(
		return _msmd->nFields();
	)
}

int msmetadata::nscans() {
	_FUNC(
		return _msmd->nScans();
	)
}

int msmetadata::nspw() {
	_FUNC(
		return _msmd->nSpw();
	)
}

int msmetadata::nstates() {
	_FUNC(
		return _msmd->nStates();
	)
}

int msmetadata::nvis() {
	_FUNC(
		return _msmd->nVisibilities();
	)
}

bool msmetadata::open(const string& msname) {
	_FUNC2(
		_msmd.reset(new MSMetaData(MeasurementSet(msname)));
		*_log << LogIO::NORMAL << "Read metadata from "
			<< _msmd->nVisibilities() << " visibilities." << LogIO::POST;
		return true;
	)
}

vector<int> msmetadata::scannumbers() {
	_FUNC(
		return _setUIntToVectorInt(_msmd->getScanNumbers());
	)
}

vector<int> msmetadata::scansforfield(const variant& field) {
	_FUNC(
		switch (field.type()) {
		case variant::INT:
			return _setUIntToVectorInt(_msmd->getScansForFieldID(field.toInt()));
			break;
		case variant::STRING:
			return _setUIntToVectorInt(_msmd->getScansForField(field.toString()));
			break;
		default:
			throw AipsError("Unacceptable type for field parameter.");
		}
	)
}

vector<int> msmetadata::scansforintent(const string& intent) {
	_FUNC(
		return _setUIntToVectorInt(_msmd->getScansForIntent(intent));
	)
}

vector<int> msmetadata::scansforspw(const int spw) {
	_FUNC(
		if (spw < 0) {
			throw AipsError("spw must be nonnegative");
		}
		return _setUIntToVectorInt(_msmd->getScansForSpw(spw));
	)
}

vector<int> msmetadata::scansfortimes(const double center, const double tol) {
	_FUNC(
		return _setUIntToVectorInt(_msmd->getScansForTimes(center, tol));
	)
}

vector<int> msmetadata::scansforstate(const int state) {
	_FUNC(
		if (state < 0) {
			throw AipsError("State ID must be nonnegative.");
		}
		return _setUIntToVectorInt(_msmd->getScansForState(state));
	)
}

vector<int> msmetadata::spwsforintent(const string& intent) {
	_FUNC(
		return _setUIntToVectorInt(_msmd->getSpwsForIntent(intent));
	)
}

vector<int> msmetadata::spwsforfield(const variant& field) {
	_FUNC(
		switch (field.type()) {
		case variant::INT:
			return _setUIntToVectorInt(_msmd->getSpwsForField(field.toInt()));
			break;
		case variant::STRING:
			return _setUIntToVectorInt(_msmd->getSpwsForField(field.toString()));
			break;
		default:
			throw AipsError("Unacceptable type for field parameter.");
		}
	)
}

vector<int> msmetadata::spwsforscan(const int scan) {
	_FUNC(
		if (scan < 0) {
			throw AipsError("Scan must be nonnegative");
		}
		return _setUIntToVectorInt(_msmd->getSpwsForScan(scan));
	)
}

vector<int> msmetadata::statesforscan(const int scan) {
	_FUNC(
		if (scan < 0) {
			throw AipsError("Scan number must be nonnegative");
		}
		return _setUIntToVectorInt(_msmd->getStatesForScan(scan));
	)
}

vector<double> msmetadata::timesforfield(const int field) {
	_FUNC(
		if (field < 0) {
			throw AipsError("Field ID must be nonnegative");
		}
		return _setDoubleToVectorDouble(_msmd->getTimesForField(field));
	)
}

vector<double> msmetadata::timesforscan(const int scan) {
	_FUNC(
		if (scan < 0) {
			throw AipsError("Scan number must be nonnegative");
		}
		return _setDoubleToVectorDouble(_msmd->getTimesForScan(scan));
	)
}

vector<double> msmetadata::timesforscans(const vector<int>& scans) {
	_FUNC(
		for (
			vector<int>::const_iterator iter=scans.begin();
			iter!=scans.end(); iter++
		) {
			if (*iter < 0) {
				throw AipsError("All scan numbers must be nonnegative");
			}
		}
		std::set<uInt> scanSet(scans.begin(), scans.end());
		return _setDoubleToVectorDouble(_msmd->getTimesForScans(scanSet));
	)
}

vector<int> msmetadata::tdmspws() {
	_FUNC(
		return _setUIntToVectorInt(_msmd->getTDMSpw());
	)
}

vector<int> msmetadata::wvrspws() {
	_FUNC(
		return _setUIntToVectorInt(_msmd->getWVRSpw());
	)
}

msmetadata::msmetadata(const MeasurementSet& ms) : _msmd(new MSMetaData(ms)), _log(new LogIO()) {
	*_log << LogIO::NORMAL << "Read metadata from "
		<< _msmd->nVisibilities() << " visibilities." << LogIO::POST;
}

vector<string> msmetadata::_fieldNames(const set<uint>& ids) {
	vector<string> names;
	for (
		std::set<uInt>::const_iterator iter=ids.begin();
		iter!=ids.end(); iter++
	) {
		names.push_back(_msmd->getFieldNameForFieldID(*iter));
	}
	return names;
}

bool msmetadata::_isAttached(const bool throwExceptionIfNotAttached) const {
	if (_msmd.get() != 0) {
		return True;
	}
	else if (throwExceptionIfNotAttached) {
		throw AipsError("Tool is not attached to an MS. Use open()");
	}
	return false;
}

void msmetadata::_handleException(const AipsError& x) const {
	*_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	RETHROW(x);
}

std::vector<double> msmetadata::_setDoubleToVectorDouble(
	const std::set<casa::Double>& inset
) const {
	vector<double> output;
	std::copy(inset.begin(), inset.end(), std::back_inserter(output));
	return output;
}

std::vector<std::string> msmetadata::_setStringToVectorString(
	const std::set<casa::String>& inset
) const {
	vector<string> output;
	std::copy(inset.begin(), inset.end(), std::back_inserter(output));
	return output;
}

std::vector<int> msmetadata::_setUIntToVectorInt(const std::set<casa::uInt>& inset) const {
	vector<int> x;
	for (
		std::set<uInt>::const_iterator iter=inset.begin();
		iter!=inset.end();iter++
	) {
		x.push_back(*iter);
	}
	return x;
}

} // casac namespace

