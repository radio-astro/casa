//# PlotMSOptions.cc: Option objects for plotms.
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
#include <msvis/PlotMS/PlotMSOptions.h>

namespace casa {

/////////////////////////////////
// PLOTMSSELECTION DEFINITIONS //
/////////////////////////////////

PlotMSSelection::PlotMSSelection() {
    initDefaults();
}

PlotMSSelection::~PlotMSSelection() { }

String PlotMSSelection::getString(Field f) const {
    if(fieldType(f) == TString)
        return const_cast<map<Field,String>&>(itsStringValues_)[f];
    else return "";
}

void PlotMSSelection::setString(Field f, const String& value) {
    if(fieldType(f) == TString) itsStringValues_[f] = value;
}

int PlotMSSelection::getInt(Field f) const {
    if(fieldType(f) == TInt)
        return const_cast<map<Field,int>&>(itsIntValues_)[f];
    else return 0;
}

void PlotMSSelection::setInt(Field f, int value) {
    if(fieldType(f) == TInt) itsIntValues_[f] = value;
}

MRadialVelocity PlotMSSelection::getMRadVel(Field f) const {
    if(fieldType(f) == TMRadVel)
        return const_cast<map<Field,MRadialVelocity>&>(itsMRadVelValues_)[f];
    else return MRadialVelocity();
}

void PlotMSSelection::setMRadVel(Field f, const MRadialVelocity& value) {
    if(fieldType(f) == TMRadVel) itsMRadVelValues_[f] = value;
}

bool PlotMSSelection::operator==(const PlotMSSelection& other) const {
    /*
    return itsStringValues_ == other.itsStringValues_ &&
           itsIntValues_ == other.itsIntValues_ &&
           itsMRadVelValues_ == other.itsMRadVelValues_;
           */
    // TODO
    return false;
}

void PlotMSSelection::initDefaults() {
    vector<Field> f = fields();
    Type t;
    for(unsigned int i = 0; i < f.size(); i++) {
        t = fieldType(f[i]);
        switch(t) {
        case TString:
            itsStringValues_[f[i]] = defaultStringValue(f[i]); break;
        case TInt:
            itsIntValues_[f[i]] = defaultIntValue(f[i]); break;
        case TMRadVel:
            itsMRadVelValues_[f[i]] = defaultMRadVelValue(f[i]); break;
            
        default: break;
        }
    }
}


//////////////////////////////////
// PLOTMSPARAMETERS DEFINITIONS //
//////////////////////////////////

const PMS::Axis PlotMSParameters::DEFAULT_XAXIS = PMS::TIME;
const PMS::Axis PlotMSParameters::DEFAULT_YAXIS = PMS::AMP;
const PMS::DataColumn PlotMSParameters::DEFAULT_DATACOLUMN = PMS::DATA;
const bool PlotMSParameters::DEFAULT_SHOWAXIS = true;
const bool PlotMSParameters::DEFAULT_GRIDMAJOR = false;
const bool PlotMSParameters::DEFAULT_GRIDMINOR = false;
const bool PlotMSParameters::DEFAULT_SHOWLEGEND = false;
const PlotCanvas::LegendPosition PlotMSParameters::DEFAULT_LEGENDPOSITION =
                                 PlotCanvas::INT_URIGHT;

PlotMSParameters::PlotMSParameters() {
    setDefaults();
}

PlotMSParameters::PlotMSParameters(const String& filename, PMS::Axis xAxis,
        PMS::Axis yAxis, bool setFlag) {
    setDefaults();
    itsMSFilename_ = filename;
    itsXAxis_ = xAxis;
    itsYAxis_ = yAxis;
    isSet_ = setFlag;
}

PlotMSParameters::~PlotMSParameters() { }

bool PlotMSParameters::isSet() const { return isSet_; }
void PlotMSParameters::setIsSet(bool set) { isSet_ = set; }

const String& PlotMSParameters::getFilename() const { return itsMSFilename_; }
void PlotMSParameters::setFilename(const String& filename, bool setFlag) {
    itsMSFilename_ = filename;
    isSet_ |= setFlag;
}

PMS::Axis PlotMSParameters::getXAxis() const { return itsXAxis_; }
PMS::Axis PlotMSParameters::getYAxis() const { return itsYAxis_; }
void PlotMSParameters::setXAxis(PMS::Axis x, bool setFlag) {
    itsXAxis_ = x;
    isSet_ |= setFlag;
}
void PlotMSParameters::setYAxis(PMS::Axis y, bool setFlag) {
    itsYAxis_ = y;
    isSet_ |= setFlag;
}

PMS::DataColumn PlotMSParameters::getXDataColumn() const {
    return itsXDataColumn_; }
void PlotMSParameters::setXDataColumn(PMS::DataColumn dc, bool setFlag) {
    itsXDataColumn_ = dc;
    isSet_ |= setFlag;
}
PMS::DataColumn PlotMSParameters::getYDataColumn() const {
    return itsYDataColumn_; }
void PlotMSParameters::setYDataColumn(PMS::DataColumn dc, bool setFlag) {
    itsYDataColumn_ = dc;
    isSet_ |= setFlag;
}

bool PlotMSParameters::showXAxis() const { return itsShowXAxis_; }
bool PlotMSParameters::showYAxis() const { return itsShowYAxis_; }
void PlotMSParameters::setShowAxes(bool showX, bool showY) {
    itsShowXAxis_ = showX;
    itsShowYAxis_ = showY;
}

bool PlotMSParameters::getGridXMajor() const { return itsGridXMaj_; }
bool PlotMSParameters::getGridXMinor() const { return itsGridXMin_; }
bool PlotMSParameters::getGridYMajor() const { return itsGridYMaj_; }
bool PlotMSParameters::getGridYMinor() const { return itsGridYMin_; }
void PlotMSParameters::setGrid(bool xMaj, bool xMin, bool yMaj, bool yMin) {
    itsGridXMaj_ = xMaj;
    itsGridXMin_ = xMin;
    itsGridYMaj_ = yMaj;
    itsGridYMin_ = yMin;
}

bool PlotMSParameters::showLegend() const { return itsShowLegend_; }
void PlotMSParameters::setShowLegend(bool show) { itsShowLegend_ = show; }
PlotCanvas::LegendPosition PlotMSParameters::getLegendPosition() const {
    return itsLegendPos_; }
void PlotMSParameters::setLegendPosition(PlotCanvas::LegendPosition pos) {
    itsLegendPos_ = pos; }

const String& PlotMSParameters::getCanvasTitle() const {
    return itsCanvasTitle_; }
void PlotMSParameters::setCanvasTitle(const String& title) {
    itsCanvasTitle_ = title; }


bool PlotMSParameters::equalsData(const PlotMSParameters& other) const {
    if(isSet_ != other.isSet_) return false;
    if(!isSet_) return true;
    return itsMSFilename_ == other.itsMSFilename_ &&
           itsMSSelection_ == other.itsMSSelection_ &&
           itsXAxis_ == other.itsXAxis_ && itsYAxis_ == other.itsYAxis_ &&
           itsXDataColumn_ == other.itsXDataColumn_ &&
           itsYDataColumn_ == other.itsYDataColumn_;
}

bool PlotMSParameters::equalsSettings(const PlotMSParameters& other) const {
    return itsCanvasTitle_ == other.itsCanvasTitle_ &&
           itsShowXAxis_ == other.itsShowXAxis_ &&
           itsShowYAxis_ == other.itsShowYAxis_ &&
           itsGridXMaj_ == other.itsGridXMaj_ &&
           itsGridXMin_ == other.itsGridXMin_ &&
           itsGridYMaj_ == other.itsGridYMaj_ &&
           itsGridYMin_ == other.itsGridYMin_ &&
           itsShowLegend_ == other.itsShowLegend_ &&
           (!itsShowLegend_ || itsLegendPos_ == other.itsLegendPos_);
}

bool PlotMSParameters::operator==(const PlotMSParameters& other) const {
    return equalsData(other) && equalsSettings(other);
}


void PlotMSParameters::setDefaults() {
    isSet_ = false;
    itsMSFilename_ = "";
    itsXAxis_ = DEFAULT_XAXIS;
    itsYAxis_ = DEFAULT_YAXIS;
    itsXDataColumn_ = itsYDataColumn_ = DEFAULT_DATACOLUMN;
    itsShowXAxis_ = itsShowYAxis_ = DEFAULT_SHOWAXIS;
    itsGridXMaj_ = itsGridYMaj_ = DEFAULT_GRIDMAJOR;
    itsGridXMin_ = itsGridYMin_ = DEFAULT_GRIDMINOR;
    itsShowLegend_ = DEFAULT_SHOWLEGEND;
    itsLegendPos_ = DEFAULT_LEGENDPOSITION;
    itsCanvasTitle_ = "";
}


///////////////////////////////
// PLOTMSOPTIONS DEFINITIONS //
///////////////////////////////

PlotMSOptions::PlotMSOptions(unsigned int nrows, unsigned int ncols) {
    setDefaults();
    if(nrows < 1) nrows = 1;
    if(ncols < 1) ncols = 1;
    if(nrows != DEFAULT_ROWS || ncols != DEFAULT_COLS) setSize(nrows, ncols);
}

PlotMSOptions::~PlotMSOptions() { }

void PlotMSOptions::setSize(unsigned int rows, unsigned int cols,
        bool clearParams) {
    if(rows < 1) rows = 1;
    if(cols < 1) cols = 1;
    if(rows == itsRows_ && cols == itsCols_ && !clearParams) return;
    if(clearParams) itsParameters_.clear();
    itsParameters_.resize(rows);
    for(unsigned int i = 0; i < rows; i++) itsParameters_[i].resize(cols);
    itsRows_ = rows;
    itsCols_ = cols;
}

void PlotMSOptions::setSpacing(unsigned int spacing) { itsSpacing_ = spacing; }

const PlotMSParameters&
PlotMSOptions::parameters(unsigned int row, unsigned int col) const {
    if(row >= itsRows_) row = itsRows_ - 1;
    if(col >= itsCols_) col = itsCols_ - 1;
    return itsParameters_[row][col];
}
PlotMSParameters& PlotMSOptions::parameters(unsigned int row,unsigned int col){
    if(row >= itsRows_) row = itsRows_ - 1;
    if(col >= itsCols_) col = itsCols_ - 1;
    return itsParameters_[row][col];
}

const PlotMSParameters& PlotMSOptions::parameters(unsigned int index) const {
    if(index >= (itsRows_ * itsCols_)) index = (itsRows_ * itsCols_) - 1;
    return itsParameters_[index / itsCols_][index % itsCols_];
}
PlotMSParameters& PlotMSOptions::parameters(unsigned int index) {
    if(index >= (itsRows_ * itsCols_)) index = (itsRows_ * itsCols_) - 1;
    return itsParameters_[index / itsCols_][index % itsCols_];
}

void PlotMSOptions::setParameters(unsigned int row, unsigned int col,
        const PlotMSParameters& params) {
    if(row >= itsRows_) row = itsRows_ - 1;
    if(col >= itsCols_) col = itsCols_ - 1;
    itsParameters_[row][col] = params;
}

void PlotMSOptions::setParameters(const vector<vector<PlotMSParameters> >& p) {
    if(p.size() == 0) return;
    unsigned int cols = p[0].size();
    for(unsigned int i = 1; i < p.size(); i++)
        if(p[i].size() < cols) cols = p[i].size();
    if(cols == 0) return;
    
    itsRows_ = p.size();
    itsCols_ = cols;
    itsParameters_.resize(itsRows_);
    for(unsigned int i = 0; i < itsRows_; i++) {
        itsParameters_[i].resize(itsCols_);
        for(unsigned int j = 0; j < itsCols_; j++)
            itsParameters_[i][j] = p[i][j];
    }
}

bool PlotMSOptions::operator==(const PlotMSOptions& other) const {
    if(itsRows_ != other.itsRows_ || itsCols_ != other.itsCols_ ||
       itsSpacing_ != other.itsSpacing_) return false;
    
    for(unsigned int i = 0; i < itsRows_; i++)
        for(unsigned int j = 0; j < itsCols_; j++)
            if(itsParameters_[i][j]!= other.itsParameters_[i][j]) return false;

    return true;
}

void PlotMSOptions::setDefaults() {
    itsRows_ = DEFAULT_ROWS;
    itsCols_ = DEFAULT_COLS;
    itsSpacing_ = DEFAULT_SPACING;
    itsParameters_.clear();
    itsParameters_.resize(itsRows_);
    for(unsigned int i = 0; i < itsRows_; i++)
        itsParameters_[i].resize(itsCols_);
}

const unsigned int PlotMSOptions::DEFAULT_ROWS = 1;
const unsigned int PlotMSOptions::DEFAULT_COLS = 1;
const unsigned int PlotMSOptions::DEFAULT_SPACING = 0;

}

