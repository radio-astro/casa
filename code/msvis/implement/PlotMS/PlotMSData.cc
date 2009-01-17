//# PlotMSData.cc: Connects data cache and plotting data objects.
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
//#
//# $Id:  $
#include <msvis/PlotMS/PlotMSData.h>

namespace casa {

////////////////////////////
// PLOTMSDATA DEFINITIONS //
////////////////////////////

PlotMSData::PlotMSData() { }

PlotMSData::~PlotMSData() { }

bool PlotMSData::isValid() const { return itsCache_.readyForPlotting(); }

unsigned int PlotMSData::size() const {
    return (unsigned int)itsCache_.nPoints();
}

double PlotMSData::xAt(unsigned int i) const {
    return const_cast<PMSMemCache&>(itsCache_).getX((int)i); }

double PlotMSData::yAt(unsigned int i) const {
    return const_cast<PMSMemCache&>(itsCache_).getY((int)i); }

bool PlotMSData::minsMaxes(double& xMin, double& xMax, double& yMin,
        double& yMax) {
    if(itsCache_.nPoints() > 0) {
        itsCache_.getRanges(xMin, xMax, yMin, yMax);
        return true;
    } else return false;
}

void PlotMSData::setupCache(PMS::Axis xAxis, PMS::Axis yAxis) {
    itsCache_.setUpPlot(xAxis, yAxis);
}

void PlotMSData::loadCache(VisSet& visSet, PMS::Axis xAxis, PMS::Axis yAxis) {
    VisIter& vi(visSet.iter());
    VisBuffer vb(vi);
    
    // count number of chunks
    int chunk = 0;
    for(vi.originChunks(); vi.moreChunks(); vi.nextChunk())
        for (vi.origin(); vi.more(); vi++) chunk++;
    itsCache_.increaseChunks(chunk);
    
    // append visbuffers
    chunk = 0;
    for(vi.originChunks(); vi.moreChunks(); vi.nextChunk())
        for(vi.origin(); vi.more(); vi++)
            itsCache_.append(vb, chunk++, xAxis, yAxis);
}

/*
void PlotMSData::loadCache(VisSet& visSet, const vector<PMS::Axis>& axes) {
    VisIter& vi(visSet.iter());
    VisBuffer vb(vi);
    
    // count number of chunks
    int chunk = 0;
    for(vi.originChunks(); vi.moreChunks(); vi.nextChunk())
        for (vi.origin(); vi.more(); vi++) chunk++;
    itsCache_.increaseChunks(chunk);
    
    // append visbuffers
    chunk = 0;
    for(vi.originChunks(); vi.moreChunks(); vi.nextChunk())
        for(vi.origin(); vi.more(); vi++) itsCache_.append(vb, chunk++);
}
*/

void PlotMSData::locate(double xMin, double xMax, double yMin, double yMax) {
    itsCache_.locateRange(xMin, xMax, yMin, yMax);
}


/////////////////////////////
// PLOTMSDATAS DEFINITIONS //
/////////////////////////////

PlotMSDatas::PlotMSDatas(unsigned int rows, unsigned int cols) {
    if(rows < 1) rows = 1;
    if(cols < 1) cols = 1;
    
    itsData_.resize(rows);
    for(unsigned int i = 0; i < rows; i++) {
        itsData_[i].resize(cols);
        for(unsigned int j = 0; j < cols; j++)
            itsData_[i][j] = new PlotMSData();
    }
    itsRows_ = rows;
    itsCols_ = cols;
}

PlotMSDatas::~PlotMSDatas() {
    clearData();
}

unsigned int PlotMSDatas::nrows() const { return itsRows_; }
unsigned int PlotMSDatas::ncols() const { return itsCols_; }

void PlotMSDatas::setRowsCols(unsigned int rows, unsigned int cols,
        bool clear) {
    if(rows < 1) rows = 1;
    if(cols < 1) cols = 1;
    if(clear) clearData();
    
    vector<PlotMSData*> oldData;
    for(unsigned int i = 0; i < itsData_.size(); i++)
        for(unsigned int j = 0; j < itsData_[i].size(); j++)
            oldData.push_back(itsData_[i][j]);
    
    unsigned int index = 0;
    itsData_.resize(rows);
    for(unsigned int i = 0; i < rows; i++) {
        itsData_[i].resize(cols);
        for(unsigned int j = 0; j < cols; j++) {
            if(index < oldData.size()) itsData_[i][j] = oldData[index++];
            else                       itsData_[i][j] = new PlotMSData();
        }
    }
    
    if(index < oldData.size()) {
        // delete any unused
        vector<PlotMSData*> del;
        bool seen;
        for(unsigned int i = index; i < oldData.size(); i++) {
            // make sure it wasn't used
            seen = false;
            for(unsigned int j = 0; !seen && j < index; j++)
                if(oldData[j] == oldData[i]) seen = true;
            if(seen) continue;
            
            // make sure it's not already in del
            seen = false;
            for(unsigned int j = 0; !seen && j < del.size(); j++)
                if(del[j] == oldData[i]) seen = true;
            if(!seen) del.push_back(oldData[i]);
        }
        for(unsigned int i = 0; i < del.size(); i++) delete del[i];
    }
    
    itsRows_ = rows;
    itsCols_ = cols;
}

unsigned int PlotMSDatas::size(unsigned int row, unsigned int col) const {
    if(row >= itsRows_) row = itsRows_ - 1;
    if(col >= itsCols_) col = itsCols_ - 1;
    return itsData_[row][col]->size();
}

PlotMSData& PlotMSDatas::getData(unsigned int row, unsigned int col) {
    if(row >= itsRows_) row = itsRows_ - 1;
    if(col >= itsCols_) col = itsCols_ - 1;
    return *itsData_[row][col];
}

void PlotMSDatas::loadCache(VisSet& visSet, PMS::Axis xAxis, PMS::Axis yAxis,
        unsigned int row, unsigned int col) {
    if(row >= itsRows_) row = itsRows_ - 1;
    if(col >= itsCols_) col = itsCols_ - 1;
    itsData_[row][col]->loadCache(visSet, xAxis, yAxis);
}

/*
void PlotMSDatas::loadCache(VisSet& visSet, const vector<PMS::Axis>& axes,
        unsigned int row, unsigned int col) {
    if(row >= itsRows_) row = itsRows_ - 1;
    if(col >= itsCols_) col = itsCols_ - 1;
    itsData_[row][col]->loadCache(visSet, axes);
}
*/

void PlotMSDatas::setupCache(PMS::Axis xAxis, PMS::Axis yAxis, unsigned int r,
        unsigned int c) {
    if(r >= itsRows_) r = itsRows_ - 1;
    if(c >= itsCols_) c = itsCols_ - 1;
    itsData_[r][c]->setupCache(xAxis, yAxis);
}

void PlotMSDatas::locate(double xMin, double xMax, double yMin, double yMax,
        int row, int col) {
    if(row < 0 || col < 0) {
        for(unsigned int r = 0; r < itsRows_; r++)
            for(unsigned int c = 0; c < itsCols_; c++)
                itsData_[r][c]->locate(xMin, xMax, yMin, yMax);
    } else {
        unsigned int r = (unsigned int)row, c = (unsigned int)col;
        if(r >= itsRows_) r = itsRows_ - 1;
        if(c >= itsCols_) c = itsCols_ - 1;
        itsData_[r][c]->locate(xMin, xMax, yMin, yMax);
    }
}

void PlotMSDatas::clearData() {
    vector<PlotMSData*> seenPtrs;
    PlotMSData* d; bool seen;
    for(unsigned int r = 0; r < itsData_.size(); r++) {
        for(unsigned int c = 0; c < itsData_[r].size(); c++) {
            d = itsData_[r][c];
            seen = false;
            for(unsigned int i = 0; !seen && i < seenPtrs.size(); i++)
                if(seenPtrs[i] == d) seen = true;
            if(!seen) seenPtrs.push_back(d);
        }
    }
    for(unsigned int i = 0; i < seenPtrs.size(); i++)
        delete seenPtrs[i];
    itsData_.clear();
}

}
