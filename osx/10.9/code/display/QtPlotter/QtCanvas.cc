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

#include <casa/aips.h>
#include <display/QtPlotter/QtCanvas.qo.h>
#include <display/QtPlotter/Util.h>
#include <display/QtPlotter/annotations/AnnotationText.h>
#include <display/QtPlotter/canvasMode/CanvasModeFactory.h>

#include <graphics/X11/X_enter.h>
#include <QDir>
#include <QColor>
#include <QHash>
#include <QWidget>
#include <QAction>
#include <QMouseEvent>
#include <QTextStream>
#include <cmath>
#include <QtGui>
#include <iostream>
#include <algorithm>
#include <graphics/X11/X_exit.h>

using namespace std;

namespace casa {

	const QString QtCanvas::FONT_NAME = "Helvetica [Cronyx]";

	QtCanvas::~QtCanvas() {
		this->clearCurve(true);
		delete modeFactory;
	}

	QtCanvas::QtCanvas(QWidget *parent)
		: QWidget(parent),
		  MARGIN_LEFT(100), MARGIN_BOTTOM(60), MARGIN_TOP(90), MARGIN_RIGHT(25), FRACZOOM(20),ZERO_LIMIT(0.0000000000000005f),
		  title(), yLabel(), welcome(),
		  showTopAxis( true ), showToolTips( true ), showFrameMarker( true ), displayStepFunction( false ),
		  contextMenu( this ),
		  frameMarkerColor( Qt::cyan), zoomColor( Qt::yellow ),
		  showLegend( true ), legendPosition( 0 ),
		  selectedAnnotation( NULL ), currentMode( NULL ) {
		setMouseTracking(true);
		setAttribute(Qt::WA_NoBackground);
		setBackgroundRole(QPalette::Dark);
		setForegroundRole(QPalette::Light);
		setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		setFocusPolicy(Qt::StrongFocus);
		rubberBandIsShown = false;
		xRangeIsShown=false;
		imageMode = false;
		traditionalColors = true;
		setPlotSettings(QtPlotSettings());
		xRangeMode=false;
		autoScaleX = true;
		autoScaleY = true;
		plotError  = 2;
		showGrid   = 2;
		gaussEstimateCount = 0;
		taskMode = LINE_OVERLAY_MODE;
		topAxisCompatible = false;

		initContextMenu();

		//CanvasMode::setReceiver( this );
		//canvasId = instanceId;
		//instanceId++;
		modeFactory = new CanvasModeFactory( this );

	}


	void QtCanvas::initContextMenu() {
		centerPeakAction = new QAction("(Center,Peak)", this );
		centerPeakAction->setStatusTip( "Set the (Center,Peak) of the Gaussian Estimate");
		connect( centerPeakAction, SIGNAL(triggered()), this, SLOT(gaussianCenterPeakSelected()));

		fwhmAction = new QAction( "FWHM/2 from Center", this );
		fwhmAction->setStatusTip("Specify the fwhm");
		connect( fwhmAction, SIGNAL(triggered()), this, SLOT(gaussianFWHMSelected()));

		findRedshiftAction = new QAction("Find redshift...", this );
		findRedshiftAction->setStatusTip( "Specify a line and get the redshift back.");
		connect( findRedshiftAction, SIGNAL(triggered()), this, SLOT(findRedshift()));

		createAnnotationAction = new QAction( "Create Annotation...", this );
		createAnnotationAction->setStatusTip( "Create a plot annotation.");
		connect( createAnnotationAction, SIGNAL(triggered()), this, SLOT( createAnnotationText()));

		editAnnotationAction = new QAction("Edit Annotation...", this );
		editAnnotationAction->setStatusTip( "Edit the properties of the selected annotation.");
		connect( editAnnotationAction, SIGNAL(triggered()), this, SLOT(editSelectedAnnotation() ));

		deleteAnnotationAction = new QAction("Delete Annotation", this );
		deleteAnnotationAction->setStatusTip( "Remove the selected annotation.");
		connect( deleteAnnotationAction, SIGNAL(triggered()), this, SLOT(deleteSelectedAnnotation()));
	}

	void QtCanvas::setPlotSettings(const QtPlotSettings &settings) {
		zoomStack.resize(1);
		curZoom = 0;

		zoomStack[curZoom] = settings;
		curMarker = 0;
		refreshPixmap();
	}

	void QtCanvas::zoomOut() {
		xRangeIsShown=false;
		if (curZoom > 0) {
			--curZoom;
			refreshPixmap();
		} else {
			if (curveMap.size() != 0) {
				defaultZoomOut();
			}
		}
		//emit xRangeChanged(1.0,0.0);
	}

	void QtCanvas::defaultZoomOut() {
		//Note: This is called only if curZoom = 0
		QtPlotSettings settings;
		if ( zoomStack.size() > 0 ) {
			settings = zoomStack[0];
		}

		double zoomFactor = (double)FRACZOOM/100.0;
		settings.zoomOut( zoomFactor, getUnits(QtPlotSettings::xTop),
		                  getUnits( QtPlotSettings::xBottom), autoScaleX, autoScaleY );
		zoomYBasedOnX( settings, zoomFactor, false );

		std::vector<QtPlotSettings>::iterator it;
		it = zoomStack.begin();
		it = zoomStack.insert ( it , settings);
		refreshPixmap();
	}


	void QtCanvas::zoomIn() {
		if (curZoom < (int)zoomStack.size() - 1) {
			xRangeIsShown=false;
			++curZoom;
			refreshPixmap();
			//emit xRangeChanged(1.0,0.0);
		} else {
			if (curveMap.size() != 0) {
				defaultZoomIn();
			}
		}
	}

	void QtCanvas::zoomYBasedOnX( QtPlotSettings& settings, double zoomFactor, bool zoomIn ) {
		//Make sure all y curve values in the x zoom appear in the plot.
		double minX = settings.getMinX(QtPlotSettings::xBottom );
		double maxX = settings.getMaxX(QtPlotSettings::xBottom );
		pair<double,double> yRange = getRangeFor( zoomFactor, zoomIn, minX, maxX );
		settings.zoomY( yRange.first, yRange.second, autoScaleY );
	}

	void QtCanvas::defaultZoomIn() {

		QtPlotSettings settings;
		double zoomFactor = (double)FRACZOOM/100.0;
		if ( zoomStack.size() > 0 ) {
			settings = zoomStack[curZoom];
		}

		//Change x zoom by a percentage.
		settings.zoomIn( zoomFactor, getUnits(QtPlotSettings::xTop),
		                 getUnits( QtPlotSettings::xBottom), autoScaleX, autoScaleY );
		zoomYBasedOnX( settings, zoomFactor, true );

		zoomStack.push_back(settings);
		zoomIn();
	}

	pair<double,double> QtCanvas::getRangeFor( double zoomFactor, bool zoomIn, double minX, double maxX ) {
		pair<double,double> yRange;
		int curveCount = curveMap.size();
		bool rangeInitialized = false;
		if ( curveCount > 0 ) {
			for ( int i = 0; i < curveCount; i++ ) {
				Bool rangeExists = false;
				pair<double,double> curveRange = curveMap[i].getRangeFor( minX, maxX, rangeExists );
				if ( rangeExists ) {
					if ( rangeInitialized ) {
						if ( curveRange.first < yRange.first ) {
							yRange.first = curveRange.first;
						}
						if ( curveRange.second > yRange.second ) {
							yRange.second = curveRange.second;
						}
					} else {
						yRange.first = curveRange.first;
						yRange.second = curveRange.second;
						rangeInitialized = true;
					}
				}
			}
		}
		if ( !rangeInitialized ) {
			QtPlotSettings prevSettings = zoomStack[curZoom];
			pair<double,double> initialRange = prevSettings.getZoomInY( zoomFactor );
			if ( !zoomIn ) {
				initialRange = prevSettings.getZoomOutY( zoomFactor );
			}
			yRange.first = initialRange.first;
			yRange.second = initialRange.second;
		}
		return yRange;
	}



	void QtCanvas::zoomNeutral() {
		//xRangeIsShown=false;
		if ( zoomStack.size() != 1 ){
			zoomStack.resize(1);
			zoomStack[0] = QtPlotSettings();
			curZoom = 0;

			setDataRange();

			if (xRangeIsShown ){
				QtPlotSettings currSettings = zoomStack[curZoom];
				double dx = currSettings.spanX(QtPlotSettings::xBottom) / getRectWidth();
				double currMinX = currSettings.getMinX( QtPlotSettings::xBottom );
				double newXStart = currMinX + dx * (xRectStart - MARGIN_LEFT);
				double newXEnd = currMinX + dx * (xRectEnd - MARGIN_LEFT );

				xRangeStart = qMax( newXStart,xRangeStart);
				xRangeEnd = qMin( newXEnd, xRangeEnd );
				update();
				emit xRangeChanged(xRangeStart,xRangeEnd);


			}
		}
	}

