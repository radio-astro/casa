//# PlotMSPlotParameterGroups.cc: Implementations of plot subparameter groups.
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
#include <plotms/Plots/PlotMSPlotParameterGroups.h>

#include <plotms/PlotMS/PlotMSLabelFormat.h>

namespace casa {

////////////////////////
// PMS_PP DEFINITIONS //
////////////////////////

const String PMS_PP::UPDATE_REDRAW_NAME = "REDRAW";
const int PMS_PP::UPDATE_REDRAW =
    PlotMSWatchedParameters::REGISTER_UPDATE_FLAG(UPDATE_REDRAW_NAME);

const String PMS_PP::UPDATE_MSDATA_NAME = "MS_DATA";
const int PMS_PP::UPDATE_MSDATA =
    PlotMSWatchedParameters::REGISTER_UPDATE_FLAG(UPDATE_MSDATA_NAME);

const String PMS_PP::UPDATE_CACHE_NAME = "CACHE";
const int PMS_PP::UPDATE_CACHE =
    PlotMSWatchedParameters::REGISTER_UPDATE_FLAG(UPDATE_CACHE_NAME);

const String PMS_PP::UPDATE_CANVAS_NAME = "CANVAS";
const int PMS_PP::UPDATE_CANVAS =
    PlotMSWatchedParameters::REGISTER_UPDATE_FLAG(UPDATE_CANVAS_NAME);

const String PMS_PP::UPDATE_DISPLAY_NAME = "DISPLAY";
const int PMS_PP::UPDATE_DISPLAY =
    PlotMSWatchedParameters::REGISTER_UPDATE_FLAG(UPDATE_DISPLAY_NAME);

const String PMS_PP::UPDATE_LOG_NAME = "LOG";
const int PMS_PP::UPDATE_LOG =
    PlotMSWatchedParameters::REGISTER_UPDATE_FLAG(UPDATE_LOG_NAME);

const String PMS_PP::UPDATE_PLOTMS_OPTIONS_NAME = "PLOTMS_OPTIONS";
const int PMS_PP::UPDATE_PLOTMS_OPTIONS =
    PlotMSWatchedParameters::REGISTER_UPDATE_FLAG(UPDATE_PLOTMS_OPTIONS_NAME);


////////////
// MACROS //
////////////

// Macro for defining a record key.
#define PMS_PP_RECKEY(CLASS, KEY, VALUE) const String CLASS::KEY = VALUE;

// Macro for defining the constructors and destructors.
#define PMS_PP_CONSTRUCTORS(CLASS)                                            \
CLASS::CLASS(PlotFactoryPtr factory) : PlotMSPlotParameters::Group(factory) { \
    setDefaults(); }                                                          \
                                                                              \
CLASS::CLASS(const CLASS& copy) : PlotMSPlotParameters::Group(copy) {         \
    setDefaults();                                                            \
    operator=(copy);                                                          \
}                                                                             \
                                                                              \
CLASS::~CLASS() { }

// Macros for defining the toRecord() method.
// <group>
#define PMS_PP_TORECORD_START(CLASS)                                          \
Record CLASS::toRecord() const {                                              \
    Record rec;

#define PMS_PP_TORECORD_DEF(KEY, VALUE)    rec.define(KEY, VALUE);
#define PMS_PP_TORECORD_DEFREC(KEY, VALUE) rec.defineRecord(KEY, VALUE);

#define PMS_PP_TORECORD_END return rec; }
// </group>

// Macros for defining the fromRecord() method.
// <group>
#define PMS_PP_FROMRECORD_START(CLASS)                                        \
void CLASS::fromRecord(const Record& record) {                                \
    bool valuesChanged = false;

#define PMS_PP_FROMRECORD_VAL(PARAM, KEY, TYPE, ASTYPE)                       \
    if(record.isDefined(KEY) && record.dataType(KEY) == TYPE &&               \
       PARAM != record.ASTYPE(KEY)) {                                         \
        PARAM = record.ASTYPE(KEY);                                           \
        valuesChanged = true;                                                 \
    }

#define PMS_PP_FROMRECORD_VALTYPE(PARAM, KEY, TYPE, ASTYPE, PTYPE)            \
    if(record.isDefined(KEY) && record.dataType(KEY) == TYPE &&               \
       PARAM != (PTYPE)record.ASTYPE(KEY)) {                                  \
        PARAM = (PTYPE)record.ASTYPE(KEY);                                    \
        valuesChanged = true;                                                 \
    }

#define PMS_PP_FROMRECORD_VALREC(PARAM, KEY, CLASS)                           \
    if(record.isDefined(KEY) && record.dataType(KEY) == TpRecord) {           \
        CLASS tmp(PARAM);                                                     \
        tmp.fromRecord(record.asRecord(KEY));                                 \
        if(PARAM != tmp) {                                                    \
            PARAM = tmp;                                                      \
            valuesChanged = true;                                             \
        }                                                                     \
    }

#define PMS_PP_FROMRECORD_VALREF(PARAM, KEY, CLASS, FACTMETH)                 \
    if(record.isDefined(KEY) && record.dataType(KEY) == TpRecord) {           \
        CLASS tmp = factory()->FACTMETH(*PARAM);                              \
        tmp->fromRecord(record.asRecord(KEY));                                \
        if(*PARAM != *tmp) {                                                  \
            *PARAM = *tmp;                                                    \
            valuesChanged = true;                                             \
        }                                                                     \
    }

#define PMS_PP_FROMRECORD_END if(valuesChanged) updated(); }
// </group>

// Macros for defining the operator=() method.
// <group>
#define PMS_PP_COPYOP_START(CLASS)                                            \
PlotMSPlotParameters::Group& CLASS::operator=(const Group& other) {           \
    const CLASS* o = dynamic_cast<const CLASS*>(&other);                      \
    if(o != NULL && *this != *o) {

#define PMS_PP_COPYOP_PARAM(PARAM) PARAM = o->PARAM;
#define PMS_PP_COPYOP_PARAMREF(PARAM) *PARAM = *o->PARAM;

#define PMS_PP_COPYOP_END                                                     \
        updated();                                                            \
    }                                                                         \
    return *this; }
// </group>

// Macros for defining the operator==() method.
// <group>
#define PMS_PP_EQUALOP_START(CLASS)                                           \
bool CLASS::operator==(const Group& other) const {                            \
    const CLASS* o = dynamic_cast<const CLASS*>(&other);                      \
    if(o == NULL) return false;

#define PMS_PP_EQUALOP_PARAM(PARAM) if(PARAM != o->PARAM) return false;
#define PMS_PP_EQUALOP_PARAMREF(PARAM) if(*PARAM != *o->PARAM) return false;
#define PMS_PP_EQUALOP_PARAMFLAG(FLAG, PARAM)                                 \
    if(FLAG != o->FLAG || (FLAG && PARAM != o->PARAM)) return false;
#define PMS_PP_EQUALOP_PARAMFLAGREF(FLAG, PARAM)                              \
    if(FLAG != o->FLAG || (FLAG && *PARAM != *o->PARAM)) return false;

#define PMS_PP_EQUALOP_END return true; }
// </group>

// Macros for defining the setDefaults() method.
// <group>
#define PMS_PP_SETDEFAULTS_START(CLASS) void CLASS::setDefaults() {
#define PMS_PP_SETDEFAULTS_PARAM(PARAM, DEFAULT) PARAM = DEFAULT;
#define PMS_PP_SETDEFAULTS_END }
// </group>


///////////////////////////////
// PMS_PP_MSDATA DEFINITIONS //
///////////////////////////////

// PMS_PP_MSData record keys.
PMS_PP_RECKEY(PMS_PP_MSData, REC_FILENAME, "filename")
PMS_PP_RECKEY(PMS_PP_MSData, REC_SELECTION, "selection")
PMS_PP_RECKEY(PMS_PP_MSData, REC_AVERAGING, "averaging")

// PMS_PP_MSData constructors/destructors.
PMS_PP_CONSTRUCTORS(PMS_PP_MSData)

// PMS_PP_MSData::toRecord().
PMS_PP_TORECORD_START(PMS_PP_MSData)
    PMS_PP_TORECORD_DEF(REC_FILENAME, itsFilename_)
    PMS_PP_TORECORD_DEFREC(REC_SELECTION, itsSelection_.toRecord())
    PMS_PP_TORECORD_DEFREC(REC_AVERAGING, itsAveraging_.toRecord())
PMS_PP_TORECORD_END

// PMS_PP_MSData::fromRecord().
PMS_PP_FROMRECORD_START(PMS_PP_MSData)
    PMS_PP_FROMRECORD_VAL(itsFilename_, REC_FILENAME, TpString, asString)
    PMS_PP_FROMRECORD_VALREC(itsSelection_, REC_SELECTION, PlotMSSelection)
    PMS_PP_FROMRECORD_VALREC(itsAveraging_, REC_AVERAGING, PlotMSAveraging)
PMS_PP_FROMRECORD_END

// PMS_PP_MSData::operator=().
PMS_PP_COPYOP_START(PMS_PP_MSData)
    PMS_PP_COPYOP_PARAM(itsFilename_)
    PMS_PP_COPYOP_PARAM(itsSelection_)
    PMS_PP_COPYOP_PARAM(itsAveraging_)
PMS_PP_COPYOP_END

// PMS_PP_MSData::operator==().
PMS_PP_EQUALOP_START(PMS_PP_MSData)
    PMS_PP_EQUALOP_PARAM(itsFilename_)
    PMS_PP_EQUALOP_PARAM(itsSelection_)
    PMS_PP_EQUALOP_PARAM(itsAveraging_)
PMS_PP_EQUALOP_END

// PMS_PP_MSData::setDefaults().
PMS_PP_SETDEFAULTS_START(PMS_PP_MSData)
    PMS_PP_SETDEFAULTS_PARAM(itsFilename_, "")
    PMS_PP_SETDEFAULTS_PARAM(itsSelection_, PlotMSSelection())
    PMS_PP_SETDEFAULTS_PARAM(itsAveraging_, PlotMSAveraging())
PMS_PP_SETDEFAULTS_END


//////////////////////////////
// PMS_PP_CACHE DEFINITIONS //
//////////////////////////////

// PMS_PP_Cache record keys.
PMS_PP_RECKEY(PMS_PP_Cache, REC_XAXIS,    "xaxis")
PMS_PP_RECKEY(PMS_PP_Cache, REC_YAXIS,    "yaxis")
PMS_PP_RECKEY(PMS_PP_Cache, REC_XDATACOL, "xdatacolumn")
PMS_PP_RECKEY(PMS_PP_Cache, REC_YDATACOL, "ydatacolumn")

// PMS_PP_Cache constructors/destructors.
PMS_PP_CONSTRUCTORS(PMS_PP_Cache)

// PMS_PP_Cache::toRecord().
PMS_PP_TORECORD_START(PMS_PP_Cache)
    PMS_PP_TORECORD_DEF(REC_XAXIS,    (int)itsXAxis_)
    PMS_PP_TORECORD_DEF(REC_YAXIS,    (int)itsYAxis_)
    PMS_PP_TORECORD_DEF(REC_XDATACOL, (int)itsXData_)
    PMS_PP_TORECORD_DEF(REC_YDATACOL, (int)itsYData_)
PMS_PP_TORECORD_END

// PMS_PP_Cache::fromRecord().
PMS_PP_FROMRECORD_START(PMS_PP_Cache)
    PMS_PP_FROMRECORD_VALTYPE(itsXAxis_, REC_XAXIS, TpInt, asInt, PMS::Axis)
    PMS_PP_FROMRECORD_VALTYPE(itsYAxis_, REC_YAXIS, TpInt, asInt, PMS::Axis)
    PMS_PP_FROMRECORD_VALTYPE(itsXData_, REC_XDATACOL, TpInt, asInt, PMS::DataColumn)
    PMS_PP_FROMRECORD_VALTYPE(itsYData_, REC_YDATACOL, TpInt, asInt, PMS::DataColumn)
PMS_PP_FROMRECORD_END

// PMS_PP_Cache::operator=().
PMS_PP_COPYOP_START(PMS_PP_Cache)
    PMS_PP_COPYOP_PARAM(itsXAxis_)
    PMS_PP_COPYOP_PARAM(itsYAxis_)
    PMS_PP_COPYOP_PARAM(itsXData_)
    PMS_PP_COPYOP_PARAM(itsYData_)
PMS_PP_COPYOP_END

// PMS_PP_Cache::operator==().
PMS_PP_EQUALOP_START(PMS_PP_Cache)
    PMS_PP_EQUALOP_PARAM(itsXAxis_)
    PMS_PP_EQUALOP_PARAM(itsYAxis_)
    PMS_PP_EQUALOP_PARAM(itsXData_)
    PMS_PP_EQUALOP_PARAM(itsYData_)
PMS_PP_EQUALOP_END

// PMS_PP_Cache::setDefaults().
PMS_PP_SETDEFAULTS_START(PMS_PP_Cache)
    PMS_PP_SETDEFAULTS_PARAM(itsXAxis_, PMS::DEFAULT_XAXIS)
    PMS_PP_SETDEFAULTS_PARAM(itsYAxis_, PMS::DEFAULT_YAXIS)
    PMS_PP_SETDEFAULTS_PARAM(itsXData_, PMS::DEFAULT_DATACOLUMN)
    PMS_PP_SETDEFAULTS_PARAM(itsYData_, PMS::DEFAULT_DATACOLUMN)
PMS_PP_SETDEFAULTS_END

void PMS_PP_Cache::setAxes(const PMS::Axis& xAxis, const PMS::Axis& yAxis,
        const PMS::DataColumn& xData, const PMS::DataColumn& yData) {
    if(itsXAxis_ != xAxis || itsYAxis_ != yAxis || itsXData_ != xData ||
       itsYData_ != yData) {
        itsXAxis_ = xAxis;
        itsYAxis_ = yAxis;
        itsXData_ = xData;
        itsYData_ = yData;
        updated();
    }
}


///////////////////////////////
// PMS_PP_CANVAS DEFINITIONS //
///////////////////////////////

// PMS_PP_Canvas record keys.
PMS_PP_RECKEY(PMS_PP_Canvas, REC_XAXIS,       "canvasXAxis")
PMS_PP_RECKEY(PMS_PP_Canvas, REC_YAXIS,       "canvasYAxis")
PMS_PP_RECKEY(PMS_PP_Canvas, REC_XRANGESET,   "xRangeSet")
PMS_PP_RECKEY(PMS_PP_Canvas, REC_YRANGESET,   "yRangeSet")
PMS_PP_RECKEY(PMS_PP_Canvas, REC_XRANGE,      "xRange")
PMS_PP_RECKEY(PMS_PP_Canvas, REC_YRANGE,      "yRange")
PMS_PP_RECKEY(PMS_PP_Canvas, REC_XLABEL,      "xLabelFormat")
PMS_PP_RECKEY(PMS_PP_Canvas, REC_YLABEL,      "yLabelFormat")
PMS_PP_RECKEY(PMS_PP_Canvas, REC_SHOWXAXIS,   "showXAxis")
PMS_PP_RECKEY(PMS_PP_Canvas, REC_SHOWYAXIS,   "showYAxis")
PMS_PP_RECKEY(PMS_PP_Canvas, REC_SHOWLEGEND,  "showLegend")
PMS_PP_RECKEY(PMS_PP_Canvas, REC_LEGENDPOS,   "legendPosition")
PMS_PP_RECKEY(PMS_PP_Canvas, REC_TITLE,       "canvasTitleFormat")
PMS_PP_RECKEY(PMS_PP_Canvas, REC_SHOWGRIDMAJ, "showGridMajor")
PMS_PP_RECKEY(PMS_PP_Canvas, REC_SHOWGRIDMIN, "showGridMinor")
PMS_PP_RECKEY(PMS_PP_Canvas, REC_GRIDMAJLINE, "gridMajorLine")
PMS_PP_RECKEY(PMS_PP_Canvas, REC_GRIDMINLINE, "gridMinorLine")

// PMS_PP_Canvas constructors/destructors.
PMS_PP_CONSTRUCTORS(PMS_PP_Canvas)

// PMS_PP_Canvas::toRecord().
PMS_PP_TORECORD_START(PMS_PP_Canvas)
    PMS_PP_TORECORD_DEF(REC_XAXIS, (int)itsXAxis_)
    PMS_PP_TORECORD_DEF(REC_YAXIS, (int)itsYAxis_)
    PMS_PP_TORECORD_DEF(REC_XRANGESET, itsXRangeSet_)
    PMS_PP_TORECORD_DEF(REC_YRANGESET, itsYRangeSet_)
    PMS_PP_TORECORD_DEF(REC_XRANGE + ".first", itsXRange_.first)
    PMS_PP_TORECORD_DEF(REC_XRANGE + ".second", itsXRange_.second)
    PMS_PP_TORECORD_DEF(REC_YRANGE + ".first", itsYRange_.first)
    PMS_PP_TORECORD_DEF(REC_YRANGE + ".second", itsYRange_.second)
    PMS_PP_TORECORD_DEF(REC_XLABEL, itsXLabel_.format)
    PMS_PP_TORECORD_DEF(REC_YLABEL, itsYLabel_.format)
    PMS_PP_TORECORD_DEF(REC_SHOWXAXIS, itsXAxisShown_)
    PMS_PP_TORECORD_DEF(REC_SHOWYAXIS, itsYAxisShown_)
    PMS_PP_TORECORD_DEF(REC_SHOWLEGEND, itsLegendShown_)
    PMS_PP_TORECORD_DEF(REC_LEGENDPOS, (int)itsLegendPos_)
    PMS_PP_TORECORD_DEF(REC_TITLE, itsTitle_.format)
    PMS_PP_TORECORD_DEF(REC_SHOWGRIDMAJ, itsGridMajShown_)
    PMS_PP_TORECORD_DEF(REC_SHOWGRIDMIN, itsGridMinShown_)
    PMS_PP_TORECORD_DEFREC(REC_GRIDMAJLINE, itsGridMajLine_->toRecord())
    PMS_PP_TORECORD_DEFREC(REC_GRIDMINLINE, itsGridMinLine_->toRecord())
PMS_PP_TORECORD_END

// PMS_PP_Canvas::fromRecord().
PMS_PP_FROMRECORD_START(PMS_PP_Canvas)    
    PMS_PP_FROMRECORD_VALTYPE(itsXAxis_, REC_XAXIS, TpInt, asInt, PlotAxis)
    PMS_PP_FROMRECORD_VALTYPE(itsYAxis_, REC_YAXIS, TpInt, asInt, PlotAxis)
    PMS_PP_FROMRECORD_VAL(itsXRangeSet_, REC_XRANGESET, TpBool, asBool)
    PMS_PP_FROMRECORD_VAL(itsYRangeSet_, REC_YRANGESET, TpBool, asBool)
    PMS_PP_FROMRECORD_VAL(itsXRange_.first, REC_XRANGE + ".first", TpDouble, asDouble)
    PMS_PP_FROMRECORD_VAL(itsXRange_.second, REC_XRANGE + ".second", TpDouble, asDouble)
    PMS_PP_FROMRECORD_VAL(itsYRange_.first, REC_YRANGE + ".first", TpDouble, asDouble)
    PMS_PP_FROMRECORD_VAL(itsYRange_.second, REC_YRANGE + ".second", TpDouble, asDouble)    
    PMS_PP_FROMRECORD_VAL(itsXLabel_.format, REC_XLABEL, TpString, asString)
    PMS_PP_FROMRECORD_VAL(itsYLabel_.format, REC_YLABEL, TpString, asString)
    PMS_PP_FROMRECORD_VAL(itsXAxisShown_, REC_SHOWXAXIS, TpBool, asBool)
    PMS_PP_FROMRECORD_VAL(itsYAxisShown_, REC_SHOWYAXIS, TpBool, asBool)
    PMS_PP_FROMRECORD_VAL(itsLegendShown_, REC_SHOWLEGEND, TpBool, asBool)
    PMS_PP_FROMRECORD_VALTYPE(itsLegendPos_, REC_LEGENDPOS, TpInt, asInt, PlotCanvas::LegendPosition)
    PMS_PP_FROMRECORD_VAL(itsTitle_.format, REC_TITLE, TpString, asString)
    PMS_PP_FROMRECORD_VAL(itsGridMajShown_, REC_SHOWGRIDMAJ, TpBool, asBool)
    PMS_PP_FROMRECORD_VAL(itsGridMinShown_, REC_SHOWGRIDMIN, TpBool, asBool)
    PMS_PP_FROMRECORD_VALREF(itsGridMajLine_, REC_GRIDMAJLINE, PlotLinePtr, line)
    PMS_PP_FROMRECORD_VALREF(itsGridMinLine_, REC_GRIDMINLINE, PlotLinePtr, line)
PMS_PP_FROMRECORD_END

// PMS_PP_Canvas::operator=().
PMS_PP_COPYOP_START(PMS_PP_Canvas)
    PMS_PP_COPYOP_PARAM(itsXAxis_)
    PMS_PP_COPYOP_PARAM(itsYAxis_)
    PMS_PP_COPYOP_PARAM(itsXRangeSet_)
    PMS_PP_COPYOP_PARAM(itsYRangeSet_)
    PMS_PP_COPYOP_PARAM(itsXRange_)
    PMS_PP_COPYOP_PARAM(itsYRange_)
    PMS_PP_COPYOP_PARAM(itsXLabel_)
    PMS_PP_COPYOP_PARAM(itsYLabel_)
    PMS_PP_COPYOP_PARAM(itsXAxisShown_)
    PMS_PP_COPYOP_PARAM(itsYAxisShown_)
    PMS_PP_COPYOP_PARAM(itsLegendShown_)
    PMS_PP_COPYOP_PARAM(itsLegendPos_)
    PMS_PP_COPYOP_PARAM(itsTitle_)
    PMS_PP_COPYOP_PARAM(itsGridMajShown_)
    PMS_PP_COPYOP_PARAM(itsGridMinShown_)
    PMS_PP_COPYOP_PARAMREF(itsGridMajLine_)
    PMS_PP_COPYOP_PARAMREF(itsGridMinLine_)
PMS_PP_COPYOP_END

// PMS_PP_Canvas::operator==().
PMS_PP_EQUALOP_START(PMS_PP_Canvas)
    PMS_PP_EQUALOP_PARAM(itsXAxis_)
    PMS_PP_EQUALOP_PARAM(itsYAxis_)
    PMS_PP_EQUALOP_PARAMFLAG(itsXRangeSet_, itsXRange_)
    PMS_PP_EQUALOP_PARAMFLAG(itsYRangeSet_, itsYRange_)
    PMS_PP_EQUALOP_PARAM(itsXLabel_)
    PMS_PP_EQUALOP_PARAM(itsYLabel_)
    PMS_PP_EQUALOP_PARAM(itsXAxisShown_)
    PMS_PP_EQUALOP_PARAM(itsYAxisShown_)
    PMS_PP_EQUALOP_PARAMFLAG(itsLegendShown_, itsLegendPos_)
    PMS_PP_EQUALOP_PARAM(itsTitle_)
    PMS_PP_EQUALOP_PARAMFLAGREF(itsGridMajShown_, itsGridMajLine_)
    PMS_PP_EQUALOP_PARAMFLAGREF(itsGridMinShown_, itsGridMinLine_)
PMS_PP_EQUALOP_END

// PMS_PP_Canvas::setDefaults().
PMS_PP_SETDEFAULTS_START(PMS_PP_Canvas)
    PMS_PP_SETDEFAULTS_PARAM(itsXAxis_, PMS::DEFAULT_CANVAS_XAXIS)
    PMS_PP_SETDEFAULTS_PARAM(itsYAxis_, PMS::DEFAULT_CANVAS_YAXIS)
    PMS_PP_SETDEFAULTS_PARAM(itsXRangeSet_, false)
    PMS_PP_SETDEFAULTS_PARAM(itsYRangeSet_, false)
    PMS_PP_SETDEFAULTS_PARAM(itsXRange_, prange_t(0.0, 0.0))
    PMS_PP_SETDEFAULTS_PARAM(itsYRange_, prange_t(0.0, 0.0))
    PMS_PP_SETDEFAULTS_PARAM(itsXLabel_, PlotMSLabelFormat(PMS::DEFAULT_CANVAS_AXIS_LABEL_FORMAT))
    PMS_PP_SETDEFAULTS_PARAM(itsYLabel_, PlotMSLabelFormat(PMS::DEFAULT_CANVAS_AXIS_LABEL_FORMAT))
    PMS_PP_SETDEFAULTS_PARAM(itsXAxisShown_, PMS::DEFAULT_SHOWAXIS)
    PMS_PP_SETDEFAULTS_PARAM(itsYAxisShown_, PMS::DEFAULT_SHOWAXIS)
    PMS_PP_SETDEFAULTS_PARAM(itsLegendShown_, PMS::DEFAULT_SHOWLEGEND)
    PMS_PP_SETDEFAULTS_PARAM(itsLegendPos_, PMS::DEFAULT_LEGENDPOSITION)
    PMS_PP_SETDEFAULTS_PARAM(itsTitle_, PlotMSLabelFormat(PMS::DEFAULT_TITLE_FORMAT))
    PMS_PP_SETDEFAULTS_PARAM(itsGridMajShown_, PMS::DEFAULT_SHOW_GRID)
    PMS_PP_SETDEFAULTS_PARAM(itsGridMinShown_, PMS::DEFAULT_SHOW_GRID)
    PMS_PP_SETDEFAULTS_PARAM(itsGridMajLine_, PMS::DEFAULT_GRID_LINE(factory()))
    PMS_PP_SETDEFAULTS_PARAM(itsGridMinLine_, PMS::DEFAULT_GRID_LINE(factory()))
PMS_PP_SETDEFAULTS_END

void PMS_PP_Canvas::setRanges(const bool& xSet, const bool& ySet,
        const prange_t& xRange, const prange_t& yRange) {
    if(itsXRangeSet_ != xSet || (xSet && itsXRange_ != xRange) ||
       itsYRangeSet_ != ySet || (ySet && itsYRange_ != yRange)) {
        itsXRangeSet_ = xSet;
        itsYRangeSet_ = ySet;
        itsXRange_ = xRange;
        itsYRange_ = yRange;
        updated();
    }
}

void PMS_PP_Canvas::showLegend(const bool& show,
        const PlotCanvas::LegendPosition& position) {
    if(itsLegendShown_ != show || (show && itsLegendPos_ != position)) {
        itsLegendShown_ = show;
        itsLegendPos_ = position;
        updated();
    }
}

void PMS_PP_Canvas::showGrid(const bool& showMajor, const bool& showMinor,
        const PlotLinePtr& majorLine, const PlotLinePtr& minorLine) {
    if(itsGridMajShown_ != showMajor ||
       (showMajor && *itsGridMajLine_ != *majorLine) ||
       itsGridMinShown_ != showMinor ||
       (showMinor && *itsGridMinLine_ != *minorLine)) {
        itsGridMajShown_ = showMajor;
        itsGridMinShown_ = showMinor;
        *itsGridMajLine_ = *majorLine;
        *itsGridMinLine_ = *minorLine;
        updated();
    }
}


////////////////////////////////
// PMS_PP_DISPLAY DEFINITIONS //
////////////////////////////////

// PMS_PP_Display record keys.
PMS_PP_RECKEY(PMS_PP_Display, REC_UNFLAGGED, "unflagged")
PMS_PP_RECKEY(PMS_PP_Display, REC_FLAGGED,   "flagged")
PMS_PP_RECKEY(PMS_PP_Display, REC_TITLE,      "title")

// PMS_PP_Display constructors/destructors.
PMS_PP_CONSTRUCTORS(PMS_PP_Display)

// PMS_PP_Display::toRecord().
PMS_PP_TORECORD_START(PMS_PP_Display)
    PMS_PP_TORECORD_DEFREC(REC_UNFLAGGED, itsUnflaggedSymbol_->toRecord())
    PMS_PP_TORECORD_DEFREC(REC_FLAGGED, itsFlaggedSymbol_->toRecord())
    PMS_PP_TORECORD_DEF(REC_TITLE, itsTitleFormat_.format)
PMS_PP_TORECORD_END

// PMS_PP_Display::fromRecord().
PMS_PP_FROMRECORD_START(PMS_PP_Display)
    PMS_PP_FROMRECORD_VALREF(itsUnflaggedSymbol_, REC_UNFLAGGED, PlotSymbolPtr, symbol)
    PMS_PP_FROMRECORD_VALREF(itsFlaggedSymbol_, REC_FLAGGED, PlotSymbolPtr, symbol)
    PMS_PP_FROMRECORD_VAL(itsTitleFormat_.format, REC_TITLE, TpString, asString)
PMS_PP_FROMRECORD_END

// PMS_PP_Display::operator=().
PMS_PP_COPYOP_START(PMS_PP_Display)
    PMS_PP_COPYOP_PARAMREF(itsUnflaggedSymbol_)
    PMS_PP_COPYOP_PARAMREF(itsFlaggedSymbol_)
    PMS_PP_COPYOP_PARAM(itsTitleFormat_)
PMS_PP_COPYOP_END

// PMS_PP_Display::operator==().
PMS_PP_EQUALOP_START(PMS_PP_Display)
    PMS_PP_EQUALOP_PARAMREF(itsUnflaggedSymbol_)
    PMS_PP_EQUALOP_PARAMREF(itsFlaggedSymbol_)
    PMS_PP_EQUALOP_PARAM(itsTitleFormat_)
PMS_PP_EQUALOP_END

// PMS_PP_Display::setDefaults().
PMS_PP_SETDEFAULTS_START(PMS_PP_Display)
    PMS_PP_SETDEFAULTS_PARAM(itsUnflaggedSymbol_, PMS::DEFAULT_UNFLAGGED_SYMBOL(factory()))
    PMS_PP_SETDEFAULTS_PARAM(itsFlaggedSymbol_, PMS::DEFAULT_FLAGGED_SYMBOL(factory()))
    PMS_PP_SETDEFAULTS_PARAM(itsTitleFormat_, PlotMSLabelFormat(PMS::DEFAULT_TITLE_FORMAT))
PMS_PP_SETDEFAULTS_END

}
