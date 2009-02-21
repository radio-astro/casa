//# PlotCanvas.cc: Main drawing area for different plot items to be attached.
//# Copyright (C) 2009
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
#include <graphics/GenericPlotter/PlotCanvas.h>

namespace casa {

////////////////////////////
// PLOTCANVAS DEFINITIONS //
////////////////////////////

// Static //

vector<PlotCanvas::LegendPosition> PlotCanvas::allLegendPositions() {
    static vector<LegendPosition> v(8);
    v[0] = INT_URIGHT; v[1] = INT_LRIGHT;
    v[2] = INT_ULEFT;  v[3] = INT_LLEFT;
    v[4] = EXT_RIGHT;  v[5] = EXT_TOP;
    v[6] = EXT_LEFT;   v[7] = EXT_BOTTOM;
    return v;
}

vector<String> PlotCanvas::allLegendPositionStrings() {
    static vector<LegendPosition> p = allLegendPositions();
    static vector<String> v(p.size());
    for(unsigned int i = 0; i < v.size(); i++) v[i] = legendPosition(p[i]);
    return v;
}

String PlotCanvas::legendPosition(LegendPosition p) {
    switch(p) {
    case INT_URIGHT: return "Upper Right";
    case INT_LRIGHT: return "Lower Right";
    case INT_ULEFT:  return "Upper Left";
    case INT_LLEFT:  return "Lower Left";
    case EXT_RIGHT:  return "Out Right";
    case EXT_TOP:    return "Out Top";
    case EXT_LEFT:   return "Out Left";
    case EXT_BOTTOM: return "Out Bottom";
    
    default: return "";
    }
}

PlotCanvas::LegendPosition PlotCanvas::legendPosition(String p, bool* ok) {
    p.downcase();
    if(ok != NULL) *ok = true;
    if(p == "upper right")      return INT_URIGHT;
    else if(p == "upper left")  return INT_ULEFT;
    else if(p == "lower right") return INT_LRIGHT;
    else if(p == "lower left")  return INT_LLEFT;
    else if(p == "out right")   return EXT_RIGHT;
    else if(p == "out left")    return EXT_LEFT;
    else if(p == "out top")     return EXT_TOP;
    else if(p == "out bottom")  return EXT_BOTTOM;
    
    if(ok != NULL) *ok = false;
    return INT_URIGHT;
}

bool PlotCanvas::legendPositionIsInternal(LegendPosition p) {
    return p == INT_URIGHT || p == INT_ULEFT || p == INT_LRIGHT ||
           p == INT_LLEFT;
}

const String PlotCanvas::OPERATION_DRAW = "draw_items";


// Non-Static //

PlotCanvas::PlotCanvas() { }

PlotCanvas::~PlotCanvas() { }

void PlotCanvas::setTitleFont(const PlotFontPtr font) {
    if(!font.null()) setTitleFont(*font); }

void PlotCanvas::setBackground(const PlotAreaFillPtr areaFill) {
    if(!areaFill.null()) setBackground(*areaFill); }
void PlotCanvas::setBackground(const String& color,
        PlotAreaFill::Pattern pattern) {
    PlotAreaFillPtr bg = background();
    bg->setColor(color);
    bg->setPattern(pattern);
    setBackground(*bg);
}

void PlotCanvas::showAxes(PlotAxis xAxis, PlotAxis yAxis, bool show) {
    showAxis(xAxis, show);
    showAxis(yAxis, show);
}

void PlotCanvas::showAxes(bool show) {
    showAxis(X_BOTTOM, show);
    showAxis(X_TOP, show);
    showAxis(Y_LEFT, show);
    showAxis(Y_RIGHT, show);
}

PlotRegion PlotCanvas::axesRanges(PlotAxis xAxis, PlotAxis yAxis) const {
    pair<double, double> x = axisRange(xAxis), y = axisRange(yAxis);
    return PlotRegion(PlotCoordinate(x.first, y.second),
                      PlotCoordinate(x.second, y.first));
}

void PlotCanvas::setAxisRange(PlotAxis axis, const pair<double,double>& range){
    setAxisRange(axis, range.first, range.second); }

void PlotCanvas::setAxesRanges(PlotAxis xAxis, double xFrom, double xTo,
        PlotAxis yAxis, double yFrom, double yTo) {
    setAxisRange(xAxis, xFrom, xTo);
    setAxisRange(yAxis, yFrom, yTo);
}

void PlotCanvas::setAxesRanges(PlotAxis xAxis,
        const pair<double, double>& xRange, PlotAxis yAxis,
        const pair<double, double>& yRange) {
    setAxesRanges(xAxis, xRange.first, xRange.second, yAxis, yRange.first,
                  yRange.second);
}

void PlotCanvas::setAxesRegion(const PlotRegion& region, PlotAxis xAxis,
        PlotAxis yAxis) {
    PlotRegion r = region;
    if(region.upperLeft().system() != PlotCoordinate::WORLD ||
       region.lowerRight().system() != PlotCoordinate::WORLD)
        r = convertRegion(region, PlotCoordinate::WORLD);
    setAxesRanges(xAxis, region.upperLeft().x(), region.lowerRight().x(),
                  yAxis, region.lowerRight().y(), region.upperLeft().y());
}

void PlotCanvas::moveAxisRange(PlotAxis axis, double delta) {
    pair<double, double> r = axisRange(axis);
    r.first += delta; r.second += delta;
    setAxisRange(axis, r.first, r.second);
}

void PlotCanvas::moveAxesRanges(PlotAxis xAxis, double xDelta,
        PlotAxis yAxis, double yDelta) {
    pair<double, double> x = axisRange(xAxis), y = axisRange(yAxis);
    x.first += xDelta; x.second += xDelta;
    y.first += yDelta; y.second += yDelta;
    setAxesRanges(xAxis, x.first, x.second, yAxis, y.first, y.second);
}

PlotAxesStack& PlotCanvas::canvasAxesStack() const {
    static PlotAxesStack stack;
    return stack;
}

bool PlotCanvas::canvasAxesStackMove(int delta) {
    PlotAxesStack& stack = canvasAxesStack();
    if(!stack.isValid()) return false;
    
    unsigned int index = stack.stackIndex(), size = stack.size();
    
    if(size == 1 || (delta <= 0 && index == 0) ||
       (delta > 0 && index == size - 1)) return false;
    
    PlotRegion r = convertRegion(stack.moveAndReturn(delta),
                                 PlotCoordinate::WORLD);
    setAxesRegion(r, stack.currentXAxis(), stack.currentYAxis());    
    return true;
}

void PlotCanvas::showCartesianAxis(PlotAxis mirrorAxis, bool show,
        bool hideNormalAxis) {
    switch(mirrorAxis) {
    case X_BOTTOM: case X_TOP:
        showCartesianAxis(mirrorAxis, Y_LEFT, show, hideNormalAxis);
        break;
    case Y_LEFT: case Y_RIGHT:
        showCartesianAxis(mirrorAxis, X_BOTTOM, show, hideNormalAxis);
        break;
    }
}

void PlotCanvas::showCartesianAxes(bool show, bool hideNormal) {
    showCartesianAxis(X_BOTTOM, show, hideNormal);
    showCartesianAxis(Y_LEFT, show, hideNormal);
}

void PlotCanvas::clearAxesLabels() {
    setAxisLabel(X_BOTTOM, "");
    setAxisLabel(X_TOP, "");
    setAxisLabel(Y_LEFT, "");
    setAxisLabel(Y_RIGHT, "");
}

void PlotCanvas::setAxisFont(PlotAxis axis, const PlotFontPtr font) {
    if(!font.null()) setAxisFont(axis, *font); }
    

PlotRegion PlotCanvas::convertRegion(const PlotRegion& region,
        PlotCoordinate::System newSystem) const {
    PlotCoordinate upperLeft = convertCoordinate(region.upperLeft(),
                               newSystem);
    PlotCoordinate lowerRight = convertCoordinate(region.lowerRight(),
                                newSystem);
    return PlotRegion(upperLeft, lowerRight);
}

vector<double>
PlotCanvas::annotationWidthHeightDescent(const PlotAnnotationPtr annot) const {
    if(annot.null()) return vector<double>();
    else return textWidthHeightDescent(annot->text(), annot->font());
}

bool PlotCanvas::plot(PlotPtr plot, bool overplot) {
    if(!overplot) clearPlots();
    return plotItem(plot, MAIN);
}
bool PlotCanvas::plotPoint(PlotPointPtr point){ return plotItem(point, MAIN); }
bool PlotCanvas::drawShape(PlotShapePtr shape){ return plotItem(shape, MAIN); }
bool PlotCanvas::drawAnnotation(PlotAnnotationPtr annotation) {
    return plotItem(annotation, MAIN); }

// Macro for common method definitions.
#define PC_ALL(ITEM,CLASS)                                                    \
vector< CLASS##Ptr > PlotCanvas::all##ITEM##s() const {                       \
    vector<PlotItemPtr> v = allPlotItems();                                   \
    vector< CLASS##Ptr > v2;                                                  \
    for(unsigned int i = 0; i < v.size(); i++)                                \
        if(!v[i].null() && dynamic_cast< CLASS *>(&*v[i]) != NULL)            \
            v2.push_back(v[i]);                                               \
    return v2;                                                                \
}                                                                             \
                                                                              \
