#include <iostream>

#include <casa/Containers/RecordField.h>
#include <casa/Exceptions/Error.h>
#include <casa/Logging/LogIO.h>
#include <measures/Measures/MeasIERS.h>
#include <spectrallines/Splatalogue/ListConverter.h>
#include <spectrallines/Splatalogue/SearchEngine.h>
#include <tables/Tables/TableRow.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/TableProxy.h>

#include <spectralline_cmpt.h>

using namespace std;
using namespace casa;

namespace casac {

spectralline::spectralline() : _log(new LogIO), _table(0) {}

spectralline::spectralline(SplatalogueTable* table) : _log(new LogIO), _table(table) {}

spectralline::~spectralline() {
	delete _log;
	delete _table;
}

casac::spectralline* spectralline::splattotable(const vector<string>& filenames, const string& tablename) {
	try {
		if (tablename.empty()) {
			throw AipsError("Output table name cannot be blank.");
		}
		Vector<String> files(filenames.size());
		for (uInt i=0; i<filenames.size(); i++) {
			files[i] = String(filenames[i]);
		}
		ListConverter converter(files, tablename);
		SplatalogueTable *converted = converter.load();
                casac:spectralline* rstat =new spectralline(converted);
                if(!rstat)
			throw AipsError("Unable to create table"+tablename);
		return rstat;
	}
	catch (AipsError x) {
		*_log << LogIO::SEVERE << "Exception Reports: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
		return 0;
	}
}

bool spectralline::open(const string& tablename) {
	try {
		if(_table) {
			close();
		}
		if (tablename.empty()) {
			// open the default table
		    Table t;
		    ROTableRow row;
		    TableRecord kws;
		    String rfn[1] = {"FREQUENCY"};
		    RORecordFieldPtr<Double> rfp[1];
		    Double dt;
		    String vs;
		    if (
		    	MeasIERS::getTable(
		    		t, kws, row, rfp, vs, dt, 1, rfn, "SplatDefault.tbl",
					"splat.line.directory", "spectralLines"
				)
		    ) {
				_table = new SplatalogueTable(t.tableName());
		    }
		    else {
		    	*_log << "Error opening default table" << LogIO::EXCEPTION;
		    }
		}
		else {
			_table = new SplatalogueTable(tablename);
		}
		return true;
	}
	catch (AipsError x) {
		*_log << LogIO::SEVERE << "Exception Reports: " << x.getMesg() << LogIO::POST;
		return false;
	}
}

bool spectralline::close() {
	try {
		if (_table) {
			_table->relinquishAutoLocks(True);
			_table->unlock();
			delete _table;
			_table = 0;
		}
		return true;
	} catch (AipsError x) {
		*_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		return false;
	}
}

bool spectralline::done() {
	try {
		return close();
	} catch (AipsError x) {
		*_log << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		return false;
	}
}

spectralline* spectralline::search(
	const string& outfile, const vector<double>& freqRange,
	const variant& species, const bool recommendedOnly,
	const variant& chemNames, const vector<string>& qns,
	const vector<double>& intensityRange,
	const vector<double>& smu2Range, const vector<double>& logaRange,
	const vector<double>& elRange, const vector<double>& euRange, 
	const bool includeRRLs, const bool onlyRRLs,
	const bool verbose, const string& logfile, const bool append
) {
	std::auto_ptr<spectralline> tool;
	std::auto_ptr<SplatalogueTable> table;

	try {
		if (_detached()) {
			return 0;
		}
		*_log << LogOrigin("spectralline", __FUNCTION__);
		if (freqRange.size() != 2) {
			*_log << "freqrange must contain exactly 2 values." << LogIO::EXCEPTION;
		}
		double intensityLow, intensityHigh;
		_checkLowHigh(
			intensityLow, intensityHigh, intensityRange, "intensity"
		);
		double smu2Low, smu2High;
		_checkLowHigh(
			smu2Low, smu2High, smu2Range, "smu2"
		);
		double logaLow, logaHigh;
		_checkLowHigh(
			logaLow, logaHigh, logaRange, "loga"
		);
		double elLow, elHigh;
		_checkLowHigh(
			elLow, elHigh, elRange, "el"
		);
		double euLow, euHigh;
		_checkLowHigh(
			euLow, euHigh, euRange, "eu"
		);

		Vector<String> mySpecies;
		String sspecies = toCasaString(species);
	    sepCommaToVectorStrings(mySpecies, sspecies);
	    if (mySpecies.size() == 1) {
	    	mySpecies[0].trim();
	    	if (mySpecies[0].empty()) {
	    		mySpecies.resize(0);
	    	}
	    }
		Vector<String> myChemNames;
		String sChemNames = toCasaString(chemNames);
	    sepCommaToVectorStrings(myChemNames, sChemNames);

	    if (myChemNames.size() == 1) {
	    	myChemNames[0].trim();
	    	if (myChemNames[0].empty()) {
	    		myChemNames.resize(0);
	    	}
	    }
		Vector<String> myQNs(0);
		if (qns.size() != 1 || ! qns[0].empty()) {
			myQNs.resize(qns.size());
			for (uInt i=0; i<qns.size(); i++) {
				myQNs[i] = qns[i];
			}
		}

		SearchEngine engine(_table, verbose, logfile, append);
		table.reset(
			engine.search(
				outfile, freqRange[0], freqRange[1], mySpecies,
				recommendedOnly, myChemNames, myQNs, intensityLow,
				intensityHigh, smu2Low, smu2High, logaLow, logaHigh,
				elLow, elHigh, euLow, euHigh, includeRRLs, onlyRRLs
			)
		);
		*_log << LogIO::NORMAL << "Search found " << table->nrow() << " spectral lines" << LogIO::POST;
		tool.reset(new spectralline(table.get()));
		table.release();
	}
	catch (const AipsError& x) {
		if (tool.get()) {
			tool->close();
		}
		if (table.get()) {
			table->relinquishAutoLocks(True);
			table->unlock();
		}
		*_log << LogIO::SEVERE << "Exception Reports: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
	return tool.release();
}

void spectralline::list() {
	try {
		if (_detached()) {
			return;
		}
		if (_table->nrow() > 10000) {
			*_log << LogIO::WARN << "Table contains " << _table->nrow()
				<< " spectral lines and so may take quite some time to list"
				<< LogIO::POST;
		}
		*_log << LogIO::NORMAL << _table->list() << LogIO::POST;
	}
	catch (AipsError x) {
		*_log << LogIO::SEVERE << "Exception Reports: " << x.getMesg() << LogIO::POST;
	}
}

record* spectralline::torecord() {
	try {
		if (_detached()) {
			return 0;
		}
		if (_table->nrow() > 10000) {
			*_log << LogIO::WARN << "Table contains " << _table->nrow()
				<< " spectral lines and so may take quite some time to process"
				<< LogIO::POST;
		}
		return fromRecord(_table->toRecord());
	}
	catch (AipsError x) {
		*_log << LogIO::SEVERE << "Exception Reports: " << x.getMesg() << LogIO::POST;
		return 0;
	}
}


record* spectralline::species(const bool verbose, const string& logfile, const bool append) {
	try {
		if (_detached()) {
			return 0;
		}
		SearchEngine engine(_table, verbose, logfile, append);
		Vector<String> species = engine.uniqueSpecies();
		*_log << LogIO::NORMAL << species << LogIO::POST;
		Record ret;
		ret.define("return", species);
		return fromRecord(ret);
	}
	catch (AipsError x) {
		*_log << LogIO::SEVERE << "Exception Reports: " << x.getMesg() << LogIO::POST;
		return 0;
	}
}

record* spectralline::chemnames(const bool verbose, const string& logfile, const bool append) {
	try {
		if (_detached()) {
			return 0;
		}
		SearchEngine engine(_table, verbose, logfile, append);
		Vector<String> chemNames = engine.uniqueChemicalNames();
		*_log << LogIO::NORMAL << chemNames << LogIO::POST;
		Record ret;
		ret.define("return", chemNames);
		return fromRecord(ret);
	}
	catch (AipsError x) {
		*_log << LogIO::SEVERE << "Exception Reports: " << x.getMesg() << LogIO::POST;
		return 0;
	}
}

int spectralline::nrows() {
	if (_detached()) {
		return 0;
	}
	return _table->nrow();
}

bool spectralline::_detached() const {
	bool detached = false;
	if (_table == 0) {
		*_log << LogOrigin("spectralline", __FUNCTION__);
	    *_log << LogIO::SEVERE
		    << "Tool has not been opened with a spectral line table" << endl
		    << "Call eg sl.open('tablename') to (re)attach." << LogIO::POST;
	    detached = true;
	}
	return detached;
}

void spectralline::_checkLowHigh(
	double& low, double& high, const vector<double> pair, const string label
) const {
	vector<double> copy = pair;
	if (copy.size() == 1 && copy[0] < 0) {
		// default
		copy.clear();
	}
	if (copy.size() != 0 && copy.size() != 2) {
		*_log << label << " must contain exactly 0 or 2 values." << LogIO::EXCEPTION;
	}
    low = (copy.size() == 2) ? copy[0] : -1;
    high = (copy.size() == 2) ? copy[1] : -1;
}

};
