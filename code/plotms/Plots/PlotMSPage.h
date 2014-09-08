//# PlotMSPage.h: Layout of PlotCanvases on a single "page".
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
#ifndef PLOTMSPAGE_H_
#define PLOTMSPAGE_H_

#include <graphics/GenericPlotter/PlotCanvas.h>

#include <casa/namespace.h>
#include <QList>

namespace casa {

//# Forward Declarations.
class PlotMSPages;
class PlotMSPlot;
class PlotMSPlotManager;


// Represents a single page of PlotCanvases, arranged in a grid.
class PlotMSPage {
    
    //# Friend class declarations.
    friend class PlotMSPlot;
    friend class PlotMSPages;
    
public:    
    // Copy constructor.
    PlotMSPage(const PlotMSPage& copy);
    
    // Destructor.
    ~PlotMSPage();

    // Returns the number of rows/columns of the canvas grid on this page.
    // <group>
    unsigned int canvasRows() const;
    unsigned int canvasCols() const;
    // </group>

    //Erase all trace of this plot
    void disown( PlotMSPlot* plot );

    //Erase all traces of a plot at the specific location including removing axes and title.
    void clearCanvas( int row, int col );

    //Remove axes and titles from all the canvases.
    void clearCanvases();

    // Copy operator.
    PlotMSPage& operator=(const PlotMSPage& copy);

    // Returns true if the canvas at (rowIndex, colIndex) does not yet have
    // a plot or if it already has the plot passed in; return false otherwise.
    bool isSpot( int rowIndex, int colIndex, PlotMSPlot* plot ) const;

    //  Returns the <rowIndex,colIndex> of the first available canvas that does
    //  not yet have a plot.  Returns <-1,-1> if there are no canvases without
    //  plots.
    pair<int,int> findEmptySpot() const;

    //Returns whether or not the plot is the owner of a canvas located at the given
    //rowIndex and colIndex.
    bool isOwner( int rowIndex, int colIndex, PlotMSPlot* plot ) const;


private:
    // Resizes the grid to the given number of rows and columns.
    void resize(unsigned int nrows, unsigned int ncols);

    // Parent.
    PlotMSPages* itsParent_;

    // Canvases grid.
    vector<vector<PlotCanvasPtr > > itsCanvases_;
    
    // Owner grid.
    vector<vector<QList<PlotMSPlot*> > > itsCanvasOwners_;
    
    
    // Constructor.
    PlotMSPage(PlotMSPages& parent);
    
    //Returns true if the canvas at the given row and col was disowned; false otherwise.
    bool disown( int row, int col );
    
    // Returns the canvas at the given row and column, or NULL if invalid.
    PlotCanvasPtr canvas(unsigned int row, unsigned int col);
    
    // Returns the owner plot(s) at the given row and column, or an empty list if invalid
    // or there is no owner for that canvas.  For overplotting, there could be multiple owners
    // of the canvas at the given location.
    QList<PlotMSPlot*> owner(unsigned int row, unsigned int col) const;
    
    // Sets the owner for the canvas at the given row and column to the given
    // plot; returns true for success, false otherwise.  If the given canvas is
    // already owned, it must first be disowned by its old owner.
    bool setOwner(unsigned int row, unsigned int col, PlotMSPlot* plot);
    
    // Returns true if the canvas at the given row and column exists and is
    // owned, false otherwise.
    bool isOwned(unsigned int row, unsigned int col);

    
    // Sets the canvas at the given row and column to be owned by no one.  All
    // items are removed from the canvas.  Returns true for success, false for
    // failure.
    bool disown(unsigned int row, unsigned int col, PlotMSPlot* plot);
    
    // Sets up this page on the plotter.
    void setupPage();
};




}

#endif /* PLOTMSPAGE_H_ */
