//# PlotFactory.h: Class to produce implementation-specific plotting objects.
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
#ifndef PLOTFACTORY_H_
#define PLOTFACTORY_H_

#include <graphics/GenericPlotter/PlotOptions.h>
#include <graphics/GenericPlotter/Plot.h>
#include <graphics/GenericPlotter/Plotter.h>
#include <graphics/GenericPlotter/PlotShape.h>
#include <graphics/GenericPlotter/PlotAnnotation.h>
#include <graphics/GenericPlotter/PlotCanvas.h>
#include <graphics/GenericPlotter/PlotData.h>
#include <graphics/GenericPlotter/PlotCanvasLayout.h>
#include <graphics/GenericPlotter/PlotTool.h>

#include <utility>

#include <casa/namespace.h>

namespace casa {

// The idea behind a plot factory is to produce classes that are useful to
// the underlying implementation without having to know what the implementation
// is.  A PlotFactory provides for creation of plot items with the given
// parameters in the given implementation.
class PlotFactory {
public:
    PlotFactory() { }
    
    virtual ~PlotFactory() { }

    
    // Top-Level GUI Methods //
    
    // Enters the GUI execution loop, if necessary, and returns the result.
    virtual int execLoop() = 0;
    
    // Return which implementation this factory is producing.
    virtual Plotter::Implementation implementation() const = 0;
    
    // Return a new instance of a Plotter for this implementation with the
    // given parameters.  The plotter's window title is set to the given.  If
    // showSingleCanvas is true, then the plotter is created with a single
    // canvas; otherwise it has no canvases.  If showGUI is true then the
    // plotter GUI will be shown after creation.  If log measurement flags are
    // given (bitwise or of PlotLogger::MeasurementEvent) they are passed to
    // the plotter.
    virtual PlotterPtr plotter(const String& windowTitle = "Plotter",
            bool showSingleCanvas = true, bool showGUI = true,
            int logMeasurementFlags = PlotLogger::NOMEASUREMENTS) const = 0;
    
    // Return a new instance of a Plotter for this implementation, with the
    // given rows and columns of canvases and parameters.  The plotter's window
    // title is set to the given.  If showGUI is true then the plotter GUI will
    // be shown after creation.  If log measurement flags are given (bitwise or
    // of PlotLogger::MeasurementEvent) they are passed to the plotter.
    virtual PlotterPtr plotter(unsigned int nrows, unsigned int ncols,
            const String& windowTitle = "Plotter", bool showGUI = true,
            int logMeasurementFlags = PlotLogger::NOMEASUREMENTS) const = 0;
    
    // Convenience method for creating a plotter with a single, given canvas.
    // The plotter's window title is set to the given.  If showGUI is true then
    // the plotter GUI will be shown after creation.  If log measurement flags
    // are given (bitwise or of PlotLogger::MeasurementEvent) they are passed
    // to the plotter.
    virtual PlotterPtr plotter(PlotCanvasPtr canvas,
            const String& windowTitle = "Plotter", bool showGUI = true,
            int logMeasurementFlags = PlotLogger::NOMEASUREMENTS) {
        PlotterPtr p= plotter(windowTitle, false, showGUI,logMeasurementFlags);
        if(!canvas.null()) p->setCanvasLayout(new PlotLayoutSingle(canvas));
        return p;
    }
    
    // Return a new instance of a PlotCanvas for this implementation.
    virtual PlotCanvasPtr canvas() const = 0;
    
    // Return a new instance of a PlotPanel for this implementation.
    virtual PlotPanelPtr panel() const = 0;
    
    // Return a new instance of a PlotButton with the given text for this
    // implementation.  If isText is true, the given string should be
    // displayed on the button as text; otherwise it is a path to an image
    // file.  If toggleable is true, then a toggleable button is returned.
    virtual PlotButtonPtr button(const String& str, bool isText = true,
                                 bool toggleable = false) const = 0;
    
