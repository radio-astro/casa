//# PlotMSAveraging.h: Averaging parameters.
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
#ifndef PLOTMSAVERAGING_H_
#define PLOTMSAVERAGING_H_

#include <casa/Containers/Record.h>
#include <plotms/PlotMS/PlotMSConstants.h>

#include <map>

#include <casa/namespace.h>

namespace casa {

// Specifies averaging parameters for an MS.
class PlotMSAveraging {
public:
    // Static //
    
    // Enum and methods to define the different fields for an MS averaging.
    // All fields have a bool flag for on/off, and some of them also have a
    // double value (see fieldHasValue()).  Fields are off by default, with a
    // default double value of 0 if applicable.
    // <group>
    PMS_ENUM1(Field, fields, fieldStrings, field,
              CHANNEL, TIME, SCAN, FIELD, BASELINE)
    PMS_ENUM2(Field, fields, fieldStrings, field,
              "channel", "time", "scan", "field", "baseline")
    // </group>
              
    // Returns true if the given field has a double value associated with it or
    // not.
    static bool fieldHasValue(Field f);
    
    
    // Non-Static //
    
    // Constructor, which uses default values.
    PlotMSAveraging();
    
    // Destructor.
    ~PlotMSAveraging();
    
    
    // Converts this object to/from a record.  Each field will have a key that
    // is its enum name, with a bool value for its flag value.  Fields that
    // also have double values will have an additional key that is its enum
    // name + "Value" (i.e. "channelValue" for CHANNEL) with a double value.
    // <group>
    void fromRecord(const RecordInterface& record);
    Record toRecord() const;
    // </group>
    
    // Gets/Sets the on/off value for the given field.
    // <group>
    bool getFlag(Field f) const;
    void getFlag(Field f, bool& flag) const { flag = getFlag(f); }
    void setFlag(Field f, bool on);
    // </group>
    
    // Gets/Sets the double value for the given field, if applicable.
    // <group>
    double getValue(Field f) const;
    void getValue(Field f, double& value) const { value = getValue(f); }
    void setValue(Field f, double value);
    // </group>
    
    // Convenience methods for returning the standard field values.
    // <group>
    bool channel() const { return getFlag(CHANNEL); }
    double channelValue() const { return getValue(CHANNEL); }
    bool time() const { return getFlag(TIME); }
    double timeValue() const { return getValue(TIME); }
    bool scan() const { return getFlag(SCAN); }
    bool field() const { return getFlag(FIELD); }
    bool baseline() const { return getFlag(BASELINE); }
    // </group>
    
    // Convenience methods for setting the standard field values.
    // <group>
    void setChannel(bool channel) { setFlag(CHANNEL, channel); }
    void setChannelValue(double value) { setValue(CHANNEL, value); }
    void setTime(bool time) { setFlag(TIME, time); }
    void setTimeValue(double value) { setValue(TIME, value); }
    void setScan(bool scan) { setFlag(SCAN, scan); }
    void setField(bool field) { setFlag(FIELD, field); }
    void setBaseline(bool baseline) { setFlag(BASELINE, baseline); }
    // </group>
    
    
    // Equality operators.
    // <group>
    bool operator==(const PlotMSAveraging& other) const;
    bool operator!=(const PlotMSAveraging& other) const {
        return !(operator==(other)); }
    // </group>
    
private:
    // Averaging field values.
    map<Field, bool> itsFlags_;
    
    // Averaging field double values.
    map<Field, double> itsValues_;

    
    // Sets the default values.
    void setDefaults();
    
    
    // String constant for what to append to the enum name in the record to
    // get the key for the double value.
    static const String RKEY_VALUE;
};

}

#endif /* PLOTMSAVERAGING_H_ */
