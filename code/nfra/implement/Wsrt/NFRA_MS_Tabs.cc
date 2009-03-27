//# NFRA_MS_Tabls - classes to access WSRT tables
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
#include <nfra/Wsrt/NFRA_MS.h>

#include <casa/namespace.h>
char TELESCOPES[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

//======================================================================
//======================================================================
// Class definitions for MS tables
//

//======================================================================
// SPECTRAL_WINDOW table
//----------------------------------------------------------------------
// Constructor
// - create an IVC object for each row in the table
// - create a list of unique names
//
NFRA_spwnd::NFRA_spwnd(Table ms)
{
  TableRecord kwds = ms.keywordSet();
  Table t = kwds.asTable(kwds.fieldNumber("SPECTRAL_WINDOW"));
  NBands = t.nrow();
  if (NBands == 0){
    return;
  }

  //
  // Get a pointer to a row with the relevant columns
  //  
  String ColsNeeded = "REF_FREQUENCY";
  ColsNeeded += ",CHAN_WIDTH";
  ColsNeeded += ",NAME";
  ColsNeeded += ",NUM_CHAN";
  ColsNeeded += ",TOTAL_BANDWIDTH";
  ROTableRow row(t, stringToVector(ColsNeeded));

  //
  // Get the relevant info and store in local variables
  //
  for (uInt i = 0; i < NBands; i++){
    NFRA_IVC tmp;
    tmp.setIVCName(row.get(i).asString("NAME"));
    tmp.setCenterFreq(row.get(i).asDouble("REF_FREQUENCY")/MHz);
    tmp.setBandWidth(row.get(i).asDouble("TOTAL_BANDWIDTH")/MHz);
    tmp.setNChan(row.get(i).asInt("NUM_CHAN"));
    Vector<Double> d = row.get(i).asArraydouble("CHAN_WIDTH")/MHz;
    tmp.setChanWidth(d[0]);
    tmp.setNPol(0);
    tmp.setEF("..");
    tmp.setSubArray(0);

    IVCs.push_back(tmp);
  }

  //
  // Sort the NAMES column unique to get the number of mosaicing
  // points
  //
  Bool deleteIt;
  ROScalarColumn<String> NameCol (t, "NAME");
  Vector<String> NameVector = NameCol.getColumn();
  const String* NameSrt = NameVector.getStorage(deleteIt);

  //
  // Sort the NAMES
  // - index will contain sorted indexes into NameCol
  //
  Sort sort;
  sort.sortKey(NameSrt, TpString);
  Vector<uInt> index;
  sort.sort(index, NBands);

  //
  // Sort unique
  // - indexUniq will contain sorted indexes into index
  // - uniq will cotain the number of unique values
  Sort sort2;
  sort2.sortKey(NameSrt, TpString);
  Vector<uInt> indexUniq;
  uInt uniq = sort2.unique(indexUniq, index);

  //
  // Get the unique names
  //
  UniqueNames.resize(uniq);
  for (uInt i = 0; i < uniq; i++){
    UniqueNames.push_back(NameCol(index[indexUniq[i]]));
  }

  //
  // Freq. mosaicing if band NAMES are re-used
  //
  if (NBands != uniq) NIVCSets = NBands / uniq;
  else NIVCSets = 1;
}

//----------------------------------------------------------------------
// show contents
// 
void NFRA_spwnd::dump()
{
  cerr << "Dump of SPECTRAL_WINDOW table info" << endl;
  cerr << "NBands=" << NBands << endl;
  cerr << "NIVCSets=" << NIVCSets << endl;
  cerr << "Unique names:";
  for (uInt i = 0; i < UniqueNames.size(); i++)
    cerr << " " << UniqueNames[i];
  cerr << endl;
  for (uInt i = 0; i < NBands; i++)
    IVCs[i].dump("", i);

}

//======================================================================
// FIELD table
//----------------------------------------------------------------------
// Constructor
// - create a NFRA_Position object for every row
// - get the epoch from the column header
//
NFRA_field::NFRA_field(Table ms)
{
  TableRecord kwds = ms.keywordSet();
  Table t = kwds.asTable(kwds.fieldNumber("FIELD"));

  //
  // Check if there is any data here
  //
  NFields = t.nrow();
  if (NFields == 0){
    return;
  }

  //
  // Get pointer to row with relevant columns and get the data
  //
  ROTableRow row(t, stringToVector("REFERENCE_DIR,NAME"));

  for (uInt i = 0; i < NFields; i++){
    Vector<Double> coords = row.get(i).asArraydouble("REFERENCE_DIR");
    NFRA_Position tmp;
    tmp.setRA(coords[0]);
    tmp.setDEC(coords[1]);
    tmp.setName(row.get(i).asString("NAME"));
    tmp.setDTStart(0);
    tmp.setDTEnd(0);
    Poss.push_back(tmp);
  }

  //
  // Get the epoch from the column header
  //
  ROTableColumn tc(t, "REFERENCE_DIR");
  TableRecord tcr = tc.keywordSet();
  Record ref = tcr.asRecord(tcr.fieldNumber("MEASINFO"));
  Epoch = ref.asString(1);
}

//----------------------------------------------------------------------
void NFRA_field::dump()
{
  cerr << "Dump of FIELD table info:" << endl;
  cerr << "NFields=" << NFields << endl;
  cerr << "Epoch=" << Epoch << endl;
  for (uInt i = 0; i < Poss.size(); i++) Poss[i].dump("", i);
}

//======================================================================
// OBSERVATION table
//----------------------------------------------------------------------
// Constructor
//
NFRA_obs::NFRA_obs(Table ms)
{
  TableRecord kwds = ms.keywordSet();
  Table t = kwds.asTable(kwds.fieldNumber("OBSERVATION"));

  uInt NRow = t.nrow();
  if (NRow == 0){
    return;
  }

  if (NRow != 1){
    cerr << "WARNING - expected only one line in OBSERVATION table." << endl;
    cerr << "          Will use first line." << endl;
  }
  ROTableRow row(t, stringToVector("SCHEDULE"));
  Vector<String> astr = row.get(0).asArrayString("SCHEDULE");

  //
  // Format of line:
  // ["<semester>/<prj.nr.>/<name>", "<Instrument>   <nr>", ....]
  // [         astr[0]             ,         astr[1]      , ....]
  // Extract semester, prj.nr., and Instrument
  //
  // Initialize all properties to 'empty' values.
  //
  Semester = "None";
  PrjNr = "None";
  FieldName = "None";

  Int i = astr[0].find('/');
  if (i > 0){
    Semester = astr[0].substr(0, i);
    Int j = astr[0].find('/', i+1);
    if (j > 0){
      PrjNr = astr[0].substr(i+1, j-i-1);
      FieldName = astr[0].substr(j+1);
    }
  }

  i = astr[1].find(' ');
  if (i > 0) Instrument = astr[1].substr(0,i);

}

//----------------------------------------------------------------------
void NFRA_obs::dump()
{
  cerr << "Dump of OBSERVATION_TABLE table info" << endl;
  cerr << "Instrument=" << Instrument << endl;
  cerr << "Semester=" << Semester << endl;
  cerr << "PrjNr=" << PrjNr << endl;
  cerr << "FieldName=" << FieldName << endl;
}

//======================================================================
// PROCESSOR table
//----------------------------------------------------------------------
//
NFRA_proc::NFRA_proc(Table ms)
{
  TableRecord kwds = ms.keywordSet();
  Table t = kwds.asTable(kwds.fieldNumber("PROCESSOR"));

  uInt NRow = t.nrow();
  if (NRow == 0){
    return;
  }
  
  if (NRow != 1){
    cerr << "WARNING - expected only one line in PROCESSOR table." << endl;
    cerr << "          Will use first line." << endl;
  }

  //
  // The backend is listed in the SUB_TYPE column
  //
  ROTableRow row(t, stringToVector("SUB_TYPE"));
  BackEnd = row.get(0).asString("SUB_TYPE");
}

//----------------------------------------------------------------------
void NFRA_proc::dump()
{
  cerr << "Dump of PROCESSOR table info" << endl;
  cerr << "BackEnd=" << BackEnd << endl;
}


//======================================================================
// ANTENNA table
//----------------------------------------------------------------------
// Constructor
// - get the baselines from 9 to all movable A...F
//
NFRA_antenna::NFRA_antenna(Table ms)
{
  TableRecord kwds = ms.keywordSet();
  Table t = kwds.asTable(kwds.fieldNumber("ANTENNA"));

  uInt NRow = t.nrow();
  if (NRow == 0) return;

  //
  // select necessary columns
  //
  String C = "POSITION";
  C += ",NAME";
  ROTableRow row(t, stringToVector(C));
  AntError = 0;

  //
  // The format of the POSITION cells is Vector<Double>, with size=3
  // Get the data for the rows for telescopes 9, A, B, C, D, E, F
  //
  Vector<Double> tmp;
  //
  // in the end we need to know for which telescopes we have data ...
  //
  Bool f9 = False;
  Bool fa = False;
  Bool fb = False;
  Bool fc = False;
  Bool fd = False;
  Bool fe = False;
  Bool ff = False;

  //
  // Create the baseline objects
  //
  NFRA_BaseLine rt9('9');
  NFRA_BaseLine rta('A');
  NFRA_BaseLine rtb('B');
  NFRA_BaseLine rtc('C');
  NFRA_BaseLine rtd('D');
  NFRA_BaseLine rte('E');
  NFRA_BaseLine rtf('F');

  //
  // Read all the rows
  // Store in correct BaseLine objects
  //
  for (uInt i = 0; i < NRow; i++){
    String Name = row.get(i).asString("NAME");
    if (Name == "RT9"){
      Vector<Double> tmp = row.get(i).asArrayDouble("POSITION");
      rt9.setXYZ(tmp);
      f9 = True;
    } else if (Name == "RTA"){
      Vector<Double> tmp = row.get(i).asArrayDouble("POSITION");
      rta.setXYZ(tmp);
      fa = True;
    } else if (Name == "RTB"){
      Vector<Double> tmp = row.get(i).asArrayDouble("POSITION");
      rtb.setXYZ(tmp);
      fb = True;
    } else if (Name == "RTC"){
      Vector<Double> tmp = row.get(i).asArrayDouble("POSITION");
      rtc.setXYZ(tmp);
      fc = True;
    } else if (Name == "RTD"){
      Vector<Double> tmp = row.get(i).asArrayDouble("POSITION");
      rtd.setXYZ(tmp);
      fd = True;
    } else if (Name == "RTE"){
      Vector<Double> tmp = row.get(i).asArrayDouble("POSITION");
      rte.setXYZ(tmp);
      fe = True;
    } else if (Name == "RTF"){
      Vector<Double> tmp = row.get(i).asArrayDouble("POSITION");
      rtf.setXYZ(tmp);
      ff = True;
    }

  }

  //
  // For all telescopes:
  // - store the properties relative to RT9 in the vector
  //
  if (!f9){
    AntError = 9;
    return;
  }

  //
  // The NFRA_BaseLine object has a -= operator that calculates the
  // x,y,z, differences
  //
  if (fa){
    rta -= rt9;
    BaseLines.push_back(rta);
  }
  if (fb){
    rtb -= rt9;
    BaseLines.push_back(rtb);
  }
  if (fc){
    rtc -= rt9;
    BaseLines.push_back(rtc);
  }
  if (fd){
    rtd -= rt9;
    BaseLines.push_back(rtd);
  }
  if (fe){
    rte -= rt9;
    BaseLines.push_back(rte);
  }
  if (ff){
    rtf -= rt9;
    BaseLines.push_back(rtf);
  }
}

//----------------------------------------------------------------------
void NFRA_antenna::dump()
{
  cerr << "Dump of ANTENNA table info" << endl;
  for (uInt i = 0; i < BaseLines.size(); i++) BaseLines[i].dump(i);

}

//======================================================================
// POLARIZATION table
//----------------------------------------------------------------------
//
NFRA_pol::NFRA_pol(Table ms)
{
  TableRecord kwds = ms.keywordSet();
  Table t = kwds.asTable(kwds.fieldNumber("POLARIZATION"));

  uInt nrow = t.nrow();
  if (nrow != 1){
    cerr << "WARNING - expected only one line in POLARIZATION table." << endl;
    cerr << "          Will use first line." << endl;
  }

  //
  // Column NUM_CORR gives the number of polarizations used,
  // Column CORR_TYPE gives the actual values (XX, ...)
  //
  ROTableRow row(t, stringToVector("NUM_CORR,CORR_TYPE"));
  NPols = row.get(0).asInt("NUM_CORR");
  Vector<Int> stks = row.get(0).asArrayInt("CORR_TYPE");

  //
  // The polarizations are given as integers, 9=XX, ..., 12=YY
  //
  Vector<String> S = stringToVector("XX,XY,YX,YY");
  for (uInt i = 0; i < NPols; i++) PolsUsed.push_back(S[stks[i]-9]);
}

//----------------------------------------------------------------------
void NFRA_pol::dump()
{
  cerr << "Dump of POLARIZATION table info" << endl;
  cerr << "NPols=" << NPols << endl;
  cerr << "PolsUsed=";
  for (uInt i = 0; i < PolsUsed.size(); i++) cerr << " " << PolsUsed[i];
  cerr << endl;
}

//======================================================================
// NFRA_TMS_PARAMETERS table
//----------------------------------------------------------------------
//----------------------------------------------------------------------
// Constructor -  Open table without and with SubArray number
//
NFRA_parameters::NFRA_parameters(Table ms)
{
  SubArray = 0;
  TableRecord kwds = ms.keywordSet();
  t = kwds.asTable(kwds.fieldNumber("NFRA_TMS_PARAMETERS"));
}
NFRA_parameters::NFRA_parameters(Table ms, Int SubA)
{
  if (SubA < 0) SubArray = 0;
  else SubArray = SubA;
  TableRecord kwds = ms.keywordSet();
  t = kwds.asTable(kwds.fieldNumber("NFRA_TMS_PARAMETERS"));
}

//----------------------------------------------------------------------
// Do a full Position investigation
//
void NFRA_parameters::FullPositionScan()
{
  //
  // Positions structure is PointingWindow
  //                        - (has multiple) - Position Mosaic
  //                        - (has multiple) - Multi Beam
  //

  //
  // Pointing window keywords are PW<n>.<keyword>, 0 < n < MAX_TELESCOPES;
  //
  // First find out how many PWs are active. This must be done by
  // inspecting the Pointing Window that is attached to a Telescope:
  // RT<n>.PointingWindow=PW<m>. This gives:
  // - the number of PWs that are used
  // - the telescopes that are bound to a PW
  //
  // There can be as many PWs as there are telescopes
  // For every PW found an NFRA_PW object is pushed into the vector
  //
  {
    //
    // Use a temp. vector to see if a PW has been seen before
    //
    vector<uInt> PWnr;
    for (uInt rt = 0; rt < MAX_TELESCOPES; rt++){
      ostringstream kwd;
      kwd << "RT" << TELESCOPES[rt] << ".PointingWindow";
      String v = getVal(kwd.str());
      if (v != "None"){
        uInt nr = atoi(v.substr(2).c_str());
        Bool found = False;
        for (uInt i = 0; i < PWnr.size(); i++){
          if (nr == PWnr[i]) found = True;
        }
        if (!found){
          PWnr.push_back(nr);
          NFRA_PW tmp;
          PWs.push_back(tmp);
        }
      }
    }
  }

  //
  // For each PW - get the Telescopes, PosMos points and MBeams
  //
  for (uInt pw = 0;pw < PWs.size(); pw++){

    //
    // telescopes that belong to a PW must be found through
    // RT<n>.PointingWindow
    // The value is PW<n>
    //
    // Note that we loop again over RT<n>.PointingWindow, but now
    // dedicated for a certain PW
    //
    ostringstream kwdName;
    kwdName << "PW" << pw+1;
    String pwName = kwdName.str();
    for (uInt t = 0; t < MAX_TELESCOPES; t++){
      ostringstream kwd;
      kwd << "RT" << TELESCOPES[t] << ".PointingWindow";
      String v = getVal(kwd.str());
      if (v == pwName)
        PWs[pw].addTelescope(TELESCOPES[t]);
    }

    //
    // Get the value of .RightAscension, .Declination, and .SourceName
    // into vectors. We need them later.
    // Use the keywords: .RAList, .DECList, .SourceName
    //
    // Note that the size of the vectors give the number of PosMos points.
    //
    vector<Double> RA;
    vector<Double> DEC;
    vector<String> Name;

    {
      ostringstream kwd;
      kwd << "PW" << pw+1 << ".RAList";
      String v = getVal(kwd.str());
      RA = CommaSepStr2vDbl(v);
    }
    {
      ostringstream kwd;
      kwd << "PW" << pw+1 << ".DecList";
      String v = getVal(kwd.str());
      DEC = CommaSepStr2vDbl(v);
    }
    {
      ostringstream kwd;
      kwd << "PW" << pw+1 << ".SourceName";
      String v = getVal(kwd.str());
      Name = CommaSepStr2vStr(v);
    }
    Bool OneName = Name.size() == 1;

    //
    // Check if we have MultiBeam
    // NMB gives the number of MultiBeam points
    //
    uInt NMB;
    {
      ostringstream kwd;
      kwd << "PW" << pw+1 << ".MultiBeam";
      String v = getVal(kwd.str());
      NMB = atoi(v.c_str());
    }

    //
    // Dwell time, different keywords for PosMos ore MultiBeam
    // multiplicity.
    //
    vector<uInt> Dwell;
    {
      ostringstream kwd;
      if (NMB == 0)
        kwd << "PW" << pw+1 << ".DwellTime";
      else
        kwd << "PW" << pw+1 << ".DwellBand";
      String v = getVal(kwd.str());
      Dwell = CommaSepStr2vuInt(v);
    }
    Bool OneDwell = Dwell.size() == 1;

    //
    // For Multi Beam, the RAList and DECList vectors contains only one
    // value. However, when the PW<n>.RABand0 contains multiple values
    // there are in fact that many PosMos points because the positions
    // are observer sequentially.
    //
    if (NMB == 0){
      //
      // No Multiple Beams.
      // Take all RA, DEC values as PosMos Points.
      // Every PosMos has one Position
      //
      // For all PosMos points
      //
      for (uInt pm = 0; pm < RA.size(); pm++){

	//
	// Create a NFRA_PosMos object
	// - store DwellTime
	//
        NFRA_PosMos PM;
        if (!OneDwell)
          PM.setDwellTime(Dwell[pm]);
        else
          PM.setDwellTime(Dwell[0]);

	//
	// Create one (1) NFRA_Position object
	// - Store items
	// 
        NFRA_Position P;
        P.setRA(RA[pm]);
        P.setDEC(DEC[pm]);
        if (P.getRA() < 0) P.setRA(P.getRA() + 360.0);
        if (!OneName)
          P.setName(Name[pm]);
        else
          P.setName(Name[0]);
        PM.addPosition(P);
        PWs[pw].addPosMos(PM);

      }
    } else {
      //
      // It is Multi Beam.
      // All PosMos positions are equal RA[0], DEC[0].
      // There are as many PosMos points as there are values for
      // RABand0.
      //
      uInt NPM;
      {
        ostringstream kwd;
        kwd << "PW" << pw+1 << ".RABand0";
        String v = getVal(kwd.str());
        vector<Double> ra0 = CommaSepStr2vDbl(v);
        NPM = ra0.size();
      }

      //
      // Create all the PMs
      //
      for (uInt pm = 0; pm < NPM; pm++){
        NFRA_PosMos PM;
        if (OneDwell > 1)
          PM.setDwellTime(Dwell[pm]);
        else
          PM.setDwellTime(Dwell[0]);
        PWs[pw].addPosMos(PM);
      }

      //
      // Get all the MBs,
      // Spread over the PMs.
      //
      for (uInt b = 0; b < NMB; b++){
        vector<Double> ra;               // RAs for this Beam
        vector<Double> dec;              // DECs for this Beam
        {
          ostringstream kwd;
          kwd << "PW" << pw+1 << ".RABand" << b;
          String v = getVal(kwd.str());
          ra = CommaSepStr2vDbl(v);
        }
        {
          ostringstream kwd;
          kwd << "PW" << pw+1 << ".DecBand" << b;
          String v = getVal(kwd.str());
          dec = CommaSepStr2vDbl(v);
        }

        //
        // The p-th value of the ra and dec vectors are the values for
        // the p-th Mosaic Point
        // RA must be > 0
        // Values are in deg - convert to rad
        //
        for (uInt p = 0; p < NPM; p++){
          NFRA_Position pos;
	  if (ra[p] < 0) ra[p] += 360.0;
          pos.setRA(deg2rad(ra[p]));
          pos.setDEC(deg2rad(dec[p]));
          if (Name.size() != NPM)
	    pos.setName(Name[0]);
          else
	    pos.setName(Name[p]);
          PWs[pw].addToPosMos(p, pos);
        }
      }
    }
  }

}
//----------------------------------------------------------------------
// Do a full Frequency investigation
//
void NFRA_parameters::FullFrequencyScan()
{
  //  cerr << "DEBUG - FullFrequencyScan" << endl;
  //
  // Frequency structure is Frequency Window - Frequency Mosaic - IVC
  //

  //
  // Keep a vector of active FW and their modes
  //
  vector<String> LMode;

  //
  // Frequency window keywords are FW<n>.<keyword>, n > 0;
  //
  // First find out how many FWs are active. Non-active FWs have the
  // keyword .Mode set to 'passive'
  // There can be as many FWs as there are telescopes
  // For every FW found a NFRA_FW object is pushed into the vector
  //
  {
    Bool eol = False;
    uInt i = 1;
    while ((i <= MAX_TELESCOPES) && (!eol)){
      ostringstream kwd;
      kwd << "FW" << i << ".Mode";
      String v = getVal(kwd.str());

      //
      // If FW<i> does not exist or is passive, don't look anu further
      //
      if ((v == "passive") || ( v == "None")){
        eol = True;
      } else {
        NFRA_FW tmp;
        FWs.push_back(tmp);
        LMode.push_back(v);
      }
      i++;
    }
  }

  //
  // Number of active Freq.Windows
  //
  uInt NFW = FWs.size();

  //
  // Telescopes that belong to a FW must be found through
  // WSRT-IF.Telescopes
  // This has as many words as there are FWs, give the correct words
  // to the FWs.
  //
  {
    ostringstream kwd;
    kwd << "WSRT-IF.Telescopes";
    String v = getVal(kwd.str());
    vector<String> Tels = CommaSepStr2vStr(v);
    for (uInt i = 0; i < NFW; i++){
      FWs[i].setTelescopes(Tels[i]);
    }
  }

  //
  // Get MFFE info
  // MultiFreq and FreqMos excludes each other (JAN2006)
  // So for multiple FWs MFFE[i] gives the info for FW i
  // and for FreqMos MFFE[i] gives the info for FM i.
  // SkyFreq[i] and BWidth[i] give info for MFFE[i]
  //
  vector<String> MFFE;
  vector<Double> SkyFreq;
  vector<Double> BWidth;
  {
    ostringstream kwd;
    kwd << "WSRT-IF.MFFEBand";
    String v = getVal(kwd.str());
    MFFE = CommaSepStr2vStr(v);

    //    cerr << "WSRT-IF.MFFEBand=" << v << ", " << MFFE[0] << endl;

  }
  {
    ostringstream kwd;
    kwd << "WSRT-IF.MFFESkyFreq";
    String v = getVal(kwd.str());
    SkyFreq = CommaSepStr2vDbl(v);

    //    cerr << "WSRT-IF.MFFESkyFreq=" << v << ", " << SkyFreq[0] << endl;

  }
  {
    ostringstream kwd;
    kwd << "WSRT-IF.ReqMFFEWidth";
    String v = getVal(kwd.str());
    BWidth = CommaSepStr2vDbl(v);

    //    cerr << "WSRT-IF.ReqMFFEWidth=" << v << ", " << BWidth[0] << endl;

  }

  //
  // For each FW - get FreqMos points and IVCs
  //
  for (uInt fw = 0; fw < NFW; fw++){

    //    cerr << "DEBUG - FW=" << fw << " of " << NFW-1 << endl;

   //
    // Get number of FreqMos positions
    //
    uInt NFreqMos;
    {
      ostringstream kwd;
      kwd << "FW" << fw+1 << ".Positions";
      String v = getVal(kwd.str());
      NFreqMos = atoi(v.c_str());
    }

    //
    // Get dwell times
    //
    vector<uInt> Dwell;
    {
      ostringstream kwd;
      kwd << "FW" << fw+1 << ".DwellTime";
      String v = getVal(kwd.str());
      Dwell = CommaSepStr2vuInt(v);
    }
    Bool OneDwell = Dwell.size() == 1;

    //
    // Create the FreqMos points
    //
    for (uInt fm = 0; fm < NFreqMos; fm++){

      //      cerr << "DEBUG - FreqMos=" << fm << " of " << NFreqMos-1 << endl;

      NFRA_FreqMos FM;

      //
      // Set Dwell time
      //
      if (OneDwell == 1)
        FM.setDwellTime(Dwell[0]);
      else
        FM.setDwellTime(Dwell[fm]);

      //
      // Add MFFE info
      // If there are multiple FreqWindows, then the MFFE specs must be
      // indexed with the FW index, otherwise with the FreqMos index.
      //

      NFRA_MFFE M;

      if (NFW == 1){

        //      cerr << "NFW=" << NFW << ", fm=" << fm << endl;

        M.setMFFEBand(MFFE[fm]);
        M.setMFFESkyFreq(SkyFreq[fm]);
        M.setMFFEBandWidth(BWidth[fm]);
      } else {

        //      cerr << "NFW=" << NFW << ", fm=" << fw << endl;

        M.setMFFEBand(MFFE[fw]);
        M.setMFFESkyFreq(SkyFreq[fw]);
        M.setMFFEBandWidth(BWidth[fw]);
      }

      FM.setMFFE(M);


      //      cerr << "DEBUG - MFFE OK" << endl;

      //
      // IVC information
      //
      // Get bandwidth and channel info
      // These are multiple values if there are multiple SubArrays,
      // take the values that belong to the current one.
      //
      // The number of channels is the number of lags / 2
      //
      Double IVCw;
      uInt NChan;
      {
        ostringstream kwd;
        kwd << "IVC-IF0.IVCReqBand";
        String v = getVal(kwd.str());
        IVCw = atof(v.c_str());
      }
      {
        ostringstream kwd;
        kwd << "DZB20-COR.ReqLags";
        String v = getVal(kwd.str());
        vector<String> vv = CommaSepStr2vStr(v);
        NChan = atoi(vv[SubArray].c_str())/2;
      }

      //      cerr << "DEBUG - BandW. and Ch. OK" << endl;

      //
      // Get Polarization info for this SubArray
      //
      uInt NPol;
      vector<String> PolsUsed;
      {
        ostringstream kwd;
        kwd << "DZB20-COR.ReqPolarizations";
        String v = getVal(kwd.str());
        vector<String> vv = CommaSepStr2vStr(v);
        NPol = atoi(vv[SubArray].c_str());
        Vector<String> S = stringToVector("XX,XY,YX,YY");
        for (uInt i = 0; i < NPol; i++)
          PolsUsed.push_back(S[i]);
      }

      //      cerr << "DEBUG - Pols. OK" << endl;

      //
      // Get EF status for this SubArray
      //
      String EF;
      {
        ostringstream kwd;
        kwd << "DZB20-COR.ReqAntennas";
        String v = getVal(kwd.str());
        vector<String> vv = CommaSepStr2vStr(v);
        EF  = vv[SubArray].substr(14);
      }

      //      cerr << "DEBUG - EF OK" << endl;

      //
      // Get Center Freqs
      // - in CONTINUUM mode: take the fw-th value from the lists
      //   FW<n>.SkyFreq<i>
      // - in LINE mode: take the frequencies from the list
      //   FW<n>.GeoSkyFreq
      // - In line mode there is NO FreqMos possible
      //
      vector<Double> CenterFreq;
      if (LMode[fw] == "line"){
        ostringstream kwd;
        kwd << "FW" << fw+1 << ".GeoSkyFreq";
        String v = getVal(kwd.str());
        CenterFreq = CommaSepStr2vDbl(v);
      } else {
        for (uInt ivc = 0; ivc < 8; ivc++){
          ostringstream kwd;
          kwd << "FW" << fw+1 << ".SkyFreq" << ivc;
          String v = getVal(kwd.str());
          vector<Double> tmp = CommaSepStr2vDbl(v);
          CenterFreq.push_back(tmp[fm]);
        }
      }

      //      cerr << "DEBUG - CenterFreq OK" << endl;

      //
      // Get RestFrequencies, Tracking speeds, vel. definitions and conversion
      //
      vector<Double> RestFreq;
      vector<Double> TrackSpeed;
      vector<String> VelDef;
      vector<String> ConvType;
      if (LMode[fw] == "line"){
        //      cerr << "DEBUG - get vel.info" << endl;

        //
        // Restfrequency is a list
        //
        {
          ostringstream kwd;
          kwd << "FW" << fw+1 << ".RestFrequency";
          String v = getVal(kwd.str());
          RestFreq = CommaSepStr2vDbl(v);
        }

        //
        // Tracking speed is per IVC
        //      
        for (uInt ivc = 0; ivc < 8; ivc++){
          ostringstream kwd;
          kwd << "FW" << fw+1 << ".TrackingSpeed" << ivc;
          String v = getVal(kwd.str());
          TrackSpeed.push_back(atof(v.c_str()));
        }

        //
        // vel.def. is a list
        //
        {
          ostringstream kwd;
          kwd << "FW" << fw+1 << ".VelocDefinition";
          String v = getVal(kwd.str());
          //      cerr << "DEBUG - " << kwd.str() << "=" << v << endl;
          VelDef = CommaSepStr2vStr(v);
        }

        //
        // ConvType is a list
        //
        {
          ostringstream kwd;
          kwd << "FW" << fw+1 << ".ConversionType";
          String v = getVal(kwd.str());
          //      cerr << "DEBUG - " << kwd.str() << "=" << v << endl;
          ConvType = CommaSepStr2vStr(v);
        }

      } else {

        //      cerr << "DEBUG - continuum mode" << endl;

        //
        // Continuum mode
        // -> No data ...
        //
        for (uInt z = 0; z < 8; z++){
          RestFreq.push_back(0);
          TrackSpeed.push_back(0);
          VelDef.push_back("");
          ConvType.push_back("");
        }
      }

      //      cerr << "DEBUG - " << RestFreq.size() << ", " << TrackSpeed.size() << ", " << VelDef.size() << ", " << ConvType.size() << endl;

      //
      // Find the IVCs that belong to this SubArray
      // Value of keyword is a multi dim. list
      //
      vector<uInt> IVCnr;
      {
        ostringstream kwd;
        kwd << "DZB20-COR.CurSubbands";
        String v = getVal(kwd.str());
        vector<String> vv = CommaSepStr2vStr(v);
        //
        // get all the individual values for the SubArray
        //
        for (uInt i = 0; i < vv[SubArray].size(); i++){
          uInt q = atoi(vv[SubArray].substr(i,1).c_str());
          IVCnr.push_back(q);
        }
      }

      //      cerr << "DEBUG - IVC info OK\n";

      //
      // Create the IVCs
      //
      for (uInt ivc = 0; ivc < IVCnr.size(); ivc++){
        //      cerr << "DEBUG - " << ivc << " - " << IVCnr[ivc] << endl;
        NFRA_IVC IVC;
        ostringstream nme;
        nme << "IVC-" << IVCnr[ivc];
        IVC.setIVCName(nme.str());
        //
        // IVCnr contains all the IVC numbers that are operational
        // IVCnr[ivc] is the index to use for the values that differ per IVC
        //
        IVC.setCenterFreq(CenterFreq[IVCnr[ivc]]);
        if (VelDef[IVCnr[ivc]] == "const")
          IVC.setObsMode("continuum");
        else
          IVC.setObsMode(LMode[fw]);
        IVC.setRestFreq(RestFreq[IVCnr[ivc]]);
        IVC.setVelDef(VelDef[IVCnr[ivc]]);
        IVC.setConvType(ConvType[IVCnr[ivc]]);
        IVC.setVelValue(TrackSpeed[IVCnr[ivc]]);
        IVC.setBandWidth(IVCw);
        IVC.setNChan(NChan);
        IVC.setChanWidth(IVCw/NChan);
        IVC.setNPol(NPol);
        IVC.setPolsUsed(PolsUsed);
        IVC.setEF(EF);
        IVC.setSubArray(SubArray);
        FM.addIVC(IVC);
      }

      FWs[fw].addFreqMoss(FM);
      
    }
  } // for (uInt fw = 0; fw < NFW; fw++){
}


//----------------------------------------------------------------------
// Get a value from the NFRA_TMS_PARAMETERS table.
// Enter with the keyword.
// If kwd=all, then the complete table will be returned.
//
String NFRA_parameters::getVal(String kwd)
{
  if (kwd == "all"){

    ROScalarColumn<String> NameCol (t, "NAME");
    ROScalarColumn<String> ValueCol (t, "VALUE");

    String rtn = "";
    for (uInt i = 0; i < t.nrow(); i++){
      rtn += NameCol(i) + "=" + ValueCol(i) + '\n';
    }
    return rtn;

  } else {

    Table sel = t(t.col("NAME") == kwd);
    if (sel.nrow() == 0){
      return "None";
    }
    return ROScalarColumn<String>(sel, "VALUE")(0);

  }

}

//----------------------------------------------------------------------
//
void NFRA_parameters::dump()
{
  cerr << "Dump of PARAMETERS table info" << endl;
  //  cerr << "SeqNr=" << SeqNr << endl;
  cerr << "NPointingWindows=" << PWs.size() << endl;
  for (uInt i = 0; i < PWs.size(); i++) PWs[i].dump(i);
  cerr << "NFrequencyWindows=" << FWs.size() << endl;
  for (uInt i = 0; i < FWs.size(); i++) FWs[i].dump(i);
}

//======================================================================
// main table
//----------------------------------------------------------------------
NFRA_main::NFRA_main(String MSName, Double RAc, Double DECc)
{

  tName = MSName;
  //
  // Sort TIME column
  //
  const MeasurementSet& tmp(MSName);
  ROMSColumns itsMsc(tmp);
  const ROScalarColumn<Double>& timeCol = itsMsc.time();
  Vector<Double> times = timeCol.getColumn();
  Int NRow = times.shape()[0];
  Bool deleteIt;
  const Double* timesData = times.getStorage (deleteIt);

  Sort tsort;
  tsort.sortKey(timesData, TpDouble);
  Vector<uInt> index;
  tsort.sort(index, NRow);
  Sort utsort;
  utsort.sortKey(timesData, TpDouble);
  Vector<uInt> indexU;
  uInt uniq = utsort.unique(indexU, index);
  Vector<Double> uniqTimes;
  uniqTimes.resize(uniq);
  for (uInt i = 0; i < uniq; i++) uniqTimes[i] = times[index[indexU[i]]];

  DateTimeStart = uniqTimes[0];
  DateTimeEnd = uniqTimes[uniq-1];

  //
  // Sort Exposure column
  //
  const ROScalarColumn<Double>& expCol = itsMsc.exposure();
  Vector<Double> exps = expCol.getColumn();
  const Double* expData = exps.getStorage (deleteIt);

  Sort esort;
  esort.sortKey(expData, TpDouble);
  Vector<uInt> eindex;
  esort.sort(eindex, NRow);
  Sort uesort;
  uesort.sortKey(expData, TpDouble);
  Vector<uInt> eindexU;
  uInt euniq = uesort.unique(eindexU, eindex);

  ExpTime.resize(euniq);
  for (uInt i = 0; i < euniq; i++) ExpTime[i] = uInt(exps[eindex[eindexU[i]]]);

  Duration = DateTimeEnd - DateTimeStart + ExpTime[0];

  HAStart = calcHA(DateTimeStart, RAc, DECc);
  HAEnd = calcHA(DateTimeEnd, RAc, DECc);

}

//----------------------------------------------------------------------
Double NFRA_main::getDTStart(uInt Fid)
{
  Table tbl(tName);
  const ROScalarColumn<Int> fldCol(tbl, "FIELD_ID");
  Vector<Int> fld = fldCol.getColumn();
  Bool deleteIt;
  const Int* fldData = fld.getStorage(deleteIt);
  
  const ROScalarColumn<Int> tmeCol(tbl, "TIME");
  Vector<Int> tme = tmeCol.getColumn();
  const Int* tmeData = tme.getStorage(deleteIt);
  
  Sort sort;
  sort.sortKey(fldData, TpInt);
  sort.sortKey(tmeData, TpDouble);
  Vector<uInt> index;
  Int NRow = tbl.nrow();
  sort.sort(index, NRow);
  Sort usort;
  usort.sortKey(fldData, TpInt);
  usort.sortKey(tmeData, TpInt);
  Vector<uInt> indexU;
  uInt uniq = usort.unique(indexU, index);
  for (uInt i = 0; i < uniq; i++) cerr << fld[index[indexU[i]]] << '-' << tme[index[indexU[i]]] << ' ';

  return 0.0;

}
