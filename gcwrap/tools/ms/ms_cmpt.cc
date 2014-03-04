// -*- C++ -*-
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
#include <casa/iostream.h>
#include <sstream>
#include <sys/wait.h>
#include <casa/BasicSL/String.h>
#include <casa/Exceptions/Error.h>
#include <ms_cmpt.h>
#include <msmetadata_cmpt.h>
#include <tools/ms/Statistics.h>
#include <msfits/MSFits/MSFitsInput.h>
#include <msfits/MSFits/MSFitsOutput.h>
#include <msfits/MSFits/MSFitsIDI.h>
#include <fits/FITS/FITSReader.h>
#include <ms/MeasurementSets/MSRange.h>
#include <ms/MeasurementSets/MSSummary.h>
#include <ms/MeasurementSets/MSLister.h>
#include <ms/MeasurementSets/MSConcat.h>
#include <ms/MeasurementSets/MSFlagger.h>
#include <ms/MeasurementSets/MSSelectionTools.h>
#include <ms/MeasurementSets/MSMainColumns.h>

#include <measures/Measures/MeasTable.h>

#include <synthesis/MSVis/MSAnalysis.h>
#include <synthesis/MSVis/MSContinuumSubtractor.h>
#include <synthesis/MSVis/Partition.h>
#include <synthesis/MSVis/Reweighter.h>
#include <synthesis/MSVis/SubMS.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/OS/DOos.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableLock.h>
#include <tables/Tables/TableParse.h>
#include <tables/Tables/ConcatTable.h>
#include <tables/Tables/TableCopy.h>
#include <casa/System/ObjectID.h>
#include <casa/Utilities/Assert.h>
#include <synthesis/MSVis/VisSet.h>
#include <synthesis/MSVis/VisSetUtil.h>
#include <synthesis/MSVis/VisBuffer.h>
#include <synthesis/MSVis/VisIterator.h>

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
     itsOriginalMS = new MeasurementSet();
     itsSel = new MSSelector();
     itsLog = new LogIO();
     itsFlag = new MSFlagger();
     itsMSS = new MSSelection();
     itsVI = NULL;//new VisibilityIterator();
     itsVB = NULL;//new VisBuffer();
     doingIterations_p=False;
   } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks(True);
       RETHROW(x);
   }
}

ms::~ms()
{
  try {
    if(itsMS)           {delete itsMS;itsMS=NULL;}
    if(itsOriginalMS)   {delete itsOriginalMS;itsOriginalMS=NULL;}
    if(itsSel)          {delete itsSel; itsSel=NULL;}
    if(itsFlag)         {delete itsFlag; itsFlag=NULL;}
    if(itsLog)          {delete itsLog; itsLog=NULL;}
    if(itsMSS)          {delete itsMSS; itsMSS=NULL;}
    if (itsVI)          {delete itsVI; itsVI=NULL;}
    if (itsVB)          {delete itsVB; itsVB=NULL;}
    doingIterations_p=False;
   } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks(True);
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
bool
ms::listfits(const std::string &fitsfile)
{
  FITSReader fr;
  fr.listFits(fitsfile.c_str());
  return 1;
}


bool
ms::createmultims(const std::string &outputTableName,
                  const std::vector<std::string> &tableNames,
                  const std::vector<std::string> &subtableNames,
                  const bool nomodify,
                  const bool lock,
		  const bool copysubtables,
		  const std::vector<std::string> &omitSubtableNames)
{
  *itsLog << LogOrigin("ms", "createmultims");

  try {
    Block<String> tableNameVector(tableNames.size());
    Block<String> subtableVector(subtableNames.size());
    Block<String> omitSubtables(omitSubtableNames.size());

    /* Copy the input vectors into Block */
    for (uInt idx=0; idx<tableNameVector.nelements(); idx++)
       tableNameVector[idx] = tableNames[idx];

    for (uInt idx=0; idx<subtableVector.nelements(); idx++)
      subtableVector[idx] = subtableNames[idx];

    for (uInt idx=0; idx<omitSubtables.nelements(); idx++)
      omitSubtables[idx] = omitSubtableNames[idx];

    if((tableNameVector.nelements()>1) && copysubtables){
      *itsLog << LogIO::NORMAL << "Copying subtables from " << tableNameVector[0] 
	      << " to the other MMS members." << LogIO::POST;
      Table firstTab(tableNameVector[0]);
      for(uInt idx=1; idx<tableNameVector.nelements(); idx++){
	Table otherTab(tableNameVector[idx], Table::Update);
	TableCopy::copySubTables (otherTab, firstTab, 
				  False, // noRows==False, i.e. subtables are copied
				  omitSubtables);
      }
    }

    TableLock tlock(TableLock::AutoNoReadLocking);

    {
      ConcatTable concatTable(tableNameVector,
                              subtableVector,
			      "SUBMSS", // move all member tables into subdirectory SUBMSS
                              Table::New,
                              tlock,
                              TSMOption::Default);
      concatTable.tableInfo().setSubType("CONCATENATED");
      concatTable.rename(outputTableName, Table::New);
    }


  } catch (AipsError ex) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << ex.getMesg()
            << LogIO::POST;
    return false;
  }
  /* Now open our new MS so it is referred to by the tool */
  return open(outputTableName, nomodify, lock);
}

bool ms::ismultims()
{
  bool rstat(False);
  try {
    if(!detached()){
      rstat = (itsMS->tableInfo().subType() == "CONCATENATED");
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    Table::relinquishAutoLocks(True);
    RETHROW(x);
  }
  Table::relinquishAutoLocks(True);
  return rstat;
}


std::vector<std::string> ms::getreferencedtables()
{

  std::vector<std::string> rvalue(0);
  try {
    if (!detached()) {
      Block<String> refTables = itsMS->getPartNames();
      rvalue.resize(refTables.nelements());

      /* Copy the return block to an output vector */
      for (uInt idx=0; idx<rvalue.size(); idx++)
        rvalue[idx] = refTables[idx];

    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    Table::relinquishAutoLocks(True);
    RETHROW(x);
  }
  Table::relinquishAutoLocks(True);
  return rvalue;
}

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
       Table::relinquishAutoLocks(True);
       RETHROW(x);
  }
  Table::relinquishAutoLocks(True);
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
       Table::relinquishAutoLocks(True);
       RETHROW(x);
  }
  Table::relinquishAutoLocks(True);
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
     *itsOriginalMS = MeasurementSet(*itsMS);
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
     if (itsMSS) 
       {
	 delete itsMSS;
	 itsMSS = new MSSelection();
	 itsMSS->resetMS(*itsMS);
       }
     itsSel->setMS(*itsMS);
     itsFlag->setMSSelector(*itsSel);
     doingIterations_p=False;
  } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks(True);
       RETHROW(x);
  }
  return True;
}

bool
ms::reset()
{
  try {
    *itsLog << LogOrigin("ms", "reset");
    // Set itsMS to the original MS, and re-make the various objects
    // that hold the pointer to working MS
    *itsMS = MeasurementSet(*itsOriginalMS);
    if(itsSel)  {delete itsSel;}  itsSel = new MSSelector();
    if(itsFlag) {delete itsFlag;} itsFlag = new MSFlagger();
    if (itsMSS) {delete itsMSS;}  itsMSS = new MSSelection();
                                    itsMSS->resetMS(*itsMS);
    itsSel->setMS(*itsMS);
    itsFlag->setMSSelector(*itsSel);
    doingIterations_p=False;
  }
  catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " 
            << x.getMesg() << LogIO::POST;
    Table::relinquishAutoLocks(True);
    RETHROW(x);
  }
  return True;
}

bool
ms::fromfits(const std::string& msfile, const std::string &fitsfile, 
             const bool nomodify, const bool lock, 
             const int obstype, const std::string &,//host,
	     bool, //forcenewserver,
	     const std::string& antnamescheme)
{
  try {
    *itsLog << LogOrigin("ms", "fromfits")
            << LogIO::NORMAL3 << "Opening fits file " << fitsfile << LogIO::POST;
    String namescheme(antnamescheme);
    namescheme.downcase();
    MSFitsInput msfitsin(String(msfile), String(fitsfile), (namescheme=="new"));
    msfitsin.readFitsFile(obstype);
    *itsLog << LogOrigin("ms", "fromfits")
            << LogIO::NORMAL3 << "Flushing MS " << msfile 
            << " to disk" << LogIO::POST;


    open(msfile, nomodify, lock);
  } catch (AipsError x) {
       //*itsLog << LogOrigin("ms", "fromfits") 
       //        << LogIO::SEVERE << "Exception Reported: " 
       //        << x.getMesg() << LogIO::POST;
       RETHROW(x);
  }
  Table::relinquishAutoLocks(True);
  return True;
}

bool
ms::fromfitsidi(const std::string& msfile, const std::string &fitsidifile, const bool nomodify, const bool lock,
		const int obstype)
{
  try {
    *itsLog << LogIO::NORMAL3 << "Opening FITS-IDI file " << fitsidifile << LogIO::POST;

    {
      MSFitsIDI msfitsidi(String(fitsidifile), String(msfile), !nomodify, obstype); // i.e. overwrite == !nomodify
      msfitsidi.fillMS();
    } // let the msfitsidi go out of scope in order to close the new MS
    
    *itsLog << LogIO::NORMAL3 << "Flushing MS " << msfile << " to disk" << LogIO::POST;
    
    open(msfile, nomodify, lock);
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    Table::relinquishAutoLocks(True);
    RETHROW(x);
  }
  Table::relinquishAutoLocks(True);
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
      delete itsMS;          itsMS = new MeasurementSet();
      delete itsOriginalMS;  itsOriginalMS = new MeasurementSet();
      itsSel->setMS(*itsMS);
      itsFlag->setMSSelector(*itsSel);
      if (itsMSS) {delete itsMSS;  itsMSS = new MSSelection();};
      if (itsVI) {delete itsVI; itsVI=NULL;// itsVI = new VisibilityIterator();
      };
      if (itsVB) {delete itsVB; // itsVB = new VisBuffer();
      };
      doingIterations_p=False;
      rstat = True;
    }
  } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks(True);
       RETHROW(x);
  }
  Table::relinquishAutoLocks(True);
  return rstat;
}

std::string
ms::name()
{
   std::string rstat("none");
   try {
      if(!detached()){
         rstat = itsOriginalMS->tableName();
      }
   } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks(True);
       RETHROW(x);
   }
   Table::relinquishAutoLocks(True);
   return rstat;
}

/*
::casac::record*
ms::command(const std::string& msfile, const std::string& command, const bool nomodify)
{

  *itsLog << LogOrigin("ms", "command");
  *itsLog << "not implemented"<<LogIO::POST;
  Table::relinquishAutoLocks(True);
  return 0;
}
*/

bool
ms::tofits(const std::string& fitsfile, const std::string& column,
           const ::casac::variant& field, const ::casac::variant& spw,
           const int width,
           const ::casac::variant& baseline, const std::string& time,
           const ::casac::variant& scan, const ::casac::variant& uvrange,
           const std::string& taql, const bool writesyscal,
           const bool multisource, const bool combinespw,
           const bool writestation, const bool padwithflags)
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
	 Int inchan=1;
	 Int istart=0;
         Int istep=1;
         Int iwidth=width;
         if (spwS==String(""))
             spwS="*";
	 Record selrec;
         try {
             selrec=itsMS->msseltoindex(spwS, fieldS);
         }
         catch (AipsError x) {
             Table::relinquishAutoLocks(True);
             *itsLog << LogOrigin("ms", "tofits") 
                     << LogIO::SEVERE << x.getMesg() << LogIO::POST;
             RETHROW(x);
         }
	 Vector<Int>fldids=selrec.asArrayInt("field");
	 Vector<Int>spwids=selrec.asArrayInt("spw");
	 
	 Matrix<Int> chansel=selrec.asArrayInt("channel");
         //cout << "chansel=" << chansel << endl;
         //cout << "chansel.nelements()=" << chansel.nelements() << endl;
	 if(chansel.nelements() !=0){
	    istep=chansel.row(0)(3);
	    if(istep < 1)
	      istep=1;
	    istart=chansel.row(0)(1);
	    inchan=(chansel.row(0)(2)-istart+1)/istep;
	    if(inchan<1) {
	      inchan=1;	  
              istep=1;
            }
	 } 
         if (iwidth < 1)
            iwidth = 1;
         if (iwidth > inchan)
            iwidth = inchan;

	 subselect = mssSetData(*itsMS, *mssel, "", timeS, baselineS, fieldS,
				  spwS, uvrangeS, taqlS, "", scanS);

         if(subselect && mssel->nrow() < itsMS->nrow()){
	   if(mssel->nrow()==0){
	     if(!mssel)
	       delete mssel; 
	     mssel=0;
	     *itsLog << LogIO::WARN << LogOrigin("ms", "tofits")
		     << "No data for selection: will convert full MeasurementSet"
		     << LogIO::POST;
	     mssel=new MeasurementSet(*itsMS);
	   } 
	   else{
           *itsLog << LogOrigin("ms", "summary")
                   << "By selection " << itsMS->nrow()
                  <<  " rows to be converted are reduced to "
                  << mssel->nrow() << LogIO::POST;
	   }
   
         }
         else{
	   if(!mssel)
	     delete mssel; 
           mssel = new MeasurementSet(*itsMS);
         }
         MeasurementSet selms(*mssel);
         //cout << "inchan=" << inchan << " istart=" << istart 
         //       << " istep=" << istep << endl; 
         if (!MSFitsOutput::writeFitsFile(fitsfile, selms, column, istart,
                                       inchan, istep, writesyscal,
                                       multisource, combinespw,
                                       writestation, 1.0, padwithflags, iwidth)) {
           *itsLog << LogOrigin("ms", "tofits") 
                   << LogIO::SEVERE << "Conversion to FITS failed"<< LogIO::POST;
           rstat = False;
         }

	 //Done...clear off the mssel
	 if(mssel)
	   delete mssel;
     }
   } catch (AipsError x) {
       *itsLog << LogOrigin("ms", "tofits") 
               << LogIO::SEVERE << "Exception Reported: " 
               << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks(True);
       RETHROW(x);
   }
   Table::relinquishAutoLocks(True);
   return rstat;
}