	int QtCanvas::getLineCount() {
		return curveMap.size();
	}

	CurveData QtCanvas::getCurveData(int id) {
		return curveMap[id].getCurveData();
	}
	ErrorData QtCanvas::getCurveError(int id) {
		return curveMap[id].getErrorData();
	}

	QString QtCanvas::getCurveName(int id) {
		return curveMap[id].getLegend();
	}

	void QtCanvas::setCurveName(int id, const QString& name ) {
		curveMap[id].setLegend( name );
	}

	QColor QtCanvas::getCurveColor( int id ) {
		return curveMap[id].getColor();
	}

	void QtCanvas::stripCurveTitleNumbers( QString& curveName ) const {
		int bracketIndex = curveName.indexOf( "<");
		if ( bracketIndex > 0 ) {
			curveName = curveName.mid( 0, bracketIndex );
			curveName = curveName.trimmed();
		}
	}


	int QtCanvas::duplicateCurve( QString& targetLabel ) {
		int duplicateCurve = -1;
		//Duplicate curves will have a number in brackets after their name such as <3>
		stripCurveTitleNumbers( targetLabel );
		for ( int i = 0; i < static_cast<int>(curveMap.size()); i++ ) {
			QString curveLabel = curveMap[i].getLegend();
			stripCurveTitleNumbers( curveLabel );
			if ( targetLabel == curveLabel ) {
				duplicateCurve = i;
				break;
			}
		}
		return duplicateCurve;
	}

	void QtCanvas::setCurveData(int id, const CurveData &data,
			double beamAngle, double beamArea,
			SpectralCoordinate coord, const ErrorData &error,
	        const QString& lbl, ColorCategory level ) {
		QString curveLabel = lbl;

		//If the curve is already there we will replace it (curve may have
		//different data but the same name, for example, a new multifit curve)
		int curveIndex = duplicateCurve( curveLabel );
		if ( curveIndex != -1 ){
			id = curveIndex;
		}

		QColor curveColor = getDiscreteColor( level, id );
		CanvasCurve curve( data, error, curveLabel, curveColor, level, beamAngle, beamArea, coord);
		//Make sure the curve is in the same units as the canvas is using.
		if ( yUnitDisplay != yUnitImage ) {
			curve.scaleYValues( yUnitImage, yUnitDisplay, getUnits() );
		}
		curveMap[id]     = curve;
		refreshPixmap();
		emit curvesChanged();
	}

	void QtCanvas::setDataRange() {
		//if (autoScaleX == 0 && autoScaleY == 0)
		//	return;


		Double xmin = 1000000000000000000000000.;
		Double xmax = -xmin;
		Double ymin = 1000000000000000000000000.;
		Double ymax = -ymin;
		std::map<int, CanvasCurve>::const_iterator it = curveMap.begin();
		while (it != curveMap.end()) {
			const CanvasCurve & canvasCurve = (*it).second;
			canvasCurve.getMinMax(xmin, xmax, ymin, ymax, plotError );
			++it;
		}

		if ( autoScaleX ) {
			adjustExtremes( &xmin, &xmax );
		}
		if ( autoScaleY ) {
			adjustExtremes( &ymin, &ymax );
		}

		//Store the results in the plot settings
		QtPlotSettings settings;
		//if (autoScaleX) {
		settings.setMinX(QtPlotSettings::xBottom, xmin);
		settings.setMaxX(QtPlotSettings::xBottom, xmax);

		settings.setMinX(QtPlotSettings::xTop, topAxisRange.first );
		settings.setMaxX(QtPlotSettings::xTop, topAxisRange.second );
		//}
		//if (autoScaleY) {
		settings.setMinY(ymin);
		settings.setMaxY(ymax);
		//}

		settings.adjust( getUnits( QtPlotSettings::xTop ), getUnits(QtPlotSettings::xBottom), autoScaleX, autoScaleY );

		if ( curZoom > 0 ) {
			// if the canvas is zoomed, keep the zoom level,
			//update unzoomed state...
			zoomStack[0] = settings;
			refreshPixmap();
		} else {
			// reset the canvas, zoom, etc.
			setPlotSettings(settings);
		}
	}

	QString QtCanvas::getUnits( QtPlotSettings::AxisIndex axisIndex ) {
		QString unitStr = xLabel[axisIndex].text;
		int unitIndex = unitStr.indexOf( "[");
		if ( unitIndex >= 0 ) {
			int unitLength = unitStr.length() - unitIndex - 2;
			unitStr = unitStr.mid(unitIndex+1, unitLength );
		} else {
			unitStr = "";
		}
		return unitStr;
	}

	void QtCanvas::clearFitCurves() {
		std::map<int, CanvasCurve>::const_iterator it = curveMap.begin();
		QList<int> removeKeys;
		while (it != curveMap.end()) {
			const CanvasCurve & canvasCurve = (*it).second;
			int curveType = canvasCurve.getCurveType();
			if ( curveType == CURVE_COLOR_PRIMARY || curveType == CURVE_COLOR_SECONDARY ) {
				removeKeys.append( (*it).first );
			}
			it++;
		}
		for ( int i = 0; i < removeKeys.size(); i++ ) {
			int removeKey = removeKeys[i];
			std::map<int,CanvasCurve>::iterator index = curveMap.find( removeKey );
			curveMap.erase( index );
		}
		curveCountPrimary = 0;
		curveCountSecondary = 0;
	}

	void QtCanvas::clearCurve( bool inDtor ) {
		curveMap.clear();
		profileFitMarkers.clear();

		//this->xRangeIsShown = false;
		curveCount = 0;
		curveCountPrimary = 0;
		curveCountSecondary = 0;

		while( !annotations.empty() ) {
			Annotation* annotation = annotations.back();
			delete annotation;
			annotations.pop_back();
		}
		delete selectedAnnotation;
		selectedAnnotation = NULL;
		if ( ! inDtor ) {
			emit curvesChanged();
			refreshPixmap();
		}
	}

	void QtCanvas::clearEverything(){
		clearMolecularLines( false );
		clearCurve();
	}

	void QtCanvas::clearMolecularLines( bool refresh ) {
		while( !molecularLineStack.isEmpty()) {
			MolecularLine* line = molecularLineStack.takeFirst();
			delete line;
		}
		if ( refresh ) {
			setDataRange();
			refreshPixmap();
		}
	}

	void QtCanvas::setTopAxisRange(const Vector<Float> &values, bool topAxisDescending ) {
		double min = 1000000000000000000000000.;
		double max = -min;
		for ( int i = 0; i < static_cast<int>(values.size()); i++ ) {
			if ( values[i] < min ) {
				min = values[i];
			}
			if ( values[i] > max ) {
				max = values[i];
			}
		}

		adjustExtremes( &min, &max );


		//We have to reset the range so the units on the top axis don't appear
		//unzoomed where the ones on the bottom axis are zoomed.
		zoomNeutral();

		//Switch the min and max if we need to draw the labels on the top
		//axis in descending order.
		if ( topAxisDescending ) {
			float tmp = min;
			min = max;
			max = tmp;
		}
		topAxisRange.first = min;
		topAxisRange.second = max;
		QtPlotSettings currentSettings = zoomStack[curZoom];
		currentSettings.setMinX(QtPlotSettings::xTop, min );
		currentSettings.setMaxX( QtPlotSettings::xTop, max );
		currentSettings.adjust(getUnits(QtPlotSettings::xTop), getUnits( QtPlotSettings::xBottom), autoScaleX, autoScaleY);
		zoomStack[curZoom] = currentSettings;
		refreshPixmap();
	}

	QSize QtCanvas::minimumSizeHint() const {
		return QSize( 4 * MARGIN_LEFT, 4 * MARGIN_BOTTOM);
	}

	QSize QtCanvas::sizeHint() const {
		return QSize( 8 * MARGIN_LEFT, 6 * MARGIN_BOTTOM );
	}

	void QtCanvas::editSelectedAnnotation() {
		Annotation* selectedAnnotation = getSelectedAnnotation();
		if ( selectedAnnotation != NULL ) {
			selectedAnnotation->showEditor();
		}
	}

	void QtCanvas::deleteSelectedAnnotation() {
		Annotation* selectedAnnotation = getSelectedAnnotation();
		if ( selectedAnnotation != NULL ) {
			vector<Annotation*>::iterator annotationIter = annotations.begin();
			while( annotationIter != annotations.end()) {
				if ( (*annotationIter) == selectedAnnotation ) {
					annotations.erase( annotationIter );
					update();
					break;
				}
				++annotationIter;
			}
		}
	}

	bool QtCanvas::isAnnotation( QMouseEvent* event ) const {
		bool annotationFound = false;
		for ( int i = 0; i < static_cast<int>(annotations.size()); i++ ) {
			int xPos = event->pos().x();
			int yPos = event->pos().y();
			if ( annotations[i]->contains( xPos, yPos) ) {
				annotationFound = true;
				break;
			}
		}
		return annotationFound;
	}

