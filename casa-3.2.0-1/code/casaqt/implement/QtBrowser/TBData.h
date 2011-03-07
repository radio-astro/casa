//# TBData.h: Data types used for loaded data.
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
#ifndef TBDATA_H_
#define TBDATA_H_

#include <casaqt/QtBrowser/TBConstants.h>

#include <casa/BasicSL/String.h>
#include <casa/Containers/Record.h>
#include <casa/Arrays/Array.h>

#include <casa/namespace.h>
using namespace std;

namespace casa {

//# Forward Declarations
class TBArray;

// <summary>
// Data types used for loaded data.
// </summary>
//
// <synopsis>
// TBData is the abstract superclass for any data type that the browser knows
// about.  The idea is to get around templates and other such unpleasantness
// that causes unnecessary code copying.
// </synopsis>

class TBData {
public:
    // Default Constructor.
    TBData();
    
    virtual ~TBData();
    
    
    // See TBData::asString().
    String displayValue();
    
    
    // asString() must be implemented by any subclass.  Returns this data
    // value in String form.  This method is especially important because
    // the browser uses this String to display in the GUI widgets.
    virtual String asString() = 0;
    
    // asDouble() must be implemented by any subclass, although the returned
    // value does not have to be valid for classes for which the value cannot
    // be represented as a double.
    virtual double asDouble() = 0;
    
    // asFloat() must be implemented by any subclass, although the returned
    // value does not have to be valid for classes for which the value cannot
    // be represented as a float.
    virtual float asFloat() = 0;
    
    // asInt() must be implemented by any subclass, although the returned
    // value does not have to be valid for classes for which the value cannot
    // be represented as an int.
    virtual int asInt() = 0;
    
    // asUInt() must be implemented by any subclass, although the returned
    // value does not have to be valid for classes for which the value cannot
    // be represented as an unsigned int.
    virtual unsigned int asUInt() = 0;
    
    // asBool() must be implemented by any subclass, although the returned
    // value does not have to be valid for classes for which the value cannot
    // be represented as a boolean.
    virtual bool asBool() = 0;
    
    // asChar() must be implemented by any subclass, although the returned
    // value does not have to be valid for classes for which the value cannot
    // be represented as a char.
    virtual char asChar() = 0;
    
    // asUChar() must be implemented by any subclass, although the returned
    // value does not have to be valid for classes for which the value cannot
    // be represented as an unsigned character.
    virtual unsigned char asUChar() = 0;
    
    // asShort() must be implemented by any subclass, although the returned
    // value does not have to be valid for classes for which the value cannot
    // be represented as a short.
    virtual short int asShort() = 0;
    
    // asComplex() must be implemented by any subclass, although the returned
    // value does not have to be valid for classes for which the value cannot
    // be represented as a complex.
    virtual pair<float, float> asComplex() = 0;
    
    // asDComplex() must be implemented by any subclass, although the returned
    // value does not have to be valid for classes for which the value cannot
    // be represented as a double complex.
    virtual pair<double, double> asDComplex() = 0;
    
    // asRecord() must be implemented by any subclass, although the returned
    // value does not have to be valid for classes for which the value cannot
    // be represented as a record.
    virtual Record* asRecord() = 0;
    
    
    // setValue() must be implemented by any subclass.  Sets this data's value
    // to the value of the given TBData.  Note: the behavior of this method
    // is undefined if the given TBData is not the same type as "this" TBData.
    virtual void setValue(TBData& value) = 0;
    
    // getType() must be implemented by any subclass.  Returns this TBData
    // object's type.  Must be one of TBConstants::TYPE_* definitions.
    virtual String getType() = 0;
    
    // equals() must be implemented by any subclass.  Returns true if the
    // given TBData is equal to this TBData object, false otherwise.  Note:
    // the behavior of this method is undefined if the given TBData is not of
    // the same type as "this" TBData.
    virtual bool equals(TBData* data) = 0;
    
    
    // Creates and returns a TBData object representing the given value and
    // type.  May return NULL if the value/type combination is invalid.
    // NOTE: this method currently does not work for Records in String form.
    static TBData* create(String value, String type);
    
    // Creates a TBArrayData object containing the data in the given TBArray
    // object with the given type.  NOTE: with the restructuring of the
    // browser to use TBDatas rather than Strings to store data, this method
    // should NOT be used.  It is currently being used as a helper method
    // for create(String, String), but its use is NOT encouraged.
    static TBData* create(TBArray* array, String type);
    
    // Creates and returns a copy of the given TBData.
    static TBData* create(TBData& data);
};


// <summary>
// Implementation of TBData for String data.
// </summary>

class TBDataString : public TBData {
public:
    // Constructor that takes the String data.
    TBDataString(String value);
    
    // Constructor that calls setValue().
    TBDataString(TBData& data);
    
    virtual ~TBDataString();
    
    
    // Returns the String value.
    String asString();
    
    // Invalid operations.
    // <group>
    double asDouble() { return 0; } 
    float asFloat() { return 0; }   
    int asInt() { return 0; }   
    unsigned int asUInt() { return 0; } 
    bool asBool() { return 0; } 
    char asChar() { return 0; } 
    unsigned char asUChar() { return 0; }   
    short int asShort() { return 0; }   
    pair<float, float> asComplex() { return pair<float, float>(0, 0); } 
    pair<double, double> asDComplex() { return pair<double, double>(0, 0); }    
    Record* asRecord() { return 0; }
    // </group>
    
    
    // Sets the value to the result of calling asString() on the given TBData.
    void setValue(TBData& value);
    
    // Returns the String type.
    String getType() { return TBConstants::TYPE_STRING; }
    
    // Returns true if the given data is a String type and the values are
    // equals, false otherwise.
    bool equals(TBData* data);
    
private:
    // Value.
    String value;
};


// <summary>
// Implementation of TBData for double data.
// </summary>

class TBDataDouble : public TBData {
public:
    // Constructor that parses a double from the given String.
    TBDataDouble(String value);
    
    // Constructor that takes the double data.
    TBDataDouble(double value);
    
    // Constructor that calls setValue().
    TBDataDouble(TBData& data);
    
    virtual ~TBDataDouble();
    
    
    // Returns the value in String form.
    String asString();
    
    // Returns the double value.
    double asDouble();
    
    
    // Invalid operations.
    // <group>
    float asFloat() { return 0; }   
    int asInt() { return 0; }   
    unsigned int asUInt() { return 0; } 
    bool asBool() { return 0; } 
    char asChar() { return 0; } 
    unsigned char asUChar() { return 0; }   
    short int asShort() { return 0; }   
    pair<float, float> asComplex() { return pair<float, float>(0, 0); } 
    pair<double, double> asDComplex() { return pair<double, double>(0, 0); }    
    Record* asRecord() { return 0; }
    // </group>
    
