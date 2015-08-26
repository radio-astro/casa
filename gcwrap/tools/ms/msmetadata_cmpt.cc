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
#include <casa/BasicSL/STLIO.h>
#include <casa/Logging/LogIO.h>
#include <casa/Quanta/QuantumHolder.h>
#include <casa/Quanta/QLogical.h>
#include <casa/Quanta/QVector.h>
#include <measures/Measures/MeasureHolder.h>
#include <measures/Measures/MDirection.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MSOper/MSMetaData.h>
#include <ms/MSOper/MSKeys.h>

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

vector<String> msmetadata::_vectorStdStringToVectorString(
	const vector<string>& inset
) {
	vector<String> outset;
	foreach_(string el, inset) {
		outset.push_back(el);
	}
	return outset;
}

record* msmetadata::antennadiameter(const variant& antenna) {
	_FUNC(
		variant::TYPE type = antenna.type();
		int antID;
		if (type == variant::INT) {
			antID = antenna.toInt();
			_checkAntennaId(antID, True);
		}
		else if (type == variant::STRING) {
			antID = _msmd->getAntennaID(antenna.toString());
		}
		else {
			ThrowCc(
				"Unsupported type for input parameter antenna. "
				"Supported types are int and string"
			);
		}
		Quantum<Vector<Double> > out = _msmd->getAntennaDiameters();
		QuantumHolder qh(casa::Quantity(out.getValue()[antID], out.getFullUnit()));
		Record rec;
		qh.toRecord(rec);
		return fromRecord(rec);
	)
	return NULL;
}

vector<int> msmetadata::antennaids(
	const variant& names, const variant& mindiameter,
	const variant& maxdiameter
) {
	_FUNC(
		vector<String> myNames;
		// because variants default to boolvecs even when we specify elsewise in the xml.
		casa::Quantity mind = mindiameter.type() == variant::BOOLVEC ?
			casa::Quantity(0, "m") : casaQuantity(mindiameter);
		casa::Quantity maxd = maxdiameter.type() == variant::BOOLVEC ?
			casa::Quantity(1, "pc") :casaQuantity(maxdiameter);
		ThrowIf(
			! mind.isConform("m"),
			"mindiameter must have units of length"
		);
		ThrowIf(
			! maxd.isConform("m"),
			"maxdiameter must have units of length"
		);
		vector<Int> foundIDs;
		variant::TYPE type = names.type();
		if (type == variant::BOOLVEC) {
			Vector<Int> x(_msmd->nAntennas());
			indgen(x, 0);
			foundIDs = x.tovector();
		}
		else if (type == variant::STRING) {
			myNames.push_back(names.toString());
		}
		else if (type == variant::STRINGVEC) {
			myNames = _vectorStdStringToVectorString(names.toStringVec());
		}
		else {
			ThrowCc(
				"Unsupported type for parameter names. Must "
				"be either a string or string array"
			);
		}
		vector<String> allNames COMMA foundNames;
		set<String> foundSet;
		if (foundIDs.empty()) {
			foreach_(String name, myNames) {
				Bool expand = name.find('*') != std::string::npos;
				if (expand) {
					if (allNames.empty()) {
						std::map<String COMMA uInt> namesToIDsMap;
						allNames = _msmd->getAntennaNames(namesToIDsMap);
					}
					vector<String> matches = _match(allNames, name);
					foreach_(String match, matches) {
						if (foundSet.find(match) == foundSet.end()) {
							foundNames.push_back(match);
							foundSet.insert(match);
						}
					}
				}
				else {
					if (foundSet.find(name) == foundSet.end()) {
						foundNames.push_back(name);
						foundSet.insert(name);
					}
				}
			}
			foundIDs = _vectorUIntToVectorInt(_msmd->getAntennaIDs(foundNames));
		}
		Quantum<Vector<Double> > diams = _msmd->getAntennaDiameters();
		casa::Quantity maxAntD(max(diams.getValue()), diams.getUnit());
		casa::Quantity minAntD(min(diams.getValue()), diams.getUnit());
		if (mind > minAntD || maxd < maxAntD) {
			vector<Int> newList;
			String unit = diams.getUnit();
			Vector<Double> v = diams.getValue();
			foreach_(uInt id, foundIDs) {
				casa::Quantity d(v[id], unit);
				if (d >= mind && d <= maxd) {
					newList.push_back(id);
				}
			}
			foundIDs = newList;
		}
		return foundIDs;
	)
	return vector<int>();
}

