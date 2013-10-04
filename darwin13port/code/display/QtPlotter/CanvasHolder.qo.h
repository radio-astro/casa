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
#ifndef CANVASHOLDER_QO_H
#define CANVASHOLDER_QO_H

#include <QtGui/QWidget>
#include <qwt_legend.h>
#include <display/QtPlotter/CanvasHolder.ui.h>
#include <display/QtPlotter/QtCanvas.qo.h>

namespace casa {

//Holds the QtCanvas where the spectral profile is drawn.  When a legend
//external to the canvas is shown, this class handles the legend and the coordination
//between the legend and the pixel canvas.

	class CanvasHolder : public QWidget {
		Q_OBJECT

	public:
		CanvasHolder(QWidget *parent = 0);

		//Sets the visibility of the the legend.
		void setShowLegend( bool showLegend );

		//Determines whether a color bar will be drawn along side the legend
		//text.  When a color bar is not used, the text will be colored to
		//identify the curve.
		void setColorBarVisibility( bool visible );

		//Returns a list of curve identifiers
		QList<QString> getCurveLabels();

		//Sets the curve identifier
		void setCurveLabels( const QList<QString>& curveNames );

		//Location of the legend
		enum LegendLocation { CANVAS, CANVAS_BELOW, CANVAS_RIGHT};
		void setLegendPosition( int location );
		QtCanvas* getCanvas();
		~CanvasHolder();

	private slots:
		void changeLegendDisplay();

	private:
		void refreshLegend();
		void addLegend( QBoxLayout* innerLayout, QBoxLayout* outerLayout );
		void addHorizontalLegend( QBoxLayout* verticalLayout );
		void addVerticalLegend( QBoxLayout* verticalLayout );

		Ui::CanvasHolderClass ui;
		bool showLegend;
		bool colorBarVisible;
		LegendLocation legendLocation;
		QtCanvas pixelCanvas;
		QWidget* legend;
		QWidget* centralWidget;
	};
}
#endif // CANVASHOLDER_H
