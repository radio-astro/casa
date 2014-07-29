//# MemoryLogSink.h: save distributed log messages
//# Copyright (C) 2005
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

#include <casa/System/Aipsrc.h>
#include <tools/casa/TSLogSink.h>
#include <casa/Logging/LogFilter.h>
#include <casa/Exceptions/Error.h>
#include <casa/Utilities/Assert.h>
#include <casa/OS/Time.h>
#include <casa/Quanta/MVTime.h>

#ifdef AIPS_LOG4CPLUS
#include <log4cplus/logger.h>
#include <log4cplus/configurator.h>

using namespace log4cplus;
#else
#include <iostream>
#include <fstream>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/StreamLogSink.h>
#endif

namespace casa { //# NAMESPACE CASA - BEGIN

String TSLogSink::localId( ) {
    return String("TSLogSink");
}

String TSLogSink::id( ) const {
    return String("TSLogSink");
}

TSLogSink::TSLogSink()
: LogSinkInterface()
{
#ifdef AIPS_LOG4CPLUS
    BasicConfigurator config;
    config.configure();
#else
   logsink = 0 ;
   logfile = 0 ;
   send2cerr = False;
   setLogSink();
#endif
}

TSLogSink::TSLogSink (LogMessage::Priority filter)
: LogSinkInterface(LogFilter(filter))
{
#ifndef AIPS_LOG4CPLUS
   logsink = 0 ;
   logfile = 0 ;
   setLogSink();
   send2cerr = False;
#endif
}

TSLogSink::TSLogSink (const LogFilterInterface& filter)
: LogSinkInterface(filter)
{
#ifndef AIPS_LOG4CPLUS
   logsink = 0 ;
   logfile = 0 ;
   setLogSink();
   send2cerr = False;
#endif
}

#ifndef AIPS_LOG4CPLUS
//
// if you supply a logname it uses that, otherwise it looks for a logname in
// the aiprc file, else it defaults to the old standby.
//
void TSLogSink::setLogSink(String logname){
   
   if(!logname.size()){
      String logfileKey="user.logfile";
      String logname2;
      if(!Aipsrc::find(logname2, logfileKey)){
         logname = "casapy.log";
      } else {
         logname = logname2;
      }
   }
   if (logfile) {
     logfile->close() ;
     delete logfile ;
   }
   logfile = new std::ofstream(logname.c_str(), ios::app);
   if(logfile) {
     delete logsink ;
     logsink = new StreamLogSink(LogMessage::NORMAL, logfile);
   }
   if(!logsink)
	   cerr << "Unable to log to " << logname << endl;
   return;
}
#endif

TSLogSink::TSLogSink (const TSLogSink& other)
{
  //copy_other (other);
  LogSinkInterface::operator= (other);
}

TSLogSink& TSLogSink::operator= (const TSLogSink& other)
{
  if (this != &other) {
    copy_other (other);
  }
  return *this;
}

void TSLogSink::copy_other (const TSLogSink& other)
{
  LogSinkInterface::operator= (other);
}

TSLogSink::~TSLogSink()
{
#ifndef AIPS_LOG4CPLUS
   logfile->close();
   delete logfile;
   delete logsink;
#endif
}

const LogFilterInterface& TSLogSink::filter () const{return logsink->filter();}

LogSinkInterface& TSLogSink::filter (const LogFilterInterface& newfilter)
{
	LogSinkInterface::filter(newfilter);
	return logsink->filter(newfilter); 
}

Bool TSLogSink::postLocally (const LogMessage& message)
{
  Bool posted = False;
  if (this->filter().pass(message)) {
    String tmp;
    message.origin().objectID().toString(tmp);
    LogOrigin theOrigin(message.origin());
    if(!message.origin().taskName().length()){
       theOrigin.taskName(LogSinkInterface::taskName);
    } else {
       theOrigin.taskName(message.origin().taskName());
    }
    logsink->setTaskName(theOrigin.taskName());
    const_cast<LogMessage &>(message).origin(theOrigin);


#ifdef AIPS_LOG4CPLUS
    switch (message.priority())
    {
	    case LogMessage::DEBUGGING:
            LOG4CPLUS_DEBUG(this, tmp);
            break;

	    case LogMessage::NORMAL:
	    LOG4CPLUS_INFO(this, tmp);
            break;

	case LogMessage::WARN:
	    LOG4CPLUS_WARN(this, tmp);
            break;

	case LogMessage::SEVERE:
            this, tmp);
            break;

        default:
    }
#else
    logsink->postLocally(message);
    posted = True;
    switch (message.priority())
    {
	case LogMessage::DEBUGGING:
		if(send2cerr)
                   std::cerr << message.toString() << std::endl;
		break;
	case LogMessage::DEBUG1:
		if(send2cerr)
		   std::cerr << message.toString() << std::endl;
		break;
	case LogMessage::DEBUG2:
		if(send2cerr)
		   std::cerr << message.toString() << std::endl;
		break;
	case LogMessage::NORMAL:
		if(send2cerr)
		   std::cerr << message.toString() << std::endl;
		break;
	case LogMessage::NORMAL1:
		if(send2cerr)
		   std::cerr << message.toString() << std::endl;
		break;
	case LogMessage::NORMAL2:
		if(send2cerr)
		   std::cerr << message.toString() << std::endl;
		break;
	case LogMessage::NORMAL3:
		if(send2cerr)
		   std::cerr << message.toString() << std::endl;
		break;
	case LogMessage::NORMAL4:
		if(send2cerr)
		   std::cerr << message.toString() << std::endl;
		break;
	case LogMessage::NORMAL5:
		if(send2cerr)
		   std::cerr << message.toString() << std::endl;
		break;
	case LogMessage::WARN:
		// if(send2cerr) // jagonzal: casalog.showconsole should control also if WARN/SEVERE msg go to console
		std::cerr << message.toString() << std::endl;
		break;
	case LogMessage::SEVERE:
		// if(send2cerr) // jagonzal: casalog.showconsole should control also if WARN/SEVERE msg go to console
		std::cerr << message.toString() << std::endl;
		break;
        default:
		break;
    }
#endif
  }

  return posted;
}

void TSLogSink::cerrToo(Bool cerr2){
	send2cerr = cerr2;
}

} //# NAMESPACE CASA - END

