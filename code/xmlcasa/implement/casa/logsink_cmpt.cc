
/***
 * Framework independent implementation file for logsink...
 *
 * Implement the logsink component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#include <iostream>
#include <fstream>
#include <xmlcasa/casa/logsink_cmpt.h>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogFilter.h>
#include <xmlcasa/casa/TSLogSink.h>
#include <casa/Logging/NullLogSink.h>
#include <casa/Logging/StreamLogSink.h>
#include <xmlcasa/version.h>
#include <unistd.h>
#include <sys/param.h>
#include <xmlcasa/utils/CasapyWatcher.h>

using namespace std;
using namespace casa;

namespace casac {

	/*
static ofstream *logfile = new ofstream("logfile");
static LogSinkInterface *thelogsink
  = new StreamLogSink(logfile);
  */
static string theLogName;

//logsink::logsink():thelogsink(0)
logsink::logsink()
{
  if(!theLogName.size()){
     char *buff = NULL;
     char *mybuff = getcwd(buff, MAXPATHLEN);
     theLogName = string(mybuff) + string("/casapy.log");
  }
  //cout << "thelogsink=" << thelogsink << endl;
  thelogsink = new casa::TSLogSink();
  setlogfile(theLogName);
  itsorigin = new LogOrigin("casa");
  thelogsink->postLocally(LogMessage("", *itsorigin, LogMessage::NORMAL));
  globalsink = false;
  //version();
}

std::string logsink::version(){
  if(!thelogsink){
       thelogsink = &LogSink().globalSink();
  }
  ostringstream os1, os2;
  os1 << "CASA Version ";
  casa::VersionInfo::report(os1);
  os2 << "  Built on: "<< casa::VersionInfo::date();
  std::string mymess = os1.str();

  // Now here's a check to see if the code/VERSION file is at the same rev
  // as VersionInfo
  
  istringstream casapath(getenv("CASAPATH"));
  string casaroot;
  casapath >> casaroot;
  string versionFile( casaroot + "/code/VERSION");
  ifstream codeVERSION(versionFile.c_str());
  if(codeVERSION.good()){
     string versionNumber;
     codeVERSION >> versionNumber;
     ostringstream osv;
     osv << casa::VersionInfo::majorVersion() << "."
         << casa::VersionInfo::minorVersion() << "."
         << casa::VersionInfo::patch(); 
     if(versionNumber != osv.str()){
	string lookout("The version in casapy "+versionNumber
			+"  is different than in code/VERSION " + osv.str());
        thelogsink->postLocally(LogMessage(string(""), *itsorigin, LogMessage::WARN));
        thelogsink->postLocally(LogMessage(lookout, *itsorigin, LogMessage::WARN));
        thelogsink->postLocally(LogMessage(string(""), *itsorigin, LogMessage::WARN));
     }
  }
  thelogsink->postLocally(LogMessage(mymess, *itsorigin, LogMessage::NORMAL));
  thelogsink->postLocally(LogMessage(os2.str(), *itsorigin, LogMessage::NORMAL));
  return mymess;
}

logsink::~logsink()
{
	delete itsorigin;
	if(!globalsink)
	   delete thelogsink;
	thelogsink=0;
	itsorigin=0;
}

bool logsink::origin(const std::string &fromwhere)
{
    bool rstat(true);
    if(!thelogsink){
       thelogsink = &LogSink().globalSink();
    }
    delete itsorigin;
    itsorigin = new LogOrigin("casa");
    //String* taskname = new String(fromwhere);
    taskname = new String(fromwhere);
    itsorigin->taskName(*taskname);
    thelogsink->setTaskName(*taskname);
    LogSink().globalSink().setTaskName(*taskname);
    return rstat;
}

bool logsink::filter(const std::string &level)
{
    if(!thelogsink){
       thelogsink = &LogSink().globalSink();
    }
	bool rstat(true);
	LogMessage::Priority priority = LogMessage::NORMAL;
        if (level == "DEBUG")
           priority = LogMessage::DEBUGGING;
        else if (level == "DEBUG1")
           priority = LogMessage::DEBUG1;
        else if (level == "DEBUG2")
           priority = LogMessage::DEBUG2;
        else if (level == "NORMAL")
           priority = LogMessage::NORMAL;
        else if (level == "NORMAL1")
           priority = LogMessage::NORMAL1;
        else if (level == "NORMAL2")
           priority = LogMessage::NORMAL2;
        else if (level == "NORMAL3")
           priority = LogMessage::NORMAL3;
        else if (level == "NORMAL4")
           priority = LogMessage::NORMAL4;
        else if (level == "NORMAL5")
           priority = LogMessage::NORMAL5;
        else if (level == "INFO")
           priority = LogMessage::NORMAL;
        else if (level == "INFO1")
           priority = LogMessage::NORMAL1;
        else if (level == "INFO2")
           priority = LogMessage::NORMAL2;
        else if (level == "INFO3")
           priority = LogMessage::NORMAL3;
        else if (level == "INFO4")
           priority = LogMessage::NORMAL4;
        else if (level == "INFO5")
           priority = LogMessage::NORMAL5;
        else if (level == "WARN")
           priority = LogMessage::WARN;
        else if (level == "ERROR")
           priority = LogMessage::SEVERE;
	else 
		rstat = false;
	if(rstat){
	   LogFilter filter(priority);
	   thelogsink->filter(filter);
	   
	   // Also set for any watchers.
	   CasapyWatcher::logChanged_(priority);
	}
	return rstat;
}

