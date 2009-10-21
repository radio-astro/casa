//# Framework independent implementation file for ms..
//# Copyright (C) 2006-2007-2008
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
//# @author 
//# @version 
//////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <sys/wait.h>
#include <casa/BasicSL/String.h>
#include <casa/Exceptions/Error.h>
#include <xmlcasa/ms/ms_cmpt.h>
#include <xmlcasa/ms/Statistics.h>
#include <msfits/MSFits/MSFitsInput.h>
#include <msfits/MSFits/MSFitsOutput.h>
#include <ms/MeasurementSets/MSRange.h>
#include <ms/MeasurementSets/MSSummary.h>
#include <ms/MeasurementSets/MSLister.h>
#include <ms/MeasurementSets/MSConcat.h>
#include <ms/MeasurementSets/MSFlagger.h>
#include <ms/MeasurementSets/MSSelectionTools.h>
#include <msvis/MSVis/MSContinuumSubtractor.h>
#include <msvis/MSVis/SubMS.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/OS/DOos.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableLock.h>
#include <tables/Tables/TableParse.h>
#include <casa/System/ObjectID.h>
#include <casa/Utilities/Assert.h>
#include <msvis/MSVis/VisSet.h>
#include <msvis/MSVis/VisSetUtil.h>

#include <lattices/Lattices/LatticeStatistics.h>
#include <lattices/Lattices/SubLattice.h>

#include <tables/Tables/SetupNewTab.h>
#include <ms/MeasurementSets/MSHistoryHandler.h>

#include <casa/namespace.h>
#include <cassert>

using namespace std;

namespace casac {

ms::ms()
{
  try {
     itsMS = new MeasurementSet();
     itsSel = new MSSelector();
     itsLog = new LogIO();
     itsFlag = new MSFlagger();
   } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks();
       RETHROW(x);
   }
}

ms::~ms()
{
  try {
     if(itsMS)
        delete itsMS;
     if(itsSel)
        delete itsSel;
     if(itsFlag)
        delete itsFlag;
     if(itsLog)
        delete itsLog;
   } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks();
       RETHROW(x);
   }
}

/////// Internal helper functions ///////

// Takes a variant a returns a casa String, converting -1 to "" along the way.
inline static String m1toBlankCStr_(const ::casac::variant& v) 
{
  String cs(toCasaString(v));
  return cs == String("-1") ? "" : cs;
}

// Returns whether or not the MS pointed to by itsMS can be written to,
// avoiding a weird exception from itsMS->isWritable() if no ms is attached.
inline Bool ms::ready2write_()
{
  return (!detached() && itsMS->isWritable());
}
////// End of helper functions //////

int
ms::nrow(const bool selected)
{
  Int rstat(0);
  try {
     if(!detached()){
        if(!selected)
           rstat = itsMS->nrow();
        else
           rstat = itsSel->nrow();
     }
  } catch (AipsError x) {
      *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks();
       RETHROW(x);
  }
  Table::relinquishAutoLocks();
  return rstat;
}

bool
ms::iswritable()
{
  Bool rstat(False);
  try {
    rstat = ready2write_();
  }
  catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks();
       RETHROW(x);
  }
  Table::relinquishAutoLocks();
  return rstat;
}

bool
ms::open(const std::string& thems, const bool nomodify, const bool lock)
{
  try {
      *itsLog << LogOrigin("ms", "open");
     const Table::TableOption openOption = nomodify ? Table::Old : Table::Update;
     TableLock tl;
     if (lock) tl = TableLock(TableLock::PermanentLocking);
     //
     //  A little book-keeping here, to make sure locks are not held and to noleak some ms's
     //
     if(!itsMS->isNull()){
        close();
     }
     *itsMS = MeasurementSet(thems, tl, openOption);
     //
     // itsSel and itsFlag were not being reset by using the set commands so
     // delete them and renew them.
     //
     if(itsSel){
	     delete itsSel;
	     itsSel = new MSSelector();
     }
     if(itsFlag){
	     delete itsFlag;
	     itsFlag = new MSFlagger();
     }
     itsSel->setMS(*itsMS);
     itsFlag->setMSSelector(*itsSel);
  } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks();
       RETHROW(x);
  }
  return True;
}

bool
ms::fromfits(const std::string& msfile, const std::string &fitsfile, const bool nomodify, const bool lock, 
             const int obstype, const std::string &host, bool forcenewserver, const std::string& antnamescheme)
{
try {
   // bool rstat(False);
	 // Well this here be a work around until we get proper use of the cfitsio
	 // routines by rewriting the underlying fits writer stuff.
   /*
   Table::relinquishAutoLocks(True);
   if(!fork()){
      try {
      */
	*itsLog << LogIO::NORMAL3 << "Opening fits file " << fitsfile << LogIO::POST;
       String namescheme(antnamescheme);
       namescheme.downcase();
       MSFitsInput msfitsin(String(msfile), String(fitsfile), (namescheme=="new"));
       msfitsin.readFitsFile(obstype);
      *itsLog << LogIO::NORMAL3 << "Flushing MS " << msfile << " to disk" << LogIO::POST;
       /*
      } catch (AipsError x) {
         *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
      } catch (...) {
         *itsLog << LogIO::SEVERE << "Unknown Exception Reported " << LogIO::POST;
      }
      exit(rstat);
   }
   int dummy;
   wait(&dummy);
   if(dummy)
      rstat=true;
      */
    open(msfile, nomodify, lock);
  } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks();
       RETHROW(x);
  }
  Table::relinquishAutoLocks();
  return True;
}

bool
ms::close()
{
  Bool rstat(False);
  try {
    if(!detached()){
      *itsLog << LogOrigin("ms", "close");
      *itsLog << LogIO::NORMAL3;
      if (itsMS->isWritable()) {
        *itsLog << "Flushing data to disk and detaching from file.";
      }
      else {
        *itsLog << "Readonly measurement set: just detaching from file.";
      }
      *itsLog << LogIO::POST;
      delete itsMS;
      itsMS = new MeasurementSet();
      itsSel->setMS(*itsMS);
      itsFlag->setMSSelector(*itsSel);
      rstat = True;
    }
  } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks();
       RETHROW(x);
  }
  Table::relinquishAutoLocks();
  return rstat;
}

