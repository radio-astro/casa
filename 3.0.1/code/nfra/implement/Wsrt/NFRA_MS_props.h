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

#ifndef NFRA_MS_props_H
#define NFRA_MS_props_H



//======================================================================
class NFRA_MS_props {

 protected:

  //
  // MS-related variables
  //
  vector<String> MSName;    // Names of MS in the data set
  vector<NFRA_SubArray> SubArrays;
                            // Number of SubArrays for the Sequence number
  Float MSVersion;          // Version of MS (1 or 2), is a float because it is
                            //  one for aips++
  String SeqNr;             // Seq.Nr of measurement
  String Instrument;        // TMS instrument
  Int NRow;                 // Number of rows in MAIN table
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

};



#endif // NFRA_MS_props
