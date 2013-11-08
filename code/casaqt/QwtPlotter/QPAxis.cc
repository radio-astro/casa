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
const String QPAxis::EXPORT_NAME = "export";


/* static (for now. someday maybe make method of QPAxis, repl grabCanvas with self - dsw) */
QImage QPAxis::grabImageFromCanvas(
        const PlotExportFormat& format, 
        QPCanvas* grabCanvas,
        QPPlotter* grabPlotter
        )   
{
	QImage image;
    
    //image = QImage(width, height, QImage::Format_ARGB32);
	
    image = QPixmap::grabWidget(grabCanvas != NULL ?
            &grabCanvas->asQwtPlot() :
            grabPlotter->canvasWidget()).toImage();

    /* (DSW) this commented-out code appears to have been for
     *  producing screen capture images, but was buggy due to thread trouble,
     * so was commented out svn rev 7788 by LG
     */
    //if(grabCanvas != NULL) grabCanvas->asQwtPlot().render(&image);
    //else                   grabPlotter->render(&image);
    
    //QPainter::setRedirected(grabCanvas != NULL ?
    //        (QWidget*)&grabCanvas->asQwtPlot() :
    //        (QWidget*)grabPlotter->canvasWidget(), &image);
    //if(grabCanvas != NULL) grabCanvas->asQwtPlot().repaint();
    //else grabPlotter->canvasWidget()->repaint();
    //QPainter::restoreRedirected(grabCanvas != NULL ?
    //        (QWidget*)&grabCanvas->asQwtPlot() :
    //        (QWidget*)grabPlotter->canvasWidget());
    
    
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



/* static */
QImage QPAxis::produceHighResImage(
        const PlotExportFormat& format,
        vector<QPCanvas*> &qcanvases,
        int width, 
        int height,
        bool &wasCanceled
        )   
{

    // make sure both width and height are set
    int widgetWidth  = width, 
        widgetHeight = height;
    if (format.width > 0)  width  = format.width;
    if (format.height > 0) height = format.height;
    QImage image = QImage(width, height, QImage::Format_ARGB32);
    
    // Fill with background color.
    QPCanvas* canv = qcanvases[0];
    image.fill(canv->palette().color(canv->backgroundRole()).rgba());
    image.fill((uint)(-1));

    // Change canvas' background color to white for a nice bright clean image file
    //PlotAreaFillPtr  pAllWhite = QPFactory::areaFill("#f0822F");
    //PlotFactory* f = implementationFactory();
    PlotFactory* f = canv->implementationFactory();
    PlotAreaFillPtr paf = f->areaFill(String("#ff77ff"));
    delete f;
    paf->setColor("#FFFFFF");

    // Print each canvas.
    QPainter painter(&image);
    double widthRatio  = ((double)width)  / widgetWidth,
           heightRatio = ((double)height) / widgetHeight;
    QRect geom, printGeom, imageRect(0, 0, width, height);
    QwtText title; 
    QColor titleColor;
    PlotOperationPtr op;

    for (unsigned int i = 0; i < qcanvases.size(); i++) {
        canv = qcanvases[i];
        PlotAreaFillPtr originalBackground = canv->background();
        canv->setBackground(*paf);

        geom = canv->geometry();
        printGeom = QRect((int)((geom.x() * widthRatio) + 0.5),
                          (int)((geom.y() * heightRatio) + 0.5),
                          (int)((geom.width() * widthRatio) + 0.5),
                          (int)((geom.height() * heightRatio) + 0.5));
        printGeom &= imageRect;
        titleColor = canv->asQwtPlot().title().color();
        
        op = canv->operationExport();
        if (!op.null()) wasCanceled |= op->cancelRequested();
        if (wasCanceled) break;
        canv->asQwtPlot().print(&painter, printGeom);
        
        // For bug where title color changes after a print.
        title = canv->asQwtPlot().title();
        if (title.color() != titleColor) {
            title.setColor(titleColor);
            canv->asQwtPlot().setTitle(title);
        }

        canv->setBackground(originalBackground);

        if (!op.null()) wasCanceled |= op->cancelRequested();
        if (wasCanceled) break;
    }
    
    return image;
}




bool QPAxis::exportToImageFile(
        const PlotExportFormat& format, 
        vector<QPCanvas*> &qcanvases,
        QPCanvas* grabCanvas,
        QPPlotter* grabPlotter
        )   
{
    
    QImage image;
    
    int width  = grabCanvas != NULL ? grabCanvas->width()  : grabPlotter->width();
    int height = grabCanvas != NULL ? grabCanvas->height() : grabPlotter->height();    
    
    // Remember the current background color, used for on-screen GUI
    // We want to temporarily change this to white for making the image file.
    // Later we will restore this color.
    PlotAreaFillPtr normal_background;
    if(grabCanvas != NULL)
        normal_background = grabCanvas->background();
    
    
    // Just grab the widget if: 1) screen resolution, or 2) high resolution
    // but size is <= widget size or not set.
    bool wasCanceled = false;
    if(format.resolution == PlotExportFormat::SCREEN)    {
        image=grabImageFromCanvas(format, grabCanvas, grabPlotter);
    } 
    else {
        // High resolution, or format size larger than widget.
        image=produceHighResImage(format, qcanvases, width, height,  wasCanceled);
    }

    // Set DPI.
    if(!wasCanceled && format.dpi > 0) {
        // convert dpi to dpm
        int dpm = QPOptions::round((format.dpi / 2.54) * 100);
        image.setDotsPerMeterX(dpm);
        image.setDotsPerMeterY(dpm);
    }
    
    
    // Set output quality.
    bool hires = (format.resolution == PlotExportFormat::HIGH);
    int quality;    // see QImage.save official documentation for its meaning
    switch (format.type)
    {
        case PlotExportFormat::JPG:
                // JPEG quality ranges from 0 (crude 8x8 blocks) to 100 (best)
                quality= (hires)? 99: 95;   // experimental; need user feedback 
                break; 
        
        case PlotExportFormat::PNG:
                // Compression is lossless.  "quality" is number of deflations.
                // First one does great compression.  More buy only a small %.      
                // Set to -1 for no compression.
                quality=1; 
                break; 
        
        default: 
                quality=-1;  // no compression of undefined/unknown formats
    }
    
    // Save to file.
    bool save_ok;
    save_ok= image.save(format.location.c_str(),
          PlotExportFormat::exportFormat(format.type).c_str(), 
          quality);
    
    // Restore background color
    if(grabCanvas != NULL)
        grabCanvas->setBackground(normal_background);
    
    return !wasCanceled && !image.isNull() && save_ok;
}




bool QPAxis::exportPostscript(
        const PlotExportFormat& format, 
        vector<QPCanvas*> &qcanvases
        )
{
    // Set resolution.
    QPrinter::PrinterMode mode = QPrinter::ScreenResolution;
    if(format.resolution == PlotExportFormat::HIGH)
        mode = QPrinter::HighResolution;
    
    // Set file.
    QPrinter printer(mode);
    switch (format.type)   {
        case PlotExportFormat::PDF:
                printer.setOutputFormat(QPrinter::PdfFormat);
                break;
        case PlotExportFormat::PS:
                printer.setOutputFormat(QPrinter::PostScriptFormat);
                break;
        default: {}
    }

    printer.setOutputFileName(format.location.c_str());

    
    // Set output settings.
    if(format.dpi > 0) printer.setResolution(format.dpi);
        printer.setColorMode(QPrinter::Color);
    
    // Print each canvas.
    bool flag = false;
    QwtText title; QColor titleColor;
    PlotOperationPtr op;
    bool wasCanceled = false;
    for(unsigned int i = 0; i < qcanvases.size(); i++) {
        QPCanvas *canv = qcanvases[i];
        
        // don't do new page only for first non-null canvas
        if(flag) printer.newPage();
        flag = true;
        
        // Set orientation.
        printer.setOrientation(canv->width() >= canv->height() ?
                      QPrinter::Landscape : QPrinter::Portrait);
              
        titleColor = canv->asQwtPlot().title().color();


        op = canv->operationExport();
        if(!op.null()) wasCanceled |= op->cancelRequested();
        if(wasCanceled) break;
        canv->asQwtPlot().print(printer);
        
        // For bug where title color changes after a print.
        title = canv->asQwtPlot().title();
        if(title.color() != titleColor) {
            title.setColor(titleColor);
            canv->asQwtPlot().setTitle(title);
        }
        
        if(!op.null()) wasCanceled |= op->cancelRequested();
        if(wasCanceled) break;
    }
    
    return !wasCanceled;

}



/* Formerly named exportHelper()  */
bool QPAxis::exportCanvases  (
		vector<PlotCanvasPtr>& /*canvases*/,
        const PlotExportFormat& format, 
        QPCanvas* grabCanvas,
        QPPlotter* grabPlotter)     
{
    
    if (format.location.empty() || ((format.type == PlotExportFormat::JPG ||
       format.type == PlotExportFormat::PNG) && (grabCanvas == NULL &&
       grabPlotter == NULL)))
        return false;
    
    // Compile vector of unique, non-null QPAxises.
    vector<QPCanvas*> qcanvases;
    //QPCanvas* canv;
    //bool found = false;
    /*for(unsigned int i = 0; i < canvases.size(); i++) {
        if(canvases[i].null()) continue;
        canv = dynamic_cast<QPCanvas*>(&*canvases[i]);
        if(canv == NULL) continue;
        
        found = false;
        for(unsigned int j = 0; !found && j < qcanvases.size(); j++)
            if(qcanvases[j] == canv) found = true;
        if(!found) qcanvases.push_back(canv);
    }
    
    if(qcanvases.size() == 0) return false;
    
    
    // Compile vector of unique, non-null loggers for export event.
    vector<PlotLoggerPtr> loggers;
    PlotLoggerPtr logger;
    for(unsigned int i = 0; i < qcanvases.size(); i++) {        
        logger = qcanvases[i]->logger();
        if(logger.null()) continue;
        
        found = false;
        for(unsigned int j = 0; !found && j < loggers.size(); j++)
            if(loggers[j] == logger) found = true;
        if(!found) loggers.push_back(logger);
    }
    
    
    // Start logging.
    for(unsigned int i = 0; i < loggers.size(); i++)
        loggers[i]->markMeasurement(CLASS_NAME, EXPORT_NAME,
                                    PlotLogger::EXPORT_TOTAL);
        
    bool ret = false;

    switch (format.type)
    {
        case PlotExportFormat::JPG:
        case PlotExportFormat::PNG:
                ret=exportToImageFile(format, qcanvases, grabCanvas, grabPlotter);
                break;
        case PlotExportFormat::PS:
        case PlotExportFormat::PDF:
                ret=exportPostscript(format, qcanvases);
                break;
        case PlotExportFormat::TEXT:
        case PlotExportFormat::NUM_FMTS:
        	qDebug() << "Cannot export axis in format: "<<format.type;
        	break;
    }
    

    // End logging.
    for(unsigned int i = 0; i < loggers.size(); i++)
        loggers[i]->releaseMeasurement();
    
    return ret;*/
    return false;
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

QPAxis::QPAxis(PlotAxis plotAxis, QPPlotter* parent) : m_parent(parent),// m_canvas(NULL),
        m_axesRatioLocked(false), m_axesRatios(4, 1), //m_stackCache(m_canvas.canvas()),
        //m_autoIncColors(false), /*m_picker(m_canvas.canvas())*/,
        //m_mouseFilter(m_canvas.canvas()), //m_legendFontSet(false),
        /*, m_ignoreNextRelease(false), m_timer(this),
        m_clickEvent(NULL)*/
        axisWidget( NULL ){
    logObject(CLASS_NAME, this, true);

    axisType = QPOptions::axis(plotAxis);
    //m_inDraggingMode = false;
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    //setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setFocusPolicy(Qt::StrongFocus);
    
    QSizePolicy sizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
    //Make the external axis we will be using
    if ( plotAxis  == Y_LEFT ){
    	axisWidget = new ExternalAxisWidgetLeft( this );
    	sizePolicy.setHorizontalPolicy( QSizePolicy::Fixed );
    }
    else if ( plotAxis == X_BOTTOM ){
    	axisWidget = new ExternalAxisWidgetBottom( this );
    	sizePolicy.setVerticalPolicy( QSizePolicy::Fixed );
    }
    else if ( plotAxis == Y_RIGHT ){
    	axisWidget = new ExternalAxisWidgetRight( this );
    	sizePolicy.setHorizontalPolicy( QSizePolicy::Fixed );
    }
    else if ( plotAxis == X_TOP ){
    	axisWidget = new ExternalAxisWidgetTop( this );
    	sizePolicy.setVerticalPolicy( QSizePolicy::Fixed );
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
    


    m_dateFormat = Plotter::DEFAULT_DATE_FORMAT;
    m_relativeDateFormat = Plotter::DEFAULT_RELATIVE_DATE_FORMAT;
    /*for(int i = 0; i < QwtPlot::axisCnt; i++) {
        m_scaleDraws[i] = new QPScaleDraw(&m_canvas, QwtPlot::Axis(i));
        m_scaleDraws[i]->setDateFormat(m_dateFormat);
        m_scaleDraws[i]->setRelativeDateFormat(m_relativeDateFormat);
        m_canvas.setAxisScaleDraw(QwtPlot::Axis(i), m_scaleDraws[i]);
    }*/
    

    //m_canvas.enableAxis(QwtPlot::xBottom, false);
    //m_canvas.enableAxis(QwtPlot::yLeft, false);
    //m_canvas.setAutoReplot(true);
    
    //connect(&m_picker, SIGNAL(selected(const QwtDoubleRect&)),
      //      this, SLOT(regionSelected(const QwtDoubleRect&)));


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
    /*return new QPAreaFill(m_canvas.canvas()->palette().brush(
                          QPalette::Window));
                          */
	return PlotAreaFillPtr();
}



void QPAxis::setBackground(const PlotAreaFill& /*areaFill*/) {
    /*QPAreaFill a(areaFill);
    QPalette p = m_canvas.canvas()->palette();
    p.setBrush(QPalette::Window, a.asQBrush());
    m_canvas.canvas()->setPalette(p);*/
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
    /*bool show;
    for(int i = 0; i < QwtPlot::axisCnt;  i++) {
    show = axes & QPOptions::axis(QwtPlot::Axis(i));
        m_canvas.enableAxis(QwtPlot::Axis(i), show);
    }*/
}


PlotAxisScale QPAxis::axisScale(PlotAxis /*axis*/) const   {
    //return m_scaleDraws[QPOptions::axis(axis)]->scale();
	return NORMAL;
}


void QPAxis::setAxisScale(PlotAxis /*axis*/, PlotAxisScale /*scale*/) {
    //m_scaleDraws[QPOptions::axis(axis)]->setScale(scale);
}


bool QPAxis::axisReferenceValueSet(PlotAxis /*axis*/) const {
    return false;
}


double QPAxis::axisReferenceValue(PlotAxis /*axis*/) const {
    return -1;
}


void QPAxis::setAxisReferenceValue(PlotAxis /*axis*/, bool /*on*/, double /*value*/) {
    //m_scaleDraws[QPOptions::axis(axis)]->setReferenceValue(on, value);
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
    //return m_canvas.axisTitle(QPOptions::axis(axis)).text().toStdString();
	String axisLabel;
	if ( axisWidget != NULL && QPOptions::axis(axisType) == axis ){
		axisLabel= axisWidget->getAxisLabel().toStdString();
	}
	return axisLabel;
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



void QPAxis::setAxisFont(PlotAxis /*axis*/, const PlotFont& /*font*/) {
    
    /*if(font != *axisFont(axis)) {
        QPFont f(font);
        QwtText t = m_canvas.axisTitle(QPOptions::axis(axis));
        t.setFont(f.asQFont());
        t.setColor(f.asQColor());
        m_canvas.setAxisTitle(QPOptions::axis(axis), t);
        f.setBold(false);
        m_canvas.setAxisFont(QPOptions::axis(axis), f.asQFont());
        if(!t.isEmpty()) m_canvas.enableAxis(QPOptions::axis(axis));
    }*/
}



bool QPAxis::colorBarShown(PlotAxis /*axis*/) const {
    //return m_canvas.axisWidget(QPOptions::axis(axis))->isColorBarEnabled();
	return false;
}



void QPAxis::showColorBar(bool /*show*/, PlotAxis /*axis*/) {
    
    /*QwtScaleWidget* scale = m_canvas.axisWidget(QPOptions::axis(axis));
    
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

    m_canvas.enableAxis(QPOptions::axis(axis), true);
    if(v.first != v.second) setAxisRange(axis, v.first, v.second);
    else                    setAxisRange(axis, v.first - 0.5, v.second + 0.5);
    */
}


prange_t QPAxis::axisRange(PlotAxis /*axis*/) const {
    
/*    const QwtScaleDiv* div = m_canvas.axisScaleDiv(QPOptions::axis(axis));
#if QWT_VERSION < 0x050200
    return prange_t(div->lBound(), div->hBound());
#else
    return prange_t(div->lowerBound(), div->upperBound());
#endif
*/
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



String QPAxis::fileChooserDialog(
            const String& title,
            const String& directory)    {
            
    QString filename = QFileDialog::getSaveFileName(this, title.c_str(),
            directory.c_str());
    return filename.toStdString();
}



const String& QPAxis::dateFormat() const {
    return m_dateFormat; 
}



void QPAxis::setDateFormat(const String& /*dateFormat*/)   {
    
   /* if(m_dateFormat == dateFormat) return;
    m_dateFormat = dateFormat;
    for(int i = 0; i < QwtPlot::axisCnt; i++)
        m_scaleDraws[i]->setDateFormat(m_dateFormat);*/
}



const String& QPAxis::relativeDateFormat() const {
    return m_relativeDateFormat; 
}



void QPAxis::setRelativeDateFormat(const String& /*dateFormat*/)   {
    
    /*if(m_relativeDateFormat == dateFormat) return;
    m_relativeDateFormat = dateFormat;
    for(int i = 0; i < QwtPlot::axisCnt; i++)
        m_scaleDraws[i]->setRelativeDateFormat(m_relativeDateFormat);*/
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



/*void QPAxis::mousePressEvent(QMouseEvent* event) {

    PlotMouseEvent::Button t = PlotMouseEvent::SINGLE;
    if(event->button() == Qt::RightButton) t = PlotMouseEvent::CONTEXT;
    else if(event->button() == Qt::MidButton) t = PlotMouseEvent::MIDDLE;

    PlotCoordinate pcoord = globalPosToPixelCoord(event);
    if(notifyPressHandlers(t, pcoord)) event->accept();
    else                               event->ignore();
}*/



/*void QPAxis::mouseReleaseEvent(QMouseEvent* event) {

    PlotMouseEvent::Button t = PlotMouseEvent::SINGLE;
    if(event->button() == Qt::RightButton) t = PlotMouseEvent::CONTEXT;
    else if(event->button() == Qt::MidButton) t = PlotMouseEvent::MIDDLE;
    
    PlotCoordinate pcoord = globalPosToPixelCoord(event);
    bool accept = notifyReleaseHandlers(t, pcoord);
    accept |= notifyClickHandlers(t, pcoord);
    

    
    if(accept) event->accept();
    else       event->ignore();
}*/



/*void QPAxis::mouseDoubleClickEvent(QMouseEvent* event) {

    PlotCoordinate pcoord = globalPosToPixelCoord(event);
    
    // Send a double-click and a release event.
    bool accept = notifyClickHandlers(PlotMouseEvent::DOUBLE, pcoord);
    accept |= notifyReleaseHandlers(PlotMouseEvent::DOUBLE, pcoord);
    
    if(accept) event->accept();
    else       event->ignore();
}*/















void QPAxis::setPlot( QwtPlot* plot ){
	if ( axisWidget != NULL ){
		axisWidget->setPlot( plot );
	}
}

}

#endif
