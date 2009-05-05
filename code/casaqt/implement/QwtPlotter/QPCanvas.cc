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
#include <casaqt/QwtPlotter/QPPlotter.qo.h>
#include <casaqt/QwtPlotter/QPRasterPlot.h>
#include <casaqt/QwtPlotter/QPShape.h>

#include <qwt_scale_engine.h>
#include <qwt_scale_widget.h>

namespace casa {

//////////////////////////
// QPCANVAS DEFINITIONS //
//////////////////////////

// Macros to reset tool stacks as needed.
#define PRE_REPLOT                                                            \
    vector<pair<double, double> > preAxes(4);                                 \
    preAxes[0] = axisRange(X_BOTTOM); preAxes[1] = axisRange(X_TOP);          \
    preAxes[2] = axisRange(Y_LEFT); preAxes[3] = axisRange(Y_RIGHT);

#define POST_REPLOT                                                           \
    vector<pair<double, double> > postAxes(4);                                \
    postAxes[0] = axisRange(X_BOTTOM); postAxes[1] = axisRange(X_TOP);        \
    postAxes[2] = axisRange(Y_LEFT); postAxes[3] = axisRange(Y_RIGHT);        \
    if(preAxes[0] != postAxes[0] || preAxes[1] != postAxes[1] ||              \
       preAxes[2] != postAxes[2] || preAxes[3] != postAxes[3])                \
        resetMouseTools();

// Static //

double QPCanvas::zOrder = 1;

const String QPCanvas::CLASS_NAME = "QPCanvas";
const String QPCanvas::DRAW_NAME = "drawItems";
const String QPCanvas::EXPORT_NAME = "export";

bool QPCanvas::exportPlotter(QPPlotter* plotter, const PlotExportFormat& fmt) {
    vector<PlotCanvasPtr> canvases;
    if(plotter != NULL && !plotter->canvasLayout().null())
        canvases = plotter->canvasLayout()->allCanvases();
    return exportHelper(canvases, fmt, NULL, plotter);
}

bool QPCanvas::exportCanvas(QPCanvas* canvas, const PlotExportFormat& format) {
    vector<PlotCanvasPtr> canvases(1, PlotCanvasPtr(canvas, false));
    return exportHelper(canvases, format, canvas, NULL);
}


bool QPCanvas::exportHelper(vector<PlotCanvasPtr>& canvases,
		const PlotExportFormat& format, QPCanvas* grabCanvas,
		QPPlotter* grabPlotter) {
    if(format.location.empty() || ((format.type == PlotExportFormat::JPG ||
       format.type == PlotExportFormat::PNG) && (grabCanvas == NULL &&
       grabPlotter == NULL)))
        return false;
    
    // Compile vector of unique, non-null QPCanvases.
    vector<QPCanvas*> qcanvases;
    QPCanvas* canv;
    bool found = false;
    for(unsigned int i = 0; i < canvases.size(); i++) {
        if(canvases[i].null()) continue;
        canv = dynamic_cast<QPCanvas*>(&*canvases[i]);
        if(canv == NULL) continue;
        
        found = false;
        for(unsigned int j = 0; !found && j < qcanvases.size(); j++)
            if(qcanvases[j] == canv) found = true;
        if(!found) qcanvases.push_back(canv);
    }
    
    if(qcanvases.size() == 0) return false;
    
    bool wasCanceled = false;
    bool ret = false;
    
    // Compile vector of unique, non-null loggers for export event.
    vector<PlotLoggerPtr> loggers;
    PlotLoggerPtr logger;
    for(unsigned int i = 0; i < qcanvases.size(); i++) {        
        logger = qcanvases[i]->loggerForEvent(PlotLogger::EXPORT_TOTAL);
        if(logger.null()) continue;
        
        found = false;
        for(unsigned int j = 0; !found && j < loggers.size(); j++)
            if(loggers[j] == logger) found = true;
        if(!found) loggers.push_back(logger);
    }
    
    // Start logging.
    for(unsigned int i = 0; i < loggers.size(); i++)
        loggers[i]->markMeasurement(CLASS_NAME, EXPORT_NAME);
    
    // Image
    if(format.type == PlotExportFormat::JPG ||
       format.type == PlotExportFormat::PNG) {
        QImage image;
        
        int width = grabCanvas != NULL ? grabCanvas->width() :
                    grabPlotter->width(),
           height = grabCanvas != NULL ? grabCanvas->height() :
                    grabPlotter->height();
        
        // Just grab the widget if: 1) screen resolution, or 2) high resolution
        // but size is <= widget size or not set.
        if(format.resolution == PlotExportFormat::SCREEN/* ||
           (format.width <= width && format.height <= height)*/) {
            //image = QImage(width, height, QImage::Format_ARGB32);
   	
        	// TODO
            image = QPixmap::grabWidget(grabCanvas != NULL ?
                    &grabCanvas->asQwtPlot() :
                    grabPlotter->canvasWidget()).toImage();
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
            if(format.width > 0 && format.height > 0) {
                // size is specified
                image = image.scaled(format.width, format.height,
                        Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            
            } else if(format.width > 0) {
                // width is specified            
                image = image.scaledToWidth(format.width,
                        Qt::SmoothTransformation);
            
            } else if(format.height > 0) {
                //  height is specified            
                image = image.scaledToHeight(format.height,
                        Qt::SmoothTransformation);
            }
        } else {
            // High resolution, or format size larger than widget.

            // make sure both width and height are set
            int widgetWidth = width, widgetHeight = height;
            if(format.width > 0)  width  = format.width;
            if(format.height > 0) height = format.height;
            image = QImage(width, height, QImage::Format_ARGB32);
            
            // Fill with background color.
            canv = qcanvases[0];
            image.fill(canv->palette().color(canv->backgroundRole()).rgba());
            
            // Print each canvas.
            QPainter painter(&image);
            double widthRatio  = ((double)width)  / widgetWidth,
                   heightRatio = ((double)height) / widgetHeight;
            QRect geom, printGeom, imageRect(0, 0, width, height);
            QwtText title; QColor titleColor;
            PlotOperationPtr op;
            for(unsigned int i = 0; i < qcanvases.size(); i++) {
                canv = qcanvases[i];
                
                geom = canv->geometry();
                printGeom = QRect((int)((geom.x() * widthRatio) + 0.5),
                                  (int)((geom.y() * heightRatio) + 0.5),
                                  (int)((geom.width() * widthRatio) + 0.5),
                                  (int)((geom.height() * heightRatio) + 0.5));
                printGeom &= imageRect;
                titleColor = canv->asQwtPlot().title().color();
                
                op = canv->operationExport();
                if(!op.null()) wasCanceled |= op->cancelRequested();
                if(wasCanceled) break;
                canv->asQwtPlot().print(&painter, printGeom);
                
                // For bug where title color changes after a print.
                title = canv->asQwtPlot().title();
                if(title.color() != titleColor) {
                    title.setColor(titleColor);
                    canv->asQwtPlot().setTitle(title);
                }
                
                if(!op.null()) wasCanceled |= op->cancelRequested();
                if(wasCanceled) break;
            }
        }

        // Set DPI.
        if(!wasCanceled && format.dpi > 0) {
            // convert dpi to dpm
            int dpm = QPOptions::round((format.dpi / 2.54) * 100);
            image.setDotsPerMeterX(dpm);
            image.setDotsPerMeterY(dpm);
        }
        
        // Set output quality.
        int f = (format.resolution == PlotExportFormat::HIGH) ? 100 : -1;
        
        // Save to file.
        ret = !wasCanceled && !image.isNull() &&
              image.save(format.location.c_str(),
              PlotExportFormat::exportFormat(format.type).c_str(), f);
        
    // PS/PDF
    } else if(format.type == PlotExportFormat::PS ||
              format.type == PlotExportFormat::PDF) {
        // Set resolution.
        QPrinter::PrinterMode mode = QPrinter::ScreenResolution;
        if(format.resolution == PlotExportFormat::HIGH)
            mode = QPrinter::HighResolution;
        
        // Set file.
        QPrinter printer(mode);
        if(format.type == PlotExportFormat::PDF)
            printer.setOutputFormat(QPrinter::PdfFormat);
        else printer.setOutputFormat(QPrinter::PostScriptFormat);
        printer.setOutputFileName(format.location.c_str());
        
        // Set output settings.
        if(format.dpi > 0) printer.setResolution(format.dpi);
        printer.setColorMode(QPrinter::Color);
        
        // Print each canvas.
        bool flag = false;
        QwtText title; QColor titleColor;
        PlotOperationPtr op;
        for(unsigned int i = 0; i < qcanvases.size(); i++) {
            canv = qcanvases[i];
            
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
        
        ret = !wasCanceled;
    }
    
    // End logging.
    for(unsigned int i = 0; i < loggers.size(); i++)
        loggers[i]->releaseMeasurement();
    
    return ret;
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
        m_inDraggingMode(false), m_ignoreNextRelease(false), m_timer(this),
        m_clickEvent(NULL) {
    logObject(CLASS_NAME, this, true);
    
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
    
    m_canvas.enableAxis(QwtPlot::xBottom, false);
    m_canvas.enableAxis(QwtPlot::yLeft, false);
    m_canvas.setAutoReplot(true);
    
    connect(&m_picker, SIGNAL(selected(const QwtDoubleRect&)),
            this, SLOT(regionSelected(const QwtDoubleRect&)));
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
}

QPCanvas::~QPCanvas() {
    vector<PlotMouseToolPtr> tools = allMouseTools();
    for(unsigned int i = 0; i < tools.size(); i++)
        unregisterMouseTool(tools[i]);
    
    logObject(CLASS_NAME, this, false);
}


// Public Methods //

pair<int, int> QPCanvas::size() const {
    return pair<int, int>(QWidget::width(), QWidget::height()); }

String QPCanvas::title() const {
    return m_canvas.title().text().toStdString(); }

void QPCanvas::setTitle(const String& title) {
    m_canvas.setTitle(title.c_str()); }

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
    return QPOptions::cursor(m_canvas.cursor().shape()); }

void QPCanvas::setCursor(PlotCursor cursor) {
    QWidget::setCursor(QPOptions::cursor(cursor));
    m_canvas.canvas()->setCursor(QPOptions::cursor(cursor)); }

void QPCanvas::refresh() {
    logMethod(CLASS_NAME, "refresh", true);
    PRE_REPLOT
    QCoreApplication::processEvents();
    m_canvas.replot();
    POST_REPLOT
    logMethod(CLASS_NAME, "refresh", false);
}

void QPCanvas::refresh(int drawLayersFlag) {
    logMethod(CLASS_NAME, "refresh(int)", true);
    
    if(drawLayersFlag != 0) {
        PRE_REPLOT
        QCoreApplication::processEvents();
        m_canvas.setLayersChanged(drawLayersFlag);
        m_canvas.replot();
        POST_REPLOT
    }
    logMethod(CLASS_NAME, "refresh(int)", false);
}


int QPCanvas::shownAxes() const {
    int axes = 0;
    for(int i = 0; i < QwtPlot::axisCnt; i++) {
        if(m_canvas.axisEnabled(i))
            axes |= QPOptions::axis(QwtPlot::Axis(i));
    }
    return axes;
}

void QPCanvas::showAxes(int axesFlag) {
    bool show;
    for(int i = 0; i < QwtPlot::axisCnt; i++) {
        show = axesFlag & QPOptions::axis(QwtPlot::Axis(i));
        m_canvas.enableAxis(QwtPlot::Axis(i), show);
    }
}

PlotAxisScale QPCanvas::axisScale(PlotAxis axis) const {
    const QwtScaleEngine* e = m_canvas.axisScaleEngine(QPOptions::axis(axis));
    
    if(dynamic_cast<const QwtLinearScaleEngine*>(e) != NULL) {
        const QwtScaleDraw* d = m_canvas.axisScaleDraw(QPOptions::axis(axis));
        const QPDateScaleDraw* dd = dynamic_cast<const QPDateScaleDraw*>(d);
        if(dd != NULL) return dd->scale();
        else           return NORMAL;
    }
    if(dynamic_cast<const QwtLog10ScaleEngine*>(e) != NULL) return LOG10;
    
    return NORMAL;
}

void QPCanvas::setAxisScale(PlotAxis axis, PlotAxisScale scale) {
    if(scale != axisScale(axis)) {
        switch(scale) {
        case NORMAL:
            m_canvas.setAxisScaleEngine(QPOptions::axis(axis),
                                        new QwtLinearScaleEngine());
            m_canvas.setAxisScaleDraw(QPOptions::axis(axis),
                                      new QwtScaleDraw());
            break;
            
        case LOG10:
            m_canvas.setAxisScaleEngine(QPOptions::axis(axis),
                                        new QwtLog10ScaleEngine());
            m_canvas.setAxisScaleDraw(QPOptions::axis(axis),
                                      new QwtScaleDraw());
            break;
            
        case DATE_MJ_DAY: case DATE_MJ_SEC:
            m_canvas.setAxisScaleEngine(QPOptions::axis(axis),
                                        new QwtLinearScaleEngine());
            m_canvas.setAxisScaleDraw(QPOptions::axis(axis),
                                      new QPDateScaleDraw(scale));
            break;
            
        default: return;
        }

        if(m_canvas.autoReplot()) m_canvas.replot();
    }
}

bool QPCanvas::cartesianAxisShown(PlotAxis axis) const {
    return m_canvas.cartesianAxisShown(axis); }

void QPCanvas::showCartesianAxis(PlotAxis mirrorAxis, PlotAxis secondaryAxis,
                                 bool show, bool hideNormalAxis) {
    m_canvas.showCartesianAxis(mirrorAxis, secondaryAxis, show);
    showAxis(mirrorAxis, !hideNormalAxis);
}

String QPCanvas::axisLabel(PlotAxis axis) const {
    return m_canvas.axisTitle(QPOptions::axis(axis)).text().toStdString(); }

void QPCanvas::setAxisLabel(PlotAxis axis, const String& title) {
    m_canvas.setAxisTitle(QPOptions::axis(axis), title.c_str());
    m_canvas.enableAxis(QPOptions::axis(axis));
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
        if(!t.isEmpty()) m_canvas.enableAxis(QPOptions::axis(axis));
    }
}

bool QPCanvas::colorBarShown(PlotAxis axis) const {
    return m_canvas.axisWidget(QPOptions::axis(axis))->isColorBarEnabled(); }

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
    
    pair<double, double> v = r->rasterData()->valueRange();
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
}


pair<double, double> QPCanvas::axisRange(PlotAxis axis) const {
    const QwtScaleDiv* div = m_canvas.axisScaleDiv(QPOptions::axis(axis));
#if QWT_VERSION < 0x050200
    return pair<double, double>(div->lBound(), div->hBound());
#else
    return pair<double, double>(div->lowerBound(), div->upperBound());
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

bool QPCanvas::axesRatioLocked() const { return m_axesRatioLocked; }

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
    return m_stackCache.memoryLimit(); }
void QPCanvas::setCachedAxesStackSizeLimit(int sizeInKilobytes) {
    m_stackCache.setMemoryLimit(sizeInKilobytes); }

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
        
