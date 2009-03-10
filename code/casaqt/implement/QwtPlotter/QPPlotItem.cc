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

namespace casa {

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

QPPlotItem::QPPlotItem() : m_canvas(NULL), m_layer(MAIN) { }

QPPlotItem::~QPPlotItem() { }


// Public Methods //

PlotCanvas* QPPlotItem::canvas() const { return m_canvas; }

String QPPlotItem::title() const {
    return QwtPlotItem::title().text().toStdString(); }

void QPPlotItem::setTitle(const String& newTitle) {
    QwtPlotItem::setTitle(newTitle.c_str());
    setItemAttribute(QwtPlotItem::Legend, !newTitle.empty());
}

PlotAxis QPPlotItem::xAxis() const {
    return QPOptions::axis(QwtPlot::Axis(QwtPlotItem::xAxis())); }
PlotAxis QPPlotItem::yAxis() const {
    return QPOptions::axis(QwtPlot::Axis(QwtPlotItem::yAxis())); }

void QPPlotItem::setXAxis(PlotAxis x) {
    QwtPlotItem::setXAxis(QPOptions::axis(x)); }
void QPPlotItem::setYAxis(PlotAxis y) {
    QwtPlotItem::setYAxis(QPOptions::axis(y)); }

void QPPlotItem::itemChanged() {
    if(m_canvas != NULL) {
        QPLayeredCanvas& c = m_canvas->asQwtPlot();
        bool oldMain = c.drawMain(), oldAnnotation = c.drawAnnotation();
        c.setDrawLayers(m_layer == MAIN, m_layer == ANNOTATION);
        QwtPlotItem::itemChanged();
        c.setDrawLayers(oldMain, oldAnnotation);
    }
}


// Protected Methods //

void QPPlotItem::attach(QPCanvas* canvas, PlotCanvasLayer layer) {
    if(canvas != NULL) {
        detach();
        m_canvas = canvas;
        m_layer = layer;
        if(canvas != NULL) canvas->asQwtPlot().attachLayeredItem(this);
    }
}

void QPPlotItem::detach() {
    if(m_canvas != NULL) {
        m_canvas->asQwtPlot().detachLayeredItem(this);
        m_canvas = NULL;
    }
}

PlotLoggerPtr QPPlotItem::loggerForEvent(PlotLogger::Event event) const {
    if(m_canvas == NULL) return PlotLoggerPtr();
    else return const_cast<QPCanvas*>(m_canvas)->loggerForEvent(event);
}

PlotOperationPtr QPPlotItem::drawOperation() const {
    if(m_canvas == NULL) return PlotOperationPtr();
    else return m_canvas->operationDraw();
}


//////////////////////////////
// QPDRAWTHREAD DEFINITIONS //
//////////////////////////////

// Static //

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

void QPDrawThread::drawItem(const QPPlotItem* item, QPainter* painter,
            const QRect& rect, const QwtScaleMap maps[QwtPlot::axisCnt],
            unsigned int drawIndex, unsigned int drawCount) {
    if(item == NULL || painter == NULL || drawCount == 0 ||
       !item->isVisible() || !item->shouldDraw()) return;
    
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing,
         item->testRenderHint(QwtPlotItem::RenderAntialiased));
    item->draw_(painter, maps[item->qwtXAxis()], maps[item->qwtYAxis()],
                rect, drawIndex, drawCount);
    painter->restore();
}

void QPDrawThread::drawItems(const QList<const QPLayerItem*>& items,
        QPainter* painter, const QRect& rect,
        const QwtScaleMap maps[QwtPlot::axisCnt]) {
    if(items.size() == 0 || painter == NULL) return;
    QList<const QPLayerItem*> copy(items);
    qSort(copy.begin(), copy.end(), itemSortByZ);
    for(int i = 0; i < copy.size(); i++)
        drawItem(copy[i], painter, rect, maps);
}

void QPDrawThread::drawItems(const QList<const QPPlotItem*>& items,
        QPainter* painter, const QRect& rect,
        const QwtScaleMap maps[QwtPlot::axisCnt]) {
    if(items.size() == 0 || painter == NULL) return;
    QList<const QPPlotItem*> copy(items);
    qSort(copy.begin(), copy.end(), itemSortByZ);
    for(int i = 0; i < copy.size(); i++)
        drawItem(copy[i], painter, rect, maps);
}


