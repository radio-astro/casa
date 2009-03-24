//# QPPlotItem.qo.h: Superclass for all plot items in qwt plotter.
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
#ifndef QPPLOTITEM_H_
#define QPPLOTITEM_H_

#ifdef AIPS_HAS_QWT

#include <graphics/GenericPlotter/PlotItem.h>

#include <graphics/GenericPlotter/PlotLogger.h>
#include <graphics/GenericPlotter/PlotOperation.h>

#include <QPainter>
#include <QThread>

#include <qwt_plot.h>

#include <casa/namespace.h>

namespace casa {

//# Forward Declarations
class QPCanvas;


// Abstract superclass for any layered item that will be drawn on a
// QPLayeredCanvas.
class QPLayerItem : public virtual QwtPlotItem {
    friend class QPCanvas;
    friend class QPLayeredCanvas;
    friend class QPDrawThread;
    
public:
    // Constructor.
    QPLayerItem();
    
    // Destructor.
    virtual ~QPLayerItem();
    
    
    // Implements QwtPlotItem::draw().
    virtual void draw(QPainter* p, const QwtScaleMap& xMap,
            const QwtScaleMap& yMap, const QRect& canvasRect) const;
    
    // See PlotItem::drawSegments().
    virtual unsigned int itemDrawSegments(unsigned int segmentThreshold) const;
    
    
    // ABSTRACT METHODS //
    
    // Forces subclasses to override QwtPlotItem::itemChanged() to redraw only
    // the necessary layer.
    virtual void itemChanged() = 0;
    
    // Returns true if this item should be drawn, false otherwise.  This is
    // used to avoid drawing attached items that are empty or otherwise have
    // nothing to draw.
    virtual bool shouldDraw() const = 0;
    
    // See PlotItem::drawCount().
    virtual unsigned int itemDrawCount() const = 0;
    
    // See PlotItem::title().
    virtual String itemTitle() const = 0;
    
protected:
    // Like QwtPlotItem::draw() except that the child item should only draw
    // drawCount items starting at drawIndex.  The indexing may not be
    // applicable to all layer items (i.e., some items may draw everything in
    // this call rather than segmenting).
    virtual void draw_(QPainter* p, const QwtScaleMap& xMap,
            const QwtScaleMap& yMap, const QRect& drawRect,
            unsigned int drawIndex, unsigned int drawCount) const = 0;
};


// Subclass of PlotItem to take care of common functionality that is provided
// by QwtPlotItem.
class QPPlotItem : public virtual PlotItem, public QPLayerItem {
    friend class QPCanvas;
    friend class QPDrawThread;
    
public:
    // Static //
    
    // Convenient access to "origin" name for draw method for logging.
    static const String DRAW_NAME;
    
    
    // Returns true if the given pointer is a Qwt plotter implementation,
    // false otherwise.
    static bool isQPPlotItem(const PlotItemPtr item);
    
    // If the given item is not a Qwt plotter implementation, returns a copy
    // of the proper class.  Otherwise, returns the item.  If wasCloned is
    // given, it will be set to true if the returned item is new, false
    // otherwise.
    static QPPlotItem* cloneItem(const PlotItemPtr item, bool* wasCloned=NULL);
    
    // Returns true if the two items are the same type (class), false
    // otherwise.
    static bool sameType(QPPlotItem* item1, QPPlotItem* item2);
    
    // Returns true if the given item is a Plot type (QPBarPlot, QPHistogram,
    // QPRasterPlot, or QPScatterPlot) or not.
    static bool isPlot(QPPlotItem* item);
    
    
    // Non-Static //
    
    // Constructor.
    QPPlotItem();
    
    // Destructor.
    virtual ~QPPlotItem();
    
    
    // PlotItem methods //
    
    // Implements PlotItem::canvas().
    PlotCanvas* canvas() const;
    
    // Implements PlotItem::title().
    String title() const;
    
    // Implements PlotItem::setTitle().
    void setTitle(const String& newTitle);
    
    // Implements PlotItem::isQWidget().
    virtual bool isQWidget() const { return false; }
    
    // Implements PlotItem::xAxis().
    PlotAxis xAxis() const;
    
    // Implements PlotItem::yAxis().
    PlotAxis yAxis() const;
    
    // Implements PlotItem::setXAxis().
    void setXAxis(PlotAxis x);
    
    // Implements PlotItem::setYAxis().
    void setYAxis(PlotAxis y);
    
    
    // QPLayerItem methods //
    
    // Implements QPLayerItem::itemChanged() to only redraw the canvas layer
    // this item is in.
    virtual void itemChanged();
    
    // Implements QPLayerItem::shouldDraw().
    virtual bool shouldDraw() const { return isValid(); }
    
    // Implements QPLayerItem::itemDrawCount().
    unsigned int itemDrawCount() const{ return shouldDraw()? drawCount() : 0; }
    
    // Implements QPLayerItem::itemTitle().
    String itemTitle() const { return title(); }
    
    
    // QPPlotItem methods //
    
    // Returns the layer this item is attached to.
    PlotCanvasLayer canvasLayer() const { return m_layer; }
    
