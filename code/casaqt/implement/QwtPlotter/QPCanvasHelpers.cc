//# QPCanvasHelpers.cc: Helper classes for QPCanvas.
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

#include <casaqt/QwtPlotter/QPCanvasHelpers.qo.h>

#include <casa/OS/Time.h>
#include <casaqt/QtUtilities/QtLayeredLayout.h>
#include <casaqt/QwtPlotter/QPCanvas.qo.h>

#include <qwt_painter.h>

#include <QPainter>

#include <iomanip>

namespace casa {

///////////////////////////////
// QPMOUSEFILTER DEFINITIONS //
///////////////////////////////

QPMouseFilter::QPMouseFilter(QwtPlotCanvas* canvas) : m_canvas(canvas) {
    canvas->installEventFilter(this);
}

QPMouseFilter::~QPMouseFilter() { }

void QPMouseFilter::turnTracking(bool on) {
    m_canvas->setMouseTracking(on); }

bool QPMouseFilter::eventFilter(QObject* obj, QEvent* ev) {
    if(obj == m_canvas) {
        QMouseEvent* me = dynamic_cast<QMouseEvent*>(ev);
        if(me != NULL) emit mouseMoveEvent(me);
    }
    return false;
}


/////////////////////////////
// QPDATESCALEDRAW METHODS //
/////////////////////////////

QPDateScaleDraw::QPDateScaleDraw(PlotAxisScale scale, unsigned int numDecimals,
        const String& format) : m_scale(scale), m_decimals(numDecimals),
        m_format(format) {
    if(!formatIsValid(m_format)) m_format = DEFAULT_FORMAT;
}

QPDateScaleDraw::~QPDateScaleDraw() { }


PlotAxisScale QPDateScaleDraw::scale() const { return m_scale; }

QwtText QPDateScaleDraw::label(double value) const {
    if(m_scale != DATE_MJ_DAY && m_scale != DATE_MJ_SEC)
        return QwtText(QString().setNum(value, 'g', m_decimals));
        
    stringstream ss;
    if(m_scale == DATE_MJ_SEC) value /= 86400;
    
    Time t(value + 2400000.5);

    char c;
    for(unsigned int i = 0; i < m_format.length(); i++) {
        c = m_format[i];
        if(c == '%' && i < m_format.length() - 1) {
            c = m_format[++i];
            switch(c) {
            case 'y': case 'Y': ss << t.year(); break;
            case 'm': case 'M':
                if(t.month() < 10) ss << '0';
                ss << t.month();
                break;
            case 'd': case 'D':
                if(t.dayOfMonth() < 10) ss << '0';
                ss << t.dayOfMonth();
                break;
            case 'h': case 'H':
                if(t.hours() < 10) ss << '0';
                ss << t.hours();
                break;
            case 'n': case 'N':
                if(t.minutes() < 10) ss << '0';
                ss << t.minutes();
                break;
            case 's': case 'S':
                double sec;
                sec = modf(value, &sec);
                sec += t.seconds();
                
                if(sec < 10) ss << '0';
                ss << fixed << setprecision(m_decimals) << sec;
                break;
            
            default: ss << m_format[i - 1] << c; break;
            }
        } else ss << c;
    }
    
    return QString(ss.str().c_str());
}


// Static //

bool QPDateScaleDraw::formatIsValid(String d) {
    unsigned int i;
    if((i = d.find("%y")) < d.size()) {
        if((i = d.find("%y", i + 1)) < d.size()) return false;
    } else return false;

    if((i = d.find("%m")) < d.size()) {
        if((i = d.find("%m", i + 1)) < d.size()) return false;
    } else return false;

    if((i = d.find("%d")) < d.size()) {
        if((i = d.find("%d", i + 1)) < d.size()) return false;
    } else return false;

    if((i = d.find("%h")) < d.size()) {
        if((i = d.find("%h", i + 1)) < d.size()) return false;
    } else return false;

    if((i = d.find("%n")) < d.size()) {
        if((i = d.find("%n", i + 1)) < d.size()) return false;
    } else return false;

    if((i = d.find("%s")) < d.size()) {
        if((i = d.find("%s", i + 1)) < d.size()) return false;
    } else return false;

    return false;
}

const String QPDateScaleDraw::DEFAULT_FORMAT = "%y/%m/%d\n%h:%n:%s";


//////////////////////////
// QPLEGEND DEFINITIONS //
//////////////////////////

QPLegend::QPLegend(QWidget* parent) : QwtLegend(parent) {
    setAutoFillBackground(false);
    
    // Clear line and background.
    m_line.setStyle(PlotLine::NOLINE);
    m_areaFill.setPattern(PlotAreaFill::NOFILL);
    
    QPalette p = palette();
    p.setBrush(backgroundRole(), QBrush(Qt::NoBrush));
    setPalette(p);
}

QPLegend::~QPLegend() { }


QSize QPLegend::sizeHint() const {
    QSize size = QwtLegend::sizeHint();
    if(!size.isValid() || m_line.style() == PlotLine::NOLINE) return size;
    
    int width = (int)(m_line.width() + 0.5);
    if(width > 0) size += QSize(width * 2 + 1, width * 2 + 1);
    return size;
}

const QPLine& QPLegend::line() const { return m_line; }
const QPen& QPLegend::pen() const { return m_line.asQPen(); }
void QPLegend::setLine(const PlotLine& line) {
    if(line != m_line) {
        m_line = QPLine(line);
        if(isVisible()) update();
    }
}

const QPAreaFill& QPLegend::areaFill() const { return m_areaFill; }
const QBrush& QPLegend::brush() const { return m_areaFill.asQBrush(); }
void QPLegend::setAreaFill(const PlotAreaFill& fill) {
    if(fill != m_areaFill) {
        m_areaFill = QPAreaFill(fill);
        if(isVisible()) update();
    }
}

void QPLegend::drawOutlineAndBackground(QPainter* painter, const QRect& rect,
        bool useQwtPainter) {
    if(painter == NULL || !isVisible() || !rect.isValid() ||
       (m_line.style() == PlotLine::NOLINE &&
        m_areaFill.pattern() == PlotAreaFill::NOFILL)) return;
    
    painter->save();
    if(m_line.style() != PlotLine::NOLINE) painter->setPen(m_line.asQPen());
    if(m_areaFill.pattern() != PlotAreaFill::NOFILL)
        painter->setBrush(m_areaFill.asQBrush());
    if(useQwtPainter) QwtPainter::drawRect(painter, rect);
    else              painter->drawRect(rect);
    painter->restore();
}


void QPLegend::paintEvent(QPaintEvent* event) {
    // Draw outline and background if needed.
    QRect geom(QPoint(0, 0), size());
    geom.setRight(geom.right() - 1);   // Compensate for +1 in sizeHint().
    geom.setBottom(geom.bottom() - 1);
    QPainter painter(this);
    drawOutlineAndBackground(&painter, geom, false);
    painter.end();
    
    // Calculate "contents" rect.
    QRect rect = event->rect();
    rect.setRight(rect.right() - 1);   // Compensate for +1 in sizeHint().
    rect.setBottom(rect.bottom() - 1);
    
    int width = 0;
    if(m_line.style() != PlotLine::NOLINE) width = (int)(m_line.width() + 0.5);
    if(width > 0) {
        rect.setTop(rect.top() + width);
        rect.setLeft(rect.left() + width);
        rect.setRight(rect.right() - width);
        rect.setBottom(rect.bottom() - width);
    }

    // Have rest of the legend draw itself normally.
    QPaintEvent innerEvent(rect);
    QwtLegend::paintEvent(&innerEvent);
}


////////////////////////////////
// QPLEGENDHOLDER DEFINITIONS //
////////////////////////////////

const int QPLegendHolder::DEFAULT_INTERNAL_PADDING = 10;

QwtPlot::LegendPosition
QPLegendHolder::legendPosition(PlotCanvas::LegendPosition pos) {
    switch(pos) {
    case PlotCanvas::EXT_TOP:    return QwtPlot::TopLegend;
    case PlotCanvas::EXT_RIGHT:  return QwtPlot::RightLegend;
    case PlotCanvas::EXT_LEFT:   return QwtPlot::LeftLegend;
    case PlotCanvas::EXT_BOTTOM: return QwtPlot::BottomLegend;
    default:                     return QwtPlot::ExternalLegend;
    }
}


// Constructor which takes the canvas and initial position.
QPLegendHolder::QPLegendHolder(QPCanvas* canvas, PlotCanvas::LegendPosition ps,
        int padding) : m_canvas(canvas), m_legend(new QPLegend()),
        m_position(ps), m_padding(padding) {
    // Set up layout.
    QGridLayout* l = new QGridLayout(this);
    int ml, mt, mr, mb;
    canvas->asQwtPlot().getContentsMargins(&ml, &mt, &mr, &mb);
    l->setContentsMargins(ml, mt, mr, mb);
    l->setSpacing(0);
    if(m_padding < 0) m_padding = DEFAULT_INTERNAL_PADDING;
    
    // Put in spacers and legend.
    l->addItem(m_spaceTop = new QSpacerItem(0, 0), 0, 0, 1, 3);
    l->addItem(m_spaceLeft = new QSpacerItem(0, 0), 1, 0);
    l->addItem(m_spaceRight = new QSpacerItem(0, 0), 1, 2);
    l->addItem(m_spaceBottom = new QSpacerItem(0, 0), 2, 0, 1, 3);
    
    // Add to canvas.
    QtLayeredLayout* cl = dynamic_cast<QtLayeredLayout*>(canvas->layout());
    if(cl != NULL) cl->addWidget(this);
    setPosition(ps);
    
    // Set up tracking, etc.
    canvas->asQwtPlot().installLegendFilter(this);
    setMouseTracking(true);
}

QPLegendHolder::~QPLegendHolder() { }


bool QPLegendHolder::legendShown() const {
    return isVisible() && m_legend->isVisible(); }
void QPLegendHolder::showLegend(bool show) {
    m_legend->setVisible(show);
    m_canvas->asQwtPlot().insertLegend(show ? m_legend : NULL,
            legendPosition(m_position));
}

bool QPLegendHolder::isInternal() const {
    return PlotCanvas::legendPositionIsInternal(m_position); }
PlotCanvas::LegendPosition QPLegendHolder::position() const { return m_position; }
void QPLegendHolder::setPosition(PlotCanvas::LegendPosition pos) {
    m_position = pos;

    QGridLayout* l = dynamic_cast<QGridLayout*>(layout());
    l->removeWidget(m_legend);
    if(isInternal()) {
        l->addWidget(m_legend, 1, 1);
        updateSpacers();
    }

    m_canvas->asQwtPlot().insertLegend(m_legend, legendPosition(pos));
}

QRect QPLegendHolder::internalLegendRect(const QRect& canvRect,
        bool useQwtPainter) const {
    if(!legendShown() || !isInternal()) return QRect();
    
    QSize size = m_legend->sizeHint();
    int paddingx = m_padding, paddingy = m_padding;
    
    if(useQwtPainter && !QwtPainter::metricsMap().isIdentity()) {
        size = QwtPainter::metricsMap().layoutToDevice(size);
        paddingx = QwtPainter::metricsMap().layoutToDeviceX(paddingx);
        paddingy = QwtPainter::metricsMap().layoutToDeviceY(paddingy);
    }
    
    QRect rect(QPoint(0, 0), size);
    if(m_position == PlotCanvas::INT_ULEFT ||
       m_position == PlotCanvas::INT_URIGHT)
        rect.moveTop(canvRect.top() + paddingy);
    if(m_position == PlotCanvas::INT_ULEFT ||
       m_position == PlotCanvas::INT_LLEFT)
        rect.moveLeft(canvRect.left() + paddingx);
    if(m_position == PlotCanvas::INT_URIGHT ||
       m_position == PlotCanvas::INT_LRIGHT)
        rect.moveRight(canvRect.right() - paddingx);
    if(m_position == PlotCanvas::INT_LLEFT ||
       m_position == PlotCanvas::INT_LRIGHT)
        rect.moveBottom(canvRect.bottom() - paddingy);
    
    return rect;
}


void QPLegendHolder::updateSpacers() {
    // Adjust spacing for position.
    int top = 0, left = 0, right = 0, bottom = 0;
    QSizePolicy::Policy pt = QSizePolicy::Expanding, pl = pt, pr = pt,
                        pb = pt;
    if(m_position==PlotCanvas::INT_ULEFT||m_position==PlotCanvas::INT_URIGHT){
        top = m_padding; pt = QSizePolicy::Fixed; }
    if(m_position==PlotCanvas::INT_LLEFT||m_position==PlotCanvas::INT_LRIGHT){
        bottom = m_padding; pb = QSizePolicy::Fixed; }
    if(m_position==PlotCanvas::INT_LLEFT||m_position==PlotCanvas::INT_ULEFT){
        left = m_padding; pl = QSizePolicy::Fixed; }
    if(m_position==PlotCanvas::INT_LRIGHT||m_position==PlotCanvas::INT_URIGHT){
        right = m_padding; pr = QSizePolicy::Fixed; }
        
    // Adjust spacing for actual canvas.
    QRect orect = m_canvas->contentsRect();
    orect.moveTopLeft(m_canvas->mapToGlobal(orect.topLeft()));
    QwtPlotCanvas* canv = m_canvas->asQwtPlot().canvas();
    QRect irect = canv->contentsRect();
    irect.moveTopLeft(canv->mapToGlobal(irect.topLeft()));
    if(top > 0) top += irect.top() - orect.top();
    if(left > 0) left += irect.left() - orect.left();
    if(right > 0) right += orect.right() - irect.right();
    if(bottom > 0) bottom += orect.bottom() - irect.bottom();
    
    // Set spacing.
    m_spaceTop->changeSize(0, top, QSizePolicy::Minimum, pt);
    m_spaceLeft->changeSize(left, 0, pl, QSizePolicy::Minimum);
    m_spaceRight->changeSize(right, 0, pr, QSizePolicy::Minimum);
    m_spaceBottom->changeSize(0, bottom, QSizePolicy::Minimum, pb);
}


////////////////////////////
// QPBASEITEM DEFINITIONS //
////////////////////////////

const double QPBaseItem::BASE_Z_CARTAXIS = -2;
const double QPBaseItem::BASE_Z_GRID = -1;

QPBaseItem::QPBaseItem() : m_canvas(NULL) { }

QPBaseItem::~QPBaseItem() { qpDetach(); }

void QPBaseItem::qpAttach(QPCanvas* canvas) {
    if(m_canvas != canvas) {
        qpDetach();
        m_canvas = canvas;
        if(m_canvas != NULL) QwtPlotItem::attach(&m_canvas->asQwtPlot());
    }
}

void QPBaseItem::qpDetach() {
    if(m_canvas != NULL) {
        QwtPlotItem::detach();
        m_canvas = NULL;
    }
}


////////////////////////
// QPGRID DEFINITIONS //
////////////////////////

QPGrid::QPGrid() { QPBaseItem::setZ(BASE_Z_GRID); }
QPGrid::~QPGrid() { }

bool QPGrid::shouldDraw() const {
    bool drawMaj = majPen().style() != Qt::NoPen && (xEnabled() || yEnabled());
    bool drawMin = minPen().style() != Qt::NoPen &&
                   (xMinEnabled() || yMinEnabled());
    return drawMaj || drawMin;
}


/////////////////////////////////
// QPCARTESIANAXIS DEFINITIONS //
/////////////////////////////////

QPCartesianAxis::QPCartesianAxis(QwtPlot::Axis master, QwtPlot::Axis slave) :
        m_axis(master), m_scaleDraw() {
    if(master == QwtPlot::yLeft || master == QwtPlot::yRight)
        setAxis(slave, master);
    else
        setAxis(master, slave);
    
    QwtScaleDraw::Alignment a;
    switch(master) {
    case QwtPlot::yLeft: a = QwtScaleDraw::LeftScale; break;
    case QwtPlot::yRight: a = QwtScaleDraw::RightScale; break;
    case QwtPlot::xTop: a = QwtScaleDraw::TopScale; break;
    default: a = QwtScaleDraw::BottomScale;
    }
    m_scaleDraw.setAlignment(a);
    
    setZ(BASE_Z_CARTAXIS);
}

QPCartesianAxis::~QPCartesianAxis() { }

void QPCartesianAxis::draw_(QPainter* painter, const QwtScaleMap& xMap,
                  const QwtScaleMap& yMap, const QRect& drawRect,
                  unsigned int drawIndex, unsigned int drawCount) const {
    QwtScaleDraw* s = const_cast<QwtScaleDraw*>(&m_scaleDraw);
    if(m_axis == QwtPlot::yLeft || m_axis == QwtPlot::yRight) {
        s->move(QPOptions::round(xMap.xTransform(0.0)),
                QPOptions::round(yMap.p2()));
        s->setLength(QPOptions::round(yMap.p1() - yMap.p2()));
    } else if(m_axis == QwtPlot::xBottom || m_axis == QwtPlot::xTop) {
        s->move(QPOptions::round(xMap.p1()),
                QPOptions::round(yMap.xTransform(0.0)));
        s->setLength(QPOptions::round(xMap.p2() - xMap.p1()));
    }
    
    s->setScaleDiv(*(plot()->axisScaleDiv(m_axis)));
    s->draw(painter, plot()->palette());
}

}

#endif
