//# NFRA_Misc.h: This program demonstrates conversion of UVW for WSRT
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
// Some misc. functions and classes
//

Double deg2rad(Double);                  // ...
Double calcHA(Double, Double, Double);   // calculate Ha from Time, Ra and Dec
vector<Double> CommaSepStr2vDbl(String); // convert comma-separated string to 
                                         // vector of doubles
vector<String> CommaSepStr2vStr(String); // ...
vector<uInt> CommaSepStr2vuInt(String);  // ...
Double Str2Time(String);                 // convert string formated time to double

//----------------------------------------------------------------------
// class NFRA_Baseline
// Position properties of movable telescopes
//
class NFRA_BaseLine {

  char Telescope;  // 0...9,A...F
  char Reference;  // distances are given w.r.t. the reference telescope
  Double X, Y, Z;  
  Double Distance;

 public:

  char getTelescope() {return Telescope;}
  char getReference() {return Reference;}
  Double getX() {return X;}
  Double getY() {return Y;}
  Double getZ() {return Z;}
  void setXYZ(Vector<Double> xyz){X = xyz[0]; Y = xyz[1]; Z = xyz[2];}
  Double getDistance() {return Distance;}

  NFRA_BaseLine(char c);

  void operator-=(NFRA_BaseLine); // position difference between two telescopes
  void dump(uInt);
};

//----------------------------------------------------------------------
// class NFRA_SubArray
// subarray info
//  - Per subarray the number of TimeSlices is kept
//
class NFRA_SubArray {

  Int SANr;
  uInt NTimeSlices;

 public:
  NFRA_SubArray(Int i){SANr = i; NTimeSlices = 1;}
  Int getSANr() {return SANr;}
  void incrNTimeSlices(){NTimeSlices++;}
  uInt getNTimeSlices() {return NTimeSlices;}
  void dump(uInt);

};