    // Provides access to QwtPlotItem methods that have been overloaded.
    // <group>
    const QwtText& qwtTitle() const { return QwtPlotItem::title(); }
    void setQwtTitle(const QwtText& text) { QwtPlotItem::setTitle(text); }
    QwtPlot::Axis qwtXAxis() const{return QwtPlot::Axis(QwtPlotItem::xAxis());}
    QwtPlot::Axis qwtYAxis() const{return QwtPlot::Axis(QwtPlotItem::yAxis());}    
    void qwtAttach(QwtPlot* plot) { QwtPlotItem::attach(plot); }
    void qwtDetach() { QwtPlotItem::detach(); }
    // </group>

    
    // ABSTRACT METHODS //
    
    // Forces children to override QwtPlotItem::boundingRect().
    virtual QwtDoubleRect boundingRect() const = 0;
    
    // Forces children to override QwtPlotItem::legendItem().
    virtual QWidget* legendItem() const = 0;
    
protected:    
    // Attached canvas (or NULL for none).
    QPCanvas* m_canvas;
    
    // Which layer this item is in.
    PlotCanvasLayer m_layer;
    
    
    // Provides access to QwtPlotItem's attach and detach methods for QPCanvas.
    // <group>
    void attach(QPCanvas* canvas, PlotCanvasLayer layer);
    void detach();
    // </group>
    
    // Provides access to QPCanvas's log measurement event methods for
    // children.
    PlotLoggerPtr loggerForEvent(PlotLogger::Event event) const;
    
    // Provides access to QPCanvas's draw operation for children.
    PlotOperationPtr drawOperation() const;
    
    
    // ABSTRACT METHODS //
    
    // Returns the class name for the child, for logging purposes.
    virtual const String& className() const = 0;
};


// Thread for drawing multiple QPPlotItems into one of two QImages based on its
// canvas layer.  Once the thread is finished, the QImages can be draw into the
// canvas caches and shown on the GUI widget as needed.  The thread will emit a
// signal after each "segment" is drawn, either the whole item for small items
// or part of a large item.
class QPDrawThread : public QThread {
    Q_OBJECT
    
public:
    // Static //
    
    // Returns the default segment threshold.
    static const unsigned int DEFAULT_SEGMENT_THRESHOLD;
    
    // Returns item1->z() < item2->z().
    // <group>
    static bool itemSortByZ(const QPLayerItem* item1,const QPLayerItem* item2);
    // </group>
    
    // Draws the given items, sorted by z-order, using the given painter, rect,
    // and maps.  If PlotOperation parameters are given, they are updated as
    // needed.
    // <group>
    static void drawItem(const QPLayerItem* item, QPainter* painter,
            const QRect& rect, const QwtScaleMap maps[QwtPlot::axisCnt]);
    static void drawItem(const QPLayerItem* item, QPainter* painter,
            const QRect& rect, const QwtScaleMap maps[QwtPlot::axisCnt],
            unsigned int drawIndex, unsigned int drawCount);
    static void drawItems(const QList<const QPLayerItem*>& items,
            QPainter* painter, const QRect& rect,
            const QwtScaleMap maps[QwtPlot::axisCnt],
            PlotOperationPtr op = PlotOperationPtr(),
            unsigned int currentSegment = 0, unsigned int totalSegments = 0,
            unsigned int segmentThreshold =
                QPDrawThread::DEFAULT_SEGMENT_THRESHOLD);
    static void drawItems(const QList<const QPPlotItem*>& items,
            QPainter* painter, const QRect& rect,
            const QwtScaleMap maps[QwtPlot::axisCnt],
            PlotOperationPtr op = PlotOperationPtr(),
            unsigned int currentSegment = 0, unsigned int totalSegments = 0,
            unsigned int segmentThreshold =
                QPDrawThread::DEFAULT_SEGMENT_THRESHOLD);
    // </group>
    
    
    // Non-Static //
    
    // Constructor which takes a list of items to draw, axes maps, the drawing
    // rectangle, and an optional segment threshold.
    QPDrawThread(const QList<const QPPlotItem*>& items,
            const QwtScaleMap maps[QwtPlot::axisCnt], const QRect& drawRect,
            unsigned int segmentTreshold = DEFAULT_SEGMENT_THRESHOLD);
    
    // Destructor.
    ~QPDrawThread();
    
    // Returns the total segments that will be drawn.  This is AT LEAST the
    // number of items to be drawn.
    unsigned int totalSegments() const;
    
    // Implements QThread::run().  Draws the items into one of the two images
    // depending on their layers.  Emits segmentDrawn() after each segment is
    // finished.
    void run();
    
    // Draws the two images into the given painters, using the draw rectangle
    // given at constructor.  Should be done AFTER the thread is finished.
    void drawIntoCaches(QPainter& mainCache, QPainter& annotationCache);
    
public slots:
    // Cancels this thread.  If the thread is currently running, it will finish
    // the segment it is on and then stop.
    void cancel();
    
private:
    // Items to draw.
    QList<const QPPlotItem*> m_items;
    
    // Axes maps.
    QwtScaleMap m_axesMaps[QwtPlot::axisCnt];
    
    // Images to draw into for the main and annotation cache, respectively.
    QImage m_mainImage, m_annotationImage;
    
    // Drawing rectangle.
    QRect m_drawRect;
    
    // Maximum number of draw items per segment.
    unsigned int m_segmentThreshold;
    
    // Flag that thread checks while running, to cancel rest of draw.
    bool m_cancelFlag;
};

}

#endif

#endif /* QPPLOTITEM_H_ */
