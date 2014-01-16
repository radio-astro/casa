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

#include "ToolTipPicker.h"
#include <guitools/Histogram/Histogram.h>
#include <QDebug>
#include <QtCore/qmath.h>
#include <qwt_plot.h>
#include <limits>

namespace casa {

ToolTipPicker::ToolTipPicker( int xAxis, int yAxis, int selectionFlags, RubberBand rubberBand,
			DisplayMode trackorMode, QwtPlotCanvas* canvas ):
				QwtPlotPicker( xAxis, yAxis, selectionFlags, rubberBand,
						trackorMode, canvas ){
	logScaleY = false;
}

void ToolTipPicker::setLogScaleY( bool logScale ){
	logScaleY = logScale;
}



void ToolTipPicker::setData( const std::vector<float>& xVal, const std::vector<float>& yVal ){
	xVector.resize( xVal.size());
	for ( int i = 0; i < static_cast<int>(xVector.size()); i++ ){
		xVector[i] = xVal[i];
	}
	yVector.resize( yVal.size());
	for ( int i = 0; i < static_cast<int>(yVector.size()); i++ ){
		yVector[i] = yVal[i];
	}
}

std::pair<float,float> ToolTipPicker::getRangeY() const {
	float min = std::numeric_limits<float>::max();
	float max = std::numeric_limits<float>::min();
	int rangeCount = yVector.size();
	for ( int i = 0; i < rangeCount; i++ ){
		float adjustedY = Histogram::computeYValue( yVector[i], logScaleY );
		if ( adjustedY < min ){
			min = adjustedY;
		}
		if ( adjustedY > max ){
			max = adjustedY;
		}
	}
	std::pair<float,float> range( min, max );
	return range;
}

int ToolTipPicker::findClosestPoint( float xValue, float yValue ) const {
	int pointIndex = -1;
	double distance = std::numeric_limits<float>::max();
	for ( int i = 0; i < static_cast<int>(xVector.size()); i++ ){
		float adjustedX = xVector[i];
		double xDistance = qAbs( adjustedX - xValue );
		if ( xDistance < distance ){
			distance = xDistance;
			pointIndex = i;
		}
	}

	const float ERROR_MARGIN_X = .1;
	if ( distance > ERROR_MARGIN_X ){
		pointIndex = -1;
	}

	if ( pointIndex >= 0 ){
		std::pair<float,float> rangeY = getRangeY();
		float spanY = rangeY.second - rangeY.first;
		const float ERROR_MARGIN_Y = spanY / 10;
		float adjustedY = Histogram::computeYValue( yVector[pointIndex], logScaleY );
		double yDistance = qAbs( adjustedY - yValue );
		if ( yDistance > ERROR_MARGIN_Y ){
			pointIndex = -1;
		}
	}
	return pointIndex;
}


QwtText ToolTipPicker::trackerText( const QwtDoublePoint & pos ) const {
	float xValue = pos.x();
	float yValue = pos.y();
	int pointIndex = findClosestPoint( xValue, yValue );
	QString toolTipText;
	if ( pointIndex != -1 ){
		float adjustedY = Histogram::computeYValue( yVector[pointIndex], logScaleY );
		float adjustedX = xVector[pointIndex];
		if ( !logScaleY ){
			toolTipText.sprintf("( %.5f, %.0f)", adjustedX, adjustedY );
		}
		else {
			toolTipText.sprintf("( %.5f, %.5f)", adjustedX, adjustedY );
		}
	}
	return QwtText( toolTipText );
}

ToolTipPicker::~ToolTipPicker() {
	// TODO Auto-generated destructor stub
}

} /* namespace casa */
