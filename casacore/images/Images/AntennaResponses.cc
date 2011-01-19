//# AntennaResponses.h: AntennaResponses provides access to antenna response data
//# Copyright (C) 1995-1999,2000-2004
//# Associated Universities, Inc. Washington DC, USA
//# Copyright by ESO (in the framework of the ALMA collaboration)
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
//# Correspondence concerning CASA should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: CASA Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id: $

//# Includes
#include <casa/aips.h>
#include <images/Images/AntennaResponses.h>
#include <casa/Arrays/Vector.h>
#include <casa/Exceptions/Error.h>
#include <measures/TableMeasures/ScalarMeasColumn.h>
#include <measures/TableMeasures/ArrayMeasColumn.h>
#include <measures/TableMeasures/TableMeasValueDesc.h>
#include <measures/TableMeasures/TableMeasOffsetDesc.h>
#include <measures/TableMeasures/TableMeasRefDesc.h>
#include <measures/TableMeasures/TableMeasDesc.h>
#include <measures/TableMeasures/ArrayQuantColumn.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MFrequency.h>
#include <measures/Measures/MCEpoch.h>
#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MCPosition.h>
#include <measures/Measures/MeasTable.h>
#include <measures/Measures/MeasData.h>
#include <measures/Measures/MeasRef.h>
#include <measures/Measures/MeasFrame.h>
#include <measures/Measures/MeasConvert.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Quanta/QLogical.h>
#include <casa/Quanta/Quantum.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ColumnDesc.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/ScalarColumn.h>
#include <casa/OS/Time.h>
#include <casa/System/AipsrcValue.h>
#include <casa/BasicSL/String.h>
#include <casa/iostream.h>

namespace casa { //# NAMESPACE CASA - BEGIN

  AntennaResponses::AntennaResponses(const String& path){
    init(path);
  }

  Bool AntennaResponses::init(const String& path){
    // reset members to empty

    paths_p.resize(); 

    numRows_p = 0;
    ObsName_p.resize();
    StartTime_p.resize();
    AntennaType_p.resize();
    ReceiverType_p.resize();
    BeamId_p.resize();
    BeamNumber_p.resize();
    ValidCenter_p.resize();
    ValidCenterMin_p.resize();
    ValidCenterMax_p.resize();
    NumBands_p.resize();
    BandName_p.resize();
    BandMinFreq_p.resize();
    BandMaxFreq_p.resize();
    FuncType_p.resize();
    FuncName_p.resize();
    FuncChannel_p.resize();
    NomFreq_p.resize();

    if(path==""){
      return True;
    }
    else{
      // fill members from table on disk
      return append(path);
    }
  }


