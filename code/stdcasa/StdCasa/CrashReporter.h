/*
 * CrashReporter.h
 *
 *  Created on: Mar 16, 2016
 *      Author: jjacobs
 */

#ifndef STDCASA_STDCASA_CRASHREPORTER_H_
#define STDCASA_STDCASA_CRASHREPORTER_H_

#include <string>

namespace casa {

class CrashReporter {

public:

    static std::string initialize (const std::string & crashDumpDirectory,
                                   const std::string & crashDumpPosterApplication,
                                   const std::string & crashPostingUrl,
                                   const std::string & logFile);

    static std::string initializeFromApplication (const char * applicationArg0);

};

} // end namespace casa

#endif /* STDCASA_STDCASA_CRASHREPORTER_H_ */
