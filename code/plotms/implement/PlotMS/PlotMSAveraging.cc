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
        if(record.isDefined(sf) && record.dataType(sf) == TpBool)
            setFlag(f, record.asBool(sf));
        if(fieldHasValue(f)) {
            sf += RKEY_VALUE;
            if(record.isDefined(sf) && record.dataType(sf) == TpDouble)
                setValue(f, record.asDouble(sf));
        }
    }
}

Record PlotMSAveraging::toRecord() const {
    Record rec(Record::Variable);
    
    const vector<Field>& f = fields();
    for(unsigned int i = 0; i < f.size(); i++) {
        rec.define(field(f[i]), getFlag(f[i]));
        if(fieldHasValue(f[i]))
            rec.define(field(f[i]) + RKEY_VALUE, getValue(f[i]));
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


bool PlotMSAveraging::operator==(const PlotMSAveraging& other) const {
    vector<Field> f = fields();
    for(unsigned int i = 0; i < f.size(); i++) {
        if(getFlag(f[i]) != other.getFlag(f[i])) return false;
        if(fieldHasValue(f[i]) && getFlag(f[i]) &&
           getValue(f[i]) != other.getValue(f[i])) return false;
    }
    
    return true;
}


void PlotMSAveraging::setDefaults() {    
    vector<Field> f = fields();
    for(unsigned int i = 0; i < f.size(); i++) {
        itsFlags_[f[i]] = false;
        if(fieldHasValue(f[i])) itsValues_[f[i]] = 0;
    }
}

}
