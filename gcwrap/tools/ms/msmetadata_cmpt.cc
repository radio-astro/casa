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

#include <casa/Containers/Record.h>
#include <casa/Logging/LogIO.h>
#include <casa/Quanta/QuantumHolder.h>
#include <measures/Measures/MeasureHolder.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSMetaDataOnDemand.h>

#include <casa/namespace.h>

#define _ORIGIN *_log << LogOrigin("msmetadata_cmpt.cc", __FUNCTION__, __LINE__);
// common method scaffold

#define COMMA ,

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

vector<int> msmetadata::antennaids(const variant& names) {
	_FUNC(
		vector<String> myNames;
		variant::TYPE type = names.type();
		if (type == variant::STRING) {
			myNames.push_back(names.toString());
		}
		else if (type == variant::STRINGVEC) {
			vector<string> tmp = names.toStringVec();
			vector<string>::const_iterator end = tmp.end();
			for (
				vector<string>::const_iterator iter=tmp.begin();
				iter!=end; iter++
			) {
				myNames.push_back(*iter);
			}
		}
		else {
			*_log << "Unsupported type for parameter names. Must be either a string or string array"
				<< LogIO::EXCEPTION;
		}
		return _vectorUIntToVectorInt(_msmd->getAntennaIDs(myNames));
	)
	return vector<int>();
}

vector<string> msmetadata::antennanames(const variant& antennaids) {
	_FUNC(
		vector<uInt> myIDs;
		variant::TYPE type = antennaids.type();
		if (type == variant::INT) {
			Int id = antennaids.toInt();
			if (id < 0) {
				*_log << "Antenna ID must be nonnegative."
					<< LogIO::EXCEPTION;
			}
			myIDs.push_back(id);
		}
		else if (type == variant::INTVEC) {
			vector<Int> tmp = antennaids.toIntVec();
			vector<Int>::const_iterator end = tmp.end();
			for (
				vector<Int>::const_iterator iter=tmp.begin();
				iter!=tmp.end(); iter++
			) {
				if (*iter < 0) {
					*_log << "Antenna ID must be nonnegative."
						<< LogIO::EXCEPTION;
				}
				myIDs.push_back(*iter);
			}
		}
		else {
			*_log << "Unsupported type for parameter antennaids. "
				<< "Must be either an integer or integer array."
				<< LogIO::EXCEPTION;
		}
		std::map<String COMMA uInt> namesToIDsMap;
		return _vectorStringToStdVectorString(
			_msmd->getAntennaNames(namesToIDsMap COMMA myIDs)
		);
	)
	return vector<string>();
}

record* msmetadata::antennaoffset(const variant& which) {
	_FUNC(
		variant::TYPE type = which.type();
		Quantum<Vector<Double> > out;
		if (type == variant::INT) {
			out = _msmd->getAntennaOffset(which.toInt());
		}
		else if (type == variant::STRING) {
			out = _msmd->getAntennaOffset(which.toString());
		}
		else {
			*_log << "Unsupported type for input parameter which. Supported types are int and string"
				<< LogIO::EXCEPTION;
		}
		Vector<Double> v = out.getValue();
		String u = out.getUnit();
		QuantumHolder longitude(casa::Quantity(v[0], u));
		QuantumHolder latitude(casa::Quantity(v[1], u));
		QuantumHolder elevation(casa::Quantity(v[2], u));
		Record x;
		Record outRec;
		longitude.toRecord(x);
		outRec.defineRecord("longitude offset", x);
		latitude.toRecord(x);
		outRec.defineRecord("latitude offset", x);
		elevation.toRecord(x);
		outRec.defineRecord("elevation offset", x);
		return fromRecord(outRec);
	)
	return 0;
}

record* msmetadata::antennaposition(const int which) {
	_FUNC(
		MeasureHolder out(_msmd->getAntennaPositions(vector<uInt>(1, which))[0]);
		Record outRec;
		out.toRecord(outRec);
		return fromRecord(outRec);
	)
	return 0;
}

