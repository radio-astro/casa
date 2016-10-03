//# MsPlotHooks.h: The hookcs (callback) classes given to TablePlot to allow
//                 MsPlot to control some of the plotting issues.
//# Copyright (C) 2003-2008
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
//# $Id:$
//#
//# -------------------------------------------------------------------------
//# Change Log
//# Date   Name       Description
//# 11/19/2007  S. Jaeger   Fixed up the message handling
//#


#if !defined CASA_MSPLOT_HOOKS_H
#define CASA_MSPLOT_HOOKS_H

//# System Includes
#include <stdio.h>
#include <iostream>
#include <iomanip>

//# General CASA includes
#include <casa/BasicSL/String.h>
#include <casa/Arrays/Slice.h>

//# casacore::Table and TablePlot includes
#include <tools/tables/TablePlot/TablePlot.h>

#include <flagging/Flagging/SLog.h>
#include <flagging/Flagging/Flagger.h>
#include <flagging/Flagging/ExtendFlagger.h>

#define LOG0 0
#define LOG2 1

namespace casa { //# NAMESPACE CASA - BEGIN

//#!//////////////////////////////////////////////////////////////////////////
//#! All the wonderful docs, that will show up in the user reference
//#! manual for this class.
    
// <summary>
// This class is provided to TablePlot to be called when someone
// resets the TablePlot class.
// </summary>
//
//#! <use visibility=local>   or   <use visibility=export>
// <use visibility=local>
//#!
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
//#! for example:
//#!  <reviewed reviewer="pshannon@nrao.edu" date="1994/10/10" tests="tMyClass, t1MyClass" demos="dMyClass, d1MyClass">
//#!  </reviewed>
// </reviewed>
//
//#! Classes or concepts you should understand before using this class.
// <prerequisite>
// <ul>    
//   <li> <linkto class="TPResetCallback">TPResetCallback</linkto>class,
//   <li> <linkto class="TablePlot">TablePlot</linkto>class, 
//   <li> <linkto class="casacore::Table">MsPlot</linkto> class
// </ul>    
// </prerequisite>
//    
// <etymology>
// Does the necessary cleanup, object destruction setting class variables
// to intial values, and such when a reset occurs.
// </etymology>
//
// <synopsis>
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// To provide a way for the TablePlot object to inform the MsPlot object
// that a reset has occured.    
// </motivation>
//
// <thrown>
//    <li> AIPSError
// </thrown>
//
// <todo asof="2007/10/29">
//   <li> Restructure MsPlot to subclass TablePlot and rewrite the
//        MsPlot reset command to do what it needs to do then call
//        the parents reset method.    
//   <li> Figure out why the plot window isn't being destroyed after
//        a reset
// </todo>
//#! End of documentation.    
//#!///////////////////////////////////////////////////////////////////////////
    
class MSPlotReset : public TPResetCallBack
{
 public:
    MSPlotReset( MsPlot *inMsPlot )
    {
       casacore::String fnname = "MSPlotReset";
       log = SLog::slog();
   log->FnEnter(fnname, clname);
   
   itsMsPlot = inMsPlot;

   log->FnExit(fnname, clname);
    };

    ~MSPlotReset() {
     casacore::String fnname = "MSPlotReset";
   log->FnEnter(fnname, clname);

   log->FnExit(fnname, clname);
    };

    casacore::Bool reset() {
     casacore::String fnname = "reset";
   log->FnEnter(fnname, clname);

   log->FnExit(fnname, clname);
   return true;
    };
    

 private:
    MsPlot *itsMsPlot;

    // Message logger
    SLog *log;
    static const casacore::String clname;
};

const casacore::String MSPlotReset::clname = "MSPlotReset"; 
//#!///////////////////////////////////////////////////////////////////////////
//#! 
//#!      HOOK CLASSES - All classes used with TablePlot, these classes are
//#!                     hooks into TablePlot.  They are called when regions
//#!                     have been "marked" and user has requested something 
//#!                     be done with these regions.
//#!
//#!///////////////////////////////////////////////////////////////////////////


//#!//////////////////////////////////////////////////////////////////////////
//#! All the wonderful docs, that will show up in the user reference
//#! manual for this class.
// <summary>
// This class is provided to TablePlot to allow MsPlot to do the proper thing
// when button's are pressed on the plotting GUI. 
// </summary>
//
//#! <use visibility=local>   or   <use visibility=export>
// <use visibility=local>
//#!
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
//#! for example:
//#!  <reviewed reviewer="pshannon@nrao.edu" date="1994/10/10" tests="tMyClass, t1MyClass" demos="dMyClass, d1MyClass">
//#!  </reviewed>
// </reviewed>
//
//#! Classes or concepts you should understand before using this class.
// <prerequisite>
// <ul>
//   <li> <linkto class="TPGuiCallBackHooks">TPGuiCallBackHooks</linkto>class,
//   <li> <linkto class="TablePlot">TablePlot</linkto>class, 
//   <li> <linkto class="casacore::Table">MsPlot</linkto> class
//   <li> <linkto class="casacore::Table">Measurement Set</linkto> class 
// </ul>    
// </prerequisite>
//    
// <etymology>
// Callbacks from TablePlot to MsPlot that are executed when buttons are
// pressed on the plotting GUI, and the data that has been plotted resides
// in the main table of the Measurement Set. 
// </etymology>
//
// <synopsis>
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// To provide a way for the TablePlot object to inform the MsPlot object
// that a particular button has been pressed on the plotting GUI and to
// allow casacore::MS Plot to over ride the default TablePlot behaviour if necessary. 
// </motivation>
//
// <thrown>
//    <li> AIPSError
// </thrown>
//
// <todo asof="2007/12/19">
//   <li> Implement the relaseTable method.
//   <li> Finish the documentation
//   <li> Restructure TablePlot to send rows of data, rather then single
//        data point.  This should improve the plotting speed as it will
//        allow reuse of calculations more easily.
//   <li> Build methods into MsPlot, or some other class that retrieves
//        information from the tables.  Examples of methods are, getFieldNames,
//        getAntNames, ...  This could help reduce the memory usage, reduce
//        code duplication, and improve robustness. 
//   <li> Restructure MsPlot to subclass TablePlot and rewrite the
//        MsPlot reset command to do what it needs to do then call
//        the parents reset method.    
// </todo>
//#! End of documentation.    
//#!///////////////////////////////////////////////////////////////////////////
class MSPlotMainMSCallBack : public TPGuiCallBackHooks
{
    public: 
      MSPlotMainMSCallBack( casacore::MeasurementSet *inMS,
         casacore::Vector<casacore::String> antNames,
         //casacore::Matrix<casacore::Int> baselines,
         casacore::Vector<casacore::String> fieldNames,
         casacore::Vector<casacore::Vector<casacore::String> >corrNames,
         casacore::Vector<casacore::Double> refFrequencies,
         casacore::String msname,
         casacore::String extendChn,
         casacore::String extendCor,
         casacore::String extendSp,
         casacore::String extendAnts,
         casacore::String extendTm,
         casacore::Bool crossbl,
         casacore::Double avetime,
         casacore::MeasurementSet *aMS)
     {
         log = SLog::slog();
         casacore::String fnname = "MSPlotMainMSCallBack";
#if LOG2 
         log->FnEnter(fnname + 
          "(inMS, antNames, fieldNames, corrNames, refFrequencies)", clname);
#endif 
                 
         localMS = inMS;
         itsAntNames = antNames;
         //itsBaselines = baselines;
         itsFieldNames = fieldNames;
         itsCorrNames = corrNames;
         itsRefFrequencies = refFrequencies;
         flagms = msname;
         extendChan = extendChn;
         extendCorr = extendCor;
         extendSpw = extendSp;
         extendAnt = extendAnts;
         extendTime = extendTm;
         aveBl = crossbl;
         aveTime = avetime;
         aveMS = aMS;
         
         LocateColumns.resize(5);
         LocateColumns[0] = "ANTENNA1";
         LocateColumns[1] = "ANTENNA2";
         LocateColumns[2] = "FIELD_ID";
         LocateColumns[3] = "TIME";
         LocateColumns[4] = "DATA_DESC_ID";
         //cout << "itsBaseLines=" << itsBaselines << endl;

#if LOG2 
         log->FnExit(fnname, clname);
#endif 
     };
      
      ~MSPlotMainMSCallBack(){
          casacore::String fnname =  "~MSPlotMainMSCallback";
#if LOG2 
          log->FnEnter(fnname, clname);

          log->FnExit(fnname, clname);
#endif 
      };
      
      casacore::Bool
      releasetable( casacore::Int /*nrows*/, casacore::Int /*ncols*/, casacore::Int /*panel*/, casacore::String /*tablename*/ )
      {
          casacore::String fnname = "releasetable";
#if LOG2 
          log->FnEnter(fnname + "(nrows,ncols,panel,tablename)", clname);
#endif 
          //# TODO free any data structures that were created
          //# with this class, maybe call the class destructor too.

#if LOG2 
          log->FnExit(fnname, clname);
#endif 
          return true;
      }

