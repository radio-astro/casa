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

//# casacore::Table and TablePlot includes
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
  
  casacore::Bool reset() {
    std::cout << "Resetting plotcal" << std::endl;
    itsPlotCal->close();
    return true;
  };
  
private:
  PlotCal *itsPlotCal;
};


class PlotCalCallBacks : public TPGuiCallBackHooks
{
public:
  PlotCalCallBacks(casacore::Record metainfo,casacore::Bool isNCT):
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
    std::cout << "PlotCalCallBacks dtor" << std::endl;
  };

  casacore::Bool flagdata(casacore::String /*tablename*/) {
    //    cout << "PlotCalCallBacks :: Completed flagging on : "
    //         << tablename << endl;
    return true;
  }
  casacore::Bool releasetable(casacore::Int /*nrows*/, casacore::Int /*ncols*/, casacore::Int /*panel*/, casacore::String /*tablename*/) {
    //    cout << "PlotCalCallBacks :: releasing "
    //         << tablename << " from panel "
    //         << nrows << "," <<ncols << "," << panel << endl;
    return true;
  }
  casacore::Bool createiterplotlabels( casacore::Vector<casacore::String> iteraxes,
				   casacore::Vector<casacore::Double> values,
				   casacore::String &titleString ) {
    //    cout << "iteraxes    = " << iteraxes << endl;
    //    cout << "values      = " << values << endl;

    titleString="    ";
    for (casacore::uInt i=0;i<iteraxes.nelements();++i) {

      if (iteraxes(i)=="ANTENNA1") {
	if (antNames_p.nelements()>0 && values(i)<antNames_p.nelements())
	  titleString = titleString + "  Antenna=\\'" + antNames_p((casacore::Int)values(i)) + "\\'";
	else
	  titleString = titleString + "  AntId=" + casacore::String::toString((casacore::Int)values(i));
      }
      if (iteraxes(i)=="FIELD_ID") {
	if (fieldNames_p.nelements()>0 && values(i)<fieldNames_p.nelements())
	  titleString = titleString + "  Field=\\'" + fieldNames_p((casacore::Int)values(i)) + "\\'";
	else
	  titleString = titleString + "  FldId=" + casacore::String::toString((casacore::Int)values(i));
      }
      if (iteraxes(i)==CDIcol()) {
	if (spwIds_p.nelements()>0 && values(i)<spwIds_p.nelements())
	  titleString = titleString + "  Spw=\\'" + casacore::String::toString(spwIds_p((casacore::Int)values(i))) + "\\'";
	else
	  titleString = titleString + "  CalId=" + casacore::String::toString((casacore::Int)values(i));
      }
      if (iteraxes(i)=="TIME") {
	titleString = titleString + "  casacore::Time=\\'" + casacore::MVTime(values(i)/casacore::C::day).string( casacore::MVTime::TIME,7) + "\\'";
      }
    }

    //    cout << "titleString = " << titleString << endl;
    
    return true;

  }

  casacore::Bool printlocateinfo(casacore::Vector<casacore::String> /*collist*/,
                             casacore::Matrix<casacore::Double> infomat,
                             casacore::Vector<casacore::String> cpol) {

    casacore::LogIO log;

    if (infomat.shape()[1]>0) {
      //      cout << "collist = " << collist << endl;
      //      cout << "infomat = " << infomat << endl;
      //      cout << "cpol    = " << cpol << endl;

      //      log << "Found the following calibration samples in the region:" << casacore::LogIO::POST;

      for (casacore::Int j=0;j<infomat.shape()[1];++j) {
        log << casacore::MVTime( infomat(4, j)/casacore::C::day).string( casacore::MVTime::YMD,7) << " ";

	if (antNames_p.nelements()>0 && casacore::uInt(infomat(2,j))<antNames_p.nelements())
	  log << " Antenna=" << "'" << antNames_p((casacore::Int)infomat(2,j)) << "'";
	else
	  log << " AntId=" << infomat(2,j);
	    
	if (spwIds_p.nelements()>0 && casacore::uInt(infomat(5,j))<spwIds_p.nelements())
	  log << " SpwId=" << spwIds_p((casacore::Int)infomat(5,j));
	else
	  log << " CalId=" << infomat(5,j);
	  
	if (fieldNames_p.nelements()>0 && casacore::uInt(infomat(3,j))<fieldNames_p.nelements())
	  log << " Field=" << "'" << fieldNames_p((casacore::Int)infomat(3,j)) << "'";
	else
	  log << " FldId=" << infomat(3,j);

	log << " pol,chan=" << cpol(j);
           
	log << casacore::LogIO::POST;

      }

    }
    return true;
  }

private:

  inline casacore::String CDIcol() { return (isNCT_p ? "SPECTRAL_WINDOW_ID" : "CAL_DESC_ID"); };


  casacore::Vector<casacore::String> fieldNames_p;
  casacore::Vector<casacore::String> antNames_p;
  casacore::Vector<casacore::Int> spwIds_p;

  casacore::Bool isNCT_p;

};


class PlotCalFreqAxes : public TPConvertChanToFreq
{
public :
  PlotCalFreqAxes(const casacore::Vector<casacore::Int>& cdlist,
		  const casacore::Vector<casacore::Double>& startFreq,
		  const casacore::Vector<casacore::Double>& stepFreq) {
    cdList = cdlist;
    offsetV = startFreq;
    intervalV = stepFreq;
    //    cout << cdList << " " << offsetV << " " << intervalV << endl;

  }
  ~PlotCalFreqAxes(){};
  casacore::Double offset,interval;
  inline casacore::Double Xconvert_col(casacore::Double x,casacore::Int /*row*/,casacore::Int tblNum){
    return x*intervalV(cdList(tblNum)) + offsetV(cdList(tblNum));
  }

private:
  casacore::Vector<casacore::Int> cdList;
  casacore::Vector<casacore::Double> offsetV;
  casacore::Vector<casacore::Double> intervalV;

};


class PlotCalParang : public TPConvertBase
{
public:
  PlotCalParang(casacore::Vector<casacore::Vector<casacore::Int> >& fldlists) {
    // Set up casacore::MSDerivedValues to do calc
    fldLists = fldlists;
  };
  ~PlotCalParang(){};
  inline casacore::Double Xconvert(casacore::Double x,
			 casacore::Int /*row*/,
			 casacore::Int /*tblNum*/) {
    //    cout << "PCP::Xconvert: " << tblNum << " " << row << " " << x 
    //	 << fldLists(tblNum)(row)
    //	 << endl;
    return x;
  }
private:

  casacore::Vector<casacore::Vector<casacore::Int> > fldLists;
};


} //# NAMESPACE CASA - END

#endif
