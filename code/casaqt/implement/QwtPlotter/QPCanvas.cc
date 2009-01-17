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

#include <casaqt/QwtPlotter/QPAnnotation.h>
#include <casaqt/QwtPlotter/QPFactory.h>
#include <casaqt/QwtPlotter/QPPlotter.qo.h>
#include <casaqt/QwtPlotter/QPRasterPlot.h>
#include <casaqt/QwtPlotter/QPShape.h>

#include <qwt_legend.h>
#include <qwt_scale_engine.h>
#include <qwt_scale_widget.h>
#include <qwt_text_label.h>

namespace casa {

// TODO QPCanvas not implemented/to be fixed:
// * weird Qt thing for legend where setting the border using a stylesheet
//   doesn't allow the background to be set using the palette
// * test exporting
// TODO General: check if trying to draw outside QRect is slower/faster than
//               adding checks before painting

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
        resetStacks();

// Static //

double QPCanvas::zOrder = 1;

const String QPCanvas::CLASS_NAME = "QPCanvas";

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
        m_cartAxes(4), m_axesRatioLocked(false), m_axesRatios(4, 1),
        m_autoIncColors(false), m_picker(m_canvas.canvas()),
        m_mouseFilter(m_canvas.canvas()), m_grid(), m_legendFontSet(false),
        m_inDraggingMode(false), m_ignoreNextRelease(false), m_timer(this),
        m_clickEvent(NULL) {
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    
    QGridLayout* gl = new QGridLayout(this);
    gl->addWidget(&m_canvas, 0, 0);
#if QT_VERSION >= 0x040300
    gl->setContentsMargins(0, 0, 0, 0);
#else
    gl->setMargin(0);
#endif
    
    QGridLayout* gl2 = new QGridLayout(&m_legendFrame);    
    int l, t, r, b;
    m_canvas.getContentsMargins(&l, &t, &r, &b);    
#if QT_VERSION >= 0x040300
    gl2->setContentsMargins(l, t, r, b);
#else
    int max = (l < t) ? l : t;
    if(r > max) max = r;
    if(b > max) max = b;
    gl2->setMargin(max);
#endif
    
    gl->addWidget(&m_legendFrame, 0, 0);
    m_legendFrame.hide();
    m_canvas.installLegendFilter(&m_legendFrame);
    m_legendFrame.setMouseTracking(true);
    
    for(unsigned int i = 0; i < m_cartAxes.size(); i++) m_cartAxes[i] = NULL;
    
    m_picker.setSelectionFlags(QwtPicker::RectSelection |
                               QwtPicker::DragSelection);
    m_picker.setRubberBand(QwtPicker::RectRubberBand);
    m_picker.setRubberBandPen(QPen(Qt::NoPen));
    m_picker.setTrackerMode(QwtPicker::AlwaysOff);
    
    m_mouseFilter.turnTracking(true);
    connect(&m_mouseFilter, SIGNAL(mouseMoveEvent(QMouseEvent*)),
            SLOT(trackerMouseEvent(QMouseEvent*)));
    
    m_grid.enableX(false);
    m_grid.enableXMin(false);
    m_grid.enableY(false);
    m_grid.enableYMin(false);
    m_grid.attach(&m_canvas);
    
    setLegendLine(QPFactory::defaultLegendLine());
    setLegendFill(QPFactory::defaultLegendAreaFill());
    
    m_canvas.canvas()->setCursor(Qt::ArrowCursor);
    
    m_canvas.enableAxis(QwtPlot::xBottom, false);
    m_canvas.enableAxis(QwtPlot::yLeft, false);
    m_canvas.setAutoReplot(true);
    
    connect(&m_picker, SIGNAL(selected(const QwtDoubleRect&)),
            this, SLOT(regionSelected(const QwtDoubleRect&)));
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
}

QPCanvas::~QPCanvas() {
    for(unsigned int i = 0; i < m_mouseTools.size(); i++)
        detachTool(m_mouseTools[i]);
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
    m_canvas.canvas()->setCursor(QPOptions::cursor(cursor)); }

void QPCanvas::refresh() {
    PRE_REPLOT
    m_canvas.replot();
    POST_REPLOT
    update();
}


bool QPCanvas::axisShown(PlotAxis axis) const {
    return m_canvas.axisEnabled(QPOptions::axis(axis)); }

void QPCanvas::showAxis(PlotAxis axis, bool show) {
    m_canvas.enableAxis(QPOptions::axis(axis), show); }

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
            break;
            
        case LOG10:
            m_canvas.setAxisScaleEngine(QPOptions::axis(axis),
                                        new QwtLog10ScaleEngine());
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

pair<double, double> QPCanvas::axisRange(PlotAxis axis) const {
    const QwtScaleDiv* div = m_canvas.axisScaleDiv(QPOptions::axis(axis));
    return pair<double, double>(div->lBound(), div->hBound());
}

void QPCanvas::setAxisRange(PlotAxis axis, double from, double to) {
    setAxesRanges(axis, from, to, axis, from, to);
}

void QPCanvas::setAxesRanges(PlotAxis xAxis, double xFrom, double xTo,
        PlotAxis yAxis, double yFrom, double yTo) {
    if(xTo == xFrom && yTo == yFrom) return;
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
                midPoint = (div->range() / 2) + div->lBound();
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
        
        PlotAxis a = X_BOTTOM;
        const QwtScaleDiv* div = m_canvas.axisScaleDiv(QPOptions::axis(a));
        m_canvas.setAxisScale(QPOptions::axis(a), div->lBound(), div->hBound(),
                              m_canvas.axisStepSize(QPOptions::axis(a)));
        
        a = X_TOP;
        div = m_canvas.axisScaleDiv(QPOptions::axis(a));
        m_canvas.setAxisScale(QPOptions::axis(a), div->lBound(), div->hBound(),
                              m_canvas.axisStepSize(QPOptions::axis(a)));
        
        a = Y_LEFT;
        div = m_canvas.axisScaleDiv(QPOptions::axis(a));
        m_canvas.setAxisScale(QPOptions::axis(a), div->lBound(), div->hBound(),
                              m_canvas.axisStepSize(QPOptions::axis(a)));
        
        a = Y_RIGHT;
        div = m_canvas.axisScaleDiv(QPOptions::axis(a));
        m_canvas.setAxisScale(QPOptions::axis(a), div->lBound(), div->hBound(),
                              m_canvas.axisStepSize(QPOptions::axis(a)));

        m_canvas.setAutoReplot(replot);
    }
}

