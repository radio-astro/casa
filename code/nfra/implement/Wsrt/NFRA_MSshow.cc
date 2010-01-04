//# NFRA_MSshow.cc: show methods for NFRA_MS
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
#include <nfra/Wsrt/NFRA_MSshow.h>

#include <casa/namespace.h>
#define MHz 1000000.0
#define MAXPOSITIONS 3

//#define RB_DEBUG

//----------------------------------------------------------------------
// Convert double to yyyy/mm/dd
String sec2date(Double ra)
{
  MVTime x(Quantity(ra, "s"));
  return(x.string(MVTime::YMD_ONLY));
}

//----------------------------------------------------------------------
// Convert double to hh:mm:ss
String sec2time(Double ra)
{
  MVTime x(Quantity(ra, "s"));
  return(x.string());
}

//----------------------------------------------------------------------
// Convert double to hms
String rad2hms(Double ra)
{
  MVTime x(Quantity(ra, "rad"));
  String s = x.string();
  Int i = s.find(':');
  if (i > 0){
    s[i] = 'h';
    i = s.find(':', i);
    if (i > 0){
      s[i] = 'm';
      s += 's';
    }
  }

  return(s);
}

//----------------------------------------------------------------------
// Convert double to hms
String sec2hms(Double xIn)
{
  MVAngle x(Quantity(xIn/3600.0, "deg"));
  String s = x.string();
  Int i = s.find('.');
  if (i > 0){
    s[i] = 'h';
    i = s.find('.', i);
    if (i > 0){
      s[i] = 'm';
      s += 's';
    }
  }

  return(s);
}

//----------------------------------------------------------------------
// Convert double to dms
String rad2dms(Double dec)
{
  MVAngle x(Quantity(dec, "rad"));
  String s = x.string(MVAngle::DIG2);
  Int i = s.find('.');
  if (i > 0){
    s[i] = 'd';
    i = s.find('.', i);
    if (i > 0){
      s[i] = 'm';
      s += 's';
    }
  }
  return(s);
}

//----------------------------------------------------------------------
// convert radian to degree and hour
Double rad2deg(Double x)
{
  Double y = x * 180.0 / C::pi;
  return y;
}
Double rad2hour(Double x)
{
  Double y = x * 180.0 / C::pi / 15.0;
  return y;
}

//----------------------------------------------------------------------
// convert radian to degree in range -180 -> 180
Double rad2deg180(Double x)
{
  Double y = x * 180.0 / C::pi;
  while (y < -180) y += 180.0;
  while (y > 180) y -= 180.0;
  return y;
}

//----------------------------------------------------------------------
// Convert Vector<Double> to comma separated string
// Use an ostringstream
// This, and the next functions, can be done with a template ...
String VDbl2Str(Vector<Double> x)
{
  uInt n = x.shape()[0];
  std::ostringstream str;
  Bool start = True;
  for (uInt i = 0; i < n; i++){
    if (start) start = False;
    else str << ",";
    str << x[i];
  }
  return str.str();
}

//----------------------------------------------------------------------
// Convert Vector<Int> to comma separated string
// Use an ostringstream
String VInt2Str(Vector<Int> x)
{
  uInt n = x.shape()[0];
  std::ostringstream str;
  Bool start = True;
  for (uInt i = 0; i < n; i++){
    if (start) start = False;
    else str << ", ";
    str << x[i];
  }
  return str.str();
}

//----------------------------------------------------------------------
// Convert Vector<uInt> to comma separated string
// Use an ostringstream
String VuInt2Str(Vector<uInt> x)
{
  uInt n = x.shape()[0];
  std::ostringstream str;
  Bool start = True;
  for (uInt i = 0; i < n; i++){
    if (start) start = False;
    else str << ",";
    str << x[i];
  }
  return str.str();
}

//----------------------------------------------------------------------
// Convert Vector<String> to comma separated string
// Use an ostringstream
String VStr2Str(Vector<String> x)
{
  uInt n = x.shape()[0];
  std::ostringstream str;
  Bool start = True;
  for (uInt i = 0; i < n; i++){
    if (x[i] != ""){
      if (start) start = False;
      else str << ",";
      str << x[i];
    }
  }
  return str.str();
}