    // Return a new instance of a PlotCheckbox with the given text for this
    // implementation.
    virtual PlotCheckboxPtr checkbox(const String& str) const = 0;
    
    
    // Plot Objects //
    
    // Returns a new instance of a ScatterPlot for this implementation with
    // the given PlotPointData.  IMPORTANT: the returned ScatterPlot should be
    // of the correct type to handle the given data.  For example, if the data
    // is of type PlotMaskedPointData, the returned scatter plot should be able
    // to be cast to a MaskedScatterPlot, and similarly for PlotErrorData and
    // ErrorPlot.  If the given data is both masked and error, the returned
    // plot should be able to handle both as well.
    virtual ScatterPlotPtr scatterPlot(PlotPointDataPtr data,
            const String& title = "Scatter Plot") const = 0;
    
    // Convenience method for specialized scatter plot and data classes.  Since
    // the scatterPlot method should be able to handle the different subclasses
    // of data and return something of the proper type, this should be fine.
    // <group>
    virtual MaskedScatterPlotPtr maskedPlot(PlotMaskedPointDataPtr data,
            const String& title = "Masked Plot") const {
        return scatterPlot(data, title); }
    virtual ErrorPlotPtr errorPlot(PlotErrorDataPtr data,
            const String& title = "Error Plot") const {
        return scatterPlot(data, title); }
    // </group>
    
    // Returns a new instance of a BarPlot for this implementation with the
    // given PlotPointData and optional title.
    virtual BarPlotPtr barPlot(PlotPointDataPtr data,
                               const String& title = "Bar Plot") const = 0;
    
    // Returns a new instance of a BarPlot set to use histogram data for this
    // implementation with the given PlotSinglePointData and number of bins.
    virtual BarPlotPtr histogramPlot(PlotSingleDataPtr data,
            unsigned int numBins,
            const String& title = "Histogram Plot") const {
        return barPlot(new PlotHistogramData(data, numBins), title); }
    
    // Returns a new instance of a RasterPlot for this implementation with the
    // given data and optional title and format.
    virtual RasterPlotPtr rasterPlot(PlotRasterDataPtr data,
            const String& title = "Raster Plot",
            PlotRasterData::Format format = PlotRasterData::RGB32) const = 0;
    
    // Returns a new instance of a RasterPlot for this implementation with the
    // given data and contour levels and optional title and format.
    virtual RasterPlotPtr contourPlot(PlotRasterDataPtr data,
            const vector<double>& contours,
            const String& title = "Contour Plot",
            PlotRasterData::Format format = PlotRasterData::RGB32) const {
        RasterPlotPtr p = rasterPlot(data, title, format);
        p->setContourLines(contours);
        return p;
    }
    
    // Returns a new instance of a RasterPlot for this implementation
    // interpreted as a spectrogram with the given data and optional title.
    virtual RasterPlotPtr spectrogramPlot(PlotRasterDataPtr data,
            const String& title = "Spectrogram") const {
        return rasterPlot(data, title, PlotRasterData::SPECTROGRAM);
    }
    
    // Returns a new instance of a RasterPlot for this implementation
    // interpreted as a spectrogram with the given data and contour levels and
    // optional title.
    virtual RasterPlotPtr contouredSpectrogramPlot(PlotRasterDataPtr data,
            const vector<double>& cont,
            const String& title = "Spectrogram Contours") const {
        RasterPlotPtr p = rasterPlot(data, title, PlotRasterData::SPECTROGRAM);
        p->setContourLines(cont);
        return p;
    }
    
    
    // Shape/Annotation/Point Objects //
    
    // Return a new instance of a PlotAnnotation for this implementation with
    // the given text and coordinates.
    virtual PlotAnnotationPtr annotation(const String& text,
            const PlotCoordinate& coord) const = 0;
    
    // Convenience method.
    virtual PlotAnnotationPtr annotation(const String& text, double x,
            double y) const {
        return annotation(text, PlotCoordinate(x, y, PlotCoordinate::WORLD));
    }
    