void QPCanvas::rescaleAxes() {
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

bool QPCanvas::cartesianAxisShown(PlotAxis axis) const {
    return m_cartAxes[axisIndex(axis)] != NULL; }

void QPCanvas::showCartesianAxis(PlotAxis mirrorAxis, PlotAxis secondaryAxis,
                                 bool show, bool hideNormalAxis) {
    int i = axisIndex(mirrorAxis);
    QPCartesianAxis* a = m_cartAxes[i];
    if(a != NULL) {
        if(!show) {
            a->detach();
            delete a;
            m_cartAxes[i] = NULL;
        }
    } else {
        if(show) {
            a = new QPCartesianAxis(QPOptions::axis(mirrorAxis),
                                    QPOptions::axis(secondaryAxis));
            a->attach(&m_canvas);
            m_cartAxes[i] = a;
        }
    }
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


bool QPCanvas::autoIncrementColors() const { return m_autoIncColors; }

void QPCanvas::setAutoIncrementColors(bool autoInc) {
    m_autoIncColors = autoInc; }

bool QPCanvas::exportToFile(const PlotExportFormat& format) {
    if(format.location.empty()) return false;
    
    // Image
    if(format.type == PlotExportFormat::JPG ||
       format.type == PlotExportFormat::PNG) {
        
        QImage image = QPixmap::grabWidget(&m_canvas).toImage();
        
        // width and height, if applicable
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
        
        if(format.dpi > 0) {
            // convert dpi to dpm
            int dpm = QPOptions::round(format.dpi / 2.54 * 100);
            image.setDotsPerMeterX(dpm);
            image.setDotsPerMeterY(dpm);
        }
        
        int f = (format.resolution == PlotExportFormat::HIGH) ? 100 : -1;
        
        return !image.isNull() && image.save(format.location.c_str(),
                PlotExportFormat::exportFormat(format.type).c_str(), f);
        
    // PS/PDF
    } else if(format.type == PlotExportFormat::PS ||
              format.type == PlotExportFormat::PDF) {
        QPrinter::PrinterMode mode = QPrinter::ScreenResolution;
        if(format.resolution == PlotExportFormat::HIGH)
            mode = QPrinter::HighResolution;
        
        QPrinter printer(mode);
        if(format.type == PlotExportFormat::PDF)
            printer.setOutputFormat(QPrinter::PdfFormat);
        else printer.setOutputFormat(QPrinter::PostScriptFormat);
        
        if(m_canvas.width() >= m_canvas.height())
            printer.setOrientation(QPrinter::Landscape);
        else printer.setOrientation(QPrinter::Portrait);
        
        if(format.dpi > 0)
            printer.setResolution(format.dpi);
        
        printer.setColorMode(QPrinter::Color);
        printer.setOutputFileName(format.location.c_str());
        m_canvas.print(printer);
        return true;
    }
    
    return false;
}

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


void QPCanvas::holdDrawing() { m_canvas.setAutoReplot(false); }

void QPCanvas::releaseDrawing() {
    PRE_REPLOT
    m_canvas.replot();
    POST_REPLOT
    m_canvas.setAutoReplot(true);
}

bool QPCanvas::drawingIsHeld() const { return !m_canvas.autoReplot(); }

bool QPCanvas::plotItem(PlotItemPtr item, PlotCanvasLayer layer) {
    if(item.null() || !item->isValid()) return false;
    
    bool createdQPI;
    QPPlotItem* qitem = QPPlotItem::cloneItem(item, &createdQPI);
    if(qitem == NULL) return false;
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
    QwtText t = qitem->asQwtPlotItem().title();
    if(m_legendFontSet) {
        t.setFont(m_legendFont.asQFont());
        t.setColor(m_legendFont.asQColor());
        qitem->asQwtPlotItem().setTitle(t);
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
    
    qitem->asQwtPlotItem().setZ(zOrder++);
    qitem->attach(this, layer);
    
    if(layer == MAIN)
        m_plotItems.push_back(pair<PlotItemPtr, QPPlotItem*>(item, qitem));
    else
        m_layeredItems.push_back(pair<PlotItemPtr, QPPlotItem*>(item, qitem));
    
    if(replot) {
        PRE_REPLOT
        m_canvas.replot(layer == MAIN, layer != MAIN);
        POST_REPLOT
    }
    
    m_canvas.setAutoReplot(replot);
    return true;
}

vector<PlotItemPtr> QPCanvas::allPlotItems() const {
    vector<PlotItemPtr> v(m_plotItems.size());
    for(unsigned int i = 0; i < v.size(); i++) v[i] = m_plotItems[i].first;
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
    bool changedMain = false, changedLayer = false;
    bool replot = m_canvas.autoReplot();
    m_canvas.setAutoReplot(false);
    for(unsigned int i = 0; i < items.size(); i++) {
        if(items[i].null()) continue;
        for(unsigned int j = 0; j < m_plotItems.size(); j++) {
            if(items[i] == m_plotItems[j].first) {
                m_plotItems[j].second->detach();
                m_plotItems.erase(m_plotItems.begin() + j);
                changedMain = true;
                break;
            }
        }
        for(unsigned int j = 0; j < m_layeredItems.size(); j++) {
            if(items[i] == m_layeredItems[j].first) {
                m_layeredItems[j].second->detach();
                m_layeredItems.erase(m_layeredItems.begin() + j);
                changedLayer = true;
                break;
            }
        }
    }
    if((changedMain || changedLayer) && replot) {
        PRE_REPLOT
        m_canvas.replot(changedMain, changedLayer);
        POST_REPLOT
    }
    m_canvas.setAutoReplot(replot);
}

void QPCanvas::clearPlotItems() {
    bool replot = m_canvas.autoReplot();
    m_canvas.setAutoReplot(false);
    for(unsigned int i = 0; i < m_plotItems.size(); i++)
        m_plotItems[i].second->detach();
    for(unsigned int i = 0; i < m_layeredItems.size(); i++)
        m_layeredItems[i].second->detach();
    m_plotItems.clear();
    m_layeredItems.clear();
    
    if(replot) {
        PRE_REPLOT
        m_canvas.replot(true, true);
        POST_REPLOT
    }
    
    m_usedColors.resize(0);
    m_canvas.setAutoReplot(replot);
}

void QPCanvas::clearPlots() {
    PlotCanvas::clearPlots();
    m_usedColors.resize(0);
}

void QPCanvas::clearLayer(PlotCanvasLayer layer) {
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
    
    if(autoreplot) {
        PRE_REPLOT
        m_canvas.replot(layer == MAIN, layer != MAIN);
        POST_REPLOT
    }
    
    m_canvas.setAutoReplot(autoreplot);
    m_usedColors.resize(0);
}


bool QPCanvas::selectLineShown() const {
    return m_picker.rubberBandPen().style() != Qt::NoPen; }

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


bool QPCanvas::gridXMajorShown() const { return m_grid.xEnabled(); }
void QPCanvas::setGridXMajorShown(bool s) { m_grid.enableX(s); }
bool QPCanvas::gridXMinorShown() const { return m_grid.xMinEnabled(); }   
void QPCanvas::setGridXMinorShown(bool s) { m_grid.enableXMin(s); }
bool QPCanvas::gridYMajorShown() const { return m_grid.yEnabled(); }    
void QPCanvas::setGridYMajorShown(bool s) { m_grid.enableY(s); }
bool QPCanvas::gridYMinorShown() const { return m_grid.yMinEnabled(); }
void QPCanvas::setGridYMinorShown(bool s) { m_grid.enableYMin(s); }

PlotLinePtr QPCanvas::gridMajorLine() const {
    return new QPLine(m_grid.majPen()); }

void QPCanvas::setGridMajorLine(const PlotLine& line) {
    if(line != *gridMajorLine()) {
        const QPLine* l = dynamic_cast<const QPLine*>(&line);
        bool del = l == NULL;
        if(l == NULL) l = new QPLine(line);
        
        m_grid.enableX(l->style() != PlotLine::NOLINE);
        m_grid.enableY(l->style() != PlotLine::NOLINE);
        m_grid.setMajPen(l->asQPen());
        
        if(del) delete l;
    }
}

PlotLinePtr QPCanvas::gridMinorLine() const {
    return new QPLine(m_grid.minPen()); }

void QPCanvas::setGridMinorLine(const PlotLine& line) {
    if(line != *gridMinorLine()) {
        const QPLine* l = dynamic_cast<const QPLine*>(&line);
        bool del = l == NULL;
        if(l == NULL) l = new QPLine(line);
        
        m_grid.enableXMin(l->style() != PlotLine::NOLINE);
        m_grid.enableYMin(l->style() != PlotLine::NOLINE);
        m_grid.setMinPen(l->asQPen());
        
        if(del) delete l;
    }
}


bool QPCanvas::legendShown() const { return m_canvas.legend() != NULL; }

void QPCanvas::showLegend(bool on, LegendPosition pos) {
    QwtLegend* l = m_canvas.legend();
    if(l != NULL && m_legendFrame.isVisible()) {
        m_legendFrame.layout()->removeWidget(l);
        m_legendFrame.hide();
        delete l;
    }
    
    if(on) {        
        l = new QwtLegend();
        
        QwtPlot::LegendPosition p;
        switch(pos) {
        case EXT_TOP: p = QwtPlot::TopLegend; break;
        case EXT_RIGHT: p = QwtPlot::RightLegend; break;
        case EXT_LEFT: p = QwtPlot::LeftLegend; break;
        case EXT_BOTTOM: p = QwtPlot::BottomLegend; break;
        default: p = QwtPlot::ExternalLegend;
        }
        
        m_canvas.insertLegend(l, p);
        m_legendPosition = pos;
        
        if(m_legendLine.style() != PlotLine::NOLINE) {
            stringstream ss;
            ss << "QwtLegend { border: " << m_legendLine.width() << "px ";
            ss << QPOptions::cssLineStyle(m_legendLine.style()) << " ";
            ss << '#' << m_legendLine.color()->asHexadecimal() << "; }";
            l->setStyleSheet(ss.str().c_str());
        }
        
        QPalette pal = l->palette();
        pal.setBrush(QPalette::Window, m_legendFill.asQBrush());
        l->setPalette(pal);
        
        // for internal positioning        
        if(pos == INT_URIGHT || pos == INT_LRIGHT || pos == INT_ULEFT ||
           pos == INT_LLEFT) {
            // clear out old spacing
            QGridLayout* gl = dynamic_cast<QGridLayout*>(
                              m_legendFrame.layout());
            QLayoutItem* i;
            while((i = gl->takeAt(0)) != NULL) delete i;
            
            // adjust spacing to bypass axes
            // top
            int height = 10;
            if(m_canvas.axisWidget(QwtPlot::xTop)->isVisible())
                height += m_canvas.axisWidget(QwtPlot::xTop)->height();
            if(!m_canvas.title().isEmpty())
                height += m_canvas.titleLabel()->height();
            
            QSizePolicy::Policy p = QSizePolicy::Fixed;
            if(pos == INT_LRIGHT || pos == INT_LLEFT)
                p = QSizePolicy::MinimumExpanding;
            gl->addItem(new QSpacerItem(0, height, QSizePolicy::Minimum, p),
                        0, 0, 1, -1);
            
            // bottom
            height = 10;
            if(m_canvas.axisWidget(QwtPlot::xBottom)->isVisible())
                height += m_canvas.axisWidget(QwtPlot::xBottom)->height();
            p = QSizePolicy::Fixed;
            if(pos == INT_URIGHT || pos == INT_ULEFT)
                p = QSizePolicy::MinimumExpanding;            
            gl->addItem(new QSpacerItem(0, height, QSizePolicy::Minimum, p),
                        2, 0, 1, -1);
            
            // left
            int width = 10;
            if(m_canvas.axisWidget(QwtPlot::yLeft)->isVisible())
                width += m_canvas.axisWidget(QwtPlot::yLeft)->width();
            p = QSizePolicy::Fixed;
            if(pos == INT_URIGHT || pos == INT_LRIGHT)
                p = QSizePolicy::MinimumExpanding;            
            gl->addItem(new QSpacerItem(width, 0, p, QSizePolicy::Minimum),
                        1, 0);
            
            // right
            width = 10;
            if(m_canvas.axisWidget(QwtPlot::yRight)->isVisible())
                width += m_canvas.axisWidget(QwtPlot::yRight)->width();
            p = QSizePolicy::Fixed;
            if(pos == INT_ULEFT || pos == INT_LLEFT)
                p = QSizePolicy::MinimumExpanding;            
            gl->addItem(new QSpacerItem(width, 0, p, QSizePolicy::Minimum),
                        1, 2);
            
            ((QGridLayout*)m_legendFrame.layout())->addWidget(l, 1, 1);
            m_legendFrame.show();
        }
    } else {
        m_canvas.insertLegend(NULL);
    }
}

PlotCanvas::LegendPosition QPCanvas::legendPosition() const {
    return m_legendPosition; }

void QPCanvas::setLegendPosition(LegendPosition pos) {   
    showLegend(legendShown(), pos); }

PlotLinePtr QPCanvas::legendLine() const { return new QPLine(m_legendLine); }

void QPCanvas::setLegendLine(const PlotLine& line) {
    if(line != m_legendLine) {
        m_legendLine = QPLine(line);
        
        if(m_canvas.legend() != NULL) {        
            if(m_legendLine.style() != PlotLine::NOLINE) {
                stringstream ss;
                ss << "QwtLegend { border: " << m_legendLine.width() << "px ";
                ss << QPOptions::cssLineStyle(m_legendLine.style()) << " ";
                ss << '#' << m_legendLine.color()->asHexadecimal() << "; }";
                m_canvas.legend()->setStyleSheet(ss.str().c_str());
            } else {
                m_canvas.legend()->setStyleSheet("");
            }
        }
    }
}

PlotAreaFillPtr QPCanvas::legendFill() const {
    return new QPAreaFill(m_legendFill); }

void QPCanvas::setLegendFill(const PlotAreaFill& area) {
    if(area != m_legendFill) {
        m_legendFill = QPAreaFill(area);
        
        if(m_canvas.legend() != NULL) {
            QPalette p = m_canvas.legend()->palette();
            p.setBrush(QPalette::Window, m_legendFill.asQBrush());
            m_canvas.legend()->setPalette(p);
        }
    }
}

PlotFontPtr QPCanvas::legendFont() const { return new QPFont(m_legendFont); }
    
void QPCanvas::setLegendFont(const PlotFont& font) {
    if(font != m_legendFont) {
        m_legendFont = font;
        m_legendFontSet = true;
        if(m_canvas.legend() != NULL) {
            for(unsigned int i = 0; i < m_plotItems.size(); i++) {
                QwtText t = m_plotItems[i].second->asQwtPlotItem().title();
                t.setFont(m_legendFont.asQFont());
                t.setColor(m_legendFont.asQColor());
                m_plotItems[i].second->asQwtPlotItem().setTitle(t);
            }
        }
    }
}


void QPCanvas::registerMouseTool(PlotMouseToolPtr tool, bool activate,
        bool blocking) {
    if(tool.null()) return;
    for(unsigned int i = 0; i < m_mouseTools.size(); i++)
        if(m_mouseTools[i] == tool) return;
    attachTool(tool);
    m_mouseTools.push_back(tool);
    tool->setActive(activate);
    tool->setBlocking(blocking);
}

vector<PlotMouseToolPtr> QPCanvas::allMouseTools() const{ return m_mouseTools;}

vector<PlotMouseToolPtr> QPCanvas::activeMouseTools() const {
    vector<PlotMouseToolPtr> v;
    for(unsigned int i = 0; i < m_mouseTools.size(); i++)
        if(m_mouseTools[i]->isActive()) v.push_back(m_mouseTools[i]);
    return v;
}

void QPCanvas::unregisterMouseTool(PlotMouseToolPtr tool) {
    if(tool.null()) return;
    for(unsigned int i = 0; i < m_mouseTools.size(); i++) {
        if(m_mouseTools[i] == tool) {
            m_mouseTools.erase(m_mouseTools.begin() + i);
            detachTool(tool);
            return;
        }
    }
}

PlotStandardMouseToolGroupPtr QPCanvas::standardMouseTools() {
    if(m_standardTools.null()) {
        m_standardTools = QPFactory().standardMouseTools();
        registerMouseTool(m_standardTools, false, true);
    }
    return m_standardTools;
}


// Macro for the handlers because they got very repetitive.
#define QPC_HANDLER1(TYPE,MEMBER)                                             \
void QPCanvas::register##TYPE##Handler(Plot##TYPE##EventHandlerPtr handler,   \
        PlotCoordinate::System system) {                                      \
    if(handler.null()) return;                                                \
    for(unsigned int i = 0; i < MEMBER .size(); i++)                          \
        if( MEMBER [i].first == handler) return;                              \
    MEMBER .push_back(pair<Plot##TYPE##EventHandlerPtr,                       \
            PlotCoordinate::System>(handler, system));                        \
}                                                                             \
                                                                              \
vector<Plot##TYPE##EventHandlerPtr> QPCanvas::all##TYPE##Handlers() const {   \
    vector<Plot##TYPE##EventHandlerPtr> v( MEMBER .size());                   \
    for(unsigned int i = 0; i < v.size(); i++) v[i] = MEMBER [i].first;       \
    return v;                                                                 \
}                                                                             \
                                                                              \
void QPCanvas::unregister##TYPE##Handler(Plot##TYPE##EventHandlerPtr handler){\
    for(unsigned int i = 0; i < MEMBER .size(); i++) {                        \
        if( MEMBER [i].first == handler) {                                    \
            MEMBER .erase( MEMBER .begin() + i);                              \
            break;                                                            \
        }                                                                     \
    }                                                                         \
}