	Annotation* QtCanvas::getSelectedAnnotation() const {
		Annotation* selectedAnnotation = NULL;
		for ( int i = 0; i < static_cast<int>(annotations.size()); i++ ) {
			if ( annotations[i]->isSelected()) {
				selectedAnnotation = annotations[i];
				break;
			}
		}
		return selectedAnnotation;
	}

	void QtCanvas::resetSelectedAnnotation( QMouseEvent* event ) {
		QPoint mouseLocation = event->pos();
		int mouseX = mouseLocation.x();
		int mouseY = mouseLocation.y();
		int annotationCount = annotations.size();

		//Only one annotation should be selected at a time -the first one we find.
		bool annotationFound = false;
		for ( int i = 0; i < annotationCount; i++ ) {
			if ( annotations[i]->contains( mouseX, mouseY) && !annotationFound) {
				annotations[i]->setSelected( true );
				annotationFound = true;
			} else {
				if ( annotations[i]->isSelected() ) {
					annotations[i]->setSelected( false );
				}
			}
		}
	}

	void QtCanvas::selectAnnotation( QMouseEvent* event, bool selection ) {
		if ( currentMode != NULL && currentMode->isMode( CanvasMode::MODE_ANNOTATION )) {
			if ( !selection ) {
				vector<Annotation*>::iterator iter=annotations.begin();
				while( iter != annotations.end()) {
					((*iter)->setSelected( false ));
					++iter;
				}
			} else {
				resetSelectedAnnotation( event );
			}
			refreshPixmap();
		}
	}


	void QtCanvas::displayToolTip( QMouseEvent* event ) const {
		if ( showToolTips ) {
			QPoint mouseLocation = event -> pos();
			int mouseX = mouseLocation.x();
			int mouseY = mouseLocation.y();
			double x = getDataX( mouseX );
			double y = getDataY( mouseY );

			QtPlotSettings settings = zoomStack[curZoom];
			QString ptCoords = findCoords( x, y );
			if ( ! ptCoords.isNull() && ! ptCoords.isEmpty() ) {
				QToolTip::showText(event->globalPos(), ptCoords );
			} else {
				QToolTip::hideText();
				event->ignore();
			}
		}
	}

	QString QtCanvas::findCoords( double x, double y ) const {
		//Error should be relative to the current zoom.
		QString coordStr;
		std::map<int, CanvasCurve>::const_iterator it = curveMap.begin();
		while (it != curveMap.end()) {
			const CanvasCurve & canvasCurve = (*it).second;
			QString toolTipStr = canvasCurve.getToolTip( x, y , toolTipXUnit, yUnitDisplay );
			if ( !toolTipStr.isEmpty() ) {
				coordStr = toolTipStr;
				break;
			}
			++it;
		}
		return coordStr;
	}

	int QtCanvas::getRectHeight() const {
		return height() - MARGIN_TOP - MARGIN_BOTTOM;
	}

	int QtCanvas::getRectWidth() const {
		return width() - MARGIN_LEFT - MARGIN_RIGHT;
	}

	int QtCanvas::getRectBottom() const {
		return height() - MARGIN_BOTTOM;
	}

	int QtCanvas::getRectLeft() const {
		return MARGIN_LEFT;
	}

	double QtCanvas::getDataX( int pixelPosition ) const {
		QtPlotSettings settings = zoomStack[curZoom];
		int rectWidth = getRectWidth();
		int rectLeft = getRectLeft();
		double xAxisSpan = settings.spanX( QtPlotSettings::xBottom );
		double xMin = settings.getMinX( QtPlotSettings::xBottom );
		double dataX = xMin + (pixelPosition - rectLeft) * xAxisSpan / (rectWidth -1);
		return dataX;
	}

	double QtCanvas::getDataY( int pixelPosition ) const {
		QtPlotSettings settings = zoomStack[curZoom];
		int rectHeight = getRectHeight();
		int rectBottom = getRectBottom();
		double yAxisSpan = settings.spanY();
		double yMin = settings.getMinY();
		double dataY = yMin + (rectBottom - pixelPosition) * yAxisSpan / (rectHeight - 1);
		return dataY;
	}

	int QtCanvas::getPixelX( double dataX ) const {
		QtPlotSettings settings = zoomStack[curZoom];
		int rectWidth = getRectWidth();
		int rectLeft = getRectLeft();
		double xAxisSpan = settings.spanX( QtPlotSettings::xBottom );
		double xMin = settings.getMinX( QtPlotSettings::xBottom );
		int pixelX = rectLeft + static_cast<int>( (dataX - xMin ) * (rectWidth - 1) / xAxisSpan );
		return pixelX;
	}

	int QtCanvas::getPixelY( double dataY ) const {
		QtPlotSettings settings = zoomStack[curZoom];
		int rectHeight = getRectHeight();
		int rectBottom = getRectBottom();
		double yAxisSpan = settings.spanY();
		double yMin = settings.getMinY();
		int pixelY = rectBottom - static_cast<int>((dataY - yMin)*(rectHeight - 1) / yAxisSpan);
		return pixelY;
	}

	void QtCanvas::paintEvent(QPaintEvent *event) {
		/*bool parentActive = isParentActive();
		qDebug() << "Paint event canvasId="<<canvasId<<" parentActive="<<parentActive;
		if ( !parentActive ){
			return;
		}*/
		QPainter painter(this);
		QVector<QRect> rects = event->region().rects();
		for (int i = 0; i < (int)rects.size(); ++i)
			painter.drawPixmap(rects[i], pixmap, rects[i]);

			//painter.drawPixmap(0, 0, pixmap);
		if (rubberBandIsShown) {
			painter.setPen(zoomColor);
			painter.fillRect(rubberBandRect, Qt::transparent);
			painter.drawRect(rubberBandRect.normalized());
		}
		if ( xcursor.isValid( ) ) {
			painter.setPen(xcursor);
			QLine line( (int) currentCursorPosition.x( ), MARGIN_TOP,
			            (int) currentCursorPosition.x( ), height() - MARGIN_BOTTOM );
			painter.drawLine(line);
		}
		if (xRangeIsShown) {
			painter.setPen(Qt::black);
			int xStart = getPixelX( xRangeStart );
			int xEnd = getPixelX( xRangeEnd );
			xRangeRect.setLeft(xStart);
			xRangeRect.setRight(xEnd);
			xRangeRect.setBottom( MARGIN_TOP );
			xRangeRect.setTop( height() - MARGIN_BOTTOM - 1);
			QColor shadeColor(100, 100, 100, 100);
			if ( !traditionalColors ) {
				shadeColor.setNamedColor( "#B3B3B3");
				shadeColor.setAlpha( 100 );
			}
			painter.fillRect(xRangeRect, shadeColor );
			painter.drawRect(xRangeRect.normalized());
		}

		//Paint the markers
		for ( int i = 0; i < profileFitMarkers.length(); i++ ) {
			profileFitMarkers[i].drawMarker( painter );
		}

		//Paint the annotations
		if ( selectedAnnotation != NULL ) {
			selectedAnnotation->draw( &painter );
		}
		drawAnnotations( painter );
		drawMolecularLines( painter );

		if (hasFocus()) {
			QStyleOptionFocusRect option;
			option.init(this);
			option.backgroundColor = palette().color(QPalette::Background);
			style()->drawPrimitive(QStyle::PE_FrameFocusRect, &option, &painter,
			                       this);
		}
	}

	void QtCanvas::drawMolecularLines( QPainter& painter ) {
		//Paint any molecular lines
		for ( int i = 0; i < static_cast<int>(molecularLineStack.size()); i++ ) {
			double centerVal = molecularLineStack[i]->getCenter();
			int centerPixel = this->getPixelX( centerVal );
			//Use the maximum y value rather than the peak (which needs
			//more information in order to be calculated correctly)
			QtPlotSettings plotSettings = zoomStack[curZoom];
			double maxYVal = plotSettings.getMaxY();
			int peakPixel = this->getPixelY( maxYVal );
			int zeroPixel = this->getRectBottom();
			molecularLineStack[i]->draw( &painter, centerPixel, peakPixel,
			                             zeroPixel, width(), height() );
		}
	}

	void QtCanvas::resizeEvent(QResizeEvent *) {
		refreshPixmap();
	}

	void QtCanvas::setGaussianEstimateCount( int count ){
		gaussEstimateCount = count;
	}


