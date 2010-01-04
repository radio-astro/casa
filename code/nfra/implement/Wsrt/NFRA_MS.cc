//# NFRA_MS.cc: This program demonstrates conversion of UVW for WSRT
//# Copyright (C) 1998,1999,2000,2001,2002,2003
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$
//
// HISTORY
// ------
// 27MAR2006 - checked, .vsd updated

//# Includes
#include <strstream>
#include <measures/Measures/MBaseline.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MeasData.h>
#include <measures/Measures/MeasFrame.h>
#include <measures/Measures/MeasRef.h>
#include <measures/Measures/MeasTable.h>
#include <measures/Measures/Muvw.h>
#include <measures/Measures/Stokes.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/MVAngle.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Quanta/MVBaseline.h>
#include <casa/Quanta/MVuvw.h>
#include <casa/Containers/Record.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <ms/MeasurementSets/MSField.h>
#include <ms/MeasurementSets/MSFieldColumns.h>
#include <ms/MeasurementSets/MSAntenna.h>
#include <ms/MeasurementSets/MSAntennaColumns.h>
#include <ms/MeasurementSets/MSSummary.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableRow.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/ExprNode.h>
#include <tables/Tables/ColumnDesc.h>
#include <tables/Tables/ArrColDesc.h>
#include <measures/TableMeasures/ArrayMeasColumn.h>
#include <measures/TableMeasures/TableMeasDesc.h>
#include <measures/TableMeasures/TableMeasValueDesc.h>
#include <measures/TableMeasures/TableMeasRefDesc.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Arrays/ArrayUtil.h>
#include <casa/System/ProgressMeter.h>
#include <casa/Inputs.h>
#include <casa/OS/Path.h>
#include <casa/OS/File.h>
#include <casa/BasicMath/Math.h>
#include <casa/Utilities/Assert.h>
#include <casa/Utilities/Sort.h>
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <casa/iomanip.h>
#include <casa/Logging/LogOrigin.h>

#include <nfra/Wsrt/NFRA_MS.h>
#include <nfra/Wsrt/NFRA_MF.h>

#include <casa/namespace.h>

String sec2date(Double);
String sec2time(Double);

//======================================================================
// Class NFRA_MS
// Public methods
//
//----------------------------------------------------------------------
// Dump the info to cout
//
void NFRA_MS::dump()
{
  cout << "Class items" << endl;
  cout << "-----------" << endl;
  cout << "MSError=" << MSError << endl;
  cout << "ErrStr=" << ErrStr << endl;
  cout << "Method=";
  switch (useMethod){
  case NFRA: cout << "NFRA_TMS_PARAMETERS table"; break;
  case MAIN: cout << "MAIN table"; break;
  default: cout << "ERROR - dump - Method not recognized."; break;
  }
  cout << endl;

  //
  // MS-related variables
  //
  cout << endl;
  cout << "MS items" << endl;
  cout << "--------" << endl;

  cout << "MSName=[" ;
  for (uInt i = 0; i < MSName.size(); i++){
    if (i > 0) cout << ", ";
    cout << MSName[i];
  }
  cout << "]" << endl;

  uInt N = SubArrays.size();
  cout << "NSubArrays=" << N << endl;
  for (uint i = 0; i < N; i++) SubArrays[i].dump(i);

  cout << "MSVersion=" << MSVersion << endl;
  cout << "SeqNr=" << SeqNr << endl;
  cout << "Instrument=" << Instrument << endl;
  cout << "NRow=" << NRow << endl;
  cout << "SetNr=" << SetNr << endl;
  cout << "Semester=" << Semester << endl;
  cout << "PrjNr=" << PrjNr << endl;
  cout << "FieldName=" << FieldName << endl;
  cout << "BackEnd=" << BackEnd << endl;
  cout << "PhaseSwitchMode=" << PhaseSwitchMode << endl;
  cout << "IFTaper=" << IFTaper << endl;

  N = BaseLines.size();
  cout << "NBaseLines=" << N << endl;
  for (uInt i = 0; i < N; i++) BaseLines[i].dump(i);

  cout << "DateTimeStartRaw=" << DateTimeStart << endl;
  cout << "DateTimeStartStr=" << sec2date(DateTimeStart) << " " << sec2time(DateTimeStart) << endl;

  cout << "DateTimeEndRaw=" << DateTimeEnd << endl;
  cout << "DateTimeEndStr=" << sec2date(DateTimeEnd) << " " << sec2time(DateTimeEnd) << endl;
  cout << "Duration=" << Duration << endl;

  cout << "ExpTime=[";
  for (uInt i = 0; i < ExpTime.size(); i++){
    if (i > 0) cout << ", ";
    cout << ExpTime[i];
  }
  cout << "]" << endl;

  cout << "Epoch=" << Epoch << endl;

  cout << "NPositions=" << getNPos() << endl;
  N = PWs.size();
  cout << "NPointingWindows=" << N << endl;
  for (uInt i = 0; i < PWs.size(); i++) PWs[i].dump(i);

  cout << "NFreqMos=" << getNFreqMos() << endl;
  N = FWs.size();
  cout << "NFrequencyWindows=" << N << endl;
  for (uInt i = 0; i < FWs.size(); i++) FWs[i].dump(i);
}