    // If the given TBData is a String, parses a double from the String.
    // Otherwise sets the value to the result of the asDouble() call.
    void setValue(TBData& value);
    
    // Returns the double type.
    String getType() { return TBConstants::TYPE_DOUBLE; }
    
    // Returns true if the given data is a double type and their values are
    // equal, false otherwise.
    bool equals(TBData* data);
    
private:
    // Value.
    double value;
};


// <summary>
// Implementation of TBData for float data.
// </summary>

class TBDataFloat : public TBData {
public:
    // Constructor that parses a float from the given String.
    TBDataFloat(String value);
    
    // Constructor that takes the float data.
    TBDataFloat(float value);
    
    // Constructor that calls setValue().
    TBDataFloat(TBData& data);
    
    virtual ~TBDataFloat();
    
    
    // Returns the value in String form.
    String asString();
    
    // Returns the value in double form.
    double asDouble();
    
    // Returns the float value.
    float asFloat();
    
    
    // Invalid operations.
    // <group>
    int asInt() { return 0; }   
    unsigned int asUInt() { return 0; } 
    bool asBool() { return 0; } 
    char asChar() { return 0; } 
    unsigned char asUChar() { return 0; }   
    short int asShort() { return 0; }   
    pair<float, float> asComplex() { return pair<float, float>(0, 0); } 
    pair<double, double> asDComplex() { return pair<double, double>(0, 0); }    
    Record* asRecord() { return 0; }
    // </group>
    
    
    // If the given TBData is a String, parses a float from the String.
    // Otherwise, if the given TBData is a float, sets the float value.
    void setValue(TBData& value);
    
    // Returns the float type.
    String getType() { return TBConstants::TYPE_FLOAT; }
    
    // Returns true if the given data is a float type and their values are
    // equal, false otherwise.
    bool equals(TBData* data);
    
private:
    // Value.
    float value;
};


// <summary>
// Implementation of TBData for integer data.
// </summary>

class TBDataInt : public TBData {
public:
    // Constructor that parses an int from the given String.
    TBDataInt(String value);
    
    // Constructor that takes the int data.
    TBDataInt(int value);
    
    // Constructor that calls setValue().
    TBDataInt(TBData& data);
    
    virtual ~TBDataInt();
    
    
    // Returns the value in String form.
    String asString();
    
    // Returns the value in double form.
    double asDouble();
    
    // Returns the value.
    int asInt();
    
    
    // Invalid operations.
    // <group>
    float asFloat() { return 0; }
    unsigned int asUInt() { return 0; } 
    bool asBool() { return 0; } 
    char asChar() { return 0; } 
    unsigned char asUChar() { return 0; }   
    short int asShort() { return 0; }   
    pair<float, float> asComplex() { return pair<float, float>(0, 0); } 
    pair<double, double> asDComplex() { return pair<double, double>(0, 0); }    
    Record* asRecord() { return 0; }
    // </group>
    
    
    // If the given TBData is a String, parses an int from the String value.
    // Otherwise, if the given TBData is an int, sets the int value.
    void setValue(TBData& value);
    
    // Returns the int type.
    String getType() { return TBConstants::TYPE_INT; }
    
    // Returns true if the given data is an int type and their values are
    // equal, false otherwise
    bool equals(TBData* data);
    
private:
    // Value.
    int value;
};


// <summary>
// Implementation of TBData for unsigned int data.
// </summary>

class TBDataUInt : public TBData {
public:
    // Constructor that parses an unsigned int from the given String.
    TBDataUInt(String value);
    
    // Constructor that takes the unsigned int data.
    TBDataUInt(unsigned int value);
    
    // Constructor that calls setValue().
    TBDataUInt(TBData& data);
    
    virtual ~TBDataUInt();
    
    
    // Returns the value in String form.
    String asString();
    
    // Returns the value in double form.
    double asDouble();
    
    // Returns the value.
    unsigned int asUInt();
    
    
    // Invalid operations.
    // <group>
    float asFloat() { return 0; }
    int asInt() { return 0; }
    bool asBool() { return 0; } 
    char asChar() { return 0; } 
    unsigned char asUChar() { return 0; }   
    short int asShort() { return 0; }   
    pair<float, float> asComplex() { return pair<float, float>(0, 0); } 
    pair<double, double> asDComplex() { return pair<double, double>(0, 0); }    
    Record* asRecord() { return 0; }
    // </group>
    
    
    // If the given TBData is a String, parses an unsigned int from the String
    // value.  Otherwise, if the given TBData is an unsigned int, sets the
    // unsigned int value.
    void setValue(TBData& value);
    
    // Returns the unsigned int type.
    String getType() { return TBConstants::TYPE_UINT; }
    
    // Returns true if the given data is an unsigned int type and their values
    // are equal, false otherwise
    bool equals(TBData* data);
    
private:
    // Value.
    unsigned int value;
};


// <summary>
// Implementation of TBData for boolean data.
// </summary>

class TBDataBool : public TBData {
public:
    // Constructor that parses a boolean from the given String.
    TBDataBool(String value);
    
    // Constructor that takes the boolean data.
    TBDataBool(bool value);
    
    // Constructor that calls setValue().
    TBDataBool(TBData& data);
    
    virtual ~TBDataBool();
    
    
    // Returns the value in String form.
    String asString();
    
    // Returns the value in double form.
    double asDouble();
    
    // Returns the value in float form.
    float asFloat();
    
    // Returns the value in int form.
    int asInt();
    
    // Returns the value in unsigned int form.
    unsigned int asUInt();
    
    // Returns the value.
    bool asBool();
    
    
    // Invalid operations.
    // <group>
    char asChar() { return 0; } 
    unsigned char asUChar() { return 0; }   
    short int asShort() { return 0; }   
    pair<float, float> asComplex() { return pair<float, float>(0, 0); } 
    pair<double, double> asDComplex() { return pair<double, double>(0, 0); }    
    Record* asRecord() { return 0; }
    // </group>
    
    
    // If the given TBData is a String, parses a boolean from the String value.
    // Otherwise, if the given TBData is a boolean, sets the boolean value.
    void setValue(TBData& value);
    
    // Returns the boolean type.
    String getType() { return TBConstants::TYPE_BOOL; }
    
