//# QPFactory.cc: Qwt implementation of generic PlotFactory class.
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

#include <casaqt/QwtPlotter/QPFactory.h>

#include <casaqt/QwtPlotter/QPAnnotation.h>
#include <casaqt/QwtPlotter/QPBarPlot.h>
#include <casaqt/QwtPlotter/QPCanvas.qo.h>
#include <casaqt/QwtPlotter/QPPanel.qo.h>
#include <casaqt/QwtPlotter/QPPlotter.qo.h>
#include <casaqt/QwtPlotter/QPRasterPlot.h>
#include <casaqt/QwtPlotter/QPScatterPlot.qo.h>
#include <casaqt/QwtPlotter/QPShape.h>
#include <casaqt/QwtPlotter/QPTool.qo.h>

namespace casa {

// TODO QPFactory: see if pixel drawing is significantly faster than circles

///////////////////////////
// QPFACTORY DEFINITIONS //
///////////////////////////

QPFactory::QPFactory() : argc(0), argv(NULL), app(NULL), createdQApp(false) {
    if(qApp == NULL) {
        app = new QApplication(argc, argv);
        createdQApp = true;
    } else app = qApp;
}

QPFactory::~QPFactory() {
    if(createdQApp) delete app;
}


int QPFactory::execLoop() {
    QApplication::processEvents();
    return app->exec();
}

PlotterPtr QPFactory::plotter(const String& windowTitle, bool showSingleCanvas,
        bool showGUI, int logMeasurementFlags) const {
    QPCanvas* canvas = showSingleCanvas ? new QPCanvas() : NULL;
    PlotterPtr plotter = new QPPlotter(canvas, logMeasurementFlags);
    plotter->setWindowTitle(windowTitle);
    plotter->showGUI(showGUI);
    return plotter;
}

PlotterPtr QPFactory::plotter(unsigned int nrows, unsigned int ncols,
        const String& windowTitle, bool showGUI,int logMeasurementFlags) const{
    if(nrows == 0 || ncols == 0)
        return plotter(windowTitle, false, showGUI, logMeasurementFlags);
    
    PlotLayoutGrid* g = new PlotLayoutGrid(nrows, ncols);
    for(unsigned int i = 0; i < nrows; i++)
        for(unsigned int j = 0; j < ncols; j++)
            g->setCanvasAt(PlotGridCoordinate(i, j), new QPCanvas());
    
    PlotterPtr plotter = new QPPlotter(g, logMeasurementFlags);
    plotter->setWindowTitle(windowTitle);
    plotter->showGUI(showGUI);
    return plotter;
}

PlotCanvasPtr QPFactory::canvas() const { return new QPCanvas(); }

PlotPanelPtr QPFactory::panel() const { return new QPPanel(); }

PlotButtonPtr QPFactory::button(const String& str, bool isText,
                                bool toggleable) const {
    if(isText) return new QPButton(str, toggleable);
    else {
        QPButton* b = new QPButton("", toggleable);
        b->setImagePath(str);
        return b;
    }
}

PlotCheckboxPtr QPFactory::checkbox(const String& str) const {
    return new QPCheckbox(str); }


ScatterPlotPtr QPFactory::scatterPlot(PlotPointDataPtr data,
        const String& title) const {
    if(data.null() || !data->isValid()) return ScatterPlotPtr();
    else return new QPScatterPlot(data, title);
}

/*
HistogramPlotPtr QPFactory::histogramPlot(PlotSingleDataPtr data,
        unsigned int numBins, const String& title) const {
    if(!data.null() && data->isValid())
        return new QPHistogram(data, numBins, title);
    else return HistogramPlotPtr();
}
*/

BarPlotPtr QPFactory::barPlot(PlotPointDataPtr data,
        const String& title) const {
    if(data.null() || !data->isValid()) return BarPlotPtr();
    else return new QPBarPlot(data, title);
}

RasterPlotPtr QPFactory::rasterPlot(PlotRasterDataPtr data,const String& title,
            PlotRasterData::Format format) const {
    if(data.null() || !data->isValid()) return RasterPlotPtr();
    else return new QPRasterPlot(data, format, title);
}


PlotAnnotationPtr QPFactory::annotation(const String& text,
        const PlotCoordinate& coord) const {
    return new QPAnnotation(text, coord); }

PlotShapeRectanglePtr QPFactory::shapeRectangle(const PlotCoordinate& ul,
        const PlotCoordinate& lr) const {
    return new QPRectangle(ul, lr); }

PlotShapeEllipsePtr QPFactory::shapeEllipse(const PlotCoordinate& center,
        const PlotCoordinate& radii) const {
    return new QPEllipse(center, radii); }

PlotShapePolygonPtr
QPFactory::shapePolygon(const vector<PlotCoordinate>& c) const {
    return new QPPolygon(c); }

PlotShapeLinePtr QPFactory::shapeLine(double location, PlotAxis axis) const {
    return new QPLineShape(location, axis); }

PlotShapeArrowPtr QPFactory::shapeArrow(const PlotCoordinate& from,
        const PlotCoordinate& to, PlotShapeArrow::Style fromStyle,
        PlotShapeArrow::Style toStyle) const {
    QPArrow* a = new QPArrow(from, to);
    a->setArrowStyles(fromStyle, toStyle);
    return a;
}

PlotShapePathPtr QPFactory::shapePath(const vector<PlotCoordinate>& c) const {
    return new QPPath(c); }

PlotShapeArcPtr QPFactory::shapeArc(const PlotCoordinate& start,
        const PlotCoordinate& wh, int startAngle, int spanAngle) const {
    return new QPArc(start, wh, startAngle, spanAngle); }

PlotPointPtr QPFactory::point(const PlotCoordinate& coord) const {
    return new QPPoint(coord); }


PlotColorPtr QPFactory::color(const String& color) const {
    return new QPColor(color); }

PlotColorPtr QPFactory::color(const PlotColor& copy) const {
    return new QPColor(copy); }

vector<String> QPFactory::allNamedColors() const {
    vector<String> v(QPPlotter::GLOBAL_COLORS.size());
    for(int i = 0; i < QPPlotter::GLOBAL_COLORS.size(); i++)
        v[i] = QPPlotter::GLOBAL_COLORS[i].toStdString();
    return v;
}

PlotFontPtr QPFactory::font(const String& family, double pointSize,
        const String& color, bool bold, bool italics, bool underline) const {
    QPFont* f = new QPFont();
    f->setFontFamily(family);
    f->setPointSize(pointSize);
    f->setColor(QPColor(color));
    f->setBold(bold);
    f->setItalics(italics);
    f->setUnderline(underline);
    return f;
}

PlotFontPtr QPFactory::font(const PlotFont& copy) const {
    return new QPFont(copy); }

PlotAreaFillPtr QPFactory::areaFill(const String& color,
        PlotAreaFill::Pattern pattern) const {
    QPAreaFill* f = new QPAreaFill();
    f->setColor(QPColor(color));
    f->setPattern(pattern);
    return f;
}

PlotAreaFillPtr QPFactory::areaFill(const PlotAreaFill& copy) const {
    return new QPAreaFill(copy); }

PlotLinePtr QPFactory::line(const String& color, PlotLine::Style style,
                            double width) const {
    QPLine* l = new QPLine();
    l->setWidth(width);
    l->setStyle(style);
    l->setColor(QPColor(color));
    return l;
}

PlotLinePtr QPFactory::line(const PlotLine& copy) const {
    return new QPLine(copy); }

PlotSymbolPtr QPFactory::symbol(PlotSymbol::Symbol style) const {
    QPSymbol* s = new QPSymbol();
    s->setSymbol(style);
    s->setSize(10, 10);
    return s;
}

PlotSymbolPtr QPFactory::symbol(char symbol) const {
    QPSymbol* s = new QPSymbol();
    s->setSymbol(symbol);
    s->setSize(20, 20);
    return s;
}

PlotSymbolPtr QPFactory::uSymbol(unsigned short unicode) const {
    QPSymbol* s = new QPSymbol();
    s->setUSymbol(unicode);
    s->setSize(20, 20);
    return s;
}

PlotSymbolPtr QPFactory::symbol(const PlotSymbol& copy) const {
    return new QPSymbol(copy); }


PlotSelectToolPtr QPFactory::selectTool() { return new QPSelectTool(); }
PlotZoomToolPtr QPFactory::zoomTool() { return new QPZoomTool(); }
PlotPanToolPtr QPFactory::panTool() { return new QPPanTool(); }
PlotTrackerToolPtr QPFactory::trackerTool() { return new QPTrackerTool(); }

PlotSelectToolPtr QPFactory::selectTool(PlotAxis xAxis, PlotAxis yAxis,
        PlotCoordinate::System system) {
    return new QPSelectTool(xAxis, yAxis, system); }
PlotZoomToolPtr QPFactory::zoomTool(PlotAxis xAxis, PlotAxis yAxis,
        PlotCoordinate::System system) {
    return new QPZoomTool(xAxis, yAxis, system); }
PlotPanToolPtr QPFactory::panTool(PlotAxis xAxis, PlotAxis yAxis,
        PlotCoordinate::System system) {
    return new QPPanTool(xAxis, yAxis, system); }
PlotTrackerToolPtr QPFactory::trackerTool(PlotAxis xAxis, PlotAxis yAxis,
        PlotCoordinate::System system) {
    return new QPTrackerTool(xAxis, yAxis, system); }


// Static //

PlotFontPtr QPFactory::defaultAnnotationFont() {
    QPFont* f = new QPFont();
    f->setFontFamily("Arial");
    f->setPointSize(14.0);
    f->setColor("000000");
    return f;
}

PlotAreaFillPtr QPFactory::defaultBarPlotAreaFill() {
    QPAreaFill* a = new QPAreaFill();
    a->setColor("0000FF");
    a->setPattern(PlotAreaFill::FILL);
    return a;
}

PlotLinePtr QPFactory::defaultLegendLine() {
    QPLine* l = new QPLine();
    l->setWidth(1.0);
    l->setColor("000000");
    l->setStyle(PlotLine::SOLID);
    return l;
}

PlotAreaFillPtr QPFactory::defaultLegendAreaFill() {
    QPAreaFill* a = new QPAreaFill();
    a->setColor("FFFFFF");
    a->setPattern(PlotAreaFill::FILL);
    return a;
}

PlotLinePtr QPFactory::defaultShapeLine() {
    QPLine* l = new QPLine();
    l->setWidth(1.0);
    l->setColor("000000");
    l->setStyle(PlotLine::SOLID);
    return l;
}

PlotAreaFillPtr QPFactory::defaultShapeAreaFill() {
    QPAreaFill* a = new QPAreaFill();
    a->setColor("0000FF");
    a->setPattern(PlotAreaFill::NOFILL);
    return a;
}

PlotLinePtr QPFactory::defaultPlotLine() {
    QPLine* l = new QPLine();
    l->setColor("000000");
    l->setWidth(1.0);
    l->setStyle(PlotLine::NOLINE);
    return l;
}

PlotSymbolPtr QPFactory::defaultPlotSymbol() {
    QPSymbol* s = new QPSymbol();
    s->setSymbol(PlotSymbol::CIRCLE);
    s->setSize(8, 8);
    s->setLine("000000", PlotLine::NOLINE, 1.0);
    s->setAreaFill("0000FF");
    return s;
}

const unsigned int QPFactory::DEFAULT_ERROR_CAP = 10;

}

#endif
