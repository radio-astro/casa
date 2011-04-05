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
#include <sys/time.h>
#include <sys/resource.h>

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

class Times {

public:

    Times () { * this = getTime();}
    Double cpu () const { return cpu_p;}
    Double elapsed () const { return elapsed_p;}

    static Times
    getTime (){

        struct timeval tVal;
        gettimeofday (& tVal, NULL);

        Double elapsed = tVal.tv_sec + tVal.tv_usec * 1e-6;

        //Double cpu = ((Double) clock ()) / CLOCKS_PER_SEC; // should be in seconds

        struct rusage usage;

        int failed = getrusage (RUSAGE_THREAD, & usage);
        assert (! failed);

        Double cpu = toSeconds (usage.ru_utime) + toSeconds (usage.ru_stime);

        return Times (elapsed, cpu);
    }

    static Double
    toSeconds (const struct timeval & t)
    {
        return t.tv_sec + t.tv_usec * 1e-6;
    }

protected:

    Double cpu_p;
    Double elapsed_p;

    Times (Double elapsed, Double cpu) : cpu_p (cpu), elapsed_p (elapsed) {}
};

class DeltaTimes : private Times {

    friend DeltaTimes operator- (const Times & tLater, const Times & tEarlier);

public:

    DeltaTimes () : Times (0, 0), doStats_p (False), n_p (0) {}
    explicit DeltaTimes (bool doStats) : Times (0,0), doStats_p (doStats), n_p (0)
    {
        cpuSsq_p = 0;
        cpuMin_p = 1e20;
        cpuMax_p = -1e20;
        elapsedSsq_p = 0;
        elapsedMin_p = 1e20;
        elapsedMax_p = -1e20;
    }

    DeltaTimes & operator += (const DeltaTimes & other);

    Double cpu () const { return Times::cpu();}
    Double cpuAvg () const { return n_p == 0 ? 0 : cpu() / n_p;}
    Double elapsed () const { return Times::elapsed();}
    Double elapsedAvg () const { return n_p == 0 ? 0 : elapsed() / n_p;}
    String formatAverage (const String & floatFormat = "%6.1f",
                          Double scale=1000.0,
                          const String & units = "ms")  const; // to convert to ms
    String formatStats (const String & floatFormat = "%6.1f",
                        Double scale=1000.0,
                        const String & units = "ms")  const; // to convert to ms
    Int n() const { return n_p;}

protected:

    DeltaTimes (Double elapsed, Double cpu) : Times (elapsed, cpu), n_p (0) {}

private:

    Double cpuMin_p;
    Double cpuMax_p;
    Double cpuSsq_p;
    Bool doStats_p;
    Double elapsedMin_p;
    Double elapsedMax_p;
    Double elapsedSsq_p;
    Int n_p;
};

DeltaTimes operator- (const Times & tLater, const Times & tEarlier);

} // end namespace utilj

} // end namespace casa


#endif /* UTILJ_H_ */
