//# Log.h: this defines a singleton logger for the TablePlot classes.
//# Copyright (C) 2007
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
//#
//# -------------------------------------------------------------------------

#ifndef SLOG_H
#define SLOG_H

#include <casa/Logging/LogIO.h>
#include <casa/BasicSL/String.h>
    
namespace casa {


class SLog : public casa::LogIO
{
public:
   static SLog *slog();
   ~SLog(); 
    
   void out(const String &msg, 
            const String& fnname = "",
            const String& clname = "", 
            LogMessage::Priority msglevel=LogMessage::DEBUG1,
            Bool onconsole = False);
   void FnEnter(String fnname, String clname);
   void FnExit(String fnname, String clname);
   void FnPass(String fnname, String clname);

private:
   static SLog* instance;
   //static uInt refCount;
   SLog();

};

};
#endif