std::string
ms::name()
{
   std::string rstat("none");
   try {
      if(!detached()){
         rstat = itsMS->tableName();
      }
   } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks();
       RETHROW(x);
   }
   Table::relinquishAutoLocks();
   return rstat;
}

/*
::casac::record*
ms::command(const std::string& msfile, const std::string& command, const bool nomodify)
{

  *itsLog << LogOrigin("ms", "command");
  *itsLog << "not implemented"<<LogIO::POST;
  Table::relinquishAutoLocks();
  return 0;
}
*/

bool
ms::tofits(const std::string& fitsfile, const std::string& column, const ::casac::variant& field, const ::casac::variant& spw, const int nchan, const int start, const int width, const ::casac::variant& baseline, const std::string& time, const ::casac::variant& scan, const ::casac::variant& uvrange, const std::string& taql,const bool writesyscal, const bool multisource, const bool combinespw, const bool writestation)
{
  Bool rstat(True);
  try{
     if(!detached()){

         MeasurementSet *mssel= new MeasurementSet();
         Bool subselect=False;
	 String fieldS(m1toBlankCStr_(field));
	 String spwS(m1toBlankCStr_(spw));
	 String baselineS=toCasaString(baseline);
	 String timeS=toCasaString(time);
	 String scanS=toCasaString(scan);
	 String uvrangeS=toCasaString(uvrange);
	 String taqlS=toCasaString(taql);
	 Int inchan=-1;
	 Int istart=0;
	 Int iwidth=1;
	 Record selrec=itsMS->msseltoindex(spwS, fieldS);
	 Vector<Int>fldids=selrec.asArrayInt("field");
	 Vector<Int>spwids=selrec.asArrayInt("spw");
	 
	 //use nchan if defined else use caret-column syntax of  msselection 
	 if(nchan>0){
	   inchan=nchan;
	   iwidth=width;
	   istart=start;
	 }
	 else{
	   Matrix<Int> chansel=selrec.asArrayInt("channel");
	   if(chansel.nelements() !=0){
	     iwidth=chansel.row(0)(3);
	     if(iwidth < 1)
	       iwidth=1;
	     istart=chansel.row(0)(1);
	     inchan=(chansel.row(0)(2)-istart+1)/iwidth;
	     if(inchan<1)
	       inchan=1;	  
	     
	   } 
	 }

	 subselect = mssSetData(*itsMS, *mssel, "", timeS, baselineS, fieldS,
				  spwS, uvrangeS, taqlS, "", scanS);

         if(subselect && mssel->nrow()< itsMS->nrow()){
	   if(mssel->nrow()==0){
	     if(!mssel)
	       delete mssel; 
	     mssel=0;
	     *itsLog << LogIO::WARN
		     << "No data for selection: will convert full MeasurementSet"
		     << LogIO::POST;
	     mssel=new MeasurementSet(*itsMS);
	   } 
	   else{
           *itsLog << "By selection " << itsMS->nrow()
                  <<  " rows to be converted are reduced to "
                  << mssel->nrow() << LogIO::POST;
	   }
   
         }
         else{
	   if(!mssel)
	     delete mssel; 
           mssel = new MeasurementSet(*itsMS);
         }
 

      //==================================================   
	 // Well this here be a work around until we get proper use of the cfitsio
	 // routines by rewriting the underlying fits writer stuff.
	 /*
   Table::relinquishAutoLocks(True);
   if(!fork()){
      rstat = true;
      try {
      */
         if (!MSFitsOutput::writeFitsFile(fitsfile, *mssel, column, istart,
                                          inchan, iwidth, writesyscal,
                                          multisource, combinespw, writestation)) {
           *itsLog << LogIO::SEVERE << "Conversion to FITS failed"<< LogIO::POST;
           rstat = False;
         }

	 //Done...clear off the mssel
	 if(mssel)
	   delete mssel;
	 /*
      } catch (AipsError x) {
         *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
         rstat = False;
      } catch (...) {
         *itsLog << LogIO::SEVERE << "Unknown Exception Reported " << LogIO::POST;
         rstat = False;
      }
      exit(rstat);
   }
   int dummy;
   wait(&dummy);
   if(dummy)
      rstat=true;
     } else {
       rstat = False;
     */
     }
   } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks();
       RETHROW(x);
   }
   Table::relinquishAutoLocks();
   return rstat;
}

bool
ms::summary(::casac::record& header, const bool verbose)
{
  Bool rstat(False);
  try {
     if(!detached()){
       *itsLog << LogOrigin("ms", "summary");
       MSSummary mss(itsMS);
       mss.list(*itsLog, verbose);
       rstat = True;
     }
   } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks();
       RETHROW(x);
   }
   Table::relinquishAutoLocks();
   return rstat;
}

bool
ms::listhistory()
{
  Bool rstat(False);
  try {
     if(!detached()){
       *itsLog << LogOrigin("ms", "listhistory");
       MSSummary mss(*itsMS);
       mss.listHistory(*itsLog);
       rstat = True;
     }
   } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks();
       RETHROW(x);
   }
   Table::relinquishAutoLocks();
   return rstat;
}

bool
ms::writehistory(const std::string& message, const std::string& parms, const std::string& origin, const std::string& msname, const std::string& app)
{
   Bool rstat(False);
   try {
     if (message.length() > 0 || parms.length() > 0) {
       MeasurementSet outMS;
       if (msname.length() > 0) {
         outMS = MeasurementSet(msname,TableLock::AutoLocking,Table::Update);
       } else {
         outMS = MeasurementSet(ms::name(),
                                TableLock::AutoLocking,Table::Update);
       }
    // make sure the MS has a HISTORY table
       if(!(Table::isReadable(outMS.historyTableName()))){
         TableRecord &kws = outMS.rwKeywordSet();
         SetupNewTable historySetup(outMS.historyTableName(),
                                    MSHistory::requiredTableDesc(),Table::New);
         kws.defineTable(MS::keywordName(MS::HISTORY), Table(historySetup));
       }
       MSHistoryHandler::addMessage(outMS, message, app, parms, origin);
       rstat = True;
     }
   } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks();
       RETHROW(x);
   }
   Table::relinquishAutoLocks();
   return rstat;
}

