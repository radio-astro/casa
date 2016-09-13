/*
 * Subtable.cc
 *
 *  Created on: May 4, 2015
 *      Author: jjacobs
 */


#include "Subtable.h"

#include <map>

using namespace std;

class Subtable::Impl {

public:

    static std::map <String, ST> types_p;
}


};


std::map<Subtable::ST, String>
Subtable::Impl::types_p = {{Subtable::ST::Antenna, "Antenna"},
                            {Subtable::ST::DataDescription, "DataDescription"},
                            {Subtable::ST::Feed, "Feed"},
                            {Subtable::ST::Field, "Field"},
                            {Subtable::ST::Observation, "Observation"},
                            {Subtable::ST::Pointing, "Pointing"},
                            {Subtable::ST::Polarization, "Polarization"},
                            {Subtable::ST::Processor, "Processor"},
                            {Subtable::ST::SpectralWindow, "SpectralWindow"},
                            {Subtable::ST::State, "State"},
                            {Subtable::ST::Source, "Source"},
                            {Subtable::ST::Syscal, "Syscal"}};
std::ostream &
operator<< (std::ostream & os, Subtables::ST type)
{
    os << Subtables::stToString (type);

    return os;
}
