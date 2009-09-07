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

const String PMS_PP::UPDATE_MSDATA_NAME = "MSDATA";
const int PMS_PP::UPDATE_MSDATA =
    PlotMSWatchedParameters::REGISTER_UPDATE_FLAG(UPDATE_MSDATA_NAME);

const String PMS_PP::UPDATE_CACHE_NAME = "CACHE";
const int PMS_PP::UPDATE_CACHE =
    PlotMSWatchedParameters::REGISTER_UPDATE_FLAG(UPDATE_CACHE_NAME);

const String PMS_PP::UPDATE_AXES_NAME = "AXES";
const int PMS_PP::UPDATE_AXES =
    PlotMSWatchedParameters::REGISTER_UPDATE_FLAG(UPDATE_AXES_NAME);

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

#define PMS_PP_TORECORD_DEFVEC(KEY, VALUE) {                                  \
    Record tmpRec;                                                            \
    for(unsigned int i = 0; i < VALUE.size(); i++)                            \
        tmpRec.define(i, VALUE[i]);                                           \
    rec.defineRecord(KEY, tmpRec);                                            \
}

#define PMS_PP_TORECORD_DEFVECMETHOD(KEY, VALUE, VALUEMETHOD) {               \
    Record tmpRec;                                                            \
    for(unsigned int i = 0; i < VALUE.size(); i++)                            \
        tmpRec.define(i, VALUE[i] VALUEMETHOD);                               \
    rec.defineRecord(KEY, tmpRec);                                            \
}

#define PMS_PP_TORECORD_DEFVECRECMETHOD(KEY, VALUE, VALUEMETHOD) {            \
    Record tmpRec;                                                            \
    for(unsigned int i = 0; i < VALUE.size(); i++)                            \
        tmpRec.defineRecord(i, VALUE[i] VALUEMETHOD);                         \
    rec.defineRecord(KEY, tmpRec);                                            \
}

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

#define PMS_PP_FROMRECORD_VALINTARR(PARAM, KEY, TYPE)                         \
    if(record.isDefined(KEY) && record.dataType(KEY) == TpArrayInt) {         \
        vector<TYPE> tmp = PMS::fromIntVector<TYPE>(record.asArrayInt(KEY));  \
        if(PARAM != tmp) {                                                    \
            PARAM = tmp;                                                      \
            valuesChanged = true;                                             \
        }                                                                     \
    }

#define PMS_PP_FROMRECORD_VALTYPEARR(PARAM, KEY, TYPE, ASTYPE, PTYPE)         \
    if(record.isDefined(KEY) && record.dataType(KEY) == TYPE) {               \
        vector<PTYPE> tmp;                                                    \
        record.ASTYPE(KEY).tovector(tmp);                                     \
        if(PARAM != tmp) {                                                    \
            PARAM = tmp;                                                      \
            valuesChanged = true;                                             \
        }                                                                     \
    }

#define PMS_PP_FROMRECORD_VEC(KEY, VALUE, TYPE, ASTYPE)                       \
    if(record.isDefined(KEY) && record.dataType(KEY) == TpRecord) {           \
        const Record& tmpRec = record.asRecord(KEY);                          \
        VALUE.resize(tmpRec.nfields());                                       \
        for(unsigned int i= 0; i < VALUE.size() && i < tmpRec.nfields(); i++){\
            if(tmpRec.dataType(i) == TYPE && VALUE[i] != tmpRec.ASTYPE(i)) {  \
                VALUE[i] = tmpRec.ASTYPE(i);                                  \
                valuesChanged = true;                                         \
            }                                                                 \
        }                                                                     \
    }

