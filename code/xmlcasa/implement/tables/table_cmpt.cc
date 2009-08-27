
/***
 * Framework independent implementation file for table...
 *
 * Implement the table component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 Note to self what happens when we open another table if one is already opened?
 ***/

#include <iostream>
#include <xmlcasa/tables/table_cmpt.h>
#include <casa/aips.h>
#include <tables/Tables/IncrementalStMan.h>
#include <tables/Tables/MemoryStMan.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableProxy.h>
#include <tables/Tables/TableColumn.h>
#include <tables/Tables/TableParse.h>
#include <tables/Tables/TableLock.h>
#include <fits/FITS/FITSTable.h>
#include <fits/FITS/SDFITSTable.h>
#include <casa/Inputs/Input.h>
#include <casa/Containers/Record.h>
#include <casa/Containers/ValueHolder.h>
#include <casa/Exceptions/Error.h>
#include <casa/Logging/LogIO.h>
#include <casa/OS/File.h>
#include <xmlcasa/utils/stdBaseInterface.h>
//begin modification
//july 4 2007
#include <xmlcasa/xerces/asdmCasaXMLUtil.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/TableIter.h>
#include <tables/Tables/TableRow.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/StandardStMan.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <casa/Utilities/Regex.h>


using namespace std;
using namespace casa;
//begin modification
//july 2 2007
using namespace casac;
//end modification

