//# MSContinuumSubtractor.h: Fit & subtract continuum from spectral line data
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
#ifndef SPLAT_SPLATALOGUETABLE_H
#define SPLAT_SPLATALOGUETABLE_H

#include <casa/aips.h>

#include <casa/Arrays/Vector.h>
#include <tables/Tables/Table.h>

namespace casa {

// <summary>Representation of a Splatalogue Table</summary>
// <use visibility=export>
// 
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
// 

// <etymology>
// Splatalogue Table
// </etymology>
//
// <synopsis>
// A table containing Splatalogue fields and metadata which holds Splatalogue data.
// </synopsis>
//
// <example>
// <srcBlock>

// </srcBlock>
// </example>
//
// <motivation>
//
// </motivation>
//
// <todo asof="">
// </todo>
 
class SplatalogueTable : public Table {
public:

	const static String SPECIES;
	const static String RECOMMENDED;
	const static String CHEMICAL_NAME;
	const static String FREQUENCY;
	const static String QUANTUM_NUMBERS;
	const static String INTENSITY;
	const static String SMU2;
	const static String LOGA;
	const static String EU;
	const static String EL;
	const static String LINELIST;
	const static String ISSPLAT;

	SplatalogueTable(
		SetupNewTable& snt, uInt nrow,
		const String& freqUnit, const String& smu2Unit,
		const String & euUnit, const String& elUnit
	);

	// open an existing splatalogue table. Checks are done to ensure the table
	// is indeed a SplatalogueTable and not just a generic table. If

	SplatalogueTable(const String& tablename);

	// copy semantics. Tests are done to insure the input table is a splatalogue table.
	SplatalogueTable(const Table& table);

	String getFrequencyUnit() const;

	// Pretty print table contents to a String.
	String list() const;

private:
	String _freqUnit, _smu2Unit, _euUnit, _elUnit;

	SplatalogueTable();
	//void _defineTable(const String& tablename, const uInt nrow);
	void _construct(const Bool setup);

	void _addKeywords();


};


} //# NAMESPACE CASA - END

#endif
