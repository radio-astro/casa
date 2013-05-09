//# Copyright (C) 2005
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

#ifndef SEARCHMOLECULESRESULTDISPLAYER_H_
#define SEARCHMOLECULESRESULTDISPLAYER_H_

#include <spectrallines/Splatalogue/SplatResult.h>
#include <vector>
namespace casa {
	/**
	 * Interface provides decoupling between classes that initialize a search
	 * such as SearchMoleculesWidget and classes that display the results of
	 * a search such as SearchMoleculesResultsWidget.
	 */

	class SearchMoleculesResultDisplayer {
	public:
		SearchMoleculesResultDisplayer();
		/**
		 * The parameters offset and count determine where the
		 * passed in results fit in the overall search.  The 'offset'
		 * parameters in the starting index and countTotal is the total
		 * number of search results available.  This may be smaller than
		 * the number of results returned if the countTotal exceeds the
		 * limit on the number of search results obtained at one time.
		 */
		virtual void displaySearchResults( const vector<SplatResult>& results,
		                                   int offset, int countTotal) = 0;
		virtual ~SearchMoleculesResultDisplayer();
	};

} /* namespace casa */
#endif /* SEARCHMOLECULESRESULTDISPLAYER_H_ */
