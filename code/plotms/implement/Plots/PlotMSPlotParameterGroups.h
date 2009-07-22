//# PlotMSPlotParameterGroups.h: Implementations of plot subparameter groups.
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
#ifndef PLOTMSPLOTPARAMETERGROUPS_H_
#define PLOTMSPLOTPARAMETERGROUPS_H_

#include <plotms/Plots/PlotMSPlotParameters.h>

#include <plotms/PlotMS/PlotMSAveraging.h>
#include <plotms/PlotMS/PlotMSSelection.h>
#include <plotms/PlotMS/PlotMSLabelFormat.h>

#include <casa/namespace.h>

namespace casa {

// Container class to hold constants for groups.
class PMS_PP {
public:
    // Update flag for redrawing.
    // <group>
    static const String UPDATE_REDRAW_NAME;
    static const int UPDATE_REDRAW;
    // </group>
    
    // Update flag for MS data group.
    // <group>
    static const String UPDATE_MSDATA_NAME;
    static const int UPDATE_MSDATA;
    // </group>
    
    // Update flag for cache group.
    // <group>
    static const String UPDATE_CACHE_NAME;
    static const int UPDATE_CACHE;
    // </group>
    
    // Update flag for canvas group.
    // <group>
    static const String UPDATE_CANVAS_NAME;
    static const int UPDATE_CANVAS;
    // </group>
    
    // Update flag for display group.
    // <group>
    static const String UPDATE_DISPLAY_NAME;
    static const int UPDATE_DISPLAY;
    // </group>
    
    // Update flag for log group.
    // <group>
    static const String UPDATE_LOG_NAME;
    static const int UPDATE_LOG;
    // </group>
    
    // Update flag for plotms_options group.
    // <group>
    static const String UPDATE_PLOTMS_OPTIONS_NAME;
    static const int UPDATE_PLOTMS_OPTIONS;
    // </group>
    
private:
    // Disable constructor.
    PMS_PP() { }
};


// Helper macros for defining a PlotMSPlotParameters::Group subclass.
// <group>
#define PMS_PP_GROUP_START(CLASS, NAME, REQREDRAW)                            \
class CLASS : public PlotMSPlotParameters::Group {                            \
public:                                                                       \
    /* Constructor which takes a factory */                                   \
    CLASS(PlotFactoryPtr factory);                                            \
                                                                              \
    /* Copy constructor.  See operator=(). */                                 \
    CLASS(const CLASS& copy);                                                 \
                                                                              \
    /* Destructor. */                                                         \
    ~CLASS();                                                                 \
                                                                              \
    /* Implements PlotMSPlotParameters::Group::clone(). */                    \
    Group* clone() const { return new CLASS(*this); }                         \
                                                                              \
    /* Implements PlotMSPlotParameters::Group::name(). */                     \
    const String& name() const {                                              \
        static String groupName = NAME;                                       \
        return groupName; }                                                   \
                                                                              \
    /* Implements PlotMSPlotParameters::Group::toRecord(). */                 \
    Record toRecord() const;                                                  \
                                                                              \
    /* Implements PlotMSPlotParameters::Group::fromRecord(). */               \
    void fromRecord(const Record& record);                                    \
                                                                              \
    /* Implements PlotMSPlotParameters::Group::requiresRedrawOnChanged(). */  \
    bool requiresRedrawOnChange() const { return REQREDRAW; }                 \
                                                                              \
    /* Overrides PlotMSPlotParameters::Group::operator=(). */                 \
    Group& operator=(const Group& other);                                     \
                                                                              \
    /* Overrides PlotMSPlotParameters::Group::operator==(). */                \
    bool operator==(const Group& other) const;

#define PMS_PP_GROUP_PARAM(TYPE, PARAM, GETTER, SETTER, RECKEY)               \
public:                                                                       \
    /* Gets/Sets the value for the given parameter. */                        \
    /* <group> */                                                             \
    const TYPE& GETTER() const { return PARAM; }                              \
    void SETTER(const TYPE& value) {                                          \
        if(PARAM != value) {                                                  \
            PARAM = value;                                                    \
            updated();                                                        \
        }                                                                     \
    }                                                                         \
    /* </group> */                                                            \
                                                                              \
private:                                                                      \
    /* Parameter. */                                                          \
    TYPE PARAM;                                                               \
                                                                              \
    /* Record key for the indicated parameter. */                             \
    static const String RECKEY;
    
#define PMS_PP_GROUP_END                                                      \
private:                                                                      \
    /* Sets the parameter defaults. */                                        \
    void setDefaults();                                                       \
};
// </group>


// Subclass of PlotMSPlotParameters::Group to handle subparameters for MS data.
// Currently includes:
// * filename
// * selection
// * averaging
PMS_PP_GROUP_START(PMS_PP_MSData, PMS_PP::UPDATE_MSDATA_NAME, true)
public:
    // Returns true if an MS has been set, false otherwise.
    bool isSet() const { return !itsFilename_.empty(); }    
    