	void QtCanvas::showContextMenu( QMouseEvent* event ) {
		contextMenu.clear();
		if ( taskMode == SPECTRAL_LINE_MODE && gaussEstimateCount > 0 ) {
			//The user is specifying the (Center,Peak) or the FWHM
			//of a Gaussian estimate. The point must be in the selected
			//range to be valid.
			bool valid = storeClickPosition( event );
			if ( valid ) {
				//Show a context menu to determine if the user is specifying
				//the (Center,Peak) or FWHM.
				contextMenu.addAction( centerPeakAction );
				contextMenu.addAction( fwhmAction );
			}

		} else if ( taskMode == LINE_OVERLAY_MODE ) {
			storeClickPosition( event );
			contextMenu.addAction( findRedshiftAction );
		}
		bool annotationFound = isAnnotation( event );
		if ( annotationFound ) {
			resetSelectedAnnotation( event );
			contextMenu.addAction( editAnnotationAction );
			contextMenu.addAction( deleteAnnotationAction );
		} else {
			contextMenu.addAction( createAnnotationAction );
		}
		contextMenu.exec( event->globalPos());
	}

	void QtCanvas::mousePressEvent(QMouseEvent *event) {
		if ( currentMode != NULL ){
			if ( currentMode->isMode(CanvasMode::MODE_CONTEXTMENU)) {
				if ( event->button() == Qt::LeftButton || event->modifiers() != Qt::NoModifier){
					contextMenu.hide();
					currentMode = NULL;
				}
			}
		}
		if ( currentMode == NULL ){
			currentMode = modeFactory->getModeForEvent( event );
		}

		if ( currentMode != NULL ) {
			currentMode->mousePressEvent( event );
		}
	}

	bool QtCanvas::storeClickPosition( QMouseEvent* event ) {
		bool validPosition  = false;
		int xPos = event->pos().x();
		int yPos = event->pos().y();
		gaussianEstimateX = getDataX( xPos );
		gaussianEstimateY = getDataY( yPos );
		if ( xRangeIsShown && xRangeRect.contains( xPos, yPos ) ) {
			validPosition = true;
		} else if ( !xRangeIsShown ) {
			validPosition = true;
		}
		return validPosition;
	}



	void QtCanvas::mouseMoveEvent(QMouseEvent *event) {
		if (showToolTips && !event->buttons() ) {
			displayToolTip( event );
			selectAnnotation( event );
		}
		if ( currentMode != NULL ) {
			currentMode->mouseMoveEvent( event );
		}
	}

	void QtCanvas::storeActiveAnnotation( QMouseEvent* event ) {
		if ( selectedAnnotation != NULL ) {
			selectedAnnotation->setDimensionsPosition( event->pos().x(), event->pos().y() );
			annotations.push_back( selectedAnnotation );
			selectedAnnotation->showEditor();
			selectedAnnotation = NULL;
			update();
		}
	}


	void QtCanvas::mouseReleaseEvent(QMouseEvent *event) {

		if ( currentMode != NULL ) {
			currentMode->mouseReleaseEvent( event );
			currentMode = NULL;
			emit clearPaletteModes();
		}
	}

	void QtCanvas::keyPressEvent(QKeyEvent *event) {
		if (!imageMode)
			switch (event->key()) {
			case Qt::Key_Plus:
				zoomIn();
				break;
			case Qt::Key_Minus:
				zoomOut();
				break;
			case Qt::Key_Left:
				zoomStack[curZoom].scroll(-1, 0);
				refreshPixmap();
				break;
			case Qt::Key_Right:
				zoomStack[curZoom].scroll(+1, 0);
				refreshPixmap();
				break;
			case Qt::Key_Down:
				zoomStack[curZoom].scroll(0, -1);
				refreshPixmap();
				break;
			case Qt::Key_Up:
				zoomStack[curZoom].scroll(0, +1);
				refreshPixmap();
				break;
			case Qt::Key_Escape:
				if (xRangeIsShown) {
					xRangeIsShown=false;
					updatexRangeBandRegion();
					//emit xRangeChanged(1.0,0.0);
				}
				currentMode = NULL;
				break;
#if defined(__APPLE__)
			case Qt::Key_Meta:
#else
			case Qt::Key_Control:
#endif

				update( );
				break;
			default:
				QWidget::keyPressEvent(event);
			}
	}

	void QtCanvas::keyReleaseEvent(QKeyEvent *event) {
		if (!imageMode)
			switch (event->key()) {
			case Qt::Key_Meta:
			case Qt::Key_Control:
				xcursor = QColor( );	// invalid color
				update( );
				break;
			default:
				xcursor = QColor( );	// invalid color
				QWidget::keyPressEvent(event);
			}
	}

	void QtCanvas::wheelEvent(QWheelEvent *event) {
		int numDegrees = event->delta() / 8;
		int numTicks = numDegrees / 15;

		if (event->orientation() == Qt::Horizontal)
			zoomStack[curZoom].scroll(numTicks, 0);
		else
			zoomStack[curZoom].scroll(0, numTicks);
		refreshPixmap();
	}

	void QtCanvas::gaussianCenterPeakSelected() {
		emit specFitEstimateSpecified( gaussianEstimateX, gaussianEstimateY, true );
	}

	void QtCanvas::gaussianFWHMSelected() {
		emit specFitEstimateSpecified( gaussianEstimateX, gaussianEstimateY, false );
	}



	void QtCanvas::findRedshift() {
		emit findRedshiftAt( gaussianEstimateX, gaussianEstimateY );
	}

	void QtCanvas::updateRubberBandRegion() {
		update();
	}

	void QtCanvas::updatexRangeBandRegion() {
		update();
	}

	void QtCanvas::setTopAxisCompatible( bool compatible ){
		bool oldTopAxisCompatible = topAxisCompatible;
		topAxisCompatible = compatible;
		if ( oldTopAxisCompatible != topAxisCompatible ){
			refreshPixmap();
		}
	}

	void QtCanvas::setShowTopAxis( bool showAxis ) {
		bool oldShowAxis = showTopAxis;
		showTopAxis = showAxis;
		if ( oldShowAxis != showTopAxis ) {
			refreshPixmap();
		}
	}

	void QtCanvas::refreshPixmap() {
		pixmap = QPixmap(size());
		pixmap.fill(this, 0, 0);
		QPainter painter(&pixmap);

		drawLabels(&painter);
		if (!imageMode) {
			drawGrid(&painter);
			drawCurves(&painter);
			drawFrameMarker(&painter);
			drawMolecularLines( painter );
			drawAnnotations( painter );
		} else {
			drawTicks( &painter );
			drawBackBuffer(&painter);
		}
		if (welcome.text !="") {
			drawWelcome(&painter);
		}
		update();
	}

	void QtCanvas::drawAnnotations( QPainter& painter ){

		for ( int i = 0; i < static_cast<int>(annotations.size()); i++ ) {
			annotations[i]->draw( &painter );
		}
	}

	void QtCanvas::drawFrameMarker( QPainter* painter ) {
		QRect rect( MARGIN_LEFT, MARGIN_TOP, getRectWidth(), getRectHeight());
		QtPlotSettings settings = zoomStack[curZoom];
		QPen framePen( frameMarkerColor );
		QPen oldPen = painter->pen();
		painter->setPen( framePen );
		int xPixel = getPixelX( framePositionX );
		if ( showFrameMarker ) {
			painter->drawLine(xPixel, rect.top(), xPixel, rect.bottom());
		}
	}



	void QtCanvas::drawBackBuffer(QPainter *painter) {
		QRect rect( MARGIN_LEFT, MARGIN_TOP, getRectWidth(), getRectHeight());
		QtPlotSettings settings = zoomStack[curZoom];
		int minX = (int)(settings.getMinX(QtPlotSettings::xBottom));
		int maxX = (int)(settings.getMaxX(QtPlotSettings::xBottom));
		int maxY = (int)(settings.getMaxY());
		int minY = (int)(settings.getMinY());
		QRect src( minX, minY, maxX, maxY) ;
		painter->drawPixmap(rect, backBuffer, src);
	}

	QString QtCanvas::getXTickLabel( int tickIndex, int tickCount, QtPlotSettings::AxisIndex axisIndex ) const {
		QtPlotSettings settings = zoomStack[curZoom];
		double label = settings.getMinX(axisIndex) + (tickIndex * settings.spanX(axisIndex) / tickCount);
		QString tickLabel = QString::number( label );
		if (abs(label) < ZERO_LIMIT) {
			tickLabel = "0";
		}
		if ( axisIndex == QtPlotSettings::xTop && !showTopAxis ) {
			tickLabel="";
		}
		return tickLabel;
	}

	int QtCanvas::getIndependentCurveCount() {
		int count = 0;
		int masterCount = curveMap.size();
		for ( int i = 0; i < masterCount; i++ ){
			QString title = curveMap[i].getLegend();
			//We don't want to count any fit curves since they are automatically
			//compatible with the primary curve.
			int fitIndex = title.indexOf( "FIT");
			if ( fitIndex < 0 ){
				count++;
			}
		}
		return count;
	}