      casacore::Bool
      createiterplotlabels(casacore::Vector<casacore::String> iteraxes,
                           casacore::Vector<casacore::Double> values, casacore::String &titleString) {
         casacore::String fnname = "createiterplotlabels";
#if LOG2 
         log->FnEnter(fnname + "(iteraxes, values, titleString)", clname );
#endif 
         //cout << " values=" << values << endl;
         //for (casacore::uInt i = 0; i < itsAntNames.nelements(); i++)
         //   cout << itsAntNames[i] << " " ;
         //cout << endl;

         titleString = "Iter: ";
         //cout << "main call back createiterplotlabels" << endl;  
         casacore::uInt iterNum = iteraxes.nelements();
         for (uint axesId = 0; axesId < iterNum; axesId++) {
            if (upcase(iteraxes[axesId]).matches("BASELINE") || 
                upcase(iteraxes[axesId]).matches("ANTENNA") ||
                upcase(iteraxes[axesId]).matches("ANTENNA1") || 
                upcase(iteraxes[axesId]).matches("ANTENNA2")) {
               //cout << "iteraxes[axesId]=" << iteraxes[axesId] << endl;
               if (iterNum == 2 && 
                   upcase(iteraxes[0]).contains("ANTENNA") && 
                   upcase(iteraxes[1]).contains("ANTENNA")) {
                  if (axesId == 0)
                     titleString += "Baseline ";
               } 
               else {
                  titleString += casacore::String("Antenna") +
                        (iteraxes[axesId].contains("2") ? "2" : "1")
                        + casacore::String(" ");
               }
            
               if (values[axesId] < itsAntNames.nelements()) {
                  if (axesId > 0)
                     titleString += casacore::String(" : ");
                  titleString += itsAntNames[casacore::Int(values[axesId])];
               } 
               else {
#if LOG2 
                   ostringstream os;
                   os << "Internal error! Antenna ID is "
                      << values[axesId]
                      << ", but there are only "
                      << itsAntNames.nelements()
                      << ".";
                   log->out(os, fnname, clname, casacore::LogMessage::DEBUG1); 
#endif 
               }
               
            } 
            else if (upcase(iteraxes[axesId]).matches("FIELD") || 
                    upcase(iteraxes[axesId]).matches("FIELD_ID") ) {
                titleString += "FIELD ";
                if ( values[axesId] < itsFieldNames.nelements() ) {
                    if ( axesId > 0 )
                        titleString += casacore::String( " : " );
                    titleString += itsFieldNames[casacore::Int(values[axesId])];
                } 
                else {
#if LOG2 
                  ostringstream os;
                  os << "Internal error! Field ID is "
                     << values[axesId]
                     << ", but there are only "
                     << itsFieldNames.nelements()
                     << ".";
                  log->out(os, fnname, clname, casacore::LogMessage::WARN); 
#endif 
               }
            } else {
                //# This includes SPW, SCAN, FEED, and ARRAY
                //# since all of these don't have names.  This
                //# may change in the future.
                titleString += iteraxes[axesId] + casacore::String( " ");
    
                if ( axesId > 0 )
                    titleString += casacore::String( " : " );
                titleString += casacore::String::toString( casacore::Int(values[axesId]) );
            }
         }
         //cout << "titleString=" << titleString << endl;
         
         /*
         casacore::Bool selected = false;
         if (titleString.contains("Base")) {
            casacore::String selBase = titleString.after("Baseline ");
            for (casacore::Int k = 0; k < itsBaselines.shape()[0]; k++) {
               casacore::String inListBase = itsAntNames(itsBaselines(k, 0))
                                   + " : "
                                   + itsAntNames(itsBaselines(k,1));
               //cout << "inListBase=" << inListBase
               //     << " selBase=" << selBase << endl;
               if (inListBase == selBase)
                  selected = true;
            }
            //cout << "Baseline selected=" << selected << endl;
            return selected;
         }
         if (titleString.contains("Ante")) {
            casacore::String selAnt = titleString.after("Antenna");
            if (selAnt.length() < 2)
               selected = false;
            else if (selAnt.before(" ") == "2") {
               selAnt = selAnt.after(" ");
               for (casacore::Int k = 0; k < itsBaselines.shape()[0]; k++) {
                   casacore::String inListBase = itsAntNames(itsBaselines(k, 1));
                   //cout << "inListA2=" << inListBase
                   //     << " selAnt=" << selAnt << endl;
                   if (inListBase == selAnt)
                      selected = true;
               }
            }
            else {
               selAnt = selAnt.after(" ");
               for (casacore::Int k = 0; k < itsBaselines.shape()[0]; k++) {
                   casacore::String inListBase = itsAntNames(itsBaselines(k, 0));
                   //cout << "inListA1=" << inListBase
                   //     << " selAnt=" << selAnt << endl;
                   if (inListBase == selAnt)
                      selected = true;
               }
            }
            //cout << "selected=" << selected << endl;
            return selected;
         }
         */ 

         log->FnExit(fnname, clname);
         return true;
     };
         
     casacore::Bool
     flagdata(casacore::String /*tablename*/) {
#if LOG2 
         casacore::String fnname = "flagdata";
         log->FnEnter(fnname, clname);
         log->FnExit(fnname, clname);
#endif 
         return true;
     }

      casacore::Bool
      flagdisplay(casacore::Int direction, casacore::Vector<casacore::String> /*collist*/,
                casacore::Matrix<casacore::Double> infomat, casacore::Vector<casacore::String> cpol) {
          casacore::String fnname = "flagdisplay";
          //cout << "MSPlotMainMSCallback::flagdisplay flag=" << direction << endl;
         
          casacore::IPosition mshape = infomat.shape();
          //cout << "infomat.shape=[" << mshape[0] << ", "
          //     << mshape[1] << "] cpol.size=" << cpol.size() << endl;

          //cout << "flag display now...." << endl;
          //cout << "aveMS=" << aveMS << endl;
          //cout << "aveMS=" << *aveMS << endl;
          //cout << "collist=" << collist << endl;
          //cout << "infomat=" << infomat << endl;
          //cout << "cpol=" << cpol << endl;
//--------------------------------------------------------------
          casacore::ROMSDataDescColumns dataDescCol(aveMS->dataDescription());
          casacore::ROMSSpWindowColumns spwColumn(aveMS->spectralWindow());

          //casacore::Int maxrows = ms.nrow();
          casacore::Int maxrows = 0;
          for (casacore::Int i = 0; i < mshape[1]; i++) {
              maxrows += (casacore::Int)infomat(1, i);
          }
          //cout << "maxrows=" << maxrows << endl;
          casacore::Vector<FlagID> flagids(maxrows);

          casacore::Int idCount = 0;
          casacore::String warnings = casacore::String("Bad row:");
          for (casacore::Int i = 0; i < mshape[1]; i++) {
             if (mshape[0] != 7) {
                warnings += casacore::String(" ") + casacore::String::toString(i);
                break;
             }

             FlagID flagid;
             flagid.rowNum = (casacore::uInt)infomat(0, i);
             //flagid.npts = (casacore::uInt)infomat(1, i);
             if (infomat(2, i) < 0 || 
                 infomat(2, i) >= itsAntNames.nelements()) {
                warnings += casacore::String(" ") + casacore::String::toString(i);
                break;
             }
             flagid.ant1 = (casacore::uInt)infomat(2, i);
             if (infomat(3, i) < 0 || 
                infomat(3, i) >= itsAntNames.nelements()) {
                warnings += casacore::String(" ") + casacore::String::toString(i);
                break;
             }
             flagid.ant2 = (casacore::uInt)infomat(3, i);
             if (infomat(4, i) < 0 || 
                infomat(4, i) >= itsFieldNames.nelements()) { 
                warnings += casacore::String(" ") + casacore::String::toString(i);
                break;
             }
             flagid.field = (casacore::uInt)infomat(4, i);
             
             if (aveTime > 0.) {
                //cout << "flag aved" << endl;
                casacore::Double start = max(infomat(5, i) - aveTime / 2, 0.);
                casacore::Double end = max(infomat(5, i) + aveTime / 2, start);
                flagid.time = casacore::MVTime(start / casacore::C::day).string(casacore::MVTime::YMD, 7) +
                              casacore::String("~") +   
                              casacore::MVTime(end / casacore::C::day).string(casacore::MVTime::YMD, 7);
             }
             else {
                casacore::String ltime = 
                     casacore::MVTime(infomat(5, i)/casacore::C::day).string(casacore::MVTime::YMD, 7);
                //cout << "ltime=" << ltime << endl;
                casacore::String ntime = 
                     casacore::MVTime((infomat(5, i) + 0.1)/casacore::C::day).string(casacore::MVTime::YMD, 7);
                //cout << "ntime=" << ntime << endl;
                flagid.time = ltime + casacore::String("~") + ntime; 
             }

             if (infomat(6, i) < 0 || 
                infomat(6, i) >= dataDescCol.spectralWindowId().nrow()) { 
                warnings += casacore::String(" ") + casacore::String::toString(i);
                break;
             }
             flagid.spw = dataDescCol.spectralWindowId()((casacore::Int)infomat(6, i));
             if (infomat(6, i) < 0 || 
                 infomat(6, i) >= dataDescCol.polarizationId().nrow()) { 
                warnings += casacore::String(" ") + casacore::String::toString(i);
                break;
             }
             casacore::Int polId = dataDescCol.polarizationId()((casacore::Int)infomat(6, i));
             if (itsCorrNames[polId].nelements() <= 0) {
                warnings += casacore::String(" ") + casacore::String::toString(i);
                break;
             }
            
             //cout << "cpol=" << cpol(i) << endl;
             //cout << "cpol.size()=" << cpol(i).size() << endl;
             //cout << "cpol.length()=" << cpol(i).length() << endl;
             casacore::Int cCnt = cpol(i).freq(']');
             casacore::Int k = 0;
             casacore::Int l = 0;
             for (casacore::Int j = 0; j < cCnt; j++) {
                l = cpol(i).index(']', k) + 1; 
                casacore::String sect = cpol(i).substr(k, l);
                sect = sect.after('[');
                sect = sect.before(']');
                //cout << "sect=" << j << " " << sect << endl;
                casacore::String corr = sect.before(',');
                casacore::String chan = sect.after(',');
                //cout << "corr=" << corr << " chan=" << chan << endl; 
                casacore::Int corrId = atoi(corr.chars());
                flagid.corr = itsCorrNames[polId][corrId];
                //cout << "corr=" << flagid.corr << endl; 
                casacore::Int colCnt = chan.freq(':');
                if (colCnt == 2) {
                   casacore::Int fst = chan.index(':');
                   fst = chan.index(':', fst + 1);
                   casacore::String s = chan.substr(0, fst);
                   //cout << "s=" << s << endl;
                   flagid.chan = s.before(':') + casacore::String('~') + s.after(':');
                }
                else if (colCnt == 1) {
                   flagid.chan = chan.before(':') + casacore::String('~') + chan.after(':');
                }
                else {
                   flagid.chan = chan;
                }
                //cout << "chan=" << flagid.chan << endl; 
                //show flag base
                //flagid.show();
                flagids(idCount++) = flagid; 
                k = l;
             }
          }
          flagids.resize(idCount, true);
          //cout << "flagids.size()=" << flagids.size() << endl;
         
          
          casacore::Vector<casacore::Int> spwNumChan = spwColumn.numChan().getColumn();
          //cout << "spwNumChan=" << spwNumChan << endl;
          //spwNumChan[0] = 4;
          //spwNumChan[1] = 1;

//---------------------------------------------------------------
          if (aveBl) {
             extendAnt="ALL";
          }

          ExtendFlagger eflg;
          eflg.attach(*aveMS);

          casacore::Bool ret;
          ret = eflg.initdata();
          eflg.setdata();
          eflg.setExtend(extendChan, extendCorr, extendSpw,
                         extendAnt, extendTime, itsCorrNames, spwNumChan);
          eflg.setUnflag(!casacore::Bool(direction));

          //for (casacore::Int i = 0; i < flagids.nelements(); i++) {
          //   flagids[i].show();
          //}
          ret = eflg.extend(flagids);
          //cout << "flag display: " << ret << endl;
	  ((void) ret);

          return true;

      }