//----------------------------------------------------------------------
//
vector<Double> NFRA_MS::getBaseLineValues()
{
  vector<Double> rtn;
  for (uInt i = 0; i < BaseLines.size(); i++)
    rtn.push_back(BaseLines[i].getDistance());
  return rtn;
}

//----------------------------------------------------------------------
// Select the method.
// May only be one of the enum Method values.
//
Bool NFRA_MS::setMethod(Method m)
{
  if (m > maxMethod){
    MSError = 1;
    ErrStr = "ERROR - Not a legal method";
    return False;
  }
  useMethod = m;
  return True;
}

//----------------------------------------------------------------------
// Get only a parameter from the NFRA_... table
//
String NFRA_MS::getNFRAkwd(String inName, String kwd)
{
  String MSName = inName;
  Table ms(MSName);
  TableRecord kwds = ms.keywordSet();

  if (!kwds.isDefined("NFRA_TMS_PARAMETERS")){
    String ErrStr = "ERROR - No NFRA_TMS_PARAMETERS table.";
    return "";
  }

  NFRA_parameters parms = NFRA_parameters(ms);
  return parms.getVal(kwd);
}

//----------------------------------------------------------------------
// Fill the class properties using the correct method
//
Bool NFRA_MS::setInfo(String inName)
{

  //
  // Check if the object has been used before
  //
  if (isFilledVar){
    MSError = 1;
    ErrStr = "setInfo may only be used for a clean object.";
    return False;
  }

  //
  // Get the SubArray number n from the filename: <SeqNr>_S<n>_T<m>.MS
  // Is -1 when it fails, however, this is not e real error because
  // the MSName need not be the WBork standard name.
  //
  Int SubArray = getSubArray(inName);
  {
    NFRA_SubArray tmp(SubArray);
    SubArrays.push_back(tmp);
  }

  //
  // name of MS
  //
  MSName[0] = inName;

  //
  // Open the MS, get the keywords
  //
  Table ms(inName);
  NRow = ms.nrow();
  TableRecord kwds = ms.keywordSet();

  //
  // MSVersion is a keyword of the MAIN table
  //
  Int i = kwds.fieldNumber("MS_VERSION");
  if (i == -1) {
    cerr << "Warning - MS_VERSION not defined." << endl;
    MSVersion = 0.0;
  } else {
    MSVersion = kwds.asFloat(i);
  }

  //
  // Even in NFRA_TMS_PARAMETER mode we need some other tables
  //

  {
    NFRA_main main(inName, 0, 0);
    DateTimeStart = main.DateTimeStart;
    DateTimeEnd = main.DateTimeEnd;
  }
  Duration = DateTimeEnd - DateTimeStart;

  //
  // From the OBSERVATION table we get:
  //  - Semester
  //  - PrjNr
  //  - FieldName
  //
  if (!kwds.isDefined("OBSERVATION")){
    MSError = 1;
    ErrStr = "No OBSERVATION table.";
    return False;
  } else {
    NFRA_obs obs = NFRA_obs(ms);
    Semester = obs.getSemester();
    PrjNr = obs.getPrjNr();
    FieldName = obs.getFieldName();
  }

  //
  // From the PROCESSOR table we get:
  //  - BackEnd
  //
  if (!kwds.isDefined("PROCESSOR")){
    MSError = 1;
    ErrStr = "No PROCESSOR table.";
    return False;
  } else {
    NFRA_proc proc = NFRA_proc(ms);
    BackEnd = proc.getBackEnd();
  }

  //
  // From the ANTENNA table we get
  //  - BaseLines
  //
  if (!kwds.isDefined("ANTENNA")){
    MSError = 1;
    ErrStr = "No ANTENNA table.";
    return False;
  } else {
    NFRA_antenna antenna = NFRA_antenna(ms);
    BaseLines = antenna.getBaseLines();
  }

  //
  // From here, the split for NFRA-table, MEAS-fie or MAIN table
  // digging must be made.
  //
  Bool rtn;
  switch (useMethod) {
  case NFRA: rtn = setInfo_NFRA(inName); break;
  case MEAS: rtn = setInfo_MEAS(inName); break;
  case MAIN:
    cerr << "ERROR - No MAIN scan yet ..." << endl;
    return False;
  default: 
    ErrStr = "ERROR - setInfo - Method not recognized"; 
    MSError = 1; 
    return False;
  }

  int l = ExpTime.size();
  Duration += ExpTime[l-1];

  return rtn;
}