msmetadata* ms::metadata(const float cachesize) {
	try {
		if (detached()) {
			return 0;
		}
		return new msmetadata(itsMS, cachesize);
	}
	catch(const AipsError& x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}
}

::casac::record*
ms::summary(bool verbose, const string& listfile, bool listunfl, double cachesize)
{
  ::casac::record *header = 0;
  try {
     if(!detached()){
       *itsLog << LogOrigin("ms", "summary");
       // pass the original MS name to the constructor
       // so that it is correctly printed in the output
       MSSummary mss(itsMS, itsOriginalMS->tableName());
       mss.setListUnflaggedRowCount(listunfl);
       mss.setMetaDataCacheSizeInMB(cachesize);
       casa::Record outRec;
       if (listfile != ""){
    	   File diskfile(listfile);
    	   if (diskfile.exists()){
    		   String errmsg = "File: "+ listfile +
    		   " already exists; delete it or choose another name.";
    		   throw(AipsError(errmsg));
    	   }
    	   else {
    		   cout << "Writing output to file: " << listfile << endl;
    	   }

    	   /* First, save output to a string so that LogMessages
    	      and time stamps can be removed from it */
    	   ostringstream ostr;
    	   streambuf *osbuf, *backup;

    	   // Backup original cout buffer
    	   backup = cout.rdbuf();

    	   // Redirect cout's buffer to string
    	   osbuf = ostr.rdbuf();
    	   cout.rdbuf(osbuf);

    	   // Sink the messages locally to a string
    	   LogSink sink(LogMessage::NORMAL, &ostr, False);
    	   LogIO os(sink);

    	   // Call the listing routines
    	   mss.list(os, outRec, verbose, True);
    	   header=fromRecord(outRec);

    	   // Restore cout's buffer
    	   cout.rdbuf(backup);

    	   // Remove the extra fields (time, severity) from the output string
    	   String str(ostr.str());
    	   str.gsub (Regex(".*\tINFO\t[+]?\t"), "");

    	   // Write output string to a file
    	   ofstream file;
    	   file.open(listfile.data());
    	   file << str;

    	   file.close();
       }
       else {
    	   mss.list(*itsLog, outRec, verbose, True);
    	   header=fromRecord(outRec);
       }
     }
   } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks(True);
       RETHROW(x);
   }
   Table::relinquishAutoLocks(True);
   return header;
}

::casac::record*
ms::getscansummary()
{
  ::casac::record *scansummary = 0;
  try {
    if(!detached()){
      MSSummary mss(itsMS);
      casa::Record outRec;
      mss.getScanSummary(outRec);
      scansummary=fromRecord(outRec);
    }
  }
  catch (const AipsError& x) {
    *itsLog << LogOrigin("ms", "getscansummary");
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    Table::relinquishAutoLocks(True);
    RETHROW(x);
  }
  Table::relinquishAutoLocks(True);
  return scansummary;
}

::casac::record*
ms::getspectralwindowinfo()
{
  ::casac::record *spwSummary;
  try {
    if(!detached()){
      //*itsLog << LogOrigin("ms", "summary");
      MSSummary mss(itsMS);
      casa::Record outRec;
      mss.getSpectralWindowInfo(outRec);
      spwSummary=fromRecord(outRec);
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    Table::relinquishAutoLocks(True);
    RETHROW(x);
  }
  Table::relinquishAutoLocks(True);
  return spwSummary;
}

variant*
ms::getfielddirmeas(
	const std::string& dircolname, int fieldid,
	double time, const string& format
)
{
 variant *retval = 0;
  try{
    if(!detached()){
       *itsLog << LogOrigin("ms", "getfielddirmeas");
      String error;       
      String colname(dircolname);
      colname.upcase();
      
      casa::MSFieldColumns msfc(itsMS->field());
      casa::MDirection d;
      if(colname=="DELAY_DIR"){
	d = msfc.delayDirMeas(fieldid, time);
      }
      else if(colname=="PHASE_DIR"){
	d = msfc.phaseDirMeas(fieldid, time);
      }
      else if(colname=="REFERENCE_DIR"){
	d = msfc.referenceDirMeas(fieldid, time);
      }
      else{
	*itsLog << LogIO::SEVERE
		<< "Illegal FIELD direction column name: " << dircolname
		<< LogIO::POST;
      }
      String f(format);
      f.downcase();
      if (f.startsWith("s")) {
    	  return new variant(d.toString());
      }
      MeasureHolder out(d);
      
      Record outRec;
      if (out.toRecord(error, outRec)) {
	retval = new variant(fromRecord(outRec));
      } else {
	error += String("Failed to generate direction return value.\n");
	*itsLog << LogIO::SEVERE << error << LogIO::POST;
      };
    }
  } catch (AipsError(x)) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
  }

  return retval;
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
       Table::relinquishAutoLocks(True);
       RETHROW(x);
   }
   Table::relinquishAutoLocks(True);
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
       Table::relinquishAutoLocks(True);
       RETHROW(x);
   }
   Table::relinquishAutoLocks(True);
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
       Table::relinquishAutoLocks(True);
       RETHROW(x);
   }
   Table::relinquishAutoLocks(True);
   return retval;
}

