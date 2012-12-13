//# PlotCalHooks.cc: Callbacks for PlotCal
//# Copyright (C) 1996,1997,1998,2001,2002,2003
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

#if !defined CASA_PLOTCAL_HOOKS_H
#define CASA_PLOTCAL_HOOKS_H

//# System Includes
#include <stdio.h>
#include <iostream>
#include <iomanip>

//# General CASA includes
#include <casa/BasicSL/String.h>

//# Table and TablePlot includes
#include <tools/tables/TablePlot/TablePlot.h>


namespace casa { //# NAMESPACE CASA - BEGIN


class PlotCalReset : public TPResetCallBack
{
public:
  PlotCalReset( PlotCal *inPlotCal )
  {
    itsPlotCal = inPlotCal;
  };
  
  ~PlotCalReset() {};
  
  Bool reset() {
    cout << "Resetting plotcal" << endl;
    itsPlotCal->close();
    return True;
  };
  
private:
  PlotCal *itsPlotCal;
};


class PlotCalCallBacks : public TPGuiCallBackHooks
{
public:
  PlotCalCallBacks(Record metainfo,Bool isNCT):
    TPGuiCallBackHooks(),
    isNCT_p(isNCT)
 {
    LocateColumns.resize(4);
    LocateColumns[0] = "ANTENNA1";
    LocateColumns[1] = "FIELD_ID";
    LocateColumns[2] = "TIME";
    LocateColumns[3] = CDIcol();

    if (metainfo.isDefined("fieldNames"))
      fieldNames_p = metainfo.asArrayString("fieldNames");
    
    if (metainfo.isDefined("antNames")) 
      antNames_p = metainfo.asArrayString("antNames");

    if (metainfo.isDefined("spwIds")) 
      spwIds_p = metainfo.asArrayInt("spwIds");

    //    cout << "fieldNames_p = " << fieldNames_p << endl;
    //    cout << "antNames_p   = " << antNames_p << endl;
    //    cout << "spwIds_p     = " << spwIds_p << endl;

  };

  ~PlotCalCallBacks(){
    cout << "PlotCalCallBacks dtor" << endl;
  };

  casa::Bool flagdata(String tablename) {
    //    cout << "PlotCalCallBacks :: Completed flagging on : "
    //         << tablename << endl;
    return True;
  }
  casa::Bool releasetable(Int nrows, Int ncols, Int panel, String tablename) {
    //    cout << "PlotCalCallBacks :: releasing "
    //         << tablename << " from panel "
    //         << nrows << "," <<ncols << "," << panel << endl;
    return True;
  }
  casa::Bool createiterplotlabels( Vector<String> iteraxes,
				   Vector<Double> values,
				   String &titleString ) {
    //    cout << "iteraxes    = " << iteraxes << endl;
    //    cout << "values      = " << values << endl;

    titleString="    ";
    for (uInt i=0;i<iteraxes.nelements();++i) {

      if (iteraxes(i)=="ANTENNA1") {
	if (antNames_p.nelements()>0 && values(i)<antNames_p.nelements())
	  titleString = titleString + "  Antenna=\\'" + antNames_p((Int)values(i)) + "\\'";
	else
	  titleString = titleString + "  AntId=" + String::toString((Int)values(i));
      }
      if (iteraxes(i)=="FIELD_ID") {
	if (fieldNames_p.nelements()>0 && values(i)<fieldNames_p.nelements())
	  titleString = titleString + "  Field=\\'" + fieldNames_p((Int)values(i)) + "\\'";
	else
	  titleString = titleString + "  FldId=" + String::toString((Int)values(i));
      }
      if (iteraxes(i)==CDIcol()) {
	if (spwIds_p.nelements()>0 && values(i)<spwIds_p.nelements())
	  titleString = titleString + "  Spw=\\'" + String::toString(spwIds_p((Int)values(i))) + "\\'";
	else
	  titleString = titleString + "  CalId=" + String::toString((Int)values(i));
      }
      if (iteraxes(i)=="TIME") {
	titleString = titleString + "  Time=\\'" + MVTime(values(i)/C::day).string( MVTime::TIME,7) + "\\'";
      }
    }

    //    cout << "titleString = " << titleString << endl;
    
    return True;

  }

  casa::Bool printlocateinfo(Vector<String> collist,
                             Matrix<Double> infomat,
                             Vector<String> cpol) {

    LogIO log;

    if (infomat.shape()[1]>0) {
      //      cout << "collist = " << collist << endl;
      //      cout << "infomat = " << infomat << endl;
      //      cout << "cpol    = " << cpol << endl;

      //      log << "Found the following calibration samples in the region:" << LogIO::POST;

      for (Int j=0;j<infomat.shape()[1];++j) {
        log << MVTime( infomat(4, j)/C::day).string( MVTime::YMD,7) << " ";

	if (antNames_p.nelements()>0 && uInt(infomat(2,j))<antNames_p.nelements())
	  log << " Antenna=" << "'" << antNames_p((Int)infomat(2,j)) << "'";
	else
	  log << " AntId=" << infomat(2,j);
	    
	if (spwIds_p.nelements()>0 && uInt(infomat(5,j))<spwIds_p.nelements())
	  log << " SpwId=" << spwIds_p((Int)infomat(5,j));
	else
	  log << " CalId=" << infomat(5,j);
	  
	if (fieldNames_p.nelements()>0 && uInt(infomat(3,j))<fieldNames_p.nelements())
	  log << " Field=" << "'" << fieldNames_p((Int)infomat(3,j)) << "'";
	else
	  log << " FldId=" << infomat(3,j);

	log << " pol,chan=" << cpol(j);
           
	log << LogIO::POST;

      }

    }
    return True;
  }

private:

  inline String CDIcol() { return (isNCT_p ? "SPECTRAL_WINDOW_ID" : "CAL_DESC_ID"); };


  Vector<String> fieldNames_p;
  Vector<String> antNames_p;
  Vector<Int> spwIds_p;

  Bool isNCT_p;

};


class PlotCalFreqAxes : public TPConvertChanToFreq
{
public :
  PlotCalFreqAxes(const Vector<Int>& cdlist,
		  const Vector<Double>& startFreq,
		  const Vector<Double>& stepFreq) {
    cdList = cdlist;
    offsetV = startFreq;
    intervalV = stepFreq;
    //    cout << cdList << " " << offsetV << " " << intervalV << endl;

  }
  ~PlotCalFreqAxes(){};
  Double offset,interval;
  inline Double Xconvert_col(Double x,Int row,Int tblNum){
    return x*intervalV(cdList(tblNum)) + offsetV(cdList(tblNum));
  }

private:
  Vector<Int> cdList;
  Vector<Double> offsetV;
  Vector<Double> intervalV;

};


class PlotCalParang : public TPConvertBase
{
public:
  PlotCalParang(Vector<Vector<Int> >& fldlists) {
    // Set up MSDerivedValues to do calc
    fldLists = fldlists;
  };
  ~PlotCalParang(){};
  inline Double Xconvert(Double x,
			 Int row,
			 Int tblNum) {
    //    cout << "PCP::Xconvert: " << tblNum << " " << row << " " << x 
    //	 << fldLists(tblNum)(row)
    //	 << endl;
    return x;
  }
private:

  Vector<Vector<Int> > fldLists;
};


} //# NAMESPACE CASA - END

#endif