  Bool AntennaResponses::append(const String& path){

    if(isInit(path)){
      // Returns False if the path was already read before.
      return False;
    }
    
    // open table
    Table tab(path);    

    // get number of rows
    uInt numRows = tab.nrow();

    if(numRows>0){
      // read columns and append to vectors;

      ROScalarColumn<Int> beamIdCol(tab, "BEAM_ID");
      ROScalarColumn<String> nameCol(tab, "NAME");
      ROScalarColumn<Int> beamNumberCol(tab, "BEAM_NUMBER");
      ROScalarMeasColumn<MEpoch> startTimeCol(tab, "START_TIME");
      ROScalarColumn<String> antennaTypeCol(tab, "ANTENNA_TYPE");
      ROScalarColumn<String> receiverTypeCol(tab, "RECEIVER_TYPE");
      ROScalarColumn<Int> numBandsCol(tab, "NUM_BANDS");
      ROArrayColumn<String> bandNameCol(tab, "BAND_NAME");
      ROArrayQuantColumn<Double> bandMinFreqCol(tab, "BAND_MIN_FREQ");
      ROArrayQuantColumn<Double> bandMaxFreqCol(tab, "BAND_MAX_FREQ");
      ROScalarMeasColumn<MDirection> centerCol(tab, "CENTER");
      ROScalarMeasColumn<MDirection> validCenterMinCol(tab, "VALID_CENTER_MIN");
      ROScalarMeasColumn<MDirection> validCenterMaxCol(tab, "VALID_CENTER_MAX");
      ROArrayColumn<Int> functionTypeCol(tab, "FUNCTION_TYPE");   
      ROArrayColumn<String> functionNameCol(tab, "FUNCTION_NAME");
      ROArrayColumn<uInt> functionChannelCol(tab, "FUNCTION_CHANNEL");
      ROArrayQuantColumn<Double> nomFreqCol(tab, "NOMINAL_FREQ");

      numRows_p += numRows;

      ObsName_p.resize(numRows_p,True);
      StartTime_p.resize(numRows_p,True);
      AntennaType_p.resize(numRows_p,True);
      ReceiverType_p.resize(numRows_p,True);
      BeamId_p.resize(numRows_p,True);
      BeamNumber_p.resize(numRows_p,True);
      ValidCenter_p.resize(numRows_p,True);
      ValidCenterMin_p.resize(numRows_p,True);
      ValidCenterMax_p.resize(numRows_p,True);
      NumBands_p.resize(numRows_p,True);
      BandName_p.resize(numRows_p,True);
      BandMinFreq_p.resize(numRows_p,True);
      BandMaxFreq_p.resize(numRows_p,True);
      FuncType_p.resize(numRows_p,True);
      FuncName_p.resize(numRows_p,True);
      FuncChannel_p.resize(numRows_p,True);
      NomFreq_p.resize(numRows_p,True);


      for(uInt i=0; i<numRows; i++){
	uInt j = i + numRows_p - numRows;
	BeamId_p(j) = beamIdCol(i);
	ObsName_p(j) = nameCol(i);
	BeamNumber_p(j) = beamNumberCol(i);
	StartTime_p(j) = startTimeCol(i);
	AntennaType_p(j) = antennaTypeCol(i);
	ReceiverType_p(j) = receiverTypeCol(i);
	NumBands_p(j) = numBandsCol(i);
	BandName_p(j).assign(bandNameCol(i));

	Vector<Quantity> tQ;
	tQ = bandMinFreqCol(i);
	for(uInt k=0; k<tQ.nelements(); k++){
	  BandMinFreq_p(j)(k) = MVFrequency(tQ(k));
	}
	tQ = bandMaxFreqCol(i);
	for(uInt k=0; k<tQ.nelements(); k++){
	  BandMaxFreq_p(j)(k) = MVFrequency(tQ(k));
	}
	tQ = nomFreqCol(i);
	for(uInt k=0; k<tQ.nelements(); k++){
	  NomFreq_p(j)(k) = MVFrequency(tQ(k));
	}

	ValidCenter_p(j) = centerCol(i);
	ValidCenterMin_p(j) = validCenterMinCol(i);
	ValidCenterMax_p(j) = validCenterMaxCol(i);

	Vector<Int> tFType;
	tFType = functionTypeCol(i);
	for(uInt k=0; k<tFType.nelements(); k++){
	  FuncType_p(j)(k) = FuncType(tFType(k));
	}

	FuncName_p(j).assign(functionNameCol(i));
	FuncChannel_p(j).assign(functionChannelCol(i));
      }

    } // end if

    uInt nPaths = paths_p.nelements();
    paths_p.resize(nPaths+1, True);
    paths_p(nPaths) = path;

    return True;

  }

  Bool AntennaResponses::isInit(const String& path){
    Bool found = False;
    for (uInt i=0; i<paths_p.nelements(); i++){
      if(paths_p(i) == path){
	found = True;
      }
    }
    return found;
  }

  // find the row containing the information pertinent to the given parameters
  // (this is also the index in the member vectors)
  // return false if no matching row could be found
  Bool AntennaResponses::getRow(uInt& row,
				const String& obsName,
				const MEpoch& obsTime,
				const MFrequency& freq,
				const FuncTypes& requFType,
				const String& antennaType,
				const MDirection& center,
				const String& receiverType,
				const Int& beamNumber){
  
    // loop over rows

    // compare all elements of the key

    // if found, set row to the found one and return True

    // else return False
    return False;

  }

  // overloaded method: as previous method but using beamId
  // (instead of obs. time, ant. type,  rec. type, center, and beam number)
  Bool AntennaResponses::getRow(uInt& row,
				const String& obsName,
				const Int& beamId,
				const MFrequency& freq){
  
    // loop over rows

    // compare all elements of the key

    // if found, set row to the found one and return True

    // else return False
    return False;

  }