    // Returns true if the given data is a boolean type and their values are
    // equal, false otherwise
    bool equals(TBData* data);
    
private:
    // Value.
    bool value;
};


// <summary>
// Implementation of TBData for character data.
// </summary>

class TBDataChar : public TBData {
public:
    // Constructor that takes the first character of the given String.
    TBDataChar(String value);
    
    // Constructor that takes the character data.
    TBDataChar(char value);
    
    // Constructor that calls setValue().
    TBDataChar(TBData& data);
    
    virtual ~TBDataChar();
    
    
    // Returns the value in String form.
    String asString();
    
    // Returns the value in int form.
    int asInt();
    
    // Returns the value in unsigned int form.
    unsigned int asUInt();
    
    // Returns the value.
    char asChar();
    
    
    // Invalid operations.
    // <group>
    double asDouble() { return 0; } 
    float asFloat() { return 0; }
    bool asBool() { return 0; }
    unsigned char asUChar() { return 0; }   
    short int asShort() { return 0; }   
    pair<float, float> asComplex() { return pair<float, float>(0, 0); } 
    pair<double, double> asDComplex() { return pair<double, double>(0, 0); }    
    Record* asRecord() { return 0; }
    // </group>
    
    
    // If the given TBData is a String, takes the first character of the
    // String value.  Otherwise, if the given TBData is a character, sets
    // the character value.
    void setValue(TBData& value);
    
    // Returns the character type.
    String getType() { return TBConstants::TYPE_CHAR; }
    
    // Returns true if the given data is a character type and their values are
    // equal, false otherwise
    bool equals(TBData* data);
    
private:
    // Value.
    char value;
};


// <summary>
// Implementation of TBData for unsigned character data.
// </summary>

class TBDataUChar : public TBData {
public:
    // Constructor that takes the first character from the given String.
    TBDataUChar(String value);
    
    // Constructor that takes the unsigned character value.
    TBDataUChar(unsigned char value);
    
    // Constructor that calls setValue().
    TBDataUChar(TBData& data);
    
    virtual ~TBDataUChar();
    
    
    // Returns the value in String form.
    String asString();
    
    // Returns the value in int form.
    int asInt();
    
    // Returns the value in unsigned int form.
    unsigned int asUInt();
    
    // Returns the value.
    unsigned char asUChar();
    
    
    // Invalid operations.
    // <group>
    double asDouble() { return 0; } 
    float asFloat() { return 0; }
    bool asBool() { return 0; }
    char asChar() { return 0; } 
    short int asShort() { return 0; }   
    pair<float, float> asComplex() { return pair<float, float>(0, 0); } 
    pair<double, double> asDComplex() { return pair<double, double>(0, 0); }    
    Record* asRecord() { return 0; }
    // </group>
    
    
    // If the given TBData is a String, takes the first character of the
    // String value.  Otherwise, if the given TBData is an unsigned character,
    // sets the unsigned character value.
    void setValue(TBData& value);
    
    // Returns the unsigned character type.
    String getType() { return TBConstants::TYPE_UCHAR; }
    
    // Returns true if the given data is an unsigned character type and their
    // values are equal, false otherwise
    bool equals(TBData* data);
    
private:
    // Value.
    unsigned char value;
};


// <summary>
// Implementation of TBData for short data.
// </summary>

class TBDataShort : public TBData {
public:
    // Constructor that parses a short from the given String.
    TBDataShort(String value);
    
    // Constructor that takes the short data.
    TBDataShort(short int value);
    
    // Constructor that calls setValue().
    TBDataShort(TBData& data);
    
    virtual ~TBDataShort();
    
    
    // Returns the value in String form.
    String asString();
    
    // Returns the value in double form.
    double asDouble();
    
    // Returns the value in float form.
    float asFloat();
    
    // Returns the value in int form.
    int asInt();
    
    // Returns the value in unsigned int form.
    unsigned int asUInt();
    
    // Returns the value.
    short int asShort();
    
    
    // Invalid operations.
    // <group>
    bool asBool() { return 0; }
    char asChar() { return 0; } 
    unsigned char asUChar() { return 0; }   
    pair<float, float> asComplex() { return pair<float, float>(0, 0); } 
    pair<double, double> asDComplex() { return pair<double, double>(0, 0); }    
    Record* asRecord() { return 0; }
    // </group>
    
    
    // If the given TBData is a String, parses a short from the String value.
    // Otherwise, if the given TBData is a short, sets the short value.
    void setValue(TBData& value);
    
    // Returns the short type.
    String getType() { return TBConstants::TYPE_SHORT; }
    
    // Returns true if the given data is a short type and their values are
    // equal, false otherwise
    bool equals(TBData* data);
    
private:
    // Value.
    short int value;
};


// <summary>
// Implementation of TBData for complex data.
// </summary>

class TBDataComplex : public TBData {
public:
    // Constructor that parses a complex from the given String.
    TBDataComplex(String value);
    
    // Constructor that takes the complex data.
    TBDataComplex(pair<float, float> value);
    
    // Constructor that takes the complex data.
    TBDataComplex(complex<float> value);
    
    // Constructor that calls setValue().
    TBDataComplex(TBData& data);
    
    virtual ~TBDataComplex();
    
    
    // Returns the value in String form.
    String asString();
    
    // Returns the value in double complex form.
    pair<double, double> asDComplex();
    
    // Returns the value.
    pair<float, float> asComplex();
    
    
    // Invalid operations.
    // <group>
    double asDouble() { return 0; }
    float asFloat() { return 0; }
    int asInt() { return 0; }
    unsigned int asUInt() { return 0; }
    short int asShort() { return 0; }
    bool asBool() { return 0; }
    char asChar() { return 0; } 
    unsigned char asUChar() { return 0; }   
    Record* asRecord() { return 0; }
    // </group>
    
    
    // If the given TBData is a String, parses a complex from the String value.
    // Otherwise, if the given TBData is a complex, sets the complex value.
    void setValue(TBData& value);
    
    // Returns the complex type.
    String getType() { return TBConstants::TYPE_COMPLEX; }
    
    // Returns true if the given data is a complex type and their values are
    // equal, false otherwise
    bool equals(TBData* data);
    
private:
    // Value.
    pair<float, float> value;
};


// <summary>
// Implementation of TBData for double complex data.
// </summary>

class TBDataDComplex : public TBData {
public:
    // Constructor that parses a double complex from the given String.
    TBDataDComplex(String value);
    
    // Constructor that takes the double complex data.
    TBDataDComplex(pair<double, double> value);
    
    // Constructor that takes the double complex data.
    TBDataDComplex(complex<double> value);
    
