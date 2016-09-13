/*
 * Antennas.cc
 *
 *  Created on: Jan 14, 2014
 *      Author: jjacobs
 */


#include "Antennas.h"

#include "Antenna.h"
#include "SubtableFiller.h"
#include <casa/Exceptions.h>
#include <tables/Tables.h>
#include <casa/BasicSL/String.h>
#include <ms/MeasurementSets/MeasurementSet.h>

namespace casa { namespace cdo {

AntennasFiller::AntennasFiller (Table & table, Antennas & antennas)
: SubtableFiller <Antenna> (),
  table_p (table),
  antennas_p (antennas)
{
    addPickler <ScalarColumn <String> >  (MSAntenna::MOUNT, & Antenna::mount, & Antenna::setMount);
    addPickler <ArrayColumn <Double> > (MSAntenna::POSITION, & Antenna::position, & Antenna::setPosition);
}


}}