	int QtCanvas::getLastAxis() {
		int lastAxis = QtPlotSettings::END_AXIS_INDEX;
		//Don't show the top axis if the user has specified NOT to see it OR
		//there are multiple curves which are NOT compatible in their top axis units.
		if ( ! showTopAxis || (!topAxisCompatible && getIndependentCurveCount()>1 )) {
			lastAxis = QtPlotSettings::xTop;
		}
		return lastAxis;
	}

	void QtCanvas::drawGrid(QPainter *painter) {
		QRect rect( MARGIN_LEFT, MARGIN_TOP, getRectWidth(), getRectHeight());
		QtPlotSettings settings = zoomStack[curZoom];
		QPen quiteDark(QPalette::Dark);
		QPen light(QPalette::Highlight);

		int endIndex = getLastAxis();
		for ( int j = 0; j < endIndex; j++ ) {
			QtPlotSettings::AxisIndex axisIndex = static_cast<QtPlotSettings::AxisIndex>(j);
			int xTickCount = settings.getNumTicksX( );
			for (int i = 0; i <= xTickCount; ++i) {
				int x = rect.left() + (i * (rect.width() - 1) / xTickCount );

				if (showGrid && axisIndex == QtPlotSettings::xBottom) {
					painter->setPen(quiteDark);
					painter->drawLine(x, rect.top(), x, rect.bottom());
				}

				//Drawing the tick marks
				painter->setPen(light);
				if ( axisIndex == QtPlotSettings::xBottom ) {
					painter->drawLine(x, rect.bottom(), x, rect.bottom() + 5);
				} else if ( axisIndex == QtPlotSettings::xTop ) {
					painter->drawLine(x, rect.top(), x, rect.top() - 5);
				}

				//Drawing the tick labels
				QString tickLabel = getXTickLabel( i, xTickCount, axisIndex );
				const int AXIS_LABEL_OFFSET = 5;
				int yPosition = rect.bottom() + AXIS_LABEL_OFFSET;
				if ( axisIndex == QtPlotSettings::xTop ) {
					yPosition = rect.top() - 4 * AXIS_LABEL_OFFSET;
				}
				painter->drawText(x - 50, yPosition, 100, 15,
				                  Qt::AlignHCenter | Qt::AlignTop,
				                  tickLabel );
			}
		}
		int tickCountY = settings.getNumTicksY();
		for (int j = 0; j <= tickCountY; ++j) {
			int y = rect.bottom() - (j * (rect.height() - 1) / tickCountY );
			double label = settings.getMinY() + (j * settings.spanY() / tickCountY );
			QString numberLabel = QString::number( label );
			if (abs(label) < ZERO_LIMIT) {
				numberLabel = "0";
			}
			if (showGrid) {
				painter->setPen(quiteDark);
				painter->drawLine(rect.left(), y, rect.right(), y);
			}
			painter->setPen(light);
			painter->drawLine(rect.left() - 5, y, rect.left(), y);
			painter->drawText(rect.left() - 2*MARGIN_LEFT/3, y - 10,
			                  MARGIN_LEFT/2, 20, Qt::AlignRight | Qt::AlignVCenter, numberLabel);
		}
		painter->drawRect(rect);

	}
	void QtCanvas::drawTicks(QPainter *painter) {
		QtPlotSettings settings = zoomStack[curZoom];
		QPen quiteDark(QPalette::Dark);
		QPen light(QPalette::Highlight);
		int endIndex = getLastAxis();
		for ( int j = 0; j < endIndex; j++ ) {
			int startY = MARGIN_TOP;
			if ( j == QtPlotSettings::xTop ) {
				startY = height() - MARGIN_TOP;
			}
			QRect rect( MARGIN_LEFT, startY, getRectWidth(), getRectHeight());
			QtPlotSettings::AxisIndex axisIndex = static_cast<QtPlotSettings::AxisIndex>(j);
			int tickCountX = settings.getNumTicksX( /*axisIndex*/ );
			for (int i = 0; i <= tickCountX; ++i) {
				int x = rect.left() + (i * (rect.width() - 1) / tickCountX );
				QString tickLabel = getXTickLabel( i, tickCountX, axisIndex );
				painter->setPen(quiteDark);
				painter->drawLine(x, rect.top(), x, rect.top() + 5);
				painter->setPen(light);
				painter->drawLine(x, rect.bottom() - 5, x, rect.bottom());
				painter->drawText(x - 50, rect.bottom() + 5, 100, 15,
				                  Qt::AlignHCenter | Qt::AlignTop,
				                  tickLabel );
			}
		}

		QRect rect( MARGIN_LEFT, MARGIN_TOP, getRectWidth(), getRectHeight());
		int tickCountY = settings.getNumTicksY();
		for (int j = 0; j <= tickCountY; ++j) {
			int y = rect.bottom() - (j * (rect.height() - 1) / tickCountY );
			double label = settings.getMinY() + (j * settings.spanY() / tickCountY );
			QString numberLabel = QString::number(label);
			if (abs(label) < ZERO_LIMIT) {
				numberLabel = "0";
			}
			painter->setPen(quiteDark);
			painter->drawLine(rect.right() - 5, y, rect.right(), y);
			painter->setPen(light);
			painter->drawLine(rect.left(), y, rect.left() + 5, y);
			painter->drawText( rect.left() - MARGIN_LEFT / 2, y - 10,
			                   MARGIN_LEFT - 5, 20, Qt::AlignRight | Qt::AlignVCenter, numberLabel );
		}
		painter->drawRect(rect);
	}

	void QtCanvas::drawLabels(QPainter *painter) {
		QFont ft(painter->font());
		QPen pen(painter->pen());

		painter->setPen(getDiscreteColor( TITLE_COLOR ));
		QFont titleFont(title.fontName, title.fontSize);
		titleFont.setBold( true );
		painter->setFont( titleFont );
		painter->drawText( MARGIN_LEFT, 15, getRectWidth(), MARGIN_TOP / 2,
		                   Qt::AlignHCenter | Qt::AlignTop, title.text);

		int endIndex = getLastAxis();
		for ( int i = 0; i < endIndex; i++ ) {
			QtPlotSettings::AxisIndex axisIndex = static_cast<QtPlotSettings::AxisIndex>(i);
			painter->setPen(xLabel[axisIndex].color);
			QFont axisLabelFont(xLabel[axisIndex].fontName, xLabel[axisIndex].fontSize);
			axisLabelFont.setBold( true );
			painter->setFont( axisLabelFont );
			int yPosition = height() - MARGIN_BOTTOM / 2;
			QString labelText = xLabel[axisIndex].text;
			if ( axisIndex == QtPlotSettings::xTop ) {
				yPosition = MARGIN_TOP / 2;
			}
			painter->drawText(MARGIN_LEFT, yPosition, getRectWidth(), MARGIN_TOP / 2,
			                  Qt::AlignHCenter | Qt::AlignTop, labelText );
		}
		QFont font(yLabel.fontName, yLabel.fontSize);
		font.setBold( true );
		QRect fontBoundingRect = QFontMetrics(font).boundingRect(yLabel.text);
		font.setPixelSize(50);
		painter->rotate(-90);
		painter->drawText(-height(), 0, height(), MARGIN_LEFT/2, Qt::AlignHCenter|Qt::AlignTop, yLabel.text);
		painter->rotate(90);

		painter->setPen(pen);
		painter->setFont(ft);
	}

	void QtCanvas::drawWelcome(QPainter *painter) {
		QFont ft(painter->font());
		QPen pen(painter->pen());

		painter->setPen(welcome.color);
		painter->setFont(QFont(title.fontName, welcome.fontSize));
		painter->drawText( MARGIN_LEFT, MARGIN_TOP,
		                   getRectWidth(), getRectHeight(),
		                   Qt::AlignHCenter | Qt::AlignVCenter,
		                   welcome.text);
		painter->setPen(pen);
		painter->setFont(ft);
	}



	void QtCanvas::setTraditionalColors( bool traditionalColors ) {
		this->traditionalColors = traditionalColors;
	}



	void QtCanvas::setTraditionalCurveColors( const QList<QString>& colors ) {
		traditionalCurveColorList = colors;
	}

	void QtCanvas::setMainCurveColors( const QList<QString>& colors ) {
		mainCurveColorList = colors;
	}

	void QtCanvas::setFitCurveColors( const QList<QString>& colors ) {
		fitCurveColorList = colors;
	}

	void QtCanvas::setSummaryCurveColors( const QList<QString>& colors ) {
		fitSummaryCurveColorList = colors;
	}

	void QtCanvas::curveColorsChanged() {
		for ( int i = 0; i < static_cast<int>(curveMap.size()); i++ ) {
			ColorCategory level = static_cast<ColorCategory>(curveMap[i].getCurveType());
			QColor newCurveColor = getDiscreteColor( level, i );
			curveMap[i].setColor( newCurveColor );
		}
		refreshPixmap();
	}