record* msmetadata::antennaposition(const string& name) {
	_FUNC(
		MeasureHolder out(_msmd->getAntennaPositions(vector<String>(1, String(name)))[0]);
		Record outRec;
		out.toRecord(outRec);
		return fromRecord(outRec);
	)
	return 0;
}


int msmetadata::baseband(int spw) {
	_FUNC2 (
		if (spw < 0 || spw >= (int)_msmd->nSpw(True)) {
			*_log << "Spectral window ID out of range" << LogIO::EXCEPTION;
		}
		return _msmd->getBBCNos()[spw];
	)
	return 0;
}

variant* msmetadata::baselines() {
	_FUNC (
		Matrix<Bool> baselines = _msmd->getUniqueBaselines();
		vector<bool> values = baselines.tovector();
		vector<int> shape = baselines.shape().asStdVector();
		return new variant(values, shape);
	)
	return 0;
}

vector<int> msmetadata::chanavgspws() {
	_FUNC (
		return _setUIntToVectorInt(_msmd->getChannelAvgSpw());
	)
	return vector<int>();
}

vector<double> msmetadata::chanfreqs(int spw, const string& unit) {
	_FUNC2 (
		if (spw < 0 || spw >= (int)_msmd->nSpw(True)) {
			*_log << "Spectral window ID out of range" << LogIO::EXCEPTION;
		}
		return _msmd->getChanFreqs()[spw].getValue(Unit(unit)).tovector();
	)
	return vector<double>();
}


bool msmetadata::close() {
	_FUNC2(
		_msmd.reset(0);
		return true;
	)
	return false;
}

bool msmetadata::done() {
	_FUNC2(
		_msmd.reset(0);
		return true;
	)
	return false;
}


record* msmetadata::effexposuretime() {
	return fromRecord(
		QuantumHolder(_msmd->getEffectiveTotalExposureTime()).toRecord()
	);
	return 0;
}

vector<int> msmetadata::fdmspws() {
	_FUNC(
		return _setUIntToVectorInt(_msmd->getFDMSpw());
	)
	return vector<int>();
}

variant* msmetadata::fieldsforintent(const string& intent, const bool asnames) {
	_FUNC(
		std::set<Int> ids = _msmd->getFieldsForIntent(intent);
		variant *x;
		if (ids.size() == 0) {
			*_log << LogIO::WARN << "Intent " << intent
				<< " does not exist in this dataset." << LogIO::POST;
			x = asnames
				? new variant(vector<string>(0))
				: new variant(vector<int>(0));
		}
		else {
			x = asnames
				? new variant(_fieldNames(ids))
				: new variant(_setIntToVectorInt(ids));
		}
		return x;
	)
	return 0;
}

vector<int> msmetadata::fieldsforname(const string& name) {
	_FUNC(
		return _setIntToVectorInt(_msmd->getFieldIDsForField(name));
	)
	return vector<int>();
}

variant* msmetadata::fieldsforscan(const int scan, const bool asnames) {
	_FUNC(
		if (scan < 0) {
			throw AipsError("Scan number must be nonnegative.");
		}
		std::set<Int> ids = _msmd->getFieldsForScan(scan);
		if (asnames) {
			return new variant(_fieldNames(ids));
		}
		else {
			return new variant(
				_setIntToVectorInt(ids)
			);
		}
	)
	return 0;
}

variant* msmetadata::fieldsforscans(const vector<int>& scans, const bool asnames) {
	_FUNC(
		std::set<Int> uscans;
		for (
			vector<int>::const_iterator scan=scans.begin();
			scan!=scans.end(); scan++
		) {
			if (*scan < 0) {
				throw AipsError("All scan numbers must be nonnegative.");
			}
			uscans.insert(*scan);
		}
		std::set<Int> ids = _msmd->getFieldsForScans(uscans);
		if (asnames) {
			return new variant(_fieldNames(ids));
		}
		else {
			return new variant(
				_setIntToVectorInt(ids)
			);
		}
	)
	return 0;
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
				_setIntToVectorInt(_msmd->getFieldIDsForSpw(spw))
			);
		}
	)
	return 0;
}

