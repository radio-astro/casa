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

#ifndef QTCANVAS_H
#define QTCANVAS_H

#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/Record.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Inputs/Input.h>
#include <casa/Arrays/IPosition.h>

#include <display/QtPlotter/QtPlotSettings.h>

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
typedef std::vector<double> ErrorData;

class GraphLabel
{
public:
	QString text;
	QString fontName;
	int fontSize;
	QColor color;
	GraphLabel() : text(""), fontName("Helvetica [Cronyx]"), fontSize(12), color(Qt::blue) {}
};

class QtCanvas : public QWidget
{
	Q_OBJECT
public:
	QtCanvas(QWidget *parent = 0);

	void setPlotSettings(const QtPlotSettings &settings);
	void setCurveData(int id, const CurveData &data, const ErrorData &error=ErrorData(), const QString& lb="");
	CurveData* getCurveData(int);
	ErrorData* getCurveError(int id);
	QString getCurveName(int);
	int getLineCount();
	void clearCurve(int id = -1);
	void clearData();
	void setDataRange();
	void setImageMode(bool);
	void setPixmap(const QImage&);
	QPixmap* graph();
	void drawBackBuffer(QPainter *);
	void plotPolyLines(QString);
	void plotPolyLine(const Vector<Int>&, const Vector<Int>&);
	void plotPolyLine(const Vector<Float> &x, const Vector<Float> &y, const Vector<Float> &e,
			const QString& lb="");
	void plotPolyLine(const Vector<Double>&, const Vector<Double>&);
	void addPolyLine(const Vector<Float> &x, const Vector<Float> &y,
			const QString& lb="");
	void plotPolyLine(const Matrix<Int> &verts);
	void plotPolyLine(const Matrix<Float> &verts);
	void plotPolyLine(const Matrix<Double> &verts);
	// template<class T>
	// void drawImage(const Matrix<uInt> &data, Matrix<uInt> *mask);
	// void drawImage(const Matrix<uInt> &data);
	//QColor getLinearColor(double);
	QColor getDiscreteColor(const int &d);
	QSize minimumSizeHint() const;
	QSize sizeHint() const;
	~QtCanvas();
	void increaseCurZoom();
	int getCurZoom();
	int getZoomStackSize();

	void setTitle(const QString &text, int fontSize = 12,   int iclr = 1,
			const QString &font = "Helvetica [Cronyx]");
	QString getTitle(){return title.text;};
	void setXLabel(const QString &text, int fontSize = 10,  int iclr = 1,
			const QString &font = "Helvetica [Cronyx]");
	void setYLabel(const QString &text, int fontSize = 10,  int iclr = 1,
			const QString &font = "Helvetica [Cronyx]");
	void setWelcome(const QString &text, int fontSize = 14, int iclr = 1,
			const QString &font = "Helvetica [Cronyx]");
	void setAutoScaleX(int a) {autoScaleX = a;}
	void setAutoScaleY(int a) {autoScaleY = a;}
	void setShowGrid(int a)   {showGrid = a; refreshPixmap();}
	int getAutoScaleX( ) {return autoScaleX;}
	int getAutoScaleY( ) {return autoScaleY;}
	int getShowGrid( )   {return showGrid;}
	void setPlotError(int a)  {plotError = a; setDataRange();}

public slots:
   void zoomIn();
   void zoomOut();
   void zoomNeutral();

signals:
	void xRangeChanged(float xmin, float xmax);
	void channelSelect(float xvalue);

protected:
	void paintEvent(QPaintEvent *event);
	void resizeEvent(QResizeEvent *event);

	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent *event);
	void wheelEvent(QWheelEvent *event);

protected:
	void updateRubberBandRegion();
	void updatexRangeBandRegion();
	void refreshPixmap();
	void drawGrid(QPainter *painter);
	void drawTicks(QPainter *painter);
	void drawLabels(QPainter *painter);
	void drawWelcome(QPainter *painter);
	void drawCurves(QPainter *painter);
	void drawRects(QPainter *painter);
	void drawxRange(QPainter *painter);
	void defaultZoomIn();
	void defaultZoomOut();

	enum { MARGIN = 80 , FRACZOOM=20};

	GraphLabel title;
	GraphLabel xLabel;
	GraphLabel yLabel;
	GraphLabel welcome;

	std::map<int, QString> legend;
	std::map<int, CurveData> curveMap;
	std::map<int, ErrorData> errorMap;
	std::vector<QtPlotSettings> zoomStack;
	std::map<int, CurveData> markerStack;
	int curZoom;
	int curMarker;
	bool rubberBandIsShown;
	bool xRangeIsShown;
	bool imageMode;
	bool xRangeMode;
	double xRangeStart;
	double xRangeEnd;
	QRect rubberBandRect;
	QRect xRangeRect;
	QPixmap pixmap;
	QPixmap backBuffer;
	Matrix<uInt> *pMask;

	QPoint currentCursorPosition;
	QColor xcursor;

	int autoScaleX;
	int autoScaleY;
	int plotError;
	int showGrid;

	int xRectStart;
	int xRectEnd;

};

}
#endif
