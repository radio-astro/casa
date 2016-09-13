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
#include <casa/typeinfo.h>
#include <casa/Utilities/CountedPtr.h>

using namespace std;

class QDoubleValidator;

namespace casa {

template <class T> class ImageInterface;

/**
 * Designed as a separaclass so if it needs to go into a different thread,
 * it can be put there.
 */
class PercentageCalculator {
public:
	PercentageCalculator( float minValue, float maxValue, const SHARED_PTR<const ImageInterface<Float> > image );
	void work();
	float getRangeMin() const;
	float getRangeMax() const;
	virtual ~PercentageCalculator();

private:
	float minValue;
	float maxValue;
	float rangeMin;
	float rangeMax;
    SHARED_PTR<const ImageInterface<Float> > image;
};


/**
 * Pluggable functionality that allows users to specify a range
 * on the histogram.
 */

class RangeControlsWidget : public QWidget {
    Q_OBJECT

public:
    RangeControlsWidget(QWidget *parent = 0);
    void setImage(const SHARED_PTR<const ImageInterface<Float > > image );
    void hideMaximum();
    void setRange( double min, double max, bool signal=true );
    void setDataLimits( double min, double max );
    void setIgnoreRange( bool ignore );
    pair<double,double> getMinMaxValues() const;

    ~RangeControlsWidget();

signals:
	void minMaxChanged();
	void rangeCleared();

public slots:
	void percentilesDone();

protected:
	virtual void keyPressEvent( QKeyEvent* event );

private slots:
	void clearRange();
	void rangeModeChanged( bool percentile );
	void percentageChanged( const QString& newPercentage );

private:
	RangeControlsWidget(const RangeControlsWidget& );
	RangeControlsWidget& operator=( const RangeControlsWidget& );
    QDoubleValidator* minMaxValidator;
    PercentageCalculator* percentCalculator;
    Ui::RangeControlsWidgetClass ui;
    SHARED_PTR<const ImageInterface<Float > > image;
    QString percentage;
    double rangeMin;
    double rangeMax;
    bool ignoreRange;
};
}
#endif // RANGECONTROLSWIDGET_QO_H

