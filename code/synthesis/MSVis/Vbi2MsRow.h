/*
 * Vbi2MsRow.h
 *
 *  Created on: Aug 22, 2013
 *      Author: jjacobs
 */

#ifndef VBI2MSROW_H_
#define VBI2MSROW_H_

#include <synthesis/MSVis/MsRows.h>

// Forward Decls

namespace casa {

namespace vi {

class VisBufferImpl2;

}

}

namespace casa {

namespace ms {

class Vbi2MsRow : public MsRow {

public:

    // Constructors

    // Constructor for read-only access.
    // Attempt to write will throw exception.

    Vbi2MsRow (Int row, const vi::VisBufferImpl2 * vb);

    // Constructor for read/write access

    Vbi2MsRow (Int row, vi::VisBufferImpl2 * vb);

    virtual ~Vbi2MsRow () {}

    void copy (Vbi2MsRow * other,
               const VisBufferComponents2 & componentsToCopy);

    Int antenna1 () const;
    Int antenna2 () const;
    Int arrayId () const;
    Int correlationType () const;
    Int dataDescriptionId () const;
    Int feed1 () const;
    Int feed2 () const;
    Int fieldId () const;
    Int observationId () const;
    Int rowId () const;
    Int processorId () const;
    Int scanNumber () const;
    Int stateId () const;
    Double exposure () const;
    Double interval () const;
    Int spectralWindow () const;
    Double time () const;
    Double timeCentroid () const;

    void setAntenna1 (Int);
    void setAntenna2 (Int);
    void setArrayId (Int);
    void setCorrelationType (Int);
    void setDataDescriptionId (Int);
    void setFeed1 (Int);
    void setFeed2 (Int);
    void setFieldId (Int);
    void setObservationId (Int);
    void setProcessorId (Int);
    void setRowId (Int);
    void setScanNumber (Int);
    void setStateId (Int);
    void setExposure (Double);
    void setInterval (Double);
    void setSpectralWindow (Int);
    void setTime (Double);
    void setTimeCentroid (Double);

    const Vector<Double> uvw () const;
    const Double & uvw (Int i) const;
    void setUvw (const Vector<Double> &);
    void setUvw (Int i, const Vector<Double> &);

    const Complex & corrected (Int correlation, Int channel) const;
    const Matrix<Complex> corrected () const;
    Matrix<Complex> correctedMutable ();
    void setCorrected (Int correlation, Int channel, const Complex & value);
    void setCorrected (const Matrix<Complex> & value);

    const Complex & model (Int correlation, Int channel) const;
    const Matrix<Complex> model () const;
    Matrix<Complex> modelMutable ();
    void setModel(Int correlation, Int channel, const Complex & value);
    void setModel (const Matrix<Complex> & value);

    const Complex & observed (Int correlation, Int channel) const;
    const Matrix<Complex> observed () const;
    Matrix<Complex> observedMutable ();
    void setObserved (Int correlation, Int channel, const Complex & value);
    void setObserved (const Matrix<Complex> & value);

    Float sigma (Int correlation) const;
    const Vector<Float> sigma () const;
    void setSigma (Int correlation, Float value);
    void setSigma (const Vector<Float> & value);
    Float weight (Int correlation) const;
    const Vector<Float> weight () const;
    void setWeight (Int correlation, Float value);
    void setWeight (const Vector<Float> & value);
    Float weightSpectrum (Int correlation, Int channel) const;
    void setWeightSpectrum (Int correlation, Int channel, Float value);
    void setWeightSpectrum (const Matrix<Float> & value);
    const Matrix<Float> weightSpectrum () const;
    Float sigmaSpectrum (Int correlation, Int channel) const;
    const Matrix<Float> sigmaSpectrum () const;
    void setSigmaSpectrum (Int correlation, Int channel, Float value);
    void setSigmaSpectrum (const Matrix<Float> & value);

    Bool isRowFlagged () const;
    const Matrix<Bool> flags () const;
    void setFlags (const Matrix<Bool> & flags);
    Bool isFlagged (Int correlation, Int channel) const;

    void setRowFlag (Bool isFlagged);
    void setFlags (Bool isFlagged, Int correlation, Int channel);

protected:

    Matrix<Bool> flagsMutable ();

private:

template <typename T, typename U>
void
copyIf (Bool copyThis, Vbi2MsRow * other,
                   void (Vbi2MsRow::* setter) (T),
                   U (Vbi2MsRow::* getter) () const);

    vi::VisBufferImpl2 * vbi2_p;

};

}

} // end namespace casa


#endif /* VBI2MSROW_H_ */