template <typename T>
static void
append(Array<T> &data, unsigned &current_length, 
       unsigned nrow,
       const Array<T> &data_chunk, 
       const string &column)
{
  unsigned dimension = data_chunk.shape().nelements();

  if (data.nelements() == 0) {
    /* Initialize.
       Allocate the full buffer at once,
       because there does not seem to exist an efficient way
       to expand an Array<T> chunk by chunk.
       (like std::vector<>::push_back()). 
       Note that Array<T>::resize() takes linear time, it is inefficent
       to use that in every iteration
    */
    IPosition shape = data_chunk.shape();
    shape(dimension - 1) = nrow;
    data.resize(shape);
    current_length = 0;
  }

  if (dimension != data.shape().nelements()) {

      stringstream ss;
      ss << "Dimension of " << column << " values changed from " << 
        data.shape().nelements() << " to " << dimension;
      throw AipsError(ss.str());
                 
  }

  /* Accumulate */              
  if (dimension == 3) {
    for (unsigned i = 0; i < (unsigned) data_chunk.shape()(0); i++) {
      for (unsigned j = 0; j < (unsigned) data_chunk.shape()(1); j++) {
        for (unsigned k = 0; k < (unsigned) data_chunk.shape()(2); k++) {
          static_cast<Cube<T> >(data)(i, j, current_length+k) =
            static_cast<Cube<T> >(data_chunk)(i, j, k);
        }
      }
    }
  }
  else if (dimension == 2) {
    for (unsigned i = 0; i < (unsigned) data_chunk.shape()(0); i++) {
      for (unsigned j = 0; j < (unsigned) data_chunk.shape()(1); j++) {
        static_cast<Matrix<T> >(data)(i, current_length+j) =
          static_cast<Matrix<T> >(data_chunk)(i, j);
      }
    }
  }
  else if (dimension == 1) {
    for (unsigned i = 0; i < (unsigned) data_chunk.shape()(0); i++) {
      static_cast<Vector<T> >(data)(current_length+i) =
        static_cast<Vector<T> >(data_chunk)(i);
    }
  }
  else {
    stringstream ss;
    ss << "Unsupported dimension of " << column << ": " << dimension;
    throw AipsError(ss.str());
  }

  current_length += data_chunk.shape()(dimension - 1);
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
               const std::string& array,
	       const std::string& obs)
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
	   "array = " << array <<
	   "obs = " << obs << endl;

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
                           array,
			   "",		// stateExpr
			   obs);

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
         
         // Be more flexible about column, i.e.
         // "MODEL_DATA"}
         // "model"     } => "MODEL",
         // "antenna2"    => "ANTENNA2"
         String mycolumn(upcase(column).before("_DATA"));

         *itsLog << "Use " << itsMS->tableName() <<
           ", useflags = " << useflags << LogIO::POST;
         *itsLog << "Compute statistics on " << mycolumn;
         
         if (complex_value != "") {
           *itsLog << ", use " << complex_value;
         }
         *itsLog << "..." << LogIO::POST;



         Block<Int> sortColumns;
         ROVisIterator vi(*sel_p, sortColumns, 0.0);

         unsigned nrow = sel_p->nrow();

         VisBuffer vb(vi);

         /* Apply selection */
         Vector<Vector<Slice> > chanSlices;
         Vector<Vector<Slice> > corrSlices;
         mssel.getChanSlices(chanSlices, itsMS);
         mssel.getCorrSlices(corrSlices, itsMS);
         vi.selectChannel(chanSlices);
         vi.selectCorrelation(corrSlices);

         /* Now loop over the column, collect the data and compute statistics.
            This is somewhat involved because:
            - each column has its own accessor function
            - each column has its own type
            - each column has its own dimension

            In the end, all collected values are linearized and 
            converted to float.
         */
         Array<Complex> data_complex;
         Array<Double> data_double;
         Array<Float> data_float;
         Array<Bool> data_bool;
         Array<Int> data_int;
         unsigned length;  // logical length of data array

         Vector<Bool> flagrows;
         Cube<Bool> flags;
         unsigned flagrows_length;
         unsigned flags_length;

         for (vi.originChunks();
              vi.moreChunks();
              vi.nextChunk()) {
           
           for (vi.origin();
                vi.more();
                vi++) {
            
             Array<Complex> data_complex_chunk;
             Array<Double> data_double_chunk;
             Array<Float> data_float_chunk;
             Array<Bool> data_bool_chunk;
             Array<Int> data_int_chunk;

             if (useflags) {
               Cube<Bool> flag_chunk;
               vi.flag(static_cast<Cube<Bool>&>(flag_chunk));

               Vector<Bool> flagrow_chunk;
               vi.flagRow(static_cast<Vector<Bool>&>(flagrow_chunk));

               /* If FLAG_ROW is set, update flags */
               for (unsigned i = 0; i < flagrow_chunk.nelements(); i++) {
                 if (flagrow_chunk(i))
                   flag_chunk.xyPlane(i).set(true);
               }

               append<Bool>(flags, flags_length, nrow, flag_chunk, "FLAG");
               append<Bool>(flagrows, flagrows_length, nrow, flagrow_chunk,
                            "FLAG_ROW");
             }

             if (mycolumn == "DATA" || mycolumn == "CORRECTED" || mycolumn == "MODEL") {
                 ROVisibilityIterator::DataColumn dc;
                 if(mycolumn == "DATA"){
                   dc = ROVisibilityIterator::Observed;
		   if(vi.msColumns().data().isNull()){
		     throw(AipsError("Data column is not present"));
		   }
		 }
                 else if(mycolumn == "CORRECTED"){
                   dc = ROVisibilityIterator::Corrected;
		   if(vi.msColumns().correctedData().isNull()){
		     throw(AipsError("Corrected Data column is not present"));
		   }
		 }
                 else{
                   dc = ROVisibilityIterator::Model;
		   if(vi.msColumns().modelData().isNull()){
		     throw(AipsError("Model Data column is not present"));
		   }
                 }
                 vi.visibility(static_cast<Cube<Complex>&>(data_complex_chunk),
                               dc);
                 
                 append<Complex>(data_complex, length, nrow, data_complex_chunk,
                                 mycolumn);
             }
             else if (mycolumn == "UVW") {
                 Vector<RigidVector<Double, 3> > uvw;
                 vi.uvw(uvw);
                 
                 data_double_chunk.resize(IPosition(2, 3, uvw.nelements()));
                 for (unsigned i = 0; i < uvw.nelements(); i++) {
                   static_cast<Matrix<Double> >(data_double_chunk)(0, i) = uvw(i)(0);
                   static_cast<Matrix<Double> >(data_double_chunk)(1, i) = uvw(i)(1);
                   static_cast<Matrix<Double> >(data_double_chunk)(2, i) = uvw(i)(2);
                 }
                 append<Double>(data_double, length, nrow, data_double_chunk, mycolumn);
             }
             else if (mycolumn == "UVRANGE") {
                 Vector<RigidVector<Double, 3> > uvw;
                 vi.uvw(uvw);
                 
                 data_double_chunk.resize(IPosition(1, uvw.nelements()));
                 for (unsigned i = 0; i < uvw.nelements(); i++) {
                   static_cast<Vector<Double> >(data_double_chunk)(i) = 
                     sqrt( uvw(i)(0)*uvw(i)(0) + uvw(i)(1)*uvw(i)(1) );
                 }
                 append<Double>(data_double, length, nrow, data_double_chunk, mycolumn);
             }
             else if (mycolumn == "FLAG") {
                 vi.flag(static_cast<Cube<Bool>&>(data_bool_chunk));
                 append<Bool>(data_bool, length, nrow, data_bool_chunk, mycolumn);
             }
             else if (mycolumn == "WEIGHT") {
                 vi.weightMat(static_cast<Matrix<Float>&>(data_float_chunk));
                 append<Float>(data_float, length, nrow, data_float_chunk, mycolumn);
             }
             else if (mycolumn == "SIGMA") {
                 vi.sigmaMat(static_cast<Matrix<Float>&>(data_float_chunk));
                 append<Float>(data_float, length, nrow, data_float_chunk, mycolumn);
             }
             else if (mycolumn == "ANTENNA1") {
                 vi.antenna1(static_cast<Vector<Int>&>(data_int_chunk));
                 append<Int>(data_int, length, nrow, data_int_chunk, mycolumn);
             }
             else if (mycolumn == "ANTENNA2") {
                 vi.antenna2(static_cast<Vector<Int>&>(data_int_chunk));
                 append<Int>(data_int, length, nrow, data_int_chunk, mycolumn);
             }
             else if (mycolumn == "FEED1") {
                 vi.feed1(static_cast<Vector<Int>&>(data_int_chunk));
                 append<Int>(data_int, length, nrow, data_int_chunk, mycolumn);
             }
             else if (mycolumn == "FEED2") {
                 vi.feed2(static_cast<Vector<Int>&>(data_int_chunk));
                 append<Int>(data_int, length, nrow, data_int_chunk, mycolumn);
             }
             else if (mycolumn == "FIELD_ID") {
                 data_int_chunk.resize(IPosition(1, 1));
                 data_int_chunk(IPosition(1, 0)) = vi.fieldId();
                 append<Int>(data_int, length, nrow, data_int_chunk, mycolumn);
             }
             else if (mycolumn == "ARRAY_ID") {
                 data_int_chunk.resize(IPosition(1, 1));
                 data_int_chunk(IPosition(1, 0)) = vi.arrayId();
                 append<Int>(data_int, length, nrow, data_int_chunk, mycolumn);
             }
             else if (mycolumn == "DATA_DESC_ID") {
                 data_int_chunk.resize(IPosition(1, 1));
                 data_int_chunk(IPosition(1, 0)) = vi.dataDescriptionId();
                 append<Int>(data_int, length, nrow, data_int_chunk, mycolumn);
             }
             else if (mycolumn == "FLAG_ROW") {
                 vi.flagRow(static_cast<Vector<Bool>&>(data_bool_chunk));
                 append<Bool>(data_bool, length, nrow, data_bool_chunk, mycolumn);
             }
             else if (mycolumn == "INTERVAL") {
                 vi.timeInterval(static_cast<Vector<Double>&>(data_double_chunk));
                 append<Double>(data_double, length, nrow, data_double_chunk, mycolumn);
             }
             else if (mycolumn == "SCAN_NUMBER" || mycolumn == "SCAN") {
                 vi.scan(static_cast<Vector<Int>&>(data_int_chunk));
                 append<Int>(data_int, length, nrow, data_int_chunk, mycolumn);
             }
             else if (mycolumn == "TIME") {
                 vi.time(static_cast<Vector<Double>&>(data_double_chunk));
                 append<Double>(data_double, length, nrow, data_double_chunk, mycolumn);
             }
             else if (mycolumn == "WEIGHT_SPECTRUM") {
                 vi.weightSpectrum(static_cast<Cube<Float>&>(data_float_chunk));
                 append<Float>(data_float, length, nrow, data_float_chunk, mycolumn);
             }
             else {
                 stringstream ss;
                 ss << "Unsupported column name: " << column;
                 throw AipsError(ss.str());
             }
             
           }
         }

         
         unsigned dimension;
         unsigned n;
         if (data_complex.nelements() > 0) {
           dimension = data_complex.shape().nelements();
           n = data_complex.shape().product();
         }
         else if (data_double.nelements() > 0) {
           dimension = data_double.shape().nelements();
           n = data_double.shape().product();
         }
         else if (data_float.nelements() > 0) {
           dimension = data_float.shape().nelements();
           n = data_float.shape().product();
         }
         else if (data_bool.nelements() > 0) {
           dimension = data_bool.shape().nelements();
           n = data_bool.shape().product();
         }
         else if (data_int.nelements() > 0) {
           dimension = data_int.shape().nelements();
           n = data_int.shape().product();
         }
         else {
           throw AipsError("No data could be found!");
         }
         
         Vector<Bool> f;
         if (useflags) {
           if (dimension == 1) {
             f = flagrows;
           }
           else if (dimension == 2) {
             f = flagrows;
           }
           else if (dimension == 3) {
             f = flags.reform(IPosition(1, n));
           }
           else {
             stringstream ss;
             ss << "Unsupported column name: " << column;
             throw AipsError(ss.str());
           }
         }
         else {
           f = Vector<Bool>(n, false);
         }

         bool supported = true;
         if (data_complex.nelements() > 0) {

           Vector<Complex> v(data_complex.reform(IPosition(1, data_complex.shape().product())));
           retval = fromRecord(Statistics<Complex>::get_stats_complex(v, f,
                                                                      mycolumn, 
                                                                      supported,
                                                                      complex_value));
         }
         else if (data_double.nelements() > 0) {
             if (dimension == 2) {
                 f.resize(0);
                 f = Vector<Bool>(data_double.shape()(1), false);
                 retval = fromRecord(Statistics<Double>::get_stats_array(static_cast<Matrix<Double> >(data_double),
                                                                     f,
                                                                     mycolumn,
                                                                     supported));
             
           }
           else {
               retval = fromRecord(Statistics<Double>::get_stats(data_double.reform(IPosition(1, data_double.shape().product())),
                                                               f,
                                                               mycolumn,
                                                               supported));
           }
         }
         else if (data_bool.nelements() > 0) {
             retval = fromRecord(Statistics<Bool>::get_stats(data_bool.reform(IPosition(1, data_bool.shape().product())),
                                                             f,
                                                             mycolumn,
                                                             supported));
         }
         else if (data_float.nelements() > 0) {
             if (dimension == 2) {
                 f.resize(0);
                 f = Vector<Bool>(data_float.shape()(1), false);
                 retval = fromRecord(Statistics<Float>::get_stats_array(static_cast<Matrix<Float> >(data_float),
                                                                        f,
                                                                        mycolumn,
                                                                        supported));
             }
             else {
               retval = fromRecord(Statistics<Float>::get_stats(data_float.reform(IPosition(1, data_float.shape().product())),
                                                                f,
                                                                mycolumn,
                                                                supported));
             }
         }
         else if (data_int.nelements() > 0) {
             retval = fromRecord(Statistics<Int>::get_stats(data_int.reform(IPosition(1, data_int.shape().product())),
                                                            f,
                                                            mycolumn,
                                                            supported));
         }
         else {
           throw AipsError("No data could be found!");
         }
       } // end if !detached
   } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks(True);
       RETHROW(x);
   }
   Table::relinquishAutoLocks(True);
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
	   const std::string& observation,
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
             correlation, scan, feed, array, observation, uvrange, average,
	     showflags, msselect, pagerows, listfile);
    rstat = True;
   } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks(True);
       RETHROW(x);
   }
   Table::relinquishAutoLocks(True);
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
       Table::relinquishAutoLocks(True);
       RETHROW(x);
   }
   Table::relinquishAutoLocks(True);
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
       Table::relinquishAutoLocks(True);
       RETHROW(x);
   }
   Table::relinquishAutoLocks(True);
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
       Table::relinquishAutoLocks(True);
       RETHROW(x);
   }
   Table::relinquishAutoLocks(True);
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
       Table::relinquishAutoLocks(True);
       RETHROW(x);
   }
   Table::relinquishAutoLocks(True);
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
       Table::relinquishAutoLocks(True);
       RETHROW(x);
   }
   Table::relinquishAutoLocks(True);
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
	      const double regrid_chan_width,
	      const bool hanning
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
				 Double(regrid_chan_width),
				 hanning
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
       Table::relinquishAutoLocks(True);
       RETHROW(x);
  }
  Table::relinquishAutoLocks(True);
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
	 const std::string& veltype,
	 const bool hanning)
{
  Bool rstat(False);

  Bool hanningDone(False);
  Bool needToClearModel(False);
  SubMS *sms = 0;

  try {

    *itsLog << LogOrigin("ms", "cvel");

    Bool t_doHanning = hanning;

    String t_phasec = toCasaString(phasec);

    String t_mode;
    String t_outframe;
    String t_regridQuantity;
    Double t_restfreq;
    String t_regridInterpMeth;
    Double t_cstart;
    Double t_bandwidth;
    Double t_cwidth;
    Bool t_centerIsStart;
    Bool t_startIsEnd;
    Int t_nchan;
    Int t_width;
    Int t_start;

    casa::MDirection  t_phaseCenter;
    Int t_phasec_fieldid = -1;

    //If phasecenter is a simple numeric value then it's taken as a fieldid 
    //otherwise its converted to a MDirection
    if(phasec.type()==::casac::variant::DOUBLEVEC 
       || phasec.type()==::casac::variant::DOUBLE
       || phasec.type()==::casac::variant::INTVEC
       || phasec.type()==::casac::variant::INT){
      t_phasec_fieldid = phasec.toInt();	
      if(t_phasec_fieldid >= (Int)itsMS->field().nrow() || t_phasec_fieldid < 0){
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

    // go over the remaining grid parameters and consolidate them

    if(!SubMS::convertGridPars(*itsLog,
			       mode, 
			       nchan, 
			       start.toString(), 
			       width.toString(),
			       interp, 
			       restfreq.toString(), 
			       outframe,
			       veltype,
			       ////// output ////
			       t_mode,
			       t_outframe,
			       t_regridQuantity,
			       t_restfreq,
			       t_regridInterpMeth,
			       t_cstart, 
			       t_bandwidth,
			       t_cwidth,
			       t_centerIsStart, 
			       t_startIsEnd,			      
			       t_nchan,
			       t_width,
			       t_start
			       )
       ){
      // an error occured
      return False;
    }

    // end prepare regridding parameters

    *itsLog << LogOrigin("ms", "cvel");

    String originalName = itsMS->tableName();

    // test parameters of input SPWs
    Bool foundInconsistentSPW = False;
    
    {
      Table spwtable(originalName+"/SPECTRAL_WINDOW");
      ROArrayColumn<Double> chanwidths(spwtable, "CHAN_WIDTH");
      ROArrayColumn<Double> chanfreqs(spwtable, "CHAN_FREQ");

      if(spwtable.nrow()>1){
	needToClearModel = True;
      }

      for(uInt ii=0; ii<spwtable.nrow(); ii++){
	Vector<Double> cw(chanwidths(ii));
	Vector<Double> cf(chanfreqs(ii));
	Int totNumChan = cw.size();
      
	Bool isEquidistant = True;
	for(Int i=0; i<totNumChan; i++){
	  cw(i) = abs(cw(i)); // ignore sign of width
	  if(abs(cw(i)-cw(0))>0.1){
	    isEquidistant = False;
	  }
	}
	Double minWidth = min(cw);
	Double maxWidth = max(cw);

	ostringstream oss;
      
	if(isEquidistant){
	  oss <<  "Input spectral window " << ii << " has " << totNumChan 
	      << " channels of width " << scientific << setprecision(6) << setw(6) << cw(0) << " Hz";
	}
	else{
	  oss << "Input spectral window " << ii << " has " << totNumChan 
	      << " channels of varying width: minimum width = " << scientific << setprecision(6) << setw(6) << minWidth 
	      << " Hz, maximum width = " << scientific << setprecision(6) << setw(6) << maxWidth << " Hz";
	}
	oss << endl;
	if(totNumChan > 1){
	  oss << "   First channel center = " << setprecision(9) << setw(9) << cf(0) 
	      << " Hz, last channel center = " << setprecision(9) << setw(9) << cf(totNumChan-1) << " Hz";
	}
	else{
	  oss << "   Channel center = " << setprecision(9) << setw(9) << cf(0) << " Hz";
	}

	Double chanOrderSign = 1.;
	if((cf(0)-cf(totNumChan-1))>0.){ // i.e. channel order is descending
	  chanOrderSign = -1.;
	  oss << "\n  i.e. channels are in order of decreasing frequency.\n";
	}
	
	for(Int i=0; i<totNumChan-2; i++){
	  if( abs((cf(i)+cw(i)/2. * chanOrderSign) - (cf(i+1)-cw(i+1)/2. * chanOrderSign))>1.0 ){
	    oss << "\n   Internal ERROR: Center of channel " << i <<  " is off nominal center by " 
		<< ((cf(i)+cw(i)/2.* chanOrderSign) - (cf(i+1)-cw(i+1)/2.* chanOrderSign)) << " Hz\n" 
		<< "   Distance between channels " << i << " and " << i+1 << " (" 
		<< scientific << setprecision(6) << setw(6) << cf(i+1)-cf(i) << " Hz) is not equal to what is"
		<< " expected\n   from their channel widths which would be " 
		<< scientific << setprecision(6) << setw(6) << + chanOrderSign * (cw(i)/2.+cw(i+1)/2.) << " Hz.\n"
		<< "   Will skip other channels in this SPW.";
	    foundInconsistentSPW = True;
	    break;
	  }
	}
	*itsLog << LogIO::NORMAL  << oss.str() << LogIO::POST;
      }
    }
    if(foundInconsistentSPW){
      throw(AipsError("Inconsistent SPECTRAL_WINDOW table in input MS."));
    }

    // check disk space: need at least twice the size of the original for safety
    if (2 * DOos::totalSize(itsMS->tableName(), True) >
	DOos::freeSpace(Vector<String>(1, itsMS->tableName()), True)(0)) {
      *itsLog << "Not enough disk space. To be on the safe side, need at least "
	      << 2 * DOos::totalSize(itsMS->tableName(), True)/1E6
	      << " MBytes on the filesystem containing " << itsMS->tableName()
	      << " for the SPW combination and regridding to succeed." << LogIO::EXCEPTION;
    }

    // need exclusive rights to this MS, will re-open it after combineSpws
    itsMS->flush();
    close();

    *itsLog << LogOrigin("ms", "cvel");

    sms = new SubMS(originalName, Table::Update);

    *itsLog << LogIO::NORMAL << "Starting combination of spectral windows ..." << LogIO::POST;

    // combine Spws
    if(!sms->combineSpws()){
      *itsLog << LogIO::SEVERE << "Error combining spectral windows." << LogIO::POST;
      delete sms;
      open(originalName,  Table::Update, False); 
      return False;
    }

    *itsLog << LogIO::NORMAL << " " << LogIO::POST; 

    //    cout << "trq " << t_regridQuantity << " ts " << t_start << " tcs " << t_cstart << " tb " 
    //    	 << t_bandwidth << " tcw " << t_cwidth << " tw " << t_width << " tn " << t_nchan << endl; 

    // Regrid

    *itsLog << LogIO::NORMAL << "Testing if spectral frame transformation/regridding is needed ..." << LogIO::POST;

    Int rval;
    String regridMessage;

    if((rval = sms->regridSpw(regridMessage,
			      t_outframe,
			      t_regridQuantity,
			      t_restfreq,
			      t_regridInterpMeth,
			      t_cstart, 
			      t_bandwidth,
			      t_cwidth,
			      t_doHanning,
			      t_phasec_fieldid, // == -1 if t_phaseCenter is valid
			      t_phaseCenter,
			      True, // use "center is start" mode
			      t_startIsEnd,			      
			      t_nchan,
			      t_width,
			      t_start
			      )
	)==1){ // successful modification of the MS took place
      hanningDone = t_doHanning;

      *itsLog << LogIO::NORMAL << "Spectral frame transformation/regridding completed." << LogIO::POST;
      if(hanningDone){
	*itsLog << LogIO::NORMAL << "Hanning smoothing was applied." << LogIO::POST;
      }
      
      

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
	    << " start= " << t_start << " bandwidth=" << t_bandwidth
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
    sms = 0;

    if(rstat){
      // print parameters of final SPW
      Table spwtable(originalName+"/SPECTRAL_WINDOW");
      ROArrayColumn<Double> chanwidths(spwtable, "CHAN_WIDTH");
      ROArrayColumn<Double> chanfreqs(spwtable, "CHAN_FREQ");
      
      Vector<Double> cw(chanwidths(0));
      Vector<Double> cf(chanfreqs(0));
      Int totNumChan = cw.size();
      
      Bool isEquidistant = True;
      for(Int i=0; i<totNumChan; i++){
	if(abs(cw(i)-cw(0))>0.1){
	  isEquidistant = False;
	}
      }
      Double minWidth = min(cw);
      Double maxWidth = max(cw);

      ostringstream oss;
      
      if(isEquidistant){
	oss <<  "Final spectral window has " << totNumChan 
	    << " channels of width " << scientific << setprecision(6) << setw(6) << cw(0) << " Hz";
      }
      else{
	oss << "Final spectral window has " << totNumChan 
	    << " channels of varying width: minimum width = " << scientific << setprecision(6) << setw(6) << minWidth 
	    << " Hz, maximum width = " << scientific << setprecision(6) << setw(6) << maxWidth << " Hz";
      }
      oss << endl;
      if(totNumChan > 1){
	oss << "First channel center = " << setprecision(9) << setw(9) << cf(0) 
	    << " Hz, last channel center = " << setprecision(9) << setw(9) << cf(totNumChan-1) << " Hz";
      }
      else{
	oss << "Channel center = " << setprecision(9) << setw(9) << cf(0) << " Hz";
      }

      Double chanOrderSign = 1.;
      if((cf(0)-cf(totNumChan-1))>0.){ // i.e. channel order is descending
	chanOrderSign = -1.;
	oss << "\n  i.e. channels are in order of decreasing frequency.\n";
      }
	
      for(Int i=0; i<totNumChan-2; i++){
	if( abs((cf(i)+cw(i)/2. * chanOrderSign) - (cf(i+1)-cw(i+1)/2. * chanOrderSign))>1.0 ){
	  oss << "\n   Internal Error: Center of channel " << i <<  " is off nominal center by " 
	      << ((cf(i)+cw(i)/2.* chanOrderSign) - (cf(i+1)-cw(i+1)/2.* chanOrderSign)) << " Hz\n"
	      << "     Will not test subsequent channels."; 
	  break;
	}
      }
      *itsLog << LogIO::NORMAL  << oss.str() << LogIO::POST;
    } 
    
    if(rstat){ // re-open MS for writing, unlocked
      open(originalName,  False, False); 
      *itsLog << LogOrigin("ms", "cvel");
      if(!hanningDone && t_doHanning){ 
	// still need to Hanning Smooth
	hanningsmooth("all");
      }

      if(needToClearModel){
	*itsLog << LogIO::NORMAL  << "NOTE: any virtual model data will be cleared." << LogIO::POST;
	VisModelData::clearModel(*itsMS);
      }

    }

  } catch (AipsError x) {
    if(sms){
      delete sms;
    }
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    Table::relinquishAutoLocks(True);
    RETHROW(x);
  }
  Table::relinquishAutoLocks(True);
  return rstat;
}

std::vector<double>
ms::cvelfreqs(const std::vector<int>& spwids,
	      const std::vector<int>& fieldids,
	      const std::string& obstime,
	      const std::string& mode, 
	      const int nchan, 
	      const ::casac::variant& start, 
	      const ::casac::variant& width,
	      const ::casac::variant& phasec, 
	      const ::casac::variant& restfreq, 
	      const std::string& outframe,
	      const std::string& veltype,
	      const bool verbose
	      )
{
  std::vector<double> rval(0); // the new channel centers

  try {

    if(!detached()){
    
      *itsLog << LogOrigin("ms", "cvelfreqs");

      if(verbose){
	*itsLog << LogIO::NORMAL << "Calculating grid ..." << LogIO::POST;
      }
      
      Vector<Double> newCHAN_FREQ; 
      Vector<Double> newCHAN_WIDTH;
      
      
      ROMSMainColumns mainCols(*itsMS);
      ROScalarColumn<Double> timeCol(mainCols.time());
      ROScalarColumn<Int> ddCol(mainCols.dataDescId());
      ROScalarColumn<Int> fieldCol(mainCols.fieldId());
      
      ROMSDataDescColumns DDCols(itsMS->dataDescription());
      ROScalarColumn<Int> spwidCol(DDCols.spectralWindowId());
      
      ROMSSpWindowColumns SPWCols(itsMS->spectralWindow());
      ROMSFieldColumns FIELDCols(itsMS->field());
      ROMSAntennaColumns ANTCols(itsMS->antenna());
      
      // extract grid from SPW given by spwids
      Vector<Double> oldCHAN_FREQ; 
      Vector<Double> oldCHAN_WIDTH;
      
      if(spwids.size() == 1){ // only one spw selected
	oldCHAN_FREQ.assign(SPWCols.chanFreq()(spwids[0])); 
	oldCHAN_WIDTH.assign(SPWCols.chanWidth()(spwids[0]));
      }
      else if(spwids.size() > 1){ // several ids selected
	SubMS theMS(*itsMS);
	Vector<Int> spwidv(spwids);
	if(!theMS.combineSpws(spwidv,
			      True, // dont't modify the MS
			      oldCHAN_FREQ,
			      oldCHAN_WIDTH,
			      verbose
			      )){
	  *itsLog << LogIO::SEVERE << "Error combining SPWs." << LogIO::POST;
	  return rval;
	}	
      }
      else{
      	*itsLog << LogIO::NORMAL << "Zero SPWs selected." << LogIO::POST;
	return rval;
      }
            
      // determine old reference frame
      casa::MFrequency::Types theOldRefFrame = MFrequency::castType(SPWCols.measFreqRef()(spwids[0]));
      
      // determine observation epoch
      casa::MEpoch theObsTime;
      String t_obstime = toCasaString(obstime);
      if (obstime.length()>0) {
	Quantum<Double> qt;
	if (MVTime::read(qt,obstime)) {
		casa::MVEpoch mv(qt);
	  theObsTime = casa::MEpoch(mv, MEpoch::UTC);
	} else {
	  *itsLog << LogIO::SEVERE << "Invalid time format: " 
		  << obstime << LogIO::POST;
	  return rval;
	}
      } else {
	// take the smallest obstime in the MS given the spw and field selection
	
	// determine the relevant DD Ids
	vector<Int> ddids;
	for(uInt irow=0; irow<DDCols.nrow(); irow++){ // loop over DD table
	  for(uInt i=0; i<spwids.size(); i++){
	    Int theSPWId = spwidCol(irow);
	    if(theSPWId==spwids[i]){ // SPWId match found
	      ddids.push_back(irow);
	    }
	  } // end for
	} // end for
	//cout << "DD IDs selected " << Vector<Int>(ddids) << endl; 
	uInt minTimeRow = 0;
	Double minTime = 1E42;
	Bool doField = (fieldids.size()!=0);
	
	for(uInt irow=0;irow<itsMS->nrow(); irow++){ // loop over main table
	  if(timeCol(irow)<minTime){
	    
	    Int theDDId = ddCol(irow);
	    for(uInt i=0; i<ddids.size(); i++){
	      if(theDDId==ddids[i]){ // DD match found
		if(doField){
		  Int theFieldId = fieldCol(irow);
		  for(uInt i=0; i<fieldids.size(); i++){
		    if(theFieldId==fieldids[i]){ // field match found
		      minTime=timeCol(irow);
		      minTimeRow = irow;
		      break;
		    }
		  }
		}
		else{ // all fields selected
		  minTime=timeCol(irow);
		  minTimeRow = irow;
		}
		break;
	      } // end if DD matches
	    } // end for
	    
	  }
	} // end for
	theObsTime = mainCols.timeMeas()(minTimeRow);
	if(verbose){
	  *itsLog << LogIO::NORMAL << "Using observation time from earliest row of the MS given the SPW and FIELD selection:" << LogIO::POST;
	  *itsLog << LogIO::NORMAL << "    " << MVTime(theObsTime.getValue().getTime()).string(MVTime::YMD)
		  << " (" << theObsTime.getRefString() << ")" << LogIO::POST;
	}
      }
      
      // determine phase center
      casa::MDirection phaseCenter;
      casa::MRadialVelocity mRV; // needed when using outframe "SOURCE"
      Int phasec_fieldid = -1;
      String t_phasec = toCasaString(phasec);
      
      //If phasecenter is a simple numeric value then it's taken as a fieldid 
      //otherwise its converted to a MDirection
      if(phasec.type()==::casac::variant::DOUBLEVEC 
	 || phasec.type()==::casac::variant::DOUBLE
	 || phasec.type()==::casac::variant::INTVEC
	 || phasec.type()==::casac::variant::INT){
	phasec_fieldid = phasec.toInt();	
	if(phasec_fieldid >= (Int)itsMS->field().nrow() || phasec_fieldid < 0){
	  *itsLog << LogIO::SEVERE << "Field id " << phasec_fieldid
		  << " selected to be used as phasecenter does not exist." << LogIO::POST;
	  return rval;
	}
	else{
	  phaseCenter = FIELDCols.phaseDirMeas(phasec_fieldid, theObsTime.get("s").getValue());
	  mRV = FIELDCols.radVelMeas(phasec_fieldid, theObsTime.get("s").getValue());
	}
      }
      else{
	if(t_phasec.empty()){
	  phasec_fieldid = 0;
	  if(fieldids.size()!=0){
	    phasec_fieldid = fieldids[0];
	  }
	  phaseCenter = FIELDCols.phaseDirMeas(phasec_fieldid, theObsTime.get("s").getValue());
	  mRV = FIELDCols.radVelMeas(phasec_fieldid, theObsTime.get("s").getValue());
	}
	else{
	  if(!casaMDirection(phasec, phaseCenter)){
	    *itsLog << LogIO::SEVERE << "Could not interprete phasecenter parameter "
		    << t_phasec << LogIO::POST;
	    return rval;
	  }
	  else{
	    if(verbose){
	      *itsLog << LogIO::NORMAL << "Using user-provided phase center." << LogIO::POST;
	    }
	  }
	}
      }


      // determine observatory position
      // use a tabulated version if available
      casa::MPosition mObsPos;
      {
	      casa::MPosition Xpos;
	String Xobservatory;
	ROMSObservationColumns XObsCols(itsMS->observation());
	if (itsMS->observation().nrow() > 0) {
	  Xobservatory = XObsCols.telescopeName()(mainCols.observationId()(0));
	}
	if (Xobservatory.length() == 0 || 
	    !casa::MeasTable::Observatory(Xpos,Xobservatory)) {
	  // unknown observatory
	  if(verbose){
	    *itsLog << LogIO::WARN << "Unknown observatory: \"" << Xobservatory 
		    << "\". Determining observatory position from antenna 0." << LogIO::POST;
	  }
	  Xpos=casa::MPosition::Convert(ANTCols.positionMeas()(0), casa::MPosition::ITRF)();
	}
	else{
	  if(verbose){
	    *itsLog << LogIO::NORMAL << "Using tabulated observatory position for " << Xobservatory << ":"
		    << LogIO::POST;
	  }
	  Xpos=casa::MPosition::Convert(Xpos, casa::MPosition::ITRF)();
	}
	if(verbose){
	  mObsPos = Xpos;
	  ostringstream oss;
	  oss <<  "   " << mObsPos << " (ITRF)";
	  *itsLog << LogIO::NORMAL << oss.str() << LogIO::POST;
	}
      }
      
      // calculate new grid
      SubMS::calcChanFreqs(*itsLog,
			   newCHAN_FREQ, 
			   newCHAN_WIDTH,
			   oldCHAN_FREQ, 
			   oldCHAN_WIDTH,
			   phaseCenter,
			   theOldRefFrame,
			   theObsTime,
			   mObsPos,
			   mode, 
			   nchan, 
			   start.toString(), 
			   width.toString(),
			   restfreq.toString(), 
			   outframe,
			   veltype,
			   verbose, 
			   mRV
			   );
      
      newCHAN_FREQ.tovector(rval);
      
    } // end if !detached

  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }

  return rval;
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
       Table::relinquishAutoLocks(True);
       RETHROW(x);
   }
   Table::relinquishAutoLocks(True);
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
       Table::relinquishAutoLocks(True);
       RETHROW(x);
  }
  Table::relinquishAutoLocks(True);
  return rstat;
}

bool
ms::concatenate(const std::string& msfile, const ::casac::variant& freqtol, const ::casac::variant& dirtol, const float weightscale, 
		const int handling, const std::string& destmsfile, const bool respectname)
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

	    addephemcol(appendedMS); // add EPHEMERIS_ID column to FIELD table of itsMS if necessary

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
	    
	    mscat.setTolerance(freqtolerance, dirtolerance);
	    mscat.setRespectForFieldName(respectname);
	    mscat.setWeightScale(weightscale);
	    mscat.concatenate(appendedMS, static_cast<uint>(handling), destmsfile);

	    String message = String(msfile) + " appended to " + itsMS->tableName();
	    ostringstream param;
	    param << "msfile='" << msfile
		  << "' freqTol='" << casaQuantity(freqtol) 
		  << "' dirTol='" << casaQuantity(dirtol) 
		  << "' respectname='" << respectname 
		  << "' handling= " << handling
		  << " destmsfile='" << destmsfile << "'";
	    String paramstr=param.str();
	    writehistory(std::string(message.data()), std::string(paramstr.data()),
			 std::string("ms::concatenate()"), msfile, "ms");
	}
	rstat = True;
    } catch (AipsError x) {
	*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	Table::relinquishAutoLocks(True);
	RETHROW(x);
    }
    Table::relinquishAutoLocks(True);
    return rstat;
}

