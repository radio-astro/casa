#include <unistd.h>

#include <errno.h>
#include <fstream>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <sstream>
#include <string>
#include <string.h>
#include <unistd.h>
#include <vector>

using namespace std;


#define Throw(x) throw CrashException (__FILE__, __LINE__, x)
#define ThrowIf(c,x) { if (c) { Throw(x); } }
#define ThrowOrLogIf(c, doThrow, log, message) \
        { if (doThrow) { ThrowIf (c, message); } \
          else if (c) { log << "Exception in " << __FILE__ << ":" << __LINE__ << ":: " << (message) << endl; } }

class CrashException : public runtime_error {

public:

    CrashException (const string & file, int line, const string & message)
    : runtime_error (message),
      file_p (file),
      line_p (line),
      message_p (message)
    {
        ostringstream os;
        os << "Exception at " << file_p << ":" << line_p << ":: " << message_p;
        what_p = os.str();
    }

    const char * what () const noexcept override
    {
        return what_p.c_str();
    }

private:

    string file_p;
    int line_p;
    string message_p;
    string what_p;
};

class CrashReportPoster {

public:

    CrashReportPoster (int nArgs, char ** args);
    int run ();

private:

    void captureAdditionalInformation ();
    void captureOne (const string & command,
                     const string & filename);
    void cleanup ();
    void createArchiveFile ();
    bool doSystem (const string & command, bool throwOnError);
    void parseArguments (int nArgs, char ** arguments);
    void postArchiveToServer ();
    void setup ();


    string archiveFilename_p;
    string arguments_p;
    ofstream logStream_p;
    vector<string> manifest_p;
    string manifestString_p;
    string outputDirectory_p;
};

int
main (int nArgs, char ** args)
{
    CrashReportPoster crashReportPoster (nArgs, args);

    int status = crashReportPoster.run ();

    exit (status);
}

CrashReportPoster::CrashReportPoster (int nArgs, char ** args)
{
    parseArguments (nArgs, args);
}

void
CrashReportPoster::cleanup ()
{
    // Delete all the files

    doSystem ("rm " + manifestString_p, false);

    // Close out the log file entry.

    logStream_p << "------------------------------" << endl;
    logStream_p.close();
}

void
CrashReportPoster::captureAdditionalInformation ()
{
    // Capture various bits of information about the platform
    // and store them in files; add the names to the manifest.

    captureOne ("cat /proc/cpuinfo", "cpuinfo.txt");
    captureOne ("cat /proc/meminfo", "meminfo.txt");
    captureOne ("mount", "mountinfo.txt");
    captureOne ("lsb_release -a", "lsbinfo.txt");
    captureOne ("uname -a", "unameinfo.txt");
}

void
CrashReportPoster::captureOne (const string & command,
                               const string & filename)
{
    bool ok = doSystem (command + " > " + filename, false);
    if (ok) {
        manifest_p.push_back (filename);
    }
}

void
CrashReportPoster::createArchiveFile ()
{
    // Take the list of files in the manifest and join them in a string.

    auto joiner = [](const string & a, const string & b) { return a + " " + b;};
    manifestString_p = accumulate (manifest_p.begin(), manifest_p.end(), string(),
                                   joiner);

    // Create a gzipped tar of all of the files.

    archiveFilename_p += ".tgz";

    logStream_p << "Creating archive file: " << archiveFilename_p << endl;

    ostringstream os;
    os << "tar czf " << archiveFilename_p << " " << manifestString_p;

    doSystem (os.str(), true);

    logStream_p << "... done" << endl;
}

bool
CrashReportPoster::doSystem (const string & command, bool throwOnError)
{

    int status = system (command.c_str());

    bool ok = status != -1;
    ThrowOrLogIf (! ok, throwOnError, logStream_p,
                  string ("Call to system failed for '") + command + "': "
                  + strerror (errno));

    int code = WEXITSTATUS (status);
    ok =  code == 0;
    ThrowOrLogIf (! ok, throwOnError, logStream_p,
                  string ("Command failed: ") + strerror (code) +
                  "(code=" + std::to_string (code) +
                  ")\n...'" + command + "'");
    return ok;
}

void
CrashReportPoster::parseArguments (int nArgs, char ** args)
{
    arguments_p = (nArgs > 0) ? "" : "*NONE*";

    for (int i = 0; i < nArgs; i++){
        arguments_p += string (" ") + args [i];
    }

    ThrowIf (nArgs < 2, "At least one argument required.");

    // Add crash report to the manifest.

    string crashDumpFile = args [1];

    // Get the output directory by chopping everything preceding
    // the rightmost slash.

    size_t i = crashDumpFile.find_last_of ("/");
    ThrowIf (i == string::npos, "No '/' in path: " + crashDumpFile);

    outputDirectory_p = crashDumpFile.substr (0, i);

    // Extract the hash code that is the base part of the filename and
    // combine it with the path to get the name of the crash report
    // archive file that will be created.  The archiving logic
    // will install the proper suffix.

    string filename = crashDumpFile.substr (i+1); // all to right of slash
    manifest_p.push_back (filename); // without path
    i = filename.find (".");

    archiveFilename_p = filename.substr (0, i - 1);
}

void
CrashReportPoster::postArchiveToServer ()
{
    logStream_p << "Posting archived report to server" << endl;
}

int
CrashReportPoster::run ()
{
    int status = 0;

    try {
        setup ();

        captureAdditionalInformation ();

        createArchiveFile ();

        postArchiveToServer ();

    } catch (exception & e){

        status = -1;
    }

    cleanup ();

    return status;
}

void
CrashReportPoster::setup ()
{
    ThrowIf (chdir (outputDirectory_p.c_str()) != 0, string ("chdir failed: ") + strerror (errno));

    string logFilename = "CrashReporter.log";

    logStream_p.open (logFilename, ios::app);

    time_t seconds;
    struct tm * timeInfo;

    time (& seconds);
    timeInfo = localtime (& seconds);

    logStream_p << "++++++++++++++++++++++++++++++\nCrash Reporter: starting at "
                << asctime (timeInfo) << endl;

    logStream_p << "Arguments: " << arguments_p << endl;

}
