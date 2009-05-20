//# PlotMSSelection.cc: MS Selection parameters.
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
#include <plotms/PlotMS/PlotMSSelection.h>

#include <ms/MeasurementSets/MSSelectionTools.h>

namespace casa {

/////////////////////////////////
// PLOTMSSELECTION DEFINITIONS //
/////////////////////////////////

// Static //

String PlotMSSelection::defaultValue(Field f) { return ""; }


// Non-Static //

PlotMSSelection::PlotMSSelection() {
    initDefaults(); }

PlotMSSelection::PlotMSSelection(const PlotMSSelection& copy) {
    operator=(copy); }

PlotMSSelection::~PlotMSSelection() { }


void PlotMSSelection::fromRecord(const RecordInterface& record) {
    const vector<String>& fields = fieldStrings();
    for(unsigned int i = 0; i < fields.size(); i++)
        if(record.isDefined(fields[i])&&record.dataType(fields[i]) == TpString)
            setValue(field(fields[i]), record.asString(fields[i]));
}

Record PlotMSSelection::toRecord() const {
    Record record(Record::Variable);
    
    const vector<Field>& f = fields();
    for(unsigned int i = 0; i < f.size(); i++)
        record.define(field(f[i]), getValue(f[i]));
    
    return record;
}

void PlotMSSelection::apply(MeasurementSet& ms, MeasurementSet& selMS,
        Matrix<Int>& chansel) const {    
    // Set the selected MeasurementSet to be the same initially as the input
    // MeasurementSet
    selMS = ms;

    if (corr()!="") {
      cout << "WARNING: Sorry, Correlation selection is not yet working." << endl;
      cout << "         Proceding with no correlation selection." << endl;
    }

    mssSetData(ms, selMS, "", timerange(), antenna(), field(), spw(),
               uvrange(), msselect(), "", scan(), array());

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

PlotMSSelection& PlotMSSelection::operator=(const PlotMSSelection& copy) {
    itsValues_ = copy.itsValues_;    
    return *this;
}


void PlotMSSelection::initDefaults() {
    vector<Field> f = fields();
    for(unsigned int i = 0; i < f.size(); i++)
        itsValues_[f[i]] = defaultValue(f[i]);
}

}
