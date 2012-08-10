/*
 * VisBufferImpl.cc
 *
 *  Created on: Jul 3, 2012
 *      Author: jjacobs
 */

#include <synthesis/MSVis/VisBufferBase2.h>

namespace casa {

namespace vi {

    VisBufferBase2::VisBufferBase2()
    {}

    VisBufferBase2::VisBufferBase2(const VisBufferBase2 & /*vb*/)
    {}

    // Destructor (detaches from VisIter)

    VisBufferBase2::~VisBufferBase2()
    {}

    VisBufferBase2::VisBufferBase2 (ROVisibilityIterator2 & /*iter*/)
    {}


} // end namespace vi

} // end namespace casa