//----------------------------------------------------------------------
// Convert a Vector<Double> to comma separated HMS strings
// Use an ostringstream
String Vrad2hmsStr(Vector<Double> x)
{
  uInt n = x.shape()[0];
  std::ostringstream str;
  Bool start = True;
  for (uInt i = 0; i < n; i++){
    if (start) start = False;
    else str << ",";
    str << rad2hms(x[i]);
  }
  return str.str();
}

//----------------------------------------------------------------------
// Convert a Vector<Double> to comma separated DMS strings
// Use an ostringstream
String Vrad2dmsStr(Vector<Double> x)
{
  uInt n = x.shape()[0];
  std::ostringstream str;
  Bool start = True;
  for (uInt i = 0; i < n; i++){
    if (start) start = False;
    else str << ",";
    str << rad2dms(x[i]);
  }
  return str.str();
}

//----------------------------------------------------------------------
// Convert a String (0, or else) to False or True
String Str2BStr(String x)
{
  return atoi(x.c_str()) == 0 ? "False" : "True";
}

// Toplevel for show - switch to correct show<mode> method
//
void NFRA_MSshow::show(String OMode)
{
  if (OMode == "std"){
    showStd();
  } else if (OMode == "readme"){
    showReadme();
  } else if (OMode == "raw"){
    dump();
  } else if (OMode == "arch"){
    showArch();
  } else if (OMode == "help"){
    cerr << "Possible output modes:" << endl;
    cerr << " - std -> [default] for human readable output," << endl;
    cerr << " - readme -> for the readme files created by Jur Sluman," << endl;
    cerr << " - raw -> simple dump of all values," << endl;
    cerr << " - arch -> for loading the MSArchive table in MySQL." << endl;
  } else {
    cerr << "Error - unknown output mode, try mode=help." << endl;
  }
}


void NFRA_MSshow::showArch()
{
  cout << "use MSArchive;" << endl;
  cout << "insert into MSArchive (";
  cout << "SequenceNumber,";
  cout << "Subarray,";
  cout << "NumberOfSets,";
  cout << "ObjectName,";
  cout << "ProjectNumber,";
  cout << "ProjectCode,";
  cout << "ObservationStartTime,";
  cout << "ObservationEndTime,";
  cout << "ObservationDuration,";
  cout << "ExposureTime,";
  cout << "HAStart,";
  cout << "HAEnd,";
  cout << "RACentroid,";
  cout << "DecCentroid,";
  cout << "Epoch,";
  cout << "NumberOfPosMosPoints,";
  cout << "PosMosDwellTime,";
  cout << "MFFEBand,";
  cout << "NumberOfIFBands,";
  cout << "CentralIFFrequencies,";
  cout << "NumberOfFreqMosPoints,";
  cout << "FreqMosDwellTime,";
  cout << "IFBandWidth,";
  cout << "NumberOfIFChannels,";
  cout << "IFChannelWidth,";
  cout << "IFTaperName,";
  cout << "NumberOfPolarizations,";
  cout << "PolarizationNames,";
  cout << "PhaseSwitchMode,";
  cout << "BackendName,";
  cout << "TMSInstrument,";
  cout << "MultiBeamUsed,";
  cout << "BaselinePositions";
  cout << ") values(";

  cout << "'" << SeqNr << "',"; // SequenceNumber
  cout << "'" << "0" << "',";   // SubArray
  cout << "'" << MSName.size() << "',";    // NumberOfSets
  cout << "'" << FieldName << "',";    // ObjectName
  cout << "'" << PrjNr << "',";    // ProjectNumber
  cout << "'" << Semester << "',";    // ProjectCode
  cout << "'" << sec2date(DateTimeStart) << " " << sec2time(DateTimeStart) << "',";    // ObservationStartTime
  cout << "'" << sec2date(DateTimeEnd) << " " << sec2time(DateTimeEnd) << "',";    // ObservationEndTime
  cout << "'" << Duration << "',";    // ObservationDuration
  cout << "'" << VInt2Str(ExpTime) << "',";    // ExposureTime
  cout << "'" << rad2deg180(getHaMin())  << "',";    // HAStart
  cout << "'" << rad2deg180(getHaMax())  << "',";    // HAEnd
  cout << "'" << rad2deg(getRACentroid()) << "',";    // RACentroid
  cout << "'" << rad2deg(getDECCentroid())  << "',";    // DecCentroid
  cout << "'" << Epoch << "',";    // Epoch
  cout << "'" << getNPos() << "',";    // NumberOfPosMosPoints
  cout << "'" << getPWDwellTime() << "',";    // PosMosDwellTime
  cout << "'" << getMFFEName() << "',";    // MFFEBand
  cout << "'" << getNIVCBands() << "',";    // NumberOfIFBands
  cout << "'" << VDbl2Str(getCenterFreqs()) << "',";    // CentralIFFrequencies
  cout << "'" << getNFreqMos() << "',";    // NumberOfFreqMosPoints
  cout << "'" << getFWDwellTime() << "',";    // FreqMosDwellTime
  cout << "'" << getIFBandWidth() << "',";    // IFBandWidth
  cout << "'" << getNChan() << "',";    // NumberOfIFChannels
  cout << "'" << getChanWidth() << "',";    // IFChannelWidth
  cout << "'" << IFTaper << "',";    // IFTaperName
  cout << "'" << getNPol() << "',";    // NumberOfPolarizations
  cout << "'" << getPolsUsed() << "',";    // PolarizationNames
  cout << "'" << PhaseSwitchMode << "',";    // PhaseSwitchMode
  cout << "'" << BackEnd << "',";    // BackendName
  cout << "'" << Instrument << "',";    // TMSInstrument
  cout << "'" << "F" << "',";    // MultiBeamUsed
  cout << "'" << VDbl2Str(getBaseLineValues()) << "'";    // BaselinePosition
  cout << ");";

  cout << endl;
}

