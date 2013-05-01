/*
 * VisBuffer2.cc
 *
 *  Created on: Aug 21, 2012
 *      Author: jjacobs
 */


#include <synthesis/MSVis/UtilJ.h>
#include <synthesis/MSVis/VisBuffer2.h>
#include <synthesis/MSVis/VisBufferImpl2.h>

namespace casa {

namespace vi {

VisBuffer2 *
VisBuffer2:: factory (VisBufferType t, VisBufferOptions options)
{
    return factory (0, t, options);
}

VisBuffer2 *
VisBuffer2::factory (VisibilityIterator2 * vi, VisBufferType t, VisBufferOptions options)
{
    VisBuffer2 * result = NULL;

    if (t == VbPlain){
        result = new VisBufferImpl2 (vi, options);
    }
    else{
        ThrowIf (true, utilj::format ("Unknown or unsupported VisBuffer2 type: id=%d", t));
    }

    return result;
}

} // end namespace vi

} // end namespace casa


