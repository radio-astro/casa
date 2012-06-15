//# TBData.cc: Data types used for loaded data.
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
#include <casaqt/QtBrowser/TBData.h>
#include <casaqt/QtBrowser/TBArray.h>
#include <casa/Arrays/ArrayIO.h>

namespace casa {

////////////////////////
// TBDATA DEFINITIONS //
////////////////////////

// Constructors/Destructors //

TBData::TBData() { }

TBData::~TBData() { }

// Public Methods //

String TBData::displayValue() { return asString(); }

// Public Static Methods //

TBData* TBData::create(String value, String type) {
    TBData* data = NULL;
    
    if(type == TBConstants::TYPE_STRING) {
        data = new TBDataString(value);
    } else if(type == TBConstants::TYPE_DOUBLE) {
        data = new TBDataDouble(value);
    } else if(type == TBConstants::TYPE_FLOAT) {
        data = new TBDataFloat(value);
    } else if(type == TBConstants::TYPE_INT) {
        data = new TBDataInt(value);
    } else if(type == TBConstants::TYPE_UINT) {
        data = new TBDataUInt(value);
    } else if(type == TBConstants::TYPE_BOOL) {
        data = new TBDataBool(value);
    } else if(type == TBConstants::TYPE_CHAR) {
        data = new TBDataChar(value);
    } else if(type == TBConstants::TYPE_UCHAR) {
        data = new TBDataUChar(value);
    } else if(type == TBConstants::TYPE_SHORT) {
        data = new TBDataShort(value);
    } else if(type == TBConstants::TYPE_COMPLEX) {
        data = new TBDataComplex(value);
    } else if(type == TBConstants::TYPE_DCOMPLEX) {
        data = new TBDataDComplex(value);
    } else if(type == TBConstants::TYPE_TABLE) {
        data = new TBDataTable(value);
    } else if(type == TBConstants::TYPE_DATE) {
        data = new TBDataDate(value);
    } else if(type == TBConstants::TYPE_RECORD) {
        // this isn't perfect.
        // format: name: type "value"\n
        Record r;
        unsigned int i, j, k;// = value.find('\n');
        String line;// = value.substr(0, i);
        //value = value.substr(i + 1);
        //TBConstants::strtrim(line);
        String n, t, v;
        do {
            i = value.find('\n');
            line = value.substr(0, i);
            TBConstants::strtrim(line);
            value = value.substr(i + 1);
            if(line.empty()) continue;
            
            j = line.find(':');
            n = line.substr(0, j);
            TBConstants::strtrim(n);
            if(j >= line.length() || n.empty()) continue;
            
            k = value.find(' ', j + 1);
            t = line.substr(j + 1, k - j);
            TBConstants::strtrim(t);
            if(k >= value.length() || t.empty()) continue;
            
            v = line.substr(k + 1);
            TBConstants::strtrim(v);
            if(v.empty()) continue;
            
            // TODO
        } while(i < value.length());
    } else if(TBConstants::typeIsArray(type)) {
        TBArray* arr = new TBArray(type, value);
        if(arr->isValid()) {
            data = create(arr, type);
        } else {
            if(TBConstants::typeName(type) == TBConstants::TYPE_STRING)
                data = new TBArrayDataString();
            else if(TBConstants::typeName(type) == TBConstants::TYPE_DOUBLE)
                data = new TBArrayDataDouble();
            else if(TBConstants::typeName(type) == TBConstants::TYPE_FLOAT)
                data = new TBArrayDataFloat();
            else if(TBConstants::typeName(type) == TBConstants::TYPE_INT)
                data = new TBArrayDataInt();
            else if(TBConstants::typeName(type) == TBConstants::TYPE_UINT)
                data = new TBArrayDataUInt();
            else if(TBConstants::typeName(type) == TBConstants::TYPE_BOOL)
                data = new TBArrayDataBool();
            else if(TBConstants::typeName(type) == TBConstants::TYPE_CHAR)
                data = new TBArrayDataChar();
            else if(TBConstants::typeName(type) == TBConstants::TYPE_UCHAR)
                data = new TBArrayDataUChar();
            else if(TBConstants::typeName(type) == TBConstants::TYPE_SHORT)
                data = new TBArrayDataShort();
            else if(TBConstants::typeName(type) == TBConstants::TYPE_COMPLEX)
                data = new TBArrayDataComplex();
            else if(TBConstants::typeName(type) == TBConstants::TYPE_DCOMPLEX)
                data = new TBArrayDataDComplex();
        }
        delete arr;
    }
    
    return data;
}

TBData* TBData::create(TBArray* array, String type) {
    if(array == NULL || !array->isValid()) return NULL;
    
    vector<int> shape2 = array->getDimensions();
    IPosition shape1 = TBConstants::ipos(shape2);
    IPosition d1(shape1.size(), 0);
    vector<int> d2 = vector<int>(shape2.size(), 0);
        
    if(type == TBConstants::TYPE_STRING ||
                                type == TBConstants::TYPE_ARRAY_STRING) {
        TBArrayDataString* data = new TBArrayDataString();
        Array<String>& arr = data->data();
        arr.resize(shape1);
        do {
            arr(d1) = array->dataAt(d2);
        } while(TBConstants::increment(shape1, d1) &&
                TBConstants::increment(shape2, d2));
        return data;
    } else if(type == TBConstants::TYPE_DOUBLE ||
                                type == TBConstants::TYPE_ARRAY_DOUBLE) {
        TBArrayDataDouble* data = new TBArrayDataDouble();
        Array<Double>& arr = data->data();
        arr.resize(shape1);
        do {
            double d; String s = array->dataAt(d2); TBConstants::atod(s, &d);
            arr(d1) = d;
        } while(TBConstants::increment(shape1, d1) &&
                TBConstants::increment(shape2, d2));
        return data;
    } else if(type == TBConstants::TYPE_FLOAT ||
                                type == TBConstants::TYPE_ARRAY_FLOAT) {
        TBArrayDataFloat* data = new TBArrayDataFloat();
        Array<Float>& arr = data->data();
        arr.resize(shape1);
        do {
            float d; String s = array->dataAt(d2); TBConstants::atof(s, &d);
            arr(d1) = d;
        } while(TBConstants::increment(shape1, d1) &&
                TBConstants::increment(shape2, d2));
        return data;
    } else if(type == TBConstants::TYPE_INT ||
                                type == TBConstants::TYPE_ARRAY_INT) {
        TBArrayDataInt* data = new TBArrayDataInt();
        Array<Int>& arr = data->data();
        arr.resize(shape1);
        do {
            int d; String s = array->dataAt(d2); TBConstants::atoi(s, &d);
            arr(d1) = d;
        } while(TBConstants::increment(shape1, d1) &&
                TBConstants::increment(shape2, d2));
        return data;
    } else if(type == TBConstants::TYPE_UINT ||
                                type == TBConstants::TYPE_ARRAY_UINT) {
        TBArrayDataUInt* data = new TBArrayDataUInt();
        Array<uInt>& arr = data->data();
        arr.resize(shape1);
        do {
            unsigned int d; String s = array->dataAt(d2);
            TBConstants::atoui(s, &d);
            arr(d1) = d;
        } while(TBConstants::increment(shape1, d1) &&
                TBConstants::increment(shape2, d2));
        return data;
    } else if(type == TBConstants::TYPE_BOOL ||
                                type == TBConstants::TYPE_ARRAY_BOOL) {
        TBArrayDataBool* data = new TBArrayDataBool();
        Array<Bool>& arr = data->data();
        arr.resize(shape1);
        do {
            bool d = array->dataAt(d2) != "0";
            arr(d1) = d;
        } while(TBConstants::increment(shape1, d1) &&
                TBConstants::increment(shape2, d2));
        return data;
    } else if(type == TBConstants::TYPE_CHAR ||
                                type == TBConstants::TYPE_ARRAY_CHAR) {
        TBArrayDataChar* data = new TBArrayDataChar();
        Array<Char>& arr = data->data();
        arr.resize(shape1);
        do {
            char d; String s = array->dataAt(d2); d = s.size() > 0 ? s[0] : 0;
            arr(d1) = d;
        } while(TBConstants::increment(shape1, d1) &&
                TBConstants::increment(shape2, d2));
        return data;
    } else if(type == TBConstants::TYPE_UCHAR ||
                                type == TBConstants::TYPE_ARRAY_UCHAR) {
        TBArrayDataUChar* data = new TBArrayDataUChar();
        Array<uChar>& arr = data->data();
        arr.resize(shape1);
        do {
            char d; String s = array->dataAt(d2); d = s.size() > 0 ? s[0] : 0;
            arr(d1) = d;
        } while(TBConstants::increment(shape1, d1) &&
                TBConstants::increment(shape2, d2));
        return data;
    } else if(type == TBConstants::TYPE_SHORT ||
                                type == TBConstants::TYPE_ARRAY_SHORT) {
        TBArrayDataShort* data = new TBArrayDataShort();
        Array<Short>& arr = data->data();
        arr.resize(shape1);
        do {
            short int d; String s=array->dataAt(d2); TBConstants::atosi(s, &d);
            arr(d1) = d;
        } while(TBConstants::increment(shape1, d1) &&
                TBConstants::increment(shape2, d2));
        return data;
    } else if(type == TBConstants::TYPE_SHORT ||
                                type == TBConstants::TYPE_ARRAY_SHORT) {
        TBArrayDataComplex* data = new TBArrayDataComplex();
        Array<Complex>& arr = data->data();
        arr.resize(shape1);
        do {
            pair<double, double> d = TBConstants::toComplex(array->dataAt(d2));
            arr(d1) = Complex(d.first, d.second);
        } while(TBConstants::increment(shape1, d1) &&
                TBConstants::increment(shape2, d2));
        return data;
    } else if(type == TBConstants::TYPE_SHORT ||
                                type == TBConstants::TYPE_ARRAY_SHORT) {
        TBArrayDataDComplex* data = new TBArrayDataDComplex();
        Array<DComplex>& arr = data->data();
        arr.resize(shape1);
        do {
            pair<double, double> d = TBConstants::toComplex(array->dataAt(d2));
            arr(d1) = DComplex(d.first, d.second);
        } while(TBConstants::increment(shape1, d1) &&
                TBConstants::increment(shape2, d2));
        return data;
    }
    
    return NULL;
}

TBData* TBData::create(TBData& data) {
    TBData* d = NULL;
    
    String type = data.getType();
    if(type == TBConstants::TYPE_STRING) {
        d = new TBDataString(data);
    } else if(type == TBConstants::TYPE_DOUBLE) {
        d = new TBDataDouble(data);
    } else if(type == TBConstants::TYPE_FLOAT) {
        d = new TBDataFloat(data);
    } else if(type == TBConstants::TYPE_INT) {
        d = new TBDataInt(data);
    } else if(type == TBConstants::TYPE_UINT) {
        d = new TBDataUInt(data);
    } else if(type == TBConstants::TYPE_BOOL) {
        d = new TBDataBool(data);
    } else if(type == TBConstants::TYPE_CHAR) {
        d = new TBDataChar(data);
    } else if(type == TBConstants::TYPE_UCHAR) {
        d = new TBDataUChar(data);
    } else if(type == TBConstants::TYPE_SHORT) {
        d = new TBDataShort(data);
    } else if(type == TBConstants::TYPE_COMPLEX) {
        d = new TBDataComplex(data);
    } else if(type == TBConstants::TYPE_DCOMPLEX) {
        d = new TBDataDComplex(data);
    } else if(type == TBConstants::TYPE_TABLE) {
        d = new TBDataTable(data);
    } else if(type == TBConstants::TYPE_RECORD) {
        d = new TBDataRecord(data);
    } else if(type == TBConstants::TYPE_ARRAY_STRING) {
        d = new TBArrayDataString(data);
    } else if(type == TBConstants::TYPE_ARRAY_DOUBLE) {
        d = new TBArrayDataDouble(data);
    } else if(type == TBConstants::TYPE_ARRAY_FLOAT) {
        d = new TBArrayDataFloat(data);
    } else if(type == TBConstants::TYPE_ARRAY_INT) {
        d = new TBArrayDataInt(data);
    } else if(type == TBConstants::TYPE_ARRAY_UINT) {
        d = new TBArrayDataUInt(data);
    } else if(type == TBConstants::TYPE_ARRAY_BOOL) {
        d = new TBArrayDataBool(data);
    } else if(type == TBConstants::TYPE_ARRAY_CHAR) {
        d = new TBArrayDataChar(data);
    } else if(type == TBConstants::TYPE_ARRAY_UCHAR) {
        d = new TBArrayDataUChar(data);
    } else if(type == TBConstants::TYPE_ARRAY_SHORT) {
        d = new TBArrayDataShort(data);
    } else if(type == TBConstants::TYPE_ARRAY_COMPLEX) {
        d = new TBArrayDataComplex(data);
    } else if(type == TBConstants::TYPE_ARRAY_DCOMPLEX) {
        d = new TBArrayDataDComplex(data);
    }
    
    return d;
}


//////////////////////////////
// TBDATASTRING DEFINITIONS //
//////////////////////////////

// Constructors/Destructors //

TBDataString::TBDataString(String v): TBData(), value(v) { }

TBDataString::TBDataString(TBData& data) : TBData(), value() {
    setValue(data);
}

TBDataString::~TBDataString() { }

// Accessors/Mutators //

String TBDataString::asString() { return value; }

void TBDataString::setValue(TBData& v) { value = v.asString(); }

// Public Methods //

bool TBDataString::equals(TBData* data) {
    if(data == NULL) return false;
    else if(data->getType() != TBConstants::TYPE_STRING) return false;
    else return value == data->asString();
}


//////////////////////////////
// TBDATADOUBLE DEFINITIONS //
//////////////////////////////

// Constructors/Destructors //

TBDataDouble::TBDataDouble(String v): TBData() {
    TBConstants::atod(v, &value);
}

TBDataDouble::TBDataDouble(double v): TBData(), value(v) { }

TBDataDouble::TBDataDouble(TBData& data) : TBData(), value(0) {
    setValue(data);
}

TBDataDouble::~TBDataDouble() { }

// Accessors/Mutators //

String TBDataDouble::asString() { return TBConstants::dtoa(value); }

double TBDataDouble::asDouble() { return value; }

void TBDataDouble::setValue(TBData& v) {
    String type = v.getType();
    if(type == TBConstants::TYPE_STRING) {
        type = v.asString();
        TBConstants::atod(type, &value);
    } else {
        value = v.asDouble();
    }
}

// Public Methods //

bool TBDataDouble::equals(TBData* data) {
    if(data == NULL) return false;
    else if(data->getType() != TBConstants::TYPE_DOUBLE) return false;
    else return value == data->asDouble();
}


/////////////////////////////
// TBDATAFLOAT DEFINITIONS //
/////////////////////////////

// Constructors/Destructors //

TBDataFloat::TBDataFloat(String v): TBData() {
    TBConstants::atof(v, &value);
}

TBDataFloat::TBDataFloat(float v): TBData(), value(v) { }

TBDataFloat::TBDataFloat(TBData& data) : TBData(), value(0) {
    setValue(data);
}

TBDataFloat::~TBDataFloat() { }

// Accessors/Mutators //

String TBDataFloat::asString() { return TBConstants::ftoa(value); }

double TBDataFloat::asDouble() { return value; }

float TBDataFloat::asFloat() { return value; }

void TBDataFloat::setValue(TBData& v) {
    String type = v.getType();
    if(type == TBConstants::TYPE_STRING) {
        type = v.asString();
        TBConstants::atof(type, &value);
    } else if(type == TBConstants::TYPE_FLOAT) {
        value = v.asFloat();
    }
}

// Public Methods //

bool TBDataFloat::equals(TBData* data) {
    if(data == NULL) return false;
    else if(data->getType() != TBConstants::TYPE_FLOAT) return false;
    else return value == data->asFloat();
}


///////////////////////////
// TBDATAINT DEFINITIONS //
///////////////////////////

// Constructors/Destructors //

TBDataInt::TBDataInt(String v): TBData() {
    TBConstants::atoi(v, &value);
}

TBDataInt::TBDataInt(int v): TBData(), value(v) { }

TBDataInt::TBDataInt(TBData& data) : TBData(), value(0) {
    setValue(data);
}

TBDataInt::~TBDataInt() { }

// Accessors/Mutators //

String TBDataInt::asString() { return TBConstants::itoa(value); }

double TBDataInt::asDouble() { return value; }

int TBDataInt::asInt() { return value; }

void TBDataInt::setValue(TBData& v) {
    String type = v.getType();
    if(type == TBConstants::TYPE_STRING) {
        type = v.asString();
        TBConstants::atoi(type, &value);
    } else if(type == TBConstants::TYPE_INT) {
        value = v.asInt();
    }
}

// Public Methods //

bool TBDataInt::equals(TBData* data) {
    if(data == NULL) return false;
    else if(data->getType() != TBConstants::TYPE_INT) return false;
    else return value == data->asInt();
}


////////////////////////////
// TBDATAUINT DEFINITIONS //
////////////////////////////

// Constructors/Destructors //

TBDataUInt::TBDataUInt(String v): TBData() {
    TBConstants::atoui(v, &value);
}

TBDataUInt::TBDataUInt(unsigned int v): TBData(), value(v) { }

TBDataUInt::TBDataUInt(TBData& data) : TBData(), value(0) {
    setValue(data);
}

TBDataUInt::~TBDataUInt() { }

// Accessors/Mutators //

String TBDataUInt::asString() { return TBConstants::uitoa(value); }

double TBDataUInt::asDouble() { return value; }

unsigned int TBDataUInt::asUInt() { return value; }

void TBDataUInt::setValue(TBData& v) {
    String type = v.getType();
    if(type == TBConstants::TYPE_STRING) {
        type = v.asString();
        TBConstants::atoui(type, &value);
    } else if(type == TBConstants::TYPE_UINT) {
        value = v.asUInt();
    }
}

// Public Methods //

bool TBDataUInt::equals(TBData* data) {
    if(data == NULL) return false;
    else if(data->getType() != TBConstants::TYPE_UINT) return false;
    else return value == data->asUInt();
}


////////////////////////////
// TBDATABOOL DEFINITIONS //
////////////////////////////

// Constructors/Destructors //

TBDataBool::TBDataBool(String v): TBData() {
    value = !(v == "0" || TBConstants::equalsIgnoreCase(v, "false") ||
               TBConstants::equalsIgnoreCase(v, "F"));
}

TBDataBool::TBDataBool(bool v): TBData(), value(v) { }

TBDataBool::TBDataBool(TBData& data) : TBData(), value(false) {
    setValue(data);
}

TBDataBool::~TBDataBool() { }

// Accessors/Mutators //

String TBDataBool::asString() { return value?"1":"0"; }

double TBDataBool::asDouble() { return value; }

float TBDataBool::asFloat() { return value; }

int TBDataBool::asInt() { return value; }

unsigned int TBDataBool::asUInt() { return value; }

bool TBDataBool::asBool() { return value; }

void TBDataBool::setValue(TBData& v) {
    String type = v.getType();
    if(type == TBConstants::TYPE_STRING) {
        type = v.asString();
        value = !(type == "0" ||
           TBConstants::equalsIgnoreCase(type, "false") ||
           TBConstants::equalsIgnoreCase(type, "F"));
    } else if(type == TBConstants::TYPE_BOOL) {
        value = v.asBool();
    }
}

// Public Methods //

bool TBDataBool::equals(TBData* data) {
    if(data == NULL) return false;
    else if(data->getType() != TBConstants::TYPE_BOOL) return false;
    else return value == data->asBool();
}


////////////////////////////
// TBDATACHAR DEFINITIONS //
////////////////////////////

// Constructors/Destructors //

TBDataChar::TBDataChar(String v): TBData() {
    if(!v.empty()) value = v[0];
    else value = 0;
}

TBDataChar::TBDataChar(char v): TBData(), value(v) { }

TBDataChar::TBDataChar(TBData& data) : TBData(), value(0) {
    setValue(data);
}

TBDataChar::~TBDataChar() { }

// Accessors/Mutators //

String TBDataChar::asString() { return String(value); }

int TBDataChar::asInt() { return value; }

unsigned int TBDataChar::asUInt() { return value; }

char TBDataChar::asChar() { return value; }

void TBDataChar::setValue(TBData& v) {
    String type = v.getType();
    if(type == TBConstants::TYPE_STRING) {
        type = v.asString();
        if(!type.empty()) value = type[0];
        else value = 0;
    } else if(type == TBConstants::TYPE_CHAR) {
        value = v.asChar();
    }
}

// Public Methods //

bool TBDataChar::equals(TBData* data) {
    if(data == NULL) return false;
    else if(data->getType() != TBConstants::TYPE_CHAR) return false;
    else return value == data->asChar();
}


/////////////////////////////
// TBDATAUCHAR DEFINITIONS //
/////////////////////////////

// Constructors/Destructors //

TBDataUChar::TBDataUChar(String v): TBData() {
    if(!v.empty()) value = v[0];
    else value = 0;
}

TBDataUChar::TBDataUChar(unsigned char v): TBData(), value(v) { }

TBDataUChar::TBDataUChar(TBData& data) : TBData(), value(0) {
    setValue(data);
}

TBDataUChar::~TBDataUChar() { }

// Accessors/Mutators //

String TBDataUChar::asString() { return String(value); }

int TBDataUChar::asInt() { return value; }

unsigned int TBDataUChar::asUInt() { return value; }

unsigned char TBDataUChar::asUChar() { return value; }

void TBDataUChar::setValue(TBData& v) {
    String type = v.getType();
    if(type == TBConstants::TYPE_STRING) {
        type = v.asString();
        if(!type.empty()) value = type[0];
        else value = 0;
    } else if(type == TBConstants::TYPE_UCHAR) {
        value = v.asUChar();
    }
}

// Public Methods //

bool TBDataUChar::equals(TBData* data) {
    if(data == NULL) return false;
    else if(data->getType() != TBConstants::TYPE_UCHAR) return false;
    else return value == data->asUChar();
}


/////////////////////////////
// TBDATASHORT DEFINITIONS //
/////////////////////////////

// Constructors/Destructors //

TBDataShort::TBDataShort(String v): TBData() {
    TBConstants::atosi(v, &value);
}

TBDataShort::TBDataShort(short int v): TBData(), value(v) { }

TBDataShort::TBDataShort(TBData& data) : TBData(), value(0) {
    setValue(data);
}

TBDataShort::~TBDataShort() { }

// Accessors/Mutators //

String TBDataShort::asString() { return TBConstants::sitoa(value); }

double TBDataShort::asDouble() { return value; }

float TBDataShort::asFloat() { return value; }

int TBDataShort::asInt() { return value; }

unsigned int TBDataShort::asUInt() { return value; }

short int TBDataShort::asShort() { return value; }

void TBDataShort::setValue(TBData& v) {
    String type = v.getType();
    if(type == TBConstants::TYPE_STRING) {
        type = v.asString();
        TBConstants::atosi(type, &value);
    } else if(type == TBConstants::TYPE_SHORT) {
        value = v.asShort();
    }
}

// Public Methods //

bool TBDataShort::equals(TBData* data) {
    if(data == NULL) return false;
    else if(data->getType() != TBConstants::TYPE_SHORT) return false;
    else return value == data->asShort();
}


///////////////////////////////
// TBDATACOMPLEX DEFINITIONS //
///////////////////////////////

// Constructors/Destructors //

TBDataComplex::TBDataComplex(String v): TBData() {
    pair<double, double> d = TBConstants::toComplex(v);
    value.first = (float)d.first;
    value.second = (float)d.second;
}

TBDataComplex::TBDataComplex(pair<float, float> v): TBData(), value(v) { }

TBDataComplex::TBDataComplex(complex<float> v): TBData(), value() {
    value.first = v.real();
    value.second = v.imag();
}

TBDataComplex::TBDataComplex(TBData& data) : TBData(), value(0, 0) {
    setValue(data);
}

TBDataComplex::~TBDataComplex() { }

// Accessors/Mutators //

String TBDataComplex::asString() {
    return '(' + TBConstants::ftoa(value.first) + ',' +
           TBConstants::ftoa(value.second) + ')';
}

pair<double, double> TBDataComplex::asDComplex() {
    pair<double, double> d;
    d.first = value.first;
    d.second = value.second;
    return d;
}

pair<float, float> TBDataComplex::asComplex() { return value; }

void TBDataComplex::setValue(TBData& v) {
    String type = v.getType();
    if(type == TBConstants::TYPE_STRING) {
        pair<double, double> d = TBConstants::toComplex(v.asString());
        value.first = (double)d.first;
        value.second = (double)d.second;
    } else if(type == TBConstants::TYPE_COMPLEX) {
        pair<float, float> d = v.asComplex();
        value.first = d.first;
        value.second = d.second;
    }
}

// Public Methods //

bool TBDataComplex::equals(TBData* data) {
    if(data == NULL) return false;
    else if(data->getType() != TBConstants::TYPE_COMPLEX) return false;
    else return value == data->asComplex();
}


////////////////////////////////
// TBDATADCOMPLEX DEFINITIONS //
////////////////////////////////

// Constructors/Destructors //

TBDataDComplex::TBDataDComplex(String v): TBData(),
                                          value(TBConstants::toComplex(v)) { }

TBDataDComplex::TBDataDComplex(pair<double, double> v): TBData(), value(v) { }

TBDataDComplex::TBDataDComplex(complex<double> v): TBData(), value() {
    value.first = v.real();
    value.second = v.imag();
}

TBDataDComplex::TBDataDComplex(TBData& data) : TBData(), value(0, 0) {
    setValue(data);
}

TBDataDComplex::~TBDataDComplex() { }

// Accessors/Mutators //

String TBDataDComplex::asString() {
    return '(' + TBConstants::dtoa(value.first) + ',' +
           TBConstants::dtoa(value.second) + ')';
}

pair<double, double> TBDataDComplex::asDComplex() { return value; }

void TBDataDComplex::setValue(TBData& v) {
    String type = v.getType();
    if(type == TBConstants::TYPE_STRING) {
        pair<double, double> d = TBConstants::toComplex(v.asString());
        value.first = d.first;
        value.second = d.second;
    } else if(type == TBConstants::TYPE_DCOMPLEX) {
        pair<double, double> d = v.asComplex();
        value.first = d.first;
        value.second = d.second;
    }
}

// Public Methods //

bool TBDataDComplex::equals(TBData* data) {
    if(data == NULL) return false;
    else if(data->getType() != TBConstants::TYPE_DCOMPLEX) return false;
    else return value == data->asDComplex();
}


//////////////////////////////
// TBDATARECORD DEFINITIONS //
//////////////////////////////

// Constructors/Destructors //

TBDataRecord::TBDataRecord(const RecordInterface& v): TBData(), value(v) { }

TBDataRecord::TBDataRecord(RecordInterface* v): TBData(), value(*v) { }

TBDataRecord::TBDataRecord(TBData& data) : TBData(), value() {
    setValue(data);
}

TBDataRecord::~TBDataRecord() { }

// Accessors/Mutators //

String TBDataRecord::asString() {
    stringstream ss;
    ss << value;
    return ss.str();
}

Record* TBDataRecord::asRecord() { return &value; }

void TBDataRecord::setValue(TBData& v) {
    String type = v.getType();
    if(type == TBConstants::TYPE_RECORD) {
        value = *(v.asRecord()->clone());
    }
}

// Public Methods //

bool TBDataRecord::equals(TBData* data) {
    if(data == NULL) return false;
    else if(data->getType() != TBConstants::TYPE_RECORD) return false;
    else {
        // compensate for lack of == in Record class
        stringstream ss;
        ss << value;
        String str = ss.str();
        ss.str("");
        ss << *(data->asRecord());
        String str2 = ss.str();
        return str == str2;
    }
}


////////////////////////////
// TBDATADATE DEFINITIONS //
////////////////////////////

// Constructors/Destructors //

TBDataDate::TBDataDate(String v, int d): TBData(), value(TBConstants::date(v)),
                                  valueStr(TBConstants::date(value, d)) { }

TBDataDate::TBDataDate(double v, int d): TBData(), value(v),
                                  valueStr(TBConstants::date(v, d)) { }

TBDataDate::TBDataDate(TBData& data) : TBData(), value(0), valueStr() {
    setValue(data);
}

TBDataDate::~TBDataDate() { }

// Accessors/Mutators //

String TBDataDate::asString() { return valueStr; }

double TBDataDate::asDouble() { return value; }

void TBDataDate::setValue(TBData& v) {
    String type = v.getType();
    if(type == TBConstants::TYPE_STRING) {
        valueStr = v.asString();
        value = TBConstants::date(valueStr);
    } else if(type == TBConstants::TYPE_DOUBLE ||
              type == TBConstants::TYPE_FLOAT) {
        value = v.asDouble();
        valueStr = TBConstants::date(value);
    }
}

// Public Methods //

bool TBDataDate::equals(TBData* data) {
    if(data == NULL) return false;
    else if(data->getType() != TBConstants::TYPE_DATE) return false;
    else return value == data->asDouble();
}


/////////////////////////////
// TBARRAYDATA DEFINITIONS //
/////////////////////////////

// Constructors/Destructors //

TBArrayData::TBArrayData() : TBData(), loaded(false), oneDim(false) { }

TBArrayData::~TBArrayData() { }

// Accessors/Mutators //

vector<int> TBArrayData::getShape() { return shape; }

bool TBArrayData::isLoaded() { return loaded; }

bool TBArrayData::isOneDimensional() { return oneDim; }

// Public Methods //

bool TBArrayData::coordIsValid(vector<int> d) { 
    if(shape.size() == 0) return false;
    
    if(d.size() != shape.size()) {
        if(d.size() == 2 && shape.size() == 1) {
            if(d.at(0) == 0) {
                int x = d.at(1);
                return x >= 0 && x < shape.at(0);
            } else return false;
        } else return false;
    } else {    
        for(unsigned int i = 0; i < d.size(); i++) {
            int x = d.at(i);
            if(x < 0 || x >= shape.at(i)) return false;
        }
    
        return true;
    }
}

bool TBArrayData::isEmpty() {
    if(shape.empty() || !loaded) return true;
    for(unsigned int i = 0; i < shape.size(); i++)
        if(shape[i] != 0) return false;
    return true;
}

TBData* TBArrayData::firstItem() {
    if(loaded && !isEmpty()) return dataAt(vector<int>(shape.size(), 0));
    else return NULL;
}


///////////////////////////////////
// TBARRAYDATASTRING DEFINITIONS //
///////////////////////////////////

// Constructors/Destructors //

TBArrayDataString::TBArrayDataString() : TBArrayData() { }

TBArrayDataString::TBArrayDataString(const Array<String>& v, bool full):
                                                                TBArrayData() {
    shape.resize(v.shape().size());
    for(unsigned int i = 0; i < v.shape().size(); i++)
        shape[i] = v.shape()[i];
    
    if(shape.size() == 1 || full) {
        value = v;
        loaded = true;
    }
}

TBArrayDataString::TBArrayDataString(TBData& data): TBArrayData() {
    if(data.getType() == TBConstants::TYPE_ARRAY_STRING) {
        TBArrayDataString* ad = (TBArrayDataString*)&data;
        
        shape.resize(ad->shape.size());
        for(unsigned int i = 0; i < ad->shape.size(); i++)
            shape[i] = ad->shape[i];
        
        if(ad->loaded) {
            value = ad->value;
            loaded = true;
        }
    }
}

TBArrayDataString::~TBArrayDataString() { }

// Accessors/Mutators //

TBData* TBArrayDataString::dataAt(vector<int> d) {
    if(!loaded || !coordIsValid(d)) return NULL;
    
    if(shape.size() == 1 && d.size() == 2)
        d.erase(d.begin());
    
    IPosition ip(d.size());
    for(unsigned int i = 0; i < d.size(); i++)
        ip(i) = d.at(i);
    
    String v = value(ip);
    return new TBDataString(v);
}

String TBArrayDataString::asString() {
    stringstream ss;
    if(shape.size() == 1 && loaded) {
        ss << value;
    } else {
        ss << '[';
        for(unsigned int i = 0; i < shape.size(); i++) {
            ss << shape.at(i);
            if(i < shape.size() - 1) ss << ", ";
        }
        ss << "] " << TBConstants::typeName(TBConstants::TYPE_STRING);
    }
    return ss.str();
}

// Public Methods //

void TBArrayDataString::load(const Array<String>& v) {
    shape.resize(v.shape().size());
    for(unsigned int i = 0; i < v.shape().size(); i++)
        shape[i] = v.shape()[i];
    
    value = v;
    loaded = true;
}

bool TBArrayDataString::release() {
    value.resize();
    loaded = false;
    return true;
}

void TBArrayDataString::setDataAt(vector<int> d, TBData& val) {
    if(coordIsValid(d)) {
        if(shape.size() == 1 && d.size() == 2)
            d.erase(d.begin());
        
        
        IPosition ip(d.size());
        for(unsigned int i = 0; i < d.size(); i++)
            ip(i) = d.at(i);
            
        String type = val.getType();
        if(type == TBConstants::TYPE_STRING) {
            value(ip) = val.asString();
        }
    }
}

bool TBArrayDataString::contains(TBData* data) {
    if(data == NULL || !loaded || data->getType() != TBConstants::TYPE_STRING)
        return false;
    
    String val = data->asString();
    
    for(Array<String>::iterator i = value.begin(); i != value.end(); i++)
        if(*i == val) return true;
    
    return false;
}

String TBArrayDataString::to1DString() {
    if(!loaded) return "";
    
    stringstream ss;
    for(Array<String>::iterator i = value.begin(); i != value.end(); i++)
        ss << '"' << *i << "\" ";
    
    return ss.str();
}


///////////////////////////////////
// TBARRAYDATADOUBLE DEFINITIONS //
///////////////////////////////////

// Constructors/Destructors //

TBArrayDataDouble::TBArrayDataDouble() : TBArrayData() { }

TBArrayDataDouble::TBArrayDataDouble(const Array<Double>& v, bool full):
                                                            TBArrayData() {
    shape.resize(v.shape().size());
    for(unsigned int i = 0; i < v.shape().size(); i++)
        shape[i] = v.shape()[i];
    
    if(shape.size() == 1 || full) {
        value = v;
        loaded = true;
    }
    oneDim = shape.size() == 1;
}

TBArrayDataDouble::TBArrayDataDouble(TBData& data): TBArrayData() {
    if(data.getType() == TBConstants::TYPE_ARRAY_DOUBLE) {
        TBArrayDataDouble* ad = (TBArrayDataDouble*)&data;
        
        shape.resize(ad->shape.size());
        for(unsigned int i = 0; i < ad->shape.size(); i++)
            shape[i] = ad->shape[i];
        
        if(ad->loaded) {
            value = ad->value;
            loaded = true;
        }
    }
}

TBArrayDataDouble::~TBArrayDataDouble() { }

// Accessors/Mutators //

TBData* TBArrayDataDouble::dataAt(vector<int> d) {
    if(!loaded || !coordIsValid(d)) return NULL;
    
    if(shape.size() == 1 && d.size() == 2)
        d.erase(d.begin());
    
    IPosition ip(d.size());
    for(unsigned int i = 0; i < d.size(); i++)
        ip(i) = d.at(i);
    
    double v = value(ip);
    return new TBDataDouble(v);
}

String TBArrayDataDouble::asString() {
    stringstream ss;
    if(shape.size() == 1 && loaded) {
        ss << value;
    } else {
        ss << '[';
        for(unsigned int i = 0; i < shape.size(); i++) {
            ss << shape.at(i);
            if(i < shape.size() - 1) ss << ", ";
        }
        ss << "] " << TBConstants::typeName(TBConstants::TYPE_DOUBLE);
    }
    return ss.str();
}

// Public Methods //

void TBArrayDataDouble::load(const Array<Double>& v) {
    shape.resize(v.shape().size());
    for(unsigned int i = 0; i < v.shape().size(); i++)
        shape[i] = v.shape()[i];
    
    value = v;
    loaded = true;
}

bool TBArrayDataDouble::release() {
    value.resize();
    loaded = false;
    return true;
}

void TBArrayDataDouble::setDataAt(vector<int> d, TBData& val) {
    if(coordIsValid(d)) {
        if(shape.size() == 1 && d.size() == 2)
            d.erase(d.begin());
        
        
        IPosition ip(d.size());
        for(unsigned int i = 0; i < d.size(); i++)
            ip(i) = d.at(i);
            
        String type = val.getType();
        if(type == TBConstants::TYPE_STRING) {
            double v;
            type = val.asString();
            TBConstants::atod(type, &v);
            value(ip) = v;
        } else if(type == TBConstants::TYPE_DOUBLE) {
            value(ip) = val.asDouble();
        }
    }
}

bool TBArrayDataDouble::contains(TBData* data) {
    if(data == NULL || !loaded || data->getType() != TBConstants::TYPE_DOUBLE)
        return false;
    
    double val = data->asDouble();
    
    for(Array<Double>::iterator i = value.begin(); i != value.end(); i++)
        if(*i == val) return true;
    
    return false;
}

bool TBArrayDataDouble::containsBetween(TBData* data, TBData* data2) {
    if(data == NULL || data2 == NULL || !loaded ||
       data->getType() != TBConstants::TYPE_DOUBLE ||
       data2->getType() != TBConstants::TYPE_DOUBLE) return false;
    
    double val = data->asDouble();
    double val2 = data2->asDouble();
    
    for(Array<Double>::iterator i = value.begin(); i != value.end(); i++) {
        double v = *i;
        if(val >= v && v <= val2) return true;
    }
    
    return false;
}

bool TBArrayDataDouble::containsLessThan(TBData* data) {
    if(data == NULL || !loaded || data->getType() != TBConstants::TYPE_DOUBLE)
        return false;
    
    double val = data->asDouble();
    
    for(Array<Double>::iterator i = value.begin(); i != value.end(); i++)
        if(*i < val) return true;
    
    return false;
}

bool TBArrayDataDouble::containsGreaterThan(TBData* data) {
    if(data == NULL || !loaded || data->getType() != TBConstants::TYPE_DOUBLE)
        return false;
    
    double val = data->asDouble();
    
    for(Array<Double>::iterator i = value.begin(); i != value.end(); i++)
        if(*i > val) return true;
    
    return false;
}

String TBArrayDataDouble::to1DString() {
    if(!loaded) return "";
    
    stringstream ss;
    for(Array<Double>::iterator i = value.begin(); i != value.end(); i++)
        ss << *i << ' ';
    
    return ss.str();
}


//////////////////////////////////
// TBARRAYDATAFLOAT DEFINITIONS //
//////////////////////////////////

// Constructors/Destructors //

TBArrayDataFloat::TBArrayDataFloat() : TBArrayData() { }

TBArrayDataFloat::TBArrayDataFloat(const Array<Float>& v, bool full):
                                                            TBArrayData() {
    shape.resize(v.shape().size());
    for(unsigned int i = 0; i < v.shape().size(); i++)
        shape[i] = v.shape()[i];
    
    if(shape.size() == 1 || full) {
        value = v;
        loaded = true;
    }
    oneDim = shape.size() == 1;
}

TBArrayDataFloat::TBArrayDataFloat(TBData& data): TBArrayData() {
    if(data.getType() == TBConstants::TYPE_ARRAY_FLOAT) {
        TBArrayDataFloat* ad = (TBArrayDataFloat*)&data;
        
        shape.resize(ad->shape.size());
        for(unsigned int i = 0; i < ad->shape.size(); i++)
            shape[i] = ad->shape[i];
        
        if(ad->loaded) {
            value = ad->value;
            loaded = true;
        }
    }
}

TBArrayDataFloat::~TBArrayDataFloat() { }

// Accessors/Mutators //

TBData* TBArrayDataFloat::dataAt(vector<int> d) {
    if(!loaded || !coordIsValid(d)) return NULL;
    
    if(shape.size() == 1 && d.size() == 2)
        d.erase(d.begin());
    
    IPosition ip(d.size());
    for(unsigned int i = 0; i < d.size(); i++)
        ip(i) = d.at(i);
    
    float v = value(ip);
    return new TBDataFloat(v);
}

String TBArrayDataFloat::asString() {
    stringstream ss;
    if(shape.size() == 1 && loaded) {
        ss << value;
    } else {
        ss << '[';
        for(unsigned int i = 0; i < shape.size(); i++) {
            ss << shape.at(i);
            if(i < shape.size() - 1) ss << ", ";
        }
        ss << "] " << TBConstants::typeName(TBConstants::TYPE_FLOAT);
    }
    return ss.str();
}

// Public Methods //

void TBArrayDataFloat::load(const Array<Float>& v) {
    shape.resize(v.shape().size());
    for(unsigned int i = 0; i < v.shape().size(); i++)
        shape[i] = v.shape()[i];
    
    value = v;
    loaded = true;
}

bool TBArrayDataFloat::release() {
    value.resize();
    loaded = false;
    return true;
}

void TBArrayDataFloat::setDataAt(vector<int> d, TBData& val) {
    if(coordIsValid(d)) {
        if(shape.size() == 1 && d.size() == 2)
            d.erase(d.begin());
        
        
        IPosition ip(d.size());
        for(unsigned int i = 0; i < d.size(); i++)
            ip(i) = d.at(i);
            
        String type = val.getType();
        if(type == TBConstants::TYPE_STRING) {
            float v;
            type = val.asString();
            TBConstants::atof(type, &v);
            value(ip) = v;
        } else if(type == TBConstants::TYPE_FLOAT) {
            value(ip) = val.asFloat();
        }
    }
}

bool TBArrayDataFloat::contains(TBData* data) {
    if(data == NULL || !loaded || data->getType() != TBConstants::TYPE_FLOAT)
        return false;
    
    float val = data->asFloat();
    
    for(Array<Float>::iterator i = value.begin(); i != value.end(); i++)
        if(*i == val) return true;
    
    return false;
}

bool TBArrayDataFloat::containsBetween(TBData* data, TBData* data2) {
    if(data == NULL || data2 == NULL || !loaded ||
       data->getType() != TBConstants::TYPE_FLOAT ||
       data2->getType() != TBConstants::TYPE_FLOAT) return false;
    
    float val = data->asFloat();
    float val2 = data2->asFloat();
    
    for(Array<Float>::iterator i = value.begin(); i != value.end(); i++) {
        float v = *i;
        if(val >= v && v <= val2) return true;
    }
    
    return false;
}

bool TBArrayDataFloat::containsLessThan(TBData* data) {
    if(data == NULL || !loaded || data->getType() != TBConstants::TYPE_FLOAT)
        return false;
    
    float val = data->asFloat();
    
    for(Array<Float>::iterator i = value.begin(); i != value.end(); i++)
        if(*i < val) return true;
    
    return false;
}

bool TBArrayDataFloat::containsGreaterThan(TBData* data) {
    if(data == NULL || !loaded || data->getType() != TBConstants::TYPE_FLOAT)
        return false;
    
    float val = data->asFloat();
    
    for(Array<Float>::iterator i = value.begin(); i != value.end(); i++)
        if(*i > val) return true;
    
    return false;
}

String TBArrayDataFloat::to1DString() {
    if(!loaded) return "";
    
    stringstream ss;
    for(Array<Float>::iterator i = value.begin(); i != value.end(); i++)
        ss << *i << ' ';
    
    return ss.str();
}


////////////////////////////////
// TBARRAYDATAINT DEFINITIONS //
////////////////////////////////

// Constructors/Destructors //

TBArrayDataInt::TBArrayDataInt() : TBArrayData() { }

TBArrayDataInt::TBArrayDataInt(const Array<Int>& v, bool full):
                                                    TBArrayData() {
    shape.resize(v.shape().size());
    for(unsigned int i = 0; i < v.shape().size(); i++)
        shape[i] = v.shape()[i];
    
    if(shape.size() == 1 || full) {
        value = v;
        loaded = true;
    }
    oneDim = shape.size() == 1;
}

TBArrayDataInt::TBArrayDataInt(TBData& data): TBArrayData() {
    if(data.getType() == TBConstants::TYPE_ARRAY_INT) {
        TBArrayDataInt* ad = (TBArrayDataInt*)&data;
        
        shape.resize(ad->shape.size());
        for(unsigned int i = 0; i < ad->shape.size(); i++)
            shape[i] = ad->shape[i];
        
        if(ad->loaded) {
            value = ad->value;
            loaded = true;
        }
    }
}

TBArrayDataInt::~TBArrayDataInt() { }

// Accessors/Mutators //

TBData* TBArrayDataInt::dataAt(vector<int> d) {
    if(!loaded || !coordIsValid(d)) return NULL;
    
    if(shape.size() == 1 && d.size() == 2)
        d.erase(d.begin());
    
    IPosition ip(d.size());
    for(unsigned int i = 0; i < d.size(); i++)
        ip(i) = d.at(i);
    
    int v = value(ip);
    return new TBDataInt(v);
}

String TBArrayDataInt::asString() {
    stringstream ss;
    if(shape.size() == 1 && loaded) {
        ss << value;
    } else {
        ss << '[';
        for(unsigned int i = 0; i < shape.size(); i++) {
            ss << shape.at(i);
            if(i < shape.size() - 1) ss << ", ";
        }
        ss << "] " << TBConstants::typeName(TBConstants::TYPE_INT);
    }
    return ss.str();
}

// Public Methods //

void TBArrayDataInt::load(const Array<Int>& v) {
    shape.resize(v.shape().size());
    for(unsigned int i = 0; i < v.shape().size(); i++)
        shape[i] = v.shape()[i];
    
    value = v;
    loaded = true;
}

bool TBArrayDataInt::release() {
    value.resize();
    loaded = false;
    return true;
}

void TBArrayDataInt::setDataAt(vector<int> d, TBData& val) {
    if(coordIsValid(d)) {
        if(shape.size() == 1 && d.size() == 2)
            d.erase(d.begin());
        
        
        IPosition ip(d.size());
        for(unsigned int i = 0; i < d.size(); i++)
            ip(i) = d.at(i);
            
        String type = val.getType();
        if(type == TBConstants::TYPE_STRING) {
            int v;
            type = val.asString();
            TBConstants::atoi(type, &v);
            value(ip) = v;
        } else if(type == TBConstants::TYPE_INT) {
            value(ip) = val.asInt();
        }
    }
}

bool TBArrayDataInt::contains(TBData* data) {
    if(data == NULL || !loaded || data->getType() != TBConstants::TYPE_INT)
        return false;
    
    int val = data->asInt();
    
    for(Array<Int>::iterator i = value.begin(); i != value.end(); i++)
        if(*i == val) return true;
    
    return false;
}

bool TBArrayDataInt::containsBetween(TBData* data, TBData* data2) {
    if(data == NULL || data2 == NULL || !loaded ||
       data->getType() != TBConstants::TYPE_INT ||
       data2->getType() != TBConstants::TYPE_INT) return false;
    
    int val = data->asUInt();
    int val2 = data2->asUInt();
    
    for(Array<Int>::iterator i = value.begin(); i != value.end(); i++) {
        int v = *i;
        if(val >= v && v <= val2) return true;
    }
    
    return false;
}

bool TBArrayDataInt::containsLessThan(TBData* data) {
    if(data == NULL || !loaded || data->getType() != TBConstants::TYPE_INT)
        return false;
    
    int val = data->asInt();
    
    for(Array<Int>::iterator i = value.begin(); i != value.end(); i++)
        if(*i < val) return true;
    
    return false;
}

bool TBArrayDataInt::containsGreaterThan(TBData* data) {
    if(data == NULL || !loaded || data->getType() != TBConstants::TYPE_INT)
        return false;
    
    int val = data->asInt();
    
    for(Array<Int>::iterator i = value.begin(); i != value.end(); i++)
        if(*i > val) return true;
    
    return false;
}

String TBArrayDataInt::to1DString() {
    if(!loaded) return "";
    
    stringstream ss;
    for(Array<Int>::iterator i = value.begin(); i != value.end(); i++)
        ss << *i << ' ';
    
    return ss.str();
}


/////////////////////////////////
// TBARRAYDATAUINT DEFINITIONS //
/////////////////////////////////

// Constructors/Destructors //

TBArrayDataUInt::TBArrayDataUInt() : TBArrayData() { }

TBArrayDataUInt::TBArrayDataUInt(const Array<uInt>& v, bool full):
                                                    TBArrayData() {
    shape.resize(v.shape().size());
    for(unsigned int i = 0; i < v.shape().size(); i++)
        shape[i] = v.shape()[i];
    
    if(shape.size() == 1 || full) {
        value = v;
        loaded = true;
    }
    oneDim = shape.size() == 1;
}

TBArrayDataUInt::TBArrayDataUInt(TBData& data): TBArrayData() {
    if(data.getType() == TBConstants::TYPE_ARRAY_UINT) {
        TBArrayDataUInt* ad = (TBArrayDataUInt*)&data;
        
        shape.resize(ad->shape.size());
        for(unsigned int i = 0; i < ad->shape.size(); i++)
            shape[i] = ad->shape[i];
        
        if(ad->loaded) {
            value = ad->value;
            loaded = true;
        }
    }
}

TBArrayDataUInt::~TBArrayDataUInt() { }

// Accessors/Mutators //

TBData* TBArrayDataUInt::dataAt(vector<int> d) {
    if(!loaded || !coordIsValid(d)) return NULL;
    
    if(shape.size() == 1 && d.size() == 2)
        d.erase(d.begin());
    
    IPosition ip(d.size());
    for(unsigned int i = 0; i < d.size(); i++)
        ip(i) = d.at(i);
    
    unsigned int v = value(ip);
    return new TBDataUInt(v);
}

String TBArrayDataUInt::asString() {
    stringstream ss;
    if(shape.size() == 1 && loaded) {
        ss << value;
    } else {
        ss << '[';
        for(unsigned int i = 0; i < shape.size(); i++) {
            ss << shape.at(i);
            if(i < shape.size() - 1) ss << ", ";
        }
        ss << "] " << TBConstants::typeName(TBConstants::TYPE_UINT);
    }
    return ss.str();
}

// Public Methods //

void TBArrayDataUInt::load(const Array<uInt>& v) {
    shape.resize(v.shape().size());
    for(unsigned int i = 0; i < v.shape().size(); i++)
        shape[i] = v.shape()[i];
    
    value = v;
    loaded = true;
}

bool TBArrayDataUInt::release() {
    value.resize();
    loaded = false;
    return true;
}

void TBArrayDataUInt::setDataAt(vector<int> d, TBData& val) {
    if(coordIsValid(d)) {
        if(shape.size() == 1 && d.size() == 2)
            d.erase(d.begin());
        
        
        IPosition ip(d.size());
        for(unsigned int i = 0; i < d.size(); i++)
            ip(i) = d.at(i);
            
        String type = val.getType();
        if(type == TBConstants::TYPE_STRING) {
            unsigned int v;
            type = val.asString();
            TBConstants::atoui(type, &v);
            value(ip) = v;
        } else if(type == TBConstants::TYPE_UINT) {
            value(ip) = val.asUInt();
        }
    }
}

bool TBArrayDataUInt::contains(TBData* data) {
    if(data == NULL || !loaded || data->getType() != TBConstants::TYPE_UINT)
        return false;
    
    unsigned int val = data->asUInt();
    
    for(Array<uInt>::iterator i = value.begin(); i != value.end(); i++)
        if(*i == val) return true;
    
    return false;
}

bool TBArrayDataUInt::containsBetween(TBData* data, TBData* data2) {
    if(data == NULL || data2 == NULL || !loaded ||
       data->getType() != TBConstants::TYPE_UINT ||
       data2->getType() != TBConstants::TYPE_UINT) return false;
    
    unsigned int val = data->asUInt();
    unsigned int val2 = data2->asUInt();
    
    for(Array<uInt>::iterator i = value.begin(); i != value.end(); i++) {
        unsigned int v = *i;
        if(val >= v && v <= val2) return true;
    }
    
    return false;
}

bool TBArrayDataUInt::containsLessThan(TBData* data) {
    if(data == NULL || !loaded || data->getType() != TBConstants::TYPE_UINT)
        return false;
    
    unsigned int val = data->asUInt();
    
    for(Array<uInt>::iterator i = value.begin(); i != value.end(); i++)
        if(*i < val) return true;
    
    return false;
}

bool TBArrayDataUInt::containsGreaterThan(TBData* data) {
    if(data == NULL || !loaded || data->getType() != TBConstants::TYPE_UINT)
        return false;
    
    unsigned int val = data->asUInt();
    
    for(Array<uInt>::iterator i = value.begin(); i != value.end(); i++)
        if(*i > val) return true;
    
    return false;
}

String TBArrayDataUInt::to1DString() {
    if(!loaded) return "";
    
    stringstream ss;
    for(Array<uInt>::iterator i = value.begin(); i != value.end(); i++)
        ss << *i << ' ';
    
    return ss.str();
}


/////////////////////////////////
// TBARRAYDATABOOL DEFINITIONS //
/////////////////////////////////

// Constructors/Destructors //

TBArrayDataBool::TBArrayDataBool() : TBArrayData() { }

TBArrayDataBool::TBArrayDataBool(const Array<Bool>& v, bool full):
                                                        TBArrayData() {
    shape.resize(v.shape().size());
    for(unsigned int i = 0; i < v.shape().size(); i++)
        shape[i] = v.shape()[i];
    
    if(shape.size() == 1 || full) {
        value = v;
        loaded = true;
    }
    oneDim = shape.size() == 1;
}

TBArrayDataBool::TBArrayDataBool(TBData& data): TBArrayData() {
    if(data.getType() == TBConstants::TYPE_ARRAY_BOOL) {
        TBArrayDataBool* ad = (TBArrayDataBool*)&data;
        
        shape.resize(ad->shape.size());
        for(unsigned int i = 0; i < ad->shape.size(); i++)
            shape[i] = ad->shape[i];
        
        if(ad->loaded) {
            value = ad->value;
            loaded = true;
        }
    }
}

TBArrayDataBool::~TBArrayDataBool() { }

// Accessors/Mutators //

TBData* TBArrayDataBool::dataAt(vector<int> d) {
    if(!loaded || !coordIsValid(d)) return NULL;
    
    if(shape.size() == 1 && d.size() == 2)
        d.erase(d.begin());
    
    IPosition ip(d.size());
    for(unsigned int i = 0; i < d.size(); i++)
        ip(i) = d.at(i);
    
    bool v = value(ip);
    return new TBDataBool(v);
}

String TBArrayDataBool::asString() {
    stringstream ss;
    if(shape.size() == 1 && loaded) {
        ss << value;
    } else {
        ss << '[';
        for(unsigned int i = 0; i < shape.size(); i++) {
            ss << shape.at(i);
            if(i < shape.size() - 1) ss << ", ";
        }
        ss << "] " << TBConstants::typeName(TBConstants::TYPE_BOOL);
    }
    return ss.str();
}

// Public Methods //

void TBArrayDataBool::load(const Array<Bool>& v) {
    shape.resize(v.shape().size());
    for(unsigned int i = 0; i < v.shape().size(); i++)
        shape[i] = v.shape()[i];
    
    value = v;
    loaded = true;
}

bool TBArrayDataBool::release() {
    value.resize();
    loaded = false;
    return true;
}

void TBArrayDataBool::setDataAt(vector<int> d, TBData& val) {
    if(coordIsValid(d)) {
        if(shape.size() == 1 && d.size() == 2)
            d.erase(d.begin());
        
        
        IPosition ip(d.size());
        for(unsigned int i = 0; i < d.size(); i++)
            ip(i) = d.at(i);
            
        String type = val.getType();
        if(type == TBConstants::TYPE_STRING) {
            type = val.asString();
            value(ip) = !(type == "0" ||
                          TBConstants::equalsIgnoreCase(type, "false") ||
                          TBConstants::equalsIgnoreCase(type, "f"));
        } else if(type == TBConstants::TYPE_BOOL) {
            value(ip) = val.asBool();
        }
    }
}

bool TBArrayDataBool::contains(TBData* data) {
    if(data == NULL || !loaded || data->getType() != TBConstants::TYPE_BOOL)
        return false;
    
    bool val = data->asBool();
    
    for(Array<Bool>::iterator i = value.begin(); i != value.end(); i++)
        if(*i == val) return true;
    
    return false;
}

bool TBArrayDataBool::containsBetween(TBData* data, TBData* data2) {
    if(data == NULL || data2 == NULL || !loaded ||
       data->getType() != TBConstants::TYPE_BOOL ||
       data2->getType() != TBConstants::TYPE_BOOL) return false;
    
    bool val = data->asBool();
    bool val2 = data2->asBool();
    
    for(Array<Bool>::iterator i = value.begin(); i != value.end(); i++) {
        bool v = *i;
        if(val >= v && v <= val2) return true;
    }
    
    return false;
}

bool TBArrayDataBool::containsLessThan(TBData* data) {
    if(data == NULL || !loaded || data->getType() != TBConstants::TYPE_BOOL)
        return false;
    
    bool val = data->asBool();
    
    for(Array<Bool>::iterator i = value.begin(); i != value.end(); i++)
        if(*i < val) return true;
    
    return false;
}

bool TBArrayDataBool::containsGreaterThan(TBData* data) {
    if(data == NULL || !loaded || data->getType() != TBConstants::TYPE_BOOL)
        return false;
    
    bool val = data->asBool();
    
    for(Array<Bool>::iterator i = value.begin(); i != value.end(); i++)
        if(*i > val) return true;
    
    return false;
}

String TBArrayDataBool::to1DString() {
    if(!loaded) return "";
    
    stringstream ss;
    for(Array<Bool>::iterator i = value.begin(); i != value.end(); i++)
        ss << *i << ' ';
    
    return ss.str();
}


/////////////////////////////////
// TBARRAYDATACHAR DEFINITIONS //
/////////////////////////////////

// Constructors/Destructors //

TBArrayDataChar::TBArrayDataChar() : TBArrayData() { }

TBArrayDataChar::TBArrayDataChar(const Array<Char>& v, bool full):
                                                            TBArrayData() {
    shape.resize(v.shape().size());
    for(unsigned int i = 0; i < v.shape().size(); i++)
        shape[i] = v.shape()[i];
    
    if(shape.size() == 1 || full) {
        value = v;
        loaded = true;
    }
    oneDim = shape.size() == 1;
}

TBArrayDataChar::TBArrayDataChar(TBData& data): TBArrayData() {
    if(data.getType() == TBConstants::TYPE_ARRAY_CHAR) {
        TBArrayDataChar* ad = (TBArrayDataChar*)&data;
        
        shape.resize(ad->shape.size());
        for(unsigned int i = 0; i < ad->shape.size(); i++)
            shape[i] = ad->shape[i];
        
        if(ad->loaded) {
            value = ad->value;
            loaded = true;
        }
    }
}

TBArrayDataChar::~TBArrayDataChar() { }

// Accessors/Mutators //

TBData* TBArrayDataChar::dataAt(vector<int> d) {
    if(!loaded || !coordIsValid(d)) return NULL;
    
    if(shape.size() == 1 && d.size() == 2)
        d.erase(d.begin());
    
    IPosition ip(d.size());
    for(unsigned int i = 0; i < d.size(); i++)
        ip(i) = d.at(i);
    
    char v = value(ip);
    return new TBDataChar(v);
}

String TBArrayDataChar::asString() {
    stringstream ss;
    if(shape.size() == 1 && loaded) {
        ss << value;
    } else {
        ss << '[';
        for(unsigned int i = 0; i < shape.size(); i++) {
            ss << shape.at(i);
            if(i < shape.size() - 1) ss << ", ";
        }
        ss << "] " << TBConstants::typeName(TBConstants::TYPE_CHAR);
    }
    return ss.str();
}

// Public Methods //

void TBArrayDataChar::load(const Array<Char>& v) {
    shape.resize(v.shape().size());
    for(unsigned int i = 0; i < v.shape().size(); i++)
        shape[i] = v.shape()[i];
    
    value = v;
    loaded = true;
}

bool TBArrayDataChar::release() {
    value.resize();
    loaded = false;
    return true;
}

void TBArrayDataChar::setDataAt(vector<int> d, TBData& val) {
    if(coordIsValid(d)) {
        if(shape.size() == 1 && d.size() == 2)
            d.erase(d.begin());
        
        
        IPosition ip(d.size());
        for(unsigned int i = 0; i < d.size(); i++)
            ip(i) = d.at(i);
            
        String type = val.getType();
        if(type == TBConstants::TYPE_STRING) {
            type = val.asString();
            if(type.size() > 0) value(ip) = type[0];
        } else if(type == TBConstants::TYPE_CHAR) {
            value(ip) = val.asChar();
        }
    }
}

bool TBArrayDataChar::contains(TBData* data) {
    if(data == NULL || !loaded || data->getType() != TBConstants::TYPE_CHAR)
        return false;
    
    char val = data->asChar();
    
    for(Array<Char>::iterator i = value.begin(); i != value.end(); i++)
        if(*i == val) return true;
    
    return false;
}

bool TBArrayDataChar::containsBetween(TBData* data, TBData* data2) {
    if(data == NULL || data2 == NULL || !loaded ||
       data->getType() != TBConstants::TYPE_CHAR ||
       data2->getType() != TBConstants::TYPE_CHAR) return false;
    
    char val = data->asChar();
    char val2 = data2->asChar();
    
    for(Array<Char>::iterator i = value.begin(); i != value.end(); i++) {
        char v = *i;
        if(val >= v && v <= val2) return true;
    }
    
    return false;
}

bool TBArrayDataChar::containsLessThan(TBData* data) {
    if(data == NULL || !loaded || data->getType() != TBConstants::TYPE_CHAR)
        return false;
    
    char val = data->asChar();
    
    for(Array<Char>::iterator i = value.begin(); i != value.end(); i++)
        if(*i < val) return true;
    
    return false;
}

bool TBArrayDataChar::containsGreaterThan(TBData* data) {
    if(data == NULL || !loaded || data->getType() != TBConstants::TYPE_CHAR)
        return false;
    
    char val = data->asChar();
    
    for(Array<Char>::iterator i = value.begin(); i != value.end(); i++)
        if(*i > val) return true;
    
    return false;
}

String TBArrayDataChar::to1DString() {
    if(!loaded) return "";
    
    stringstream ss;
    for(Array<Char>::iterator i = value.begin(); i != value.end(); i++)
        ss << *i << ' ';
    
    return ss.str();
}


//////////////////////////////////
// TBARRAYDATAUCHAR DEFINITIONS //
//////////////////////////////////

// Constructors/Destructors //

TBArrayDataUChar::TBArrayDataUChar() : TBArrayData() { }

TBArrayDataUChar::TBArrayDataUChar(const Array<uChar>& v, bool full):
                                                        TBArrayData() {
    shape.resize(v.shape().size());
    for(unsigned int i = 0; i < v.shape().size(); i++)
        shape[i] = v.shape()[i];
    
    if(shape.size() == 1 || full) {
        value = v;
        loaded = true;
    }
    oneDim = shape.size() == 1;
}

TBArrayDataUChar::TBArrayDataUChar(TBData& data): TBArrayData() {
    if(data.getType() == TBConstants::TYPE_ARRAY_UCHAR) {
        TBArrayDataUChar* ad = (TBArrayDataUChar*)&data;
        
        shape.resize(ad->shape.size());
        for(unsigned int i = 0; i < ad->shape.size(); i++)
            shape[i] = ad->shape[i];
        
        if(ad->loaded) {
            value = ad->value;
            loaded = true;
        }
    }
}

TBArrayDataUChar::~TBArrayDataUChar() { }

// Accessors/Mutators //

TBData* TBArrayDataUChar::dataAt(vector<int> d) {
    if(!loaded || !coordIsValid(d)) return NULL;
    
    if(shape.size() == 1 && d.size() == 2)
        d.erase(d.begin());
    
    IPosition ip(d.size());
    for(unsigned int i = 0; i < d.size(); i++)
        ip(i) = d.at(i);
    
    unsigned char v = value(ip);
    return new TBDataUChar(v);
}

String TBArrayDataUChar::asString() {
    stringstream ss;
    if(shape.size() == 1 && loaded) {
        ss << value;
    } else {
        ss << '[';
        for(unsigned int i = 0; i < shape.size(); i++) {
            ss << shape.at(i);
            if(i < shape.size() - 1) ss << ", ";
        }
        ss << "] " << TBConstants::typeName(TBConstants::TYPE_UCHAR);
    }
    return ss.str();
}

// Public Methods //

void TBArrayDataUChar::load(const Array<uChar>& v) {
    shape.resize(v.shape().size());
    for(unsigned int i = 0; i < v.shape().size(); i++)
        shape[i] = v.shape()[i];
    
    value = v;
    loaded = true;
}

bool TBArrayDataUChar::release() {
    value.resize();
    loaded = false;
    return true;
}

void TBArrayDataUChar::setDataAt(vector<int> d, TBData& val) {
    if(coordIsValid(d)) {
        if(shape.size() == 1 && d.size() == 2)
            d.erase(d.begin());
        
        
        IPosition ip(d.size());
        for(unsigned int i = 0; i < d.size(); i++)
            ip(i) = d.at(i);
            
        String type = val.getType();
        if(type == TBConstants::TYPE_STRING) {
            type = val.asString();
            if(type.size() > 0) value(ip) = type[0];
        } else if(type == TBConstants::TYPE_UCHAR) {
            value(ip) = val.asUChar();
        }
    }
}

bool TBArrayDataUChar::contains(TBData* data) {
    if(data == NULL || !loaded || data->getType() != TBConstants::TYPE_UCHAR)
        return false;
    
    unsigned char val = data->asUChar();
    
    for(Array<uChar>::iterator i = value.begin(); i != value.end(); i++)
        if(*i == val) return true;
    
    return false;
}

bool TBArrayDataUChar::containsBetween(TBData* data, TBData* data2) {
    if(data == NULL || data2 == NULL || !loaded ||
       data->getType() != TBConstants::TYPE_UCHAR ||
       data2->getType() != TBConstants::TYPE_UCHAR) return false;
    
    unsigned char val = data->asUChar();
    unsigned char val2 = data2->asUChar();
    
    for(Array<uChar>::iterator i = value.begin(); i != value.end(); i++) {
        unsigned char v = *i;
        if(val >= v && v <= val2) return true;
    }
    
    return false;
}

bool TBArrayDataUChar::containsLessThan(TBData* data) {
    if(data == NULL || !loaded || data->getType() != TBConstants::TYPE_UCHAR)
        return false;
    
    unsigned char val = data->asUChar();
    
    for(Array<uChar>::iterator i = value.begin(); i != value.end(); i++)
        if(*i < val) return true;
    
    return false;
}

bool TBArrayDataUChar::containsGreaterThan(TBData* data) {
    if(data == NULL || !loaded || data->getType() != TBConstants::TYPE_UCHAR)
        return false;
    
    unsigned char val = data->asUChar();
    
    for(Array<uChar>::iterator i = value.begin(); i != value.end(); i++)
        if(*i > val) return true;
    
    return false;
}

String TBArrayDataUChar::to1DString() {
    if(!loaded) return "";
    
    stringstream ss;
    for(Array<uChar>::iterator i = value.begin(); i != value.end(); i++)
        ss << *i << ' ';
    
    return ss.str();
}


//////////////////////////////////
// TBARRAYDATASHORT DEFINITIONS //
//////////////////////////////////

// Constructors/Destructors //

TBArrayDataShort::TBArrayDataShort() : TBArrayData() { }

TBArrayDataShort::TBArrayDataShort(const Array<Short>& v, bool full):
                                                            TBArrayData() {
    shape.resize(v.shape().size());
    for(unsigned int i = 0; i < v.shape().size(); i++)
        shape[i] = v.shape()[i];
    
    if(shape.size() == 1 || full) {
        value = v;
        loaded = true;
    }
    oneDim = shape.size() == 1;
}

TBArrayDataShort::TBArrayDataShort(TBData& data): TBArrayData() {
    if(data.getType() == TBConstants::TYPE_ARRAY_SHORT) {
        TBArrayDataShort* ad = (TBArrayDataShort*)&data;
        
        shape.resize(ad->shape.size());
        for(unsigned int i = 0; i < ad->shape.size(); i++)
            shape[i] = ad->shape[i];
        
        if(ad->loaded) {
            value = ad->value;
            loaded = true;
        }
    }
}

TBArrayDataShort::~TBArrayDataShort() { }

// Accessors/Mutators //

TBData* TBArrayDataShort::dataAt(vector<int> d) {
    if(!loaded || !coordIsValid(d)) return NULL;
    
    if(shape.size() == 1 && d.size() == 2)
        d.erase(d.begin());
    
    IPosition ip(d.size());
    for(unsigned int i = 0; i < d.size(); i++)
        ip(i) = d.at(i);
    
    short int v = value(ip);
    return new TBDataShort(v);
}

String TBArrayDataShort::asString() {
    stringstream ss;
    if(shape.size() == 1 && loaded) {
        ss << value;
    } else {
        ss << '[';
        for(unsigned int i = 0; i < shape.size(); i++) {
            ss << shape.at(i);
            if(i < shape.size() - 1) ss << ", ";
        }
        ss << "] " << TBConstants::typeName(TBConstants::TYPE_SHORT);
    }
    return ss.str();
}

// Public Methods //

void TBArrayDataShort::load(const Array<Short>& v) {
    shape.resize(v.shape().size());
    for(unsigned int i = 0; i < v.shape().size(); i++)
        shape[i] = v.shape()[i];
    
    value = v;
    loaded = true;
}

bool TBArrayDataShort::release() {
    value.resize();
    loaded = false;
    return true;
}

void TBArrayDataShort::setDataAt(vector<int> d, TBData& val) {
    if(coordIsValid(d)) {
        if(shape.size() == 1 && d.size() == 2)
            d.erase(d.begin());
        
        
        IPosition ip(d.size());
        for(unsigned int i = 0; i < d.size(); i++)
            ip(i) = d.at(i);
            
        String type = val.getType();
        if(type == TBConstants::TYPE_STRING) {
            short int v;
            type = val.asString();
            TBConstants::atosi(type, &v);
            value(ip) = v;
        } else if(type == TBConstants::TYPE_SHORT) {
            value(ip) = val.asShort();
        }
    }
}

bool TBArrayDataShort::contains(TBData* data) {
    if(data == NULL || !loaded || data->getType() != TBConstants::TYPE_SHORT)
        return false;
    
    short int val = data->asShort();
    
    for(Array<Short>::iterator i = value.begin(); i != value.end(); i++)
        if(*i == val) return true;
    
    return false;
}

bool TBArrayDataShort::containsBetween(TBData* data, TBData* data2) {
    if(data == NULL || data2 == NULL || !loaded ||
       data->getType() != TBConstants::TYPE_SHORT ||
       data2->getType() != TBConstants::TYPE_SHORT) return false;
    
    short int val = data->asShort();
    short int val2 = data2->asShort();
    
    for(Array<Short>::iterator i = value.begin(); i != value.end(); i++) {
        short int v = *i;
        if(val >= v && v <= val2) return true;
    }
    
    return false;
}

bool TBArrayDataShort::containsLessThan(TBData* data) {
    if(data == NULL || !loaded || data->getType() != TBConstants::TYPE_SHORT)
        return false;
    
    short val = data->asShort();
    
    for(Array<Short>::iterator i = value.begin(); i != value.end(); i++)
        if(*i < val) return true;
    
    return false;
}

bool TBArrayDataShort::containsGreaterThan(TBData* data) {
    if(data == NULL || !loaded || data->getType() != TBConstants::TYPE_SHORT)
        return false;
    
    short val = data->asShort();
    
    for(Array<Short>::iterator i = value.begin(); i != value.end(); i++)
        if(*i > val) return true;
    
    return false;
}

String TBArrayDataShort::to1DString() {
    if(!loaded) return "";
    
    stringstream ss;
    for(Array<Short>::iterator i = value.begin(); i != value.end(); i++)
        ss << *i << ' ';
    
    return ss.str();
}


////////////////////////////////////
// TBARRAYDATACOMPLEX DEFINITIONS //
////////////////////////////////////

// Constructors/Destructors //

TBArrayDataComplex::TBArrayDataComplex() : TBArrayData() { }

TBArrayDataComplex::TBArrayDataComplex(const Array<Complex>& v, bool full):
                                                            TBArrayData() {
    shape.resize(v.shape().size());
    for(unsigned int i = 0; i < v.shape().size(); i++)
        shape[i] = v.shape()[i];
    
    if(shape.size() == 1 || full) {
        value = v;
        loaded = true;
    }
    oneDim = shape.size() == 1;
}

TBArrayDataComplex::TBArrayDataComplex(TBData& data): TBArrayData() {
    if(data.getType() == TBConstants::TYPE_ARRAY_COMPLEX) {
        TBArrayDataComplex* ad = (TBArrayDataComplex*)&data;
        
        shape.resize(ad->shape.size());
        for(unsigned int i = 0; i < ad->shape.size(); i++)
            shape[i] = ad->shape[i];
        
        if(ad->loaded) {
            value = ad->value;
            loaded = true;
        }
    }
}

TBArrayDataComplex::~TBArrayDataComplex() { }

// Accessors/Mutators //

TBData* TBArrayDataComplex::dataAt(vector<int> d) {
    if(!loaded || !coordIsValid(d)) return NULL;
    
    if(shape.size() == 1 && d.size() == 2)
        d.erase(d.begin());
    
    IPosition ip(d.size());
    for(unsigned int i = 0; i < d.size(); i++)
        ip(i) = d.at(i);
    
    complex<float> v = value(ip);
    return new TBDataComplex(v);
}

String TBArrayDataComplex::asString() {
    stringstream ss;
    if(shape.size() == 1 && loaded) {
        ss << value;
    } else {
        ss << '[';
        for(unsigned int i = 0; i < shape.size(); i++) {
            ss << shape.at(i);
            if(i < shape.size() - 1) ss << ", ";
        }
        ss << "] " << TBConstants::typeName(TBConstants::TYPE_COMPLEX);
    }
    return ss.str();
}

// Public Methods //

void TBArrayDataComplex::load(const Array<Complex>& v) {
    shape.resize(v.shape().size());
    for(unsigned int i = 0; i < v.shape().size(); i++)
        shape[i] = v.shape()[i];
    
    value = v;
    loaded = true;
}

bool TBArrayDataComplex::release() {
    value.resize();
    loaded = false;
    return true;
}

void TBArrayDataComplex::setDataAt(vector<int> d, TBData& val) {
    if(coordIsValid(d)) {
        if(shape.size() == 1 && d.size() == 2)
            d.erase(d.begin());
        
        
        IPosition ip(d.size());
        for(unsigned int i = 0; i < d.size(); i++)
            ip(i) = d.at(i);
            
        String type = val.getType();
        if(type == TBConstants::TYPE_STRING) {
            pair<double, double> v = TBConstants::toComplex(val.asString());
            value(ip) = complex<float>((float)v.first, (float)v.second);
        } else if(type == TBConstants::TYPE_COMPLEX) {
            pair<float, float> v = val.asComplex();
            value(ip) = complex<float>(v.first, v.second);
        }
    }
}

bool TBArrayDataComplex::contains(TBData* data) {
    if(data == NULL || !loaded || data->getType() != TBConstants::TYPE_COMPLEX)
        return false;
    
    pair<float, float> d = data->asDComplex();
    Complex val(d.first, d.second);
    
    for(Array<Complex>::iterator i = value.begin(); i != value.end(); i++)
        if(*i == val) return true;
    
    return false;
}

bool TBArrayDataComplex::containsBetween(TBData* data, TBData* data2) {
    if(data == NULL || data2 == NULL || !loaded ||
       data->getType() != TBConstants::TYPE_COMPLEX ||
       data2->getType() != TBConstants::TYPE_COMPLEX) return false;
    
    pair<float, float> d = data->asComplex();
    Complex val(d.first, d.second);
    d = data2->asComplex();
    Complex val2(d.first, d.second);
    
    for(Array<Complex>::iterator i = value.begin(); i != value.end(); i++) {
        Complex v = *i;
        if(val >= v && v <= val2) return true;
    }
    
    return false;
}

bool TBArrayDataComplex::containsLessThan(TBData* data) {
    if(data == NULL || !loaded || data->getType() != TBConstants::TYPE_COMPLEX)
        return false;
    
    pair<float, float> d = data->asComplex();
    Complex val(d.first, d.second);
    
    for(Array<Complex>::iterator i = value.begin(); i != value.end(); i++)
        if(*i < val) return true;
    
    return false;
}

bool TBArrayDataComplex::containsGreaterThan(TBData* data) {
    if(data == NULL || !loaded || data->getType() != TBConstants::TYPE_COMPLEX)
        return false;
    
    pair<float, float> d = data->asComplex();
    Complex val(d.first, d.second);
    
    for(Array<Complex>::iterator i = value.begin(); i != value.end(); i++)
        if(*i > val) return true;
    
    return false;
}

String TBArrayDataComplex::to1DString() {
    if(!loaded) return "";
    
    stringstream ss;
    for(Array<Complex>::iterator i = value.begin(); i != value.end(); i++)
        ss << *i << ' ';
    
    return ss.str();
}


/////////////////////////////////////
// TBARRAYDATADCOMPLEX DEFINITIONS //
/////////////////////////////////////

// Constructors/Destructors //

TBArrayDataDComplex::TBArrayDataDComplex() : TBArrayData() { }

TBArrayDataDComplex::TBArrayDataDComplex(const Array<DComplex>& v, bool full):
                                                            TBArrayData() {
    shape.resize(v.shape().size());
    for(unsigned int i = 0; i < v.shape().size(); i++)
        shape[i] = v.shape()[i];
    
    if(shape.size() == 1 || full) {
        value = v;
        loaded = true;
    }
    oneDim = shape.size() == 1;
}

TBArrayDataDComplex::TBArrayDataDComplex(TBData& data): TBArrayData() {
    if(data.getType() == TBConstants::TYPE_ARRAY_DCOMPLEX) {
        TBArrayDataDComplex* ad = (TBArrayDataDComplex*)&data;
        
        shape.resize(ad->shape.size());
        for(unsigned int i = 0; i < ad->shape.size(); i++)
            shape[i] = ad->shape[i];
        
        if(ad->loaded) {
            value = ad->value;
            loaded = true;
        }
    }
}

TBArrayDataDComplex::~TBArrayDataDComplex() { }

// Accessors/Mutators //

TBData* TBArrayDataDComplex::dataAt(vector<int> d) {
    if(!loaded || !coordIsValid(d)) return NULL;
    
    if(shape.size() == 1 && d.size() == 2)
        d.erase(d.begin());
    
    IPosition ip(d.size());
    for(unsigned int i = 0; i < d.size(); i++)
        ip(i) = d.at(i);
    
    complex<double> v = value(ip);
    return new TBDataDComplex(v);
}

String TBArrayDataDComplex::asString() {
    stringstream ss;
    if(shape.size() == 1 && loaded) {
        ss << value;
    } else {
        ss << '[';
        for(unsigned int i = 0; i < shape.size(); i++) {
            ss << shape.at(i);
            if(i < shape.size() - 1) ss << ", ";
        }
        ss << "] " << TBConstants::typeName(TBConstants::TYPE_DCOMPLEX);
    }
    return ss.str();
}

// Public Methods //

void TBArrayDataDComplex::load(const Array<DComplex>& v) {
    shape.resize(v.shape().size());
    for(unsigned int i = 0; i < v.shape().size(); i++)
        shape[i] = v.shape()[i];
    
    value = v;
    loaded = true;
}

bool TBArrayDataDComplex::release() {
    value.resize();
    loaded = false;
    return true;
}

void TBArrayDataDComplex::setDataAt(vector<int> d, TBData& val) {
    if(coordIsValid(d)) {
        if(shape.size() == 1 && d.size() == 2)
            d.erase(d.begin());
        
        IPosition ip(d.size());
        for(unsigned int i = 0; i < d.size(); i++)
            ip(i) = d.at(i);
            
        String type = val.getType();
        if(type == TBConstants::TYPE_STRING) {
            pair<double, double> v = TBConstants::toComplex(val.asString());
            value(ip) = complex<double>(v.first, v.second);
        } else if(type == TBConstants::TYPE_DCOMPLEX) {
            pair<double, double> v = val.asDComplex();
            value(ip) = complex<double>(v.first, v.second);
        }
    }
}

bool TBArrayDataDComplex::contains(TBData* data) {
    if(data == NULL || !loaded || data->getType()!= TBConstants::TYPE_DCOMPLEX)
        return false;
    
    pair<double, double> d = data->asDComplex();
    DComplex val(d.first, d.second);
    
    for(Array<DComplex>::iterator i = value.begin(); i != value.end(); i++)
        if(*i == val) return true;
    
    return false;
}

bool TBArrayDataDComplex::containsBetween(TBData* data, TBData* data2) {
    if(data == NULL || data2 == NULL || !loaded ||
       data->getType() != TBConstants::TYPE_DCOMPLEX ||
       data2->getType() != TBConstants::TYPE_DCOMPLEX) return false;
    
    pair<double, double> d = data->asDComplex();
    DComplex val(d.first, d.second);
    d = data2->asDComplex();
    DComplex val2(d.first, d.second);
    
    for(Array<DComplex>::iterator i = value.begin(); i != value.end(); i++) {
        DComplex v = *i;
        if(val >= v && v <= val2) return true;
    }
    
    return false;
}

bool TBArrayDataDComplex::containsLessThan(TBData* data) {
    if(data == NULL || !loaded || data->getType()!= TBConstants::TYPE_DCOMPLEX)
        return false;
    
    pair<double, double> d = data->asDComplex();
    DComplex val(d.first, d.second);
    
    for(Array<DComplex>::iterator i = value.begin(); i != value.end(); i++)
        if(*i < val) return true;
    
    return false;
}

bool TBArrayDataDComplex::containsGreaterThan(TBData* data) {
    if(data == NULL || !loaded || data->getType()!= TBConstants::TYPE_DCOMPLEX)
        return false;
    
    pair<double, double> d = data->asDComplex();
    DComplex val(d.first, d.second);
    
    for(Array<DComplex>::iterator i = value.begin(); i != value.end(); i++)
        if(*i > val) return true;
    
    return false;
}

String TBArrayDataDComplex::to1DString() {
    if(!loaded) return "";
    
    stringstream ss;
    for(Array<DComplex>::iterator i = value.begin(); i != value.end(); i++)
        ss << *i << ' ';
    
    return ss.str();
}

}
