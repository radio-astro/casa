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

#include <casaqt/QwtPlotter/QPCanvas.qo.h>
#include <casaqt/QwtPlotter/QPOptions.h>

#include <casa/OS/Time.h>

#include <qwt_painter.h>

#include <QApplication>
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


//////////////////////////////////////
// QPCARTESIANSCALEDRAW DEFINITIONS //
//////////////////////////////////////

QPCartesianScaleDraw::QPCartesianScaleDraw() { }

QPCartesianScaleDraw::~QPCartesianScaleDraw() { }

void QPCartesianScaleDraw::drawBackbone(QPainter* p) const {
    QPen pen = p->pen();
    if(pen.style() == Qt::NoPen) {
        // we're in PDF/PS-printing mode, so fix weird bug
        pen.setStyle(Qt::SolidLine);
        p->setPen(pen);
    }
    
    /* The rest is basically copied from
     * http://qwt.sourceforge.net/qwt__scale__draw_8cpp-source.html
     */
    
    const int bw2 = p->pen().width() / 2;
    const QPoint pos = this->pos();
    const int len = length() - 1;

    switch(alignment()) {
        case LeftScale:
            QwtPainter::drawLine(p, pos.x() - bw2,
                pos.y(), pos.x() - bw2, pos.y() + len );
            break;
        case RightScale:
            QwtPainter::drawLine(p, pos.x() + bw2,
                pos.y(), pos.x() + bw2, pos.y() + len);
            break;
        case TopScale:
            QwtPainter::drawLine(p, pos.x(), pos.y() - bw2,
                pos.x() + len, pos.y() - bw2);
            break;
        case BottomScale:
            QwtPainter::drawLine(p, pos.x(), pos.y() + bw2,
                pos.x() + len, pos.y() + bw2);
            break;
    }
}

void QPCartesianScaleDraw::drawTick(QPainter* p, double value, int len) const {
    QPen pen = p->pen();
    if(pen.style() == Qt::NoPen) {
        // we're in PDF/PS-printing mode, so fix weird bug
        pen.setStyle(Qt::SolidLine);
        p->setPen(pen);
    }
    
    /* The rest is basically copied from
     * http://qwt.sourceforge.net/qwt__scale__draw_8cpp-source.html
     */
    
    if(len <= 0) return;
    
    int pw2 = qwtMin((int)p->pen().width(), len) / 2;
    
    QwtScaleMap scaleMap = map();
    const QwtMetricsMap metricsMap = QwtPainter::metricsMap();
    QPoint pos = this->pos();
    
    if(!metricsMap.isIdentity()) {
        QwtPainter::resetMetricsMap();
        pos = metricsMap.layoutToDevice(pos);
        
        if(orientation() == Qt::Vertical) {
            scaleMap.setPaintInterval(
                    metricsMap.layoutToDeviceY((int)scaleMap.p1()),
                    metricsMap.layoutToDeviceY((int)scaleMap.p2()));
            len = metricsMap.layoutToDeviceX(len);
        } else {
            scaleMap.setPaintInterval(
                    metricsMap.layoutToDeviceX((int)scaleMap.p1()),
                    metricsMap.layoutToDeviceX((int)scaleMap.p2()));
            len = metricsMap.layoutToDeviceY(len);
        }
    }
    
    const int tval = scaleMap.transform(value);
    
    switch(alignment()) {
    case LeftScale:
        QwtPainter::drawLine(p, pos.x() - pw2, tval, pos.x() - len, tval);
        break;
    case RightScale:
        QwtPainter::drawLine(p, pos.x() + pw2, tval, pos.x() + len, tval);
        break;
    case BottomScale:
        QwtPainter::drawLine(p, tval, pos.y() + pw2, tval, pos.y() + len);
        break;
    case TopScale:
        QwtPainter::drawLine(p, tval, pos.y() - pw2, tval, pos.y() - len);
        break;
    }
    
    QwtPainter::setMetricsMap(metricsMap);
}


