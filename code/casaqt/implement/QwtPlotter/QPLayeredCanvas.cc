//# QPLayeredCanvas.cc: Subclass of QwtPlot to add layers and other features.
//# Copyright (C) 2009
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

#include <casaqt/QwtPlotter/QPLayeredCanvas.qo.h>

#include <casaqt/QwtPlotter/QPCanvas.qo.h>

#include <qwt_painter.h>

namespace casa {

/////////////////////////
// QPLAYER DEFINITIONS //
/////////////////////////

QPLayer::QPLayer() { }
QPLayer::~QPLayer() { }

void QPLayer::clearImage() const {
    const_cast<QPixmap&>(m_cachedImage) = QPixmap(); }

void QPLayer::initializeImage(const QSize& size,
        const QwtScaleMap maps[QwtPlot::axisCnt]) const {
    const_cast<QPixmap&>(m_cachedImage) = QPixmap(size);
    const_cast<QPixmap&>(m_cachedImage).fill(Qt::transparent);
    
    for(int i = 0; i < QwtPlot::axisCnt; i++)
        const_cast<QwtScaleMap&>(m_cachedMaps[i]) = maps[i];
}

bool QPLayer::scaleMapsChanged(const QwtScaleMap maps[QwtPlot::axisCnt]) const{
    if(m_cachedImage.isNull()) return true;
    if(maps == m_cachedMaps) return false;
    
    QList<const QPLayerItem*> it = items();
    const QPLayerItem* li;
    for(int i = 0; i < it.size(); i++) {
        li = it[i];
        if(li == NULL || !li->shouldDraw()) continue;
        if(maps[li->xAxis()].s1() != m_cachedMaps[li->xAxis()].s1() ||
           maps[li->xAxis()].s2() != m_cachedMaps[li->xAxis()].s2() ||
           maps[li->yAxis()].s1() != m_cachedMaps[li->yAxis()].s1() ||
           maps[li->yAxis()].s2() != m_cachedMaps[li->yAxis()].s2())
            return true;
    }
    return false;
}

bool QPLayer::hasItemsToDraw() const {
    QList<const QPLayerItem*> it = items();
    for(int i = 0; i < it.size(); i++)
        if(it[i] != NULL && it[i]->shouldDraw()) return true;
    return false;
}

void QPLayer::drawItems(QPainter* painter, const QRect& canvasRect,
        const QwtScaleMap maps[QwtPlot::axisCnt]) const {
    QPDrawThread::drawItems(items(), painter, canvasRect, maps);
}

void QPLayer::cacheItems(const QRect& canvasRect,
        const QwtScaleMap maps[QwtPlot::axisCnt]) const {
    QPainter painter(&const_cast<QPixmap&>(m_cachedImage));
    drawItems(&painter, canvasRect, maps);
}

void QPLayer::drawCache(QPainter* painter, const QRect& rect) const {
    if(!m_cachedImage.isNull())
        painter->drawPixmap(rect.x(), rect.y(), m_cachedImage);
}

QPixmap& QPLayer::cachedImage() { return m_cachedImage; }
const QPixmap& QPLayer::cachedImage() const { return m_cachedImage; }


///////////////////////////////
// QPCANVASLAYER DEFINITIONS //
///////////////////////////////

QPCanvasLayer::QPCanvasLayer() { }
QPCanvasLayer::~QPCanvasLayer() { }

void QPCanvasLayer::addItem(QPPlotItem* item) {
    if(item == NULL || m_items.contains(item)) return;
    m_items.append(item);
}

void QPCanvasLayer::removeItem(QPPlotItem* item) { m_items.removeAll(item); }

bool QPCanvasLayer::containsItem(QPPlotItem* item) {
    return m_items.contains(item); }

#define QPL_ITEMS_HELPER(TYPE) \
    QList< TYPE *> it; \
    for(int i = 0; i < m_items.size(); i++) \
        it << m_items[i]; \
    return it;

#define QPL_ITEMS QPL_ITEMS_HELPER(QPLayerItem)
#define QPL_ITEMS_CONST QPL_ITEMS_HELPER(const QPLayerItem)

QList<const QPPlotItem*> QPCanvasLayer::canvasItems() const {
    QPL_ITEMS_HELPER(const QPPlotItem) }

QList<QPLayerItem*> QPCanvasLayer::items() { QPL_ITEMS }

QList<const QPLayerItem*> QPCanvasLayer::items() const { QPL_ITEMS_CONST }


/////////////////////////////
// QPBASELAYER DEFINITIONS //
/////////////////////////////

QPBaseLayer::QPBaseLayer() { }
QPBaseLayer::~QPBaseLayer() { }

void QPBaseLayer::addItem(QPBaseItem* item) {
    if(item == NULL || m_items.contains(item)) return;
    m_items.append(item);
}

void QPBaseLayer::removeItem(QPBaseItem* item) { m_items.removeAll(item); }

bool QPBaseLayer::containsItem(QPBaseItem* item) {
    return m_items.contains(item); }

QList<const QPBaseItem*> QPBaseLayer::canvasItems() const {
    QPL_ITEMS_HELPER(const QPBaseItem) }

QList<QPLayerItem*> QPBaseLayer::items() { QPL_ITEMS }

QList<const QPLayerItem*> QPBaseLayer::items() const { QPL_ITEMS_CONST }


/////////////////////////////////
// QPLAYEREDCANVAS DEFINITIONS //
/////////////////////////////////

// Constructors/Destructors //

QPLayeredCanvas::QPLayeredCanvas(QPCanvas* parent, QWidget* parentWidget) :
        QwtPlot(parentWidget), m_parent(parent) {
    initialize();
}

QPLayeredCanvas::QPLayeredCanvas(const QwtText& title, QPCanvas* parent,
        QWidget* parentWidget): QwtPlot(title, parentWidget), m_parent(parent){
    initialize();
}

QPLayeredCanvas::~QPLayeredCanvas() { }


// Public Methods //

QList<const QPPlotItem*> QPLayeredCanvas::allLayerItems(bool includeMain,
        bool includeAnnotation) const {
    QList<const QPPlotItem*> list;
    
    if(includeMain)       list << m_layerMain.canvasItems();
    if(includeAnnotation) list << m_layerAnnotation.canvasItems();
    
    return list;
}

void QPLayeredCanvas::print(QPainter* painter, const QRect& rect,
        const QwtPlotPrintFilter& filter) const {
    const_cast<bool&>(m_isPrinting) = true;
    QwtPlot::print(painter, rect, filter);
    const_cast<bool&>(m_isPrinting) = false;
}

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


// Protected Methods //

void QPLayeredCanvas::attachLayeredItem(QPPlotItem* item) {
    if(item == NULL) return;
    
    QPCanvasLayer* layer = NULL;
    if(item->canvasLayer() == MAIN)            layer = &m_layerMain;
    else if(item->canvasLayer() == ANNOTATION) layer = &m_layerAnnotation;
    
    if(layer == NULL) return;
    
    if(!layer->containsItem(item)) {
        layer->addItem(item);
        item->qwtAttach(this);
    }
}

void QPLayeredCanvas::detachLayeredItem(QPPlotItem* item) {
    if(item == NULL) return;
    
    QPCanvasLayer* layer = NULL;
    if(item->canvasLayer() == MAIN)            layer = &m_layerMain;
    else if(item->canvasLayer() == ANNOTATION) layer = &m_layerAnnotation;
    
    if(layer == NULL) return;
    
    if(layer->containsItem(item)) {
        layer->removeItem(item);
        item->qwtDetach();
    }
}

void QPLayeredCanvas::drawBaseItems() { replot(false, false); }

void QPLayeredCanvas::drawItems(QPainter* painter, const QRect& rect,
        const QwtScaleMap maps[axisCnt], const QwtPlotPrintFilter& pf) const {
    if(m_isPrinting) {
        printItems(painter, rect, maps, pf);
        return;
    }
    
    // Always draw the base items.
    m_layerBase.initializeImage(rect.size(), maps);
    m_layerBase.cacheItems(rect, maps);
    
    bool drawMain = m_drawMain || (m_layerMain.hasItemsToDraw() &&
                    m_layerMain.scaleMapsChanged(maps)),
         drawAnnotation=m_drawAnnotation||(m_layerAnnotation.hasItemsToDraw()&&
                        m_layerAnnotation.scaleMapsChanged(maps));
    
    // If drawing is held, just draw pixmap caches.
    if(m_drawingHeld || (!drawMain && !drawAnnotation)) {
        paintCaches(painter, rect);
        return;
    }
    
    // Check if a drawing thread is currently underway.
    if(m_drawThread != NULL) {
        const_cast<bool&>(m_redrawWaiting) = true;
        paintCaches(painter, rect);
        const_cast<QPDrawThread*&>(m_drawThread)->cancel();
        return;
    } else const_cast<bool&>(m_redrawWaiting) = false;
    
    // Set up caches.
    if(drawMain)       m_layerMain.initializeImage(rect.size(), maps);
    if(drawAnnotation) m_layerAnnotation.initializeImage(rect.size(), maps);
    
    bool doDraw = (drawMain && m_layerMain.hasItemsToDraw()) ||
                  (drawAnnotation && m_layerAnnotation.hasItemsToDraw());
    
    // Set up drawing thread.
    QPDrawThread* drawThread = NULL;
    if(doDraw) {
        drawThread = new QPDrawThread(allLayerItems(drawMain, drawAnnotation),
                                      maps, rect);
        connect(drawThread, SIGNAL(finished()), SLOT(itemDrawingFinished()));
        connect(drawThread, SIGNAL(terminated()), SLOT(itemDrawingFinished()));
        const_cast<QPDrawThread*&>(m_drawThread) = drawThread;
    }
    
    // Set up operation.
    PlotOperationPtr op = m_parent->operationDraw();
    if(!op.null()) {
        op->reset();
        op->setInProgress(true);
    }
    
    // Start logging.
    PlotLoggerPtr log;
    if(m_parent != NULL) log= m_parent->loggerForEvent(PlotLogger::DRAW_TOTAL);
    if(!log.null())
        log->markMeasurement(QPCanvas::CLASS_NAME, QPCanvas::DRAW_NAME);
            
    // Start drawing thread if needed.
    if(doDraw)
        drawThread->start();
    else {
        // Finish logging.
        PlotLoggerPtr log;
        if(m_parent != NULL)
            log = m_parent->loggerForEvent(PlotLogger::DRAW_TOTAL);
        if(!log.null()) log->releaseMeasurement();

        // Finish operation.
        PlotOperationPtr op;
        if(m_parent != NULL) op = m_parent->operationDraw();
        if(!op.null()) op->finish();
        
        // Draw pixmaps on widget.
        paintCaches(painter, rect);
    }
}

void QPLayeredCanvas::printItems(QPainter* painter, const QRect& rect,
        const QwtScaleMap maps[axisCnt], const QwtPlotPrintFilter& pf) const {    
    // Draw layers.
    m_layerBase.drawItems(painter, rect, maps);
    m_layerMain.drawItems(painter, rect, maps);
    m_layerAnnotation.drawItems(painter, rect, maps);
    
    // Draw internal legend if needed.
    if(m_parent->m_legend->legendShown() && m_parent->m_legend->isInternal()) {
        // printLegend needs the metrics map set.
        QwtMetricsMap old = QwtPainter::metricsMap();
        QwtPainter::setMetricsMap(this, painter->device());
        
        // Have to convert the canvas rect from device to layout, then find the
        // legend draw rect.
        QRect legendRect = QwtPainter::metricsMap().deviceToLayout(rect);
        legendRect = m_parent->m_legend->internalLegendRect(legendRect, false);

        printLegend_(painter, legendRect);
        QwtPainter::setMetricsMap(old);
    }
}

void QPLayeredCanvas::printLegend(QPainter* painter, const QRect& rect) const {
    // Only draw if external.  Internal plots will be drawn later because they
    // must go on top of the plot items.
    if(m_parent->m_legend->legendShown() && !m_parent->m_legend->isInternal())
        printLegend_(painter, rect);
}

void QPLayeredCanvas::printLegend_(QPainter* painter, const QRect& rect) const{
    m_parent->m_legend->drawOutlineAndBackground(painter, rect, true);
    QwtPlot::printLegend(painter, rect);
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

void QPLayeredCanvas::replot(bool drawMain, bool drawAnnotation) {
    bool oldMain = m_drawMain, oldAnnotation = m_drawAnnotation;
    setDrawLayers(drawMain, drawAnnotation);
    replot();
    setDrawLayers(oldMain, oldAnnotation);
}

void QPLayeredCanvas::setDrawLayers(bool main, bool annotation) {
    m_drawMain = main;
    m_drawAnnotation = annotation;
}

bool QPLayeredCanvas::drawMain() const { return m_drawMain; }
bool QPLayeredCanvas::drawAnnotation() const { return m_drawAnnotation; }

const QPGrid& QPLayeredCanvas::grid() const { return m_grid; }
QPGrid& QPLayeredCanvas::grid() { return m_grid; }

const QMap<PlotAxis, QPCartesianAxis*>& QPLayeredCanvas::cartesianAxes() const{
    return m_cartAxes; }
QMap<PlotAxis, QPCartesianAxis*>& QPLayeredCanvas::cartesianAxes() {
    return m_cartAxes; }

bool QPLayeredCanvas::cartesianAxisShown(PlotAxis axis) const {
    return m_cartAxes[axis] != NULL; }

void QPLayeredCanvas::showCartesianAxis(PlotAxis mirrorAxis,
        PlotAxis secondaryAxis, bool show) {
    QPCartesianAxis* a = m_cartAxes[mirrorAxis];
    if(a != NULL) {
        if(!show) {
            m_layerBase.removeItem(a);
            a->qpDetach();
            delete a;
            m_cartAxes[mirrorAxis] = NULL;
        }
    } else {
        if(show) {
            a = new QPCartesianAxis(QPOptions::axis(mirrorAxis),
                                    QPOptions::axis(secondaryAxis));
            m_layerBase.addItem(a);
            a->qpAttach(m_parent);
            m_cartAxes[mirrorAxis] = a;
        }
    }
}

void QPLayeredCanvas::installLegendFilter(QWidget* legendFrame) {
    m_legendFrame = legendFrame;
    if(legendFrame != NULL) legendFrame->installEventFilter(this);
}


// Private Methods //

void QPLayeredCanvas::initialize() {
    m_drawMain = m_drawAnnotation = true;
    m_isPrinting = false;
    m_drawingHeld = false;
    m_legendFrame = NULL;
    m_drawThread = NULL;
    m_redrawWaiting = false;
    
    m_grid.enableX(false);
    m_grid.enableXMin(false);
    m_grid.enableY(false);
    m_grid.enableYMin(false);
    m_layerBase.addItem(&m_grid);
    m_grid.qpAttach(m_parent);
    
    m_cartAxes[X_BOTTOM] = NULL;
    m_cartAxes[X_TOP] = NULL;
    m_cartAxes[Y_LEFT] = NULL;
    m_cartAxes[Y_RIGHT] = NULL;
    
    QwtPlotCanvas* c = canvas();
    c->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
    c->setAttribute(Qt::WA_PaintOutsidePaintEvent, true);
    //c->installEventFilter(this);    
}

void QPLayeredCanvas::paintCaches(QPainter* painter, const QRect& rect) const {
    if(painter == NULL) return;
    m_layerBase.drawCache(painter, rect);
    m_layerMain.drawCache(painter, rect);
    m_layerAnnotation.drawCache(painter, rect);
}


// Private Slots //

void QPLayeredCanvas::itemDrawingFinished() {
    if(m_drawThread == NULL) return;
        
    if(m_drawThread->isRunning() || !m_drawThread->isFinished()) {
        m_drawThread->cancel();
        return;
    }
        
    // Update caches.
    QPainter mainPainter(&m_layerMain.cachedImage()),
             annotationPainter(&m_layerAnnotation.cachedImage());
    m_drawThread->drawIntoCaches(mainPainter, annotationPainter);
    mainPainter.end();
    annotationPainter.end();
        
    // Cleanup.
    delete m_drawThread;
    m_drawThread = NULL;
    
    // Finish logging.
    PlotLoggerPtr log;
    if(m_parent != NULL) log= m_parent->loggerForEvent(PlotLogger::DRAW_TOTAL);
    if(!log.null()) log->releaseMeasurement();

    // Finish operation.
    PlotOperationPtr op;
    if(m_parent != NULL) op = m_parent->operationDraw();
    if(!op.null()) op->finish();
    
    // Draw pixmaps on widget.
    if(!m_redrawWaiting) replot(false, false);
    else                 replot();
}

}

#endif
