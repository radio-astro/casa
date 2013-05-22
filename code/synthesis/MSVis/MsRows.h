/*
 * MsRows.h
 *
 *  Created on: Feb 20, 2013
 *      Author: jjacobs
 */

#include <synthesis/MSVis/VisBuffer2.h>

namespace casa {
namespace vi{

    class VisBuffer2;
}
}


namespace casa {

namespace ms {

//class MsRowAttributes {
//
//public:
//
//    MsRowAttributes (Int row, VisBuffer2 * vb)
//    : row_p (row), vb_p (vb)
//    {}
//
//    Int antenna1 () const;
//    Int antenna2 () const;
//    Int array () const;
//    Int dataDescriptionId () const;
//    Int feed1 () const;
//    Int feed2 () const;
//    Int field () const;
//    Int observation () const;
//    Int processorId () const;
//    Int scan () const;
//    Int state () const;
//
//private:
//};
//
//class MsRowCoordinates {
//
//public:
//
//    MsRowCoordinates (Int row, VisBuffer2 * vb)
//    : row_p (row), vb_p (vb)
//    {}
//
//    Double exposure () const;
//    Int fieldId () const;
//    Double interval () const;
//    Double time () const {
//        return vb_p->time() (row_p);
//    }
//    Double timeCentroid () const;
//    const Vector<Double> & uvw () const;
//
//private:
//
//    Int row_p;
//    vi::VisBuffer2 * vb_p;
//
//};
//
//class MsRowData {
//
//public:
//
//    MsRowData (Int row, VisBuffer2 * vb)
//    : row_p (row), vb_p (vb)
//    {}
//
//    const Complex & corrected (Int correlation, Int channel) const;
//    const Complex & model (Int correlation, Int channel) const;
//    const Complex & observed (Int correlation, Int channel) const
//    {
//       return vb_p->visCube () (correlation, channel, row_p);
//    }
//    const Float sigma () const;
//    const Float weight () const;
//    const Float weightSpectrum (Int correlation, Int channel) const;
//
//private:
//
//    Int row_p;
//    const vi::VisBuffer2 * vb_p;
//
//};
//
//class MsRowFlagging {
//
//public:
//
//    MsRowFlagging (Int row, VisBuffer2 * vb)
//    : row_p (row), vb_p (vb)
//    {}
//
//    Bool isFlagged () const;
//    Bool isFlagged (Int correlation, Int channel) const;
//
//    void setFlag (Bool isFlagged);
//    void setFlag (Bool isFlagged, Int correlation, Int channel);
//
//private:
//
//    Int row_p;
//    vi::VisBuffer2 * vb_p;
//};
//
//class MsRow {
//
//public:
//
//    const MsRowAttributes & attributes () const;
//    const MsRowCoordinates & coordinates () const;
//    const MsRowData & data () const;
//    const MsRowFlagging & flagging () const;
//
//protected:
//
//private:
//
//    int row_p;
//};

class MsRow {

    MsRow (Int row, VisBuffer * vb);

    Int antenna1 () const;
    Int antenna2 () const;
    Int arrayId () const;
    Int dataDescriptionId () const;
    Int feed1 () const;
    Int feed2 () const;
    Int fieldId () const;
    Int observationId () const;
    Int processorId () const;
    Int scanNumber () const;
    Int stateId () const;

    Double exposure () const;
    Double interval () const;
    Double time () const;
    Double timeCentroid () const;
    const Vector<Double> & uvw () const;

    const Complex & corrected (Int correlation, Int channel) const;
    void setCorrected (Int correlation, Int channel, const Complex & value);
    void setCorrected (const Matrix<Complex> & value);

    const Complex & model (Int correlation, Int channel) const;
    void setModel(Int correlation, Int channel, const Complex & value);
    void setModel (const Matrix<Complex> & value);

    const Complex & observed (Int correlation, Int channel) const;
    void setObserved (Int correlation, Int channel, const Complex & value);
    void setObserved (const Matrix<Complex> & value);

    const Float sigma (Int correlation) const;
    const Float weight (Int correlation) const;
    const Float weightSpectrum (Int correlation, Int channel) const;

    Bool isFlagged () const;
    Bool isFlagged (Int correlation, Int channel) const;

    void setFlag (Bool isFlagged);
    void setFlag (Bool isFlagged, Int correlation, Int channel);

private:

    Int row_p;
    vi::VisBuffer2 * vb_p; // [use]
};

class MsRows {

public:

    typedef Vector<MsRow> Rows;
    typedef Rows::const_iterator const_iterator;
    typedef Rows::iterator iterator;

    MsRows (vi::VisBuffer2 * vb);

    MsRow & operator() (Int i);
    const MsRow & operator() (Int i) const;

    const_iterator begin () const;
    iterator begin ();
    const_iterator end () const;
    iterator end ();
    MsRow & row (Int row);
    const MsRow & row (Int row) const;
    size_t size () const;

protected:

private:

    Rows rows_p;
    vi::VisBuffer2 * visBuffer_p;
};

// MsRows rows;
// Double t = rows(i).time();
// Double t = vb.time()(i)

} // end namespace ms

} // end namespace casa

