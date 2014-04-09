#include <iostream>

// #include <libsakura/sakura.h>

#include <casa_sakura/SakuraUtils.h>

namespace casa {
Bool SakuraUtils::InitializeSakura() {
  std::cout << "Initializing Sakura..." << std::endl;
  Bool return_value = False;
  // sakura_Status status = sakura_Initialize(NULL, NULL);
  // if (status == sakura_Status_kOK) {
  //   std::cout << "SUCCESS" << std::endl;
  //   return_value = True;
  // }
  // else {
  //   std::cout << "FAILED" << std::endl;
  // }
  return_value = True;
  return return_value;
}

void SakuraUtils::CleanUpSakura() {
  std::cout << "Cleaning up Sakura..." << std::endl;
}

Bool SakuraUtils::IsSakuraInitialized() {
  std::cout << "Sakura is initialized" << std::endl;
  // sakura_CleanUp();
  return True;
}
}  // End of casa namespace.
