//# QPLayeredCanvas.qo.h: Subclass of QwtPlot to add layers and other features.
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
#ifndef QPLAYEREDCANVAS_QO_H_
#define QPLAYEREDCANVAS_QO_H_

#ifdef AIPS_HAS_QWT

#include <casaqt/QwtPlotter/QPCanvasHelpers.qo.h>
#include <casaqt/QwtPlotter/QPPlotItem.qo.h>

#include <qwt_plot.h>

#include <QPicture>

#include <casa/namespace.h>

namespace casa {

//# Forward declarations
class QPCanvas;


// Abstract class for a single cached layer.
class QPLayer {
public:
    // Constructor.
    QPLayer();
    
    // Destructor.
    virtual ~QPLayer();
    
    
    // Clears the cache image.
    virtual void clearImage() const;
    
    // Initializes the cache image to the given size using the given color for
    // a fill base (see QPImageCache::fill()).
    virtual void initializeImage(const QSize& size,
            unsigned int fillValue = Qt::transparent) const;
    
    // Returns whether this layer has at least one item for which
    // QPLayerItem::shouldDraw() returns true.
    virtual bool hasItemsToDraw() const;
    
    // Returns the draw count for items on this layer (see
    // QPLayerItem::drawCount()).
    virtual unsigned int itemDrawCount() const;
    
    // Returns the number of draw segments for items on this layer, using the
    // given segment threshold.
    virtual unsigned int itemDrawSegments(unsigned int threshold =
            QPDrawThread::DEFAULT_SEGMENT_THRESHOLD) const;
    
    // Draws the attached items using the given painter, canvas rect, and scale
    // maps.  If PlotOperation parameters are given, they are updated as
    // needed.
    virtual void drawItems(QPainter* painter, const QRect& canvasRect,
            const QwtScaleMap maps[QwtPlot::axisCnt],
            PlotOperationPtr op = PlotOperationPtr(),
            unsigned int currentSegment = 0,
            unsigned int totalSegments = 0,
            unsigned int segmentThreshold =
                QPDrawThread::DEFAULT_SEGMENT_THRESHOLD) const;

    // Draws the attached layer items into the image cache, using the given
    // canvas rect and scale maps.  If PlotOperation parametesr are given, they
    // are updated as needed.
    virtual void cacheItems(const QRect& canvasRect,
            const QwtScaleMap maps[QwtPlot::axisCnt],
            PlotOperationPtr op = PlotOperationPtr(),
            unsigned int currentSegment = 0,
            unsigned int totalSegments = 0,
            unsigned int segmentThreshold =
                QPDrawThread::DEFAULT_SEGMENT_THRESHOLD) const;
    
    // Draws the cached image using the given painter and draw rect.
    virtual void drawCache(QPainter* painter, const QRect& rect) const;
    
    // Returns the cached image.
    // <group>
    QPImageCache& cachedImage();
    const QPImageCache& cachedImage() const;
    // </group>
    
    
    // ABSTRACT METHODS //
    
    // Returns all attached layer items.
    // <group>
    virtual QList<QPLayerItem*> items() = 0;
    virtual QList<const QPLayerItem*> items() const = 0;
    // </group>
    
protected:
    // Cached image.
    QPImageCache m_cachedImage;
};


// Subclass of QPLayer for QPPlotItems.
class QPCanvasLayer : public QPLayer {
public:
    // Constructor.
    QPCanvasLayer();
    
    // Destructor.
    ~QPCanvasLayer();

    
    // Adds/Removes the given QPPlotItem.
    // <group>
    void addItem(QPPlotItem* item);
    void removeItem(QPPlotItem* item);
    // </group>
    
    // Returns true if the given item is in this layer, false otherwise.
    bool containsItem(QPPlotItem* item);
    
    // Returns all attached canvas items.
    // <group>
    QList<QPPlotItem*> canvasItems() { return m_items; }
    QList<const QPPlotItem*> canvasItems() const;
    // </group>
    
    
    // Implements QPLayer::items().
    // <group>
    QList<QPLayerItem*> items();
    QList<const QPLayerItem*> items() const;
    // </group>
    
private:
    // Items.
    QList<QPPlotItem*> m_items;
};


// Subclass of QPLayer for QPBaseItems.
class QPBaseLayer : public QPLayer {
public:
    // Constructor.
    QPBaseLayer();
    
    // Destructor.
    ~QPBaseLayer();
    
    
    // Adds/Removes the given QPBaseItem.
    // <group>
    void addItem(QPBaseItem* item);
    void removeItem(QPBaseItem* item);
    // </group>
    
    // Returns true if the given item is in this layer, false otherwise.
    bool containsItem(QPBaseItem* item);
    
    // Returns all attached canvas items.
    // <group>
    QList<QPBaseItem*> canvasItems() { return m_items; }
    QList<const QPBaseItem*> canvasItems() const;
    // </group>
    
    
    // Implements QPLayer::items().
    // <group>
    QList<QPLayerItem*> items();
    QList<const QPLayerItem*> items() const;
    // </group>
    
private:
    // Items.
    QList<QPBaseItem*> m_items;
};


// Subclass of QwtPlot to manage layers as specified by PlotCanvas and
// PlotCanvasLayer.  Also has an additional "base" layer for things like the
// grid and Cartesian axes.  Works with QPCanvas and QPPlotItem.
class QPLayeredCanvas : public QwtPlot {
    Q_OBJECT
    
    friend class QPAxesCache;
    friend class QPCanvas;
    friend class QPAxis;
    friend class QPPlotItem;
    friend class QPBaseItem;
    friend class QPLegendHolder;
    
public:
    // Convenient access to class name.
    static const String CLASS_NAME;
    
    
    // Constructor which takes parent canvas (optional) parent widget.
    QPLayeredCanvas(QPCanvas* parent, QWidget* parentWidget = NULL);
    
