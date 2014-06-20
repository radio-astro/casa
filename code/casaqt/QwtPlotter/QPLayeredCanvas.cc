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
#include <qwt_scale_widget.h>

namespace casa {

/////////////////////////
// QPLAYER DEFINITIONS //
/////////////////////////

QPLayer::QPLayer() { }
QPLayer::~QPLayer() { }

void QPLayer::clearImage() const {
    const_cast<QPImageCache&>(m_cachedImage) = QPImageCache(); }

void QPLayer::initializeImage(const QSize& size, unsigned int fillValue) const{
    const_cast<QPImageCache&>(m_cachedImage) = QPImageCache(size);
    const_cast<QPImageCache&>(m_cachedImage).fill(fillValue);
}

bool QPLayer::hasItemsToDraw() const {
    QList<const QPLayerItem*> it = items();
    for(int i = 0; i < it.size(); i++)
        if(it[i] != NULL && it[i]->shouldDraw()) return true;
    return false;
}

unsigned int QPLayer::itemDrawCount() const {
    unsigned int n = 0;
    QList<const QPLayerItem*> it = items();
    for(int i = 0; i < it.size(); i++)
        if(it[i] != NULL && it[i]->shouldDraw()) n += it[i]->itemDrawCount();
    return n;
}

unsigned int QPLayer::itemDrawSegments(unsigned int threshold) const {
    unsigned int n = 0;
    QList<const QPLayerItem*> it = items();
    for(int i = 0; i < it.size(); i++) {
        if(it[i] != NULL && it[i]->shouldDraw())
            n += it[i]->itemDrawSegments(threshold);
    }
    return n;
}

void QPLayer::drawItems(QPainter* painter, const QRect& canvasRect,
        const QwtScaleMap maps[QwtPlot::axisCnt], PlotOperationPtr op,
        unsigned int currentSegment, unsigned int totalSegments,
        unsigned int segmentThreshold) const {
    QPDrawThread::drawItems(items(), painter, canvasRect, maps, op,
            currentSegment, totalSegments, segmentThreshold);
}

void QPLayer::cacheItems(const QRect& canvasRect,
        const QwtScaleMap maps[QwtPlot::axisCnt], PlotOperationPtr op,
        unsigned int currentSegment, unsigned int totalSegments,
        unsigned int segmentThreshold) const {
    QPainter* painter = const_cast<QPImageCache&>(m_cachedImage).painter();
    drawItems(painter, canvasRect, maps, op, currentSegment, totalSegments,
              segmentThreshold);
    delete painter;
}

void QPLayer::drawCache(QPainter* painter, const QRect& rect) const {
    m_cachedImage.paint(painter, rect); }

QPImageCache& QPLayer::cachedImage() { return m_cachedImage; }
const QPImageCache& QPLayer::cachedImage() const { return m_cachedImage; }


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

// Static //

const String QPLayeredCanvas::CLASS_NAME = "QPLayeredCanvas";


// Constructors/Destructors //

QPLayeredCanvas::QPLayeredCanvas(QPCanvas* parent, QWidget* parentWidget) :
        QwtPlot(parentWidget), m_parent(parent) {
    initialize();
}

QPLayeredCanvas::QPLayeredCanvas(const QwtText& title, QPCanvas* parent,
        QWidget* parentWidget): QwtPlot(title, parentWidget), m_parent(parent){
    initialize();
}

QPLayeredCanvas::~QPLayeredCanvas() {
    foreach(QPCanvasLayer* layer, m_layers)
        delete layer;
}


// Public Methods //

QList<const QPPlotItem*> QPLayeredCanvas::allLayerItems(int layersFlag) const {
    QList<const QPPlotItem*> list;
    
    const QPCanvasLayer* l;
    foreach(PlotCanvasLayer layer, m_layers.keys())
        if(layersFlag & layer)
            list << (l = m_layers.value(layer))->canvasItems();
    
    return list;
}

QRect QPLayeredCanvas::canvasDrawRect() const {
    QRect rect;
    
    QwtScaleMap s = canvasMap(xBottom);
    rect.setLeft((int)(s.p1() + 0.5));
    rect.setRight((int)(s.p2() + 0.5));
    
    s = canvasMap(QwtPlot::yLeft);
    rect.setBottom((int)(s.p1() + 0.5));
    rect.setTop((int)(s.p2() + 0.5));
    
    return rect;
}

void QPLayeredCanvas::print(QPainter* painter, const QRect& rect,
        const QwtPlotPrintFilter& filter) const {
    m_parent->logMethod(CLASS_NAME, "print", true);
    if(painter == NULL) {
        m_parent->logMethod(CLASS_NAME, "print", false);
        return;
    }
    
    // Set up operation.
    PlotOperationPtr op = m_parent->operationExport();
    if(!op.null()) {
        op->reset();
        op->setInProgress(true);
    }
    
    if(!m_printPainters.contains(painter))
        const_cast<QList<QPainter*>&>(m_printPainters).append(painter);
    const_cast<bool&>(m_isPrinting) = true;
    QwtPlot::print(painter, rect, filter);
    const_cast<QList<QPainter*>&>(m_printPainters).removeAll(painter);
    const_cast<bool&>(m_isPrinting) = false;
    
    // Finish operation and set status as saving to file.
    if(!op.null()) {
        op->finish();
        op->setCurrentStatus("Saving to file (may take a while)...");
    }
    m_parent->logMethod(CLASS_NAME, "print", false);
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
    
    QPCanvasLayer* layer = m_layers.value(item->canvasLayer());    
    if(layer == NULL) return;
    
    if(!layer->containsItem(item)) {
        layer->addItem(item);
        item->qwtAttach(this);
    }
}

void QPLayeredCanvas::detachLayeredItem(QPPlotItem* item) {
    if(item == NULL) return;
    
    QPCanvasLayer* layer = m_layers.value(item->canvasLayer());    
    if(layer == NULL) return;
    
    if(layer->containsItem(item)) {
        layer->removeItem(item);
        item->qwtDetach();
    }
}

void QPLayeredCanvas::drawItems(QPainter* painter, const QRect& cRect,
        const QwtScaleMap cMaps[axisCnt], const QwtPlotPrintFilter& pf) const {
    m_parent->logMethod(CLASS_NAME, "drawItems", true);    
    
    if(m_printPainters.contains(painter)) {
        printItems(painter, cRect, cMaps, pf);
        m_parent->logMethod(CLASS_NAME, "drawItems", false);
        return;
    }
    
    // Adjust drawing rectangle and scale maps for drawing into a cached image.
    QRect rect = cRect;
    if(cRect == canvas()->contentsRect()) rect = canvasDrawRect();
    
    QwtScaleMap maps[axisCnt];
    for(int i = 0; i < axisCnt; i++) {
        maps[i] = cMaps[i];
        if(i == xBottom || i == xTop) maps[i].setPaintInterval(0,rect.width());
        else maps[i].setPaintInterval(0, rect.height());
    }
    
    PlotLoggerPtr infoLog = m_parent->logger();
    
    // We can used a cached image if:
    // 1) none of the items have changed, which is checked via the
    //    m_changedLayers member;
    // 2) the axes match (both ranges and which axes have items attached to
    //    them), which is taken care of by the QPAxesCache::Key class; and
    // 3) the cached image has the same size as the drawing rect.
    QPAxesCache& axesCache = m_parent->axesCache();
    bool cacheAvailable = axesCache.currHasImage() &&               // check #2
        axesCache.currImageSize().width() >= rect.size().width() && // check #3
        axesCache.currImageSize().height() >= rect.size().height();
    
    // See which layers need to be drawn.
    QHash<PlotCanvasLayer, bool> draw;
    bool anyDraw = false;
    foreach(PlotCanvasLayer layer, m_layers.keys()) {
        // check #1
        draw[layer] = !cacheAvailable || changedLayer(layer) ||
                      (m_layers.value(layer)->hasItemsToDraw() &&
                      !axesCache.currHasImage(layer));
        anyDraw |= draw[layer];
    }
    
    // If drawing isn't needed, just draw the cache(s) as needed.
    if(m_drawingHeld || m_isPrinting || !anyDraw) {
        if(!infoLog.null()) {
            stringstream ss;
            ss << "Drawing caches";
            if(m_drawingHeld || m_isPrinting) {
               ss << " because:";
               if(m_drawingHeld) ss << " drawing is held";
               if(m_isPrinting) {
                   if(m_drawingHeld) ss << ",";
                   ss << " canvas is printing";
               }
            }
            ss << '.';
            infoLog->postMessage(CLASS_NAME, "drawItems", ss.str(),
                                 PlotLogger::MSG_DEBUG);
        }
        
        m_layerBase.drawItems(painter, cRect, cMaps);
            
        QPImageCache image;
        foreach(PlotCanvasLayer layer, m_layers.keys()) {
            image = axesCache.currImage(layer);
            image.paint(painter, rect);
        }

        m_parent->logMethod(CLASS_NAME, "drawItems", false);
        return;
    }
    
    // We want to clear the cache if:
    // 1) the cached image size is larger from our draw rect (indicating a
    //    resize), or
    // 2) one or more items have changed, in which case just the relevant
    //    layer caches need to be cleared.
    bool clearCache = axesCache.size() > 0 && // check #1
                      axesCache.currImageSize() != rect.size();
    bool resize = false;

    if(clearCache) {
        // Only clear cache if new size is larger than cache size.  Shrinking
        // images won't lose data.
        QSize cacheSize = axesCache.currImageSize(), rectSize = rect.size();
        clearCache = cacheSize.isValid() &&
                     (rectSize.width() > cacheSize.width() ||
                     rectSize.height() > cacheSize.height());
        
        // Make sure the set fixed size is >= the current draw area.  Otherwise
        // we'll end up in an infinite loop.
        cacheSize = axesCache.fixedImageSize();
        if(cacheSize.isValid())
            resize = cacheSize.width() < rect.width() ||
                     cacheSize.height() < rect.height();
    }
     
    if(clearCache) {
        if(!infoLog.null())
            infoLog->postMessage(CLASS_NAME, "drawItems", "Clearing cache "
                    "because canvas size has changed.", PlotLogger::MSG_DEBUG);
        axesCache.clear();
        
        if(resize) axesCache.setFixedSize(QSize());
    } else if(anyChangedLayer()) { // check #2
        if(!infoLog.null())
            infoLog->postMessage(CLASS_NAME, "drawItems", "Clearing cache of "
                    "changed layers.", PlotLogger::MSG_DEBUG);
        // clear out just those layers that have changed
        axesCache.clearLayers(changedLayersFlag());
    }
    
    // Check if a drawing thread is currently underway.
    if(m_drawThread != NULL) {
        if(!infoLog.null())
            infoLog->postMessage(CLASS_NAME, "drawItems", "A draw thread is "
                    "currently running.  Canceling it and restarting draw.",
                    PlotLogger::MSG_DEBUG);
        
        const_cast<bool&>(m_redrawWaiting) = true;
        
        m_layerBase.drawItems(painter, cRect, cMaps);
        foreach(QPCanvasLayer* layer, m_layers)
            layer->drawCache(painter, rect);
        
        const_cast<QPDrawThread*&>(m_drawThread)->cancel();
        m_parent->logMethod(CLASS_NAME, "drawItems", false);
        return;
    } else const_cast<bool&>(m_redrawWaiting) = false;
        
    // Set up operation.
    PlotOperationPtr op = m_parent->operationDraw();
    if(!op.null()) op->reset();
    
    int drawLayers = 0;
    foreach(PlotCanvasLayer layer, m_layers.keys())
        if(draw[layer]) drawLayers |= layer;
    
    // Make sure there's actually something to draw.
    anyDraw = false;
    foreach(PlotCanvasLayer layer, m_layers.keys()) {
        if(draw[layer]) {
            anyDraw |= m_layers.value(layer)->hasItemsToDraw();
            if(anyDraw) break;
        }
    }
    
    // Notify draw watchers as needed.
    if(anyDraw) {
        bool res = m_parent->notifyDrawWatchers(op, true, drawLayers);
        
        // The draw watchers told us not to continue, so assume they know what
        // they're doing.
        if(!res) {
            m_layerBase.drawItems(painter, cRect, cMaps);
            foreach(QPCanvasLayer* layer, m_layers)
                layer->drawCache(painter, rect);
            if(!op.null()) op->finish();
            m_parent->logMethod(CLASS_NAME, "drawItems", false);
            return;
        }
    }
    
    if(!op.null()) op->setInProgress(true);
    
    // Set up drawing thread.
    QPDrawThread* drawThread = NULL;
    if(anyDraw) {
        QSize size = axesCache.fixedImageSize();
        if(!size.isValid()) size = rect.size();
        drawThread = new QPDrawThread(allLayerItems(drawLayers), maps, size);
        connect(drawThread, SIGNAL(finished()), SLOT(itemDrawingFinished()));
        connect(drawThread, SIGNAL(terminated()), SLOT(itemDrawingFinished()));
        const_cast<QPDrawThread*&>(m_drawThread) = drawThread;
    }
    
    // Start logging.
    PlotLoggerPtr log;
    if(m_parent != NULL) log= m_parent->logger();
    if(!log.null() && anyDraw)
        log->markMeasurement(QPCanvas::CLASS_NAME, QPCanvas::DRAW_NAME,
                             PlotLogger::DRAW_TOTAL);
            
    // Start drawing thread if needed.
    if(anyDraw) {
        if(!infoLog.null()) {
            String layers;
            foreach(PlotCanvasLayer layer, draw.keys()) {
                if(draw[layer]) {
                    if(layers.length() > 0) layers += ", ";
                    layers += String::toString((int)layer);
                }
            }

            infoLog->postMessage(CLASS_NAME, "drawItems", "Starting draw "
                    "thread for layer(s): " + layers + ".",
                    PlotLogger::MSG_DEBUG);
        }
        
        drawThread->start();
    } else {
        // Finish operation.
        PlotOperationPtr op;
        if(m_parent != NULL) op = m_parent->operationDraw();
        if(!op.null()) op->finish();
        
        // Update and draw caches.
        m_layerBase.drawItems(painter, cRect, cMaps);
        foreach(PlotCanvasLayer layer, m_layers.keys()) {
            m_layers.value(layer)->cachedImage() = axesCache.currImage(layer);
            m_layers.value(layer)->drawCache(painter, rect);
        }
    }
    m_parent->logMethod(CLASS_NAME, "drawItems", false);
}

Bool QPLayeredCanvas::isDrawing( bool scripting ){
	bool canvasDrawing = false;
	if ( scripting ){
		//Terminate the draw thread sooner rather than later if it is done.
		if ( m_drawThread != NULL ){
			if ( m_drawThread->isFinished() && !m_drawThread->isRunning()){
				itemDrawingFinished();
			}
		}
		bool threadRunning = (m_drawThread != NULL );
		bool redrawing = (m_drawThread == NULL) && m_redrawWaiting;
		canvasDrawing = ( threadRunning || redrawing );
	}
	else {
		canvasDrawing = m_drawThread != NULL && (m_drawThread->isRunning() ||  !m_drawThread->isFinished());
	}
	return canvasDrawing;
}

void QPLayeredCanvas::printItems(QPainter* painter, const QRect& rect,
        const QwtScaleMap maps[axisCnt], const QwtPlotPrintFilter& /*pf*/) const {
    m_parent->logMethod(CLASS_NAME, "printItems", true);
    
    // Determine total draw segments.
    unsigned int bseg = m_layerBase.itemDrawSegments();
    unsigned int nseg = bseg;
    QList<QPCanvasLayer*> layers = m_layers.values();
    QVector<unsigned int> segments(layers.size());
    for(int i = 0; i < layers.size(); i++) {
        segments[i] = layers[i]->itemDrawSegments();
        nseg += segments[i];
    }
    bool drawLegend = m_parent->m_legend->legendShown() &&
                      m_parent->m_legend->isInternal();
    if(drawLegend) nseg++;
    
    // Draw layers.
    PlotOperationPtr op = m_parent->operationExport();
    m_layerBase.drawItems(painter, rect, maps, op, 0, nseg);
    unsigned int index = bseg;
    for(int i = 0; i < layers.size(); i++) {
        layers[i]->drawItems(painter, rect, maps, op, index, nseg);
        index += segments[i];
    }
    
    // Draw internal legend if needed.
    if(drawLegend) {
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
    
    m_parent->logMethod(CLASS_NAME, "printItems", false);
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

void QPLayeredCanvas::setLayerChanged(PlotCanvasLayer layer) {
    m_changedLayers.insert(layer, true); }
void QPLayeredCanvas::setLayersChanged(int layersFlag) {
    foreach(PlotCanvasLayer layer, m_changedLayers.keys())
        if(layersFlag & layer) m_changedLayers.insert(layer, true);
}

bool QPLayeredCanvas::changedLayer(PlotCanvasLayer layer) const {
    return m_changedLayers.contains(layer) && m_changedLayers.value(layer); }

bool QPLayeredCanvas::anyChangedLayer() const {
    foreach(PlotCanvasLayer layer, m_changedLayers.keys())
        if(m_changedLayers.value(layer)) return true;
    return false;
}

int QPLayeredCanvas::changedLayersFlag() const {
    int flag = 0;
    foreach(PlotCanvasLayer layer, m_changedLayers.keys())
        if(m_changedLayers.value(layer)) flag |= layer;
    return flag;
}

const QPGrid& QPLayeredCanvas::grid() const { return m_grid; }
QPGrid& QPLayeredCanvas::grid() { return m_grid; }

const QHash<PlotAxis, QPCartesianAxis*>& QPLayeredCanvas::cartesianAxes()const{
    return m_cartAxes; }
QHash<PlotAxis, QPCartesianAxis*>& QPLayeredCanvas::cartesianAxes() {
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
    vector<PlotCanvasLayer> layers = PlotCanvas::allLayers();
    for(unsigned int i = 0; i < layers.size(); i++) {
        m_layers[layers[i]] = new QPCanvasLayer();
        m_changedLayers[layers[i]] = false;
    }
    
    m_drawingHeld = false;
    m_legendFrame = NULL;
    m_drawThread = NULL;
    m_redrawWaiting = false;
    m_isPrinting = false;
    
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
}

// Private Slots //

void QPLayeredCanvas::itemDrawingFinished() {
    m_parent->logMethod(CLASS_NAME, "itemDrawingFinished", true);

    if(m_drawThread == NULL) {
        m_parent->logMethod(CLASS_NAME, "itemDrawingFinished", false);
        return;
    }
        
    if(m_drawThread->isRunning() || !m_drawThread->isFinished()) {
        m_drawThread->cancel();
        m_parent->logMethod(CLASS_NAME, "itemDrawingFinished", false);
        return;
    }
        
    // Update caches.
    QPImageCache result;
    foreach(PlotCanvasLayer layer, m_layers.keys()) {
        result = m_drawThread->drawResult(layer);
        if(!result.isNull()) m_parent->axesCache().addCurrImage(layer, result);
        m_layers.value(layer)->cachedImage() = m_parent->axesCache().currImage(
                                               layer);
    }
    
    // Cleanup.
    delete m_drawThread;
    m_drawThread = NULL;
    
    // Finish logging.
    PlotLoggerPtr log;
    if(m_parent != NULL) log = m_parent->logger();
    if(!log.null()) {
    	log->releaseMeasurement();
    }

    // Finish operation.
    PlotOperationPtr op;
    if(m_parent != NULL) op = m_parent->operationDraw();
    if(!op.null()) op->finish();
    
    // Update layer changed flags, and draw pixmaps on widget (or start new
    // draw if redraw waiting flag is on).
    foreach(PlotCanvasLayer layer, m_changedLayers.keys())
        m_changedLayers.insert(layer, m_redrawWaiting);
    replot();

    m_parent->logMethod(CLASS_NAME, "itemDrawingFinished", false);
}

}

#endif
