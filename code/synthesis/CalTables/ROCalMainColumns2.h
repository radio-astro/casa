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
#include <msvis/MSVis/MSCalEnums.h>

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
    const casacore::ROScalarColumn<casacore::Double>& time() const {return time_p;};
    const casacore::ROScalarMeasColumn<casacore::MEpoch>& timeMeas() const {return timeMeas_p;};
    const casacore::ROScalarColumn<casacore::Double>& timeEP() const {return timeEP_p;};
    const casacore::ROScalarQuantColumn<casacore::Double>& timeEPQuant() const
    {return timeEPQuant_p;};
    const casacore::ROScalarColumn<casacore::Double>& interval() const {return interval_p;};
    const casacore::ROScalarQuantColumn<casacore::Double>& intervalQuant() const
    {return intervalQuant_p;};
    const casacore::ROScalarColumn<casacore::Int>& antenna1() const {return antenna1_p;};
    const casacore::ROScalarColumn<casacore::Int>& feed1() const {return feed1_p;};
    const casacore::ROScalarColumn<casacore::Int>& fieldId() const {return fieldId_p;};
    const casacore::ROScalarColumn<casacore::Int>& arrayId() const {return arrayId_p;};
    const casacore::ROScalarColumn<casacore::Int>& obsId() const {return obsId_p;};
    const casacore::ROScalarColumn<casacore::Int>& scanNo() const {return scanNo_p;};
    const casacore::ROScalarColumn<casacore::Int>& processorId() const {return processorId_p;};
    const casacore::ROScalarColumn<casacore::Int>& stateId() const {return stateId_p;};
    const casacore::ROScalarColumn<casacore::Int>& phaseId() const {return phaseId_p;};
    const casacore::ROScalarColumn<casacore::Int>& pulsarBin() const {return pulsarBin_p;};
    const casacore::ROScalarColumn<casacore::Int>& pulsarGateId() const {return pulsarGateId_p;};
    const casacore::ROScalarColumn<casacore::Int>& freqGrp() const {return freqGrp_p;};
    const casacore::ROScalarColumn<casacore::String>& freqGrpName() const {return freqGrpName_p;};
    const casacore::ROScalarColumn<casacore::String>& fieldName() const {return fieldName_p;};
    const casacore::ROScalarColumn<casacore::String>& fieldCode() const {return fieldCode_p;};
    const casacore::ROScalarColumn<casacore::String>& sourceName() const {return sourceName_p;};
    const casacore::ROScalarColumn<casacore::String>& sourceCode() const {return sourceCode_p;};
    const casacore::ROScalarColumn<casacore::Int>& calGrp() const {return calGrp_p;};
    //const casacore::ROArrayColumn<casacore::Complex>& gain() const {return gain_p;};
    const casacore::ROArrayColumn<T>& gain() const {return gain_p;};
    const casacore::ROArrayColumn<casacore::Float>& solvePar() const {return solvePar_p;};
    const casacore::ROArrayColumn<casacore::Int>& refAnt() const {return refAnt_p;};
    const casacore::ROArrayColumn<casacore::Int>& refFeed() const {return refFeed_p;};
    const casacore::ROArrayColumn<casacore::Int>& refReceptor() const {return refReceptor_p;};
    const casacore::ROArrayColumn<casacore::Double>& refFreq() const {return refFreq_p;};
    const casacore::ROArrayMeasColumn<casacore::MFrequency>& refFreqMeas() const
    {return refFreqMeas_p;};
    const casacore::ROScalarColumn<casacore::Int>& measFreqRef() const {return measFreqRef_p;};
    const casacore::ROArrayColumn<casacore::Double>& refDir() const {return refDir_p;};
    const casacore::ROArrayMeasColumn<casacore::MDirection>& refDirMeas() const
    {return refDirMeas_p;};
    const casacore::ROScalarColumn<casacore::Int>& measDirRef() const {return measDirRef_p;};
    const casacore::ROScalarColumn<casacore::Int>& calDescId() const {return calDescId_p;};
    const casacore::ROScalarColumn<casacore::Int>& calHistoryId() const {return calHistoryId_p;};
    
  protected:
    // Prohibit public use of the null constructor, which
    // does not produce a usable object.
    ROCalMainColumns2() {};
    
    // Return a CalTable as a casacore::Table reference. Utilizes friendship
    // relationship with class CalTable.
    const casacore::Table& asTable(const CalTable2& calTable) 
    {return calTable.calMainAsTable();}
    
    // Attach a table column accessor
    void attach (const CalTable2& calTable, casacore::ROTableColumn& tabCol, 
		 MSCalEnums::colDef colEnum, const casacore::Bool& optional = false);
    void attach (const CalTable2& calTable, 
		 casacore::ROArrayMeasColumn<casacore::MEpoch>& tabCol,
		 MSCalEnums::colDef colEnum, const casacore::Bool& optional = false);
    void attach (const CalTable2& calTable, 
		 casacore::ROArrayMeasColumn<casacore::MFrequency>& tabCol,
		 MSCalEnums::colDef colEnum, const casacore::Bool& optional = false);
    void attach (const CalTable2& calTable, 
		 casacore::ROArrayMeasColumn<casacore::MDirection>& tabCol,
		 MSCalEnums::colDef colEnum, const casacore::Bool& optional = false);
    void attach (const CalTable2& calTable, casacore::ROScalarMeasColumn<casacore::MEpoch>& tabCol,
		 MSCalEnums::colDef colEnum, const casacore::Bool& optional = false);
    void attach (const CalTable2& calTable, casacore::ROScalarQuantColumn<casacore::Double>& tabCol,
		 MSCalEnums::colDef colEnum, const casacore::Bool& optional = false);
    
  private:
    // Prohibit copy constructor and assignment operator 
    ROCalMainColumns2 (const ROCalMainColumns2&);
    ROCalMainColumns2& operator= (const ROCalMainColumns2&);
    
    // Private column accessors
    casacore::ROScalarColumn<casacore::Double> time_p;
    casacore::ROScalarMeasColumn<casacore::MEpoch> timeMeas_p;
    casacore::ROScalarColumn<casacore::Double> timeEP_p;
    casacore::ROScalarQuantColumn<casacore::Double> timeEPQuant_p;
    casacore::ROScalarColumn<casacore::Double> interval_p;
    casacore::ROScalarQuantColumn<casacore::Double> intervalQuant_p;
    casacore::ROScalarColumn<casacore::Int> antenna1_p;
    casacore::ROScalarColumn<casacore::Int> feed1_p;
    casacore::ROScalarColumn<casacore::Int> fieldId_p;
    casacore::ROScalarColumn<casacore::Int> arrayId_p;
    casacore::ROScalarColumn<casacore::Int> obsId_p;
    casacore::ROScalarColumn<casacore::Int> scanNo_p;
    casacore::ROScalarColumn<casacore::Int> processorId_p;
    casacore::ROScalarColumn<casacore::Int> stateId_p;
    casacore::ROScalarColumn<casacore::Int> phaseId_p;
    casacore::ROScalarColumn<casacore::Int> pulsarBin_p;
    casacore::ROScalarColumn<casacore::Int> pulsarGateId_p;
    casacore::ROScalarColumn<casacore::Int> freqGrp_p;
    casacore::ROScalarColumn<casacore::String> freqGrpName_p;
    casacore::ROScalarColumn<casacore::String> fieldName_p;
    casacore::ROScalarColumn<casacore::String> fieldCode_p;
    casacore::ROScalarColumn<casacore::String> sourceName_p;
    casacore::ROScalarColumn<casacore::String> sourceCode_p;
    casacore::ROScalarColumn<casacore::Int> calGrp_p;
    //casacore::ROArrayColumn<casacore::Complex> gain_p;
    casacore::ROArrayColumn<T> gain_p;
    casacore::ROArrayColumn<casacore::Float> solvePar_p;
    casacore::ROArrayColumn<casacore::Int> refAnt_p;
    casacore::ROArrayColumn<casacore::Int> refFeed_p;
    casacore::ROArrayColumn<casacore::Int> refReceptor_p;
    casacore::ROArrayColumn<casacore::Double> refFreq_p;
    casacore::ROArrayMeasColumn<casacore::MFrequency> refFreqMeas_p;
    casacore::ROScalarColumn<casacore::Int> measFreqRef_p;
    casacore::ROArrayColumn<casacore::Double> refDir_p;
    casacore::ROArrayMeasColumn<casacore::MDirection> refDirMeas_p;
    casacore::ROScalarColumn<casacore::Int> measDirRef_p;
    casacore::ROScalarColumn<casacore::Int> calDescId_p;
    casacore::ROScalarColumn<casacore::Int> calHistoryId_p;
  };
  
}


#ifndef AIPS_NO_TEMPLATE_SRC
#include <synthesis/CalTables/ROCalMainColumns2.tcc>
#endif

#endif
