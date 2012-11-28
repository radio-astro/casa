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

#include "RangePicker.h"
#include <guitools/Histogram/HeightSource.h>
#include <QPainter>
#include <qwt_plot_canvas.h>
#include <qwt_plot.h>

namespace casa {


RangePicker::RangePicker( QwtPlotCanvas* canvas):
	QwtPlotPicker( QwtPlot::xBottom, QwtPlot::yLeft,
			QwtPlotPicker::RectSelection | QwtPlotPicker::DragSelection,
			QwtPlotPicker::RectRubberBand, QwtPlotPicker::AlwaysOn, canvas ){
}

void RangePicker::setHeightSource( HeightSource* source ){
	heightSource = source;
}

void RangePicker::drawRubberBand( QPainter* painter )const {
	QPolygon polySelect = selection();
	int count = polySelect.count();
	if ( count >= 2 ){
		QPoint firstPoint = polySelect.point(0);
		QPoint secondPoint = polySelect.point( 1 );
		int ptX = static_cast<int>( firstPoint.x() );
		if ( firstPoint.x() > secondPoint.x() ){
			ptX = static_cast<int>( secondPoint.x() );
		}
		int width = qAbs( firstPoint.x() - secondPoint.x() );
		QColor shadeColor(100,100,100 );
		shadeColor.setAlpha( 100 );
		int rectHeight = heightSource->getCanvasHeight();
		QRect rect(ptX, 0, width, rectHeight );
		painter->fillRect( rect , shadeColor );
	}
}

RangePicker::~RangePicker(){
}


} /* namespace casa */
