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
    
    // Update flag for axes group.
    // <group>
    static const String UPDATE_AXES_NAME;
    static const int UPDATE_AXES;
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

#define PMS_PP_GROUP_VECPARAM(TYPE, PARAM, GETTER, GETTER_SINGLE, SETTER,     \
                              SETTER_SINGLE, RECKEY)                          \
public:                                                                       \
    /* Gets/Sets the vector value for the given parameter. */                 \
    /* <group> */                                                             \
    const vector<TYPE>& GETTER() const { return PARAM; }                      \
    void SETTER(const vector<TYPE>& value) {                                  \
        if(PARAM != value) {                                                  \
            PARAM = value;                                                    \
            updated();                                                        \
        }                                                                     \
    }                                                                         \
    /* </group> */                                                            \
                                                                              \
    /* Gets/Sets a single parameter for the given parameter. */               \
    /* <group> */                                                             \
    TYPE GETTER_SINGLE(unsigned int index = 0) const {                        \
        if(index >= PARAM.size())                                             \
            const_cast<vector<TYPE>&>(PARAM).resize(index + 1);               \
        return PARAM[index]; }                                                \
    void SETTER_SINGLE(const TYPE& value, unsigned int index = 0) {           \
        if(index >= PARAM.size()) PARAM.resize(index + 1);                    \
        if(PARAM[index] != value) {                                           \
            PARAM[index] = value;                                             \
            updated();                                                        \
        }                                                                     \
    }                                                                         \
    /* </group> */                                                            \
                                                                              \
private:                                                                      \
    /* Parameter. */                                                          \
    vector<TYPE> PARAM;                                                       \
                                                                              \
    /* Record key for the indicated parameter. */                             \
    static const String RECKEY;

#define PMS_PP_GROUP_VECREFPARAM(TYPE, PARAM, GETTER, GETTER_SINGLE, SETTER,  \
                                 SETTER_SINGLE, RECKEY)                       \
public:                                                                       \
    /* Gets/Sets the vector value for the given parameter. */                 \
    /* <group> */                                                             \
    const vector<TYPE>& GETTER() const { return PARAM; }                      \
    void SETTER(const vector<TYPE>& value) {                                  \
        if(PARAM != value) {                                                  \
            PARAM = value;                                                    \
            updated();                                                        \
        }                                                                     \
    }                                                                         \
    /* </group> */                                                            \
                                                                              \
    /* Gets/Sets a single parameter for the given parameter. */               \
    /* <group> */                                                             \
    const TYPE& GETTER_SINGLE(unsigned int index = 0) const {                 \
        return PARAM[index]; }                                                \
    void SETTER_SINGLE(const TYPE& value, unsigned int index = 0) {           \
        if(PARAM[index] != value) {                                           \
            PARAM[index] = value;                                             \
            updated();                                                        \
        }                                                                     \
    }                                                                         \
    /* </group> */                                                            \
                                                                              \
private:                                                                      \
    /* Parameter. */                                                          \
    vector<TYPE> PARAM;                                                       \
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
// Parameters are vector-based, on a per-plot basis.
PMS_PP_GROUP_START(PMS_PP_Cache, PMS_PP::UPDATE_CACHE_NAME, true)
public:
    // Gets how many axes and data columns there are.
    // <group>
    unsigned int numXAxes() const;
    unsigned int numYAxes() const;
    // </group>
    
    // Sets the single axes and data columns for the given index.
    // <group>
    void setXAxis(const PMS::Axis& axis, const PMS::DataColumn& data,
            unsigned int index = 0) {
        setAxes(axis, yAxis(index), data, yDataColumn(index), index); }
    void setYAxis(const PMS::Axis& axis, const PMS::DataColumn& data,
            unsigned int index = 0) {
        setAxes(xAxis(index), axis, xDataColumn(index), data, index); }    
    void setAxes(const PMS::Axis& xAxis, const PMS::Axis& yAxis,
            const PMS::DataColumn& xData, const PMS::DataColumn& yData,
            unsigned int index = 0);
    // </group>
    
    PMS_PP_GROUP_VECPARAM(PMS::Axis, itsXAxes_, xAxes, xAxis, setXAxes, setXAxis, REC_XAXES)
    PMS_PP_GROUP_VECPARAM(PMS::Axis, itsYAxes_, yAxes, yAxis, setYAxes, setYAxis, REC_YAXES)
    PMS_PP_GROUP_VECPARAM(PMS::DataColumn, itsXData_, xDataColumns, xDataColumn, setXDataColumns, setXDataColumn, REC_XDATACOLS)
    PMS_PP_GROUP_VECPARAM(PMS::DataColumn, itsYData_, yDataColumns, yDataColumn, setYDataColumns, setYDataColumn, REC_YDATACOLS)
