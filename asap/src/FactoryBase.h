//
// C++ Interface: FactoryBase
//
// Description:
//
//
// Author: Malte Marquarding <Malte.Marquarding@csiro.au>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAPFACTORYBASE_H
#define ASAPFACTORYBASE_H

namespace asap {
/**
A templated factory pattern

@author Malte Marquarding
@date $Date:$

*/
template <class BT>
class FactoryBase {
public:
    FactoryBase() {}
    virtual ~FactoryBase() {}
    virtual BT* create() = 0;
};
} //namespace
#endif
