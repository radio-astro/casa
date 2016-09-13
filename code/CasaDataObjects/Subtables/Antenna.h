/*
 * Antenna.h
 *
 *  Created on: Jul 11, 2013
 *      Author: jjacobs
 */

#ifndef ANTENNA_H_
#define ANTENNA_H_

#include "CasaTableRow.h"

#include <casa/BasicSL/String.h>
#include <casa/Arrays/Vector.h>

namespace casa {

class String;
template <typename T> class Vector;

namespace cdo {

class Antenna : public CasaTableRow {

public:

    Antenna ();
    Antenna (const Antenna & other);
    Antenna (int id,
             double dishDiameter,
             bool flagged,
             const String & mount,
             const String & name,
             const Vector<float> & offset,
             const Vector<float> & position,
             const String & station,
             const String & type);

    Antenna & operator= (const Antenna &) = delete;

    double dishDiameter () const;
    bool flagged () const;
    String name () const;
    String station () const;
    String type () const;
    String mount () const;
    const Vector<float> & position () const;
    const Vector<float> & offset () const;

    String toString () const;

protected:

private:

    double dishDiameter_p;
    bool flagged_p;
    String mount_p;
    String name_p;
    Vector<Float> offset_p;
    Vector<Float> position_p;
    String station_p;
    String type_p;
};

}
}




#endif /* ANTENNA_H_ */