//----------------------------------------------------------------------
// Merge a new MS with the class properties using the correct method
//
Bool NFRA_MS::mergeInfo(String inName)
{
  switch (useMethod) {
  case NFRA: return mergeInfo_NFRA(inName);
  case MAIN: return mergeInfo_MAIN(inName);
  default: ErrStr = "ERROR - mergeInfo - Method not recognized"; MSError = 1; return False;
  }
}

//======================================================================
// Private class method
//

//----------------------------------------------------------------------
// Init the info items
//
void NFRA_MS::init()
{
  MSError = 0;
  ErrStr = "No Error";
  isFilledVar = False;
  useMethod = NFRA_MS::Method(-1);
  maxMethod = MAIN;

  MSName.resize(1); MSName[0] = "";
  MSVersion = -1;
  SeqNr = "None";
  Instrument = "None";
  NRow = -1;
  Semester = "None";
  PrjNr = "None";
  FieldName = "None";
  BackEnd = "None";
  PhaseSwitchMode = "None";
  IFTaper = "None";
  DateTimeStart = -1;
  DateTimeEnd = -1;
  Duration = -1;
  ExpTime.resize(1); ExpTime[0] = 0;
  Epoch = "None";
}

//----------------------------------------------------------------------
// setInfo - enter with MSname, set all properteis
// - only do this for a clean object
//
Bool NFRA_MS::setInfo_NFRA(String inName)
{
  //
  // Open the MS, get the keywords
  //
  Table ms(inName);
  TableRecord kwds = ms.keywordSet();

  //
  // Do a full position and a full frequency scan in the
  // NFRA_TMS_PARAMETERS table
  // This creates properties in the NFRA_parameters object
  //
  if (!kwds.isDefined("NFRA_TMS_PARAMETERS")){
    MSError = 1;
    ErrStr = "No NFRA_TMS_PARAMETERS table.";
    return False;
  }
  NFRA_parameters parms = NFRA_parameters(ms, SubArrays[0].getSANr());
  parms.FullPositionScan();
  parms.FullFrequencyScan();

  //
  // We now can obtain info from the NFRA_TMS_PARAMETERS table
  //

  SeqNr = parms.getVal("RTSystem.ActSeqNumber");
  Instrument = parms.getVal("Instrument");
  PhaseSwitchMode = parms.getVal("DZB20-COR.PhaseSwitchPattern");
  IFTaper = parms.getVal("DZBReadout.Taper");
  SetNr = parms.getVal("DZBReadout.SetNr");

  //
  // Times are already obtained from the MAIN table
  //
  // Times must be converted to double
  //
  //  {
  //    String v = parms.getVal("RTSystem.ActStartTime");
  //    DateTimeStart = Str2Time(v);
  //    v = parms.getVal("RTSystem.ActEndTime");
  //    DateTimeEnd = Str2Time(v);
  //  }
  //  Duration = DateTimeEnd - DateTimeStart;

  //
  // The integration time must be converted to Int
  // The property is a vector because in the MAIN table different
  // values may occur for e.g. mosaic measurements.
  // In the NFRA_TMS_PARAMETERS only one value is listed.
  // The integration time is listed as number of ticks, one tick is 10
  // sec, so we must multiply by 10.
  //
  {
    String v = parms.getVal("DZBReadout.IntegrationTime");
    uInt t = 10*atoi(v.c_str());
    ExpTime[0] = t;
  }

  Epoch = parms.getVal("PW1.Reference");

  //
  // Get the Pointing Windows
  // The Start and End times must be given to the Position Windows
  // to have them calculate their HA start and -end
  // 
  PWs = parms.getPWs();
  for (uInt p = 0; p < PWs.size(); p++){
    PWs[p].setDTStart(DateTimeStart);
    PWs[p].setDTEnd(DateTimeEnd);
  }

  //
  // Get the Frequency Windows
  //
  FWs = parms.getFWs();

  //
  // We have information
  //
  isFilledVar = True;
  
  //
  // That's all folks
  //
  return True;

}