void drawLine0()
{
  cout << endl;
  for (uInt i = 0; i < 80; i++) cout << '=';
  cout << endl;
}
void drawLine1()
{
  cout << endl;
  for (uInt i = 0; i < 80; i++) cout << '-';
  cout << endl;
}
void drawLine2()
{
  cout << endl;
  for (uInt i = 0; i < 40; i++) cout << "- ";
  cout << endl;
}

//----------------------------------------------------------------------
void NFRA_MSshow::showStd(vector<NFRA_BaseLine> B)
{
  cout << "Baselines from Telescope 9 to:" << endl;
  uInt N = B.size();
  for (uInt i = 0; i < N; i++){
    cout << B[i].getTelescope() << "        ";
  }
  cout << endl;
  for (uInt i = 0; i < N; i++){
    cout << B[i].getDistance() << "  ";
  }
  cout << endl;
}

//======================================================================
// Show Positions in standard format - human readable
//
//----------------------------------------------------------------------
void NFRA_MSshow::showStd(NFRA_Position P)
{
  cout << rad2hms(P.getRA()) << " ";
  cout << rad2dms(P.getDEC()) << " ";
  cout.setf(std::ios_base::fixed, std::ios_base::floatfield);
  cout.setf(std::ios_base::right, std::ios_base::adjustfield);
  Int p = cout.precision(4);
  cout << setw(9) << rad2deg(P.getHaMin()) << " ";
  cout << setw(9) << rad2deg(P.getHaMax()) << " ";
  cout.precision(p);
  cout.setf(std::ios_base::fmtflags(0), std::ios_base::floatfield);
  cout << P.getName();
}


void NFRA_MSshow::showStdOnePosMos(NFRA_PosMos P)
{
  vector<NFRA_Position> Positions =  P.getPositions();
  uInt NMB = Positions.size();

  if (NMB == 1){
    cout << endl;
    cout << "RA        DEC          HAstart   HAend     Name" << endl;
    showStd(Positions[0]);
    cout << endl;
  } else {
    cout << "        Multi Beam Positions = " << NMB << endl;
    cout << endl;
    cout << "RA        DEC          HAstart   HAend     Name" << endl;
    for (uInt p = 0; p < NMB; p++){
      showStd(Positions[p]);
      cout << endl;
    }
  }

}