#define PMS_PP_FROMRECORD_VECREF(KEY, VALUE, CLASS, FACTMETHOD)               \
    if(record.isDefined(KEY) && record.dataType(KEY) == TpRecord) {           \
        const Record& tmpRec = record.asRecord(KEY);                          \
        VALUE.resize(tmpRec.nfields(), tmpRec.nfields() > 0 ?                 \
                     factory()->FACTMETHOD(*VALUE[0]) : NULL);                \
        CLASS tmp= VALUE.size() > 0 ? factory()->FACTMETHOD(*VALUE[0]) : NULL;\
        for(unsigned int i= 0; i < VALUE.size() && i < tmpRec.nfields(); i++){\
            if(tmpRec.dataType(i) == TpRecord) {                              \
                tmp->fromRecord(tmpRec.asRecord(i));                          \
                if(*VALUE[i] != *tmp) {                                       \
                    *VALUE[i] = *tmp;                                         \
                    valuesChanged = true;                                     \
                }                                                             \
            }                                                                 \
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

#define PMS_PP_COPYOP_PARAMVECREF(PARAM, FACTMETHOD)                          \
    PARAM.resize(o->PARAM.size());                                            \
    for(unsigned int i = 0; i < PARAM.size(); i++)                            \
        PARAM[i] = factory()->FACTMETHOD(*o->PARAM[i]);

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
#define PMS_PP_EQUALOP_PARAMVEC(PARAM)                                        \
    if(PARAM.size() != o->PARAM.size()) return false;                         \
    for(unsigned int i = 0; i < PARAM.size(); i++)                            \
        if(*PARAM[i] != *o->PARAM[i]) return false;

#define PMS_PP_EQUALOP_PARAMFLAG(FLAG, PARAM)                                 \
    if(FLAG != o->FLAG || (FLAG && PARAM != o->PARAM)) return false;
#define PMS_PP_EQUALOP_PARAMFLAGREF(FLAG, PARAM)                              \
    if(FLAG != o->FLAG || (FLAG && *PARAM != *o->PARAM)) return false;

#define PMS_PP_EQUALOP_PARAMFLAGVEC(FLAG, PARAM)                              \
    if(FLAG.size() != o->FLAG.size() || PARAM.size() != o->PARAM.size() ||    \
       FLAG.size() != PARAM.size()) return false;                             \
    for(unsigned int i = 0; i < FLAG.size(); i++)                             \
        if(FLAG[i] && PARAM[i] != o->PARAM[i]) return false;

#define PMS_PP_EQUALOP_PARAMFLAGVECREF(FLAG, PARAM)                           \
    if(FLAG.size() != o->FLAG.size() || PARAM.size() != o->PARAM.size() ||    \
       FLAG.size() != PARAM.size()) return false;                             \
    for(unsigned int i = 0; i < FLAG.size(); i++)                             \
        if(FLAG[i] && *PARAM[i] != *o->PARAM[i]) return false;

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
PMS_PP_RECKEY(PMS_PP_Cache, REC_XAXES,     "xaxes")
PMS_PP_RECKEY(PMS_PP_Cache, REC_YAXES,     "yaxes")
PMS_PP_RECKEY(PMS_PP_Cache, REC_XDATACOLS, "xdatacolumns")
PMS_PP_RECKEY(PMS_PP_Cache, REC_YDATACOLS, "ydatacolumns")

// PMS_PP_Cache constructors/destructors.
PMS_PP_CONSTRUCTORS(PMS_PP_Cache)

// PMS_PP_Cache::toRecord().
PMS_PP_TORECORD_START(PMS_PP_Cache)
    PMS_PP_TORECORD_DEF(REC_XAXES,     PMS::toIntVector<PMS::Axis>(itsXAxes_))
    PMS_PP_TORECORD_DEF(REC_YAXES,     PMS::toIntVector<PMS::Axis>(itsYAxes_))
    PMS_PP_TORECORD_DEF(REC_XDATACOLS, PMS::toIntVector<PMS::DataColumn>(itsXData_))
    PMS_PP_TORECORD_DEF(REC_YDATACOLS, PMS::toIntVector<PMS::DataColumn>(itsYData_))
PMS_PP_TORECORD_END

// PMS_PP_Cache::fromRecord().
PMS_PP_FROMRECORD_START(PMS_PP_Cache)
    PMS_PP_FROMRECORD_VALINTARR(itsXAxes_, REC_XAXES,     PMS::Axis)
    PMS_PP_FROMRECORD_VALINTARR(itsYAxes_, REC_YAXES,     PMS::Axis)
    PMS_PP_FROMRECORD_VALINTARR(itsXData_, REC_XDATACOLS, PMS::DataColumn)
    PMS_PP_FROMRECORD_VALINTARR(itsYData_, REC_YDATACOLS, PMS::DataColumn)
PMS_PP_FROMRECORD_END

// PMS_PP_Cache::operator=().
PMS_PP_COPYOP_START(PMS_PP_Cache)
    PMS_PP_COPYOP_PARAM(itsXAxes_)
    PMS_PP_COPYOP_PARAM(itsYAxes_)
    PMS_PP_COPYOP_PARAM(itsXData_)
    PMS_PP_COPYOP_PARAM(itsYData_)
PMS_PP_COPYOP_END

// PMS_PP_Cache::operator==().
PMS_PP_EQUALOP_START(PMS_PP_Cache)
    PMS_PP_EQUALOP_PARAM(itsXAxes_)
    PMS_PP_EQUALOP_PARAM(itsYAxes_)
    PMS_PP_EQUALOP_PARAM(itsXData_)
    PMS_PP_EQUALOP_PARAM(itsYData_)
PMS_PP_EQUALOP_END

// PMS_PP_Cache::setDefaults().
PMS_PP_SETDEFAULTS_START(PMS_PP_Cache)
    PMS_PP_SETDEFAULTS_PARAM(itsXAxes_, vector<PMS::Axis>(1, PMS::DEFAULT_XAXIS))
    PMS_PP_SETDEFAULTS_PARAM(itsYAxes_, vector<PMS::Axis>(1, PMS::DEFAULT_YAXIS))
    PMS_PP_SETDEFAULTS_PARAM(itsXData_, vector<PMS::DataColumn>(1, PMS::DEFAULT_DATACOLUMN))
    PMS_PP_SETDEFAULTS_PARAM(itsYData_, vector<PMS::DataColumn>(1, PMS::DEFAULT_DATACOLUMN))
PMS_PP_SETDEFAULTS_END

unsigned int PMS_PP_Cache::numXAxes() const { return itsXAxes_.size(); }
unsigned int PMS_PP_Cache::numYAxes() const { return itsYAxes_.size(); }

void PMS_PP_Cache::setAxes(const PMS::Axis& xAxis, const PMS::Axis& yAxis,
        const PMS::DataColumn& xData, const PMS::DataColumn& yData,
        unsigned int index) {
    if(itsXAxes_[index] != xAxis || itsYAxes_[index] != yAxis ||
       itsXData_[index] != xData || itsYData_[index] != yData) {
        itsXAxes_[index] = xAxis;
        itsYAxes_[index] = yAxis;
        itsXData_[index] = xData;
        itsYData_[index] = yData;
        updated();
    }
}


/////////////////////////////
// PMS_PP_AXES DEFINITIONS //
/////////////////////////////

// PMS_PP_Axes record keys.
PMS_PP_RECKEY(PMS_PP_Axes, REC_XAXES,      "canvasXAxes")
PMS_PP_RECKEY(PMS_PP_Axes, REC_YAXES,      "canvasYAxes")
PMS_PP_RECKEY(PMS_PP_Axes, REC_XRANGESSET, "xRangesSet")
PMS_PP_RECKEY(PMS_PP_Axes, REC_YRANGESSET, "yRangesSet")
PMS_PP_RECKEY(PMS_PP_Axes, REC_XRANGES,    "xRanges")
PMS_PP_RECKEY(PMS_PP_Axes, REC_YRANGES,    "yRanges")

// PMS_PP_Axes constructors/destructors.
PMS_PP_CONSTRUCTORS(PMS_PP_Axes)

// PMS_PP_Axes::toRecord().
PMS_PP_TORECORD_START(PMS_PP_Axes)
    PMS_PP_TORECORD_DEF(REC_XAXES,      PMS::toIntVector<PlotAxis>(itsXAxes_))
    PMS_PP_TORECORD_DEF(REC_YAXES,      PMS::toIntVector<PlotAxis>(itsYAxes_))
    PMS_PP_TORECORD_DEF(REC_XRANGESSET, Vector<bool>(itsXRangesSet_))
    PMS_PP_TORECORD_DEF(REC_YRANGESSET, Vector<bool>(itsYRangesSet_))
    
    vector<double> firsts(itsXRanges_.size()), seconds(itsXRanges_.size());
    for(unsigned int i = 0; i < itsXRanges_.size(); i++) {
        firsts[i] = itsXRanges_[i].first;
        seconds[i] = itsXRanges_[i].second;
    }
    PMS_PP_TORECORD_DEFVEC(REC_XRANGES + ".first", firsts)
    PMS_PP_TORECORD_DEFVEC(REC_XRANGES + ".second", seconds)
    
    firsts.resize(itsYRanges_.size()); seconds.resize(itsYRanges_.size());
    for(unsigned int i = 0; i < itsYRanges_.size(); i++) {
        firsts[i] = itsYRanges_[i].first;
        seconds[i] = itsYRanges_[i].second;
    }
    PMS_PP_TORECORD_DEFVEC(REC_YRANGES + ".first", firsts)
    PMS_PP_TORECORD_DEFVEC(REC_YRANGES + ".second", seconds)
PMS_PP_TORECORD_END

// PMS_PP_Axes::fromRecord().
PMS_PP_FROMRECORD_START(PMS_PP_Axes)
    PMS_PP_FROMRECORD_VALINTARR(itsXAxes_, REC_XAXES, PlotAxis)
    PMS_PP_FROMRECORD_VALINTARR(itsYAxes_, REC_YAXES, PlotAxis)
    PMS_PP_FROMRECORD_VALTYPEARR(itsXRangesSet_, REC_XRANGESSET, TpArrayBool, asArrayBool, bool)
    PMS_PP_FROMRECORD_VALTYPEARR(itsYRangesSet_, REC_YRANGESSET, TpArrayBool, asArrayBool, bool)
    
    vector<double> firsts(itsXRanges_.size()), seconds(itsXRanges_.size());
    for(unsigned int i = 0; i < itsXRanges_.size(); i++) {
        firsts[i] = itsXRanges_[i].first;
        seconds[i] = itsXRanges_[i].second;
    }
    PMS_PP_FROMRECORD_VEC(REC_XRANGES + ".first", firsts, TpDouble, asDouble)
    PMS_PP_FROMRECORD_VEC(REC_XRANGES + ".second", seconds, TpDouble, asDouble)
    itsXRanges_.resize(min((uInt)firsts.size(), (uInt)seconds.size()));
    for(unsigned int i = 0; i < itsXRanges_.size(); i++) {
        itsXRanges_[i].first = firsts[i];
        itsXRanges_[i].second = seconds[i];
    }
    
    firsts.resize(itsYRanges_.size()); seconds.resize(itsYRanges_.size());
    for(unsigned int i = 0; i < itsYRanges_.size(); i++) {
        firsts[i] = itsYRanges_[i].first;
        seconds[i] = itsYRanges_[i].second;
    }
    PMS_PP_FROMRECORD_VEC(REC_YRANGES + ".first", firsts, TpDouble, asDouble)
    PMS_PP_FROMRECORD_VEC(REC_YRANGES + ".second", seconds, TpDouble, asDouble)
    itsYRanges_.resize(min((uInt)firsts.size(), (uInt)seconds.size()));
    for(unsigned int i = 0; i < itsYRanges_.size(); i++) {
        itsYRanges_[i].first = firsts[i];
        itsYRanges_[i].second = seconds[i];
    }
PMS_PP_FROMRECORD_END

// PMS_PP_Axes::operator=().
PMS_PP_COPYOP_START(PMS_PP_Axes)
    PMS_PP_COPYOP_PARAM(itsXAxes_)
    PMS_PP_COPYOP_PARAM(itsYAxes_)
    PMS_PP_COPYOP_PARAM(itsXRangesSet_)
    PMS_PP_COPYOP_PARAM(itsYRangesSet_)
    PMS_PP_COPYOP_PARAM(itsXRanges_)
    PMS_PP_COPYOP_PARAM(itsYRanges_)
PMS_PP_COPYOP_END

// PMS_PP_Axes::operator==().
PMS_PP_EQUALOP_START(PMS_PP_Axes)
    PMS_PP_EQUALOP_PARAM(itsXAxes_)
    PMS_PP_EQUALOP_PARAM(itsYAxes_)
    PMS_PP_EQUALOP_PARAMFLAGVEC(itsXRangesSet_, itsXRanges_)
    PMS_PP_EQUALOP_PARAMFLAGVEC(itsYRangesSet_, itsYRanges_)
PMS_PP_EQUALOP_END

// PMS_PP_Axes::setDefaults().
PMS_PP_SETDEFAULTS_START(PMS_PP_Axes)
    PMS_PP_SETDEFAULTS_PARAM(itsXAxes_, vector<PlotAxis>(1, PMS::DEFAULT_CANVAS_XAXIS))
    PMS_PP_SETDEFAULTS_PARAM(itsYAxes_, vector<PlotAxis>(1, PMS::DEFAULT_CANVAS_YAXIS))
    PMS_PP_SETDEFAULTS_PARAM(itsXRangesSet_, vector<bool>(1, false))
    PMS_PP_SETDEFAULTS_PARAM(itsYRangesSet_, vector<bool>(1, false))
    PMS_PP_SETDEFAULTS_PARAM(itsXRanges_, vector<prange_t>(1, prange_t(0.0, 0.0)))
    PMS_PP_SETDEFAULTS_PARAM(itsYRanges_, vector<prange_t>(1, prange_t(0.0, 0.0)))
PMS_PP_SETDEFAULTS_END

unsigned int PMS_PP_Axes::numXAxes() const { return itsXAxes_.size(); }
unsigned int PMS_PP_Axes::numYAxes() const { return itsYAxes_.size(); }

void PMS_PP_Axes::setAxes(const PlotAxis& xAxis, const PlotAxis& yAxis,
        unsigned int index) {
    if(itsXAxes_[index] != xAxis || itsYAxes_[index] != yAxis) {
        itsXAxes_[index] = xAxis;
        itsYAxes_[index] = yAxis;
        updated();
    }
}

void PMS_PP_Axes::setRanges(const bool& xSet, const bool& ySet,
        const prange_t& xRange, const prange_t& yRange, unsigned int index) {
    if(itsXRangesSet_[index] != xSet || itsYRangesSet_[index] != ySet ||
       (xSet && itsXRanges_[index] != xRange) ||
       (ySet && itsYRanges_[index] != yRange)) {
        itsXRangesSet_[index] = xSet;
        itsYRangesSet_[index] = ySet;
        itsXRanges_[index] = xRange;
        itsYRanges_[index] = yRange;
    }
}


///////////////////////////////
// PMS_PP_CANVAS DEFINITIONS //
///////////////////////////////

// PMS_PP_Canvas record keys.
PMS_PP_RECKEY(PMS_PP_Canvas, REC_XLABELS,      "xLabelFormats")
PMS_PP_RECKEY(PMS_PP_Canvas, REC_YLABELS,      "yLabelFormats")
PMS_PP_RECKEY(PMS_PP_Canvas, REC_SHOWXAXES,    "showXAxes")
PMS_PP_RECKEY(PMS_PP_Canvas, REC_SHOWYAXES,    "showYAxes")
PMS_PP_RECKEY(PMS_PP_Canvas, REC_SHOWLEGENDS,  "showLegends")
PMS_PP_RECKEY(PMS_PP_Canvas, REC_LEGENDSPOS,   "legendPositions")
PMS_PP_RECKEY(PMS_PP_Canvas, REC_TITLES,       "canvasTitleFormats")
PMS_PP_RECKEY(PMS_PP_Canvas, REC_SHOWGRIDMAJS, "showGridMajors")
PMS_PP_RECKEY(PMS_PP_Canvas, REC_SHOWGRIDMINS, "showGridMinors")
PMS_PP_RECKEY(PMS_PP_Canvas, REC_GRIDMAJLINES, "gridMajorLines")
PMS_PP_RECKEY(PMS_PP_Canvas, REC_GRIDMINLINES, "gridMinorLines")

// PMS_PP_Canvas constructors/destructors.
PMS_PP_CONSTRUCTORS(PMS_PP_Canvas)

// PMS_PP_Canvas::toRecord().
PMS_PP_TORECORD_START(PMS_PP_Canvas)
    PMS_PP_TORECORD_DEFVECMETHOD(REC_XLABELS, itsXLabels_, .format)
    PMS_PP_TORECORD_DEFVECMETHOD(REC_YLABELS, itsYLabels_, .format)
    PMS_PP_TORECORD_DEF(REC_SHOWXAXES, Vector<bool>(itsXAxesShown_))
    PMS_PP_TORECORD_DEF(REC_SHOWYAXES, Vector<bool>(itsYAxesShown_))
    PMS_PP_TORECORD_DEF(REC_SHOWLEGENDS, Vector<bool>(itsLegendsShown_))
    PMS_PP_TORECORD_DEF(REC_LEGENDSPOS, PMS::toIntVector<PlotCanvas::LegendPosition>(itsLegendsPos_))
    PMS_PP_TORECORD_DEFVECMETHOD(REC_TITLES, itsTitles_, .format)
    PMS_PP_TORECORD_DEF(REC_SHOWGRIDMAJS, Vector<bool>(itsGridMajsShown_))
    PMS_PP_TORECORD_DEF(REC_SHOWGRIDMINS, Vector<bool>(itsGridMinsShown_))
    PMS_PP_TORECORD_DEFVECRECMETHOD(REC_GRIDMAJLINES, itsGridMajLines_, ->toRecord())
    PMS_PP_TORECORD_DEFVECRECMETHOD(REC_GRIDMINLINES, itsGridMinLines_, ->toRecord())
PMS_PP_TORECORD_END

// PMS_PP_Canvas::fromRecord().
PMS_PP_FROMRECORD_START(PMS_PP_Canvas)
    PMS_PP_FROMRECORD_VEC(REC_XLABELS, itsXLabels_, TpString, asString)
    PMS_PP_FROMRECORD_VEC(REC_YLABELS, itsYLabels_, TpString, asString)
    PMS_PP_FROMRECORD_VALTYPEARR(itsXAxesShown_, REC_SHOWXAXES, TpArrayBool, asArrayBool, bool)
    PMS_PP_FROMRECORD_VALTYPEARR(itsYAxesShown_, REC_SHOWYAXES, TpArrayBool, asArrayBool, bool)
    PMS_PP_FROMRECORD_VALTYPEARR(itsLegendsShown_, REC_SHOWLEGENDS, TpArrayBool, asArrayBool, bool)
    PMS_PP_FROMRECORD_VALINTARR(itsLegendsPos_, REC_LEGENDSPOS, PlotCanvas::LegendPosition)
    PMS_PP_FROMRECORD_VEC(REC_TITLES, itsTitles_, TpString, asString)
    PMS_PP_FROMRECORD_VALTYPEARR(itsGridMajsShown_, REC_SHOWGRIDMAJS, TpArrayBool, asArrayBool, bool)
    PMS_PP_FROMRECORD_VALTYPEARR(itsGridMinsShown_, REC_SHOWGRIDMINS, TpArrayBool, asArrayBool, bool)
    PMS_PP_FROMRECORD_VECREF(REC_GRIDMAJLINES, itsGridMajLines_, PlotLinePtr, line)
    PMS_PP_FROMRECORD_VECREF(REC_GRIDMINLINES, itsGridMinLines_, PlotLinePtr, line)
PMS_PP_FROMRECORD_END

// PMS_PP_Canvas::operator=().
PMS_PP_COPYOP_START(PMS_PP_Canvas)
    PMS_PP_COPYOP_PARAM(itsXLabels_)
    PMS_PP_COPYOP_PARAM(itsYLabels_)
    PMS_PP_COPYOP_PARAM(itsXAxesShown_)
    PMS_PP_COPYOP_PARAM(itsYAxesShown_)
    PMS_PP_COPYOP_PARAM(itsLegendsShown_)
    PMS_PP_COPYOP_PARAM(itsLegendsPos_)
    PMS_PP_COPYOP_PARAM(itsTitles_)
    PMS_PP_COPYOP_PARAM(itsGridMajsShown_)
    PMS_PP_COPYOP_PARAM(itsGridMinsShown_)    
    PMS_PP_COPYOP_PARAMVECREF(itsGridMajLines_, line)
    PMS_PP_COPYOP_PARAMVECREF(itsGridMinLines_, line)
PMS_PP_COPYOP_END

// PMS_PP_Canvas::operator==().
PMS_PP_EQUALOP_START(PMS_PP_Canvas)
    PMS_PP_EQUALOP_PARAM(itsXLabels_)
    PMS_PP_EQUALOP_PARAM(itsYLabels_)
    PMS_PP_EQUALOP_PARAM(itsXAxesShown_)
    PMS_PP_EQUALOP_PARAM(itsYAxesShown_)
    PMS_PP_EQUALOP_PARAMFLAGVEC(itsLegendsShown_, itsLegendsPos_)
    PMS_PP_EQUALOP_PARAM(itsTitles_)
    PMS_PP_EQUALOP_PARAMFLAGVECREF(itsGridMajsShown_, itsGridMajLines_)
    PMS_PP_EQUALOP_PARAMFLAGVECREF(itsGridMinsShown_, itsGridMinLines_)
PMS_PP_EQUALOP_END

// PMS_PP_Canvas::setDefaults().
PMS_PP_SETDEFAULTS_START(PMS_PP_Canvas)
    PMS_PP_SETDEFAULTS_PARAM(itsXLabels_, vector<PlotMSLabelFormat>(1, PlotMSLabelFormat(PMS::DEFAULT_CANVAS_AXIS_LABEL_FORMAT)))
    PMS_PP_SETDEFAULTS_PARAM(itsYLabels_, vector<PlotMSLabelFormat>(1, PlotMSLabelFormat(PMS::DEFAULT_CANVAS_AXIS_LABEL_FORMAT)))
    PMS_PP_SETDEFAULTS_PARAM(itsXAxesShown_, vector<bool>(1, PMS::DEFAULT_SHOWAXIS))
    PMS_PP_SETDEFAULTS_PARAM(itsYAxesShown_, vector<bool>(1, PMS::DEFAULT_SHOWAXIS))
    PMS_PP_SETDEFAULTS_PARAM(itsLegendsShown_, vector<bool>(1, PMS::DEFAULT_SHOWLEGEND))
    PMS_PP_SETDEFAULTS_PARAM(itsLegendsPos_, vector<PlotCanvas::LegendPosition>(1, PMS::DEFAULT_LEGENDPOSITION))
    PMS_PP_SETDEFAULTS_PARAM(itsTitles_, vector<PlotMSLabelFormat>(1, PlotMSLabelFormat(PMS::DEFAULT_TITLE_FORMAT)))
    PMS_PP_SETDEFAULTS_PARAM(itsGridMajsShown_, vector<bool>(1, PMS::DEFAULT_SHOW_GRID))
    PMS_PP_SETDEFAULTS_PARAM(itsGridMinsShown_, vector<bool>(1, PMS::DEFAULT_SHOW_GRID))
    PMS_PP_SETDEFAULTS_PARAM(itsGridMajLines_, vector<PlotLinePtr>(1, PMS::DEFAULT_GRID_LINE(factory())))
    PMS_PP_SETDEFAULTS_PARAM(itsGridMinLines_, vector<PlotLinePtr>(1, PMS::DEFAULT_GRID_LINE(factory())))
PMS_PP_SETDEFAULTS_END

unsigned int PMS_PP_Canvas::numCanvases() const { return itsXLabels_.size(); }

void PMS_PP_Canvas::setLabelFormats(const PlotMSLabelFormat& xFormat,
        const PlotMSLabelFormat& yFormat, unsigned int index) {
    if(itsXLabels_[index] != xFormat || itsYLabels_[index] != yFormat) {
        itsXLabels_[index] = xFormat;
        itsYLabels_[index] = yFormat;
        updated();
    }
}

void PMS_PP_Canvas::showAxes(const bool& xShow, const bool& yShow,
        unsigned int index) {
    if(itsXAxesShown_[index] != xShow || itsYAxesShown_[index] != yShow) {
        itsXAxesShown_[index] = xShow;
        itsYAxesShown_[index] = yShow;
        updated();
    }
}

void PMS_PP_Canvas::showLegend(const bool& show,
        const PlotCanvas::LegendPosition& pos, unsigned int index) {
    if(itsLegendsShown_[index]!= show|| (show && itsLegendsPos_[index]!= pos)){
        itsLegendsShown_[index] = show;
        itsLegendsPos_[index] = pos;
        updated();
    }
}

void PMS_PP_Canvas::showGrid(const bool& showMajor, const bool& showMinor,
        const PlotLinePtr& majorLine, const PlotLinePtr& minorLine,
        unsigned int index) {
    if(itsGridMajsShown_[index] != showMajor ||
       (showMajor && *itsGridMajLines_[index] != *majorLine) ||
       itsGridMinsShown_[index] != showMinor ||
       (showMinor && *itsGridMinLines_[index] != *minorLine)) {
        itsGridMajsShown_[index] = showMajor;
        itsGridMinsShown_[index] = showMinor;
        *itsGridMajLines_[index] = *majorLine;
        *itsGridMinLines_[index] = *minorLine;
        updated();
    }
}


////////////////////////////////
// PMS_PP_DISPLAY DEFINITIONS //
////////////////////////////////

// PMS_PP_Display record keys.
PMS_PP_RECKEY(PMS_PP_Display, REC_UNFLAGGEDS, "unflaggedSymbols")
PMS_PP_RECKEY(PMS_PP_Display, REC_FLAGGEDS,   "flaggedSymbols")
PMS_PP_RECKEY(PMS_PP_Display, REC_TITLES,     "titles")
PMS_PP_RECKEY(PMS_PP_Display, REC_COLFLAGS,   "colorizeFlags")
PMS_PP_RECKEY(PMS_PP_Display, REC_COLAXES,    "colorizeAxes")

// PMS_PP_Display constructors/destructors.
PMS_PP_CONSTRUCTORS(PMS_PP_Display)

// PMS_PP_Display::toRecord().
PMS_PP_TORECORD_START(PMS_PP_Display)
    PMS_PP_TORECORD_DEFVECRECMETHOD(REC_UNFLAGGEDS, itsUnflaggedSymbols_, ->toRecord())
    PMS_PP_TORECORD_DEFVECRECMETHOD(REC_FLAGGEDS, itsFlaggedSymbols_, ->toRecord())
    PMS_PP_TORECORD_DEFVECMETHOD(REC_TITLES, itsTitleFormats_, .format)
    PMS_PP_TORECORD_DEF(REC_COLFLAGS, Vector<bool>(itsColorizeFlags_))
    PMS_PP_TORECORD_DEF(REC_COLAXES,  PMS::toIntVector<PMS::Axis>(itsColorizeAxes_))
PMS_PP_TORECORD_END

// PMS_PP_Display::fromRecord().
PMS_PP_FROMRECORD_START(PMS_PP_Display)
    PMS_PP_FROMRECORD_VECREF(REC_UNFLAGGEDS, itsUnflaggedSymbols_, PlotSymbolPtr, symbol)
    PMS_PP_FROMRECORD_VECREF(REC_FLAGGEDS, itsFlaggedSymbols_, PlotSymbolPtr, symbol)
    PMS_PP_FROMRECORD_VEC(REC_TITLES, itsTitleFormats_, TpString, asString)
    PMS_PP_FROMRECORD_VALTYPEARR(itsColorizeFlags_, REC_COLFLAGS, TpArrayBool, asArrayBool, bool)
    PMS_PP_FROMRECORD_VALINTARR(itsColorizeAxes_, REC_COLAXES, PMS::Axis)
PMS_PP_FROMRECORD_END

// PMS_PP_Display::operator=().
PMS_PP_COPYOP_START(PMS_PP_Display)
    PMS_PP_COPYOP_PARAMVECREF(itsUnflaggedSymbols_, symbol)
    PMS_PP_COPYOP_PARAMVECREF(itsFlaggedSymbols_, symbol)
    PMS_PP_COPYOP_PARAM(itsTitleFormats_)
    PMS_PP_COPYOP_PARAM(itsColorizeFlags_)
    PMS_PP_COPYOP_PARAM(itsColorizeAxes_)
PMS_PP_COPYOP_END

// PMS_PP_Display::operator==().
PMS_PP_EQUALOP_START(PMS_PP_Display)
    PMS_PP_EQUALOP_PARAMVEC(itsUnflaggedSymbols_)
    PMS_PP_EQUALOP_PARAMVEC(itsFlaggedSymbols_)
    PMS_PP_EQUALOP_PARAM(itsTitleFormats_)
    PMS_PP_EQUALOP_PARAMFLAGVEC(itsColorizeFlags_, itsColorizeAxes_)
PMS_PP_EQUALOP_END

// PMS_PP_Display::setDefaults().
PMS_PP_SETDEFAULTS_START(PMS_PP_Display)
    PMS_PP_SETDEFAULTS_PARAM(itsUnflaggedSymbols_, vector<PlotSymbolPtr>(1, PMS::DEFAULT_UNFLAGGED_SYMBOL(factory())))
    PMS_PP_SETDEFAULTS_PARAM(itsFlaggedSymbols_, vector<PlotSymbolPtr>(1, PMS::DEFAULT_FLAGGED_SYMBOL(factory())))
    PMS_PP_SETDEFAULTS_PARAM(itsTitleFormats_, vector<PlotMSLabelFormat>(1, PlotMSLabelFormat(PMS::DEFAULT_TITLE_FORMAT)))
    PMS_PP_SETDEFAULTS_PARAM(itsColorizeFlags_, vector<bool>(1, false))
    PMS_PP_SETDEFAULTS_PARAM(itsColorizeAxes_, vector<PMS::Axis>(1, PMS::DEFAULT_COLOR_AXIS))
PMS_PP_SETDEFAULTS_END

void PMS_PP_Display::setColorize(const bool& colorize, const PMS::Axis& axis,
        unsigned int index) {
    if(itsColorizeFlags_[index]!= colorize || itsColorizeAxes_[index]!= axis) {
        itsColorizeFlags_[index] = colorize;
        itsColorizeAxes_[index] = axis;
        updated();
    }
}

void PMS_PP_Display::resizeVectors(unsigned int newSize) {
    if(newSize == 0) newSize = 1;
    itsUnflaggedSymbols_.resize(newSize);
    itsFlaggedSymbols_.resize(newSize);
    itsTitleFormats_.resize(newSize, PlotMSLabelFormat(PMS::DEFAULT_TITLE_FORMAT));
    itsColorizeFlags_.resize(newSize, false);
    itsColorizeAxes_.resize(newSize, PMS::DEFAULT_COLOR_AXIS);
    
    for(unsigned int i = 0; i < newSize; i++) {
        if(itsUnflaggedSymbols_[i].null())
            itsUnflaggedSymbols_[i] = PMS::DEFAULT_UNFLAGGED_SYMBOL(factory());
        if(itsFlaggedSymbols_[i].null())
            itsFlaggedSymbols_[i] = PMS::DEFAULT_FLAGGED_SYMBOL(factory());
    }
}

}
