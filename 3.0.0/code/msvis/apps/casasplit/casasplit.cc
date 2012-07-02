//# casasplit.cc: Application for running subms standalone.
//# Copyright (C) 2008
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
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id$
#include <iostream>
#include <casa/BasicSL/String.h>
#include <msvis/MSVis/SubMS.h>

using namespace casa;

int main(int argc, char* argv[]) {
  // Parse arguments.
  if(argc < 3 || argc > 6 || argv[1] == "-h" || argv[1] == "--help"){
    cout << argv[0] << ": Stand-alone executable for splitting an MS.\n"
	 << "\nUse:\n\t" << argv[0] << " [-n] [-s spwsel] inputms outputms [timebin]\n\n"
	 << "where timebin is an averaging time in seconds.\n"
	 << "\n\t-n: Open inputms read-only.\n"
	 << "\n\t-s: Spectral window selection." << endl;
    return !(argv[1] == "-h" || argv[1] == "--help");
  }
  
  // Defaults
  Bool nomodify = false; // Known to work.
  String spwsel("");

  Vector<Int> step;
  step.resize(0);
  
  String t_field("");
  String t_antenna("");
  String t_scan("");
  String t_uvrange("");
  String t_taql("");
  String timerange("");
  String t_whichcol("corrected");
  Bool   averchan = true;

  char **args = argv + 1;
  while(args[0][0] == '-'){
    if(args[0] == "-n")
      nomodify = true;
    else if(args[0] == "-s"){
      spwsel = args[1];
      ++args;
    }
    else if(args[0] == "-t"){
      timerange = args[1];
      ++args;
    }
    else if(args[0] == "-w"){
      t_whichcol = args[1];
      ++args;
    }
    ++args;              // <- Like "shift".
  }
  
  String inms(args[0]);
  String outms(args[1]);
  
  Float  timebin = -1.0;
  if(argc > 3)
    timebin = atof(args[2]);
    
  Bool rstat(False);
  
  SubMS *splitter = new SubMS(inms);

  //*itsLog << LogIO::NORMAL2 << "Sub MS created" << LogIO::POST;

  splitter->setmsselect(spwsel, t_field, t_antenna, t_scan, t_uvrange,
			t_taql, step, averchan);
  splitter->selectTime(timebin, timerange);
  splitter->makeSubMS(outms, t_whichcol);
  //*itsLog << LogIO::NORMAL2 << "SubMS made" << LogIO::POST;
  delete splitter;
  
  return rstat;
}