    // Constructor that calls setValue().
    TBDataDComplex(TBData& data);
    
    virtual ~TBDataDComplex();
    
    
    // Returns the value in String form.
    String asString();
    
    // Returns the value.
    pair<double, double> asDComplex();
    
    
    // Invalid operations.
    // <group>
    double asDouble() { return 0; }
    float asFloat() { return 0; }
    int asInt() { return 0; }
    unsigned int asUInt() { return 0; }
    short int asShort() { return 0; }
    bool asBool() { return 0; }
    char asChar() { return 0; } 
    unsigned char asUChar() { return 0; }   
    pair<float, float> asComplex() { return pair<float, float>(0, 0); }
    Record* asRecord() { return 0; }
    // </group>
    
    
    // If the given TBData is a String, parses a double complex from the
    // String value.  Otherwise, if the given TBData is a double complex,
    // sets the double complex value.
    void setValue(TBData& value);
    
    // Returns the double complex type.
    String getType() { return TBConstants::TYPE_DCOMPLEX; }
    
    // Returns true if the given data is a double complex type and their
    // values are equal, false otherwise
    bool equals(TBData* data);
    
private:
    // Value.
    pair<double, double> value;
};


// <summary>
// Implementation of TBData for Table data.
// </summary>
//
// <synopsis>
// Although Table types are stored differently on disk, for the browser all
// we really care about is the location.  Therefore TBDataTable is really just
// a TBDataString.
// </synopsis>

class TBDataTable : public TBDataString {
public:
    // Constructor that takes the String value.  See
    // TBDataString::TBDataString().
    TBDataTable(String value) : TBDataString(value) { }
    
    // Constructor that calls setValue().  See TBDataString::setValue().
    TBDataTable(TBData& data) : TBDataString(data) { }
    
    ~TBDataTable() { }
    
    // Returns the table type.
    String getType() { return TBConstants::TYPE_TABLE; }
};


// <summary>
// Implementation of TBData for Record data.
// </summary>

class TBDataRecord : public TBData {
public:
    // Constructor that takes the Record data.
    TBDataRecord(const RecordInterface& value);
    
    // Constructor that takes the Record data.
    TBDataRecord(RecordInterface* value);
    
    // Constructor that calls setValue().
    TBDataRecord(TBData& data);
    
    virtual ~TBDataRecord();
    
    
    // Returns the value in String form.
    String asString();
    
    // Returns the value.
    Record* asRecord();
    
    
    // Invalid operations.
    // <group>
    double asDouble() { return 0; }
    float asFloat() { return 0; }
    int asInt() { return 0; }
    unsigned int asUInt() { return 0; }
    short int asShort() { return 0; }
    bool asBool() { return 0; }
    char asChar() { return 0; } 
    unsigned char asUChar() { return 0; }   
    pair<float, float> asComplex() { return pair<float, float>(0, 0); }
    pair<double, double> asDComplex() { return pair<double, double>(0, 0); }
    // </group>
    
    
    // Iff the given TBData is a Record, sets the Record value.
    void setValue(TBData& value);
    
    // Returns the Record type.
    String getType() { return TBConstants::TYPE_RECORD; }
    
    // Returns true if the given data is a Record type and their values are
    // equal, false otherwise
    bool equals(TBData* data);
    
private:
    // Value.
    Record value;
};


// <summary>
// Implementation of TBData for date data.
// </summary>
//
// <synopsis>
// A date is somewhat special in that the data is stored as a double that
// represents Modified Julian Seconds, but when we view the data we want it
// in a human-readable form.  For this reason, a TBDataDate stores two
// values: the double representation and the String representation.
// </synopsis>

class TBDataDate : public TBData {
public:
    // Constructor that parses a date from the given String and then stores
    // the human-readable value with the given number of decimals.
    TBDataDate(String value, int decimals= TBConstants::DEFAULT_DATE_DECIMALS);
    
    // Constructor that stores the date from the given double and then stores
    // the human-readable value with the given number of decimals.
    TBDataDate(double value, int decimals= TBConstants::DEFAULT_DATE_DECIMALS);
    
    // Constructor that calls setValue().
    TBDataDate(TBData& data);
    
    virtual ~TBDataDate();
    
    
    // Returns the human-readable value.
    String asString();
    
    // Returns the value.
    double asDouble();
    
    
    // Invalid operations.
    // <group>
    float asFloat() { return 0; }   
    int asInt() { return 0; }   
    unsigned int asUInt() { return 0; } 
    bool asBool() { return 0; } 
    char asChar() { return 0; } 
    unsigned char asUChar() { return 0; }   
    short int asShort() { return 0; }   
    pair<float, float> asComplex() { return pair<float, float>(0, 0); } 
    pair<double, double> asDComplex() { return pair<double, double>(0, 0); }    
    Record* asRecord() { return 0; }
    // </group>
    
    
    // If the given TBData is a String, parses a date from the String value.
    // Otherwise, if the given TBData is a double or date, sets the date value.
    void setValue(TBData& value);
    
    // Returns the date type.
    String getType() { return TBConstants::TYPE_DATE; }
    
    // Returns true if the given data is a date type and their values are
    // equal, false otherwise
    bool equals(TBData* data);
    
private:
    // Value.
    double value;
    
    // Human-readable representation of value.
    String valueStr;
};


// <summary>
// Data type that holds an array.
// </summary>
//
// <synopsis>
// TBArrayData is the abstract class for array data types that the browser
// knows about.  Subclasses of TBArrayData do not have to implement much of
// the abstract methods in TBData, but they do have to implement some
// array-specific methods.  Because of the way the browser is set up, an
// array may or may not have actual data loaded into it; generally speaking,
// one-dimensional arrays are loaded (copied) immediately upon construction
// whereas other arrays must be manually loaded.  This is to save space.
// </synopsis>

class TBArrayData : public TBData {
public:
    // Default Constructor.
    TBArrayData();
    
    ~TBArrayData();
    
    
    // Returns the array's shape.
    vector<int> getShape();
    
    // Returns true if the array has data loaded into it, false otherwise.
    bool isLoaded();
    
    // Returns true if the array is one-dimensional, false otherwise.
    bool isOneDimensional();
    
    // Returns true if the given coordinate is a valid index for this array
    // given its shape, false otherwise.
    bool coordIsValid(vector<int> d);
    
    // Returns true if the array is empty (i.e., all dimensions have size 0),
    // false otherwise.
    bool isEmpty();
    