// Second macro which doesn't have the PlotCoordiate::System stuff.
#define QPC_HANDLER2(TYPE,MEMBER)                                             \
void QPCanvas::register##TYPE##Handler(Plot##TYPE##EventHandlerPtr handler) { \
    if(handler.null()) return;                                                \
    for(unsigned int i = 0; i < MEMBER .size(); i++)                          \
        if( MEMBER [i] == handler) return;                                    \
    MEMBER .push_back(handler);                                               \
}                                                                             \
                                                                              \
vector<Plot##TYPE##EventHandlerPtr> QPCanvas::all##TYPE##Handlers() const {   \
    return MEMBER; }                                                          \
                                                                              \
void QPCanvas::unregister##TYPE##Handler(Plot##TYPE##EventHandlerPtr handler){\
    for(unsigned int i = 0; i < MEMBER .size(); i++) {                        \
        if( MEMBER [i] == handler) {                                          \
            MEMBER .erase( MEMBER .begin() + i);                              \
            break;                                                            \
        }                                                                     \
    }                                                                         \
}

QPC_HANDLER1(Select, m_selectHandlers)
QPC_HANDLER1(Click, m_clickHandlers)
QPC_HANDLER1(MousePress, m_pressHandlers)
QPC_HANDLER1(MouseRelease, m_releaseHandlers)
QPC_HANDLER1(MouseDrag, m_dragHandlers)
QPC_HANDLER1(MouseMove, m_moveHandlers)
QPC_HANDLER1(Wheel, m_wheelHandlers)
QPC_HANDLER2(Key, m_keyHandlers)
QPC_HANDLER2(Resize, m_resizeHandlers)