void NFRA_MSshow::showStdPosMos(NFRA_PosMos P)
{
  vector<NFRA_Position> Positions =  P.getPositions();
  uInt NMB = Positions.size();

  cout << "              Dwell Time [s] = " << P.getDwellTime() << endl;
  cout << "Number of Beam Positions     = " << NMB << endl;
  cout << endl;
  cout << "RA        DEC          HAstart   HAend     Name" << endl;
  for (uInt b = 0; b < NMB; b++){
    showStd(Positions[b]);
    cout << endl;
  }

}

//----------------------------------------------------------------------
// Write a vector of PosMoss to stdout
//
// - if there are multiple PosMoss, a dwell time must be written.
//
// Writing a DwellTime os somewhat complicated. For a non-MultiBeam
// case, the dwelltime must be written in-line, for a MultiBeam case
// the DwellTime must be written in the header.
//
void NFRA_MSshow::showStd(vector<NFRA_PosMos> P)
{
  uInt NPM = P.size();

  if (NPM == 1){

    showStdOnePosMos(P[0]);

  } else {

    cout << "   Pointing Mosaic Positions = " << NPM << endl;

    //
    // count the total number of positions to write
    //
    uInt NPos = 0;
    for (uInt i = 0; i < NPM; i++) NPos += P[i].getNPos();

    //
    // If there are no Multi Beams (the total number of Postions
    // equals the number of PosMos points) the dwell time is written
    // in-line.
    //
    if (NPos == NPM){
      cout << endl;
      cout << "DwTm RA        DEC          HAstart   HAend     Name" << endl;
      for (uInt i = 0; i < NPM; i++){
	cout << setw(4) << P[i].getDwellTime() << " ";
	showStd(P[i].getPositions()[0]);
	cout << endl;
      }
    } else {
      //
      // Write every PosMos individually
      //
      for (uInt i = 0; i < NPM; i++){
	drawLine2();
	cout << "Position Mosaic Point: " << i << endl;
	showStdPosMos(P[i]);
      }
    }

  }
}

//----------------------------------------------------------------------
// Write a vector of PWs to stdout
//
// - always write the telescope word
// - if there is only one PW, do not write a header
//
void NFRA_MSshow::showStd(vector<NFRA_PW> P)
{
  uInt N = P.size();

  if (N == 1){

    cout << "       Telescopes used       = " << P[0].getTelescopes() << endl;

    showStd(P[0].getPosMoss());    

  } else {

    cout << "  Number of Pointing Windows = " << N << endl;
    for (uInt i = 0; i < N; i++){
      drawLine2();
      cout << "Pointing Window: " << i+1 << endl;
      cout << "       Telescopes used       = " << P[i].getTelescopes() << endl;

      showStd(P[i].getPosMoss());

    }
  }
}

//======================================================================
// Show frequency in Std format - human readable
//----------------------------------------------------------------------
void NFRA_MSshow::showStd(vector<NFRA_FW> F)
{
  uInt N = F.size();

  if (N > 1)
    cout << " Number of Frequency Windows = " << N << endl;
  for (uInt i = 0; i < N; i++){
    if (N > 1){
      drawLine2();
      cout << "Frequency Window: " << i+1 << endl;
    }
    cout << "       Telescopes used       = " << F[i].getTelescopes() << endl;
    showStd(F[i].getFreqMoss());
  }
}

//----------------------------------------------------------------------
void NFRA_MSshow::showStd(vector<NFRA_FreqMos> F)
{
  uInt N = F.size();

  if (N > 1)
    cout << "  Freq. Mos. Positions       = " << N << endl;

  for (uInt i = 0; i < N; i++){
    if (N > 1){
      drawLine2();
      cout << "Frequency Mosaic Point: " << i << endl;
      cout << "            Dwell Time [s]   = " << F[i].getDwellTime() << endl;
      cout << endl;
    }
    showStd(F[i].getMFFE());
    showStd(F[i].getIVCs());
  }
}