namespace casac {

table::table()
{
   itsTable = 0;
   itsLog = new casa::LogIO;
}

table::table(casa::TableProxy *theTable)
{
   //itsTable = new TableProxy(*theTable);
   itsTable = theTable;
   itsLog = new casa::LogIO;
}

table::~table()
{
  delete itsLog;
  if(itsTable)
     delete itsTable;
}

bool
table::open(const std::string& tablename, const ::casac::record& lockoptions, const bool nomodify)
{
 Bool rstat(False);
 try {
	 Record *tlock = toRecord(lockoptions);
	 //TableLock *itsLock = getLockOptions(tlock);
	 if(nomodify){
	     if(itsTable)close();
             itsTable = new casa::TableProxy(String(tablename),*tlock,Table::Old);
	 } else {
	     if(itsTable)close();
             itsTable = new casa::TableProxy(String(tablename),*tlock,Table::Update);
	 }
	 delete tlock;
     rstat = True;
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
table::flush()
{
 Bool rstat(False);
 try {
	 if(itsTable){
		 itsTable->flush( true );
		 rstat = True;
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
table::resync()
{

 Bool rstat(False);
 try {
	 if(itsTable){
		 itsTable->resync();
		 rstat = True;
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
table::close()
{

 Bool rstat(False);
 try {
    delete itsTable;
    itsTable = 0;
    rstat = True;
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}
casac::table* 
table::fromfits(const std::string& tablename, const std::string& fitsfile, const int whichhdu, const std::string& storage, const std::string& convention, const bool nomodify, const bool ack){

  casac::table *rstat(0);
  try {

    String inputFilename =String(fitsfile);
    String outputFilename = String(tablename);
    String storageManagerType = String(storage);
    Int whichHDU = whichhdu;
    String conv=String(convention);
    conv.downcase();
    
    Bool sdfits = conv.contains("sdfi");

    storageManagerType.downcase();

    Bool useIncrSM;
    if (storageManagerType == String("incremental")) {
      useIncrSM = True;
    } else 	if ((storageManagerType == String("standard"))||(storageManagerType == String("memory"))) {
      useIncrSM = False;


    } else {
      throw(AipsError(storageManagerType+String("  is not a valid storage manager")));  
    }

    if (whichHDU < 1) {
      throw(AipsError("whichHDU is not valid, must be >= 1"));
    }

    File inputFile(inputFilename);
    if (! inputFile.isReadable()) {
      
      throw(AipsError(inputFilename+String(" is not readable")));
    }

    // construct the FITS table of the appropriate type
    FITSTable *infits = 0;
    if (sdfits) {
      infits = new SDFITSTable(inputFilename, whichHDU);
    } else {
      infits = new FITSTable(inputFilename, whichHDU);
    }
    AlwaysAssert(infits, AipsError);
    if (!infits->isValid()) {
      throw(AipsError("The indicated FITS file does not have a valid binary table at HDU requested")); 
    }

    TableDesc td(FITSTabular::tableDesc(*infits));
    // if sdfits, remove any TDIM columns from td, FITSTable takes care of interpreting them
    // and if sdfits is true, that most likely means we don't want to see them
    
    if (sdfits) {
      Vector<String> cols(td.columnNames());
      for (uInt i=0;i<cols.nelements();i++) {
	if (cols(i).matches(Regex("^TDIM.*"))) {
	  td.removeColumn(cols(i));
	}
      }
    }
	    
    SetupNewTable newtab(outputFilename, td, Table::NewNoReplace);
    if (useIncrSM) {
      IncrementalStMan stman("ISM");
      newtab.bindAll(stman);
    }
    else if(storageManagerType == String("memory")){
      MemoryStMan stman("MemSt");
      newtab.bindAll(stman);
    }
    Table tab(newtab, TableLock::AutoNoReadLocking, infits->nrow());
    TableRow row(tab);
    uInt rownr = 0;
    
    while (rownr < tab.nrow()) {
      row.putMatchingFields(rownr, TableRecord(infits->currentRow()));
      infits->next();
      rownr++;
    }
    
    tab.flush();
    TableProxy *tb = new casa::TableProxy(tab);
    rstat = new casac::table(tb);
    cout << "done." << endl;


    
    
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;

}


casac::table*
table::copy(const std::string& newtablename, const bool deep, const bool valuecopy, const ::casac::record& dminfo, const std::string& endian, const bool memorytable, const bool returnobject)
{
 casac::table *rstat(0);
 try {
	 if(itsTable){
		 Record *tdminfo = toRecord(dminfo);
		 TableProxy *mycopy = new TableProxy;
		 *mycopy = itsTable->copy(newtablename, memorytable, deep, valuecopy, endian, *tdminfo, false);
		 delete tdminfo;
		 rstat = new casac::table(mycopy);
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST; }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
table::copyrows(const std::string& outtable, const int startrowin, const int startrowout, const int nrow)
{
 Bool rstat(False);
 try {
	 if(itsTable){
		Record lockOpts = itsTable->lockOptions();
		lockOpts.define("option", "auto");
                TableProxy theOutTab(outtable, lockOpts, Table::Update);
		itsTable->copyRows(theOutTab, startrowin, startrowout, nrow); 
		rstat = True;
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
table::done()
{
 Bool rstat(False);
 try {
    rstat = close();
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
table::iswritable()
{
 Bool rstat(False);
 try {
	 if(itsTable){
		 rstat = itsTable->isWritable();
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

std::string
table::endianformat()
{
 std::string rstat("little");
 try {
    if(itsTable){
	 rstat = itsTable->endianFormat();
     } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
     }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
table::lock(const bool write, const int nattempts)
{

 Bool rstat(False);
 try {
	 if(itsTable){
		 itsTable->lock(write, nattempts);
		 rstat = true;
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
table::unlock()
{

 Bool rstat(False);
 try {
	 if(itsTable){
		 itsTable->unlock();
		 rstat = true;
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
table::datachanged()
{
 Bool rstat(False);
 try {
	 if(itsTable){
		rstat = itsTable->hasDataChanged(); 
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
table::haslock(const bool write)
{
 Bool rstat(False);
 try {
	 if(itsTable){
		 rstat = itsTable->hasLock(write);
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

::casac::record*
table::lockoptions()
{
 ::casac::record *rstat(0);
 try {
	 if(itsTable){
		rstat = fromRecord(itsTable->lockOptions());
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
table::ismultiused(const bool checksubtables)
{
 Bool rstat(False);
 try {
	 if(itsTable){
		 rstat = itsTable->isMultiUsed(checksubtables);
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
table::browse()
{
 Bool rstat(False);
 try {
         pid_t pID = vfork();
         if(!pID){
		 // Because the browser is a stand-alone program we need to do fork and exec
		 // Here we spawn the browser in a seperate process
		 // If no table has been open we just open the browser
		 // otherwise we tell the browser to look at the current table
	   *itsLog << LogIO::NORMAL << "Spawning table browser one moment..." << LogIO::POST;
           if(itsTable){
	       String myName = itsTable->table().tableName();
               execlp("casabrowser", "casabrowser", myName.c_str(), (char *)0);
	   } else {
               execlp("casabrowser", "casabrowser", (char *)0);
	   }
	   // If we get here something bad has happened and the exec has faild
           *itsLog << LogIO::SEVERE << "Bad news, unable to start the table browser." << LogIO::POST;
           exit(-1);
         } else if (pID < 0) {
             throw AipsError("Table.browse fork failed. Unable to render table.");
         } else {
	    rstat = True;
            *itsLog << LogIO::NORMAL << "Table rendered" << LogIO::POST;
         }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

std::string
table::name()
{
   std::string myName("");
   if(itsTable){
      myName = itsTable->table().tableName();
   } else {
      *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
    }
    return myName;
}

bool
table::toasciifmt(const std::string& asciifile, const std::string& headerfile, const std::vector<std::string>& columns, const std::string& sep)
{
    Bool rstat(False);
    try {
	if(!itsTable){
	    *itsLog << LogIO::WARN << "toasciifmt: No table specified, please open first" << LogIO::POST;
	}
	else if(asciifile.empty()){
	    *itsLog << LogIO::WARN << "toasciifmt: No output file specified" << LogIO::POST;
	}
	else {
	    String message;
	    Vector<Int> precision; // optional vector describing the output precision for each column in "columns"
	                           // - leave empty for now to use default precision
	    Bool useBrackets(True); // use bracket format for array output by default
	    message = itsTable->toAscii(String(asciifile), String(headerfile), toVectorString(columns), 
					String(sep), precision, useBrackets);
	    if(message.size() > 0){
		*itsLog << LogIO::WARN << "toasciifmt: " << message << LogIO::POST;
	    }
	    else {
	      rstat = True;
	    }
		
	}	    
    } catch (AipsError x) {
	*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	RETHROW(x);
    }
    return rstat;

}

::casac::table*
table::queryC(const std::string& query, const std::string& resultTable, const std::string& sortlist, const std::string& columns)
{
 ::casac::table *rstat(0);
 try {
	 if(itsTable){
		 std::ostringstream taqlString;
                 taqlString <<  "select ";
	         if(!columns.empty())
	            taqlString << columns;
	         taqlString << "from " << this->name() << " where ";
	         taqlString <<   query;
	         if(!sortlist.empty())
	            taqlString << " orderby " << sortlist;
	         if(!resultTable.empty())
	            taqlString << " giving " << resultTable;
                 casa::TableProxy *theQTab = new TableProxy(tableCommand(taqlString.str()));
                 rstat = new ::casac::table(theQTab);
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}
::casac::table*
table::query(const std::string& query, const std::string& name, const std::string& sortlist, const std::string& columns)
{
 ::casac::table *rstat(0);
 try {
	 if(itsTable){
		 std::ostringstream taqlString;
		 taqlString <<  "select from " << this->name() << " where ";
		 taqlString <<   query;
		 casa::TableProxy *theQTab = new TableProxy(tableCommand(taqlString.str()));
                 rstat = new ::casac::table(theQTab);
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

::casac::variant*
table::calc(const std::string& expr)
{
 ::casac::variant *rstat(0);
 try {
	 if(itsTable){
		 std::ostringstream calcString;
		 calcString <<  "calc from " << itsTable->table().tableName() << " calc ";
		 calcString <<   expr;
		 //casa::Table *theQTab = new Table(tableCommand(taqlString.str()));
		 *itsLog << LogIO::WARN << "Calc not implemented!" << LogIO::POST;
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
	      // TODO : IMPLEMENT ME HERE !
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

::casac::table*
table::selectrows(const std::vector<int>& rownrs, const std::string& name)
{
 ::casac::table *rstat(0);
 try {
	 if(itsTable){
		 rstat = new casac::table(new TableProxy(itsTable->selectRows(rownrs, String(name))));
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
table::putinfo(const ::casac::record& value)
{
 Bool rstat(False);
 try {
	 if(itsTable){
		 Record *tvalue = toRecord(value);
		 itsTable->putTableInfo(*tvalue);
		 delete tvalue;
		 rstat = True;
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
table::addreadmeline(const std::string& value)
{
 Bool rstat(False);
 try {
	 if(itsTable){
		 itsTable->addReadmeLine(value);
		 rstat = True;
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
table::summary(const bool recurse)
{
 Bool rstat(False);
 try {
   if(itsTable){
     *itsLog << LogOrigin("summary",name()) << LogIO::NORMAL
             << "Table summary: " << name() << LogIO::POST;
     *itsLog << LogOrigin("summary",name()) << LogIO::NORMAL
             << "Shape: " << ncols() << " columns by " << nrows() << LogIO::POST;
     Record tabinfo = itsTable->tableInfo();
     ostringstream tabinfo_string;
     tabinfo_string << tabinfo;
     *itsLog << LogOrigin("summary",name()) << LogIO::NORMAL
             << "Info: " << tabinfo_string.str() << LogIO::POST;
     Record tabkeys = itsTable->getKeywordSet(String());
     if(tabkeys.nfields() > 0){
       ostringstream keys_string;
       keys_string << tabkeys;
       *itsLog << LogOrigin("summary", name()) 
               << LogIO::NORMAL 
               << "Table keywords: " << keys_string.str() 
               << LogIO::POST;
       if(recurse){
         for(unsigned int i = 0; i < tabkeys.nfields(); ++i){
           switch(tabkeys.type(i)){
           case TpTable :
             *itsLog << LogOrigin("summary",name())
                     << LogIO::WARN
                     << "No recursion just yet " << LogIO::POST;
             break;
           case TpString :
             {
               String theString = tabkeys.asString(i);
               if(theString.contains("Table:")){
                 table *subtab = new table;
                 record dummy;
                 subtab->open(string(theString.from((size_t)7).chars()),
                              dummy);
                 subtab->summary(false);
                 subtab->close();
                 delete subtab;
               }
             }
             break;
           default :
             break;
           }
         }
       }
     }
     vector<string> cols = colnames();
     if(cols.size() > 0){
       *itsLog << LogOrigin("summary",name()) << LogIO::NORMAL << "Columns: ";
       for(unsigned int i = 0; i < cols.size(); ++i)
         *itsLog  << cols[i] << " ";
       *itsLog << LogIO::POST;
       for(unsigned int i = 0; i < cols.size(); ++i){
         Record colkeys = itsTable->getKeywordSet(cols[i]);
         ostringstream outs;
         if(colkeys.nfields() > 0)
           outs << cols[i] << " keywords: " << colkeys; 
         else
           outs << cols[i] << " keywords: None";
         *itsLog << LogOrigin("summary",name()) << LogIO::NORMAL
                 << outs.str() << LogIO::POST;
       }
     }
     rstat = True;
   } else {
     *itsLog << LogOrigin("table","summary") << LogIO::WARN
             << "No table specified, please open first" << LogIO::POST;
   }
   // TODO : IMPLEMENT ME HERE !
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "
            << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

std::vector<std::string>
table::colnames()
{
 std::vector<std::string> rstat(0);
 try {
	 if(itsTable){
            Vector<String> colNames = itsTable->columnNames();
	    rstat = fromVectorString(colNames);
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

std::vector<int>
table::rownumbers(const ::casac::record& tab, const int nbytes)
{
 std::vector<int> rstat(0);
 try {
	 if(itsTable){
		 TableProxy dummy;
		 itsTable->rowNumbers(dummy).tovector(rstat);
		 // *itsLog << LogIO::WARN << "rownumbers not implemented" << LogIO::POST;
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
	      // TODO : IMPLEMENT ME HERE !
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
table::setmaxcachesize(const std::string& columnname, const int nbytes)
{
 Bool rstat(False);
 try {
	 if(itsTable){
	    if(columnname.size()){
		 if(nbytes > 0){
		    itsTable->setMaximumCacheSize(columnname, nbytes);
		     rstat = True;
		 } else {
		    *itsLog << LogIO::WARN << "Need to specify cache size greater than 0" << LogIO::POST;
		 }
	     } else {
		 *itsLog << LogIO::WARN << "Need to specify a column name" << LogIO::POST;
             }
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
table::isscalarcol(const std::string& columnname)
{
 Bool rstat(False);
 try {
	 if(itsTable){
		 rstat = itsTable->isScalarColumn(columnname);
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
table::isvarcol(const std::string& columnname)
{
 Bool rstat(False);
 try {
	 if(itsTable){
		 Record myDesc = itsTable->getColumnDescription(columnname, True);
		 rstat = (myDesc.isDefined("ndim") && !myDesc.isDefined("shape"));
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

std::string
table::coldatatype(const std::string& columnname)
{
    std::string myDataType("");
    if(itsTable){
	    myDataType = itsTable->columnDataType(columnname);
    } else {
	 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
    }
    return myDataType;
}

std::string
table::colarraytype(const std::string& columnname)
{
   std::string myArrayTypeName("");
   if(itsTable){
	   myArrayTypeName = itsTable->columnArrayType(columnname);
   } else {
      *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
   }
   return myArrayTypeName;
}

int
table::ncols()
{
 int rstat(0);
 try {
	 if(itsTable){
	    Vector<Int> myshape = itsTable->shape();
	    rstat = myshape[0];
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

int
table::nrows()
{
 Int rstat(0);
 try {
	 if(itsTable){
	    Vector<Int> myshape = itsTable->shape();
	    rstat = myshape[1];
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
table::addrows(const int nrow)
{
 Bool rstat(False);
 try {
	 if(itsTable){
		 itsTable->addRow(nrow);
		 rstat = True;
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
table::removerows(const std::vector<int>& rownrs)
{
 Bool rstat(False);
 try {
	 if(itsTable){
		 itsTable->removeRow(rownrs);
		 rstat = True;
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
table::addcols(const ::casac::record& desc, const ::casac::record& dminfo)
{
 Bool rstat(False);
 try {
	 if(itsTable){
		 Record *tdesc = toRecord(desc);
		 Record *tdminfo = toRecord(dminfo);
		 itsTable->addColumns(*tdesc, *tdminfo);
		 delete tdesc;
		 delete tdminfo;
		 rstat = True;
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
table::renamecol(const std::string& oldname, const std::string& newname)
{
 Bool rstat(False);
 try {
	 if(itsTable){
		 itsTable->renameColumn(oldname, newname);
		 rstat = True;
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
table::removecols(const std::vector<std::string>& columnames)
{
 Bool rstat(False);
 try {
	 if(itsTable){
		 itsTable->removeColumns(toVectorString(columnames));
		 rstat = True;
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
table::iscelldefined(const std::string& columnname, const int rownr)
{
 Bool rstat(False);
 try {
	 if(itsTable){
		 ROTableColumn tabColumn(itsTable->table(), columnname);
		 rstat = tabColumn.isDefined(rownr);
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

::casac::variant*
table::getcell(const std::string& columnname, const int rownr)
{
 ::casac::variant *rstat(0);
 try {
	 if(itsTable){
		 ValueHolder theVal = itsTable->getCell(columnname, rownr);
		 rstat = fromValueHolder(theVal);
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

::casac::variant*
table::getcellslice(const std::string& columnname, const int rownr, const std::vector<int>& blc, const std::vector<int>& trc, const std::vector<int>& incr)
{
 ::casac::variant *rstat(0);
 try {
	 if(itsTable){
		 ValueHolder theVal = itsTable->getCellSlice(columnname, rownr, blc, trc, incr);
		 rstat = fromValueHolder(theVal);
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

::casac::variant*
table::getcol(const std::string& columnname, const int startrow, const int nrow, const int rowincr)
{
 ::casac::variant *rstat(0);
 try {
	 if(itsTable){
                 // ValueHolder theVal = itsTable->getColumn(columnname, startrow, nrow, rowincr);
		 rstat = fromValueHolder(itsTable->getColumn(columnname, startrow, nrow, rowincr));
                 // rstat = fromValueHolder(theVal);
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

::casac::record*
table::getvarcol(const std::string& columnname, const int startrow, const int nrow, const int rowincr)
{
 ::casac::record *rstat(0);
 try {
	 if(itsTable){
		 Record theVal = itsTable->getVarColumn(columnname, startrow, nrow, rowincr);
		 rstat = fromRecord(theVal);
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

::casac::variant*
table::getcolslice(const std::string& columnname, const std::vector<int>& blc, const std::vector<int>& trc, const std::vector<int>& incr, const int startrow, const int nrow, const int rowincr)
{
 ::casac::variant *rstat(0);
 try {
	 if(itsTable){
		 ValueHolder theVal = itsTable->getColumnSlice(columnname, startrow, nrow, rowincr, blc,
				                               trc, incr);
		 rstat = fromValueHolder(theVal);
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
table::putcell(const std::string& columnname, const std::vector<int>& rownr, const ::casac::variant& thevalue)
{
 Bool rstat(False);
 try {
	 if(itsTable){
		 ValueHolder *aval = toValueHolder(thevalue);
		 itsTable->putCell(columnname, rownr, *aval);
		 delete aval;
		 return True;
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
table::putcellslice(const std::string& columnname, const int rownr, const ::casac::variant& value, const std::vector<int>& blc, const std::vector<int>& trc, const std::vector<int>& incr)
{
 Bool rstat(False);
 try {
	 if(itsTable){
		 ValueHolder *aval = toValueHolder(value);
		 itsTable->putCellSlice(columnname, rownr, blc, trc, incr, *aval);
		 delete aval;
		 return True;
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
table::putcol(const std::string& columnname, const ::casac::variant& value, const int startrow, const int nrow, const int rowincr)
{
 Bool rstat(False);
 try {
	 if(itsTable){
		 ValueHolder *aval = toValueHolder(value);
		 itsTable->putColumn(String(columnname), startrow, nrow, rowincr, *aval);
		 delete aval;
		 rstat = True;
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
table::putvarcol(const std::string& columnname, const ::casac::record& value, const int startrow, const int nrow, const int rowincr)
{
 Bool rstat(False);
 try {
	 if(itsTable){
		 Record *aval = toRecord(value);
		 itsTable->putVarColumn(String(columnname), startrow, nrow, rowincr, *aval);
		 delete aval;
		 rstat = True;
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
table::putcolslice(const std::string& columnname, const ::casac::variant& value, const std::vector<int>& blc, const std::vector<int>& trc, const std::vector<int>& incr, const int startrow, const int nrow, const int rowincr)
{
 Bool rstat(False);
 try {
	 if(itsTable){
		 ValueHolder *aval = toValueHolder(value);
		 Vector<Int> iinc(incr);
		 if((iinc.nelements()==1) && (iinc[0]==1)){
		   iinc.resize(blc.size());
		   iinc.set(1);
		 }
		 itsTable->putColumnSlice(String(columnname), startrow, nrow, rowincr, blc, trc, iinc, *aval);
		 delete aval;
		 rstat = True;
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

std::vector<std::string>
table::getcolshapestring(const std::string& columnname, const int startrow, const int nrow, const int rowincr)
{
 std::vector<std::string> rstat(0);
 try {
	 if(itsTable){
		 Vector<String> dum = itsTable->getColumnShapeString(String(columnname), startrow, nrow,
				                                     rowincr);
		 rstat = fromVectorString(dum);;
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

::casac::variant*
table::getkeyword(const ::casac::variant& keyword)
{
 ::casac::variant *rstat(0);
 try {
	 if(itsTable){
		 ValueHolder theVal;
		 switch(keyword.type()){
			 case variant::STRING :
		                 theVal =  itsTable->getKeyword(String(), String(keyword.toString()), -1);
		                 rstat = fromValueHolder(theVal);
				 break;
			 case variant::INT :
		                 theVal =  itsTable->getKeyword(String(), String(), keyword.toInt());
		                 rstat = fromValueHolder(theVal);
				 break;
			 default :
		                 *itsLog << LogIO::WARN << "Keyword must be string or int" << LogIO::POST;
				 break;
		 }
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

::casac::record*
table::getkeywords()
{
 ::casac::record *rstat(0);
 try {
	 if(itsTable){
		 rstat = fromRecord(itsTable->getKeywordSet(String() ));
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

::casac::variant*
table::getcolkeyword(const std::string& columnname, const ::casac::variant& keyword)
{
 ::casac::variant *rstat(0);
 try {
	 if(itsTable){
            if(columnname.size()){
		 ValueHolder theVal;
		 switch(keyword.type()){
			 case variant::STRING :
		                 theVal =  itsTable->getKeyword(String(columnname), keyword.toString(), -1);
		                 rstat = fromValueHolder(theVal);
				 break;
			 case variant::INT :
		                 theVal =  itsTable->getKeyword(String(columnname), String(), keyword.toInt());
		                 rstat = fromValueHolder(theVal);
				 break;
			 default :
		                 *itsLog << LogIO::WARN << "Keyword must be string or int" << LogIO::POST;
				 break;
		 }
		 // rstat = fromValueHolder(theVal);
	    } else {
	      *itsLog << LogIO::WARN << "Need to specify a column name" << LogIO::POST;
	    }
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

::casac::variant*
table::getcolkeywords(const std::string& columnname)
{
 ::casac::variant *rstat(0);
 try {
	 if(itsTable){
		 rstat = fromValueHolder(ValueHolder(itsTable->getKeywordSet(String(columnname))));
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
table::putkeyword(const ::casac::variant& keyword, const ::casac::variant& value, const bool makesubrecord)
{
 Bool rstat(False);
 try {
	 if(itsTable){
		 ValueHolder *aval = toValueHolder(value);
		 switch(keyword.type()){
			 case variant::STRING :
		                 itsTable->putKeyword(String(), String(keyword.toString()), -1, makesubrecord, *aval);
		                 rstat = True;
				 break;
			 case variant::INT :
		                 itsTable->putKeyword(String(), String(), keyword.toInt(), makesubrecord, *aval);
		                 rstat = True;
				 break;
			 default :
		                 *itsLog << LogIO::WARN << "Keyword must be string or int" << LogIO::POST;
				 break;
		 }
		 delete aval;
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
table::putkeywords(const ::casac::record& value)
{
 Bool rstat(False);
 try {
	 if(itsTable){
		 Record *tvalue = toRecord(value);
		 itsTable->putKeywordSet(String(), *tvalue);
		 delete tvalue;
		 rstat = True;
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
table::putcolkeyword(const std::string& columnname, const ::casac::variant& keyword, const ::casac::variant& value)
{
 Bool rstat(False);
 try {
	 if(itsTable){
		 ValueHolder *aval = toValueHolder(value);
		 switch(keyword.type()){
			 case variant::STRING :
		                 itsTable->putKeyword(String(columnname), String(keyword.toString()), -1, True, *aval);
		                 rstat = True;
				 break;
			 case variant::INT :
		                 itsTable->putKeyword(String(columnname), String(), keyword.toInt(), True, *aval);
		                 rstat = True;
				 break;
			 default :
		                 *itsLog << LogIO::WARN << "Keyword must be string or int" << LogIO::POST;
				 break;
		 }
		 delete aval;
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
table::putcolkeywords(const std::string& columnname, const ::casac::record& value)
{
 Bool rstat(False);
 try {
	 if(itsTable){
		 Record *tvalue = toRecord(value);
		 itsTable->putKeywordSet(String(columnname), *tvalue);
		 delete tvalue;
		 rstat = True;
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
table::removekeyword(const ::casac::variant& keyword)
{
 Bool rstat(False);
 try {
	 if(itsTable){
		 switch(keyword.type()){
			 case variant::STRING :
		                 itsTable->removeKeyword(String(), keyword.toString(), -1);
		                 rstat = True;
				 break;
			 case variant::INT :
		                 itsTable->removeKeyword(String(), String(), keyword.toInt());
		                 rstat = True;
				 break;
			 default :
		                 *itsLog << LogIO::WARN << "Keyword must be string or int" << LogIO::POST;
				 break;
		 }
		 rstat = True;
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool
table::removecolkeyword(const std::string& columnname, const ::casac::variant& keyword)
{
 Bool rstat(False);
 try {
	 if(itsTable){
		 switch(keyword.type()){
			 case variant::STRING :
		                 itsTable->removeKeyword(String(columnname), String(keyword.toString()), -1);
		                 rstat = True;
				 break;
			 case variant::INT :
		                 itsTable->removeKeyword(String(columnname), String(), keyword.toInt());
		                 rstat = True;
				 break;
			 default :
		                 *itsLog << LogIO::WARN << "Keyword must be string or int" << LogIO::POST;
				 break;
		 }
		 rstat = True;
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
 }

::casac::record*
table::getdminfo()
{
 ::casac::record *rstat(0);
 try {
	 if(itsTable){
		 rstat = fromRecord(itsTable->getDataManagerInfo());
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

std::vector<std::string>
table::keywordnames()
{
 std::vector<std::string> rstat(0);
 try {
	 if(itsTable){
		 Vector<String> tmp = itsTable->getFieldNames(String(), String(), -1);
		 rstat = fromVectorString(tmp);
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

std::vector<std::string>
table::fieldnames(const std::string& keyword)
{
 std::vector<std::string> rstat(0);
 try {
	 if(itsTable){
		 Vector<String> tmp = itsTable->getFieldNames(String(), String(keyword), -1);
		 rstat = fromVectorString(tmp);
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

std::vector<std::string>
table::colkeywordnames(const std::string& columnname)
{
 std::vector<std::string> rstat(0);
 try {
	 if(itsTable){
		 Vector<String> tmp = itsTable->getFieldNames(String(columnname), String(), -1);
		 rstat = fromVectorString(tmp);
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

std::vector<std::string>
table::colfieldnames(const std::string& columnname, const std::string& keyword)
{
 std::vector<std::string> rstat(0);
 try {
	 if(itsTable){
		 Vector<String> tmp = itsTable->getFieldNames(String(columnname), String(keyword), -1);
		 rstat = fromVectorString(tmp);
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

::casac::record*
table::getdesc(const bool actual)
{
 ::casac::record *rstat(0);
 try {
	 if(itsTable){
		 rstat = fromRecord(itsTable->getTableDescription(actual));
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

::casac::record*
table::getcoldesc(const std::string& columnname)
{
 ::casac::record *rstat(0);
 try {
	 if(itsTable){
		 rstat = fromRecord(itsTable->getColumnDescription(columnname, True));
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool table::clearlocks()
{
	Bool rstat(True);
	Table::relinquishAutoLocks(True);
	return rstat;
}

bool
table::ok()
{
 Bool rstat(False);
 try {
	 if(itsTable){
		 rstat = True;
	 }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

casa::TableLock *
table::getLockOptions(casac::record &lockoptions){
	TableLock::LockOption opt = TableLock::AutoLocking;
	if( lockoptions.find("option") != lockoptions.end()){
	    String str = lockoptions["option"].toString();
            str.downcase();
	    if (str == "default") {
	        opt = TableLock::DefaultLocking;
	    } else if (str == "auto") {
	        opt = TableLock::AutoLocking;
	    } else if (str == "autonoread") {
	        opt = TableLock::AutoNoReadLocking;
	    } else if (str == "user") {
	        opt = TableLock::UserLocking;
	    } else if (str == "usernoread") {
	        opt = TableLock::UserNoReadLocking;
	    } else if (str == "permanent") {
	        opt = TableLock::PermanentLocking;
	    } else if (str == "permanentwait") {
	        opt = TableLock::PermanentLockingWait;
	    } else {
	        String message = "'" + str + "' is an unknown lock option; valid are "
	    "default,auto,autonoread,user,usernoread,permanent,permanentwait";
		*itsLog << LogIO::SEVERE << message << LogIO::POST;
	    }
	} else {
		*itsLog << LogIO::WARN << "No lock option set, do default AutoLocking" << LogIO::POST;
	}
	Double interval = 5.0;
	Int maxWait = 0;

	return new TableLock(opt, interval, maxWait);
}
bool table::fromascii(const std::string& tablename, const std::string& asciifile, const std::string& headerfile, const bool autoheader, const std::vector<int>& autoshape, const std::string& sep, const std::string& commentmarker, const int firstline, const int lastline, const bool nomodify, const std::vector<string> &columnnames, const std::vector<string> &datatypes){
bool rstatus(false);

   try {
      Vector<String> atmp, btmp;
      IPosition tautoshape;
      if(!itsTable)
         delete itsTable;
      if(columnnames[0] != "")
	      atmp = toVectorString(columnnames);
      if(datatypes[0] != "")
	      btmp = toVectorString(columnnames);
      itsTable = new casa::TableProxy(String(asciifile), String(headerfile), String(tablename), autoheader, tautoshape, String(sep), String(commentmarker), firstline, lastline, atmp, btmp);
      // itsTable = new casa::TableProxy(asciifile, headerfile, String(tablename));
      rstatus = true;
   } catch (AipsError x) {
      *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
      RETHROW(x);
   }
return rstatus;
}

//begin modification
//july 2 2007
//This function will write the final table. The table will
//depened on the structure of incoming Record, which was
//built using the XML ASDM table structure.

void test_record(Record &myRecord)
{

//Record related declarations
    //RecordDesc headdesc;
    Record     head;
    Record     *row;
    String     tableName("Scan.xml");


//Now adding elements to the record
    //headdesc.addField("tableName",TpString);
    head.define("tableName",tableName);
    for (int i=0;i<2;i++){
	row=new Record();
	row->define("scanNumber",i);
	head.defineRecord(tableName.toString(i),*row);
    }
    //cout << head << endl;

    std::string itsTag("scanNumber");
    std::string list("scanNumber scanTable obsList etc");
    cout << "find: " << list.find(itsTag) << endl;
    itsTag="startTime";
    cout << "no find: " << list.find(itsTag) << "npos: " << std::string::npos << endl;
    if (list.find(itsTag) == std::string::npos) cout << "False condition" << endl;
}

void write_table(Table &outTab)
{

    string tabName=outTab.tableName();
    cout << "TableName: " << tabName << endl;
    if (!(tabName.find("ScanTable") == std::string::npos))
    {

      ScalarColumn<String>  execBlockId(outTab,"execBlockId");
      ScalarColumn<Int>    scanNumber(outTab,"scanNumber");
      ScalarColumn<String> startTime(outTab,"startTime");
      ScalarColumn<String> endTime(outTab,"endTime");
      ScalarColumn<Int>    numSubScan(outTab,"numSubScan");
      ScalarColumn<Int>    numIntent(outTab,"numIntent");
      ArrayColumn<String>  scanIntent(outTab,"scanIntent");
      ScalarColumn<Bool>   flagRow(outTab,"flagRow");
      ScalarColumn<Int>    numField(outTab,"numField");
      ScalarColumn<String> sourceName(outTab,"sourceName");
      ArrayColumn<String>  fieldName(outTab,"fieldName");

      cout << "execBlockId: " << execBlockId.getColumn() << endl;
      cout << "scanNumber: " << scanNumber.getColumn() << endl;
      cout << "startTime: " << startTime.getColumn() << endl;
      cout << "endTime: " << endTime.getColumn() << endl;
      cout << "numSubScan: " << numSubScan.getColumn() << endl;
      cout << "numIntent: " << numIntent.getColumn() << endl;
      cout << "scanIntent: " << scanIntent.getColumn() << endl;
      cout << "flagRow: " << flagRow.getColumn() << endl;
      cout << "numField: " << numField.getColumn() << endl;
      cout << "sourceName: " << sourceName.getColumn() << endl;
      cout << "fieldName: " << fieldName.getColumn() << endl;

    } else if (!(tabName.find("MainTable") == std::string::npos))
    {
      ScalarColumn<String> configDesciptionId;
      ScalarColumn<String> fieldId;
      ScalarColumn<String> time;
      ScalarColumn<String> execBlockId;
      ArrayColumn<String>  stateId;
      ScalarColumn<Int>    scanNumber;
      ScalarColumn<Int>   subscanNumber;
      ScalarColumn<Int>    integrationNumber;
      ArrayColumn<Double>  uvw;
      ArrayColumn<String>  exposure;
      ArrayColumn<String>  timeCentroid;
      ScalarColumn<String> dataOid;
      ArrayColumn<Int>    flagAnt;
      ArrayColumn<Int>    flagPol;
      ArrayColumn<Int>    flagBaseband;
      ScalarColumn<String> interval;
      ScalarColumn<Int>    subintegrationNumber;

      /*cout << "configDesciptionId: " <<  endl;
      cout << "configDesciptionId: " <<  configDesciptionId.getColumn() << endl;
      cout <<  fieldId.getColumn() << endl;
      cout <<  time.getColumn() << endl;
      cout <<  execBlockId.getColumn() << endl;
      cout <<   stateId.getColumn() << endl;
      cout <<  scanNumber.getColumn() << endl;
      cout << subscanNumber.getColumn() << endl;
      cout <<  integrationNumber.getColumn() << endl;
      cout <<   uvw.getColumn() << endl;
      cout <<   exposure.getColumn() << endl;
      cout <<   timeCentroid.getColumn() << endl;
      cout <<  dataOid.getColumn() << endl;
      cout <<  flagAnt.getColumn() << endl;
      cout <<  flagPol.getColumn() << endl;
      cout <<  flagBaseband.getColumn() << endl;
      cout <<  interval.getColumn() << endl;
      cout <<  subintegrationNumber.getColumn() << endl;*/
    }

}

//bool table::fromASDM(const std::string& tablename, const std::string& xmlfile, const std::string& headerfile, const bool autoheader, const std::vector<int>& autoshape, const std::string& sep, const std::string& commentmarker, const int firstline, const int lastline, const bool nomodify, const std::vector<string> &columnnames, const std::vector<string> &datatypes){
bool table::fromASDM(const std::string& tablename, const std::string& xmlfile){
bool rstatus(false);

 try{
   asdmCasaXMLUtil myXMLUtil;
   Table outTab;
   
   //The root has the table name and total number of rows
   //There is somthing that I don't understand here
   //I declared nRows as Int, but if I use int values
   //at test_table for the row records, the task will
   //trigger an error of wrong datatype for nRows field,
   //which does not make any sense. I'll have to think about it 
   //there something wrongly implemented here. 

   std::string XMLfile = xmlfile;
   myXMLUtil.readXMLFile(outTab, XMLfile, tablename);
   rstatus=true;
 } catch (AipsError x) {
      *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
      RETHROW(x);
   }



return rstatus;
}
//end modification
} // casac namespace

