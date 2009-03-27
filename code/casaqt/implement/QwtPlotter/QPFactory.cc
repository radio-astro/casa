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
#include <casaqt/QwtPlotter/QPScatterPlot.h>
#include <casaqt/QwtPlotter/QPShape.h>
#include <casaqt/QwtPlotter/QPTool.qo.h>

namespace casa {

///////////////////////////
// QPFACTORY DEFINITIONS //
///////////////////////////

QPFactory::QPFactory() : m_argc(0), m_argv(NULL), m_app(NULL),
        m_createdQApp(false) {
    if(qApp == NULL) {
        m_app = new QApplication(m_argc, m_argv);
        m_createdQApp = true;
    } else m_app = qApp;
}

QPFactory::~QPFactory() {
    if(m_createdQApp) delete m_app;
}


int QPFactory::execLoop() {
    QApplication::processEvents();
    return m_app->exec();
}


PlotterPtr QPFactory::plotter(const String& windowTitle, bool showSingleCanvas,
        bool showGUI, int logEventFlags, bool smartDelete) const {
    QPCanvas* canvas = showSingleCanvas ? new QPCanvas() : NULL;
    PlotterPtr plotter(new QPPlotter(canvas, logEventFlags),smartDelete);
    plotter->setWindowTitle(windowTitle);
    plotter->showGUI(showGUI);
    return plotter;
}

PlotterPtr QPFactory::plotter(unsigned int nrows, unsigned int ncols,
        const String& windowTitle, bool showGUI, int logEventFlags,
        bool smartDelete) const{
    if(nrows == 0 || ncols == 0)
        return plotter(windowTitle, false, showGUI, logEventFlags,
                smartDelete);
    
    PlotLayoutGrid* g = new PlotLayoutGrid(nrows, ncols);
    for(unsigned int i = 0; i < nrows; i++)
        for(unsigned int j = 0; j < ncols; j++)
            g->setCanvasAt(PlotGridCoordinate(i, j), new QPCanvas());
    
    PlotterPtr plotter(new QPPlotter(g, logEventFlags), smartDelete);
    plotter->setWindowTitle(windowTitle);
    plotter->showGUI(showGUI);
    return plotter;
}

PlotCanvasPtr QPFactory::canvas(bool smartDelete) const {
    return PlotCanvasPtr(new QPCanvas(), smartDelete); }

PlotPanelPtr QPFactory::panel(bool smartDelete) const {
    return PlotPanelPtr(new QPPanel(), smartDelete); }

PlotButtonPtr QPFactory::button(const String& str, bool isText,
        bool toggleable, bool smartDelete) const {
    if(isText) return PlotButtonPtr(new QPButton(str, toggleable),smartDelete);
    else {
        QPButton* b = new QPButton("", toggleable);
        b->setImagePath(str);
        return PlotButtonPtr(b, smartDelete);
    }
}

PlotCheckboxPtr QPFactory::checkbox(const String& str, bool smartDelete) const{
    return PlotCheckboxPtr(new QPCheckbox(str), smartDelete); }


ScatterPlotPtr QPFactory::scatterPlot(PlotPointDataPtr data,
        const String& title, bool smartDelete) const {
    if(data.null() || !data->isValid()) return ScatterPlotPtr();
    else return ScatterPlotPtr(new QPScatterPlot(data, title), smartDelete);
}

BarPlotPtr QPFactory::barPlot(PlotPointDataPtr data,
        const String& title, bool smartDelete) const {
    if(data.null() || !data->isValid()) return BarPlotPtr();
    else return BarPlotPtr(new QPBarPlot(data, title), smartDelete);
}

RasterPlotPtr QPFactory::rasterPlot(PlotRasterDataPtr data,const String& title,
            PlotRasterData::Format format, bool smartDelete) const {
    if(data.null() || !data->isValid()) return RasterPlotPtr();
    else return RasterPlotPtr(new QPRasterPlot(data,format,title),smartDelete);
}

PlotAnnotationPtr QPFactory::annotation(const String& text,
        const PlotCoordinate& coord, bool smartDelete) const {
    return PlotAnnotationPtr(new QPAnnotation(text, coord), smartDelete); }

PlotShapeRectanglePtr QPFactory::shapeRectangle(const PlotCoordinate& ul,
        const PlotCoordinate& lr, bool smartDelete) const {
    return PlotShapeRectanglePtr(new QPRectangle(ul, lr), smartDelete); }

PlotShapeEllipsePtr QPFactory::shapeEllipse(const PlotCoordinate& center,
        const PlotCoordinate& radii, bool smartDelete) const {
    return PlotShapeEllipsePtr(new QPEllipse(center, radii), smartDelete); }

PlotShapePolygonPtr QPFactory::shapePolygon(const vector<PlotCoordinate>& c,
        bool smartDelete) const {
    return PlotShapePolygonPtr(new QPPolygon(c), smartDelete); }

PlotShapeLinePtr QPFactory::shapeLine(double location, PlotAxis axis,
        bool smartDelete) const {
    return PlotShapeLinePtr(new QPLineShape(location, axis), smartDelete); }

PlotShapeArrowPtr QPFactory::shapeArrow(const PlotCoordinate& from,
        const PlotCoordinate& to, PlotShapeArrow::Style fromStyle,
        PlotShapeArrow::Style toStyle, bool smartDelete) const {
    QPArrow* a = new QPArrow(from, to);
    a->setArrowStyles(fromStyle, toStyle);
    return PlotShapeArrowPtr(a, smartDelete);
}

PlotShapePathPtr QPFactory::shapePath(const vector<PlotCoordinate>& c,
        bool smartDelete) const {
    return PlotShapePathPtr(new QPPath(c), smartDelete); }

PlotShapeArcPtr QPFactory::shapeArc(const PlotCoordinate& start,
        const PlotCoordinate& wh, int startAngle, int spanAngle,
        bool smartDelete) const {
    return PlotShapeArcPtr(new QPArc(start, wh, startAngle, spanAngle),
            smartDelete); }

PlotPointPtr QPFactory::point(const PlotCoordinate& coord,
        bool smartDelete) const {
    return PlotPointPtr(new QPPoint(coord), smartDelete); }


PlotColorPtr QPFactory::color(const String& color, bool smartDelete) const {
    return PlotColorPtr(new QPColor(color), smartDelete); }

PlotColorPtr QPFactory::color(const PlotColor& copy, bool smartDelete) const {
    return PlotColorPtr(new QPColor(copy), smartDelete); }

vector<String> QPFactory::allNamedColors() const {
    vector<String> v(QPPlotter::GLOBAL_COLORS.size());
    for(int i = 0; i < QPPlotter::GLOBAL_COLORS.size(); i++)
        v[i] = QPPlotter::GLOBAL_COLORS[i].toStdString();
    return v;
}

PlotFontPtr QPFactory::font(const String& family, double pointSize,
        const String& color, bool bold, bool italics, bool underline,
        bool smartDelete) const {
    QPFont* f = new QPFont();
    f->setFontFamily(family);
    f->setPointSize(pointSize);
    f->setColor(QPColor(color));
    f->setBold(bold);
    f->setItalics(italics);
    f->setUnderline(underline);
    return PlotFontPtr(f, smartDelete);
}

PlotFontPtr QPFactory::font(const PlotFont& copy, bool smartDelete) const {
    return PlotFontPtr(new QPFont(copy), smartDelete); }

PlotAreaFillPtr QPFactory::areaFill(const String& color,
        PlotAreaFill::Pattern pattern, bool smartDelete) const {
    QPAreaFill* f = new QPAreaFill();
    f->setColor(QPColor(color));
    f->setPattern(pattern);
    return PlotAreaFillPtr(f, smartDelete);
}

PlotAreaFillPtr QPFactory::areaFill(const PlotAreaFill& copy,
        bool smartDelete) const {
    return PlotAreaFillPtr(new QPAreaFill(copy), smartDelete); }

PlotLinePtr QPFactory::line(const String& color, PlotLine::Style style,
        double width, bool smartDelete) const {
    QPLine* l = new QPLine();
    l->setWidth(width);
    l->setStyle(style);
    l->setColor(QPColor(color));
    return PlotLinePtr(l, smartDelete);
}

PlotLinePtr QPFactory::line(const PlotLine& copy, bool smartDelete) const {
    return PlotLinePtr(new QPLine(copy), smartDelete); }

PlotSymbolPtr QPFactory::symbol(PlotSymbol::Symbol style,
        bool smartDelete) const {
    QPSymbol* s = new QPSymbol();
    s->setSymbol(style);
    s->setSize(10, 10);
    return PlotSymbolPtr(s, smartDelete);
}

PlotSymbolPtr QPFactory::symbol(const PlotSymbol& copy, bool smartDelete)const{
    return PlotSymbolPtr(new QPSymbol(copy), smartDelete); }


PlotSelectToolPtr QPFactory::selectTool(bool smartDelete) const {
    return PlotSelectToolPtr(new QPSelectTool(), smartDelete); }
PlotZoomToolPtr QPFactory::zoomTool(bool smartDelete) const {
    return PlotZoomToolPtr(new QPZoomTool(), smartDelete); }
PlotPanToolPtr QPFactory::panTool(bool smartDelete) const {
    return PlotPanToolPtr(new QPPanTool(), smartDelete); }
PlotTrackerToolPtr QPFactory::trackerTool(bool smartDelete) const {
    return PlotTrackerToolPtr(new QPTrackerTool(), smartDelete); }

PlotSelectToolPtr QPFactory::selectTool(PlotAxis xAxis, PlotAxis yAxis,
        PlotCoordinate::System system, bool smartDelete) const {
    return PlotSelectToolPtr(new QPSelectTool(xAxis, yAxis, system),
            smartDelete); }
PlotZoomToolPtr QPFactory::zoomTool(PlotAxis xAxis, PlotAxis yAxis,
        PlotCoordinate::System system, bool smartDelete) const {
    return PlotZoomToolPtr(new QPZoomTool(xAxis, yAxis, system), smartDelete);}
PlotPanToolPtr QPFactory::panTool(PlotAxis xAxis, PlotAxis yAxis,
        PlotCoordinate::System system, bool smartDelete) const {
    return PlotPanToolPtr(new QPPanTool(xAxis, yAxis, system), smartDelete); }
PlotTrackerToolPtr QPFactory::trackerTool(PlotAxis xAxis, PlotAxis yAxis,
        PlotCoordinate::System system, bool smartDelete) const {
    return PlotTrackerToolPtr(new QPTrackerTool(xAxis, yAxis, system),
            smartDelete); }

PlotMutexPtr QPFactory::mutex(bool smartDelete) const {
    return PlotMutexPtr(new QPMutex(), smartDelete); }


// Static //

PlotFontPtr QPFactory::defaultAnnotationFont(bool smartDelete) {
    QPFont* f = new QPFont();
    f->setFontFamily("Arial");
    f->setPointSize(14.0);
    f->setColor("000000");
    return PlotFontPtr(f, smartDelete);
}

PlotAreaFillPtr QPFactory::defaultBarPlotAreaFill(bool smartDelete) {
    QPAreaFill* a = new QPAreaFill();
    a->setColor("0000FF");
    a->setPattern(PlotAreaFill::FILL);
    return PlotAreaFillPtr(a, smartDelete);
}

PlotLinePtr QPFactory::defaultLegendLine(bool smartDelete) {
    QPLine* l = new QPLine();
    l->setWidth(1.0);
    l->setColor("000000");
    l->setStyle(PlotLine::SOLID);
    return PlotLinePtr(l, smartDelete);
}

PlotAreaFillPtr QPFactory::defaultLegendAreaFill(bool smartDelete) {
    return new QPAreaFill(QApplication::palette().brush(QPalette::Window));
}

PlotLinePtr QPFactory::defaultShapeLine(bool smartDelete) {
    QPLine* l = new QPLine();
    l->setWidth(1.0);
    l->setColor("000000");
    l->setStyle(PlotLine::SOLID);
    return PlotLinePtr(l, smartDelete);
}

PlotAreaFillPtr QPFactory::defaultShapeAreaFill(bool smartDelete) {
    QPAreaFill* a = new QPAreaFill();
    a->setColor("0000FF");
    a->setPattern(PlotAreaFill::NOFILL);
    return PlotAreaFillPtr(a, smartDelete);
}

PlotLinePtr QPFactory::defaultPlotLine(bool smartDelete) {
    QPLine* l = new QPLine();
    l->setColor("000000");
    l->setWidth(1.0);
    l->setStyle(PlotLine::NOLINE);
    return PlotLinePtr(l, smartDelete);
}

PlotSymbolPtr QPFactory::defaultPlotSymbol(bool smartDelete) {
    QPSymbol* s = new QPSymbol();
    s->setSymbol(PlotSymbol::CIRCLE);
    s->setSize(8, 8);
    s->setLine("000000", PlotLine::NOLINE, 1.0);
    s->setAreaFill("0000FF");
    return PlotSymbolPtr(s, smartDelete);
}

PlotSymbolPtr QPFactory::defaultPlotMaskedSymbol(bool smartDelete) {
    QPSymbol* s = new QPSymbol();
    s->setSymbol(PlotSymbol::NOSYMBOL);
    s->setSize(8, 8);
    s->setLine("000000", PlotLine::NOLINE, 1.0);
    s->setAreaFill("FF0000");
    return PlotSymbolPtr(s, smartDelete);
}

const unsigned int QPFactory::DEFAULT_ERROR_CAP = 10;

}

#endif
