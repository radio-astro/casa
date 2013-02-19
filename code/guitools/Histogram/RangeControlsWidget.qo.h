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

#ifndef RANGECONTROLSWIDGET_QO_H
#define RANGECONTROLSWIDGET_QO_H

#include <QtGui/QWidget>
#include <guitools/Histogram/RangeControlsWidget.ui.h>

using namespace std;

class QDoubleValidator;

namespace casa {

/**
 * Pluggable functionality that allows users to specify a range
 * on the histogram.
 */

class RangeControlsWidget : public QWidget {
    Q_OBJECT

public:
    RangeControlsWidget(QWidget *parent = 0);
    void setRange( double min, double max );
    void setRangeLimits( double min, double max );
    void setDataLimits( double min, double max );
    pair<double,double> getMinMaxValues() const;
    ~RangeControlsWidget();

signals:
	void minMaxChanged();
	void rangeCleared();

private slots:
	void clearRange();

private:
	RangeControlsWidget(const RangeControlsWidget& );
	RangeControlsWidget& operator=( const RangeControlsWidget& );
    QDoubleValidator* minMaxValidator;
    Ui::RangeControlsWidgetClass ui;
    double rangeMin;
    double rangeMax;
};
}
#endif // RANGECONTROLSWIDGET_QO_H
