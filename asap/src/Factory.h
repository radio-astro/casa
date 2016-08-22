//
// C++ Interface: Factory
//
// Description:
//
//
// Author: Malte Marquarding <Malte.Marquarding@csiro.au>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAPFACTORY_H
#define ASAPFACTORY_H

#include "FactoryBase.h"

namespace asap
{

/**
A templated factory pattern

@author Malte Marquarding
@date $Date:$

*/

template <class BT,class DT>
class Factory : public FactoryBase<BT> {
public:
    Factory() {}
    virtual ~Factory() {}
    virtual BT* create() { return new DT;}
};

} //namespace

#endif
