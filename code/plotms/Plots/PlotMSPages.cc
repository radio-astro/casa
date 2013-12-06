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

#include "PlotMSPages.h"
#include <plotms/Plots/PlotMSPlotManager.h>

namespace casa {

/////////////////////////////
// PLOTMSPAGES DEFINITIONS //
/////////////////////////////

// Public Methods //

PlotMSPages::PlotMSPages(const PlotMSPages& copy) {
    operator=(copy); }

PlotMSPages::~PlotMSPages() { }


unsigned int PlotMSPages::currentPageNumber() const {
    return itsCurrentPageNum_; }

PlotMSPage PlotMSPages::currentPage() const {
    return itsPages_[itsCurrentPageNum_]; }

unsigned int PlotMSPages::totalPages() const { return itsPages_.size(); }


PlotMSPages& PlotMSPages::operator=(const PlotMSPages& copy) {
    itsManager_ = copy.itsManager_;
    itsPages_ = copy.itsPages_;
    itsCurrentPageNum_ = copy.itsCurrentPageNum_;
    if(itsPages_.empty()) {
        insertPage();
        itsCurrentPageNum_ = 0;
    }
    return *this;
}

void PlotMSPages::firstPage() {
    itsCurrentPageNum_ = 0;
}

void PlotMSPages::nextPage() {
    if(itsCurrentPageNum_ < (totalPages() - 1)) {
        ++itsCurrentPageNum_;
    }
}

void PlotMSPages::previousPage() {
    if(itsCurrentPageNum_ > 0) {
        --itsCurrentPageNum_;
    }
}

void PlotMSPages::lastPage() {
    if(totalPages() > 0) {
        itsCurrentPageNum_ = totalPages() - 1;
    }
}


// Private Methods //

PlotMSPages::PlotMSPages(PlotMSPlotManager& manager) : itsManager_(&manager),
        itsCurrentPageNum_(0) {
    insertPage(); }


PlotMSPage PlotMSPages::insertPage(int index) {
    PlotMSPage page(*this);
    if(index >= 0 && (unsigned int)index < itsPages_.size()) {
        itsPages_.insert(itsPages_.begin() + index, page);
        if((unsigned int)index <= itsCurrentPageNum_) itsCurrentPageNum_++;
    }
    else {
        itsPages_.push_back(page);
    }
    return page;
}

void PlotMSPages::clearPages() {
    itsManager_->plotter()->setCanvasLayout(PlotCanvasLayoutPtr());
    for(unsigned int i = 0; i < itsPages_.size(); i++)
        itsPages_[i].resize(0, 0);
}

void PlotMSPages::setupCurrentPage() {
    itsPages_[itsCurrentPageNum_].setupPage();

}



} /* namespace PLOTMSPAGES */