::casac::record*
ms::range(const std::vector<std::string>& items, const bool useflags, const int blocksize)
{
    ::casac::record *retval(0);
    try {
       if(!detached()){
          MSRange msrange(*itsSel);
          msrange.setBlockSize(blocksize);
          retval = fromRecord(msrange.range(toVectorString(items), useflags, False));
       }
   } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks();
       RETHROW(x);
   }
   Table::relinquishAutoLocks();
   return retval;
}


::casac::record* 
ms::statistics(const std::string& column, 
               const std::string& complex_value,
               const bool useflags, 
               const std::string& spw, 
               const std::string& field, 
               const std::string& baseline, 
               const std::string& uvrange, 
               const std::string& time, 
               const std::string& correlation,
               const std::string& scan, 
               const std::string& array)
{
    *itsLog << LogOrigin("ms", "statistics");

    ::casac::record *retval(0);
    try {
       if(!detached()){

         /* This tools built-in itsSel is of type
            MSSelector.
            That is something completely different than 
            MSSelection.
         */
         const String dummyExpr = String("");

         if (0) cerr << "selection: " << endl <<
           "time = " << time << endl << 
           "baseline = " << baseline << endl <<
           "field = " << field << endl <<
           "spw = " << spw << endl <<
           "uvrange = " << uvrange << endl <<
           "correlation = " << correlation << endl <<
           "scan = " << scan << endl <<
           "array = " << array << endl;

         MSSelection mssel(*itsMS,
                           MSSelection::PARSE_NOW, 
                           time,
                           baseline, 
                           field,
                           spw,
                           uvrange,
                           dummyExpr,   // taqlExpr
                           correlation,
                           scan,
                           array);

         MeasurementSet *sel_p;
         MeasurementSet sel;
         if (mssel.getSelectedMS(sel)) {
           /* It is undocumented, but
              getSelectedMS() seems to return True
              if there's a non-trivial selection.
              If it returns false, the output MS is null.
           */

           sel_p = &sel;
           if (0) cout << "Got the subset MS!" << endl;
         }
         else {
           sel_p = itsMS;
         }


         retval = fromRecord(Statistics::get_statistics(*sel_p,
                                                        column,
                                                        complex_value,
                                                        itsLog));

       } // end if !detached
   } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks();
       RETHROW(x);
   }
   Table::relinquishAutoLocks();
   return retval;
}

bool
ms::lister(const std::string& options,
           const std::string& datacolumn,
           const std::string& field, 
           const std::string& spw, 
           const std::string& antenna, 
           const std::string& timerange, 
           const std::string& correlation, 
           const std::string& scan,
           const std::string& feed,
           const std::string& array,
           const std::string& uvrange,
           const std::string& average,
           const bool         showflags,
           const std::string& msselect,
           const int          pagerows,
           const std::string& listfile)
{
   Bool rstat(False);
   try {
    if(detached()) return False;
    *itsLog << LogOrigin("ms", "lister");
    
    MSLister msl(*itsMS, *itsLog);
    msl.list(options, datacolumn, field, spw, antenna, timerange,
             correlation, scan, feed, array, uvrange, average, showflags,
             msselect, pagerows, listfile);
    rstat = True;
   } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks();
       RETHROW(x);
   }
   Table::relinquishAutoLocks();
   return rstat;
}

bool
ms::selectinit(const int datadescid, const bool reset)
{
  Bool retval = False;
  try {
     Vector<Int> ddId(1, datadescid);
     if(!detached()){
       Int n=ddId.nelements();
       if (n>0 && min(ddId)<0 && !reset) {
         *itsLog << "The data description id must be a list of "
           "positive integers" << LogIO::EXCEPTION;
       }
       if (n>0) {
         Vector<Int> tmp(ddId.nelements());
         tmp=ddId;
         retval = itsSel->initSelection(tmp, reset);
       } else {
         retval = itsSel->initSelection(ddId, reset);
       }
     }
   } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks();
       RETHROW(x);
   }
   Table::relinquishAutoLocks();
  return retval;
}

bool
ms::select(const ::casac::record& items)
{
  Bool retval = False;
  try {
     if(!detached()){
        Record *myTmp = toRecord(items);
        retval = itsSel->select(*myTmp, False);
        delete myTmp;
     }
   } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks();
       RETHROW(x);
   }
   Table::relinquishAutoLocks();
  return retval;
}

bool
ms::selecttaql(const std::string& msselect)
{
   Bool retval(False);
   try {
     if(!detached())
       retval = itsSel->select(msselect);
   } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks();
       RETHROW(x);
   }
   Table::relinquishAutoLocks();
    return retval;
}

bool
ms::selectchannel(const int nchan, const int start, const int width, const int inc)
{
   Bool retval(False);
   try {
     if(!detached())
       retval = itsSel->selectChannel(nchan, start, width, inc);
   } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks();
       RETHROW(x);
   }
   Table::relinquishAutoLocks();
    return retval;
}

bool
ms::selectpolarization(const std::vector<std::string>& wantedpol)
{
   Bool retval(False);
   try {
    if(!detached())
       retval = itsSel->selectPolarization(toVectorString(wantedpol));
   } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks();
       RETHROW(x);
   }
   Table::relinquishAutoLocks();
   return retval;
}

