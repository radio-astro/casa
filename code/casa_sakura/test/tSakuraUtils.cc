#include <iostream>

#include <casa_sakura/SakuraUtils.cc>

int main(int argc, char *argv[]) {

  // Initialize Sakura
  casa::Bool initialize_success = casa::SakuraUtils::InitializeSakura();

  // Query if Sakura is initialized
  casa::Bool is_sakura_initialized = casa::SakuraUtils::IsSakuraInitialized();

  // Clean up Sakura
  casa::SakuraUtils::CleanUpSakura();
  
  return 0;
}
