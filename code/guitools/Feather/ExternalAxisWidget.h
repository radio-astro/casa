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

#ifndef EXTERNALAXIS_WIDGET_H_
#define EXTERNALAXIS_WIDGET_H_

#include <QWidget>
#include <qwt_plot.h>

class QwtPlotCanvas;
class QwtScaleDiv;

namespace casa {

class ExternalAxisWidget : public QWidget {
public:
	ExternalAxisWidget(QWidget* parent);
	void setScaleDiv( QwtScaleDiv* scaleDiv );
	void setAxisLabel( const QString& label );
	void setPlotCanvas( QwtPlotCanvas* canvas );
	virtual void paintEvent( QPaintEvent* event );
	virtual ~ExternalAxisWidget();

protected:
	virtual void drawTicks( QPainter* painter, int tickLength ) = 0;
	virtual void defineAxis( QLine& axisLine ) = 0;
	virtual void drawAxisLabel( QPainter* painter ) = 0;
	int getTickIncrement( int tickCount, bool horizontal ) const;
	//static int getSmallestDivisor( int value );
	QwtPlotCanvas* canvas;
	QwtScaleDiv* scaleDiv;
	QString axisLabel;
	const int AXIS_SMALL_SIDE;

private:
	void drawBackBone( QPainter* painter );
	void drawTicks( QPainter* painter );
	void drawLabel( QPainter* painter );
	const int FONT_SIZE;
	const int FONT_SIZE_AXIS_LABEL;
};

} /* namespace casa */
#endif /* EXTERNALAXIS_WIDGET_H_ */
