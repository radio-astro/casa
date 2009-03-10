//# QPBarPlot.h: Qwt implementation of generic BarPlot class.
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
#ifndef QPBARPLOT_H_
#define QPBARPLOT_H_

#ifdef AIPS_HAS_QWT

#include <graphics/GenericPlotter/Plot.h>
#include <casaqt/QwtPlotter/QPPlotItem.qo.h>

#include <casaqt/QwtPlotter/QPData.h>
#include <casaqt/QwtPlotter/QPOptions.h>

#include <casa/namespace.h>

namespace casa {

// Implementation of BarPlot for Qwt plotter.
class QPBarPlot : public QPPlotItem, public BarPlot {
public:
    // Static //
    
    // Convenient access to class name (QPBarPlot).
    static const String CLASS_NAME;
    
    
    // Non-Static //
    
    // Constructor which takes the data and an optional title.
    QPBarPlot(PlotPointDataPtr data, const String& title = "Qwt Bar Plot");
    
    // Copy constructor for generic BarPlot.
    QPBarPlot(const BarPlot& copy);
    
    // Destructor.
    ~QPBarPlot();
    
    
    // Include overloaded methods.
    using Plot::setLine;
    using BarPlot::setAreaFill;
    
    
    // PlotItem Methods //
    
    // Implements PlotItem::isValid().
    bool isValid() const;
    
    
    // QPPlotItem Methods //
    
    // Overrides QwtPlotItem::boundingRect();
    QwtDoubleRect boundingRect() const;
    
    // Overrides QwtPlotItem::legendItem().
    QWidget* legendItem() const;
    
    
    // Plot Methods //
    
    // Implements Plot::linesShown().
    bool linesShown() const;

    // Implements Plot::setLinesShown().
    void setLinesShown(bool linesShown = true);

    // Implements Plot::line().
    PlotLinePtr line() const;

    // Implements Plot::setLine().
    void setLine(const PlotLine& line);
    
    
    // BarPlot Methods //
    
    // Implements BarPlot::pointData().
    PlotPointDataPtr pointData() const;

    // Implements BarPlot::areaIsFilled().
    bool areaIsFilled() const;
    
    // Implements BarPlot::setAreaFilled().
    void setAreaFilled(bool fill = true);
    
    // Implements BarPlot::areaFill().
    PlotAreaFillPtr areaFill() const;

    // Implements BarPlot::setAreaFill().
    void setAreaFill(const PlotAreaFill& areaFill);
    
protected:
    // Implements QPPlotItem::className().
    const String& className() const { return CLASS_NAME; }
    
    // Implements QPPlotItem::draw_().
    void draw_(QPainter* painter, const QwtScaleMap& xMap,
              const QwtScaleMap& yMap, const QRect& canvasRect,
              unsigned int drawIndex, unsigned int drawCount) const;
    
private:
    QPPointData m_data;    // Data
    double m_barWidth;     // Bar width
    QPLine m_line;         // Line
    QPAreaFill m_areaFill; // Area fill
    
    
    // Calculates the bar width from the data.  Currently only set to happen
    // once when the data is set.
    void calculateBarWidth();
};

}

#endif

#endif /*QPBARPLOT_H_*/