    // Return a new instance of a PlotShapeRectangle for this implementation
    // with the given coordinates.
    virtual PlotShapeRectanglePtr shapeRectangle(const PlotCoordinate& upperLeft,
            const PlotCoordinate& lowerRight) const = 0;
    
    // Convenience method.
    virtual PlotShapeRectanglePtr shapeRectangle(double left, double top,
            double right, double bottom) const {
        return shapeRectangle(PlotCoordinate(left, top, PlotCoordinate::WORLD),
                         PlotCoordinate(right, bottom, PlotCoordinate::WORLD));
    }
    
    // Return a new instance of a PlotShapeEllipse for this implementation
    // with the given coordinates and radii.
    virtual PlotShapeEllipsePtr shapeEllipse(const PlotCoordinate& center,
            const PlotCoordinate& radii) const = 0;
    
    // Convenience method.
    virtual PlotShapeEllipsePtr shapeEllipse(double x, double y,
            double xRadius, double yRadius) const {
        return shapeEllipse(PlotCoordinate(x, y, PlotCoordinate::WORLD),
                      PlotCoordinate(xRadius, yRadius, PlotCoordinate::WORLD));
    }
    
    // Return a new instance of a PlotShapePolygon for this implementation
    // with the given coordinates.
    virtual PlotShapePolygonPtr shapePolygon(
            const vector<PlotCoordinate>& coords) const = 0;
    
    // Convenience method.
    virtual PlotShapePolygonPtr shapePolygon(const vector<double>& x,
            const vector<double>& y) const {
        vector<PlotCoordinate> c(min((uInt)x.size(), (uInt)y.size()));
        for(unsigned int i = 0; i < c.size(); i++)
            c[i] = PlotCoordinate(x[i], y[i], PlotCoordinate::WORLD);
        return shapePolygon(c);
    }
    
    // Returns a new instance of a PlotShapeLine for this implementation
    // at the given location.
    virtual PlotShapeLinePtr shapeLine(double location,
                                       PlotAxis axis) const = 0;
    
    // Returns a new instance of a PlotShapeArrow for this implementation
    // at the given coordinates with the given arrow style.
    virtual PlotShapeArrowPtr shapeArrow(const PlotCoordinate& from,
            const PlotCoordinate& to, PlotShapeArrow::Style fromArrow =
            PlotShapeArrow::NOARROW, PlotShapeArrow::Style toArrow =
            PlotShapeArrow::V_ARROW) const = 0;
    
    // Convenience methods.
    // <group>
    virtual PlotShapeArrowPtr shapeArrow(double fromX, double fromY,
            double toX, double toY, PlotShapeArrow::Style fromArrow =
            PlotShapeArrow::NOARROW, PlotShapeArrow::Style toArrow =
            PlotShapeArrow::V_ARROW) const {
        return shapeArrow(PlotCoordinate(fromX, fromY, PlotCoordinate::WORLD),
                          PlotCoordinate(toX, toY, PlotCoordinate::WORLD),
                          fromArrow, toArrow);
    }
    virtual PlotShapeArrowPtr shapeLineSegment(const PlotCoordinate& from,
            const PlotCoordinate& to) const {
        return shapeArrow(from, to, PlotShapeArrow::NOARROW,
                          PlotShapeArrow::NOARROW);
    }
    virtual PlotShapeArrowPtr shapeLineSegment(double fromX, double fromY,
            double toX, double toY) const {
        return shapeArrow(PlotCoordinate(fromX, fromY, PlotCoordinate::WORLD),
                          PlotCoordinate(toX, toY, PlotCoordinate::WORLD),
                          PlotShapeArrow::NOARROW, PlotShapeArrow::NOARROW);
    }
    // </group>
    
    // Returns a new instance of a PlotShapePath for this implementation
    // with the given coordinates.
    virtual PlotShapePathPtr shapePath(
            const vector<PlotCoordinate>& coords) const = 0;
    
