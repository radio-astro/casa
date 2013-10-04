//# CTMainColumns.h: Calibration table cal_main column access
//# Copyright (C) 2011 
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
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id$

#ifndef CALIBRATION_NEWCALMAINCOLUMNS_H
#define CALIBRATION_NEWCALMAINCOLUMNS_H

#include <casa/aips.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MFrequency.h>
#include <measures/Measures/MDirection.h>
#include <tables/Tables/TableColumn.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <measures/TableMeasures/TableMeasColumn.h>
#include <measures/TableMeasures/ScalarMeasColumn.h>
#include <measures/TableMeasures/ArrayMeasColumn.h>
#include <measures/TableMeasures/ScalarQuantColumn.h>
#include <synthesis/CalTables/NewCalTable.h>
#include <synthesis/CalTables/CTEnums.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class NewCalTable;

class ROCTMainColumns
{
 public:
  // Construct from a calibration table
  ROCTMainColumns (const NewCalTable& calTable);
  
  // Default destructor
  virtual ~ROCTMainColumns() {};
  
  // Read-only column accessors
  const ROScalarColumn<Double>& time() const {return time_p;};
  const ROScalarQuantColumn<Double>& timeQuant() const {return timeQuant_p;};
  const ROScalarMeasColumn<MEpoch>& timeMeas() const {return timeMeas_p;};
  const ROScalarColumn<Double>& timeEP() const {return timeEP_p;};
  const ROScalarQuantColumn<Double>& timeEPQuant() const 
  {return timeEPQuant_p;};
  const ROScalarColumn<Double>& interval() const {return interval_p;};
  const ROScalarQuantColumn<Double>& intervalQuant() const
  {return intervalQuant_p;};
  const ROScalarColumn<Int>& antenna1() const {return antenna1_p;};
  const ROScalarColumn<Int>& antenna2() const {return antenna2_p;};
  const ROScalarColumn<Int>& fieldId() const {return fieldId_p;};
  const ROScalarColumn<Int>& spwId() const {return spwId_p;};
  const ROScalarColumn<Int>& scanNo() const {return scanNo_p;};
  const ROScalarColumn<Int>& obsId() const {return obsId_p;};
  const ROArrayColumn<Complex>& cparam() const {return cparam_p;};
  const ROArrayColumn<Float>& fparam() const {return fparam_p;};
  const ROArrayColumn<Float>& paramerr() const {return paramerr_p;};
  const ROArrayColumn<Bool>& flag() const {return flag_p;};
  const ROArrayColumn<Float>& snr() const {return snr_p;};
  const ROArrayColumn<Float>& weight() const {return weight_p;};

  // Some additional methods to extract cparam into Array<Float>
  //   what can be: "","AP"
  Array<Float> fparamArray(String what="",const Vector<uInt>& rows=Vector<uInt>());
  void fparamArray(Array<Float>& arr,String what="",const Vector<uInt>& rows=Vector<uInt>());

  // Some additional methods to extract phase-like info
  Array<Float> phase();
  void phase(Array<Float>& ph);
  Array<Float> cycles();
  void cycles(Array<Float>& cy);
  
 protected:
  // Prohibit public use of the null constructor, which
  // does not produce a usable object.
  ROCTMainColumns() {};
  
  // Return a CalTable as a Table reference. Utilizes friendship
  // relationship with class CalTable.
  //const Table& asTable(const CalTable2& calTable) 
  //{return calTable.calMainAsTable();}
  
  // Attach a table column accessor
  void attach (const NewCalTable& calTable, ROTableColumn& tabCol, 
	       CTEnums::colDef colEnum, const Bool& optional = False);
  void attach (const NewCalTable& calTable, 
	       ROArrayMeasColumn<MEpoch>& tabCol, 
	       CTEnums::colDef colEnum, const Bool& optional = False);
  void attach (const NewCalTable& calTable, 
	       ROArrayMeasColumn<MFrequency>& tabCol, 
	       CTEnums::colDef colEnum, const Bool& optional = False);
  void attach (const NewCalTable& calTable, 
	       ROArrayMeasColumn<MDirection>& tabCol, 
	       CTEnums::colDef colEnum, const Bool& optional = False);
  void attach (const NewCalTable& calTable, ROScalarMeasColumn<MEpoch>& tabCol, 
	       CTEnums::colDef colEnum, const Bool& optional = False);
  void attach (const NewCalTable& calTable, ROScalarQuantColumn<Double>& tabCol, 
	       CTEnums::colDef colEnum, const Bool& optional = False);
  
 private:
  // Prohibit copy constructor and assignment operator 
  ROCTMainColumns (const ROCTMainColumns&);
  ROCTMainColumns& operator= (const ROCTMainColumns&);
  