      casacore::Bool
      flagdata(casacore::Int direction, casacore::Vector<casacore::String> /*collist*/,
                casacore::Matrix<casacore::Double> infomat, casacore::Vector<casacore::String> cpol, casacore::Bool /*ave*/) {
          casacore::String fnname = "flagdata";
          //cout << "MSPlotMainMSCallback::flagdata flag=" << direction << endl;
         
          casacore::IPosition mshape = infomat.shape();
          //cout << "infomat.shape=[" << mshape[0] << ", "
          //     << mshape[1] << "] cpol.size=" << cpol.size() << endl;

          if (mshape[1] < 1 || cpol.size() < 1) 
             return true;

          //this can be commented out 
          //if (ave == true) {
          //   return flagdisplay(direction, collist, infomat, cpol);
          //}
//--------------------------------------------------------------
          casacore::ROMSDataDescColumns dataDescCol(localMS->dataDescription());


          //cout << "flagms=" << flagms << endl; 
          casacore::MS ms(flagms, casacore::Table::Update);

          casacore::ROMSSpWindowColumns spwColumn(ms.spectralWindow());

          //casacore::Int maxrows = ms.nrow();
          casacore::Int maxrows = 0;
          for (casacore::Int i = 0; i < mshape[1]; i++) {
              maxrows += (casacore::Int)infomat(1, i);
          }

          //cout << "maxrows=" << maxrows << endl;
          casacore::Vector<FlagID> flagids(maxrows);

          casacore::Int idCount = 0;
          casacore::String warnings = casacore::String("Bad row:");
          for (casacore::Int i = 0; i < mshape[1]; i++) {
             if (mshape[0] != 7) {
                warnings += casacore::String(" ") + casacore::String::toString(i);
                break;
             }

             FlagID flagid;
             flagid.rowNum = (casacore::uInt)infomat(0, i);
             //flagid.npts = (casacore::uInt)infomat(1, i);
             if (infomat(2, i) < 0 || 
                 infomat(2, i) >= itsAntNames.nelements()) {
                warnings += casacore::String(" ") + casacore::String::toString(i);
                break;
             }
             flagid.ant1 = (casacore::uInt)infomat(2, i);
             if (infomat(3, i) < 0 || 
                infomat(3, i) >= itsAntNames.nelements()) {
                warnings += casacore::String(" ") + casacore::String::toString(i);
                break;
             }
             flagid.ant2 = (casacore::uInt)infomat(3, i);
             if (infomat(4, i) < 0 || 
                infomat(4, i) >= itsFieldNames.nelements()) { 
                warnings += casacore::String(" ") + casacore::String::toString(i);
                break;
             }
             flagid.field = (casacore::uInt)infomat(4, i);
             
             if (aveTime > 0.) {
                //cout << "flag aved" << endl;
                casacore::Double start = max(infomat(5, i) - aveTime / 2, 0.);
                casacore::Double end = max(infomat(5, i) + aveTime / 2, start);
                flagid.time = casacore::MVTime(start / casacore::C::day).string(casacore::MVTime::YMD, 7) +
                              casacore::String("~") +   
                              casacore::MVTime(end / casacore::C::day).string(casacore::MVTime::YMD, 7);
             }
             else {
                casacore::String ltime = 
                     casacore::MVTime(infomat(5, i)/casacore::C::day).string(casacore::MVTime::YMD, 7);
                //cout << "ltime=" << ltime << endl;
                //casacore::String ntime = 
                //    casacore::MVTime((infomat(5, i) + 0.1)/casacore::C::day).string(casacore::MVTime::YMD, 7);
                //cout << "ntime=" << ntime << endl;
                //flagid.time = ltime + casacore::String("~") + ntime; 
                flagid.time = ltime; 
             }

             if (infomat(6, i) < 0 || 
                infomat(6, i) >= dataDescCol.spectralWindowId().nrow()) { 
                warnings += casacore::String(" ") + casacore::String::toString(i);
                break;
             }
             flagid.spw = dataDescCol.spectralWindowId()((casacore::Int)infomat(6, i));
             if (infomat(6, i) < 0 || 
                 infomat(6, i) >= dataDescCol.polarizationId().nrow()) { 
                warnings += casacore::String(" ") + casacore::String::toString(i);
                break;
             }
             casacore::Int polId = dataDescCol.polarizationId()((casacore::Int)infomat(6, i));
             if (itsCorrNames[polId].nelements() <= 0) {
                warnings += casacore::String(" ") + casacore::String::toString(i);
                break;
             }
            
             //cout << "cpol=" << cpol(i) << endl;
             //cout << "cpol.size()=" << cpol(i).size() << endl;
             //cout << "cpol.length()=" << cpol(i).length() << endl;
             casacore::Int cCnt = cpol(i).freq(']');
             casacore::Vector<casacore::String> chans(4);
             for (casacore::Int s = 0; s < 4; s++)
                chans(s) = "";
             casacore::Int k = 0;
             casacore::Int l = 0;
             casacore::String tmpChan = "";
             casacore::String tmpCorr = "";
             casacore::Int secCnt = 0;
             for (casacore::Int j = 0; j < cCnt; j++) {
                l = cpol(i).index(']', k) + 1; 
                casacore::String sect = cpol(i).substr(k, l);
                sect = sect.after('[');
                sect = sect.before(']');
                //cout << "sect=" << j << " " << sect << endl;
                casacore::String corr = sect.before(',');
                casacore::String chan = sect.after(',');
                //cout << "corr=" << corr << " chan=" << chan << endl; 
                casacore::Int corrId = atoi(corr.chars());
                tmpCorr = itsCorrNames[polId][corrId];

                casacore::Int colCnt = chan.freq(':');
                if (secCnt > 0) {
                   tmpChan += ";";
                }
                if (colCnt == 2) {
                   casacore::Int fst = chan.index(':');
                   fst = chan.index(':', fst + 1);
                   casacore::String s = chan.substr(0, fst);
                   //cout << "s=" << s << endl;
                   tmpChan += (s.before(':') + casacore::String('~') + s.after(':'));
                }
                else if (colCnt == 1) {
                   tmpChan += (chan.before(':') + casacore::String('~') + chan.after(':'));
                }
                else {
                   tmpChan += chan;
                }
                chans(corrId) = tmpChan;
                secCnt++;
                k = l;
             }
             for (casacore::Int s = 0; s < 4; s++) {
                if (chans(s) != "") {
                   flagid.chan = chans(s);
                   flagid.corr = itsCorrNames[polId][s];
                   flagids(idCount++) = flagid; 
                   //show flag base
                   //flagid.show();
                }
             }
          }
          if (idCount < 1)
              return true;

          flagids.resize(idCount, true);
          //cout << "flagids.size()=" << flagids.size() << endl;
          //for (casacore::uInt i = 0; i < flagids.size(); i++) {
          //    flagids(i).show();
          //}

          casacore::Vector<casacore::Int> spwNumChan = spwColumn.numChan().getColumn();

//---------------------------------------------------------------
          if (aveBl) {
             extendAnt="ALL";
          }

          //cout << "sizeof(uint)=" << 8 * sizeof(uint) << endl;
          casacore::Int maxSel = 8 * sizeof(uint) - 5;
          casacore::Int nSel = maxSel; 
          casacore::Int start = 0;
          while (start < idCount) {

             nSel = min(idCount - start, maxSel);

             ExtendFlagger eflg;
             eflg.attach(ms);
             //cout << "ms=" << ms << endl;

             casacore::Bool ret;
             ret = eflg.initdata();
             eflg.setdata();
             eflg.setExtend(extendChan, extendCorr, extendSpw,
                         extendAnt, extendTime, itsCorrNames, spwNumChan);
             eflg.setUnflag(!casacore::Bool(direction));

             //cout << "start=" << start << " nSel=" << nSel 
             //     << " iCount=" << idCount
             //     << endl;
             ret = eflg.extend(flagids(casacore::Slice(start, nSel)));
	     ((void) ret);

             //eflg.detach();
             start += nSel;
          } 
          //cout << "Flagger return: " << ret << endl;

          //eflg.setSpw("0:3");
          //eflg.setCorrelation("lr");
          //eflg.selectdata();
          //eflg.setmanualflags(false);
          //eflg.run();
        
          //Flagger flagger;
          //
          //flagger.attach(ms);
          //
          //casacore::Bool useoriginalms=true;
          //
          //casacore::String field="";
          //casacore::String spw="";
          //casacore::String array="";
          //casacore::String feed="";
          //casacore::String scan="";
          //casacore::String baseline="";
          //casacore::String uvrange="";
          //casacore::String time="";
          //casacore::String correlation="";
          //
          //casacore::Bool ret = false;
          //ret = flagger.setdata(field, spw, array, 
          //       feed, scan, baseline, uvrange, time, correlation);
          //
          //cout << "------setdata=" << ret << endl;
          //
          //casacore::Bool autocorr = false;
          //casacore::String clipexpr = "";
          //casacore::Vector<casacore::Double> cliprange(0);
          //casacore::String clipcolumn = "DATA";
          //casacore::Bool outside = false;
          //casacore::Double quackinterval = 0.0;
          //casacore::String opmode = "flag";
          //
          //casacore::Bool unflag = false;
          //spw = "0:3";
          //correlation = "lr";
          //
          //cout << "useoriginalms=" << useoriginalms
          //     << " field=" << field << " spw=" << spw
          //     << " array=" << array << " feed=" << feed
          //     << " scan=" << scan << " baseline=" << baseline
          //     << " uvrange=" << uvrange << " time=" << time
          //<< " correlation=" << correlation << endl;
          // 
          //ret = flagger.selectdata(useoriginalms, field, spw, array, 
          //        feed, scan, baseline, uvrange, time, correlation);
          //cout << "------selectdata=" << ret << endl;
          //
          //cout << "autocorr=" << autocorr
          //   << " unflag=" << unflag
          //   << " clipexpr=" << clipexpr << " cliprange=" << cliprange
          //   << " clipcolumn=" << clipcolumn << " outside=" << outside
          //   << " quackinterval=" << quackinterval << " opmode=" << opmode
          //   << endl;
          //
          //ret = flagger.setmanualflags(autocorr, unflag, 
          //      clipexpr, cliprange, clipcolumn, outside, 
          //      quackinterval, opmode);
          //cout << "------setmanualflags=" << ret << endl;
          //
          //ret = flagger.selectdata(useoriginalms, field, spw, array, 
          //                feed, scan, baseline, uvrange, time, correlation);
          //cout << "------selectdata=" << ret << endl;
          //
          //ret = flagger.setmanualflags(false, unflag, 
          //      clipexpr, cliprange, clipcolumn, outside, 
          //      quackinterval, "SUMMARY");
          //cout << "------summary=" << ret << endl;
          //
          //flagger.run(false, false);
          //cout << "------run=" << ret << endl;
          return true;

      }

