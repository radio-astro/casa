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
#ifdef AIPS_HAS_QWT

#include <casaqt/QwtPlotter/QPPlotItem.h>

#include <casaqt/QwtPlotter/QPAnnotation.h>
#include <casaqt/QwtPlotter/QPBarPlot.h>
#include <casaqt/QwtPlotter/QPCanvas.qo.h>
#include <casaqt/QwtPlotter/QPRasterPlot.h>
#include <casaqt/QwtPlotter/QPScatterPlot.qo.h>
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


// Non-Static //

QPPlotItem::QPPlotItem() : m_canvas(NULL), m_layer(MAIN) { }

QPPlotItem::~QPPlotItem() { }


PlotCanvas* QPPlotItem::canvas() const { return m_canvas; }

String QPPlotItem::title() const {
    return asQwtPlotItem().title().text().toStdString(); }

void QPPlotItem::setTitle(const String& newTitle) {
    QwtPlotItem& item = asQwtPlotItem();
    item.setTitle(newTitle.c_str());
    item.setItemAttribute(QwtPlotItem::Legend, !newTitle.empty());
}

PlotAxis QPPlotItem::xAxis() const {
    return QPOptions::axis(QwtPlot::Axis(asQwtPlotItem().xAxis())); }
PlotAxis QPPlotItem::yAxis() const {
    return QPOptions::axis(QwtPlot::Axis(asQwtPlotItem().yAxis())); }

void QPPlotItem::setXAxis(PlotAxis x) {
    asQwtPlotItem().setXAxis(QPOptions::axis(x)); }
void QPPlotItem::setYAxis(PlotAxis y) {
    asQwtPlotItem().setYAxis(QPOptions::axis(y)); }


void QPPlotItem::attach(QPCanvas* canvas, PlotCanvasLayer layer) {
    if(canvas != NULL) {
        detach();
        m_canvas = canvas;
        m_layer = layer;
        if(layer == MAIN)
            asQwtPlotItem().attach(&canvas->asQwtPlot());
        else
            canvas->asQwtPlot().attachLayeredItem(&asQwtPlotItem());
    }
}

void QPPlotItem::detach() {
    if(m_canvas != NULL) {
        if(m_layer == MAIN) asQwtPlotItem().detach();
        else m_canvas->asQwtPlot().detachLayeredItem(&asQwtPlotItem());
        m_canvas = NULL;
    }
}

PlotLoggerPtr
QPPlotItem::loggerForMeasurement(PlotLogger::MeasurementEvent event) const {
    if(m_canvas == NULL) return PlotLoggerPtr();
    else return const_cast<QPCanvas*>(m_canvas)->loggerForMeasurement(event);
}

}

#endif
