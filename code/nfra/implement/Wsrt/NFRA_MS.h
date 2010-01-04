//# NFRA_MS.h: header file for NFRA_MS info class
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
// Defines:
// - class NFRA_MS
//
//

#ifndef NFRA_MS_H
#define NFRA_MS_H


//# Includes

#include <casa/namespace.h>

#include <nfra/Wsrt/NFRA_MS_PW.h>
#include <nfra/Wsrt/NFRA_MS_FW.h>
#include <nfra/Wsrt/NFRA_Misc.h>
#include <nfra/Wsrt/NFRA_MS_Tabs.h>

#define MHz 1000000.0
#define MAX_TELESCOPES 16


//======================================================================
class NFRA_MS {

//----------------------------------------------------------------------
// Publics
//
 public:
  NFRA_MS(){init();}
  ~NFRA_MS(){}

  enum Method {NFRA, MEAS, MAIN};
  Int maxMethod;
  Bool setMethod(Method);

  //
  // get<property> methods
  //
  vector<String> getMSName(){return MSName;}
  vector<NFRA_SubArray> getSubArrays(){return SubArrays;}
  Float getMSVersion(){return MSVersion;}
  String getSeqNr() {return SeqNr;}
  String getInstrument() {return Instrument;}
  Int getNRow() {return NRow;}
  String getSetNr() {return SetNr;}
  String getSemester() {return Semester;}
  String getPrjNr() {return PrjNr;}
  String getFieldname() {return FieldName;}
  String getBackEnd() {return BackEnd;}
  String getPhaseSwitchMode() {return PhaseSwitchMode;}
  String getIFTaper() {return IFTaper;}
  vector<NFRA_BaseLine> getBaseLines() {return BaseLines;}
  vector<Double> getBaseLineValues();
  Double getDateTimeStart() {return DateTimeStart;}
  Double getDateTimeEnd() {return DateTimeEnd;}
  Double getDuration() {return Duration;}
  vector<Int> getExpTime() {return ExpTime;}
  String getEpoch() {return Epoch;}
  vector<NFRA_PW> getPWs() {return PWs;}
  vector<NFRA_FW> getFWs() {return FWs;}

  //
  // Different counters
  //
  uInt getNPos();          // Total number of positions, added over all 
                           // Pointing Windows
  uInt getNFreqMos();      // Total number of Frequency Mosaicing points, 
                           // added over all Frequency Windows

  Bool isFilled(){return isFilledVar;}
                           // isFilledVar is True when the info has been
                           // read from the MS
  Bool setInfo(String);    // Read info from MS, enter with MSName
  Bool mergeInfo(String);  // Merge info from MS, enter with MSName
  String getNFRAkwd(String, String);
                           // get a kwd=value from the NFRA_TMS_PARAMETERS
                           // table, enter with MSName and kwd

  Double getHaMin();
  Double getHaMax();
  Double getRACentroid();
  Double getDECCentroid();
  Double getPWDwellTime(){return PWs[0].getDwellTime();}
  Double getFWDwellTime(){return FWs[0].getDwellTime();}
  String getMFFEName(){return FWs[0].getMFFEName();}
  Int getNIVCBands(){return FWs[0].getNIVCBands();}
  vector<Double> getCenterFreqs(){return FWs[0].getCenterFreqs();}
  Double getIFBandWidth(){return FWs[0].getIFBandWidth();}
  Int getNChan(){return FWs[0].getNChan();}
  Double getChanWidth(){return FWs[0].getChanWidth();}
  Int getNPol(){return FWs[0].getNPol();}
  String getPolsUsed(){return FWs[0].getPolsString();}

  //
  // internal error number and string
  //
  Int getError(){return MSError;}
  String getErrStr(){return ErrStr;}

  void dump();             // dump property values to stdout
 protected:
  //
  // Class variables
  //
  Int MSError;             // Internal Error number,
  String ErrStr;           // Internal Error string,
  Bool isFilledVar;        // Flag if MSinfo has been gathered,
  Method useMethod;        // How should the info be gathered, from 
                           // NFRA_PARAMATERS table or from MAIN table,

  //
  // MS-related variables
  //
  vector<String> MSName;    // Names of MS in the data set
  vector<NFRA_SubArray> SubArrays;
                            // Number of timeslices per SubArrays for the 
                            // Sequence number
  Float MSVersion;          // Version of MS (1 or 2), is a float because it is
                            // one for aips++
  String SeqNr;             // Seq.Nr of measurement
  String Instrument;        // TMS instrument
  Int NRow;                 // Number of rows in MAIN table
  String SetNr;             //
  String Semester;          // Semester code of measurement
  String PrjNr;             // Project code of measurement
  String FieldName;         // Field name of observation
  String BackEnd;           // backend used
  String PhaseSwitchMode;   // PhaseSwitchMode used
  String IFTaper;           // Online taper used
  vector<NFRA_BaseLine> BaseLines;
                            // Baselines to movable telescopes
  Double DateTimeStart;     // Date/Time of first sample
  Double DateTimeEnd;       // Date/Time of last sample
  Double Duration;          // Duration [s] of measurement
  vector<Int> ExpTime;      // All exposure times used
  String Epoch;             // Epoch of coordinates
  vector<NFRA_PW> PWs;      // Pointing Windows

  vector<NFRA_FW> FWs;      // Frequency Windows

  Int getSubArray(String);  // Get the SubArray number from the MSname

  void init();              // Initialise,

  Bool setInfo_NFRA(String);    // Gather info from NFRA_PRM table
  Bool mergeInfo_NFRA(String);  // Merge info from NFRA_PRM table
  Bool setInfo_MEAS(String);    // Gather info from STFMA file
  Bool mergeInfo_MEAS(String);  // Merge info from STFMA file
  Bool setInfo_MAIN(String);    // Gather info from MAIN table
  Bool mergeInfo_MAIN(String);  // Merge info from MAIN table

//----------------------------------------------------------------------
// No privates
};



#endif // NFRA_MS