  // Private column accessors
  ROScalarColumn<Double> time_p;
  ROScalarQuantColumn<Double> timeQuant_p;
  ROScalarMeasColumn<MEpoch> timeMeas_p;
  ROScalarColumn<Double> timeEP_p;
  ROScalarQuantColumn<Double> timeEPQuant_p;
  ROScalarColumn<Double> interval_p;
  ROScalarQuantColumn<Double> intervalQuant_p;
  ROScalarColumn<Int> antenna1_p;
  ROScalarColumn<Int> antenna2_p;
  ROScalarColumn<Int> fieldId_p;
  ROScalarColumn<Int> spwId_p;
  ROScalarColumn<Int> scanNo_p;
  ROScalarColumn<Int> obsId_p;
  ROArrayColumn<Complex> cparam_p;
  ROArrayColumn<Float> fparam_p;
  ROArrayColumn<Float> paramerr_p;
  ROArrayColumn<Bool> flag_p;
  ROArrayColumn<Float> snr_p;
  ROArrayColumn<Float> weight_p;
};

class CTMainColumns
{
  public:
    // Construct from a calibration table
    CTMainColumns (NewCalTable& calTable);
    
    // Default destructor
    virtual ~CTMainColumns() {};
    
    // Read-write column accessors
    ScalarColumn<Double>& time() {return time_p;};
    ScalarQuantColumn<Double>& timeQuant() {return timeQuant_p;};
    ScalarMeasColumn<MEpoch>& timeMeas() {return timeMeas_p;};
    ScalarColumn<Double>& timeEP() {return timeEP_p;};
    ScalarQuantColumn<Double>& timeEPQuant() {return timeEPQuant_p;};
    ScalarColumn<Double>& interval() {return interval_p;};
    ScalarQuantColumn<Double>& intervalQuant() {return intervalQuant_p;};
    ScalarColumn<Int>& fieldId() {return fieldId_p;};
    ScalarColumn<Int>& spwId() {return spwId_p;};
    ScalarColumn<Int>& antenna1() {return antenna1_p;};
    ScalarColumn<Int>& antenna2() {return antenna2_p;};
    ScalarColumn<Int>& scanNo() {return scanNo_p;};
    ScalarColumn<Int>& obsId() {return obsId_p;};
    ArrayColumn<Complex>& cparam() {return cparam_p;};
    ArrayColumn<Float>& fparam() {return fparam_p;};
    ArrayColumn<Float>& paramerr() {return paramerr_p;};
    ArrayColumn<Bool>& flag() {return flag_p;};
    ArrayColumn<Float>& snr() {return snr_p;};
    ArrayColumn<Float>& weight() {return weight_p;};
    
  protected:
    // Prohibit public use of the null constructor, which
    // does not produce a usable object.
    CTMainColumns() {};
    
    // Return a CalTable as a Table reference. Utilizes friendship
    // relationship with class CalTable.
    //Table& asTable(CalTable2& calTable) {return calTable.calMainAsTable();}
    
    // Attach a table column accessor
    void attach (NewCalTable& calTable, TableColumn& tabCol, 
		 CTEnums::colDef colEnum, const Bool& optional = False);
    void attach (NewCalTable& calTable, ArrayMeasColumn<MEpoch>& tabCol, 
		 CTEnums::colDef colEnum, const Bool& optional = False);
    void attach (NewCalTable& calTable, ArrayMeasColumn<MFrequency>& tabCol, 
		 CTEnums::colDef colEnum, const Bool& optional = False);
    void attach (NewCalTable& calTable, ArrayMeasColumn<MDirection>& tabCol, 
		 CTEnums::colDef colEnum, const Bool& optional = False);
    void attach (NewCalTable& calTable, ScalarMeasColumn<MEpoch>& tabCol, 
		 CTEnums::colDef colEnum, const Bool& optional = False);
    void attach (NewCalTable& calTable, ScalarQuantColumn<Double>& tabCol, 
		 CTEnums::colDef colEnum, const Bool& optional = False);
    
  private:
    // Prohibit copy constructor and assignment operator 
    CTMainColumns (const CTMainColumns&);
    CTMainColumns& operator= (const CTMainColumns&);
    
    // Private column accessors
    ScalarColumn<Double> time_p;
    ScalarQuantColumn<Double> timeQuant_p;
    ScalarMeasColumn<MEpoch> timeMeas_p;
    ScalarColumn<Double> timeEP_p;
    ScalarQuantColumn<Double> timeEPQuant_p;
    ScalarColumn<Double> interval_p;
    ScalarQuantColumn<Double> intervalQuant_p;
    ScalarColumn<Int> fieldId_p;
    ScalarColumn<Int> spwId_p;
    ScalarColumn<Int> antenna1_p;
    ScalarColumn<Int> antenna2_p;
    ScalarColumn<Int> scanNo_p;
    ScalarColumn<Int> obsId_p;
    ArrayColumn<Complex> cparam_p;
    ArrayColumn<Float> fparam_p;
    ArrayColumn<Float> paramerr_p;
    ArrayColumn<Bool> flag_p;
    ArrayColumn<Float> snr_p;
    ArrayColumn<Float> weight_p;
  };
} //# NAMESPACE CASA - END
#endif
