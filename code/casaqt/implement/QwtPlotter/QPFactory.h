//# QPFactory.h: Qwt implementation of generic PlotFactory class.
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
#ifndef QPFACTORY_H_
#define QPFACTORY_H_

#ifdef AIPS_HAS_QWT

#include <graphics/GenericPlotter/PlotFactory.h>

#include <QApplication>

#include <casa/namespace.h>

namespace casa {

// Implementation of PlotFactory for Qwt plotter.
class QPFactory : public virtual PlotFactory {
public:
    // Constructor.
    QPFactory();
    
    // Destructor.
    ~QPFactory();
    
    
    // Import overloaded methods.    
    using PlotFactory::plotter;
    using PlotFactory::annotation;
    using PlotFactory::shapeRectangle;
    using PlotFactory::shapeEllipse;
    using PlotFactory::shapePolygon;
    using PlotFactory::shapeArrow;
    using PlotFactory::shapePath;
    using PlotFactory::point;
    using PlotFactory::color;
    using PlotFactory::font;
    using PlotFactory::line;
    using PlotFactory::symbol;
    using PlotFactory::areaFill;

    
    // Implements PlotFactory::execLoop().
    int execLoop();
    
    // Implements PlotFactory::implementation().
    Plotter::Implementation implementation() const { return Plotter::QWT; }
    
    // Implements PlotFactory::plotter().
    PlotterPtr plotter(const String& windowTitle = "Qwt Plotter",
            bool showSingleCanvas = true, bool showGUI = true,
            int logMeasurementFlags = PlotLogger::NOMEASUREMENTS,
            bool smartDelete = true) const;
    
    // Implements PlotFactory::plotter().
    PlotterPtr plotter(unsigned int nrows, unsigned int ncols,
            const String& windowTitle = "Qwt Plotter", bool showGUI = true,
            int logMeasurementFlags = PlotLogger::NOMEASUREMENTS,
            bool smartDelete = true) const;
    
    // Implements PlotFactory::canvas().
    PlotCanvasPtr canvas(bool smartDelete = true) const;
    
    // Implements PlotFactory::panel().
    PlotPanelPtr panel(bool smartDelete = true) const;
    
    // Implements PlotFactory::button().
    PlotButtonPtr button(const String& str, bool isText = true,
            bool toggleable = false, bool smartDelete = true) const;
    
    // Implements PlotFactory::checkbox().
    PlotCheckboxPtr checkbox(const String& str, bool smartDelete = true) const;


    // Implements PlotFactory::scatterPlot().
    ScatterPlotPtr scatterPlot(PlotPointDataPtr data,
            const String& title= "Scatter Plot", bool smartDelete= true) const;
    
    // Implements PlotFactory::barPlot().
    BarPlotPtr barPlot(PlotPointDataPtr data, const String& title = "Bar Plot",
            bool smartDelete = true) const;
    
    // Implements PlotFactory::rasterPlot().
    RasterPlotPtr rasterPlot(PlotRasterDataPtr data,
            const String& title = "Raster Plot",
            PlotRasterData::Format format = PlotRasterData::RGB32,
            bool smartDelete = true) const;

    
    // Implements PlotFactory::annotation().
    PlotAnnotationPtr annotation(const String& text,
            const PlotCoordinate& coord, bool smartDelete = true) const;
    
    // Implements PlotFactory::shapeRectangle().
    PlotShapeRectanglePtr shapeRectangle(const PlotCoordinate& upperLeft,
            const PlotCoordinate& lowerRight, bool smartDelete = true) const;
    
    // Implements PlotFactory::shapeEllipse().
    PlotShapeEllipsePtr shapeEllipse(const PlotCoordinate& center,
            const PlotCoordinate& radii, bool smartDelete = true) const;
    
    // Implements PlotFactory::shapePolygon().
    PlotShapePolygonPtr shapePolygon(const vector<PlotCoordinate>& c,
            bool smartDelete = true) const;
    
    // Implements PlotFactory::shapeLine().
    PlotShapeLinePtr shapeLine(double location, PlotAxis axis,
            bool smartDelete = true) const;
    
    // Implements PlotFactory::shapeArrow().
    PlotShapeArrowPtr shapeArrow(const PlotCoordinate& from,
            const PlotCoordinate& to, PlotShapeArrow::Style fromStyle =
            PlotShapeArrow::NOARROW, PlotShapeArrow::Style toStyle =
            PlotShapeArrow::V_ARROW, bool smartDelete = true) const;
    
    // Implements PlotFactory::shapePath().
    PlotShapePathPtr shapePath(const vector<PlotCoordinate>& coords,
            bool smartDelete = true) const;
    
