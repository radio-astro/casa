//# NFRA_MS_Tabs - classes to access WSRT tables
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

#include <nfra/Wsrt/NFRA_MF.h>

//======================================================================
// Class definitions for MS tables
//
//----------------------------------------------------------------------
// SPECTRAL-WINDOW - frequency information
//
class NFRA_spwnd {

 private:
  uInt NBands;
  uInt NIVCSets;

  vector<NFRA_IVC> IVCs;
  vector<String> UniqueNames;

 public:
  NFRA_spwnd(Table);
  uInt getNBands(){return NBands;}
  uInt getNIVCSets(){return NIVCSets;}
  NFRA_IVC getIVC(uInt i){return IVCs[i];}

  void dump();
};

//----------------------------------------------------------------------

class NFRA_field {

 private:
  uInt NFields;
  String Epoch;
  vector<NFRA_Position> Poss;

 public:
  NFRA_field(Table);
  uInt getNFields(){return NFields;}
  NFRA_Position getField(uInt i){return Poss[i];}
  String getEpoch(){return Epoch;}

  void dump();
};

//----------------------------------------------------------------------

class NFRA_obs {

 private:
  String Instrument;
  String Semester;
  String PrjNr;
  String FieldName;

 public:
  NFRA_obs(Table);

  String getInstrument(){ return Instrument;}
  String getSemester(){ return Semester;}
  String getPrjNr(){ return PrjNr;}
  String getFieldName(){ return FieldName;}

  void dump();

};


//----------------------------------------------------------------------
class NFRA_proc {

 private:
  String BackEnd;

 public:
  NFRA_proc(Table);
  String getBackEnd(){return BackEnd;}
  void dump();

};

//----------------------------------------------------------------------
class NFRA_antenna {

 private:
  Int AntError;
  vector<NFRA_BaseLine> BaseLines;

 public:
  NFRA_antenna(Table);

  vector<NFRA_BaseLine> getBaseLines(){ return BaseLines;}
  Int getError() { return AntError;}
  Bool hasError() { return AntError != 0;}

  void dump();
};

//----------------------------------------------------------------------
class NFRA_pol {

 private:
  uInt NPols;
  vector<String> PolsUsed;

 public:
  NFRA_pol(Table);

  uInt getNPols(){ return NPols;}
  vector<String> getPolsUsed(){ return PolsUsed;}

  void dump();
};

//----------------------------------------------------------------------
//
class NFRA_parameters {

 private:
  Table t;
  Int SubArray;
  vector<NFRA_PW> PWs;
  vector<NFRA_FW> FWs;

 public:
  NFRA_parameters(NFRA_MF q){};
  NFRA_parameters(Table);
  NFRA_parameters(Table, Int);

  void FullPositionScan();
  void FullFrequencyScan();
  vector<NFRA_PW> getPWs(){return PWs;}
  vector<NFRA_FW> getFWs(){return FWs;}

  String getVal(String);
  void dump();

};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//----------------------------------------------------------------------
class NFRA_main {

  String tName;

public:
  Double DateTimeStart;
  Double DateTimeEnd;
  Double Duration;
  Double HAStart;
  Double HAEnd;
  vector<uInt> ExpTime;

  NFRA_main(String, Double, Double);
  uInt getNExposure(){return ExpTime.size();}
  uInt getExposure(uInt i){return ExpTime[i];}

  Double getDTStart(uInt);
  Double getDTEnd();

};

