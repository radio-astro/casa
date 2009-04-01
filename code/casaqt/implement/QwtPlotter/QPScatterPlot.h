//# QPScatterPlot.h: Qwt implementation of generic ScatterPlot class.
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
#ifndef QPSCATTERPLOT_H_
#define QPSCATTERPLOT_H_

#ifdef AIPS_HAS_QWT

#include <graphics/GenericPlotter/Plot.h>
#include <casaqt/QwtPlotter/QPOptions.h>
#include <casaqt/QwtPlotter/QPPlotItem.qo.h>

#include <qwt_plot_item.h>
#include <qwt_legend_item.h>

#include <QObject>

#include <casa/namespace.h>

namespace casa {

// Implementation of ScatterPlot, MaskedPlot, and ErrorPlot for Qwt plotter.
class QPScatterPlot : public QPPlotItem, public MaskedScatterPlot,
                      public ErrorPlot {
public:
    // Static //
    
    // Convenient access to class name (QPScatterPlot).
    const static String CLASS_NAME;
    
    
    // Non-Static //
    
    // Constructor which takes the data (and determines its type) and an
    // optional title.
    QPScatterPlot(PlotPointDataPtr data, const String& title = "Scatter Plot");
    
    // Copy constructor for generic ScatterPlot.
    QPScatterPlot(const ScatterPlot& copy);
    
    // Destructor.
    ~QPScatterPlot();
    
    
    // Include overloaded methods.
    using Plot::setLine;
    using ScatterPlot::setSymbol;
    using MaskedScatterPlot::setMaskedLine;
    using MaskedScatterPlot::setMaskedSymbol;
    using ErrorPlot::setErrorLine;
    
    
    // PlotItem Methods //
    
    // Implements PlotItem::isValid().
    bool isValid() const;
    
    
    // QPPlotItem Methods //
    
    // Overrides QPPlotItem::shouldDraw().
    bool shouldDraw() const;
    
    // Overrides QwtPlotItem::boundingRect();
    QwtDoubleRect boundingRect() const;
    
    // Overrides QwtPlotItem::legendItem().
    QWidget* legendItem() const;
    
    
    // Plot Methods //
    
    // Implements Plot::dataChanged().
    void dataChanged() { itemChanged(); }
    
    // Implements Plot::linesShown().
    bool linesShown() const;
    
    // Implements Plot::setLinesShown().
    void setLinesShown(bool linesShown = true);
    
    // Implements Plot::line().
    PlotLinePtr line() const;
    
    // Implements Plot::setLine().
    void setLine(const PlotLine& line);
    
    
    // ScatterPlot Methods //
    
    // Implements ScatterPlot::pointData().
    PlotPointDataPtr pointData() const;
    
    // Implements ScatterPlot::symbolsShown().
    bool symbolsShown() const;
    
    // Implements ScatterPlot::setSymbolsShown().
    void setSymbolsShown(bool symbolsShown = true);
    
    // Implements ScatterPlot::symbol().
    PlotSymbolPtr symbol() const;
    
    // Implements ScatterPlot::setSymbol().
    void setSymbol(const PlotSymbol& symbol);
    
    
    // MaskedScatterPlot Methods //
    
    // Implements MaskedScatterPlot::maskedData().
    PlotMaskedPointDataPtr maskedData() const;

    // Implements MaskedScatterPlot::maskedLinesShown().
    bool maskedLinesShown() const;

    // Implements MaskedScatterPlot::setMaskedLinesShown().
    void setMaskedLinesShown(bool linesShown = true);

    // Implements MaskedScatterPlot::maskedLine().
    PlotLinePtr maskedLine() const;

    // Implements MaskedScatterPlot::setMaskedLine().
    void setMaskedLine(const PlotLine& line);

    // Implements MaskedScatterPlot::maskedSymbolsShown().
    bool maskedSymbolsShown() const;

    // Implements MaskedScatterPlot::setMaskedSymbolsShown().
    void setMaskedSymbolsShown(bool symbolsShown = true);

    // Implements MaskedScatterPlot::maskedSymbol().
    PlotSymbolPtr maskedSymbol() const;

    // Implements MaskedScatterPlot::setMaskedSymbol().
    void setMaskedSymbol(const PlotSymbol& symbol);
    
    
    // ErrorPlot Methods //
    
    // Implements ErrorPlot::errorData().
    PlotErrorDataPtr errorData() const;
    
    // Implements ErrorPlot::errorLineShown().
    bool errorLineShown() const;
    
    // Implements ErrorPlot::setErrorLineShown().
    void setErrorLineShown(bool show = true);
    
    // Implements ErrorPlot::errorLine().
    PlotLinePtr errorLine() const;
    
    // Implements ErrorPlot::setErrorLine().
    void setErrorLine(const PlotLine& line);
    
    // Implements ErrorPlot::errorCapSize().
    unsigned int errorCapSize() const;
    
    // Implements ErrorPlot::setErrorCapSize().
    void setErrorCapSize(unsigned int capSize);
    
protected:
    // Implements QPPlotItem::className().
    const String& className() const { return CLASS_NAME; }
    
    // Implements QPLayerItem::draw_().
    void draw_(QPainter* painter, const QwtScaleMap& xMap,
              const QwtScaleMap& yMap, const QRect& canvasRect,
              unsigned int drawIndex, unsigned int drawCount) const;
    
private:
    // Data pointers.
    // <group>
    PlotPointDataPtr m_data;
    PlotMaskedPointDataPtr m_maskedData;
    PlotErrorDataPtr m_errorData;
    // </group>
    
    // Customization objects.
    // <group>
    QPSymbol m_symbol;
    QPLine m_line;
    QPSymbol m_maskedSymbol;
    QPLine m_maskedLine;
    QPLine m_errorLine;
    unsigned int m_errorCap;
    // </group>
};

}

#endif

#endif /*QPSCATTERPLOT_H_*/
