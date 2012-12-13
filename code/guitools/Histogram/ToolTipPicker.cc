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

#include <QDebug>
#include <QtCore/qmath.h>
#include <qwt_plot.h>

namespace casa {

ToolTipPicker::ToolTipPicker( int xAxis, int yAxis, int selectionFlags, RubberBand rubberBand,
			DisplayMode trackorMode, QwtPlotCanvas* canvas ):
				QwtPlotPicker( xAxis, yAxis, selectionFlags, rubberBand,
						trackorMode, canvas ){
	logScale = false;
}

void ToolTipPicker::setLogScale( bool logScale ){
	this->logScale = logScale;
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

QwtText ToolTipPicker::trackerText( const QwtDoublePoint & pos ) const {
	float xValue = pos.x();
	//float yValue = pos.y();
	int pointIndex = -1;
	double distance = std::numeric_limits<float>::max();
	for ( int i = 0; i < static_cast<int>(xVector.size()); i++ ){
		double xDistance = qAbs( xVector[i] - xValue );
		if ( xDistance < distance ){
			distance = xDistance;
			pointIndex = i;
		}
	}

	const float ERROR_MARGIN_X = .1;
	if ( distance > ERROR_MARGIN_X ){
		pointIndex = -1;
	}

	QString toolTipText;
	if ( pointIndex != -1 ){
		if ( !logScale ){
			toolTipText.sprintf("( %.5f, %.0f)", xVector[pointIndex], yVector[pointIndex]);
		}
		else {
			float logValue = qLn( yVector[pointIndex] ) / qLn( 10 );
			toolTipText.sprintf("( %.5f, %.5f)", xVector[pointIndex], logValue );
		}
	}
	return QwtText( toolTipText );
}

ToolTipPicker::~ToolTipPicker() {
	// TODO Auto-generated destructor stub
}

} /* namespace casa */
