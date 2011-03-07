/*
 * UtilJ.h
 *
 *  Created on: Nov 4, 2010
 *      Author: jjacobs
 */

#ifndef UTILJ_H_
#define UTILJ_H_

#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <casa/Exceptions/Error.h>
#include <cassert>
#include <cstdlib>

// The Assert macro is an alias to the standard assert macro when NDEBUG is defined.  When
// NDEBUG is not defined (release build) then a throw is used to report the error.

#ifdef NDEBUG
#define Assert(c) assert (c)
#else
#define Assert(c) { throwIf (! (c), "Assertion failed: " #c, __FILE__, __LINE__); }
#endif

#if defined (NDEBUG)
#    define Throw(m) \
    { AipsError anAipsError ((m), __FILE__, __LINE__);\
      toStdErr (anAipsError.what());\
      throw anAipsError; }
#else
#    define Throw(m) throw AipsError ((m), __FILE__, __LINE__)
#endif

#define ThrowIf(c,m) casa::utilj::throwIf ((c), (m), __FILE__, __LINE__)

#define ThrowIfError(c,m) casa::utilj::throwIfError ((c), (m), __FILE__, __LINE__)

namespace casa {

class String;

namespace utilj {

class AipsErrorTrace : public AipsError {

public:

    AipsErrorTrace ( const String &msg, const String &filename, uInt lineNumber,
                     Category c = GENERAL);

};

template <typename Element, typename Container>
Bool
contains (const Element & e, const Container & c)
{
	return c.find(e) != c.end();
}

String format (const char * formatString, ...);

template<typename T>
T
getEnv (const String & name, const T & defaultValue)
{
	char * value = getenv (name.c_str());

	if (value == NULL){
		return defaultValue;
	}
	else{
		return T (value);
	}
}

Bool
getEnv (const String & name, const Bool & defaultValue);

Int
getEnv (const String & name, const Int & defaultValue);


String getTimestamp ();

Bool isEnvDefined (const String & name);
void printBacktrace (ostream & os, const String & prefix = "");
void sleepMs (Int milliseconds);
void toStdError (const String & m, const String & prefix = "*E* ");
void throwIf (Bool condition, const String & message, const String & file, Int line);
void throwIfError (Int errorCode, const String & prefix, const String & file, Int line);


} // end namespace utilj

} // end namespace casa


#endif /* UTILJ_H_ */
