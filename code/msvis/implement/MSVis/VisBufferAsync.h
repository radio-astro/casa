/*
 * VisibilityBufferAsync.h
 *
 *  Created on: Nov 3, 2010
 *      Author: jjacobs
 */

#ifndef VISIBILITYBUFFERASYNC_H_
#define VISIBILITYBUFFERASYNC_H_

#include <msvis/MSVis/VisBuffer.h>

namespace casa {

class ROVisibilityIteratorAsync;

class VisBufferAsync : public VisBuffer {

    friend class ROVisibilityIteratorAsync;
    friend class VisBufferAsyncWrapper;
    friend class VisBufferAutoPtr;
    friend class VLAT;
    friend class VlaDatum;

public:

    //VisBufferAsync (const VisBuffer& vb);

    ~VisBufferAsync ();

    VisBufferAsync & operator= (const VisBufferAsync & other);

    virtual void allSelectedSpectralWindows(Vector<Int>& spws, Vector<Int>& nvischan);
    virtual VisBufferAsync & assign (const VisBuffer & vb, Bool copy);
    virtual Vector<MDirection> azel(Double time) const;
    virtual MDirection azel0(Double time) const;
    virtual Int dataDescriptionId() const;
    virtual void detachFromVisIter ();
    virtual Vector<Float> feed_pa(Double time) const;
    virtual Double hourang(Double time) const;
    virtual void invalidate (); // This one is booby-trapped right now
    virtual void invalidateAsync (); // Use this one in async code
    virtual Vector<Double> & lsrFrequency ();
    virtual const Vector<Double> & lsrFrequency () const;
    virtual void lsrFrequency(const Int& spw, Vector<Double>& freq, Bool& convert) const;
    virtual const ROMSColumns& msColumns() const;
    Int msId () const;
    Bool newMS() const;
    Int numberAnt () const;
    Int numberCoh () const;
    virtual Vector<Float> parang(Double time) const;
    virtual Float parang0(Double time) const;
    virtual Vector<uInt>& rowIds(){throw(AipsError("rowIds() not implemented for VBA."));}
    virtual const Vector<uInt>& rowIds() const {throw(AipsError("rowIds() const not implemented for VBA."));}
    virtual void setCorrectedVisCube(Complex c);
    virtual void setCorrectedVisCube (const Cube<Complex> & vis);
    virtual void setModelVisCube(Complex c);
    virtual void setModelVisCube (const Cube<Complex> & vis);
    virtual void setModelVisCube (const Vector<Float> & stokes);
    virtual void setVisCube(Complex c);
    virtual void setVisCube (const Cube<Complex>& vis);

    //static VisBuffer * create (const VisBuffer & other);
//    static VisBuffer * create (ROVisibilityIterator & iter);
//    static VisBuffer * create (ROVisibilityIterator * iter);

protected:

    // The constructors are not public because creation should be performed
    // by a factory object (e.g., VisBufferAutoPtr).  The use of a factory
    // makes it possible to fine tune at run time whether a VisBuffer or a
    // VisBufferAsync is created.

    VisBufferAsync ();
    VisBufferAsync (const VisBufferAsync & other);
    VisBufferAsync (ROVisibilityIteratorAsync & iter);