    // Convenience method.
    virtual PlotShapePathPtr shapePath(const vector<double>& x,
            const vector<double>& y) const {
        vector<PlotCoordinate> c(min((uInt)x.size(), (uInt)y.size()));
        for(unsigned int i = 0; i < c.size(); i++)
            c[i] = PlotCoordinate(x[i], y[i], PlotCoordinate::WORLD);
        return shapePath(c);
    }
    
    // Returns a new instance of a PlotShapeArc for this implementation
    // with the given start position, width and height, start angle, and span
    // angle.
    virtual PlotShapeArcPtr shapeArc(const PlotCoordinate& start,
            const PlotCoordinate& widthHeight, int startAngle,
            int spanAngle) const = 0;    
    
    // Returns a new instance of a PlotPoint for this implementation at the
    // given coordinates.
    virtual PlotPointPtr point(const PlotCoordinate& coord) const = 0;
    
    // Convenience methods.
    // <group>
    virtual PlotPointPtr point(double x, double y) const {
        return point(PlotCoordinate(x, y, PlotCoordinate::WORLD)); }
    virtual PlotPointPtr point(float x, float y) const {
        return point(PlotCoordinate(x, y, PlotCoordinate::WORLD)); }
    virtual PlotPointPtr point(int x, int y) const {
        return point(PlotCoordinate(x, y, PlotCoordinate::WORLD)); }
    virtual PlotPointPtr point(unsigned int x, unsigned int y) const {
        return point(PlotCoordinate(x, y, PlotCoordinate::WORLD)); }
    // </group>
    
    
    // Customization Objects //
    
    // Color could be a name (i.e. "black") or a hex value (i.e. "000000").
    virtual PlotColorPtr color(const String& color) const = 0;
    
    // Make a copy of the given color for this implementation.
    virtual PlotColorPtr color(const PlotColor& copy) const = 0;
    
    // Convenience method.
    virtual PlotColorPtr color(const PlotColorPtr copy) const {
        if(!copy.null()) return color(*copy);
        else             return copy;
    }
    
    // Returns a list of all the named colors that the implementation supports.
    virtual vector<String> allNamedColors() const = 0;
    
    // Return a new font with the given characteristics.  Color can either be
    // in hexadecimal form or name form.
    virtual PlotFontPtr font(const String& family = "Arial",
            double pointSize = 12, const String& color = "000000",
            bool bold = false, bool italics = false,
            bool underline = false) const = 0;
    
    // Make a copy of the given font for this implementation.
    virtual PlotFontPtr font(const PlotFont& copy) const = 0;
    
    // Convenience method.
    virtual PlotFontPtr font(const PlotFontPtr copy) const {
        if(!copy.null()) return font(*copy);
        else             return copy;
    }
    
    // Returns a new area fill with the given color and pattern.  Color can
    // either be in hexadecimal form or name form.
    virtual PlotAreaFillPtr areaFill(const String& color,
            PlotAreaFill::Pattern pattern = PlotAreaFill::FILL) const = 0;
    
    // Returns a copy of the given area fill for this implementation.
    virtual PlotAreaFillPtr areaFill(const PlotAreaFill& copy) const = 0;
    
    // Convenience method.
    virtual PlotAreaFillPtr areaFill(const PlotAreaFillPtr copy) const {
        if(!copy.null()) return areaFill(*copy);
        else             return copy;
    }
    
    // Returns a new line with the given color, style, and width.  Color can
    // either be in hexadecimal form or name form.
    virtual PlotLinePtr line(const String& color,
                             PlotLine::Style style = PlotLine::SOLID,
                             double width = 1.0) const = 0;
    
    // Make a copy of the given line for this implementation.
    virtual PlotLinePtr line(const PlotLine& copy) const = 0;
    
    // Convenience method.
    virtual PlotLinePtr line(const PlotLinePtr copy) const {
        if(!copy.null()) return line(*copy);
        else             return copy;
    }
    
