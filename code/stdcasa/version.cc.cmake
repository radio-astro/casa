// This file is a template that is processed by shell commands during
// the make process.  Be very cautious about changing the parts in the
// first namespace block since the shell commands are fairly simple
// and can be confused by altering even one character in this section.

#include <stdcasa/version.h>

namespace casa {

const int    VersionInfo::aips_major_version = @CASA_MAJOR_VERSION@;
const int    VersionInfo::aips_minor_version = @CASA_MINOR_VERSION@;
const int    VersionInfo::aips_patch_version = @CASA_PATCH_VERSION@;

const char * VersionInfo::aips_version_date  = "@COMPILE_TIME@";
const char * VersionInfo::aips_version_info  = "";

} //# NAMESPACE CASA - END

#include <iostream>
#include <iomanip>

using namespace std;

namespace casa { //# NAMESPACE CASA

void report_aips_version(ostream &os)
{
   os << "CASA version: "
      << setfill('0')
      << setw(2)  << VersionInfo::aips_major_version
      << "."
      << setw(3)  << VersionInfo::aips_minor_version
      << "."
      << setw(2)  << VersionInfo::aips_patch_version
      << " "
      << setfill(' ')
      << setiosflags(ios::left)
      << setw(31) << VersionInfo::aips_version_date
      << "  "
      << setw(0)  << VersionInfo::aips_version_info
      << endl;
}

} //# NAMESPACE CASA - END