QPLayeredCanvas& QPCanvas::asQwtPlot() { return m_canvas; }

QwtPlotPicker& QPCanvas::getSelecter() { return m_picker; }

void QPCanvas::reinstallTrackerFilter() {
    m_canvas.canvas()->removeEventFilter(&m_mouseFilter);
    m_canvas.canvas()->installEventFilter(&m_mouseFilter);
}


// Protected Methods //

void QPCanvas::setQPPlotter(QPPlotter* parent) { m_parent = parent; }

PlotLoggerPtr QPCanvas::loggerForMeasurement(PlotLogger::MeasurementEvent e) {
    if(e == PlotLogger::NOMEASUREMENTS || m_parent == NULL)
        return PlotLoggerPtr();
    if(e & m_parent->logMeasurementEvents()) return m_parent->logger();
    else                                     return PlotLoggerPtr();
}

void QPCanvas::mousePressEvent(QMouseEvent* event) {
    vector<PlotMouseToolPtr> active = activeMouseTools();
    if(active.size() == 0 && m_pressHandlers.size() == 0) {
        event->ignore();
        return;
    }
    PlotMouseEvent::Button t = PlotMouseEvent::SINGLE;
    if(event->button() == Qt::RightButton) t = PlotMouseEvent::CONTEXT;
    else if(event->button() == Qt::MidButton) t = PlotMouseEvent::MIDDLE;
    
    double x = event->globalX(), y = event->globalY();
    QRect rect = m_canvas.canvas()->contentsRect();
    QPoint p(rect.left(), rect.top());
    p = m_canvas.canvas()->mapToGlobal(p);
    
    PlotCoordinate pcoord(x - p.x(), y - p.y(), PlotCoordinate::PIXEL);
    PlotCoordinate wcoord = convertCoordinate(pcoord, PlotCoordinate::WORLD);
    PlotCoordinate ncoord = convertCoordinate(wcoord,
                                            PlotCoordinate::NORMALIZED_WORLD);
    PlotMousePressEvent pe(this, t, pcoord);
    PlotMousePressEvent we(this, t, wcoord);
    PlotMousePressEvent ne(this, t, ncoord);
    
    for(unsigned int i = 0; i < active.size(); i++) {
        switch(active[i]->getCoordinateSystem()) {
        case PlotCoordinate::WORLD: active[i]->handleMousePress(we); break;
        case PlotCoordinate::PIXEL: active[i]->handleMousePress(pe); break;
        case PlotCoordinate::NORMALIZED_WORLD:
            active[i]->handleMousePress(ne); break;
            
        default: continue;
        }
        if(active[i]->isBlocking() && active[i]->lastEventWasHandled()) {
            event->accept();
            return;
        }
    }
    
    for(unsigned int i = 0; i < m_pressHandlers.size(); i++) {
        if(m_pressHandlers[i].second == PlotCoordinate::PIXEL)
            m_pressHandlers[i].first->handleMousePress(pe);
        else if(m_pressHandlers[i].second == PlotCoordinate::WORLD)
            m_pressHandlers[i].first->handleMousePress(we);
        else if(m_pressHandlers[i].second == PlotCoordinate::NORMALIZED_WORLD)
            m_pressHandlers[i].first->handleMousePress(ne);
    }
}

