/*
 * Antennas.h
 *
 *  Created on: Jul 11, 2013
 *      Author: jjacobs
 */

#ifndef ANTENNAS_H_
#define ANTENNAS_H_

#include "Antenna.h"
#include "Subtable.h"

namespace casa {

namespace cdo {

class Antennas : public Subtable <Antenna> {

public:

    Antennas ();
    Antennas (const Antennas & other);

};

class AntennasFiller : SubtableFiller <Antennas> {

public:

    AntennasFiller (Table & table, Antennas & antennas);

    void fill ();

private:

    Antennas & antennas_p;
    Table & table_p;

};

}
}




#endif /* ANTENNAS_H_ */
