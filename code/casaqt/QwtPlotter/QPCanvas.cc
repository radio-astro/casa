#include <stdio.h>
//# QPCanvas.cc: Qwt implementation of generic PlotCanvas class.
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

#include <casaqt/QwtPlotter/QPCanvas.qo.h>

#include <casaqt/QtUtilities/QtLayeredLayout.h>
#include <casaqt/QwtPlotter/QPAnnotation.h>
#include <casaqt/QwtPlotter/QPFactory.h>
#include <casaqt/QwtPlotter/QPExporter.h>
#include <casaqt/QwtPlotter/QPPlotter.qo.h>
#include <casaqt/QwtPlotter/QPRasterPlot.h>
#include <casaqt/QwtPlotter/QPShape.h>
#include <casaqt/QwtPlotter/AxisListener.h>

#include <qwt_scale_widget.h>

namespace casa {

//////////////////////////
// QPCANVAS DEFINITIONS //
//////////////////////////

// Macros to reset tool stacks as needed.
#define PRE_REPLOT                                                            \
    vector<prange_t> preAxes(4);                                              \
    preAxes[0] = axisRange(X_BOTTOM); preAxes[1] = axisRange(X_TOP);          \
    preAxes[2] = axisRange(Y_LEFT); preAxes[3] = axisRange(Y_RIGHT);

#define POST_REPLOT                                                           \
    vector<prange_t> postAxes(4);                                             \
    postAxes[0] = axisRange(X_BOTTOM); postAxes[1] = axisRange(X_TOP);        \
    postAxes[2] = axisRange(Y_LEFT); postAxes[3] = axisRange(Y_RIGHT);        \
    if(preAxes[0] != postAxes[0] || preAxes[1] != postAxes[1] ||              \
       preAxes[2] != postAxes[2] || preAxes[3] != postAxes[3])                \
        resetMouseTools();



// Static //

double QPCanvas::zOrder = 1;

const String QPCanvas::CLASS_NAME = "QPCanvas";
const String QPCanvas::DRAW_NAME = "drawItems";


bool QPCanvas::print(  QPainter* painter, PlotAreaFillPtr paf, double widgetWidth,
		double widgetHeight,int externalAxisWidth, int /*externalAxisHeight*/,
		int rowIndex, int colIndex, QRect /*imageRect*/ ){
	PlotAreaFillPtr originalBackground = background();
	setBackground(*paf);

	//Decide the print rectangle inside the image rectangle.
	QRect printGeom;

	/*if ( isVisible() ){
		QRect geom = geometry();
		double widthRatio = 1;
		double heightRatio = 1;
		printGeom = QRect((int)((geom.x() * widthRatio) + 0.5),
	                          (int)((geom.y() * heightRatio) + 0.5),
	                          (int)((geom.width() * widthRatio) + 0.5),
	                          (int)((geom.height() * heightRatio) + 0.5));
		printGeom &= imageRect;
	}
	else {*/
		//Note we have to get the position and size differently in scripting mode,
		//because paint calls do not update the widget when it is hidden.
		int widthMultiplier = colIndex;
		if ( colIndex >= 1 && commonY ){
			widthMultiplier = colIndex - 1;
		}
		int xPosition = static_cast<int>(widthMultiplier * widgetWidth + externalAxisWidth);
		int yPosition = static_cast<int>(rowIndex * widgetHeight);
		printGeom = QRect( xPosition, yPosition, static_cast<int>(widgetWidth),
			static_cast<int>(widgetHeight));
	//}

	QColor titleColor = asQwtPlot().title().color();

	PlotOperationPtr op = operationExport();
	bool wasCanceled = false;
	if (!op.null()) wasCanceled |= op->cancelRequested();
	if (!wasCanceled){

	        asQwtPlot().print(painter, printGeom);
	        painter->drawRect( printGeom );

	        // For bug where title color changes after a print.
	        QwtText title = asQwtPlot().title();
	        if (title.color() != titleColor) {
	            title.setColor(titleColor);
	            asQwtPlot().setTitle(title);
	        }

	        setBackground(originalBackground);

	        if (!op.null()) wasCanceled |= op->cancelRequested();
	}
	return wasCanceled;
}


bool QPCanvas::print( QPrinter& printer ){
	 // Set orientation.
	 bool continuePrinting = true;
	 printer.setOrientation(width() >= height() ?
	                      QPrinter::Landscape : QPrinter::Portrait);

	 QColor titleColor = asQwtPlot().title().color();

	 PlotOperationPtr op = operationExport();
	 bool wasCanceled = false;
	 if(!op.null()){
		 wasCanceled |= op->cancelRequested();
	 }
	 if(wasCanceled){
		 continuePrinting = false;
	 }
	 else {
		 print(printer);

	        // For bug where title color changes after a print.
		 QwtText title = asQwtPlot().title();
		 if(title.color() != titleColor) {
			 title.setColor(titleColor);
			 asQwtPlot().setTitle(title);
		 }


		 if(!op.null()){
			 wasCanceled |= op->cancelRequested();
		 }
		 if(wasCanceled){
			 continuePrinting = false;
		 }
	 }
	 return continuePrinting;
}


QImage QPCanvas::grabImageFromCanvas(
        const PlotExportFormat& format /*,
        QPCanvas* grabCanvas,
        QPPlotter* grabPlotter*/
        )   {
	QImage image;
    /*image = QPixmap::grabWidget(grabCanvas != NULL ?
            &grabCanvas->asQwtPlot() :
            grabPlotter->canvasWidget()).toImage();*/
	image = QPixmap::grabWidget( &m_canvas ).toImage();


    
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


unsigned int QPCanvas::axisIndex(PlotAxis a) {
    switch(a) {
    case X_BOTTOM: return 0;
    case X_TOP: return 1;
    case Y_LEFT: return 2;
    case Y_RIGHT: return 3;
    default: return 0;
    }
}   


PlotAxis QPCanvas::axisIndex(unsigned int i) {
    switch(i) {
    case 0: return X_BOTTOM;
    case 1: return X_TOP;
    case 2: return Y_LEFT;
    case 3: return Y_RIGHT;
    default: return X_BOTTOM;
    }
}


// Constructors/Destructors //

QPCanvas::QPCanvas(QPPlotter* parent) : m_parent(parent), m_canvas(this),
        m_axesRatioLocked(false), m_axesRatios(4, 1), m_stackCache(*this),
        m_autoIncColors(false), m_picker(m_canvas.canvas()),
        m_mouseFilter(m_canvas.canvas()), m_legendFontSet(false),
        m_inDraggingMode(false)/*, m_ignoreNextRelease(false), m_timer(this),
        m_clickEvent(NULL)*/ {
    logObject(CLASS_NAME, this, true);
    
    commonX = false;
    commonY = false;

    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setFocusPolicy(Qt::StrongFocus);
    
    QtLayeredLayout* ll = new QtLayeredLayout(this);
    ll->setContentsMargins(0, 0, 0, 0);
    ll->addWidget(&m_canvas);
    
    m_legend = new QPLegendHolder(this, PlotCanvas::INT_URIGHT);
    setLegendLine(QPFactory::defaultLegendLine());
    setLegendFill(QPFactory::defaultLegendAreaFill());
    m_legend->showLegend(false);
    
    setCursor(NORMAL_CURSOR);
    
    m_picker.setSelectionFlags(QwtPicker::RectSelection |
                               QwtPicker::DragSelection);
    m_picker.setRubberBand(QwtPicker::RectRubberBand);
    m_picker.setRubberBandPen(QPen(Qt::NoPen));
    m_picker.setTrackerMode(QwtPicker::AlwaysOff);
    
    m_mouseFilter.turnTracking(true);
    connect(&m_mouseFilter, SIGNAL(mouseMoveEvent(QMouseEvent*)),
            SLOT(trackerMouseEvent(QMouseEvent*)));
    
    m_dateFormat = Plotter::DEFAULT_DATE_FORMAT;
    m_relativeDateFormat = Plotter::DEFAULT_RELATIVE_DATE_FORMAT;
    for(int i = 0; i < QwtPlot::axisCnt; i++) {
        m_scaleDraws[i] = new QPScaleDraw(&m_canvas, QwtPlot::Axis(i));
        m_scaleDraws[i]->setDateFormat(m_dateFormat);
        m_scaleDraws[i]->setRelativeDateFormat(m_relativeDateFormat);
        m_canvas.setAxisScaleDraw(QwtPlot::Axis(i), m_scaleDraws[i]);
    }
    
    m_canvas.enableAxis(QwtPlot::xBottom, false);
    m_canvas.enableAxis(QwtPlot::yLeft, false);
    m_canvas.setAutoReplot(true);
    
    connect(&m_picker, SIGNAL(selected(const QwtDoubleRect&)),
            this, SLOT(regionSelected(const QwtDoubleRect&)));
    //connect(&m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
}

void QPCanvas::enableAxis( QwtPlot::Axis axis, bool enable ){
	bool horizontalAxis = QPOptions::isAxisX( QPOptions::axis(axis) );

	bool actualEnable = enable;
	if ( commonX && horizontalAxis ){
		actualEnable = false;
	}
	else if ( commonY && !horizontalAxis ){
		actualEnable = false;
	}

	m_canvas.enableAxis( axis, actualEnable );
}

QPCanvas::~QPCanvas() {
    vector<PlotMouseToolPtr> tools = allMouseTools();
    for(unsigned int i = 0; i < tools.size(); i++)
        unregisterMouseTool(tools[i]);
    
    logObject(CLASS_NAME, this, false);
    operationDraw()->finish();
}


// Public Methods //
pair<int, int> QPCanvas::size() const {
    return pair<int, int>(QWidget::width(), QWidget::height());
}

String QPCanvas::title() const {
    return m_canvas.title().text().toStdString();
}

void QPCanvas::setTitle(const String& newTitle) {
	String oldTitle(title().c_str());
    const char* c = newTitle.c_str();
    m_canvas.setTitle(c);
}

PlotFontPtr QPCanvas::titleFont() const {
    QwtText t = m_canvas.title();
    return new QPFont(t.font(), t.color());
}

void QPCanvas::setTitleFont(const PlotFont& font) {
    QPFont f(font);
    QwtText t = m_canvas.title();
    t.setFont(f.asQFont());
    t.setColor(f.asQColor());
    m_canvas.setTitle(t);
}




PlotAreaFillPtr QPCanvas::background() const {
    return new QPAreaFill(m_canvas.canvas()->palette().brush(
                          QPalette::Window));
}

void QPCanvas::setBackground(const PlotAreaFill& areaFill) {
    QPAreaFill a(areaFill);    
    QPalette p = m_canvas.canvas()->palette();
    p.setBrush(QPalette::Window, a.asQBrush());
    m_canvas.canvas()->setPalette(p);
}

PlotCursor QPCanvas::cursor() const {
    return QPOptions::cursor(m_canvas.cursor().shape()); 
}

void QPCanvas::setCursor(PlotCursor cursor) {
    
    QWidget::setCursor(QPOptions::cursor(cursor));
    m_canvas.canvas()->setCursor(QPOptions::cursor(cursor)); 
}


void QPCanvas::refresh() {
    logMethod(CLASS_NAME, "refresh", true);
    PRE_REPLOT
    m_canvas.replot();
    POST_REPLOT
    logMethod(CLASS_NAME, "refresh", false);
}

void QPCanvas::refresh(int drawLayersFlag) {
    logMethod(CLASS_NAME, "refresh(int)", true);
    if(drawLayersFlag != 0) {
        PRE_REPLOT
        m_canvas.setLayersChanged(drawLayersFlag);
        m_canvas.replot();
        POST_REPLOT
    }
    logMethod(CLASS_NAME, "refresh(int)", false);
}



PlotAxisBitset  QPCanvas::shownAxes() const {
    PlotAxisBitset axes = 0;
    for(int i = 0; i < QwtPlot::axisCnt; i++) {
        if(m_canvas.axisEnabled(i))
            axes |= (PlotAxisBitset)QPOptions::axis(QwtPlot::Axis(i));
    }
    return axes;
}

void QPCanvas::showAxes(PlotAxisBitset axes) {
    bool show;
    for(int i = 0; i < QwtPlot::axisCnt /* 4 */;  i++) {
        show = axes & QPOptions::axis(QwtPlot::Axis(i));
        enableAxis( static_cast<QwtPlot::Axis>(i), show );
    }
}

void QPCanvas::setCommonAxes( bool commonX, bool commonY ){
	this->commonX = commonX;
	this->commonY = commonY;
}

PlotAxisScale QPCanvas::axisScale(PlotAxis axis) const   {
    return m_scaleDraws[QPOptions::axis(axis)]->scale(); 
}

void QPCanvas::setAxisScale(PlotAxis axis, PlotAxisScale scale) {
    m_scaleDraws[QPOptions::axis(axis)]->setScale(scale);
    int axisListenerCount = axisListeners.size();
    for ( int i = 0; i < axisListenerCount; i++ ){
    	axisListeners[i]->setAxisScale( axis, scale );
    }
}

bool QPCanvas::axisReferenceValueSet(PlotAxis axis) const {
    return m_scaleDraws[QPOptions::axis(axis)]->referenceValueSet(); 
}

double QPCanvas::axisReferenceValue(PlotAxis axis) const {
    return m_scaleDraws[QPOptions::axis(axis)]->referenceValue(); 
}

void QPCanvas::setAxisReferenceValue(PlotAxis axis, bool on, double value) {
    m_scaleDraws[QPOptions::axis(axis)]->setReferenceValue(on, value); 
    int axisListenerCount = axisListeners.size();
       for ( int i = 0; i < axisListenerCount; i++ ){
       	axisListeners[i]->setAxisReferenceValue( axis, on, value );
    }
}


bool QPCanvas::cartesianAxisShown(PlotAxis axis) const {
    return m_canvas.cartesianAxisShown(axis); 
}


void QPCanvas::showCartesianAxis(PlotAxis mirrorAxis, PlotAxis secondaryAxis,
                                 bool show, bool hideNormalAxis) {
    m_canvas.showCartesianAxis(mirrorAxis, secondaryAxis, show);
    showAxis(mirrorAxis, !hideNormalAxis);
}



String QPCanvas::axisLabel(PlotAxis axis) const {
    return m_canvas.axisTitle(QPOptions::axis(axis)).text().toStdString(); 
}

bool QPCanvas::isCommonAxis( PlotAxis axis ) const{
	bool common = false;
	bool horizontal = QPOptions::isAxisX( axis );
	if ( commonX && horizontal ){
		common = true;
	}
	else if ( commonY && !horizontal ){
		common = true;
	}
	return common;
}

void QPCanvas::addAxisListener( AxisListener* listener ){
	axisListeners.append( listener );
}

void QPCanvas::clearAxisListeners(){
	axisListeners.clear();
}


void QPCanvas::setAxisLabel(PlotAxis axis, const String& title) {
	String actualTitle(title);
	if ( (commonX || commonY) && isCommonAxis( axis ) ){
		actualTitle = "";
		for ( int i = 0; i < axisListeners.size(); i++ ){
			axisListeners[i]->setAxisLabel(axis, title );
		}
	}

	QwtPlot::Axis plotAxis = QPOptions::axis( axis );
    m_canvas.setAxisTitle(plotAxis, actualTitle.c_str());
    enableAxis( plotAxis, true );

}

PlotFontPtr QPCanvas::axisFont(PlotAxis a) const {
    
    QwtText t = m_canvas.axisTitle(QPOptions::axis(a));
    return new QPFont(t.font(), t.color());
}



void QPCanvas::setAxisFont(PlotAxis axis, const PlotFont& font) {
    
    if(font != *axisFont(axis)) {
        QPFont f(font);
        QwtText t = m_canvas.axisTitle(QPOptions::axis(axis));
        t.setFont(f.asQFont());
        t.setColor(f.asQColor());
        m_canvas.setAxisTitle(QPOptions::axis(axis), t);
        f.setBold(false);
        m_canvas.setAxisFont(QPOptions::axis(axis), f.asQFont());
        if(!t.isEmpty()){
        	enableAxis( QPOptions::axis(axis), true );
        }
    }

	for ( int i = 0; i < axisListeners.size(); i++ ){
		axisListeners[i]->setAxisFont(axis, font );
	}
}



bool QPCanvas::colorBarShown(PlotAxis axis) const {
    return m_canvas.axisWidget(QPOptions::axis(axis))->isColorBarEnabled(); 
}



void QPCanvas::showColorBar(bool show, PlotAxis axis) {
    
    QwtScaleWidget* scale = m_canvas.axisWidget(QPOptions::axis(axis));
    
    if(!show) {
        scale->setColorBarEnabled(false);
        return;
    }
    
    QPRasterPlot* r = NULL;
    for(unsigned int i = 0; i < m_plotItems.size(); i++) {
        r = dynamic_cast<QPRasterPlot*>(m_plotItems[i].second);
        if(r != NULL) break;
    }
    
    if(r == NULL) {
        scale->setColorBarEnabled(false);
        return;
    }
    
    prange_t v = r->rasterData()->valueRange();
    scale->setColorBarEnabled(true);
    
    if(r->dataFormat() == PlotRasterData::SPECTROGRAM) {
        scale->setColorMap(QwtDoubleInterval(v.first, v.second),
                           r->colorMap());
    } else {
        vector<double>* vals = r->rasterData()->colorBarValues();
        scale->setColorMap(QwtDoubleInterval(v.first, v.second),
                           QPOptions::rasterMap(*vals));
        delete vals;
    }
    enableAxis( QPOptions::axis(axis), true );
    if(v.first != v.second) setAxisRange(axis, v.first, v.second);
    else                    setAxisRange(axis, v.first - 0.5, v.second + 0.5);
}


prange_t QPCanvas::axisRange(PlotAxis axis) const {
    
    const QwtScaleDiv* div = m_canvas.axisScaleDiv(QPOptions::axis(axis));
#if QWT_VERSION < 0x050200
    return prange_t(div->lBound(), div->hBound());
#else
    return prange_t(div->lowerBound(), div->upperBound());
#endif
}



void QPCanvas::setAxisRange(PlotAxis axis, double from, double to) {
    setAxesRanges(axis, from, to, axis, from, to);
}



void QPCanvas::setAxesRanges(PlotAxis xAxis, double xFrom, double xTo,
        PlotAxis yAxis, double yFrom, double yTo) {
    logMethod(CLASS_NAME, "setAxesRanges", true);
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
    
    logMethod(CLASS_NAME, "setAxesRanges", false);
}



bool QPCanvas::axesAutoRescale() const {
    return !m_axesRatioLocked &&
           m_canvas.axisAutoScale(QPOptions::axis(X_BOTTOM)) &&
           m_canvas.axisAutoScale(QPOptions::axis(X_TOP)) &&
           m_canvas.axisAutoScale(QPOptions::axis(Y_LEFT)) &&
           m_canvas.axisAutoScale(QPOptions::axis(Y_RIGHT));
}



void QPCanvas::setAxesAutoRescale(bool autoRescale) {
    if(autoRescale) {
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
    }
}



void QPCanvas::rescaleAxes() {
    logMethod(CLASS_NAME, "rescaleAxes", true);
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
    logMethod(CLASS_NAME, "rescaleAxes", false);
}



bool QPCanvas::axesRatioLocked() const { 
    return m_axesRatioLocked; 
}



void QPCanvas::setAxesRatioLocked(bool locked) {
    m_axesRatioLocked = locked;
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
    }
}



int QPCanvas::cachedAxesStackSizeLimit() const {
    return m_stackCache.memoryLimit(); 
}



void QPCanvas::setCachedAxesStackSizeLimit(int sizeInKilobytes) {
    m_stackCache.setMemoryLimit(sizeInKilobytes); 
}



pair<int, int> QPCanvas::cachedAxesStackImageSize() const {
    QSize size = m_stackCache.fixedImageSize();
    if(!size.isValid()) return pair<int, int>(-1, -1);
    else return pair<int, int>(size.width(), size.height());
}



void QPCanvas::setCachedAxesStackImageSize(int width, int height) {
    m_stackCache.setFixedSize(QSize(width, height)); }


bool QPCanvas::plotItem(PlotItemPtr item, PlotCanvasLayer layer) {
    logMethod(CLASS_NAME, "plotItem", true);
    if(item.null() || !item->isValid()) {
        logMethod(CLASS_NAME, "plotItem", false);
        return false;
    }
    
    bool createdQPI;
    QPPlotItem* qitem = QPPlotItem::cloneItem(item, &createdQPI);
    if(qitem == NULL) {
        logMethod(CLASS_NAME, "plotItem", false);
        return false;
    }
    if(createdQPI) item = qitem;
    
    bool replot = m_canvas.autoReplot();
    m_canvas.setAutoReplot(false);
    
    // Make sure it's not already on canvas.
    int foundMain = -1, foundLayer = -1;
    for(unsigned int i = 0; foundMain == -1 && i < m_plotItems.size(); i++)
        if(m_plotItems[i].second == qitem) foundMain = (int)i;
    if(foundMain == -1)
        for(unsigned int i=0; foundLayer== -1 && i< m_layeredItems.size(); i++)
            if(m_layeredItems[i].second == qitem) foundLayer = (int)i;
    
    if(foundMain > -1 || foundLayer > -1) {
        if((foundMain > -1 && layer == MAIN) ||
           (foundLayer > -1 && layer == ANNOTATION)) {
            m_canvas.setAutoReplot(replot);
            logMethod(CLASS_NAME, "plotItem", false);
            return true;
        }
        
        // we need to switch layers
        removePlotItem(item);
    }
    
    // Autoincrement colors for plots.
    ScatterPlot* sp;
    if(m_autoIncColors && ((sp = dynamic_cast<ScatterPlot*>(&*item)) != NULL)){
        QPLine line = sp->line();
        QPSymbol symbol = sp->symbol();
        
        int ri;
        bool contains = true;
        unsigned int i;
        while(contains) {
            ri = rand() % QPPlotter::GLOBAL_COLORS.size();

            i = 0;
            for(; i < m_usedColors.size(); i++) {
                if(m_usedColors[i] == ri) break;
            }
            contains = (i < m_usedColors.size());
        }
        
	QString eh = QPPlotter::GLOBAL_COLORS[ri];
        QPColor color(eh);
        line.setColor(color);
        symbol.setColor(color);
        sp->setLine(line);
        sp->setSymbol(symbol);
    }
    
    // Update title font for legend.
    QwtText t = qitem->qwtTitle();
    if(m_legendFontSet) {
        t.setFont(m_legendFont.asQFont());
        t.setColor(m_legendFont.asQColor());
        qitem->setQwtTitle(t);
    } else {
        m_legendFont.setAsQFont(t.font());
        m_legendFont.setAsQColor(t.color());
        m_legendFontSet = true;
    }
    
    PlotAxis xAxis = item->xAxis(), yAxis = item->yAxis();
    if(!cartesianAxisShown(xAxis)){
    	enableAxis( QPOptions::axis(xAxis), true );
    }
    if(!cartesianAxisShown(yAxis)){
    	enableAxis( QPOptions::axis(xAxis), true );
    }
    
    qitem->setZ(zOrder++);
    qitem->attach(this, layer);
    
    if(layer == MAIN)
        m_plotItems.push_back(pair<PlotItemPtr, QPPlotItem*>(item, qitem));
    else
        m_layeredItems.push_back(pair<PlotItemPtr, QPPlotItem*>(item, qitem));
    m_canvas.setLayerChanged(layer);
    
    if(replot) {
        PRE_REPLOT
        m_canvas.replot();
        POST_REPLOT
    }    
    m_canvas.setAutoReplot(replot);
    
    logMethod(CLASS_NAME, "plotItem", false);
    return true;
}



vector<PlotItemPtr> QPCanvas::allPlotItems() const {
    
    vector<PlotItemPtr> v(m_plotItems.size() + m_layeredItems.size());
    unsigned int i = 0, n = m_plotItems.size();
    for(; i < m_plotItems.size(); i++) v[i] = m_plotItems[i].first;
    for(; i < v.size(); i++) v[i] = m_layeredItems[i - n].first;
    return v;
}



vector<PlotItemPtr> QPCanvas::layerPlotItems(PlotCanvasLayer layer) const {
    vector<PlotItemPtr> v((layer == MAIN) ? m_plotItems.size() :
                                            m_layeredItems.size());
    if(layer == MAIN) {
        for(unsigned int i = 0; i < v.size(); i++)
            v[i] = m_plotItems[i].first;
    } else {
        for(unsigned int i = 0; i < v.size(); i++)
            v[i] = m_layeredItems[i].first;
    }
    return v;
}



unsigned int QPCanvas::numPlotItems() const { 
    return m_plotItems.size(); 
}



unsigned int QPCanvas::numLayerPlotItems(PlotCanvasLayer layer) const {
    
    if(layer == MAIN) return m_plotItems.size();
    else              return m_layeredItems.size();
}



void QPCanvas::removePlotItems(const vector<PlotItemPtr>& items) {
    logMethod(CLASS_NAME, "removePlotItems", true);
    bool replot = m_canvas.autoReplot();
    m_canvas.setAutoReplot(false);
    for(unsigned int i = 0; i < items.size(); i++) {
        if(items[i].null()) continue;
        for(unsigned int j = 0; j < m_plotItems.size(); j++) {
            if(items[i] == m_plotItems[j].first) {
                m_plotItems[j].second->detach();
                m_plotItems.erase(m_plotItems.begin() + j);
                m_canvas.setLayerChanged(MAIN);
                break;
            }
        }
        for(unsigned int j = 0; j < m_layeredItems.size(); j++) {
            if(items[i] == m_layeredItems[j].first) {
                m_layeredItems[j].second->detach();
                m_layeredItems.erase(m_layeredItems.begin() + j);
                m_canvas.setLayerChanged(ANNOTATION);
                break;
            }
        }
    }
    if(replot) {
        PRE_REPLOT
        m_canvas.replot();
        POST_REPLOT
    }
    m_canvas.setAutoReplot(replot);
    logMethod(CLASS_NAME, "removePlotItems", false);
}



void QPCanvas::clearPlotItems() {
    logMethod(CLASS_NAME, "clearPlotItems", true);
    bool replot = m_canvas.autoReplot();
    m_canvas.setAutoReplot(false);
    for(unsigned int i = 0; i < m_plotItems.size(); i++)
        m_plotItems[i].second->detach();
    for(unsigned int i = 0; i < m_layeredItems.size(); i++)
        m_layeredItems[i].second->detach();
    m_plotItems.clear();
    m_layeredItems.clear();
    m_canvas.setAllLayersChanged();
    
    if(replot) {
        PRE_REPLOT
        m_canvas.replot();
        POST_REPLOT
    }
    
    m_usedColors.resize(0);
    m_canvas.setAutoReplot(replot);
    logMethod(CLASS_NAME, "clearPlotItems", false);
}



void QPCanvas::clearPlots() {
    PlotCanvas::clearPlots();
    m_usedColors.resize(0);
}



void QPCanvas::clearLayer(PlotCanvasLayer layer) {
    logMethod(CLASS_NAME, "clearLayer", true);
    bool autoreplot = m_canvas.autoReplot();
    m_canvas.setAutoReplot(false);
    if(layer == MAIN) {
        for(unsigned int i = 0; i < m_plotItems.size(); i++)
            m_plotItems[i].second->detach();
        m_plotItems.clear();    
    } else {
        for(unsigned int i = 0; i < m_layeredItems.size(); i++)
            m_layeredItems[i].second->detach();
        m_layeredItems.clear();    
    }
    m_canvas.setLayerChanged(layer);
    
    if(autoreplot) {
        PRE_REPLOT
        m_canvas.replot();
        POST_REPLOT
    }
    
    m_canvas.setAutoReplot(autoreplot);
    m_usedColors.resize(0);
    logMethod(CLASS_NAME, "clearLayer", false);
}



void QPCanvas::holdDrawing() { 
        m_canvas.holdDrawing(); 
}

void QPCanvas::releaseDrawing() {
    logMethod(CLASS_NAME, "releaseDrawing", true);
    PRE_REPLOT
    m_canvas.releaseDrawing();
    POST_REPLOT
    logMethod(CLASS_NAME, "releaseDrawing", false);
}



bool QPCanvas::drawingIsHeld() const   { 
    return m_canvas.drawingIsHeld(); 
}



void QPCanvas::setSelectLineShown(bool shown) {
    if(shown != selectLineShown()) {
        QPen p = m_picker.rubberBandPen();
        p.setStyle(shown ? Qt::SolidLine : Qt::NoPen);
        m_picker.setRubberBandPen(p);
    }
}

PlotLinePtr QPCanvas::selectLine() const {
    return new QPLine(m_picker.rubberBandPen()); }

void QPCanvas::setSelectLine(const PlotLine& line) {
    if(line != *selectLine())
        m_picker.setRubberBandPen(QPLine(line).asQPen());
}


bool QPCanvas::gridShown(bool* xMajor, bool* xMinor, bool* yMajor,
        bool* yMinor) const {
            
    bool ret = false;
    
    bool tmp = m_canvas.grid().xEnabled();
    ret |= tmp;
    if(xMajor != NULL) *xMajor = tmp;
    
    ret |= tmp = m_canvas.grid().xMinEnabled();
    if(xMinor != NULL) *xMinor = tmp;
    
    ret |= tmp = m_canvas.grid().yEnabled();
    if(yMajor != NULL) *yMajor = tmp;
    
    ret |= tmp = m_canvas.grid().yMinEnabled();
    if(yMinor != NULL) *yMinor = tmp;
    
    return ret;
}

void QPCanvas::showGrid(bool xMajor, bool xMinor, bool yMajor,bool yMinor) {
    m_canvas.grid().enableX(xMajor);
    m_canvas.grid().enableXMin(xMinor);
    m_canvas.grid().enableY(yMajor);
    m_canvas.grid().enableYMin(yMinor);
}



PlotLinePtr QPCanvas::gridMajorLine() const {
    return new QPLine(m_canvas.grid().majPen()); 
}



void QPCanvas::setGridMajorLine(const PlotLine& line) {
    if(line != *gridMajorLine()) {
        QPLine l(line);
        
        m_canvas.grid().enableX(l.style() != PlotLine::NOLINE);
        m_canvas.grid().enableY(l.style() != PlotLine::NOLINE);
        m_canvas.grid().setMajPen(l.asQPen());
    }
}



PlotLinePtr QPCanvas::gridMinorLine() const {
    return new QPLine(m_canvas.grid().minPen()); }

void QPCanvas::setGridMinorLine(const PlotLine& line) {
    if(line != *gridMinorLine()) {
        QPLine l(line);
        
        m_canvas.grid().enableXMin(l.style() != PlotLine::NOLINE);
        m_canvas.grid().enableYMin(l.style() != PlotLine::NOLINE);
        m_canvas.grid().setMinPen(l.asQPen());
    }
}



bool QPCanvas::legendShown() const { 
    return m_legend->legendShown(); 
}



void QPCanvas::showLegend(bool on, LegendPosition pos) {
    m_legend->showLegend(on);
    m_legend->setPosition(pos);
}



PlotCanvas::LegendPosition QPCanvas::legendPosition() const {
    return m_legend->position(); 
}



void QPCanvas::setLegendPosition(LegendPosition pos) {   
    m_legend->setPosition(pos); 
}



PlotLinePtr QPCanvas::legendLine() const{ 
    return new QPLine(m_legend->line());
}



void QPCanvas::setLegendLine(const PlotLine& line) { m_legend->setLine(line); }



PlotAreaFillPtr QPCanvas::legendFill() const {
    return new QPAreaFill(m_legend->areaFill()); 
}



void QPCanvas::setLegendFill(const PlotAreaFill& area) {
    m_legend->setAreaFill(area); 
}



PlotFontPtr QPCanvas::legendFont() const { 
    return new QPFont(m_legendFont); 
}


    
void QPCanvas::setLegendFont(const PlotFont& font) {
    
    if(font != m_legendFont) {
        m_legendFont = font;
        for(unsigned int i = 0; i < m_plotItems.size(); i++) {
            QwtText t = m_plotItems[i].second->qwtTitle();
            t.setFont(m_legendFont.asQFont());
            t.setColor(m_legendFont.asQColor());
            m_plotItems[i].second->setQwtTitle(t);
        }
    }
    m_legendFontSet = true;
}


bool QPCanvas::autoIncrementColors() const { 
    return m_autoIncColors; 
}



void QPCanvas::setAutoIncrementColors(bool autoInc) {
    m_autoIncColors = autoInc; 
}



bool QPCanvas::exportToFile(const PlotExportFormat& format) {
    return QPExporter::exportCanvas(this, format);
}



String QPCanvas::fileChooserDialog(
            const String& title,
            const String& directory)    {
            
    QString filename = QFileDialog::getSaveFileName(this, title.c_str(),
            directory.c_str());
    return filename.toStdString();
}



const String& QPCanvas::dateFormat() const { 
    return m_dateFormat; 
}



void QPCanvas::setDateFormat(const String& dateFormat)   {
    
    if(m_dateFormat == dateFormat) return;
    m_dateFormat = dateFormat;
    for(int i = 0; i < QwtPlot::axisCnt; i++){
        m_scaleDraws[i]->setDateFormat(m_dateFormat);
    }
    int axisListenerCount = axisListeners.size();
    for ( int i = 0; i < axisListenerCount; i++ ){
    	axisListeners[i]->setDateFormat( m_dateFormat );
    }
}


const String& QPCanvas::relativeDateFormat() const {
    return m_relativeDateFormat; 
}


void QPCanvas::setRelativeDateFormat(const String& dateFormat)   {
    
    if(m_relativeDateFormat == dateFormat) return;
    m_relativeDateFormat = dateFormat;
    for(int i = 0; i < QwtPlot::axisCnt; i++){
        m_scaleDraws[i]->setRelativeDateFormat(m_relativeDateFormat);
    }
    int axisListenerCount = axisListeners.size();
    for ( int i = 0; i < axisListenerCount; i++ ){
    	axisListeners[i]->setRelativeDateFormat( m_dateFormat );
    }
}



PlotCoordinate QPCanvas::convertCoordinate(const PlotCoordinate& coord,
                                 PlotCoordinate::System newSystem) const  {
                                     
    if(coord.system() == newSystem) return coord;
    
    if(coord.system() == PlotCoordinate::WORLD) {
        if(newSystem == PlotCoordinate::NORMALIZED_WORLD) {
            prange_t range = axisRange(X_BOTTOM);
            double x = (coord.x() - range.first)/(range.second - range.first);            
            range = axisRange(Y_LEFT);
            double y = (coord.y() - range.first)/(range.second - range.first);
            
            return PlotCoordinate(x, y, newSystem);
            
        } 
        else if(newSystem == PlotCoordinate::PIXEL) {
            QwtScaleMap map = m_canvas.canvasMap(QwtPlot::xBottom);
            double x = map.xTransform(coord.x());
            map = m_canvas.canvasMap(QwtPlot::yLeft);
            double y = map.xTransform(coord.y());
            
            return PlotCoordinate(x, y, newSystem);
        }
        
    } 
    else if(coord.system() == PlotCoordinate::NORMALIZED_WORLD) {
        if(newSystem == PlotCoordinate::WORLD ||
           newSystem == PlotCoordinate::PIXEL) {
            prange_t range = axisRange(X_BOTTOM);
            double x = (coord.x()*(range.first - range.second)) + range.first;
            range = axisRange(Y_LEFT);
            double y = (coord.y()*(range.first - range.second)) + range.first;
            
            if(newSystem == PlotCoordinate::PIXEL) {
                QwtScaleMap map = m_canvas.canvasMap(QwtPlot::xBottom);
                x = map.xTransform(x);
                map = m_canvas.canvasMap(QwtPlot::yLeft);
                y = map.xTransform(y);
            }
            
            return PlotCoordinate(x, y, newSystem);

        }
        
    } 
    else if(coord.system() == PlotCoordinate::PIXEL) {
        if(newSystem == PlotCoordinate::WORLD ||
           newSystem == PlotCoordinate::NORMALIZED_WORLD) {
            QwtScaleMap map = m_canvas.canvasMap(QwtPlot::xBottom);
            double x = map.invTransform(coord.x());
            map = m_canvas.canvasMap(QwtPlot::yLeft);
            double y = map.invTransform(coord.y());
            
            if(newSystem == PlotCoordinate::NORMALIZED_WORLD) {
                prange_t r = axisRange(X_BOTTOM);
                x = (x - r.first) / (r.second - r.first);
                r = axisRange(Y_LEFT);
                y = (y - r.first) / (r.second - r.first);
            }
            
            return PlotCoordinate(x, y, newSystem);
        }
        
    }
    
    // somehow invalid
    return coord;
}



vector<double> QPCanvas::textWidthHeightDescent(const String& text,
                                                PlotFontPtr font) const   {
    vector<double> v(3, 0);

    QFontMetrics mets(QPFont(*font).asQFont());
    v[0] = mets.width(text.c_str());
    v[1] = mets.ascent();
    v[2] = mets.descent();
    
    return v;
}



PlotFactory* QPCanvas::implementationFactory() const { 
    return new QPFactory();
}



QPLayeredCanvas& QPCanvas::asQwtPlot() { 
    return m_canvas; 
}



const QPLayeredCanvas& QPCanvas::asQwtPlot() const { 
    return m_canvas; 
}



QwtPlotPicker& QPCanvas::getSelecter() { 
    return m_picker; 
}



void QPCanvas::reinstallTrackerFilter() {
    m_canvas.canvas()->removeEventFilter(&m_mouseFilter);
    m_canvas.canvas()->installEventFilter(&m_mouseFilter);
}


QSize QPCanvas::sizeHint() const { 
	QSize canvasSize = m_canvas.sizeHint();
    return m_canvas.sizeHint();
}


QSize QPCanvas::minimumSizeHint() const {
	QSize canvasSize = m_canvas.minimumSizeHint();
    return m_canvas.minimumSizeHint();
}

// Protected Methods //

void QPCanvas::setQPPlotter(QPPlotter* parent) {
    m_parent = parent;
    if(parent != NULL) {
        PlotLoggerPtr log = logger();
        if(!log.null())
            for(unsigned int i = 0; i < m_queuedLogs.size(); i++)
                log->postMessage(m_queuedLogs[i]);
        m_queuedLogs.clear();
    }
}


PlotLoggerPtr QPCanvas::logger() const {
    if(m_parent == NULL) return PlotLoggerPtr();
    else                 return m_parent->logger();
}


void QPCanvas::logObject(const String& className, void* address, bool creation,
        const String& message) {
    if(m_parent != NULL)
        m_parent->logObject(className, address, creation, message);
    else
        m_queuedLogs.push_back(
                PlotLogObject(className, address, creation, message,
                              PlotLogger::OBJECTS_MAJOR));
}


void QPCanvas::logMethod(const String& className, const String& methodName,
        bool entering, const String& message) {
    if(m_parent != NULL)
        m_parent->logMethod(className, methodName, entering, message);
}


QPAxesCache& QPCanvas::axesCache()   { 
    return m_stackCache; 
}


const QPAxesCache& QPCanvas::axesCache() const   {  
    return m_stackCache; 
}


void QPCanvas::mousePressEvent(QMouseEvent* event) {

    PlotMouseEvent::Button t = PlotMouseEvent::SINGLE;
    if(event->button() == Qt::RightButton) t = PlotMouseEvent::CONTEXT;
    else if(event->button() == Qt::MidButton) t = PlotMouseEvent::MIDDLE;

    PlotCoordinate pcoord = globalPosToPixelCoord(event);
    if(notifyPressHandlers(t, pcoord)) event->accept();
    else                               event->ignore();
}


void QPCanvas::mouseReleaseEvent(QMouseEvent* event) {    

    PlotMouseEvent::Button t = PlotMouseEvent::SINGLE;
    if(event->button() == Qt::RightButton) t = PlotMouseEvent::CONTEXT;
    else if(event->button() == Qt::MidButton) t = PlotMouseEvent::MIDDLE;
    
    PlotCoordinate pcoord = globalPosToPixelCoord(event);
    bool accept = notifyReleaseHandlers(t, pcoord);
    accept |= notifyClickHandlers(t, pcoord);
    
    /*
    if(event->button() == Qt::LeftButton) {
        if(!m_ignoreNextRelease) {
            m_timer.start(QApplication::doubleClickInterval());
            m_clickEvent = event;
        }
        m_ignoreNextRelease = false;
        
    } else if(event->button() == Qt::RightButton ||
              event->button() == Qt::MidButton) {
        // Also send a click event
        accept |= notifyClickHandlers(t, pcoord);
    }
    */
    
    if(accept) event->accept();
    else       event->ignore();
}


void QPCanvas::mouseDoubleClickEvent(QMouseEvent* event) {

    PlotCoordinate pcoord = globalPosToPixelCoord(event);
    
    // Send a double-click and a release event.
    bool accept = notifyClickHandlers(PlotMouseEvent::DOUBLE, pcoord);
    accept |= notifyReleaseHandlers(PlotMouseEvent::DOUBLE, pcoord);
    
    if(accept) event->accept();
    else       event->ignore();
}


void QPCanvas::trackerMouseEvent(QMouseEvent* event) {
    PlotMouseEvent::Button t = PlotMouseEvent::SINGLE;
    if(event->button() == Qt::RightButton) t = PlotMouseEvent::CONTEXT;
    else if(event->button() == Qt::MidButton) t = PlotMouseEvent::MIDDLE;
    
    PlotCoordinate pcoord = globalPosToPixelCoord(event);
    
    if(m_canvas.canvas()->cursor().shape()!= QPOptions::cursor(NORMAL_CURSOR)){
        QRect crect(m_canvas.canvas()->parentWidget()->mapToGlobal(
                    m_canvas.canvas()->pos()), m_canvas.canvas()->size());
        if(crect.contains(event->globalPos()))
            QWidget::setCursor(m_canvas.canvas()->cursor());
        else unsetCursor();
    } else unsetCursor();
    
    bool accept = notifyMoveHandlers(t, pcoord);
    
    // Also notify drag handlers if necessary.
    if(m_inDraggingMode) accept |= notifyDragHandlers(t, pcoord);
    
    if(accept) event->accept();
    else       event->ignore();
}


void QPCanvas::keyReleaseEvent(QKeyEvent* event) {
    int key = event->key();
    QString text = event->text();
    Qt::KeyboardModifiers mods = QApplication::keyboardModifiers();
    
    char c = ' ';
    vector<PlotKeyEvent::Modifier> modifiers;
    bool accept = false;
    
    if (key >= Qt::Key_F1 && key <= Qt::Key_F35) {
        modifiers.push_back(PlotKeyEvent::F);
        int i = key - Qt::Key_F1;
        c = '1' + i;
        accept = true;

    } 
    else if ((key >= Qt::Key_0 && key <= Qt::Key_9) ||
              (key >= Qt::Key_A && key <= Qt::Key_Z)) {        
        if(mods.testFlag(Qt::ControlModifier))
            modifiers.push_back(PlotKeyEvent::CONTROL);
        if(mods.testFlag(Qt::AltModifier))
            modifiers.push_back(PlotKeyEvent::ALT);
        if(mods.testFlag(Qt::ShiftModifier))
            modifiers.push_back(PlotKeyEvent::SHIFT);
        
        int i;        
        if(key >= Qt::Key_0 && key <= Qt::Key_9) {
            i = key - Qt::Key_0;
            c = '0' + i;
        } else {
            i = key - Qt::Key_A;
            c = 'a' + i;
        }
        accept = true;
        
    } 
    else if (key == Qt::Key_Space)  {
        c = ' ';
        accept=true; 
    }
    
    
    if(accept) accept = notifyKeyHandlers(c, modifiers);
    
    if(accept) event->accept();
    else       event->ignore();
}


void QPCanvas::wheelEvent(QWheelEvent* event) {
    int delta = (event->delta() / 8) / 15;
    PlotCoordinate pcoord = globalPosToPixelCoord(event);
    
    if(notifyWheelHandlers(delta, pcoord)) event->accept();
    else                                   event->ignore();
}


void QPCanvas::resizeEvent(QResizeEvent* event) {
    QSize o = event->oldSize(), n = event->size();
    if(notifyResizeHandlers(o.width(), o.height(), n.width(), n.height()))
        event->accept();
    else event->ignore();
}


// Private Methods //

PlotCoordinate QPCanvas::globalPosToPixelCoord(int x, int y) {
    QRect rect = m_canvas.canvas()->contentsRect();
    QPoint p = m_canvas.canvas()->mapToGlobal(rect.topLeft());
    return PlotCoordinate(x - p.x(), y - p.y(), PlotCoordinate::PIXEL);
}


// Private Slots //

void QPCanvas::regionSelected(const QwtDoubleRect& region) {
    // wtf?
    double l = (region.left() < region.right())? region.left(): region.right(),
           r = (region.right() > region.left())? region.right(): region.left(),
           t = (region.top() > region.bottom())? region.top(): region.bottom(),
           b = (region.bottom() < region.top())? region.bottom(): region.top();
           
           /*
    if(!region.isValid()) {
        // It was really a click.
        PlotCoordinate wcoord(l, t, PlotCoordinate::WORLD);
        notifyClickHandlers(PlotMouseEvent::SINGLE, wcoord);

    } else {
    */
    if(region.isValid()) {
        PlotRegion wreg(PlotCoordinate(l, t, PlotCoordinate::WORLD),
                        PlotCoordinate(r, b, PlotCoordinate::WORLD));
        notifySelectHandlers(wreg);
    }
}



}

#endif
