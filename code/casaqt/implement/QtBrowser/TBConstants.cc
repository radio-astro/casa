//# TBConstants.cc: Constants, defaults, and common functions for the browser.
//# Copyright (C) 2005
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
#include <casaqt/QtBrowser/TBConstants.h>

#include <cctype>
#include <cmath>
#include <iomanip>

#include <QDir>
#include <QHBoxLayout>

#include <casa/OS/Time.h>
#include <casaqt/QtUtilities/QtUtilities.h>

#include <casa/namespace.h>

namespace casa {

///////////////////////////////
// TBLOCATEDROWS DEFINITIONS //
///////////////////////////////

// Constructors/Destructors //

TBLocatedRows::TBLocatedRows() { }

TBLocatedRows::~TBLocatedRows() {
    for(map<TBTableTabs*, vector<int>*>::iterator iter = results.begin();
        iter != results.end(); iter++) {
        delete iter->second;
    }
}

// Public Methods //

vector<TBTableTabs*> TBLocatedRows::tables() {
    vector<TBTableTabs*> t;

    for(map<TBTableTabs*, vector<int>*>::iterator iter = results.begin();
    iter != results.end(); iter++)
        if(iter->first != NULL) t.push_back(iter->first);

    return t;
}

void TBLocatedRows::put(TBTableTabs* tt, vector<int>* r) { results[tt] = r; }


/////////////////////////////
// TBCONSTANTS DEFINITIONS //
/////////////////////////////

/* Debugging Constants/Defaults */

// Current debug threshold.
int TBConstants::debugThreshold = DEBUG_OFF;

const int TBConstants::DEBUG_OFF = -1;
const int TBConstants::DEBUG_LOW = 0;
const int TBConstants::DEBUG_MED = 1;
const int TBConstants::DEBUG_HIGH = 2;

void TBConstants::dprint(int level, String message, int indentLevel) {
    if(debugThreshold <= level && debugThreshold != DEBUG_OFF) {
        String str = "";
        for(int i = 0; i < indentLevel; i++) str += '\t';
        cout << str << message << endl;
    }
}


/* Browsing Constants/Defaults */

const unsigned int TBConstants::DEFAULT_SELECT_NUM = 1000;
const unsigned int TBConstants::DEFAULT_EXPORT_NUM = 1000;
const unsigned int TBConstants::MAX_SELECT_NUM = 1000000;
const unsigned int TBConstants::MAX_ACTION_BUFFER = 100;
const unsigned int TBConstants::DEFAULT_ROW_INTERVAL = 5;

const String TBConstants::QUERY_QUERY = "query";
const String TBConstants::QUERY_ARRAY = "array";
const String TBConstants::QUERY_UPDATE = "updat";
const String TBConstants::QUERY_FULL = "qfull";

String TBConstants::compToString(Comparator c) {
    switch(c) {
    case EQUALS: return "equal to";
    case CONTAINS: return "contain";
    case BETWEEN: return "between";
    case CONTAINSBT: return "contain (between)";
    case LESSTHAN: return "less than";
    case CONTAINSLT: return "contain (less than)";
    case GREATERTHAN: return "greater than";
    case CONTAINSGT: return "contain (greater than)";
    }
    return "";
}

Comparator TBConstants::stringToComp(String str) {
    String s = "equal to";
    if(equalsIgnoreCase(str, s)) return EQUALS;
    s = "contain";
    if(equalsIgnoreCase(str, s)) return CONTAINS;
    s = "between";
    if(equalsIgnoreCase(str, s)) return BETWEEN;
    s = "contain (between)";
    if(equalsIgnoreCase(str, s)) return CONTAINSBT;
    s = "less than";
    if(equalsIgnoreCase(str, s)) return LESSTHAN;
    s = "contain (less than)";
    if(equalsIgnoreCase(str, s)) return CONTAINSLT;
    s = "greater than";
    if(equalsIgnoreCase(str, s)) return GREATERTHAN;
    s = "contain (greater than)";
    if(equalsIgnoreCase(str, s)) return CONTAINSGT;

    return EQUALS;
}

String TBConstants::bFormToString(BooleanFormat bf) {
    if(bf == DEFAULT) return "default";
    else if(bf == TRUEFALSE) return "true/false";
    else if(bf == TF) return "t/f";
    else if(bf == B10) return "1/0";
    else return "";
}

BooleanFormat TBConstants::stringToBForm(String str) {
    if(equalsIgnoreCase(str, "default")) return DEFAULT;
    else if(equalsIgnoreCase(str, "true/false")) return TRUEFALSE;
    else if(equalsIgnoreCase(str, "t/f")) return TF;
    else if(equalsIgnoreCase(str, "1/0")) return B10;
    
    return DEFAULT;
}

const int TBConstants::SLICER_ROW_AXIS = -1;
const int TBConstants::SLICER_COL_AXIS = -2;

const unsigned int TBConstants::DEFAULT_RECORD_VISIBLE_ROWS = 4;

String TBConstants::AIPS_PATH = "";
String TBConstants::aipspath() {
    if(AIPS_PATH.empty()) {
        char* ap = getenv("CASAPATH");
        if(ap != NULL) {
            String str(ap);
            unsigned int i = TBConstants::findWS(str);

            if(i < str.length()) {
                str = str.substr(0, i);
                TBConstants::strtrim(str);
                if(!str.empty()) AIPS_PATH = String(str);
            }
        }
    }

    return AIPS_PATH;
}

String TBConstants::DOT_CASAPY_DIR = "";

String TBConstants::dotCasapyDir() {
    if(DOT_CASAPY_DIR.empty()) {
        DOT_CASAPY_DIR = qPrintable(QDir::homePath());
        DOT_CASAPY_DIR += "/.casa/";
    }
    return DOT_CASAPY_DIR;
}

IPosition TBConstants::ipos(vector<int>& d) {
    IPosition ip(d.size());
    for(unsigned int i = 0; i < d.size(); i++)
        ip[i] = d[i];
    return ip;
}

vector<int> TBConstants::ipos(IPosition& d) {
    vector<int> ip(d.size());
    for(unsigned int i = 0; i < d.size(); i++)
        ip[i] = d[i];
    return ip;
}

bool TBConstants::increment(IPosition& shape, IPosition& d) {
    if(shape.size() != d.size()) return false;
    
    int ind = d.size() - 1;
    int val = d[ind] + 1;
    int max = shape[ind];
    while(0 <= ind && val >= max) {
        d[ind] = 0;
        ind--;
        if(ind < 0) return false;
        val = d[ind] + 1;
        max = shape[ind];
    }
    
    d[ind] = val;
    return true;
}

bool TBConstants::increment(vector<int>& shape, vector<int>& d) {
    if(shape.size() != d.size()) return false;
    
    int ind = d.size() - 1;
    int val = d[ind] + 1;
    int max = shape[ind];
    while(0 <= ind && val >= max) {
        d[ind] = 0;
        ind--;
        if(ind < 0) return false;
        val = d[ind] + 1;
        max = shape[ind];
    }
    
    d[ind] = val;
    return true;
}

void TBConstants::insert(QFrame* frame, QWidget* widget) {
    QtUtilities::putInFrame(frame, widget); }

void TBConstants::insert(QFrame* frame, QLayout* layout) {
    QtUtilities::putInFrame(frame, layout); }


/* Data Types and Related Methods */

const String TBConstants::TYPE_STRING = "TpString";
const String TBConstants::TYPE_DOUBLE = "TpDouble";
const String TBConstants::TYPE_FLOAT = "TpFloat";
const String TBConstants::TYPE_INT = "TpInt";
const String TBConstants::TYPE_UINT = "TpUInt";
const String TBConstants::TYPE_BOOL = "TpBool";
const String TBConstants::TYPE_CHAR = "TpChar";
const String TBConstants::TYPE_UCHAR = "TpUChar";
const String TBConstants::TYPE_SHORT = "TpShort";
const String TBConstants::TYPE_COMPLEX = "TpComplex";
const String TBConstants::TYPE_DCOMPLEX = "TpDComplex";
const String TBConstants::TYPE_TABLE = "TpTable";
const String TBConstants::TYPE_RECORD = "TpRecord";
const String TBConstants::TYPE_DATE = "TpDate";

const String TBConstants::TYPE_ARRAY_STRING = "TpArrayString";
const String TBConstants::TYPE_ARRAY_DOUBLE = "TpArrayDouble";
const String TBConstants::TYPE_ARRAY_FLOAT = "TpArrayFloat";
const String TBConstants::TYPE_ARRAY_INT = "TpArrayInt";
const String TBConstants::TYPE_ARRAY_UINT = "TpArrayUInt";
const String TBConstants::TYPE_ARRAY_CHAR = "TpArrayChar";
const String TBConstants::TYPE_ARRAY_UCHAR = "TpArrayUChar";
const String TBConstants::TYPE_ARRAY_BOOL = "TpArrayBool";
const String TBConstants::TYPE_ARRAY_SHORT = "TpArrayShort";
const String TBConstants::TYPE_ARRAY_COMPLEX = "TpArrayComplex";
const String TBConstants::TYPE_ARRAY_DCOMPLEX = "TpArrayDComplex";

// Unsupported types
// const String TYPE_USHORT = "TpUShort";
// const String TYPE_ARRAY_USHORT = "TpArrayUShort";
// const String TYPE_OTHER = "TpOther";

vector<String>* TBConstants::allTypes() {
    vector<String>* types = new vector<String>(23);
    (*types)[0] = TYPE_STRING;
    (*types)[1] = TYPE_FLOAT;
    (*types)[2] = TYPE_DOUBLE;
    (*types)[3] = TYPE_INT;
    (*types)[4] = TYPE_UINT;
    (*types)[5] = TYPE_BOOL;
    (*types)[6] = TYPE_UCHAR;
    (*types)[7] = TYPE_SHORT;
    (*types)[8] = TYPE_COMPLEX;
    (*types)[9] = TYPE_DCOMPLEX;
    (*types)[10] = TYPE_TABLE;
    (*types)[11] = TYPE_RECORD;
    (*types)[12] = TYPE_DATE;
    (*types)[13] = TYPE_ARRAY_STRING;
    (*types)[14] = TYPE_ARRAY_FLOAT;
    (*types)[15] = TYPE_ARRAY_DOUBLE;
    (*types)[16] = TYPE_ARRAY_INT;
    (*types)[17] = TYPE_ARRAY_UINT;
    (*types)[18] = TYPE_ARRAY_BOOL;
    (*types)[19] = TYPE_ARRAY_UCHAR;
    (*types)[20] = TYPE_ARRAY_SHORT;
    (*types)[21] = TYPE_ARRAY_COMPLEX;
    (*types)[22] = TYPE_ARRAY_DCOMPLEX;
    return types;
}

vector<String>* TBConstants::arrayTypes() {
    vector<String>* types = new vector<String>(10);
    (*types)[0] = TYPE_ARRAY_STRING;
    (*types)[1] = TYPE_ARRAY_FLOAT;
    (*types)[2] = TYPE_ARRAY_DOUBLE;
    (*types)[3] = TYPE_ARRAY_INT;
    (*types)[4] = TYPE_ARRAY_UINT;
    (*types)[5] = TYPE_ARRAY_BOOL;
    (*types)[6] = TYPE_ARRAY_UCHAR;
    (*types)[7] = TYPE_ARRAY_SHORT;
    (*types)[8] = TYPE_ARRAY_COMPLEX;
    (*types)[9] = TYPE_ARRAY_DCOMPLEX;
    return types;
}

vector<String>* TBConstants::nonArrayTypes() {
    vector<String>* types = new vector<String>(12);
    (*types)[0] = TYPE_STRING;
    (*types)[1] = TYPE_FLOAT;
    (*types)[2] = TYPE_DOUBLE;
    (*types)[3] = TYPE_INT;
    (*types)[4] = TYPE_UINT;
    (*types)[5] = TYPE_BOOL;
    (*types)[6] = TYPE_UCHAR;
    (*types)[7] = TYPE_SHORT;
    (*types)[8] = TYPE_COMPLEX;
    (*types)[9] = TYPE_DCOMPLEX;
    (*types)[10] = TYPE_TABLE;
    (*types)[11] = TYPE_DATE;
    return types;
}

const String TBConstants::COMMENT_DATE = "Modified Julian Day";
const String TBConstants::COMMENT_TIMP = "Time interval midpoint";  // e.g. in POINTING
const String TBConstants::COMMENT_TIMP2 = "Midpoint of time for which this set of parameters is accurate.";  // e.g. in SOURCE

String TBConstants::typeName(DataType dt) {
    if(dt == TpBool) return TYPE_BOOL;
    else if(dt == TpUChar) return TYPE_UCHAR;
    else if(dt == TpShort) return TYPE_SHORT;
    else if(dt == TpInt) return TYPE_INT;
    else if(dt == TpUInt) return TYPE_UINT;
    else if(dt == TpFloat) return TYPE_FLOAT;
    else if(dt == TpDouble) return TYPE_DOUBLE;
    else if(dt == TpComplex) return TYPE_COMPLEX;
    else if(dt == TpDComplex) return TYPE_DCOMPLEX;
    else if(dt == TpString) return TYPE_STRING;
    else if(dt == TpTable) return TYPE_TABLE;
    else if(dt == TpArrayBool) return TYPE_ARRAY_BOOL;
    else if(dt == TpArrayUChar) return TYPE_ARRAY_UCHAR;
    else if(dt == TpArrayShort) return TYPE_ARRAY_SHORT;
    else if(dt == TpArrayInt) return TYPE_ARRAY_INT;
    else if(dt == TpArrayUInt) return TYPE_ARRAY_UINT;
    else if(dt == TpArrayFloat) return TYPE_ARRAY_FLOAT;
    else if(dt == TpArrayDouble) return TYPE_ARRAY_DOUBLE;
    else if(dt == TpArrayComplex) return TYPE_ARRAY_COMPLEX;
    else if(dt == TpArrayDComplex) return TYPE_ARRAY_DCOMPLEX;
    else if(dt == TpArrayString) return TYPE_ARRAY_STRING;
    else if(dt == TpRecord) return TYPE_RECORD;
    else return "";
}

String TBConstants::typeName(String type) {
    if(type == TYPE_STRING)
        return "String";
    else if(type == TYPE_FLOAT)
        return "Float";
    else if(type == TYPE_DOUBLE)
        return "Double";
    else if(type == TYPE_INT)
        return "Integer";
    else if(type == TYPE_UINT)
        return "Unsigned Integer";
    else if(type == TYPE_BOOL)
        return "Boolean";
    else if(type == TYPE_UCHAR)
        return "Unsigned Character";
    else if(type == TYPE_CHAR)
        return "Character";
    else if(type == TYPE_SHORT)
        return "Short Integer";
    else if(type == TYPE_COMPLEX)
        return "Complex";
    else if(type == TYPE_DCOMPLEX)
        return "Double Complex";
    else if(type == TYPE_TABLE)
        return "Table";
    else if(type == TYPE_RECORD)
        return "Record";
    else if(type == TYPE_DATE)
        return "Date";
    else if(typeIsArray(type)) {
        String t = arrayType(type);
        return typeName(t) + " Array";
    }

    return type;
}

String TBConstants::VOType(String& type) {
    if(type == TYPE_STRING || type == TYPE_TABLE ||
       type == TYPE_RECORD || type == TYPE_ARRAY_STRING) return "char";
    else if(type == TYPE_FLOAT || type == TYPE_ARRAY_FLOAT) return "float";
    else if(type == TYPE_DOUBLE || type == TYPE_DATE ||
            type == TYPE_ARRAY_DOUBLE) return "double";
    else if(type == TYPE_INT || type == TYPE_ARRAY_INT) return "int";
    else if(type == TYPE_UINT || type == TYPE_ARRAY_UINT) return "int";
    else if(type == TYPE_BOOL || type == TYPE_ARRAY_BOOL) return "boolean";
    else if(type == TYPE_UCHAR || type == TYPE_ARRAY_UCHAR)
        return "unicodeChar";
    else if(type == TYPE_SHORT || type == TYPE_ARRAY_SHORT)
        return "short";
    else if(type == TYPE_COMPLEX || type == TYPE_ARRAY_COMPLEX)
        return "floatComplex";
    else if(type == TYPE_DCOMPLEX || type == TYPE_ARRAY_DCOMPLEX)
        return "doubleComplex";
    else return "";
}

bool TBConstants::typeIsTable(String& type) {
    return type == TYPE_TABLE;
}

bool TBConstants::typeIsArray(String& type) {
    return type == TYPE_ARRAY_BOOL || type == TYPE_ARRAY_FLOAT ||
           type == TYPE_ARRAY_DOUBLE || type == TYPE_ARRAY_UCHAR ||
           type == TYPE_ARRAY_SHORT || type == TYPE_ARRAY_INT ||
           type == TYPE_ARRAY_UINT || type == TYPE_ARRAY_COMPLEX ||
           type == TYPE_ARRAY_DCOMPLEX || type == TYPE_ARRAY_STRING;
}

bool TBConstants::typeIsPlottable(String& type) {
    return type != TYPE_STRING && type != TYPE_TABLE &&
           type != TYPE_RECORD && type != TYPE_ARRAY_STRING;
}

bool TBConstants::typeIsNumberable(String& type) {
    return typeIsPlottable(type) && !typeIsArray(type);
}

bool TBConstants::typeIsIndexable(String& type) {
    return type == TYPE_INT || type == TYPE_UINT || type == TYPE_SHORT;
}

bool TBConstants::typeIsComplex(String& type) {
    return type == TYPE_COMPLEX || type == TYPE_DCOMPLEX ||
           type == TYPE_ARRAY_COMPLEX || type == TYPE_ARRAY_DCOMPLEX;
}

String TBConstants::arrayType(String& at) {
    if(at == TYPE_ARRAY_BOOL)
        return TYPE_BOOL;
    else if(at == TYPE_ARRAY_FLOAT)
        return TYPE_FLOAT;
    else if(at == TYPE_ARRAY_DOUBLE)
        return TYPE_DOUBLE;
    else if(at == TYPE_ARRAY_UCHAR)
        return TYPE_UCHAR;
    else if(at == TYPE_ARRAY_SHORT)
        return TYPE_SHORT;
    else if(at == TYPE_ARRAY_INT)
        return TYPE_INT;
    else if(at == TYPE_ARRAY_UINT)
        return TYPE_UINT;
    else if(at == TYPE_ARRAY_COMPLEX)
        return TYPE_COMPLEX;
    else if(at == TYPE_ARRAY_DCOMPLEX)
        return TYPE_DCOMPLEX;
    else if(at == TYPE_ARRAY_STRING)
        return TYPE_STRING;
    return at;
}

bool TBConstants::valueIsValid(String& value, String& type) {
    if(type == TYPE_STRING) {
        return true;
    } else if(type == TYPE_FLOAT) {
        float f;
        int v = sscanf(value.c_str(), "%f", &f);
        return findWS(value) >= value.length() && v != EOF && v == 1;
    } else if(type == TYPE_DOUBLE) {
        double d;
        int v = sscanf(value.c_str(), "%lf", &d);
        return findWS(value) >= value.length() && v != EOF && v == 1;
    } else if(type == TYPE_INT) {
        int i;
        int v = sscanf(value.c_str(), "%d", &i);
        return findWS(value) >= value.length() && v != EOF && v == 1;
    } else if(type == TYPE_UINT) {
        unsigned int i;
        int v = sscanf(value.c_str(), "%u", &i);
        return findWS(value) >= value.length() && v != EOF && v == 1;
    } else if(type == TYPE_BOOL) {
        String t = "true";
        String f = "false";
        return findWS(value) >= value.length() && value == "0" || value == "1"
               || equalsIgnoreCase(value, t) || equalsIgnoreCase(value, f);
    } else if(type == TYPE_UCHAR) {
        return findWS(value) >= value.length() && value.length() == 1;
    } else if(type == TYPE_SHORT) {
        short int i;
        int v = sscanf(value.c_str(), "%hd", &i);
        return findWS(value) >= value.length() && v != EOF && v == 1;
    } else if(type == TYPE_COMPLEX) {
        float r, c;
        int v = sscanf(value.c_str(), "(%f,%f)", &r, &c);
        return findWS(value) >= value.length() && v != EOF && v == 2;
    } else if(type == TYPE_DCOMPLEX) {
        double r, c;
        int v = sscanf(value.c_str(), "(%lf,%lf)", &r, &c);
        return findWS(value) >= value.length() && v != EOF && v == 2;
    } else if(type == TYPE_TABLE) {
        return true;
    } else if(type == TYPE_RECORD) {

    } else if(type == TYPE_DATE) {
        int y, m, d, h, n, s;
        int v= sscanf(value.c_str(),"%d-%d-%d-%d:%d:%d",&y,&m, &d, &h, &n, &s);
        return findWS(value) >= value.length() && v != EOF && v == 6;
    }

    return true;
}

String TBConstants::formatValue(String& value, String& type) {
    strtrim(value);

    if(type == TYPE_BOOL) {
        String t = "true";
        String f = "false";
        if(equalsIgnoreCase(value, t) || value == "1")
            return "1";
        else if(equalsIgnoreCase(value, f) || value == "0")
            return "0";
    } else if(type == TYPE_COMPLEX) {
        float r, c;
        int v = sscanf(value.c_str(), "%*s%f%*s%f%*s", &r, &c);
        if(v == 2) {
            stringstream ss;
            ss << '(' << r << ',' << c << ')';
            return ss.str();
        }
    } else if(type == TYPE_DCOMPLEX) {
        double r, c;
        int v = sscanf(value.c_str(), "%*s%lf%*s%lf%*s", &r, &c);
        if(v == 2) {
            stringstream ss;
            ss << '(' << r << ',' << c << ')';
            return ss.str();
        }
    } else if(type == TYPE_DATE) {
        int y, m, d, h, n, s;
        int v = sscanf(value.c_str(), "%*s%d%*s%d%*s%d%*s%d%*s%d%*s%d", &y, &m,
                &d, &h, &n, &s);
        if(v == 6) {
            stringstream ss;
            ss << y << '-' << m << '-' << d << '-' << h << ':' << n << ':';
            ss << s;
            return ss.str();
        }
    }

    return value;
}

double TBConstants::valueToDouble(String& value, String& type) {
    if(type == TYPE_BOOL || type == TYPE_ARRAY_BOOL) {
        String t = "true";
        if(equalsIgnoreCase(value, t) || value == "1") return 1;
        else return 0;
    } else if(type == TYPE_DATE) {
        return date(value);
    } else {
        double d;
        if(sscanf(value.c_str(), "%lf", &d) == 1) return d;
        else return -1;
    }
}

double TBConstants::date(String str) {
    int y, m, d, h, n;
    double s;
    if(sscanf(str.c_str(),"%d-%d-%d-%d:%d:%lf", &y, &m, &d, &h, &n, &s) == 6) {
        Time t(y, m, d, h, n, s);
        // Time::modifiedJulianDay returns in days, but we want to return
        // in seconds.
        return t.modifiedJulianDay()*86400;
    } else return -1;
}

String TBConstants::date(const double d, const int numDecimals) {
    stringstream ss;
    // d is in modified julian day seconds, but Time::Time() takes julian days.
    Time t((d / 86400) + 2400000.5);

    ss << t.year() << '-';
    if(t.month() < 10) ss << '0';
    ss << t.month() << '-';
    if(t.dayOfMonth() < 10) ss << '0';
    ss << t.dayOfMonth() << '-';
    
    if(t.hours() < 10) ss << '0';    // Time stores things internally as days.
    ss << t.hours() << ':';          // Do we really want to use it for sub-day units
    if(t.minutes() < 10) ss << '0';  // when we start with d in s?
    ss << t.minutes() << ':';
    
    double sec;
    sec = fmod(d, 60.0);   // Don't do modf() + Time::seconds()!  It can be off by
    // sec += t.seconds(); // a second if d is close to an integer.
    
    if(sec < 10) ss << '0';
    if(numDecimals >= 0)
        ss << fixed << setprecision(numDecimals) << sec;
    else
        ss << TBConstants::dtoa(sec);
    
    return ss.str();
}

const int TBConstants::DEFAULT_DECIMALS = -1;
const int TBConstants::DEFAULT_DATE_DECIMALS = 2;

bool TBConstants::dateFormatIsValid(String& d) {
    unsigned int i;
    if((i = d.find("%y")) < d.length()) {
        if((i = d.find("%y", i + 1)) < d.length()) return false;
    } else return false;

    if((i = d.find("%m")) < d.length()) {
        if((i = d.find("%m", i + 1)) < d.length()) return false;
    } else return false;

    if((i = d.find("%d")) < d.length()) {
        if((i = d.find("%d", i + 1)) < d.length()) return false;
    } else return false;

    if((i = d.find("%h")) < d.length()) {
        if((i = d.find("%h", i + 1)) < d.length()) return false;
    } else return false;

    if((i = d.find("%n")) < d.length()) {
        if((i = d.find("%n", i + 1)) < d.length()) return false;
    } else return false;

    if((i = d.find("%s")) < d.length()) {
        if((i = d.find("%s", i + 1)) < d.length()) return false;
    } else return false;

    return true;
}

pair<double, double> TBConstants::toComplex(String str) {
    unsigned int i = str.find('(');
    unsigned int j = str.find(',', i + 1);
    unsigned int k = str.find(')', j + 1);

    if(i >= str.length() || j >= str.length() || k >= str.length()) 
        return pair<double, double>(0, 0);
    
    String first = str.substr(i + 1, j - i - 1);
    TBConstants::strtrim(first);
    String second = str.substr(j + 1, k - j - 1);
    TBConstants::strtrim(second);

    double a = 0, b = 0;
    sscanf(first.c_str(), "%lf", &a);
    sscanf(second.c_str(), "%lf", &b);
    return pair<double, double>(a, b);
}

/*
String typeFormat(String& type) {
    if(type == TYPE_STRING)
        return "any characters";
    else if(type == TYPE_FLOAT)
        return "a number with optional decimal values";
    else if(type == TYPE_DOUBLE)
        return "a number with optional decimal values";
    else if(type == TYPE_INT)
        return "an integer number";
    else if(type == TYPE_UINT)
        return "an unsigned integer number";
    else if(type == TYPE_BOOL)
        return "'true', 'false', '1', or '0'";
    else if(type == TYPE_UCHAR)
        return "any character";
    else if(type == TYPE_SHORT)
        return "a short integer number";
    else if(type == TYPE_COMPLEX)
        return "([float],[float])";
    else if(type == TYPE_DCOMPLEX)
        return "([double],[double])";
    else if(type == TYPE_DATE)
        return "[year]-[month]-[day]-[hour]:[minute]:[second]";

    return "";
}
*/


/* XML Driver Constants/Defaults */
const String TBConstants::XML_VOTABLE = "VOTABLE";
const String TBConstants::XML_RESOURCE = "RESOURCE";
const String TBConstants::XML_TABLE = "TABLE";
const String TBConstants::XML_TOTAL = "TOTAL";
const String TBConstants::XML_FIELD = "FIELD";
const String TBConstants::XML_KEYWORD = "KEYWORD";
const String TBConstants::XML_COLUMNKW = "COLUMNKW";
const String TBConstants::XML_RWINFO = "RWINFO";
const String TBConstants::XML_DATA = "DATA";
const String TBConstants::XML_TABLEDATA = "TABLEDATA";
const String TBConstants::XML_TR = "TR";
const String TBConstants::XML_TD = "TD";
const String TBConstants::XML_INSERTROW = "insertRow";
const String TBConstants::XML_REMOVEROW = "removeRow";
const String TBConstants::XML_TRUE = "true";
const String TBConstants::XML_FALSE = "false";
const String TBConstants::XML_ROW = "row";
const String TBConstants::XML_NAME = "name";

const String TBConstants::XML_ID = "ID";
const String TBConstants::XML_KEYWORD_NAME = "name";
const String TBConstants::XML_KEYWORD_TYPE = "type";
const String TBConstants::XML_KEYWORD_VAL = "val";
const String TBConstants::XML_COLKW_COL = "col";
const String TBConstants::XML_FIELD_NAME = "name";
const String TBConstants::XML_FIELD_TYPE = "datatype";
const String TBConstants::XML_FIELD_UNIT = "unit";
const String TBConstants::XML_FIELD_UCD = "ucd";
const String TBConstants::XML_FIELD_REF = "ref";
const String TBConstants::XML_FIELD_PRECISION = "precision";
const String TBConstants::XML_FIELD_WIDTH = "width";

const String TBConstants::ARRAY_AXES_LENGTHS = "Axis Lengths:";

const String TBConstants::ERROR_EMPTY = "AipsError: taEMPTY";

const XMLCh* TBConstants::xstr(char* str) {
    return XMLString::transcode(str);
}

const XMLCh* TBConstants::xstr(String str) {
    return XMLString::transcode((char*)str.c_str());
}

String TBConstants::xstr(XMLCh* str) {
    char* chst = XMLString::transcode(str);
    String s(chst);
    XMLString::release(&chst);
    return s;
}

String TBConstants::xstr(const XMLCh* str) {
    char* chst = XMLString::transcode(str);
    String s(chst);
    XMLString::release(&chst);
    return s;
}


/* String and Number Methods */

String TBConstants::itoa(int n) {
    stringstream ss;
        ss << n;
        return ss.str();
}

String TBConstants::uitoa(unsigned int n) {
    stringstream ss;
    ss << n;
    return ss.str();
}

String TBConstants::sitoa(short int n) {
    stringstream ss;
    ss << n;
    return ss.str();
}

String TBConstants::ftoa(float f, int decimals, bool sf) {
    stringstream ss;
    if(decimals >= 0) ss << setprecision(decimals);
    if(sf) ss << scientific;
    ss << f;
    return ss.str();
}

String TBConstants::dtoa(double d, int decimals, bool sf) {
    stringstream ss;
    if(decimals >= 0) ss << setprecision(decimals);
    if(sf) ss << scientific;
    ss << d;
    return ss.str();
}

int TBConstants::atoi(String& str, int* i) {
    return sscanf(str.c_str(), "%d", i);
}

int TBConstants::atoui(String& str, unsigned int* i) {
    return sscanf(str.c_str(), "%ud", i);
}

int TBConstants::atosi(String& str, short int* i) {
    return sscanf(str.c_str(), "%hd", i);
}

int TBConstants::atof(String& str, float* f) {
    return sscanf(str.c_str(), "%f", f);
}

int TBConstants::atod(String& str, double* d) {
    return sscanf(str.c_str(), "%lf", d);
}

bool TBConstants::isWS(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

void TBConstants::strtrim(String& str) {
    if(str.length() == 0) return;

    // find beginning whitespace
    unsigned int n = str.length();
    unsigned int i = 0;
    while(i < n && isWS(str[i])) i++;

    // find ending whitespace
    unsigned int j = n - 1;
    while(j >= 0 && isWS(str[j])) j--;

    if(j != n - 1) str.erase(j + 1);
    if(i != 0) str.erase(0, i);
}

void TBConstants::toLower(String& str) {
    for(unsigned int i = 0; i < str.length(); i++) {
        char c = str[i];
        if(c >= 'A' && c <= 'Z') {
            c = static_cast<char>(tolower(c));
            str[i] = c;
        }
    }
}

unsigned int TBConstants::findWS(String& str, int index) {
    unsigned int min = str.length();
    unsigned int i = str.find(' ', index);
    if(i < str.length()) min = i;

    i = str.find('\t', index);
    if(i < str.length() && i < min) min = i;

    i = str.find('\n', index);
    if(i < str.length() && i < min) min = i;

    i = str.find('\r', index);
    if(i < str.length() && i < min) min = i;

    return min;
}

unsigned int TBConstants::findIgnoreCase(String& str, String& find, int index){
    char c1 = find[0];
    char c2;
    if(isupper(c1)) c2 = tolower(c1);
    else c2 = toupper(c1);
    bool done = false;

    
    unsigned int i, a, b;
    i = index;
    while(!done) {
        a = str.find(c1, i);
        b = str.find(c2, i);

        if(a >= str.length() && b >= str.length()) return str.length();

        if(b >= str.length() || a < b) i = a;
        else i = b;

        for(unsigned int j = i + 1; j < find.length() + i; j++) {
            char ca = find[j - i];
            char cb;
            if(isupper(ca)) cb = tolower(ca);
            else cb = toupper(ca);
            
            if(j >= str.length() || (str[j] != ca && str[j] != cb))
                break;

            if(j == find.length() + i - 1)
                return i;
        }

        i++;
    }

    return str.length();
}

bool TBConstants::equalsIgnoreCase(String str1, String str2) {
    if(str1 == str2) return true;
    else return (str1.length() == str2.length()) &&
                (findIgnoreCase(str1, str2) == 0);
}

String TBConstants::nameFromPath(String& path) {
    if(path[path.length() - 1] == '/')
        path = path.substr(0, path.length() - 1);
        
    unsigned int i = path.find_last_of('/', path.length() - 1);
    if(i >= path.length()) return path;
    else return path.substr(i + 1);
}

String TBConstants::dirFromPath(String& path) {
    if(path[path.length() - 1] == '/')
        path = path.substr(0, path.length() - 1);

    unsigned int i = path.find_last_of('/', path.length() - 1);
    if(i >= path.length()) return path;
    else return path.substr(0, i + 1);
}

int TBConstants::round(double d) {
    return int(d + 0.5);
}

/* TBConnection Constants/Defaults */

const String TBConstants::OPEN_PAGE = String("The table browser loads a ") +
    String("given number of rows at a time to provide buffering for the ") +
    String("underlying table.  At the beginning, the rows from ") +
    String("<b>START</b> to <b>START + NUMBER</b> are loaded and page ") +
    String("navigation is provided.");

const String TBConstants::OPEN_TEXT_LOCAL = String("Select this option ") +
    String("if the table to be opened is accessible from the local ") +
    String("filesystem.  (Default)\n\nNote: At this time, remote connection") +
    String("is not available.");

const String TBConstants::OPEN_TEXT_REMOTE = String("Select this option if") +
    String("the table to be opened is accessible from a remote system.  ") +
    String("Host (such as an I.P. address) and port must be specified.\n\n") +
    String("Note: At this time, remote connection is not available.");

const String TBConstants::OPEN_TEXT_HOST = String("Enter the host on which ") +
    String("the remote table is located.  This can be an I.P. address or a") +
    String("server name.");

const String TBConstants::OPEN_TEXT_PORT = String("Enter the port on which") +
    String("the remote table server is located on the remote system.");

const String TBConstants::OPEN_TEXT_LOCATION = String("Enter the location ") +
    String("of the table in the file system, or click 'Browse' to open a ") +
    String("filebrowser.");

const String TBConstants::OPEN_TEXT_DIRECT = String("Selecting this option") +
    String(" will use a table driver that reads directly from the table on ") +
    String("disk using the CASA table interface.  For local tables, this is") +
 String(" the fastest and most efficient driver choice.  (Default for local)");

const String TBConstants::OPEN_TEXT_XML = String("Selecting this option ") +
    String("will use a table driver that browses the table using an XML ") +
    String("interface.  For local tables, this will be noticably slower ") +
    String("than the direct driver; XML is only recommended for remote ") +
    String("tables.  (Default for remote)");

const String TBConstants::OPEN_TEXT_HOME = String("Selecting this option ") +
    String("will use the \"home\" XML parser that is included with the ") +
    String("table browser to parse the table XML.");

const String TBConstants::OPEN_TEXT_DOM = String("Selecting this option ") +
    String("will use the Xerces DOM parser to parse the table XML.");

const String TBConstants::OPEN_TEXT_SAX = String("Selecting this option ") +
   String("will use the Xerces SAX parser to parse the table XML.  (Default)");

const String TBConstants::OPEN_TEXT_START = String("Enter the starting row") +
    String("for buffered loading.  ") + OPEN_PAGE + String("  (Default: 0)");

const String TBConstants::OPEN_TEXT_NUM = String("Enter the number of rows") +
    String("for buffered loading.  ") + OPEN_PAGE + String("  (Default: ") +
    itoa(TBConstants::DEFAULT_SELECT_NUM) + String(")");


/* Help Constants/Defaults */

const String TBConstants::HELP_DIR = "/code/casaqt/implement/QtBrowser/help/";
const String TBConstants::HELP_INDEX = "index.html";
const String TBConstants::HELP_XML = "help.xml";

const String TBConstants::HELP_XML_HELP = "help";
const String TBConstants::HELP_XML_CATEGORY = "category";
const String TBConstants::HELP_XML_NAME = "name";
const String TBConstants::HELP_XML_GROUP = "group";
const String TBConstants::HELP_XML_ITEM = "item";


/* Plotting Constants/Defaults */


const Plotter::Implementation TBConstants::defaultPlotterImplementation =
      Plotter::QWT;

PlotLinePtr TBConstants::defaultPlotLine(PlotFactoryPtr factory) {
    if(factory.null()) return PlotLinePtr();
    else return factory->line("000000", PlotLine::NOLINE, 1);
}

PlotSymbolPtr TBConstants::defaultPlotSymbol(PlotFactoryPtr factory) {
    if(factory.null()) return PlotSymbolPtr();
    
    PlotSymbolPtr symbol = factory->symbol(PlotSymbol::DIAMOND);
    symbol->setSize(5, 5);
    symbol->setLine("000000", PlotLine::NOLINE, 1);
    symbol->setAreaFill("BBBBBB", PlotAreaFill::FILL);
    return symbol;
}


/* View Constants/Defaults */

const String TBConstants::VIEW_SAVE_LOC = "view.tb";

const String TBConstants::VIEW_DOCUMENT = "casabrowser";
const String TBConstants::VIEW_VIEW = "view";
const String TBConstants::VIEW_LASTDIR = "lastdir";
const String TBConstants::VIEW_HISTLIMIT = "histlimit";
const String TBConstants::VIEW_TABLE = "table";
const String TBConstants::VIEW_LOCATION = "location";
const String TBConstants::VIEW_SELECTED = "selected";
const String TBConstants::VIEW_TAQL = "taql";
const String TBConstants::VIEW_HIDDEN = "hidden";
const String TBConstants::VIEW_HIDDEN_LENGTH = "length";
const String TBConstants::VIEW_VISIND = "visibleIndex";
const String TBConstants::VIEW_ROWS = "rows";
const String TBConstants::VIEW_ROWS_FROM = "loadedFrom";
const String TBConstants::VIEW_ROWS_NUM = "loadedNum";
const String TBConstants::VIEW_FILTER = "filter";
const String TBConstants::VIEW_FILTER_RULE = "rule";
const String TBConstants::VIEW_FILTER_RULE_FIELD = "field";
const String TBConstants::VIEW_FILTER_RULE_COMPARATOR = "comparator";
const String TBConstants::VIEW_FILTER_RULE_VALUE = "value";
const String TBConstants::VIEW_FILTER_RULE_VALUE2 = "value2";
const String TBConstants::VIEW_FILTER_RULE_NOT = "not";
const String TBConstants::VIEW_FILTER_RULE_ANY = "any";
const String TBConstants::VIEW_FILTER_RULE_TYPE = "type";
const String TBConstants::VIEW_FORMATS = "formats";
const String TBConstants::VIEW_FORMAT = "format";
const String TBConstants::VIEW_FORMAT_COL = "col";
const String TBConstants::VIEW_FORMAT_DECIMALS = "decimals";
const String TBConstants::VIEW_FORMAT_SFORMAT = "sformat";
const String TBConstants::VIEW_FORMAT_BFORMAT = "bformat";
const String TBConstants::VIEW_FORMAT_DFORMAT = "dformat";
const String TBConstants::VIEW_FORMAT_VTHRESHOLD = "vthreshold";
const String TBConstants::VIEW_FORMAT_ALLFONT = "allfont";
const String TBConstants::VIEW_FORMAT_FONT = "font";
const String TBConstants::VIEW_FORMAT_COLOR = "color";
const String TBConstants::VIEW_FORMAT_FAMILY = "family";
const String TBConstants::VIEW_FORMAT_SIZE = "size";
const String TBConstants::VIEW_FORMAT_BOLD = "bold";
const String TBConstants::VIEW_FORMAT_ITALICS = "italics";
const String TBConstants::VIEW_FORMAT_ULINE = "uline";
const String TBConstants::VIEW_FORMAT_STRIKE = "strike";
const String TBConstants::VIEW_SORT = "sort";
const String TBConstants::VIEW_NAME = "name";
const String TBConstants::VIEW_SORT_ASCENDING = "ascending";

}