    // Implements PlotFactory::shapeArc().
    PlotShapeArcPtr shapeArc(const PlotCoordinate& start,
            const PlotCoordinate& widthHeight, int startAngle,
            int spanAngle, bool smartDelete = true) const;
    
    // Implements PlotFactory::point().
    PlotPointPtr point(const PlotCoordinate& coord,
            bool smartDelete = true) const;

    
    // Implements PlotFactory::color().
    // <group>
    PlotColorPtr color(const String& color, bool smartDelete = true) const;
    PlotColorPtr color(const PlotColor& copy, bool smartDelete = true) const;
    // </group>
    
    // Implements PlotFactory::allNamedColors().
    vector<String> allNamedColors() const;
    
    // Implements PlotFactory::font().
    // <group>
    PlotFontPtr font(const String& family = "Arial", double pointSize = 12.0,
            const String& color = "000000", bool bold = false,
            bool italics = false, bool underline = false,
            bool smartDelete = true) const;
    PlotFontPtr font(const PlotFont& copy, bool smartDelete = true) const;
    // </group>
    
    // Implements PlotFactory::areaFill().
    // <group>
    PlotAreaFillPtr areaFill(const String& color,
            PlotAreaFill::Pattern pattern = PlotAreaFill::NOFILL,
            bool smartDelete = true) const;    
    PlotAreaFillPtr areaFill(const PlotAreaFill& copy,
            bool smartDelete = true) const;
    // </group>
    
    // Implements PlotFactory::line().
    // <group>
    PlotLinePtr line(const String& color,
            PlotLine::Style style = PlotLine::SOLID, double width = 1.0,
            bool smartDelete = true) const;
    PlotLinePtr line(const PlotLine& copy, bool smartDelete = true) const;
    // </group>
    
    // Implements PlotFactory::symbol().
    // <group>
    PlotSymbolPtr symbol(PlotSymbol::Symbol style,
            bool smartDelete = true) const;
    PlotSymbolPtr symbol(const PlotSymbol& copy, bool smartDelete= true) const;
    // </group>


    // Overrides PlotFactory tool methods.
    // <group>
    PlotSelectToolPtr selectTool(bool smartDelete = true) const;
    PlotZoomToolPtr zoomTool(bool smartDelete = true) const;
    PlotPanToolPtr panTool(bool smartDelete = true) const;
    PlotTrackerToolPtr trackerTool(bool smartDelete = true) const;
    
    PlotSelectToolPtr selectTool(PlotAxis xAxis, PlotAxis yAxis,
            PlotCoordinate::System system, bool smartDelete = true) const;
    PlotZoomToolPtr zoomTool(PlotAxis xAxis, PlotAxis yAxis,
            PlotCoordinate::System system, bool smartDelete = true) const;
    PlotPanToolPtr panTool(PlotAxis xAxis, PlotAxis yAxis,
            PlotCoordinate::System system, bool smartDelete = true) const;
    PlotTrackerToolPtr trackerTool(PlotAxis xAxis, PlotAxis yAxis,
            PlotCoordinate::System system, bool smartDelete = true) const;
    // </group>
    
    
    // Implements PlotFactory::mutex().
    PlotMutexPtr mutex(bool smartDelete = true) const;
    
    
    // Defaults //
    
    // Returns a default font for annotations.  Currently: 14-point black
    // Arial.
    static PlotFontPtr defaultAnnotationFont(bool smartDelete = true);
    
    // Returns a default area fill for bar plots.  Currently: blue filled.
    static PlotAreaFillPtr defaultBarPlotAreaFill(bool smartDelete = true);
    
    // Returns a default line for legends.  Currently: 1px black solid.
    static PlotLinePtr defaultLegendLine(bool smartDelete = true);
    
    // Returns a default area fill for legends.  Currently: white filled.
    static PlotAreaFillPtr defaultLegendAreaFill(bool smartDelete = true);
    
    // Returns a default line for shapes.  Currently: 1px black solid.
    static PlotLinePtr defaultShapeLine(bool smartDelete = true);
    
    // Returns a default area fill for shapes.  Currently: no fill.
    static PlotAreaFillPtr defaultShapeAreaFill(bool smartDelete = true);
    
    // Returns a default line for scatter plots.  Currently: no line.
    static PlotLinePtr defaultPlotLine(bool smartDelete = true);

    // Returns a default symbol for scatter plots.  Currently: 8x8 blue
    // circle with no outline.
    static PlotSymbolPtr defaultPlotSymbol(bool smartDelete = true);
    
    // The default error bar cap, currently 10.
    static const unsigned int DEFAULT_ERROR_CAP;
    
private:
    int argc; char** argv;
    QApplication* app;
    bool createdQApp;
};

}

#endif

#endif /*QPFACTORY_H_*/
