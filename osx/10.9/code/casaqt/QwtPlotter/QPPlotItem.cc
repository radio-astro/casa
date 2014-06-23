//# QPPlotItem.cc: Superclass for all plot items in qwt plotter.
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

#include <casaqt/QwtPlotter/QPPlotItem.qo.h>

#include <casaqt/QwtPlotter/QPAnnotation.h>
#include <casaqt/QwtPlotter/QPBarPlot.h>
#include <casaqt/QwtPlotter/QPCanvas.qo.h>
#include <casaqt/QwtPlotter/QPRasterPlot.h>
#include <casaqt/QwtPlotter/QPScatterPlot.h>
#include <casaqt/QwtPlotter/QPShape.h>

#include <QTime>

#include <qwt_painter.h>

namespace casa {

/////////////////////////////
// QPLAYERITEM DEFINITIONS //
/////////////////////////////

QPLayerItem::QPLayerItem() { }

QPLayerItem::~QPLayerItem() { }

void QPLayerItem::draw(QPainter* p, const QwtScaleMap& xMap,
        const QwtScaleMap& yMap, const QRect& canvasRect) const {
    draw_(p, xMap, yMap, canvasRect, 0, itemDrawCount()); }

unsigned int QPLayerItem::itemDrawSegments(unsigned int segThreshold) const {
    if(!shouldDraw()) return 0;
    unsigned int count = itemDrawCount();
    unsigned int seg = count / segThreshold;
    if(count % segThreshold > 0) seg++;
    return seg;
}

////////////////////////////
// QPPLOTITEM DEFINITIONS //
////////////////////////////

// Static //

const String QPPlotItem::DRAW_NAME = "draw";

bool QPPlotItem::isQPPlotItem(const PlotItemPtr item) {
    if(item.null()) return false;
    return dynamic_cast<const QPPlotItem*>(&*item) != NULL;
}

QPPlotItem* QPPlotItem::cloneItem(const PlotItemPtr pi, bool* wasCloned) {
    if(wasCloned != NULL) *wasCloned = false;
    if(pi.null()) return NULL;
    else if(isQPPlotItem(pi)) {
        return const_cast<QPPlotItem*>(dynamic_cast<const QPPlotItem*>(&*pi));
    } else {
        if(wasCloned != NULL) *wasCloned = true;
        
        const PlotAnnotation* annotation;
        const Plot* plot;
        const PlotPoint* point;
        const PlotShape* shape;
        
        // Annotation
        if((annotation = dynamic_cast<const PlotAnnotation*>(&*pi))!= NULL) {
            return new QPAnnotation(*annotation);
            
        // Plot
        } else if((plot = dynamic_cast<const Plot*>(&*pi)) != NULL) {
            const BarPlot* bp;
            const RasterPlot* rp;
            const ScatterPlot* sp; // will also take care of MaskedScatterPlot
                                   // and ErrorPlot since they are subclasses
            
            if((bp = dynamic_cast<const BarPlot*>(plot)) != NULL)
                return new QPBarPlot(*bp);
            else if((rp = dynamic_cast<const RasterPlot*>(plot)) != NULL)
                return new QPRasterPlot(*rp);
            else if((sp = dynamic_cast<const ScatterPlot*>(plot)) != NULL)
                return new QPScatterPlot(*sp);
            else {
                if(wasCloned != NULL) *wasCloned = false;
                return NULL; // shouldn't happen
            }
            
        // Point
        } else if((point = dynamic_cast<const PlotPoint*>(&*pi)) != NULL) {
            return new QPPoint(*point);
            
        // Shape
        } else if((shape = dynamic_cast<const PlotShape*>(&*pi)) != NULL) {
            QPShape* s = QPShape::clone(*shape);
            if(s == NULL && wasCloned != NULL) *wasCloned = false;
            return s;
            
        // Unknown (shouldn't happen)
        } else {
            if(wasCloned != NULL) *wasCloned = false;
            return NULL;
        }
    }
}

bool QPPlotItem::sameType(QPPlotItem* i1, QPPlotItem* i2) {
    return (dynamic_cast<QPAnnotation*>(i1)&&dynamic_cast<QPAnnotation*>(i2))||
           (dynamic_cast<QPPoint*>(i1) && dynamic_cast<QPPoint*>(i2)) ||
           (dynamic_cast<QPShape*>(i1) && dynamic_cast<QPShape*>(i2)) ||
           (isPlot(i1) && isPlot(i2));
}

bool QPPlotItem::isPlot(QPPlotItem* item) {
    return dynamic_cast<QPBarPlot*>(item) ||
           dynamic_cast<QPRasterPlot*>(item) ||
           dynamic_cast<QPScatterPlot*>(item);
}


// Constructors/Destructors //

QPPlotItem::QPPlotItem() : m_canvas(NULL), m_loggedCreation(false),
        m_layer(MAIN) { }

QPPlotItem::~QPPlotItem() { }


// Public Methods //

PlotCanvas* QPPlotItem::canvas() const { return m_canvas; }

String QPPlotItem::title() const {
    String s = QwtPlotItem::title().text().toStdString(); 
    return s;
    
}

void QPPlotItem::setTitle(const String& newTitle) {
    if(newTitle != title()) {
        // Quick hack because we don't need to redraw the whole plot if only
        // the title changes..
        QPCanvas* c = m_canvas;
        m_canvas = NULL;
        
        QwtPlotItem::setTitle(newTitle.c_str());
        setItemAttribute(QwtPlotItem::Legend, !newTitle.empty());
        
        // Update the legend.
        m_canvas = c;
        if(m_canvas != NULL) updateLegend(m_canvas->asQwtPlot().legend());
    }
}

PlotAxis QPPlotItem::xAxis() const {
    return QPOptions::axis(QwtPlot::Axis(QwtPlotItem::xAxis())); }
PlotAxis QPPlotItem::yAxis() const {
    return QPOptions::axis(QwtPlot::Axis(QwtPlotItem::yAxis())); }

void QPPlotItem::setXAxis(PlotAxis x) {
    if(x != xAxis()) QwtPlotItem::setXAxis(QPOptions::axis(x)); }
void QPPlotItem::setYAxis(PlotAxis y) {
    if(y != yAxis()) QwtPlotItem::setYAxis(QPOptions::axis(y)); }

void QPPlotItem::itemChanged() {
    if(m_canvas != NULL) {
        m_canvas->asQwtPlot().setLayerChanged(m_layer);
        QwtPlotItem::itemChanged();
    }
}


// Protected Methods //

void QPPlotItem::attach(QPCanvas* canvas, PlotCanvasLayer layer) {
    if(canvas != NULL) {
        detach();
        m_canvas = canvas;
        m_layer = layer;
        if(canvas != NULL) {
            canvas->asQwtPlot().attachLayeredItem(this);
            if(!m_loggedCreation) {
                canvas->logObject(className(), this, true);
                m_loggedCreation = true;
            }
        }
    }
}

void QPPlotItem::detach() {
    if(m_canvas != NULL) {
        m_canvas->asQwtPlot().detachLayeredItem(this);
        m_canvas = NULL;
    }
}

PlotLoggerPtr QPPlotItem::logger() const {
    if(m_canvas == NULL) return PlotLoggerPtr();
    else                 return m_canvas->logger();
}

void QPPlotItem::logDestruction() {
    if(m_canvas != NULL) m_canvas->logObject(className(), this, false); }

void QPPlotItem::logMethod(const String& methodName, bool entering,
        const String& message) const {
    if(m_canvas != NULL)
        m_canvas->logMethod(className(), methodName, entering, message);
}

PlotOperationPtr QPPlotItem::drawOperation() const {
    if(m_canvas == NULL) return PlotOperationPtr();
    else return m_canvas->operationDraw();
}


//////////////////////////////
// QPDRAWTHREAD DEFINITIONS //
//////////////////////////////

// Static //

const String QPDrawThread::CLASS_NAME = "QPDrawThread";

const unsigned int QPDrawThread::DEFAULT_SEGMENT_THRESHOLD = 50000;

bool QPDrawThread::itemSortByZ(const QPLayerItem* item1,
        const QPLayerItem* item2) { return item1->z() < item2->z(); }

void QPDrawThread::drawItem(const QPLayerItem* item, QPainter* painter,
            const QRect& rect, const QwtScaleMap maps[QwtPlot::axisCnt]) {
    if(item == NULL || painter == NULL || !item->isVisible() ||
       !item->shouldDraw()) return;
    
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing,
         item->testRenderHint(QwtPlotItem::RenderAntialiased));
    item->draw(painter, maps[item->xAxis()], maps[item->yAxis()],
               rect);
    painter->restore();
}

