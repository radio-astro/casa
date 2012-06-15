// 
// harvestrn - a wee program to scan the output of svn log and 
// if the log contains a notation to include the text in the
// release notes it will spit that out to stdout.
//
//# Copyright (C) 2007
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
//# Includes
//
//
#include <iostream>
#include <fstream>
#include <fstream>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Regex.h>

using namespace std;

int main(int argc, char **argv)
{
   bool inRelNotes(false);
   istream *ifs;
   if(argc > 1)
      ifs = new ifstream(argv[1]);
   else
      ifs = &cin;
   //
   //
   //
   casa::String jiraLine;
   while(!ifs->eof()){
      char buffer[1025];
      ifs->getline(buffer, 1024);
      casa::String theLine(buffer);
      if(theLine.contains("JIRA Issue:")){
	      jiraLine = casa::String(theLine);
      }
      if(theLine.contains(casa::Regex("Put in Release Notes:.*[Yy][Ee][Ss]"))){
         if(!theLine.contains(casa::Regex("Put in Release Notes:.*[yY][Ee][Ss]/[Nn][oO]"))){
	    inRelNotes = true;
	    cout << "--------------------" << endl;
	    cout << jiraLine << endl;
	 }
      }
      if(theLine.contains("---------------------")){
	 inRelNotes = false;
      }
      if(inRelNotes)
	      cout << theLine << endl;

   }
   cout << "--------------------" << endl;
   return 0;
}