bool
ms::regridspw(const std::string& outframe, 
	      const std::string& regrid_quantity, 
	      const double regrid_velo_restfrq, 
	      const std::string& regrid_interp_meth,
	      const double regrid_start, 
	      const double regrid_center, 
	      const double regrid_bandwidth, 
	      const double regrid_chan_width 
	      )
{
  Bool rstat(False);
  try {
     *itsLog << LogOrigin("ms", "regridspw");
     if(!ready2write_()){
       *itsLog << LogIO::SEVERE
            << "Please open ms with parameter nomodify=false. Write access to ms is needed."
            << LogIO::POST;

       return False;
     }

     double center = regrid_center;
     if(center > -1E30 && regrid_start > -1E30){
       Bool agree(False);
       if( regrid_quantity == "chan" ){
	 agree = (center == floor(regrid_bandwidth/2. + regrid_start));
       }
       else{
	 agree = (center == regrid_bandwidth/2. + regrid_start);
       }	 
       if(!agree){ // start and center don't agree
	 *itsLog << LogIO::SEVERE
		 << "Please give only the start (lower edge) or the center of the new spectral window, not both."
		 << LogIO::POST;
       return False;       
       }
     }
     else if(regrid_start > -1E30){ // only start given, need to calculate center
       if( regrid_quantity == "chan" ){
	 center = floor(regrid_bandwidth/2. + regrid_start);
       }
       else{
	 center = regrid_bandwidth/2. + regrid_start;
       }
     } 
     
     SubMS *subms = new SubMS(*itsMS);
     *itsLog << LogIO::NORMAL << "Starting spectral frame transformation / regridding ..." << LogIO::POST;
     String t_outframe=toCasaString(outframe);
     String t_regridQuantity=toCasaString(regrid_quantity);
     String t_regridInterpMeth=toCasaString(regrid_interp_meth);
     Int rval;
     String regridMessage;


     if((rval = subms->regridSpw(regridMessage,
				 t_outframe,
				 t_regridQuantity,
				 Double(regrid_velo_restfrq),
				 t_regridInterpMeth,
				 Double(center), 
				 Double(regrid_bandwidth),
				 Double(regrid_chan_width)
				 )
	 )==1){ // successful modification of the MS took place
       *itsLog << LogIO::NORMAL << "Spectral frame transformation/regridding completed." << LogIO::POST;
   
       // Update HISTORY table of modfied MS
       String message= "Transformed/regridded with regridspw";
       writehistory(message, regridMessage, "ms::regridspw()", "", "ms"); // empty name writes to itsMS
       rstat = True;
     }
     else if(rval==0) { // an unsuccessful modification of the MS took place
       String message= "Frame transformation to " + t_outframe + " failed. MS probably invalid.";
       *itsLog << LogIO::WARN << message << LogIO::POST;
       // Update HISTORY table of the unsuccessfully modfied MS
       ostringstream param;
       param << "Original input parameters: outframe=" << t_outframe << " mode= " <<  t_regridQuantity
	     << " center= " << center << " bandwidth=" << regrid_bandwidth
	     << " chanwidth= " << regrid_chan_width << " restfreq= " << regrid_velo_restfrq 
	     << " interpolation= " << t_regridInterpMeth;
       String paramstr=param.str();
       writehistory(message,paramstr,"ms::regridspw()", "", "ms"); // empty name writes to itsMS
     }
     else {
       *itsLog << LogIO::NORMAL << "MS not modified." << LogIO::POST;
       rstat = True;
     }       

     delete subms;

  } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks();
       RETHROW(x);
  }
  Table::relinquishAutoLocks();
  return rstat;
}