vector< CLASS##Ptr > PlotCanvas::layer##ITEM##s(PlotCanvasLayer layer) const {\
    vector<PlotItemPtr> v = layerPlotItems(layer);                            \
    vector< CLASS##Ptr > v2;                                                  \
    for(unsigned int i = 0; i < v.size(); i++)                                \
        if(!v[i].null() && dynamic_cast< CLASS *>(&*v[i]) != NULL)            \
            v2.push_back(v[i]);                                               \
    return v2;                                                                \
}                                                                             \
                                                                              \
void PlotCanvas::removeLast##ITEM () {                                        \
    vector< CLASS##Ptr > v = all##ITEM##s();                                  \
    for(int i = v.size() - 1; i >= 0; i--) {                                  \
        if(!v[i].null()) {                                                    \
            removePlotItem(v[i]);                                             \
            break;                                                            \
        }                                                                     \
    }                                                                         \
}                                                                             \
                                                                              \
void PlotCanvas::clear##ITEM##s() {                                           \
    vector< CLASS##Ptr > v = all##ITEM##s();                                  \
    vector<PlotItemPtr> v2(v.size());                                         \
    for(unsigned int i = 0; i < v.size(); i++) v2[i] = v[i];                  \
    removePlotItems(v2);                                                      \
}

PC_ALL(Plot, Plot)
PC_ALL(Point, PlotPoint)
PC_ALL(Shape, PlotShape)
PC_ALL(Annotation, PlotAnnotation)

