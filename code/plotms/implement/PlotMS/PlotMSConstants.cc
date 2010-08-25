//# PlotMSConstants.cc: Constants and useful classes/methods for plotms.
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
#include <plotms/PlotMS/PlotMSConstants.h>

#include <casa/OS/Time.h>
#include <plotms/PlotMS/PlotMSLabelFormat.h>
#include <plotms/PlotMS/PlotMSWatchedParameters.h>

#include <ctype.h>

namespace casa {

/////////////////////
// PMS DEFINITIONS //
/////////////////////

PlotAxisScale PMS::axisScale(Axis axis) {
    switch(axis) {
    
    case TIME: return DATE_MJ_SEC;
    
    default: return NORMAL;
    }
}

bool PMS::axisIsData(Axis axis) {
    switch(axis) {
    case AMP: case PHASE: case REAL: case IMAG: return true;
    default: return false;
    }
}

PMS::AxisType PMS::axisType(Axis axis) {
    switch(axis) {
    case FLAG:
        return TBOOL;
    
    case FIELD: case SCAN: case SPW: case CHANNEL: case CORR:
    case ANTENNA1: case ANTENNA2: case BASELINE:
        return TINT;
    
    case AMP: case PHASE: case REAL: case IMAG: case PARANG:
        return TFLOAT;
    
    case TIME: case TIME_INTERVAL:
        return TTIME;
        
    default: return TDOUBLE;
    }
}

PMS::AxisUnit PMS::axisUnit(Axis axis) {
    switch(axis) {
    case TIME: return UDATETIME;
    
    default: return UNONE;
    }
}


double PMS::dateDouble(unsigned int year, unsigned int mon, unsigned int day,
        unsigned int hour, unsigned int min, double sec, PlotAxisScale scale) {
    Time t(year, mon, day, hour, min, sec);
    if(scale == DATE_MJ_SEC) return t.modifiedJulianDay() * 86400;
    else                     return t.modifiedJulianDay();
}

void PMS::dateDouble(double value, unsigned int& year, unsigned int& mon,
        unsigned int& day, unsigned int& hour, unsigned int& min, double& sec,
        PlotAxisScale scale) {
    if(scale == DATE_MJ_SEC) value /= 86400;
    
    Time t(value + 2400000.5);

    year = t.year();
    mon = t.month();
    day = t.dayOfMonth();
    hour = t.hours();
    min = t.minutes();
    
    sec = modf(value, &sec);
    sec += t.seconds();
}

bool PMS::strEq(const String& str1, const String& str2, bool ignoreCase) {
    if(str1.size() != str2.size()) return false;
    if(!ignoreCase) return str1 == str2;
    for(unsigned int i = 0; i < str1.size(); i++)
        if(tolower(str1[i]) != tolower(str2[i])) return false;
    
    return true;
}

#define PMS_RE(TYPE, ASTYPE) \
    case TYPE: if(rec1.ASTYPE(name) != rec2.ASTYPE(name)) return false; break;

#define PMS_REA(TYPE, ASTYPE, ATYPE)                                          \
    case TYPE: {                                                              \
        if(rec1. ASTYPE (name).shape() != rec2. ASTYPE (name).shape())        \
            return false;                                                     \
        const Array< ATYPE >& a1 = rec1.ASTYPE(name), &a2 = rec2.ASTYPE(name);\
        Array< ATYPE >::const_iterator it1 = a1.begin(), it2 = a2.begin();    \
        while(it1 != a1.end() && it2 != a2.end()) {                           \
            if(*it1 != *it2) return false;                                    \
            it1++; it2++;                                                     \
        }                                                                     \
        break; }

bool PMS::recEq(const Record& rec1, const Record& rec2) {
    if(rec1.nfields() != rec2.nfields()) return false;

    String name;
    for(unsigned int i = 0; i < rec1.nfields(); i++) {
        name = rec1.name(i);
        if(!rec2.isDefined(name) || rec1.dataType(name) != rec2.dataType(name))
            return false;
        
        switch(rec1.dataType(name)) {
        PMS_RE(TpBool, asBool)
        // PMS_RE(TpChar, asChar) no asChar
        PMS_RE(TpUChar, asuChar)
        PMS_RE(TpShort, asShort)
        PMS_RE(TpInt, asInt)
        PMS_RE(TpUInt, asuInt)
        PMS_RE(TpFloat, asFloat)
        PMS_RE(TpDouble, asDouble)
        PMS_RE(TpComplex, asComplex)
        PMS_RE(TpDComplex, asDComplex)
        PMS_RE(TpString, asString)
        PMS_REA(TpArrayBool, asArrayBool, Bool)
        //PMS_REA(TpArrayChar, asArrayChar, Char) no asArrayChar
        PMS_REA(TpArrayUChar, asArrayuChar, uChar)
        PMS_REA(TpArrayShort, asArrayShort, Short)
        //PMS_REA(TpArrayUShort, asArrayuShort, uShort) no assArrayuShort
        PMS_REA(TpArrayInt, asArrayInt, Int)
        PMS_REA(TpArrayUInt, asArrayuInt, uInt)
        PMS_REA(TpArrayFloat, asArrayFloat, Float)
        PMS_REA(TpArrayDouble, asArrayDouble, Double)
        PMS_REA(TpArrayComplex, asArrayComplex, Complex)
        PMS_REA(TpArrayDComplex, asArrayDComplex, DComplex)
        PMS_REA(TpArrayString, asArrayString, String)
        
        case TpRecord:
            if(!recEq(rec1.asRecord(name), rec2.asRecord(name)))
                return false;
            break;

        default: break;
        }
    }
    
    return true;
}


const vector<String>& PMS::COLORS_LIST() {
    static vector<String> colors;
    if(colors.size() == 0) {
        colors.resize(10);
		colors[0] = "#202020";
		colors[1] = "#E00066";
		colors[2] = "#E07600";
		colors[3] = "#66D000";
		colors[4] = "#AC6600";
		colors[5] = "#0091A0";
		colors[6] = "#10E050";
		colors[7] = "#6600E0";
		colors[8] = "#0066F0";
		colors[9] = "#A868D8";
	}
    
	return colors;
}


const String PMS::DEFAULT_LOG_FILENAME = "";
const int PMS::DEFAULT_LOG_EVENTS = PlotLogger::NO_EVENTS;
const LogMessage::Priority PMS::DEFAULT_LOG_PRIORITY = LogMessage::DEBUGGING;
const bool PMS::DEFAULT_CLEAR_SELECTIONS = true;
const int PMS::DEFAULT_CACHED_IMAGE_WIDTH = -1;
const int PMS::DEFAULT_CACHED_IMAGE_HEIGHT = -1;

const PMS::Axis PMS::DEFAULT_XAXIS = TIME;
const PMS::Axis PMS::DEFAULT_YAXIS = AMP;
const PMS::DataColumn PMS::DEFAULT_DATACOLUMN = DATA;
const PMS::Axis PMS::DEFAULT_COLOR_AXIS = SPW;

const PlotAxis PMS::DEFAULT_CANVAS_XAXIS = X_BOTTOM;
const PlotAxis PMS::DEFAULT_CANVAS_YAXIS = Y_LEFT;
const String PMS::DEFAULT_CANVAS_AXIS_LABEL_FORMAT =
    PlotMSLabelFormat::TAG(PlotMSLabelFormat::TAG_AXIS()) +
    PlotMSLabelFormat::TAG(PlotMSLabelFormat::TAG_IF_REFVALUE()) + " (from " +
    PlotMSLabelFormat::TAG(PlotMSLabelFormat::TAG_REFVALUE()) + ")" +
    PlotMSLabelFormat::TAG(PlotMSLabelFormat::TAG_ENDIF_REFVALUE());
const bool PMS::DEFAULT_SHOWAXIS = true;
const bool PMS::DEFAULT_SHOWLEGEND = false;
const PlotCanvas::LegendPosition PMS::DEFAULT_LEGENDPOSITION =
    PlotCanvas::INT_URIGHT;
const bool PMS::DEFAULT_SHOW_GRID = false;

PlotLinePtr PMS::DEFAULT_GRID_LINE(PlotFactoryPtr factory) {
    static PlotLinePtr line = factory->line("gray");
    return factory->line(*line);
}

const String PMS::DEFAULT_TITLE_FORMAT =
    PlotMSLabelFormat::TAG(PlotMSLabelFormat::TAG_YAXIS()) + " vs. " +
    PlotMSLabelFormat::TAG(PlotMSLabelFormat::TAG_XAXIS());

PlotSymbolPtr PMS::DEFAULT_UNFLAGGED_SYMBOL(PlotFactoryPtr factory) {
    static PlotSymbolPtr symbol = factory->symbol(PlotSymbol::CIRCLE);
    symbol->setSize(2, 2);
    symbol->setLine("#000000", PlotLine::NOLINE, 1.0);
    symbol->setAreaFill("#0000FF");
    return factory->symbol(*symbol);
}

PlotSymbolPtr PMS::DEFAULT_FLAGGED_SYMBOL(PlotFactoryPtr factory) {
    static PlotSymbolPtr symbol = factory->symbol(PlotSymbol::NOSYMBOL);
    symbol->setSize(2, 2);
    symbol->setLine("#000000", PlotLine::NOLINE, 1.0);
    symbol->setAreaFill("#FF0000");
    return factory->symbol(*symbol);
}

map<PlotSymbol::Symbol, int> PMS::SYMBOL_MINIMUM_SIZES() {
    map<PlotSymbol::Symbol, int> m;
    
    m[PlotSymbol::CIRCLE] = 2;
    m[PlotSymbol::DIAMOND] = 3;
    
    return m;
}

PlotFontPtr PMS::DEFAULT_ANNOTATION_TEXT_FONT(PlotFactoryPtr factory) {
    static PlotFontPtr font = factory->font();
    return factory->font(*font);
}

PlotLinePtr PMS::DEFAULT_ANNOTATION_TEXT_OUTLINE(PlotFactoryPtr factory) {
    static PlotLinePtr line = factory->line("000000", PlotLine::NOLINE, 1.0);
    return factory->line(*line);
}

PlotAreaFillPtr PMS::DEFAULT_ANNOTATION_TEXT_BACKGROUND(PlotFactoryPtr f) {
    static PlotAreaFillPtr fill = f->areaFill("0000FF", PlotAreaFill::NOFILL);
    return f->areaFill(*fill);
}

PlotLinePtr PMS::DEFAULT_ANNOTATION_RECT_LINE(PlotFactoryPtr factory) {
    static PlotLinePtr line = factory->line("000000");
    return factory->line(*line);
}

PlotAreaFillPtr PMS::DEFAULT_ANNOTATION_RECT_FILL(PlotFactoryPtr f) {
    static PlotAreaFillPtr fill = f->areaFill("0000FF", PlotAreaFill::NOFILL);
    return f->areaFill(*fill);
}


const String PMS::LOG_ORIGIN = "PlotMS";

// Macro to help with defining log names and flags.
#define PMS_LOG(TYPE, NAME, PRIORITY)                                         \
const String PMS::LOG_ORIGIN_##TYPE = NAME;                                   \
const int PMS::LOG_EVENT_##TYPE =                                             \
    PlotLogger::REGISTER_EVENT_TYPE("plotms_" + LOG_ORIGIN_##TYPE,            \
                                    LogMessage::PRIORITY);

PMS_LOG(DBUS, "dbus", NORMAL3)
PMS_LOG(FLAG, "flag", NORMAL)
PMS_LOG(LOAD_CACHE, "load_cache", NORMAL)
PMS_LOG(LOCATE, "locate", NORMAL)
PMS_LOG(PARAMS_CHANGED, "params_changed", NORMAL)
PMS_LOG(PLOT, "plot", NORMAL)
PMS_LOG(RELEASE_CACHE, "release_cache", NORMAL)
PMS_LOG(UNFLAG, "unflag", NORMAL)
PMS_LOG(SUMMARY, "summary", NORMAL)

}