void QPCanvas::mouseReleaseEvent(QMouseEvent* event) {
    vector<PlotMouseToolPtr> active = activeMouseTools();
    if(active.size() == 0 && m_clickHandlers.size() == 0 &&
       m_releaseHandlers.size() == 0) {
        event->ignore();
        return;
    }
    
    PlotMouseEvent::Button t = PlotMouseEvent::SINGLE;
    if(event->button() == Qt::RightButton) t = PlotMouseEvent::CONTEXT;
    else if(event->button() == Qt::MidButton) t = PlotMouseEvent::MIDDLE;
    
    double x = event->globalX(), y = event->globalY();
    QRect rect = m_canvas.canvas()->contentsRect();
    QPoint p(rect.left(), rect.top());
    p = m_canvas.canvas()->mapToGlobal(p);
    
    PlotCoordinate pcoord(x - p.x(), y - p.y(), PlotCoordinate::PIXEL);
    PlotCoordinate wcoord = convertCoordinate(pcoord, PlotCoordinate::WORLD);
    PlotCoordinate ncoord = convertCoordinate(wcoord,
                                           PlotCoordinate::NORMALIZED_WORLD);
    PlotMouseReleaseEvent pe(this, t, pcoord);
    PlotMouseReleaseEvent we(this, t, wcoord);
    PlotMouseReleaseEvent ne(this, t, ncoord);
    
    bool blocked = false;
    for(unsigned int i = 0; !blocked && i < active.size(); i++) {
        switch(active[i]->getCoordinateSystem()) {
        case PlotCoordinate::WORLD: active[i]->handleMouseRelease(we); break;
        case PlotCoordinate::PIXEL: active[i]->handleMouseRelease(pe); break;
        case PlotCoordinate::NORMALIZED_WORLD:
            active[i]->handleMouseRelease(ne); break;
            
        default: continue;
        }
        if(active[i]->isBlocking() && active[i]->lastEventWasHandled())
            blocked = true;
    }
    
    for(unsigned int i = 0; !blocked && i < m_releaseHandlers.size(); i++) {
        if(m_releaseHandlers[i].second == PlotCoordinate::PIXEL)
            m_releaseHandlers[i].first->handleMouseRelease(pe);
        else if(m_releaseHandlers[i].second == PlotCoordinate::WORLD)
            m_releaseHandlers[i].first->handleMouseRelease(we);
        else if(m_releaseHandlers[i].second ==
            PlotCoordinate::NORMALIZED_WORLD)
            m_releaseHandlers[i].first->handleMouseRelease(ne);
    }
    
    if(event->button() == Qt::LeftButton) {
        if(m_releaseHandlers.size() > 0) event->accept();
        
        if(!m_ignoreNextRelease) {
            m_timer.start(QApplication::doubleClickInterval());
            m_clickEvent = event;
        }
        m_ignoreNextRelease = false;
        
    } else if(event->button() == Qt::RightButton ||
              event->button() == Qt::MidButton) {
        // Also send a click event
        PlotMouseEvent::Button t= event->button() == Qt::RightButton ?
                                PlotMouseEvent::CONTEXT:PlotMouseEvent::MIDDLE;
        PlotClickEvent pe2(this, t, pcoord);
        PlotClickEvent we2(this, t, wcoord);
        PlotClickEvent ne2(this, t, ncoord);
        
        blocked = false;
        for(unsigned int i = 0; !blocked && i < active.size(); i++) {
            switch(active[i]->getCoordinateSystem()) {
            case PlotCoordinate::WORLD:
                active[i]->handleClick(we2); break;
            case PlotCoordinate::PIXEL:
                active[i]->handleClick(pe2); break;
            case PlotCoordinate::NORMALIZED_WORLD:
                active[i]->handleClick(ne2); break;
                
            default: continue;
            }
            if(active[i]->isBlocking() && active[i]->lastEventWasHandled())
                blocked = true;
        }
        
        for(unsigned int i = 0; !blocked && i < m_clickHandlers.size(); i++) {
            if(m_clickHandlers[i].second == PlotCoordinate::PIXEL)
                m_clickHandlers[i].first->handleClick(pe2);
            else if(m_clickHandlers[i].second == PlotCoordinate::WORLD)
                m_clickHandlers[i].first->handleClick(we2);
            else if(m_clickHandlers[i].second ==
                PlotCoordinate::NORMALIZED_WORLD)
                m_clickHandlers[i].first->handleClick(ne2);
        }
    }
    event->accept();
}

