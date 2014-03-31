#ifndef _CASA_SAKURA_UTILS_H_
#define _CASA_SAKURA_UTILS_H_

#include <iostream>

#include <casa/aipstype.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class SakuraUtils {
public:
  static Bool InitializeSakura();
  static void CleanUpSakura();
  static Bool IsSakuraInitialized();
};


inline Bool SakuraUtils::InitializeSakura() {
  std::cout << "Initializing Sakura..." << std::endl;
  return True;
}

inline void SakuraUtils::CleanUpSakura() {
  std::cout << "Cleaning up Sakura..." << std::endl;
}

inline Bool SakuraUtils::IsSakuraInitialized() {
  std::cout << "Sakura is initialized" << std::endl;
  return True;
}
  
} //# NAMESPACE CASA - END
  
#endif /* _CASA_SAKURA_UTILS_H_ */
