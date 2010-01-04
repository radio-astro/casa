
/***
 * Framework independent implementation file for utils...
 *
 * Implement the utils component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#include <iostream>
#include <fstream>
#include <xmlcasa/record.h>
#include <xmlcasa/utils/utils_cmpt.h>
#include <xmlcasa/utils/stdBaseInterface.h>
#include <xmlcasa/xerces/stdcasaXMLUtil.h>
#include <casa/Logging/LogIO.h>
#include <casa/BasicSL/String.h>
#include <casa/OS/File.h>
#include <casa/OS/DOos.h>
#include <tables/Tables/Table.h>
#include <casa/System/Aipsrc.h>
#include <casa/OS/HostInfo.h>


using namespace std;
using namespace casa;

namespace casac {

utils::utils()
{
  myConstraints = 0;
  itsLog = new casa::LogIO;
}

utils::~utils()
{
  if(myConstraints)
     delete myConstraints;
  delete itsLog;
}

bool
utils::verify(const ::casac::record& input, const ::casac::variant& xmldescriptor)
{

   bool rstat(true);
   record *constraints(0);
   *itsLog << LogOrigin("utils", "verify") << LogIO::NORMAL3 << "Verifying arguments....";
   switch(xmldescriptor.type()){
      case variant::STRING :
	 constraints = torecord(xmldescriptor.getString());
	 // std::cerr << "constraints record: ";
	 // dumpRecord(std::cerr, *constraints);
         break;
      case variant::RECORD :
         constraints = new record(xmldescriptor.getRecord());
         break;
      default :
         rstat = false;
         break;
   }
   if(rstat){
	   rstat = stdBaseInterface::verify(const_cast<record &>(input), *constraints, *itsLog);
	   if(rstat){
		   *itsLog << LogOrigin("utils", "verify") << LogIO::NORMAL3 << "verified." << LogIO::POST;
	   }else{
		   *itsLog <<  LogIO::POST;
		   *itsLog << LogOrigin("utils", "verify") << LogIO::WARN << "Some arguments failed to verify!" << LogIO::POST;
	   }
   }
   if(constraints)
	   delete constraints;
   // std::cerr << "return from verify is " << rstat << std::endl;
   return rstat;
}

bool
utils::setconstraints(const ::casac::variant& xmldescriptor)
{
   bool rstat(true);
   if(myConstraints)
	   delete myConstraints;
   *itsLog << LogOrigin("utils", "setconstraints") << LogIO::NORMAL3 << "Setting constraints ...";
   switch(xmldescriptor.type()){
      case variant::STRING :
	 myConstraints = torecord(xmldescriptor.getString());
	 // std::cerr << "constraints record: ";
	 // dumpRecord(std::cerr, *constraints);
         break;
      case variant::RECORD :
         myConstraints = new record(xmldescriptor.getRecord());
         break;
      default :
	 rstat = false;
   }
   *itsLog << LogIO::NORMAL3 << "Constraints set." << LogIO::POST;
	 //std::cerr << "constraints record: ";
	 //dumpRecord(std::cerr, *myConstraints);
   return rstat;
}

bool
utils::verifyparam(const ::casac::record& param)
{
   bool rstat(true);
   if(myConstraints){
      rec_map::iterator iter = myConstraints->begin(); // We need the underlying record...
      rstat = stdBaseInterface::verifyOne(const_cast<record &>(param), (*iter).second.asRecord(), *itsLog);
      /*
      if(rstat){
         *itsLog << LogOrigin("utils", "verifyparam") <<LogIO::INFO<< "verified." << LogIO::POST;
      }else{
         *itsLog << LogIO::POST;
         *itsLog << LogOrigin("utils", "verifyparam") << LogIO::WARN 
		 << "Some arguments failed to verify!" << LogIO::POST;
      }
      */
   } else {
         *itsLog << LogOrigin("utils", "verifyparam") << LogIO::WARN
		 << "Contraints record not set, unable to verify parameter" << LogIO::POST;
   }
   return rstat;
}

::casac::variant*
utils::expandparam(const std::string& name , const ::casac::variant& value )
{
   ::casac::variant *rstat(0);
   rec_map::iterator iter = myConstraints->begin(); // We need the underlying record...
   if(myConstraints){
       //dumpRecord(std::cerr, (*iter).second.asRecord()["parameters"].asRecord()[name].asRecord());
       if((*iter).second.asRecord()["parameters"].asRecord().count(name) &&
		       (*iter).second.asRecord()["parameters"].asRecord()[name].asRecord().count("allowed")){
          rstat = stdBaseInterface::expandEnum((*iter).second.asRecord()["parameters"].asRecord()[name].asRecord()["allowed"], value, *itsLog);
       }else{
	  rstat = new variant(value);
       }
   } else {
       rstat = new variant(casac::initialize_variant(""));;
       *itsLog << LogOrigin("utils", "expandparam") << LogIO::WARN
		 << "Contraints record not set, unable to expand parameter" << LogIO::POST;
   }
   return rstat;
}

