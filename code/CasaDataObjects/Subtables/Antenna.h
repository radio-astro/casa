/*
 * Antenna.h
 *
 *  Created on: Jul 11, 2013
 *      Author: jjacobs
 */

#ifndef ANTENNA_H_
#define ANTENNA_H_

#include "CasaTableRow.h"

namespace casacore{

class String;
template <typename T> class Vector;
}

namespace casa {


namespace cdo {

class Antenna : public CasaTableRow {

public:

    Antenna ();
    Antenna (const Antenna & other);
    Antenna (casacore::Double dishDiameter_p,
             casacore::Bool flagged_p,
             const casacore::String & mount_p,
             const casacore::String & name_p,
             const casacore::Vector<casacore::Float> & offset_p,
             const casacore::Vector<casacore::Float> & position_p,
             const casacore::String & station_p,
             const casacore::String & type_p);

    Antenna & operator= (const Antenna & other);

    casacore::Double dishDiameter () const;
    casacore::Bool flagged () const;
    casacore::String name () const;
    casacore::String station () const;
    casacore::String type () const;
    casacore::String mount () const;
    const casacore::Vector<casacore::Float> & position () const;
    const casacore::Vector<casacore::Float> & offset () const;

    void setDishDiameter (casacore::Double);
    void setFlagged (casacore::Bool);
    void setName (const casacore::String &);
    void setStation (const casacore::String &);
    void setType (const casacore::String &);
    void setMount (const casacore::String &);
    void setPosition (const casacore::Vector<casacore::Float> &);
    void setOffset (const casacore::Vector<casacore::Float> &);

protected:

private:

    casacore::Double dishDiameter_p;
    casacore::Bool flagged_p;
    casacore::String mount_p;
    casacore::String name_p;
    casacore::Vector<casacore::Float> offset_p;
    casacore::Vector<casacore::Float> position_p;
    casacore::String station_p;
    casacore::String type_p;
};

}
}




#endif /* ANTENNA_H_ */
