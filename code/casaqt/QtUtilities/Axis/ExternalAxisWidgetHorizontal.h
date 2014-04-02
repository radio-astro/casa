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

#ifndef EXTERNALAXISWIDGETHORIZONTAL_H_
#define EXTERNALAXISWIDGETHORIZONTAL_H_

#include <casaqt/QtUtilities/Axis/ExternalAxisWidget.h>

namespace casa {

/**
 * Common code for drawing a horizontal axis external to the plot
 */

class ExternalAxisWidgetHorizontal : public ExternalAxisWidget {
public:
	ExternalAxisWidgetHorizontal(QWidget* parent, QwtPlot* plot,
			bool leftAxisInternal, bool bottomAxisInternal,
			bool rightAxisInternal );
	virtual ~ExternalAxisWidgetHorizontal();
protected:
	//Return the x pixel coordinate for the start of the axis.
	virtual int getStartX() const;
	//Return the x pixel coordinate for the end of the axis.
	virtual int getEndX() const;
	//Return the length of the axis in pixels.
	virtual int getAxisLength() const;
	//Draw tick marks on the axis of the specified length.
	virtual void drawTicks( QPainter* painter, int tickLength );
	/*
	 *Overriden by specific horizontal axes to draw a tick.
	 *@param xPixel the x pixel location for the tick.
	 *@param value a numeric representation of the tick label.
	 *@param tickLength the length of the tick vertically in pixels.
	 */
	virtual void drawTick( QPainter* painter, double xPixel, double value, int tickLength)=0;
};

} /* namespace casa */
#endif /* EXTERNALAXISWIDGETHORIZONTAL_H_ */