void QPDrawThread::drawItem(const QPLayerItem* item, QPainter* painter,
            const QRect& rect, const QwtScaleMap maps[QwtPlot::axisCnt],
            unsigned int drawIndex, unsigned int drawCount) {
    if(item == NULL || painter == NULL || !item->isVisible() ||
       !item->shouldDraw()) return;
    
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing,
         item->testRenderHint(QwtPlotItem::RenderAntialiased));
    item->draw_(painter, maps[item->xAxis()], maps[item->yAxis()], rect,
                drawIndex, drawCount);
    painter->restore();
}

// Convenience macro for defining both drawItems methods.
#define QPT_DRAW(TYPE)                                                        \
    if(items.size() == 0 || painter == NULL) return;                          \
    QList<const TYPE *> copy(items);                                          \
    qSort(copy.begin(), copy.end(), itemSortByZ);                             \
                                                                              \
    if(!op.null() && totalSegments > 0 && segmentThreshold > 0) {             \
        unsigned int nseg;                                                    \
        double progress = currentSegment;                                     \
        for(int i = 0; i < copy.size(); i++) {                                \
            if(op->cancelRequested()) break;                                  \
            op->setCurrentStatus("Drawing item \"" + copy[i]->itemTitle() +   \
                                 "\"");                                       \
            nseg = copy[i]->itemDrawSegments(segmentThreshold);               \
            for(unsigned int j = 0; j < nseg; j++) {                          \
                if(op->cancelRequested()) break;                              \
                drawItem(copy[i], painter, rect, maps, j * segmentThreshold,  \
                         segmentThreshold);                                   \
                progress++;                                                   \
                op->setCurrentProgress((unsigned int)                         \
                        (((progress / totalSegments) * 100) + 0.5));          \
            }                                                                 \
        }                                                                     \
    } else {                                                                  \
        for(int i = 0; i < copy.size(); i++)                                  \
            drawItem(copy[i], painter, rect, maps);                           \
    }