        QPColor color(QColor(QPPlotter::GLOBAL_COLORS[ri]));
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
    if(!cartesianAxisShown(xAxis)) 
        m_canvas.enableAxis(QPOptions::axis(xAxis), true);
    if(!cartesianAxisShown(yAxis)) 
        m_canvas.enableAxis(QPOptions::axis(yAxis), true);
    
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

unsigned int QPCanvas::numPlotItems() const { return m_plotItems.size(); }

unsigned int QPCanvas::numLayerPlotItems(PlotCanvasLayer layer) const {
    if(layer == MAIN) return m_plotItems.size();
    else              return m_layeredItems.size();
}

void QPCanvas::removePlotItems(const vector<PlotItemPtr>& items) {
    logMethod(CLASS_NAME, "removePlotItems", true);
    int changedLayers = 0;
    bool replot = m_canvas.autoReplot();
    m_canvas.setAutoReplot(false);
    for(unsigned int i = 0; i < items.size(); i++) {
        if(items[i].null()) continue;
        for(unsigned int j = 0; j < m_plotItems.size(); j++) {
            if(items[i] == m_plotItems[j].first) {
                m_plotItems[j].second->detach();
                m_plotItems.erase(m_plotItems.begin() + j);
                changedLayers |= MAIN;
                break;
            }
        }
        for(unsigned int j = 0; j < m_layeredItems.size(); j++) {
            if(items[i] == m_layeredItems[j].first) {
                m_layeredItems[j].second->detach();
                m_layeredItems.erase(m_layeredItems.begin() + j);
                changedLayers |= ANNOTATION;
                break;
            }
        }
    }
    if(changedLayers != 0 && replot) {
        PRE_REPLOT
        m_canvas.setLayersChanged(changedLayers);
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


void QPCanvas::holdDrawing() { m_canvas.holdDrawing(); }

void QPCanvas::releaseDrawing() {
    logMethod(CLASS_NAME, "releaseDrawing", true);
    PRE_REPLOT
    QCoreApplication::processEvents();
    m_canvas.releaseDrawing();
    POST_REPLOT
    logMethod(CLASS_NAME, "releaseDrawing", false);
}

bool QPCanvas::drawingIsHeld() const { return m_canvas.drawingIsHeld(); }


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
    return new QPLine(m_canvas.grid().majPen()); }

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


bool QPCanvas::legendShown() const { return m_legend->legendShown(); }

void QPCanvas::showLegend(bool on, LegendPosition pos) {
    m_legend->showLegend(on);
    m_legend->setPosition(pos);
}

PlotCanvas::LegendPosition QPCanvas::legendPosition() const {
    return m_legend->position(); }

void QPCanvas::setLegendPosition(LegendPosition pos) {   
    m_legend->setPosition(pos); }

PlotLinePtr QPCanvas::legendLine() const{ return new QPLine(m_legend->line());}
void QPCanvas::setLegendLine(const PlotLine& line) { m_legend->setLine(line); }

PlotAreaFillPtr QPCanvas::legendFill() const {
    return new QPAreaFill(m_legend->areaFill()); }
void QPCanvas::setLegendFill(const PlotAreaFill& area) {
    m_legend->setAreaFill(area); }

PlotFontPtr QPCanvas::legendFont() const { return new QPFont(m_legendFont); }
    
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


bool QPCanvas::autoIncrementColors() const { return m_autoIncColors; }

void QPCanvas::setAutoIncrementColors(bool autoInc) {
    m_autoIncColors = autoInc; }

bool QPCanvas::exportToFile(const PlotExportFormat& format) {
    return exportCanvas(this, format); }

String QPCanvas::fileChooserDialog(const String& title,
        const String& directory) {
    QString filename = QFileDialog::getSaveFileName(this, title.c_str(),
            directory.c_str());
    return filename.toStdString();
}

PlotCoordinate QPCanvas::convertCoordinate(const PlotCoordinate& coord,
                                 PlotCoordinate::System newSystem) const {
    if(coord.system() == newSystem) return coord;
    
    if(coord.system() == PlotCoordinate::WORLD) {
        if(newSystem == PlotCoordinate::NORMALIZED_WORLD) {
            pair<double, double> range = axisRange(X_BOTTOM);
            double x = (coord.x() - range.first)/(range.second - range.first);            
            range = axisRange(Y_LEFT);
            double y = (coord.y() - range.first)/(range.second - range.first);
            
            return PlotCoordinate(x, y, newSystem);
            
        } else if(newSystem == PlotCoordinate::PIXEL) {
            QwtScaleMap map = m_canvas.canvasMap(QwtPlot::xBottom);
            double x = map.xTransform(coord.x());
            map = m_canvas.canvasMap(QwtPlot::yLeft);
            double y = map.xTransform(coord.y());
            
            return PlotCoordinate(x, y, newSystem);
        }
        
    } else if(coord.system() == PlotCoordinate::NORMALIZED_WORLD) {
        if(newSystem == PlotCoordinate::WORLD ||
           newSystem == PlotCoordinate::PIXEL) {
            pair<double, double> range = axisRange(X_BOTTOM);
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
        
    } else if(coord.system() == PlotCoordinate::PIXEL) {
        if(newSystem == PlotCoordinate::WORLD ||
           newSystem == PlotCoordinate::NORMALIZED_WORLD) {
            QwtScaleMap map = m_canvas.canvasMap(QwtPlot::xBottom);
            double x = map.invTransform(coord.x());
            map = m_canvas.canvasMap(QwtPlot::yLeft);
            double y = map.invTransform(coord.y());
            
            if(newSystem == PlotCoordinate::NORMALIZED_WORLD) {
                pair<double, double> r = axisRange(X_BOTTOM);
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
                                                PlotFontPtr font) const {
    vector<double> v(3, 0);

    QFontMetrics mets(QPFont(*font).asQFont());
    v[0] = mets.width(text.c_str());
    v[1] = mets.ascent();
    v[2] = mets.descent();
    
    return v;
}

PlotFactory* QPCanvas::implementationFactory() const { return new QPFactory();}


QPLayeredCanvas& QPCanvas::asQwtPlot() { return m_canvas; }
const QPLayeredCanvas& QPCanvas::asQwtPlot() const { return m_canvas; }

QwtPlotPicker& QPCanvas::getSelecter() { return m_picker; }

void QPCanvas::reinstallTrackerFilter() {
    m_canvas.canvas()->removeEventFilter(&m_mouseFilter);
    m_canvas.canvas()->installEventFilter(&m_mouseFilter);
}

QSize QPCanvas::sizeHint() const { return QSize(); }
QSize QPCanvas::minimumSizeHint() const { return QSize(); }


// Protected Methods //

void QPCanvas::setQPPlotter(QPPlotter* parent) {
    m_parent = parent;
    if(parent != NULL) {
        PlotLoggerPtr log = loggerForEvent(PlotLogger::OBJECTS_MAJOR);
        if(!log.null())
            for(unsigned int i = 0; i < m_queuedLogs.size(); i++)
                log->postMessage(m_queuedLogs[i]);
        m_queuedLogs.clear();
    }
}

PlotLoggerPtr QPCanvas::loggerForEvent(PlotLogger::Event event) const {
    if(event == PlotLogger::NO_EVENTS || m_parent == NULL)
        return PlotLoggerPtr();
    if(event & m_parent->logEventFlags()) return m_parent->logger();
    else                                  return PlotLoggerPtr();
}

void QPCanvas::logObject(const String& className, void* address, bool creation,
        const String& message) {
    if(m_parent != NULL)
        m_parent->logObject(className, address, creation, message);
    else
        m_queuedLogs.push_back(
                PlotLogObject(className, address, creation, message));
}

void QPCanvas::logMethod(const String& className, const String& methodName,
        bool entering, const String& message) {
    if(m_parent != NULL)
        m_parent->logMethod(className, methodName, entering, message);
}

QPAxesCache& QPCanvas::axesCache() { return m_stackCache; }
const QPAxesCache& QPCanvas::axesCache() const { return m_stackCache; }

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
    
    if(accept) event->accept();
    else       event->ignore();
}

void QPCanvas::mouseDoubleClickEvent(QMouseEvent* event) {
    m_ignoreNextRelease = true;
    m_timer.stop();
    
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
    
    if(key >= Qt::Key_F1 && key <= Qt::Key_F35) {
        modifiers.push_back(PlotKeyEvent::F);
        int i = key - Qt::Key_F1;
        c = '1' + i;
        accept = true;

    } else if((key >= Qt::Key_0 && key <= Qt::Key_9) ||
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
        
    } /* else if(text == "!" || text == "@" || text == "#" || text == "$" ||
              text == "%" || text == "^" || text == "&" || text == "*" ||
              text == "(" || text == ")") {
        if(mods.testFlag(Qt::ControlModifier))
            modifiers.push_back(PlotKeyEvent::CONTROL);
        if(mods.testFlag(Qt::AltModifier))
            modifiers.push_back(PlotKeyEvent::ALT);
        modifiers.push_back(PlotKeyEvent::SHIFT);
        if(text == "!")      c = '1';
        else if(text == "@") c = '2';
        else if(text == "#") c = '3';
        else if(text == "$") c = '4';
        else if(text == "%") c = '5';
        else if(text == "^") c = '6';
        else if(text == "&") c = '7';
        else if(text == "*") c = '8';
        else if(text == "(") c = '9';
        else if(text == ")") c = '0';
    } */
    
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
           
    if(!region.isValid()) {
        // It was really a click.
        PlotCoordinate wcoord(l, t, PlotCoordinate::WORLD);
        notifyClickHandlers(PlotMouseEvent::SINGLE, wcoord);

    } else {
        PlotRegion wreg(PlotCoordinate(l, t, PlotCoordinate::WORLD),
                        PlotCoordinate(r, b, PlotCoordinate::WORLD));
        notifySelectHandlers(wreg);
    }
}

void QPCanvas::timeout() {    
    // single click has occurred    
    m_timer.stop();
    
    PlotCoordinate pcoord = globalPosToPixelCoord(m_clickEvent);
    if(notifyClickHandlers(PlotMouseEvent::SINGLE, pcoord))
        m_clickEvent->accept();
}

}

#endif
