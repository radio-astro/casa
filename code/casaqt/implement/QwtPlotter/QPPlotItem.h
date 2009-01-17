//# QPPlotItem.h: Superclass for all plot items in qwt plotter.
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

#include <qwt_plot.h>

#include <casa/namespace.h>

namespace casa {

//# Forward Declarations
class QPCanvas;


// Subclass of PlotItem to take care of common functionality that is provided
// by QwtPlotItem.
class QPPlotItem : public virtual PlotItem {
    friend class QPCanvas;
    
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
    virtual PlotCanvas* canvas() const;
    
    // Implements PlotItem::title().
    virtual String title() const;
    
    // Implements PlotItem::setTitle().
    virtual void setTitle(const String& newTitle);
    
    // Implements PlotItem::isQWidget().
    virtual bool isQWidget() const { return false; }
    
    // Implements PlotItem::xAxis().
    virtual PlotAxis xAxis() const;
    
    // Implements PlotItem::yAxis().
    virtual PlotAxis yAxis() const;
    
    
    // ABSTRACT METHODS //
    
    // Returns a reference to the QwtPlotItem that actually draws this item.
    // <group>
    virtual QwtPlotItem& asQwtPlotItem() = 0;
    virtual const QwtPlotItem& asQwtPlotItem() const = 0;
    // </group>
    
protected:    
    // Attached canvas (or NULL for none).
    QPCanvas* m_canvas;
    
    // Which layer this item is in.
    PlotCanvasLayer m_layer;
    
    
    // Implements PlotItem::setXAxis().
    virtual void setXAxis(PlotAxis x);
    
    // Implements PlotItem::setYAxis().
    virtual void setYAxis(PlotAxis y);
    
    // Provides access to QwtPlotItem's attach and detach methods for QPCanvas.
    // <group>
    void attach(QPCanvas* canvas, PlotCanvasLayer layer);
    void detach();
    // </group>
    
    // Provides access to QPCanvas's log measurement event methods for
    // children.
    PlotLoggerPtr loggerForMeasurement(PlotLogger::MeasurementEvent ev) const;
};

// Convenience macros for children to use to log drawing measurement events.
// Assumes that the class has CLASS_NAME defined and is a child of QPPlotItem
// to use its DRAW_NAME member.
// <group>
#define QPI_DRAWLOG1                                                          \
    PlotLoggerPtr log = loggerForMeasurement(PlotLogger::DRAW_INDIVIDUAL);    \
    if(!log.null()) log->markMeasurement(CLASS_NAME, DRAW_NAME);

#define QPI_DRAWLOG1_(VAR)                                                    \
    VAR = loggerForMeasurement(PlotLogger::DRAW_INDIVIDUAL);                  \
    if(! VAR .null()) VAR ->markMeasurement(CLASS_NAME, DRAW_NAME);

#define QPI_DRAWLOG2 if(!log.null()) log->releaseMeasurement();
#define QPI_DRAWLOG2_(VAR)                                                    \
    if(! VAR .null()) VAR ->releaseMeasurement();                             \
    VAR = NULL;
// </group>

}

#endif

#endif /* QPPLOTITEM_H_ */
