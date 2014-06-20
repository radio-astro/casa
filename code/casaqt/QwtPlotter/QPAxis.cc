#include <stdio.h>
//# QPAxis.cc: Qwt implementation of generic PlotCanvas class.
//# Copyright (C) 2008
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
//# $Id: $
#ifdef AIPS_HAS_QWT

#include <casaqt/QwtPlotter/QPAxis.qo.h>
#include <casaqt/QwtPlotter/QPCanvas.qo.h>
#include <casaqt/QtUtilities/QtLayeredLayout.h>
#include <casaqt/QtUtilities/Axis/ExternalAxisWidgetLeft.h>
#include <casaqt/QtUtilities/Axis/ExternalAxisWidgetRight.h>
#include <casaqt/QtUtilities/Axis/ExternalAxisWidgetTop.h>
#include <casaqt/QtUtilities/Axis/ExternalAxisWidgetBottom.h>
#include <casaqt/QwtPlotter/QPAnnotation.h>
#include <casaqt/QwtPlotter/QPFactory.h>
#include <casaqt/QwtPlotter/QPPlotter.qo.h>
#include <casaqt/QwtPlotter/QPRasterPlot.h>
#include <casaqt/QwtPlotter/QPShape.h>

#include <qwt_scale_widget.h>

namespace casa {

//////////////////////////
// QPAXIS DEFINITIONS //
//////////////////////////



// Static //

double QPAxis::zOrder = 1;

const String QPAxis::CLASS_NAME = "QPAxis";
const String QPAxis::DRAW_NAME = "drawItems";


bool QPAxis::print( QPrinter& /*printer*/ ){

	 return false;
}

const QPalette& QPAxis::palette() const {
	return axisWidget->palette();
}

QPalette::ColorRole QPAxis::backgroundRole() const {
	return axisWidget->backgroundRole();
}

bool QPAxis::print(  QPainter* painter, PlotAreaFillPtr paf, double widgetWidth,
		double widgetHeight, int axisWidth, int axisHeight,
		int rowIndex, int colIndex, QRect /*imageRect*/ ){
	PlotAreaFillPtr originalBackground = background();
	setBackground(*paf);

	QRect printGeom;

	if ( isVertical()){
		int xPosition = static_cast<int>(colIndex * widgetWidth );
		int yPosition = static_cast<int>(rowIndex * widgetHeight + axisHeight);
		printGeom = QRect( xPosition, yPosition, axisWidth,
					static_cast<int>(widgetHeight));
	}
	else {
		int xPosition = static_cast<int>(colIndex * widgetWidth + axisWidth);
		int yPosition = static_cast<int>(rowIndex * widgetHeight);
		printGeom = QRect( xPosition, yPosition, static_cast<int>(widgetWidth),
								axisHeight);
	}

	PlotOperationPtr op = operationExport();
	bool wasCanceled = false;
	if (!op.null()){
		wasCanceled |= op->cancelRequested();
	}

	if (!wasCanceled && axisWidget != NULL ){
	    axisWidget->print(painter, printGeom);
	    painter->drawRect( printGeom );

	    setBackground(*originalBackground);

	    if (!op.null()) wasCanceled |= op->cancelRequested();
	}
	return wasCanceled;
}


QImage QPAxis::grabImageFromCanvas(
        const PlotExportFormat& format )   {
	QImage image;
    /*image = QPixmap::grabWidget(grabCanvas != NULL ?
            &grabCanvas->asQwtPlot() :
            grabPlotter->canvasWidget()).toImage();*/
	image = QPixmap::grabWidget( axisWidget ).toImage();



    // Scale to set width and height, if applicable.
    if (format.width > 0 && format.height > 0) {
        // size is specified
        image = image.scaled(format.width, format.height,
                Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    } else if (format.width > 0) {
        // width is specified
        image = image.scaledToWidth(format.width,
                Qt::SmoothTransformation);

    } else if (format.height > 0) {
        //  height is specified
        image = image.scaledToHeight(format.height,
                Qt::SmoothTransformation);
	}
    return image;
}


unsigned int QPAxis::axisIndex(PlotAxis a) {
    switch(a) {
    case X_BOTTOM: return 0;
    case X_TOP: return 1;
    case Y_LEFT: return 2;
    case Y_RIGHT: return 3;
    default: return 0;
    }
}   



PlotAxis QPAxis::axisIndex(unsigned int i) {
    switch(i) {
    case 0: return X_BOTTOM;
    case 1: return X_TOP;
    case 2: return Y_LEFT;
    case 3: return Y_RIGHT;
    default: return X_BOTTOM;
    }
}


// Constructors/Destructors //

QPAxis::QPAxis(PlotAxis plotAxis, QPPlotter* parent, QwtPlot* associatedPlot,
		bool leftAxisInternal, bool bottomAxisInternal,
		bool rightAxisInternal ) : m_parent(parent),
		// m_canvas(NULL),
        m_axesRatioLocked(false), m_axesRatios(4, 1),
        axisWidget( NULL ){
    logObject(CLASS_NAME, this, true);

    axisType = QPOptions::axis(plotAxis);
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    setFocusPolicy(Qt::StrongFocus);
    
    QSizePolicy sizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
    const int AXIS_SMALL_SIDE = 50;


    //Make the external axis we will be using
    if ( plotAxis  == Y_LEFT ){

    	axisWidget = new ExternalAxisWidgetLeft( this, associatedPlot,
    			leftAxisInternal, bottomAxisInternal, rightAxisInternal );
    	sizePolicy.setHorizontalPolicy( QSizePolicy::Fixed );
    	setFixedWidth( AXIS_SMALL_SIDE );
    	setMinimumWidth( AXIS_SMALL_SIDE );
    }
    else if ( plotAxis == X_BOTTOM ){
    	axisWidget = new ExternalAxisWidgetBottom( this, associatedPlot,
    			leftAxisInternal, bottomAxisInternal, rightAxisInternal );
    	sizePolicy.setVerticalPolicy( QSizePolicy::Fixed );
    	setFixedHeight( AXIS_SMALL_SIDE );
    	setMinimumHeight( AXIS_SMALL_SIDE );
    }
    else if ( plotAxis == Y_RIGHT ){

    	axisWidget = new ExternalAxisWidgetRight( this, associatedPlot,
    			leftAxisInternal, bottomAxisInternal, rightAxisInternal );
    	sizePolicy.setHorizontalPolicy( QSizePolicy::Fixed );
    	setFixedWidth( AXIS_SMALL_SIDE );
    	setMinimumWidth( AXIS_SMALL_SIDE );
    }
    else if ( plotAxis == X_TOP ){
    	axisWidget = new ExternalAxisWidgetTop( this, associatedPlot,
    			leftAxisInternal, bottomAxisInternal, rightAxisInternal );
    	sizePolicy.setVerticalPolicy( QSizePolicy::Fixed );
    	setFixedHeight( AXIS_SMALL_SIDE );
    	setMinimumHeight( AXIS_SMALL_SIDE );
    }
    else {
    	qDebug() << "QPAxis: unsupported axis"<< plotAxis;
    }
    setSizePolicy( sizePolicy );
    if ( axisWidget != NULL ){
    	QSize axisWidgetMin = axisWidget->minimumSize();
    	setMinimumSize( axisWidgetMin.width(), axisWidgetMin.height() );
    }
    
    if ( axisWidget != NULL ){
    	QtLayeredLayout* ll = new QtLayeredLayout(this);
    	ll->setContentsMargins(0, 0, 0, 0);
    	ll->addWidget(axisWidget);
    }


    setCursor(NORMAL_CURSOR);
    
    if ( axisWidget != NULL ){
    	axisWidget ->setDateFormat ( Plotter::DEFAULT_DATE_FORMAT );
    	axisWidget->setRelativeDateFormat( Plotter::DEFAULT_RELATIVE_DATE_FORMAT );
    }
    m_dateFormat = Plotter::DEFAULT_DATE_FORMAT;
    m_relativeDateFormat = Plotter::DEFAULT_RELATIVE_DATE_FORMAT;

    //m_canvas.enableAxis(QwtPlot::xBottom, false);
    //m_canvas.enableAxis(QwtPlot::yLeft, false);
    //m_canvas.setAutoReplot(true);
}

void QPAxis::setMinimumSizeHint( int /*width*/, int /*height*/ ){

}

QPAxis::~QPAxis() {
    vector<PlotMouseToolPtr> tools = allMouseTools();
    for(unsigned int i = 0; i < tools.size(); i++)
        unregisterMouseTool(tools[i]);
    
    logObject(CLASS_NAME, this, false);
    operationDraw()->finish();
}


// Public Methods //

pair<int, int> QPAxis::size() const {
    return pair<int, int>(QWidget::width(), QWidget::height()); }

String QPAxis::title() const {
    //return m_canvas.title().text().toStdString();
	return "";
}

void QPAxis::setTitle(const String& /*title*/) {
    /*const char* c = title.c_str();
    m_canvas.setTitle(c); */
}

PlotFontPtr QPAxis::titleFont() const {
    //QwtText t = m_canvas.title();
    //return new QPFont(t.font(), t.color());
	return PlotFontPtr();
}



void QPAxis::setTitleFont(const PlotFont& /*font*/) {

}


PlotAreaFillPtr QPAxis::background() const {
	QPAreaFill* areaFill = NULL;
	if ( axisWidget != NULL ){
		QPalette pal = axisWidget->palette();
		QBrush brush = pal.brush( QPalette::Window);
		areaFill = new QPAreaFill( brush );
	}
	PlotAreaFillPtr fill(areaFill );
	return fill;
}


void QPAxis::setBackground(const PlotAreaFill& areaFill) {
	if ( axisWidget != NULL ){
		QPAreaFill a(areaFill);
		QPalette p = axisWidget->palette();
		p.setBrush(QPalette::Window, a.asQBrush());
		axisWidget->setPalette( p );
    }
}



PlotCursor QPAxis::cursor() const {
    //return QPOptions::cursor(m_canvas.cursor().shape());
	return PlotCursor();
}


void QPAxis::setCursor(PlotCursor /*cursor*/) {
}


void QPAxis::refresh() {
   /* logMethod(CLASS_NAME, "refresh", true);
    PRE_REPLOT
    m_canvas.replot();
    POST_REPLOT
    logMethod(CLASS_NAME, "refresh", false);*/
}


void QPAxis::refresh(int /*drawLayersFlag*/) {

}



PlotAxisBitset  QPAxis::shownAxes() const {
    PlotAxisBitset axes = 0;
    axes |= (PlotAxisBitset)QPOptions::axis(axisType);
    return axes;
}



void QPAxis::showAxes(PlotAxisBitset /*axes*/) {

}


PlotAxisScale QPAxis::axisScale(PlotAxis /*axis*/) const   {
    //return m_scaleDraws[QPOptions::axis(axis)]->scale();
	return NORMAL;
}


void QPAxis::setAxisScale(PlotAxis axis, PlotAxisScale scale) {
	QwtPlot::Axis dAxis = QPOptions::axis(axis);
	if ( axisType == dAxis  && axisWidget != NULL ){
		axisWidget->setAxisScale( scale );
	}
}


bool QPAxis::axisReferenceValueSet(PlotAxis /*axis*/) const {
    return false;
}


double QPAxis::axisReferenceValue(PlotAxis /*axis*/) const {
    return -1;
}


void QPAxis::setAxisReferenceValue(PlotAxis axis, bool on, double value) {
	QwtPlot::Axis dAxis = QPOptions::axis(axis);
	if ( axisType == dAxis  && axisWidget != NULL ){
		axisWidget->setReferenceValue( on, value );
	}
}


bool QPAxis::cartesianAxisShown(PlotAxis /*axis*/) const {
   // return m_canvas.cartesianAxisShown(axis);
	return false;
}



void QPAxis::showCartesianAxis(PlotAxis /*mirrorAxis*/, PlotAxis /*secondaryAxis*/,
                                 bool /*show*/, bool /*hideNormalAxis*/) {
    /*m_canvas.showCartesianAxis(mirrorAxis, secondaryAxis, show);
    showAxis(mirrorAxis, !hideNormalAxis);*/
}

String QPAxis::axisLabel(PlotAxis axis) const {
	String axisLabel;
	if ( axisWidget != NULL && QPOptions::axis(axisType) == axis ){
		axisLabel= axisWidget->getAxisLabel().toStdString();
	}
	return axisLabel;
}

bool QPAxis::isDrawing( bool /*scripting*/ ) {
	return false;
}


void QPAxis::setAxisLabel(PlotAxis axis, const String& title) {
	if ( axisWidget != NULL ){
		if ( QPOptions::axis(axisType) == axis ){
			axisWidget->setAxisLabel( title.c_str() );
		}
	}
}

PlotFontPtr QPAxis::axisFont(PlotAxis /*a*/) const {
    
    /*QwtText t = m_canvas.axisTitle(QPOptions::axis(a));
    return new QPFont(t.font(), t.color());
    */
	return PlotFontPtr();
}


void QPAxis::setAxisFont(PlotAxis axis, const PlotFont& font) {
	if ( axisWidget != NULL ){
		if ( QPOptions::axis(axisType) == axis ){
			QPFont f( font );
			axisWidget->setAxisFont( f.asQFont() );
		}
	}
}


bool QPAxis::colorBarShown(PlotAxis /*axis*/) const {
    //return m_canvas.axisWidget(QPOptions::axis(axis))->isColorBarEnabled();
	return false;
}

void QPAxis::showColorBar(bool /*show*/, PlotAxis /*axis*/) {

}


prange_t QPAxis::axisRange(PlotAxis /*axis*/) const {
    

	return prange_t(0,0);
}


void QPAxis::setAxisRange(PlotAxis /*axis*/, double /*from*/, double /*to*/) {
    //setAxesRanges(axis, from, to, axis, from, to);
}


void QPAxis::setAxesRanges(PlotAxis /*xAxis*/, double /*xFrom*/, double /*xTo*/,
        PlotAxis /*yAxis*/, double /*yFrom*/, double /*yTo*/) {
    /*logMethod(CLASS_NAME, "setAxesRanges", true);
    if(xTo == xFrom && yTo == yFrom) {
        logMethod(CLASS_NAME, "setAxesRanges", false);
        return;
    }
    if(xTo < xFrom) {
        double temp = xTo;
        xTo = xFrom;
        xFrom = temp;
    }
    if(yTo < yFrom) {
        double temp = yTo;
        yTo = yFrom;
        yFrom = temp;
    }
    
    bool autoreplot = m_canvas.autoReplot();
    m_canvas.setAutoReplot(false);
    bool changed = false;
    
    // set bounds
    if(xFrom != xTo) {
        m_canvas.setAxisScale(QPOptions::axis(xAxis), xFrom, xTo,
                      m_canvas.axisStepSize(QPOptions::axis(xAxis)));
        changed = true;
    }
    if(yAxis != xAxis && yFrom != yTo) {
        m_canvas.setAxisScale(QPOptions::axis(yAxis), yFrom, yTo);
        changed = true;
    }
    
    // update axes ratios if needed
    if(changed && m_axesRatioLocked) {
        double r = 1 / m_axesRatios[axisIndex(xAxis)];
        for(unsigned int i = 0; i < m_axesRatios.size(); i++)
            m_axesRatios[i] *= r;
        
        double size = xTo - xFrom;
        double newSize, midPoint, newLow, newHigh;
        const QwtScaleDiv* div;
        
        for(unsigned int i = 0; i < m_axesRatios.size(); i++) {
            if(axisIndex(xAxis) != i) {
                newSize = size * m_axesRatios[i];
                div = m_canvas.axisScaleDiv(QPOptions::axis(axisIndex(i)));
#if QWT_VERSION < 0x050200
                midPoint = (div->range() / 2) + div->lBound();
#else
                midPoint = (div->range() / 2) + div->lowerBound();
#endif
                newLow = midPoint - (newSize / 2);
                newHigh = midPoint + (newSize / 2);
                m_canvas.setAxisScale(QPOptions::axis(axisIndex(i)),
                                      newLow, newHigh);
            }
        }
    }
    
    // restore canvas
    m_canvas.setAutoReplot(autoreplot);
    if(changed && autoreplot) m_canvas.replot();
    
    logMethod(CLASS_NAME, "setAxesRanges", false);*/
}



bool QPAxis::axesAutoRescale() const {
    /*return !m_axesRatioLocked &&
           m_canvas.axisAutoScale(QPOptions::axis(X_BOTTOM)) &&
           m_canvas.axisAutoScale(QPOptions::axis(X_TOP)) &&
           m_canvas.axisAutoScale(QPOptions::axis(Y_LEFT)) &&
           m_canvas.axisAutoScale(QPOptions::axis(Y_RIGHT));
           */
	return false;
}



void QPAxis::setAxesAutoRescale(bool /*autoRescale*/) {
    /*if(autoRescale) {
        m_canvas.setAxisAutoScale(QPOptions::axis(X_BOTTOM));
        m_canvas.setAxisAutoScale(QPOptions::axis(X_TOP));
        m_canvas.setAxisAutoScale(QPOptions::axis(Y_LEFT));
        m_canvas.setAxisAutoScale(QPOptions::axis(Y_RIGHT));
        m_axesRatioLocked = false;
        
    } else {
        // round-about way because qwt doesn't let you turn off auto-rescaling
        // with a direct call.  setAxisScale, however, will turn it off.
        bool replot = m_canvas.autoReplot();
        m_canvas.setAutoReplot(false);
        
        const QwtScaleDiv* div;
        for(int a = 0; a < QwtPlot::axisCnt; a++) {
            div = m_canvas.axisScaleDiv(a);
#if QWT_VERSION < 0x050200
            m_canvas.setAxisScale(a, div->lBound(), div->hBound(),
                    m_canvas.axisStepSize(a));
#else
            m_canvas.setAxisScale(a, div->lowerBound(), div->upperBound(),
                    m_canvas.axisStepSize(a));
#endif
        }

        m_canvas.setAutoReplot(replot);
    }*/
}



void QPAxis::rescaleAxes() {
    /*logMethod(CLASS_NAME, "rescaleAxes", true);
    m_axesRatioLocked = false;
    m_canvas.setAxisAutoScale(QPOptions::axis(X_BOTTOM));
    m_canvas.setAxisAutoScale(QPOptions::axis(X_TOP));
    m_canvas.setAxisAutoScale(QPOptions::axis(Y_LEFT));
    m_canvas.setAxisAutoScale(QPOptions::axis(Y_RIGHT));
    
    if(m_canvas.autoReplot()) {
        PRE_REPLOT
        m_canvas.replot();    
        POST_REPLOT
    }
    logMethod(CLASS_NAME, "rescaleAxes", false);*/
}



bool QPAxis::axesRatioLocked() const {
    //return m_axesRatioLocked;
	return false;
}



void QPAxis::setAxesRatioLocked(bool /*locked*/) {
    /*m_axesRatioLocked = locked;
    setAxesAutoRescale(false);
    
    if(!m_axesRatioLocked) return;
    
    // recalculate ratios
    m_axesRatios[axisIndex(X_BOTTOM)] = 1;
    const QwtScaleDiv* div = m_canvas.axisScaleDiv(QPOptions::axis(X_BOTTOM));
    double xSize = div->range();
    
    double size;
    for(unsigned int i = 0; i < m_axesRatios.size(); i++) {
        if(i != axisIndex(X_BOTTOM)) {
            div = m_canvas.axisScaleDiv(QPOptions::axis(axisIndex(i)));
            size = div->range();
            m_axesRatios[i] = size / xSize;
        }
    }*/
}



int QPAxis::cachedAxesStackSizeLimit() const {
    //return m_stackCache.memoryLimit();
	return 1;
}



void QPAxis::setCachedAxesStackSizeLimit(int /*sizeInKilobytes*/) {
    //m_stackCache.setMemoryLimit(sizeInKilobytes);
}



pair<int, int> QPAxis::cachedAxesStackImageSize() const {
    //QSize size = m_stackCache.fixedImageSize();
    /*if(!size.isValid())*/ return pair<int, int>(-1, -1);
    //else return pair<int, int>(size.width(), size.height());
}



void QPAxis::setCachedAxesStackImageSize(int /*width*/, int /*height*/) {
    //m_stackCache.setFixedSize(QSize(width, height));
}


bool QPAxis::plotItem(PlotItemPtr /*item*/, PlotCanvasLayer /*layer*/) {
    return true;
}



vector<PlotItemPtr> QPAxis::allPlotItems() const {
    vector<PlotItemPtr> v(0);
    return v;
}


vector<PlotItemPtr> QPAxis::layerPlotItems(PlotCanvasLayer /*layer*/) const {
    vector<PlotItemPtr> v(0);
    return v;
}


unsigned int QPAxis::numPlotItems() const {
    return 0;
}


unsigned int QPAxis::numLayerPlotItems(PlotCanvasLayer /*layer*/) const {
    
   return 0;
}



void QPAxis::removePlotItems(const vector<PlotItemPtr>& /*items*/) {

}



void QPAxis::clearPlotItems() {
}



void QPAxis::clearPlots() {
    //PlotCanvas::clearPlots();
    //m_usedColors.resize(0);
}



void QPAxis::clearLayer(PlotCanvasLayer /*layer*/) {
}


void QPAxis::holdDrawing() {
       // m_canvas.holdDrawing();
}

void QPAxis::releaseDrawing() {
    /*logMethod(CLASS_NAME, "releaseDrawing", true);
    PRE_REPLOT
    m_canvas.releaseDrawing();
    POST_REPLOT
    logMethod(CLASS_NAME, "releaseDrawing", false);*/
}



bool QPAxis::drawingIsHeld() const   {
    //return m_canvas.drawingIsHeld();
	return false;
}



void QPAxis::setSelectLineShown(bool /*shown*/) {
}

PlotLinePtr QPAxis::selectLine() const {
    return PlotLinePtr();
}

void QPAxis::setSelectLine(const PlotLine& /*line*/) {

}


bool QPAxis::gridShown(bool* /*xMajor*/, bool* /*xMinor*/, bool* /*yMajor*/,
        bool* /*yMinor*/) const {
            
    bool ret = false;
    
    /*bool tmp = m_canvas.grid().xEnabled();
    ret |= tmp;
    if(xMajor != NULL) *xMajor = tmp;
    
    ret |= tmp = m_canvas.grid().xMinEnabled();
    if(xMinor != NULL) *xMinor = tmp;
    
    ret |= tmp = m_canvas.grid().yEnabled();
    if(yMajor != NULL) *yMajor = tmp;
    
    ret |= tmp = m_canvas.grid().yMinEnabled();
    if(yMinor != NULL) *yMinor = tmp;
    */
    return ret;
}

void QPAxis::showGrid(bool /*xMajor*/, bool /*xMinor*/, bool /*yMajor*/,bool /*yMinor*/) {
    /*m_canvas.grid().enableX(xMajor);
    m_canvas.grid().enableXMin(xMinor);
    m_canvas.grid().enableY(yMajor);
    m_canvas.grid().enableYMin(yMinor);*/
}


PlotLinePtr QPAxis::gridMajorLine() const {
    //return new QPLine(m_canvas.grid().majPen());
	return PlotLinePtr();
}


void QPAxis::setGridMajorLine(const PlotLine& /*line*/) {
    /*if(line != *gridMajorLine()) {
        QPLine l(line);
        
        m_canvas.grid().enableX(l.style() != PlotLine::NOLINE);
        m_canvas.grid().enableY(l.style() != PlotLine::NOLINE);
        m_canvas.grid().setMajPen(l.asQPen());
    }*/
}



PlotLinePtr QPAxis::gridMinorLine() const {
    //return new QPLine(m_canvas.grid().minPen());
	return PlotLinePtr();
}

void QPAxis::setGridMinorLine(const PlotLine& /*line*/) {
    /*if(line != *gridMinorLine()) {
        QPLine l(line);
        
        m_canvas.grid().enableXMin(l.style() != PlotLine::NOLINE);
        m_canvas.grid().enableYMin(l.style() != PlotLine::NOLINE);
        m_canvas.grid().setMinPen(l.asQPen());
    }*/
}






bool QPAxis::autoIncrementColors() const {
    return false;
}


void QPAxis::setAutoIncrementColors(bool /*autoInc*/) {
   // m_autoIncColors = autoInc;
}


bool QPAxis::exportToFile(const PlotExportFormat& /*format*/) {
    //return exportCanvas(this, format);
	return false;
}



String QPAxis::fileChooserDialog(const String& /*title*/, const String& /*directory*/){
	return "";
}



const String& QPAxis::dateFormat() const {
    return m_dateFormat; 
}



void QPAxis::setDateFormat(const String& dateFormat)   {
	if ( axisWidget != NULL ){
		axisWidget->setDateFormat( dateFormat );
	}
	m_dateFormat = dateFormat;
}



const String& QPAxis::relativeDateFormat() const {
    return m_relativeDateFormat; 
}



void QPAxis::setRelativeDateFormat(const String& dateFormat)   {
	if ( axisWidget != NULL ){
		axisWidget->setRelativeDateFormat( dateFormat );
	}
	m_relativeDateFormat = dateFormat;
}



PlotCoordinate QPAxis::convertCoordinate(const PlotCoordinate& coord,
                                 PlotCoordinate::System /*newSystem*/) const  {
	return coord;
}



vector<double> QPAxis::textWidthHeightDescent(const String& text,
                                                PlotFontPtr font) const   {
    vector<double> v(3, 0);

    QFontMetrics mets(QPFont(*font).asQFont());
    v[0] = mets.width(text.c_str());
    v[1] = mets.ascent();
    v[2] = mets.descent();
    
    return v;
}



PlotFactory* QPAxis::implementationFactory() const {
    return new QPFactory();
}



// Protected Methods //

void QPAxis::setQPPlotter(QPPlotter* parent) {
    m_parent = parent;
    if(parent != NULL) {
        PlotLoggerPtr log = logger();
        if(!log.null())
            for(unsigned int i = 0; i < m_queuedLogs.size(); i++)
                log->postMessage(m_queuedLogs[i]);
        m_queuedLogs.clear();
    }
}



PlotLoggerPtr QPAxis::logger() const {
    if(m_parent == NULL) return PlotLoggerPtr();
    else                 return m_parent->logger();
}



void QPAxis::logObject(const String& className, void* address, bool creation,
        const String& message) {
    if(m_parent != NULL)
        m_parent->logObject(className, address, creation, message);
    else
        m_queuedLogs.push_back(
                PlotLogObject(className, address, creation, message,
                              PlotLogger::OBJECTS_MAJOR));
}



void QPAxis::logMethod(const String& className, const String& methodName,
        bool entering, const String& message) {
    if(m_parent != NULL)
        m_parent->logMethod(className, methodName, entering, message);
}



/*QPAxesCache& QPAxis::axesCache()   {
    return m_stackCache; 
}



const QPAxesCache& QPAxis::axesCache() const   {
    return m_stackCache; 
}*/



}

#endif
