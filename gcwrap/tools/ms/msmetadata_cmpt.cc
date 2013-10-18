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

#include <stdcasa/cboost_foreach.h>
#include <boost/regex.hpp>

#include <boost/iterator/counting_iterator.hpp>

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

vector<int> msmetadata::almaspws(
	bool chavg, bool fdm, bool sqld, bool tdm, bool wvr, bool complement
) {
	_FUNC(
	    std::set<uInt> x;
	    if (chavg) {
	    	std::set<uInt> y = _msmd->getChannelAvgSpw();
	    	x.insert(y.begin(), y.end());
	    }
	    if (fdm) {
	    	std::set<uInt> y = _msmd->getFDMSpw();
	    	x.insert(y.begin(), y.end());
	    }
	    if (sqld) {
	    	std::set<uInt> y = _msmd->getSQLDSpw();
	    	x.insert(y.begin(), y.end());
	    }
	    if (tdm) {
	    	std::set<uInt> y = _msmd->getTDMSpw();
	    	x.insert(y.begin(), y.end());
	    }
	    if (wvr) {
	    	std::set<uInt> y = _msmd->getWVRSpw();
	    	x.insert(y.begin(), y.end());
	    }
		if (complement) {
			uInt nspw = _msmd->nSpw(True);
			set<uInt> allSpws(
				boost::counting_iterator<int>(0),
				boost::counting_iterator<int>(nspw)
			);
			vector<uInt> mycompl(nspw);
			vector<uInt>::iterator begin = mycompl.begin();
			vector<uInt>::iterator end = std::set_difference(
				allSpws.begin(), allSpws.end(), x.begin(),
				x.end(), begin
			);
			mycompl.resize(end - begin);
			return _vectorUIntToVectorInt(mycompl);
		}
		return _setUIntToVectorInt(x);
	)
	return vector<int>();
}

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

record* msmetadata::antennaposition(const variant& which) {
	_FUNC(
		variant::TYPE type = which.type();
		ThrowIf(
			type != variant::BOOLVEC && type != variant::INT
			&& type != variant::STRING,
			"Unsupported type for which, must be either int or string"
		);
		MeasureHolder out;
		if (type == variant::BOOLVEC || type == variant::INT) {
			out = MeasureHolder(
				_msmd->getAntennaPositions(
					type == variant::BOOLVEC ? vector<uInt>(1, 0)
					: vector<uInt>(1, which.toInt())
				)[0]
			);
		}
		else {
			out = MeasureHolder(
				_msmd->getAntennaPositions(
					vector<String>(1, which.toString())
				)[0]
			);
		}
		Record outRec;
		out.toRecord(outRec);
		return fromRecord(outRec);
	)
	return 0;
}

variant* msmetadata::bandwidths(const variant& spws) {
	_FUNC(
		variant::TYPE type = spws.type();
		ThrowIf(
			type != variant::INT && type != variant::INTVEC
			&& type != variant::BOOLVEC,
			"Unsupported type for spws (" + spws.typeString()
			+ "), must be either an int or an array of ints"
		)

		int mymax = 0;
		if (type == variant::INT) {
			mymax = spws.toInt();
		}
		else if (type == variant::INTVEC) {
			Vector<Int> tmp(spws.toIntVec());
			ThrowIf(
				min(tmp) < 0,
				"When specified as an array, no element of spws may be < 0"
			);
			mymax = max(tmp);
		}
		_checkSpwId(mymax, False);
		vector<Double> bws = _msmd->getBandWidths();
		if (type == variant::BOOLVEC) {
			return new variant(bws);
		}
		if (type == variant::INT) {
			if (spws.toInt() < 0) {
				return new variant(bws);
			}
			else {
				return new variant(bws[spws.toInt()]);
			}
		}
		else {
			vector<Double> ret;
			foreach_(int id, spws.toIntVec()) {
				ret.push_back(bws[id]);
			}
			return new variant(ret);
		}
	)
	return 0;
}

int msmetadata::baseband(int spw) {
	_FUNC (
		_checkSpwId(spw, True);
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
			/*
		*_log << LogIO::WARN << "This method is deprecated and will be removed. "
			<< "Use almaspws(chavg=True) instead." << LogIO::POST;
			*/
		return _setUIntToVectorInt(_msmd->getChannelAvgSpw());
	)
	return vector<int>();
}

vector<double> msmetadata::chanfreqs(int spw, const string& unit) {
	_FUNC2 (
		_checkSpwId(spw, True);
		return _msmd->getChanFreqs()[spw].getValue(Unit(unit)).tovector();
	)
	return vector<double>();
}

