/*
 * Antenna.h
 *
 *  Created on: Jul 11, 2013
 *      Author: jjacobs
 */

#ifndef ANTENNA_H_
#define ANTENNA_H_

#include "CasaTableRow.h"

<<<<<<< .mine
namespace casacore{
=======
#include <casa/BasicSL/String.h>
#include <casa/Arrays/Vector.h>

namespace casa {
>>>>>>> .r38222

class String;
template <typename T> class Vector;
}

namespace casa {


namespace cdo {

class Antenna : public CasaTableRow {

public:

    Antenna ();
    Antenna (const Antenna & other);
<<<<<<< .mine
    Antenna (casacore::Double dishDiameter_p,
             casacore::Bool flagged_p,
             const casacore::String & mount_p,
             const casacore::String & name_p,
             const casacore::Vector<casacore::Float> & offset_p,
             const casacore::Vector<casacore::Float> & position_p,
             const casacore::String & station_p,
             const casacore::String & type_p);
=======
    Antenna (int id,
             double dishDiameter,
             bool flagged,
             const String & mount,
             const String & name,
             const Vector<float> & offset,
             const Vector<float> & position,
             const String & station,
             const String & type);
>>>>>>> .r38222

    Antenna & operator= (const Antenna &) = delete;

<<<<<<< .mine
    casacore::Double dishDiameter () const;
    casacore::Bool flagged () const;
    casacore::String name () const;
    casacore::String station () const;
    casacore::String type () const;
    casacore::String mount () const;
    const casacore::Vector<casacore::Float> & position () const;
    const casacore::Vector<casacore::Float> & offset () const;
=======
    double dishDiameter () const;
    bool flagged () const;
    String name () const;
    String station () const;
    String type () const;
    String mount () const;
    const Vector<float> & position () const;
    const Vector<float> & offset () const;
>>>>>>> .r38222

<<<<<<< .mine
    void setDishDiameter (casacore::Double);
    void setFlagged (casacore::Bool);
    void setName (const casacore::String &);
    void setStation (const casacore::String &);
    void setType (const casacore::String &);
    void setMount (const casacore::String &);
    void setPosition (const casacore::Vector<casacore::Float> &);
    void setOffset (const casacore::Vector<casacore::Float> &);
=======
    String toString () const;
>>>>>>> .r38222

protected:

private:

<<<<<<< .mine
    casacore::Double dishDiameter_p;
    casacore::Bool flagged_p;
    casacore::String mount_p;
    casacore::String name_p;
    casacore::Vector<casacore::Float> offset_p;
    casacore::Vector<casacore::Float> position_p;
    casacore::String station_p;
    casacore::String type_p;
=======
    double dishDiameter_p;
    bool flagged_p;
    String mount_p;
    String name_p;
    Vector<Float> offset_p;
    Vector<Float> position_p;
    String station_p;
    String type_p;
>>>>>>> .r38222
};

}
}




#endif /* ANTENNA_H_ */
