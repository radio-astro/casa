//# PlotMSFlagging.cc: Flagging parameters.
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
#include <plotms/PlotMS/PlotMSFlagging.h>

#include <msvis/MSVis/VisSet.h>

namespace casa {

////////////////////////////////
// PLOTMSFLAGGING DEFINITIONS //
////////////////////////////////

// Static //

bool PlotMSFlagging::fieldHasValue(Field f) {
    return f == ANTENNA_ANTENNA; }

bool PlotMSFlagging::fieldHasSelectionValue(Field f) {
    return f == PlotMSFlagging::SEL_ALTERNATE; }

bool PlotMSFlagging::fieldDefault(Field f) {
    return f == CORR_ALL || f == ANTENNA_ANTENNA || f == SEL_SELECTED; }

const vector<PlotMSFlagging::Field>&
PlotMSFlagging::fieldMutuallyExclusiveGroup(Field f) {
    // Group: CORR_ALL, CORR_POLN_DEP
    if(f == CORR_ALL) {
        static vector<Field> v(1, CORR_POLN_DEP);
        return v;
    } else if(f == CORR_POLN_DEP) {
        static vector<Field> v(1, CORR_ALL);
        return v;
        
    // Group: ANTENNA_ANTENNA, ANTENNA_BASELINES
    } else if(f == ANTENNA_ANTENNA) {
        static vector<Field> v(1, ANTENNA_BASELINES);
        return v;
    } else if(f == ANTENNA_BASELINES) {
        static vector<Field> v(1, ANTENNA_ANTENNA);
        return v;
        
    // Group: SEL_SELECTED, SEL_ALTERNATE
    } else if(f == SEL_SELECTED) {
        static vector<Field> v(1, SEL_ALTERNATE);
        return v;
    } else if(f == SEL_ALTERNATE) {
        static vector<Field> v(1, SEL_SELECTED);
        return v;
        
    // No Group
    } else {
        static vector<Field> v;
        return v;
    }
}


const String PlotMSFlagging::RKEY_VALUE = "Value";
const String PlotMSFlagging::RKEY_SELVALUE = "SelectionValue";


// Constructors/Destructors //

PlotMSFlagging::PlotMSFlagging() : itsMS_(NULL), itsSelectedMS_(NULL),
        itsVisSet_(NULL) {
    setDefaults(); }

PlotMSFlagging::PlotMSFlagging(MeasurementSet* ms, MeasurementSet* selectedMS,
        VisSet* visSet) : itsMS_(ms), itsSelectedMS_(selectedMS),
        itsVisSet_(visSet) {
    setDefaults(); }

PlotMSFlagging::~PlotMSFlagging() { }


// Public Methods //

MeasurementSet* PlotMSFlagging::getMS() const { return itsMS_; }
MeasurementSet* PlotMSFlagging::getSelectedMS() const{ return itsSelectedMS_; }
VisSet* PlotMSFlagging::getVisSet() const { return itsVisSet_; }

void PlotMSFlagging::setMS(MeasurementSet* ms, MeasurementSet* selectedMS,
        VisSet* visSet) {
    itsMS_ = ms;
    itsSelectedMS_ = selectedMS;
    itsVisSet_ = visSet;
}


void PlotMSFlagging::fromRecord(const RecordInterface& record) {
    const vector<String>& fields = fieldStrings();
    String sf; Field f;
    for(unsigned int i = 0; i < fields.size(); i++) {
        sf = fields[i]; f = field(sf);
        if(!record.isDefined(sf)) continue;
        
        // Set as bool/double/record.
        if(record.dataType(sf) == TpBool) {
            setFlag(f, record.asBool(sf));
            if(fieldHasValue(f)) {
                sf = fields[i] + RKEY_VALUE;
                if(record.isDefined(sf) && record.dataType(sf) == TpDouble)
                    setValue(f, record.asDouble(sf));
            }
            
            if(fieldHasSelectionValue(f)) {
                sf = fields[i] + RKEY_SELVALUE;
                if(record.isDefined(sf) && record.dataType(sf) == TpRecord) {
                    PlotMSSelection value;
                    value.fromRecord(record.asRecord(sf));
                    setSelectionValue(f, value);
                }
            }
            
        // Set as String.
        } else if(record.dataType(sf) == TpString) {
            setValue(f, record.asString(sf));
        }
    }
}

Record PlotMSFlagging::toRecord(bool useStrings) const {
    Record rec(Record::Variable);
    
    const vector<Field>& f = fields();
    for(unsigned int i = 0; i < f.size(); i++) {
        // Set as String.
        if(useStrings && (f[i] == CORR_ALL || f[i] == CORR_POLN_DEP ||
           f[i] == ANTENNA_ANTENNA || f[i] == ANTENNA_BASELINES)) continue;
        else if(useStrings && (f[i] == CORR || f[i] == ANTENNA)) {
            rec.define(field(f[i]), getValueStr(f[i]));
            
        // Set as bool/double/record.
        } else {
            rec.define(field(f[i]), getFlag(f[i]));
            if(fieldHasValue(f[i]))
                rec.define(field(f[i]) + RKEY_VALUE, getValue(f[i]));
            if(fieldHasSelectionValue(f[i]))
                rec.defineRecord(field(f[i]) + RKEY_SELVALUE,
                        getSelectionValue(f[i]).toRecord());
        }
    }
    
    return rec;
}
    

bool PlotMSFlagging::getFlag(Field f) const {
    return const_cast<map<Field, bool>&>(itsFlags_)[f]; }
void PlotMSFlagging::setFlag(Field f, bool on) {
    itsFlags_[f] = on;
    
    // Check for mutually exclusive fields.  The mutual exclusivity (should) be
    // already enforced in the GUI, but this check is needed for non-GUI
    // averaging parameter setting.
    const vector<Field>& mutExFields = fieldMutuallyExclusiveGroup(f);
    if(mutExFields.size() > 0) {
        if(on) { // make sure this one is the only one that is on
            for(unsigned int i = 0; i < mutExFields.size(); i++)
                itsFlags_[mutExFields[i]] = false;
            
        } else { // make sure that one of the others is on
            bool valid = false;
            for(unsigned int i = 0; !valid && i < mutExFields.size(); i++)
                if(itsFlags_[mutExFields[i]]) valid = true;
            
            // if none are on, but this one back to true (sorry, caller!)
            if(!valid) itsFlags_[f] = true;
        }
    }
}

double PlotMSFlagging::getValue(Field f) const {
    if(!fieldHasValue(f)) return 0;
    else return const_cast<map<Field, double>&>(itsValues_)[f];
}
void PlotMSFlagging::setValue(Field f, double value) {
    if(fieldHasValue(f)) itsValues_[f] = value; }

String PlotMSFlagging::getValueStr(Field f) const {
    if(f == CORR || f == CORR_ALL || f == CORR_POLN_DEP) {
        if(!getFlag(CORR)) return "";
        else if(getFlag(CORR_ALL)) return "all";
        else if(getFlag(CORR_POLN_DEP)) return "poln-dep";
        else return "";
    } else if(f == ANTENNA || f == ANTENNA_ANTENNA || f == ANTENNA_BASELINES) {
        if(!getFlag(ANTENNA)) return "";
        else if(getFlag(ANTENNA_ANTENNA))
            return String::toString(getValue(ANTENNA_ANTENNA));
        else if(getFlag(ANTENNA_BASELINES)) return "all";
        else return "";
    } else return "";
}

void PlotMSFlagging::setValue(Field f, const String& value) {
    String val = value;
    val.downcase();
    if(f == CORR || f == CORR_ALL || f == CORR_POLN_DEP) {
        if(val.empty()) {
            setFlag(CORR, false);
            setFlag(CORR_ALL, false);
            setFlag(CORR_POLN_DEP, false);
        } else if(val == "all") {
            setFlag(CORR, true);
            setFlag(CORR_ALL, true);
            setFlag(CORR_POLN_DEP, false);
        } else if(val == "poln-dep") {
            setFlag(CORR, true);
            setFlag(CORR_ALL, false);
            setFlag(CORR_POLN_DEP, true);
        }
    } else if(f == ANTENNA || f == ANTENNA_ANTENNA || f == ANTENNA_BASELINES) {
        if(val.empty()) {
            setFlag(ANTENNA, false);
            setFlag(ANTENNA_ANTENNA, false);
            setFlag(ANTENNA_BASELINES, false);
        } else if(val == "all") {
            setFlag(ANTENNA, true);
            setFlag(ANTENNA_ANTENNA, false);
            setFlag(ANTENNA_BASELINES, true);
        } else {
            double d;
            if(sscanf(value.c_str(), "%lf", &d) >= 1) {
                setFlag(ANTENNA, true);
                setFlag(ANTENNA_ANTENNA, true);
                setValue(ANTENNA_ANTENNA, d);
                setFlag(ANTENNA_BASELINES, false);
            }
        }
    }
}


PlotMSSelection PlotMSFlagging::getSelectionValue(Field f) const {
    if(!fieldHasSelectionValue(f)) return PlotMSSelection();
    else return const_cast<map<Field, PlotMSSelection>&>(
            itsSelectionValues_)[f];
}
void PlotMSFlagging::setSelectionValue(Field f, const PlotMSSelection& value) {
    if(fieldHasSelectionValue(f)) itsSelectionValues_[f] = value; }


bool PlotMSFlagging::operator==(const PlotMSFlagging& other) const {
    vector<Field> f = fields();
    for(unsigned int i = 0; i < f.size(); i++) {
        if(getFlag(f[i]) != other.getFlag(f[i])) return false;
        if(fieldHasValue(f[i]) && getFlag(f[i]) &&
           getValue(f[i]) != other.getValue(f[i])) return false;
        if(fieldHasSelectionValue(f[i]) &&
           getSelectionValue(f[i]) != other.getSelectionValue(f[i]))
            return false;
    }

    return true;
}


// Private Methods //

void PlotMSFlagging::setDefaults() {
    vector<Field> f = fields();
    for(unsigned int i = 0; i < f.size(); i++) {
        itsFlags_[f[i]] = fieldDefault(f[i]);
        if(fieldHasValue(f[i])) itsValues_[f[i]] = 0;
        if(fieldHasSelectionValue(f[i]))
            itsSelectionValues_[f[i]] = PlotMSSelection();
    }
}

}