bool
ms::cvel(const std::string& mode, 
	 const int nchan, 
	 const ::casac::variant& start, const ::casac::variant& width,
	 const std::string& interp, 
	 const ::casac::variant& phasec, 
	 const ::casac::variant& restfreq, 
	 const std::string& outframe,
	 const std::string& veltype)
{
  Bool rstat(False);
  try {

    *itsLog << LogOrigin("ms", "cvel");
    
    *itsLog << LogIO::NORMAL << "Selecting data ..." << LogIO::POST;
    
    String t_interp = toCasaString(interp);
    String t_phasec = toCasaString(phasec);
    String t_mode = toCasaString(mode);
    Double t_restfreq = 0.; // rest frequency, convert to Hz
    if(!restfreq.toString().empty()){
      t_restfreq = casaQuantity(restfreq).getValue("Hz");
    }

    // Determine grid
    Double t_start = -9e99; // default value indicating that the original start of the SPW should be used
    Double t_center= -3E30; // default value indicating that the original center of the SPW should be used
    Double t_bandwidth = -1.; // default value indicating that the original width of the SPW should be used
    Double t_width = -1.; // default value indicating that the original channel width of the SPW should be used

    if(!start.toString().empty()){ // start was set
      if(t_mode == "channel"){
	t_start = Double(atoi(start.toString().c_str()));
      }
      else if(t_mode == "frequency"){
	t_start = casaQuantity(start).getValue("Hz");
      }
      else if(t_mode == "velocity"){
	t_start = casaQuantity(start).getValue("m/s");
      }
      else{
	*itsLog << LogIO::WARN << "Invalid mode " << t_mode << LogIO::POST;
	return false;
      }
    
      // determine channel width
      if(t_mode == "channel"){
	if(!width.toString().empty()){
	  t_width = Double(atoi(width.toString().c_str()));
	}
	else{
	  t_width = 1;
	}
      }
      else if(t_mode == "frequency"){
	if(!width.toString().empty()){
	  t_width = casaQuantity(width).getValue("Hz");
	}
	else{
	  *itsLog << LogIO::WARN << "In frequency mode, need to set channel width if start is set." << LogIO::POST;
	  return false;
	}
      }
      else if(t_mode == "velocity"){
	if(!width.toString().empty()){
	  t_width = casaQuantity(width).getValue("m/s");
	}   
	else{
	  *itsLog << LogIO::WARN << "In velocity mode, need to set channel width if start is set." << LogIO::POST;
	  return false;
	}
      }
      // determine bandwidth and center
      if(nchan > 0){ // we are not using the default, which is all channels
	if(t_mode == "channel"){
	  t_bandwidth = Double(nchan*t_width);
	  t_center = floor(t_bandwidth/2. + t_start);
	}
	else{
	  t_bandwidth = nchan*t_width;
	  t_center = t_bandwidth/2. + t_start;
	}
      }
    }
    else { // start was not set
      if(t_mode == "channel"){
	t_start = 0;
	if(!width.toString().empty()){
	  t_width = Double(atoi(width.toString().c_str()));
	}
	else{
	  t_width = 1;
	}
	if(nchan > 0){
	  t_bandwidth = Double(nchan*t_width);
	}
      }
      else if(t_mode == "frequency"){
	if(!width.toString().empty()){
	  if(nchan<=0){
	    *itsLog << LogIO::WARN << "In frequency mode, need to set nchan if width is set." << LogIO::POST;
	    return false;
	  }
	  else{
	    t_width = casaQuantity(width).getValue("Hz");
	    t_bandwidth = nchan*t_width;
	  }	
	}
      }
      else if(t_mode == "velocity"){
	if(!width.toString().empty()){
	  if(nchan<=0){
	    *itsLog << LogIO::WARN << "In velocity mode, need to set nchan if width is set." << LogIO::POST;
	    return false;
	  }
	  else{
	    t_width = casaQuantity(width).getValue("m/s");
	    t_bandwidth = nchan*t_width;
	  }	
	}
      }
      else{
	*itsLog << LogIO::WARN << "Invalid mode " << t_mode << LogIO::POST;
	return false;
      }
    } // end if start was set

    String t_veltype = toCasaString(veltype); 
    String t_regridQuantity;
    if(t_mode == "channel"){
      t_regridQuantity = "chan";
    }
    else if(t_mode == "frequency"){
      t_regridQuantity = "freq";
    }
    else if(t_mode == "velocity"){
      if(t_restfreq == 0.){
	*itsLog << LogIO::SEVERE << "Need to set restfreq in velocity mode." << LogIO::POST; 
	return False;
      }	
      t_regridQuantity = "vrad";
      if(t_veltype == "optical"){
	t_regridQuantity = "vopt";
      }
      else{
	*itsLog << LogIO::SEVERE << "Invalid velocity type "<< veltype << LogIO::POST; 
	return False;
      }
    }   
    
    String t_outframe=toCasaString(outframe);
    String t_regridInterpMeth=toCasaString(interp);
    
    casa::MDirection  t_phaseCenter;
    Int t_phasec_fieldid=-1;
    //If phasecenter is a simple numeric value then it's taken as a fieldid 
    //otherwise its converted to a MDirection
    if(phasec.type()==::casac::variant::DOUBLEVEC 
       || phasec.type()==::casac::variant::DOUBLE
       || phasec.type()==::casac::variant::INTVEC
       || phasec.type()==::casac::variant::INT){
      t_phasec_fieldid = phasec.toInt();	
      if(t_phasec_fieldid >= itsMS->field().nrow() || t_phasec_fieldid < 0){
	*itsLog << LogIO::SEVERE << "Field id " << t_phasec_fieldid
		<< " selected to be used as phasecenter does not exist." << LogIO::POST;
	return False;
      }
    }
    else{
      if(t_phasec.empty()){
	t_phasec_fieldid = 0;
      }
      else{
	if(!casaMDirection(phasec, t_phaseCenter)){
	  *itsLog << LogIO::SEVERE << "Could not interprete phasecenter parameter "
		  << t_phasec << LogIO::POST;
	  return False;
	}
	*itsLog << LogIO::NORMAL << "Using user-provided phase center." << LogIO::POST;
      }
    }

    // end prepare regridding parameters

    // need exclusive rights to this MS, will re-open it after combineSpws
    String originalName = itsMS->tableName();
    itsMS->flush();
    close();

    *itsLog << LogOrigin("ms", "cvel");

    SubMS *sms = new SubMS(originalName);

    // combine Spws
    if(!sms->combineSpws()){
      *itsLog << LogIO::SEVERE << "Error combining spectral windows." << LogIO::POST;
      delete sms;
      open(originalName,  Table::Update, False); 
      return False;
    }

    // Regrid
    Int rval;
    String regridMessage;
    
    if((rval = sms->regridSpw(regridMessage,
			      t_outframe,
			      t_regridQuantity,
			      t_restfreq,
			      t_regridInterpMeth,
			      t_center, 
			      t_bandwidth,
			      t_width,
			      t_phasec_fieldid, // == -1 if t_phaseCenter is valid
			      t_phaseCenter
			      )
	)==1){ // successful modification of the MS took place
      *itsLog << LogIO::NORMAL << "Spectral frame transformation/regridding completed." << LogIO::POST;
      
      // Update HISTORY table of modfied MS
      String message = "Transformed/regridded with cvel";
      writehistory(message, regridMessage, "ms::cvel()", originalName, "ms"); 
      rstat = True;
    }
    else if(rval==0) { // an unsuccessful modification of the MS took place
      String message= "Frame transformation to " + t_outframe + " failed. MS probably invalid.";
      *itsLog << LogIO::WARN << message << LogIO::POST;
      // Update HISTORY table of the unsuccessfully modfied MS
      ostringstream param;
      param << "Original input parameters: outframe=" << t_outframe << " mode= " <<  t_regridQuantity
	    << " center= " << t_center << " bandwidth=" << t_bandwidth
	    << " chanwidth= " << t_width << " restfreq= " << t_restfreq 
	    << " interpolation= " << t_regridInterpMeth;
      String paramstr=param.str();
      writehistory(message,paramstr,"ms::cvel()", originalName, "ms"); 
      rstat = False;
    }
    else { // there was no need to regrid
      *itsLog << LogIO::NORMAL << "SubMS not modified by regridding." << LogIO::POST;
      rstat = True;
    }       
    
    delete sms;
    open(originalName,  Table::Update, False);

  } catch (AipsError x) {
      *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
      Table::relinquishAutoLocks();
      RETHROW(x);
  }
  Table::relinquishAutoLocks();
  return rstat;
}