    // Returns the first item in the array, or NULL if there is no data loaded.
    TBData* firstItem();

    
    // dataAt() must be implemented by any subclass.  Returns a TBData copy
    // of the data at the given coordinates, or NULL if the coordinates are
    // invalid or there is no loaded data.  NOTE: generally speaking, since
    // subclasses do not internally store their data as arrays of TBData*,
    // the returned TBData object must be deleted by the caller.
    virtual TBData* dataAt(vector<int> d) = 0;  
    
    // asString() must be implemented by any subclass.  Generally speaking,
    // should return the array data for one-dimensional arrays or the shape
    // and type otherwise.
    virtual String asString() = 0;
    
    // release() must be implemented by any subclass.  If data is loaded,
    // release it and free the memory.
    virtual bool release() = 0;
    
    // setDataAt() must be implemented by any subclass.  Sets the data at
    // the given coordinates to the given value.  NOTE: this method's behavior
    // is undefined if the type of the TBData does not match the type of the
    // array.
    virtual void setDataAt(vector<int> d, TBData& value) = 0;
    
    
    // Invalid operations.
    // <group>
    double asDouble() { return 0; }
    float asFloat() { return 0; }   
    int asInt() { return 0; }   
    unsigned int asUInt() { return 0; } 
    bool asBool() { return 0; } 
    char asChar() { return 0; } 
    unsigned char asUChar() { return 0; }   
    short int asShort() { return 0; }   
    pair<float, float> asComplex() { return pair<float, float>(0, 0); } 
    pair<double, double> asDComplex() { return pair<double, double>(0, 0); }    
    Record* asRecord() { return 0; }
    void setValue(TBData& value) { (void)value; }
    
    bool equals(TBData* data) { return false; (void)data; }
    // </group>
    
    
    // contains() must be implemented by any subclass.  Returns true if this
    // array has data loaded and contains the given value, false otherwise.
    virtual bool contains(TBData* data) = 0;
    
    // containsBetween() must be implemented by any subclass.  Returns true
    // if this array has data loaded and contains a value between the two
    // given values, false otherwise.  NOTE: the behavior is undefined for
    // arrays with non-numerical values.
    virtual bool containsBetween(TBData* data, TBData* data2) = 0;
    
    // containsLessThan() must be implemented by any subclass.  Returns true
    // if this array has data loaded and contains a value less than the
    // given value, false otherwise.  NOTE: the behavior is undefined for
    // arrays with non-numerical values.
    virtual bool containsLessThan(TBData* data) = 0;
    
    // containsGreaterThan() must be implemented by any subclass.  Returns true
    // if this array has data loaded and contains a value greater than the
    // given value, false otherwise.  NOTE: the behavior is undefined for
    // arrays with non-numerical values.
    virtual bool containsGreaterThan(TBData* data) = 0;
    
    // to1DString() must be implemented by any subclass.  Returns a "flattened"
    // version of the array.
    virtual String to1DString() = 0;
    
protected:
    // Array shape.
    vector<int> shape;
    
    // Whether data is loaded.
    bool loaded;
    
    // Whether the array is one-dimensional.
    bool oneDim;
};


// <summary>
// Implementation of TBArrayData for String array data.
// </summary>

class TBArrayDataString : public TBArrayData {
public:
    // Default constructor.  Builds an empty, unloaded array.
    TBArrayDataString();
    
    // Constructor that takes the value and whether or not to load (copy) the
    // given data or not.  Note: data is always loaded for one-dimensional
    // arrays.
    TBArrayDataString(const Array<String>& value, bool full = false);
    
    // Constructor that copies the given data if it is the correct type.
    TBArrayDataString(TBData& data);
    
    ~TBArrayDataString();
    
    
    // See TBArrayData::dataAt().  Returns data of type String.
    TBData* dataAt(vector<int> d);
    
    // Returns the value.
    Array<String>& data() { return value; }
    
    // Returns the String representation of this array.  For one-dimensional,
    // loaded arrays returns the values; otherwise returns the shape and type.
    String asString();
    
    // Loads the given data into the array.
    void load(const Array<String>& value);
    
    // Releases the loaded data, if any.  Returns whether the release was
    // successful or not.
    bool release();
    
    // If the array is loaded, sets the value at the given coordinates (if
    // valid) to the given data.  This method is not defined if the given
    // value is not of the correct type.
    void setDataAt(vector<int> d, TBData& value);
    
    
    // Returns the String array type.
    String getType() { return TBConstants::TYPE_ARRAY_STRING; }
    
    // See TBArrayData::contains().  Returns false if the given data is not
    // of type String.
    bool contains(TBData* data);
    
    // See TBArrayData::containsBetween().  Returns false because Strings
    // are not numberable.
    bool containsBetween(TBData* data, TBData* data2) { return false; (void)data,(void)data2; }
    
    // See TBArrayData::containsLessThan().  Returns false because Strings
    // are not numberable.
    bool containsLessThan(TBData* data) { return false; (void)data; }
    
    // See TBArrayData::containsGreaterThan().  Returns false because Strings
    // are not numberable.
    bool containsGreaterThan(TBData* data)  { (void)data;   return false; }
    
    // See TBArrayData::to1DString().
    String to1DString();
    
private:
    // Value.
    Array<String> value;
};


// <summary>
// Implementation of TBArrayData for double array data.
// </summary>

class TBArrayDataDouble : public TBArrayData {
public:
    // Default constructor.  Builds an empty, unloaded array.
    TBArrayDataDouble();
    
    // Constructor that takes the value and whether or not to load (copy) the
    // given data or not.  Note: data is always loaded for one-dimensional
    // arrays.
    TBArrayDataDouble(const Array<Double>& value, bool full = false);
    
    // Constructor that copies the given data if it is the correct type.
    TBArrayDataDouble(TBData& data);
    
    ~TBArrayDataDouble();
    
    
    // See TBArrayData::dataAt().  Returns data of type double.
    TBData* dataAt(vector<int> d);
    
    // Returns the value.
    Array<Double>& data() { return value; }
    
    // Returns the String representation of this array.  For one-dimensional,
    // loaded arrays returns the values; otherwise returns the shape and type.
    String asString();
    
    // Loads the given data into the array.
    void load(const Array<Double>& value);
    
    // Releases the loaded data, if any.  Returns whether the release was
    // successful or not.
    bool release();
    
    // If the array is loaded, sets the value at the given coordinates (if
    // valid) to the given data.  This method is not defined if the given
    // value is not of the correct type.
    void setDataAt(vector<int> d, TBData& value);
    
    
    // Returns the double array type.
    String getType() { return TBConstants::TYPE_ARRAY_DOUBLE; }
    