bool
ms::testconcatenate(const std::string& msfile, const ::casac::variant& freqtol, const ::casac::variant& dirtol,
		    const bool respectname)
{
    Bool rstat(False);
    try {
	if(!detached()){
	    *itsLog << LogOrigin("ms", "testconcatenate");
	    
	    *itsLog << LogIO::NORMAL << "*** Note: this method does _not_ merge the Main and Pointing tables!"
		    << LogIO::POST;

	    if (!Table::isReadable(msfile)) {
		*itsLog << "Cannot read the measurement set called " << msfile
			<< LogIO::EXCEPTION;
	    }                   

	    const MeasurementSet appendedMS(msfile);
	    
	    addephemcol(appendedMS); // add EPHEMERIS_ID column to FIELD table of itsMS if necessary

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
	    
	    mscat.setTolerance(freqtolerance, dirtolerance);
	    mscat.setRespectForFieldName(respectname);
	    mscat.concatenate(appendedMS, 3); // 3 meaning "don't concatenate Main and Pointing table"

	    String message = "Subtables from "+String(msfile) + " appended to those from " + itsMS->tableName();
	    ostringstream param;
	    param << "msfile= " << msfile
		  << " freqTol='" << casaQuantity(freqtol) << "' dirTol='"
		  << casaQuantity(dirtol) << "'" << "' respectname='" << respectname;
	    String paramstr=param.str();
	    writehistory(std::string(message.data()), std::string(paramstr.data()),
			 std::string("ms::testconcatenate()"), msfile, "ms");
	    itsMS->flush(True);
	}
	rstat = True;
    } catch (AipsError x) {
	*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	Table::relinquishAutoLocks(True);
	RETHROW(x);
    }
    Table::relinquishAutoLocks(True);
    return rstat;
}

