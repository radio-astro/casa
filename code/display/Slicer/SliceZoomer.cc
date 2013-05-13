//# Copyright (C) 1994,1995,1996,1997,1998,1999,2000
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

#include "SliceZoomer.h"

#include <qwt_plot_canvas.h>
#include <qwt_plot.h>
#include <QKeyEvent>
#include <QDebug>
#include <QCursor>

namespace casa {

	SliceZoomer::SliceZoomer(QwtPlotCanvas* plotCanvas ):
		QwtPlotZoomer(QwtPlot::xBottom, QwtPlot::yLeft, plotCanvas) {
		canvas = plotCanvas;
		zoomMode = false;

		//You can zoom by dragging a rectangle or you can zoom a fixed amount
		//by clicking
		setSelectionFlags( QwtPicker::DragSelection | QwtPicker::PointSelection);
		setRubberBand( QwtPicker::RectRubberBand );

		//Normally the zoom activates with the control-left button.
		//MouseSelect1 draws rectangles
		setMousePattern( QwtEventPattern::MouseSelect1, Qt::LeftButton, Qt::ControlModifier);

		//MouseSelect6 zooms in one position on the stack based on a point
		//********************************************************************
		//       Problem need to compute a rectangle based on a point
		//*********************************************************
		//setMousePattern( QwtEventPattern::MouseSelect6, Qt::LeftButton, Qt::ControlModifier);

		//The cursor
		zoomMap.load( ":/images/zoomCursor.png");
		QSize zoomMapSize = zoomMap.size();
		zoomCursor = new QCursor( zoomMap );

		//Use the +,-, and Home keys to zoom in and out
		//These zoom up and down the stack.
		setKeyPattern( QwtEventPattern::KeyRedo, Qt::Key_Down, Qt::ControlModifier  );
		setKeyPattern( QwtEventPattern::KeyUndo, Qt::Key_Up, Qt::ControlModifier   );
		setKeyPattern( QwtEventPattern::KeyHome, Qt::Key_Home, Qt::ControlModifier );
	}



	void SliceZoomer::zoom( const QwtDoubleRect& rect) {
		QwtPlotZoomer::zoom( rect );
		zoomMode = false;
	}



	void SliceZoomer::zoomIn() {
		//The zoom in mode was activated.  We can now zoom in using a rectangle
		//or clicking on a point with the left mouse button without any modifiers
		setMousePattern( QwtEventPattern::MouseSelect1, Qt::LeftButton, Qt::NoModifier);
		zoomMode = true;
		canvas->setCursor( *zoomCursor );
	}

	void SliceZoomer::widgetMouseReleaseEvent( QMouseEvent* event ) {
		QwtPlotZoomer::widgetMouseReleaseEvent( event );
		if ( zoomMode ) {

			//Note:  we should calculate our own rectangle based on the
			//point selected and then call zoom( rect) to put it on the stack?


			QPointF position = event->pos();
			QSize canvasSize = canvas->size();
			if ( position.x() < canvasSize.width() && position.y() < canvasSize.height()) {

				//The zoom rectangle must be in the canvas.
				float widthMax = qMin( position.x(), canvasSize.width() - position.x() );
				float heightMax = qMin( position.y(), canvasSize.height() - position.y());

				float zoomPercent = 0.8;
				float zoomedWidth = canvasSize.width()/2 * zoomPercent;
				float zoomedHeight = canvasSize.height()/2 * zoomPercent;

				float boxHalfWidth = qMin( widthMax, zoomedWidth);
				float boxHalfHeight = qMin( heightMax, zoomedHeight );

				QwtDoubleRect rect( position.x() - boxHalfWidth, position.y() - boxHalfHeight,
				                    2 * boxHalfWidth, 2 * boxHalfHeight );
				zoom( rect );

			}
		}


		//We change back to requiring the Ctrl key for zooming.
		setMousePattern( QwtEventPattern::MouseSelect1, Qt::LeftButton, Qt::ControlModifier );

		canvas->unsetCursor();

	}

	void SliceZoomer::zoomOut() {
		zoomMode = false;
		canvas->unsetCursor();
		QwtPlotZoomer::zoom( -1 );
	}

	void SliceZoomer::zoomNeutral() {
		zoomMode = false;
		canvas->unsetCursor();
		QwtPlotZoomer::zoom( 0 );
	}

	SliceZoomer::~SliceZoomer() {
		delete zoomCursor;
	}

} /* namespace casa */