void QPCanvas::mouseDoubleClickEvent(QMouseEvent* event) {
    m_ignoreNextRelease = true;
    m_timer.stop();
    
    vector<PlotMouseToolPtr> active = activeMouseTools();
    if(active.size() == 0 && m_clickHandlers.size() == 0 &&
       m_pressHandlers.size() == 0) {
        event->ignore();
        return;
    }
    
    // event->x() and event->y() are relative to the whole QwtPlot, not just
    // the canvas, so adjust accordingly
    double x = event->globalX(), y = event->globalY();
    QRect rect = m_canvas.canvas()->contentsRect();
    QPoint p(rect.left(), rect.top());
    p = m_canvas.canvas()->mapToGlobal(p);
    
    PlotCoordinate pcoord(x - p.x(), y - p.y(), PlotCoordinate::PIXEL);
    PlotCoordinate wcoord = convertCoordinate(pcoord, PlotCoordinate::WORLD);
    PlotCoordinate ncoord = convertCoordinate(wcoord,
            PlotCoordinate::NORMALIZED_WORLD);
    
    // also a press event
    PlotMousePressEvent pe(this, PlotMouseEvent::DOUBLE, pcoord);
    PlotMousePressEvent we(this, PlotMouseEvent::DOUBLE, wcoord);
    PlotMousePressEvent ne(this, PlotMouseEvent::DOUBLE, ncoord);
    
    bool blocked = false;
    for(unsigned int i = 0; !blocked && i < active.size(); i++) {
        switch(active[i]->getCoordinateSystem()) {
        case PlotCoordinate::WORLD:
            active[i]->handleMousePress(we); break;
        case PlotCoordinate::PIXEL:
            active[i]->handleMousePress(pe); break;
        case PlotCoordinate::NORMALIZED_WORLD:
            active[i]->handleMousePress(ne); break;
            
        default: continue;
        }
        if(active[i]->isBlocking() && active[i]->lastEventWasHandled())
            blocked = true;
    }

    for(unsigned int i = 0; !blocked && i < m_pressHandlers.size(); i++) {
        if(m_pressHandlers[i].second == PlotCoordinate::PIXEL)
            m_pressHandlers[i].first->handleMousePress(pe);
        else if(m_pressHandlers[i].second == PlotCoordinate::WORLD)
            m_pressHandlers[i].first->handleMousePress(we);
        else if(m_pressHandlers[i].second == PlotCoordinate::NORMALIZED_WORLD)
            m_pressHandlers[i].first->handleMousePress(ne);
    }
    
    PlotClickEvent pe2(this, PlotMouseEvent::DOUBLE, pcoord);
    PlotClickEvent we2(this, PlotMouseEvent::DOUBLE, wcoord);
    PlotClickEvent ne2(this, PlotMouseEvent::DOUBLE, ncoord);
    
    blocked = false;
    for(unsigned int i = 0; !blocked && i < active.size(); i++) {
        switch(active[i]->getCoordinateSystem()) {
        case PlotCoordinate::WORLD:
            active[i]->handleClick(we2); break;
        case PlotCoordinate::PIXEL:
            active[i]->handleClick(pe2); break;
        case PlotCoordinate::NORMALIZED_WORLD:
            active[i]->handleClick(ne2); break;
            
        default: continue;
        }
        if(active[i]->isBlocking() && active[i]->lastEventWasHandled())
            blocked = true;
    }
    
    for(unsigned int i = 0; !blocked && i < m_clickHandlers.size(); i++) {
        if(m_clickHandlers[i].second == PlotCoordinate::PIXEL)
            m_clickHandlers[i].first->handleClick(pe2);
        else if(m_clickHandlers[i].second == PlotCoordinate::WORLD)
            m_clickHandlers[i].first->handleClick(we2);
        else if(m_clickHandlers[i].second == PlotCoordinate::NORMALIZED_WORLD)
            m_clickHandlers[i].first->handleClick(ne2);
    }
    
    event->accept();
}

void QPCanvas::trackerMouseEvent(QMouseEvent* event) {
    vector<PlotMouseToolPtr> active = activeMouseTools();
    if(active.size() == 0 && m_dragHandlers.size() == 0 &&
       m_moveHandlers.size() == 0) {
        event->ignore();
        return;
    }
    
    double x = event->globalX(), y = event->globalY();
    QRect rect = m_canvas.canvas()->contentsRect();
    QPoint p(rect.left(), rect.top());
    p = m_canvas.canvas()->mapToGlobal(p);
    
    PlotCoordinate pcoord(x - p.x(), y - p.y(), PlotCoordinate::PIXEL);
    PlotCoordinate wcoord = convertCoordinate(pcoord, PlotCoordinate::WORLD);
    PlotCoordinate ncoord = convertCoordinate(wcoord,
            PlotCoordinate::NORMALIZED_WORLD);
    
    PlotMouseEvent::Button t = PlotMouseEvent::SINGLE;
    if(event->button() == Qt::RightButton) t = PlotMouseEvent::CONTEXT;
    else if(event->button() == Qt::MidButton) t = PlotMouseEvent::MIDDLE;
    
    if(active.size() > 0 || m_moveHandlers.size() > 0) {
        PlotMouseMoveEvent pe(this, t, pcoord);
        PlotMouseMoveEvent we(this, t, wcoord);
        PlotMouseMoveEvent ne(this, t, ncoord);
        
        bool blocked = false;
        for(unsigned int i = 0; !blocked && i < active.size(); i++) {
            switch(active[i]->getCoordinateSystem()) {
            case PlotCoordinate::WORLD:
                active[i]->handleMouseMove(we); break;
            case PlotCoordinate::PIXEL:
                active[i]->handleMouseMove(pe); break;
            case PlotCoordinate::NORMALIZED_WORLD:
                active[i]->handleMouseMove(ne); break;
                
            default: continue;
            }
            if(active[i]->isBlocking() && active[i]->lastEventWasHandled())
                blocked = true;
        }
        
        for(unsigned int i = 0; !blocked && i < m_moveHandlers.size(); i++) {
            if(m_moveHandlers[i].second == PlotCoordinate::PIXEL)
                m_moveHandlers[i].first->handleMouseMove(pe);
            else if(m_moveHandlers[i].second == PlotCoordinate::WORLD)
                m_moveHandlers[i].first->handleMouseMove(we);
            else if(m_moveHandlers[i].second==PlotCoordinate::NORMALIZED_WORLD)
                m_moveHandlers[i].first->handleMouseMove(ne);
        }
    }
    
    if(m_inDraggingMode && (active.size() > 0 || m_dragHandlers.size() > 0)) {    
        PlotMouseDragEvent pe2(this, t, pcoord);
        PlotMouseDragEvent we2(this, t, wcoord);
        PlotMouseDragEvent ne2(this, t, ncoord);
        
        bool blocked = false;
        for(unsigned int i = 0; !blocked && i < active.size(); i++) {
            active[i]->handleMouseDrag(we2);
            if(active[i]->isBlocking() && active[i]->lastEventWasHandled())
                blocked = true;
        }
        
        for(unsigned int i = 0; !blocked && i < m_dragHandlers.size(); i++) {
            if(m_dragHandlers[i].second == PlotCoordinate::PIXEL)
                m_dragHandlers[i].first->handleMouseDrag(pe2);
            else if(m_dragHandlers[i].second == PlotCoordinate::WORLD)
                m_dragHandlers[i].first->handleMouseDrag(we2);
            else if(m_dragHandlers[i].second==PlotCoordinate::NORMALIZED_WORLD)
                m_dragHandlers[i].first->handleMouseDrag(ne2);
        }
    }
    
    event->accept();
}

