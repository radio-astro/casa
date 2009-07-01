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

    year = t.month();
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


const PMS::Axis PMS::DEFAULT_XAXIS = TIME;
const PMS::Axis PMS::DEFAULT_YAXIS = AMP;
const PMS::DataColumn PMS::DEFAULT_DATACOLUMN = DATA;

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

PlotSymbolPtr PMS::DEFAULT_SYMBOL(PlotFactoryPtr factory) {
    static PlotSymbolPtr symbol = factory->symbol(PlotSymbol::CIRCLE);
    symbol->setSize(2, 2);
    symbol->setLine("#000000", PlotLine::NOLINE, 1.0);
    symbol->setAreaFill("#0000FF");
    return factory->symbol(*symbol);
}

PlotSymbolPtr PMS::DEFAULT_MASKED_SYMBOL(PlotFactoryPtr factory) {
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

const String PMS::DEFAULT_TITLE_FORMAT =
    PlotMSLabelFormat::TAG(PlotMSLabelFormat::TAG_YAXIS()) + " vs. " +
    PlotMSLabelFormat::TAG(PlotMSLabelFormat::TAG_XAXIS());

const bool PMS::DEFAULT_SHOW_GRID = false;

PlotLinePtr PMS::DEFAULT_GRID_LINE(PlotFactoryPtr factory) {
    static PlotLinePtr line = factory->line("gray");
    return factory->line(*line);
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


const int PMS::LOG_LOAD_CACHE = PlotLogger::REGISTER_EVENT_TYPE("LOAD_CACHE");
const int PMS::LOG_DBUS = PlotLogger::REGISTER_EVENT_TYPE("DBUS",
                          LogMessage::NORMAL3);

}
