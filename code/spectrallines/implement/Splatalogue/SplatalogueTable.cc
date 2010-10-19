//# MSContinuumSubtractor.cc:  Subtract continuum from spectral line data
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
//# $Id$
//#

#include <spectrallines/Splatalogue/SplatalogueTable.h>

#include <casa/Exceptions/Error.h>
#include <casa/Quanta/MVTime.h>
#include <tables/Tables/ExprNode.h>

#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/ScalarColumn.h>
#include <iomanip>

namespace casa {

const String SplatalogueTable::SPECIES = "SPECIES";
const String SplatalogueTable::RECOMMENDED = "RECOMMENDED";
const String SplatalogueTable::CHEMICAL_NAME = "CHEMICAL_NAME";
const String SplatalogueTable::FREQUENCY = "FREQUENCY";
const String SplatalogueTable::QUANTUM_NUMBERS = "QUANTUM_NUMBERS";
const String SplatalogueTable::INTENSITY = "INTENSITY";
const String SplatalogueTable::SMU2 = "SMU2";
const String SplatalogueTable::LOGA = "LOGA";
const String SplatalogueTable::EL = "EL";
const String SplatalogueTable::EU = "EU";
const String SplatalogueTable::LINELIST = "LINELIST";
const String SplatalogueTable::ISSPLAT = "isSplat";

SplatalogueTable::SplatalogueTable(
	SetupNewTable& snt, uInt nrow,
	const String& freqUnit, const String& smu2Unit,
	const String& elUnit, const String& euUnit  
) : Table(snt, nrow), _freqUnit(freqUnit), _smu2Unit(smu2Unit),
	 _elUnit(elUnit), _euUnit(euUnit) {
	_construct(True);
}

SplatalogueTable::SplatalogueTable(
	const String& tablename
) : Table(tablename) {
	_construct(False);
}

SplatalogueTable::SplatalogueTable(
	const Table& table
) : Table(table) {
	_construct(False);
}

String SplatalogueTable::getFrequencyUnit() const {
	return keywordSet().asString("FreqUnit");
}

String SplatalogueTable::list() const {
	ROScalarColumn<String> species(*this, SPECIES);
	ROScalarColumn<Bool> recommended(*this, RECOMMENDED);
	ROScalarColumn<String> chemName(*this, CHEMICAL_NAME);
	ROScalarColumn<Double> freq(*this, FREQUENCY);
	ROScalarColumn<String> qns(*this, QUANTUM_NUMBERS);
	ROScalarColumn<Float> intensity(*this, INTENSITY);
	ROScalarColumn<Float> smu2(*this, SMU2);
	ROScalarColumn<Float> logA(*this, LOGA);
	ROScalarColumn<Float> el(*this, EL);
	ROScalarColumn<Float> eu(*this, EU);
	ROScalarColumn<String> linelist(*this, LINELIST);

	char cspecies[15], crec[3], cchemName[21], cfreq[12], cqns[21],
		cintensity[10], csmu2[10], clogA[10], cel[10], ceu[10],
		clinelist[11];
	ostringstream os;
	String rec;
	os << SPECIES << "       " << RECOMMENDED << "        "
		<< CHEMICAL_NAME << "   " << FREQUENCY << "     "
		<< QUANTUM_NUMBERS << "  " << INTENSITY << "      "
		<< SMU2 << "      " << LOGA << "        "
		<< EL  << "        " << EU << "  " << LINELIST << endl;
    for (uInt i=0; i<nrow(); i++) {
    	sprintf(cspecies, "%-14.14s", species.asString(i).chars());
    	rec = recommended.asBool(i) ? "*" : " ";
    	sprintf(crec, "%10s", rec.chars());
    	sprintf(cchemName, "%-20.20s", chemName.asString(i).chars());
    	sprintf(cfreq, "%11.6f", freq.asdouble(i));
    	sprintf(cqns, "%-20.20s", qns.asString(i).chars());
    	sprintf(cintensity, "%9.5f", intensity.asfloat(i));
    	sprintf(csmu2, "%9.5f", smu2.asfloat(i));
    	sprintf(clogA, "%9.5f", logA.asfloat(i));
    	sprintf(cel, "%9.5f", el.asfloat(i));
    	sprintf(ceu, "%9.5f", eu.asfloat(i));
    	sprintf(clinelist, "%-10.10s", linelist.asString(i).chars());

    	os << cspecies << " " << crec << " " << cchemName
    		<< " " << cfreq << " " << cqns << " " << cintensity
    		<< " " << csmu2 << " " << clogA << " " << cel
    		<< " " << ceu << "  " << clinelist << endl;
	}
	return os.str();
}

void SplatalogueTable::_construct(const Bool setup) {
	if (
		! setup
		&& (
			! keywordSet().isDefined(ISSPLAT)
			|| ! keywordSet().asBool(ISSPLAT)
		)
	) {
		throw AipsError("Table is not a splatalogue table");
	}
	Vector<String> colNames = tableDesc().columnNames();
	Vector<String> reqColNames(11);
	if (colNames.size() != reqColNames.size()) {
		throw AipsError("Table does not have the required number of columns to be a Splatalogue table");
	}

	reqColNames[0] = SPECIES;
	reqColNames[1] = RECOMMENDED;
	reqColNames[2] = CHEMICAL_NAME;
	reqColNames[3] = FREQUENCY;
	reqColNames[4] = QUANTUM_NUMBERS;
	reqColNames[5] = INTENSITY;
	reqColNames[6] = SMU2;
	reqColNames[7] = LOGA;
	reqColNames[8] = EL;
	reqColNames[9] = EU;
	reqColNames[10] = LINELIST;
	for (
		Vector<String>::const_iterator riter=reqColNames.begin();
			riter!=reqColNames.end(); riter++
	) {
		Bool found = False;
		for (
			Vector<String>::const_iterator iter=colNames.begin();
				iter!=colNames.end(); iter++
			) {
			if (*iter == *riter) {
				found = True;
				break;
			}
		}
		if (! found) {
			throw AipsError("Column " + *riter + " does not exist in the input table.");
		}
	}
	// TODO also check column data types
	if (setup) {
		Bool writable = isWritable();
		if (! writable) {
			reopenRW();
		}
		_addKeywords();
	}
}


void SplatalogueTable::_addKeywords() {
	if (_freqUnit.empty()) {
		_freqUnit = "GHz";
	}
	ScalarColumn<Double> freq(*this, FREQUENCY);
	freq.rwKeywordSet().define("Unit", _freqUnit);
	ScalarColumn<Float> smu2(*this, SMU2);
	smu2.rwKeywordSet().define("Unit", _smu2Unit);
	ScalarColumn<Float> el(*this, EL);
	el.rwKeywordSet().define("Unit", _elUnit);
	ScalarColumn<Float> eu(*this, EU);
	eu.rwKeywordSet().define("Unit", _euUnit);
	rwKeywordSet().define(ISSPLAT, True);
	Time now;
	MVTime mv(now);
	mv.setFormat(MVTime::YMD, 3);
	String timeString = mv.string();
	timeString = String(timeString.before(timeString.size() - 1));
	// These are required by the standard measures loading code.
	rwKeywordSet().define("VS_CREATE", timeString);
	rwKeywordSet().define("VS_DATE", timeString);
	rwKeywordSet().define("VS_VERSION", timeString);
	rwKeywordSet().define("VS_TYPE", "List of spectral lines in Splatalogue format");
	tableInfo().setType("IERS");
}

} //# NAMESPACE CASA - END