      casacore::Bool
      printlocateinfo( casacore::Vector<casacore::String> /*collist*/,
                  casacore::Matrix<casacore::Double> infomat,casacore::Vector<casacore::String> cpol)
      {
          casacore::String fnname = "printlocateinfo";
#if LOG2 
          log->FnEnter(fnname + "(collist, infomat, cpol)", clname);
#endif 
          //collist never used!

          //cout << "cpol=" << cpol << endl;

          //# Define the column headings and other reptitive strings used.
          casacore::Vector<casacore::uInt> columnWidths(9);
          columnWidths[0]=8; columnWidths[1]=8; columnWidths[2]=9;
          columnWidths[3]=25; columnWidths[4]=22; columnWidths[5]=6;
          columnWidths[6]=10; columnWidths[7]=14; columnWidths[8]=30; 
         
          casacore::String colNames;
          ostringstream tmp;
     tmp << std::setw( columnWidths[0] ) <<  "ROW #   " << " | "
         << std::setw( columnWidths[1] ) <<  "# Points" << " |"
         << std::setw( columnWidths[2] ) <<  "Baseline" << "| "
         << std::setw( columnWidths[3] ) <<  "Field ID/Name      "  << " | "
         << std::setw( columnWidths[4] ) <<  "casacore::Time         "  << " | "
         << std::setw( columnWidths[5] ) <<  "SPW ID" << " | "
         << std::setw( columnWidths[6] ) <<  "Ref. Freq." << " | "
         << std::setw( columnWidths[7] ) <<  "Corr. Id/Name"  << " |"
         << std::setw( columnWidths[8] ) <<  "[pol /chan]" << " | "
         << endl;
          colNames += casacore::String( tmp );
          tmp.str( "" );

          //# get the DATA_DESC_ID column so we can extract polarization
          //# and spectral window information.
          casacore::ROMSDataDescColumns dataDescCol( localMS->dataDescription());

          casacore::IPosition mshape = infomat.shape();
          casacore::String warnings;
          casacore::String info;
          if (mshape[1] > 0)
             info += colNames;
          
          //cout << "infomat.shape=[" << mshape[0] << ", "
          //     << mshape[1] << "] cpol.size=" << cpol.size() << endl;
//          for(casacore::Int i=0; i < mshape[1]; i++)
//          {
//         if ( mshape[0] != 7 )
//         {
//             warnings += casacore::String( "Skipping the " )
//            + casacore::String::toString( i )
//            + casacore::String( " set of data, its not a complete row. " );
//             break;
//         }
//
//         //# Put the Row number and the # of points in the row
//         tmp << std::noshowpoint << std::setw( columnWidths[0] )
//             << casacore::String::toString( infomat( 0, i ) ) << " | "
//             << std::noshowpoint << std::setw( columnWidths[1] )
//             << infomat( 1, i ) << " |  ";
//
//         //# Printout the Baseline, note that the information
//         //# we are provided is the antenna IDs, but we want
//         //# to display the antenna names.
//         casacore::String baseline;
//         if ( infomat(2,i) < 0
//            || infomat(2,i) > itsAntNames.nelements() )
//             baseline += casacore::String::toString( infomat(2,i) )
//            + casacore::String( "(ID)" );
//         else 
//             baseline += itsAntNames( (casacore::uInt)infomat(2,i) );
//         baseline += casacore::String( "/" );
//         if ( infomat(3,i) < 0
//            || infomat(3,i) > itsAntNames.nelements() )
//             baseline += casacore::String::toString( infomat(3,i) )
//            + casacore::String( "(ID)" );
//         else 
//             baseline += itsAntNames( (casacore::uInt)infomat(3,i) );
//         baseline += "| ";
//         tmp << std::setw( columnWidths[2] )
//             << baseline;
//         
//
//         //# Print out the Field ID, and Field Name
//         casacore::String fieldStr = casacore::String::toString(infomat(4,i) )
//             + casacore::String( "/" );
//         if ( infomat(4,i) < itsFieldNames.nelements() ) 
//             fieldStr += itsFieldNames[ casacore::Int( infomat(4,i) ) ];
//         else {
//             warnings += casacore::String( "Field ID, " )
//            + casacore::String::toString( casacore::Int( infomat(4,i) ) )
//            + casacore::String( ", is larger then the" )
//            + casacore::String( " number of field names, " )
//            + casacore::String::toString( itsFieldNames.nelements() )
//            + casacore::String( ". \n" );
//             fieldStr += "??";
//         }
//             tmp << std::noshowpoint << std::setw( columnWidths[3] )
//            << fieldStr << " | ";
//
//         //Print out the time in YY/MM/DD/hh:mm:ss.s format
//         tmp << std::setw( columnWidths[4] )
//             << casacore::MVTime( infomat(5, i)/casacore::C::day).string( casacore::MVTime::DMY,7)
//             << " | ";
//
//         //# spectral window IDs and Ref Frequency
//         casacore::Int spwId = dataDescCol.spectralWindowId()( (casacore::Int)infomat(6, i ) );
//         tmp << std::noshowpoint << std::setw( columnWidths[5] )
//             << spwId << " | ";
//
//         if ( spwId < (casacore::Int)itsRefFrequencies.nelements() ) 
//             tmp << std::showpoint
//            << std::setprecision( columnWidths[6]-7)
//            << std::setw( columnWidths[6] )
//            << itsRefFrequencies[ spwId ] << " | ";
//         else {
//             warnings += casacore::String( "SPW ID, " )
//            + casacore::String::toString( spwId )
//            + casacore::String( ", is larger then the" )
//            + casacore::String( " number of reference frequencies, " )
//            + casacore::String::toString( itsRefFrequencies.nelements() )
//            + casacore::String( ". \n" );
//             tmp << " ?? " << " | ";
//         }
//         
//         
//         //# Polarization ID and list of polls and chans selected
//         //#  in this row.
//         //# TODO add some check on spw ID here
//         casacore::Int polId = dataDescCol.polarizationId()( (casacore::Int)infomat(6, i ) );
//         casacore::String polStr = casacore::String::toString( polId ) + casacore::String( "/" );
//         if ( itsCorrNames[polId].nelements() > 0 )
//             for ( casacore::Int corrId=0;
//              corrId < (casacore::Int)itsCorrNames[polId].nelements();
//              corrId++ ) {
//            if ( corrId > 0 )
//                polStr += casacore::String( ", " );
//            polStr += itsCorrNames[polId][corrId];
//             }
//         else {
//             warnings += casacore::String( "Pol ID, " )
//            + casacore::String::toString( polId )
//            + casacore::String( ", is larger then the" )
//            + casacore::String( " number of polarizations, " )
//            + casacore::String::toString( itsCorrNames[polId].nelements() )
//            + casacore::String( ", \n" );
//             polStr += "??";
//         }
//         tmp << std::noshowpoint << std::setw( columnWidths[7] )
//             << polStr << " | ";
//
//         tmp  << std::setw( columnWidths[8] )
//              << cpol(i)  << "| " << endl;
//         
//         info += casacore::String( tmp );
//         tmp.str( "" );         
//          }

          for (casacore::Int i=0; i < mshape[1]; i++) {
             if ( mshape[0] != 7 ) {
                warnings += casacore::String( "Skipping the " )
                         + casacore::String::toString( i )
                         + casacore::String( " set of data, its not a complete row. " );
                break;
             }

             tmp.str("");         

             //# Put the Row number and the # of points in the row
             tmp << std::noshowpoint << std::setw( columnWidths[0] )
                    << casacore::String::toString(infomat(0, i) ) << " | "
                    << std::noshowpoint << std::setw( columnWidths[1] )
                    << infomat(1, i) << " |  ";

             //# Printout the Baseline, note that the information
             //# we are provided is the antenna IDs, but we want
             //# to display the antenna names.
             casacore::String baseline;
             if (infomat(2, i) < 0 || infomat(2, i) >= itsAntNames.nelements()) {
                 baseline += casacore::String::toString(infomat(2, i)) + casacore::String("(ID)");
                 continue;
             }
             else 
                 baseline += itsAntNames((casacore::uInt)infomat(2, i));
             baseline += casacore::String( "/" );
             if (infomat(3, i) < 0 || infomat(3, i) >= itsAntNames.nelements()) {
                 baseline += casacore::String::toString(infomat(3, i)) + casacore::String("(ID)");
                 continue;
             }
             else 
                 baseline += itsAntNames((casacore::uInt)infomat(3, i));
             baseline += "| ";
             tmp << std::setw( columnWidths[2]) << baseline;
         
             //# Print out the Field ID, and Field Name
             casacore::String fieldStr = casacore::String::toString(infomat(4, i)) + casacore::String("/");
             if (infomat(4, i) < 0 || infomat(4, i) >= itsFieldNames.nelements()) { 
                warnings += casacore::String( "Field ID, " )
                         + casacore::String::toString(casacore::Int(infomat(4, i)))
                         + casacore::String(" does not exist" )
                         + casacore::String( ". \n" );
                continue;
             }
             else {
                fieldStr += itsFieldNames[casacore::Int(infomat(4, i))];
             }

             tmp << std::noshowpoint << std::setw(columnWidths[3])
                    << fieldStr << " | ";

             //Print out the time in YY/MM/DD/hh:mm:ss.s format
             tmp << std::setw(columnWidths[4])
                    << casacore::MVTime(infomat(5, i)/casacore::C::day).string(casacore::MVTime::DMY, 7)
                    << " | ";

             //# spectral window IDs and Ref Frequency
             if (infomat(6, i) < 0 || 
                 infomat(6, i) >= dataDescCol.spectralWindowId().nrow()) { 
                warnings += casacore::String("row ")
                         + casacore::String::toString(casacore::Int(infomat(6, i)))
                         + casacore::String(" does not exist in SpectralWindow")
                         + casacore::String(". \n");
                continue;
             }

             casacore::Int spwId = dataDescCol.spectralWindowId()((casacore::Int)infomat(6, i));
             tmp << std::noshowpoint << std::setw(columnWidths[5])
                    << spwId << " | ";

             if (spwId < (casacore::Int)itsRefFrequencies.nelements()) { 
                tmp << std::showpoint
                       << std::setprecision( columnWidths[6]-7)
                       << std::setw(columnWidths[6])
                       << itsRefFrequencies[spwId] << " | ";
             }
             else {
                 warnings += casacore::String("SPW ID, ")
                          + casacore::String::toString(spwId)
                          + casacore::String(" number of reference frequencies, ")
                          + casacore::String::toString(itsRefFrequencies.nelements())
                          + casacore::String(". \n" );
                 tmp << " ?? " << " | ";
             }
         
             //# Polarization ID and list of polls and chans selected
             //#  in this row.
             //# TODO add some check on spw ID here
             if (infomat(6, i) < 0 || 
                 infomat(6, i) >= dataDescCol.polarizationId().nrow()) { 
                warnings += casacore::String("row ")
                         + casacore::String::toString(casacore::Int(infomat(6, i)))
                         + casacore::String(" does not exist in Polarization")
                         + casacore::String( ". \n" );
                continue;
             }

             casacore::Int polId = dataDescCol.polarizationId()((casacore::Int)infomat(6, i));
             casacore::String polStr = casacore::String::toString(polId) + casacore::String( "/" );
             if (itsCorrNames[polId].nelements() > 0) {
                for (casacore::Int corrId=0;
                     corrId < (casacore::Int)itsCorrNames[polId].nelements(); corrId++ ) {
                   if (corrId > 0)
                      polStr += casacore::String( ", " );
                   polStr += itsCorrNames[polId][corrId];
                }
             }
             else {
                warnings += casacore::String("Pol ID, ")
                         + casacore::String::toString( polId )
                         + casacore::String( ", is larger then the" )
                         + casacore::String( " number of polarizations, " )
                         + casacore::String::toString( itsCorrNames[polId].nelements() )
                         + casacore::String( ", \n" );
                polStr += "??";
            }
            tmp << std::noshowpoint << std::setw( columnWidths[7] )
                   << polStr << " | ";

            tmp << std::setw( columnWidths[8] )
                   << cpol(i)  << "| " << endl;
         
            info += casacore::String(tmp);
          }

          //if (mshape[1] > 0)
          //info += colNames;
          //# Display the information in the list.
          if ( info.length() > colNames.length() )
             log->out(info, fnname, clname, casacore::LogMessage::NORMAL);

          //# Display any warnings encountered.
          if (warnings.length() > 0)
             //log->out(warnings, fnname, clname, casacore::LogMessage::WARN);
             log->out("Error occured while locating data points", 
                  fnname, clname, casacore::LogMessage::WARN);

#if LOG2 
          log->FnExit(fnname, clname);
#endif 
          return true;
      };
    private:
      casacore::MeasurementSet *localMS;             
      casacore::MeasurementSet *aveMS;             
      casacore::Vector<casacore::String> itsAntNames;
      //casacore::Matrix<casacore::Int> itsBaselines;
      casacore::Vector<casacore::String> itsFieldNames;
      casacore::Vector< casacore::Vector<casacore::String> > itsCorrNames;
      casacore::Vector<casacore::Double> itsRefFrequencies;
      casacore::String flagms;
      casacore::String extendChan;
      casacore::String extendCorr;
      casacore::String extendSpw;
      casacore::String extendAnt;
      casacore::String extendTime;
      casacore::Bool aveBl;
      casacore::Double aveTime;