    // Returns a new symbol with the given style.
    virtual PlotSymbolPtr symbol(PlotSymbol::Symbol style) const = 0;
    
    // Returns a new symbol with the given character.
    virtual PlotSymbolPtr symbol(char sym) const {
        PlotSymbolPtr s = symbol(PlotSymbol::CHARACTER);
        s->setSymbol(sym);
        return s;
    }
    
    // Return a new symbol with the given unicode #.
    virtual PlotSymbolPtr uSymbol(unsigned short unicode) const {
        PlotSymbolPtr s = symbol(PlotSymbol::CHARACTER);
        s->setUSymbol(unicode);
        return s;
    }
    
    // Make a copy of the given symbol for this implementation.
    virtual PlotSymbolPtr symbol(const PlotSymbol& copy) const = 0;
    
    // Convenience method.
    virtual PlotSymbolPtr symbol(const PlotSymbolPtr copy) const {
        if(!copy.null()) return symbol(*copy);
        else             return copy;
    }
    
    
    // Tool Objects //
    
    // Returns a standard mouse tool group for this implementation.
    virtual PlotStandardMouseToolGroupPtr standardMouseTools(
            PlotStandardMouseToolGroup::Tool activeTool =
            PlotStandardMouseToolGroup::NONE) {
        PlotSelectToolPtr sel = selectTool();
        sel->setSelectLine(line("black", PlotLine::SOLID, 1.0));
        sel->setRectLine(line("black", PlotLine::SOLID, 1.0));
        sel->setRectFill(areaFill("black", PlotAreaFill::MESH3));
        return new PlotStandardMouseToolGroup(sel, zoomTool(),
                   panTool(), trackerTool(), activeTool);
    }
    
    // Returns a standard mouse tool group for this implementation on the given
    // axes.
    virtual PlotStandardMouseToolGroupPtr standardMouseTools(PlotAxis xAxis,
            PlotAxis yAxis, PlotCoordinate::System sys,
            PlotStandardMouseToolGroup::Tool activeTool =
            PlotStandardMouseToolGroup::NONE) {
        PlotSelectToolPtr sel = selectTool(xAxis, yAxis, sys);
        sel->setSelectLine(line("black", PlotLine::SOLID, 1.0));
        sel->setRectLine(line("black", PlotLine::SOLID, 1.0));
        sel->setRectFill(areaFill("black", PlotAreaFill::MESH3));
        return new PlotStandardMouseToolGroup(sel,
                zoomTool(xAxis, yAxis, sys), panTool(xAxis, yAxis, sys),
                trackerTool(xAxis, yAxis, sys), activeTool);
    }
    