void QPCanvas::keyReleaseEvent(QKeyEvent* event) {
    if(m_keyHandlers.size() == 0) {
        event->ignore();
        return;
    }
    
    int key = event->key();
    QString text = event->text();
    Qt::KeyboardModifiers mods = QApplication::keyboardModifiers();
    
    PlotKeyEvent* e = NULL;
    vector<PlotKeyEvent::Modifier> modifiers;
    
    if(key >= Qt::Key_F1 && key <= Qt::Key_F35) {
        modifiers.push_back(PlotKeyEvent::F);
        int i = key - Qt::Key_F1;
        char c = '1' + i;
        
        e = new PlotKeyEvent(this, c, modifiers);

    } else if((key >= Qt::Key_0 && key <= Qt::Key_9) ||
              (key >= Qt::Key_A && key <= Qt::Key_Z)) {        
        if(mods.testFlag(Qt::ControlModifier))
            modifiers.push_back(PlotKeyEvent::CONTROL);
        if(mods.testFlag(Qt::AltModifier))
            modifiers.push_back(PlotKeyEvent::ALT);
        if(mods.testFlag(Qt::ShiftModifier))
            modifiers.push_back(PlotKeyEvent::SHIFT);
        
        int i;
        char c;
        
        if(key >= Qt::Key_0 && key <= Qt::Key_9) {
            i = key - Qt::Key_0;
            c = '0' + i;
        } else {
            i = key - Qt::Key_A;
            c = 'a' + i;
        }
        
        e = new PlotKeyEvent(this, c, modifiers);
        
    } else if(text == "!") {
        if(mods.testFlag(Qt::ControlModifier))
            modifiers.push_back(PlotKeyEvent::CONTROL);
        if(mods.testFlag(Qt::AltModifier))
            modifiers.push_back(PlotKeyEvent::ALT);
        modifiers.push_back(PlotKeyEvent::SHIFT);
        e = new PlotKeyEvent(this, '1', modifiers);
    } else if(text == "@") {
        if(mods.testFlag(Qt::ControlModifier))
            modifiers.push_back(PlotKeyEvent::CONTROL);
        if(mods.testFlag(Qt::AltModifier))
            modifiers.push_back(PlotKeyEvent::ALT);
        modifiers.push_back(PlotKeyEvent::SHIFT);
        e = new PlotKeyEvent(this, '2', modifiers);
    } else if(text == "#") {
        if(mods.testFlag(Qt::ControlModifier))
            modifiers.push_back(PlotKeyEvent::CONTROL);
        if(mods.testFlag(Qt::AltModifier))
            modifiers.push_back(PlotKeyEvent::ALT);
        modifiers.push_back(PlotKeyEvent::SHIFT);
        e = new PlotKeyEvent(this, '3', modifiers);
    } else if(text == "$") {
        if(mods.testFlag(Qt::ControlModifier))
            modifiers.push_back(PlotKeyEvent::CONTROL);
        if(mods.testFlag(Qt::AltModifier))
            modifiers.push_back(PlotKeyEvent::ALT);
        modifiers.push_back(PlotKeyEvent::SHIFT);
        e = new PlotKeyEvent(this, '4', modifiers);
    } else if(text == "%") {
        if(mods.testFlag(Qt::ControlModifier))
            modifiers.push_back(PlotKeyEvent::CONTROL);
        if(mods.testFlag(Qt::AltModifier))
            modifiers.push_back(PlotKeyEvent::ALT);
        modifiers.push_back(PlotKeyEvent::SHIFT);
        e = new PlotKeyEvent(this, '5', modifiers);
    } else if(text == "^") {
        if(mods.testFlag(Qt::ControlModifier))
            modifiers.push_back(PlotKeyEvent::CONTROL);
        if(mods.testFlag(Qt::AltModifier))
            modifiers.push_back(PlotKeyEvent::ALT);
        modifiers.push_back(PlotKeyEvent::SHIFT);
        e = new PlotKeyEvent(this, '6', modifiers);
    } else if(text == "&") {
        if(mods.testFlag(Qt::ControlModifier))
            modifiers.push_back(PlotKeyEvent::CONTROL);
        if(mods.testFlag(Qt::AltModifier))
            modifiers.push_back(PlotKeyEvent::ALT);
        modifiers.push_back(PlotKeyEvent::SHIFT);
        e = new PlotKeyEvent(this, '7', modifiers);
    } else if(text == "*") {
        if(mods.testFlag(Qt::ControlModifier))
            modifiers.push_back(PlotKeyEvent::CONTROL);
        if(mods.testFlag(Qt::AltModifier))
            modifiers.push_back(PlotKeyEvent::ALT);
        modifiers.push_back(PlotKeyEvent::SHIFT);
        e = new PlotKeyEvent(this, '8', modifiers);
    } else if(text == "(") {
        if(mods.testFlag(Qt::ControlModifier))
            modifiers.push_back(PlotKeyEvent::CONTROL);
        if(mods.testFlag(Qt::AltModifier))
            modifiers.push_back(PlotKeyEvent::ALT);
        modifiers.push_back(PlotKeyEvent::SHIFT);
        e = new PlotKeyEvent(this, '9', modifiers);
    } else if(text == ")") {
        if(mods.testFlag(Qt::ControlModifier))
            modifiers.push_back(PlotKeyEvent::CONTROL);
        if(mods.testFlag(Qt::AltModifier))
            modifiers.push_back(PlotKeyEvent::ALT);
        modifiers.push_back(PlotKeyEvent::SHIFT);
        e = new PlotKeyEvent(this, '0', modifiers);
    }
    
    if(e != NULL) {
        for(unsigned int i = 0; i < m_keyHandlers.size(); i++)
            m_keyHandlers[i]->handleKey(*e);
        delete e;
        event->accept();
    } else event->ignore();
}

void QPCanvas::wheelEvent(QWheelEvent* event) {
    vector<PlotMouseToolPtr> active = activeMouseTools();
    if(active.size() > 0 || m_wheelHandlers.size() > 0) {
        // event->x() and event->y() are relative to the whole QwtPlot, not
        // just the canvas, so adjust accordingly
        double x = event->globalX(), y = event->globalY();
        QRect rect = m_canvas.canvas()->contentsRect();
        QPoint p(rect.left(), rect.top());
        p = m_canvas.canvas()->mapToGlobal(p);
        int delta = (event->delta() / 8) / 15;
        
        PlotCoordinate pcoord(x - p.x(), y - p.y(), PlotCoordinate::PIXEL);
        PlotCoordinate wcoord = convertCoordinate(pcoord,
                PlotCoordinate::WORLD);
        PlotCoordinate ncoord = convertCoordinate(wcoord,
                PlotCoordinate::NORMALIZED_WORLD);        
        PlotWheelEvent pe(this, delta, pcoord);
        PlotWheelEvent we(this, delta, wcoord);
        PlotWheelEvent ne(this, delta, ncoord);
        
        for(unsigned int i = 0; i < active.size(); i++) {
            active[i]->handleWheel(we);
            if(active[i]->isBlocking() && active[i]->lastEventWasHandled()) {
                event->accept();
                return;
            }
        }
        
        for(unsigned int i = 0; i < m_wheelHandlers.size(); i++) {
            if(m_wheelHandlers[i].second == PlotCoordinate::PIXEL)
                m_wheelHandlers[i].first->handleWheel(pe);
            else if(m_wheelHandlers[i].second == PlotCoordinate::WORLD)
                m_wheelHandlers[i].first->handleWheel(we);
            else if(m_wheelHandlers[i].second ==
                PlotCoordinate::NORMALIZED_WORLD)
                m_wheelHandlers[i].first->handleWheel(ne);
        }
        event->accept();
    } else event->ignore();
}