PMS_PP_GROUP_END


// Subclass of PlotMSPlotParameters::Group to handle axes parameters.
// Currently includes:
// * canvas attach axes
// * axes ranges, if any
// Parameters are vector-based, on a per-plot basis.
PMS_PP_GROUP_START(PMS_PP_Axes, PMS_PP::UPDATE_AXES_NAME, true)
public:
    // Gets how many axes there are.
    // <group>
    unsigned int numXAxes() const;
    unsigned int numYAxes() const;
    // </group>
    
    // Sets single versions of the parameters for the given index.
    // <group>
    void setAxes(const PlotAxis& xAxis, const PlotAxis& yAxis,
            unsigned int index = 0);    
    void setXRange(const bool& set, const prange_t& range,
            unsigned int index = 0) {
        setRanges(set, yRangeSet(index), range, yRange(index), index); }
    void setYRange(const bool& set, const prange_t& range,
            unsigned int index = 0) {
        setRanges(xRangeSet(index), set, xRange(index), range, index); }
    void setRanges(const bool& xSet, const bool& ySet, const prange_t& xRange,
            const prange_t& yRange, unsigned int index = 0);
    // </group>
    
    PMS_PP_GROUP_VECPARAM(PlotAxis, itsXAxes_, xAxes, xAxis, setXAxes, setXAxis, REC_XAXES)
    PMS_PP_GROUP_VECPARAM(PlotAxis, itsYAxes_, yAxes, yAxis, setYAxes, setYAxis, REC_YAXES)
    PMS_PP_GROUP_VECPARAM(bool, itsXRangesSet_, xRangesSet, xRangeSet, setXRanges, setXRange, REC_XRANGESSET)
    PMS_PP_GROUP_VECPARAM(bool, itsYRangesSet_, yRangesSet, yRangeSet, setYRanges, setYRange, REC_YRANGESSET)
    PMS_PP_GROUP_VECREFPARAM(prange_t, itsXRanges_, xRanges, xRange, setXRanges, setXRange, REC_XRANGES)
    PMS_PP_GROUP_VECREFPARAM(prange_t, itsYRanges_, yRanges, yRange, setYRanges, setYRange, REC_YRANGES)