      // Message logger
      SLog* log;         
      static const casacore::String clname;
};

const casacore::String MSPlotMainMSCallBack::clname = "MSPlotMainMSCallBack";
//#!//////////////////////////////////////////////////////////////////////////
//#! All the wonderful docs, that will show up in the user reference
//#! manual for this class.
// <summary>
// This class is provided to TablePlot to allow MsPlot to do the proper thing
// when button's are pressed on the plotting GUI. 
// </summary>
//
//#! <use visibility=local>   or   <use visibility=export>
// <use visibility=local>
//#!
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
//#! for example:
//#!  <reviewed reviewer="pshannon@nrao.edu" date="1994/10/10" tests="tMyClass, t1MyClass" demos="dMyClass, d1MyClass">
//#!  </reviewed>
// </reviewed>
//
//#! Classes or concepts you should understand before using this class.
// <prerequisite>
// <ul>
//   <li> <linkto class="TPGuiCallBackHooks">TPGuiCallBackHooks</linkto>class,
//   <li> <linkto class="TablePlot">TablePlot</linkto>class, 
//   <li> <linkto class="casacore::Table">MsPlot</linkto> class
//   <li> <linkto class="casacore::Table">Measurement Set</linkto> class 
// </ul>    
// </prerequisite>
//    
// <etymology>
// Callbacks from TablePlot to MsPlot that are executed when buttons are
// pressed on the plotting GUI, and the data that has been plotted resides
// in the Antenna subtable of the Measurement Set. 
// </etymology>
//
// <synopsis>
// Note:  The full list of columns in the Antenna subtable (VLA) are:
// <table border="2">
// <tbody>
//   <tr><th align="left">OFFSET</th>
//       <th align="left">(x,y,z) offset value</th></tr>
//   <tr><th align="left">POSITIION</th>
//    <th align="left">(x,y,z) position value</th></tr>
//   <tr><th align="left">TYPE</th>
//    <th align="left">ground-base or not telescope</th></tr>
//   <tr><th align="left">DISH_DIAMETER</th>
//       <th align="left">&nbsp;</th></tr>    
//   <tr><th align="left">FLAG_ROW</tr>
//       <th align="left">&nbsp;</th></tr>    
//   <tr><th align="left">MOUNT</th>
//       <th align="left">type of mount</th></tr>
//   <tr><th align="left">NAME</th>
//       <th align="left">antenna name</th></tr>
//   <tr><th align="left">STATION</th>
//       <th align="left">antenna station name</th></tr>
// </tbody>
// </table>
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// To provide a way for the TablePlot object to inform the MsPlot object
// that a particular button has been pressed on the plotting GUI and to
// allow casacore::MS Plot to over ride the default TablePlot behaviour if necessary. 
// </motivation>
//
// <thrown>
//    <li> AIPSError
// </thrown>
//
// <todo asof="2007/12/19">
//   <li> Implement the relaseTable method.</li>
//   <li> Finish the documentation</li>
//   <li>use enums in the casacore::MS code for the string values for the
//      locate columns</li>
//   <li> Restructure TablePlot to send rows of data, rather then single
//        data point.  This should improve the plotting speed as it will
//        allow reuse of calculations more easily.</li>
//   <li> Build methods into MsPlot, or some other class that retrieves
//        information from the tables.  Examples of methods are, getFieldNames,
//        getAntNames, ...  This could help reduce the memory usage, reduce
//        code duplication, and improve robustness.</li>
//   <li> Restructure MsPlot to subclass TablePlot and rewrite the
//        MsPlot reset command to do what it needs to do then call
//        the parents reset method.</li>
// </todo>
//#! End of documentation.    
//#!///////////////////////////////////////////////////////////////////////////
class MSPlotAntennaCallBack : public TPGuiCallBackHooks
{
    public: 
           MSPlotAntennaCallBack(casacore::Vector<casacore::String> inAntNames, casacore::Vector<casacore::String> inFieldNames )
      {
         log = SLog::slog();
         casacore::String fnname =  "MSPlotAntennaMSCallBack" ;
#if LOG2 
         log->FnEnter(fnname + "(antNames, fieldNames)", clname );
#endif 
                  
          itsAntNames = inAntNames;
          itsFieldNames = inFieldNames;

          LocateColumns.resize(3);
          //itsPlotOptions.LocateColumns[0] = "NAME";
          //itsPlotOptions.LocateColumns[1] = "STATION";
          LocateColumns[0] = "POSITION[1]";
          LocateColumns[1] = "POSITION[2]";
          LocateColumns[2] = "POSITION[3]";

#if LOG2
          log->FnExit(fnname, clname);
#endif 
           };
      
