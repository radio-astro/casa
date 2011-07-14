//# NewCalMainColumns.h: Calibration table cal_main column access
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

#ifndef CALIBRATION_RONEWCALMAINCOLUMNS_H
#define CALIBRATION_RONEWCALMAINCOLUMNS_H

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
#include <calibration/CalTables/NewCalTable.h>
#include <calibration/CalTables/NewCalTableEnums.h>

namespace casa { //# NAMESPACE CASA - BEGIN

  class RONewCalMainColumns
  {
  public:
    // Construct from a calibration table
    RONewCalMainColumns (const NewCalTable& calTable);
    
    // Default destructor
    virtual ~RONewCalMainColumns() {};
    
    // Read-only column accessors
    const ROScalarColumn<Double>& time() const {return time_p;};
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
    const ROArrayColumn<Float>& param() const {return param_p;};
    const ROArrayColumn<Float>& paramerr() const {return paramerr_p;};
    const ROArrayColumn<Bool>& flag() const {return flag_p;};
    const ROArrayColumn<Float>& snr() const {return snr_p;};
    const ROArrayColumn<Float>& weigtht() const {return weight_p;};
    
  protected:
    // Prohibit public use of the null constructor, which
    // does not produce a usable object.
    RONewCalMainColumns() {};
    
    // Return a CalTable as a Table reference. Utilizes friendship
    // relationship with class CalTable.
    //const Table& asTable(const CalTable2& calTable) 
    //{return calTable.calMainAsTable();}
    
    // Attach a table column accessor
    void attach (const NewCalTable& calTable, ROTableColumn& tabCol, 
		 NewCalTableEnums::colDef colEnum, const Bool& optional = False);
    void attach (const NewCalTable& calTable, 
		 ROArrayMeasColumn<MEpoch>& tabCol, 
		 NewCalTableEnums::colDef colEnum, const Bool& optional = False);
    void attach (const NewCalTable& calTable, 
		 ROArrayMeasColumn<MFrequency>& tabCol, 
		 NewCalTableEnums::colDef colEnum, const Bool& optional = False);
    void attach (const NewCalTable& calTable, 
		 ROArrayMeasColumn<MDirection>& tabCol, 
		 NewCalTableEnums::colDef colEnum, const Bool& optional = False);
    void attach (const NewCalTable& calTable, ROScalarMeasColumn<MEpoch>& tabCol, 
		 NewCalTableEnums::colDef colEnum, const Bool& optional = False);
    void attach (const NewCalTable& calTable, ROScalarQuantColumn<Double>& tabCol, 
		 NewCalTableEnums::colDef colEnum, const Bool& optional = False);
    
  private:
    // Prohibit copy constructor and assignment operator 
    RONewCalMainColumns (const RONewCalMainColumns&);
    RONewCalMainColumns& operator= (const RONewCalMainColumns&);
    
    // Private column accessors
    ROScalarColumn<Double> time_p;
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
    ROArrayColumn<Float> param_p;
    ROArrayColumn<Float> paramerr_p;
    ROArrayColumn<Bool> flag_p;
    ROArrayColumn<Float> snr_p;
    ROArrayColumn<Float> weight_p;
  };
  
}


#endif
