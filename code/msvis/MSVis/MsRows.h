/*
 * MsRows.h
 *
 *  Created on: Feb 20, 2013
 *      Author: jjacobs
 */

#if ! defined (MsVis_MsRows_H)
#define MsVis_MsRows_H

#include <msvis/MSVis/VisBuffer2.h>
#include <boost/noncopyable.hpp>
#include <vector>

namespace casa {

namespace vi{

    class VisBuffer2;
}

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

public:

    // Constructors

    // Constructor for read-only access.
    // Attempt to write will throw exception.

    MsRow (Int row, const vi::VisBuffer2 * vb);

    // Constructor for read/write access

    MsRow (Int row, vi::VisBuffer2 * vb);

    virtual ~MsRow () {}

    virtual void changeRow (Int row) { row_p = row;}

    virtual Int antenna1 () const = 0;
    virtual Int antenna2 () const = 0;
    virtual Int arrayId () const = 0;
    virtual Int correlationType () const = 0;
    virtual Int dataDescriptionId () const = 0;
    virtual Int feed1 () const = 0;
    virtual Int feed2 () const = 0;
    virtual Int fieldId () const = 0;
    virtual Int observationId () const = 0;
    virtual Int processorId () const = 0;
    virtual Int scanNumber () const = 0;
    virtual Int stateId () const = 0;
    virtual Double exposure () const = 0;
    virtual Double interval () const = 0;
    virtual Int rowId () const = 0;
    virtual Int spectralWindow () const = 0;
    virtual Double time () const = 0;
    virtual Double timeCentroid () const = 0;

    virtual void setAntenna1 (Int) = 0;
    virtual void setAntenna2 (Int) = 0;
    virtual void setArrayId (Int) = 0;
    virtual void setCorrelationType (Int) = 0;
    virtual void setDataDescriptionId (Int) = 0;
    virtual void setFeed1 (Int) = 0;
    virtual void setFeed2 (Int) = 0;
    virtual void setFieldId (Int) = 0;
    virtual void setObservationId (Int) = 0;
    virtual void setProcessorId (Int) = 0;
    virtual void setScanNumber (Int) = 0;
    virtual void setStateId (Int) = 0;
    virtual void setExposure (Double) = 0;
    virtual void setInterval (Double) = 0;
    virtual void setRowId (Int) = 0;
    virtual void setSigma (Int correlation, Float value) = 0;
    virtual void setSigma (const Vector<Float> &  value) = 0;
    virtual void setTime (Double) = 0;
    virtual void setTimeCentroid (Double) = 0;
    virtual void setWeight (Int correlation, Float value) = 0;
    virtual void setWeight (const Vector<Float> & value) = 0;

    virtual const Vector<Double> uvw () const = 0;
    virtual void setUvw (const Vector<Double> &) = 0;

    virtual const Complex & corrected (Int correlation, Int channel) const = 0;
    virtual const Matrix<Complex> & corrected () const = 0;
    virtual void setCorrected (Int correlation, Int channel, const Complex & value) = 0;
    virtual void setCorrected (const Matrix<Complex> & value) = 0;

    virtual const Complex & model (Int correlation, Int channel) const = 0;
    virtual const Matrix<Complex> & model () const = 0;
    virtual void setModel(Int correlation, Int channel, const Complex & value) = 0;
    virtual void setModel (const Matrix<Complex> & value) = 0;

    virtual const Complex & observed (Int correlation, Int channel) const = 0;
    virtual const Matrix<Complex> & observed () const = 0;
    virtual void setObserved (Int correlation, Int channel, const Complex & value) = 0;
    virtual void setObserved (const Matrix<Complex> & value) = 0;

    virtual const Float & singleDishData (Int correlation, Int channel) const = 0;
    virtual const Matrix<Float> singleDishData () const = 0;
    virtual void setSingleDishData (Int correlation, Int channel, const Float & value) = 0;
    virtual void setSingleDishData (const Matrix<Float> & value) = 0;

    virtual Float sigma (Int correlation) const = 0;
    virtual const Vector<Float> & sigma () const = 0;
    virtual Float weight (Int correlation) const = 0;
    virtual const Vector<Float> & weight () const = 0;
    virtual Float weightSpectrum (Int correlation, Int channel) const = 0;
    virtual const Matrix<Float> & weightSpectrum () const = 0;
    virtual Float sigmaSpectrum (Int correlation, Int channel) const = 0;
    virtual const Matrix<Float> & sigmaSpectrum () const = 0;

    virtual Bool isRowFlagged () const = 0;
    virtual const Matrix<Bool> & flags () const = 0;
    virtual void setFlags (const Matrix<Bool> &) = 0;
    virtual Bool isFlagged (Int correlation, Int channel) const = 0;

    virtual void setRowFlag (Bool isFlagged) = 0;
    virtual void setFlags (Bool isFlagged, Int correlation, Int channel) = 0;

protected:

    Bool isWritable () const;
    uInt row () const;
    vi::VisBuffer2 * vb () const;

private:

    MsRow (const MsRow & other); // no copying

    const Bool isWritable_p;
    Int row_p;
    vi::VisBuffer2 * vb_p; // [use]
};

class MsRows {

public:

    typedef std::vector<MsRow *> Rows;
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
    vi::VisBuffer2 * visBuffer_p; // [use]
};

// MsRows rows;
// Double t = rows(i).time();
// Double t = vb.time()(i)

} // end namespace ms

} // end namespace casa

#endif // ! defined (MsVis_MsRows_H)
