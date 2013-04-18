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

#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSSelectionTools.h>
#include <synthesis/CalTables/NewCalTable.h>
#include <synthesis/CalTables/CTInterface.h>

namespace casa {

/////////////////////////////////
// PLOTMSSELECTION DEFINITIONS //
/////////////////////////////////

// Static //

String PlotMSSelection::defaultValue(Field /*f*/) { return ""; }


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
    if (record.isDefined("forceNew")&&record.dataType("forceNew")==TpInt)
      setForceNew(record.asInt("forceNew"));
}

Record PlotMSSelection::toRecord() const {
    Record record(Record::Variable);
    
    const vector<Field>& f = fields();
    for(unsigned int i = 0; i < f.size(); i++)
        record.define(field(f[i]), getValue(f[i]));
    record.define("forceNew",forceNew());
    
    return record;
}

void PlotMSSelection::apply(MeasurementSet& ms, MeasurementSet& selMS,
			    Vector<Vector<Slice> >& chansel,
			    Vector<Vector<Slice> >& corrsel) const {    
    // Set the selected MeasurementSet to be the same initially as the input
    // MeasurementSet
    selMS = ms;

    mssSetData(ms, selMS, chansel,corrsel, "", 
	       timerange(), antenna(), field(), spw(),
	       uvrange(), msselect(), corr(), scan(), array(),
	       "", observation());
}

void PlotMSSelection::apply(NewCalTable& ct, NewCalTable& selCT,
  		            Vector<Vector<Slice> >& /*chansel*/,
  		            Vector<Vector<Slice> >& /*corrsel*/) const {
  // Trap unsupported selections

  if (uvrange().length()>0)
    throw(AipsError("Selection by uvrange not supported for NewCalTable"));
  if (corr().length()>0)
    throw(AipsError("Selection by corr not supported for NewCalTable"));
  if (array().length()>0)
    throw(AipsError("Selection by array not supported for NewCalTable"));

  // Set the selected NewCalTable to be the same initially as the input
  // NewCalTable
  selCT = ct;

  //cout << "Whole NCT nrows    = " << ct.nrow() << endl;

  CTInterface cti(ct);
  MSSelection mss;
  mss.setTimeExpr(timerange());
  mss.setObservationExpr(observation());
  mss.setScanExpr(scan());
  mss.setSpwExpr(spw());
  mss.setFieldExpr(field());
  mss.setAntennaExpr(antenna());
  TableExprNode ten=mss.toTableExprNode(&cti);
  try {
    getSelectedTable(selCT,ct,ten,"");
  } catch (AipsError x) {
    //    logSink() << x.getMesg() << LogIO::SEVERE;
    throw(AipsError("Error selecing on caltable: "+ct.tableName()));
  }

  // TBD: fill chansel, corrsel


  //cout << "Selected NCT nrows = " << selCT.nrow() << endl;

}


const String& PlotMSSelection::getValue(Field f) const {
    return const_cast<map<Field,String>&>(itsValues_)[f]; }
void PlotMSSelection::setValue(Field f, const String& value) {
    itsValues_[f] = value; }


bool PlotMSSelection::operator==(const PlotMSSelection& other) const {    

  // Check forceNew counter first
  //  not equal (first reset forceNew so that it isn't sticky)
  if (forceNew()!=other.forceNew())  {
    return false;
  }

  return fieldsEqual(other);

}

bool PlotMSSelection::fieldsEqual(const PlotMSSelection& other) const {    

  vector<Field> f = fields();
  for(unsigned int i = 0; i < f.size(); i++)
    if(getValue(f[i]) != other.getValue(f[i])) return false;
  
  return true;
}

PlotMSSelection& PlotMSSelection::operator=(const PlotMSSelection& copy) {
    itsValues_ = copy.itsValues_;    
    forceNew_ = copy.forceNew_;
    return *this;
}


void PlotMSSelection::initDefaults() {
    vector<Field> f = fields();
    for(unsigned int i = 0; i < f.size(); i++)
        itsValues_[f[i]] = defaultValue(f[i]);

    // forceNew_ is a counter, start it at zero
    forceNew_=0;
}

}
