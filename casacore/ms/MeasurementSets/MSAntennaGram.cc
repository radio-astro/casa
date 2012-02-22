//# MSAntennaGram.cc: Grammar for antenna expressions
//# Copyright (C) 1998,1999,2001,2003
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
//# $Id: MSAntennaGram.cc 20934 2010-08-17 09:16:24Z gervandiepen $

// MSAntennaGram; grammar for antenna command lines

// This file includes the output files of bison and flex for
// parsing command lines operating on lattices.
// This is a preliminary version; eventually it has to be incorporated
// in the AIPS++ command language.

#include <tables/Tables/ExprNode.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSAntennaGram.h>
#include <ms/MeasurementSets/MSAntennaParse.h> // routines used by bison actions
#include <ms/MeasurementSets/MSAntennaIndex.h>
#include <ms/MeasurementSets/MSSelectionError.h>

//# stdlib.h is needed for bison 1.28 and needs to be included here
//# (before the flex/bison files).
#include <casa/stdlib.h>
#include "MSAntennaGram.ycc"                  // flex output
#include "MSAntennaGram.lcc"                  // bison output

// Define the yywrap function for flex.
int MSAntennaGramwrap()
{
    return 1;
}

namespace casa { //# NAMESPACE CASA - BEGIN

//# Declare a file global pointer to a char* for the input string.
  static const char*           strpMSAntennaGram = 0;
  static Int                   posMSAntennaGram = 0;

//# Parse the command.
//# Do a yyrestart(yyin) first to make the flex scanner reentrant.

  TableExprNode msAntennaGramParseCommand (const MeasurementSet* ms,
                                           const String& command, 
                                           Vector<Int>& selectedAnts1,
                                           Vector<Int>& selectedAnts2,
                                           Matrix<Int>& selectedBaselines) 
{
  MSAntennaParse parser(ms);               // setup measurement set
  try 
    {
      MSAntennaGramrestart (MSAntennaGramin);
      yy_start = 1;
      strpMSAntennaGram = command.chars();     // get pointer to command string
      posMSAntennaGram  = 0;                   // initialize string position

      parser.setComplexity();
      MSAntennaParse::thisMSAParser = &parser; // The global pointer to the parser
      MSAntennaGramparse();                    // parse command string
    
      selectedAnts1.reference (parser.selectedAnt1());
      selectedAnts2.reference (parser.selectedAnt2());
      selectedBaselines.reference (parser.selectedBaselines());
      return parser.node();
    } 
  catch (MSSelectionAntennaError& x) 
    {
      String newMesgs;
      newMesgs = constructMessage(msAntennaGramPosition(),command);
      x.addMessage(newMesgs);
      String token("");
      //      cerr << "New Mesg : " << x.getMesg() << endl;
      MSAntennaParse::thisMSAErrorHandler->reportError(token.c_str(), x.getMesg());
 
      return parser.node();
      //      throw;
    }
}

//# Give the string position.
Int& msAntennaGramPosition()
{
    return posMSAntennaGram;
}

//# Get the next input characters for flex.
int msAntennaGramInput (char* buf, int max_size)
{
    int nr=0;
    while (*strpMSAntennaGram != 0) 
      {
	if (nr >= max_size) break;                         // get max. max_size char.
	buf[nr++] = *strpMSAntennaGram++;
      }
    return nr;
}

void MSAntennaGramerror (const char*)
{
  throw (MSSelectionAntennaParseError ("Antenna Expression: Parse error at or near '" +
  					 String(MSAntennaGramtext) + "'"));
  // String token("");
  // MSAntennaParse::thisMSAErrorHandler->reportError(token.c_str(), String("Antenna Expression: Parse error at or near '" +
  // 									 String(MSAntennaGramtext) + "'"));
  
}

} //# NAMESPACE CASA - END
