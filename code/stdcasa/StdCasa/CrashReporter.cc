/*
 appli* CrashReporter.cc
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

using namespace casacore;
namespace casa {

std::string
CrashReporter::initialize (const std::string &, const std::string &, const std::string &)
{
    return "";
}

std::string
CrashReporter::initializeFromApplication (const char *)
{
    // This is a stub (duh!)

    return "";
}

using namespace casacore;
} // end namespace casa

#else

// Real code to handle crash reporting

#if  defined (__APPLE__)
#include <client/mac/handler/exception_handler.h>
#else
#include <client/linux/handler/exception_handler.h>
#endif

#include <iostream>
#include <regex>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>

#undef check

#include <casa/OS/File.h>
#include <casa/System/AipsrcValue.h>

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
crashCallbackCommon (const char * dumpPath,
                     bool succeeded)
{
    // Start the crash poster executable by using a fork.  The crash dump
    // has already been written to disk before this function is called.
    // The crash poster will grab any other info desired, tar it up and
    // post it to the server via HTTP.

    pid_t pid = fork ();

    if (pid == 0){

        // This is the new process; replace the current process image with
        // the crashPoster executable image.
        //
        // First make the process the leader of a new session so that we don't
        // die when the parent process does (and that's likely to happen
        // pretty quickly).

        setsid (); // Create/join new session

        execl (crashDumpPoster.c_str(),       // File to run
               crashDumpPoster.c_str(),       // Argument 0
               dumpPath, // Argument #1
               crashUrl.c_str(),
               nullptr);          // No more arguments

       // If the execl call succeeds then control will never reach here
       // since the new executable image should have replaced this one.

        cerr << "*** execl failed: errno=" << errno << endl;

    } else if (pid == -1){

        cerr << "*** Failed to fork: errno=" << errno << endl;
    } else {

        // Wait for the crash poster process to finish otherwise exiting will
        // likely kill the poster application, too.

        pid_t child = waitpid (pid, nullptr, 0);
        if (child == -1){
            cerr << "*** waitpid failed: errno= << errno << endl";
        }
    }

    // Only the original process gets to here.  Exiting the routine will
    // cause the original signal to terminate the the process.

    cerr << "--> Now on to our untimely death ..." << endl;

    return succeeded;
}

#if defined (__APPLE__)
bool crashCallback (const char * dumpPath,
                    const char * /*minidump_id*/,
                    void * /*context*/,
                    bool succeeded)
{
    return crashCallbackCommon (dumpPath, succeeded);
}

#else // Linux

bool
crashCallback (const google_breakpad::MinidumpDescriptor& descriptor,
               void* /*context*/,
               bool succeeded)
{
    return crashCallbackCommon (descriptor.path(), succeeded);
}
#endif


// The breakpad exceptionHandler has to be alive at the time the crash occurs.
// Keeping it in a global does the trick since the handler should stay active
// until the end of the process.

google_breakpad::ExceptionHandler * exceptionHandler = nullptr;

} // end unnamed namespace

using namespace casacore;
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

    crashDumpPoster = crashDumpPosterApplication;

    crashUrl = crashPostingUrl;
    if (crashUrl.empty()){
        String s;
        AipsrcValue<String>::find (s, String ("CrashReportUrl"),
                                   String ("https://casa.nrao.edu/cgi-bin/crash-report.pl"));
        crashUrl = s;
    }

    auto filter = [] (void *) -> bool {/* cerr << "... filter called" << endl; */ return true;};

    // Dynamically allocate a breakpad exception handler.  The object needs to kept alive for
    // the duration of the session (hence the "new").

#if defined (__APPLE__)

    exceptionHandler =
            new google_breakpad::ExceptionHandler (crashDumpDirectory,
                                                   filter,
                                                   crashCallback,
                                                   nullptr,
                                                   true,
                                                   nullptr /*serverFd*/);
#else
    google_breakpad::MinidumpDescriptor descriptor (crashDumpDirectory);

    exceptionHandler =
            new google_breakpad::ExceptionHandler (descriptor,
                                                   filter,
                                                   crashCallback,
                                                   nullptr,
                                                   true,
                                                   -1 /*serverFd*/);
#endif

cerr << "--> CrashReporter initialized." << endl << flush;

    return "";
}

std::string
CrashReporter::initializeFromApplication (const char * applicationArg0)
{
    // The single argument to this method is the zeroeth argument passed to the
    // main function in the application.  The path portion of arg0 will be extracted
    // and used to find the bin directory and thereby the crash dump poster application.

    // Convert the provided path to an absolute one with all symbolic links expanded.

    char exePathBuffer [4096];
    char * status = realpath (applicationArg0, exePathBuffer);
    String exePath = (status != 0) ? exePathBuffer : "";

    if (exePath.empty()){

        // No path from arg0 so use the first chunk of CASAPATH

        char * s = getenv ("CASAPATH");
        exePath = (s) ? s : "";

        // Replace a string of contiguous spaces (should be one of these) with
        // a "slash".
        std::regex spaces (" +");
        exePath = regex_replace (exePath, spaces, "/");

        exePath += "/bin/bogusExe";

    }

    // Extract the path portion of the file spec.

    std::cmatch match;
    std::string binPath;
    std::regex re ("(.*/)[^/]*$"); // Match everything through the last slash

    if (std::regex_match (exePath.c_str(), match, re)){
        binPath = match.str(1);
    } else {
        return String::format ("CrashReporter could not find path in '%s'", exePath.c_str());
    }

    // Determine the temporary directory.  On Mac and sometimes on linux this will be
    // contained in the TMPDIR environment variable.  If that's not defined then use /tmp.

    char * tmpDir = getenv ("TMPDIR");
    string crashReportDirectory = (tmpDir) ? tmpDir : "";
    if (crashReportDirectory.empty()){
        crashReportDirectory = "/tmp";
    }

    string crashReportPoster = binPath + "CrashReportPoster";

    // Look up the crash report posting URL in the .casarc file.

    String crashReportUrl;
    AipsrcValue<String>::find (crashReportUrl, "CrashReporter.url", String ());

    return casa::CrashReporter::initialize (crashReportDirectory, crashReportPoster, crashReportUrl);
}


using namespace casacore;
} // end namespace casa

#endif


