//# QPScatterPlot.qo.h: Qwt implementation of generic ScatterPlot class.
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
#ifndef QWTSCATTERPLOT_H_
#define QWTSCATTERPLOT_H_

#ifdef AIPS_HAS_QWT

#include <graphics/GenericPlotter/Plot.h>
#include <casaqt/QwtPlotter/QPOptions.h>
#include <casaqt/QwtPlotter/QPPlotItem.h>

#include <qwt_plot_item.h>
#include <qwt_legend_item.h>

#include <QObject>

#include <casa/namespace.h>

namespace casa {

//# Forward Declarations
class QPCanvas;

// Curve that can draw scatter plot data and/or masked data and/or error data.
class QPCurve : public QObject, public QwtPlotItem {
    Q_OBJECT
    
    friend class QPScatterPlot;
    
public:
    // Constructor that takes the point data, and determines which type(s) it
    // is.
    QPCurve(PlotPointDataPtr data);
    
    // Destructor.
    ~QPCurve();
    
    
    // Returns the data.
    // <group>
    PlotPointDataPtr pointData() const;
    PlotMaskedPointDataPtr maskedData() const;
    PlotErrorDataPtr errorData() const;
    // </group>
    
    // Returns true if the curve is valid, false otherwise.
    bool isValid() const;
    
    
    // QwtPlotItem Methods //
    
    // Implements QwtPlotItem::draw().
    // <group>
    void draw(QPainter* p, const QwtScaleMap& xMap, const QwtScaleMap& yMap,
              const QRect& r) const {
        draw(p, xMap, yMap, 0, -1); }    
    void draw(QPainter* p, const QwtScaleMap& xMap, const QwtScaleMap& yMap,
              int from, int to) const;
    // </group>
    
    // Overrides QwtPlotItem::boundingRect().
    QwtDoubleRect boundingRect() const;
    
    // Overrides QwtPlotItem::legendItem().
    QWidget* legendItem() const;
    
signals:
    // These signals are emitted when at the beginning and end of the draw
    // method, respectively.  Used for logging.
    // <group>
    void drawStarted();    
    void drawEnded();
    // </group>
    
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
    
    // Provides access to objects for QPScatterPlot.
    // <group>
    QPSymbol& symbol_() { return m_symbol; }
    QPLine& line_() { return m_line; }
    QPSymbol& maskedSymbol_() { return m_maskedSymbol; }
    QPLine& maskedLine_() { return m_maskedLine; }
    QPLine& errorLine_() { return m_errorLine; }
    unsigned int& errorCap_() { return m_errorCap; }
    
    const QPSymbol& symbol_() const { return m_symbol; }
    const QPLine& line_() const { return m_line; }
    const QPSymbol& maskedSymbol_() const { return m_maskedSymbol; }
    const QPLine& maskedLine_() const { return m_maskedLine; }
    const QPLine& errorLine_() const { return m_errorLine; }
    const unsigned int& errorCap_() const { return m_errorCap; }
    // </group>
};


// Implementation of ScatterPlot, MaskedPlot, and ErrorPlot for Qwt plotter.
class QPScatterPlot : public QObject, public QPPlotItem,
                      public virtual MaskedScatterPlot,
                      public virtual ErrorPlot {
    Q_OBJECT
    
public:
    // Static //
    
    // Convenient access to class name (QPScatterPlot).
    const static String CLASS_NAME;
    
    // Default cap for error lines (currently 10).
    const static unsigned int DEFAULT_ERROR_CAP;
    
    
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
    
    // Implements QPPlotItem::asQwtPlotItem().
    // <group>
    QwtPlotItem& asQwtPlotItem();
    const QwtPlotItem& asQwtPlotItem() const;
    // </group>
    
    
    // Plot Methods //
    
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
    
private:
    QPCurve m_curve;        // Curve.
    PlotLoggerPtr m_logger; // For logging curve drawing.
    
private slots:
    // Catches the curve's drawStarted() and drawEnded() signals, respectively.
    // Used for logging.
    // <group>
    void curveDrawingStarted();
    void curveDrawingEnded();
    // </group>
};

}

#endif

#endif /*QWTSCATTERPLOT_H_*/
