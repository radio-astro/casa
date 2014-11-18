#include <iostream>

#include <libsakura/sakura.h>
//#include <libsakura/config.h>

#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>

#include <casa_sakura/SakuraUtils.h>
#include <casa_sakura/SakuraAlignedArray.h>

namespace casa {
Bool SakuraUtils::is_initialized_ = False;
  
Bool SakuraUtils::InitializeSakura(const std::string &level) {
  // return if already initialized
  if (is_initialized_) {
    return True;
  }
  
  LogIO logger(LogOrigin("SakuraUtils", "InitializeSakura", WHERE));
  logger << LogIO::DEBUGGING << "Initializing Sakura...";
  is_initialized_ = False;
  LIBSAKURA_SYMBOL(Status) status = LIBSAKURA_SYMBOL(Initialize)(NULL, NULL);
  if (status == LIBSAKURA_SYMBOL(Status_kOK)) {
    logger << LogIO::DEBUGGING << "SUCCESS!" << LogIO::POST;
    is_initialized_ = True;
  }
  else {
    logger << LogIO::DEBUGGING << "FAILED!" << LogIO::POST;
  }
  return is_initialized_;
}

void SakuraUtils::CleanUpSakura() {
  if (is_initialized_) {
    LogIO logger(LogOrigin("SakuraUtils", "CleanUpSakura", WHERE));
    logger << LogIO::DEBUGGING << "Cleaning up Sakura..." << LogIO::POST;
    LIBSAKURA_SYMBOL(CleanUp)();
    is_initialized_ = False;
  }
}

Bool SakuraUtils::IsSakuraInitialized() {
  LogIO logger(LogOrigin("SakuraUtils", "IsSakuraInitialized", WHERE));
  logger << LogIO::DEBUGGING << "sakura is "
	 << ((is_initialized_) ? "" : "not ") << "initialized." << LogIO::POST;
  return is_initialized_;
}

}  // End of casa namespace.