    PMS_PP_GROUP_PARAM(String, itsFilename_, filename, setFilename, REC_FILENAME)
    PMS_PP_GROUP_PARAM(PlotMSSelection, itsSelection_, selection, setSelection, REC_SELECTION)
    PMS_PP_GROUP_PARAM(PlotMSAveraging, itsAveraging_, averaging, setAveraging, REC_AVERAGING)
PMS_PP_GROUP_END
    
// Subclass of PlotMSPlotParameters::Group to handle cache parameters.
// Currently includes:
// * x and y axes
// * x and y data columns
PMS_PP_GROUP_START(PMS_PP_Cache, PMS_PP::UPDATE_CACHE_NAME, true)
public:
    // Sets the axes.
    // <group>
    void setXAxis(const PMS::Axis& axis, const PMS::DataColumn& data) {
        setAxes(axis, yAxis(), data, yDataColumn()); }
    void setYAxis(const PMS::Axis& axis, const PMS::DataColumn& data) {
        setAxes(xAxis(), axis, xDataColumn(), data); }
    void setAxes(const PMS::Axis& xAxis, const PMS::Axis& yAxis,
            const PMS::DataColumn& xData, const PMS::DataColumn& yData);
    // </group>
    
    PMS_PP_GROUP_PARAM(PMS::Axis, itsXAxis_, xAxis, setXAxis, REC_XAXIS)
    PMS_PP_GROUP_PARAM(PMS::Axis, itsYAxis_, yAxis, setYAxis, REC_YAXIS)
    PMS_PP_GROUP_PARAM(PMS::DataColumn, itsXData_, xDataColumn, setXDataColumn, REC_XDATACOL)
    PMS_PP_GROUP_PARAM(PMS::DataColumn, itsYData_, yDataColumn, setYDataColumn, REC_YDATACOL)
PMS_PP_GROUP_END
    
    
// Subclass of PlotMSPlotParameters::Group to handle canvas parameters.
// Currently includes:
// * which canvas axes x and y are attached to
// * reference values for the axes, if any
// * ranges for the axes, if any
// * axes label formats
// * whether to show the canvas axes or not
// * whether to show the legend or not, and its position
// * canvas title label format
// * whether to show grid lines, and their properties
PMS_PP_GROUP_START(PMS_PP_Canvas, PMS_PP::UPDATE_CANVAS_NAME, true)
public:    
    // Sets the axes range flags and values.
    // <group>
    void setXRange(const bool& set, const prange_t& range) {
        setRanges(set, yRangeSet(), range, yRange()); }
    void setYRange(const bool& set, const prange_t& range) {
        setRanges(xRangeSet(), set, xRange(), range); }
    void setRanges(const bool& xSet, const bool& ySet, const prange_t& xRange,
            const prange_t& yRange);
    // </group>
    
    // Sets the legend flag and position.
    void showLegend(const bool& show, const PlotCanvas::LegendPosition& pos);
    