      ~MSPlotAntennaCallBack(){
#if LOG2 
          casacore::String fnname = "~MSPlotAntennaCallback";
          log->FnEnter(fnname, clname);
          log->FnExit(fnname, clname);
#endif 
      };
      
      casacore::Bool
      releasetable( casacore::Int /*nrows*/,
         casacore::Int /*ncols*/,
         casacore::Int /*panel*/,
         casacore::String /*tablename */)
      {
          casacore::String fnname = "releasetable";
#if LOG2
          log->FnEnter(fnname + "(nrows,ncols,panel,tablename)", clname );
#endif 
             
          //# TODO free any data structures that were created
          //# with this class, maybe call the class destructor too.
#if LOG2
          log->FnExit(fnname, clname);
#endif 
          return true;
      }

      casacore::Bool
      createiterplotlabels( casacore::Vector<casacore::String> iteraxes,
         casacore::Vector<casacore::Double> values,
         casacore::String &titleString )
     {
         casacore::String fnname = "createiterplotlabels";
#if LOG2 
         log->FnEnter(fnname + "(iteraxes, values, titleString)", clname );
#endif 
         
         titleString = "Iter: ";
         //# TODO add a check that the # of values = # of iteraxes.
         
         for ( uint axesId = 0; axesId < iteraxes.nelements(); axesId++ )
         {
        if ( upcase( iteraxes[axesId] ).matches( "BASELINE" )
           || upcase( iteraxes[axesId] ).matches( "ANTENNA" ) 
           || upcase( iteraxes[axesId] ).matches( "ANTENNA1" )
           || upcase( iteraxes[axesId] ).matches( "ANTENNA2" ) )
        {
            if  ( iteraxes.nelements() == 2 
               && upcase( iteraxes[0] ).contains( "ANTENNA" )
               && upcase( iteraxes[1] ).contains( "ANTENNA" ) )
            {
           if ( axesId == 0 )
               titleString += "Baseline ";
            } else
           titleString += "Antenna ";

            if ( values[axesId] < itsAntNames.nelements() )
            {
           if ( axesId > 0 )
               titleString += casacore::String( " : " );
           titleString += itsAntNames[casacore::Int(values[axesId])];
            } else {
                          ostringstream os;
              os       << "Internal error! Antenna ID is "
             << values[axesId]
             << ", but there are only "
             << itsAntNames.nelements()
             << ".";
                        log->out(os, fnname, clname, casacore::LogMessage::DEBUG1);
            }   
        } else if ( upcase(iteraxes[axesId]).matches("FIELD") 
           || upcase(iteraxes[axesId]).matches("FIELD_ID") )
        {
            titleString += "FIELD ";
            if ( values[axesId] < itsFieldNames.nelements() )
            {           
           if ( axesId > 0 )
               titleString += casacore::String( " : " );
           titleString += itsFieldNames[casacore::Int(values[axesId])];
            } else {
                        ostringstream os;
         os    << "Internal error! Antenna ID is "
             << values[axesId]
             << ", but there are only "
             << itsFieldNames.nelements()
             << ".";
                        log->out(os, fnname, clname, casacore::LogMessage::DEBUG1);
            }               
        } else {
            //# This includes SPW, SCAN, FEED, and ARRAY
            //# since all of these don't have names.  This
            //# may change in the future.
            titleString += iteraxes[axesId] + casacore::String( " ");
            
            if ( axesId > 0 )
           titleString += casacore::String( " : " );
            titleString += casacore::String::toString( casacore::Int(values[axesId]) );
        }
         }

#if LOG2 
         log->FnExit(fnname, clname);
#endif 
         return true;
     };
     

      casacore::Bool
      flagdata(casacore::String /*tablename*/) {
#if LOG2 
          casacore::String fnname = "flagdata";
          log->FnEnter(fnname, clname);
          log->FnExit(fnname, clname);
#endif 
          return true;
      }

      casacore::Bool
      flagdata(casacore::Int /*direction*/, casacore::Vector<casacore::String> /*collist*/,
                  casacore::Matrix<casacore::Double> /*infomat*/,casacore::Vector<casacore::String> /*cpol*/, casacore::Bool /*ave*/) {
          casacore::String fnname = "flagdata";
          //cout << "MSPlotAntennaCallBack::flagdata" << endl;

         
          return true;

      }
      