	QColor QtCanvas::getDiscreteColor(ColorCategory colorCategory, int id ) {
		QColor color;
		if ( colorCategory == TITLE_COLOR ) {
			color = Qt::black;
		} else if ( colorCategory == CURVE_TRADITIONAL || traditionalColors ) {
			int index = id % traditionalCurveColorList.size();
			color.setNamedColor(traditionalCurveColorList[index]);
		} else if ( colorCategory == CURVE_COLOR ) {
			int mainColorCount = mainCurveColorList.size();
			if ( mainColorCount > 0 ) {
				int index = curveCount % mainColorCount;
				color.setNamedColor(mainCurveColorList[index]);
			} else {
				color = Qt::black;
			}
			curveCount++;
		} else if ( colorCategory == CURVE_COLOR_SECONDARY ) {
			int summaryColorCount = fitSummaryCurveColorList.size();
			if ( summaryColorCount > 0 ) {
				int index = curveCountSecondary % summaryColorCount;
				color.setNamedColor( fitSummaryCurveColorList[index]);
			} else {
				color = Qt::black;
			}
			curveCountSecondary++;
		} else if ( colorCategory == CURVE_COLOR_PRIMARY ) {
			int fitCount = fitCurveColorList.size();
			if ( fitCount > 0 ) {
				int index = curveCountPrimary % fitCount;
				color.setNamedColor( fitCurveColorList[index]);
			} else {
				color = Qt::black;
			}
			curveCountPrimary++;
		}
		return color;
	}

	void QtCanvas::addDiamond( int x, int y, int diamondSize, QPainterPath& points ) const {
		// plots a diamond from
		// primitive lines to have a 2D item
		for (int i = 0; i < diamondSize; ++i) {
			// closes the centre
			if (i==1) {
				points.lineTo(x + i, y);
			} else {
				points.moveTo(x + i, y);
			}
			points.lineTo(x, y - i);
			points.lineTo(x - i, y);
			points.lineTo(x, y + i);
			points.lineTo(x + i, y);
		}
	}

	void QtCanvas::drawCurves(QPainter *painter) {

		QFont ft(painter->font());
		QPen pen(painter->pen());
		QColor defaultColor = pen.color();

		QtPlotSettings settings = zoomStack[curZoom];
		QRect rect( MARGIN_LEFT, MARGIN_TOP, getRectWidth(), getRectHeight());
		painter->setClipRect(rect.x() + 1, rect.y() + 1,
		                     rect.width() - 2, rect.height() - 2);

		std::map<int, CanvasCurve>::iterator it = curveMap.begin();
		int siz = curveMap.size();
		int sz = siz > 1 ? siz : 1;
		QColor *colorFolds = new QColor[sz];
		const int MAX_PIXEL = 32768;
		while (it != curveMap.end()) {
			int id = (*it).first;
			const CurveData &data = (*it).second.getCurveData();
			const ErrorData &error =(*it).second.getErrorData();

			int maxPoints = data.size() / 2;
			QPainterPath points;

			// get a colour
			colorFolds[id] = (*it).second.getColor();

			if (maxPoints == 1) {
				int x = getPixelX( data[0] );
				int y = getPixelY( data[1] );
				if (fabs(x) < MAX_PIXEL && fabs(y) < MAX_PIXEL) {
					addDiamond( x, y, QT_DIAMOND_SIZE, points );
				}

				if (plotError && (error.size() > 0)) {

					double dx = data[0] - settings.getMinX(QtPlotSettings::xBottom);
					double dy = data[1] - settings.getMinY();
					double de = error[0];

					double x = rect.left() + (dx * (rect.width() - 1)
					                          / settings.spanX(QtPlotSettings::xBottom));
					double yl = rect.bottom() - ((dy-de) * (rect.height() - 1)
					                             / settings.spanY());
					double yu = rect.bottom() - ((dy+de) * (rect.height() - 1)
					                             / settings.spanY());

					if (fabs(x) < MAX_PIXEL && fabs(yl) < MAX_PIXEL && fabs(yu) < MAX_PIXEL) {
						points.moveTo((int)x, (int)yl);
						points.lineTo((int)x, (int)yu);
					}
				}

			} else {
				for (int i = 0; i < maxPoints; ++i) {
					int x = getPixelX( data[2*i] );
					int y = getPixelY( data[2*i+1] );
					if (fabs(x) < 32768 && fabs(y) < 32768) {
						if (i == 0) {
							points.moveTo(x, y);
						} else {
							if (displayStepFunction ) {
								int prevX = getPixelX( data[2*(i-1)] );
								int prevY = getPixelY( data[2*i-1] );
								int midX = (prevX + x )/2;
								points.moveTo(prevX, prevY);
								points.lineTo( midX, prevY);
								points.lineTo( midX, y);
								points.lineTo( x, y );
								//Plot the last point
								if ( i == maxPoints - 1) {
									addDiamond( x, y, 2, points );
								}
							} else {
								points.lineTo(x, y);
							}
						}
					}
				}

				if (plotError && (error.size() > 0)) {
					for (int i = 0; i < maxPoints; ++i) {
						double dx = data[2 * i] - settings.getMinX(QtPlotSettings::xBottom);
						double dy = data[2 * i + 1] - settings.getMinY();
						double de = error[i];
						double x = rect.left() + (dx * (rect.width() - 1)
						                          / settings.spanX(QtPlotSettings::xBottom));
						double yl = rect.bottom() - ((dy-de) * (rect.height() - 1)
						                             / settings.spanY());
						double yu = rect.bottom() - ((dy+de) * (rect.height() - 1)
						                             / settings.spanY());

						if (fabs(x) < MAX_PIXEL && fabs(yl) < MAX_PIXEL && fabs(yu) < MAX_PIXEL) {
							points.moveTo((int)x, (int)yl);
							points.lineTo((int)x, (int)yu);
						}
					}
				}
			}

			pen.setColor( colorFolds[id]);
			painter->setPen( pen );
			painter->drawPath(points);

			if (siz > 1 && showLegend && legendPosition == 0) {
				QFont curveLabelFont(xLabel[QtPlotSettings::xBottom].fontName, xLabel[QtPlotSettings::xBottom].fontSize);
				painter->setFont( curveLabelFont);
				painter->drawText(MARGIN_LEFT+4, MARGIN_TOP + (5 +id*15),
				                  getRectWidth(), MARGIN_TOP / 2,
				                  Qt::AlignLeft | Qt::AlignTop, (*it).second.getLegend());
			}
			++it;
		}

		delete [] colorFolds;

		pen.setColor( defaultColor );
		painter->setPen(pen);
		painter->setFont(ft);
	}





	void QtCanvas::addPolyLine(const Vector<Float> &x,
	                           const Vector<Float> &y,
	                           double beamAngle, double beamArea,
	                           SpectralCoordinate coord,
	                           const QString& lb, ColorCategory colorCategory) {
		Int xl, yl;
		x.shape(xl);
		y.shape(yl);
		CurveData data;
		for (uInt i = 0; i < (uInt)min(xl, yl); i++) {
			data.push_back(x[i]);
			data.push_back(y[i]);
		}

		int j = curveMap.size();
		setCurveData(j, data, beamAngle, beamArea, coord, ErrorData(), lb, colorCategory );

		setDataRange();

	}

	void QtCanvas::plotPolyLines(QString path) {
		QFile file(path);

		if (file.open(QIODevice::ReadOnly)) {
			QTextStream in(&file);
			int lineCount = 0;
			int pointCount = 0;
			CurveData data;
			while (!in.atEnd()) {
				QString line = in.readLine();
				if (!line.isNull() && !line.isEmpty()) {
					if (line.startsWith('#')) {
						if (line.contains("title"))
							setTitle(line.mid(7));
						if (line.contains("xLabel"))
							setXLabel(line.mid(8));
						if (line.contains("yLabel"))
							setYLabel(line.mid(8));
					} else {
						QStringList coords = line.trimmed().split(QRegExp("\\s+"));
						if (coords.size() >= 1) {
							double x = coords[0].toDouble();
							double y = coords[1].toDouble();
							data.push_back(x);
							data.push_back(y);
							pointCount++;
						}
						if (in.atEnd() && pointCount > 0) {
							setCurveData(lineCount, data, 0, 0, SpectralCoordinate());
						}
					}
				} else {
					if (pointCount > 0) {
						setCurveData(lineCount, data, 0, 0, SpectralCoordinate());
						pointCount = 0;
						lineCount++;
						data.clear();
					}
				}

			}
		}

		setDataRange();
		return;
	}
	void QtCanvas::plotPolyLine(const Vector<Float> &x, const Vector<Float> &y, const Vector<Float> &e,
	                            double beamAngle, double beamArea,
	                            SpectralCoordinate coord, const QString& lb) {
		//for (int i=0; i< x.nelements(); i++)
		//cout << x(i) << " " << y(i) << endl;
		Int xl, yl, el;
		x.shape(xl);
		y.shape(yl);
		e.shape(el);
		CurveData data;
		ErrorData error;
		for (uInt i = 0; i < (uInt)min(xl, yl); i++) {
			data.push_back(x[i]);
			data.push_back(y[i]);
		}

		for (uInt i = 0; i < (uint)el; i++)
			error.push_back(e[i]);

		setCurveData(0, data, beamAngle, beamArea, coord, error, lb);

		setDataRange();
		return;
	}