/////////////////////////////////
// QPCARTESIANAXIS DEFINITIONS //
/////////////////////////////////

QPCartesianAxis::QPCartesianAxis(QwtPlot::Axis master, QwtPlot::Axis slave) :
        QwtPlotItem(), m_axis(master), m_scaleDraw() {
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
}

QPCartesianAxis::~QPCartesianAxis() { }

void QPCartesianAxis::draw(QPainter* painter, const QwtScaleMap& xMap,
                  const QwtScaleMap& yMap, const QRect&) const {
    QPCartesianScaleDraw* s = const_cast<QPCartesianScaleDraw*>(&m_scaleDraw);
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

const String QPDateScaleDraw::DEFAULT_FORMAT = "%y-%m-%d\n%h:%n:%s";


/////////////////////////////////
// QPLAYEREDCANVAS DEFINITIONS //
/////////////////////////////////

QPLayeredCanvas::QPLayeredCanvas(QPCanvas* parent, QWidget* parentWidget) :
        QwtPlot(parentWidget), m_parent(parent) {
    initialize();
}

QPLayeredCanvas::QPLayeredCanvas(const QwtText& title, QPCanvas* parent,
        QWidget* parentWidget): QwtPlot(title, parentWidget), m_parent(parent){
    initialize();
}

QPLayeredCanvas::~QPLayeredCanvas() { }


bool QPLayeredCanvas::eventFilter(QObject* watched, QEvent* event) {
    if(watched == NULL || event == NULL || m_parent == NULL) return false;
    
    QwtPlotCanvas* c = canvas();
    
    // Steal input events and send to canvas.
    if(watched == m_legendFrame && dynamic_cast<QInputEvent*>(event) != NULL) {
        QEvent* sendEvent = event;
        QMouseEvent* me; QWheelEvent* we;
        
        // need to move the event to be relative to the canvas
        if((me = dynamic_cast<QMouseEvent*>(event)) != NULL) {
            sendEvent = new QMouseEvent(me->type(),
                                        c->mapFromGlobal(me->globalPos()),
                                        me->button(),
                                        me->buttons(),
                                        me->modifiers());
        } else if((we = dynamic_cast<QWheelEvent*>(event)) != NULL) {
            sendEvent = new QWheelEvent(c->mapFromGlobal(we->globalPos()),
                                        we->delta(),
                                        we->buttons(),
                                        we->modifiers(),
                                        we->orientation());
        }
        
        // first try sending to picker
        if(!m_parent->getSelecter().eventFilter(c, sendEvent)) {
            // ..otherwise send to canvas
            bool ret = qApp->notify(c, sendEvent);
            if(sendEvent != event) delete sendEvent;
            return ret;
        }
        if(sendEvent != event) delete sendEvent;
    }
    
    // We didn't need it, so pass it up the chain.
    return QwtPlot::eventFilter(watched, event);
}


void QPLayeredCanvas::drawItems(QPainter* painter, const QRect& rect,
        const QwtScaleMap maps[axisCnt], const QwtPlotPrintFilter& pf) const {
    // If drawing is held, just draw pixmap caches.
    if(m_drawingHeld) {
        if(!m_main.isNull())  painter->drawPixmap(rect.x(), rect.y(), m_main);
        if(!m_layer.isNull()) painter->drawPixmap(0, 0, m_layer);
        return;
    }
    
    PlotOperationPtr op;
    if(m_parent != NULL) op = m_parent->operationDraw();
    if(!op.null()) {
        op->reset();
        op->setInProgress(true);
    }
    
    PlotLoggerPtr log;
    if(m_parent != NULL)
        log = m_parent->loggerForMeasurement(PlotLogger::DRAW_TOTAL);
    if(!log.null()) log->markMeasurement(QPCanvas::CLASS_NAME,"drawItems");
    
    // First draw normally...
    if(m_drawMain) {
        if(!op.null()) {
            op->setCurrentStatus("Drawing main layer.");
        }
        
        QwtPlot::drawItems(painter, rect, maps, pf);
        QPixmap& pm = const_cast<QPixmap&>(m_main);
        const QPixmap* cache = canvas()->paintCache();
        if(cache != NULL) pm = QPixmap(*cache);
        else {
            pm = QPixmap(rect.width(), rect.height());
            pm.fill(Qt::transparent);
        }
        
        if(!op.null() && m_drawLayer) {
            op->setCurrentProgress(50);
        }
        
    } else if(!m_main.isNull())
        painter->drawPixmap(rect.x(), rect.y(), m_main);
    
    // Then draw layered items as needed.
    if(m_drawLayer) {
        if(!op.null()) {
            op->setCurrentStatus("Drawing annotation layer.");
        }
        
        QPixmap& pm = const_cast<QPixmap&>(m_layer);
        pm = QPixmap(rect.width(), rect.height());
        pm.fill(Qt::transparent);
    
        if(m_layeredItems.size() > 0) {
            QPainter p(&pm);
            QwtPlotItem* item;
            for(unsigned int i = 0; i < m_layeredItems.size(); i++) {
                item = m_layeredItems[i];
                if(item != NULL && item->isVisible()) {
                    if(!(pf.options() & QwtPlotPrintFilter::PrintGrid) &&
                       item->rtti() == QwtPlotItem::Rtti_PlotGrid) continue;
                
                    p.save();
                    p.setRenderHint(QPainter::Antialiasing,
                         item->testRenderHint(QwtPlotItem::RenderAntialiased));
                    item->draw(&p, maps[item->xAxis()], maps[item->yAxis()],
                               rect);
                    p.restore();
                }
            }
        }
    }
    
    if(!m_layer.isNull()) // I don't know why this must be offset?
        painter->drawPixmap(0, 0, m_layer);
    
    if(!log.null()) log->releaseMeasurement();
    
    if(!op.null()) {
        op->setCurrentProgress(100);
        op->setIsFinished(true);
    }
}

void QPLayeredCanvas::attachLayeredItem(QwtPlotItem* item) {
    if(item == NULL) return;
    for(unsigned int i = 0; i < m_layeredItems.size(); i++)
        if(item == m_layeredItems[i]) return;
    m_layeredItems.push_back(item);
}

void QPLayeredCanvas::detachLayeredItem(QwtPlotItem* item) {
    for(unsigned int i = 0; i < m_layeredItems.size(); i++) {
        if(item == m_layeredItems[i]) {
            m_layeredItems.erase(m_layeredItems.begin() + i);
            break;
        }
    }
}

void QPLayeredCanvas::setDrawLayers(bool main, bool layer) {
    m_drawMain = main;
    m_drawLayer = layer;
}

bool QPLayeredCanvas::drawingIsHeld() const { return m_drawingHeld; }

void QPLayeredCanvas::holdDrawing() {
    setAutoReplot(false);
    m_drawingHeld = true;
}

void QPLayeredCanvas::releaseDrawing() {
    setAutoReplot(true);
    m_drawingHeld = false;
    replot();
}

void QPLayeredCanvas::replot(bool drawMain, bool drawLayer) {
    setDrawLayers(drawMain, drawLayer);
    replot();
    setDrawLayers(true, true);
}

void QPLayeredCanvas::installLegendFilter(QFrame* legendFrame) {
    m_legendFrame = legendFrame;
    if(legendFrame != NULL) legendFrame->installEventFilter(this);
}


void QPLayeredCanvas::initialize() {
    m_drawMain = m_drawLayer = true;
    m_drawingHeld = false;
    m_legendFrame = NULL;
    
    QwtPlotCanvas* c = canvas();
    c->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
    c->setAttribute(Qt::WA_PaintOutsidePaintEvent, true);
    //c->installEventFilter(this);    
}

}

#endif
