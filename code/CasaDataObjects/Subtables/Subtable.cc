/*
 * Subtable.cc
 *
 *  Created on: May 4, 2015
 *      Author: jjacobs
 */


#include "Subtable.h"

#include <map>
#include <iostream>

using namespace std;

namespace casa {
namespace cdo {

class Subtable::Impl {

public:

    static std::map <SubtableType, String> types_p;

};




//std::ostream &
//operator<< (std::ostream & os, SubtableType type)
//{
//    os << types_p [type];
//
//    return os;
//}

} // end namespace cdo
} // end namespace casa
