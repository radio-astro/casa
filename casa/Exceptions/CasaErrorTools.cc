/*
 * CasaErrorTools.cc
 *
 *  Created on: May 10, 2012
 *      Author: jjacobs
 */


#include "CasaErrorTools.h"

// This symbol determines whether the
//
#define CasaErrorTools_SharedObjects_Enabled

// The shared objects functionality relies on a Linux specific API
// so supress the option if not on Linux

#if ! defined (AIPS_LINUX) & defined (CasaErrorTools_SharedObjects_Enabled)

#    undef CasaErrorTools_SharedObjects_Enabled

#endif


#if ! defined (CasaErrorTools_SharedObjects_Enabled)

// Stub out the related functions if functionality not enabled.

namespace casa {

void
CasaErrorTools::generateSharedObjectMap ()
{}

String
CasaErrorTools::replaceStackAddresses (const String & stackTrace)
{
    return "-->CasaErrorTools:: FYI: Could not replace stack addresses\n\n" + stackTrace;
}

} // end namespace casa


#else

#include <link.h>
#include <stdlib.h>
#include <stdio.h>

namespace {

std::map<casa::String, casa::uInt64> sharedObjectMap;

extern "C"
int
callback(struct dl_phdr_info *info, size_t, void *)
{
    sharedObjectMap [info->dlpi_name] = info->dlpi_addr;

    return 0;
}

} // end namespace UNNAMED

namespace casa {

void
CasaErrorTools::generateSharedObjectMap ()
{
    sharedObjectMap.clear();

    dl_iterate_phdr(callback, NULL);
}

String
CasaErrorTools::replaceStackAddresses (const String & stackTrace)
{

    if (sharedObjectMap.empty()){
        generateSharedObjectMap ();
    }

    String cleanedStackTrace;

    // Break the stack trace into lines by splitting at the '\n'

    string lines [500];
    int nSplits = split (stackTrace, lines, 500, "\n");

    for (int i = 0; i < nSplits; i++){

        string & line = lines [i];
        String cleanedLine;

        try {

            // Find the shared object name.

            if (line [0] != '/'){
                throw True;
            }

            size_t openParanthesis = line.find ("(");
            if (openParanthesis == String::npos){
                throw True;
            }

            String objectName = line.substr (0, openParanthesis);

            // Find the stack address.  This will be a hex number enclosed in square brackets.            // be relative to the start of the shared object.

            size_t leftSquare = line.find ("[");
            size_t rightSquare = line.find ("]");

            if (leftSquare == String::npos || rightSquare == String::npos ||
                    rightSquare <= leftSquare){
                throw True;
            }

            // Extract the address and subtract the base of the shared object's
            // starting address to produce the offset within the shared object.

            String addressText = line.substr (leftSquare + 1, rightSquare - leftSquare - 1);

            uInt64 address = strtoll (addressText.c_str(), NULL, 16);

            uInt64 objectBase = sharedObjectMap [objectName];

            uInt64 offset = address - objectBase;

            // Now rebuild the line replacing the original address ([0xHHHH...]) with
            // the offset (format [+0xHHHH...]

            char offsetInHex [128];

            snprintf (offsetInHex, 127, "0x%llx", offset);

            cleanedLine = line.substr (0, leftSquare) + "[+" + offsetInHex + "]";
        }
        catch (Bool){

            // The line was not parseable so just copy it to the result.

            cleanedLine = line;
        }

        // Put the cleaned line into the result.

        cleanedStackTrace += "\n" + cleanedLine;

    }

    return cleanedStackTrace;


}

} // end namespace casa

#endif