bool logsink::post(const std::string& message,
		   const std::string& priority,
		   const std::string& origin)
{
	return postLocally(message, priority, origin);
}

bool
logsink::postLocally(const std::string& message,
                     const std::string& priority,
		     const std::string& origin)
{
    LogMessage::Priority messagePriority = LogMessage::NORMAL;

    if(!thelogsink){
       thelogsink = &LogSink().globalSink();
    }
    if(!itsorigin)
       itsorigin = new LogOrigin("casa");
    itsorigin->className(origin);
    taskname = new String(origin);
    //String* taskname = new String(origin);
    //cout << "(*taskname==\"\")=" <<  (*taskname == "") << endl;
    thelogsink->setTaskName(*taskname);
    //LogSink().globalSink().setTaskName(*taskname);
    if (priority == "DEBUG")
       messagePriority = LogMessage::DEBUGGING;
    else if (priority == "DEBUG1")
       messagePriority = LogMessage::DEBUG1;
    else if (priority == "DEBUG2")
       messagePriority = LogMessage::DEBUG2;
    else if (priority == "NORMAL5")
       messagePriority = LogMessage::NORMAL5;
    else if (priority == "NORMAL4")
       messagePriority = LogMessage::NORMAL4;
    else if (priority == "NORMAL3")
       messagePriority = LogMessage::NORMAL3;
    else if (priority == "NORMAL2")
       messagePriority = LogMessage::NORMAL2;
    else if (priority == "NORMAL1")
       messagePriority = LogMessage::NORMAL1;
    else if (priority == "NORMAL")
       messagePriority = LogMessage::NORMAL;
    else if (priority == "INFO5")
       messagePriority = LogMessage::NORMAL5;
    else if (priority == "INFO4")
       messagePriority = LogMessage::NORMAL4;
    else if (priority == "INFO3")
       messagePriority = LogMessage::NORMAL3;
    else if (priority == "INFO2")
       messagePriority = LogMessage::NORMAL2;
    else if (priority == "INFO1")
       messagePriority = LogMessage::NORMAL1;
    else if (priority == "INFO")
       messagePriority = LogMessage::NORMAL;
    else if (priority == "WARN")
       messagePriority = LogMessage::WARN;
    else if (priority == "SEVERE")
       messagePriority = LogMessage::SEVERE;
    else if (priority == "ERROR")
       messagePriority = LogMessage::SEVERE;
    return thelogsink->postLocally(LogMessage(message,
       *itsorigin,
       messagePriority));
}

std::string
logsink::localId()
{
  return thelogsink->localId();
}

std::string
logsink::id()
{
  return thelogsink->id();
}

bool logsink::setglobal(const bool isglobal)
{
   bool rstat(true);
   
   if(isglobal){
      //cout << "isglobal=" << isglobal << endl;
      //cout << "thelogsink=" << thelogsink << endl;
      LogSink().globalSink(thelogsink);
      globalsink = isglobal;
      //cout << "setglobal(True) ok" << endl;
   } else {
      //cout << "isglobal=" << isglobal << endl;
      LogSinkInterface *dummy = new StreamLogSink(LogMessage::NORMAL, &cerr);
      //cout << "dummy==" << dummy << endl;
      //LogSink().globalSink(dummy);
      //cout << "setglobal(True) ok" << endl;
   }
   return rstat;
}
//
bool logsink::setlogfile(const std::string& filename)
{
   bool rstat(true);
   if(filename != "null")
      static_cast<TSLogSink*>(thelogsink)->setLogSink(filename);
   else
      thelogsink = new NullLogSink();
      
   //
   // Also set for any watchers.
   CasapyWatcher::logChanged_(filename);
      
   return rstat;
}

bool
logsink::showconsole(const bool onconsole)
{
   bool rstat(true);
   if(thelogsink)
      thelogsink->cerrToo(onconsole);
   return rstat;
}

} // casac namespace

