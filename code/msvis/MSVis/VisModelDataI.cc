#include "VisModelDataI.h"
#include <casa/Exceptions.h>

namespace casa {

VisModelDataI::Factory VisModelDataI::factory_p = 0;

VisModelDataI *
VisModelDataI::create ()
{
  ThrowIf (factory_p == 0, "No VisModelDataI::factory available");

  return factory_p ();
}

bool 
VisModelDataI::setFactory (Factory factory)
{
  factory_p = factory;

  return True;
}

}
