#ifndef CASA_STLIO_H
#define CASA_STLIO_H

//# Includes
#include <casa/aips.h>
#include <casa/BasicSL/String.h>

//# Forward declarations
#include <casa/iosfwd.h>

#include <set>
#include <vector>
#include <map>

namespace casa { //# NAMESPACE CASA - BEGIN

class AipsIO;
class LogIO;
class IPosition;
template<class T> class Array;
template<class T> class Matrix;
template<class T> class Vector;
template<class T> class Cube;
template<class T> class Block;


// <summary>
//    Input/output operators for standard template library containers.
// </summary>

// <use visibility=export>

// <etymology>
//  STLIO is shorthand for "standard template library input/output".
// </etymology>

// <synopsis> 
// These global functions provide easy input and output of standard template
// library containers.
// </synopsis>


// <example>
// <srcblock>
// std::pair<String, Float> pair;
// pair.first = "Hello";
// pair.second = 3.14;
// cout << "pair: " << pair;
// </srcblock>
//
// <motivation>
// Effortless input/output is clearly a big win.
// </motivation>
//

ostream &operator << (ostream &, const std::map<Int, uInt> &);

template<class T, class U> ostream &operator << (ostream &, const std::map<T, U> &);
template<class T, class U> ostream &operator << (ostream &, const std::pair<T, U> &);
template<class T> ostream &operator << (ostream &, const std::set<T> &);
template<class T> ostream &operator << (ostream &, const std::vector<T> &);

// Write a formatted copy of the array to the LogIO output object. Merely calls
// the ostream operator<< in turn.

template<class T, class U> LogIO &operator<<(LogIO &os, const std::map<T,U> &a);
template<class T, class U> LogIO &operator<<(LogIO &os, const std::pair<T,U> &a);
template<class T> LogIO &operator<<(LogIO &os, const std::set<T> &a);
template<class T> LogIO &operator<<(LogIO &os, const std::vector<T> &a);

} //# NAMESPACE CASA - END

#ifndef CASACORE_NO_AUTO_TEMPLATES
#include <casa/IO/STLIO.tcc>
#endif //# CASACORE_NO_AUTO_TEMPLATES
#endif
