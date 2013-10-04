//# CalMainColumns.h: Calibration table cal_main column access
//# Copyright (C) 1996,1997,1998,2001,2002,2003
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

#ifndef CALIBRATION_CALMAINCOLUMNS2_H
#define CALIBRATION_CALMAINCOLUMNS2_H

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
#include <synthesis/CalTables/CalTable2.h>
#include <synthesis/MSVis/MSCalEnums.h>

namespace casa { //# NAMESPACE CASA - BEGIN

template<class T> 
class CalMainColumns2 {
  public:
    // Construct from a calibration table
    CalMainColumns2 (CalTable2& calTable);
    
    // Default destructor
    virtual ~CalMainColumns2() {};
    
    // Read-write column accessors
    ScalarColumn<Double>& time() {return time_p;};
    ScalarMeasColumn<MEpoch>& timeMeas() {return timeMeas_p;};
    ScalarColumn<Double>& timeEP() {return timeEP_p;};
    ScalarQuantColumn<Double>& timeEPQuant() {return timeEPQuant_p;};
    ScalarColumn<Double>& interval() {return interval_p;};
    ScalarQuantColumn<Double>& intervalQuant() {return intervalQuant_p;};
    ScalarColumn<Int>& antenna1() {return antenna1_p;};
    ScalarColumn<Int>& feed1() {return feed1_p;};
    ScalarColumn<Int>& fieldId() {return fieldId_p;};
    ScalarColumn<Int>& arrayId() {return arrayId_p;};
    ScalarColumn<Int>& obsId() {return obsId_p;};
    ScalarColumn<Int>& scanNo() {return scanNo_p;};
    ScalarColumn<Int>& processorId() {return processorId_p;};
    ScalarColumn<Int>& stateId() {return stateId_p;};
    ScalarColumn<Int>& phaseId() {return phaseId_p;};
    ScalarColumn<Int>& pulsarBin() {return pulsarBin_p;};
    ScalarColumn<Int>& pulsarGateId() {return pulsarGateId_p;};
    ScalarColumn<Int>& freqGrp() {return freqGrp_p;};
    ScalarColumn<String>& freqGrpName() {return freqGrpName_p;};
    ScalarColumn<String>& fieldName() {return fieldName_p;};
    ScalarColumn<String>& fieldCode() {return fieldCode_p;};
    ScalarColumn<String>& sourceName() {return sourceName_p;};
    ScalarColumn<String>& sourceCode() {return sourceCode_p;};
    ScalarColumn<Int>& calGrp() {return calGrp_p;};
    //ArrayColumn<Complex>& gain() {return gain_p;};
    ArrayColumn<T>& gain() {return gain_p;};
    ArrayColumn<T>& solvePar() {return solvePar_p;};
    ArrayColumn<Int>& refAnt() {return refAnt_p;};
    ArrayColumn<Int>& refFeed() {return refFeed_p;};
    ArrayColumn<Int>& refReceptor() {return refReceptor_p;};
    ArrayColumn<Double>& refFreq() {return refFreq_p;};
    ArrayMeasColumn<MFrequency>& refFreqMeas() {return refFreqMeas_p;};
    ScalarColumn<Int>& measFreqRef() {return measFreqRef_p;};
    ArrayColumn<Double>& refDir() {return refDir_p;};
    ArrayMeasColumn<MDirection>& refDirMeas() {return refDirMeas_p;};
    ScalarColumn<Int>& measDirRef() {return measDirRef_p;};
    ScalarColumn<Int>& calDescId() {return calDescId_p;};
    ScalarColumn<Int>& calHistoryId() {return calHistoryId_p;}; 
    
  protected:
    // Prohibit public use of the null constructor, which
    // does not produce a usable object.
    CalMainColumns2() {};
    
    // Return a CalTable as a Table reference. Utilizes friendship
    // relationship with class CalTable.
    Table& asTable(CalTable2& calTable) {return calTable.calMainAsTable();}
    
    // Attach a table column accessor
    void attach (CalTable2& calTable, TableColumn& tabCol, 
		 MSCalEnums::colDef colEnum, const Bool& optional = False);
    void attach (CalTable2& calTable, ArrayMeasColumn<MEpoch>& tabCol, 
		 MSCalEnums::colDef colEnum, const Bool& optional = False);
    void attach (CalTable2& calTable, ArrayMeasColumn<MFrequency>& tabCol, 
		 MSCalEnums::colDef colEnum, const Bool& optional = False);
    void attach (CalTable2& calTable, ArrayMeasColumn<MDirection>& tabCol, 
		 MSCalEnums::colDef colEnum, const Bool& optional = False);
    void attach (CalTable2& calTable, ScalarMeasColumn<MEpoch>& tabCol, 
		 MSCalEnums::colDef colEnum, const Bool& optional = False);
    void attach (CalTable2& calTable, ScalarQuantColumn<Double>& tabCol, 
		 MSCalEnums::colDef colEnum, const Bool& optional = False);
    
  private:
    // Prohibit copy constructor and assignment operator 
    CalMainColumns2 (const CalMainColumns2&);
    CalMainColumns2& operator= (const CalMainColumns2&);
    
    // Private column accessors
    ScalarColumn<Double> time_p;
    ScalarMeasColumn<MEpoch> timeMeas_p;
    ScalarColumn<Double> timeEP_p;
    ScalarQuantColumn<Double> timeEPQuant_p;
    ScalarColumn<Double> interval_p;
    ScalarQuantColumn<Double> intervalQuant_p;
    ScalarColumn<Int> antenna1_p;
    ScalarColumn<Int> feed1_p;
    ScalarColumn<Int> fieldId_p;
    ScalarColumn<Int> arrayId_p;
    ScalarColumn<Int> obsId_p;
    ScalarColumn<Int> scanNo_p;
    ScalarColumn<Int> processorId_p;
    ScalarColumn<Int> stateId_p;
    ScalarColumn<Int> phaseId_p;
    ScalarColumn<Int> pulsarBin_p;
    ScalarColumn<Int> pulsarGateId_p;
    ScalarColumn<Int> freqGrp_p;
    ScalarColumn<String> freqGrpName_p;
    ScalarColumn<String> fieldName_p;
    ScalarColumn<String> fieldCode_p;
    ScalarColumn<String> sourceName_p;
    ScalarColumn<String> sourceCode_p;
    ScalarColumn<Int> calGrp_p;
    //ArrayColumn<Complex> gain_p;
    ArrayColumn<T> gain_p;
    ArrayColumn<T> solvePar_p;
    ArrayColumn<Int> refAnt_p;
    ArrayColumn<Int> refFeed_p;
    ArrayColumn<Int> refReceptor_p;
    ArrayColumn<Double> refFreq_p;
    ArrayMeasColumn<MFrequency> refFreqMeas_p;
    ScalarColumn<Int> measFreqRef_p;
    ArrayColumn<Double> refDir_p;
    ArrayMeasColumn<MDirection> refDirMeas_p;
    ScalarColumn<Int> measDirRef_p;
    ScalarColumn<Int> calDescId_p;
    ScalarColumn<Int> calHistoryId_p;
  };
} //# NAMESPACE CASA - END

#ifndef AIPS_NO_TEMPLATE_SRC
#include <synthesis/CalTables/CalMainColumns2.tcc>
#endif

#endif