//----------------------------------------------------------------------
void NFRA_MSshow::showStd(NFRA_MFFE M)
{
  cout << "             MFFE Band       = " << M.getMFFEBand() << endl;
  cout << "         MFFE Sky Freq [MHz] = " << M.getMFFESkyFreq() << endl;
  cout << "       MFFE Band Width [MHz] = " << M.getMFFEBandWidth() << endl;
}

//----------------------------------------------------------------------
void NFRA_MSshow::showStd(vector<NFRA_IVC> I)
{

  uInt N = I.size();
  cout << "   Number of IVC bands       = " << N << endl;
  cout << endl;
  cout << "Name  Freq      RestFrq   Vel   Def     Conv    BW    NC CW         P EF SA" << endl;

  for (uInt i = 0; i < N; i++){

    cout << I[i].getIVCName() << " ";
    cout.setf(std::ios_base::fixed, std::ios_base::floatfield);
    cout.setf(std::ios_base::right, std::ios_base::adjustfield);
    Int p = cout.precision(4);
    cout << setw(9) << I[i].getCenterFreq() << " ";
    if (I[i].getObsMode() == "continuum"){
      cout.precision(p);
      cout.setf(std::ios_base::fmtflags(0), std::ios_base::floatfield);
      cout << "- - - - continuum band - - - -  ";
    } else {
      cout << setw(7) << I[i].getRestFreq() << " ";
      cout.precision(p);
      cout.setf(std::ios_base::fmtflags(0), std::ios_base::floatfield);
      cout.setf(std::ios_base::left, std::ios_base::adjustfield);
      cout << setw(5) << I[i].getVelValue() << " ";
      cout << setw(7) << I[i].getVelDef() << " ";
      cout << setw(7) << I[i].getConvType() << " ";
    }
    cout.setf(std::ios_base::right, std::ios_base::adjustfield);
    cout << setw(3) << I[i].getBandWidth() << " ";
    cout.setf(std::ios_base::right, std::ios_base::adjustfield);
    cout << setw(4) << I[i].getNChan() << " ";
    cout.setf(std::ios_base::left, std::ios_base::adjustfield);
    cout << setw(10) << I[i].getChanWidth() << " ";
    cout.setf(std::ios_base::right, std::ios_base::adjustfield);
    cout << I[i].getNPol() << " ";
    cout << I[i].getEF() << "  ";
    if (I[i].getSubArray() == -1)
      cout << "?";
    else
      cout << I[i].getSubArray();
    if (I[i].getNPol() == 1) cout << I[i].getPolsUsed()[i];
    cout << endl;

  }

}

//----------------------------------------------------------------------
void NFRA_MSshow::showStd()
{

  cout << "Information on Measurementsets: ";
  for (uInt i = 0; i < MSName.size(); i++){
    if (i > 0) cout << endl << "                                ";
    cout << MSName[i];
  }
  cout << endl;
  cout << "             MSversion       = " << MSVersion << endl;
  cout << "       Sequence Number       = " << SeqNr << endl;
  cout << "               Project       = " << Semester << "/" << PrjNr << endl;
  cout << "            Field name       = " << FieldName << endl;
  cout << "            Instrument       = " << Instrument << endl;
  cout << "               BackEnd       = " << BackEnd << endl;
  cout << "                 Epoch       = " << Epoch << endl;
  cout << "       PhaseSwitchMode       = " << PhaseSwitchMode << endl;
  cout << "          Online Taper       = " << IFTaper << endl;  
  cout << "    Rows in MAIN table       = " << NRow << endl;
  cout << "            Set Number       = " << SetNr << endl;

  drawLine1();

  cout << "            Start date       = " << sec2date(DateTimeStart) << endl;
  cout << "            Start time [UT]  = " << sec2time(DateTimeStart) << endl;
  cout << "              End date       = " << sec2date(DateTimeEnd) << endl;
  cout << "              End time [UT]  = " << sec2time(DateTimeEnd) << endl;
  cout << "              Duration ([s]) = " << sec2hms(Duration) << " (" << Duration << ")" << endl;
  cout << "         Exposure Time [s]   = " << VInt2Str(ExpTime) << endl;

  drawLine1();
  showStd(BaseLines);

  drawLine1();
  cout << "POSITION INFORMATION" << endl;
  showStd(PWs);

  drawLine1();
  cout << "FREQUENCY INFORMATION" << endl;
  showStd(FWs);

}

