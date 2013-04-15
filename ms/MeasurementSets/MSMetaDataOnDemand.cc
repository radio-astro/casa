//# MSMetaData.cc
//# Copyright (C) 1998,1999,2000,2001
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

#include <ms/MeasurementSets/MSMetaDataOnDemand.h>

#include <casa/OS/File.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/TableParse.h>
#include <tables/Tables/TableProxy.h>

#include <iomanip>

// DEBUG ONLY
/*
#include <casa/Arrays/ArrayIO.h>
#include <casa/OS/PrecTimer.h>
*/

#define _ORIGIN "MSMetaDataOnDemand::" + String(__FUNCTION__) + ": "

namespace casa {

MSMetaDataOnDemand::MSMetaDataOnDemand(const MeasurementSet *const &ms, const Float maxCacheSizeMB)
	: _ms(ms), _cacheMB(0), _maxCacheMB(maxCacheSizeMB), _nStates(0),
	  _nACRows(0), _nXCRows(0), _nSpw(0), _nFields(0),
	  _nAntennas(0), _nObservations(0), _nScans(0), _nArrays(0),
	  _nrows(0), _uniqueIntents(), _scanToSpwsMap(),
	  _uniqueScanNumbers(),_uniqueFieldIDs(), _uniqueStateIDs(),
	  _avgSpw(), _tdmSpw(),
	  _fdmSpw(), _wvrSpw(),_antenna1(), _antenna2(),
	  _scans(), _fieldIDs(), _stateIDs(), _dataDescIDs(),
	  _observationIDs(),
	  _scanToNACRowsMap(), _scanToNXCRowsMap(),
	  _fieldToNACRowsMap(), _fieldToNXCRowsMap(),
	  _dataDescIDToSpwMap(),
	  _scanToIntentsMap(), _stateToIntentsMap(),
	  _spwToIntentsMap(),
	  _spwInfo(0), _fieldToSpwMap(),
	  _spwToFieldIDsMap(0), _spwToScansMap(0),
	  _scanToStatesMap(), _scanToFieldsMap(),
	  _fieldToScansMap(),
	  _fieldNames(0),
	  _antennaNames(0), _observatoryNames(0),
	  _antennaNameToIDMap(), _times(),
	  _scanToTimesMap(), _intentToFieldIDMap(),
	  _fieldToTimesMap(), _observatoryPositions(0),
	  _antennaOffsets(0), _uniqueBaselines(0, 0),
	  _exposureTime(0), _nUnflaggedACRows(0),
	  _nUnflaggedXCRows(0), _unflaggedFieldNACRows(),
	  _unflaggedFieldNXCRows(), _unflaggedScanNACRows(),
	  _unflaggedScanNXCRows(),
	  _taqlTableName(
		File(ms->tableName()).exists() ? ms->tableName() : "$1"
	  ),
	  _taqlTempTable(
		File(ms->tableName()).exists() ? 0 : 1, ms
	  ), _flagsColumn(), _scanToTimeRangeMap(),
	  _scanSpwToIntervalMap() {}

MSMetaDataOnDemand::~MSMetaDataOnDemand() {}

uInt MSMetaDataOnDemand::nStates() {
	if (_nStates == 0) {
		_nStates = _getNStates(*_ms);
	}
	return _nStates;
}

std::set<String> MSMetaDataOnDemand::getIntents() {
	if (! _uniqueIntents.empty()) {
		return _uniqueIntents;
	}
	vector<std::set<String> > statesToIntentsMap;
	std::set<String> uniqueIntents;
	_getStateToIntentsMap(
		statesToIntentsMap,
		uniqueIntents
	);
	return uniqueIntents;
}

void MSMetaDataOnDemand::_getStateToIntentsMap(
	vector<std::set<String> >& stateToIntentsMap,
	std::set<String>& uniqueIntents
) {
	if (! _uniqueIntents.empty() && ! _stateToIntentsMap.empty()) {
		uniqueIntents = _uniqueIntents;
		stateToIntentsMap = _stateToIntentsMap;
		return;
	}
	MSMetaData::_getStateToIntentsMap(
		stateToIntentsMap,
		uniqueIntents, *_ms
	);

	std::set<String>::const_iterator lastIntent = uniqueIntents.end();
	uInt mysize = 0;
	for (
		std::set<String>::const_iterator intent=uniqueIntents.begin();
		intent!=lastIntent; intent++
	) {
		mysize += intent->size();
	}
	vector<std::set<String> >::const_iterator lastState = stateToIntentsMap.end();
	for (
		vector<std::set<String> >::const_iterator iter=stateToIntentsMap.begin();
		iter!=lastState; iter++
	) {
		std::set<String>::const_iterator lastIntent=iter->end();
		for (
			std::set<String>::const_iterator intent=iter->begin();
			intent!=lastIntent; intent++
		) {
			mysize += intent->size();
		}
	}
	if (_cacheUpdated(mysize)) {
		_uniqueIntents = uniqueIntents;
		_stateToIntentsMap = stateToIntentsMap;
	}
}

std::set<Int> MSMetaDataOnDemand::getScanNumbers() {
	if (_uniqueScanNumbers.size() > 0) {
		return _uniqueScanNumbers;
	}
	String taql = "select unique(SCAN_NUMBER) from " + _taqlTableName;
	Table result(tableCommand(taql, _taqlTempTable));
	ROScalarColumn<Int> scanCol(result, "SCAN_NUMBER");
	Vector<Int> scans = scanCol.getColumn();
	std::set<Int> myUniqueScans(scans.begin(), scans.end());
	Float mysize = _cacheMB + sizeof(Int)*scans.size()/1e6;
	if (mysize < _maxCacheMB) {
		_cacheMB = mysize;
		_uniqueScanNumbers = myUniqueScans;
	}
	return myUniqueScans;
}

uInt MSMetaDataOnDemand::nScans() {
	if (_nScans == 0) {
		_nScans = getScanNumbers().size();
	}
	return _nScans;
}

uInt MSMetaDataOnDemand::nObservations() {
	if (_nObservations == 0) {
		_nObservations = _ms->observation().nrow();
	}
	return _nObservations;
}

uInt MSMetaDataOnDemand::nArrays() {
	if (_nArrays == 0) {
		// because the ARRAY table apparently is optional
		_nArrays = max(*_getArrayIDs()) + 1;
	}
	return _nArrays;
}

uInt MSMetaDataOnDemand::nRows() {
	return _ms->nrow();
	/*
    if (_nrows == 0) {
        _nrows = _ms->nrow();
    }
	return _nrows;
	*/
}
uInt MSMetaDataOnDemand::nRows(CorrelationType cType) {

	if (cType == BOTH) {
		return nRows();
	}
	uInt nACRows, nXCRows;
	std::tr1::shared_ptr<AOSFMapI> scanToNACRowsMap, scanToNXCRowsMap;
	std::tr1::shared_ptr<std::map<Int, uInt> > fieldToNACRowsMap, fieldToNXCRowsMap;
	_getRowStats(
		nACRows, nXCRows, scanToNACRowsMap,
		scanToNXCRowsMap, fieldToNACRowsMap,
		fieldToNXCRowsMap
	);
	if (cType == AUTO) {
		return nACRows;
	}
	else {
		return nXCRows;
	}
}

uInt MSMetaDataOnDemand::nRows(
	CorrelationType cType, Int arrayID, Int observationID,
	Int scanNumber, Int fieldID
) {
	uInt nACRows, nXCRows;
	std::tr1::shared_ptr<AOSFMapI> scanToNACRowsMap, scanToNXCRowsMap;
	std::tr1::shared_ptr<std::map<Int, uInt> > fieldToNACRowsMap, fieldToNXCRowsMap;
    _getRowStats(
		nACRows, nXCRows, scanToNACRowsMap,
		scanToNXCRowsMap, fieldToNACRowsMap,
		fieldToNXCRowsMap
	);
	if (cType == AUTO) {
		return (*scanToNACRowsMap)[arrayID][observationID][scanNumber][fieldID];
	}
	else if (cType == CROSS) {
		return (*scanToNXCRowsMap)[arrayID][observationID][scanNumber][fieldID];
	}
	else {
		return (*scanToNACRowsMap)[arrayID][observationID][scanNumber][fieldID]
		    + (*scanToNXCRowsMap)[arrayID][observationID][scanNumber][fieldID];
	}

}

uInt MSMetaDataOnDemand::nRows(CorrelationType cType, Int fieldID) {
	uInt nACRows, nXCRows;
	std::tr1::shared_ptr<AOSFMapI> scanToNACRowsMap, scanToNXCRowsMap;
	std::tr1::shared_ptr<std::map<Int, uInt> > fieldToNACRowsMap, fieldToNXCRowsMap;
	_getRowStats(
		nACRows, nXCRows, scanToNACRowsMap,
		scanToNXCRowsMap, fieldToNACRowsMap,
		fieldToNXCRowsMap
	);
	if (cType == AUTO) {
		return (*fieldToNACRowsMap)[fieldID];
	}
	else if (cType == CROSS) {
		return (*fieldToNXCRowsMap)[fieldID];
	}
	else {
		return (*fieldToNACRowsMap)[fieldID] + (*fieldToNXCRowsMap)[fieldID];
	}
}

Double MSMetaDataOnDemand::nUnflaggedRows() {
	Double nACRows, nXCRows;
	std::tr1::shared_ptr<AOSFMapD> scanToNACRowsMap, scanToNXCRowsMap;
	std::tr1::shared_ptr<std::map<Int, Double> > fieldToNACRowsMap, fieldToNXCRowsMap;
	_getUnflaggedRowStats(
		nACRows, nXCRows, scanToNACRowsMap,
		scanToNXCRowsMap, fieldToNACRowsMap,
		fieldToNXCRowsMap
	);
	return nACRows + nXCRows;
}
Double MSMetaDataOnDemand::nUnflaggedRows(CorrelationType cType) {
	if (cType == BOTH) {
		return nUnflaggedRows();
	}
	Double nACRows, nXCRows;
	std::tr1::shared_ptr<AOSFMapD> scanToNACRowsMap, scanToNXCRowsMap;
	std::tr1::shared_ptr<std::map<Int, Double> > fieldToNACRowsMap, fieldToNXCRowsMap;
	_getUnflaggedRowStats(
		nACRows, nXCRows, scanToNACRowsMap,
		scanToNXCRowsMap, fieldToNACRowsMap,
		fieldToNXCRowsMap
	);
	if (cType == AUTO) {
		return nACRows;
	}
	else {
		return nXCRows;
	}
}

Double MSMetaDataOnDemand::nUnflaggedRows(
	CorrelationType cType, Int arrayID, Int observationID,
	Int scanNumber, Int fieldID
) {
	Double nACRows, nXCRows;
	std::tr1::shared_ptr<AOSFMapD> scanToNACRowsMap, scanToNXCRowsMap;
	std::tr1::shared_ptr<std::map<Int, Double> > fieldToNACRowsMap, fieldToNXCRowsMap;
	_getUnflaggedRowStats(
		nACRows, nXCRows, scanToNACRowsMap,
		scanToNXCRowsMap, fieldToNACRowsMap,
		fieldToNXCRowsMap
	);
	if (cType == AUTO) {
		return (*scanToNACRowsMap)[arrayID][observationID][scanNumber][fieldID];
	}
	else if (cType == CROSS) {
		return (*scanToNXCRowsMap)[arrayID][observationID][scanNumber][fieldID];
	}
	else {
		return (*scanToNACRowsMap)[arrayID][observationID][scanNumber][fieldID]
		    + (*scanToNXCRowsMap)[arrayID][observationID][scanNumber][fieldID];
	}
}

Double MSMetaDataOnDemand::nUnflaggedRows(CorrelationType cType, Int fieldID) {
	Double nACRows, nXCRows;
	std::tr1::shared_ptr<AOSFMapD> scanToNACRowsMap, scanToNXCRowsMap;
	std::tr1::shared_ptr<std::map<Int, Double> > fieldToNACRowsMap, fieldToNXCRowsMap;
	_getUnflaggedRowStats(
		nACRows, nXCRows, scanToNACRowsMap,
		scanToNXCRowsMap, fieldToNACRowsMap,
		fieldToNXCRowsMap
	);
	if (cType == AUTO) {
		return (*fieldToNACRowsMap)[fieldID];
	}
	else if (cType == CROSS) {
		return (*fieldToNXCRowsMap)[fieldID];
	}
	else {
		return (*fieldToNACRowsMap)[fieldID] + (*fieldToNXCRowsMap)[fieldID];
	}
}

void MSMetaDataOnDemand::_getRowStats(
	uInt& nACRows, uInt& nXCRows,
	std::tr1::shared_ptr<AOSFMapI>& scanToNACRowsMap,
	std::tr1::shared_ptr<AOSFMapI>& scanToNXCRowsMap,
	std::tr1::shared_ptr<std::map<Int, uInt> >& fieldToNACRowsMap,
	std::tr1::shared_ptr<std::map<Int, uInt> >& fieldToNXCRowsMap
) {
	// this method is responsible for setting _nACRows, _nXCRows, _scanToNACRowsMap,
	// _scanToNXCRowsMap, _fieldToNACRowsMap, _fieldToNXCRowsMap
	if (
		_nACRows > 0 || _nXCRows > 0
	) {
		nACRows = _nACRows;
		nXCRows = _nXCRows;
		scanToNACRowsMap = _scanToNACRowsMap;
		scanToNXCRowsMap = _scanToNXCRowsMap;
		fieldToNACRowsMap = _fieldToNACRowsMap;
		fieldToNXCRowsMap = _fieldToNXCRowsMap;
		return;
	}
	std::tr1::shared_ptr<Vector<Int> > ant1, ant2;
	_getAntennas(ant1, ant2);
	AOSFMapI *myScanToNACRowsMap, *myScanToNXCRowsMap;
	std::map<Int, uInt> *myFieldToNACRowsMap, *myFieldToNXCRowsMap;
	MSMetaData::_getRowStats(
		nACRows, nXCRows, myScanToNACRowsMap,
		myScanToNXCRowsMap, myFieldToNACRowsMap,
		myFieldToNXCRowsMap, *ant1, *ant2,
		*(_getScans()), *(_getFieldIDs()),
		*(_getObservationIDs()), *(_getArrayIDs())
	);
	scanToNACRowsMap.reset(myScanToNACRowsMap);
	scanToNXCRowsMap.reset(myScanToNXCRowsMap);
	fieldToNACRowsMap.reset(myFieldToNACRowsMap);
	fieldToNXCRowsMap.reset(myFieldToNXCRowsMap);

	Float newSize = _cacheMB + sizeof(Int)*(
		2 + 2*scanToNACRowsMap->size()
		+ 2*scanToNXCRowsMap->size()
		+ 2*fieldToNACRowsMap->size()
		+ fieldToNACRowsMap->size()
		+ fieldToNXCRowsMap->size()
	);

	if (_cacheUpdated(newSize)) {
		_nACRows = nACRows;
		_nXCRows = nXCRows;
		_scanToNACRowsMap = scanToNACRowsMap;
		_scanToNXCRowsMap = scanToNXCRowsMap;
		_fieldToNACRowsMap = fieldToNACRowsMap;
		_fieldToNXCRowsMap = fieldToNXCRowsMap;
	}
}

void MSMetaDataOnDemand::_getAntennas(
	std::tr1::shared_ptr<Vector<Int> >& ant1,
	std::tr1::shared_ptr<Vector<Int> >& ant2
) {
	if (
		_antenna1 && _antenna1->size() > 0
		&& _antenna2 && _antenna2->size() > 0
	) {
		ant1 = _antenna1;
		ant2 = _antenna2;
	}
	Vector<Int> a1, a2;
	MSMetaData::_getAntennas(a1, a2, *_ms);
	ant1.reset(new Vector<Int>(a1));
	ant2.reset(new Vector<Int>(a2));

	if (_cacheUpdated(2*sizeof(Int)*ant1->size())) {
		_antenna1 = ant1;
		_antenna2 = ant2;
	}
}

std::tr1::shared_ptr<Vector<Int> > MSMetaDataOnDemand::_getScans() {
	if (_scans && _scans->size() > 0) {
		return _scans;
	}
	std::tr1::shared_ptr<Vector<Int> > scans(new Vector<Int>(MSMetaData::_getScans(*_ms)));
	if (_cacheUpdated(sizeof(Int)*scans->size())) {
		_scans = scans;
	}
	return scans;
}

std::tr1::shared_ptr<Vector<Int> > MSMetaDataOnDemand::_getObservationIDs() {
	if (_observationIDs && _observationIDs->size() > 0) {
		return _observationIDs;
	}
	std::tr1::shared_ptr<Vector<Int> > obsIDs(
		new Vector<Int>(MSMetaData::_getObservationIDs(*_ms))
	);
	if (_cacheUpdated(sizeof(Int)*obsIDs->size())) {
		_observationIDs = obsIDs;
	}
	return obsIDs;
}

std::tr1::shared_ptr<Vector<Int> > MSMetaDataOnDemand::_getArrayIDs() {
	if (_arrayIDs && _arrayIDs->size() > 0) {
		return _arrayIDs;
	}
	std::tr1::shared_ptr<Vector<Int> > arrIDs(
		new Vector<Int>(MSMetaData::_getArrayIDs(*_ms))
	);
	if (_cacheUpdated(sizeof(Int)*arrIDs->size())) {
		_arrayIDs = arrIDs;
	}
	return arrIDs;
}

std::tr1::shared_ptr<Vector<Int> > MSMetaDataOnDemand::_getFieldIDs() {
	if (_fieldIDs && _fieldIDs->size() > 0) {
		return _fieldIDs;
	}
	std::tr1::shared_ptr<Vector<Int> > fields(
		new Vector<Int>(MSMetaData::_getFieldIDs(*_ms))
	);
	if (_cacheUpdated(sizeof(Int)*fields->size())) {
		_fieldIDs = fields;
	}
	return fields;
}

std::tr1::shared_ptr<Vector<Int> > MSMetaDataOnDemand::_getStateIDs() {
	if (_stateIDs && _stateIDs->size() > 0) {
		return _stateIDs;
	}
	std::tr1::shared_ptr<Vector<Int> > states(
		new Vector<Int>(MSMetaData::_getStates(*_ms))
	);
	if (_cacheUpdated(sizeof(Int)*states->size())) {
		_stateIDs = states;
	}
	return states;
}

std::tr1::shared_ptr<Vector<Int> > MSMetaDataOnDemand::_getDataDescIDs() {
	if (_dataDescIDs && ! _dataDescIDs->empty()) {
		return _dataDescIDs;
	}
	std::tr1::shared_ptr<Vector<Int> > dataDescIDs(
		new Vector<Int>(MSMetaData::_getDataDescIDs(*_ms))
	);
	if (_cacheUpdated(sizeof(Int)*dataDescIDs->size())) {
		_dataDescIDs = dataDescIDs;
	}
	return dataDescIDs;
}

std::set<Int> MSMetaDataOnDemand::getScansForState(const Int stateID) {
	_checkStateID(stateID);
	std::set<Int> uniqueScans;
	std::map<Int, std::set<Int> > myScanToStatesMap = _getScanToStatesMap();
	std::tr1::shared_ptr<Vector<Int> > scans = _getScans();
	uniqueScans.insert(scans->begin(), scans->end());
	std::set<Int>::const_iterator lastScan = uniqueScans.end();
	std::set<Int> scansForState;
	for (
		std::set<Int>::const_iterator scanNum=uniqueScans.begin();
		scanNum!=lastScan; scanNum++
	) {
		std::set<Int> statesSet = myScanToStatesMap.find(*scanNum)->second;
		if (statesSet.find(stateID) != statesSet.end()) {
			scansForState.insert(*scanNum);
		}
	}
	return scansForState;
}

std::map<Int, std::set<Int> > MSMetaDataOnDemand::_getScanToStatesMap() {
	if (! _scanToStatesMap.empty()) {
		return _scanToStatesMap;
	}
	std::map<Int, std::set<Int> > myScanToStatesMap;
	uInt mySize = 0;

	if (_ms->state().nrow() == 0) {
		std::set<Int> empty;
		std::set<Int> uniqueScans = getScanNumbers();
		std::set<Int>::const_iterator end = uniqueScans.end();
		for (
			std::set<Int>::const_iterator scanNum=uniqueScans.begin();
			scanNum!=end; scanNum++
		) {
			myScanToStatesMap[*scanNum] = empty;
		}
	}
	else {
		myScanToStatesMap = MSMetaData::_getScanToStatesMap(
			*(_getScans()), *(_getStateIDs())
		);
	}
	std::map<Int, std::set<Int> >::const_iterator end = myScanToStatesMap.end();
	for (
		std::map<Int, std::set<Int> >::const_iterator iter=myScanToStatesMap.begin();
		iter!=end; iter++
	) {
		mySize += iter->second.size() + 1;
	}
	if (_cacheUpdated(mySize*sizeof(Int)/1e6)) {
		_scanToStatesMap = myScanToStatesMap;
	}
	return myScanToStatesMap;
}
/*
std::set<String> MSMetaDataOnDemand::getIntentsForScan(const uInt scan) {
	if (_scanToIntentsMap.find(scan) != _scanToIntentsMap.end()) {
		return _scanToIntentsMap.find(scan)->second;
	}
	_checkScan(scan, getScanNumbers());
	String stateTable = _ms->tableName() + "/STATE";
	vector<const Table *> tempTables = _taqlTempTable;
	if (_taqlTempTable.size() > 0) {
		stateTable = "$2";
		tempTables.push_back(&_ms->state());
	}
	String taql = "select OBS_MODE from " + stateTable + " where "
		+ "ROWID() in [select unique(STATE_ID) from " + _taqlTableName
		+ " where SCAN_NUMBER==" + String::toString(scan) + "]";
	Table result(tableCommand(taql, tempTables));
	ROScalarColumn<String> intentsCol(result, "OBS_MODE");
	Vector<String> intents = intentsCol.getColumn();
	Vector<String>::const_iterator end = intents.end();
	std::set<String> intentsForScan;
	uInt mysize = sizeof(uInt);
	for (
		Vector<String>::const_iterator iter=intents.begin();
		iter!=end; iter++
	) {
		Vector<String> intentSet = casa::stringToVector(*iter, ',');
		intentsForScan.insert(intentSet.begin(), intentSet.end());
		mysize += iter->size();
	}
	if (_cacheUpdated(mysize)) {
		_scanToIntentsMap[scan] = intentsForScan;
	}
	return intentsForScan;
}
*/

std::set<String> MSMetaDataOnDemand::getIntentsForScan(const Int scan) {
	if (_scanToIntentsMap.find(scan) != _scanToIntentsMap.end()) {
		return _scanToIntentsMap.find(scan)->second;
	}
	_checkScan(scan, getScanNumbers());
	vector<std::set<String> > stateToIntentsMap;
	std::set<String> uniqueIntents;
	_getStateToIntentsMap(
		stateToIntentsMap, uniqueIntents
	);
	std::map<Int, std::set<Int> > scanToStatesMap = _getScanToStatesMap();
	/*
	std::tr1::shared_ptr<Vector<Int> > stateIDs = _getStateIDs();
	std::tr1::shared_ptr<Vector<Int> > scans = _getScans();
	Vector<Int>::const_iterator end = stateIDs->end();
	Vector<Int>::const_iterator curScan = scans->begin();
	*/
	std::map<Int, std::set<Int> >::const_iterator end = scanToStatesMap.end();
	std::map<Int, std::set<String> > myScanToIntentsMap;
	std::set<Int> states;
	std::set<String> intents;
	for (
		std::map<Int, std::set<Int> >::const_iterator iter=scanToStatesMap.begin();
		iter!=end; iter++
	) {
		states = iter->second;
		if (myScanToIntentsMap.find(iter->first) == myScanToIntentsMap.end()) {
			myScanToIntentsMap[iter->first] = std::set<String>();
		}
		std::set<Int>::const_iterator endState = states.end();
		for (
			std::set<Int>::const_iterator myState=states.begin();
			myState!=endState; myState++
		) {
			intents = stateToIntentsMap[*myState];
			myScanToIntentsMap[iter->first].insert(intents.begin(), intents.end());
		}
	}
	/*
	for (
		Vector<Int>::const_iterator curState=stateIDs->begin();
		curState!=end; curState++, curScan++
	) {
		if (myScanToIntentsMap.find(*curScan) == myScanToIntentsMap.end()) {
			myScanToIntentsMap[*curScan] = std::set<String>();
		}
		intents = stateToIntentsMap[*curState];
		myScanToIntentsMap[*curScan].insert(intents.begin(), intents.end());
	}
	*/
	std::map<Int, std::set<String> >::const_iterator end2 = myScanToIntentsMap.end();
	uInt mysize = myScanToIntentsMap.size()*sizeof(Int);
	for (
		std::map<Int, std::set<String> >::const_iterator iter=myScanToIntentsMap.begin();
		iter!=end2; iter++
	) {
		intents = iter->second;
		std::set<String>::const_iterator end3 = intents.end();
		for (
			std::set<String>::const_iterator intent=intents.begin();
			intent!=end3; intent++
		) {
			mysize += intent->size();
		}
	}
	if (_cacheUpdated(mysize)) {
		_scanToIntentsMap = myScanToIntentsMap;
	}
	return myScanToIntentsMap[scan];
}


Bool MSMetaDataOnDemand::_cacheUpdated(const Float incrementInBytes) {
	Float newSize = _cacheMB + incrementInBytes/1e6;
	if (newSize <= _maxCacheMB) {
		_cacheMB = newSize;
		return True;
	}
	return False;
}

std::set<uInt> MSMetaDataOnDemand::getSpwsForIntent(const String& intent) {
	std::set<String> uniqueIntents = getIntents();
	if (uniqueIntents.find(intent) == uniqueIntents.end()) {
		throw AipsError(
			_ORIGIN + "Unknown intent "
			+ intent + " for this dataset"
		);
	}
	vector<std::set<String> > spwToIntentsMap = _getSpwToIntentsMap();
	std::set<uInt> spws;
	for (uInt i=0; i<spwToIntentsMap.size(); i++) {
		if (
			spwToIntentsMap[i].find(intent) != spwToIntentsMap[i].end()
		) {
			spws.insert(i);
		}
	}
	return spws;
}

uInt MSMetaDataOnDemand::nSpw(Bool includewvr) {
	if (_nSpw > 0) {
		return includewvr ? _nSpw : _nSpw - getWVRSpw().size();
	}
	uInt nSpw = _ms->spectralWindow().nrow();
	_nSpw = nSpw;
	return includewvr ? nSpw : nSpw - getWVRSpw().size();
}

std::set<String> MSMetaDataOnDemand::getIntentsForSpw(const uInt spw) {
	if (spw >= nSpw(True)) {
		throw AipsError(
			_ORIGIN + "spectral window out of range"
		);
	}
	return _getSpwToIntentsMap()[spw];
}

std::set<String> MSMetaDataOnDemand::getIntentsForField(Int fieldID) {
	_checkFieldID(fieldID);
	if (! _fieldToIntentsMap.empty()) {
		return _fieldToIntentsMap[fieldID];
	}
	return _getFieldToIntentsMap()[fieldID];
}


uInt MSMetaDataOnDemand::nFields() {
	if (_nFields > 0) {
		return _nFields;
	}
	uInt nFields = _ms->field().nrow();
	_nFields = nFields;
	return nFields;
}

std::set<uInt> MSMetaDataOnDemand::getSpwsForField(const Int fieldID) {
	_checkFieldID(fieldID);
	if (_fieldToSpwMap.empty()) {
		_fieldToSpwMap.clear();
		std::set<Int> uniqueFields = _getUniqueFiedIDs();
		std::set<Int>::const_iterator end = uniqueFields.end();
		for (
			std::set<Int>::const_iterator f=uniqueFields.begin();
			f!=end; f++
		) {
			_fieldToSpwMap[*f] = std::set<uInt>();
		}
	}
	else if (! _fieldToSpwMap[fieldID].empty()) {
		return _fieldToSpwMap[fieldID];
	}
	String ddIDTable = _ms->tableName() + "/DATA_DESCRIPTION";
	vector<const Table *> tempTables = _taqlTempTable;
	if (_taqlTempTable.size() > 0) {
		ddIDTable = "$2";
		tempTables.push_back(&_ms->dataDescription());
	}


	String taql = "select unique(SPECTRAL_WINDOW_ID) from " + ddIDTable
		+ " where ROWID() in "
		+ "[select unique(DATA_DESC_ID) from " + _taqlTableName
		+ " where FIELD_ID == " + String::toString(fieldID) + "]";
	Table result(tableCommand(taql, tempTables));
	ROScalarColumn<Int> spwCol(result, "SPECTRAL_WINDOW_ID");
	vector<uInt> spws = _toUIntVector(spwCol.getColumn().tovector());
	std::set<uInt> spwIds(spws.begin(), spws.end());
	uInt mysize = sizeof(uInt) * spwIds.size();
	if (_cacheUpdated(mysize)) {
		_fieldToSpwMap[fieldID] = spwIds;
	}
	return spwIds;
}

std::set<uInt> MSMetaDataOnDemand::getSpwsForField(const String& fieldName) {
	uInt myNFields = nFields();
	vector<String> fieldNames = _getFieldNames();

	for (uInt i=0; i<myNFields; i++) {
		if (fieldNames[i] == fieldName) {
			return getSpwsForField(i);
		}
	}
	throw AipsError(
		_ORIGIN + "field (" + fieldName + " does not exist"
	);
}

vector<String> MSMetaDataOnDemand::_getFieldNames() {
	if (! _fieldNames.empty()) {
		return _fieldNames;
	}
	vector<String> fieldNames = MSMetaData::_getFieldNames(*_ms);
	uInt mysize = 0;
	vector<String>::const_iterator end = fieldNames.end();
	for (
		vector<String>::const_iterator name=fieldNames.begin();
		name!=end; name++
	) {
		mysize += name->size();
	}
	if (_cacheUpdated(mysize)) {
		_fieldNames = fieldNames;
	}
	return fieldNames;
}

std::set<Int> MSMetaDataOnDemand::getFieldIDsForSpw(const uInt spw) {
	uInt myNSpw = nSpw(True);
	if (spw >= myNSpw) {
		throw AipsError(_ORIGIN + "spectral window out of range");
	}
	if (_spwToFieldIDsMap.empty()) {
		_spwToFieldIDsMap.resize(myNSpw);
	}
	else if (! _spwToFieldIDsMap[spw].empty()) {
		return _spwToFieldIDsMap[spw];
	}
	String taql = "select unique(FIELD_ID) from " + _taqlTableName
		+ " where DATA_DESC_ID in " +
		"[select ROWID() from ::DATA_DESCRIPTION where SPECTRAL_WINDOW_ID=="
		+ String::toString(spw) + "]";
	Table result(tableCommand(taql, _taqlTempTable ));
	ROScalarColumn<Int> fieldCol(result, "FIELD_ID");
	Vector<Int> fields = fieldCol.getColumn().tovector();
	std::set<Int> fieldIds(fields.begin(), fields.end());
	if (_cacheUpdated(sizeof(Int)*fieldIds.size())) {
		_spwToFieldIDsMap[spw] = fieldIds;
	}
	return fieldIds;
}

std::set<String> MSMetaDataOnDemand::getFieldNamesForSpw(const uInt spw) {
	std::set<Int> fieldIDs = getFieldIDsForSpw(spw);
	std::set<String> fieldNames;
	vector<String> allFieldNames = _getFieldNames();
	for (
		std::set<Int>::const_iterator fieldID = fieldIDs.begin();
		fieldID!=fieldIDs.end(); fieldID++
	) {
		fieldNames.insert(allFieldNames[*fieldID]);
	}
	return fieldNames;
}

/*
std::set<uInt> MSMetaDataOnDemand::getSpwsForScan(const uInt scan) {
	if (_scanToSpwsMap.find(scan) != _scanToSpwsMap.end()) {
		return _scanToSpwsMap[scan];
	}
	_checkScan(scan, getScanNumbers());
	String ddIDTable = _ms->tableName() + "/DATA_DESCRIPTION";
	vector<const Table *> tempTables = _taqlTempTable;
	if (_taqlTempTable.size() > 0) {
		ddIDTable = "$2";
		tempTables.push_back(&_ms->dataDescription());
	}
	String taql = "select unique(SPECTRAL_WINDOW_ID) from " + ddIDTable
		+ " where ROWID() in "
		+ "[select unique(DATA_DESC_ID) from " + _taqlTableName
		+ " where SCAN_NUMBER == " + String::toString(scan) + "]";
	Table result(tableCommand(taql, tempTables));
	ROScalarColumn<Int> spwCol(result, "SPECTRAL_WINDOW_ID");
	vector<uInt> spws = _toUIntVector(spwCol.getColumn().tovector());
	std::set<uInt> spwIds(spws.begin(), spws.end());
	if (_cacheUpdated(sizeof(uInt)*spwIds.size())) {
		_scanToSpwsMap[scan] = spwIds;
	}
	return spwIds;
}
*/

std::set<uInt> MSMetaDataOnDemand::getSpwsForScan(const Int scan) {
    _checkScan(scan, getScanNumbers());
    if (_scanToSpwsMap.find(scan) != _scanToSpwsMap.end()) {
        return _scanToSpwsMap[scan];
    }
    std::tr1::shared_ptr<Vector<Int> > ddIDs = _getDataDescIDs();
    std::tr1::shared_ptr<Vector<Int> > scans = _getScans();
    std::map<Int, uInt> ddToSpw = _getDataDescIDToSpwMap();
    Vector<Int>::const_iterator end = ddIDs->end();
    Vector<Int>::const_iterator myscan = scans->begin();
    std::map<Int, std::set<uInt> > scanToSpwMap;
    for (
        Vector<Int>::const_iterator ddID=ddIDs->begin();
        ddID!=end; ddID++, myscan++
    ) {
        if (scanToSpwMap.find(*myscan) == scanToSpwMap.end()) {
            scanToSpwMap[*myscan] = std::set<uInt>();
        }
        scanToSpwMap[*myscan].insert(ddToSpw[*ddID]);
    }
    // FIXME do caching
    _scanToSpwsMap = scanToSpwMap;
    return scanToSpwMap[scan];
}

std::set<Int> MSMetaDataOnDemand::getScansForSpw(const uInt spw) {
	uInt myNSpw = nSpw(True);
	if (spw >= myNSpw) {
		throw AipsError(
			_ORIGIN + "spectral window out of range"
		);
	}
	if (_spwToScansMap.empty()) {
		_spwToScansMap.resize(myNSpw);
	}
	else if (! _spwToScansMap[spw].empty()) {
		return _spwToScansMap[spw];
	}
	String taql = "select unique(SCAN_NUMBER) from " + _taqlTableName
		+ " where DATA_DESC_ID in " +
		"[select ROWID() from ::DATA_DESCRIPTION where SPECTRAL_WINDOW_ID=="
		+ String::toString(spw) + "]";
	Table result(tableCommand(taql, _taqlTempTable));
	ROScalarColumn<Int> scanCol(result, "SCAN_NUMBER");
	vector<Int> scans = scanCol.getColumn().tovector();
	std::set<Int> scanIds(scans.begin(), scans.end());
	if (_cacheUpdated(sizeof(Int)*scanIds.size())) {
		_spwToScansMap[spw] = scanIds;
	}
	return scanIds;
}

uInt MSMetaDataOnDemand::nAntennas() {
	if (_nAntennas > 0) {
		return _nAntennas;
	}
	uInt nAnts = _ms->antenna().nrow();
	_nAntennas = nAnts;
	return nAnts;
}

vector<String> MSMetaDataOnDemand::getAntennaNames(
	std::map<String, uInt>& namesToIDsMap,
	const vector<uInt>& antennaIDs
) {
	uInt nAnts = nAntennas();
	vector<String> names, myAnts;
	if (antennaIDs.empty()) {
		if (_antennaNames.empty()) {
			std::map<String, uInt> mymap;
			names = _getAntennaNames(namesToIDsMap, *_ms);
			myAnts = names;
		}
		else {
			namesToIDsMap = _antennaNameToIDMap;
			return _antennaNames;
		}
	}
	else {
		uInt mymax = max(Vector<uInt>(antennaIDs));
		if (mymax >= nAnts) {
			throw AipsError(
				_ORIGIN + "Antenna ID " + String::toString(mymax)
			+ " out of range."
			);
		}
		if (! _antennaNames.empty()) {
			vector<uInt>::const_iterator end = antennaIDs.end();
			for (
				vector<uInt>::const_iterator id=antennaIDs.begin();
				id!=end; id++
			) {
				names.push_back(_antennaNames[*id]);
			}
			namesToIDsMap = _antennaNameToIDMap;
			return names;
		}
		else {
			std::map<String, uInt> mymap;
			myAnts = _getAntennaNames(namesToIDsMap, *_ms);
			vector<uInt>::const_iterator end = antennaIDs.end();
			for (
				vector<uInt>::const_iterator id=antennaIDs.begin();
				id!=end; id++
			) {
				names.push_back(myAnts[*id]);
			}
		}
	}
	vector<String>::const_iterator end1 = myAnts.end();
	uInt mysize = nAnts*sizeof(uInt);
	for (
		vector<String>::const_iterator name=myAnts.begin();
		name!=end1; name++
	) {
		mysize += 2*name->size();
	}
	if (_cacheUpdated(mysize)) {
		_antennaNames = myAnts;
		_antennaNameToIDMap = namesToIDsMap;
	}
	return names;
}

vector<uInt> MSMetaDataOnDemand::getAntennaIDs(
	const vector<String>& antennaNames
) {
	std::map<String, uInt> namesToIDsMap;
	vector<String> names = getAntennaNames(namesToIDsMap);
	vector<String>::const_iterator end = antennaNames.end();
	std::map<String, uInt>::const_iterator mapEnd = namesToIDsMap.end();
	vector<uInt> ids;
	for (
		vector<String>::const_iterator name=antennaNames.begin();
		name!=end; name++
	) {
		std::map<String, uInt>::const_iterator pair = namesToIDsMap.find(*name);
		if (pair == mapEnd) {
			throw AipsError(
				_ORIGIN + "Unknown antenna " + *name
			);
		}
		ids.push_back(pair->second);
	}
	return ids;
}

std::set<uInt> MSMetaDataOnDemand::getTDMSpw() {
	if (! _tdmSpw.empty()) {
		return _tdmSpw;
	}
	std::set<uInt> avgSpw, tdmSpw, fdmSpw, wvrSpw;
	_getSpwInfo(avgSpw, tdmSpw, fdmSpw, wvrSpw);
	return tdmSpw;
}

vector<Double> MSMetaDataOnDemand::getBandWidths() {
	std::set<uInt> avgSpw, tdmSpw, fdmSpw, wvrSpw;
	vector<MSMetaData::SpwProperties> props = _getSpwInfo(
		avgSpw, tdmSpw, fdmSpw, wvrSpw
	);
	vector<MSMetaData::SpwProperties>::const_iterator end = props.end();
	vector<Double> out;
	for (
		vector<MSMetaData::SpwProperties>::const_iterator iter=props.begin();
		iter!=end; iter++
	) {
		out.push_back(iter->bandwidth);
	}
	return out;
}

vector<Quantum<Vector<Double> > > MSMetaDataOnDemand::getChanFreqs() {
	std::set<uInt> avgSpw, tdmSpw, fdmSpw, wvrSpw;
	vector<MSMetaData::SpwProperties> props = _getSpwInfo(
		avgSpw, tdmSpw, fdmSpw, wvrSpw
	);
	vector<MSMetaData::SpwProperties>::const_iterator end = props.end();
	vector<Quantum<Vector<Double> > > out;
	for (
		vector<MSMetaData::SpwProperties>::const_iterator iter=props.begin();
		iter!=end; iter++
	) {
		out.push_back(iter->chanfreqs);
	}
	return out;
}

vector<vector<Double> > MSMetaDataOnDemand::getChanWidths() {
	std::set<uInt> avgSpw, tdmSpw, fdmSpw, wvrSpw;
	vector<MSMetaData::SpwProperties> props = _getSpwInfo(
		avgSpw, tdmSpw, fdmSpw, wvrSpw
	);
	vector<MSMetaData::SpwProperties>::const_iterator end = props.end();
	vector<vector<Double> > out;
	for (
		vector<MSMetaData::SpwProperties>::const_iterator iter=props.begin();
		iter!=end; iter++
	) {
		out.push_back(iter->chanwidths);
	}
	return out;
}

vector<Int> MSMetaDataOnDemand::getNetSidebands() {
	std::set<uInt> avgSpw, tdmSpw, fdmSpw, wvrSpw;
	vector<MSMetaData::SpwProperties> props = _getSpwInfo(
		avgSpw, tdmSpw, fdmSpw, wvrSpw
	);
	vector<MSMetaData::SpwProperties>::const_iterator end = props.end();
	vector<Int> out;
	for (
		vector<MSMetaData::SpwProperties>::const_iterator iter=props.begin();
		iter!=end; iter++
	) {
		out.push_back(iter->netsideband);
	}
	return out;
}

vector<Quantity> MSMetaDataOnDemand::getMeanFreqs() {
	std::set<uInt> avgSpw, tdmSpw, fdmSpw, wvrSpw;
	vector<MSMetaData::SpwProperties> props = _getSpwInfo(
		avgSpw, tdmSpw, fdmSpw, wvrSpw
	);
	vector<MSMetaData::SpwProperties>::const_iterator end = props.end();
	vector<Quantity> out;
	for (
		vector<MSMetaData::SpwProperties>::const_iterator iter=props.begin();
		iter!=end; iter++
	) {
		out.push_back(iter->meanfreq);
	}
	return out;
}

vector<uInt> MSMetaDataOnDemand::nChans() {
	std::set<uInt> avgSpw, tdmSpw, fdmSpw, wvrSpw;
	vector<MSMetaData::SpwProperties> props = _getSpwInfo(
		avgSpw, tdmSpw, fdmSpw, wvrSpw
	);
	vector<MSMetaData::SpwProperties>::const_iterator end = props.end();
	vector<uInt> out;
	for (
		vector<MSMetaData::SpwProperties>::const_iterator iter=props.begin();
		iter!=end; iter++
	) {
		out.push_back(iter->nchans);
	}
	return out;
}

vector<vector<Double> > MSMetaDataOnDemand::getEdgeChans() {
	std::set<uInt> avgSpw, tdmSpw, fdmSpw, wvrSpw;
	vector<MSMetaData::SpwProperties> props = _getSpwInfo(
		avgSpw, tdmSpw, fdmSpw, wvrSpw
	);
	vector<MSMetaData::SpwProperties>::const_iterator end = props.end();
	vector<vector<Double> > out;
	for (
		vector<MSMetaData::SpwProperties>::const_iterator iter=props.begin();
		iter!=end; iter++
	) {
		out.push_back(iter->edgechans);
	}
	return out;
}

vector<uInt> MSMetaDataOnDemand::getBBCNos() {
	if (! hasBBCNo(*_ms)) {
		throw AipsError("This MS's SPECTRAL_WINDOW table does not have a BBC_NO column");
	}
	std::set<uInt> avgSpw, tdmSpw, fdmSpw, wvrSpw;
	vector<MSMetaData::SpwProperties> props = _getSpwInfo(
		avgSpw, tdmSpw, fdmSpw, wvrSpw
	);
	vector<MSMetaData::SpwProperties>::const_iterator end = props.end();
	vector<uInt> out;
	for (
		vector<MSMetaData::SpwProperties>::const_iterator iter=props.begin();
		iter!=end; iter++
	) {
		out.push_back(iter->bbcno);
	}
	return out;
}

vector<String> MSMetaDataOnDemand::getSpwNames() {
	std::set<uInt> avgSpw, tdmSpw, fdmSpw, wvrSpw;
	vector<MSMetaData::SpwProperties> props = _getSpwInfo(
		avgSpw, tdmSpw, fdmSpw, wvrSpw
	);
	vector<MSMetaData::SpwProperties>::const_iterator end = props.end();
	vector<String> out;
	for (
		vector<MSMetaData::SpwProperties>::const_iterator iter=props.begin();
		iter!=end; iter++
	) {
		out.push_back(iter->name);
	}
	return out;
}

std::set<uInt> MSMetaDataOnDemand::getFDMSpw() {
	if (! _fdmSpw.empty()) {
		return _fdmSpw;
	}
	std::set<uInt> avgSpw, tdmSpw, fdmSpw, wvrSpw;
	_getSpwInfo(avgSpw, tdmSpw, fdmSpw, wvrSpw);
	return fdmSpw;
}

std::set<uInt> MSMetaDataOnDemand::getChannelAvgSpw() {
	if (! _avgSpw.empty()) {
		return _avgSpw;
	}
	std::set<uInt> avgSpw, tdmSpw, fdmSpw, wvrSpw;
	_getSpwInfo(avgSpw, tdmSpw, fdmSpw, wvrSpw);
	return avgSpw;
}

std::set<uInt> MSMetaDataOnDemand::getWVRSpw() {
	if (! _wvrSpw.empty()) {
		return _wvrSpw;
	}
	std::set<uInt> avgSpw, tdmSpw, fdmSpw, wvrSpw;
	_getSpwInfo(avgSpw, tdmSpw, fdmSpw, wvrSpw);
	return wvrSpw;
}

std::set<Int> MSMetaDataOnDemand::getScansForTimes(
	const Double center, const Double tol
) {
	_checkTolerance(tol);
	std::set<Int> uniqueScans = getScanNumbers();
	std::tr1::shared_ptr<std::map<Int, std::set<Double> > > scanToTimesMap = _getScanToTimesMap();
	Double minTime = center - tol;
	Double maxTime = center + tol;
	std::set<Int> scans;
	std::set<Int>::const_iterator end = uniqueScans.end();
	for (
		std::set<Int>::const_iterator scan=uniqueScans.begin();
		scan!=end; scan++
	) {
		std::set<Double> times = scanToTimesMap->find(*scan)->second;
		if (*(++times.rend()) >= minTime && *times.begin() <= maxTime) {
			scans.insert(*scan);
		}
	}
	return scans;
}

std::tr1::shared_ptr<std::map<Int, std::set<Double> > > MSMetaDataOnDemand::_getScanToTimesMap() {
	if (_scanToTimesMap && ! _scanToTimesMap->empty()) {
		return _scanToTimesMap;
	}
	std::tr1::shared_ptr<std::map<Int, std::set<Double> > > scanToTimesMap(
		new std::map<Int, std::set<Double> >(
			MSMetaData::_getScanToTimesMap(
				*_getScans(), *_getTimes()
			)
		)
	);
	uInt mysize = 0;
	std::map<Int, std::set<Double> >::const_iterator end = scanToTimesMap->end();
	for (
		std::map<Int, std::set<Double> >::const_iterator iter=scanToTimesMap->begin();
		iter!=end; iter++
	) {
		mysize += iter->second.size();
	}
	mysize *= sizeof(Double);
	mysize += sizeof(Int)*scanToTimesMap->size();
	if (_cacheUpdated(mysize)) {
		_scanToTimesMap = scanToTimesMap;
	}
	return scanToTimesMap;
}

std::tr1::shared_ptr<Vector<Double> > MSMetaDataOnDemand::_getTimes() {
	if (_times && ! _times->empty()) {
		return _times;
	}
	std::tr1::shared_ptr<Vector<Double> > times(
		new Vector<Double>(MSMetaData::_getTimes(*_ms))
	);
	if (_cacheUpdated(sizeof(Double)*times->size())) {
		_times = times;
	}
	return times;
}

std::tr1::shared_ptr<ArrayColumn<Bool> > MSMetaDataOnDemand::_getFlags() {
	if (_flagsColumn && ! _flagsColumn->nrow() > 0) {
			return _flagsColumn;
		}
		std::tr1::shared_ptr<ArrayColumn<Bool> > flagsColumn(
			MSMetaData::_getFlags(*_ms)
		);
		uInt mysize = 0;
		for (uInt i=0; i<flagsColumn->nrow(); i++) {
			mysize += flagsColumn->get(i).size();
		}
		if (_cacheUpdated(sizeof(Bool)*mysize)) {
			_flagsColumn = flagsColumn;
		}
		return flagsColumn;
}


std::set<Double> MSMetaDataOnDemand::getTimesForScans(
	const std::set<Int>& scans
) {
	std::set<Double> times;
	std::tr1::shared_ptr<std::map<Int, std::set<Double> > > scanToTimesMap = _getScanToTimesMap();
	std::set<Int> scanNumbers = getScanNumbers();
	for (
		std::set<Int>::const_iterator scan=scans.begin();
		scan!=scans.end(); scan++
	) {
		_checkScan(*scan, scanNumbers);
		times.insert(
			scanToTimesMap->find(*scan)->second.begin(),
			scanToTimesMap->find(*scan)->second.end()
		);
	}
	return times;
}

void MSMetaDataOnDemand::_getTimesAndInvervals(
	std::map<Int, vector<Double> >& scanToTimeRangeMap,
	std::map<Int, std::map<uInt, Double> >& scanSpwToIntervalMap
) {

	scanToTimeRangeMap = MSMetaData::_getScanToTimeRangeMap(
		scanSpwToIntervalMap,
		*_getScans(), MSMetaData::_getTimeCentroids(*_ms), _getIntervals(*_ms),
		*_getDataDescIDs(), _getDataDescIDToSpwMap(), getScanNumbers()
	);
	uInt mysize = scanToTimeRangeMap.size()*(sizeof(Int)+2*sizeof(Double));
	if (_cacheUpdated(mysize)) {
		_scanToTimeRangeMap = scanToTimeRangeMap;
	}
	mysize = scanSpwToIntervalMap.size() * nSpw(True) * sizeof(Double);
	if (_cacheUpdated(mysize)) {
		_scanSpwToIntervalMap = scanSpwToIntervalMap;
	}
}

vector<Double> MSMetaDataOnDemand::getTimeRangeForScan(Int scan) {
	_checkScan(scan, getScanNumbers());
	if (! _scanToTimeRangeMap.empty()) {
		return _scanToTimeRangeMap[scan];
	}
	std::map<Int, vector<Double> > scanToTimeRangeMap;
	std::map<Int, std::map<uInt, Double> > scanSpwToIntervalMap;
	_getTimesAndInvervals(
		scanToTimeRangeMap,
		scanSpwToIntervalMap
	);
	return scanToTimeRangeMap[scan];
}

std::map<uInt, Double> MSMetaDataOnDemand::getAverageIntervalsForScan(Int scan) {
	_checkScan(scan, getScanNumbers());
	if (! _scanSpwToIntervalMap.empty()) {
		return _scanSpwToIntervalMap[scan];
	}
	std::map<Int, vector<Double> > scanToTimeRangeMap;
	std::map<Int, std::map<uInt, Double> > scanSpwToIntervalMap;
	_getTimesAndInvervals(
		scanToTimeRangeMap,
		scanSpwToIntervalMap
	);
	return scanSpwToIntervalMap[scan];

}

std::set<Int> MSMetaDataOnDemand::getStatesForScan(const Int scan) {
	_checkScan(scan, getScanNumbers());
	return _getScanToStatesMap().find(scan)->second;
}

std::set<Int> MSMetaDataOnDemand::getScansForIntent(const String& intent) {
	if (_intentToScansMap.find(intent) != _intentToScansMap.end()) {
		return _intentToScansMap[intent];
	}
	String taql = "select unique(SCAN_NUMBER) from " + _taqlTableName
		+ " where STATE_ID in "
		+ "[select ROWID() from ::STATE where OBS_MODE=pattern('*" + intent + "*')]";
	Table result(tableCommand(taql, _taqlTempTable));
	ROScalarColumn<Int> scanCol(result, "SCAN_NUMBER");
	Vector<Int> scans = scanCol.getColumn();
	std::set<Int> myscans(scans.begin(), scans.end());
	uInt mysize = intent.size() + sizeof(uInt)*myscans.size();
	if (_cacheUpdated(mysize)) {
		_intentToScansMap[intent] = myscans;
	}
	return myscans;
}

std::set<Int> MSMetaDataOnDemand::getScansForFieldID(const Int fieldID) {
	_checkFieldID(fieldID);
	if (! _fieldToScansMap.empty() && ! _fieldToScansMap[fieldID].empty()) {
		return _fieldToScansMap[fieldID];
	}
	std::set<Int> scans;
	std::tr1::shared_ptr<Vector<Int> > fieldIds = _getFieldIDs();
	Vector<Int>::const_iterator curFieldID = fieldIds->begin();
	Vector<Int>::const_iterator end = fieldIds->end();
	std::tr1::shared_ptr<Vector<Int> > allScans = _getScans();
	Vector<Int>::const_iterator curScan = allScans->begin();
	while (curFieldID != end) {
		if (fieldID == *curFieldID) {
			scans.insert(*curScan);
		}
		curFieldID++;
		curScan++;
	}
	if (_cacheUpdated(sizeof(uInt)*scans.size())) {
		_fieldToScansMap[fieldID] = scans;
	}
	return scans;
}

std::set<Int> MSMetaDataOnDemand::getFieldIDsForField(
	const String& field
) {
	std::set<Int> fieldIDs;
	String name = field;
	vector<String> fieldNames = _getFieldNames();
	uInt nNames = fieldNames.size();
	name.upcase();
	for (uInt i=0; i<nNames; i++) {
		String testName = fieldNames[i];
		testName.upcase();
		if (name == testName) {
			fieldIDs.insert(i);
		}
	}
	if (fieldIDs.empty()) {
		throw AipsError(
			_ORIGIN + "Unknown field name " + field
		);
	}
	return fieldIDs;
}

std::set<Int> MSMetaDataOnDemand::getFieldsForScan(const Int scan) {
	_checkScan(scan, getScanNumbers());
	if (_scanToFieldsMap.find(scan) != _scanToFieldsMap.end()) {
		return _scanToFieldsMap[scan];
	}
	String taql = "select unique(FIELD_ID) from " + _taqlTableName
		+ " where SCAN_NUMBER == " + String::toString(scan);
	Table result(tableCommand(taql, _taqlTempTable));
	ROScalarColumn<Int> fieldCol(result, "FIELD_ID");
	Vector<Int> fields = fieldCol.getColumn();
	std::set<Int> myfields (fields.begin(), fields.end());
	if (_cacheUpdated(sizeof(uInt)*(1 + myfields.size()))) {
		_scanToFieldsMap[scan] = myfields;
	}
	return myfields;
}

std::set<Int> MSMetaDataOnDemand::getFieldsForScans(const std::set<Int>& scans) {
	_checkScan(*(++scans.rend()), getScanNumbers());
	String scanString;
	std::set<Int>::const_iterator end = scans.end();
	std::set<Int> myfields;
	for (
		std::set<Int>::const_iterator iter=scans.begin();
		iter!=end; iter++
	) {
		if (_scanToFieldsMap.find(*iter) != _scanToFieldsMap.end()) {
			std::set<Int> newfields = _scanToFieldsMap[*iter];
			myfields.insert(newfields.begin(), newfields.end());
		}
		else {
			if (! scanString.empty()) {
				scanString += ", ";
			}
			scanString += String::toString(*iter);
		}
	}
	if (! scanString.empty()) {
		String taql = "select unique(FIELD_ID) from " + _taqlTableName
			+ " where SCAN_NUMBER in [" + scanString + "]";
		Table result(tableCommand(taql, _taqlTempTable));
		ROScalarColumn<Int> fieldCol(result, "FIELD_ID");
		Vector<Int> fields = fieldCol.getColumn();
		myfields.insert(fields.begin(), fields.end());
	}
	return myfields;
}

std::set<Int> MSMetaDataOnDemand::getFieldsForIntent(const String& intent) {
	if (_intentToFieldIDMap.find(intent) != _intentToFieldIDMap.end()) {
		return _intentToFieldIDMap[intent];
	}
	String taql = "select unique(FIELD_ID) from " + _taqlTableName
		+ " where STATE_ID in "
		+ "[select ROWID() from ::STATE "
		+ "where OBS_MODE=pattern('*" + intent + "*')]";
	Table result(tableCommand(taql, _taqlTempTable));
	ROScalarColumn<Int> fieldCol(result, "FIELD_ID");
	Vector<Int> fields = fieldCol.getColumn().tovector();
	std::set<Int> myfields(fields.begin(), fields.end());
	if (_cacheUpdated(intent.size() + sizeof(Int)*myfields.size())) {
		_intentToFieldIDMap[intent] = myfields;
	}
	return myfields;
}

vector<String> MSMetaDataOnDemand::getFieldNamesForFieldIDs(
	const vector<Int>& fieldIDs
) {
	if (fieldIDs.size() == 0) {
		return _getFieldNames();
	}
	_checkFieldIDs(fieldIDs);
	vector<String> allNames = _getFieldNames();
	vector<String> names;
	vector<Int>::const_iterator end = fieldIDs.end();
	for (
		vector<Int>::const_iterator iter=fieldIDs.begin();
		iter!=end; iter++
	) {
		names.push_back(allNames[*iter]);
	}
	return names;
}

std::set<Int> MSMetaDataOnDemand::getFieldsForTimes(
	const Double center, const Double tol
) const {
	_checkTolerance(tol);
	Double minTime = center - tol;
	Double maxTime = center + tol;
	ostringstream timeString;
	timeString << std::setprecision(12) << minTime << " and " << maxTime;
	String taql = "select FIELD_ID from " + _taqlTableName
		+ " where TIME BETWEEN " + String(timeString.str());
	Table result(tableCommand(taql, _taqlTempTable));
	ROScalarColumn<Int> fieldCol(result, "FIELD_ID");
	Vector<Int> fields = fieldCol.getColumn().tovector();
	return std::set<Int>(fields.begin(), fields.end());
}

std::set<Double> MSMetaDataOnDemand::getTimesForField(const Int fieldID) {
	_checkFieldID(fieldID);
	if (_fieldToTimesMap) {
		std::map<Int, std::set<Double> >::const_iterator iter = _fieldToTimesMap->find(fieldID);
		if (iter != _fieldToTimesMap->end() && ! iter->second.empty()) {
			return _fieldToTimesMap->find(fieldID)->second;
		}
	}
	else {
		_fieldToTimesMap.reset(new std::map<Int, std::set<Double> >());
	}
	String taql = "select unique(TIME) from " + _taqlTableName
		+ " where FIELD_ID=" + String::toString(fieldID);
	Table result(tableCommand(taql, _taqlTempTable));
	ROScalarColumn<Double> timeCol(result, "TIME");
	Vector<Double> times = timeCol.getColumn();
	std::set<Double> mytimes(times.begin(), times.end());
	if (_cacheUpdated(sizeof(Double)*mytimes.size())) {
		(*_fieldToTimesMap)[fieldID] = mytimes;
	}
	return mytimes;
}

vector<String> MSMetaDataOnDemand::getObservatoryNames() {
	if (! _observatoryNames.empty()) {
		return _observatoryNames;
	}
	String tnameColName = MSObservation::columnName(MSObservationEnums::TELESCOPE_NAME);
	ROScalarColumn<String> telescopeNameCol(_ms->observation(), tnameColName);
	vector<String> names = telescopeNameCol.getColumn().tovector();
	uInt mysize = 0;
	vector<String>::const_iterator end = names.end();
	for (
		vector<String>::const_iterator iter=names.begin();
		iter!=end; iter++
	) {
		mysize += iter->size();
	}
	if (_cacheUpdated(mysize)) {
		_observatoryNames = names;
	}
	return names;
}

MPosition MSMetaDataOnDemand::getObservatoryPosition(uInt which) {
	if (which >= _ms->observation().nrow()) {
		throw AipsError(_ORIGIN + " out of range exception.");
	}
	if (! _observatoryPositions.empty()) {
		return _observatoryPositions[which];
	}

	vector<String> names;
	vector<MPosition> positions = _getObservatoryPositions(names, *_ms);
	if (_cacheUpdated(30*positions.size())) {
		_observatoryPositions = positions;
	}
	return positions[which];
}

vector<MPosition> MSMetaDataOnDemand::getAntennaPositions(
	const vector<uInt>& which
) {
	vector<String> antNames;
	vector<MPosition> output, allAnts;
	if (which.size() == 0) {
		if (_antennaPositions.empty()) {
			output = _getAntennaPositions(antNames, *_ms);
			allAnts = output;
		}
		else {
			return _antennaPositions;
		}
	}
	else if (max(Vector<uInt>(which)) >= nAntennas()) {
		throw AipsError(_ORIGIN + "out of range exception.");
	}
	else {
		allAnts = _antennaPositions.empty()
			? _getAntennaPositions(antNames, *_ms)
			: _antennaPositions;
		vector<uInt>::const_iterator end = which.end();
		for (
			vector<uInt>::const_iterator iter=which.begin();
			iter!=end; iter++
		) {
			output.push_back(allAnts[*iter]);
		}
	}
	if (_antennaPositions.empty() && _cacheUpdated(30*allAnts.size())) {
		_antennaPositions = allAnts;
	}
	return output;
}

vector<MPosition> MSMetaDataOnDemand::getAntennaPositions(
	const vector<String>& names
) {
	if (names.size() == 0) {
		throw AipsError(_ORIGIN + "names cannot be empty");
	}
	return getAntennaPositions(getAntennaIDs(names));
}

Quantum<Vector<Double> > MSMetaDataOnDemand::getAntennaOffset(uInt which) {
	if (which >= nAntennas()) {
		throw AipsError(_ORIGIN + "Out of range exception.");
	}
	return getAntennaOffsets()[which];
}

vector<Quantum<Vector<Double> > > MSMetaDataOnDemand::getAntennaOffsets(
	const vector<MPosition>& positions
) {
	if (! _antennaOffsets.empty()) {
		return _antennaOffsets;
	}
	uInt nPos = positions.size();
	vector<String> names;
	vector<String> obsNames;
	vector<Quantum<Vector<Double> > > offsets;
	if (nPos > 0 && nPos != nAntennas()) {
		throw AipsError(_ORIGIN + "Incorrect number of positions provided.");
	}
	offsets = (nPos > 0)
		? _getAntennaOffsets(
			positions, _getObservatoryPositions(obsNames, *_ms)[0]
		)
		: _getAntennaOffsets(
			_getAntennaPositions(names, *_ms),
			_getObservatoryPositions(obsNames, *_ms)[0]
		);
	if (_cacheUpdated(30*offsets.size())) {
		_antennaOffsets = offsets;
	}
	return offsets;
}

/*
std::map<Double, Double> MSMetaDataOnDemand::getExposuresForTimes() const {
	return _getTimeToAggregateExposureMap(_getTimes(_ms), _getExposures(_ms));
}
*/

Matrix<Bool> MSMetaDataOnDemand::getUniqueBaselines() {
	if (! _uniqueBaselines.empty()) {
		return _uniqueBaselines;
	}
	std::tr1::shared_ptr<Vector<Int> > ant1, ant2;
	_getAntennas(ant1, ant2);
	Matrix<Bool> uBaselines = _getUniqueBaselines(*ant1, *ant2);
	if (_cacheUpdated(sizeof(Bool)*uBaselines.size())) {
		_uniqueBaselines = uBaselines;
	}
	return uBaselines;
}

Quantum<Vector<Double> > MSMetaDataOnDemand::getAntennaOffset(
	const String& name
) {
	vector<String> names(1);
	names[0] = name;
	return getAntennaOffset(getAntennaIDs(names)[0]);
}

Quantity MSMetaDataOnDemand::getEffectiveTotalExposureTime() {
	if (_exposureTime.getValue() > 0) {
		return _exposureTime;
	}
	std::tr1::shared_ptr<Vector<Double> > times = _getTimes();
	std::set<uInt> avgSpw, tdmSpw, fdmSpw, wvrSpw;
	vector<SpwProperties> spwInfo = _getSpwInfo(avgSpw, tdmSpw, fdmSpw, wvrSpw);
	std::map<Double, Double> timeToBWMap = _getTimeToTotalBWMap(
		*times, *_getDataDescIDs(), _getDataDescIDToSpwMap(), spwInfo
	);
	Quantity eTime = _getTotalExposureTime(
		*_ms, timeToBWMap, spwInfo, _getDataDescIDToSpwMap()
	);
	if (_cacheUpdated(10)) {
		_exposureTime = eTime;
	}
	return eTime;
}

void MSMetaDataOnDemand::_getUnflaggedRowStats(
	Double& nACRows, Double& nXCRows,
	std::tr1::shared_ptr<AOSFMapD>& scanNACRows,
	std::tr1::shared_ptr<AOSFMapD>& scanNXCRows,
	std::tr1::shared_ptr<std::map<Int, Double> >& fieldNACRows,
	std::tr1::shared_ptr<std::map<Int, Double> >& fieldNXCRows
) {
	// This method is responsible for setting _nUnflaggedACRows, _nUnflaggedXCRows,
	// _unflaggedFieldNACRows, _unflaggedFieldNXCRows, _unflaggedScanNACRows,
	// _unflaggedScanNXCRows
	if (_unflaggedFieldNACRows && ! _unflaggedFieldNACRows->empty()) {
		nACRows = _nUnflaggedACRows;
		nXCRows = _nUnflaggedXCRows;
		fieldNACRows = _unflaggedFieldNACRows;
		fieldNXCRows = _unflaggedFieldNXCRows;
		scanNACRows = _unflaggedScanNACRows;
		scanNXCRows = _unflaggedScanNXCRows;
		return;
	}
	AOSFMapD *myScanNACRows, *myScanNXCRows;
	std::map<Int, Double> *myFieldNACRows, *myFieldNXCRows;

	std::tr1::shared_ptr<Vector<Int> > ant1, ant2;
	_getAntennas(ant1, ant2);
	std::set<uInt> a, b, c, d;
	MSMetaData::_getUnflaggedRowStats(
		nACRows, nXCRows, myFieldNACRows,
		myFieldNXCRows, myScanNACRows, myScanNXCRows, *ant1,
		*ant2, _getFlagRows(*_ms), *_getDataDescIDs(),
		_getDataDescIDToSpwMap(),
		_getSpwInfo(a, b, c, d), *MSMetaData::_getFlags(*_ms), *_getFieldIDs(),
		*_getScans(), *_getObservationIDs(), *_getArrayIDs()
	);
	fieldNACRows.reset(myFieldNACRows);
	fieldNXCRows.reset(myFieldNXCRows);
	scanNACRows.reset(myScanNACRows);
	scanNXCRows.reset(myScanNXCRows);

	uInt mysize = fieldNACRows->size() + fieldNXCRows->size()
		+ scanNACRows->size() + scanNXCRows->size();
	mysize *= sizeof(Double);
	if (_cacheUpdated(mysize)) {
		_nUnflaggedACRows = nACRows;
		_nUnflaggedXCRows = nXCRows;
		_unflaggedFieldNACRows = fieldNACRows;
		_unflaggedFieldNXCRows = fieldNXCRows;
		_unflaggedScanNACRows = scanNACRows;
		_unflaggedScanNXCRows = scanNXCRows;
	}
}

vector<std::set<String> > MSMetaDataOnDemand::_getSpwToIntentsMap() {
	if (! _spwToIntentsMap.empty()) {
		return _spwToIntentsMap;
	}
	std::set<uInt> avgSpw, tdmSpw, fdmSpw, wvrSpw;
	vector<SpwProperties> spwInfo = _getSpwInfo(avgSpw, tdmSpw, fdmSpw, wvrSpw);
	std::set<String> emptySet;
	vector<SpwProperties>::const_iterator end = spwInfo.end();
	vector<std::set<String> > spwToIntentsMap;
	for (
		vector<SpwProperties>::const_iterator iter=spwInfo.begin();
		iter!=end; iter++
	) {
		spwToIntentsMap.push_back(emptySet);
	}
	vector<std::set<String> > stateToIntentsMap;
	std::set<String> uniqueIntents;
	_getStateToIntentsMap(stateToIntentsMap, uniqueIntents);
	if (uniqueIntents.size() == 0) {
		_spwToIntentsMap = spwToIntentsMap;
		return spwToIntentsMap;
	}
	//std::map<uInt, std::set<uInt> > checkedMap;
	std::tr1::shared_ptr<Vector<Int> > dataDescIDs = _getDataDescIDs();
	Vector<Int>::const_iterator curDDID = dataDescIDs->begin();
	Vector<Int>::const_iterator endDDID = dataDescIDs->end();
	std::tr1::shared_ptr<Vector<Int> > states = _getStateIDs();
	Vector<Int>::const_iterator curState = states->begin();
	std::map<Int, uInt> dataDescToSpwMap = _getDataDescIDToSpwMap();
	uInt mysize = 0;
	while (curDDID!=endDDID) {
		uInt spw = dataDescToSpwMap[*curDDID];
		std::set<String> intents = stateToIntentsMap[*curState];
		std::set<String>::const_iterator endIntent = intents.end();
		for (
			std::set<String>::const_iterator curIntent=intents.begin();
			curIntent!=endIntent; curIntent++
		) {
			mysize += curIntent->size();
		}
		spwToIntentsMap[spw].insert(intents.begin(), endIntent);
		//checkedMap[*curDDID].insert(curState);
		curDDID++;
		curState++;
	}
	if (_cacheUpdated(mysize)) {
		_spwToIntentsMap = spwToIntentsMap;
	}
	return spwToIntentsMap;
}

vector<std::set<String> > MSMetaDataOnDemand::_getFieldToIntentsMap() {
	if (! _fieldToIntentsMap.empty()) {
		return _fieldToIntentsMap;
	}
	std::set<String> emptySet;
	vector<std::set<String> > fieldToIntentsMap;
	fieldToIntentsMap.assign(nFields(), emptySet);
	std::set<String> uniqueIntents = getIntents();

	if (uniqueIntents.empty()) {
		_fieldToIntentsMap = fieldToIntentsMap;
		return fieldToIntentsMap;
	}
	std::set<String>::const_iterator end = uniqueIntents.end();
	uInt mysize = 0;
	for (
		std::set<String>::const_iterator iter=uniqueIntents.begin();
		iter!=end; iter++
	) {
		std::set<Int> fieldIDs = getFieldsForIntent(*iter);
		std::set<Int>::const_iterator fEnd = fieldIDs.end();
		for (
			std::set<Int>::const_iterator fiter=fieldIDs.begin();
			fiter!=fEnd; fiter++
		) {
			fieldToIntentsMap[*fiter].insert(*iter);
			mysize += iter->size();
		}
	}
	if (_cacheUpdated(mysize)) {
		_fieldToIntentsMap = fieldToIntentsMap;
	}
	return fieldToIntentsMap;
}

std::map<Int, uInt> MSMetaDataOnDemand::_getDataDescIDToSpwMap() {
	if (! _dataDescIDToSpwMap.empty()) {
		return _dataDescIDToSpwMap;
	}
	std::map<Int, uInt> dataDescToSpwMap = MSMetaData::_getDataDescIDToSpwMap(*_ms);
	uInt mysize = sizeof(Int) * dataDescToSpwMap.size();
	if (_cacheUpdated(mysize)) {
		_dataDescIDToSpwMap = dataDescToSpwMap;
	}
	return dataDescToSpwMap;
}

vector<MSMetaData::SpwProperties> MSMetaDataOnDemand::_getSpwInfo(
	std::set<uInt>& avgSpw, std::set<uInt>& tdmSpw,
	std::set<uInt>& fdmSpw, std::set<uInt>& wvrSpw
) {
	if (
		! _spwInfo.empty() && ! _avgSpw.empty() && ! _tdmSpw.empty()
		&& ! _fdmSpw.empty() && ! _wvrSpw.empty()
	) {
		avgSpw = _avgSpw;
		tdmSpw = _tdmSpw;
		fdmSpw = _fdmSpw;
		wvrSpw = _wvrSpw;
		return _spwInfo;
	}
	vector<SpwProperties> spwInfo = MSMetaData::_getSpwInfo(
		avgSpw, tdmSpw, fdmSpw, wvrSpw, *_ms
	);
	uInt mysize = sizeof(uInt)*(
			avgSpw.size() + tdmSpw.size() + fdmSpw.size() + wvrSpw.size()
		) + 2*sizeof(Int)*spwInfo.size()
		+ 2*sizeof(Double)*spwInfo.size();
	vector<SpwProperties>::const_iterator end = spwInfo.end();
	for (
		vector<SpwProperties>::const_iterator iter=spwInfo.begin();
		iter!=end; iter++
	) {
		mysize += 2*(sizeof(Double)*iter->nchans + 20);
		mysize += sizeof(Double)*iter->edgechans.size();
	}
	if (_cacheUpdated(mysize)) {
		_avgSpw = avgSpw;
		_tdmSpw = tdmSpw;
		_fdmSpw = fdmSpw;
		_wvrSpw = wvrSpw;
		_spwInfo = spwInfo;
	}
	return spwInfo;
}

void MSMetaDataOnDemand::_checkScan(const Int scan, const std::set<Int> uniqueScans) {
	if (uniqueScans.find(scan) == uniqueScans.end()) {
		throw AipsError(
			_ORIGIN + "Unknown scan number " + String::toString(scan)
		);
	}
}

void MSMetaDataOnDemand::_checkFieldID(const Int fieldID) {
	std::set<Int> uniqueFields = _getUniqueFiedIDs();
	if (uniqueFields.find(fieldID) == uniqueFields.end() ) {
		throw AipsError(
			_ORIGIN + "field ID ("
			+ String::toString(fieldID) + ") is not present in this dataset."
		);
	}
}

void MSMetaDataOnDemand::_checkFieldIDs(const vector<Int>& fieldIDs) {
	vector<Int>::const_iterator end = fieldIDs.end();
	for (
		vector<Int>::const_iterator f=fieldIDs.begin();
		f!=end; f++
	) {
		_checkFieldID(*f);
	}
}

std::set<Int> MSMetaDataOnDemand::_getUniqueFiedIDs() {
	if (_uniqueFieldIDs.empty()) {
		std::tr1::shared_ptr<Vector<Int> > allFieldIDs = _getFieldIDs();
		_uniqueFieldIDs.insert(allFieldIDs->begin(), allFieldIDs->end());
	}
	return _uniqueFieldIDs;
}


void MSMetaDataOnDemand::_checkStateID(const Int stateID) {
	if (_uniqueStateIDs.empty()) {
		std::tr1::shared_ptr<Vector<Int> > allStateIDs = _getStateIDs();
		_uniqueStateIDs.insert(allStateIDs->begin(), allStateIDs->end());
	}
	if (_uniqueStateIDs.find(stateID) == _uniqueStateIDs.end() ) {
		throw AipsError(
			_ORIGIN + "state ID ("
			+ String::toString(stateID) + ") is not present in this dataset."
		);
	}
}

/*
std::set<uInt> MSMetaDataOnDemand::_getUnique(const vector<uInt>& v) {
	std::set<uInt> ret;
	ret.insert(v.begin(), v.end());
	return ret;
}
*/

}

