#include <iostream>

#include <libsakura/sakura.h>
//#include <libsakura/config.h>

#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>

#include <casa_sakura/SakuraUtils.h>
#include <singledish/SingleDish/SingleDishDummy.h>

namespace casa {
void SingleDishDummy::DummyMethod() {
  LogIO logger(LogOrigin("SingleDishDummy", "DummyMethod", WHERE));
  logger << LogIO::DEBUGGING << "Dummy Method of SingleDishDummy..." << LogIO::POST;
  //LIBSAKURA_SYMBOL(CleanUp)();
}

}  // End of casa namespace.
