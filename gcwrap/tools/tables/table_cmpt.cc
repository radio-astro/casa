
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
#include <table_cmpt.h>
#include <casa/aips.h>
#include <tables/Tables/IncrementalStMan.h>
#include <tables/Tables/IncrStManAccessor.h>
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
#include <tools/utils/stdBaseInterface.h>
#include <tools/ms/Statistics.h>
//begin modification
//july 4 2007
#include <tools/xerces/asdmCasaXMLUtil.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/TableIter.h>
#include <tables/Tables/TableRow.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/StandardStMan.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/PlainTable.h>
#include <casa/Utilities/Regex.h>
// jagonzal: Needed for ISM error detection tool
#include <tables/Tables/DataManError.h>


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
        *itsLog << LogOrigin(__func__, tablename);
        *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
        RETHROW(x);
    }
    return rstat;
}

bool
table::create(const std::string& tablename, const ::casac::record& tabledesc,
              const ::casac::record& lockoptions,
              const std::string& endianformat, // "" == "aipsrc", "local",
                                               // "little", or "big".
	      const std::string& memtype,      // "memory" -> Table::Memory,
                                               // anything else -> Table::Plain.
	      int nrow,                        // 0 seems like a good default.
  	      const ::casac::record& dminfo)
{
 Bool rstat(False);
 try{
   Record *tlock = toRecord(lockoptions);
   Record *tdesc = toRecord(tabledesc);
   Record *dmI   = toRecord(dminfo);

   if(itsTable)
     close();
   itsTable = new casa::TableProxy(String(tablename), *tlock,
                                   String(endianformat), String(memtype),
                                   nrow, *tdesc, *dmI);
   delete tlock;
   delete tdesc;
   delete dmI;
   
   rstat = True;
 }
 catch (AipsError x) {
   *itsLog << LogOrigin("create", "")
           << LogIO::SEVERE
           << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;  
}

bool
table::flush()
{
 *itsLog << LogOrigin(__func__, name());

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
 *itsLog << LogOrigin(__func__, name());

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
 *itsLog << LogOrigin(__func__, name());

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

 *itsLog << LogOrigin(__func__, name());

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
 if(!rstat)
	 throw AipsError("Unable to create table");
  return rstat;

}


casac::table*
table::copy(const std::string& newtablename, const bool deep, const bool valuecopy, const ::casac::record& dminfo, const std::string& endian, const bool memorytable, const bool returnobject, const bool norows)
{
 *itsLog << LogOrigin(__func__, name());
 casac::table *rstat(0);
 try {
	 if(itsTable){
		 Record *tdminfo = toRecord(dminfo);
		 TableProxy *mycopy = new TableProxy;
		 *mycopy = itsTable->copy(newtablename, memorytable, deep, valuecopy, endian, *tdminfo, norows);
		 delete tdminfo;
		 rstat = new casac::table(mycopy);
	 } else {
		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST; }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 if(!rstat)
	 throw AipsError("Unable to create table");
 return rstat;
}

bool
table::copyrows(const std::string& outtable, const int startrowin, const int startrowout, const int nrow)
{
 *itsLog << LogOrigin(__func__, name());
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
 *itsLog << LogOrigin(__func__, name());
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
 *itsLog << LogOrigin(__func__, name());
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
 *itsLog << LogOrigin(__func__, name());
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
 *itsLog << LogOrigin(__func__, name());
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
 *itsLog << LogOrigin(__func__, name());
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
 *itsLog << LogOrigin(__func__, name());
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
 *itsLog << LogOrigin(__func__, name());
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
 *itsLog << LogOrigin(__func__, name());
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
 *itsLog << LogOrigin(__func__, name());
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
 *itsLog << LogOrigin(__func__, name());
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
               execlp("casabrowser", "casabrowser", myName.c_str(), "--casapy", (char *)0);
	   } else {
               execlp("casabrowser", "casabrowser", "--casapy", (char *)0);
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

bool
table::createmultitable(const std::string &outputTableName,
			const std::vector<std::string> &tableNames,
			const std::string &subDirName)
{
  *itsLog << LogOrigin("tb", "createmultitable");

  try {
    Block<String> tableNameVector(tableNames.size());
    Block<String> subtableVector(0);

    /* Copy the input vectors into Block */
    for (uInt idx=0; idx<tableNameVector.nelements(); idx++)
       tableNameVector[idx] = tableNames[idx];

    TableLock tlock(TableLock::AutoNoReadLocking);

    {
      ConcatTable concatTable(tableNameVector,
                              subtableVector,
			      subDirName, // move all member tables into this subdirectory
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
  return true;
}



std::string
table::name()
{
   *itsLog << LogOrigin(__func__, "");
   std::string myName("");
   if(itsTable){
      myName = itsTable->table().tableName();
   } else {
      *itsLog << LogIO::NORMAL << "No table opened." << LogIO::POST;
   }
   return myName;
}

bool
table::toasciifmt(const std::string& asciifile, const std::string& headerfile, const std::vector<std::string>& columns, const std::string& sep)
{
    *itsLog << LogOrigin(__func__, name());
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
table::taql(const std::string& taqlcommand)
{
 *itsLog << LogOrigin(__func__, this->name());
 ::casac::table *rstat(0);
 try {
   if(itsTable){
     casa::TableProxy *theQTab = new TableProxy(tableCommand(taqlcommand));
     rstat = new ::casac::table(theQTab);
   } else {
     *itsLog << LogIO::WARN
             << "No table specified, please open first" << LogIO::POST;
   }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
            << LogIO::POST;
    RETHROW(x);
 }
 if(!rstat)
	 throw AipsError("Unable to create table");
 return rstat;
}

::casac::table*
table::query(const std::string& query, const std::string& name,
             const std::string& sortlist, const std::string& columns,
             const std::string& style)
{
 *itsLog << LogOrigin(__func__, this->name());
 ::casac::table *rstat(0);
 try {
   if(itsTable){
     std::ostringstream taqlString;
     if(!style.empty())
       taqlString << "usingstyle " << style << " ";
     taqlString << "select";
     if(!columns.empty())
       taqlString << " " << columns;
     taqlString << " from \"" << this->name() << "\"";
     if(!query.empty())
       taqlString << " where " << query;
     if(!sortlist.empty())
       taqlString << " orderby " << sortlist;
     if(!name.empty())
       taqlString << " giving \"" << name << "\"";
     casa::TableProxy *theQTab = new TableProxy(tableCommand(taqlString.str()));
     rstat = new ::casac::table(theQTab);
   } else {
     *itsLog << LogIO::WARN
             << "No table specified, please open first" << LogIO::POST;
   }
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
            << LogIO::POST;
    RETHROW(x);
 }
 if(!rstat)
	 throw AipsError("Unable to create table");
 return rstat;
}

::casac::variant*
table::calc(const std::string& expr, const std::string& prefix, const bool showtaql)
{
  string str;
 *itsLog << LogOrigin(__func__, name());
 ::casac::variant *rstat(0);
 try {

   str=prefix+" calc "+expr;
   // if(itsTable){
   //		 std::ostringstream calcString;
   //		 calcString <<  "calc from " << itsTable->table().tableName() << " calc ";
   //		 calcString <<   expr;
		 //casa::Table *theQTab = new Table(tableCommand(taqlString.str()));
   //		 *itsLog << LogIO::WARN << "Calc not implemented!" << LogIO::POST;
   //	 } else {
   //		 *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
   //	 }

   TaQLResult result;
   if(showtaql)
     cerr << "TaQL string: " << str << endl;
   result = tableCommand (str);
   if(result.isTable()){
     *itsLog << LogIO::SEVERE << "Result is a table tool please use table.taql for now till the developers fix this " << LogIO::POST;
   }
   else{
     std::vector<int> shape;
     Vector<uInt> rownrs (result.node().nrow());
     indgen (rownrs);
     //cerr << "rownrs " << result.node().nrow() << "  is scalar " << result.node().isScalar() << endl;
     if(result.node().isScalar())
     {
       switch (result.node().getColumnDataType()) {
       case TpBool:{
	 Array<Bool> oBool;
	 oBool=result.node().getColumnBool(rownrs);
	 oBool.shape().asVector().tovector(shape);
	 std::vector<bool> s_bool(oBool.begin(), oBool.end());
	 rstat=new ::casac::variant(s_bool, shape);
       }
	 break;
       case TpUChar:{
	 Array<uChar> ouChar;
	 ouChar=result.node().getColumnuChar(rownrs);
	 ouChar.shape().asVector().tovector(shape);
	 std::vector<int> s_uchar(ouChar.begin(), ouChar.end());
	 rstat=new ::casac::variant(s_uchar, shape);
       }
	 break;
       case TpShort:{
	 Array<Short> oShort;
	 oShort=result.node().getColumnShort(rownrs);
	 oShort.shape().asVector().tovector(shape);
	 std::vector<int> s_short(oShort.begin(), oShort.end());
	 rstat=new ::casac::variant(s_short, shape);
       }
	 break;
	case TpUShort:{
	 Array<uShort> oushort;
	 oushort=result.node().getColumnuShort(rownrs);
	 oushort.shape().asVector().tovector(shape);
	 std::vector<int> s_ushort(oushort.begin(), oushort.end());
	 rstat=new ::casac::variant(s_ushort, shape);
       }
	 break; 
       case TpInt:{
	 Array<Int> oInt;
	 oInt=result.node().getColumnInt(rownrs);
	 oInt.shape().asVector().tovector(shape);
	 std::vector<int> s_int(oInt.begin(), oInt.end());
	 rstat=new ::casac::variant(s_int, shape);
       }
	 break;
       case TpUInt:{
	 Array<uInt> ouInt;
	 ouInt=result.node().getColumnuInt(rownrs);
	 ouInt.shape().asVector().tovector(shape);
	 std::vector<int> s_uint(ouInt.begin(), ouInt.end());
	 rstat=new ::casac::variant(s_uint, shape);
       } 
	 break;
       case TpFloat:{
	 Array<Float> oFloat;
	 oFloat=result.node().getColumnFloat(rownrs);
	 oFloat.shape().asVector().tovector(shape);
	 std::vector<double> s_float(oFloat.begin(), oFloat.end());
	 rstat=new ::casac::variant(s_float, shape);
       }
	 break;
       case TpDouble:{
	 Array<Double> oDouble;
	 oDouble=result.node().getColumnDouble(rownrs);
	 oDouble.shape().asVector().tovector(shape);
	 std::vector<double> s_double(oDouble.begin(), oDouble.end());
	 rstat=new ::casac::variant(s_double, shape);
       }	
	 break;
       case TpComplex:{
	 Array<Complex> oComplex;
	 oComplex=result.node().getColumnComplex(rownrs);
	 oComplex.shape().asVector().tovector(shape);
	 std::vector<std::complex<double> > s_complex(oComplex.begin(), oComplex.end());
	 rstat=new ::casac::variant(s_complex, shape);
       }	
	 break;
       case TpDComplex:{
	 Array<DComplex> odComplex;
	 odComplex=result.node().getColumnDComplex(rownrs);
	 odComplex.shape().asVector().tovector(shape);
	 std::vector<std::complex<double> > s_dcomplex(odComplex.begin(), odComplex.end());
	 rstat=new ::casac::variant(s_dcomplex, shape);
       }	
	 break;
       case TpString:{
	 Array<String> oString;
	 oString=result.node().getColumnString(rownrs);
	 oString.shape().asVector().tovector(shape);
	 std::vector<string> s_string(oString.begin(), oString.end());
	 rstat=new ::casac::variant(s_string, shape);
       }	
	 break;
	  
       default:
	 *itsLog << LogIO::SEVERE << "Don't know how to interprete result" << LogIO::POST;
	 break;

       }
     }
     else{
       Record outrec;
        for (uInt i=0; i< result.node().nrow(); i++) {
	  switch (result.node().dataType()) {
	  case TpBool:
	    outrec.define(String::toString(i), result.node().getArrayBool(i));
	    break;
	  case TpInt:
	    outrec.define(String::toString(i), result.node().getArrayInt(i));
	    break;
	  case TpDouble:
	    outrec.define(String::toString(i), result.node().getArrayDouble(i));
	    break;
	  case TpDComplex:
	    outrec.define(String::toString(i), result.node().getArrayDComplex(i));
	    break;
	  case TpString:
	    outrec.define(String::toString(i), result.node().getArrayString(i));
	    break;
	  default:
	     *itsLog << LogIO::SEVERE << "Don't know how to interprete result" << LogIO::POST;
	     break;
	  }
	}
	::casac::record *rec=fromRecord(outrec);
	rstat=new ::casac::variant(*rec);
     }
     
     
   }	      
   } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 if(!rstat)
	 throw AipsError("Failed while using TaQL expression "+str);
 return rstat;
}

::casac::table*
table::selectrows(const std::vector<int>& rownrs, const std::string& name)
{
 *itsLog << LogOrigin(__func__, this->name());
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
 if(!rstat)
	 throw AipsError("Unable to create table");
 return rstat;
}


::casac::record*
table::info()
{
 *itsLog << LogOrigin(__func__, name());
 ::casac::record *rstat(0);
 try {
	 if(itsTable){
		 rstat = fromRecord(itsTable->tableInfo());
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
 *itsLog << LogOrigin(__func__, name());
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
 *itsLog << LogOrigin(__func__, name());
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
 *itsLog << LogOrigin(__func__, name());
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
 *itsLog << LogOrigin(__func__, name());
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
 *itsLog << LogOrigin(__func__, name());
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
 *itsLog << LogOrigin(__func__, columnname);
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
 *itsLog << LogOrigin(__func__, columnname);
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
 *itsLog << LogOrigin(__func__, columnname);
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
    *itsLog << LogOrigin(__func__, columnname);
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
   *itsLog << LogOrigin(__func__, columnname);
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
 *itsLog << LogOrigin(__func__, name());
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
 *itsLog << LogOrigin(__func__, name());
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
 *itsLog << LogOrigin(__func__, name());
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
 *itsLog << LogOrigin(__func__, name());
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
 *itsLog << LogOrigin("addcols", name());
 
 try {
	 if(itsTable){
		 Record *tdesc = toRecord(desc);
		 Record *tdminfo = toRecord(dminfo);
		 itsTable->addColumns(*tdesc, *tdminfo, False);
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
 *itsLog << LogOrigin(__func__, name());
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
 *itsLog << LogOrigin(__func__, name());
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
 *itsLog << LogOrigin(__func__, columnname);
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
 *itsLog << LogOrigin(__func__, columnname);
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
 *itsLog << LogOrigin(__func__, columnname);
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
 *itsLog << LogOrigin(__func__, columnname);
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
 *itsLog << LogOrigin(__func__, columnname);
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
 *itsLog << LogOrigin(__func__, columnname);
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
table::putcell(const std::string& columnname, const std::vector<int>& rownr,
               const ::casac::variant& thevalue)
{
  Bool rstat(False);

  *itsLog << LogOrigin("putcell", columnname);
 
  try {
    if(itsTable){
      if(!itsTable->isWritable()){
        *itsLog << LogIO::WARN
                << "The table is not modifiable.  Was it opened with nomodify=False?"
                << LogIO::POST;
        return False;
      }

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
table::putcellslice(const std::string& columnname, const int rownr,
                    const ::casac::variant& value, const std::vector<int>& blc,
                    const std::vector<int>& trc, const std::vector<int>& incr)
{
  Bool rstat(False);

  *itsLog << LogOrigin("putcellslice", columnname);
 
  try {
    if(itsTable){
      if(!itsTable->isWritable()){
        *itsLog << LogIO::WARN
                << "The table is not modifiable.  Was it opened with nomodify=False?"
                << LogIO::POST;
        return False;
      }

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
table::putcol(const std::string& columnname, const ::casac::variant& value,
              const int startrow, const int nrow, const int rowincr)
{
 Bool rstat(False);

 *itsLog << LogOrigin("putcol", columnname);
 
 try {
   if(itsTable){
     if(!itsTable->isWritable()){
       *itsLog << LogIO::WARN
               << "The table is not modifiable.  Was it opened with nomodify=False?"
               << LogIO::POST;
       return False;
     }
     
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
table::putvarcol(const std::string& columnname, const ::casac::record& value,
                 const int startrow, const int nrow, const int rowincr)
{
  Bool rstat(False);

  *itsLog << LogOrigin("putvarcol", columnname);
 
  try {
    if(itsTable){
      if(!itsTable->isWritable()){
        *itsLog << LogIO::WARN
                << "The table is not modifiable.  Was it opened with nomodify=False?"
                << LogIO::POST;
        return False;
      }

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
table::putcolslice(const std::string& columnname, const ::casac::variant& value,
                   const std::vector<int>& blc, const std::vector<int>& trc,
                   const std::vector<int>& incr, const int startrow,
                   const int nrow, const int rowincr)
{
  Bool rstat(False);

  *itsLog << LogOrigin("putcolslice", columnname);
 
  try {
    if(itsTable){
      if(!itsTable->isWritable()){
        *itsLog << LogIO::WARN
                << "The table is not modifiable.  Was it opened with nomodify=False?"
                << LogIO::POST;
        return False;
      }

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
 *itsLog << LogOrigin(__func__, columnname);
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
 *itsLog << LogOrigin(__func__, name());
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
 *itsLog << LogOrigin(__func__, name());
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
 *itsLog << LogOrigin(__func__, columnname);
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
 *itsLog << LogOrigin(__func__, columnname);
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
 *itsLog << LogOrigin(__func__, name());
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
 *itsLog << LogOrigin(__func__, name());
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
 *itsLog << LogOrigin(__func__, columnname);
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
 *itsLog << LogOrigin(__func__, columnname);
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
 *itsLog << LogOrigin(__func__, name());
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
 *itsLog << LogOrigin(__func__, columnname);
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
 *itsLog << LogOrigin(__func__, name());
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
 *itsLog << LogOrigin(__func__, name());
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
 *itsLog << LogOrigin(__func__, name());
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
 *itsLog << LogOrigin(__func__, columnname);
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
 *itsLog << LogOrigin(__func__, columnname);
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
 *itsLog << LogOrigin(__func__, name());
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
 *itsLog << LogOrigin(__func__, columnname);
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

bool table::listlocks()
{
	Bool rstat(True);

	Vector<String> lockedTables = Table::getLockedTables();
	cout << "Locked tables: " << endl;
	for (uInt i=0;i<lockedTables.nelements();++i)
	  cout << "   " << lockedTables(i) << endl;
	cout << "--" << endl;
	return rstat;
}

bool
table::ok()
{
 *itsLog << LogOrigin(__func__, name());
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
        *itsLog << LogOrigin(__func__, name());
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

   *itsLog << LogOrigin(__func__, tablename);
   try {
      Vector<String> atmp, btmp;
      IPosition tautoshape;
      if(!itsTable)
         delete itsTable;
      if(columnnames[0] != "")
	      atmp = toVectorString(columnnames);
      if(datatypes[0] != "")
	      btmp = toVectorString(datatypes);
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
//depend on the structure of incoming Record, which was
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

 *itsLog << LogOrigin(__func__, tablename);

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


::casac::record* 
table::statistics(const std::string& column, 
                  const std::string& complex_value, 
                  const bool useflags)
{
    *itsLog << LogOrigin(__func__, column);
    ::casac::record *retval(NULL);

    try {
        if(itsTable){

            if (itsTable->nrows() == 0) {
                throw AipsError("Table has zero rows, cannot continue");
            }

            retval = fromRecord(casac::Statistics<Int>::get_statistics(itsTable->table(),
                                                                  column,
                                                                  complex_value,
                                                                  itsLog));

        } else {
            *itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
        }
    } catch (AipsError x) {
        *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
        RETHROW(x);
    } catch (std::exception &e) {
        *itsLog << LogIO::SEVERE << "Exception Reported: " << e.what() << LogIO::POST;
        RETHROW(e);
    } catch (...) {
        *itsLog << LogIO::SEVERE << "Unknown exception happened!" << LogIO::POST;
    }
    
    return retval;
}

std::vector<std::string>
table::showcache(const bool verbose)
{
 *itsLog << LogOrigin(__func__, name());
 std::vector<std::string> rstat(0);
 try {
     const TableCache& cache = PlainTable::tableCache();
     if(verbose){
	 if(cache.ntable()==0){
	     *itsLog << LogIO::NORMAL << "The Table Cache is empty." << LogIO::POST;
	 }
	 else{
	     *itsLog << LogIO::NORMAL << "The Table Cache has the following " << cache.ntable() << " entries:"  << LogIO::POST;
	 }	     
     }
     for (uInt i=0; i<cache.ntable(); ++i) {
	 if(verbose){
	     *itsLog << LogIO::NORMAL << "    " << i << ": \"" <<  cache(i)->tableName() << "\"" << LogIO::POST;
	 }
	 rstat.push_back(cache(i)->tableName());
     } 
 } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
 }
 return rstat;
}

bool table::testincrstman(const std::string& column)
{
	Bool ok(False);

	if (itsTable)
	{
		try
		{
			Record columnDescription = itsTable->getColumnDescription(column, True);

			int pos;

			// Get data manager type
			pos = -1;
			String dataManagerType;
			pos = columnDescription.fieldNumber ("dataManagerType");
			if (pos >= 0)
			{
				columnDescription.get (pos, dataManagerType);

				// Check that data manager type is IncrementalStMan
				if (dataManagerType != "IncrementalStMan")
				{
					*itsLog << LogIO::WARN
							<< "Data manager type of column "
							<< column
							<< " is not Incremental Store Manager"
							<< LogIO::POST;

					if (column == "FLAG_ROW")
					{
						*itsLog << LogIO::NORMAL	<< "Starting at 4.2.2 the default store manager for FLAG_ROW is Standard Store Manager " << endl
													<< "Therefore a MS imported with CASA version >= 4.2.2 is not exposed to the Incremental Store Manager problem"
													<< LogIO::POST;
					}

					return True;
				}
			}
			else
			{
				*itsLog << LogIO::WARN << "Data manager type not found for column: " << column << LogIO::POST;
				return False;
			}

			// Get data manager group
			pos = -1;
			String dataManagerGroup;
			pos = columnDescription.fieldNumber ("dataManagerGroup");
			if (pos >= 0)
			{
				columnDescription.get (pos, dataManagerGroup);
			}
			else
			{
				*itsLog << LogIO::SEVERE << "Data manager group not found for column: " << column << LogIO::POST;
				return False;
			}

			uInt offenndingCursor = 0;
			uInt offendingBucketStartRow = 0;
			uInt offendingBucketNrow = 0;
			uInt offendingBucketNr = 0;
			uInt offendingCol = 0;
			uInt offendingIndex = 0;
			uInt offendingRow = 0;
			uInt offendingPrevRow = 0;

			ROIncrementalStManAccessor acc(itsTable->table(), dataManagerGroup);
			ok = acc.checkBucketLayout (	offenndingCursor,
											offendingBucketStartRow,
											offendingBucketNrow,
											offendingBucketNr,
											offendingCol,
											offendingIndex,
											offendingRow,
											offendingPrevRow);

			if (not ok)
			{
				*itsLog << LogIO::SEVERE 	<< "Incremental Store Manager corruption (not ascending rowId) detected at "
											<< "bucket number " << offendingBucketNr
											<< " (startRow=" << offendingBucketStartRow
											<< ", nRows=" << offendingBucketNrow
											<< ", offendingBucketNr=" << offendingBucketNr
											<< ", cursor=" << offenndingCursor
											<< ") column=" << offendingCol
											<< " index=" << offendingIndex
											<< " rowId=" << offendingRow
											<< " preRowId=" << offendingPrevRow
											<< LogIO::POST;
			}
			else
			{
				*itsLog << LogIO::NORMAL 	<< "Incremental Store Manager corruption not detected in column "
											<< column
											<<  LogIO::POST;
			}


		}
		catch (AipsError x)
		{
			*itsLog << LogIO::SEVERE << x.getMesg() << LogIO::POST;
			return False;
		}
	}
	else
	{
		*itsLog << LogIO::WARN << "No table specified, please open first" << LogIO::POST;
		return False;
	}

	return ok;
}




} // casac namespace

