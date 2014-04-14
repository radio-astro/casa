#ifndef _CASA_SAKURA_UTILS_H_
#define _CASA_SAKURA_UTILS_H_

#include <iostream>
#include <string>

//#include <libsakura/sakura.h>

#include <casa/aipstype.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class SakuraUtils {
public:
  static Bool InitializeSakura(const std::string &loglevel="WARN");
  static void CleanUpSakura();
  static Bool IsSakuraInitialized();
  static void SetLogLevel(const std::string &loglevel);
private:
  static Bool is_initialized_;
};

} //# NAMESPACE CASA - END
  
#endif /* _CASA_SAKURA_UTILS_H_ */