// Non-Static //

QPDrawThread::QPDrawThread(const QList<const QPPlotItem*>& items,
        const QwtScaleMap maps[QwtPlot::axisCnt], const QRect& drawRect,
        unsigned int segmentTreshold) : m_items(items), m_drawRect(drawRect),
        m_segmentThreshold(segmentTreshold) {
    qSort(m_items.begin(), m_items.end(), itemSortByZ);
    if(m_segmentThreshold == 0) m_segmentThreshold = DEFAULT_SEGMENT_THRESHOLD;
    
    for(int i = 0; i < QwtPlot::axisCnt; i++) m_axesMaps[i] = maps[i];
    
    bool hasMain = false, hasAnnotation = false;
    for(int i = 0; (!hasMain || !hasAnnotation) && i < m_items.size(); i++) {
        if(m_items[i]->canvasLayer() == MAIN) hasMain = true;
        else                                  hasAnnotation = true;
    }
    
    if(hasMain) {
        m_mainImage = QImage(drawRect.size(), QImage::Format_ARGB32);
        m_mainImage.fill(Qt::transparent);
    }
    
    if(hasAnnotation) {
        m_annotationImage = QImage(drawRect.size(), QImage::Format_ARGB32);
        m_annotationImage.fill(Qt::transparent);
    }
    
    m_cancelFlag = false;
}

QPDrawThread::~QPDrawThread() { }

unsigned int QPDrawThread::totalSegments() const {
    unsigned int segments = 0;
    
    unsigned int t1, t2;
    for(int i = 0; i < m_items.size(); i++) {
        t2 = m_items[i]->indexedDrawCount();
        t1 = t2 / m_segmentThreshold;
        t2 = t2 % m_segmentThreshold;
        segments += t1;
        if(t2 > 0) segments++;
    }
    
    return segments;
}

void QPDrawThread::run() {
    if(m_items.size() == 0 || m_cancelFlag) return;
    
    // Set up painters.
    QPainter mainPainter, annotationPainter;
    if(!m_mainImage.isNull()) mainPainter.begin(&m_mainImage);
    if(!m_annotationImage.isNull())annotationPainter.begin(&m_annotationImage);

    // Set up log and operation.
    PlotLoggerPtr log= m_items[0]->loggerForEvent(PlotLogger::DRAW_INDIVIDUAL);
    PlotOperationPtr op = m_items[0]->drawOperation();
    
    // Temp variables.
    const QPPlotItem* item;
    QPainter* painter;
    unsigned int drawCount;
    String title;    
    double segmentsTotal = totalSegments(), progress;
    unsigned int segmentsDrawn = 0;
    
    // Item draw loop.
    for(int i = 0; i < m_items.size(); i++) {
        // Check if the draw's been canceled.
        if(m_cancelFlag) break;
        
        // Update temp variables.
        item = m_items[i];
        painter= item->canvasLayer() == MAIN? &mainPainter: &annotationPainter;
        drawCount = item->indexedDrawCount();
        
        // Update operation.
        if(!op.null()) {
            title = item->title();
            if(title.empty()) title = item->className();
            op->setCurrentStatus("Drawing item \"" + title + "\".");
        }
        
        // Start logging.
        if(!log.null())
            log->markMeasurement(item->className(), QPPlotItem::DRAW_NAME);
        
        // Segment draw loop.
        for(unsigned int j = 0; j < drawCount; j += m_segmentThreshold) {
            // Check if the draw's been canceled.
            if(m_cancelFlag) break;
            
            // Draw segment.
            drawItem(item, painter, m_drawRect, m_axesMaps, j,
                     m_segmentThreshold);
            
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
}

void QPDrawThread::drawIntoCaches(QPainter& mainCache,
        QPainter& annotationCache) {
    if(!m_mainImage.isNull()) mainCache.drawImage(m_drawRect, m_mainImage);
    if(!m_annotationImage.isNull())
        annotationCache.drawImage(m_drawRect, m_annotationImage);
    
    // Clear up memory.
    m_mainImage = QImage();
    m_annotationImage = QImage();
}

void QPDrawThread::cancel() { m_cancelFlag = true; }

}

#endif
