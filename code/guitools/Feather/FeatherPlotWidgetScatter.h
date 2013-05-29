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

#ifndef FEATHERPLOTWIDGETSCATTER_H_
#define FEATHERPLOTWIDGETSCATTER_H_

#include <guitools/Feather/FeatherPlotWidget.qo.h>

namespace casa {

/**
 * A specialization of FeatherPlotWidget for scatter plots.
 */

class FeatherPlotWidgetScatter : public FeatherPlotWidget {
public:
	FeatherPlotWidgetScatter(const QString& title, FeatherPlot::PlotType plotType, QWidget *parent = 0);
	virtual ~FeatherPlotWidgetScatter();
protected:
	virtual void resetColors();
	virtual void addZoomNeutralCurves();
	virtual void zoomRectangleOther( double minX, double maxX, double minY, double maxY );
	virtual void zoom90Other( double dishPosition, const QVector<double>& singleDishZoomDataX, const QVector<double>& singleDishZoomDataY,
			const QVector<double>& interferometerZoomDataX, const QVector<double>& interferometerZoomDataY );
};

} /* namespace casa */
#endif /* FEATHERPLOTWIDGETSCATTER_H_ */