unsigned int PlotCanvas::numPlotItems() const { return allPlotItems().size(); }

unsigned int PlotCanvas::numLayerPlotItems(PlotCanvasLayer layer) const {
    return layerPlotItems(layer).size(); }

unsigned int PlotCanvas::numPlots() const { return allPlots().size(); }
unsigned int PlotCanvas::numPoints() const { return allPoints().size(); }
unsigned int PlotCanvas::numShapes() const { return allShapes().size(); }
unsigned int PlotCanvas::numAnnotations() const {
    return allAnnotations().size(); }

unsigned int PlotCanvas::numLayerPlots(PlotCanvasLayer layer) const {
    return layerPlots(layer).size(); }
unsigned int PlotCanvas::numLayerPoints(PlotCanvasLayer layer) const {
    return layerPoints(layer).size(); }
unsigned int PlotCanvas::numLayerShapes(PlotCanvasLayer layer) const {
    return layerShapes(layer).size(); }
unsigned int PlotCanvas::numLayerAnnotations(PlotCanvasLayer layer) const {
    return layerAnnotations(layer).size(); }

void PlotCanvas::removePlotItem(PlotItemPtr item) {
    if(!item.null()) removePlotItems(vector<PlotItemPtr>(1, item)); }

void PlotCanvas::removePlot(PlotPtr plot) { removePlotItem(plot); }
void PlotCanvas::removePoint(PlotPointPtr point) { removePlotItem(point); }
void PlotCanvas::removeShape(PlotShapePtr shape) { removePlotItem(shape); }
void PlotCanvas::removeAnnotation(PlotAnnotationPtr annotation) {
    removePlotItem(annotation); }

void PlotCanvas::removeLastPlotItem() {
    vector<PlotItemPtr> v = allPlotItems();
    for(int i = v.size() - 1; i >= 0; i--) {
        if(!v[i].null()) {
            removePlotItem(v[i]);
            break;
        }
    }
}

void PlotCanvas::clearItems() { removePlotItems(allPlotItems()); }

void PlotCanvas::clearLayer(PlotCanvasLayer layer) {
    removePlotItems(layerPlotItems(layer)); }
    
vector<vector<pair<unsigned int, unsigned int> > >*
PlotCanvas::locate(const PlotRegion& r) const {
    PlotRegion region = convertRegion(r, PlotCoordinate::WORLD);
    
    vector<PlotPtr> plots = allPlots();
    
    vector<vector<pair<unsigned int, unsigned int> > >* v =
        new vector<vector<pair<unsigned int, unsigned int> > >(plots.size());
    if(plots.size() == 0) return v;
    
    PlotPointDataPtr data;
    double xLeft = region.upperLeft().x(), xRight = region.lowerRight().x(),
           yTop = region.upperLeft().y(), yBottom = region.lowerRight().y();
    double x, y;
    pair<unsigned int, unsigned int> p;
    vector<pair<unsigned int, unsigned int> >* v2;
    unsigned int rangeLow = 0;
    bool inRange;
    
    for(unsigned int i = 0; i < plots.size(); i++) {
        data = plots[i]->data();
        if(data.null()) continue; // shouldn't happen
        
        v2 = &v->at(i);
        inRange = false;
        for(unsigned int j = 0; j < data->size(); j++) {
            x = data->xAt(j);
            y = data->yAt(j);
            
            if(x >= xLeft && x <= xRight && y <= yTop && y >= yBottom) {
                if(!inRange) {
                    rangeLow = j;
                    inRange = true;
                }
            } else if(inRange) {
                v2->push_back(pair<unsigned int, unsigned int>(rangeLow,
                                                               j - 1));
                inRange = false;
            }
        }
        
        // catch last range
        if(inRange) {
            v2->push_back(pair<unsigned int, unsigned int>(rangeLow,
                                                           data->size() - 1));
        }
    }
    
    return v;
}