    // See TBArrayData::contains().  Returns false if the given data is not
    // of type double.
    bool contains(TBData* data);
    
    // See TBArrayData::containsBetween().  Returns false if either data is
    // not of type double.
    bool containsBetween(TBData* data, TBData* data2);
    
    // See TBArrayData::containsLessThan().  Returns false if either data is
    // not of type double.
    bool containsLessThan(TBData* data);
    
    // See TBArrayData::containsGreaterThan().  Returns false if either data is
    // not of type double.
    bool containsGreaterThan(TBData* data);
    
    // See TBArrayData::to1DString().
    String to1DString();
    
private:
    // Value.
    Array<Double> value;
};


// <summary>
// Implementation of TBArrayData for float array data.
// </summary>

class TBArrayDataFloat : public TBArrayData {
public:
    // Default constructor.  Builds an empty, unloaded array.
    TBArrayDataFloat();
    
    // Constructor that takes the value and whether or not to load (copy) the
    // given data or not.  Note: data is always loaded for one-dimensional
    // arrays.
    TBArrayDataFloat(const Array<Float>& value, bool full = false);
    
    // Constructor that copies the given data if it is the correct type.
    TBArrayDataFloat(TBData& data);
    
    ~TBArrayDataFloat();
    
    
    // See TBArrayData::dataAt().  Returns data of type float.
    TBData* dataAt(vector<int> d);
    
    // Returns the value.
    Array<Float>& data() { return value; }
    
    // Returns the String representation of this array.  For one-dimensional,
    // loaded arrays returns the values; otherwise returns the shape and type.
    String asString();
    
    // Loads the given data into the array.
    void load(const Array<Float>& value);
    
    // Releases the loaded data, if any.  Returns whether the release was
    // successful or not.
    bool release();
    
    // If the array is loaded, sets the value at the given coordinates (if
    // valid) to the given data.  This method is not defined if the given
    // value is not of the correct type.
    void setDataAt(vector<int> d, TBData& value);
    
    
    // Returns the float array type.
    String getType() { return TBConstants::TYPE_ARRAY_FLOAT; }
    
    // See TBArrayData::contains().  Returns false if the given data is not
    // of type float.
    bool contains(TBData* data);
    
    // See TBArrayData::containsBetween().  Returns false if either data is
    // not of type float.
    bool containsBetween(TBData* data, TBData* data2);
    
    // See TBArrayData::containsLessThan().  Returns false if either data is
    // not of type float.
    bool containsLessThan(TBData* data);
    
    // See TBArrayData::containsGreaterThan().  Returns false if either data is
    // not of type float.
    bool containsGreaterThan(TBData* data);
    
    // See TBArrayData::to1DString().
    String to1DString();
    
private:
    // Value.
    Array<Float> value;
};


// <summary>
// Implementation of TBArrayData for int array data.
// </summary>

class TBArrayDataInt : public TBArrayData {
public:
    // Default constructor.  Builds an empty, unloaded array.
    TBArrayDataInt();
    
    // Constructor that takes the value and whether or not to load (copy) the
    // given data or not.  Note: data is always loaded for one-dimensional
    // arrays.
    TBArrayDataInt(const Array<Int>& value, bool full = false);
    
    // Constructor that copies the given data if it is the correct type.
    TBArrayDataInt(TBData& data);
    
    ~TBArrayDataInt();
    
    
    // See TBArrayData::dataAt().  Returns data of type int.
    TBData* dataAt(vector<int> d);
    
    // Returns the value.
    Array<Int>& data() { return value; }
    
    // Returns the String representation of this array.  For one-dimensional,
    // loaded arrays returns the values; otherwise returns the shape and type.
    String asString();
    
    // Loads the given data into the array.
    void load(const Array<Int>& value);
    
    // Releases the loaded data, if any.  Returns whether the release was
    // successful or not.
    bool release();
    
    // If the array is loaded, sets the value at the given coordinates (if
    // valid) to the given data.  This method is not defined if the given
    // value is not of the correct type.
    void setDataAt(vector<int> d, TBData& value);
    
    
    // Returns the int array type.
    String getType() { return TBConstants::TYPE_ARRAY_INT; }
    
    // See TBArrayData::contains().  Returns false if the given data is not
    // of type int.
    bool contains(TBData* data);
    
    // See TBArrayData::containsBetween().  Returns false if either data is
    // not of type int.
    bool containsBetween(TBData* data, TBData* data2);
    
    // See TBArrayData::containsLessThan().  Returns false if either data is
    // not of type int.
    bool containsLessThan(TBData* data);
    
    // See TBArrayData::containsGreaterThan().  Returns false if either data is
    // not of type int.
    bool containsGreaterThan(TBData* data);
    
    // See TBArrayData::to1DString().
    String to1DString();
    
private:
    // Value.
    Array<Int> value;
};


// <summary>
// Implementation of TBArrayData for unsigned int array data.
// </summary>

class TBArrayDataUInt : public TBArrayData {
public:
    // Default constructor.  Builds an empty, unloaded array.
    TBArrayDataUInt();
    
    // Constructor that takes the value and whether or not to load (copy) the
    // given data or not.  Note: data is always loaded for one-dimensional
    // arrays.
    TBArrayDataUInt(const Array<uInt>& value, bool full = false);
    
    // Constructor that copies the given data if it is the correct type.
    TBArrayDataUInt(TBData& data);
    
    ~TBArrayDataUInt();
    
    
    // See TBArrayData::dataAt().  Returns data of type unsigned int.
    TBData* dataAt(vector<int> d);
    
    // Returns the value.
    Array<uInt>& data() { return value; }
    
    // Returns the String representation of this array.  For one-dimensional,
    // loaded arrays returns the values; otherwise returns the shape and type.
    String asString();
    
    // Loads the given data into the array.
    void load(const Array<uInt>& value);
    
    // Releases the loaded data, if any.  Returns whether the release was
    // successful or not.
    bool release();
    
    // If the array is loaded, sets the value at the given coordinates (if
    // valid) to the given data.  This method is not defined if the given
    // value is not of the correct type.
    void setDataAt(vector<int> d, TBData& value);
    
    
    // Returns the unsigned int array type.
    String getType() { return TBConstants::TYPE_ARRAY_UINT; }
    
    // See TBArrayData::contains().  Returns false if the given data is not
    // of type unsigned int.
    bool contains(TBData* data);
    
    // See TBArrayData::containsBetween().  Returns false if either data is
    // not of type unsigned int.
    bool containsBetween(TBData* data, TBData* data2);
    