bool
ms::virtconcatenate(const std::string& msfile, const std::string& auxfile, const ::casac::variant& freqtol, 
		    const ::casac::variant& dirtol, const float weightscale, const bool respectname)
{
    Bool rstat(False);
    try {
	if(!detached()){
	    *itsLog << LogOrigin("ms", "virtconcatenate");
	    
	    if (!Table::isReadable(msfile)) {
		*itsLog << "Cannot read the measurement set called " << msfile
			<< LogIO::EXCEPTION;
	    }                   

	    MeasurementSet appendedMS(msfile, Table::Update);

	    if (!appendedMS.isWritable()) {
		*itsLog << "Cannot write to the measurement set called " << msfile
			<< LogIO::EXCEPTION;
	    }                   

	    addephemcol(appendedMS); // add EPHEMERIS_ID to FIELD table of itsMS if necessary

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
	    
	    mscat.setTolerance(freqtolerance, dirtolerance);
	    mscat.setRespectForFieldName(respectname);
	    mscat.setWeightScale(weightscale);

	    mscat.virtualconcat(appendedMS, True, casa::String(auxfile));

	    String message = String(msfile) + " virtually concatenated with " + itsMS->tableName();
	    ostringstream param;
	    param << "msfile='" << msfile
		  << "' freqTol='" << casaQuantity(freqtol) 
		  << "' dirTol='" << casaQuantity(dirtol) 
		  << "' respectname='" << respectname << "'";
	    String paramstr=param.str();
	    writehistory(std::string(message.data()), std::string(paramstr.data()),
			 std::string("ms::virtconcatenate()"), msfile, "ms");
	}
	rstat = True;
    } catch (AipsError x) {
	*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	Table::relinquishAutoLocks(True);
	RETHROW(x);
    }
    Table::relinquishAutoLocks(True);
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
	Table::relinquishAutoLocks(True);
	RETHROW(x);
    }
    Table::relinquishAutoLocks(True);
    return rstat;
}

bool
ms::sort(const std::string& msname,const std::vector<std::string>& columns)
{
    Bool rstat(False);
    try {
	if(!detached()){
	    *itsLog << LogOrigin("ms", "sort");

	    if (DOos::totalSize(itsMS->tableName(), True) >
		DOos::freeSpace(Vector<String>(1, itsMS->tableName()), True)(0)) {
		*itsLog << "There does not appear to be enough free disk space "
			<< "(on the filesystem containing " << itsMS->tableName()
			<< ") for the sorting to succeed." << LogIO::EXCEPTION;
	    }

	    {

		// Prepare columns for Table::sort method
		Block<String> cols(columns.size());
		for (uInt col=0;col<columns.size();col++)
		{
			cols[col] = columns.at(col);
		}

		// Prepare columns for loggin info
		ostringstream oss;
		oss.clear();
		oss << columns;
		String strCols(oss.str());

		String originalName = itsMS->tableName();
		itsMS->flush();

		MeasurementSet sortedMS = itsMS->sort(cols);

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
		    *itsLog << LogOrigin("ms", "sort");
		    String message = "Sorted by " + oss.str() + " in ascending order.";
		    writehistory(std::string(message.data()), "", std::string("ms::sort()"), originalName, "ms");
		    *itsLog << LogIO::NORMAL << "Sorted main table of " << originalName << " by  " + strCols + " ."
			    << LogIO::POST;
		}
		else { // sort into a new MS
		    sortedMS.deepCopy(msname, Table::New);
		    String message = "Generated from " + originalName + " by sorting by  " + strCols + "  in ascending order.";
		    writehistory(std::string(message.data()), "", std::string("ms::sort()"), msname, "ms");

		    *itsLog << LogIO::NORMAL << "Sorted main table of " << originalName << " by  " + strCols + "  and stored it in "
			    << msname << " ."<< LogIO::POST;

		}
	    }

	    rstat = True;
	}

    } catch (AipsError x) {
	*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	Table::relinquishAutoLocks(True);
	RETHROW(x);
    }
    Table::relinquishAutoLocks(True);
    return rstat;
}

