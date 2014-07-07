/*
 * MsRows.cc
 *
 *  Created on: Feb 20, 2013
 *      Author: jjacobs
 */

#include "MsRows.h"

#include <msvis/MSVis/VisBuffer2.h>
#include <casa/Exceptions.h>

namespace casa {
namespace ms {

using vi::VisBuffer2;

#define AssertWritable() ThrowIf (! isWritable_p, "Cannot write to this MsRow object")

MsRow::MsRow (Int row, const VisBuffer2 * vb)
: isWritable_p (False),
  row_p (row),
  vb_p (const_cast<VisBuffer2 * > (vb)) // take const into our own hands ;-(
{
    ThrowIf (vb == 0, "VisBuffer cannot be null.");
}


MsRow::MsRow (Int row, VisBuffer2 * vb)
: isWritable_p (True), row_p (row), vb_p (vb)
{
    ThrowIf (vb == 0, "VisBuffer cannot be null.");
}

Bool
MsRow::isWritable () const
{
    return isWritable_p;
}

uInt
MsRow::row () const
{
    return row_p;
}

vi::VisBuffer2 *
MsRow::vb () const
{
    return vb_p;
}


MsRows::MsRows (vi::VisBuffer2 * vb)
{
    rows_p.resize (vb->nRows());

    for (Int i = 0; i < vb->nRows(); i ++){
        rows_p [i] = vb->getRow(i);
    }
}



} // end namespace ms
} // end namespace casa
