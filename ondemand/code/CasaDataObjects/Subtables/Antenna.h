/*
 * Antenna.h
 *
 *  Created on: Jul 11, 2013
 *      Author: jjacobs
 */

#ifndef ANTENNA_H_
#define ANTENNA_H_

#include "CasaTableRow.h"

namespace casa {

class String;
template <typename T> class Vector;

namespace cdo {

class Antenna : public CasaTableRow {

public:

    Antenna ();
    Antenna (const Antenna & other);
    Antenna (Double dishDiameter_p,
             Bool flagged_p,
             const String & mount_p,
             const String & name_p,
             const Vector<Float> & offset_p,
             const Vector<Float> & position_p,
             const String & station_p,
             const String & type_p);

    Antenna & operator= (const Antenna & other);

    Double dishDiameter () const;
    Bool flagged () const;
    String name () const;
    String station () const;
    String type () const;
    String mount () const;
    const Vector<Float> & position () const;
    const Vector<Float> & offset () const;

    void setDishDiameter (Double);
    void setFlagged (Bool);
    void setName (const String &);
    void setStation (const String &);
    void setType (const String &);
    void setMount (const String &);
    void setPosition (const Vector<Float> &);
    void setOffset (const Vector<Float> &);

protected:

private:

    Double dishDiameter_p;
    Bool flagged_p;
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
