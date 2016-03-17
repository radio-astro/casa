/*
 * CrashReporter.cc
 *
 *  Created on: Mar 16, 2016
 *      Author: jjacobs
 */

#include "CrashReporter.h"

#if ! defined (UseCrashReporter)

namespace casa {

bool
CrashReporter::initialize ()
{
    return false;
}

} // end namespace casa

#else

// Code to handle crash reporting

#include "client/linux/handler/exception_handler.h"
#include <casa/System/AipsrcValue.h>
#include <iostream>
using namespace std;

namespace {

bool
crashCallback (const google_breakpad::MinidumpDescriptor& descriptor,
               void* /*context*/,
               bool succeeded)
{
    cerr<<"*** entering callback; succeeded=" << succeeded << endl;

    pid_t pid = fork ();

    if (pid == 0){


        execl ("/users/jjacobs/bin/crashAux", "/users/jjacobs/bin/crashAux",
               descriptor.path(), "Argument2", nullptr);

        cerr << "*** execl failed: errno=" << errno << endl;

    } else if (pid == -1){

        cerr << "*** Failed to fork: errno=" << errno << endl;
    }

    cerr << "--> Now on to our untimely death ..." << endl;

    return succeeded;
}

google_breakpad::ExceptionHandler * exceptionHandler = nullptr;

} // end blank namespace

namespace casa {

bool
CrashReporter::initialize ()
{
    bool useCrashReporter = false;
    AipsrcValue<Bool>::find (useCrashReporter, String ("UseCrashReporter"), false);

    if (! useCrashReporter){
        return false;
    }

    google_breakpad::MinidumpDescriptor descriptor ("/home/orion/casa/testing/cdr");

    auto filter = [] (void *) -> bool {/* cerr << "... filter called" << endl; */ return true;};

    exceptionHandler =
            new google_breakpad::ExceptionHandler (descriptor,
                                                   filter,
                                                   crashCallback,
                                                   nullptr,
                                                   true,
                                                   -1 /*serverFd*/);

    return true;
}

} // end namespace casa

#endif


