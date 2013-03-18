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

namespace casa {

//# Forward Declarations.
class PlotMSPages;
class PlotMSPlot;
class PlotMSPlotManager;


// Represents a single page of PlotCanvases, arranged in a grid.
class PlotMSPage {
    
    //# Friend class declarations.
    friend class PlotMSOverPlot;
    friend class PlotMSMultiPlot;
    friend class PlotMSPages;
    friend class PlotMSSinglePlot;
    friend class PlotMSIterPlot;
    
public:    
    // Copy constructor.
    PlotMSPage(const PlotMSPage& copy);
    
    // Destructor.
    ~PlotMSPage();
    
    
    // Returns this page's page number.
    unsigned int pageNumber() const;
    
    // Returns the number of rows/columns of the canvas grid on this page.
    // <group>
    unsigned int canvasRows() const;
    unsigned int canvasCols() const;
    // </group>
    
    
    // Copy operator.
    PlotMSPage& operator=(const PlotMSPage& copy);
    
private:
    // Parent.
    PlotMSPages* itsParent_;
    
    // Page number.
    unsigned int itsPageNum_;
    
    // Canvases grid.
    vector<vector<PlotCanvasPtr> > itsCanvases_;
    
    // Owner grid.
    vector<vector<PlotMSPlot*> > itsCanvasOwners_;
    
    
    // Constructor.
    PlotMSPage(PlotMSPages& parent, unsigned int pageNumber);
    
    
    // Resizes the grid to the given number of rows and columns.
    void resize(unsigned int nrows, unsigned int ncols);
    
    // Returns the canvas at the given row and column, or NULL if invalid.
    PlotCanvasPtr canvas(unsigned int row, unsigned int col);
    
    // Returns the owner plot at the given row and column, or NULL if invalid
    // or there is no owner for that canvas.
    PlotMSPlot* owner(unsigned int row, unsigned int col);
    
    // Sets the owner for the canvas at the given row and column to the given
    // plot; returns true for success, false otherwise.  If the given canvas is
    // already owned, it must first be disowned by its old owner.
    bool setOwner(unsigned int row, unsigned int col, PlotMSPlot* plot);
    
    // Returns true if the canvas at the given row and column exists and is
    // owned, false otherwise.
    bool isOwned(unsigned int row, unsigned int col) {
        return owner(row, col) != NULL; }
    
    // Sets the canvas at the given row and column to be owned by no one.  All
    // items are removed from the canvas.  Returns true for success, false for
    // failure.
    bool disown(unsigned int row, unsigned int col);
    
    // Sets up this page on the plotter.
    void setupPage();
};


// Represents (potentially) multiple pages for PlotMS, with one being current
// (visible) at a time.
class PlotMSPages {
    
    //# Friend class declarations.
    friend class PlotMSMultiPlot;
    friend class PlotMSPage;
    friend class PlotMSPlot;
    friend class PlotMSPlotManager;
    friend class PlotMSSinglePlot;
    friend class PlotMSIterPlot;

public:
    // Constructor, which the plot manager.
    PlotMSPages(PlotMSPlotManager& manager);

    // Copy constructor.
    PlotMSPages(const PlotMSPages& copy);

    // Destructor.
    ~PlotMSPages();

    // Returns the current page number.
    unsigned int currentPageNumber() const;

    // Returns a COPY of the current page.
    PlotMSPage currentPage() const;

    void setCurrentPageNum(uInt num) {
        if(num < totalPages()) itsCurrentPageNum_ = num;
    }

    // Accessor
    PlotMSPage& operator[](uInt index) { return itsPages_[index]; }

    // Iterators
    typedef vector<PlotMSPage>::iterator iterator;
    iterator begin() { return itsPages_.begin(); }
    iterator end() { return itsPages_.end(); }

    typedef vector<PlotMSPage>::const_iterator const_iterator;
    const_iterator begin() const { return itsPages_.begin(); }
    const_iterator end() const { return itsPages_.end(); }

    // Returns the total pages.
    unsigned int totalPages() const;

    // Clear all pages
    void clear() { itsPages_.clear(); }

    void resize(size_t pages) {
        size_t currentSize = itsPages_.size();
        // Shrink if needed
        if(pages < currentSize) {
            itsPages_.resize(pages, PlotMSPage(*this, 0));
        }
        // If we are adding new pages, initialize them
        for(size_t i = currentSize; i < pages; ++i) {
            insertPage(i);
            itsPages_[i].resize(itsPages_[0].canvasRows(),
                                itsPages_[0].canvasCols());
            itsPages_[i].setupPage();
        }
    }

    // Copy operator.
    PlotMSPages& operator=(const PlotMSPages& copy);

    // Iterators
    void firstPage();
    void nextPage();
    void previousPage();
    void lastPage();

    // Inserts a new page at the given index, and returns it.  If the given
    // index is invalid, the page is inserted at the end.
    PlotMSPage insertPage(int index = -1);
    
    // Clears all pages.
    void clearPages();
    
    // Sets up the current page (see PlotMSPage::setupPage()).
    void setupCurrentPage();

private:
    // Plot manager.
    PlotMSPlotManager* itsManager_;

    // Pages.
    vector<PlotMSPage> itsPages_;

    // Current page number.
    unsigned int itsCurrentPageNum_;
};

}

#endif /* PLOTMSPAGE_H_ */
