/*
 * Subtables.cc
 *
 *  Created on: May 4, 2015
 *      Author: jjacobs
 */

#include "Subtables.h"
#include <casacore/casa/BasicSL.h>
#include <map>
#include <msvis/MSVis/UtilJ.h>
#include <ostream>

using namespace std;

namespace casa {

namespace cdo {


class Subtables::Impl {

public:

    typedef std::map<Subtables::ST, String> SubtablesRep;

    std::map<ST, Subtable *> subtables_p;
    static std::map <String, ST> types_p;
}



Subtables::Subtables ()
: impl_p (new Subtables::Impl())
{}

Subtables::~Subtables ()
{
    for (Impl::SubtablesRep::iterator i = impl_p->subtables_p.begin();
         i != impl_p->subtables_p.end();
         i++){

        delete i->second;
    }

    delete impl_p;
}

void
Subtables::add (Subtable * subtable)
{
    ST type = subtable->getType ();

    if (isPresent (type)){
        delete get (type);
    }

    set (type, subtable);
}

const Subtable *
Subtables::get (ST subtableType) const
{
    ThrowIf (! isPresent (subtableType),
             String::format ("Subtables::get: table %s not present", stToString (subtableType)));

    return impl_p->subtables_p [subtableType];
}

Subtable *
Subtables::get (ST subtableType)
{
    ThrowIf (! isPresent (subtableType),
             String::format ("Subtables::get: table %s not present", stToString (subtableType)));

    return impl_p->subtables_p [subtableType];
}

Bool
Subtables::isPresent (ST subtableType) const
{
    Bool present = utilj::containsKey (subtableType, impl_p->subtables_p);
    present = present && impl_p->subtables_p [subtableType] != 0;
    return present;
}

void
Subtables::set (ST type, Subtable * subtable)
{
    if (isPresent (type)){
        delete get (type);
    }

    impl_p->subtables_p [type] = subtable;
}

uInt
Subtables::size () const
{
    return impl_p->subtables_p.size();
}

Subtables::ST
Subtables::stFromString (const String & text)
{
    String canonical = downcase (text);

    for (Impl::SubtablesRep::const_iterator i = Impl::types_p;
         i != Impl::types_p.end();
         i++){

        if (downcase (i->second) == canonical){
            return i->first;
        }
    }

    ThrowIf (True, String::format ("Unknown subtable type: '%s'", text));
}


String
Subtables::stToString (Subtables::ST type)
{
    return Impl::types_p [type];
}


};
