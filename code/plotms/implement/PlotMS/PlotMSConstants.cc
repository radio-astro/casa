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
#include <ms/MeasurementSets/MSSelectionTools.h>

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


/////////////////////////////////
// PLOTMSSELECTION DEFINITIONS //
/////////////////////////////////

// Static //

String PlotMSSelection::defaultValue(Field f) { return ""; }


// Non-Static //

PlotMSSelection::PlotMSSelection() {
    initDefaults();
}

PlotMSSelection::~PlotMSSelection() { }

void PlotMSSelection::apply(MeasurementSet& ms, MeasurementSet& selMS,
        Matrix<Int>& chansel) const {    
    // Set the selected MeasurementSet to be the same initially as the input
    // MeasurementSet
    selMS = ms;
    mssSetData(ms, selMS, "", timerange(), antenna(), field(), spw(),
               uvrange(), msselect(), corr(), scan(), array());

    MSSelection mss;
    mss.setSpwExpr(spw());
    chansel=mss.getChanList(&selMS);
}

const String& PlotMSSelection::getValue(Field f) const {
    return const_cast<map<Field,String>&>(itsValues_)[f]; }
void PlotMSSelection::setValue(Field f, const String& value) {
    itsValues_[f] = value; }

bool PlotMSSelection::operator==(const PlotMSSelection& other) const {    
    vector<Field> f = fields();
    for(unsigned int i = 0; i < f.size(); i++)
        if(getValue(f[i]) != other.getValue(f[i])) return false;
    
    return true;
}

void PlotMSSelection::initDefaults() {
    vector<Field> f = fields();
    for(unsigned int i = 0; i < f.size(); i++)
        itsValues_[f[i]] = defaultValue(f[i]);
}


///////////////////////////////////
// PLOTMSLABELFORMAT DEFINITIONS //
///////////////////////////////////

// Static //

const String PlotMSLabelFormat::TAGSEPARATOR = "%%";

const String PlotMSLabelFormat::TAG_AXIS = "axis";
const String PlotMSLabelFormat::TAG_XAXIS = "xaxis";
const String PlotMSLabelFormat::TAG_YAXIS = "yaxis";

const String PlotMSLabelFormat::TAG_UNIT = "unit";
const String PlotMSLabelFormat::TAG_XUNIT = "xunit";
const String PlotMSLabelFormat::TAG_YUNIT = "yunit";

const String PlotMSLabelFormat::TAG_IF_UNIT = "ifunit";
const String PlotMSLabelFormat::TAG_IF_XUNIT = "ifxunit";
const String PlotMSLabelFormat::TAG_IF_YUNIT = "ifyunit";
const String PlotMSLabelFormat::TAG_ENDIF_UNIT = "endifunit";
const String PlotMSLabelFormat::TAG_ENDIF_XUNIT = "endifxunit";
const String PlotMSLabelFormat::TAG_ENDIF_YUNIT = "endifyunit";

String PlotMSLabelFormat::getLabel(const String& format, PMS::Axis axis,
            PMS::Axis xAxis, PMS::Axis yAxis) {
    stringstream ss;
    
    PMS::AxisUnit unit = PMS::axisUnit(axis), xUnit = PMS::axisUnit(xAxis),
                  yUnit = PMS::axisUnit(yAxis);
    
    String tempFormat = format, token, tag;
    bool tokenWasTag, ifUnit = false, ifXUnit = false, ifYUnit = false;
    
    while(nextToken(tempFormat, token, tokenWasTag)) {
        if(tokenWasTag) {
            tag = "";
            
            if(PMS::strEq(token, TAG_AXIS, true)) tag = PMS::axis(axis);
            else if(PMS::strEq(token, TAG_XAXIS, true)) tag = PMS::axis(xAxis);
            else if(PMS::strEq(token, TAG_YAXIS, true)) tag =  PMS::axis(yAxis);
            else if(PMS::strEq(token, TAG_UNIT, true))
                tag = PMS::axisUnit(unit);
            else if(PMS::strEq(token, TAG_XUNIT, true))
                tag = PMS::axisUnit(xUnit);
            else if(PMS::strEq(token, TAG_YUNIT, true))
                tag = PMS::axisUnit(yUnit);
            else if(PMS::strEq(token, TAG_IF_UNIT, true))
                ifUnit = true;
            else if(PMS::strEq(token, TAG_IF_XUNIT, true))
                ifXUnit = true;
            else if(PMS::strEq(token, TAG_IF_YUNIT, true))
                ifYUnit = true;
            else if(PMS::strEq(token, TAG_ENDIF_UNIT, true))
                ifUnit = false;
            else if(PMS::strEq(token, TAG_ENDIF_XUNIT, true))
                ifXUnit = false;
            else if(PMS::strEq(token, TAG_ENDIF_YUNIT, true))
                ifYUnit = false;
            else tag = TAGSEPARATOR + token + TAGSEPARATOR;
        } else tag = token;
        
        if((!ifUnit || unit != PMS::UNONE) && (!ifXUnit || xUnit != PMS::UNONE)
           && (!ifYUnit || yUnit != PMS::UNONE)) ss << tag;
    }
    
    return ss.str();
}

