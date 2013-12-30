//# Error2.cc: Base class for all AIPS++ errors (non-templated classes)
//# Copyright (C) 1993,1994,1995,1996,1997,2000,2001
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
//# $Id: Error2.cc 21040 2011-04-07 13:26:55Z gervandiepen $

#include <casa/Exceptions/Error.h>
#include <casa/Exceptions/CasaErrorTools.h>
#include <casa/stdlib.h>
#include <casa/iostream.h>
#include <casa/System/AipsrcValue.h>
#include <casa/string.h>      //# needed for strerror
#include <execinfo.h> //# needed for backtrace

#define AipsError_StackTracing  // Allows removing calls to trace addition logic at compile time

#define AipsError_StackTracing_Default False  // Assume enabled unless told otherwise

#if defined (AipsError_StackTracing)
#    define AddStackTrace() addStackTrace()
#else
#    define AddStackTrace()
#endif

namespace casa { //# NAMESPACE CASA - BEGIN

String AipsError::lastMessage = AipsError::noMessage ();
String AipsError::lastStackTrace = AipsError::noStackTrace();
Mutex  AipsError::lastErrorMutex;


AipsError::AipsError (Category c)
: message(), category(c)
{
    AddStackTrace();
}


AipsError::AipsError(const Char *str,Category c)
  : message(str), category(c)
{
    AddStackTrace();
}

AipsError::AipsError(const String &str,Category c)
  : message(str), category(c)
{
    AddStackTrace();
}

AipsError::AipsError (const String &msg, const String& filename,
                      uInt lineNumber, Category c)
  : category(c)
{
  ostringstream os;
  os << msg << " at File: " << filename << ", line: " << lineNumber;
  message = os.str();

  AddStackTrace ();
}

AipsError::~AipsError() throw()
{}

void
AipsError::addStackTrace ()
{

    // Always generate a stack trace and keep it around in a static
    // for later retrieval via casapy

    stackTrace = generateStackTrace();

    {
        ScopedMutexLock lock(lastErrorMutex);
        lastMessage = message;
        lastStackTrace = stackTrace;
    }

    // See if the default is to tack on the stack trace on the exception
    // message.  N.B.: Turning this on will break some of the low-level tests
    // which simply compare expected to actual output.

    Bool enabled;
    AipsrcValue<Bool>::find (enabled, "AipsError.enableStackTrace", AipsError_StackTracing_Default);

    if (enabled) {

        // If permitted, append to the error message.

        message += stackTrace;
    }
}

String
AipsError::generateStackTrace()
{
    // Get the most recent 512 levels of the stack trace.
    // This ought to handle all cases but very deep recursion.

    void * stack [512]; // Allow for up to 512 levels
    int nLevels = backtrace (stack, 512);

    // Convert the internal stack representation into one string
    // per level.

    char ** trace = backtrace_symbols (stack, nLevels);

    // Put a header on the message and then append all of the
    // strings onto the message.

    String stackTrace;
    stackTrace += "\n||> Stack trace (use c++filt to demangle):\n";

    for (int i = 0; i < nLevels; i++){
        stackTrace += trace[i] + String ("\n");
    }

    // Free up the array returned by backtrace_symbols.  The
    // strings it points to must not be deleted by this method.

    free (trace);

    return stackTrace;
}


void
AipsError::getLastInfo (String & message, String & stackTrace)
{
    ScopedMutexLock lock(lastErrorMutex);

    message = getLastMessage ();
    stackTrace = getLastStackTrace ();
}

String
AipsError::getLastMessage ()
{
    return lastMessage;
}

String
AipsError::getLastStackTrace ()
{
    return CasaErrorTools::replaceStackAddresses (lastStackTrace);
}

String
AipsError::getStackTrace () const
{
    return CasaErrorTools::replaceStackAddresses (stackTrace);
}

void
AipsError::clearLastInfo ()
{
    ScopedMutexLock lock(lastErrorMutex);
    lastMessage = noMessage ();
    lastStackTrace = noStackTrace ();
}



String
AipsError::noMessage ()
{
    return "*none*";
}

String
AipsError::noStackTrace ()
{
    return "*no-stack-trace*";
}

void
AipsError::throwIf (bool condition, const String & message, const String & file, Int line, const String & func)
{

	// If the condition is met then throw an AipsError

	if (condition) {
	    String m = String::format ("Exception: %s.\n... thrown by %s", message.c_str(), func.c_str());
	    AipsError e (m.c_str(), file.c_str(), line);

	    throw e;
	}
}

void
AipsError::throwIfError (int errorCode, const String & prefix, const String & file, Int line, const String & func)
{
	// If the provided error code is not equal to success (0) then
	// throw an AipsError using the provided suffix and then details
	// of the error.

	if (errorCode != 0) {
		AipsError e (String::format ("Exception: %s.\n...Thrown by %s\n...(errno=%d): %s", prefix.c_str(),
		                             func.c_str(), errorCode, strerror (errorCode)),
		             file.c_str(), line);

	    throw e;
	}
}

AipsError
AipsError::repackageAipsError (AipsError & error, const String & message, const String & file, Int line, const String & func)
{
    ostringstream os;

    AipsError tmp (message, file, line);
    os << "+++Exception: " << tmp.what() << ".\n...Thrown by " << func << ": "
       << "\n...Lower level exception: " << error.what() << "\n--- end exception\n";

    return AipsError (os.str());
}



AllocError::~AllocError() throw()
{}


IndexError::~IndexError() throw()
{}


DuplError::~DuplError() throw()
{}


SystemCallError::SystemCallError(const String& funcName, int error, Category c)
  : AipsError("Error in " + funcName + ": " + errorMessage(error), c),
    itsError (error)
{}
SystemCallError::~SystemCallError() throw()
{}
String SystemCallError::errorMessage(int error)
{
  // Use strerror_r for thread-safety.
  char buffer[128];
  // There are two incompatible versions of versions of strerror_r()
#if !__linux__ || (!_GNU_SOURCE && (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600))
  if (strerror_r(error, buffer, sizeof buffer) == 0) {
    return String(buffer);
  }
  return "errno " + String::toString(error);
#else
  return strerror_r(error, buffer, sizeof buffer);
#endif
}

SystemCallError::SystemCallError (int error, const String &msg, const String &filename,
                                  uInt lineNumber, Category c)
  : AipsError (msg + String::format (": errno=%d: %s", error,
				     errorMessage (error).c_str()),
             filename, lineNumber, c),
  itsError (error)
{}


// Exception which causes an abort instead of continuing
AbortError::AbortError(const Char *str,Category c)
: AipsError(str,c)
{
    cerr << "An unrecoverable error occurred: " << endl;
    cerr << str << endl;
#ifndef CASACORE_NOEXIT
    exit(1);
#endif
}

AbortError::AbortError(const String &str,Category c)
: AipsError(str,c)
{
    cerr << "An unrecoverable error occurred: " << endl;
    cerr << str << endl;
#ifndef CASACORE_NOEXIT
    exit(1);
#endif
}

AbortError::~AbortError() throw()
{}

} //# NAMESPACE CASA - END

