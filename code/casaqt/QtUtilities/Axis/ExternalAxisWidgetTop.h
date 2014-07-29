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

#ifndef CASAQT_EXTERNALAXISWIDGETTOP_H_
#define CASAQT_EXTERNALAXISWIDGETTOP_H_

#include <casaqt/QtUtilities/Axis/ExternalAxisWidgetHorizontal.h>

namespace casa {

class ExternalAxisWidgetTop : public ExternalAxisWidgetHorizontal {
public:
	ExternalAxisWidgetTop(QWidget* parent, QwtPlot* plot,
			bool leftAxisInternal, bool bottomAxisInternal,
			bool rightAxisInternal );
	virtual ~ExternalAxisWidgetTop();
protected:

	virtual void defineAxis( QLine& axisLine );
	//virtual void drawTicks( QPainter* painter, int tickLength);
	virtual void drawAxisLabel( QPainter* painter );
	virtual void drawTick( QPainter* painter, double xPixel, double value, int tickLength);
};

} /* namespace casa */
#endif /* EXTERNALAXISWIDGETTOP_H_ */
