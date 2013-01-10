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
//#include <casacore/casa/OS/PrecTimer.h>
#include <measures/Measures/MeasureHolder.h>
#include <casa/Quanta/QuantumHolder.h>

#include <ms/MeasurementSets/MSMetaDataPreload.h>

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
		return _vectorStringToStdVectorString(_msmd->getAntennaNames(myIDs));
	)
}

record* msmetadata::antennaoffset(const int which) {
	_FUNC(
		Quantum<Vector<Double> > out = _msmd->getAntennaOffset(which);
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
}


record* msmetadata::antennaoffset(const string& name) {
	_FUNC(
		Quantum<Vector<Double> > out = _msmd->getAntennaOffset(name);
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
}

record* msmetadata::antennaposition(const int which) {
	_FUNC(
		MeasureHolder out(_msmd->getAntennaPositions(vector<uInt>(1, which))[0]);
		Record outRec;
		out.toRecord(outRec);
		return fromRecord(outRec);
	)
}

record* msmetadata::antennaposition(const string& name) {
	_FUNC(
		MeasureHolder out(_msmd->getAntennaPositions(vector<String>(1, String(name)))[0]);
		Record outRec;
		out.toRecord(outRec);
		return fromRecord(outRec);
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

vector<string> msmetadata::namesforfields(const variant& fieldids) {
	_FUNC(
		variant::TYPE myType = fieldids.type();
		vector<uInt> fieldIDs;
		if (myType == variant::INT) {
			Int id = fieldids.toInt();
			if (id < 0) {
				throw AipsError("Field ID must be nonnegative.");
			}
			fieldIDs.push_back(id);
		}
		else if (myType == variant::INTVEC) {
			if (min(Vector<Int>(fieldids.toIntVec())) < 0 ) {
				throw AipsError("All field IDs must be nonnegative.");
			}

			fieldIDs = _vectorIntToVectorUInt(fieldids.toIntVec());
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

vector<string> msmetadata::observatorynames() {
	_FUNC(
		return _vectorStringToStdVectorString(_msmd->getObservatoryNames());
	)
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
}

bool msmetadata::open(const string& msname) {
	_FUNC2(
		_msmd.reset(new MSMetaDataPreload(MeasurementSet(msname)));
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

msmetadata::msmetadata(const MeasurementSet& ms) : _msmd(new MSMetaDataPreload(ms)), _log(new LogIO()) {
	*_log << LogIO::NORMAL << "Read metadata from "
		<< _msmd->nVisibilities() << " visibilities." << LogIO::POST;
}

vector<string> msmetadata::_fieldNames(const set<uint>& ids) {
	return _vectorStringToStdVectorString(
		_msmd->getFieldNamesForFieldIDs(vector<uInt>(ids.begin(), ids.end()))
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
	/*
	for (
		std::set<uInt>::const_iterator iter=inset.begin();
		iter!=inset.end();iter++
	) {
		x.push_back(*iter);
	}
	*/
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

