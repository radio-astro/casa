/*
 * Subtables.cc
 *
 *  Created on: May 4, 2015
 *      Author: jjacobs
 */

#include "Subtables.h"
#include "Subtable.h"
#include <casacore/casa/BasicSL.h>
#include <map>
#include <msvis/MSVis/UtilJ.h>
#include <ostream>

using namespace std;

using namespace casacore;
namespace casa {

namespace cdo {



class Subtables::Impl {

    // Pimpl class for Subtables.

public:

    typedef std::map<SubtableType, String> SubtablesRep;

    std::map<SubtableType, Subtable *> subtables_p;

    class CaselessCompare {

    public:

        bool operator () (String a, String b) const
        {
            a.downcase();
            b.downcase();
            return a < b;
        }
    };

    static std::map <SubtableType, String> typeToName_p;
    static std::map <String, SubtableType, CaselessCompare> nameToType_p;

    static map <SubtableType, String> initializeTypeToName ()
    {
        // Single place to provide starting values for both direction
        // lookup table

        map <SubtableType, String> result =
            {{SubtableType::Antenna, "Antenna"},
             {SubtableType::DataDescription, "DataDescription"},
             {SubtableType::Feed, "Feed"},
             {SubtableType::Field, "Field"},
             {SubtableType::Observation, "Observation"},
             {SubtableType::Pointing, "Pointing"},
             {SubtableType::Polarization, "Polarization"},
             {SubtableType::Processor, "Processor"},
             {SubtableType::SpectralWindow, "SpectralWindow"},
             {SubtableType::State, "State"},
             {SubtableType::Source, "Source"},
             {SubtableType::Syscal, "Syscal"}};

        return result;
    }

    static map <String, SubtableType, CaselessCompare> initializeNameToType ()
    {
        // The values are defined as type to name so we need to
        // create a name to type mapping.

        std::map <String, SubtableType, CaselessCompare> result;
        for (auto i : initializeTypeToName()){
            result [i.second] = i.first;
        }

        return result;
    }
};

std::map<SubtableType, String>
Subtables::Impl::typeToName_p = initializeTypeToName ();

std::map<String, SubtableType, Subtables::Impl::CaselessCompare>
Subtables::Impl::nameToType_p = initializeNameToType ();


Subtables::Subtables ()
: impl_p (new Subtables::Impl())
{}

Subtables::~Subtables ()
{
    // Destroy all of the subtables.

    for (auto i : impl_p->subtables_p)
    {
        delete i.second;
    }

    // Now destroy the implementation.

    delete impl_p;
}

void
Subtables::add (Subtable * subtable)
{
    SubtableType type = subtable->getType ();

    if (isPresent (type)){
        delete get (type);
    }

    set (type, subtable);
}

const Subtable *
Subtables::get (SubtableType subtableType) const
{
    ThrowIf (! isPresent (subtableType),
             String::format ("Subtables::get: table %s not present", typeToName (subtableType).c_str()));

    return impl_p->subtables_p [subtableType];
}

//Subtable *
//Subtables::get (SubtableType subtableType)
//{
//    ThrowIf (! isPresent (subtableType),
//             String::format ("Subtables::get: table %s not present", stToString (subtableType)));
//
//    return impl_p->subtables_p [subtableType];
//}

Bool
Subtables::isPresent (SubtableType subtableType) const
{
    Bool present = utilj::containsKey (subtableType, impl_p->subtables_p);
    present = present && impl_p->subtables_p [subtableType] != 0;
    return present;
}


SubtableType
Subtables::nameToType (String name)
{
    // Returns SubtableType::Unknown if no such type

    SubtableType result = SubtableType::Unknown;
    name.downcase ();

    try {
        result = Impl::nameToType_p.at (name);
    } catch (out_of_range & e){
        // Nothing to do
    }

    return result;
}


//void
//Subtables::set (SubtableType type, Subtable * subtable)
//{
//    if (isPresent (type)){
//        delete get (type);
//    }
//
//    impl_p->subtables_p [type] = subtable;
//}

uInt
Subtables::size () const
{
    return impl_p->subtables_p.size();
}

String
Subtables::typeToName (SubtableType type)
{
    // Returns "" if no such type

    String result = "";

    try {
        result = Impl::typeToName_p.at (type);
    } catch (out_of_range & e){
        // Nothing to do
    }

    return result;
}


} // end namespace cdo
} // end namespace casa