bool ms::contsub(const std::string& outputms,    const ::casac::variant& fitspw,
                 const int fitorder,             const std::string& combine,
                 const ::casac::variant& spw,    const ::casac::variant& unionspw,
                 const ::casac::variant& field,  const ::casac::variant& scan,
                 const std::string&      intent, const std::string& correlation,
                 const std::string&      obs,    const std::string& whichcol)
{
  Bool rstat(False);

  try{
    *itsLog << LogOrigin("ms", "contsub");

    SubMS subtractor(*itsMS);
    *itsLog << LogIO::NORMAL2 << "Sub MS created" << LogIO::POST;
    String t_field(m1toBlankCStr_(field));
    String t_fitspw(m1toBlankCStr_(fitspw));
    String t_spw(m1toBlankCStr_(spw));
    String t_unionspw(m1toBlankCStr_(unionspw));
    if(t_spw == "")   // MSSelection doesn't respond well to "", and setting it
      t_spw = "*";    // at the XML level does not work.
    if(t_spw != "*" && t_spw != t_unionspw){
      *itsLog << LogIO::WARN
              << "The spws in the output will be remapped according to "
              << t_unionspw << ", not " << t_spw
              << LogIO::POST;
      *itsLog << LogIO::WARN
              << "This only affects the numbering of the spws, not their validity or frequencies."
              << LogIO::POST;
    }
    String t_scan    = toCasaString(scan);
    String t_intent  = toCasaString(intent);
    String t_obs     = toCasaString(obs);
    String t_correlation = upcase(correlation);

    if(!subtractor.setmsselect(t_unionspw, t_field, 
                               "",                      // antenna
                               t_scan,
                               "",                      // uvrange
                               "",                      // taql
                               Vector<Int>(1, 1),       // step
                               "",                      // subarray
                               t_correlation,
                               t_intent, t_obs)){
      *itsLog << LogIO::SEVERE
	      << "Error selecting data."
	      << LogIO::POST;
      return false;
    }
    
    String t_outputms(outputms);
    String t_whichcol(whichcol);
    Vector<Int> t_tileshape(1, 0);
    const String t_combine = downcase(combine);

    subtractor.setFitOrder(fitorder);
    subtractor.setFitSpw(t_fitspw);
    subtractor.setFitOutSpw(t_spw);

    if(!subtractor.makeSubMS(t_outputms, t_whichcol, t_tileshape, t_combine)){
      *itsLog << LogIO::SEVERE
	      << "Error subtracting from " << itsMS->tableName() << " to "
	      << t_outputms
	      << LogIO::POST;
      return false;
    }
       
    *itsLog << LogIO::NORMAL2 << "Continuum subtracted" << LogIO::POST;

    {// Update HISTORY table of newly created MS
      String message = "Continuum subtracted from " + itsMS->tableName();
      ostringstream param;
      param << "fieldids=" << t_field << " spwids=" << t_spw
	    << " whichcol='" << whichcol << "'";
      String paramstr=param.str();
      writehistory(message, paramstr, "ms::contsub()", outputms, "ms");
    }

    rstat = True;
  }
  catch (AipsError x){
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    Table::relinquishAutoLocks(True);
    RETHROW(x);
  }
  Table::relinquishAutoLocks(True);
  return rstat;
}

bool ms::statwt(const bool dorms,                const bool byantenna,
                const bool sepacs,               const ::casac::variant& fitspw,
                const ::casac::variant& fitcorr, const std::string& combine,
                const ::casac::variant& timebin, const int minsamp,
                const ::casac::variant& field,   const ::casac::variant& spw,
                const ::casac::variant& baseline, const std::string& timerange,
                const ::casac::variant& scan,    const std::string&      intent,
                const ::casac::variant& subarray,const std::string& correlation,
                const std::string&      obs,     const std::string& datacol)
{
  Bool rstat(False);

  try {
    *itsLog << LogOrigin("ms", "statwt");

    Reweighter reweighter(itsMS->tableName(), dorms, minsamp);

    *itsLog << LogIO::NORMAL2 << "Reweighter created" << LogIO::POST;
    String t_field(m1toBlankCStr_(field));
    String t_fitspw(m1toBlankCStr_(fitspw));
    String t_spw(m1toBlankCStr_(spw));
    if(t_spw == "")   // MSSelection doesn't respond well to "", and setting it
      t_spw = "*";    // at the XML level does not work.

    String t_baseline = toCasaString(baseline);
    String t_scan    = toCasaString(scan);
    String t_intent  = toCasaString(intent);
    String t_obs     = toCasaString(obs);
    String t_subarray    = toCasaString(subarray);
    String t_correlation = upcase(correlation);

    if(!reweighter.setmsselect(t_fitspw, t_spw,
                               t_field, 
                               t_baseline,                      // antenna
                               t_scan,
                               t_subarray,                      // subarray
                               t_correlation,
                               t_intent, t_obs)){
      *itsLog << LogIO::SEVERE
	      << "Error selecting data."
	      << LogIO::POST;
      return false;
    }
    Double timeInSec=casaQuantity(timebin).get("s").getValue();
    reweighter.selectTime(timeInSec, String(timerange));

    String t_whichcol(datacol);
    const String t_combine = downcase(combine);

    reweighter.setFitSpw(t_fitspw);
    reweighter.setOutSpw(t_spw);
    if(!reweighter.reweight(t_whichcol, t_combine)){
      *itsLog << LogIO::SEVERE
	      << "Error reweighting " << itsMS->tableName()
	      << LogIO::POST;
      return false;
    }
       
    *itsLog << LogIO::NORMAL2
            << itsMS->tableName() << " reweighted"
            << LogIO::POST;
    rstat = True;
  }
  catch(AipsError x){
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    Table::relinquishAutoLocks(True);
    RETHROW(x);
  }
  Table::relinquishAutoLocks(True);
  return rstat;
}

bool
ms::split(const std::string&      outputms,  const ::casac::variant& field, 
	  const ::casac::variant& spw,       const std::vector<int>& step,
          const ::casac::variant& antenna,   const ::casac::variant& timebin,
          const std::string&      timerange, const ::casac::variant& scan,
          const ::casac::variant& uvrange,   const std::string&      taql,
          const std::string&      whichcol,  const ::casac::variant& tileShape,
          const ::casac::variant& subarray,  const std::string&      combine,
          const std::string& correlation,    const std::string&      intent,
          const std::string&      obs)
{
  Bool rstat(False);
  SubMS *splitter = NULL;
  try {
    *itsLog << LogOrigin("ms", "split");
    splitter = new SubMS(*itsMS);
    *itsLog << LogIO::NORMAL2 << "Sub MS created" << LogIO::POST;
    String t_field(m1toBlankCStr_(field));
    String t_spw(m1toBlankCStr_(spw));
    if(t_spw == "")   // MSSelection doesn't respond well to "", and setting it
      t_spw = "*";    // at the XML level does not work.

    String t_antenna = toCasaString(antenna);
    String t_scan    = toCasaString(scan);
    String t_intent  = toCasaString(intent);
    String t_obs     = toCasaString(obs);
    String t_uvrange = toCasaString(uvrange);
    String t_taql(taql);
    const String t_subarray = toCasaString(subarray);
    String t_correlation = upcase(correlation);
    //if(t_correlation == "")
    //  t_correlation = "*";   // * doesn't work.
     
    if(!splitter->setmsselect(t_spw, t_field, t_antenna, t_scan, t_uvrange, 
			      t_taql, Vector<Int>(step), t_subarray, t_correlation,
                              t_intent, t_obs)){
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
    Vector<Int> t_tileshape(1, 0);
    if(toCasaString(tileShape) != String("")){
      t_tileshape.resize();
      t_tileshape=tileShape.toIntVec();
    }
    const String t_combine = downcase(combine);

    if(!splitter->makeSubMS(t_outputms, t_whichcol, t_tileshape, t_combine)){
      //      *itsLog << LogIO::WARN
      //	      << "Splitting " << itsMS->tableName() << " to "
      //	      << t_outputms << " failed."
      //	      << LogIO::POST;
      delete splitter;
      return false;
    }
       
    *itsLog << LogIO::NORMAL2 << "SubMS made" << LogIO::POST;
    delete splitter;
    splitter = NULL;
   
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
    if(splitter){
      delete splitter;
    }
    Table::relinquishAutoLocks(True);
    RETHROW(x);
  }
  Table::relinquishAutoLocks(True);
  return rstat;
}

bool
ms::partition(const std::string&      outputms,   const ::casac::variant& field, 
	      const ::casac::variant& spw,        const ::casac::variant& antenna,
	      const ::casac::variant& timebin,	  const std::string&      timerange,
	      const ::casac::variant& scan,       const ::casac::variant& uvrange,
	      const std::string&      taql,       const std::string&      whichcol,
	      const ::casac::variant& tileShape,  const ::casac::variant& subarray,
	      const std::string&      combine,    const std::string&      intent,
              const std::string&      obs)
{
  Bool rstat(False);
  try {
    *itsLog << LogOrigin("ms", "partition");
    Partition *partitioner = new Partition(*itsMS);
    *itsLog << LogIO::NORMAL2 << "Sub MS created" << LogIO::POST;
    String t_field(m1toBlankCStr_(field));
    String t_spw(m1toBlankCStr_(spw));
    if(t_spw == "")   // MSSelection doesn't respond well to "", and setting it
      t_spw = "*";    // at the XML level does not work.

    String t_antenna = toCasaString(antenna);
    String t_scan    = toCasaString(scan);
    String t_intent  = toCasaString(intent);
    String t_obs     = toCasaString(obs);
    String t_uvrange = toCasaString(uvrange);
    String t_taql(taql);
    const String t_subarray = toCasaString(subarray);
     
    if(!partitioner->setmsselect(t_spw, t_field, t_antenna, t_scan, t_uvrange,
				 t_taql, t_subarray, t_intent, t_obs)){
      *itsLog << LogIO::SEVERE
	      << "Error selecting data."
	      << LogIO::POST;
      delete partitioner;
      return false;
    }
       
    Double timeInSec=casaQuantity(timebin).get("s").getValue();
    partitioner->selectTime(timeInSec, String(timerange));
    String t_outputms(outputms);
    String t_whichcol(whichcol);
    Vector<Int> t_tileshape(1,0);
    if(toCasaString(tileShape) != String("")){
      t_tileshape.resize();
      t_tileshape=tileShape.toIntVec();
    }
    const String t_combine = downcase(combine);

    if(!partitioner->makePartition(t_outputms, t_whichcol, t_tileshape, t_combine)){
      *itsLog << LogIO::SEVERE
	      << "Error partitioning " << itsMS->tableName() << " to "
	      << t_outputms
	      << LogIO::POST;
      delete partitioner;
      return false;
    }
       
    *itsLog << LogIO::NORMAL2 << "Partition made" << LogIO::POST;
    delete partitioner;
   
    rstat = True;
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    Table::relinquishAutoLocks(True);
    RETHROW(x);
  }
  Table::relinquishAutoLocks(True);
  return rstat;
}

bool
ms::iterinit(const std::vector<std::string>& columns, const double interval,
             const int maxrows, const bool adddefaultsortcolumns)
{
   Bool rstat(False);
   try {
     if(!detached()){
       Vector<String> cols=toVectorString(columns);
       if(cols.nelements()==1 && (cols[0]==String("")))
	  cols.resize();
         rstat = itsSel->iterInit(cols, interval, maxrows,
                                  adddefaultsortcolumns);
     }
   } catch (AipsError x) {
     *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
     Table::relinquishAutoLocks(True);
     RETHROW(x);
   }
   Table::relinquishAutoLocks(True);
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
       Table::relinquishAutoLocks(True);
       RETHROW(x);
   }
   Table::relinquishAutoLocks(True);
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
       Table::relinquishAutoLocks(True);
       RETHROW(x);
   }
   Table::relinquishAutoLocks(True);
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
       Table::relinquishAutoLocks(True);
       RETHROW(x);
   }
   Table::relinquishAutoLocks(True);
   return rstat; 
}

