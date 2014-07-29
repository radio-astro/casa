/*
 * VisibilityBufferAsync.cc
 *
 *  Created on: Nov 1, 2010
 *      Author: jjacobs
 */

#include "UtilJ.h"
using namespace casa::utilj;

#include <casa/Containers/Record.h>

#include <msvis/MSVis/VisBufferImplAsync2.h>
#include <msvis/MSVis/VisBufferAsyncWrapper2.h>
#include <msvis/MSVis/VisibilityIterator2.h>
#include <msvis/MSVis/AsynchronousInterface2.h>
#include <msvis/MSVis/VLAT.h>

#include <algorithm>

using std::transform;

#include <ms/MeasurementSets/MSColumns.h>
#include <typeinfo>

#define Log(level, ...) \
    {if (casa::asyncio::AsynchronousInterface::logThis (level)) \
         casa::async::Logger::get()->log (__VA_ARGS__);};

namespace casa {

namespace vi {

template <typename MeasureType, typename AsMeasure>
MeasureType
unsharedCopyMeasure (const MeasureType & measure, AsMeasure asMeasure)
{
    // MeasureType is a derived type of MeasBase<> (e.g., MDirection, MEpoch, etc.)
    // AsMeasure is the MeasureHolder converter for the desired type (e.g., asMDirection, asMEpoch, etc.)

    // Make a completely distinct copy of this measure.  The
    // methods provided by a Measure always result in the sharing
    // of the measurement reference object so this roundabout
    // approach is necessary.

    // Using a MeasureHolder object, transform the contents of the
    // measure into a generic record.

    MeasureHolder original (measure);
    Record record;
    String err;

    original.toRecord(err, record);

    // Now use the record to create another MMeasure object using
    // the record created to hold the original's information.

    MeasureHolder copy;
    copy.fromRecord (err, record);

    MeasureType result = (copy .* asMeasure) ();

    return result;
}

VisBufferImplAsync2::VisBufferImplAsync2 ()
  : VisBufferImpl2 ()
{
    construct();
}

//VisBufferImplAsync2::VisBufferImplAsync2 (const VisBufferImplAsync2 & vb)
//: VisBufferImpl2 ()
//{
////    const VisBufferImplAsync2 * vb = dynamic_cast<const VisBufferImplAsync2 *> (& vb0);
////    ThrowIf (vb == NULL, "Cannot copy VisBufferImpl2 into VisBufferImplAsync2");
//
//    construct ();
//
//    * this = vb;
//}

VisBufferImplAsync2::VisBufferImplAsync2 (VisibilityIterator2 & iter, Bool isWritable)
  : VisBufferImpl2 (& iter, isWritable)
{
    construct ();

    Log (2, "VisBufferImplAsync2::VisBufferImplAsync2: attaching in constructor this=%08x\n", this);
}


VisBufferImplAsync2::~VisBufferImplAsync2 ()
{
    Log (2, "Destroying VisBufferImplAsync2; addr=0x%016x\n", this);

    // Should never be attached at the synchronous level

    delete msColumns_p;
    delete msd_p;
}

//void
//VisBufferImplAsync2::checkVisIter (const char * func, const char * file, int line, const char * extra) const
//{
//    // This is called from a VisBufferImpl2 fill method.  Throw an error if the this is not
//    // part of the VLAT filling operation or if there is not visibility iterator attached
//
//    if (visIter_p == NULL){
//        Log (1, "VisBufferImplAsync2: request for column not in prefetched set (in call to %s (%s))\n)",
//             func, extra);
//        throw AipsErrorTrace ("VisBufferImplAsync2: request for column not in prefetched set (in call to "
//                              + String (func) + String (extra) + ")", file, line);
//    }
//}

void
VisBufferImplAsync2::construct ()
{
    Log (2, "Constructing VisBufferImplAsync2; addr=0x%016x\n", this);
}


//Vector<uInt>&
//VisBufferImplAsync2::rowIds()
//{
//    return rowIds_p;
//}

//void
//VisBufferImplAsync2::setDataDescriptionId (Int id)
//{
//    dataDescriptionId_p = id;
//}

//void
//VisBufferImplAsync2::setFilling (Bool isFilling)
//{
//    isFilling_p = isFilling;
//}




} // end namespace vi
} // end namespace casa