      casacore::Bool
      printlocateinfo( casacore::Vector<casacore::String> /*collist*/,
                  casacore::Matrix<casacore::Double> infomat,casacore::Vector<casacore::String> /*cpol*/)
      {
          casacore::String fnname = "printlocateinfo";
#if LOG2 
          log->FnEnter(fnname + "(collist, infomat, cpol)", clname);
#endif 

          //# Note that TablePlot prepends the ROW (ant ID), and # of
          //# points to the front of the list.

          //# Define the column widths
          casacore::Vector<casacore::uInt> columnWidths(5);
          columnWidths[0]=10; columnWidths[1]=10; columnWidths[2]=10;
          columnWidths[3]=10; columnWidths[4]=10;

          //# Define the column headings
          casacore::String colNames;
          ostringstream tmp;
          tmp << std::setw( columnWidths[0] ) <<  "Ant ID  " << " | " 
         << std::setw( columnWidths[1] ) <<  "Ant. Names" << " | "
         << std::setw( columnWidths[2] ) <<  "X     "  << " | "
         << std::setw( columnWidths[3] ) <<  "Y     "  << " | "
         << std::setw( columnWidths[4] ) <<  "Z     "  << " |"
         << endl;
          colNames += casacore::String( tmp );
          tmp.str( "" );
                    
          //# Set up a string stream for warning msgs and for
          //# display the data.
          casacore::String warnings;
          casacore::String info;
          info += colNames;
          
          casacore::IPosition mshape = infomat.shape();
          casacore::Vector<casacore::Double> position(3);

          if ( mshape[1] < 1 )
         warnings += casacore::String( "No data to display" );
          
          for( casacore::Int i=0; i < mshape[1]; i++)
          {
         if ( mshape[0] != 5 )
         {
             warnings += casacore::String( "Skipping the " )
            +  casacore::String::toString( i )
            + casacore::String( " set of data, its not a complete row.\n" );
         } else {
             if ( infomat( 0, i ) < 0
                || infomat( 0, i ) >= itsAntNames.nelements() )
             {
            warnings += casacore::String( "Unable to find antenna name ")
                + casacore::String( "for antenna ID: " )
                + casacore::String::toString( infomat( 0, i ) );
             }
             tmp << std::noshowpoint << std::setw( columnWidths[0] )
            << infomat( 0, i ) << " | "
            << std::noshowpoint << std::setw( columnWidths[1] )
            << itsAntNames[ casacore::Int(infomat( 0, i ) ) ] << " | "
            << std::showpoint
            << std::setprecision( columnWidths[2]-7)
            << std::setw( columnWidths[2] )
            << infomat( 2, i ) << " | "
            << std::showpoint
            << std::setprecision( columnWidths[3]-7)
            << std::setw( columnWidths[3] )
            << infomat( 3, i ) << " | "
            << std::showpoint
            << std::setprecision( columnWidths[4]-7)
            << std::setw( columnWidths[4] )
            << infomat( 4, i ) << " | " <<endl;;
             info += casacore::String( tmp );
             tmp.str( "" );
         }
          }
          info += colNames;
             
          //# Display the information in the list.
          if ( info.length() > 0 )
           log->out( info, fnname, clname, casacore::LogMessage::NORMAL);

          //# Display any warnings encountered.
          if ( warnings.length() > 0 )
           log->out(warnings, fnname, clname, casacore::LogMessage::NORMAL);

#if LOG2 
          log->FnExit(fnname, clname);
#endif 
          return true;
      };
                 
    private:
      casacore::Vector<casacore::String> itsAntNames;
      casacore::Vector<casacore::String> itsFieldNames;

      // Message logger
      SLog *log;         
      static const casacore::String clname;
};

const casacore::String MSPlotAntennaCallBack::clname = "MSPlotAntennaCallBack";




//#!//////////////////////////////////////////////////////////////////////////
//#! All the wonderful docs, that will show up in the user reference
//#! manual for this class.
// <summary>
// This class is provided to TablePlot to allow MsPlot to do the proper thing
// when button's are pressed on the plotting GUI. 
// </summary>
//
//#! <use visibility=local>   or   <use visibility=export>
// <use visibility=local>
//#!
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
//#! for example:
//#!  <reviewed reviewer="pshannon@nrao.edu" date="1994/10/10" tests="tMyClass, t1MyClass" demos="dMyClass, d1MyClass">
//#!  </reviewed>
// </reviewed>
//
//#! Classes or concepts you should understand before using this class.
// <prerequisite>
// <ul>
//   <li> <linkto class="TPGuiCallBackHooks">TPGuiCallBackHooks</linkto>class,
//   <li> <linkto class="TablePlot">TablePlot</linkto>class, 
//   <li> <linkto class="casacore::Table">MsPlot</linkto> class
//   <li> <linkto class="casacore::Table">Measurement Set</linkto> class 
// </ul>    
// </prerequisite>
//    
// <etymology>
// Callbacks from TablePlot to MsPlot that are executed when buttons are
// pressed on the plotting GUI, and the data that has been plotted resides
// in the UVW column of the main table of the Measurement Set. 
// </etymology>
//
// <synopsis>
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// To provide a way for the TablePlot object to inform the MsPlot object
// that a particular button has been pressed on the plotting GUI and to
// allow casacore::MS Plot to over ride the default TablePlot behaviour if necessary. 
// </motivation>
//
// <thrown>
//    <li> AIPSError
// </thrown>
//
// <todo asof="2007/12/19">
//   <li> Implement the relaseTable method.
//   <li> Finish the documentation
//   <li> Restructure TablePlot to send rows of data, rather then single
//        data point.  This should improve the plotting speed as it will
//        allow reuse of calculations more easily.
//   <li> Build methods into MsPlot, or some other class that retrieves
//        information from the tables.  Examples of methods are, getFieldNames,
//        getAntNames, ...  This could help reduce the memory usage, reduce
//        code duplication, and improve robustness. 
//   <li> Restructure MsPlot to subclass TablePlot and rewrite the
//        MsPlot reset command to do what it needs to do then call
//        the parents reset method.    
// </todo>
//#! End of documentation.    
//#!///////////////////////////////////////////////////////////////////////////

class MSPlotUVWCallBack : public TPGuiCallBackHooks
{
    public: 
         MSPlotUVWCallBack( casacore::MeasurementSet *inMS,
         casacore::Vector<casacore::String> antNames,
         casacore::Vector<casacore::String> fieldNames,
         casacore::Vector<casacore::Vector<casacore::String> >corrNames,
         casacore::Vector<casacore::Double> refFrequencies )
     {
              
         log = SLog::slog();
         casacore::String fnname = "MSPlotUVWCallback";
#if LOG2 
         log->FnEnter(fnname + 
                "( inMS, antNames, fieldNames, corrNames, refFrequencies )",
                clname);
#endif 
         
         localMS = inMS;
         itsAntNames = antNames;
         itsFieldNames = fieldNames;
         itsCorrNames = corrNames;
         itsRefFrequencies = refFrequencies;
         
         LocateColumns.resize(8);
         LocateColumns[0] = "UVW[1]";
         LocateColumns[1] = "UVW[2]";
         LocateColumns[2] = "UVW[3]";
         LocateColumns[3] = "ANTENNA1";
         LocateColumns[4] = "ANTENNA2";
         LocateColumns[5] = "FIELD_ID";
         LocateColumns[6] = "TIME";
         LocateColumns[7] = "DATA_DESC_ID";

#if LOG2 
         log->FnExit(fnname, clname);
#endif 
      };
      
       ~MSPlotUVWCallBack(){
#if LOG2 
          casacore::String fnname = "~MSPlotUVWCallback";
          log->FnEnter(fnname, clname);
          log->FnExit(fnname, clname);
#endif 
      };


      casacore::Bool
      releasetable( casacore::Int /*nrows*/, casacore::Int /*ncols*/, casacore::Int /*panel*/, casacore::String /*tablename */)
      {
          casacore::String fnname = "releasetable";
#if LOG2 
          log->FnEnter(fnname + "(nrows,ncols,panel,tablename )", clname);
#endif 
          //# TODO free any data structures that were created
          //# with this class, maybe call the class destructor too.

#if LOG2 
          log->FnExit(fnname, clname);
#endif 
          return true;
      }

            
      casacore::Bool
      createiterplotlabels( casacore::Vector<casacore::String> iteraxes,
         casacore::Vector<casacore::Double> values,
         casacore::String &titleString )
     {
         casacore::String fnname = "createiterplotlabels";
#if LOG2 
        log->FnEnter(fnname + "(iteraxes, values, titleString)", clname);   
#endif 
         titleString = "Iter: ";
         
         for ( uint axesId = 0; axesId < iteraxes.nelements(); axesId++ )
         {
        if ( upcase( iteraxes[axesId] ).matches( "BASELINE" )
           || upcase( iteraxes[axesId] ).matches( "ANTENNA" ) 
           || upcase( iteraxes[axesId] ).matches( "ANTENNA1" )
           || upcase( iteraxes[axesId] ).matches( "ANTENNA2" ) )
        {
            if  ( iteraxes.nelements() == 2 
               && upcase( iteraxes[0] ).contains( "ANTENNA" )
               && upcase( iteraxes[1] ).contains( "ANTENNA" ) )
            {
           if ( axesId == 0 )
               titleString += "Baseline ";
            } else
           titleString += "Antenna ";
            
            
            if ( values[axesId] < itsAntNames.nelements() )
            {
           if ( axesId > 0 )
               titleString += casacore::String( " : " );
           titleString += itsAntNames[casacore::Int(values[axesId])];
            } else {
#if LOG2 
         ostringstream os;
         os    << "Internal error! Antenna ID is "
             << values[axesId]
             << ", but there are only "
             << itsAntNames.nelements()
             << ".";
         log->out(os, fnname, clname, casacore::LogMessage::DEBUG1);
#endif 
            }
        } else if ( upcase(iteraxes[axesId]).matches("FIELD") 
           || upcase(iteraxes[axesId]).matches("FIELD_ID") )
        {
            titleString += "FIELD ";
            if ( values[axesId] < itsFieldNames.nelements() )
            {
           if ( axesId > 0 )
               titleString += casacore::String( " : " );
           titleString += itsFieldNames[casacore::Int(values[axesId])];
            } else {
#if LOG2 
            ostringstream os;
            os  << "Internal error! Antenna ID is "
             << values[axesId]
             << ", but there are only "
             << itsFieldNames.nelements()
             << ".";
             log->out(os, fnname, clname, casacore::LogMessage::DEBUG1);
#endif 
            }
        } else {
            //# This includes SPW, SCAN, FEED, and ARRAY
            //# since all of these don't have names.  This
            //# may change in the future.
            titleString += iteraxes[axesId] + casacore::String( " ");

            if ( axesId > 0 )
           titleString += casacore::String( " : " );
            titleString += casacore::String::toString( casacore::Int(values[axesId]) );
        }
         }
         
#if LOG2 
        log->FnExit(fnname, clname);
#endif 
         return true;
     };
      