  Bool AntennaResponses::getImageName(String& functionImageName, // the path to the image
				      uInt& functionChannel, // the channel to use
				      MFrequency& nomFreq, // nominal frequency of the image (at the given channel)
				      FuncTypes& fType, // the function type of the image
				      const String& obsName, // (the observatory name, e.g. "ALMA" or "ACA")
				      const MEpoch& obsTime,
				      const MFrequency& freq,
				      const FuncTypes& requFType, // the requested function type
				      const String& antennaType,
				      const MDirection& center,
				      const String& receiverType,
				      const Int& beamNumber){
    return False;
  }
		
  // overloaded method: as previous method but using beamId
  // (instead of obs. time, ant. type,  rec. type, and center)
  Bool AntennaResponses::getImageName(String& functionImageName,
				      uInt& functionChannel,
				      MFrequency& nomFreq, // nominal frequency of the image
				      FuncTypes& fType, // the function type of the image
				      const String& obsName, // (the observatory name, e.g. "ALMA" or "ACA")
				      const Int& beamId,
				      const MFrequency& freq){
    return False;
  }


  Bool AntennaResponses::putRow(uInt& row,
				const String& obsName,
				const Int& beamId,
				const Vector<String>& bandName,
				const Vector<MVFrequency>& bandMinFreq,
				const Vector<MVFrequency>& bandMaxFreq,
				const Vector<FuncTypes>& funcType,
				const Vector<String>& funcName,
				const Vector<uInt>& funcChannel,
				const Vector<MVFrequency>& nomFreq,
				const String& antennaType,
				const MEpoch& startTime,
				const MDirection& center,
				const MDirection& validCenterMin,
				const MDirection& validCenterMax,
				const String& receiverType,
				const Int& beamNumber){
    // Put the given row into the present antenna reponses table (in memory).

    // Returns false, if the table was not initialised or the given data was
    // not consistent.
    if(paths_p.nelements()==0){
      return False;
    }
    // Consistency checks: 
    //   - all vectors have same dimension which is then used to set numBands
    uInt tNumBands = bandName.nelements();
    if(!(
	 tNumBands == bandMinFreq.nelements() &&
	 tNumBands == bandMaxFreq.nelements() &&
	 tNumBands == funcType.nelements() &&
	 tNumBands == funcName.nelements() &&
	 tNumBands == funcChannel.nelements() &&
	 tNumBands == nomFreq.nelements())
       ){
      return False;
    }
    //   - beamId is unique for the given observatory
    Bool isUnique = True;
    for(uInt i=0; i<numRows_p; i++){
      if(ObsName_p(i)==obsName && BeamId_p(i)==beamId && row!=i){
	isUnique = False;
	break;
      }
    }
    if(!isUnique){
      return False;
    }
    //   - center, validCenterMin, and validCenterMax have the same MDirection type
    String dirRef = center.getRefString();
    if(!(dirRef == validCenterMin.getRefString() &&
	 dirRef == validCenterMax.getRefString())
       ){
	 return False;
    }
    
    uInt theRow = 0;

    // If the row exists at the position given by uInt row, it is overwritten.
    if(row<numRows_p){
      theRow = row;
    }
    else{ // If it doesn't exist, the table is resized by one in memory and the new
      // row is added at the last position. The variable "row" then contains the
      // actual row that was filled.
      theRow = row = numRows_p;
      numRows_p++;
      ObsName_p.resize(numRows_p,True);
      StartTime_p.resize(numRows_p,True);
      AntennaType_p.resize(numRows_p,True);
      ReceiverType_p.resize(numRows_p,True);
      BeamId_p.resize(numRows_p,True);
      BeamNumber_p.resize(numRows_p,True);
      ValidCenter_p.resize(numRows_p,True);
      ValidCenterMin_p.resize(numRows_p,True);
      ValidCenterMax_p.resize(numRows_p,True);
      NumBands_p.resize(numRows_p,True);
      BandName_p.resize(numRows_p,True);
      BandMinFreq_p.resize(numRows_p,True);
      BandMaxFreq_p.resize(numRows_p,True);
      FuncType_p.resize(numRows_p,True);
      FuncName_p.resize(numRows_p,True);
      FuncChannel_p.resize(numRows_p,True);
      NomFreq_p.resize(numRows_p,True);
    }  

    ObsName_p(theRow) = obsName;
    StartTime_p(theRow) = startTime;
    AntennaType_p(theRow) = antennaType;
    ReceiverType_p(theRow) = receiverType;
    BeamId_p(theRow) = beamId;
    BeamNumber_p(theRow) = beamNumber;
    ValidCenter_p(theRow) = center;
    ValidCenterMin_p(theRow) = validCenterMin;
    ValidCenterMax_p(theRow) = validCenterMax;
    NumBands_p(theRow) = tNumBands;
    BandName_p(theRow).assign(bandName);
    BandMinFreq_p(theRow).assign(bandMinFreq);
    BandMaxFreq_p(theRow).assign(bandMaxFreq);
    FuncType_p(theRow).assign(funcType);
    FuncName_p(theRow).assign(funcName);
    FuncChannel_p(theRow).assign(funcChannel);
    NomFreq_p(theRow).assign(nomFreq);

    return True;

  }