void None()
{
  cout << "None" << endl;
}

//----------------------------------------------------------------------
// Output for the readme file generator
// Output is in strict keyword=value format
//
void NFRA_MSshow::showReadme(vector<NFRA_Position> P, String pre)
{
  uInt N = P.size();
  cout << pre << "NPositions=" << N << endl;
  for (uInt i = 0; i < N; i++){
    cout << pre << "Pos" << i << ".RA=" << rad2hms(P[i].getRA()) << endl;
    cout << pre << "Pos" << i << ".DEC=" << rad2dms(P[i].getDEC()) << endl;
    cout << pre << "Pos" << i << ".Name=" << P[i].getName() << endl;
    cout << pre << "Pos" << i << ".HAStart=" << rad2deg(P[i].getHaMin()) << endl;
    cout << pre << "Pos" << i << ".HAStartHMS=" << rad2hms(P[i].getHaMin()) << endl;
    cout << pre << "Pos" << i << ".HAEnd=" << rad2deg(P[i].getHaMax()) << endl;
    cout << pre << "Pos" << i << ".HAEndHMS=" << rad2hms(P[i].getHaMax()) << endl;
  }

}

void NFRA_MSshow::showReadme(vector<NFRA_PosMos> P, String pre)
{
  uInt N = P.size();
  cout << pre << "NPosMosPoints=" << N << endl;
  for (uInt i = 0; i < N; i++){
    ostringstream s;
    s << pre << "PM" << i << ".";
    String ppre = s.str();
    cout << ppre << "DwellTime=" << P[i].getDwellTime() << endl;
    showReadme(P[i].getPositions(), ppre);
  }
}

void NFRA_MSshow::showReadme(vector<NFRA_PW> P)
{
  uInt N = P.size();
  cout << "NPointingWindows=" << N << endl;
  for (uInt i = 0; i < N; i++){
    ostringstream s;
    s << "PW" << i << ".";
    String pre = s.str();
    cout << pre << "Telescopes=" << P[i].getTelescopes() << endl;
    showReadme(P[i].getPosMoss(), pre);
  }
}

//----------------------------------------------------------------------
void NFRA_MSshow::showReadme(vector<NFRA_IVC> I, String pre)
{
  uInt N = I.size();
  cout << pre << "NIVCs=" << N << endl;
  for (uInt i = 0; i < N; i++){
    cout << pre << "IVC" << i << ".IVCName=" << I[i].getIVCName() << endl;
    Int p = cout.precision(15);
    cout << pre << "IVC" << i << ".IVCFreq=" << I[i].getCenterFreq() << endl;
    cout << pre << "IVC" << i << ".IVCBandWidth=" << I[i].getBandWidth() << endl;
    cout << pre << "IVC" << i << ".ObsMode=" << I[i].getObsMode() << endl;
    cout << pre << "IVC" << i << ".RestFreq=" << I[i].getRestFreq() << endl;
    cout.precision(p);
    cout << pre << "IVC" << i << ".Velocity=" << I[i].getVelValue() << endl;
    cout << pre << "IVC" << i << ".VelDef=" << I[i].getVelDef() << endl;
    cout << pre << "IVC" << i << ".ConvType=" << I[i].getConvType() << endl;
    cout << pre << "IVC" << i << ".NChan=" << I[i].getNChan() << endl;
    cout << pre << "IVC" << i << ".ChanWidth=" << I[i].getChanWidth() << endl;
    cout << pre << "IVC" << i << ".NPol=" << I[i].getNPol() << endl;
    cout << pre << "IVC" << i << ".EFUsed=" << I[i].getEF() << endl;
    cout << pre << "IVC" << i << ".SubArray=" << I[i].getSubArray() << endl;
  }

}

void NFRA_MSshow::showReadme(NFRA_MFFE M, String pre)
{
  cout << pre << "MFFEBand=" << M.getMFFEBand() << endl;
  cout << pre << "MFFESkyFreq=" << M.getMFFESkyFreq() << endl;
  cout << pre << "MFFEBandWidth=" << M.getMFFEBandWidth() << endl;
}