     casacore::Bool
     flagdata(casacore::String /*tablename*/) {
#if LOG2 
         casacore::String fnname = "flagdata";
         log->FnEnter(fnname, clname);
         log->FnExit(fnname, clname);
#endif 
         return true;
     }

      casacore::Bool
      flagdata(casacore::Int /*direction*/, casacore::Vector<casacore::String> /*collist*/,
                  casacore::Matrix<casacore::Double> /*infomat*/,casacore::Vector<casacore::String> /*cpol*/, casacore::Bool /*ave*/) {
          casacore::String fnname = "flagdata";
          //cout << "MSPlotUVWCallBack::flagdata" << endl;
         
          return true;

      }
     
     casacore::Bool
     printlocateinfo( casacore::Vector<casacore::String> /*collist*/,
                  casacore::Matrix<casacore::Double> infomat,casacore::Vector<casacore::String> cpol)
     {
         casacore::String fnname = "printlocateinfo";
#if LOG2 
         log->FnEnter(fnname + "(collist, infomat, cpol)", clname);
#endif 

         //# Define the column headings and other reptitive strings used.
         casacore::Vector<casacore::uInt> columnWidths(12);
         columnWidths[0]=8; columnWidths[1]=8; columnWidths[2]=11;
         columnWidths[3]=11; columnWidths[4]=11; columnWidths[5]=8;
         columnWidths[6]=25; columnWidths[7]=22; columnWidths[8]=6;
         columnWidths[9]=10; columnWidths[10]=14; columnWidths[11]=30; 
         
         casacore::String colNames;
         ostringstream tmp;
         tmp << std::setw( columnWidths[0] ) <<  "ROW #" << " | "
        << std::setw( columnWidths[1] ) <<  "# Points"  << " | "
        << std::setw( columnWidths[2] ) <<  "U   " << " | "
        << std::setw( columnWidths[3] ) <<  "V   " << " | "
        << std::setw( columnWidths[4] ) <<  "W   " << " | "
        
        << std::setw( columnWidths[5] ) <<  "Baseline" << " | "
        << std::setw( columnWidths[6] ) <<  "Field ID/Name      "  << " | "
        << std::setw( columnWidths[7] ) <<  "casacore::Time         "  << " | "
        << std::setw( columnWidths[8] ) <<  "SPW ID" << " | "
        << std::setw( columnWidths[9] ) <<  "Ref. Freq." << " | "
        << std::setw( columnWidths[10] ) <<  "Corr. Id/Name"  << " |"
        << std::setw( columnWidths[11] ) <<  "[pol /chan]" << " | "
        << endl;
         colNames += casacore::String( tmp );
         tmp.str( "" );

         //# get the DATA_DESC_ID column so we can extract polarization
         //# and spectral window information.
         casacore::ROMSDataDescColumns dataDescCol( localMS->dataDescription() );
         
         casacore::IPosition mshape = infomat.shape();
         casacore::String warnings;
         casacore::String info;
         info += colNames;
         
         for(casacore::Int i=0; i < mshape[1]; i++)
         {
        if ( mshape[0] != 10 )
        {
            warnings += casacore::String( "Skipping the " )
           + casacore::String::toString( i )
           + casacore::String( " set of data, its not a complete row. " );
            break;
        }

         
        //# Put the Row number and the # of points in the row
        tmp << std::noshowpoint << std::setw( columnWidths[0] )
            << casacore::String::toString( infomat( 0, i ) ) << " | "
            << std::noshowpoint << std::setw( columnWidths[1] )
            << infomat( 1, i ) << " | ";
         
        //# Print the U, V and W values
        tmp << std::showpoint
            << std::setprecision( columnWidths[2]-7)
            << std::setw( columnWidths[2] )
            << infomat( 2, i ) << " | ";
        tmp << std::showpoint
            << std::setprecision( columnWidths[3]-7)
            << std::setw( columnWidths[3] )
            << infomat( 3, i ) << " | ";
        tmp << std::showpoint
            << std::setprecision( columnWidths[4]-7)
            << std::setw( columnWidths[4] )
            << infomat( 4, i ) << " | ";
         
        //# Printout the Baseline
         casacore::String baseline;
         if ( infomat(5,i) < 0
            || infomat(5,i) > itsAntNames.nelements() )
             baseline += casacore::String::toString( infomat(5,i) )
            + casacore::String( "(ID)" );
         else 
             baseline += itsAntNames( (casacore::uInt)infomat(5,i) );
         baseline += casacore::String( "/" );
         if ( infomat(6,i) < 0
            || infomat(6,i) > itsAntNames.nelements() )
             baseline += casacore::String::toString( infomat(6,i) )
            + casacore::String( "(ID)" );
         else 
             baseline += itsAntNames( (casacore::uInt)infomat(6,i) );
         baseline += " | ";
         tmp << std::setw( columnWidths[5] )
             << baseline;
         
        //# Print out the Field ID, and Field Name
        casacore::String fieldStr = casacore::String::toString(infomat(7,i) )
            + casacore::String( "/" );
        if ( infomat(7,i) < itsFieldNames.nelements() ) 
            fieldStr += itsFieldNames[ casacore::Int( infomat(7,i) ) ];
        else {
            warnings += casacore::String( "Field ID, " )
           + casacore::String::toString( casacore::Int( infomat(7,i) ) )
           + casacore::String( ", is larger then the" )
           + casacore::String( " number of field names, " )
           + casacore::String::toString( itsFieldNames.nelements() )
           + casacore::String( ". \n" );
            fieldStr += "??";
        }
        tmp << std::noshowpoint << std::setw( columnWidths[6] )
            << fieldStr << " | ";
        
        //Print out the time in YY/MM/DD/hh:mm:ss.s format
        tmp << std::setw( columnWidths[7] )
            << casacore::MVTime( infomat(8, i)/casacore::C::day).string( casacore::MVTime::DMY,7)
            << " | ";
        
        //# spectral window IDs and Ref Frequency
        //# TODO add a check that infomat(9,i) isn't too big.
        casacore::Int spwId = dataDescCol.spectralWindowId()( (casacore::Int)infomat(9, i ) );
        tmp << std::noshowpoint << std::setw( columnWidths[8] )
            << spwId << " | ";
             
        if ( spwId < (casacore::Int)itsRefFrequencies.nelements() ) 
            tmp << std::showpoint
           << std::setprecision( columnWidths[9]-7)
           << std::setw( columnWidths[9] )
           << itsRefFrequencies[ spwId ] << " | ";
        else {
            warnings += casacore::String( "SPW ID, " )
           + casacore::String::toString( spwId )
           + casacore::String( ", is larger then the" )
           + casacore::String( " number of reference frequencies, " )
           + casacore::String::toString( itsRefFrequencies.nelements() )
           + casacore::String( ". \n" );
            tmp << " ?? " << " | ";
        }
             
        //# Polarization ID and list of polls and chans selected
        //#  in this row.
        //# TODO add a check that infomat(9,i) isn't too big.
        //#      add a check on SPWId    
        casacore::Int polId = dataDescCol.polarizationId()( (casacore::Int)infomat(9, i ) );
        casacore::String polStr = casacore::String::toString( polId ) + casacore::String( "/" );
        if ( itsCorrNames[polId].nelements() > 0 )
            for ( casacore::Int corrId=0;
             corrId < (casacore::Int)itsCorrNames[polId].nelements();
             corrId++ ) {
           if ( corrId > 0 )
               polStr += casacore::String( ", " );
           polStr += itsCorrNames[polId][corrId];
            }
        else {
            warnings += casacore::String( "Pol ID, " )
           + casacore::String::toString( polId )
           + casacore::String( ", is larger then the" )
           + casacore::String( " number of polarizations, " )
           + casacore::String::toString( itsCorrNames[spwId].nelements() )
           + casacore::String( ", \n" );
            polStr += "??";
        }
        tmp << std::noshowpoint << std::setw( columnWidths[10] )
            << polStr << " | ";
        
        tmp  << std::setw( columnWidths[11] )
             << cpol(i)  << " | " << endl;
         
        info += casacore::String( tmp );
        tmp.str( "" );         
          }   
          info += colNames;
             
          //# Display the information in the list.
          if ( info.length() > colNames.length() )
           log->out(info, fnname, clname, casacore::LogMessage::NORMAL);

          //# Display any warnings encountered.
          if ( warnings.length() > 0 )
           log->out( warnings, fnname, clname, casacore::LogMessage::WARN);

#if LOG2 
          log->FnExit(fnname, clname);
#endif 
          return true;
     }
                 
    private:
      casacore::MeasurementSet *localMS;
      casacore::Vector<casacore::String> itsAntNames;
      casacore::Vector<casacore::String> itsFieldNames;
      casacore::Vector< casacore::Vector<casacore::String> > itsCorrNames;
      casacore::Vector<casacore::Double> itsRefFrequencies;

      // Message logger
      SLog* log;         
      static const casacore::String clname;
};

const casacore::String MSPlotUVWCallBack::clname = "MSPlotUVWCallBack";

};


#endif //CASA_MSPLOT__HOOKS_H
