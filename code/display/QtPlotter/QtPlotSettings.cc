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

#include <display/QtPlotter/QtPlotSettings.h>
#include <assert.h>
#include <QtGui>
namespace casa { 

QtPlotSettings::QtPlotSettings()
{
	const int TICK_MIN = 0;
	const int TICK_MAX = 10;
	const int TICK_COUNT = 5;
	for ( int i = 0; i < QtPlotSettings::END_AXIS_INDEX; i++ ){
		AxisIndex axisIndex = static_cast<AxisIndex>(i);
		minX[axisIndex] = TICK_MIN;
		maxX[axisIndex] = TICK_MAX;
		numXTicks[axisIndex] = TICK_COUNT;
	}

    minY = TICK_MIN;
    maxY = TICK_MAX;
    numYTicks = TICK_COUNT;
}

void QtPlotSettings::scroll(int dx, int dy)
{
	for ( int i = 0; i < QtPlotSettings::END_AXIS_INDEX; i++ ){
		AxisIndex axisIndex = static_cast<AxisIndex>(i);
		double stepX = spanX( axisIndex ) / numXTicks[i];
		minX[i] += dx * stepX;
		maxX[i] += dx * stepX;
	}

    double stepY = spanY() / numYTicks;
    minY += dy * stepY;
    maxY += dy * stepY;
}

void QtPlotSettings::zoomOut( double zoomFactor ){
	for ( int i = 0; i < END_AXIS_INDEX; i++ ){
		AxisIndex axisIndex = static_cast<AxisIndex>(i);
		int prevSpanX = spanX(axisIndex);
		minX[i] = minX[i] - zoomFactor * prevSpanX;
		maxX[i] = maxX[i] + zoomFactor * prevSpanX;
	}
	int prevSpanY = spanY();
	minY = minY - zoomFactor * prevSpanY;
	maxY = maxY + zoomFactor * prevSpanY;
	adjust();
}

void QtPlotSettings::zoomIn( double zoomFactor ){
	for ( int i = 0; i < END_AXIS_INDEX; i++ ){
		AxisIndex axisIndex = static_cast<AxisIndex>(i);
		int prevSpanX = spanX( axisIndex );
		minX[i] = minX[i] + zoomFactor * prevSpanX;
		maxX[i] = maxX[i] - zoomFactor * prevSpanX;
	}
	int prevSpanY = spanY();
	minY = minY + zoomFactor * prevSpanY;
	maxY = maxY - zoomFactor * prevSpanY;
	adjust();
}

void QtPlotSettings::adjust(){
	for ( int i = 0; i < END_AXIS_INDEX; i++ ){
		adjustAxis(minX[i], maxX[i], numXTicks[i]);
	}
    adjustAxis(minY, maxY, numYTicks);
}

void QtPlotSettings::adjustAxis(double &min, double &max,
                                   int &numTicks)
{
    const int MinTicks = 4;
    double grossStep = fabs(max - min) / MinTicks;
    double step = std::pow(10, floor(log10(grossStep)));

    if (5 * step < grossStep)
        step *= 5;
    else if (2 * step < grossStep)
        step *= 2;
    numTicks = (int)fabs(ceil(max / step) - floor(min / step));
    min = floor(min / step) * step;
    max = ceil(max / step) * step;
}

void QtPlotSettings::setMinX( AxisIndex index, double value ){
 	minX[static_cast<int>(index)] = value;
}

void QtPlotSettings::setMaxX( AxisIndex index, double value ){
     maxX[static_cast<int>(index)] = value;
}

void QtPlotSettings::setMinY( double value ){
     minY = value;
}

void QtPlotSettings::setMaxY( double value ){
  	maxY = value;
}



}