::casac::record*
utils::torecord(const std::string& input)
{
   stdcasaXMLUtil xmlUtils;
   casac::record *rstat = new casac::record;
   if(!input.find("<?xml version")){
      xmlUtils.toCasaRecord(*rstat, input);
   }else{
      if(!input.find("file:///")){
         Bool ok = xmlUtils.readXMLFile(*rstat, input.substr(7));
	 if(!ok){
            *itsLog << LogIO::SEVERE << "Unable to read XML file " << input << ", unable to verify input" << LogIO::POST;
	 }
      } else {
         *itsLog << LogIO::SEVERE << "Defaults specified are not an XML string, unable to verify input" << LogIO::POST;
      }
   }
   return rstat;
}

std::string
utils::toxml(const ::casac::record& input, const bool asfile, const std::string& filename)
{   string rstat;

   stdcasaXMLUtil xmlUtils;
   if(asfile){
      std::ofstream xmlout(filename.c_str(), ios::out);
      xmlUtils.fromCasaRecord(xmlout, input);
      rstat = filename;
   } else {
      ostringstream xmlout;
      xmlUtils.fromCasaRecord(xmlout, input);
      rstat = xmlout.str();
   }
   return rstat;
}

std::string
utils::getrc(const std::string& rcvar)
{
  String rstat1;
  if(!rcvar.length()){
	  rstat1 = Aipsrc::aipsRoot();
  } else {
	  if(!Aipsrc::find(rstat1, rcvar))
		  rstat1 = "Unknown value";
  }
  string rstat(rstat1.c_str());
  return rstat;
}

bool
utils::removetable(const std::vector<std::string> &tablenames)
{
  bool rstat(true);
  try {
     *itsLog << LogOrigin("utils", "removetable");
     for(vector<std::string>::const_iterator iter = tablenames.begin();
		     iter != tablenames.end(); iter++){
       String fileName(*iter);
       if (fileName.empty()) {
          *itsLog << LogIO::WARN << "Empty filename" << LogIO::POST;
          rstat = false;
       }
       File f(fileName);
       if (! f.exists()) {
           *itsLog << LogIO::WARN << fileName << " does not exist." << LogIO::POST;
          rstat = false;
       }

// Now try and blow it away.  If it's open, tabledelete won't delete it.
       String message;
       if(rstat && Table::isReadable(fileName)){
          if (Table::canDeleteTable(message, fileName, True)) {
             Table::deleteTable(fileName, True);
          } else {
             *itsLog << LogIO::WARN << "Cannot delete file " << fileName
             << " because " << message << LogIO::POST;
          }
       } else {
           *itsLog << LogIO::WARN << "Cannot delete file " << fileName
           << " because it's not a table." << LogIO::POST;
       }
  }
    } catch (AipsError x) {
       *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
       << LogIO::POST;
       RETHROW(x);
  }
  return rstat;
}

::casac::record *utils::tableinfo(const std::string &tablename) {
    Vector<Int> info = casa::DOos::lockInfo(tablename);
    ::casac::record *result = new record( );

    switch( info[0] ) {
    case 3:
	result->insert("lockstatus", "write");
	break;
    case 2:
	result->insert("lockstatus", "read");
	break;
    case 1:
	result->insert("lockstatus", "open");
	break;
    case 0:
	result->insert("lockstatus", "not in use");
	break;
    default:
	result->insert("lockstatus", "unknown");
    }

    result->insert("lockpid", info[1]);
    result->insert("lockperm", info[2] ? true : false);
    return result;
}

std::vector<std::string> utils::lockedtables( ) {
    Vector<String> locks = Table::getLockedTables( );
    std::vector<std::string> result;
    for (unsigned int x = 0; x < locks.nelements(); ++x ) {
	result.push_back(locks[x]);
    }
    return result;
}


typedef int SIZETCAST;
::casac::record *utils::hostinfo( ) {
    ::casac::record *result = new record( );

    ::casac::record *swap = new record( );
    swap->insert( "total", (SIZETCAST) HostInfo::swapTotal( ) );
    swap->insert( "used", (SIZETCAST) HostInfo::swapUsed( ) );
    swap->insert( "free", (SIZETCAST) HostInfo::swapFree( ) );
    result->insert( "swap", swap );

    ::casac::record *memory = new record( );
    memory->insert( "total", (SIZETCAST) HostInfo::memoryTotal( ) );
    memory->insert( "available", (SIZETCAST) HostInfo::memoryTotal(true) );
    memory->insert( "used", (SIZETCAST) HostInfo::memoryUsed( ) );
    memory->insert( "free", (SIZETCAST) HostInfo::memoryFree( ) );
    result->insert( "memory", memory );

    ::casac::record *cpus = new record( );
    cpus->insert( "total", HostInfo::numCPUs( ) );
    cpus->insert( "available", HostInfo::numCPUs(true) );
    result->insert( "cpus", cpus );

    result->insert( "endian", HostInfo::bigEndian( ) ? "big" : "little" );
    result->insert( "hostname", HostInfo::hostName( ) );
    result->insert( "pid", HostInfo::processID( ) );

    result->insert( "seconds", HostInfo::secondsFrom1970( ) );
    
    return result;
}

} // casac namespace

