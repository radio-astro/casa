//# PlotMSPage.cc: Layout of PlotCanvases on a single "page".
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
#include <plotms/Plots/PlotMSPage.h>
#include <plotms/PlotMS/PlotMS.h>
#include <plotms/Plots/PlotMSPlot.h>

namespace casa {

////////////////////////////
// PLOTMSPAGE DEFINITIONS //
////////////////////////////

// Public Methods //

PlotMSPage::PlotMSPage(const PlotMSPage& copy) {
    operator=(copy); }

PlotMSPage::~PlotMSPage() { }


unsigned int PlotMSPage::canvasRows() const { return itsCanvases_.size(); }
unsigned int PlotMSPage::canvasCols() const {
    if(itsCanvases_.size() > 0) return itsCanvases_[0].size();
    else                        return 0;
}


PlotMSPage& PlotMSPage::operator=(const PlotMSPage& copy) {
    itsParent_ = copy.itsParent_;
    itsCanvases_ = copy.itsCanvases_;
    itsCanvasOwners_ = copy.itsCanvasOwners_;
    return *this;
}


// Private Methods //

PlotMSPage::PlotMSPage(PlotMSPages& parent) :
        itsParent_(&parent){ }


void PlotMSPage::resize(unsigned int nrows, unsigned int ncols) {
    unsigned int oldrows = canvasRows(), oldcols = canvasCols();
    if(nrows == oldrows && ncols == oldcols) return;

    // we're shrinking in rows or columns, so disown those that will be
    // deleted
    for(unsigned int r = 0; r < itsCanvases_.size(); r++){
    	for(unsigned int c = 0; c < itsCanvases_[r].size(); c++){

    		disown(r, c);
    	}
    }

    itsCanvases_.clear();
    itsCanvasOwners_.clear();
    itsCanvases_.resize(nrows);
    itsCanvasOwners_.resize(nrows);
    
    PlotMSApp* plotms = itsParent_->itsManager_->parent();
    PlotFactoryPtr factory = plotms->getPlotFactory();
    PlotStandardMouseToolGroupPtr tools;
    pair<int, int> cimg = plotms->getParameters().cachedImageSize();
    PlotCanvasPtr canvas;
    for(unsigned int r = 0; r < nrows; r++) {
        itsCanvases_[r].resize(ncols);
        itsCanvasOwners_[r].resize(ncols, NULL);
        for(unsigned int c = 0; c < ncols; c++) {

        	canvas = factory->canvas();
        	itsCanvases_[r][c] = canvas;

        	plotms->canvasAdded( canvas );

        	// set cached image size
        	canvas->setCachedAxesStackImageSize(cimg.first, cimg.second);
        }
    }
}

PlotCanvasPtr PlotMSPage::canvas(unsigned int row, unsigned int col) {
    if(row >= itsCanvases_.size() || col >= itsCanvases_[row].size())
        return PlotCanvasPtr();
    else return itsCanvases_[row][col];
}

PlotMSPlot* PlotMSPage::owner(unsigned int row, unsigned int col) {
    if(row >= itsCanvasOwners_.size() || col >= itsCanvasOwners_[row].size())
        return NULL;
    else return itsCanvasOwners_[row][col];
}

bool PlotMSPage::setOwner(unsigned int row, unsigned int col, PlotMSPlot* plot) {
    if(row >= itsCanvasOwners_.size() || col >= itsCanvasOwners_[row].size() ||
       plot == NULL || (itsCanvasOwners_[row][col] != NULL &&
       itsCanvasOwners_[row][col] != plot)) return false;

    itsCanvasOwners_[row][col] = plot;
    return true;
}

bool PlotMSPage::disown(unsigned int row, unsigned int col) {
    if(row >= itsCanvasOwners_.size() || col >= itsCanvasOwners_[row].size() ||
       itsCanvasOwners_[row][col] == NULL) return false;
    itsCanvases_[row][col]->clearItems();
    itsCanvasOwners_[row][col]->canvasWasDisowned(itsCanvases_[row][col]);
    itsCanvasOwners_[row][col] = NULL;
    return true;
}

void PlotMSPage::setupPage() {
    // Clear out old canvases.
    PlotterPtr plotter = itsParent_->itsManager_->plotter();
    plotter->setCanvasLayout(PlotCanvasLayoutPtr());
    
    // Create a new grid layout.
    unsigned int rows = canvasRows(), cols = canvasCols();
    PlotLayoutGrid* grid = new PlotLayoutGrid(rows, cols);
    PlotGridCoordinate coord(0, 0);
    for(unsigned int r = 0; r < rows; r++) {
        for(unsigned int c = 0; c < cols; c++) {
            coord.row = r; coord.col = c;
            grid->setCanvasAt(coord, itsCanvases_[r][c]);
        }
    }
    plotter->setCanvasLayout(grid);
}
}