//----------------------------------------------------------------------
// Merge info from an other timeslice of subarray
//
Bool NFRA_MS::mergeInfo_NFRA(String inName)
{
  //
  // Before we do anything we need to know if the MS has the same SeqNr.
  // Therefore we need the SubArray number and the NFRA_TMS_PARAMETERS
  // table.
  //
  Int SubArray = getSubArray(inName);
  Table ms(inName);
  TableRecord kwds = ms.keywordSet();
  if (!kwds.isDefined("NFRA_TMS_PARAMETERS")){
    MSError = 1;
    ErrStr = "No NFRA_TMS_PARAMETERS table.";
    return False;
  }
  NFRA_parameters parms = NFRA_parameters(ms, SubArray);

  //
  // Check sequence numbers
  //
  String tmp = parms.getVal("RTSystem.ActSeqNumber");
  if (tmp != SeqNr){
    MSError = 1;
    ErrStr = "This MS is from a different measurement: "+SeqNr + " != " + tmp;
    return False;
  }

  //
  // Store the name
  //
  MSName.push_back(inName);

  //
  // If the subarray is new, merge the IVCs into the existing FW tree
  // for the same FreqMos point.
  // If the subarray has been seen before, all FWs are already known,
  // so do nothing.
  //
  Bool Found = False;
  uInt I = 0;
  for (uInt i = 0; i < SubArrays.size(); i++){
    if (SubArrays[i].getSANr() == SubArray){
      Found = True;  // have seen this subarray before
      I = i;         // stored at this index
    }
  }


  {
    NFRA_main main(inName, 0, 0);
    DateTimeEnd = main.DateTimeEnd;
  }
  Duration = DateTimeEnd - DateTimeStart;
  int l = ExpTime.size();
  Duration += ExpTime[l-1];

  NRow += ms.nrow();

  //
  // If this is an existing subarray, increase the number of
  // TimeSlices and return. 
  // All properties are supposed to be equal - this, however, is not
  // checked ...
  //
  if (Found){
    SubArrays[I].incrNTimeSlices();
    return True;
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // So, this is a new subarray

  //
  // New subarray -> need to add its frequency information
  //
  {
    NFRA_SubArray tmp(SubArray);
    SubArrays.push_back(tmp);
  }

  //
  // Only the NFRA_TMS_PARAMETERS table is processed, the other tables
  // are supposed to be equal ...
  //

  //
  // Only need to do a frequency scan - the positions are equal
  //
  parms.FullFrequencyScan();

  vector<NFRA_FW> NewFWs = parms.getFWs();

  for (uInt i = 0; i < FWs.size(); i++)
    FWs[i].addIVCs(NewFWs[i]);

  return True;
}

//----------------------------------------------------------------------
// return the min/max HA
//
Double NFRA_MS::getHaMin()
{
  Double rtn = 100.0;
  for (uInt i = 0; i < PWs.size(); i++){
    rtn = min(rtn, PWs[i].getHaMin());
  }
  return rtn;
}
Double NFRA_MS::getHaMax()
{
  Double rtn = 0;
  for (uInt i = 0; i < PWs.size(); i++){
    rtn = max(rtn, PWs[i].getHaMax());
  }
  return rtn;
}
//----------------------------------------------------------------------
// RA, DEC centroids
//
Double NFRA_MS::getRACentroid()
{
  Double sum = 0;
  for (uInt i = 0; i < PWs.size(); i++){
    sum += PWs[i].getRACentroid();
  }
  return sum / PWs.size();
}
Double NFRA_MS::getDECCentroid()
{
  Double sum = 0;
  for (uInt i = 0; i < PWs.size(); i++){
    sum += PWs[i].getDECCentroid();
  }
  return sum / PWs.size();
}


//----------------------------------------------------------------------
// Get the TOTAL number of position, thus accumulated over all PWs
//
uInt NFRA_MS::getNPos()
{
  uInt rtn = 0;
  for (uInt i = 0; i < PWs.size(); i++){
    rtn += PWs[i].getNPos();
  }

  return rtn;
}
//----------------------------------------------------------------------
// Get the TOTAL number of Frequency mosaicings, thus accumulated over
// all FWs.
//
uInt NFRA_MS::getNFreqMos()
{
  uInt rtn = 0;
  for (uInt i = 0; i < FWs.size(); i++){
    rtn += FWs[i].getNFreqMos();
  }

  return rtn;
}

//----------------------------------------------------------------------
// setInfo - enter with MSname, set all properteis
// - only do this for a clean object
//
Bool NFRA_MS::setInfo_MEAS(String inName)
{

  cerr << "Not yet implemented" << endl;

  return False;

  //
  // Check if the object has been used before
  //
  if (isFilledVar){
    MSError = 1;
    ErrStr = "setInfo may only be used for a clean object.";
    return False;
  }

  NFRA_MF MF(inName);

  //
  // name of MS
  //
  MSName[0] = inName;
  MSVersion = 0.0;

  Semester = MF.getSemester();
  PrjNr = MF.getPrjNr();
  FieldName = MF.getFieldName();
  BackEnd = MF.getBackEnd();
  //  BaseLines = MF.getBaseLines();

  //
  // The NFRA_PARAMETERS can be read from MS or from file
  //
  NFRA_parameters parms = NFRA_parameters(MF);
  parms.FullPositionScan();
  parms.FullFrequencyScan();

  //
  // We now can obtain info from the NFRA_TMS_PARAMETERS table
  //

  SeqNr = parms.getVal("RTSystem.ActSeqNumber");
  Instrument = parms.getVal("Instrument");
  PhaseSwitchMode = parms.getVal("DZB20-COR.PhaseSwitchPattern");
  IFTaper = parms.getVal("DZBReadout.Taper");

  //
  // 31AUG2006 0 RA - Start/End time are already obtaind from MAIN table
  //
  // Times must be converted to double
  //
//   {
//     String v = parms.getVal("RTSystem.ActStartTime");
//     DateTimeStart = Str2Time(v);
//     v = parms.getVal("RTSystem.ActEndTime");
//     DateTimeEnd = Str2Time(v);
//   }
//  Duration = DateTimeEnd - DateTimeStart;

  //
  // The integration time must be converted to Int
  // The property is a vector because in the MAIN table different
  // values may occur for e.g. mosaic measurements.
  // In the NFRA_TMS_PARAMETERS only one value is listed.
  // The integration time is listed as number of ticks, one tick is 10
  // sec, so we must multiply by 10.
  //
  {
    String v = parms.getVal("DZBReadout.IntegrationTime");
    uInt t = 10*atoi(v.c_str());
    ExpTime[0] = t;
  }

  Epoch = parms.getVal("PW1.Reference");

  //
  // Get the Pointing Windows
  // The Start and End times must be given to the Position Windows
  // to have them calculate their HA start and -end
  // 
  PWs = parms.getPWs();
  for (uInt p = 0; p < PWs.size(); p++){
    PWs[p].setDTStart(DateTimeStart);
    PWs[p].setDTEnd(DateTimeEnd);
  }

  //
  // Get the Frequency Windows
  //
  FWs = parms.getFWs();

  //
  // We have information
  //
  isFilledVar = True;
  
  //
  // That's all folks
  //
  return True;

}

//----------------------------------------------------------------------
// No MAIN scan yet ...
Bool NFRA_MS::setInfo_MAIN(String inName)
{
  cerr << "ERROR - No MAIN scan yet ..." << endl;
  return False;
}
Bool NFRA_MS::mergeInfo_MAIN(String inName)
{
  cerr << "ERROR - No MAIN scan yet ..." << endl;
  return False;
}

//----------------------------------------------------------------------
// Parse the filename.
// If it has the format <seqnr>_S<n>_T<m>, the n is returned.
//
Int NFRA_MS::getSubArray(String InName)
{
  Int i0 = InName.find("_S");
  if (i0 == -1) return -1;
  Int i1 = InName.find("_T");
  if ((i1 == -1) || (i1 < i0)) return -1;

  i0 += 2;
  String s = InName.substr(i0, i1-i0);

  try{
    uInt i = atoi(s.c_str());
    return i;
  }
  catch(...){
    return -1;
  }
}