    // See TBArrayData::containsLessThan().  Returns false if either data is
    // not of type unsigned int.
    bool containsLessThan(TBData* data);
    
    // See TBArrayData::containsGreaterThan().  Returns false if either data is
    // not of type unsigned int.
    bool containsGreaterThan(TBData* data);
    
    // See TBArrayData::to1DString().
    String to1DString();
    
private:
    // Value.
    Array<uInt> value;
};


// <summary>
// Implementation of TBArrayData for boolean array data.
// </summary>

class TBArrayDataBool : public TBArrayData {
public:
    // Default constructor.  Builds an empty, unloaded array.
    TBArrayDataBool();
    
    // Constructor that takes the value and whether or not to load (copy) the
    // given data or not.  Note: data is always loaded for one-dimensional
    // arrays.
    TBArrayDataBool(const Array<Bool>& value, bool full = false);
    
    // Constructor that copies the given data if it is the correct type.
    TBArrayDataBool(TBData& data);
    
    ~TBArrayDataBool();
    
    
    // See TBArrayData::dataAt().  Returns data of type boolean.
    TBData* dataAt(vector<int> d);
    
    // Returns the value.
    Array<Bool>& data() { return value; }
    
    // Returns the String representation of this array.  For one-dimensional,
    // loaded arrays returns the values; otherwise returns the shape and type.
    String asString();
    
    // Loads the given data into the array.
    void load(const Array<Bool>& value);
    
    // Releases the loaded data, if any.  Returns whether the release was
    // successful or not.
    bool release();
    
    // If the array is loaded, sets the value at the given coordinates (if
    // valid) to the given data.  This method is not defined if the given
    // value is not of the correct type.
    void setDataAt(vector<int> d, TBData& value);
    
    
    // Returns the boolean array type.
    String getType() { return TBConstants::TYPE_ARRAY_BOOL; }
    
    // See TBArrayData::contains().  Returns false if the given data is not
    // of type boolean.
    bool contains(TBData* data);
    
    // See TBArrayData::containsBetween().  Returns false if either data is
    // not of type boolean.
    bool containsBetween(TBData* data, TBData* data2);
    
    // See TBArrayData::containsLessThan().  Returns false if either data is
    // not of type boolean.
    bool containsLessThan(TBData* data);
    
    // See TBArrayData::containsGreaterThan().  Returns false if either data is
    // not of type boolean.
    bool containsGreaterThan(TBData* data);
    
    // See TBArrayData::to1DString().
    String to1DString();
    
private:
    // Value.
    Array<Bool> value;
};


// <summary>
// Implementation of TBArrayData for character array data.
// </summary>

class TBArrayDataChar : public TBArrayData {
public:
    // Default constructor.  Builds an empty, unloaded array.
    TBArrayDataChar();
    
    // Constructor that takes the value and whether or not to load (copy) the
    // given data or not.  Note: data is always loaded for one-dimensional
    // arrays.
    TBArrayDataChar(const Array<Char>& value, bool full = false);
    
    // Constructor that copies the given data if it is the correct type.
    TBArrayDataChar(TBData& data);
    
    ~TBArrayDataChar();
    
    
    // See TBArrayData::dataAt().  Returns data of type character.
    TBData* dataAt(vector<int> d);
    
    // Returns the value.
    Array<Char>& data() { return value; }
    
    // Returns the String representation of this array.  For one-dimensional,
    // loaded arrays returns the values; otherwise returns the shape and type.
    String asString();
    
    // Loads the given data into the array.
    void load(const Array<Char>& value);
    
    // Releases the loaded data, if any.  Returns whether the release was
    // successful or not.
    bool release();
    
    // If the array is loaded, sets the value at the given coordinates (if
    // valid) to the given data.  This method is not defined if the given
    // value is not of the correct type.
    void setDataAt(vector<int> d, TBData& value);
    
    
    // Returns the character array type.
    String getType() { return TBConstants::TYPE_ARRAY_CHAR; }
    
    // See TBArrayData::contains().  Returns false if the given data is not
    // of type character.
    bool contains(TBData* data);
    
    // See TBArrayData::containsBetween().  Returns false if either data is
    // not of type character.
    bool containsBetween(TBData* data, TBData* data2);
    
    // See TBArrayData::containsLessThan().  Returns false if either data is
    // not of type character.
    bool containsLessThan(TBData* data);
    
    // See TBArrayData::containsGreaterThan().  Returns false if either data is
    // not of type character.
    bool containsGreaterThan(TBData* data);
    
    // See TBArrayData::to1DString().
    String to1DString();
    
private:
    // Value.
    Array<Char> value;
};


// <summary>
// Implementation of TBArrayData for unsigned character array data.
// </summary>

class TBArrayDataUChar : public TBArrayData {
public:
    // Default constructor.  Builds an empty, unloaded array.
    TBArrayDataUChar();
    
    // Constructor that takes the value and whether or not to load (copy) the
    // given data or not.  Note: data is always loaded for one-dimensional
    // arrays.
    TBArrayDataUChar(const Array<uChar>& value, bool full = false);
    
    // Constructor that copies the given data if it is the correct type.
    TBArrayDataUChar(TBData& data);
    
    ~TBArrayDataUChar();
    
    
    // See TBArrayData::dataAt().  Returns data of type unsigned character.
    TBData* dataAt(vector<int> d);
    
    // Returns the value.
    Array<uChar>& data() { return value; }
    
    // Returns the String representation of this array.  For one-dimensional,
    // loaded arrays returns the values; otherwise returns the shape and type.
    String asString();
    
    // Loads the given data into the array.
    void load(const Array<uChar>& value);
    
    // Releases the loaded data, if any.  Returns whether the release was
    // successful or not.
    bool release();
    
    // If the array is loaded, sets the value at the given coordinates (if
    // valid) to the given data.  This method is not defined if the given
    // value is not of the correct type.
    void setDataAt(vector<int> d, TBData& value);
    
    
    // Returns the unsigned character array type.
    String getType() { return TBConstants::TYPE_ARRAY_UCHAR; }
    
    // See TBArrayData::contains().  Returns false if the given data is not
    // of type unsigned char.
    bool contains(TBData* data);
    
    // See TBArrayData::containsBetween().  Returns false if either data is
    // not of type unsigned character.
    bool containsBetween(TBData* data, TBData* data2);
    
    // See TBArrayData::containsLessThan().  Returns false if either data is
    // not of type unsigned character.
    bool containsLessThan(TBData* data);
    
    // See TBArrayData::containsGreaterThan().  Returns false if either data is
    // not of type unsigned character.
    bool containsGreaterThan(TBData* data);
    