//----------------------------------------------------------------------
void NFRA_MSshow::showReadme(vector<NFRA_FreqMos> F, String pre)
{
  uInt N = F.size();
  cout << pre << "NFreqMosPoints=" << N << endl;
  for (uInt i = 0; i < N; i++){
    ostringstream s;
    s << pre << "FM" << i << ".";
    String ppre = s.str();
    cout << ppre << "DwellTime=" << F[i].getDwellTime() << endl;
    showReadme(F[i].getMFFE(), ppre);
    showReadme(F[i].getIVCs(), ppre);
  }
  
}
//----------------------------------------------------------------------
void NFRA_MSshow::showReadme(vector<NFRA_FW> F)
{
  uInt N = F.size();
  cout << "NFrequencyWindows=" << N << endl;
  for (uInt i = 0; i < N; i++){
    ostringstream s;
    s << "FW" << i << ".";
    String pre = s.str();
    cout << pre << "Telescopes=" << F[i].getTelescopes() << endl;
    showReadme(F[i].getFreqMoss(), pre);
  }
}


//----------------------------------------------------------------------
void NFRA_MSshow::showReadme()
{
  cout << "START_README" << endl;
  cout << "NFRAParam=Y" << endl;

  //
  // global info
  // 'empty' strings are 'None' already
  // 'empty' numeric values must be set to 'None'
  //

  cout << "MSVersion=";
  if (MSVersion == 0) None(); else cout <<  MSVersion << endl;

  cout << "NSubArrays=" << SubArrays.size() << endl;
  for (uInt i = 0; i < SubArrays.size(); i++){
    if (SubArrays[i].getSANr() == -1)
      cout << "S" << i;
    else
      cout << "S" << SubArrays[i].getSANr();
    cout << ".NTimeSlices=" <<  SubArrays[i].getNTimeSlices() << endl;
  }

  cout << "NRow=" << NRow << endl;
  cout << "SetNr=" << SetNr << endl;

  cout << "SeqNr=" <<  SeqNr << endl;
  cout << "Semester=" <<  Semester << endl;
  cout << "PrjNr=" <<  PrjNr << endl;
  cout << "FieldName=" << FieldName << endl;
  cout << "Instrument=" <<  Instrument << endl;
  cout << "BackEnd=" <<  BackEnd << endl;

  cout << "DateStart=";
  if (DateTimeStart == 0) None(); else cout << sec2date(DateTimeStart) << endl;
  cout << "TimeStart=";
  if (DateTimeStart == 0) None(); else cout << sec2time(DateTimeStart) << endl;
  cout << "DateEnd=";
  if (DateTimeEnd == 0) None(); else cout << sec2date(DateTimeEnd) << endl;
  cout << "TimeEnd=";
  if (DateTimeEnd == 0) None(); else cout << sec2time(DateTimeEnd) << endl;

  cout << "Duration=" <<  Duration << endl;
  cout << "DurationHMS=" << sec2hms(Duration) << endl;

  cout << "ExpTime=" << ExpTime[0] << endl;
  cout << "ShortExpTime=" << ExpTime[0] << endl;

  cout << "Epoch=" <<  Epoch << endl;
  cout << "PhaseSwitchMode=" <<  PhaseSwitchMode << endl;
  cout << "IFTaper=" <<  IFTaper << endl;

  uInt N = BaseLines.size();
  cout << "NBaseLines=" << N << endl;
  for (uInt i = 0; i < N; i++){
    cout << "BL" << i << ".Telescope=" << BaseLines[i].getTelescope() << endl;
    cout << "BL" << i << ".Distance=" << BaseLines[i].getDistance() << endl;
    cout << "BL" << i << ".X=" << BaseLines[i].getX() << endl;
    cout << "BL" << i << ".Y=" << BaseLines[i].getY() << endl;
    cout << "BL" << i << ".Z=" << BaseLines[i].getZ() << endl;
  }

  cout << "MaxPositions=" << getNPos() << endl;
  showReadme(PWs);
  cout << "MaxFreqMosPoints=" << getNFreqMos() << endl;
  showReadme(FWs);

  cout << "END_README" << endl;
}

