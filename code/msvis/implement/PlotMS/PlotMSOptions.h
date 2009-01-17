//# PlotMSOptions.h: Option objects for plotms.
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
#ifndef PLOTMSOPTIONS_H_
#define PLOTMSOPTIONS_H_

#include <casa/Arrays/Vector.h>
#include <graphics/GenericPlotter/PlotCanvas.h>
#include <measures/Measures/MRadialVelocity.h>
#include <msvis/PlotMS/PlotMSConstants.h>

#include <map>

namespace casa {

// MS Selection object.
class PlotMSSelection {
public:
    // Static //
    
    PMS_ENUM1(Field, fields, fieldStrings, field,
              TIME, SPW, SCAN, FIELD, BASELINE, UVRANGE, CHANMODE, NCHAN,
              START, STEP, MSTART, MSTEP, MSSELECT)
    PMS_ENUM2(Field, fields, fieldStrings, field,
              "time", "spw", "scan", "field", "baseline", "uvrange",
              "chanmode", "nchan", "start", "step", "mStart", "mStep", "msSelect")
    
    enum Type {
        TString, TInt, TMRadVel
    };
    
    static Type fieldType(Field f) {
        switch(f) {
        case SPW: case SCAN: case FIELD: case BASELINE: case UVRANGE:
        case CHANMODE: case MSSELECT: return TString;
        case NCHAN: case START: case STEP: return TInt;
        case MSTART: case MSTEP: return TMRadVel;
        
        default: return TString;
        }
    }
    
    static String defaultStringValue(Field f) {
        switch(f) {
        case SPW: case SCAN: case FIELD: case BASELINE: case UVRANGE:
        case MSSELECT: return "";
        
        case CHANMODE: return "none";
        
        default: return "";
        }
    }
    
    static int defaultIntValue(Field f) {
        switch(f) {
        case NCHAN: case STEP: return 1;
        
        case START: return 0;
        
        default: return 0;
        }
    }
    
    static MRadialVelocity defaultMRadVelValue(Field f) {
        switch(f) {
        case MSTART: case MSTEP: return MRadialVelocity();
        
        default: return MRadialVelocity();
        }
    }
    
    
    // Non-Static //
    
    PlotMSSelection();
    
    ~PlotMSSelection();
    
    String getString(Field f) const;
    void setString(Field f, const String& value);
    int getInt(Field f) const;
    void setInt(Field f, int value);
    MRadialVelocity getMRadVel(Field f) const;
    void setMRadVel(Field f, const MRadialVelocity& value);
    
    String time() const            { return getString(TIME);     }
    String spw() const             { return getString(SPW);      }
    String scan() const            { return getString(SCAN);     }
    String field() const           { return getString(FIELD);    }
    String baseline() const        { return getString(BASELINE); }
    String uvrange() const         { return getString(UVRANGE);  }
    String chanmode() const        { return getString(CHANMODE); }
    String msSelect() const        { return getString(MSSELECT); }
    int nchan() const              { return getInt(NCHAN);       }
    int start() const              { return getInt(START);       }
    int step() const               { return getInt(STEP);        }
    MRadialVelocity mStart() const { return getMRadVel(MSTART);  }
    MRadialVelocity mStep() const  { return getMRadVel(MSTEP);   }
    
    void setTime(const String& v)     { setString(TIME, v);     }
    void setSpw(const String& v)      { setString(SPW, v);      }
    void setScan(const String& v)     { setString(SCAN, v);     }
    void setField(const String& v)    { setString(FIELD, v);    }
    void setBaseline(const String& v) { setString(BASELINE, v); }
    void setUvrange(const String& v)  { setString(UVRANGE, v);  }
    void setChanmode(const String& v) { setString(CHANMODE, v); }
    void setMsSelect(const String& v) { setString(MSSELECT, v); }
    void setNchan(int v)              { setInt(NCHAN, v);       }
    void setStart(int v)              { setInt(START, v);       }
    void setStep(int v)               { setInt(STEP, v);        }
    void setMStart(const MRadialVelocity& v) { setMRadVel(MSTART, v); }
    void setMStep(const MRadialVelocity& v)  { setMRadVel(MSTEP, v);  }
    
    bool operator==(const PlotMSSelection& other) const;
    bool operator!=(const PlotMSSelection& other) const {
        return !(operator==(other)); }
    
private:
    map<Field, String> itsStringValues_;
    map<Field, int> itsIntValues_;
    map<Field, MRadialVelocity> itsMRadVelValues_;
    
    void initDefaults();
};


// Parameters for a single plot canvas.
class PlotMSParameters {
public:
    static const PMS::Axis DEFAULT_XAXIS;
    static const PMS::Axis DEFAULT_YAXIS;
    static const PMS::DataColumn DEFAULT_DATACOLUMN;
    static const bool DEFAULT_SHOWAXIS;
    static const bool DEFAULT_GRIDMAJOR;
    static const bool DEFAULT_GRIDMINOR;
    static const bool DEFAULT_SHOWLEGEND;
    static const PlotCanvas::LegendPosition DEFAULT_LEGENDPOSITION;
    