    // See TBArrayData::to1DString().
    String to1DString();
    
private:
    // Value.
    Array<uChar> value;
};


// <summary>
// Implementation of TBArrayData for short array data.
// </summary>

class TBArrayDataShort : public TBArrayData {
public:
    // Default constructor.  Builds an empty, unloaded array.
    TBArrayDataShort();
    
    // Constructor that takes the value and whether or not to load (copy) the
    // given data or not.  Note: data is always loaded for one-dimensional
    // arrays.
    TBArrayDataShort(const Array<Short>& value, bool full = false);
    
    // Constructor that copies the given data if it is the correct type.
    TBArrayDataShort(TBData& data);
    
    ~TBArrayDataShort();
    
    
    // See TBArrayData::dataAt().  Returns data of type short.
    TBData* dataAt(vector<int> d);
    
    // Returns the value.
    Array<Short>& data() { return value; }
    
    // Returns the String representation of this array.  For one-dimensional,
    // loaded arrays returns the values; otherwise returns the shape and type.
    String asString();
    
    // Loads the given data into the array.
    void load(const Array<Short>& value);
    
    // Releases the loaded data, if any.  Returns whether the release was
    // successful or not.
    bool release();
    
    // If the array is loaded, sets the value at the given coordinates (if
    // valid) to the given data.  This method is not defined if the given
    // value is not of the correct type.
    void setDataAt(vector<int> d, TBData& value);
    
    
    // Returns the short array type.
    String getType() { return TBConstants::TYPE_ARRAY_SHORT; }
    
    // See TBArrayData::contains().  Returns false if the given data is not
    // of type short.
    bool contains(TBData* data);
    
    // See TBArrayData::containsBetween().  Returns false if either data is
    // not of type short.
    bool containsBetween(TBData* data, TBData* data2);
    
    // See TBArrayData::containsLessThan().  Returns false if either data is
    // not of type short.
    bool containsLessThan(TBData* data);
    
    // See TBArrayData::containsGreaterThan().  Returns false if either data is
    // not of type short.
    bool containsGreaterThan(TBData* data);
    
    // See TBArrayData::to1DString().
    String to1DString();
    
private:
    // Value.
    Array<Short> value;
};


// <summary>
// Implementation of TBArrayData for complex array data.
// </summary>

class TBArrayDataComplex : public TBArrayData {
public:
    // Default constructor.  Builds an empty, unloaded array.
    TBArrayDataComplex();
    
    // Constructor that takes the value and whether or not to load (copy) the
    // given data or not.  Note: data is always loaded for one-dimensional
    // arrays.
    TBArrayDataComplex(const Array<Complex>& value, bool full = false);
    
    // Constructor that copies the given data if it is the correct type.
    TBArrayDataComplex(TBData& data);
    
    ~TBArrayDataComplex();
    
    
    // See TBArrayData::dataAt().  Returns data of type complex.
    TBData* dataAt(vector<int> d);
    
    // Returns the value.
    Array<Complex>& data() { return value; }
    
    // Returns the String representation of this array.  For one-dimensional,
    // loaded arrays returns the values; otherwise returns the shape and type.
    String asString();
    
    // Loads the given data into the array.
    void load(const Array<Complex>& value);
    
    // Releases the loaded data, if any.  Returns whether the release was
    // successful or not.
    bool release();
    
    // If the array is loaded, sets the value at the given coordinates (if
    // valid) to the given data.  This method is not defined if the given
    // value is not of the correct type.
    void setDataAt(vector<int> d, TBData& value);
    
    
    // Returns the complex array type.
    String getType() { return TBConstants::TYPE_ARRAY_COMPLEX; }
    
    // See TBArrayData::contains().  Returns false if the given data is not
    // of type complex.
    bool contains(TBData* data);
    
    // See TBArrayData::containsBetween().  Returns false if either data is
    // not of type complex.
    bool containsBetween(TBData* data, TBData* data2);
    
    // See TBArrayData::containsLessThan().  Returns false if either data is
    // not of type complex.
    bool containsLessThan(TBData* data);
    
    // See TBArrayData::containsGreaterThan().  Returns false if either data is
    // not of type complex.
    bool containsGreaterThan(TBData* data);
    
    // See TBArrayData::to1DString().
    String to1DString();
    
private:
    // Value.
    Array<Complex> value;
};


// <summary>
// Implementation of TBArrayData for double complex array data.
// </summary>

class TBArrayDataDComplex : public TBArrayData {
public:
    // Default constructor.  Builds an empty, unloaded array.
    TBArrayDataDComplex();
    
    // Constructor that takes the value and whether or not to load (copy) the
    // given data or not.  Note: data is always loaded for one-dimensional
    // arrays.
    TBArrayDataDComplex(const Array<DComplex>& value, bool full = false);
    
    // Constructor that copies the given data if it is the correct type.
    TBArrayDataDComplex(TBData& data);
    
    ~TBArrayDataDComplex();
    
    
    // See TBArrayData::dataAt().  Returns data of type double complex.
    TBData* dataAt(vector<int> d);
    
    // Returns the value.
    Array<DComplex>& data() { return value; }
    
    // Returns the String representation of this array.  For one-dimensional,
    // loaded arrays returns the values; otherwise returns the shape and type.
    String asString();
    
    // Loads the given data into the array.
    void load(const Array<DComplex>& value);
    
    // Releases the loaded data, if any.  Returns whether the release was
    // successful or not.
    bool release();
    
    // If the array is loaded, sets the value at the given coordinates (if
    // valid) to the given data.  This method is not defined if the given
    // value is not of the correct type.
    void setDataAt(vector<int> d, TBData& value);
    
    
    // Returns the double complex array type.
    String getType() { return TBConstants::TYPE_ARRAY_DCOMPLEX; }
    
    // See TBArrayData::contains().  Returns false if the given data is not
    // of type double complex.
    bool contains(TBData* data);
    
    // See TBArrayData::containsBetween().  Returns false if either data is
    // not of type double complex.
    bool containsBetween(TBData* data, TBData* data2);
    
    // See TBArrayData::containsLessThan().  Returns false if either data is
    // not of type double complex.
    bool containsLessThan(TBData* data);
    
    // See TBArrayData::containsGreaterThan().  Returns false if either data is
    // not of type double complex.
    bool containsGreaterThan(TBData* data);
    
    // See TBArrayData::to1DString().
    String to1DString();
    
private:
    // Value.
    Array<DComplex> value;
};

}

#endif /*TBDATA_H_*/
