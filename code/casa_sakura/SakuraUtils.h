#ifndef _CASA_SAKURA_UTILS_H_
#define _CASA_SAKURA_UTILS_H_

#include <iostream>
#include <string>

//#include <libsakura/sakura.h>

#include <casa/aipstype.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class SakuraUtils {
public:
  static Bool InitializeSakura();
  static void CleanUpSakura();
  static Bool IsSakuraInitialized();
private:
  static Bool is_initialized_;
};

class SakuraDriver {
public:
  SakuraDriver();
  ~SakuraDriver();
private:
  static int num_instances_;
};    

} //# NAMESPACE CASA - END
  
#endif /* _CASA_SAKURA_UTILS_H_ */
