/*
 * VisBuffer2.cc
 *
 *  Created on: Aug 21, 2012
 *      Author: jjacobs
 */


#include <msvis/MSVis/UtilJ.h>
#include <msvis/MSVis/VisBuffer2.h>
#include <msvis/MSVis/VisBufferImpl2.h>

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
        ThrowIf (true, String::format ("Unknown or unsupported VisBuffer2 type: id=%d", t));
    }

    return result;
}

ms::MsRow *
VisBuffer2::getRow (Int) const
{
    ThrowIf (True, "Not implemented by this subclass.");

    return 0;
}

ms::MsRow *
VisBuffer2::getRowMutable (Int)
{
    ThrowIf (True, "Not implemented by this subclass.");

    return 0;
}

} // end namespace vi

} // end namespace casa


