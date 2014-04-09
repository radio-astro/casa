#ifndef _CASA_SAKURA_UTILS_H_
#define _CASA_SAKURA_UTILS_H_

#include <iostream>

//#include <libsakura/sakura.h>

#include <casa/aipstype.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class SakuraUtils {
public:
  static Bool InitializeSakura();
  static void CleanUpSakura();
  static Bool IsSakuraInitialized();
};

} //# NAMESPACE CASA - END
  
#endif /* _CASA_SAKURA_UTILS_H_ */
