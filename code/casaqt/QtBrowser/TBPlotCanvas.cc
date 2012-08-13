//# TBPlotCanvas.cc: Canvas for data plotting using qwt.
//# Copyright (C) 2005
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

#include <casaqt/QtBrowser/TBPlotCanvas.qo.h>
#include <casaqt/QtBrowser/TBConstants.h>
#include <casaqt/QtBrowser/TBPlotter.qo.h>

#include <QHBoxLayout>

namespace casa {

//////////////////////////////
// TBPLOTFORMAT DEFINITIONS //
//////////////////////////////

// Constructors/Destructors //

TBPlotFormat::TBPlotFormat(PlotFactoryPtr factory) :
        line(TBConstants::defaultPlotLine(factory)),
        symbol(TBConstants::defaultPlotSymbol(factory)) { }

TBPlotFormat::~TBPlotFormat() { }

// Public Methods //

void TBPlotFormat::setCurveStyle(QString str) {
    str = str.toLower();
    if(str == "none") line->setStyle(PlotLine::NOLINE);
    else if(str == "solid") line->setStyle(PlotLine::SOLID);
    else if(str == "dashed") line->setStyle(PlotLine::DASHED);
    else if(str == "dotted") line->setStyle(PlotLine::DOTTED);
}

void TBPlotFormat::setPointStyle(QString str) {
    str = str.toLower();
    
    if(str == "diamond") symbol->setSymbol(PlotSymbol::DIAMOND);
    else if(str == "circle") symbol->setSymbol(PlotSymbol::CIRCLE);
    else if(str == "square") symbol->setSymbol(PlotSymbol::SQUARE);
    else if(str == "autoscaling") symbol->setSymbol(PlotSymbol::AUTOSCALING);
    else if(str == "none") symbol->setSymbol(PlotSymbol::NOSYMBOL);
}


//////////////////////////////
// TBPLOTCANVAS DEFINITIONS //
//////////////////////////////

// Constructors/Destructors //

TBPlotCanvas::TBPlotCanvas(PlotFactoryPtr f) : QWidget(), factory(f) {
    setupUi(this);
    
    canvas = factory->canvas();
    plotter = factory->plotter(canvas, "TableBrowser Plotter");
    
    if(plotter->isQWidget()) {
        QWidget* widget = dynamic_cast<QWidget*>(plotter.operator->());
        TBConstants::insert(frame, widget);
    } else {
        // just let it do its own thing
        frame->setVisible(false);
    }
    plotter->showGUI();
    
    // Set up canvas
    canvas->showAxis(X_BOTTOM, false);
    canvas->showAxis(Y_LEFT, false);
    canvas->showAxis(X_TOP, false);
    canvas->showAxis(Y_RIGHT, false);
    canvas->registerSelectHandler(PlotSelectEventHandlerPtr(this, false));
    
    // Set up plotter
    plotter->showDefaultPanel(Plotter::HAND_TOOLS, true);
}

TBPlotCanvas::~TBPlotCanvas() {
    for(unsigned int i = 0; i < data.size(); i++)
        delete data[i];
}

// Accessors/Mutators //

void TBPlotCanvas::setXAxisTitle(String title) {
    canvas->showAxis(X_BOTTOM, true);
    canvas->setAxisLabel(X_BOTTOM, title);
}

void TBPlotCanvas::setYAxisTitle(String title) {
    canvas->showAxis(Y_LEFT, true);
    canvas->setAxisLabel(Y_LEFT, title);
}

void TBPlotCanvas::setShownGrids(bool xMaj, bool xMin, bool yMaj, bool yMin) {
    canvas->showGrid(xMaj, xMin, yMaj, yMin);
}

void TBPlotCanvas::setXAxisDate(bool date, bool mjsec) {
    if(date && mjsec) canvas->setAxisScale(X_BOTTOM, DATE_MJ_SEC);
    else if(date) canvas->setAxisScale(X_BOTTOM, DATE_MJ_DAY);
    else canvas->setAxisScale(X_BOTTOM, NORMAL);
}

void TBPlotCanvas::setYAxisDate(bool date, bool mjsec) {
    if(date && mjsec) canvas->setAxisScale(Y_LEFT, DATE_MJ_SEC);
    else if(date) canvas->setAxisScale(Y_LEFT, DATE_MJ_DAY);
    else canvas->setAxisScale(Y_LEFT, NORMAL);
}

String TBPlotCanvas::getCurrentTable() { return currentTable; }

void TBPlotCanvas::setTable(String table) { currentTable = table; }

int TBPlotCanvas::getNumPlots() { 
    return canvas->allPlots().size();
}

vector<TBPlotData*> TBPlotCanvas::allData() { return data; }

// Public Methods //

void TBPlotCanvas::handleSelect(const PlotSelectEvent& event) {
    if(!selectedRegion.null())
        canvas->removeShape(selectedRegion);
    
    PlotRegion region = event.region();
    selectedRegion = factory->shapeRectangle(region.upperLeft(),
            region.lowerRight());
    
    selectedRegion->setLine(factory->line("000000"));
    selectedRegion->setAreaFill(
            factory->areaFill("000000", PlotAreaFill::MESH3));
    
    canvas->drawShape(selectedRegion);
    emit regionSelected(true);
}

void TBPlotCanvas::plot(TBPlotData* pdata, const TBPlotFormat& format,
                        bool overplot) {
    if(pdata == NULL) return;
    
    canvas->setTitle(currentTable);
    canvas->showAxis(X_BOTTOM, true);
    canvas->showAxis(Y_LEFT, true);
    
    PlotPointDataPtr d = pdata->data;
    ScatterPlotPtr plot;
    if(pdata->title.empty()) plot = factory->scatterPlot(d);
    else plot = factory->scatterPlot(d, pdata->title);
    
    plot->setLine(format.line);
    plot->setSymbol(format.symbol);
    canvas->plot(plot, overplot);
    data.push_back(pdata);
}

Result TBPlotCanvas::exportToImage(String format, String location) {
    PlotExportFormat::Type f = PlotExportFormat::exportFormat(format);
    PlotExportFormat ex(f, location);
    bool b = canvas->exportToFile(ex);
    if(b) return Result("", b);
    else return Result("Error during export to " + location + ".", b);
}

void TBPlotCanvas::clearSelectedRectangle() {
    if(!selectedRegion.null())
        canvas->removeShape(selectedRegion);
    selectedRegion = PlotShapeRectanglePtr();
}

PlotRegion TBPlotCanvas::currentSelection() {
    if(selectedRegion.null()) return PlotRegion();
    
    vector<PlotCoordinate> coords = selectedRegion->coordinates();
    if(coords.size() < 2) return PlotRegion();
    else return PlotRegion(coords[0], coords[1]);
}

// Public Slots //

void TBPlotCanvas::clearAndHideAxes() {
    clearPlots();    
    emit regionSelected(false);
    
    canvas->showAxis(X_BOTTOM, false);
    canvas->showAxis(Y_LEFT, false);
}


// Private Slots //

void TBPlotCanvas::clearPlots() {
    for(unsigned int i = 0; i < data.size(); i++)
        delete data[i];
    data.clear();
    canvas->clearPlots();
}

}