bool PlotMSLabelFormat::nextToken(String& format, String& token,
        bool& tokenWasTag) {
    if(format.size() == 0) {
        token = "";
        tokenWasTag = false;
        return false;
    }
    
    unsigned int i = format.find(TAGSEPARATOR), j;
    if(i >= format.size() ||
       (j = format.find(TAGSEPARATOR, i + 1)) >= format.size()) {
        // no more tags left
        token = format;
        tokenWasTag = false;
        format = "";
        return true;
    }

    if(i == 0) {
        // tag is next token
        token = format.substr(TAGSEPARATOR.size(), j - TAGSEPARATOR.size());
        tokenWasTag = true;
        format = format.substr(j + TAGSEPARATOR.size());        
    } else {
        // text is next token
        token = format.substr(0, i);
        tokenWasTag = false;
        format = format.substr(i);
    }
    
    return true;
}


// Non-Static //

PlotMSLabelFormat::PlotMSLabelFormat(const String& f) : format(f) { }

PlotMSLabelFormat::PlotMSLabelFormat(const PlotMSLabelFormat& copy) {
    operator=(copy); }

PlotMSLabelFormat::~PlotMSLabelFormat() { }

String PlotMSLabelFormat::getLabel(PMS::Axis axis) const {
    return getLabel(format, axis, axis, axis); }

String PlotMSLabelFormat::getLabel(PMS::Axis xAxis, PMS::Axis yAxis) const {
    return getLabel(format, xAxis, xAxis, yAxis); }

bool PlotMSLabelFormat::operator==(const PlotMSLabelFormat& other) const {
    return format == other.format; }

PlotMSLabelFormat& PlotMSLabelFormat::operator=(const PlotMSLabelFormat& copy){
    format = copy.format;
    return *this;
}


//////////////////
// PMS DEFAULTS //
//////////////////

// Have to be defined after PlotMSLabelFormat definitions.

const PMS::Axis PMS::DEFAULT_XAXIS = TIME;
const PMS::Axis PMS::DEFAULT_YAXIS = AMP;
const PMS::DataColumn PMS::DEFAULT_DATACOLUMN = DATA;

const PlotAxis PMS::DEFAULT_CANVAS_XAXIS = X_BOTTOM;
const PlotAxis PMS::DEFAULT_CANVAS_YAXIS = Y_LEFT;

const String PMS::DEFAULT_CANVAS_AXIS_LABEL_FORMAT =
    PlotMSLabelFormat::TAG(PlotMSLabelFormat::TAG_AXIS) +
    PlotMSLabelFormat::TAG(PlotMSLabelFormat::TAG_IF_UNIT) + " (" +
    PlotMSLabelFormat::TAG(PlotMSLabelFormat::TAG_UNIT) + ")" +
    PlotMSLabelFormat::TAG(PlotMSLabelFormat::TAG_ENDIF_UNIT);

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

const String PMS::DEFAULT_TITLE_FORMAT =
    PlotMSLabelFormat::TAG(PlotMSLabelFormat::TAG_YAXIS) + " vs. " +
    PlotMSLabelFormat::TAG(PlotMSLabelFormat::TAG_XAXIS);

}
