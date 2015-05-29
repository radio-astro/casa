/*
 * Vbi2MsRow.h
 *
 *  Created on: Aug 22, 2013
 *      Author: jjacobs
 */

#ifndef VBI2MSROW_H_
#define VBI2MSROW_H_

#include <casa/Arrays/Array.h>
#include <msvis/MSVis/MsRows.h>

// Forward Decls

namespace casa {

namespace vi {

class VisBufferImpl2;

}

}

namespace casa {

namespace ms {

class CachedArrayBase {

public:

    CachedArrayBase () : cached_p (False) {}
    virtual ~CachedArrayBase () {}

    void clearCache () { cached_p = False;}

protected:

    Bool isCached () const { return cached_p;}
    void setCached () { cached_p = true;}

private:

    Bool cached_p;
};

template <typename T>
class CachedPlane : public CachedArrayBase {

public:

typedef const Cube<T> & (casa::vi::VisBufferImpl2::* Accessor) () const;

CachedPlane (Accessor accessor) : accessor_p (accessor) {}

Matrix<T> &
getCachedPlane (casa::vi::VisBufferImpl2 * vb, Int row)
{
    if (! isCached()){

        //cache_p.reference ((vb ->* accessor_p)().xyPlane (row)); // replace with something more efficient
        referenceMatrix (cache_p, (vb ->* accessor_p)(), row);
        setCached ();
    }

    return cache_p;
}

private:

    static void
    referenceMatrix (Matrix<T> & cache, const Cube<T> & src, Int row)
    {
        IPosition shape = src.shape ();
        shape.resize (2);

        // This is a bit sleazy but it seems to be helpful to performance.
        // Assumes contiguously stored cube.

        T * storage = const_cast <T *> (& src (IPosition (3, 0, 0, row)));

        cache.takeStorage (shape, storage, casa::SHARE);
    }

    Accessor accessor_p;
    Matrix<T> cache_p;
};

template <typename T>
class CachedColumn : public CachedArrayBase {

public:

typedef const Matrix<T> & (casa::vi::VisBufferImpl2::* Accessor) () const;

CachedColumn (Accessor accessor) : accessor_p (accessor) {}

Vector<T> &
getCachedColumn (casa::vi::VisBufferImpl2 * vb, Int row)
{
    if (! isCached()){

        referenceVector (cache_p, (vb ->* accessor_p)(), row);
        setCached ();
    }

    return cache_p;
}

private:

    static void
    referenceVector (Vector<T> & cache, const Matrix<T> & src, Int row)
    {
        IPosition shape = src.shape ();
        shape.resize (1);

        // This is a bit sleazy but it seems to be helpful to performance.
        // Assumes contiguously stored cube.

        T * storage = const_cast <T *> (& src (IPosition (2, 0, row)));

        cache.takeStorage (shape, storage, casa::SHARE);
    }

    Accessor accessor_p;
    Vector<T> cache_p;
};


class Vbi2MsRow : public MsRow {

public:

    // Constructors

    // Constructor for read-only access.
    // Attempt to write will throw exception.

    Vbi2MsRow (Int row, const vi::VisBufferImpl2 * vb);

    // Constructor for read/write access

    Vbi2MsRow (Int row, vi::VisBufferImpl2 * vb);

    virtual ~Vbi2MsRow () {}

    void changeRow (Int row);
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
    const Matrix<Complex> & corrected () const;
    Matrix<Complex> & correctedMutable ();
    void setCorrected (Int correlation, Int channel, const Complex & value);
    void setCorrected (const Matrix<Complex> & value);

    const Complex & model (Int correlation, Int channel) const;
    const Matrix<Complex> & model () const;
    Matrix<Complex> & modelMutable ();
    void setModel(Int correlation, Int channel, const Complex & value);
    void setModel (const Matrix<Complex> & value);

    const Complex & observed (Int correlation, Int channel) const;
    const Matrix<Complex> & observed () const;
    Matrix<Complex> & observedMutable ();
    void setObserved (Int correlation, Int channel, const Complex & value);
    void setObserved (const Matrix<Complex> & value);

    const Float & singleDishData (Int correlation, Int channel) const;
    const Matrix<Float> singleDishData () const;
    Matrix<Float> singleDishDataMutable ();
    void setSingleDishData (Int correlation, Int channel, const Float & value);
    void setSingleDishData (const Matrix<Float> & value);

    Float sigma (Int correlation) const;
    const Vector<Float> & sigma () const;
    Vector<Float> & sigmaMutable () const;
    void setSigma (Int correlation, Float value);
    void setSigma (const Vector<Float> & value);
    Float weight (Int correlation) const;
    const Vector<Float> & weight () const;
    Vector<Float> & weightMutable () const;
    void setWeight (Int correlation, Float value);
    void setWeight (const Vector<Float> & value);
    Float weightSpectrum (Int correlation, Int channel) const;
    void setWeightSpectrum (Int correlation, Int channel, Float value);
    void setWeightSpectrum (const Matrix<Float> & value);
    const Matrix<Float> & weightSpectrum () const;
    Matrix<Float> & weightSpectrumMutable () const;
    Float sigmaSpectrum (Int correlation, Int channel) const;
    const Matrix<Float> & sigmaSpectrum () const;
    Matrix<Float> & sigmaSpectrumMutable () const;
    void setSigmaSpectrum (Int correlation, Int channel, Float value);
    void setSigmaSpectrum (const Matrix<Float> & value);

    Bool isRowFlagged () const;
    const Matrix<Bool> & flags () const;
    void setFlags (const Matrix<Bool> & flags);
    Bool isFlagged (Int correlation, Int channel) const;

    void setRowFlag (Bool isFlagged);
    void setFlags (Bool isFlagged, Int correlation, Int channel);

protected:

    template <typename T>
    void addToCachedArrays (T & cachedArray)
    {
        arrayCaches_p.push_back (& cachedArray);
    }

    void clearArrayCaches();
    Matrix<Bool> & flagsMutable ();
    vi::VisBufferImpl2 * getVbi () const;

private:

    void configureArrayCaches(); // called in ctor so do not override

    mutable CachedPlane<Complex> correctedCache_p;
    mutable CachedPlane<Bool> flagCache_p;
    mutable CachedPlane<Complex> modelCache_p;
    mutable CachedPlane<Complex> observedCache_p;
    mutable CachedColumn<Float> sigmaCache_p;
    mutable CachedPlane<Float> sigmaSpectrumCache_p;
    mutable CachedColumn<Float> weightCache_p;
    mutable CachedPlane<Float> weightSpectrumCache_p;

    std::vector<CachedArrayBase *> arrayCaches_p;

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