vector<string> msmetadata::antennanames(const variant& antennaids) {
	_FUNC(
		vector<uInt> myIDs;
		variant::TYPE type = antennaids.type();
		if (type == variant::BOOLVEC) {
			// do nothing, all names will be returned.
		}
		else if (type == variant::INT) {
			Int id = antennaids.toInt();
			ThrowIf(id < 0, "Antenna ID must be nonnegative");
			myIDs.push_back(id);
		}
		else if (type == variant::INTVEC) {
			vector<Int> tmp = antennaids.toIntVec();
			vector<Int>::const_iterator end = tmp.end();
			for (
				vector<Int>::const_iterator iter=tmp.begin();
				iter!=tmp.end(); iter++
			) {
				ThrowIf(*iter < 0, "Antenna ID must be nonnegative");
				myIDs.push_back(*iter);
			}
		}
		else {
			ThrowCc(
				"Unsupported type for parameter antennaids. "
				"Must be either an integer or integer array"
			);
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
			ThrowCc(
				"Unsupported type for input parameter which. "
				"Supported types are int and string"
			);
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

vector<string> msmetadata::antennastations(const variant& ants) {
	_FUNC(
		variant::TYPE type = ants.type();
		if (type == variant::INT) {
			vector<uInt> ids;
			int id = ants.toInt();
			if (id >= 0) {
				ids.push_back(id);
			}
			return _vectorStringToStdVectorString(
				_msmd->getAntennaStations(ids)
			);
		}
		else if (type == variant::INTVEC) {
			vector<Int> ids = ants.toIntVec();
			if (ids.empty() || (ids.size() == 1 && ids[0] < 0)) {
				return _vectorStringToStdVectorString(
					_msmd->getAntennaStations(vector<uInt>())
				);
			}
			else if (min(Vector<Int>(ids)) < 0) {
				throw AipsError("No antenna ID may be less than zero when multiple IDs specified.");
			}
			else {
				return _vectorStringToStdVectorString(
					_msmd->getAntennaStations(
						_vectorIntToVectorUInt(ants.toIntVec())
					)
				);
			}
		}
		else if (type == variant::STRING) {
			vector<String> names(1, String(ants.toString()));
			return _vectorStringToStdVectorString(
				_msmd->getAntennaStations(names)
			);
		}
		else if (type == variant::STRINGVEC) {
			return _vectorStringToStdVectorString(
				_msmd->getAntennaStations(
					_vectorStdStringToVectorString(
						ants.toStringVec()
					)
				)
			);
		}
		else if (type == variant::BOOLVEC) {
			return _vectorStringToStdVectorString(
				_msmd->getAntennaStations(vector<uInt>())
			);
		}
		else {
			throw AipsError(
				"Unsupported type (" + ants.typeString() + ") for ants."
			);
		}
	)
	return vector<string>();
}

vector<int> msmetadata::antennasforscan(int scan, int obsid, int arrayid) {
	_FUNC(
		auto scanKeys = _getScanKeys(scan, obsid, arrayid);
		std::set<int> myres;
		for (auto scanKey: scanKeys) {
			auto t = _msmd->getAntennasForScan(scanKey);
			myres.insert(t.begin(), t.end());
		}
		return vector<int>(myres.begin(), myres.end());
	)
	return vector<int>();
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
			for(auto id : spws.toIntVec()) {
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
	_FUNC (
		_checkSpwId(spw, True);
		return _msmd->getChanFreqs()[spw].getValue(Unit(unit)).tovector();
	)
	return vector<double>();
}

vector<double> msmetadata::chanwidths(int spw, const string& unit) {
	_FUNC (
		_checkSpwId(spw, True);
		return _msmd->getChanWidths()[spw].getValue(Unit(unit), True).tovector();
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

variant* msmetadata::corrprodsforpol(int polid) {
	_FUNC(
		_checkPolId(polid, True);
		Array<Int> prods = _msmd->getCorrProducts()[polid];
		return new variant(
			vector<int>(prods.begin(), prods.end()),
			prods.shape().asStdVector()
		);
	)
	return NULL;
}

vector<int> msmetadata::corrtypesforpol(int polid) {
	_FUNC(
		_checkPolId(polid, True);
		return _msmd->getCorrTypes()[polid];
	)
	return vector<int>();
}

vector<int> msmetadata::datadescids(int spw, int pol) {
	_FUNC(
		_checkSpwId(spw, False);
		_checkPolId(pol, False);
		auto mymap = _msmd->getSpwIDPolIDToDataDescIDMap();
		vector<int> ddids;
		for(auto iter : mymap) {
			uInt myspw = iter.first.first;
			uInt mypol = iter.first.second;
			uInt ddid = iter.second;
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
	_FUNC(
		return fromRecord(
			QuantumHolder(_msmd->getEffectiveTotalExposureTime()).toRecord()
		);
	)
	return 0;
}

record* msmetadata::exposuretime(
	int scan, int spwid, int polid, int obsid, int arrayid
) {
	_FUNC(
		_checkSpwId(spwid, True);
		_checkObsId(obsid, True);
		if (polid >= 0) {
			_checkPolId(polid, True);
		}
		else {
			std::set<uInt> polids = _msmd->getPolarizationIDs(
				(uInt)obsid, arrayid, scan, (uInt)spwid
			);
			ThrowIf(
				polids.empty(),
				"This dataset has no records for the specified scan and spwid"
			);
			if (polids.size() == 1) {
				polid = *(polids.begin());
			}
			else {
				*_log << LogIO::WARN	<< "This scan and spwID has multiple "
					<< " polarization IDs which are " << polids
					<< ". You must specify one of those."
					<< LogIO::POST;
				return 0;
			}
		}
		std::map<std::pair<uInt COMMA uInt> COMMA uInt> ddidMap = _msmd->getSpwIDPolIDToDataDescIDMap();
		std::pair<uInt COMMA uInt> mykey;
		mykey.first = spwid;
		mykey.second = polid;
		ThrowIf(
			ddidMap.find(mykey) == ddidMap.end(),
			"MS has no data description ID for spectral window ID "
			+ String::toString(spwid) + " and polarization ID "
			+ String::toString(polid)
		);
		uInt dataDescID = ddidMap[mykey];
		std::map<Int COMMA casa::Quantity> map2 = _msmd->getFirstExposureTimeMap()[dataDescID];
		ThrowIf(
			map2.find(scan) == map2.end(),
			"MS has no records for scan number " + String::toString(scan)
			+ ", spectral window ID " + String::toString(spwid) + ", and polarization ID "
			+ String::toString(polid)
		);

		return fromRecord(QuantumHolder(map2[scan]).toRecord());
	)
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


vector<string> msmetadata::fieldnames() {
	_FUNC(
		return _vectorStringToStdVectorString(_msmd->getFieldNames());
	)
	return vector<string>();
}

variant* msmetadata::fieldsforintent(
    const string& intent, const bool asnames
) {
    _FUNC(
        std::set<Int> ids; 
        auto expand = intent.find('*') != std::string::npos;
        if (intent == "*" && _msmd->getIntents().empty()) {
            auto nFields = _msmd->nFields();
            for (uInt i=0; i<nFields; ++i) {
                ids.insert(i);
            }
        }
        else if (expand) {
            auto mymap = _msmd->getIntentToFieldsMap();
            ids = _idsFromExpansion(mymap, intent);
        }
        else {
            ids = _msmd->getFieldsForIntent(intent);
        }
        variant *x;
        if (ids.size() == 0) { 
            *_log << LogIO::WARN << "No intent " << (expand ? "matching '" : "'") 
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
    )
    return nullptr;
}

vector<int> msmetadata::fieldsforname(const string& name) {
	_FUNC(
		if (name.empty()) {
			return _setIntToVectorInt(_msmd->getUniqueFiedIDs());
		}
		return _setIntToVectorInt(_msmd->getFieldIDsForField(name));
	)
	return vector<int>();
}

variant* msmetadata::fieldsforscan(int scan, bool asnames, int obsid, int arrayid) {
	_FUNC(
		ThrowIf(
			scan < 0,
			"Scan number must be nonnegative."
		);
		auto scanKeys = _getScanKeys(scan, obsid, arrayid);
		std::set<int> ids;
		for (auto scanKey: scanKeys) {
			auto t = _msmd->getFieldsForScan(scanKey);
			ids.insert(t.begin(), t.end());
		}
		if (asnames) {
			return new variant(_fieldNames(ids));
		}
		else {
			return new variant(
				_setIntToVectorInt(ids)
			);
		}
	)
	return nullptr;
}

variant* msmetadata::fieldsforscans(
	const vector<int>& scans, const bool asnames,
	int obsid, int arrayid
) {
	_FUNC(
		ThrowIf(
			scans.empty(), "Scans array cannot be empty"
		);
		ThrowIf(
			*std::min_element (scans.begin(), scans.end()) < 0,
			"All scan numbers must be non-negative"
		);
		auto scanKeys = _getScanKeys(scans, obsid, arrayid);
		std::set<int> ids;
		for (auto scanKey: scanKeys) {
			auto t = _msmd->getFieldsForScan(scanKey);
			ids.insert(t.begin(), t.end());
		}
		if (asnames) {
			return new variant(_fieldNames(ids));
		}
		else {
			return new variant(
				_setIntToVectorInt(ids)
			);
		}
	)
	return nullptr;
}

variant* msmetadata::fieldsforsource(const int sourceID, const bool asnames) {
	_FUNC(
		if (asnames) {
			std::map<Int COMMA std::set<String> > res = _msmd->getFieldNamesForSourceMap();
			if (res.find(sourceID) == res.end()) {
				return new variant(vector<string>());
			}
			else {
				return new variant(
					_setStringToVectorString(res[sourceID])
				);
			}
		}
		else {
			std::map<Int COMMA std::set<Int> > res = _msmd->getFieldsForSourceMap();
			if (res.find(sourceID) == res.end()) {
				return new variant(vector<int>());
			}
			else {
				return new variant(
					_setIntToVectorInt(res[sourceID])
				);
			}
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

vector<string> msmetadata::intentsforscan(int scan, int obsid, int arrayid) {
	_FUNC(
		if (scan < 0) {
			throw AipsError("Scan number must be nonnegative.");
		}
		auto scanKeys = _getScanKeys(scan, obsid, arrayid);
		std::set<String> intents;
		for (auto scanKey: scanKeys) {
			auto t = _msmd->getIntentsForScan(scanKey);
			intents.insert(t.begin(), t.end());
		}
		return _setStringToVectorString(intents);
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
	_FUNC (
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

vector<string> msmetadata::namesforspws(const variant& spwids) {
	_FUNC(
		variant::TYPE myType = spwids.type();
		vector<uInt> spwIDs;
		if (myType == variant::INT) {
			Int id = spwids.toInt();
			ThrowIf(id < 0, "Spectral window ID must be nonnegative.");
			ThrowIf(
				id >= (Int)_msmd->nSpw(True),
				"Spectral window ID must be less than total number of spws"
			);
			spwIDs.push_back(id);
		}
		else if (myType == variant::INTVEC) {
			vector<Int> kk = spwids.toIntVec();
			Vector<Int> xx(kk);
			ThrowIf(
				min(xx) < 0,
				"All spectral window IDs must be nonnegative."
			);
			ThrowIf(
				max(xx) >= (Int)_msmd->nSpw(True),
				"All spectral window IDs must be less than "
				"the total number of spws"
			);
			spwIDs = _vectorIntToVectorUInt(kk);
		}
		else if (
			(myType == variant::STRING && spwids.toString().empty())
			|| myType == variant::BOOLVEC
		) {
			return _vectorStringToStdVectorString(
				_msmd->getSpwNames()
			);
		}
		else if (spwids.size() != 0) {
			ThrowCc(
				"Unsupported type for spwids. It must be a "
				"nonnegative integer or nonnegative integer array"
			);
		}
		vector<String> allNames = _msmd->getSpwNames();
		vector<String> names;
		for(auto i : spwIDs) {
			names.push_back(allNames[i]);
		}
		return _vectorStringToStdVectorString(names);
	)
	return vector<string>();
}

string msmetadata::name() {
	_FUNC(
		return _ms->tableName();
	)
	return "";
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
	_FUNC (
		_checkSpwId(spw, True);
		return _msmd->nChans()[spw];
	)
	return 0;
}

variant* msmetadata::ncorrforpol(int polid) {
	_FUNC(
		_checkPolId(polid, False);
		vector<Int> ncorr = _msmd->getNumCorrs();
		if (polid < 0) {
			return new variant(ncorr);
		}
		return new variant(ncorr[polid]);
	)
	return NULL;
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

int msmetadata::nsources() {
	_FUNC(
		return _msmd->nUniqueSourceIDsFromSourceTable();
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

vector<string> msmetadata::observers() {
	_FUNC(
		return _vectorStringToStdVectorString(_msmd->getObservers());
	)
	return vector<string>();
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

::casac::record* msmetadata::phasecenter(const int fieldid, const ::casac::record& epoch){
	::casac::record *rval=0;
    _FUNC(   
    	PtrHolder<Record> ep(toRecord(epoch));
	  	Record outRec;
	  	MeasureHolder mh;
	  	String err;
	  	if (ep->nfields() == 0) {
	  		mh = MeasureHolder (_msmd->phaseDirFromFieldIDAndTime(fieldid));
	  	}
	  	else {
	  		MeasureHolder ephold;
	  		ThrowIf(! ephold.fromRecord(err, *ep), "Epoch cannot be converted \n" + err);
	  		ThrowIf(! ephold.isMEpoch(), "Epoch parameter is not an MEpoch  \n");
	  		mh = MeasureHolder (_msmd->phaseDirFromFieldIDAndTime(fieldid, ephold.asMEpoch()));
	  	}
	  	ThrowIf(! mh.toRecord(err, outRec), "Could not convert phasecenter \n" + err);
	  	rval = fromRecord(outRec);
    )
    return rval;
}

record* msmetadata::pointingdirection(int rowid, bool const interpolate, int const initialrow) {
	_FUNC(
		Int ant1 COMMA ant2;
		Double time;
		
		std::pair<casa::MDirection COMMA casa::MDirection> pDirs = _msmd->getPointingDirection(
			ant1, ant2, time, rowid, interpolate, initialrow
		);
		MeasureHolder m1(pDirs.first);
		MeasureHolder m2(pDirs.second);
		Record ret;
		ret.define("time", time);
		Record ant1Rec COMMA ant2Rec COMMA m1rec COMMA m2rec;
		ant1Rec.define("id", ant1);
		m1.toRecord(m1rec);
		ant1Rec.defineRecord("pointingdirection", m1rec);
		ret.defineRecord("antenna1", ant1Rec);
		ant2Rec.define("id", ant2);
		m2.toRecord(m2rec);
		ant2Rec.defineRecord("pointingdirection", m2rec);
		ret.defineRecord("antenna2", ant2Rec);
		return fromRecord(ret);
	);
	return NULL;
}

void msmetadata::_init(const casa::MeasurementSet *const &ms, const float cachesize) {
    _msmd.reset(new MSMetaData(ms, cachesize));
}

bool msmetadata::open(const string& msname, const float cachesize) {
	_FUNC2(
		_ms.reset(new MeasurementSet(msname));
		_init(_ms.get(), cachesize);
		return true;
	)
	return false;
}

variant* msmetadata::polidfordatadesc(int ddid) {
	_FUNC(
		vector<uInt> pols = _msmd->getDataDescIDToPolIDMap();
		if (ddid < 0) {
			return new variant(pols);
		}
		int nddids = pols.size();
		ThrowIf(ddid >= nddids, "ddid must be less than " + String::toString(nddids));
		return new variant(pols[ddid]);
	)
	return NULL;
}

vector<string> msmetadata::projects() {
	_FUNC(
		return _vectorStringToStdVectorString(_msmd->getProjects());
	)
	return vector<string>();
}

record* msmetadata::propermotions() {
	_FUNC(
		vector<std::pair<casa::Quantity COMMA casa::Quantity> > mu = _msmd->getProperMotions();
		Record rec;
		Record subrec;
		uInt n = mu.size();
		Vector<Record> v(2);
		for (uInt i=0; i<n; ++i) {
			QuantumHolder q0(mu[i].first);
			QuantumHolder q1(mu[i].second);
			q0.toRecord(v[0]);
			q1.toRecord(v[1]);
			subrec.defineRecord("longitude", v[0]);
			subrec.defineRecord("latitude", v[1]);
			rec.defineRecord(casa::String::toString(i), subrec);
		}
		return fromRecord(rec);
	)
	return NULL;
}

record* msmetadata::refdir(
    const variant& field, const record& epoch
) {
    _FUNC(
        Int id;
        switch (field.type()) {
        case variant::STRING:
            id = *(_msmd->getFieldIDsForField(field.toString()).begin());
            break;
        case variant::INT:
            id = field.toInt();
            break;
        default:
            ThrowCc(
                "Unsupported type for field which must be "
                "a nonnegative int or string."
            );
        }
        unique_ptr<Record> ep(toRecord(epoch));
        Record outRec;
        MeasureHolder mh;
        String err; 
        if (ep->nfields() == 0) { 
            mh = MeasureHolder(_msmd->getReferenceDirection(id));
        }
        else {
            MeasureHolder ephold;
            ThrowIf(
                ! ephold.fromRecord(err, *ep),
                "Epoch cannot be converted \n" + err
            );
            ThrowIf(
                ! ephold.isMEpoch(), "Epoch parameter is not an MEpoch  \n"
            );
            mh = MeasureHolder(_msmd->getReferenceDirection(id, ephold.asMEpoch()));
        }
        ThrowIf(
            ! mh.toRecord(err, outRec),
            "Could not convert reference direction to Record \n" + err
        );
        return fromRecord(outRec);
    )    
    return nullptr;
}

record* msmetadata::reffreq(int spw) {
	_FUNC(
		_checkSpwId(spw, True);
		MeasureHolder freq(_msmd->getRefFreqs()[spw]);
		Record ret;
		freq.toRecord(ret);
		return fromRecord(ret);
	)
	return NULL;
}

vector<int> msmetadata::scannumbers(int obsid, int arrayid) {
	_FUNC(
		_checkObsId(obsid, False);
		_checkArrayId(arrayid, False);
		return _setIntToVectorInt(_msmd->getScanNumbers(obsid, arrayid));
	)
	return vector<int>();
}

vector<int> msmetadata::scansforfield(
	const variant& field, int obsid, int arrayid
) {
	_FUNC(
		_checkObsId(obsid, False);
		_checkArrayId(arrayid, False);
		switch (field.type()) {
		case variant::INT:
			return _setIntToVectorInt(_msmd->getScansForFieldID(field.toInt(), obsid, arrayid));
			break;
		case variant::STRING:
			return _setIntToVectorInt(_msmd->getScansForField(field.toString(), obsid, arrayid));
			break;
		default:
			throw AipsError("Unsupported type for field parameter.");
		}
	)
	return vector<int>();
}

vector<int> msmetadata::scansforintent(const string& intent, int obsid, int arrayid) {
	_FUNC(
		_checkObsId(obsid, False);
		_checkArrayId(arrayid, False);
		Bool expand = intent.find('*') != std::string::npos;
		if (expand) {
			auto mymap = _msmd->getIntentToScansMap();
			auto scanKeys = _idsFromExpansion(mymap, intent);
			auto doAllObs = obsid < 0;
			auto doAllArrays = arrayid < 0;
			std::set<Int> myScanNumbers;
			for (const auto k : scanKeys) {
				if (
					(doAllObs || k.obsID == obsid)
					&& (doAllArrays || k.arrayID == arrayid)
				) {
					myScanNumbers.insert(k.scan);
				}
			}
			return _setIntToVectorInt(myScanNumbers);
		}
		else {
			return _setIntToVectorInt(_msmd->getScansForIntent(intent, obsid, arrayid));
		}
	)
	return vector<int>();
}

vector<int> msmetadata::scansforspw(const int spw, int obsid, int arrayid) {
	_FUNC(
		_checkSpwId(spw, True);
		_checkObsId(obsid, False);
		_checkArrayId(arrayid, False);
		return _setIntToVectorInt(_msmd->getScansForSpw(spw, obsid, arrayid));
	)
	return vector<int>();
}

vector<int> msmetadata::scansforstate(int state, int obsid, int arrayid) {
	_FUNC(
		ThrowIf(
			state < 0, "State must be nonnegative."
		);
		_checkObsId(obsid, False);
		_checkArrayId(arrayid, False);
		return _setIntToVectorInt(_msmd->getScansForState(state, obsid, arrayid));
	)
	return vector<int>();
}

vector<int> msmetadata::scansfortimes(
	double center, double tol, int obsid, int arrayid
) {
	_FUNC(
		_checkObsId(obsid, False);
		_checkArrayId(arrayid, False);
		return _setIntToVectorInt(
			_msmd->getScansForTimes(center, tol, obsid, arrayid)
		);
	)
	return vector<int>();
}

vector<string> msmetadata::schedule(int obsid) {
	_FUNC(
		_checkObsId(obsid, True);
		return _vectorStringToStdVectorString(_msmd->getSchedules()[obsid]);
	)
	return vector<string>();
}

int msmetadata::sideband(int spw) {
	_FUNC (
		_checkSpwId(spw, True);
        Int ret = _msmd->getNetSidebands()[spw] == 2 ? 1 : -1;
        return ret;
	)
	return 0;
}

record* msmetadata::sourcedirs() {
	_FUNC(
		std::vector<casacore::MDirection> mdirs = _msmd->getSourceDirections();
		uInt i = 0;
		vector<casacore::MDirection>::const_iterator iter = mdirs.begin();
		vector<casacore::MDirection>::const_iterator end = mdirs.end();
		Record r;
		Record mr;
		while (iter != end) {
			MeasureHolder mh(*iter);
			mh.toRecord(mr);
			r.defineRecord(casa::String::toString(i), mr);
			++iter;
			++i;
		}
		return fromRecord(r);
	)
	return nullptr;
}

int msmetadata::sourceidforfield(int field) {
	_FUNC(
		_checkFieldId(field, True);
		return _msmd->getFieldTableSourceIDs()[field];
	)
	return 0;
}

vector<int> msmetadata::sourceidsfromsourcetable() {
	_FUNC(
		return _msmd->getSourceTableSourceIDs();
	)
	return vector<int>();
}

vector<string> msmetadata::sourcenames() {
	_FUNC(
		return _vectorStringToStdVectorString(_msmd->getSourceNames());
	)
	return vector<string>();
}

variant* msmetadata::spwfordatadesc(int ddid) {
	_FUNC(
		vector<uInt> spws = _msmd->getDataDescIDToSpwMap();
		if (ddid < 0) {
			return new variant(spws);
		}
		int nddids = spws.size();
		ThrowIf(ddid >= nddids, "ddid must be less than " + String::toString(nddids));
		return new variant(spws[ddid]);
	)
	return NULL;
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

vector<int> msmetadata::spwsforintent(const string& intent) {
	_FUNC(
		Bool expand = intent.find('*') != std::string::npos;
		if (expand) {
			std::map<String COMMA std::set<uInt> > mymap = _msmd->getIntentToSpwsMap();
			std::set<Int> ids = _idsFromExpansion(mymap, intent);
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

vector<int> msmetadata::spwsforscan(int scan, int obsid, int arrayid) {
	_FUNC(
		if (scan < 0) {
			throw AipsError("Scan must be nonnegative");
		}
		auto scanKeys = _getScanKeys(scan, obsid, arrayid);
		std::set<uInt> spws;
		for (const auto scanKey : scanKeys) {
			auto t = _msmd->getSpwsForScan(scanKey);
			spws.insert(t.begin(), t.end());
		}
		return _setUIntToVectorInt(spws);
	)
	return vector<int>();
}

vector<int> msmetadata::statesforscan(int scan, int obsid, int arrayid) {
	_FUNC(
		ThrowIf(scan < 0, "Scan number must be nonnegative");
		_checkObsId(obsid, False);
		_checkArrayId(arrayid, False);
		return _setIntToVectorInt(_msmd->getStatesForScan(obsid, arrayid, scan));
	)
	return vector<int>();
}

record* msmetadata::timerangeforobs(int obsid) {
	_FUNC(
		_checkObsId(obsid, True);
		auto range = _msmd->getTimeRangesOfObservations()[obsid];
		MeasureHolder begin(range.first);
		MeasureHolder end(range.second);
		Record ret COMMA beginRec COMMA endRec;
		begin.toRecord(beginRec);
		end.toRecord(endRec);
		ret.defineRecord("begin", beginRec);
		ret.defineRecord("end", endRec);
		return fromRecord(ret);
	)
	return NULL;
}

vector<double> msmetadata::timesforfield(int field) {
	_FUNC(
		if (field < 0) {
			throw AipsError("Field ID must be nonnegative");
		}
		return _setDoubleToVectorDouble(_msmd->getTimesForField(field));
	)
	return vector<double>();
}

record* msmetadata::summary() {
	_FUNC(
		return fromRecord(_msmd->getSummary());
	)
	return NULL;
}

vector<double> msmetadata::timesforintent(const string& intent) {
	_FUNC(
		return _setDoubleToVectorDouble(_msmd->getTimesForIntent(intent));
	)
	return vector<double>();
}

variant* msmetadata::timesforscan(int scan, int obsid, int arrayid, bool perspw) {
	_FUNC(
		if (scan < 0) {
			throw AipsError("Scan number must be nonnegative");
		}
		auto scanKeys = _getScanKeys(scan, obsid, arrayid);
		if (perspw) {
			std::map<uInt COMMA std::set<Double> > spwToTimes;
			Record ret;
			for (const auto scanKey : scanKeys) {
				auto mymap = _msmd->getSpwToTimesForScan(scanKey);
				for (const auto& kv : mymap) {
					auto spw = kv.first;
					auto times = kv.second;
					if (spwToTimes.find(spw) == spwToTimes.end()) {
						spwToTimes[spw] = times;
					}
					else {
						spwToTimes[spw].insert(times.begin(), times.end());
					}
				}
			}
			for (const auto& kv : spwToTimes) {
				ret.define(
					casa::String::toString(kv.first) COMMA
					Vector<Double>(_setDoubleToVectorDouble(kv.second)
					)
				);
			}
			SHARED_PTR<record> rec(fromRecord(ret));
			return new variant(*rec);
		}
		else {
			std::set<Double> times;
			for (const auto& scanKey : scanKeys) {
				auto t = _msmd->getTimesForScan(scanKey);
				times.insert(t.begin(), t.end());
			}
			return new variant(_setDoubleToVectorDouble(times));
		}
	)
	return nullptr;
}

vector<double> msmetadata::timesforscans(const vector<int>& scans, int obsid, int arrayid) {
	_FUNC(
		ThrowIf(
			*std::min_element(scans.begin(), scans.end()) < 0,
			"All scan numbers must be nonnegative"
		);
		auto scanKeys = _getScanKeys(scans, obsid, arrayid);
		return _setDoubleToVectorDouble(_msmd->getTimesForScans(scanKeys));
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
) : _msmd(), _ms(), _log(new LogIO()) {
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

void msmetadata::_checkAntennaId(int id, bool throwIfNegative) const {
	ThrowIf(
		id >= (int)_msmd->nAntennas() || (throwIfNegative && id < 0),
		"Antenna ID " + String::toString(id)
		+ " out of range, must be less than "
		+ String::toString((int)_msmd->nAntennas())
	);
}

void msmetadata::_checkArrayId(int id, bool throwIfNegative) const {
	ThrowIf(
		id >= (int)_msmd->nArrays() || (throwIfNegative && id < 0),
		"Array ID " + String::toString(id)
		+ " out of range, must be less than "
		+ String::toString((int)_msmd->nArrays())
	);
}

void msmetadata::_checkFieldId(int id, bool throwIfNegative) const {
	ThrowIf(
		id >= (int)_msmd->nFields() || (throwIfNegative && id < 0),
		"Antenna ID " + String::toString(id)
		+ " out of range, must be less than "
		+ String::toString((int)_msmd->nFields())
	);
}

void msmetadata::_checkObsId(int id, bool throwIfNegative) const {
	ThrowIf(
		id >= (int)_msmd->nObservations() || (throwIfNegative && id < 0),
		"Observation ID " + String::toString(id)
		+ " out of range, must be less than "
		+ String::toString((int)_msmd->nObservations())
	);
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
		id >= (int)_msmd->nPol(),
		"Polarization ID " + String::toString(id)
		+ " out of range, must be less than "
		+ String::toString((int)_msmd->nPol())
	);
	ThrowIf(throwIfNegative && id < 0, "Polarization ID cannot be negative");
}

std::set<ScanKey> msmetadata::_getScanKeys(int scan, int obsid, int arrayid) const {
	_checkObsId(obsid, False);
	_checkArrayId(arrayid, False);
	if (obsid >= 0 && arrayid >= 0) {
		std::set<ScanKey> scanKey;
		ScanKey x;
		x.scan = scan;
		x.obsID = obsid;
		x.arrayID = arrayid;
		scanKey.insert(x);
		return scanKey;
	}
	else {
		ArrayKey ak;
		ak.obsID = obsid;
		ak.arrayID = arrayid;
		auto scanKeys = _msmd->getScanKeys(ak);
		std::set<ScanKey> myKeys;
		for (const auto k : scanKeys) {
			if (k.scan == scan) {
				myKeys.insert(k);
			}
		}
		ThrowIf(myKeys.empty(), "No matching scans found");
		return myKeys;
	}
}

std::set<ScanKey> msmetadata::_getScanKeys(
	const vector<int>& scans, int obsid, int arrayid
) const {
	_checkObsId(obsid, False);
	_checkArrayId(arrayid, False);
	if (obsid >= 0 && arrayid >= 0) {
		std::set<ScanKey> scanKeys;
		ScanKey x;
		x.obsID = obsid;
		x.arrayID = arrayid;
		for (auto scan : scans) {
			x.scan = scan;
			scanKeys.insert(x);
		}
		return scanKeys;
	}
	else {
		ArrayKey ak;
		ak.obsID = obsid;
		ak.arrayID = arrayid;
		auto scanKeys = _msmd->getScanKeys(ak);
		std::set<ScanKey> myKeys;
		for (const auto k : scanKeys) {
			if (
				std::find(scans.begin(), scans.end(), k.scan) != scans.end()
			) {
				myKeys.insert(k);
			}
		}
		ThrowIf(myKeys.empty(), "No matching scans found");
		return myKeys;
	}
}

template <class T>
std::set<T> msmetadata::_idsFromExpansion(
	const std::map<String, std::set<T> >& mymap, const String& matchString
) {
	std::set<T> ids;
	boost::regex re;
	re.assign(_escapeExpansion(matchString));
	for(auto kv : mymap) {
		if (boost::regex_match(kv.first, re)) {
			ids.insert(kv.second.begin(), kv.second.end());
		}
	}
	return ids;
}

std::set<Int> msmetadata::_idsFromExpansion(
	const std::map<String, std::set<uInt> >& mymap, const String& matchString
) {
	std::set<Int> ids;
	boost::regex re;
	re.assign(_escapeExpansion(matchString));
	for(auto kv : mymap) {
		if (boost::regex_match(kv.first, re)) {
			ids.insert(kv.second.begin(), kv.second.end());
		}
	}
	return ids;
}

std::vector<casa::String> msmetadata::_match(
	const vector<casa::String>& candidates, const casa::String& matchString
) {
	vector<casa::String> matches;
	boost::regex re;
	re.assign(_escapeExpansion(matchString));
	for(auto candidate : candidates) {
		if (boost::regex_match(candidate, re)) {
			matches.push_back(candidate);
		}
	}
	return matches;
}

std::string msmetadata::_escapeExpansion(const casa::String& stringToEscape) {
	const boost::regex esc("[\\^\\.\\$\\|\\(\\)\\[\\]\\+\\?\\/\\\\]");
	const std::string rep("\\\\\\1");
	std::string result = regex_replace(
		stringToEscape, esc, rep, boost::match_default | boost::format_sed
	);
	const boost::regex expand("\\*");
	const std::string rep1(".*");
	return regex_replace(
		result, expand, rep1, boost::match_default | boost::format_sed
	);
}

} // casac namespace
