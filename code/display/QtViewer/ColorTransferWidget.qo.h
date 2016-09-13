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
#ifndef COLORTRANSFERWIDGET_QO_H
#define COLORTRANSFERWIDGET_QO_H

#include <QtGui/QWidget>
#include <casa/Arrays/Vector.h>
#include <display/QtViewer/ColorTransferWidget.ui.h>

class QwtPlot;
class QwtPlotCurve;
class QwtLinearColorMap;

namespace casa {

	class ColorTransferWidget : public QWidget {
		Q_OBJECT

	public:
		ColorTransferWidget(QWidget *parent = 0);
		~ColorTransferWidget();

		void setColorMap(QwtLinearColorMap* linearMap );
		void setIntensities( std::vector<float>& intensities );
		void setColorLookups( const Vector<uInt>& lookups );

	private:
		ColorTransferWidget( const ColorTransferWidget& other );
		ColorTransferWidget operator=( const ColorTransferWidget& other );
		int colorScaleMin;
		int colorScaleMax;
		void resetColorBar();
		void resetColorCurve();

		Ui::ColorTransferWidgetClass ui;

		Vector<uInt> colorLookups;
		QwtPlot* plot;
		QwtPlotCurve* colorCurve;
		QwtLinearColorMap* colorMap;

		QVector<double> intensities;
	};

}
#endif // COLORTRANSFERWIDGET_QO_H
