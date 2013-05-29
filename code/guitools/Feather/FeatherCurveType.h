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

#ifndef FEATHERCURVETYPE_H_
#define FEATHERCURVETYPE_H_

namespace casa {

/**
 * An enumeration of the different curves that a user can ask to
 * be displayed on the plot.  This does not exactly match the enumeration
 * of data types in FeatherDataType because several different curves can
 * use the same set of data and other curves can be shown using derived
 * data.
 */

class FeatherCurveType {
public:

	//Note:  Order in this listing is important.
	//First, we list things which will have legend items in the slice plot.
	//Next, we list things in the slice plot that do not have legend items.
	//Finally, we list items in the scatter plot
	typedef enum CurveType {SUM_LOW_HIGH,  WEIGHT_LOW,
	    	WEIGHT_HIGH, LOW_ORIGINAL, LOW_WEIGHTED, LOW_CONVOLVED_HIGH,
	    	LOW_CONVOLVED_HIGH_WEIGHTED, LOW_CONVOLVED_DIRTY,
	    	LOW_CONVOLVED_DIRTY_WEIGHTED, HIGH_ORIGINAL, HIGH_WEIGHTED,
	    	HIGH_CONVOLVED_LOW, HIGH_CONVOLVED_LOW_WEIGHTED, DIRTY_ORIGINAL,
	    	DIRTY_WEIGHTED, DIRTY_CONVOLVED_LOW, DIRTY_CONVOLVED_LOW_WEIGHTED,
	    	//Items in slice plot not appearing in legend
	    	ZOOM, DISH_DIAMETER,
	    	//Scatter plot curves should follow SCATTER_LOW_HIGH in the listing
	    	SCATTER_LOW_HIGH,X_Y,CURVES_END};
private:
	FeatherCurveType();
	virtual ~FeatherCurveType();
};

} /* namespace casa */
#endif /* FEATHERTYPE_H_ */
