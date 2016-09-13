//# Copyright (C) 2008
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

#ifndef AXISLISTENER_H_
#define AXISLISTENER_H_

#include <casaqt/QwtPlotter/QPOptions.h>
#include <casa/BasicSL/String.h>

namespace casa {

/**
 * Interface implemented by classes wanting to receive notice when a plot
 * property changes.
 */

class AxisListener {
public:
	/**
	 * Set a new plot axis label.
	 * @param axis the axis where the label change is ocurring.
	 * @param title the new label for the axis.
	 */
	virtual void setAxisLabel(PlotAxis axis, const String& title) = 0;
	/**
	 * New format for displaying dates.
	 * @param dateFormat the new format to use for displaying dates.
	 */
	virtual void setDateFormat(const String& dateFormat) = 0;
	/**
	 * New format for displaying dates based off a given start position.
	 * @param dateFormat the new format for displaying relative dates.
	 */
	virtual void setRelativeDateFormat(const String& dateFormat) = 0;
	/**
	 * Set a new scale for the plot axis (logarithmic, etc).
	 * @param axis, the PlotAxis to which the scale applies.
	 * @param scale, the new scale (linear, logarithmic, etc).
	 */
	virtual void setAxisScale(PlotAxis axis, PlotAxisScale scale) = 0;
	/**
	 * Set a starting value for an axis using a relative scale.
	 * @param axis, the PlotAxis to which the base scale applies.
	 * @param on, whether it should be used.
	 * @param value, the starting value for the relative scale.
	 */
	virtual void setAxisReferenceValue(PlotAxis axis, bool on, double value) = 0;
	/**
	 * Set a font for use on an axis.
	 * @param axis, the PlotAxis to which the font applies.
	 * @param font, the font to use when drawing tick labels and axis labels.
	 */
	virtual void setAxisFont(PlotAxis axis, const PlotFont& font) = 0;
	virtual ~AxisListener(){}
	AxisListener(){}
};

}
#endif /* AXISLISTENER_H_ */
