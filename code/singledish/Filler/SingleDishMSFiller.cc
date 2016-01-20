/*
 * Scantable2MSFiller.cc
 *
 *  Created on: Jan 5, 2016
 *      Author: nakazato
 */

#include <singledish/Filler/SingleDishMSFiller.h>

#include <singledish/Filler/Scantable2MSReader.h>

namespace casa { //# NAMESPACE CASA - BEGIN

template class SingleDishMSFiller<Scantable2MSReader>;

} //# NAMESPACE CASA - END