  void AntennaResponses::create(const String& path){

    // set up table description

    TableDesc tD("AntennaResponsesDesc", TableDesc::New);
    tD.comment() = "antenna responses table";

    tD.addColumn(ScalarColumnDesc<Int> ("BEAM_ID", "unique for the given observatory name"));
    tD.addColumn(ScalarColumnDesc<String> ("NAME", "name of the observatory as in the Observatories table"));
    tD.addColumn(ScalarColumnDesc<Int> ("BEAM_NUMBER", "for observataories which support several simultaneous beams, zero-based"));
    tD.addColumn(ScalarColumnDesc<Double> ("START_TIME", "the time from which onwards this table row is valid, measure fixed to UTC"));
    tD.addColumn(ScalarColumnDesc<String> ("ANTENNA_TYPE", "for heterogeneous arrays: indication of the antenna type"));
    tD.addColumn(ScalarColumnDesc<String> ("RECEIVER_TYPE", "permits multiple receivers per band"));
    tD.addColumn(ScalarColumnDesc<Int> ("NUM_BANDS", "number of elements in the array columns in this table"));
    tD.addColumn(ArrayColumnDesc<String> ("BAND_NAME", "name of the frequency band"));
    tD.addColumn(ArrayColumnDesc<Double> ("BAND_MIN_FREQ", "minimum frequency of the band in the observatory frame"));
    tD.addColumn(ArrayColumnDesc<Double> ("BAND_MAX_FREQ", "maximum frequency of the band in the observatory frame"));
    tD.addColumn(ArrayColumnDesc<Double> ("CENTER", "the nominal center sky position where this row is valid"));
    tD.addColumn(ScalarColumnDesc<Int> ("CENTER_REF", ColumnDesc::Direct));
    tD.addColumn(ArrayColumnDesc<Double> ("VALID_CENTER_MIN", "sky position validity range min values"));
    tD.addColumn(ArrayColumnDesc<Double> ("VALID_CENTER_MAX", "sky position validity range max values"));
    tD.addColumn(ArrayColumnDesc<Int> ("FUNCTION_TYPE"));
    tD.addColumn(ArrayColumnDesc<String> ("FUNCTION_NAME"));
    tD.addColumn(ArrayColumnDesc<uInt> ("FUNCTION_CHANNEL"));
    tD.addColumn(ArrayColumnDesc<Double> ("NOMINAL_FREQ"));

    // Add TableMeasures information for designated Measures/Quanta columns

    TableMeasValueDesc timeMeasVal(tD, "START_TIME");
    TableMeasRefDesc timeMeasRef(MEpoch::DEFAULT);
    TableMeasDesc<MEpoch> timeMeasCol(timeMeasVal, timeMeasRef);
    timeMeasCol.write(tD);

    TableQuantumDesc timeQuantDesc(tD, "START_TIME", Unit ("s"));
    timeQuantDesc.write(tD);

    TableQuantumDesc freqQuantDesc(tD, "BAND_MIN_FREQ", Unit ("Hz"));
    freqQuantDesc.write (tD);
    TableQuantumDesc freqQuantDesc2(tD, "BAND_MAX_FREQ", Unit ("Hz"));
    freqQuantDesc2.write (tD);
    TableQuantumDesc freqQuantDesc3(tD, "NOMINAL_FREQ", Unit ("Hz"));
    freqQuantDesc3.write (tD);

    TableMeasValueDesc refDirMeasVal (tD, "CENTER");
    TableMeasRefDesc refDirMeasRef (tD, "CENTER_REF");
    TableMeasDesc<MDirection> refDirMeasCol (refDirMeasVal, refDirMeasRef);
    refDirMeasCol.write(tD);
    TableMeasValueDesc refDirMeasValMin (tD, "VALID_CENTER_MIN");
    TableMeasDesc<MDirection> refDirMeasColMin (refDirMeasValMin, refDirMeasRef);
    refDirMeasColMin.write(tD);
    TableMeasValueDesc refDirMeasValMax (tD, "VALID_CENTER_MAX");
    TableMeasDesc<MDirection> refDirMeasColMax (refDirMeasValMax, refDirMeasRef);
    refDirMeasColMax.write(tD);

    // create the table
    SetupNewTable newtab (path, tD, Table::New);
    Table tab(newtab, numRows_p);

    if(numRows_p>0){
      // fill the table

      ScalarColumn<Int> beamIdCol(tab, "BEAM_ID");
      ScalarColumn<String> nameCol(tab, "NAME");
      ScalarColumn<Int> beamNumberCol(tab, "BEAM_NUMBER");
      ScalarMeasColumn<MEpoch> startTimeCol(tab, "START_TIME");
      ScalarColumn<String> antennaTypeCol(tab, "ANTENNA_TYPE");
      ScalarColumn<String> receiverTypeCol(tab, "RECEIVER_TYPE");
      ScalarColumn<Int> numBandsCol(tab, "NUM_BANDS");
      ArrayColumn<String> bandNameCol(tab, "BAND_NAME");
      ArrayQuantColumn<Double> bandMinFreqCol(tab, "BAND_MIN_FREQ");
      ArrayQuantColumn<Double> bandMaxFreqCol(tab, "BAND_MAX_FREQ");
      ScalarMeasColumn<MDirection> centerCol(tab, "CENTER");
      ScalarMeasColumn<MDirection> validCenterMinCol(tab, "VALID_CENTER_MIN");
      ScalarMeasColumn<MDirection> validCenterMaxCol(tab, "VALID_CENTER_MAX");
      ArrayColumn<Int> functionTypeCol(tab, "FUNCTION_TYPE");   
      ArrayColumn<String> functionNameCol(tab, "FUNCTION_NAME");
      ArrayColumn<uInt> functionChannelCol(tab, "FUNCTION_CHANNEL");
      ArrayQuantColumn<Double> nomFreqCol(tab, "NOMINAL_FREQ");

      for(uInt i=0; i<numRows_p; i++){
	beamIdCol.put(i, BeamId_p(i));
	nameCol.put(i, ObsName_p(i));
	beamNumberCol.put(i, BeamNumber_p(i));
	startTimeCol.put(i, StartTime_p(i));
	antennaTypeCol.put(i, AntennaType_p(i));
	receiverTypeCol.put(i, ReceiverType_p(i));
	numBandsCol.put(i, NumBands_p(i));
	bandNameCol.put(i, BandName_p(i));

	Vector<Quantity> bMF(BandMinFreq_p(i).nelements());
	for(uInt k=0; k<bMF.nelements(); k++){
	  bMF(k) = (BandMinFreq_p(i)(k)).get(); // convert MVFrequency to Quantity in Hz
	}
	bandMinFreqCol.put(i, bMF); 

	for(uInt k=0; k<bMF.nelements(); k++){
	  bMF(k) = (BandMaxFreq_p(i)(k)).get(); // convert MVFrequency to Quantity in Hz
	}
	bandMaxFreqCol.put(i, bMF);

	for(uInt k=0; k<bMF.nelements(); k++){
	  bMF(k) = (NomFreq_p(i)(k)).get(); // convert MVFrequency to Quantity in Hz
	}
	nomFreqCol.put(i, bMF);

	centerCol.put(i, ValidCenter_p(i));
	validCenterMinCol.put(i, ValidCenterMin_p(i));
	validCenterMaxCol.put(i, ValidCenterMax_p(i));

	Vector<Int> iFT(FuncType_p(i).nelements());
	for(uInt k=0; k<iFT.nelements(); k++){
	  iFT(k) = static_cast<Int>(FuncType_p(i)(k));
	}
	functionTypeCol.put(i, iFT);

	functionNameCol.put(i, FuncName_p(i));
	functionChannelCol.put(i, FuncChannel_p(i));
      }

    }

    return;

  }

  AntennaResponses::FuncTypes AntennaResponses::FuncType(Int i){
    if(-1 < i && i < static_cast<Int>(AntennaResponses::N_FuncTypes) ){
      return static_cast<AntennaResponses::FuncTypes>(i);
    }
    else{
      return AntennaResponses::INVALID;
    }
  }


} //# NAMESPACE CASA - END

