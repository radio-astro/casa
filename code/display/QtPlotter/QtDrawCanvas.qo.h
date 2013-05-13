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


#ifndef TQDRAWCANVAS_H
#define TQDRAWCANVAS_H

#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/Record.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Inputs/Input.h>
#include <casa/Arrays/IPosition.h>

#include <display/QtPlotter/QtDrawSettings.h>

#include <graphics/X11/X_enter.h>
#include <QDir>
#include <QColor>
#include <QHash>
#include <QWidget>
#include <QMouseEvent>
#include <QToolButton>
#include <QDialog>
#include <QPixmap>
#include <QVBoxLayout>
#include <QLabel>
#include <map>
#include <vector>
#include <graphics/X11/X_exit.h>

namespace casa {

	typedef std::vector<double> CurveData;

	class GraphLabel {
	public:
		QString text;
		QString fontName;
		int fontSize;
		QColor color;
		GraphLabel() : text(""), fontName("Helvetica [Cronyx]"),
			fontSize(12), color(Qt::blue) {}
	};

	class QtDrawCanvas : public QWidget {
		Q_OBJECT
	public:
		enum {LINE = 0, POINT, FPOINT, CIRCLE, FCIRCLE,
		      RECT, FRECT, DIM, FDIM,
		      PLUS, FPLUS, CROSS, FCROSS,
		      TRIGU, FTRIGU, TRIGD, FTRIGD,
		      TRIGL, FTRIGL, TRIGR, FTRIGR,
		      SAND, FSAND, WING, FWING,
		      BEAM, FBEAM, BED, FBED,
		      HIST, FHIST
		     };
		QtDrawCanvas(QWidget *parent = 0);

		void setPlotSettings(const QtDrawSettings &settings);
		void setCurveData(int id, const CurveData data,
		                  int tp = 0, int cl = 0);
		void clearCurve(int id);
		void setDataRange();
		void setImageMode(bool);
		void setPixmap(const QImage&);
		QPixmap* graph();
		void drawBackBuffer(QPainter *);
		void plotPolyLines(QString);

		//ln = 0, new chart
		//ln < 0, remove trace
		//ln > 0, append trace if ln exists, otherwise add trace
		//tp = 0, continous line
		//tp > 0, discrete symbols
		//tp < 0, line and symbols
		//cl, color value range from 0 up
		void plotPolyLine(const Vector<Int>&,
		                  const Vector<Int>&,
		                  int ln = 0, int tp = 0, int cl = 0);
		void plotPolyLine(const Vector<Float> &x,
		                  const Vector<Float> &y,
		                  int ln = 0, int tp = 0, int cl = 0);
		void plotPolyLine(const Vector<Double>&,
		                  const Vector<Double>&,
		                  int ln = 0, int tp = 0, int cl = 0);

		//Each row is a trace
		//ln = 0, new chart
		//ln < 0, remove traces
		//ln > 0, append traces from (ln)th, add if not exists
		//tp < 0, same shape for all traces
		//tp = 0, continuous lines
		//tp > 0, different discrete shape for each trace
		//cl > 0, different color for each trace
		//cl <= 0, same color for all traces
		void plotPolyLine(const Matrix<Int> &verts,
		                  int ln = 0, int tp = 0, int cl = 0);
		void plotPolyLine(const Matrix<Float> &verts,
		                  int ln = 0, int tp = 0, int cl = 0);
		void plotPolyLine(const Matrix<Double> &verts,
		                  int ln = 0, int tp = 0, int cl = 0);

		// template<class T>
		void drawImage(const Matrix<uInt> &data, Matrix<uInt> *mask);
		void drawImage(const Matrix<uInt> &data);
		//void setMarkMode(bool);

		QColor getLinearColor(double);
		QSize minimumSizeHint() const;
		QSize sizeHint() const;
		~QtDrawCanvas();
		void increaseCurZoom();
		int getCurZoom();
		int getZoomStackSize();
		int getCurLine();
		int getTotalLines();

		void setTitle(const QString &text,
		              int fontSize = 12, int color = 0,
		              const QString &font = "Helvetica [Cronyx]");
		void setXLabel(const QString &text,
		               int fontSize = 10, int color = 1,
		               const QString &font = "Helvetica [Cronyx]");
		void setYLabel(const QString &text,
		               int fontSize = 10, int color = 2,
		               const QString &font = "Helvetica [Cronyx]");
		void setWelcome(const QString &text, int fontSize = 14,
		                int color = 1,
		                const QString &font = "Helvetica [Cronyx]");

	public slots:
		void zoomIn();
		void zoomOut();
		void markPrev();
		void markNext();

	signals:
		void zoomChanged();
		void gotFocus();

	protected:
		void paintEvent(QPaintEvent *event);
		void resizeEvent(QResizeEvent *event);
		void mousePressEvent(QMouseEvent *event);
		void mouseMoveEvent(QMouseEvent *event);
		void mouseReleaseEvent(QMouseEvent *event);
		void keyPressEvent(QKeyEvent *event);
		void wheelEvent(QWheelEvent *event);
		void focusInEvent(QFocusEvent *event);

	private:
		void updateRubberBandRegion();
		void refreshPixmap();
		void drawGrid(QPainter *painter);
		void drawTicks(QPainter *painter);
		void drawLabels(QPainter *painter);
		void drawWelcome(QPainter *painter);
		void drawCurves(QPainter *painter);
		void drawRects(QPainter *painter);

		enum { Margin = 60 };

		GraphLabel title;
		GraphLabel xLabel;
		GraphLabel yLabel;
		GraphLabel welcome;

		std::map<int, CurveData> curveMap;
		std::map<int, int> typeMap;
		std::map<int, int> colorMap;
		int curLine;

		std::vector<QtDrawSettings> zoomStack;
		int curZoom;

		std::map<int, CurveData> markerStack;
		int curMarker;

		QRect rubberBandRect;
		bool rubberBandIsShown;

		bool imageMode;
		bool markMode;

		QPixmap pixmap;
		QPixmap backBuffer;
		Matrix<uInt> *pMask;

	};

}
#endif