::casac::record*
ms::getdata(const std::vector<std::string>& items, const bool ifraxis, const int ifraxisgap, const int increment, const bool average)
{

  ::casac::record *retval(0);
  try {
	  /*
	  Matrix<Int> m(10u,10u);
	  m=0;
          for(int i=1;i<11;i++){
              for(int j=1;j<11;j++){
                  m(i-1,j-1) = i*10+j;
	      }
	  }
	  std::cerr << m << std::endl;
	  Int *storage;
	  Bool deleteIt(False);
	  storage = m.getStorage(deleteIt);
	  for(int k=0;k<10;k++){
	      for(int l=0;l<10;l++){
		      std::cerr << *(storage + k*10+l) << " ";
	      }
	      std::cerr << std::endl;
	  }
	  */
     if(!detached())
         retval = fromRecord(itsSel->getData(toVectorString(items), ifraxis, ifraxisgap, increment, average, False)); 
   } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks();
       RETHROW(x);
   }
   Table::relinquishAutoLocks();
   return retval;
}

bool
ms::putdata(const ::casac::record& items)
{
  Bool rstat(False);
  try {
     if(!detached()){
        Record *myTmp = toRecord(items);
        rstat = itsSel->putData(*myTmp);
        delete myTmp;
     }
  } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks();
       RETHROW(x);
  }
  Table::relinquishAutoLocks();
  return rstat;
}

bool
ms::concatenate(const std::string& msfile, const ::casac::variant& freqtol, const ::casac::variant& dirtol)
{
    Bool rstat(False);
    try {
	if(!detached()){
	    *itsLog << LogOrigin("ms", "concatenate");
	    
	    if (!Table::isReadable(msfile)) {
		*itsLog << "Cannot read the measurement set called " << msfile
			<< LogIO::EXCEPTION;
	    }                   
	    if (DOos::totalSize(msfile, True) >
		DOos::freeSpace(Vector<String>(1, itsMS->tableName()), True)(0)) {
		*itsLog << "There does not appear to be enough free disk space "
			<< "(on the filesystem containing " << itsMS->tableName()
			<< ") for the concatantion to succeed." << LogIO::EXCEPTION;
	    }
	    const MeasurementSet appendedMS(msfile);
	    
	    MSConcat mscat(*itsMS);
	    Quantum<Double> dirtolerance;
	    Quantum<Double> freqtolerance;
	    if(freqtol.toString().empty()){
		freqtolerance=casa::Quantity(1.0,"Hz");
	    }
	    else{
		freqtolerance=casaQuantity(freqtol);
	    }
	    if(dirtol.toString().empty()){
		dirtolerance=casa::Quantity(1.0, "mas");
	    }
	    else{
		dirtolerance=casaQuantity(dirtol);
	    }
	    
	    *itsLog << LogIO::DEBUGGING << "MSConcat created" << LogIO::POST;
	    mscat.setTolerance(freqtolerance, dirtolerance);
	    mscat.concatenate(appendedMS);

	    String message = String(msfile) + " appended to " + itsMS->tableName();
	    ostringstream param;
	    param << "msfile= " << msfile
		  << " freqTol='" << casaQuantity(freqtol) << "' dirTol='"
		  << casaQuantity(dirtol) << "'";
	    String paramstr=param.str();
	    writehistory(std::string(message.data()), std::string(paramstr.data()),
			 std::string("ms::concatenate()"), msfile, "ms");
	}
	rstat = True;
    } catch (AipsError x) {
	*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
		<< LogIO::POST;
	Table::relinquishAutoLocks();
	RETHROW(x);
    }
    Table::relinquishAutoLocks();
    return rstat;
}

bool
ms::timesort(const std::string& msname)
{
    Bool rstat(False);
    try {
	if(!detached()){
	    *itsLog << LogOrigin("ms", "timesort");

	    if (DOos::totalSize(itsMS->tableName(), True) >
		DOos::freeSpace(Vector<String>(1, itsMS->tableName()), True)(0)) {
		*itsLog << "There does not appear to be enough free disk space "
			<< "(on the filesystem containing " << itsMS->tableName()
			<< ") for the sorting to succeed." << LogIO::EXCEPTION;
	    }
		
	    {
		String originalName = itsMS->tableName();
		itsMS->flush();
		MeasurementSet sortedMS = itsMS->sort("TIME");

		if (msname.length() == 0) { // no name given, sort and don't keep copy
		    // make deep copy
		    sortedMS.deepCopy(originalName+".sorted", Table::New);
		    // close reference table
		    sortedMS = MeasurementSet();
		    // close original MS 
		    close();
		    // rename copy to original name
		    Table newMSmain(originalName+".sorted", Table::Update);
		    newMSmain.rename(originalName, Table::New);    // will also delete original table
		    newMSmain = Table();
		    // reopen 
		    open(originalName,  Table::Update, False); 
		    *itsLog << LogOrigin("ms", "timesort");
		    String message = "Sorted by TIME in ascending order.";
		    writehistory(std::string(message.data()), "", std::string("ms::timesort()"), originalName, "ms");
		    *itsLog << LogIO::NORMAL << "Sorted main table of " << originalName << " by TIME." 
			    << LogIO::POST;
		}
		else { // sort into a new MS
		    sortedMS.deepCopy(msname, Table::New);		   
		    String message = "Generated from " + originalName + " by sorting by TIME in ascending order.";
		    writehistory(std::string(message.data()), "", std::string("ms::timesort()"), msname, "ms");

		    *itsLog << LogIO::NORMAL << "Sorted main table of " << originalName << " by TIME and stored it in " 
			    << msname << " ."<< LogIO::POST;
		
		}
	    }

	    rstat = True;
	}
	
    } catch (AipsError x) {
	*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	Table::relinquishAutoLocks();
	RETHROW(x);
    }
    Table::relinquishAutoLocks();
    return rstat;
}