void QPDrawThread::drawItems(const QList<const QPLayerItem*>& items,
        QPainter* painter, const QRect& rect,
        const QwtScaleMap maps[QwtPlot::axisCnt], PlotOperationPtr op,
        unsigned int currentSegment, unsigned int totalSegments,
        unsigned int segmentThreshold) {
    QPT_DRAW(QPLayerItem)
}

void QPDrawThread::drawItems(const QList<const QPPlotItem*>& items,
        QPainter* painter, const QRect& rect,
        const QwtScaleMap maps[QwtPlot::axisCnt], PlotOperationPtr op,
        unsigned int currentSegment, unsigned int totalSegments,
        unsigned int segmentThreshold) {
    QPT_DRAW(QPPlotItem)
}


// Non-Static //

QPDrawThread::QPDrawThread(const QList<const QPPlotItem*>& items,
        const QwtScaleMap maps[QwtPlot::axisCnt], QSize imageSize,
        unsigned int segmentTreshold) : m_items(items),
        m_segmentThreshold(segmentTreshold) {
    qSort(m_items.begin(), m_items.end(), itemSortByZ);
    if(m_segmentThreshold == 0) m_segmentThreshold = DEFAULT_SEGMENT_THRESHOLD;
    
    for(int i = 0; i < QwtPlot::axisCnt; i++) m_axesMaps[i] = maps[i];
    
    QSet<PlotCanvasLayer> seenLayers;
    for(int i = 0; i < m_items.size(); i++)
        seenLayers.insert(m_items[i]->canvasLayer());
    
    QPImageCache* image;
    foreach(PlotCanvasLayer layer, seenLayers) {
        image = new QPImageCache(imageSize);
        image->fill(Qt::transparent);
        m_images.insert(layer, image);
    }
    
    m_cancelFlag = false;
    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());
    id = qrand() % 100;
}

QPDrawThread::~QPDrawThread() {
    foreach(QPImageCache* image, m_images) delete image; }

unsigned int QPDrawThread::totalSegments() const {
    unsigned int segments = 0;
    for(int i = 0; i < m_items.size(); i++)
        segments += m_items[i]->drawSegments(m_segmentThreshold);    
    return segments;
}