/*
bool
ms::tosdfits(const std::string& fitsfile)
{

  *itsLog << LogOrigin("ms", "tosdfits");
  *itsLog << "not implemented"<<LogIO::POST;
  Table::relinquishAutoLocks(True);
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
       Table::relinquishAutoLocks(True);
       RETHROW(x);
   }
   Table::relinquishAutoLocks(True);
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
       Table::relinquishAutoLocks(True);
       RETHROW(x);
   }
   Table::relinquishAutoLocks(True);
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
       Table::relinquishAutoLocks(True);
       RETHROW(x);
   }
   Table::relinquishAutoLocks(True);
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
       Table::relinquishAutoLocks(True);
       RETHROW(x);
   }
   Table::relinquishAutoLocks(True);
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
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks(True);
       RETHROW(x);
   }
   Table::relinquishAutoLocks(True);
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
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks(True);
       RETHROW(x);
   }
   Table::relinquishAutoLocks(True);
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
       //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks(True);
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
       Table::relinquishAutoLocks(True);
       RETHROW(x);
   }
   Table::relinquishAutoLocks(True);
   return rstat; 
}

std::string
ms::asdmref(const std::string& abspath)
{
  std::string retval;
  *itsLog << LogOrigin("ms", "asdmref");
  try {
    if(!detached()){
       // get the data manager of the DATA column
       TableDesc mSTD(itsMS->actualTableDesc());
       ColumnDesc myColDesc(mSTD.columnDesc("DATA"));
       String hcName(myColDesc.dataManagerGroup());
       DataManager* myDM = itsMS->findDataManager(hcName);
       String dataManName(myDM->dataManagerName());

       if(dataManName != "AsdmStMan"){
	 *itsLog << LogIO::NORMAL << "MS does not reference an ASDM." << LogIO::POST;      
       }
       else{

	 AsdmStMan* myASTMan = static_cast<AsdmStMan*>(itsMS->findDataManager(hcName));
	 
	 Block<String> bDFNames;       
	 myASTMan->getBDFNames(bDFNames);

	 if(bDFNames.size()!=0){
	   // from name 0 determine path
	   Path tmpPath(bDFNames[0]);
	   tmpPath = Path(tmpPath.dirName()); // remove BLOB name
	   String binDir(tmpPath.baseName()); // memorize the ASDMBinary dir name
	   String presentPath(tmpPath.dirName()); // remove ASDMBinary dir name
	   *itsLog << LogIO::NORMAL << "Present ASDM reference path:\n" 
		   << presentPath << LogIO::POST;      
	   if(abspath == ""){
	     retval = presentPath;
	   }
	   else{
	     if(abspath=="/"){
	       *itsLog << LogIO::SEVERE << "Choosing abspath==\"/\" is not a good idea ..." << LogIO::POST;
	       retval = presentPath;
	     }
	     else{
	       String absPath(abspath);
	       if(absPath.lastchar()!='/'){
		 absPath += "/";
	       }
	       if(!File(absPath).isDirectory()){
		 *itsLog << LogIO::WARN << "\""+absPath+"\" is presently not a valid path ..." << LogIO::POST;
	       }

	       // modify the bDFNames and write them back
	       for(uInt i=0; i<bDFNames.size(); i++){
		 bDFNames[i] = absPath+binDir+"/"+Path(bDFNames[i]).baseName();
	       }
	       myASTMan->setBDFNames(bDFNames);
	       myASTMan->writeIndex();

	       retval = abspath;
	       *itsLog << LogIO::NORMAL << "New ASDM reference path:\n" 
		       << retval << LogIO::POST;
	     }      
	   }
	 }
       }
    }
  } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks(True);
       RETHROW(x);
  }
  Table::relinquishAutoLocks(True);
  return retval;

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
       Table::relinquishAutoLocks(True);
       RETHROW(x);
  }
  Table::relinquishAutoLocks(True);
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
       Table::relinquishAutoLocks(True);
       RETHROW(x);
   }
   Table::relinquishAutoLocks(True);
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
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks(True);
       RETHROW(x);
   }
   Table::relinquishAutoLocks(True);
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
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks(True);
       RETHROW(x);
 }
 Table::relinquishAutoLocks(True);
 return rstat;
}

/*
bool
ms::ptsrc(const std::vector<int>& fldid, const std::vector<int>& spwid)
{

  *itsLog << LogOrigin("ms", "ptsrc");
  *itsLog << "not implemented"<<LogIO::POST;
  Table::relinquishAutoLocks(True);
  return False;
}
*/

bool
ms::done()
{
  *itsLog << LogOrigin("ms", "done");
  Table::relinquishAutoLocks(True);
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
       Table::relinquishAutoLocks(True);
       RETHROW(x);
  }
  Table::relinquishAutoLocks(True);
  return rstat;
}

::casac::record* 
ms::msseltoindex(const std::string& vis, const ::casac::variant& spw, 
		 const ::casac::variant& field, 
		 const ::casac::variant& baseline, 
		 const ::casac::variant& time, 
		 const ::casac::variant& scan, const ::casac::variant& uvrange,
                 const ::casac::variant& observation,
		 const ::casac::variant& polarization,
		 const std::string& taql){
  casac::record* rstat(0);
  try {

    
    MeasurementSet thisms(vis);
    Record selected=thisms.msseltoindex(toCasaString(spw), toCasaString(field),
					toCasaString(baseline), 
					toCasaString(time),toCasaString(scan),
					toCasaString(uvrange),
                                        toCasaString(observation),
					toCasaString(polarization),
					String(taql));
    rstat = fromRecord(selected); 
					

  }catch (AipsError x){
       Table::relinquishAutoLocks(True);
    RETHROW(x);
  }
  
  Table::relinquishAutoLocks(True);
  return rstat;

}

bool
ms::hanningsmooth(const std::string& datacolumn)
{
  Bool rstat(False);
  try {
     *itsLog << LogOrigin("ms", "hanningsmooth");
     if(!ready2write_()){
       *itsLog << LogIO::SEVERE
	       << "Please open ms with parameter nomodify=false so "
	       << "smoothed channel data can be stored."
	       << LogIO::POST;
       return rstat;
     }

     Block<int> noSort;
     Matrix<Int> allChannels;
     Double intrvl = 0;
     String dcol(datacolumn);
     dcol.downcase();
     Bool addScratch = !(dcol=="data") && !(dcol=="all"); // don't add scratch columns if they don't exist already
                                                          // and the requested output column is == "data" or "all"
     VisSet vs(*itsMS, noSort, allChannels, addScratch, intrvl, False);
     if(dcol=="all"){
       MSMainColumns mainCols(*itsMS);
       if(!mainCols.correctedData().isNull()){ // there are scratch columns
	 *itsLog << LogIO::NORMAL << "Smoothing MS Main Table column CORRECTED_DATA ... " << LogIO::POST;
	 VisSetUtil::HanningSmooth(vs, "corrected", False); // False, i.e. don't change flags and weights here, will be done below
       }
       *itsLog << LogIO::NORMAL << "Smoothing MS Main Table column DATA ... " << LogIO::POST;
       VisSetUtil::HanningSmooth(vs, "data", True);
     }
     else{
       *itsLog << LogIO::NORMAL << "Smoothing MS Main Table column \'" 
	       << datacolumn << "\'" << LogIO::POST;
       VisSetUtil::HanningSmooth(vs, datacolumn, True);
     }

     rstat = True;
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
       Table::relinquishAutoLocks(True);
    RETHROW(x);
  }
  Table::relinquishAutoLocks(True);
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

     // Ensure CORRECTED_DATA exists and is initialized
     //   (no-op if CORRECTED_DATA already present)
     VisSetUtil::addScrCols(*itsMS,False,True,True,False);

     // Open VisSet w/out triggering scr cols
     //  because CORRECTED_DATA has already been added above and
     //  MODEL_DATA is either present or automatic in VSU::UVSub
     Block<int> noSort;
     Matrix<Int> allChannels;
     Double intrvl = 0;

     VisSet vs(*itsMS, noSort, allChannels,False, intrvl, False,False);
     VisSetUtil::UVSub(vs, reverse);

     rstat = True;
  }
  catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    Table::relinquishAutoLocks(True);
    RETHROW(x);
  }
  Table::relinquishAutoLocks(True);
  return rstat;
}

//bool
table * 
ms::moments(const std::vector<int>& moments, 
            const ::casac::variant& antenna,
            const ::casac::variant& field,
            const ::casac::variant& spw,
            //const ::casac::variant& vmask,
            //const ::casac::variant& stokes,
            //const std::vector<std::string>& in_method,
            //const std::vector<int>& smoothaxes,
            //const ::casac::variant& smoothtypes,
            //const std::vector<double>& smoothwidths,
            const std::vector<double>& d_includepix,
            const std::vector<double>& d_excludepix,
            const double peaksnr, const double stddev,
            const std::string& velocityType, const std::string& out,
            //const std::string& smoothout, 
            //const std::string& pgdevice,
            //const int nx, const int ny, const bool yind,
            const bool overwrite, const bool //async
	    )
{
  table *rstat = 0 ;
  try {
     *itsLog << LogOrigin("ms", "moments");

     Vector<Int> whichmoments( moments ) ;

     String strAnt ;
     if ( antenna.type() == ::casac::variant::INT ) {
       strAnt = String::toString( antenna.toInt() ) ;
     }
     else if ( antenna.type() == ::casac::variant::STRING ) {
       strAnt = String( antenna.toString() ) ;
     }
     String strField ;
     if ( field.type() == ::casac::variant::INT ) {
       strField = String::toString( field.toInt() ) ;
     }
     else if ( field.type() == ::casac::variant::STRING ) {
       strField = String( field.toString() ) ;
     }
     String strSpw ;
     if ( spw.type() == ::casac::variant::INT ) {
       strSpw = String::toString( spw.toInt() ) ;
     }
     else if ( spw.type() == ::casac::variant::STRING ) {
       strSpw = String( spw.toString() ) ;
     }

//      String mask = vmask.toString() ;
//      if ( mask == "[]" ) 
//        mask = "" ;
     //String mask = "" ;
//     Vector<String> method = toVectorString( in_method ) ;
     Vector<String> method( 0 ) ;

     Vector<String> kernels ;
     kernels.resize( 0 ) ;
//      if ( smoothtypes.type() == ::casac::variant::BOOLVEC ) {
//        kernels.resize( 0 ) ; // unset
//      }
//      else if ( smoothtypes.type() == ::casac::variant::STRING ) {
//        sepCommaEmptyToVectorStrings( kernels, smoothtypes.toString() ) ;
//      }
//      else if ( smoothtypes.type() == ::casac::variant::STRINGVEC ) {
//        kernels = toVectorString( smoothtypes.toStringVec() ) ;
//      }
//      else {
//        *itsLog << LogIO::WARN << "Unrecognized smoothetypes datatype" 
//                << LogIO::POST ;
//      }

     int num = kernels.size() ;

     Vector< Quantum<Double> > kernelwidths( num ) ;
//      Unit u( "pix" ) ;
//      for ( int i = 0 ; i < num ; i++ ) {
//        kernelwidths[i] = casa::Quantity( smoothwidths[i], u ) ;
//      }

     Vector<Float> includepix ;
     num = d_includepix.size() ;
     if ( !(num == 1 && d_includepix[0] == -1) ) {
       includepix.resize( num ) ;
       for ( int i = 0 ; i < num ; i++ )
         includepix[i] = d_includepix[i] ;
     }

     Vector<Float> excludepix ;
     num = d_excludepix.size() ;
     if ( !(num == 1 && d_excludepix[0] == -1) ) {
       excludepix.resize( num ) ;
       for ( int i = 0 ; i < num ; i++ ) 
         excludepix[i] = d_excludepix[i] ;
     }

     // if MS doesn't have FLOAT_DATA column, throw exception
     if ( !(itsMS->isColumn( MSMainEnums::FLOAT_DATA )) ) {
       throw AipsError( "ms::moments() is only applicable for MS with FLOAT_DATA" ) ; 
       return rstat ;
     }

     MSAnalysis momentMaker( itsMS ) ;
     //Vector<Int> smoothAxes( smoothaxes ) ;
     Vector<Int> smoothAxes( 0 ) ;
     String smoothout = "" ;
     MeasurementSet *mMS = momentMaker.moments( whichmoments,
                                                //mask,
                                                strAnt,
                                                strField,
                                                strSpw,
                                                method,
                                                smoothAxes,
                                                kernels,
                                                kernelwidths,
                                                includepix,
                                                excludepix,
                                                peaksnr,
                                                stddev,
                                                velocityType,
                                                out,
                                                smoothout,
                                                //pgdevice,
                                                //nx, 
                                                //ny,
                                                //yind,
                                                overwrite ) ;

     if ( mMS != 0 ) {
       rstat = new table() ;
       rstat->open( mMS->tableName() ) ;
       delete mMS ;
       mMS = 0 ;
     }
  }
  catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    Table::relinquishAutoLocks(True);
    RETHROW(x);
  }
  Table::relinquishAutoLocks(True);

  if(!rstat)
     throw AipsError("ms.moments unable to create moments table");
  return rstat;
}

