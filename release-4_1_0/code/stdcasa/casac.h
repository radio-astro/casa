//==============================================================================
// casac - python module definition - <MModuleDef>
//
//Stolen from the old CCM tools implementation
//
//==============================================================================
#ifndef _casac_h
#define _casac_h

#include <stdarg.h>
#include <vector>

#define USING_NUMPY_ARRAYS 1

namespace casac {

    template<class T>
    std::vector<T> initialize_vector(int count, T v1, ...) {
        va_list ap;
	va_start(ap, v1);
	std::vector<T> result(count);
	result[0] = v1;
	for ( int i=1; i < count; ++i ) {
	    T val = va_arg(ap,T);
	    result[i] = val;
	}
	return result;
    }

} // casac namespace

#endif

