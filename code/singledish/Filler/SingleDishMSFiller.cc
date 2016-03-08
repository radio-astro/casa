/*
 * Scantable2MSFiller.cc
 *
 *  Created on: Jan 5, 2016
 *      Author: nakazato
 */

#include <singledish/Filler/SingleDishMSFiller.h>

#include <singledish/Filler/Scantable2MSReader.h>

namespace casa { //# NAMESPACE CASA - BEGIN

PCMContext *g_context_p = nullptr;
DataRecord *g_storage_p = nullptr;

template class SingleDishMSFiller<Scantable2MSReader>;

} //# NAMESPACE CASA - END
