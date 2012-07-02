//# NFRA_PS_PW.h: This program demonstrates conversion of UVW for WSRT
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
//
//======================================================================
// Defines Position classes:
// - NFRA_Position
// - NFRA_PosMos
// - NFRA_PW
//
//----------------------------------------------------------------------
// Position
//
class NFRA_Position {

 private:
  Double RA;
  Double DEC;
  String Name;
  Double DTStart;
  Double DTEnd;
  Double HaMin;
  Double HaMax;

 public:
  NFRA_Position();

  Double getRA(){return RA;}
  Double getDEC(){return DEC;}
  String getName(){return Name;}
  Double getDTStart(){return DTStart;}
  Double getDTEnd(){return DTEnd;}
  Double getHaMin(){return HaMin;}
  Double getHaMax(){return HaMax;}

  void setRA(Double d){RA = d;}
  void setDEC(Double d){DEC = d;}
  void setName(String s){Name = s;}
  void setDTStart(Double);
  void setDTEnd(Double);
  //
  // There is no setHaMin/Max - those are _always_ calculated from the DTStart/End
  //

  void dump(String, uInt);
};

//----------------------------------------------------------------------
// Position mosaic Point
//
class NFRA_PosMos {

 private:
  uInt DwellTime;
  vector<NFRA_Position> Positions;

 public:
  uInt getDwellTime(){return DwellTime;}
  vector<NFRA_Position> getPositions(){return Positions;}

  void setDwellTime(uInt i){DwellTime = i;}
  void addPosition(NFRA_Position p){Positions.push_back(p);}
  void setPositions(vector<NFRA_Position> v){Positions = v;}

  void setDTStart(Double);
  void setDTEnd(Double);

  NFRA_PosMos(){DwellTime = 0;}
  uInt getNPos(){return Positions.size();}

  Double getHaMin();
  Double getHaMax();
  Double getRACentroid();
  Double getDECCentroid();

  void dump(String, uInt);
};

//----------------------------------------------------------------------
// Pointing Window - toplevel of position information
//
class NFRA_PW {
  
 private:
  String Telescopes;
  vector<NFRA_PosMos> PosMoss;

 public:
  String getTelescopes(){return Telescopes;}
  vector<NFRA_PosMos> getPosMoss(){return PosMoss;}

  void setTelescopes(String s){Telescopes = s;}
  void setPosMoss(vector<NFRA_PosMos> v){PosMoss = v;}
  void addTelescope(char t){Telescopes += t;}
  void addPosMos(NFRA_PosMos p){PosMoss.push_back(p);}
  void addToPosMos(Int pm, NFRA_Position p){PosMoss[pm].addPosition(p);}

  void setDTStart(Double);
  void setDTEnd(Double);

  uInt getNPos();
  Double getHaMin();
  Double getHaMax();
  Double getRACentroid();
  Double getDECCentroid();
  Int getDwellTime(){return PosMoss[0].getDwellTime();}

  void dump(uInt);
};

