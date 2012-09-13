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
#ifndef SEARCHERSQLITE_H_
#define SEARCHERSQLITE_H_

#include <spectrallines/Splatalogue/Searcher.h>
#include <spectrallines/Splatalogue/SplatResult.h>

class sqlite3;
class sqlite3_stmt;

using namespace std;
namespace casa {

/**
 * Searches a local sqlite database for molecular lines meeting the specified
 * search criteria.
 */

class SearcherSQLite : public Searcher{
public:
	SearcherSQLite( const string& databasePath);

	/**
	 * Returns a string containing (columnName, columnType) information
	 * for all the columns in the table.
	 */
	string tableInfo( string& errorMsg ) const;

	/**
	 * Returns the date the database file was installed.
	 */
	string getCreatedDate() const;

	//Search Paramaters
	virtual void setRecommendedOnly( bool recomended );
	virtual void setChemicalNames( const vector<string>& chemNames );
	virtual void setSpeciesNames( const vector<string>& speciesNames );

	//virtual void setResultFile( const string& name );
	virtual void setSearchRangeFrequency( double minValue, double maxValue );

	//Astronomical Filters
	virtual void setAstroFilterTop20( bool filter = true );
	virtual void setAstroFilterPlanetaryAtmosphere( bool filter = true );
	virtual void setAstroFilterHotCores( bool filter = true );
	virtual void setAstroFilterDarkClouds( bool filter = true );
	virtual void setAstroFilterDiffuseClouds( bool filter = true );
	virtual void setAstroFilterComets( bool filter = true );
	virtual void setAstroFilterAgbPpnPn( bool filter = true );
	virtual void setAstroFilterExtragalactic( bool filter = true );

	//Performing the Search
	/**
	 * The offset is used when there is a limit on the number of rows
	 * that will be returned from the database.  In the case that the
	 * number of rows that meet the search criteria exceeds the search
	 * limit, the offset indicates the starting index of the rows that
	 * are returned.
	 */
	virtual vector<SplatResult> doSearch( string& errorMsg, int offset );
	virtual long doSearchCount( string& errorMsg );
	/**
	 * Sets the maximum number of rows that will be returned as a result
	 * of a database search.  Setting this value to a negative or zero
	 * value means that there will be no limit on the number of search results
	 * from the database.
	 */
	virtual void setSearchResultLimit( int limit );
	virtual ~SearcherSQLite();

private:
	bool executeQuery( sqlite3_stmt*& statement, const string& query,
		string& errorMsg ) const;
	/*
	 * Constructs the SQL for a SELECT query based on the search parameters
	 * that have been previously set.  If 'countOnly' is set to true, the
	 * query will return the number of rows matching the search criteria;
	 * otherwise, it will all columns in the rows matching the search criteria.
	 * The 'offset' parameter indicates the starting index for the first row
	 * matching the search criteria.
	 */
	string prepareQuery( bool countOnly, int offset ) const;
	std::string getTrue() const;
	string numToString( double number ) const;

	//Set-up
	sqlite3* db;

	//Search parameters
	double minValueFreq;
	double maxValueFreq;
	bool recommendedOnly;
	std::vector<std::string> speciesNames;
	std::vector<std::string> chemicalNames;

	bool filterTop20;
	bool filterPlanetaryAtmosphere;
	bool filterHotCores;
	bool filterDarkClouds;
	bool filterDiffuseClouds;
	bool filterComets;
	bool filterAgbPpnPn;
	bool filterExtragalactic;

	//Database Name
	const static std::string DB_NAME;

	//Table columns
	const static std::string FREQUENCY_COLUMN;
	const static std::string SPECIES_ID_COLUMN;
	const static std::string SPECIES_COLUMN;
	const static std::string RECOMMENDED_COLUMN;
	const static std::string CHEMICAL_NAME_COLUMN;
	const static std::string FILTER_KNOWN_AST_COLUMN;
	const static std::string FILTER_PLANET_COLUMN;
	const static std::string FILTER_HOTCORE_COLUMN;
	const static std::string FILTER_DIFFUSECLOUD_COLUMN;
	const static std::string FILTER_DARKCLOUD_COLUMN;
	const static std::string FILTER_COMET_COLUMN;
	const static std::string FILTER_EXTRAGALACTIC_COLUMN;
	const static std::string FILTER_AGB_PPN_PN_COLUMN;
	const static std::string FILTER_TOP20_COLUMN;
	const static std::string INTENSITY_COLUMN;
	const static std::string RESOLVED_QNS_COLUMN;
	enum TableColumns { SPECIES_ID_COL, SPECIES_NAME_COL, CHEMICAL_NAME_COL,
		FREQUENCY_COL, RESOLVED_QNS_COL, INTENSITY_COL, SMU2_COL, EL_COL,
		EU_COL, END_COL };

	//SQL Constants
	const static std::string FROM;
	const static std::string SELECT;
	const static std::string AND;
	const static std::string OPEN_PAREN;
	const static std::string CLOSE_PAREN;
	const static std::string SINGLE_QUOTE;
	const static std::string COMMA;
	const static std::string EQUALS;
	const static std::string IN;

	//Limiting the number of rows returned by a search.
	int rowLimit;
};

} /* namespace casa */
#endif /* SEARCHLOCAL_H_ */