    // Returns tools for this implementation.  (Defaults to standard tools.)
    // <group>
    virtual PlotSelectToolPtr selectTool() { return new PlotSelectTool(); }
    virtual PlotZoomToolPtr zoomTool() { return new PlotZoomTool(); }
    virtual PlotPanToolPtr panTool() { return new PlotPanTool(); }
    virtual PlotTrackerToolPtr trackerTool() { return new PlotTrackerTool(); }
    virtual PlotSelectToolPtr selectTool(PlotAxis xAxis, PlotAxis yAxis,
            PlotCoordinate::System system) {
        return new PlotSelectTool(xAxis, yAxis, system); }
    virtual PlotZoomToolPtr zoomTool(PlotAxis xAxis, PlotAxis yAxis,
            PlotCoordinate::System system) {
        return new PlotZoomTool(xAxis, yAxis, system); }
    virtual PlotPanToolPtr panTool(PlotAxis xAxis, PlotAxis yAxis,
            PlotCoordinate::System system) {
        return new PlotPanTool(xAxis, yAxis, system); }
    virtual PlotTrackerToolPtr trackerTool(PlotAxis xAxis, PlotAxis yAxis,
            PlotCoordinate::System system) {
        return new PlotTrackerTool(xAxis, yAxis, system); }
    // </group>
    
    
    // Data Objects //
    
// Macro for different permutations of the default data objects for different
// types.
#define PFDATA(TYPE)                                                          \
    virtual PlotPointDataPtr data(TYPE *& y, unsigned int n,                  \
                                  bool shouldDelete = true) const {           \
        return new PlotPointDataImpl< TYPE >(y, n, shouldDelete); }           \
    virtual PlotPointDataPtr data(Vector< TYPE >& y,                          \
                                  bool shouldDelete = false) const {          \
        return new PlotPointDataImpl< TYPE >(y, shouldDelete); }              \
    virtual PlotPointDataPtr data(vector< TYPE >& y,                          \
                                  bool shouldDelete = false) const {          \
        return new PlotPointDataImpl< TYPE >(y, shouldDelete); }              \
    virtual PlotPointDataPtr data(TYPE *& x, TYPE *& y, unsigned int n,       \
                                  bool shouldDelete = true) const {           \
        return new PlotPointDataImpl< TYPE >(x, y, n, shouldDelete); }        \
    virtual PlotPointDataPtr data(Vector< TYPE >& x, Vector< TYPE >& y,       \
                                  bool shouldDelete = false) const {          \
        return new PlotPointDataImpl< TYPE >(x, y, shouldDelete); }           \
    virtual PlotPointDataPtr data(vector< TYPE >& x, vector< TYPE >& y,       \
                                  bool shouldDelete = false) const {          \
        return new PlotPointDataImpl< TYPE >(x, y, shouldDelete); }           \
    virtual PlotSingleDataPtr singleData(TYPE *& data, unsigned int n,        \
                                   bool shouldDelete = true) const {          \
        return new PlotSingleDataImpl< TYPE >(data, n, shouldDelete); }       \
    virtual PlotSingleDataPtr singleData(Vector< TYPE >& data,                \
                                   bool shouldDelete = false) const {         \
        return new PlotSingleDataImpl< TYPE >(data, shouldDelete); }          \
    virtual PlotSingleDataPtr singleData(vector< TYPE >& data,                \
                                   bool shouldDelete = false) const {         \
        return new PlotSingleDataImpl< TYPE >(data, shouldDelete); }          \
    virtual PlotPointDataPtr histogramData(TYPE *& data, unsigned int n,      \
            unsigned int numBins, bool shouldDel = true) const {              \
        return new PlotHistogramData(singleData(data, n, shouldDel),numBins);}\
    virtual PlotPointDataPtr histogramData(vector< TYPE >& data,              \
            unsigned int numBins, bool shouldDel = false) const {             \
        return new PlotHistogramData(singleData(data, shouldDel), numBins); } \
    virtual PlotPointDataPtr histogramData(Vector< TYPE >& data,              \
            unsigned int numBins, bool shouldDel = false) const {             \
        return new PlotHistogramData(singleData(data, shouldDel), numBins); } \
    virtual PlotMaskedPointDataPtr data(TYPE *& x, TYPE*& y, bool*& mask,     \
                                        unsigned int n,                       \
                                        bool shouldDelete = true) {           \
        return new PlotMaskedPointDataImpl< TYPE >(x, y, mask, n,             \
                                                   shouldDelete); }           \
    virtual PlotMaskedPointDataPtr data(Vector< TYPE >& x, Vector< TYPE >& y, \
                                        Vector<bool>& mask,                   \
                                        bool shouldDelete = false){           \
        return new PlotMaskedPointDataImpl< TYPE >(x, y, mask, shouldDelete);}\
    virtual PlotMaskedPointDataPtr data(vector< TYPE >& x, vector< TYPE >& y, \
                                        vector<bool>& mask,                   \
                                        bool shouldDelete = false) {          \
        return new PlotMaskedPointDataImpl< TYPE >(x, y, mask, shouldDelete);}\
    virtual PlotErrorDataPtr data(TYPE *& x, TYPE *& y, unsigned int n,       \
                                  TYPE xLeftError, TYPE xRightError,          \
                                  TYPE yBottomError, TYPE yTopError,          \
                                  bool shouldDelete = true) const {           \
        return new PlotScalarErrorDataImpl< TYPE >(x, y, n, xLeftError,       \
                xRightError, yBottomError, yTopError, shouldDelete); }        \
    virtual PlotErrorDataPtr data(Vector< TYPE >& x, Vector< TYPE >& y,       \
                                  TYPE xLeftError, TYPE xRightError,          \
                                  TYPE yBottomError, TYPE yTopError,          \
                                  bool shouldDelete = true) const {           \
        return new PlotScalarErrorDataImpl< TYPE >(x, y, xLeftError,          \
                xRightError, yBottomError, yTopError, shouldDelete); }        \
    virtual PlotErrorDataPtr data(vector< TYPE >& x, vector< TYPE >& y,       \
                                  TYPE xLeftError, TYPE xRightError,          \
                                  TYPE yBottomError, TYPE yTopError,          \
                                  bool shouldDelete = true) const {           \
        return new PlotScalarErrorDataImpl< TYPE >(x, y, xLeftError,          \
                xRightError, yBottomError, yTopError, shouldDelete); }        \
    virtual PlotErrorDataPtr data(TYPE *& x, TYPE *& y, TYPE *& xLeftError,   \
                                  TYPE *& xRightError, TYPE *& yBottomError,  \
                                  TYPE *& yTopError, unsigned int n,          \
                                  bool shouldDelete = true) const {           \
        return new PlotErrorDataImpl< TYPE >(x, y, xLeftError, xRightError,   \
                                 yBottomError, yTopError, n, shouldDelete); } \
    virtual PlotErrorDataPtr data(vector< TYPE >& x, vector< TYPE >& y,       \
                                  vector< TYPE >& xLeftError,                 \
                                  vector< TYPE >& xRightError,                \
                                  vector< TYPE >& yBottomError,               \
                                  vector< TYPE >& yTopError,                  \
                                  bool shouldDelete = false) const {          \
        return new PlotErrorDataImpl< TYPE >(x, y, xLeftError, xRightError,   \
                                 yBottomError, yTopError, shouldDelete); }    \
    virtual PlotErrorDataPtr data(Vector< TYPE >& x, Vector< TYPE >& y,       \
                                  Vector< TYPE >& xLeftError,                 \
                                  Vector< TYPE >& xRightError,                \
                                  Vector< TYPE >& yBottomError,               \
                                  Vector< TYPE >& yTopError,                  \
                                  bool shouldDelete = false) const {          \
        return new PlotErrorDataImpl< TYPE >(x, y, xLeftError, xRightError,   \
                                    yBottomError, yTopError, shouldDelete); } \
    virtual PlotRasterDataPtr data(Matrix< TYPE >& data,                      \
                                   bool shouldDelete = false) const {         \
        return new PlotRasterMatrixData< TYPE >(data, shouldDelete); }        \
    virtual PlotRasterDataPtr data(Matrix< TYPE >& data, double fromX,        \
                                   double toX, double fromY, double toY,      \
                                   bool shouldDelete = false) const {         \
        PlotRasterMatrixData< TYPE >* d = new PlotRasterMatrixData< TYPE > (  \
                                          data, shouldDelete);                \
        d->setXRange(fromX, toX);                                             \
        d->setYRange(fromY, toY);                                             \
        return d;                                                             \
    }
    
    PFDATA(double)
    PFDATA(float)
    PFDATA(int)
    PFDATA(unsigned int)
    virtual PlotPointDataPtr histogramData(PlotSingleDataPtr data,
            unsigned int numBins) const {
        return new PlotHistogramData(data, numBins); }
};
typedef CountedPtr<PlotFactory> PlotFactoryPtr;

}

#endif /*PLOTFACTORY_H_*/