    // Sets the grid flags and lines.
    // <group>
    void showGridMajor(const bool& show, const PlotLinePtr& line) {
        showGrid(show, gridMinorShown(), line, gridMinorLine()); }
    void showGridMinor(const bool& show, const PlotLinePtr& line) {
        showGrid(gridMajorShown(), show, gridMajorLine(), line); }
    void showGrid(const bool& showMajor, const bool& showMinor,
            const PlotLinePtr& majorLine, const PlotLinePtr& minorLine);
    // </group>    
    
    PMS_PP_GROUP_PARAM(PlotAxis, itsXAxis_, xAxis, setXAxis, REC_XAXIS)
    PMS_PP_GROUP_PARAM(PlotAxis, itsYAxis_, yAxis, setYAxis, REC_YAXIS)
    PMS_PP_GROUP_PARAM(bool, itsXRangeSet_, xRangeSet, setXRange, REC_XRANGESET)
    PMS_PP_GROUP_PARAM(bool, itsYRangeSet_, yRangeSet, setYRange, REC_YRANGESET)
    PMS_PP_GROUP_PARAM(prange_t, itsXRange_, xRange, setXRange, REC_XRANGE)
    PMS_PP_GROUP_PARAM(prange_t, itsYRange_, yRange, setYRange, REC_YRANGE)
    PMS_PP_GROUP_PARAM(PlotMSLabelFormat, itsXLabel_, xLabelFormat, setXLabelFormat, REC_XLABEL)
    PMS_PP_GROUP_PARAM(PlotMSLabelFormat, itsYLabel_, yLabelFormat, setYLabelFormat, REC_YLABEL)
    PMS_PP_GROUP_PARAM(bool, itsXAxisShown_, xAxisShown, showXAxis, REC_SHOWXAXIS)
    PMS_PP_GROUP_PARAM(bool, itsYAxisShown_, yAxisShown, showYAxis, REC_SHOWYAXIS)
    PMS_PP_GROUP_PARAM(bool, itsLegendShown_, legendShown, showLegend, REC_SHOWLEGEND)
    PMS_PP_GROUP_PARAM(PlotCanvas::LegendPosition, itsLegendPos_, legendPosition, showLegend, REC_LEGENDPOS)
    PMS_PP_GROUP_PARAM(PlotMSLabelFormat, itsTitle_, titleFormat, setTitleFormat, REC_TITLE)
    PMS_PP_GROUP_PARAM(bool, itsGridMajShown_, gridMajorShown, showGridMajor, REC_SHOWGRIDMAJ)
    PMS_PP_GROUP_PARAM(bool, itsGridMinShown_, gridMinorShown, showGridMinor, REC_SHOWGRIDMIN)
    PMS_PP_GROUP_PARAM(PlotLinePtr, itsGridMajLine_, gridMajorLine, setGridMajorLine, REC_GRIDMAJLINE)
    PMS_PP_GROUP_PARAM(PlotLinePtr, itsGridMinLine_, gridMinorLine, setGridMinorLine, REC_GRIDMINLINE)
PMS_PP_GROUP_END


// Subclass of PlotMSPlotParameters::Group to handle display parameters.
// Currently includes:
// * flagged and unflagged symbols
// * plot title format
PMS_PP_GROUP_START(PMS_PP_Display, PMS_PP::UPDATE_DISPLAY_NAME, true)
public:
    PMS_PP_GROUP_PARAM(PlotSymbolPtr, itsUnflaggedSymbol_, unflaggedSymbol, setUnflaggedSymbol, REC_UNFLAGGED)
    PMS_PP_GROUP_PARAM(PlotSymbolPtr, itsFlaggedSymbol_, flaggedSymbol, setFlaggedSymbol, REC_FLAGGED)
    PMS_PP_GROUP_PARAM(PlotMSLabelFormat, itsTitleFormat_, titleFormat, setTitleFormat, REC_TITLE)
PMS_PP_GROUP_END

}

#endif /* PLOTMSPLOTPARAMETERGROUPS_H_ */