void QPCanvas::resizeEvent(QResizeEvent* event) {
    if(m_resizeHandlers.size() > 0) {
        QSize o = event->oldSize(), n = event->size();
        PlotResizeEvent e(this, o.width(), o.height(), n.width(), n.height());
        for(unsigned int i = 0; i < m_resizeHandlers.size(); i++)
            m_resizeHandlers[i]->handleResize(e);
        event->accept();
    } else event->ignore();
}


// Private Methods //

void QPCanvas::resetStacks() {
    for(unsigned int i = 0; i < m_mouseTools.size(); i++)
        m_mouseTools[i]->reset();
}


// Private Slots //

void QPCanvas::regionSelected(const QwtDoubleRect& region) {
    // wtf?
    double l = (region.left() < region.right())? region.left(): region.right(),
           r = (region.right() > region.left())? region.right(): region.left(),
           t = (region.top() > region.bottom())? region.top(): region.bottom(),
           b = (region.bottom() < region.top())? region.bottom(): region.top();
           
    vector<PlotMouseToolPtr> active = activeMouseTools();
    if(!region.isValid()) {
        // it was really a click
        if(m_clickHandlers.size() == 0 && active.size() == 0) return;
        
        PlotCoordinate wc(l, t);
        PlotCoordinate nc = convertCoordinate(wc,
                PlotCoordinate::NORMALIZED_WORLD);
        PlotCoordinate pc = convertCoordinate(wc, PlotCoordinate::PIXEL);        
        PlotClickEvent pe(this, PlotMouseEvent::SINGLE, pc);
        PlotClickEvent we(this, PlotMouseEvent::SINGLE, wc);
        PlotClickEvent ne(this, PlotMouseEvent::SINGLE, nc);
        
        for(unsigned int i = 0; i < active.size(); i++) {
            switch(active[i]->getCoordinateSystem()) {
            case PlotCoordinate::WORLD: active[i]->handleClick(we); break;
            case PlotCoordinate::PIXEL: active[i]->handleClick(pe); break;
            case PlotCoordinate::NORMALIZED_WORLD:
                active[i]->handleClick(ne); break;
            default: continue;
            }
            if(active[i]->isBlocking() && active[i]->lastEventWasHandled())
                return;
        }
        
        for(unsigned int i = 0; i < m_clickHandlers.size(); i++) {
            if(m_clickHandlers[i].second == PlotCoordinate::PIXEL)
                m_clickHandlers[i].first->handleClick(pe);
            else if(m_clickHandlers[i].second == PlotCoordinate::WORLD)
                m_clickHandlers[i].first->handleClick(we);
            else if(m_clickHandlers[i].second ==
                PlotCoordinate::NORMALIZED_WORLD)
                m_clickHandlers[i].first->handleClick(ne);
        }
    } else {
        if(m_selectHandlers.size() == 0 && active.size() == 0) return;
        
        PlotRegion wreg(PlotCoordinate(l, t), PlotCoordinate(r, b));
        PlotRegion nreg(convertCoordinate(wreg.upperLeft(),
                PlotCoordinate::NORMALIZED_WORLD), convertCoordinate(
                        wreg.lowerRight(), PlotCoordinate::NORMALIZED_WORLD));
        PlotRegion preg(convertCoordinate(wreg.upperLeft(),
                PlotCoordinate::PIXEL), convertCoordinate(wreg.lowerRight(),
                        PlotCoordinate::PIXEL));    
        PlotSelectEvent pe(this, preg);
        PlotSelectEvent we(this, wreg);
        PlotSelectEvent ne(this, nreg);
        
        for(unsigned int i = 0; i < active.size(); i++) {
            switch(active[i]->getCoordinateSystem()) {
            case PlotCoordinate::WORLD: active[i]->handleSelect(we); break;
            case PlotCoordinate::PIXEL: active[i]->handleSelect(pe); break;
            case PlotCoordinate::NORMALIZED_WORLD:
                active[i]->handleSelect(ne); break;
            default: continue;
            }
            if(active[i]->isBlocking() && active[i]->lastEventWasHandled())
                return;
        }
        
        for(unsigned int i = 0; i < m_selectHandlers.size(); i++) {
            if(m_selectHandlers[i].second == PlotCoordinate::PIXEL)
                m_selectHandlers[i].first->handleSelect(pe);
            else if(m_selectHandlers[i].second == PlotCoordinate::WORLD)
                m_selectHandlers[i].first->handleSelect(we);
            else if(m_selectHandlers[i].second ==
                PlotCoordinate::NORMALIZED_WORLD)
                m_selectHandlers[i].first->handleSelect(ne);
        }
    }
}

void QPCanvas::timeout() {    
    // single click has occurred
    
    m_timer.stop();
    
    double x = m_clickEvent->globalX(), y = m_clickEvent->globalY();
    QRect rect = m_canvas.canvas()->contentsRect();
    QPoint p(rect.left(), rect.top());
    p = m_canvas.canvas()->mapToGlobal(p);
    
    PlotCoordinate pcoord(x - p.x(), y - p.y(), PlotCoordinate::PIXEL);
    PlotCoordinate wcoord = convertCoordinate(pcoord, PlotCoordinate::WORLD);
    PlotCoordinate ncoord = convertCoordinate(wcoord,
            PlotCoordinate::NORMALIZED_WORLD);
    
    PlotClickEvent pe(this, PlotMouseEvent::SINGLE, pcoord);
    PlotClickEvent we(this, PlotMouseEvent::SINGLE, wcoord);
    PlotClickEvent ne(this, PlotMouseEvent::SINGLE, ncoord);
    for(unsigned int i = 0; i < m_clickHandlers.size(); i++) {
        if(m_clickHandlers[i].second == PlotCoordinate::PIXEL)
            m_clickHandlers[i].first->handleClick(pe);
        else if(m_clickHandlers[i].second == PlotCoordinate::WORLD)
            m_clickHandlers[i].first->handleClick(we);
        else if(m_clickHandlers[i].second == PlotCoordinate::NORMALIZED_WORLD)
            m_clickHandlers[i].first->handleClick(ne);
    }
    
    m_clickEvent->accept();
}

}

#endif
