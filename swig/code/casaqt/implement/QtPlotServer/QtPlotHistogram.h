//# QtPlotHistogram.h: QwtPlotItem specialization for histograms
//# with surrounding Gui functionality
//# Copyright (C) 2010
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
//# $Id: QtPlotServer.qo.h,v 1.7 2006/10/10 21:42:05 dking Exp $

#ifndef QTPLOTHISTOGRAM_H_
#define QTPLOTHISTOGRAM_H_

#include <qglobal.h>
#include <qcolor.h>
#include <qwt_plot_item.h>
#include <qwt_interval_data.h>

class QtPlotHistogram: public QwtPlotItem {
    public:
	explicit QtPlotHistogram( const QString &label = QString::null );
	explicit QtPlotHistogram( const QwtText &title );
	~QtPlotHistogram( ) { }

	void setData( const QwtIntervalData &data );
	const QwtIntervalData &data( ) const { return data_; }

	QwtDoubleRect boundingRect( ) const;
	int rtti( ) const { return QwtPlotItem::Rtti_PlotHistogram; }

	void draw( QPainter *, const QwtScaleMap &xMap, const QwtScaleMap &yMap, const QRect & ) const;

	void setBaseline(double reference);
	double baseline() const { return reference_; }

	void setColor(const QColor &c);
	QColor color() const { return color_; }


    private:
	virtual void draw_bar( QPainter *, Qt::Orientation orientation, const QRect & ) const;
	QwtIntervalData data_;
	double reference_;
	QColor color_;
};

#endif