vector<int> msmetadata::fieldsfortimes(const double center, const double tol) {
	_FUNC(
		return _setIntToVectorInt(_msmd->getFieldsForTimes(center, tol));
	)
	return vector<int>();
}

vector<string> msmetadata::intents() {
	_FUNC(
		return _setStringToVectorString(_msmd->getIntents());
	)
	return vector<string>();
}

vector<string> msmetadata::intentsforfield(const variant& field) {
	_FUNC(
		Int id = -1;
		switch (field.type()) {
		case variant::STRING:
			id = *(_msmd->getFieldIDsForField(field.toString()).begin());
			break;
		case variant::INT:
			id = field.toInt();
			break;
		default:
			*_log << "Unsupported type for field which must be "
				<< "a nonnegative int or string." << LogIO::EXCEPTION;
		}
		if (id < 0) {
			throw AipsError("field must be nonnegative if an int.");
		}
		return _setStringToVectorString(_msmd->getIntentsForField(id));
	)
	return vector<string>();
}

vector<string> msmetadata::intentsforscan(int scan) {
	_FUNC(
		if (scan < 0) {
			throw AipsError("Scan number must be nonnegative.");
		}
		return _setStringToVectorString(_msmd->getIntentsForScan(scan));
	)
	return vector<string>();
}

vector<string> msmetadata::intentsforspw(int spw) {
	_FUNC(
		if (spw < 0) {
			throw AipsError("Spectral window ID must be nonnegative.");
		}
		return _setStringToVectorString(_msmd->getIntentsForSpw(spw));
	)
	return vector<string>();
}

double msmetadata::meanfreq(int spw, const string& unit) {
	_FUNC2 (
		if (spw < 0 || spw >= (int)_msmd->nSpw(True)) {
			*_log << "Spectral window ID out of range" << LogIO::EXCEPTION;
		}
		return _msmd->getMeanFreqs()[spw].getValue(Unit(unit));
	)
	return 0;
}

vector<string> msmetadata::namesforfields(const variant& fieldids) {
	_FUNC(
		variant::TYPE myType = fieldids.type();
		vector<Int> fieldIDs;
		if (myType == variant::INT) {
			Int id = fieldids.toInt();
			if (id < 0) {
				throw AipsError("Field ID must be nonnegative.");
			}
			fieldIDs.push_back(id);
		}
		else if (myType == variant::INTVEC) {
			fieldIDs = fieldids.toIntVec();
			if (min(Vector<Int>(fieldids.toIntVec())) < 0 ) {
				throw AipsError("All field IDs must be nonnegative.");
			}
		}
		else if (fieldids.size() != 0) {
			throw AipsError(
				"Unsupported type for fieldids. It must be a nonnegative integer or integer array"
			);
		}
		return _vectorStringToStdVectorString(
			_msmd->getFieldNamesForFieldIDs(fieldIDs)
		);
	)
	return vector<string>();
}

int msmetadata::nantennas() {
	_FUNC(
		return _msmd->nAntennas();
	)
	return 0;
}

int msmetadata::nbaselines() {
	_FUNC(
		return _msmd->nBaselines();
	)
	return 0;
}

int msmetadata::nchan(int spw) {
	_FUNC2 (
		if (spw < 0 || spw >= (int)_msmd->nSpw(True)) {
			*_log << "Spectral window ID out of range" << LogIO::EXCEPTION;
		}
		return _msmd->nChans()[spw];
	)
	return 0;
}


int msmetadata::nfields() {
	_FUNC(
		return _msmd->nFields();
	)
	return 0;

}

int msmetadata::nscans() {
	_FUNC(
		return _msmd->nScans();
	)
	return 0;
}

