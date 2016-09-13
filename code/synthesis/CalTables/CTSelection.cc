//# CTSelection.cc: Implementation of CTSelection class
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2003
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
//# $Id$
//----------------------------------------------------------------------------

#include <ms/MSSel/MSObservationGram.h>
#include <ms/MSSel/MSScanGram.h>
#include <synthesis/CalTables/CTSelection.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <ms/MSSel/MSAntennaGram.h>
#include <ms/MSSel/MSCorrGram.h>
#include <ms/MSSel/MSFieldGram.h>
#include <ms/MSSel/MSSpwGram.h>
#include <ms/MSSel/MSTimeGram.h>
#include <ms/MSSel/MSPolnGram.h>
//#include <casa/Containers/Record.h>
#include <tables/TaQL/RecordGram.h>
#include <ms/MSSel/MSAntennaParse.h>

namespace casa { //# NAMESPACE CASA - BEGIN

  //
  //----------------------------------------------------------------------------
  //
  CTSelection::CTSelection()    :MSSelection()  {};
  //
  //----------------------------------------------------------------------------
  //
  CTSelection::CTSelection(const NewCalTable& ct,
			   const MSSMode& mode,
			   const String& timeExpr,
			   const String& antennaExpr,
			   const String& fieldExpr,
			   const String& spwExpr,
			   const String& uvDistExpr,
			   const String& taqlExpr,
			   const String& polnExpr,
			   const String& scanExpr,
			   const String& arrayExpr,
			   const String& stateExpr,
			   const String& observationExpr)
  {
  }
  //
  //----------------------------------------------------------------------------
  //
  CTSelection::CTSelection (const CTSelection& other):
    MSSelection(other)
  {
  }
  //
  //----------------------------------------------------------------------------
  //
  CTSelection& CTSelection::operator= (const CTSelection& other)
  {
    MSSelection::operator=(other);
    
    return *this;
  }
  //
  //----------------------------------------------------------------------------
  //
  CTSelection::~CTSelection() 
  {

  };
  //
  //----------------------------------------------------------------------------
  //
  void CTSelection::reset(MSSelectableTable& msLike,
			  const MSSMode& mode,
			  const String& timeExpr,
			  const String& antennaExpr,
			  const String& fieldExpr,
			  const String& spwExpr,
			  const String& uvDistExpr,
			  const String& taqlExpr,
			  const String& polnExpr,
			  const String& scanExpr,
			  const String& arrayExpr,
			  const String& stateExpr,
			  const String& observationExpr)
  {
  };
  //
  //----------------------------------------------------------------------------
  //
  TableExprNode CTSelection::toTableExprNode(MSSelectableTable* msLike)
  {
    // Convert the MS selection to a TableExprNode object, 
    // representing a TaQL selection in C++.
    // Input:
    //    msLike           const MSSelectableTable&  MeasurementSet or CalTable 
    //                                               to bind TaQL
    // Output:
    //    toTableExprNode  TableExprNode             Table expression node
    //
    // Interpret all expressions and produce a consolidated TEN.  
    //
    if (fullTEN_p.isNull()==False) return fullTEN_p;

    const MeasurementSet *ms=getMS(msLike);
    resetMS(*ms);
    toTENCalled_p=True;
    //    ms_p = msLike->asMS();

    TableExprNode condition;
    
    initErrorHandler(ANTENNA_EXPR);
    initErrorHandler(STATE_EXPR);

    try
      {
	for(uInt i=0; i<exprOrder_p.nelements(); i++)
	  {
	    TableExprNode node;
	    switch(exprOrder_p[i])
	      {
	      case ANTENNA_EXPR:
		{
		  if(antennaExpr_p != "")
		    {
		      antenna1IDs_p.resize(0);
		      antenna2IDs_p.resize(0);
		      baselineIDs_p.resize(0,2);
		      TableExprNode col1TEN = msLike->col(msLike->columnName(MS::ANTENNA1)),
			col2TEN = msLike->col(msLike->columnName(MS::ANTENNA2));

		      MSAntennaParse thisParser(msLike->antenna(), col1TEN, col2TEN);

		      node = baseMSAntennaGramParseCommand(&thisParser, antennaExpr_p, 
							   antenna1IDs_p, antenna2IDs_p, 
							   baselineIDs_p);
		      // node = msAntennaGramParseCommand(*msLike, antennaExpr_p, 
		      // 				       antenna1IDs_p, antenna2IDs_p, 
		      // 				       baselineIDs_p);
		    }
		  break;
		}
	      case FIELD_EXPR:
		{
		  if(fieldExpr_p != "")
		    {
		      fieldIDs_p.resize(0);
		      
		      TableExprNode colAsTEN = msLike->col(msLike->columnName(MS::FIELD_ID));
		      node = msFieldGramParseCommand(msLike->field(), colAsTEN, fieldExpr_p,fieldIDs_p);
		    }
		  break;
		}
	      case SPW_EXPR:
		{
		  if (spwExpr_p != "")
		    {
		      TableExprNode colAsTEN = msLike->col(msLike->columnName(MS::DATA_DESC_ID));
		      spwIDs_p.resize(0);
		      if (msSpwGramParseCommand(msLike->spectralWindow(), 
						msLike->dataDescription(),
						colAsTEN, spwExpr_p,
						spwIDs_p, chanIDs_p,spwDDIDs_p) == 0)
			node = *(msSpwGramParseNode());
		    }
		  break;
		}
	      case SCAN_EXPR:
		{
		  TableExprNode colAsTEN = msLike->col(msLike->columnName(MS::SCAN_NUMBER));
		  scanIDs_p.resize(0);
		  if(scanExpr_p != "")
		    node = msScanGramParseCommand(ms, colAsTEN, scanExpr_p, scanIDs_p, maxScans_p);
		  break;
		}
	      case OBSERVATION_EXPR:
		{
		  TableExprNode colAsTEN = msLike->col(msLike->columnName(MS::OBSERVATION_ID));
		  observationIDs_p.resize(0);
		  if(observationExpr_p != "")
		    node = msObservationGramParseCommand(ms, msLike->observation(),
							 colAsTEN,
							 observationExpr_p, 
							 observationIDs_p);
		  break;
		}
	      case TAQL_EXPR:
		{
		  if(taqlExpr_p != "")
		    {
		      node = RecordGram::parse(*msLike->table(),taqlExpr_p);
		    }
		  break;
		}
		// case POLN_EXPR:
		// 	{
		// 	  // This expression is a pure in-row selection.  No
		// 	  // need to add to the tree of TENs (the condition
		// 	  // variable).
		// 	  if (polnExpr_p != "")
		// 	    {
		// 	      msPolnGramParseCommand(ms, 
		// 				     polnExpr_p,
		// 				     node,
		// 				     ddIDs_p,
		// 				     selectedPolMap_p,
		// 				     selectedSetupMap_p);
		// 	    }
		// 	  break;
		// 	}
	      case NO_EXPR:break;
	      default:  break;
	      } // Switch
		
	    condition = condition && node;
	  }//For
	//
	// Now parse the time expression.  Internally use the condition
	// generated so far to find the first logical row to use to get
	// value of the wild-card fields in the time expression. 
	//
	selectedTimesList_p.resize(2,0);
	
	const TableExprNode *timeNode = 0x0;
	TableExprNode colAsTEN = msLike->col(msLike->columnName(MS::TIME));
	MSSelectableMainColumn *mainColInterface=msLike->mainColumns();
	// MSMainColInterface msMainColInterface;
	// msMainColInterface.init(*(msLike->table()));
	
	if(timeExpr_p != "" &&
	   //msTimeGramParseCommand(ms, timeExpr_p, condition, selectedTimesList_p) == 0)
	   msTimeGramParseCommand(ms, timeExpr_p, colAsTEN, *mainColInterface, condition, selectedTimesList_p) == 0)
	  timeNode = msTimeGramParseNode();
	
	//
	// Add the time-expression TEN to the condition
	//
	if(timeNode && !timeNode->isNull()) 
	  {
	    if(condition.isNull()) 
	      condition = *timeNode;
	    else 
	      condition = condition && *timeNode;
	  }
	
	fullTEN_p = condition;
      }
    catch(AipsError& x)
      {
	runErrorHandler();
	deleteNodes();
	deleteErrorHandlers();
	throw(x);
      }	
    
    runErrorHandler();
    deleteErrorHandlers();
    deleteNodes();
    return condition;
  };
} //# NAMESPACE CASA - END