vector<double> msmetadata::chanwidths(int spw, const string& unit) {
	_FUNC2 (
		_checkSpwId(spw, True);
		return _msmd->getChanWidths()[spw].getValue(Unit(unit)).tovector();
	)
	return vector<double>();
}

bool msmetadata::close() {
	_FUNC2(
		_msmd.reset(0);
		_ms.reset(0);
		return true;
	)
	return false;
}

vector<int> msmetadata::datadescids(int spw, int pol) {
	_FUNC(
		_checkSpwId(spw, False);
		_checkPolId(pol, False);
		std::map<std::pair<uInt COMMA uInt> COMMA Int> mymap = _msmd->getSpwIDPolIDToDataDescIDMap();
		vector<int> ddids;
		foreach_ (std::pair<std::pair<uInt COMMA uInt> COMMA Int> iter, mymap) {
			uInt myspw = iter.first.first;
			uInt mypol = iter.first.second;
			Int ddid = iter.second;
			if (
				(spw < 0 || (Int)myspw == spw)
				&& (pol < 0 || (Int)mypol == pol)
			) {
				ddids.push_back(ddid);
			}
		}
		std::sort (ddids.begin(),ddids.end());
		return ddids;
	)
	return vector<int>();
}

bool msmetadata::done() {
	_FUNC2(
		_msmd.reset(0);
		_ms.reset(0);
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
			/*
		*_log << LogIO::WARN << __FUNCTION__ << " is deprecated and will be removed. "
			<< "Use almaspws(fdm=True) instead." << LogIO::POST;
			*/
		return _setUIntToVectorInt(_msmd->getFDMSpw());
	)
	return vector<int>();
}

variant* msmetadata::fieldsforintent(
	const string& intent, const bool asnames, const bool regex
) {
	//_FUNC(
		std::set<Int> ids;
		if (regex) {
			std::map<String COMMA std::set<Int> > mymap = _msmd->getIntentToFieldsMap();
			ids = _idsFromRegex(mymap, intent);
		}
		else {
			ids = _msmd->getFieldsForIntent(intent);
		}
		variant *x;
		if (ids.size() == 0) {
			*_log << LogIO::WARN << "No intent " << (regex ? "matching '" : "'")
				<< intent << "' exists in this dataset." << LogIO::POST;
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
	//)
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
		_checkSpwId(spw, True);
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
		_checkSpwId(spw, True);
		return _setStringToVectorString(_msmd->getIntentsForSpw(spw));
	)
	return vector<string>();
}

