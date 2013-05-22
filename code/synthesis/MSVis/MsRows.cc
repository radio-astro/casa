/*
 * MsRows.cc
 *
 *  Created on: Feb 20, 2013
 *      Author: jjacobs
 */

#include "MsRows.h"

#include <synthesis/MSVis/VisBuffer2.h>

namespace casa {
namespace ms {

MsRow::MsRow (Int row, VisBuffer * vb)
: row_p (row), vb_p (vb)
{}

const Complex &
MsRow::observed (Int correlation, Int channel) const
{
    return vb_p->visCube () (correlation, channel, row_p);
}

Double
MsRow::time () const {
    return vb_p->time() (row_p);
}

MsRows::MsRows (vi::VisBuffer2 * vb)
{
    rows_p.resize (vb->nRows());

    for (Int i = 0; i < vb->nRows(); i ++){
        rows_p [i] = MsRow (i, vb);
    }
}

} // end namespace ms
} // end namespace casa
