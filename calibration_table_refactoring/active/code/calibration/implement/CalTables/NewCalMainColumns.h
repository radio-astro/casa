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
#include <calibration/CalTables/NewCalTable.h>
#include <calibration/CalTables/NewCalTableEnums.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class NewCalMainColumns {
  public:
    // Construct from a calibration table
    NewCalMainColumns (NewCalTable& calTable);
    
    // Default destructor
    virtual ~NewCalMainColumns() {};
    
    // Read-write column accessors
    ScalarColumn<Double>& time() {return time_p;};
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
    ArrayColumn<Float>& param() {return param_p;};
    ArrayColumn<Float>& paramerr() {return paramerr_p;};
    ArrayColumn<Bool>& flag() {return flag_p;};
    ArrayColumn<Float>& snr() {return snr_p;};
    ArrayColumn<Float>& weight() {return weight_p;};
    
  protected:
    // Prohibit public use of the null constructor, which
    // does not produce a usable object.
    NewCalMainColumns() {};
    
    // Return a CalTable as a Table reference. Utilizes friendship
    // relationship with class CalTable.
    //Table& asTable(CalTable2& calTable) {return calTable.calMainAsTable();}
    
    // Attach a table column accessor
    void attach (NewCalTable& calTable, TableColumn& tabCol, 
		 NewCalTableEnums::colDef colEnum, const Bool& optional = False);
    void attach (NewCalTable& calTable, ArrayMeasColumn<MEpoch>& tabCol, 
		 NewCalTableEnums::colDef colEnum, const Bool& optional = False);
    void attach (NewCalTable& calTable, ArrayMeasColumn<MFrequency>& tabCol, 
		 NewCalTableEnums::colDef colEnum, const Bool& optional = False);
    void attach (NewCalTable& calTable, ArrayMeasColumn<MDirection>& tabCol, 
		 NewCalTableEnums::colDef colEnum, const Bool& optional = False);
    void attach (NewCalTable& calTable, ScalarMeasColumn<MEpoch>& tabCol, 
		 NewCalTableEnums::colDef colEnum, const Bool& optional = False);
    void attach (NewCalTable& calTable, ScalarQuantColumn<Double>& tabCol, 
		 NewCalTableEnums::colDef colEnum, const Bool& optional = False);
    
  private:
    // Prohibit copy constructor and assignment operator 
    NewCalMainColumns (const NewCalMainColumns&);
    NewCalMainColumns& operator= (const NewCalMainColumns&);
    
    // Private column accessors
    ScalarColumn<Double> time_p;
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
    ArrayColumn<Float> param_p;
    ArrayColumn<Float> paramerr_p;
    ArrayColumn<Bool> flag_p;
    ArrayColumn<Float> snr_p;
    ArrayColumn<Float> weight_p;
  };
} //# NAMESPACE CASA - END

#endif