	template<class T> void QtCanvas::plotPolyLine(const Vector<T> &x, const Vector<T>&y,
			double beamAngle, double beamArea, SpectralCoordinate coord) {

		Int xl, yl;
		x.shape(xl);
		y.shape(yl);
		CurveData data;
		for (Int i = 0; i < min(xl, yl); i++) {
			data.push_back(x[i]);
			data.push_back(y[i]);
		}
		setCurveData(0, data, beamAngle, beamArea, coord);

		setDataRange();
		return;
	}

	template<class T> void QtCanvas::plotPolyLine(const Matrix<T> &x,
			double beamAngle, double beamArea, SpectralCoordinate coord, double restValue ) {

		Int xl, yl;
		x.shape(xl, yl);
		int nr = xl / 2;
		int nc = yl / 2;
		int n = min (nr, nc);
		if (n > 0) {
			// CurveData *data = new CurveData[n];
			std::vector<CurveData> data;
			if (n < nr) {
				for (int i = 0; i < n; i++) {
					for (int j = 0; j < xl; j++) {
						data[i].push_back(x(uInt(j), uInt(2 * i)));
						data[i].push_back(x(uInt(j), uInt(2 * i + 1)));
					}
					setCurveData(i, data[i], beamAngle, beamArea, coord);
				}
			} else {
				for (int i = 0; i < n; i++) {
					for (int j = 0; j < yl; j++) {
						data[i].push_back(x(2 * i, j));
						data[i].push_back(x(2 * i + 1, j));
					}
					setCurveData(i, data[i], beamAngle, beamArea, coord);
				}
			}
			//delete [] data;
		}
		setDataRange();
		return;
	}

	void QtCanvas::setImageMode(bool b) {
		imageMode = b;
	}

	void QtCanvas::increaseCurZoom() {
		curZoom++;

	}
	int QtCanvas::getCurZoom() {
		return curZoom;
	}
	int QtCanvas::getZoomStackSize() {
		return (int)zoomStack.size();
	}
	void QtCanvas::setTitle(const QString &text, int fontSize, const QString &font) {
		title.text = text;
		title.fontName = font;
		title.fontSize = fontSize;
		title.color = getDiscreteColor(TITLE_COLOR);
	}
	void QtCanvas::setXLabel(const QString &text, int fontSize, const QString &font, QtPlotSettings::AxisIndex axisIndex ) {
		xLabel[axisIndex].text = text;
		xLabel[axisIndex].fontName = font;
		xLabel[axisIndex].fontSize = fontSize;
		xLabel[axisIndex].color = getDiscreteColor(TITLE_COLOR);
	}
	void QtCanvas::setYLabel(const QString &text, int fontSize, const QString &font) {
		yLabel.text = text;
		yLabel.fontName = font;
		yLabel.fontSize = fontSize;
		yLabel.color = getDiscreteColor(TITLE_COLOR);
	}
	void QtCanvas::setWelcome(const QString &text, int fontSize, const QString &font) {
		welcome.text = text;
		welcome.fontName = font;
		welcome.fontSize = fontSize;
		welcome.color = getDiscreteColor(WARNING_COLOR);
	}
	QPixmap* QtCanvas::graph() {
		refreshPixmap();
		return &pixmap;
	}

	void QtCanvas::adjustExtremes( double* const min, double* const max) const {
		const float BOUND = 0.0001f;
		const double ADJUSTMENT = 0.00001f;
		if ( fabs(*max - *min) < BOUND ) {
			//Calculate the actual adjustment based on the magnitude of the
			//difference.
			float diff = fabs(*max - *min );
			double scaleFactor = 1;
			if ( diff > 0 ) {
				while ( diff < 1 ) {
					diff = diff * 10;
					scaleFactor = scaleFactor / 10;
				}
			} else {
				//The values are equal so base the difference on the
				//same scale as the value.
				float value = *min;
				while( value > 0 && value < 1 ) {
					value = value * 10;
					scaleFactor = scaleFactor / 10;
				}
			}
			double actualAdjustment = std::min( ADJUSTMENT, scaleFactor );
			*max = *max + actualAdjustment;
			*min = *min - actualAdjustment;
		}
	}

	ProfileFitMarker QtCanvas::getMarker( int index ) const {
		ProfileFitMarker marker( this );
		if ( index < profileFitMarkers.length() ) {
			marker = profileFitMarkers[index];
		}
		return marker;
	}

	void QtCanvas::setMarker( int index, ProfileFitMarker& marker ) {
		if ( index < profileFitMarkers.length() ) {
			profileFitMarkers[index] = marker;
		} else {
			profileFitMarkers.append( marker );
		}
	}

	void QtCanvas::setProfileFitMarkerCenterPeak( int index, double center, double peak ) {
		ProfileFitMarker marker = getMarker( index );
		marker.setCenterPeak( center, peak );
		setMarker( index, marker );
		update();
	}

	void QtCanvas::setProfileFitMarkerFWHM( int index, double fwhm, double fwhmHeight ) {
		ProfileFitMarker marker = getMarker( index );
		marker.setFWHM( fwhm, fwhmHeight );
		setMarker( index, marker );
		update();
	}

	bool QtCanvas::isDisplayStepFunction() const {
		return displayStepFunction;
	}

	void QtCanvas::setDisplayStepFunction( bool displayAsStepFunction ) {
		bool oldDisplayStepFunction = displayStepFunction;
		displayStepFunction = displayAsStepFunction;
		if ( oldDisplayStepFunction != displayStepFunction ) {
			refreshPixmap();
		}
	}

	void QtCanvas::setShowLegend( bool visible ) {
		bool oldVisible = showLegend;
		showLegend = visible;
		if ( oldVisible != visible ) {
			refreshPixmap();
		}
	}

	bool QtCanvas::isShowLegend() const {
		return showLegend;
	}

	void QtCanvas::setLegendPosition( int position ) {
		int oldPosition = legendPosition;
		legendPosition = position;
		if ( oldPosition != legendPosition ) {
			refreshPixmap();
		}
	}

	bool QtCanvas::isShowChannelLine() {
		return showFrameMarker;
	}

	void QtCanvas::setShowChannelLine( bool showLine ) {
		bool oldShowFrameMarker = showFrameMarker;
		showFrameMarker = showLine;
		if ( oldShowFrameMarker != showFrameMarker ) {
			refreshPixmap();
		}
	}

	void QtCanvas::setChannelLineColor( QColor color ) {
		frameMarkerColor = color;
	}

	void QtCanvas::setZoomRectColor( QColor color ) {
		zoomColor = color;
	}

	int QtCanvas::getLegendPosition() const {
		return legendPosition;
	}

	void QtCanvas::curveLabelsChanged() {
		refreshPixmap();
	}

	void QtCanvas::setFrameMarker( float framePosX ) {
		float oldFramePosX = framePositionX;
		framePositionX = framePosX;
		if ( oldFramePosX != framePositionX ) {
			refreshPixmap();
		}
	}

	void QtCanvas::addMolecularLine( MolecularLine* line ) {
		bool newLine = true;
		for ( int i = 0; i < molecularLineStack.size(); i++ ) {
			if ( molecularLineStack[i]->equalTo( line )) {
				newLine = false;
				break;
			}
		}
		if ( newLine ) {
			molecularLineStack.append( line );
			refreshPixmap();
		}
	}

	void QtCanvas::changeTaskMode( int mode ) {
		if ( 0 <= mode && mode < MODE_COUNT ) {
			taskMode = static_cast<TaskMode>(mode);
		} else {
			qDebug()<<"Changing task mode to an unsupported mode: "<< mode;
		}
	}

	QList<QString> QtCanvas::getMolecularLineNames() const {
		QList<QString> nameList;
		for ( int i = 0; i < molecularLineStack.size(); i++ ) {
			QString name = molecularLineStack[i]->getLabel();
			nameList.append( name );
		}
		return nameList;
	}

	QList<MolecularLine*> QtCanvas::getMolecularLines() const {
		return molecularLineStack;
	}

	void QtCanvas::setImageYUnits( const QString& imageUnits ) {
		QString oldUnits = this->yUnitImage;
		yUnitImage = imageUnits;
		if ( yUnitDisplay.length() == 0 ) {
			setDisplayYUnits( yUnitImage );
		} else if ( oldUnits != yUnitImage ) {
			//The curves are all in the old image units and would not
			//be valid.
			clearCurve();
		}
	}