bool
ms::split(const std::string&      outputms,  const ::casac::variant& field, 
	  const ::casac::variant& spw,       const std::vector<int>& step,
          const ::casac::variant& antenna,   const ::casac::variant& timebin,
          const std::string&      timerange, const ::casac::variant& scan,
          const ::casac::variant& uvrange,   const std::string&      taql,
          const std::string&      whichcol,  const ::casac::variant& tileShape,
          const ::casac::variant& subarray,  const bool averchan)
{
  Bool rstat(False);
  try {
     *itsLog << LogOrigin("ms", "split");
     /*if(!ready2write_()){
       *itsLog << LogIO::SEVERE
            << "Please open ms with parameter nomodify=false.  Write access to ms is needed by split to store some temporary selection information. "
            << LogIO::POST;

       return False;
     }
     */
     SubMS *splitter = new SubMS(*itsMS);
     *itsLog << LogIO::NORMAL2 << "Sub MS created" << LogIO::POST;
     String t_field(m1toBlankCStr_(field));
     String t_spw(m1toBlankCStr_(spw));
     String t_antenna = toCasaString(antenna);
     String t_scan    = toCasaString(scan);
     String t_uvrange = toCasaString(uvrange);
     String t_taql(taql);
     const String t_subarray = toCasaString(subarray);
     
     if(!splitter->setmsselect(t_spw, t_field, t_antenna, t_scan, t_uvrange, 
                               t_taql, Vector<Int>(step), averchan,
                               t_subarray)){
       *itsLog << LogIO::SEVERE
               << "Error selecting data."
               << LogIO::POST;
       delete splitter;
       return false;
     }
       
     Double timeInSec=casaQuantity(timebin).get("s").getValue();
     splitter->selectTime(timeInSec, String(timerange));
     String t_outputms(outputms);
     String t_whichcol(whichcol);
     Vector<Int> t_tileshape(1,0);
     if(toCasaString(tileShape) != String("")){
       t_tileshape.resize();
       t_tileshape=tileShape.toIntVec();
     }
     if(!splitter->makeSubMS(t_outputms, t_whichcol, t_tileshape)){
       *itsLog << LogIO::SEVERE
               << "Error splitting " << itsMS->tableName() << " to "
               << t_outputms
               << LogIO::POST;
       delete splitter;
       return false;
     }
       
     *itsLog << LogIO::NORMAL2 << "SubMS made" << LogIO::POST;
     delete splitter;
   
     {// Update HISTORY table of newly created MS
       String message= toCasaString(outputms) + " split from " + itsMS->tableName();
       ostringstream param;
       param << "fieldids=" << t_field << " spwids=" << t_spw
             << " step=" << Vector<Int>(step) << " which='" << whichcol << "'";
       String paramstr=param.str();
       writehistory(message, paramstr, "ms::split()", outputms, "ms");
     }

     rstat = True;
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    Table::relinquishAutoLocks();
    RETHROW(x);
  }
  Table::relinquishAutoLocks();
  return rstat;
}

bool
ms::iterinit(const std::vector<std::string>& columns, const double interval,
             const int maxrows, const bool adddefaultsortcolumns)
{
   Bool rstat(False);
   try {
      if(!detached())
         rstat = itsSel->iterInit(toVectorString(columns), interval, maxrows,
                                  adddefaultsortcolumns);
   } catch (AipsError x) {
     *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
             << LogIO::POST;
     Table::relinquishAutoLocks();
     RETHROW(x);
   }
   Table::relinquishAutoLocks();
   return rstat; 
}

bool
ms::iterorigin()
{
   Bool rstat(False);
   try {
      if(!detached())
          rstat =  itsSel->iterOrigin();
   } catch (AipsError x) {
          *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks();
       RETHROW(x);
   }
   Table::relinquishAutoLocks();
   return rstat; 
}

bool
ms::iternext()
{
   Bool rstat(False);
   try {
      if(!detached())
          rstat =  itsSel->iterNext();
   } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks();
       RETHROW(x);
   }
   Table::relinquishAutoLocks();
   return rstat; 
}

bool
ms::iterend()
{
   Bool rstat(False);
   try {
      if(!detached())
         rstat =  itsSel->iterEnd();
   } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks();
       RETHROW(x);
   }
   Table::relinquishAutoLocks();
   return rstat; 
}

/*
bool
ms::tosdfits(const std::string& fitsfile)
{

  *itsLog << LogOrigin("ms", "tosdfits");
  *itsLog << "not implemented"<<LogIO::POST;
  Table::relinquishAutoLocks();
  return False;
}

bool
ms::createflaghistory(const int numlevel)
{
   Bool rstat(False);
   try {
     if(!detached())
       rstat =  itsFlag->createFlagHistory(numlevel);
   } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks();
       RETHROW(x);
   }
   Table::relinquishAutoLocks();
   return rstat; 
}

bool
ms::saveflags(const bool newlevel)
{
   Bool rstat(False);
   try {
     if(!detached())
       rstat =  itsFlag->saveFlags(newlevel);
   } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks();
       RETHROW(x);
   }
   Table::relinquishAutoLocks();
   return rstat; 
}

bool
ms::restoreflags(const int level)
{
   Bool rstat(False);
   try {
     if(!detached())
        rstat =  itsFlag->restoreFlags(level);
   } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks();
       RETHROW(x);
   }
   Table::relinquishAutoLocks();
   return rstat; 
}

int
ms::flaglevel()
{
   Bool rstat(False);
    try {
   if(!detached())
      rstat =  itsFlag->flagLevel();
   } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks();
       RETHROW(x);
   }
   Table::relinquishAutoLocks();
   return rstat; 
}
*/

bool
ms::fillbuffer(const std::string& item, const bool ifraxis)
{
   Bool rstat(False);
   try {
     if(!detached())
        rstat =  itsFlag->fillDataBuffer(item, ifraxis);
   } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
	       << LogIO::POST;
       Table::relinquishAutoLocks();
       RETHROW(x);
   }
   Table::relinquishAutoLocks();
   return rstat; 
}

::casac::record*
ms::diffbuffer(const std::string& direction, const int window)
{
   Bool domedian(False); //Not in the help file but in glish/C++
   ::casac::record* retval(0);
   try {
     if(!detached()){
	casa::Record daRec = itsFlag->diffDataBuffer(casa::String(direction),
						     window, domedian);
        retval = fromRecord(daRec);
	}
   } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
	       << LogIO::POST;
       Table::relinquishAutoLocks();
       RETHROW(x);
   }
   Table::relinquishAutoLocks();
   return retval;
}