    PlotMSParameters();
    
    PlotMSParameters(const String& filename, PMS::Axis xAxis = DEFAULT_XAXIS,
                     PMS::Axis yAxis = DEFAULT_YAXIS, bool setFlag = true);
    
    ~PlotMSParameters();
    
    // Whether or not parameters have been set or not.  Unset parameters would
    // theoretically lead to a blank plot canvas.
    // <group>
    bool isSet() const;
    void setIsSet(bool set = true);
    // </group>
    
    // MS filename
    const String& getFilename() const;
    void setFilename(const String& filename, bool setFlag = true);
    
    // Axes
    PMS::Axis getXAxis() const;
    PMS::Axis getYAxis() const;
    void setXAxis(PMS::Axis x, bool setFlag = false);
    void setYAxis(PMS::Axis y, bool setFlag = false);
    void setAxes(PMS::Axis x, PMS::Axis y, bool setFlag = false) {
        setXAxis(x);
        setYAxis(y);
    }
    
    // Data column
    PMS::DataColumn getXDataColumn() const;
    PMS::DataColumn getYDataColumn() const;
    void setXDataColumn(PMS::DataColumn dc, bool setFlag = false);
    void setYDataColumn(PMS::DataColumn dc, bool setFlag = false);
    void setDataColumns(PMS::DataColumn x, PMS::DataColumn y,
                        bool setFlag = false) {
        setXDataColumn(x);
        setYDataColumn(y);
    }
    
    // Show/hide axes
    // <group>
    bool showXAxis() const;
    bool showYAxis() const;
    void setShowAxes(bool showX, bool showY);
    // </group>
    
    // Grid
    // <group>
    bool getGridXMajor() const;
    bool getGridXMinor() const;
    bool getGridYMajor() const;
    bool getGridYMinor() const;
    void setGrid(bool xMaj, bool xMin, bool yMaj, bool yMin);
    // </group>
    
    // Legend
    // <group>
    bool showLegend() const;
    void setShowLegend(bool show);
    PlotCanvas::LegendPosition getLegendPosition() const;
    void setLegendPosition(PlotCanvas::LegendPosition pos);
    // </group>
    
    // Canvas title
    // <group>
    const String& getCanvasTitle() const;
    void setCanvasTitle(const String& title);
    // </group>
    
    // Operators //
    
    bool equalsData(const PlotMSParameters& other) const;
    bool equalsSettings(const PlotMSParameters& other) const;
    bool equalsAll(const PlotMSParameters& other) const {
        return equalsData(other) && equalsSettings(other);
    }
    
    bool operator==(const PlotMSParameters& other) const;    
    bool operator!=(const PlotMSParameters& other) const {
        return !(*this == other);
    }
    
private:
    bool isSet_;
    String itsMSFilename_;
    PlotMSSelection itsMSSelection_;
    PMS::Axis itsXAxis_, itsYAxis_;
    PMS::DataColumn itsXDataColumn_, itsYDataColumn_;
    bool itsShowXAxis_, itsShowYAxis_;
    bool itsGridXMaj_, itsGridXMin_, itsGridYMaj_, itsGridYMin_;
    bool itsShowLegend_;
    PlotCanvas::LegendPosition itsLegendPos_;
    
    String itsCanvasTitle_;
    
    void setDefaults();
};


// Parameters for the whole plotter.
class PlotMSOptions {
public:
    static const unsigned int DEFAULT_ROWS;
    static const unsigned int DEFAULT_COLS;
    static const unsigned int DEFAULT_SPACING;
    
    
    PlotMSOptions(unsigned int nrows = DEFAULT_ROWS,
                  unsigned int ncols = DEFAULT_COLS);
    
    ~PlotMSOptions();
    
    unsigned int nrows() const   { return itsRows_; }
    unsigned int ncols() const   { return itsCols_; }
    unsigned int spacing() const { return itsSpacing_; }
    
    void setSize(unsigned int rows, unsigned int cols, bool clearParams= true);
    void setSpacing(unsigned int spacing);
    
    const PlotMSParameters& parameters(unsigned int row,unsigned int col)const;
    PlotMSParameters& parameters(unsigned int row, unsigned int col);
    const PlotMSParameters& parameters(unsigned int index) const;
    PlotMSParameters& parameters(unsigned int index);
    
    void setParameters(unsigned int row, unsigned int col,
                       const PlotMSParameters& params);
    void setParameters(const vector<vector<PlotMSParameters> >& params);
    
    // Operators //
    
    bool operator==(const PlotMSOptions& other) const;    
    bool operator!=(const PlotMSOptions& other) const {
        return !(*this == other);
    }
    
private:
    unsigned int itsRows_, itsCols_;
    unsigned int itsSpacing_;
    vector<vector<PlotMSParameters> > itsParameters_;
    
    void setDefaults();
};

}

#endif /* PLOTMSOPTIONS_H_ */