void PlotCanvas::locateAndLog(const PlotRegion& region,
        PlotLoggerPtr logger) const {
    if(logger.null()) return;
    vector<vector<pair<unsigned int, unsigned int> > >* res = locate(region);
    PlotLogLocate msg("PlotCanvas", "locate", region, res);
    logger->postMessage(msg);
}

void PlotCanvas::setSelectLine(const PlotLinePtr line) {
    if(!line.null()) setSelectLine(*line);
    else             setSelectLineShown(false);
}

void PlotCanvas::setSelectLine(const String& color, PlotLine::Style style,
        double width) {
    PlotLinePtr line = selectLine();
    line->setColor(color);
    line->setStyle(style);
    line->setWidth(width);
    setSelectLine(*line);
}

void PlotCanvas::setGridShown(bool showAll) {
    setGridXMajorShown(showAll);
    setGridXMinorShown(showAll);
    setGridYMajorShown(showAll);
    setGridYMinorShown(showAll);
}

void PlotCanvas::setGridShown(bool xMajor, bool xMinor, bool yMajor,
        bool yMinor) {
    setGridXMajorShown(xMajor);
    setGridXMinorShown(xMinor);
    setGridYMajorShown(yMajor);
    setGridYMinorShown(yMinor);
}

void PlotCanvas::setGridMajorShown(bool show) {
    setGridXMajorShown(show);
    setGridYMajorShown(show);
}

void PlotCanvas::setGridMinorShown(bool show) {
    setGridXMinorShown(show);
    setGridYMinorShown(show);
}

void PlotCanvas::setGridMajorLine(const PlotLinePtr line) {
    if(!line.null()) setGridMajorLine(*line);
    else             setGridMajorShown(false);
}
void PlotCanvas::setGridMajorLine(const String& color, PlotLine::Style style,
        double width) {
    PlotLinePtr line = gridMajorLine();
    line->setColor(color);
    line->setStyle(style);
    line->setWidth(width);
    setGridMajorLine(*line);
}

void PlotCanvas::setGridMinorLine(const PlotLinePtr line) {
    if(!line.null()) setGridMinorLine(*line);
    else             setGridMinorShown(false);
}

void PlotCanvas::setGridMinorLine(const String& color, PlotLine::Style style,
        double width) {
    PlotLinePtr line = gridMinorLine();
    line->setColor(color);
    line->setStyle(style);
    line->setWidth(width);
    setGridMinorLine(*line);
}

void PlotCanvas::setLegendLine(const PlotLinePtr line) {
    if(!line.null()) setLegendLine(*line);
    else             setLegendLine("000000", PlotLine::NOLINE);
}

void PlotCanvas::setLegendLine(const String& color, PlotLine::Style style,
        double width) {
    PlotLinePtr line = legendLine();
    line->setColor(color);
    line->setStyle(style);
    line->setWidth(width);
    setLegendLine(*line);
}

void PlotCanvas::setLegendFill(const PlotAreaFillPtr area) {
    if(!area.null()) setLegendFill(*area);
    else             setLegendFill("000000", PlotAreaFill::NOFILL);
}

void PlotCanvas::setLegendFill(const String& color,
        PlotAreaFill::Pattern pattern) {
    PlotAreaFillPtr fill = legendFill();
    fill->setColor(color);
    fill->setPattern(pattern);
    setLegendFill(*fill);
}

void PlotCanvas::setLegendFont(const PlotFontPtr font) {
    if(!font.null()) setLegendFont(*font); }

PlotOperationPtr PlotCanvas::operationDraw() const {
    return operationDraw(PlotMutexPtr());
}

PlotOperationPtr PlotCanvas::operationDraw(PlotMutexPtr m) const {
    static PlotOperationPtr draw = new PlotOperation(OPERATION_DRAW, mutex());
    if(!m.null()) draw->setMutex(m);
    return draw;
}


// Protected Methods //

void PlotCanvas::attachTool(PlotToolPtr tool) {
    if(!tool.null()) tool->attach(this); }
void PlotCanvas::detachTool(PlotToolPtr tool) {
    if(!tool.null()) tool->detach(); }

}