    void attachToVisIter(ROVisibilityIterator & iter);
    void checkVisIter (const char * func, const char * file, int line) const;
    void clear ();
    void construct ();
    virtual void copyAsyncValues (const VisBufferAsync & other);
    virtual void copyCache (const VisBuffer & other);
    template<typename T> void copyVector (const Vector<T> & from, Vector<T> & to);
    void fillFrom (const VisBufferAsync & other);
    Vector<MDirection>& fillDirection1();
    Vector<MDirection>& fillDirection2();
    MDirection & fillPhaseCenter();
    void setDataDescriptionId (Int id);
    void setFilling (Bool isFilling);
    void setLsrInfo (const Block<Int> & channelStart,
                     const Block<Int> & channelWidth,
                     const Block<Int> & channelIncrement,
                     const Block<Int> & channelGroupNumber,
                     const ROArrayColumn <Double> * chanFreqs,
                     const ROScalarColumn<Int> * obsMFreqTypes,
                     const MPosition & observatoryPositon,
                     const MDirection & phaseCenter,
                     Bool velocitySelection);
    void setMeasurementSet (const MeasurementSet & ms);
    void setMeasurementSetId (Int id, bool isNew);
    void setMEpoch (const MEpoch & mEpoch);
    void setMSD (const MSDerivedValues & msd);
    void setNAntennas (Int);
    void setNCoh (Int);
    void setReceptor0Angle (const Vector<Float> & receptor0Angle);
    void setRowIds (const Vector<uInt> & rowIds);
    void setSelectedNVisibilityChannels (const Vector<Int> & nVisibilityChannels);
    void setSelectedSpectralWindows (const Vector<Int> & spectralWindows);
    void setTopoFreqs (const Vector<Double> & lsrFreq, const Vector<Double> & selFreq_p);
    void setVisibilityShape (const IPosition & pvisibilityShape);
    void updateCoordInfo (const VisBuffer *, const Bool dirDependent=True);

    static MDirection unsharedCopyDirection (const MDirection & direction);
    static void unsharedCopyDirectionVector (Vector<MDirection> & direction);
    static MEpoch unsharedCopyEpoch (const MEpoch & mEpoch);
    static MPosition unsharedCopyPosition (const MPosition & position);

private:

    const ROArrayColumn <Double> * chanFreqs_p;  // [use]
    Block<Int>                     channelGroupNumber_p;
    Block<Int>                     channelIncrement_p;
    Block<Int>                     channelStart_p;
    Block<Int>                     channelWidth_p;
    Int                            dataDescriptionId_p;
    Bool                           isFilling_p;
    MEpoch                         mEpoch_p;
    const MeasurementSet *         measurementSet_p;  // [use]
    mutable ROMSColumns *          msColumns_p; // [own]
    MSDerivedValues *              msd_p; // [own]
    Int                            msID_p;
    Int                            nAntennas_p;
    Int                            nCoh_p;
    const ROScalarColumn<Int> *    obsMFreqTypes_p; // [use]
    MPosition                      observatoryPosition_p;
    Vector<Float>                  receptor0Angle_p;
    Vector<Double>                 selFreq_p;
    Vector<Int>                    selectedNVisibilityChannels_p;
    Vector<Int>                    selectedSpectralWindows_p;
    Bool                           velSelection_p;
    IPosition                      visibilityShape_p;
};

class VisBufferAutoPtr {

public:

    VisBufferAutoPtr ();
    VisBufferAutoPtr (VisBufferAutoPtr & other);
    explicit VisBufferAutoPtr (VisBuffer &);
    explicit VisBufferAutoPtr (VisBuffer *);
    explicit VisBufferAutoPtr (ROVisibilityIterator * rovi);
    explicit VisBufferAutoPtr (ROVisibilityIterator & rovi);
    ~VisBufferAutoPtr ();

    VisBufferAutoPtr & operator= (VisBufferAutoPtr & other);
    VisBuffer & operator* () const;
    VisBuffer * operator-> () const;

    VisBuffer * get () const;
    VisBuffer * release ();
    void set (VisBuffer &);
    void set (VisBuffer *);
    void set (ROVisibilityIterator * rovi);
    void set (ROVisibilityIterator & rovi);

protected:

    void construct (ROVisibilityIterator * rovi, Bool attachVi);
    void constructVb (VisBuffer * rovi);

private:

    VisBuffer * visBuffer_p;

};

template<typename T>
void VisBufferAsync::copyVector (const Vector<T> & from, Vector<T> & to)
{
    // Make an independent copy of the vector.
    // N.B.: the independence is only at the top Vector level
    //       so any deep dependence is not undone

    Vector<T> tmp = from;
    to = tmp.copy();
}


} // end namespace casa


#endif /* VISIBILITYBUFFERASYNC_H_ */
