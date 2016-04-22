/*
 * CrashReporter.cc
 *
 *  Created on: Mar 16, 2016
 *      Author: jjacobs
 */

#include "CrashReporter.h"

//#warning "Defining UseCrashReport to compile main code"
//#define UseCrashReporter

#if ! defined (UseCrashReporter)

// Stubs to allow safely eliminating the breakpad/crash-reporting functionality from
// CASA by the use of the above build time symbol.

namespace casa {

std::string
CrashReporter::initialize (const std::string &, const std::string &, const std::string &)
{
    return "";
}

std::string
CrashReporter::initializeFromApplication (const char * arg0)
{
    // This is a stub (duh!)

    return "";
}

} // end namespace casa

#else

// Real code to handle crash reporting

#if  defined (__APPLE__)
#include "exception_handler_mac.h"
#else
#include "exception_handler_linux.h"
#endif

#include <casa/OS/File.h>
#include <casa/System/AipsrcValue.h>
#include <iostream>
#include <regex>
#include <string>

using namespace std;

namespace {  // Unnamed namespace to limit the scope

// The callback routine, crashCallback, has its arguments set so these two
// limited scope globals will be used to pass information to the callback.
// The passed arguments will include the breakpad crash report file so
// that need not be passed through the globals.

string crashDumpPoster; // Path to the crash report posting application
string crashUrl; // Url to post the crash dump to

// crashCallback - callback routine called by breakpad after the crash
// dump has been successfully created.  The routine will execute a fork
// and then execl in favor of the crash Dump poster application; that
// application will collect other system information, bundle all information
// up and post it to an HTTP server.

bool
crashCallback (const google_breakpad::MinidumpDescriptor& descriptor,
               void* /*context*/,
               bool succeeded)
{
    cerr<<"*** entering callback; succeeded=" << succeeded << endl;

    // Start the crash poster executable by using a fork.  The crash dump
    // has already been written to disk before this function is called.
    // The crash poster will grab any other info desired, tar it up and
    // post it to the server via HTTP.

    pid_t pid = fork ();

    if (pid == 0){

        // This is the new process; replace the current process image with
        // the crashPoster executable image.

        execl (crashDumpPoster.c_str(),       // File to run
               crashDumpPoster.c_str(),       // Argument 0
               descriptor.path(), // Argument #1
               crashUrl.c_str(),
               nullptr);          // No more arguments

       // If the execl call succeeds then control will never reach here
       // since the new executable image should have replaced this one.

        cerr << "*** execl failed: errno=" << errno << endl;

    } else if (pid == -1){

        cerr << "*** Failed to fork: errno=" << errno << endl;
    }

    // Only the original process gets to here.  Exiting the routine will
    // cause the original signal to terminate the the process.

    cerr << "--> Now on to our untimely death ..." << endl;

    return succeeded;
}

// The breakpad exceptionHandler has to be alive at the time the crash occurs.
// Keeping it in a global does the trick since the handler should stay active
// until the end of the process.

google_breakpad::ExceptionHandler * exceptionHandler = nullptr;

} // end unnamed namespace

namespace casa {

string
CrashReporter::initialize (const string & crashDumpDirectory,
                           const string & crashDumpPosterApplication,
                           const string & crashPostingUrl)
{
    bool useCrashReporter = false;
    AipsrcValue<Bool>::find (useCrashReporter, String ("UseCrashReporter"), false);

    if (! useCrashReporter){
        return "";
    }

    // Validate the directory and application provided.

    File cdd (crashDumpDirectory);

    if (! cdd.exists() || ! cdd.isDirectory()){
        return String::format ("Crash dump path '%s' does not exist or is not a directory.",
                               crashDumpDirectory.c_str());
    }

    if (! cdd.isWritable()){
        return String::format ("Crash dump directory '%s' is not writable.", crashDumpDirectory.c_str());
    }

    File cpa (crashDumpPosterApplication);

    if (! cpa.exists()){
        return String::format ("Crash dump poster application '%s' does not exist.",
                               crashDumpPosterApplication.c_str());
    }

    if (! cpa.isExecutable()){
        return String::format ("Crash dump poster application '%s' is not executable.",
                               crashDumpPosterApplication.c_str());
    }

    google_breakpad::MinidumpDescriptor descriptor (crashDumpDirectory);
    crashDumpPoster = crashDumpPosterApplication;
    crashUrl = crashPostingUrl;

    auto filter = [] (void *) -> bool {/* cerr << "... filter called" << endl; */ return true;};

    // Dynamically allocate a breakpad exception handler.  The object needs to kept alive for
    // the duration of the session (hence the "new").

    exceptionHandler =
            new google_breakpad::ExceptionHandler (descriptor,
                                                   filter,
                                                   crashCallback,
                                                   nullptr,
                                                   true,
                                                   -1 /*serverFd*/);

    return "";
}

std::string
CrashReporter::initializeFromApplication (const char * applicationArg0)
{
    // The single argument to this method is the zeroeth argument passed to the
    // main function in the applicaiton.  The path portion of arg0 will be extracted
    // and used to find the bin directory and thereby the crash dump poster application.

    // Convert the provided path to an absolute one with all symbolic links expanded.

    char exePath [4096];
    char * status = realpath (applicationArg0, exePath);

    if (status == 0){

        return format ("CrashReporter call to realpath during initialization failed: %s",
                       strerror (errno));
    }

    // Extract the path portion of the file spec.

    std::cmatch match;
    std::string binPath;
    std::regex re ("(.*/)[^/]*$"); // Match everything through the last slash

    if (std::regex_match (exePath, match, re)){
        binPath = match.str(1);
    } else {
        return format ("CrashReporter could not find path in '%s'", exePath);
    }

    // Determine the temporary directory.  On Mac and sometimes on linux this will be
    // contained in the TMPDIR environment variable.  If that's not defined then use /tmp.

    string crashReportDirectory = getenv ("TMPDIR");
    if (crashReportDirectory.empty()){
        crashReportDirectory = "/tmp";
    }

    string crashReportPoster = binPath + "CrashReportPoster";

    // Look up the crash report posting URL in the .casarc file.

    string crashReportUrl;
    AipsrcValue<String>::find (crashReportUrl, "CrashReporter.url", "");

    return casa::CrashReporter::initialize (crashReportDirectory, crashReportPoster, crashReportUrl);
}


} // end namespace casa

#endif


