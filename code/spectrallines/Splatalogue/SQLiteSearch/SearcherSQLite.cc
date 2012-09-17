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
#include "SearcherSQLite.h"


#include <spectrallines/Splatalogue/SQLiteSearch/DatabaseConnector.h>
#include <sqlite3.h>

#include <iostream>
#include <sstream>

using namespace std;

namespace casa {

const std::string SearcherSQLite::DB_NAME = "splat";
const std::string SearcherSQLite::SPECIES_ID_COLUMN = "species_id"; //Type 1
const std::string SearcherSQLite::FREQUENCY_COLUMN = "orderedfreq"; //Type 2
const std::string SearcherSQLite::SPECIES_COLUMN = "s_name"; //Type 3
const std::string SearcherSQLite::RECOMMENDED_COLUMN = "";
const std::string SearcherSQLite::RESOLVED_QNS_COLUMN = "resolved_QNs"; //Type 3
const std::string SearcherSQLite::INTENSITY_COLUMN = "intintensity"; //Type 5
const std::string SearcherSQLite::CHEMICAL_NAME_COLUMN = "chemical_name";


const std::string FILTER_KNOWN_AST_COLUMN = "";
const std::string FILTER_PLANET_COLUMN = "";
const std::string FILTER_HOTCORE_COLUMN = "";
const std::string FILTER_DIFFUSECLOUD_COLUMN = "";
const std::string FILTER_DARKCLOUD_COLUMN = "";
const std::string FILTER_COMET_COLUMN = "";
const std::string FILTER_EXTRAGALACTIC_COLUMN = "";
const std::string FILTER_AGB_PPN_PN_COLUMN = "";
const std::string FILTER_TOP20_COLUMN = "";

const std::string SearcherSQLite::AND = " AND ";
const std::string SearcherSQLite::OPEN_PAREN = "(";
const std::string SearcherSQLite::CLOSE_PAREN = ")";
const std::string SearcherSQLite::SINGLE_QUOTE = "'";
const std::string SearcherSQLite::COMMA = ", ";
const std::string SearcherSQLite::EQUALS = " = ";
const std::string SearcherSQLite::IN = " IN ";
const std::string SearcherSQLite::SELECT = "SELECT ";
const std::string SearcherSQLite::FROM = " FROM ";


SearcherSQLite::SearcherSQLite( const string& databasePath): rowLimit(500){
	db = DatabaseConnector::getDatabase(databasePath);
}

void SearcherSQLite::setSearchResultLimit( int limit ){
	rowLimit = limit;
}


//*********************************************************************
//                    Search Parameters
//*********************************************************************

void SearcherSQLite::setChemicalNames( const vector<string>& chemNames ){
	chemicalNames = chemNames;
}

void SearcherSQLite::setRecommendedOnly( bool recommended ){
	recommendedOnly = recommended;
}

void SearcherSQLite::setSpeciesNames( const vector<string>& species ){
	speciesNames = species;
}

void SearcherSQLite::setSearchRangeFrequency( double minVal, double maxVal ){
	minValueFreq = minVal;
	maxValueFreq = maxVal;
}

void SearcherSQLite::setAstroFilterTop20( bool filter ){
	filterTop20 = filter;
}
void SearcherSQLite::setAstroFilterPlanetaryAtmosphere( bool filter ){
	filterPlanetaryAtmosphere = filter;
}
void SearcherSQLite::setAstroFilterHotCores( bool filter ){
	filterHotCores = filter;
}
void SearcherSQLite::setAstroFilterDarkClouds( bool filter ){
	filterDarkClouds = filter;
}
void SearcherSQLite::setAstroFilterDiffuseClouds( bool filter ){
	filterDiffuseClouds = filter;
}
void SearcherSQLite::setAstroFilterComets( bool filter ){
	filterComets = filter;
}
void SearcherSQLite::setAstroFilterAgbPpnPn( bool filter ){
	filterAgbPpnPn = filter;
}
void SearcherSQLite::setAstroFilterExtragalactic( bool filter ){
	filterExtragalactic = filter;
}


//**********************************************************************
//                  Database Information
//**********************************************************************

string SearcherSQLite::tableInfo( string& errorMsg ) const{
	string query;
	query.append( SELECT );
	query.append( "*");
	query.append( FROM );
	query.append( DB_NAME );
	query.append(" LIMIT 1" );
	sqlite3_stmt* statement = NULL;
	bool successfulQuery = executeQuery( statement, query, errorMsg );
	string tableInfo;
	if ( successfulQuery ){
		//Returns one row for each column in the table
		int columnCount = sqlite3_column_count( statement );
		while( sqlite3_step( statement ) == SQLITE_ROW ){
			for ( int j = 0; j < columnCount; j++ ){
				const char* columnName = sqlite3_column_name( statement, j );
				int columnType = sqlite3_column_type( statement, j );
				string rowInfo( columnName );
				rowInfo.append( " ");
				rowInfo.append( numToString(columnType) );
				rowInfo.append( "\n");
				tableInfo.append( rowInfo );
			}
		}
	}
	else {
		errorMsg = sqlite3_errmsg( db );
		//cout << "Error obtaining table information "<<errorMsg<< endl;
	}
	return tableInfo;
}

string SearcherSQLite::getCreatedDate() const {
	return DatabaseConnector::getCreatedDate();
}
//************************************************************************
//                    Querying the Database
//************************************************************************



long SearcherSQLite::doSearchCount( string& errorMsg ){
	string query = prepareQuery( true, 0 );
	sqlite3_stmt* statement;
	long resultCount = 0;
	bool success =  executeQuery( statement, query, errorMsg );
	if ( success ){
		if( sqlite3_step( statement ) == SQLITE_ROW ){
			resultCount = sqlite3_column_int( statement, 0 );
		}
		else {
			cout << "Could not find a row with the count: "<<resultCount << endl;
		}
	}
	else {
		errorMsg = sqlite3_errmsg( db );
		//cout << "Error was " << errorMsg << endl;
	}
	sqlite3_finalize(statement);
	return resultCount;
}

vector<SplatResult> SearcherSQLite::doSearch( string& errorMsg, int offset ){
	string query = prepareQuery( false, offset );
	sqlite3_stmt* statement;
	vector<SplatResult> results;
	bool success =  executeQuery( statement, query, errorMsg );
	if ( success ){
		while( sqlite3_step( statement ) == SQLITE_ROW ){
			int speciesId = sqlite3_column_int( statement, SPECIES_ID_COL);
			string chemicalName = reinterpret_cast<const char*>(sqlite3_column_text( statement, CHEMICAL_NAME_COL ));
			string speciesName = reinterpret_cast<const char*>(sqlite3_column_text( statement, SPECIES_NAME_COL ));
			double frequency = sqlite3_column_double( statement, FREQUENCY_COL );
			double smu2 = sqlite3_column_double( statement, SMU2_COL );
			double el = sqlite3_column_double( statement, EL_COL );
			double eu = sqlite3_column_double( statement, EU_COL );
			string resolvedQNs = reinterpret_cast<const char*>(sqlite3_column_text( statement, RESOLVED_QNS_COL ));
			int intensity = sqlite3_column_int( statement, INTENSITY_COL );
			pair<double,string> emptyPair( -1, "");
			pair<double,string> freqPair( frequency, "");
			pair<double,string> elPair( el, "K" );
			pair<double,string> euPair( eu, "K" );
			pair<double,string> smu2Pair( smu2, "");
			SplatResult result( speciesId, speciesName, chemicalName, resolvedQNs, ""/*catalogueName*/, false,
						-1/*molecularType*/, freqPair, smu2Pair, elPair, euPair,
						-1/*loga*/, -1/*wavelength*/, intensity );

			results.push_back( result );
			//cout << result.toString() << endl;

		}
	}
	else {
		errorMsg = sqlite3_errmsg( db );
		//cout << "Error was " << errorMsg << endl;
	}
	sqlite3_finalize(statement);
	return results;
}

bool SearcherSQLite:: executeQuery( sqlite3_stmt*& statement, const string& query,
		string& errorMsg ) const {
	bool success = true;
	int status =  sqlite3_prepare( db, query.c_str(), -1, &statement, NULL);
	if ( status != SQLITE_OK ){
		success = false;
		errorMsg = sqlite3_errmsg( db );
	}
	return success;
}

string SearcherSQLite::prepareQuery( bool countOnly, int offset ) const {
	//bool includeRRLs = true;
	//bool onlyRRLs = false;

	std::string query;
	query.append( SELECT );
	if ( countOnly ){
		query.append( "Count()");
	}
	else {
		query.append( "*");
	}
	query.append( FROM );
	query.append( DB_NAME );
	query.append( " WHERE ");

	//Frequency
	query.append( OPEN_PAREN );
	query.append( FREQUENCY_COLUMN );
	query.append( " BETWEEN ");
	query.append( numToString( minValueFreq ) );
	query.append( AND );
	query.append( numToString( maxValueFreq ) );
	query.append( CLOSE_PAREN );

	//Species
	if (speciesNames.size() > 0) {
		query.append( AND );
		query.append( OPEN_PAREN );
		query.append( SPECIES_COLUMN );
		query.append( IN );
		query.append( OPEN_PAREN );
		for (unsigned int i=0; i<speciesNames.size(); i++) {
			query.append( SINGLE_QUOTE );
			query.append( speciesNames[i] );
			query.append( SINGLE_QUOTE );
			if (i != speciesNames.size() - 1) {
				query.append( ", ");
			}
		}
		query.append( CLOSE_PAREN );
		query.append( CLOSE_PAREN );
	}

	//TODO:  Recommended is not supported by the current database.
	/*if (recommendedOnly) {
		query.append( AND );
		query.append( OPEN_PAREN );
		query.append( RECOMMENDED_COLUMN );
		query.append( EQUALS );
		query.append( getTrue() );
		query.append( CLOSE_PAREN );

	}*/

	//Chemical Names
	if (chemicalNames.size() > 0) {
		query.append( AND );
		query.append( OPEN_PAREN );
		query.append( CHEMICAL_NAME_COLUMN );
		query.append( IN );
		query.append( OPEN_PAREN );
		for (unsigned int i=0; i<chemicalNames.size(); i++) {
			query.append( SINGLE_QUOTE );
			query.append( chemicalNames[i] );
			query.append( SINGLE_QUOTE );
			if (i != chemicalNames.size() - 1) {
				query.append(COMMA);
			}
		}
		query.append( CLOSE_PAREN );
		query.append( CLOSE_PAREN );
	}

	//*****************************************************************
	//TODO:  Put in query for astronomical filters
	//*****************************************************************

	/*	if (qns.size() > 0) {
			query << " AND (" << SplatalogueTable::QUANTUM_NUMBERS << " IN (";
			for (uInt i=0; i<qns.size(); i++) {
				query << "'" << qns[i] << "'";
				if (i != qns.size() - 1) {
					query << ", ";
				}
			}
			query << "))";
		}
		String rrlPortion = (includeRRLs || onlyRRLs)
			? "LINELIST = 'Recomb'"
			: "";
		ostringstream nonRRLPortion;
		if (! onlyRRLs) {
			nonRRLPortion << "(LINELIST != 'Recomb')";
			if (intensityLow < intensityHigh) {
				nonRRLPortion << " AND " << _getBetweenClause(
					SplatalogueTable::INTENSITY, intensityLow, intensityHigh
				);
			}
			if (smu2Low < smu2High) {
				nonRRLPortion << " AND " << _getBetweenClause(
					SplatalogueTable::SMU2, smu2Low, smu2High
				);
			}
			if (logaLow < logaHigh) {
				nonRRLPortion << " AND " << _getBetweenClause(
					SplatalogueTable::LOGA, logaLow, logaHigh
				);
			}
			if (elLow < elHigh) {
				nonRRLPortion << " AND " << _getBetweenClause(
					SplatalogueTable::EL, elLow, elHigh
				);
			}
			if (euLow < euHigh) {
				nonRRLPortion << " AND " << _getBetweenClause(
					SplatalogueTable::EU, euLow, euHigh
				);
			}
		}
		if (onlyRRLs) {
			query << " AND " << rrlPortion;
		}
		else if (includeRRLs) {
			query << " AND ((" << rrlPortion << ") OR (" << nonRRLPortion.str() << "))";
		}
		else {
			query << " AND " << nonRRLPortion.str();
		}*/
	if ( !countOnly ){
		query.append(" ORDER BY ");
		query.append(FREQUENCY_COLUMN);
		if ( rowLimit > 0 ){
			query.append( " LIMIT ");
			query.append( numToString(rowLimit) );
			if ( offset > 0 ){
				query.append( " OFFSET ");
				query.append( numToString(offset) );
			}
		}
	}
	return query;
}



//******************************************************************
//                       Utility
//******************************************************************
string SearcherSQLite::numToString( double number ) const {
	string result;
	ostringstream convert;
	convert << number;
	result = convert.str();
	return result;
}
std::string numToString( float val ){
	std::stringstream stream;
	std::string str;
	stream << val;
	str = stream.str();
	return str;
}

//TODO:: Check this, the boolean columns may be coded differently.
std::string SearcherSQLite::getTrue() const {
	return "1";
}


SearcherSQLite::~SearcherSQLite() {
	// TODO Auto-generated destructor stub
}

} /* namespace casa */
