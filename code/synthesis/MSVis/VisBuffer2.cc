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

VisBuffer2 *
VisBuffer2:: factory (Type t)
{
    return factory (0, t);
}

VisBuffer2 *
VisBuffer2::factory (ROVisibilityIterator2 * vi, Type t)
{
    VisBuffer2 * result = NULL;

    if (t == Plain){
        result = new vi::VisBufferImpl2 (vi);
    }
    else{
        ThrowIf (true, utilj::format ("Unknown VisBuffer2 type: id=%d", t));
    }

    return result;
}


} // end namespace casa


