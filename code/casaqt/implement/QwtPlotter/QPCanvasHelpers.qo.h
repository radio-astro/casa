//# QPCanvasHelpers.qo.h: Helper classes for QPCanvas.
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
#ifndef QPCANVASHELPERS_QO_H_
#define QPCANVASHELPERS_QO_H_

#ifdef AIPS_HAS_QWT

#include <graphics/GenericPlotter/PlotOptions.h>

#include <casa/BasicSL/String.h>

#include <qwt_scale_draw.h>
#include <qwt_plot.h>
#include <qwt_plot_canvas.h>

#include <QObject>
#include <QMouseEvent>

#include <casa/namespace.h>

namespace casa {

//# Forward declarations
class QPCanvas;

// Filter to install on the QwtPlotCanvas to catch mouse move events (which
// are otherwise stolen by other filters in QwtPicker classes).
class QPMouseFilter : public QObject {
    Q_OBJECT
    
public:
    // Constructor which takes the canvas to install itself on.
    QPMouseFilter(QwtPlotCanvas* canvas);
    
    // Destructor.
    ~QPMouseFilter();
    
    // Turns mouse tracking on or off on the underlying canvas.
    void turnTracking(bool on);
    
signals:
    // This signal is emitted if tracking is turned on, and when a
    // QMouseEvent is received for a mouse move on the underlying canvas.
    void mouseMoveEvent(QMouseEvent* e);
    
protected:
    // Filter the events on the given object.  Checks only for mouse events
    // on the underlying canvas.
    bool eventFilter(QObject* obj, QEvent* ev);
    
private:
    // Canvas.
    QwtPlotCanvas* m_canvas;
};


// Subclass of QwtScaleDraw to reimplement some drawing methods in order to fix
// a weird bug when printing to PDF or PS.  This bug may be fixed when we
// switch to qwt 5.1 rather than 5.0.2, in which case this class may be
// removed.
class QPCartesianScaleDraw : public QwtScaleDraw {
public:
    // Constructor.
    QPCartesianScaleDraw();
    
    // Destructor.
    ~QPCartesianScaleDraw();
    
    // Overrides QwtScaleDraw::drawBackbone().
    void drawBackbone(QPainter* p) const;
    
    // Overrides QwtScaleDraw::drawTick().
    void drawTick(QPainter* p, double value, int len) const;
};


// Plot item for drawing cartesian axes.  See
// http://pyqwt.sourceforge.net/examples/CartesianDemo.py.html .
class QPCartesianAxis : public QwtPlotItem {
public:
    // "Master" is the one being drawn; "slave" is the one that the master
    // is attached to.
    QPCartesianAxis(QwtPlot::Axis master, QwtPlot::Axis slave);
    
    // Destructor.
    ~QPCartesianAxis();
    
    // Implements QwtPlotItem::draw().
    void draw(QPainter* painter, const QwtScaleMap& xMap,
              const QwtScaleMap& yMap, const QRect&) const;
    
private:
    // Master axis.
    QwtPlot::Axis m_axis;
    
    // Scale draw.
    QPCartesianScaleDraw m_scaleDraw;
};


// Subclass of QwtScaleDraw that converts a double in either modified julian
// seconds or modified julian days into a String representation of the date.
// The format and number of decimals for the second can be set.
class QPDateScaleDraw : public QwtScaleDraw {
public:    
    // Constructor that takes the scale type and, optionally, the number of
    // decimals and the format.
    QPDateScaleDraw(PlotAxisScale scale, unsigned int numDecimals = 3,
                    const String& format = DEFAULT_FORMAT);
    
    ~QPDateScaleDraw();
    
    // Returns the scale used.
    PlotAxisScale scale() const;
    
    // Returns the label for the given date value.
    QwtText label(double value) const;
    
private:
    // Scale.
    PlotAxisScale m_scale;
    
    // Number of decimals for the seconds.
    unsigned int m_decimals;
    
    // Format to use.
    String m_format;
    
    // Returns true if the given format is a valid date format.  A format is
    // valid if all of the format tags appear exactly once.  The tags are:
    // * %y : year
    // * %m : month
    // * %d : day of month
    // * %h : hours
    // * %n : minutes
    // * %s : seconds
    static bool formatIsValid(String format);
    
    // The default format if none or given or an invalid format is given.
    // "%y-%m-%d\n%h:%n:%s"
    static const String DEFAULT_FORMAT;
};


// Subclass of QwtPlot to have the main canvas layer (the QwtPlotCanvas), as
// well as an additional layer on top via transparent pixmaps.
class QPLayeredCanvas : public QwtPlot {
    friend class QwtPlotItem;
    friend class QPPlotItem;
    friend class QPCanvas;
    
public:
    // Constructor which takes parent canvas (optional) parent widget.
    QPLayeredCanvas(QPCanvas* parent, QWidget* parentWidget = NULL);
    
    // Constructor which takes the canvas title, the parent canvas, and an
    // (optional) parent widget.
    QPLayeredCanvas(const QwtText& title, QPCanvas* parent,
                    QWidget* parentWidget = NULL);
    
    // Destructor.
    ~QPLayeredCanvas();
    
    
    // Include overloaded methods.
    using QwtPlot::replot;
    
    
    // Overrides QObject::eventFilter().  Used to pass events to the canvas
    // when it is being covered by an interior legend.
    bool eventFilter(QObject* watched, QEvent* event);
    
    
protected:
    // Overrides QwtPlot::drawItems().
    void drawItems(QPainter* painter, const QRect& rect,
                   const QwtScaleMap maps[axisCnt],
                   const QwtPlotPrintFilter& filter) const;
    
    // Attaches the given item to the layer rather than the main canvas.
    void attachLayeredItem(QwtPlotItem* item);
    
    // Detaches the given layered item.
    void detachLayeredItem(QwtPlotItem* item);
    
    // Sets whether to redraw the layers on the next draw cycle (true), or to
    // use the cached version (false).  Should be set immediately prior to a
    // replot, and then immediately after with both set to true (for resizing,
    // etc.).
    void setDrawLayers(bool main, bool layer);
    
    // Compounds a call to setDrawLayers(drawMain, drawLayer), replot(), and
    // setDrawLayers(true, true).  For common uses.
    void replot(bool drawMain, bool drawLayer);
    
    // Filters input events on the given frame to pass to the canvas.
    void installLegendFilter(QFrame* legendFrame);
    
private:
    // Parent QPCanvas.
    QPCanvas* m_parent;
    
    // Layered plot items.
    vector<QwtPlotItem*> m_layeredItems;
    
    // Layer draw flags.
    bool m_drawMain;
    bool m_drawLayer;
    
    // Layer pixmaps.
    QPixmap m_main;
    QPixmap m_layer;
    
    // Legend frame to watch for events.
    QFrame* m_legendFrame;
    
    
    // Initializes object (meant to be called from constructor).
    void initialize();
};

}

#endif

#endif /* QPCANVASHELPERS_QO_H_ */