PMS_PP_GROUP_END
    
    
// Subclass of PlotMSPlotParameters::Group to handle canvas parameters.
// Currently includes:
// * axes label formats
// * whether to show the canvas axes or not
// * whether to show the legend or not, and its position
// * canvas title label format
// * whether to show grid lines, and their properties
// Parameters are vector-based, on a per-canvas basis.
PMS_PP_GROUP_START(PMS_PP_Canvas, PMS_PP::UPDATE_CANVAS_NAME, true)
public:
    // Gets how many canvases there are.
    unsigned int numCanvases() const;
    
    // Sets single versions of the parameters for the given index.
    // <group>
    void setLabelFormats(const PlotMSLabelFormat& xFormat,
            const PlotMSLabelFormat& yFormat, unsigned int index = 0);
    void showAxes(const bool& xShow, const bool& yShow, unsigned int index= 0);    
    void showLegend(const bool& show, const PlotCanvas::LegendPosition& pos,
            unsigned int index = 0);

    void showGridMajor(const bool& show, const PlotLinePtr& line,
            unsigned int index = 0) {
        showGrid(show,gridMinorShown(index),line,gridMinorLine(index),index); }
    void showGridMinor(const bool& show, const PlotLinePtr& line,
            unsigned int index = 0) {
        showGrid(gridMajorShown(index),show,gridMajorLine(index),line,index); }
    void showGrid(const bool& showMajor, const bool& showMinor,
            const PlotLinePtr& majorLine, const PlotLinePtr& minorLine,
            unsigned int index = 0);
    // </group>
    
    PMS_PP_GROUP_VECREFPARAM(PlotMSLabelFormat, itsXLabels_, xLabelFormats, xLabelFormat, setXLabelFormats, setXLabelFormat, REC_XLABELS)
    PMS_PP_GROUP_VECREFPARAM(PlotMSLabelFormat, itsYLabels_, yLabelFormats, yLabelFormat, setYLabelFormats, setYLabelFormat, REC_YLABELS)
    PMS_PP_GROUP_VECPARAM(bool, itsXAxesShown_, xAxesShown, xAxisShown, showXAxes, showXAxis, REC_SHOWXAXES)
    PMS_PP_GROUP_VECPARAM(bool, itsYAxesShown_, yAxesShown, yAxisShown, showYAxes, showYAxis, REC_SHOWYAXES)
    PMS_PP_GROUP_VECPARAM(bool, itsLegendsShown_, legendsShown, legendShown, showLegends, showLegend, REC_SHOWLEGENDS)
    PMS_PP_GROUP_VECPARAM(PlotCanvas::LegendPosition, itsLegendsPos_, legendPositions, legendPosition, showLegends, showLegend, REC_LEGENDSPOS)
    PMS_PP_GROUP_VECREFPARAM(PlotMSLabelFormat, itsTitles_, titleFormats, titleFormat, setTitleFormats, setTitleFormat, REC_TITLES)
    PMS_PP_GROUP_VECPARAM(bool, itsGridMajsShown_, gridMajorsShown, gridMajorShown, showGridMajors, showGridMajor, REC_SHOWGRIDMAJS)
    PMS_PP_GROUP_VECPARAM(bool, itsGridMinsShown_, gridMinorsShown, gridMinorShown, showGridMinors, showGridMinor, REC_SHOWGRIDMINS)
    PMS_PP_GROUP_VECPARAM(PlotLinePtr, itsGridMajLines_, gridMajorLines, gridMajorLine, setGridMajorLines, setGridMajorLine, REC_GRIDMAJLINES)
    PMS_PP_GROUP_VECPARAM(PlotLinePtr, itsGridMinLines_, gridMinorLines, gridMinorLine, setGridMinorLines, setGridMinorLine, REC_GRIDMINLINES)
PMS_PP_GROUP_END


// Subclass of PlotMSPlotParameters::Group to handle display parameters.
// Currently includes:
// * flagged and unflagged symbols
// * plot title format
// * colorize flag and axis
// Parameters are vector-based, on a per-plot basis.
PMS_PP_GROUP_START(PMS_PP_Display, PMS_PP::UPDATE_DISPLAY_NAME, true)
public:
    // Sets whether to colorize, and which axis to use.
    void setColorize(const bool& colorize, const PMS::Axis& axis,
            unsigned int index = 0);
    
    // Resizes the vectors to the given, using the default values.
    void resizeVectors(unsigned int newSize);
    
    PMS_PP_GROUP_VECPARAM(PlotSymbolPtr, itsUnflaggedSymbols_, unflaggedSymbols, unflaggedSymbol, setUnflaggedSymbols, setUnflaggedSymbol, REC_UNFLAGGEDS)
    PMS_PP_GROUP_VECPARAM(PlotSymbolPtr, itsFlaggedSymbols_, flaggedSymbols, flaggedSymbol, setFlaggedSymbols, setFlaggedSymbol, REC_FLAGGEDS)
    PMS_PP_GROUP_VECREFPARAM(PlotMSLabelFormat, itsTitleFormats_, titleFormats, titleFormat, setTitleFormats, setTitleFormat, REC_TITLES)
    PMS_PP_GROUP_VECPARAM(bool, itsColorizeFlags_, colorizeFlags, colorizeFlag, setColorize, setColorize, REC_COLFLAGS)
    PMS_PP_GROUP_VECPARAM(PMS::Axis, itsColorizeAxes_, colorizeAxes, colorizeAxis, setColorize, setColorize, REC_COLAXES)
PMS_PP_GROUP_END

}

#endif /* PLOTMSPLOTPARAMETERGROUPS_H_ */
