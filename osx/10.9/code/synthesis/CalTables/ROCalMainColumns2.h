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

#ifndef CALIBRATION_ROCALMAINCOLUMNS2_H
#define CALIBRATION_ROCALMAINCOLUMNS2_H

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
  class ROCalMainColumns2
  {
  public:
    // Construct from a calibration table
    ROCalMainColumns2 (const CalTable2& calTable);
    
    // Default destructor
    virtual ~ROCalMainColumns2() {};
    
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
    const ROScalarColumn<Int>& feed1() const {return feed1_p;};
    const ROScalarColumn<Int>& fieldId() const {return fieldId_p;};
    const ROScalarColumn<Int>& arrayId() const {return arrayId_p;};
    const ROScalarColumn<Int>& obsId() const {return obsId_p;};
    const ROScalarColumn<Int>& scanNo() const {return scanNo_p;};
    const ROScalarColumn<Int>& processorId() const {return processorId_p;};
    const ROScalarColumn<Int>& stateId() const {return stateId_p;};
    const ROScalarColumn<Int>& phaseId() const {return phaseId_p;};
    const ROScalarColumn<Int>& pulsarBin() const {return pulsarBin_p;};
    const ROScalarColumn<Int>& pulsarGateId() const {return pulsarGateId_p;};
    const ROScalarColumn<Int>& freqGrp() const {return freqGrp_p;};
    const ROScalarColumn<String>& freqGrpName() const {return freqGrpName_p;};
    const ROScalarColumn<String>& fieldName() const {return fieldName_p;};
    const ROScalarColumn<String>& fieldCode() const {return fieldCode_p;};
    const ROScalarColumn<String>& sourceName() const {return sourceName_p;};
    const ROScalarColumn<String>& sourceCode() const {return sourceCode_p;};
    const ROScalarColumn<Int>& calGrp() const {return calGrp_p;};
    //const ROArrayColumn<Complex>& gain() const {return gain_p;};
    const ROArrayColumn<T>& gain() const {return gain_p;};
    const ROArrayColumn<Float>& solvePar() const {return solvePar_p;};
    const ROArrayColumn<Int>& refAnt() const {return refAnt_p;};
    const ROArrayColumn<Int>& refFeed() const {return refFeed_p;};
    const ROArrayColumn<Int>& refReceptor() const {return refReceptor_p;};
    const ROArrayColumn<Double>& refFreq() const {return refFreq_p;};
    const ROArrayMeasColumn<MFrequency>& refFreqMeas() const 
    {return refFreqMeas_p;};
    const ROScalarColumn<Int>& measFreqRef() const {return measFreqRef_p;};
    const ROArrayColumn<Double>& refDir() const {return refDir_p;};
    const ROArrayMeasColumn<MDirection>& refDirMeas() const 
    {return refDirMeas_p;};
    const ROScalarColumn<Int>& measDirRef() const {return measDirRef_p;};
    const ROScalarColumn<Int>& calDescId() const {return calDescId_p;};
    const ROScalarColumn<Int>& calHistoryId() const {return calHistoryId_p;};
    
  protected:
    // Prohibit public use of the null constructor, which
    // does not produce a usable object.
    ROCalMainColumns2() {};
    
    // Return a CalTable as a Table reference. Utilizes friendship
    // relationship with class CalTable.
    const Table& asTable(const CalTable2& calTable) 
    {return calTable.calMainAsTable();}
    
    // Attach a table column accessor
    void attach (const CalTable2& calTable, ROTableColumn& tabCol, 
		 MSCalEnums::colDef colEnum, const Bool& optional = False);
    void attach (const CalTable2& calTable, 
		 ROArrayMeasColumn<MEpoch>& tabCol, 
		 MSCalEnums::colDef colEnum, const Bool& optional = False);
    void attach (const CalTable2& calTable, 
		 ROArrayMeasColumn<MFrequency>& tabCol, 
		 MSCalEnums::colDef colEnum, const Bool& optional = False);
    void attach (const CalTable2& calTable, 
		 ROArrayMeasColumn<MDirection>& tabCol, 
		 MSCalEnums::colDef colEnum, const Bool& optional = False);
    void attach (const CalTable2& calTable, ROScalarMeasColumn<MEpoch>& tabCol, 
		 MSCalEnums::colDef colEnum, const Bool& optional = False);
    void attach (const CalTable2& calTable, ROScalarQuantColumn<Double>& tabCol, 
		 MSCalEnums::colDef colEnum, const Bool& optional = False);
    
  private:
    // Prohibit copy constructor and assignment operator 
    ROCalMainColumns2 (const ROCalMainColumns2&);
    ROCalMainColumns2& operator= (const ROCalMainColumns2&);
    
    // Private column accessors
    ROScalarColumn<Double> time_p;
    ROScalarMeasColumn<MEpoch> timeMeas_p;
    ROScalarColumn<Double> timeEP_p;
    ROScalarQuantColumn<Double> timeEPQuant_p;
    ROScalarColumn<Double> interval_p;
    ROScalarQuantColumn<Double> intervalQuant_p;
    ROScalarColumn<Int> antenna1_p;
    ROScalarColumn<Int> feed1_p;
    ROScalarColumn<Int> fieldId_p;
    ROScalarColumn<Int> arrayId_p;
    ROScalarColumn<Int> obsId_p;
    ROScalarColumn<Int> scanNo_p;
    ROScalarColumn<Int> processorId_p;
    ROScalarColumn<Int> stateId_p;
    ROScalarColumn<Int> phaseId_p;
    ROScalarColumn<Int> pulsarBin_p;
    ROScalarColumn<Int> pulsarGateId_p;
    ROScalarColumn<Int> freqGrp_p;
    ROScalarColumn<String> freqGrpName_p;
    ROScalarColumn<String> fieldName_p;
    ROScalarColumn<String> fieldCode_p;
    ROScalarColumn<String> sourceName_p;
    ROScalarColumn<String> sourceCode_p;
    ROScalarColumn<Int> calGrp_p;
    //ROArrayColumn<Complex> gain_p;
    ROArrayColumn<T> gain_p;
    ROArrayColumn<Float> solvePar_p;
    ROArrayColumn<Int> refAnt_p;
    ROArrayColumn<Int> refFeed_p;
    ROArrayColumn<Int> refReceptor_p;
    ROArrayColumn<Double> refFreq_p;
    ROArrayMeasColumn<MFrequency> refFreqMeas_p;
    ROScalarColumn<Int> measFreqRef_p;
    ROArrayColumn<Double> refDir_p;
    ROArrayMeasColumn<MDirection> refDirMeas_p;
    ROScalarColumn<Int> measDirRef_p;
    ROScalarColumn<Int> calDescId_p;
    ROScalarColumn<Int> calHistoryId_p;
  };
  
}


#ifndef AIPS_NO_TEMPLATE_SRC
#include <synthesis/CalTables/ROCalMainColumns2.tcc>
#endif

#endif