	void QtCanvas::setDisplayYUnits( const QString& displayUnits ) {
		QString oldDisplayUnits = this->yUnitDisplay;
		yUnitDisplay = displayUnits;
		setYLabel( "("+yUnitDisplay+")");
		if ( oldDisplayUnits != yUnitDisplay && oldDisplayUnits.length() > 0  ) {
			//Tell all the curves to convert their yUnits
			for ( int i = 0; i < static_cast<int>(this->curveMap.size()); i++ ) {
				curveMap[i].scaleYValues( oldDisplayUnits, yUnitDisplay, getUnits() );
			}
			//We won't bother to scale the curve markers.
			profileFitMarkers.clear();
			setDataRange();
		}
	}

	QString QtCanvas::getDisplayYUnits() {
		return this->yUnitDisplay;
	}

	CanvasCurve QtCanvas::getCurve( const QString& curveName ) {
		CanvasCurve target;
		target.setLegend( "Not Found");
		for ( int i = 0; i < static_cast<int>(curveMap.size()); i++ ) {
			if ( curveName == curveMap[i].getLegend() ) {
				target = curveMap[i];
				break;
			}
		}
		return target;
	}

	void QtCanvas::rangeSelectionMode() {
		currentMode = modeFactory->getMode( CanvasMode::MODE_RANGESELECTION );
	}

	void QtCanvas::channelPositioningMode() {
		currentMode = modeFactory->getMode( CanvasMode::MODE_CHANNEL );
	}

	void QtCanvas::createAnnotationText() {
		currentMode = modeFactory->getMode( CanvasMode::MODE_ANNOTATION );
		emit togglePalette( CanvasMode::MODE_ANNOTATION );
	}

	bool QtCanvas::isAnnotationActive() const {
		bool annotationActive = false;
		if ( selectedAnnotation != NULL ) {
			annotationActive = true;
		}
		return annotationActive;
	}
	Annotation* QtCanvas::getActiveAnnotation() const {
		return selectedAnnotation;
	}
	void QtCanvas::selectChannel( QMouseEvent* event ) {
		xcursor = this->frameMarkerColor;
		if ( xcursor.isValid( ) ) {
			QtPlotSettings currSettings = zoomStack[curZoom];
			double dx = currSettings.spanX(QtPlotSettings::xBottom) / getRectWidth();
			channelSelectValue = currSettings.getMinX(QtPlotSettings::xBottom ) + dx * (event->pos().x()-MARGIN_LEFT);
			emit channelSelect(channelSelectValue);
		}
	}

	void QtCanvas::updateChannel( QMouseEvent* event ) {
		currentCursorPosition = event->pos( );
		if ( xcursor.isValid( )   ) {
			update( );
		}
		if ( currentMode != NULL &&
				currentMode->isMode( CanvasMode::MODE_CHANNEL) ){
			QtPlotSettings currSettings = zoomStack[curZoom];
			double dx = currSettings.spanX(QtPlotSettings::xBottom) /
					getRectWidth();
			float endChannelSelectValue = currSettings.getMinX(QtPlotSettings::xBottom ) + dx * (event->pos().x()-MARGIN_LEFT);
			emit channelRangeSelect(channelSelectValue, endChannelSelectValue );
			channelSelectValue = endChannelSelectValue;
		}
	}


	void QtCanvas::startRangeX( QMouseEvent* event ) {
		xRangeMode    = true;
		xRangeIsShown = true;
		xRangeRect.setLeft(event->pos().x());
		xRangeRect.setRight(event->pos().x());
		xRangeRect.setBottom(MARGIN_TOP);
		xRangeRect.setTop( height() - MARGIN_BOTTOM - 1);
		xRectStart= event->pos().x();
		xRectEnd  = event->pos().x();
		updatexRangeBandRegion();
	}

	void QtCanvas::getCanvasDomain( double* minValue, double* maxValue,
	                                QString& units) {

		QtPlotSettings currSettings = zoomStack[curZoom];
		*minValue = currSettings.getMinX( QtPlotSettings::xBottom );
		*maxValue = currSettings.getMaxX( QtPlotSettings::xBottom );
		if ( *minValue > *maxValue ) {
			double tmp = *minValue;
			*minValue = *maxValue;
			*maxValue = tmp;
		}
		units = this->getUnits();
	}

	void QtCanvas::updateRangeX( QMouseEvent* event ) {
		if (xRangeIsShown) {
			updatexRangeBandRegion();
			xRectEnd = event->pos().x();
			QtPlotSettings currSettings = zoomStack[curZoom];
			double dx = currSettings.spanX(QtPlotSettings::xBottom) / static_cast<double>(getRectWidth());
			double currMinX = currSettings.getMinX(QtPlotSettings::xBottom);
			xRangeStart = currMinX + dx * ( xRectStart - MARGIN_LEFT);
			xRangeEnd = currMinX + dx * ( xRectEnd - MARGIN_LEFT );
			if ( xRangeStart < xRangeEnd ) {
				emit xRangeChanged( xRangeStart, xRangeEnd);
			} else {
				emit xRangeChanged( xRangeEnd, xRangeStart);
			}
			updatexRangeBandRegion();
		}
	}

	void QtCanvas::endRangeX( QMouseEvent* /*event*/ ) {
		QRect rect = xRangeRect.normalized();

		if (rect.left() < 0 || rect.top() < 0 ||
		        rect.right() > width() ||
		        rect.bottom() > height())
			return;

		if (rect.width() < 4) {
			xRangeIsShown=false;
			return;
		}

		// zero the coordinates on the plot region
		QtPlotSettings currSettings = zoomStack[curZoom];
		double dx = currSettings.spanX(QtPlotSettings::xBottom) / getRectWidth();
		double currMinX = currSettings.getMinX( QtPlotSettings::xBottom );
		xRangeStart = currMinX + dx * (xRectStart - MARGIN_LEFT);
		xRangeEnd = currMinX + dx * (xRectEnd - MARGIN_LEFT );
		if ( xRangeStart< xRangeEnd ) {
			emit xRangeChanged( xRangeStart, xRangeEnd);
		} else {
			emit xRangeChanged(xRangeEnd, xRangeStart);
		}
		xRangeMode=false;
	}

	void QtCanvas::startZoomRect( QMouseEvent* event ) {
		xRangeIsShown     = false;
		rubberBandIsShown = true;
		rubberBandRect.setTopLeft(event->pos());
		rubberBandRect.setBottomRight(event->pos());
		updateRubberBandRegion();
	}

	void QtCanvas::updateZoomRect( QMouseEvent* event ) {
		if (rubberBandIsShown) {
			updateRubberBandRegion();
			rubberBandRect.setBottomRight(event->pos());
			updateRubberBandRegion();
		}
	}

	void QtCanvas::endZoomRect( QMouseEvent* /*event*/ ) {
		rubberBandIsShown = false;
		updateRubberBandRegion();

		QRect rect = rubberBandRect.normalized();
		//        //zoom only if zoom box is in the plot region
		if (rect.left() < 0 || rect.top() < 0 ||
		        rect.right() > width() ||
		        rect.bottom() > height()) {
			return;
		}

		if (rect.width() < 4 || rect.height() < 4) {
			return;
		}
		// zero the coordinates on the plot region
		rect.translate( -1 * MARGIN_LEFT, -1 * MARGIN_TOP);

		QtPlotSettings prevSettings = zoomStack[curZoom];
		QtPlotSettings settings;
		for ( int i = 0; i < QtPlotSettings::END_AXIS_INDEX; i++ ) {
			QtPlotSettings::AxisIndex axisIndex = static_cast<QtPlotSettings::AxisIndex>(i);
			double dx = prevSettings.spanX(axisIndex) / getRectWidth();
			double prevMinX = prevSettings.getMinX( axisIndex );
			settings.setMinX( axisIndex, prevMinX + dx * rect.left() );
			settings.setMaxX( axisIndex, prevMinX + dx * rect.right() );
		}
		double dy = prevSettings.spanY() / (getRectHeight());
		double prevMaxY = prevSettings.getMaxY();
		settings.setMinY( prevMaxY - dy * rect.bottom() );
		settings.setMaxY( prevMaxY - dy * rect.top() );
		//qDebug() << "min-x: " << settings.minX << " max-x: " << settings.maxX;
		settings.adjust( getUnits(QtPlotSettings::xTop), getUnits( QtPlotSettings::xBottom), autoScaleX, autoScaleY );
		if ( curveMap.size() != 0) {
			zoomStack.resize(curZoom + 1);
			zoomStack.push_back(settings);
			zoomIn();
		}
	}

	void QtCanvas::setAutoScaleX(bool autoScale) {
		bool oldAutoScale = autoScaleX;
		if ( oldAutoScale != autoScale ) {
			autoScaleX = autoScale;
			setDataRange();
		}
	}

	void QtCanvas::setAutoScaleY(bool autoScale) {
		bool oldAutoScale = autoScaleY;
		if ( oldAutoScale != autoScale ) {
			autoScaleY = autoScale;
			setDataRange();
		}
	}
}