    // Constructor which takes the canvas title, the parent canvas, and an
    // (optional) parent widget.
    QPLayeredCanvas(const QwtText& title, QPCanvas* parent,
                    QWidget* parentWidget = NULL);
    
    // Destructor.
    ~QPLayeredCanvas();
    
    
    // Include overloaded methods.
    using QwtPlot::print;
    using QwtPlot::replot;

    
    // Item Methods //
    
    // Returns a list of all attached QPPlotItems on this canvas.
    QList<const QPPlotItem*> allAttachedItems() const {
        return allLayerItems(PlotCanvas::allLayersFlag()); }
    
    // Returns a list of the QPPlotItems attached to this canvas in the
    // layers as indicated (an or'ed value of PlotCanvasLayer enum values).
    QList<const QPPlotItem*> allLayerItems(int layersFlag) const;
    
    // Returns the rect for drawing items.
    QRect canvasDrawRect() const;
    
    
    // Draw Methods //
    
    // Overrides QwtPlot::print() to properly handle pixmap caches.
    void print(QPainter* painter, const QRect& rect,
            const QwtPlotPrintFilter& filter = QwtPlotPrintFilter()) const;
    
    
    // Event Methods //
    
    // Overrides QObject::eventFilter().  Used to pass events to the canvas
    // when it is being covered by an interior legend.
    bool eventFilter(QObject* watched, QEvent* event);
    
protected:
    // Item Methods //
    
    // Attaches the given item to the given layer.
    void attachLayeredItem(QPPlotItem* item);
    
    // Detaches the given item.
    void detachLayeredItem(QPPlotItem* item);
   
    // Is drawing in progress?
    virtual Bool isDrawing() const;

 
    
    // Draw Methods //
    
    // Overrides QwtPlot::drawItems().
    void drawItems(QPainter* painter, const QRect& rect,
                   const QwtScaleMap maps[axisCnt],
                   const QwtPlotPrintFilter& filter) const;
    
    // Like drawItems, but doesn't do threaded/cached drawing.
    void printItems(QPainter* painter, const QRect& rect,
                   const QwtScaleMap maps[axisCnt],
                   const QwtPlotPrintFilter& filter) const;
    
    // Overrides QwtPlot::printLegend().
    void printLegend(QPainter* painter, const QRect& rect) const;
    
    // Provides access to QwtPlot::printLegend(), and also lets the legend
    // draw its outline and background if needed.
    void printLegend_(QPainter* painter, const QRect& rect) const;
    
    // Hold/Release drawing.
    // <group>
    bool drawingIsHeld() const;
    void holdDrawing();
    void releaseDrawing();
    // </group>
    
    
    // Layer Methods //
    
    // Sets whether items in the layers have changed or not.  Each layer has
    // a changed flag that is or'ed with the given value.  These flags are
    // reset to false after the next draw.
    // <group>
    void setLayerChanged(PlotCanvasLayer layer);
    void setLayersChanged(int layersFlag);
    void setAllLayersChanged() {
        setLayersChanged(PlotCanvas::allLayersFlag()); }
    // </group>
    
    // Returns whether the given layer has changed or not since the last draw.
    bool changedLayer(PlotCanvasLayer layer) const;
    
    // Returns whether any layer has changed since the last draw or not.
    bool anyChangedLayer() const;
    
    // Returns the or'ed value of PlotCanvasLayers which have changed since the
    // last draw.
    int changedLayersFlag() const;
    
    
    // Base Items Methods //
    
    // Provides access to the grid.
    // <group>
    const QPGrid& grid() const;
    QPGrid& grid();
    // </group>
    
    // Provides access to the cartesian axes.
    // <group>
    const QHash<PlotAxis, QPCartesianAxis*>& cartesianAxes() const;
    QHash<PlotAxis, QPCartesianAxis*>& cartesianAxes();
    // </group>
    
    // See PlotCanvas::cartesianAxisShown().
    bool cartesianAxisShown(PlotAxis axis) const;
    
    // See PlotCanvas::showCartesianAxis().
    void showCartesianAxis(PlotAxis mirrorAxis, PlotAxis secondaryAxis,
            bool show);
    

    // Event Methods //
    
    // Filters input events on the given frame to pass to the canvas.
    void installLegendFilter(QWidget* legendFrame);
    
private:
    // Parent QPCanvas.
    QPCanvas* m_parent;
    
    
    // Base layer.
    QPBaseLayer m_layerBase;
    
    // Canvas layers.
    QMap<PlotCanvasLayer, QPCanvasLayer*> m_layers;
    
    // Layer changed flags.
    QHash<PlotCanvasLayer, bool> m_changedLayers;
    
    
    // Flag for whether drawing is currently held or not.
    bool m_drawingHeld;
    
    // Flag for whether we're printing rather than drawing onto a widget.
    bool m_isPrinting;
    
    // Printing painters, so we can call printItem insteads of drawItems.
    QList<QPainter*> m_printPainters;
    
    // Current draw thread, or NULL for none.
    QPDrawThread* m_drawThread;
    
    // Flag for whether a redraw currently in progress should be restarted.
    bool m_redrawWaiting;
    
    
    // Canvas grid.
    QPGrid m_grid;
    
    // Cartesian axes.
    QHash<PlotAxis, QPCartesianAxis*> m_cartAxes;

    
    // Legend frame to watch for events.
    QWidget* m_legendFrame;
    
    
    // Initializes object (meant to be called from constructor).
    void initialize();
    
private slots:
    // Slot for when the current draw thread is finished.
    void itemDrawingFinished();
};

}

#endif

#endif /* QPLAYEREDCANVAS_QO_H_ */
