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

#include <casa/BasicSL/String.h>
#include <casa/Arrays/Vector.h>
#include <casa/Containers/Record.h>

namespace casa {

//Interface needed to support molecular line searching and identification.

class Searcher {
public:
	Searcher();
	virtual void setChemicalNames( const Vector<String>& chemNames ) = 0;
	virtual void setSpeciesNames( const Vector<String>& speciesNames ) = 0;
	virtual void setDatabasePath( const String& databasePath ) = 0;
	virtual void setResultFile( const String& fileName ) = 0;
	virtual void setSearchRangeFrequency( double minValue, double maxValue ) = 0;
	virtual Record doSearch(String& errorMsg) = 0;

	virtual void setAstroFilterTop20() = 0;
	virtual void setAstroFilterPlanetaryAtmosphere() = 0;
	virtual void setAstroFilterHotCores() = 0;
	virtual void setAstroFilterDarkClouds() = 0;
	virtual void setAstroFilterDiffuseClouds( ) = 0;
	virtual void setAstroFilterComets() = 0;
	virtual void setAstroFilterAgbPpnPn() = 0;
	virtual void setAstroFilterExtragalactic() = 0;
	virtual ~Searcher();
};

} /* namespace casa */
#endif /* SEARCHER_H_ */