int msmetadata::nspw(bool includewvr) {
	_FUNC(
		return _msmd->nSpw(includewvr);
	)
	return 0;
}

int msmetadata::nstates() {
	_FUNC(
		return _msmd->nStates();
	)
	return 0;
}

double msmetadata::nrows(const bool ac, const bool flagged) {
	_FUNC(
		if (ac) {
			return flagged ? _msmd->nRows() : _msmd->nUnflaggedRows();
		}
		else {
			return flagged ? _msmd->nRows(MSMetaData::CROSS)
				: _msmd->nUnflaggedRows(MSMetaData::CROSS);
		}
	)
	return 0;
}

vector<string> msmetadata::observatorynames() {
	_FUNC(
		return _vectorStringToStdVectorString(_msmd->getObservatoryNames());
	)
	return vector<string>();
}

record* msmetadata::observatoryposition(const int which) {
	_FUNC(
		MeasureHolder out(_msmd->getObservatoryPosition(which));
		Record outRec;
		String error;
		if (!out.toRecord(error, outRec)) {
			error += "Failed to generate position.\n";
			*_log << LogIO::SEVERE << error << LogIO::POST;
			return 0;
		}
		return fromRecord(outRec);
	)
	return 0;
}

void msmetadata::_init(const casa::MeasurementSet *const &ms, const float cachesize) {
    _msmd.reset(new MSMetaDataOnDemand(ms, cachesize));
}


bool msmetadata::open(const string& msname, const float cachesize) {
	_FUNC2(
	       _ms.reset(new MeasurementSet(msname));
	       _init(_ms.get(), cachesize);
	       return true;
	)
	return false;
}

vector<int> msmetadata::scannumbers() {
	_FUNC(
		return _setIntToVectorInt(_msmd->getScanNumbers());
	)
	return vector<int>();
}

vector<int> msmetadata::scansforfield(const variant& field) {
	_FUNC(
		switch (field.type()) {
		case variant::INT:
			return _setIntToVectorInt(_msmd->getScansForFieldID(field.toInt()));
			break;
		case variant::STRING:
			return _setIntToVectorInt(_msmd->getScansForField(field.toString()));
			break;
		default:
			throw AipsError("Unacceptable type for field parameter.");
		}
	)
	return vector<int>();
}

vector<int> msmetadata::scansforintent(const string& intent) {
	_FUNC(
		return _setIntToVectorInt(_msmd->getScansForIntent(intent));
	)
	return vector<int>();
}

vector<int> msmetadata::scansforspw(const int spw) {
	_FUNC(
		if (spw < 0) {
			throw AipsError("spw must be nonnegative");
		}
		return _setIntToVectorInt(_msmd->getScansForSpw(spw));
	)
	return vector<int>();
}

vector<int> msmetadata::scansfortimes(const double center, const double tol) {
	_FUNC(
		return _setIntToVectorInt(_msmd->getScansForTimes(center, tol));
	)
	return vector<int>();
}

vector<int> msmetadata::scansforstate(const int state) {
	_FUNC(
		if (state < 0) {
			throw AipsError("State ID must be nonnegative.");
		}
		return _setIntToVectorInt(_msmd->getScansForState(state));
	)
	return vector<int>();
}

int msmetadata::sideband(int spw) {
	_FUNC2 (
		if (spw < 0 || spw >= (int)_msmd->nSpw(True)) {
			*_log << "Spectral window ID out of range" << LogIO::EXCEPTION;
		}
		return _msmd->getNetSidebands()[spw];
	)
	return 0;
}

vector<int> msmetadata::spwsforintent(const string& intent) {
	_FUNC(
		vector<int> x = _setUIntToVectorInt(_msmd->getSpwsForIntent(intent));
		if (x.size() == 0) {
			*_log << LogIO::WARN << "Intent " << intent
				<< " does not exist in this dataset." << LogIO::POST;
		}
		return x;
	)
	return vector<int>();
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
	return vector<int>();
}

