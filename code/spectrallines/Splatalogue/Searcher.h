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
#ifndef SEARCHER_H_
#define SEARCHER_H_

#include <spectrallines/Splatalogue/SplatResult.h>
#include <string>
#include <vector>

namespace casa {

//Interface needed to support molecular line searching and identification.

class Searcher {
public:
	Searcher();

	//Set all search parameters back to their defaults.
	virtual void reset() = 0;

	//Search Parameters
	virtual void setChemicalNames( const vector<string>& chemNames ) = 0;
	virtual void setSpeciesNames( const vector<string>& speciesNames ) = 0;
	virtual void setSearchRangeFrequency( double minValue, double maxValue ) = 0;

	//Filters
	virtual void setAstroFilterTop20( bool filter = true) = 0;
	virtual void setAstroFilterPlanetaryAtmosphere( bool filter = true ) = 0;
	virtual void setAstroFilterHotCores(bool filter = true) = 0;
	virtual void setAstroFilterDarkClouds(bool filter = true) = 0;
	virtual void setAstroFilterDiffuseClouds( bool filter = true) = 0;
	virtual void setAstroFilterComets(bool filter = true) = 0;
	virtual void setAstroFilterAgbPpnPn(bool filter = true) = 0;
	virtual void setAstroFilterExtragalactic(bool filter = true) = 0;

	//Support for scolling
	virtual void setSearchResultLimit( int limit ) = 0;

	//Doing the search
	virtual vector<SplatResult> doSearch(string& errorMsg, int offset ) = 0;
	virtual long doSearchCount( string& errorMsg ) = 0;

	//Information about the database
	virtual string getCreatedDate() const = 0;
	virtual string tableInfo( const string& tableName, string& errorMessage ) const= 0;

	virtual ~Searcher();
};

} /* namespace casa */
#endif /* SEARCHER_H_ */
