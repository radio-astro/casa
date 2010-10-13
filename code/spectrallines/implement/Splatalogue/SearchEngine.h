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
#ifndef SPLAT_SEARCHENGINE_H
#define SPLAT_SEARCHENGINE_H

#include <casa/aips.h>

#include <casa/Arrays/Vector.h>
#include <casa/BasicSL/String.h>
#include <casa/Logging/LogIO.h>
#include <spectrallines/Splatalogue/SplatalogueTable.h>

namespace casa {

// <summary>Performs a query on a splatalogue spectral line table</summary>
// <use visibility=export>
// 
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
// 

// <etymology>
// Searches a splatalogue table.
// </etymology>
//
// <synopsis>
// It is a requirement that users be able to perform searches on spectral
// line tables they import from Splatalogue.
// </synopsis>
//
// <example>
// <srcBlock>
// SearchEngine searcher(...);
// SplatalogueTable *resTable = searcher.search();
// </srcBlock>
// </example>
//
// <motivation>
// A class to for splatalogue spectral line searches.
// </motivation>
//
// <todo asof="">
// </todo>
 

class SearchEngine
{
public:
	// <src>table</src> Input spectral line table to search.
	// <src>list</src> List result set of search to logger (and optional logfile)?
	// <src>logfile</src> Logfile to list results to. Only used if list=True. If empty, no logfile is created.
	// <src>append</src> Append results to logfile (True) or overwrite logfile (False) if it exists? Only used if list=True and logfile not empty.
	SearchEngine(
		const SplatalogueTable* const table, const Bool list,
		const String& logfile, const Bool append
	);

	//destuctor
	~SearchEngine();

	// Search the table. It is the caller's responsibility to delete the returned pointer.
	SplatalogueTable* search(
		const String& resultsTableName, const Double freqLow, const Double freqHigh,
		const Vector<String>& species, const Bool recommendedOnly,
		const Vector<String>& chemNames, const Vector<String>& qns,
		const Double intensityLow, const Double intensityHigh,
		const Double smu2Low, const Double smu2High,
		const Double logaLow, const Double logaHigh,
		const Double elLow, const Double elHigh,
		const Double euLow, const Double euHigh,
		const Bool includeRRLs, const Bool onlyRRLs
	) const;

	// Get the unique species in the table.
	Vector<String> uniqueSpecies() const;

	// Get the unique chemical names in the table.
	Vector<String> uniqueChemicalNames() const;

private:
	LogIO *_log;
	const SplatalogueTable *_table;
	String _logfile;
	const Bool _list, _append;

	SearchEngine();

	String _getBetweenClause(
		const String& col, const Double low, const Double high
	) const;

	// If the query table is currently not on disk, this method will temporarily
	// write a copy of it to disk and query it, and then remove the copy from disk.
	// The method returns the query results table.
	Table _runQuery(const String& query) const;

	void _logIt(const String& logString) const;
};


} //# NAMESPACE CASA - END

#endif