vector<int> msmetadata::spwsforscan(const int scan) {
	_FUNC(
		if (scan < 0) {
			throw AipsError("Scan must be nonnegative");
		}
		return _setUIntToVectorInt(_msmd->getSpwsForScan(scan));
	)
	return vector<int>();

}

vector<int> msmetadata::statesforscan(const int scan) {
	_FUNC(
		if (scan < 0) {
			throw AipsError("Scan number must be nonnegative");
		}
		return _setIntToVectorInt(_msmd->getStatesForScan(scan));
	)
	return vector<int>();
}

vector<double> msmetadata::timesforfield(const int field) {
	_FUNC(
		if (field < 0) {
			throw AipsError("Field ID must be nonnegative");
		}
		return _setDoubleToVectorDouble(_msmd->getTimesForField(field));
	)
	return vector<double>();
}

vector<double> msmetadata::timesforscan(const int scan) {
	_FUNC(
		if (scan < 0) {
			throw AipsError("Scan number must be nonnegative");
		}
		return _setDoubleToVectorDouble(_msmd->getTimesForScan(scan));
	)
	return vector<double>();
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
		std::set<Int> scanSet(scans.begin(), scans.end());
		return _setDoubleToVectorDouble(_msmd->getTimesForScans(scanSet));
	)
	return vector<double>();
}

vector<int> msmetadata::tdmspws() {
	_FUNC(
		return _setUIntToVectorInt(_msmd->getTDMSpw());
	)
	return vector<int>();
}

vector<int> msmetadata::wvrspws() {
	_FUNC(
		return _setUIntToVectorInt(_msmd->getWVRSpw());
	)
	return vector<int>();
}

/*
msmetadata::msmetadata(const MeasurementSet& ms) : _msmd(new MSMetaDataPreload(ms)), _log(new LogIO()) {
	*_log << LogIO::NORMAL << "Read metadata from "
		<< _msmd->nRows() << " rows." << LogIO::POST;
}
*/

msmetadata::msmetadata(
	const MeasurementSet *const &ms, const float cachesize
) : _msmd(0), _ms(0), _log(new LogIO()) {
	_init(ms, cachesize);
}


vector<string> msmetadata::_fieldNames(const set<int>& ids) {
	return _vectorStringToStdVectorString(
		_msmd->getFieldNamesForFieldIDs(vector<Int>(ids.begin(), ids.end()))
	);
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
) {
	vector<double> output;
	std::copy(inset.begin(), inset.end(), std::back_inserter(output));
	return output;
}

std::vector<std::string> msmetadata::_setStringToVectorString(
	const std::set<casa::String>& inset
) {
	vector<string> output;
	std::copy(inset.begin(), inset.end(), std::back_inserter(output));
    return output;
}

std::vector<int> msmetadata::_setUIntToVectorInt(const std::set<casa::uInt>& inset) {
	vector<int> output;
    std::copy(inset.begin(), inset.end(), std::back_inserter(output));
	return output;
}

std::vector<int> msmetadata::_setIntToVectorInt(const std::set<casa::Int>& inset) {
	vector<int> output;
    std::copy(inset.begin(), inset.end(), std::back_inserter(output));
	return output;
}

std::vector<std::string> msmetadata::_vectorStringToStdVectorString(const std::vector<casa::String>& inset) {
	vector<string> output;
	std::copy(inset.begin(), inset.end(), std::back_inserter(output));
	return output;
}

std::vector<int> msmetadata::_vectorUIntToVectorInt(const std::vector<uInt>& inset) {
	vector<int> output;
	std::copy(inset.begin(), inset.end(), std::back_inserter(output));
	return output;
}

std::vector<uint> msmetadata::_vectorIntToVectorUInt(const std::vector<Int>& inset) {
	vector<uint> output;
	std::copy(inset.begin(), inset.end(), std::back_inserter(output));
	return output;
}

} // casac namespace