double msmetadata::meanfreq(int spw, const string& unit) {
	_FUNC2 (
		_checkSpwId(spw, True);
		return _msmd->getMeanFreqs()[spw].getValue(Unit(unit));
	)
	return 0;
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
			vector<Int> kk = fieldids.toIntVec();
			if (min(Vector<Int>(kk)) < 0 ) {
				throw AipsError("All field IDs must be nonnegative.");
			}
			fieldIDs = _vectorIntToVectorUInt(kk);
		}
		else if (fieldids.size() != 0) {
			throw AipsError(
				"Unsupported type for fieldids. It must be a nonnegative integer or nonnegative integer array"
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
		_checkSpwId(spw, True);
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

int msmetadata::nobservations() {
	_FUNC(
		return _msmd->nObservations();
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

vector<int> msmetadata::scansforintent(const string& intent, bool regex) {
	_FUNC(
		if (regex) {
			std::map<String COMMA std::set<Int> > mymap = _msmd->getIntentToScansMap();
			std::set<Int> ids = _idsFromRegex(mymap, intent);
			return _setIntToVectorInt(ids);
		}
		else {
			return _setIntToVectorInt(_msmd->getScansForIntent(intent));
		}
	)
	return vector<int>();
}

vector<int> msmetadata::scansforspw(const int spw) {
	_FUNC(
		_checkSpwId(spw, True);
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
		_checkSpwId(spw, True);
		return _msmd->getNetSidebands()[spw];
	)
	return 0;
}

variant* msmetadata::spwsforbaseband(int bb, const string& sqldmode) {
	_FUNC(
		String mode = sqldmode;
		mode.downcase();
		MSMetaData::SQLDSwitch sqld;
		if (mode.startsWith("i")) {
			sqld = MSMetaData::SQLD_INCLUDE;
		}
		else if (mode.startsWith("e")) {
			sqld = MSMetaData::SQLD_EXCLUDE;
		}
		else if (mode.startsWith("o")) {
			sqld = MSMetaData::SQLD_ONLY;
		}
		else {
			throw AipsError(
				"Unsupported sqldmode " + sqldmode
				+ ". Must be either i(nclude), e(xclude), or o(nly)."
			);
		}
		map<uInt COMMA set<uInt> > x = _msmd->getBBCNosToSpwMap(sqld);
		if (bb >= 0) {
			if (x.find(bb) == x.end()) {
				return new variant(vector<int>(0));
			}
			else {
				return new variant(_setUIntToVectorInt(x[bb]));
			}
		}
		else {
			record out;
			std::map<uInt COMMA std::set<uInt> >::const_iterator end = x.end();
			for (
				std::map<uInt COMMA std::set<uInt> >::const_iterator iter=x.begin();
				iter!=end; iter++
			) {
				vector<int> v = _setUIntToVectorInt(iter->second);
				out.insert(String::toString(iter->first), variant(v));
			}
			return new variant(out);
		}
	)
	return 0;
}

vector<int> msmetadata::spwsforintent(const string& intent, bool regex) {
	_FUNC(
		if (regex) {
			std::map<String COMMA std::set<uInt> > mymap = _msmd->getIntentToSpwsMap();
			std::set<Int> ids = _idsFromRegex(mymap, intent);
			return _setIntToVectorInt(ids);
		}
		else {
			vector<int> x = _setUIntToVectorInt(_msmd->getSpwsForIntent(intent));
			if (x.size() == 0) {
				*_log << LogIO::WARN << "Intent " << intent
					<< " does not exist in this dataset." << LogIO::POST;
			}
			return x;
		}
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
			/*
		*_log << LogIO::WARN << __FUNCTION__ << " is deprecated and will be removed. "
			<< "Use almaspws(tdm=True) instead." << LogIO::POST;
			*/
		return _setUIntToVectorInt(_msmd->getTDMSpw());
	)
	return vector<int>();
}

vector<int> msmetadata::wvrspws(bool complement) {
	_FUNC(
			/*
		*_log << LogIO::WARN << __FUNCTION__ << " is deprecated and will be removed. "
			<< "Use almaspws(tdm=True) instead." << LogIO::POST;
			*/
		vector<int> wvrs = _setUIntToVectorInt(_msmd->getWVRSpw());
		if (complement) {
			vector<int> nonwvrs(
				boost::counting_iterator<int>(0),
				boost::counting_iterator<int>(_msmd->nSpw(True)));
			vector<int>::iterator begin = nonwvrs.begin();
			foreach_r_(int spw, wvrs) {
				nonwvrs.erase(begin + spw);
			}
			return nonwvrs;
		}
		else {
			return wvrs;
		}
	)
	return vector<int>();
}

msmetadata::msmetadata(
	const MeasurementSet *const &ms, const float cachesize
) : _msmd(0), _ms(0), _log(new LogIO()) {
	_init(ms, cachesize);
}


vector<string> msmetadata::_fieldNames(const set<int>& ids) {
	if (*min_element(ids.begin(), ids.end()) < 0) {
		throw AipsError("All provided IDs must be greater than 0");
	}
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

void msmetadata::_checkSpwId(int id, bool throwIfNegative) const {
	ThrowIf(
		id >= (int)_msmd->nSpw(True) || (throwIfNegative && id < 0),
		"Spectral window ID " + String::toString(id)
		+ " out of range, must be less than "
		+ String::toString((int)_msmd->nSpw(True))
	);
}

void msmetadata::_checkPolId(int id, bool throwIfNegative) const {
	ThrowIf(
		id >= (int)_msmd->nPol() || (throwIfNegative && id < 0),
		"Spectral window ID " + String::toString(id)
		+ " out of range, must be less than "
		+ String::toString((int)_msmd->nPol())
	);
}

std::set<Int> msmetadata::_idsFromRegex(
	const std::map<String, std::set<Int> >& mymap, const String& regex
) const {
	std::set<Int> ids;
	boost::regex re;
	re.assign(regex);
	foreach_(std::pair<String COMMA std::set<Int> > kv, mymap) {
		if (boost::regex_match(kv.first, re)) {
			ids.insert(kv.second.begin(), kv.second.end());
		}
	}
	return ids;
}

std::set<Int> msmetadata::_idsFromRegex(
	const std::map<String, std::set<uInt> >& mymap, const String& regex
) const {
	std::set<Int> ids;
	boost::regex re;
	re.assign(regex);
	foreach_(std::pair<String COMMA std::set<uInt> > kv, mymap) {
		if (boost::regex_match(kv.first, re)) {
			ids.insert(kv.second.begin(), kv.second.end());
		}
	}
	return ids;
}

} // casac namespace

