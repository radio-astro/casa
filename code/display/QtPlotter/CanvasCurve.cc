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
#include "CanvasCurve.h"

#include <cmath>

namespace casa {

CanvasCurve::CanvasCurve(){
}

CanvasCurve::CanvasCurve( CurveData cData, ErrorData eData,
			QString legendTitle, QColor cColor){
	curveData = cData;
	errorData = eData;
	legend = legendTitle;
	curveColor = cColor;
}

QColor CanvasCurve::getColor() const{
	return curveColor;
}

QString CanvasCurve::getLegend() const{
	return legend;
}

CurveData CanvasCurve::getCurveData(){
	return curveData;
}

ErrorData CanvasCurve::getErrorData(){
	return errorData;
}

QString CanvasCurve::getToolTip( double x, double y , const double X_ERROR,
		const double Y_ERROR, const QString& xUnit, const QString& yUnit ) const {
	QString toolTipStr;
	for ( int i = 0; i < static_cast<int>(curveData.size()); i++ ){
		double curveX = curveData[2*i];
		double curveY = curveData[2*i+1];
		if ( fabs(curveX - x )< X_ERROR && fabs(curveY-y) < Y_ERROR ){
			toolTipStr.append( "(" );
			toolTipStr.append(QString::number( curveX ));
			toolTipStr.append( " " +xUnit +", " );
			toolTipStr.append(QString::number( curveY ));
			toolTipStr.append( " " + yUnit+ ")");
			break;
		}
	}
	return toolTipStr;
}


void CanvasCurve::getMinMax(Double& xmin, Double& xmax, Double& ymin,
		Double& ymax, bool plotError ) const {

	int maxPoints = curveData.size() / 2;
	int nErrPoints= errorData.size();

	if (plotError && nErrPoints>0){
		for (int i = 0; i < maxPoints; ++i){
			double dx = curveData[2 * i];
			double dyl = curveData[2 * i + 1] - errorData[i];
			double dyu = curveData[2 * i + 1] + errorData[i];
			xmin = (xmin > dx)  ? dx : xmin;
			xmax = (xmax < dx)  ? dx : xmax;
			ymin = (ymin > dyl) ? dyl : ymin;
			ymax = (ymax < dyu) ? dyu : ymax;
		}
	}
	else {
		for (int i = 0; i < maxPoints; ++i){
			double dx = curveData[2 * i];
			double dy = curveData[2 * i + 1];
			xmin = (xmin > dx) ? dx : xmin;
			xmax = (xmax < dx) ? dx : xmax;
			ymin = (ymin > dy) ? dy : ymin;
			ymax = (ymax < dy) ? dy : ymax;
		}
	}
}

CanvasCurve::~CanvasCurve() {
	// TODO Auto-generated destructor stub
}

} /* namespace casa */