bool ms::msselect(const ::casac::record& exprs, const bool onlyparse)
{
  Bool retVal;
  try
    {
     *itsLog << LogOrigin("ms", "msselect");
      Record *casaRec = toRecord(exprs);
      String spwExpr, timeExpr, fieldExpr, baselineExpr, scanExpr, scanIntentExpr,
	polnExpr, uvDistExpr, obsExpr, arrayExpr, taQLExpr;
      Int nFields = casaRec->nfields();
      for (Int i=0; i<nFields; i++)
	{
	  if (casaRec->name(i) == "spw")           {spwExpr        = casaRec->asString(RecordFieldId(i));}
	  if (casaRec->name(i) == "time")          {timeExpr       = casaRec->asString(RecordFieldId(i));}
	  if (casaRec->name(i) == "field")         {fieldExpr      = casaRec->asString(RecordFieldId(i));}
	  if (casaRec->name(i) == "baseline")      {baselineExpr   = casaRec->asString(RecordFieldId(i));}
	  if (casaRec->name(i) == "scan")          {scanExpr       = casaRec->asString(RecordFieldId(i));}
	  if (casaRec->name(i) == "scanintent")    {scanIntentExpr = casaRec->asString(RecordFieldId(i));}
	  if (casaRec->name(i) == "polarization")  {polnExpr       = casaRec->asString(RecordFieldId(i));}
	  if (casaRec->name(i) == "uvdist")        {uvDistExpr     = casaRec->asString(RecordFieldId(i));}
	  if (casaRec->name(i) == "observation")   {obsExpr        = casaRec->asString(RecordFieldId(i));}
	  if (casaRec->name(i) == "array")         {arrayExpr      = casaRec->asString(RecordFieldId(i));}
	  if (casaRec->name(i) == "taql")          {taQLExpr      = casaRec->asString(RecordFieldId(i));}
	}
      // if (itsSelectedMS) delete itsSelectedMS;
      // itsSelectedMS = new MeasurementSet();
      
      //
      // If only parsing is requested, just set up the itsMSS object.
      // This is much faster if one is only interseted in the indices
      // and not the actual selected MS.
      //
      if (onlyparse)
	itsMSS->reset(*itsMS, MSSelection::PARSE_NOW,timeExpr,baselineExpr,fieldExpr,spwExpr,uvDistExpr,
		      taQLExpr,polnExpr,scanExpr,arrayExpr,scanIntentExpr,obsExpr);
      else
	retVal = mssSetData(*itsMS, *itsMS, "",/*outMSName*/
			    timeExpr, baselineExpr, fieldExpr, spwExpr, uvDistExpr,
			    taQLExpr, polnExpr, scanExpr,
			    arrayExpr, scanIntentExpr, obsExpr, itsMSS);
      itsSel->setMS(*itsMS);
      return retVal;
    }
  catch (AipsError x)
    {
      RETHROW(x);
    }
  Table::relinquishAutoLocks(True);
  return retVal;
}

::casac::record*
ms::msselectedindices()
{
  casac::record *selectedIndices(0);
  try
    {
     *itsLog << LogOrigin("ms", "msselectedindices");
      Record tmp =  mssSelectedIndices(*itsMSS, itsMS);
      selectedIndices = fromRecord(tmp);
    }
  catch (AipsError x)
    {
      Table::relinquishAutoLocks(True);
      RETHROW(x);
    }
  return selectedIndices;
}

bool
ms::addephemeris(const int id,
		 const std::string& ephemerisname,  
		 const std::string& comment,
		 const ::casac::variant& field)
{
  Bool rstat(False);
  try {
    *itsLog << LogOrigin("ms", "addephemeris");

    String t_field(m1toBlankCStr_(field));
    String t_name     = toCasaString(ephemerisname);
    String t_comment = toCasaString(comment);
    Record selrec;
    Vector<Int> fieldids;

    if(detached()){
      return False;
    }

    if(t_field.size()>0){
      try {
	selrec=itsMS->msseltoindex("*", t_field);
      }
      catch (AipsError x) {
	*itsLog << LogOrigin("ms", "addephemeris") 
		<< LogIO::SEVERE << x.getMesg() << LogIO::POST;
	RETHROW(x);
      }    
      fieldids=selrec.asArrayInt("field");
    }

    Double startTime;

    Array<Double> timeRanges = MSObservationColumns(itsMS->observation()).timeRange().getColumn();

    try{
      MeasComet mc(t_name);
      startTime = casa::Quantity(mc.getStart(),"d").getValue("s");
      Double endTime = casa::Quantity(mc.getEnd(),"d").getValue("s");
      if(startTime>min(timeRanges)){
	*itsLog << LogOrigin("ms", "addephemeris") 
		<< LogIO::WARN << "Ephemeris validity time range starts after start of observation." << LogIO::POST;
      }
      if(endTime<max(timeRanges)){
	*itsLog << LogOrigin("ms", "addephemeris") 
		<< LogIO::WARN << "Ephemeris validity time range ends before end of observation." << LogIO::POST;
      }
    }
    catch (AipsError x){
      *itsLog << LogOrigin("ms", "addephemeris") 
	      << LogIO::SEVERE << "Error reading input ephemeris table. No changes made to MS." << endl
	      << x.getMesg() << LogIO::POST;
      RETHROW(x);

    }

    uInt theId=0;
    if(id>=0){
      theId = id;
    }
    else{ // if the given id is invalid, determine the next free id
      String ephIDName = MSField::columnName(MSField::EPHEMERIS_ID);
      if(itsMS->field().actualTableDesc().isColumn(ephIDName)){
	ScalarColumn<Int> ephid(itsMS->field(), ephIDName);
	Vector<Int> ids = ephid.getColumn();
	for(uInt i=0; i<ids.size(); i++){
	  for(uInt j=0; j<fieldids.size(); j++){
	    if((Int)i==fieldids[j] && ids[i]>=0){ // these are the ids to be overwritten
	      ids[i] = -1; // exclude them from the search
	    }
	  }
	}
	theId = max(ids)+1; 
      }
    }

    if(!itsMS->field().addEphemeris(theId, t_name, t_comment)){
      *itsLog << LogOrigin("ms", "addephemeris") 
	      << LogIO::SEVERE << "Error adding ephemeris to MS." << LogIO::POST;
      return False;
    }

    MSFieldColumns msfc(itsMS->field());

    for(uInt i=0; i<fieldids.size(); i++){
      Double presentStartTime = msfc.time()(fieldids(i));
      if(presentStartTime<min(timeRanges) || presentStartTime>max(timeRanges)){ 
	// present start time is inconsistent with values of observation table
	*itsLog << LogOrigin("ms", "addephemeris") 
		<< LogIO::WARN << "The TIME column entry for field " << fieldids(i) 
		<< "is outside the observation time range given by the OBSERVATION table." << LogIO::POST;
	if(min(timeRanges)<=startTime || startTime<=max(timeRanges)){
	  // start time of ephemeris is OK, use it as new time column entry
	  msfc.time().put(fieldids(i), startTime);
	  *itsLog << LogIO::WARN << "   Will replace it by the start time of the added ephemeris." << LogIO::POST;
	}
      }
      msfc.ephemerisId().put(fieldids(i), theId);
    }
   
    {// Update HISTORY table of newly created MS
      String message= "Added ephemeris to FIELD table.";
      ostringstream param;
      param << "field=" << t_field << " id=" << id
	    << " name='" << t_name << "' coment='" << comment << "'";
      String paramstr=param.str();
      writehistory(message, paramstr, "ms::addephemeris()", itsMS->tableName(), "ms");
    }

    rstat = True;
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }

  return rstat;
}

void
ms::addephemcol(const casa::MeasurementSet& appendedMS)
{
  if(!itsMS->field().actualTableDesc().isColumn(MSField::columnName(MSField::EPHEMERIS_ID))){
    // if not, test if the other MS uses ephem objects
    Bool usesEphems = False;
    const ROMSFieldColumns otherFldCol(appendedMS.field());
    for(uInt i=0; i<otherFldCol.nrow(); i++){
      if(!otherFldCol.ephemPath(i).empty()){
	usesEphems = True;
	break;
      }
    }
    if(usesEphems){ // if yes, the ephID column needs to be added to this MS FIELD table
      String thisMSName = Path(itsMS->tableName()).absoluteName(); 
      *itsLog << LogIO::NORMAL << "Adding the EPHEMERIS_ID column to the FIELD table of first MS. " 
	      << LogIO::POST;
      if(!itsMS->field().addEphemeris(0,"","")){
	*itsLog << "Cannot add the EPHEMERIS_ID column to the FIELD table of MS " << thisMSName
		<< LogIO::EXCEPTION;
      }
      // reopen this MS
      close();
      open(thisMSName, False, False);
    }
  }
}
//
// New iteration control interface.  Uses the VI for iterations and
// getting the requested data out.
//
bool
ms::niterinit(const std::vector<std::string>& columns, const double interval,
             const int maxrows, const bool adddefaultsortcolumns)
{
   Bool rstat(False);
   Block<Int> sort(1);
   sort[0]=MS::TIME;
   try
     {
       if (itsVI == NULL)
	 itsVI = new VisibilityIterator(*itsMS, sort, adddefaultsortcolumns, interval);
       else
	 *itsVI = VisibilityIterator(*itsMS, sort, adddefaultsortcolumns, interval);
       if (interval <= 0) itsVI->setRowBlocking(itsMS->nrow());
       if (maxrows > 0) itsVI->setRowBlocking(maxrows);
       //       *itsVB = VisBuffer(*itsVI);
       rstat=True;
     }
   catch (AipsError x)
     {
       RETHROW(x);
     }
  
   niterorigin();
   doingIterations_p=True;
   return rstat;
}

bool
ms::niterorigin()
{
   Bool rstat(False);
   if (!detached())
     {
       try
	 {
	   if (itsVI) 
	     {
	       itsVI->originChunks();
	       rstat=True;
	     }
	   else
	     *itsLog << "ms::niterorigin:  Please call niterinit() first." << LogIO::EXCEPTION;
	 }
       catch (AipsError x)
	 {
	   RETHROW(x);
	 }
     }
   return rstat;
}

bool
ms::niterend()
{
  Bool rstat(False);
  if (!detached())
    {
      try
	{
	  rstat = !itsVI->moreChunks();
	}
      catch (AipsError x)
	{
	  RETHROW(x);
	}
    }   
  return rstat;
}

bool
ms::niternext()
{
  Bool rstat(False);
  if (!detached())
    {
      try
	{
	  if (!niterend())
	    {
	      itsVI->nextChunk();
	      rstat=True;
	    }
	}
      catch (AipsError x)
	{
	  RETHROW(x);
	}
    }
  
  return rstat;
}

::casac::record*
ms::ngetdata(const std::vector<std::string>& items, const bool ifraxis, const int ifraxisgap, const int increment, const bool average)
{
  try
    {
      if (itsVI == NULL) 
	niterinit(items,0.0,0,False); 
      // if (doingIterations_p == False) 
      // 	niterorigin();

      casa::Record rec;
      Int nItems = items.size();
      for (Int i=0; i<nItems; i++) 
	{
	  String item(items[i]);
	  item.downcase();
	  MSS::Field fld=MSS::field(item);
	  switch (fld) 
	    {
	    case MSS::DATA:
	      {
		Cube<Complex> vis;
		itsVI->visibility(vis,VisibilityIterator::Observed);
		rec.define(item,vis);
		break;
	      }
	    case MSS::MODEL_DATA:
	      {
		Cube<Complex> vis;
		itsVI->visibility(vis,VisibilityIterator::Model);
		rec.define(item,vis);
		break;
	      }
	    case MSS::CORRECTED_DATA:
	      {
		Cube<Complex> vis;
		itsVI->visibility(vis,VisibilityIterator::Corrected);
		rec.define(item,vis);
		break;
	      }
	    case MSS::ANTENNA1:
	      {
		Vector<Int> a1;
		a1 = itsVI->antenna1(a1);
		rec.define(item,a1);
		break;
	      }
	    case MSS::ANTENNA2:
	      {
		Vector<Int> a;
		a = itsVI->antenna1(a);
		rec.define(item,a);
		break;
	      }
	    case MSS::FLAG:
	      {
		Cube<Bool> flag;
		flag = itsVI->flag(flag);
		rec.define(item,flag);
		break;
	      }
	    case MSS::TIME:
	      {
		Vector<Double> time;
		time = itsVI->time(time);
		rec.define(item,time);
		break;
	      }
	    case MSS::ROWS:
	      {
		Vector<uInt> rowIds;
		rowIds = itsVI->rowIds(rowIds);
		Vector<Int> tmp(rowIds.shape());
		for (Int ii=0;ii<(Int)tmp.nelements(); ii++)
		  tmp(ii)=rowIds(ii);
		rec.define(item,tmp);
		break;
	      }
	    case MSS::WEIGHT:
	      {
		Vector<Float> weight;
		weight = itsVI->weight(weight);
		rec.define(item,weight);
		break;
	      }
	    // case MSS::UVW:
	    //   {
	    // 	Vector<RigidVector<double, 3> > uvw;
	    // 	uvw = itsVI->uvw(uvw);
	    // 	rec.define(item,uvw);
	    // 	break;
	    //   }
	      
	    default:
	      {
		*itsLog  << "ngetdata: Unsupported item requrested (" << items[i] << ")" <<LogIO::EXCEPTION;
	      }
	    }
	}
      return fromRecord(rec);
    }
  catch (AipsError x)
    {
      RETHROW(x);
    }
}


} // casac namespace