void QPDrawThread::run() {
    // Compile vector of unique, non-null QPCanvases.
    vector<QPCanvas*> canvases;
    QPCanvas* canvas;
    bool found = false;
    for(int i = 0; i < m_items.size(); i++) {
        if(m_items[i] == NULL){
        	continue;
        }
        canvas = dynamic_cast<QPCanvas*>(m_items[i]->canvas());
        if(canvas == NULL){
        	continue;
        }
        found = false;
        for(unsigned int j = 0; !found && j < canvases.size(); j++)
            if(canvases[j] == canvas) found = true;
        if(!found) canvases.push_back(canvas);
    }
    
    for(unsigned int i = 0; i < canvases.size(); i++)
        canvases[i]->logMethod(CLASS_NAME, "run", true);
    
    if(m_items.size() == 0 || m_cancelFlag) {
        for(unsigned int i = 0; i < canvases.size(); i++)
            canvases[i]->logMethod(CLASS_NAME, "run", false);
        return;
    }
    
    // Set up painters.
    QHash<PlotCanvasLayer, QPainter*> painters;
    foreach(PlotCanvasLayer layer, m_images.keys())
        painters.insert(layer, m_images.value(layer)->painter());

    // Set up log and operation.
    PlotLoggerPtr log = m_items[0]->logger();
    PlotOperationPtr op = m_items[0]->drawOperation();
    
    // Temp variables.
    const QPPlotItem* item;
    QPainter* painter;
    unsigned int drawCount;
    String title;    
    double segmentsTotal = totalSegments(), progress;
    unsigned int segmentsDrawn = 0;
    QRect drawRect(QPoint(0, 0), QSize(0, 0));
    
    // Item draw loop.
    for(int i = 0; i < m_items.size(); i++) {
        // Check the PlotOperation to see if cancel is requested.
        if(!op.null() && op->cancelRequested()){
        	cancel();
        }
        
        // Cancel if needed.
        if(m_cancelFlag) break;
        
        // Update temp variables.
        item = m_items[i];
        painter = painters.value(item->canvasLayer());
        drawCount = item->drawCount();
        drawRect.setSize(m_images.value(item->canvasLayer())->size());
        
        // Update operation.
        if(!op.null()) {
//            title = item->title();  -- gives wrong info when shown in progress bar popup
            if (item->canvas())   {    // prevents crash when marking/subtracting regions
                title = item->canvas()->title();
                if(title.empty()) title = item->className();
                op->setCurrentStatus("Drawing item \"" + title + "\".");
            }
        }
        
        // Start logging.
        if(!log.null())
            log->markMeasurement(item->className(), QPPlotItem::DRAW_NAME,
                                 PlotLogger::DRAW_INDIVIDUAL);
        
        // Set up maps.
        m_axesMaps[QwtPlot::xBottom].setPaintInterval(0, drawRect.width());
        m_axesMaps[QwtPlot::xTop].setPaintInterval(0, drawRect.width());
        m_axesMaps[QwtPlot::yLeft].setPaintInterval(drawRect.height(), 0);
        m_axesMaps[QwtPlot::yRight].setPaintInterval(drawRect.height(), 0);
        
        // Segment draw loop.
        for(unsigned int j = 0; j < drawCount; j += m_segmentThreshold) {
            // Check the PlotOperation to see if cancel is requested.
            if(!op.null() && op->cancelRequested()){
            	cancel();
            }
            
            // Cancel if needed.
            if(m_cancelFlag) break;
            
            // Draw segment.
            drawItem(item,painter,drawRect,m_axesMaps,j,m_segmentThreshold);
            
            // Update operation.
            if(!op.null()) {
                segmentsDrawn++;
                progress = segmentsDrawn / segmentsTotal;
                op->setCurrentProgress((unsigned int)(progress * 100));
            }
        }

        // Finish logging.
        if(!log.null()) log->releaseMeasurement();
    }

    // Clean up painters.
    foreach(QPainter* painter, painters) delete painter;
    painters.clear();
    
    for(unsigned int i = 0; i < canvases.size(); i++)
        canvases[i]->logMethod(CLASS_NAME, "run", false);
}

QPImageCache QPDrawThread::drawResult(PlotCanvasLayer layer, bool clearResult) {
    QHash<PlotCanvasLayer,QPImageCache*>& i =
        const_cast<QPDrawThread*>(this)->m_images;
    
    QPImageCache ret;
    QPImageCache* image = i.value(layer);
    if(image != NULL) {
        ret = *image;
        if(clearResult) {
            delete image;
            i.remove(layer);
        }
    }
    
    return ret;
}

void QPDrawThread::cancel() { m_cancelFlag = true; }

}

#endif
