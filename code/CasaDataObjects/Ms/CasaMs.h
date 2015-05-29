/*
 * CasaMs.h
 *
 *  Created on: May 20, 2015
 *      Author: jjacobs
 */

#ifndef Casa_Cdo_CasaMs_H_
#define Casa_Cdo_CasaMs_H_

namespace casa {

namespace cdo {

class CasaMs {

public:

    const Antennas & getAntennas () const;
    Antennas & getAntennas ();

protected:

    CasaMs ();

};

} // end namespace cdo

} // end namespace casa

#endif // ! defined (Casa_Cdo_CasaMs_H_)
