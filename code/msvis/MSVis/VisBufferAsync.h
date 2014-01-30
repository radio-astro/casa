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

namespace asyncio {
    class VlaDatum;
    class VLAT;
}

class VisBufferAsync : public VisBuffer {

    friend class Rovia_Test;
    friend class ViReadImplAsync;
    friend class VisBufferAsyncWrapper;
    friend class VisBufferAutoPtr;
    friend class asyncio::VlaDatum;
    friend class asyncio::VLAT;

public:

    //VisBufferAsync (const VisBuffer& vb);

    ~VisBufferAsync ();

    VisBufferAsync & operator= (const VisBufferAsync & other);

    virtual void allSelectedSpectralWindows(Vector<Int>& spws, Vector<Int>& nvischan);
    virtual VisBufferAsync & assign (const VisBuffer & vb, Bool copy);
    virtual Vector<MDirection> azel(Double time) const;
    virtual MDirection azel0(Double time) const;
    virtual VisBuffer * clone ();
    //virtual Int dataDescriptionId() const;
    virtual void detachFromVisIter ();
    virtual Vector<Float> feed_pa(Double time) const;
    virtual Double hourang(Double time) const;
    virtual void invalidate (); // This one is booby-trapped right now
    virtual void invalidateAsync (); // Use this one in async code
//    virtual Vector<Double> & lsrFrequency ();
//    virtual const Vector<Double> & lsrFrequency () const;
    virtual void lsrFrequency(const Int& spw, Vector<Double>& freq, Bool& convert) const;
    virtual const ROMSColumns& msColumns() const;
    Int msId () const;
    virtual Bool newArrayId () const;
    virtual Bool newFieldId () const;
    Bool newMS() const;
    virtual Bool newSpectralWindow () const;
    Int nRowChunk() const{
      return nRowChunk_p;
    }

    Int numberAnt () const;
    Int numberCoh () const;
    virtual Vector<Float> parang(Double time) const;
    virtual Float parang0(Double time) const;
    virtual Int polarizationId() const;
    virtual Vector<uInt>& rowIds(){throw(AipsError("rowIds() not implemented for VBA."));}
    virtual const Vector<uInt>& rowIds() const {throw(AipsError("rowIds() const not implemented for VBA."));}
    virtual void setCorrectedVisCube(Complex c);
    virtual void setCorrectedVisCube (const Cube<Complex> & vis);
    virtual void setModelVisCube(Complex c);
    virtual void setModelVisCube (const Cube<Complex> & vis);
    virtual void setModelVisCube (const Vector<Float> & stokes);
    virtual void setVisCube(Complex c);
    virtual void setVisCube (const Cube<Complex>& vis);

protected:

    // The constructors are not public because creation should be performed
    // by a factory object (e.g., VisBufferAutoPtr).  The use of a factory
    // makes it possible to fine tune at run time whether a VisBuffer or a
    // VisBufferAsync is created.

    VisBufferAsync ();
    VisBufferAsync (const VisBufferAsync & other);
    VisBufferAsync (ROVisibilityIterator & iter);

    void attachToVisIter(ROVisibilityIterator & iter);
    void checkVisIter (const char * func, const char * file, int line, const char * extra = "") const;
    void clear ();
    void construct ();
    virtual void copyAsyncValues (const VisBufferAsync & other);
    virtual void copyCache (const VisBuffer & other, Bool force);
    template<typename T> void copyVector (const Vector<T> & from, Vector<T> & to);
    Vector<MDirection>& fillDirection1();
    Vector<MDirection>& fillDirection2();
    void fillFrom (const VisBufferAsync & other);
    MDirection & fillPhaseCenter();
    Bool getAllBeamOffsetsZero () const;
    const Vector <String> & getAntennaMounts () const;
    const Cube <RigidVector <Double, 2> > & getBeamOffsets () const;
    const MeasurementSet & getMs () const;
    Int getNSpw () const;
    MDirection getPhaseCenter () const;
    const Cube <Double> & getReceptorAngles () const;
    void setAngleInfo (Bool allBeamOffsetsZero,
                       const Vector<String> antennaMounts,
                       Cube<RigidVector<Double, 2> > beamOffsets,
                       const Cube<Double> & receptorAngles);
    void initializeScalars ();
    /////void setDataDescriptionId (Int id);
    void setFilling (Bool isFilling);
    void setLsrInfo (const Block <Int> & channelGroupNumber,
                     const Block <Int> & channelIncrement,
                     const Block <Int> & channelStart,
                     const Block <Int> & channelWidth,
                     const MPosition & observatoryPosition,
                     const MDirection & phaseCenter,
                     Bool velocitySelection);
    void setMeasurementSet (const MeasurementSet & ms);
    void setMeasurementSetId (Int id, bool isNew);
    void setMEpoch (const MEpoch & mEpoch);
    void setMSD (const MSDerivedValues & msd);
    void setNAntennas (Int);
    void setNCoh (Int);
    void setNSpw (Int);
    void setNewEntityFlags (bool newArrayId, bool newFieldId, bool newSpectralWindow);
    void setPolarizationId (Int);
    void setNRowChunk (Int);
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

    Bool fillAllBeamOffsetsZero ();
    Vector <String> fillAntennaMounts ();
    Cube <RigidVector <Double, 2> > fillBeamOffsets ();
    Cube <Double> fillReceptorAngles ();

    Bool                           allBeamOffsetsZero_p;
    Vector<String>                 antennaMounts_p;
    mutable Vector<MDirection>     azelCached_p;      // mutable because it is a cached value
    mutable Double                 azelCachedTime_p;  // mutable because it is a cached value
    Cube<RigidVector<Double, 2> >  beamOffsets_p;
    Block<Int>                     channelGroupNumber_p;
    Block<Int>                     channelIncrement_p;
    Block<Int>                     channelStart_p;
    Block<Int>                     channelWidth_p;
    //Int                            dataDescriptionId_p;
    mutable Vector<Float>          feedpaCached_p;      // mutable because it is a cached value
    mutable Double                 feedpaCachedTime_p;  // mutable because it is a cached value
    Bool                           isFilling_p;
    Vector<Double>                 lsrFrequency_p; // calculated by getTopoFreqs if velSelection_p
    MEpoch                         mEpoch_p;
    const MeasurementSet *         measurementSet_p;  // [use]
    mutable ROMSColumns *          msColumns_p; // [own]
    MSDerivedValues *              msd_p; // [own]
    Int                            nAntennas_p;
    Int                            nCoh_p;
    Bool                           newArrayId_p;
    Bool                           newFieldId_p;
    Bool                           newSpectralWindow_p;
    Int                            nRowChunk_p;
    Int                            nSpw_p;
    //const ROScalarColumn<Int> *    obsMFreqTypes_p; // [use]
    MPosition                      observatoryPosition_p;
    mutable Vector<Float>          parangCached_p;      // mutable because it is a cached value
    mutable Double                 parangCachedTime_p;  // mutable because it is a cached value
    Int                            polarizationId_p;
    Vector<Float>                  receptor0Angle_p;
    Cube<Double>                   receptorAngles_p;
    Vector<Double>                 selFreq_p;
    Vector<Int>                    selectedNVisibilityChannels_p;
    Vector<Int>                    selectedSpectralWindows_p;
    Bool                           velSelection_p;
    IPosition                      visibilityShape_p;
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
