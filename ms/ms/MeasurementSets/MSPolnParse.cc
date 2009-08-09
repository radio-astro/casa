//# MSPolnParse.cc: Classes to hold results from Poln grammar parseing
//# Copyright (C) 1994,1995,1997,1998,1999,2000,2001,2003
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

#include <ms/MeasurementSets/MSPolnParse.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <ms/MeasurementSets/MSSelectionError.h>
#include <ms/MeasurementSets/MSSelectionTools.h>
#include <ms/MeasurementSets/MSSpwGram.h>
#include <casa/BasicSL/String.h>
#include <casa/Logging/LogIO.h>
#include <casa/Containers/MapIO.h>
#include <casa/Containers/OrderedMap.h>

namespace casa { //# NAMESPACE CASA - BEGIN
  
  //  MSPolnParse* MSPolnParse::thisMSSParser = 0x0; // Global pointer to the parser object
  // TableExprNode* MSPolnParse::node_p = 0x0;
  // Vector<Int> MSPolnParse::ddIDList;
  // OrderedMap<Int, Vector<Int> > MSPolnParse::polList(Vector<Int>(0)); 
  //# Constructor
  //------------------------------------------------------------------------------
  //  
  MSPolnParse::MSPolnParse ()
    : MSParse(),
      node_p(0x0), 
      ddIDList_p(), 
      polMap_p(Vector<Int>(0)),
      setupMap_p(Vector<Vector<Int> >(0))
  {
    if (MSPolnParse::node_p!=0x0) delete MSPolnParse::node_p;
    MSPolnParse::node_p=0x0;
    node_p = new TableExprNode();
  }
  //# Constructor with given ms name.
  //------------------------------------------------------------------------------
  //  
  MSPolnParse::MSPolnParse (const MeasurementSet* ms)
    : MSParse(ms, "Pol"),
      node_p(0x0), 
      ddIDList_p(), 
      polMap_p(Vector<Int>(0)),
      setupMap_p(Vector<Vector<Int> >(0))
  {
    ddIDList_p.resize(0);
    if(MSPolnParse::node_p) delete MSPolnParse::node_p;
    node_p = new TableExprNode();
  }
  //
  //------------------------------------------------------------------------------
  //  
  const TableExprNode *MSPolnParse::selectFromIDList(const Vector<Int>& ddIDs)
  {
    TableExprNode condition;
    Int n=ddIDs.nelements();
    const String DATA_DESC_ID = MS::columnName(MS::DATA_DESC_ID);

    if (n > 0)
      {
	for(Int i=0; i<n; i++)
	  if (condition.isNull())
	    condition = ((ms()->col(DATA_DESC_ID)==ddIDs[i]));
	  else
	    condition = condition || ((ms()->col(DATA_DESC_ID)==ddIDs[i]));
      }

    if (condition.isNull()) 
      {
	ostringstream Mesg;
	Mesg << "No match for the [SPW:]POLN specifications ";
	throw(MSSelectionPolnError(Mesg.str()));
      }
    if(node_p->isNull())
      *node_p = condition;
    else
      *node_p = *node_p || condition;
    
    return node_p;
  }
  //
  //------------------------------------------------------------------------------
  //  
  Vector<Int> MSPolnParse::getMapToDDIDs(MSDataDescIndex& msDDNdx, 
					 MSPolarizationIndex& msPolNdx,
					 const Vector<Int>& spwIDs, 
					 const Vector<Int>& polnIDs)
  //				   Matrix<Int>& ddIDMap)
  {
    Vector<Int> ddIDs;
    if (polnIDs.nelements() == 0)
      {
	ostringstream mesg;
	mesg << "No match for polarization ID(s) " << polnIDs << " ";
	throw(MSSelectionPolnParseError(String(mesg.str())));
      }
    // cout << "No. of pol IDs = " << polnIDs.nelements() << endl;
    // cout << "No. of SPW IDs = " << spwIDs.nelements() << endl;
    for (uInt p=0; p<polnIDs.nelements(); p++)
      {
	Vector<Int> thisDDList;
	thisDDList.resize(0);
	for (uInt s=0; s<spwIDs.nelements(); s++)
	  {
	    Int n;
	    Vector<Int> tmp=msDDNdx.matchSpwIdAndPolznId(spwIDs[s],polnIDs[p]);
	    if (tmp.nelements() > 0)
	      {
		ddIDs.resize((n=ddIDs.nelements())+1,True);
		ddIDs[n]=tmp[0];
		thisDDList.resize((n=thisDDList.nelements())+1,True);
		thisDDList[n]=tmp[0];
	      }
	  }
	//	cout << "P = " << polnIDs[p] << " " << thisDDList << endl;
	setIDLists(polnIDs[p], 1, thisDDList);
      }
    return ddIDs;
  }
  //
  //------------------------------------------------------------------------------
  //  Give a list of pol IDs, return the list of row numbers in the
  //  POLARIZATION sub-table which contains the listed Pol IDs.  Pol
  //  IDs are defined as the enumrations Stokes::StokesTypes -
  //  i.e. "RR", "LL" etc.
  //
  Vector<Int> MSPolnParse::matchPolIDsToPolTableRow(const Vector<Int>& polIds,
						    OrderedMap<Int, Vector<Int> >& polIndexMap)
  {
    Vector<Int> rowList;
    MSPolarization mspol(ms()->polarizationTableName());
    ROMSPolarizationColumns mspolC(mspol);
    //
    // First extract the corrType column of the Polarization sub-table
    // row-by-row (since this column can be of variable shape!)
    //
    for (uInt row=0; row<mspolC.nrow();row++)
      {
	Vector<Int> corrType;
	mspolC.corrType().get(row,corrType);
	//
	// Next - look for match between the suppliced polId list in
	// the extracted corrType.  User support: Do not assume the
	// order of the supplied pol IDs (human free-will was involved
	// in generating that list!).  Also do a min-match.  E.g. a
	// supplied polID list from "RR LL" should match all of the
	// following corrType lists: "RR LL", "RR LL LR RL", "RR",
	// "LL".
	//
	//	OrderedMap<Int, Vector<Int> > polIndexMap(polIndices);
	Bool allFound=False;
	Int foundCounter=0;
	Vector<Int> polIndices(0,-1);
	for(uInt i=0; i<polIds.nelements(); i++)
	  {
	    for(uInt j=0; j<corrType.nelements(); j++)
	      if (polIds[i] == corrType[j])
		{
		  Int m=0;
		  polIndices.resize((m=polIndices.nelements())+1,True);
		  polIndices[m]=j;
		  foundCounter++;
		  break;
		}
	  }

	if (allFound=(foundCounter == polIds.nelements()))
	  {
	    polIndexMap(row)=polIndices;
	    setIDLists((Int)row,0,polIndices);
	  }
	if (allFound)
	  {
	    uInt n;
	    rowList.resize((n=rowList.nelements())+1,True);
	    rowList[n]=row;
	  }
      }

    return rowList;
  }
  //
  //------------------------------------------------------------------------------
  //  
  Vector<Int> MSPolnParse::getPolnIDs(const String& polSpec)
  {
    String sep(";");
    Vector<String> tokens;
    Vector<Int> idList, polIDList;
    // Vector<Stokes::StokesTypes> ttt;
    //
    // Split the given string into ";" separated tokens.  Upcase the
    // string before splitting.
    //
    tokenize(polSpec,sep,tokens,True);
    idList.resize(tokens.nelements());
    for(uInt i=0;i<idList.nelements();i++)
      idList[i]=Stokes::type(tokens[i]);

    //
    //  Generate a list of DDIDs which will be used to the actual row
    //  selection.  Also make a map of the poln IDs and list of in-row
    //  indices which will then be used for in-row selection.
    //
    polIDList=matchPolIDsToPolTableRow(idList,polMap_p);

    // cout << "PolIDs = " << polIDList << "    polIndices = " << polMap_p << endl;
    return polIDList;
  }
  //
  //------------------------------------------------------------------------------
  //  
  // The actual parser.  Does three things:
  //   1. Tokenize the [SPW:]POLN string into SPW and POLN tokens.
  //   2. Generate list of indices using SPW and POLN tokens.
  //   3. Generate the {SPW, POLN} --> DDID map
  //
  Int MSPolnParse::theParser(const String& command) 
			     // Vector<Int>& selectedDDIDs, 
			     // Matrix<Int>& selectedSpwPolnMap)
  {
    Int ret=0, nSpecList=0;
    Vector<String> polnSpecList;
    String sep(",");
    //    cout << "Poln selection = " << command << endl;
    nSpecList=tokenize(command,sep,polnSpecList);
    //    cout << polnSpecList << endl;
    for(Int i=0;i<nSpecList;i++)
      {
	Vector<String> tokens,tmp;
	Vector<Int> spwIDs;
	Matrix<Int> chanIDs;
	Vector<Int> polnIDs;

	String s(":"), spwExpr, polnExpr;
	Int nSpw, nCorr, nTokens;
	//
	// User suppport: Check if they tried [SPW:CHAN:]POLN kind of
	// specification.  Darn - String::freq(...) does not work!
	//
	nSpw=tokenize(polnSpecList[i],s,tokens);
	tokenize(tokens[0],s,tmp);
	nTokens = tokens.nelements();
	// cout << tokens.nelements() << " " << tokens << " " << nTokens << endl;
	if (nTokens > 2)

	  //	  MSPolnGramerror
	  throw(MSSelectionPolnParseError(String("Too many ':'s.  Tip: Channel "
						 "specification is not useful "
						 "and not allowed.")));
	//
	// If there were two ":" separate tokens, they were of the form SPW:POLN
	//
	if (nTokens == 2)
	  {
	    spwExpr = tokens[0];
	    polnExpr= tokens[1];
	  }
	//
	// If there was only one token, it was POLN - equivalent of *:POLN
	//
	if (nTokens == 1)
	  {
	    spwExpr="*";
	    polnExpr=tokens[0];
	  }
	//
	// Parse the SPW part.  Pass the token to the SPW parser.
	//
	try
	  {
	    msSpwGramParseCommand(ms(), spwExpr,spwIDs, chanIDs);
	    //	    cout << "SPW/Chan = " << spwIDs << " " << chanIDs << endl;
	  }
	catch (MSSelectionSpwError &x)
	  {
	    String mesg;
	    mesg = " "+polnSpecList[i];
	    x.addMessage(mesg);
	    //	    MSPolnGramerror((char *)(x.getMesg().c_str()));
	    throw(MSSelectionPolnParseError(x.getMesg()));
	  }
	//
	// Parse the POLN part.
	//
	try
	  {
	    polnIDs=getPolnIDs(polnExpr);
	    // cout << "Tokens = " << tokens << endl
	    // 	 << "SPW = " << spwExpr << " " << spwIDs << endl
	    // 	 << "Poln = " << polnExpr << " " << polnIDs << endl;
	    MSDataDescIndex msDDNdx(ms()->dataDescription());
	    MSPolarizationIndex msPolNdx(ms()->polarization());
	    ddIDList_p=getMapToDDIDs(msDDNdx, msPolNdx, spwIDs, polnIDs);

	    // cout << "DD IDs = " << ddIDList_p << endl;
	  }
	catch (MSSelectionPolnParseError& x)
	  {
	    String mesg("(named ");
	    mesg = mesg + polnExpr + ")";
	    x.addMessage(mesg);
	    throw;
	  }
	selectFromIDList(ddIDList_p);
      }
    return ret;
  }

  const TableExprNode* MSPolnParse::node() { return node_p; }
  //
  //------------------------------------------------------------------------------
  //  A convenience method to set the vectors of Poln or DD IDs in the setupMap.
  //
  void MSPolnParse::setIDLists(const Int key, const Int ndx, Vector<Int>& val)
  {
    setupMap_p(key).resize(2,True);
    if (ndx>1)
      throw(MSSelectionError("Internal error in MSPolnParse::setIDLists(): Index greater 1"));
    setupMap_p(key)[ndx]=val;
  }
} //# NAMESPACE CASA - END
