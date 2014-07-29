/*
 * CasaErrorTools.h
 *
 *  Created on: May 10, 2012
 *      Author: jjacobs
 */

#ifndef CASAERRORTOOLS_H_
#define CASAERRORTOOLS_H_

#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <map>

namespace casa {

class CasaErrorTools {

public:

    static String replaceStackAddresses (const String & stackTrace);

private:

    static void generateSharedObjectMap ();

};

} // end namespace casa

#endif /* CASAERRORTOOLS_H_ */