::casac::record*
ms::getbuffer()
{
 ::casac::record* retval(0);
 try {
   if(!detached())
      retval = fromRecord(itsFlag->getDataBuffer());
 } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
	       << LogIO::POST;
       Table::relinquishAutoLocks();
       RETHROW(x);
 }
 return retval;
}

bool
ms::clipbuffer(const double pixellevel, const double timelevel, const double channellevel)
{
   Bool rstat(False);
   try {
     if(!detached())
        rstat =  itsFlag->clipDataBuffer(pixellevel, timelevel, channellevel);
   } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks();
       RETHROW(x);
   }
   Table::relinquishAutoLocks();
   return rstat; 
}

bool
ms::setbufferflags(const ::casac::record& flags)
{
  Bool retval = False;
  try {
    if(!detached()){
       Record *myTmp = toRecord(flags);
       retval = itsFlag->setDataBufferFlags(*myTmp);
       delete myTmp;
    }
  } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks();
       RETHROW(x);
  }
  Table::relinquishAutoLocks();
  return retval;

}

bool
ms::writebufferflags()
{
   Bool rstat(False);
   try {
     if(!detached())
        rstat =  itsFlag->writeDataBufferFlags();
   } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks();
       RETHROW(x);
   }
   Table::relinquishAutoLocks();
   return rstat; 
}

bool
ms::clearbuffer()
{
   Bool rstat(False);
   try {
     if(!detached())
        rstat =  itsFlag->clearDataBuffer();
   } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
	       << LogIO::POST;
       Table::relinquishAutoLocks();
       RETHROW(x);
   }
   Table::relinquishAutoLocks();
   return rstat; 
}

bool ms::continuumsub(const ::casac::variant& field,
		      const ::casac::variant& fitspw,
		      const ::casac::variant& spw,
		      const ::casac::variant& solint,
		      const int fitorder,
		      const std::string& mode)
{
  Bool rstat(False);
  try {
    *itsLog << LogOrigin("ms", "continuumsub");
    *itsLog << LogIO::NORMAL2 << "continuumsub starting" << LogIO::POST;
      
    MSContinuumSubtractor sub(*itsMS);
    sub.setField(toCasaString(field));
    sub.setFitSpw(toCasaString(fitspw));
    sub.setSubSpw(toCasaString(spw));
    sub.setSolutionInterval(toCasaString(solint));
    sub.setOrder(fitorder);
    sub.setMode(mode);
    sub.subtract();
    *itsLog << LogIO::NORMAL2 << "continuumsub finished" << LogIO::POST;  
    rstat = True;
 } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
	       << LogIO::POST;
       Table::relinquishAutoLocks();
       RETHROW(x);
 }
 Table::relinquishAutoLocks();
 return rstat;
}

/*
bool
ms::ptsrc(const std::vector<int>& fldid, const std::vector<int>& spwid)
{

  *itsLog << LogOrigin("ms", "ptsrc");
  *itsLog << "not implemented"<<LogIO::POST;
  Table::relinquishAutoLocks();
  return False;
}
*/

bool
ms::done()
{
  *itsLog << LogOrigin("ms", "done");
  Table::relinquishAutoLocks();
  return close();
}

bool
ms::detached()
{
  Bool rstat(False);
  try {
     if (itsMS->isNull()) {
       *itsLog << LogOrigin("ms", "detached");
       *itsLog << LogIO::SEVERE
              << "ms is not attached to a file - cannot perform operation.\n"
              << "Call ms.open('filename') to reattach." << LogIO::POST;
       rstat = True;
     }
  } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks();
       RETHROW(x);
  }
  Table::relinquishAutoLocks();
  return rstat;
}

::casac::record* 
ms::msseltoindex(const std::string& vis, const ::casac::variant& spw, 
		 const ::casac::variant& field, 
		 const ::casac::variant& baseline, 
		 const ::casac::variant& time, 
		 const ::casac::variant& scan, const ::casac::variant& uvrange,
		 const std::string& taql){
  casac::record* rstat(0);
  try {

    
    MeasurementSet thisms(vis);
    Record selected=thisms.msseltoindex(toCasaString(spw), toCasaString(field),
					toCasaString(baseline), 
					toCasaString(time),toCasaString(scan),
					toCasaString(uvrange),
					String(taql));
    rstat = fromRecord(selected); 
					

  }catch (AipsError x){
       Table::relinquishAutoLocks();
    RETHROW(x);
  }
  
  Table::relinquishAutoLocks();
  return rstat;

}

bool
ms::hanningsmooth()
{
  Bool rstat(False);
  try {
     *itsLog << LogOrigin("ms", "hanningsmooth");
     if(!ready2write_()){
       *itsLog << LogIO::SEVERE
	       << "Please open ms with parameter nomodify=false so "
	       << "smoothed channel data can be stored (in the CORRECTED_DATA column)."
	       << LogIO::POST;
       return rstat;
     }

     Block<int> noSort;
     Matrix<Int> allChannels;
     Double intrvl = 0;

     VisSet vs(*itsMS,noSort,allChannels,intrvl);
     VisSetUtil::HanningSmooth(vs);

     rstat = True;
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks();
    RETHROW(x);
  }
  Table::relinquishAutoLocks();
  return rstat;
}

bool
ms::uvsub(Bool reverse)
{
  Bool rstat(False);
  try {
     *itsLog << LogOrigin("ms", "uvsub");
     if(!ready2write_()){
       *itsLog << LogIO::SEVERE
	       << "Please open ms with parameter nomodify=false. "
	       << "Write access to ms is needed by uvsub."
	       << LogIO::POST;
       return rstat;
     }

     Block<int> noSort;
     Matrix<Int> allChannels;
     Double intrvl = 0;

     VisSet vs(*itsMS, noSort, allChannels, intrvl);
     VisSetUtil::UVSub(vs, reverse);

     rstat = True;
  }
  catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
	    << LogIO::POST;
    Table::relinquishAutoLocks();
    RETHROW(x);
  }
  Table::relinquishAutoLocks();
  return rstat;
}

} // casac namespace

