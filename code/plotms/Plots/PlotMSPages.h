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

#ifndef PLOTMSPAGES_H_
#define PLOTMSPAGES_H_

#include <plotms/Plots/PlotMSPage.h>

namespace casa {

class PlotMSParameters;

// Represents (potentially) multiple pages for PlotMS, with one being current
// (visible) at a time.
class PlotMSPages {

    //# Friend class declarations.
    friend class PlotMSPage;
    friend class PlotMSPlot;
    friend class PlotMSPlotManager;

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

    void setCurrentPageNum(uInt num);

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

    void resize(size_t pages);

    //Erase all traces of a plot at the specific location including removing axes and title.
    void clearCanvas( int row, int col );

	//Resize the page to the current number of rows and columns.
    bool gridChanged( int rows, int cols);

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

    //Remove the plot from the canvas.
    void disown( PlotMSPlot* plot );

    PlotMSParameters getPageParameters();
    
    //Returns whether the spot at the given location is available for
    //the plot (either empty or the plot is already occupying it).
    bool isSpot( int rowIndex, int colIndex, PlotMSPlot* plot ) const;
    
    //Returns the row and column index of the first canvas on the
    //page, or <-1,-1> if the page is full.
    pair<int,int> findEmptySpot() const;

private:
	//Returns whether or not (rows,cols) would represent
	//a change in the current page size.
    bool isGridChanged( int rows, int cols ) const;

    // Plot manager.
    PlotMSPlotManager* itsManager_;

    // Pages.
    vector<PlotMSPage> itsPages_;

    // Current page number.
    unsigned int itsCurrentPageNum_;
};



} /* namespace ff */
#endif /* PLOTMSPAGES_H_ */
