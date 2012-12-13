//# PlotMSAveraging.cc: Averaging parameters.
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
#include <plotms/PlotMS/PlotMSAveraging.h>

namespace casa {

/////////////////////////////////
// PLOTMSAVERAGING DEFINITIONS //
/////////////////////////////////

// Static //

bool PlotMSAveraging::fieldHasValue(Field f) {
    return f == CHANNEL || f == TIME; }

const vector<PlotMSAveraging::Field>&
PlotMSAveraging::fieldMutuallyExclusiveGroup(Field f) {
    if(f == BASELINE) {
        static vector<Field> v(1, ANTENNA);
        return v;
    } else if(f == ANTENNA) {
        static vector<Field> v(1, BASELINE);
        return v;
    } else {
        static vector<Field> v;
        return v;
    }
}

const String PlotMSAveraging::RKEY_VALUE = "Value";


// Non-Static //

PlotMSAveraging::PlotMSAveraging() { setDefaults(); }
PlotMSAveraging::~PlotMSAveraging() { }


void PlotMSAveraging::fromRecord(const RecordInterface& record) {    
    const vector<String>& fields = fieldStrings();
    String sf; Field f;
    for(unsigned int i = 0; i < fields.size(); i++) {
        sf = fields[i]; f = field(sf);
        if(!record.isDefined(sf)) continue;
        
        // Set as bool/double.
        if(record.dataType(sf) == TpBool) {
            setFlag(f, record.asBool(sf));
            if(fieldHasValue(f)) {
                sf += RKEY_VALUE;
                if(record.isDefined(sf) && record.dataType(sf) == TpDouble)
                    setValue(f, record.asDouble(sf));
            }
            
        // Set as String.
        } else if(record.dataType(sf) == TpString) {
            setValue(f, record.asString(sf));
        }
    }
}

Record PlotMSAveraging::toRecord(bool useStrings) const {
    Record rec(Record::Variable);
    
    const vector<Field>& f = fields();
    for(unsigned int i = 0; i < f.size(); i++) {
        // Set as String.
        if(useStrings && fieldHasValue(f[i])) {
            rec.define(field(f[i]), getValueStr(f[i]));
            
        // Set as bool/double.
        } else {
            rec.define(field(f[i]), getFlag(f[i]));
            if(fieldHasValue(f[i]))
                rec.define(field(f[i]) + RKEY_VALUE, getValue(f[i]));
        }
    }
    
    return rec;
}

bool PlotMSAveraging::getFlag(Field f) const {
    return const_cast<map<Field, bool>&>(itsFlags_)[f]; }
void PlotMSAveraging::setFlag(Field f, bool on) {
	itsFlags_[f] = on;
	
	// Check for mutually exclusive fields.  The mutual exclusivity (should) be
	// already enforced in the GUI, but this check is needed for non-GUI
	// averaging parameter setting.
	const vector<Field>& mutExFields = fieldMutuallyExclusiveGroup(f);
	if(on && mutExFields.size() > 0)
	    for(unsigned int i = 0; i < mutExFields.size(); i++)
	        itsFlags_[mutExFields[i]] = false;
}

double PlotMSAveraging::getValue(Field f) const {
    if(!fieldHasValue(f)) return 0;
    else return const_cast<map<Field, double>&>(itsValues_)[f];
}
void PlotMSAveraging::setValue(Field f, double value) {
    if(fieldHasValue(f)) itsValues_[f] = value; }

String PlotMSAveraging::getValueStr(Field f) const {
    if(!getFlag(f) || !fieldHasValue(f)) return "";
    else return String::toString(getValue(f));
}
void PlotMSAveraging::setValue(Field f, const String& value) {
    setFlag(f, !value.empty());
    if(fieldHasValue(f)) {
        double d;
        if(sscanf(value.c_str(), "%lf", &d) >= 1)
            setValue(f, d);
    }
}


bool PlotMSAveraging::operator==(const PlotMSAveraging& other) const {
    vector<Field> f = fields();
    for(unsigned int i = 0; i < f.size(); i++) {
        if(getFlag(f[i]) != other.getFlag(f[i])) return false;
        if(fieldHasValue(f[i]) && getFlag(f[i]) &&
           getValue(f[i]) != other.getValue(f[i])) return false;
    }
    
    return true;
}


String PlotMSAveraging::summary() const {

  stringstream ss;
  ss.precision(6);

  Bool anyAveraging=channel()||time()||baseline()||antenna()||spw();

  ss << "Data Averaging: ";

  ss << boolalpha;

  if (anyAveraging) {

    ss << endl;

    ss << " Using" << (scalarAve() ? " SCALAR " : " VECTOR ") << "averaging." 
       << endl;

    if (channel()) {
      ss << " Channel: ";
      Double val = channelValue();
      if (val <=0)
	ss << "None.";
      else
	ss << val << (val > 1 ? " channels" : " (fraction of spw)");
      ss << endl;
    }
    if (time()) {
      ss << " Time: " << timeValue() << " seconds. ";
      ss << " Scan: " << scan() << ";  Field: " << field() << endl;
    }
    if (baseline()) 
      ss << " All Baselines: " << baseline() << endl;
    if (antenna()) 
      ss << " Per Antenna: " << antenna() << endl;
    if (spw())
      ss << " All Spectral Windows: " << spw() << endl;

  }
  else
    ss << "None." << endl;

  return ss.str();

	
}


void PlotMSAveraging::setDefaults() {    
    vector<Field> f = fields();
    for(unsigned int i = 0; i < f.size(); i++) {
        itsFlags_[f[i]] = false;
        if(fieldHasValue(f[i])) itsValues_[f[i]] = 0;
    }
}

}
