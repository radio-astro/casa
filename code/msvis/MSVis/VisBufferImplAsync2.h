/*
 * VisibilityBufferAsync.h
 *
 *  Created on: Nov 3, 2010
 *      Author: jjacobs
 */

#ifndef VISIBILITYBUFFERASYNC2_H_
#define VISIBILITYBUFFERASYNC2_H_

#include <msvis/MSVis/VisBufferImpl2.h>

namespace casa {
namespace vi {

class VisibilityIteratorAsync2;
class VlaDatum;
class VLAT;

class MsIterInfo {

public:

    void fill (const MSIter & msIter);

    Bool allBeamOffsetsZero () const;
    Vector<String>  & antennaMounts () const;
    Int arrayId () const;
    Int dataDescriptionId () const;
    Int fieldId () const;
    String fieldName () const;
    const MFrequency & frequency0 () const;
    const Vector<Double> & getBeamOffsets () const;
    Bool more () const;
    Int msId () const;
    Bool newArray () const;
    Int newDataDescriptionId () const;
    Bool newField () const;
    Bool newMS () const;
    Bool newSpectralWindow () const;
    Int numMS () const;
    const MDirection & phaseCenter () const;
    Int polFrame () const;
    Int polarizationId () const;
    const Matrix<Double> & receptorAngle () const;
    const String & sourceName () const;
    Int spectralWindowId () const;
    const MPosition & telescopePosition () const;


private:

    Bool allBeamOffsetsZero_p;
    Vector<String>  antennaMounts_p;
    Int arrayId_p;
    Int dataDescriptionId_p;
    Int fieldId_p;
    String fieldName_p;
    MFrequency frequency0_p;
    Vector<Double> getBeamOffsets_p;
    Bool more_p;
    Int msId_p;
    Bool newArray_p;
    Int newDataDescriptionId_p;
    Bool newField_p;
    Bool newMS_p;
    Bool newSpectralWindow_p;
    Int numMS_p;
    MDirection phaseCenter_p;
    Int polFrame_p;
    Int polarizationId_p;
    Matrix<Double> receptorAngle_p;
    String sourceName_p;
    Int spectralWindowId_p;
    MPosition telescopePosition_p;

};

class VisBufferImplAsync2 : public VisBufferImpl2 {

    friend class Rovia_Test;
    friend class ViImplAsync2;
    friend class VisBufferAsync2Wrapper2;
    friend class VlaDatum;
    friend class VLAT;

public:

    //VisBufferAsync2 (const VisBuffer& vb);

    ~VisBufferImplAsync2 ();

    const MsIterInfo & msIterInfo () const;

protected:

    // The constructors are not public because creation should be performed
    // by a factory object (e.g., VisBufferAutoPtr).  The use of a factory
    // makes it possible to fine tune at run time whether a VisBuffer or a
    // VisBufferAsync2 is created.

    VisBufferImplAsync2 ();
    VisBufferImplAsync2 (VisibilityIterator2 & iter, Bool isWritable);

//    void attachToVisIter(VisibilityIterator2 & iter);
//    void checkVisIter (const char * func, const char * file, int line, const char * extra = "") const;
//    void clear ();
    void construct ();
//    virtual void copyAsyncValues (const VisBufferAsync2 & other);
//    virtual void copyCache (const VisBuffer2 & other, Bool force);
//    template<typename T> void copyVector (const Vector<T> & from, Vector<T> & to);
//    Vector<MDirection>& fillDirection1();
//    Vector<MDirection>& fillDirection2();
//    void fillFrom (const VisBufferAsync2 & other);
//    MDirection & fillPhaseCenter();
//    Bool getAllBeamOffsetsZero () const;
//    const Vector <String> & getAntennaMounts () const;
//    const Cube <RigidVector <Double, 2> > & getBeamOffsets () const;
//    const MeasurementSet & getMs () const;
//    Int getNSpw () const;
//    MDirection getPhaseCenter () const;
//    const Cube <Double> & getReceptorAngles () const;
//    void setAngleInfo (Bool allBeamOffsetsZero,
//                       const Vector<String> antennaMounts,
//                       Cube<RigidVector<Double, 2> > beamOffsets,
//                       const Cube<Double> & receptorAngles);
//    void initializeScalars ();
//    /////void setDataDescriptionId (Int id);
//    void setFilling (Bool isFilling);
//    void setLsrInfo (const Block <Int> & channelGroupNumber,
//                     const Block <Int> & channelIncrement,
//                     const Block <Int> & channelStart,
//                     const Block <Int> & channelWidth,
//                     const MPosition & observatoryPosition,
//                     const MDirection & phaseCenter,
//                     Bool velocitySelection);
//    void setMeasurementSet (const MeasurementSet & ms);
//    void setMeasurementSetId (Int id, bool isNew);
//    void setMEpoch (const MEpoch & mEpoch);
//    void setMSD (const MSDerivedValues & msd);
//    void setNAntennas (Int);
//    void setNCoh (Int);
//    void setNSpw (Int);
//    void setNewEntityFlags (bool newArrayId, bool newFieldId, bool newSpectralWindow);
//    void setPolarizationId (Int);
//    void setNRowChunk (Int);
//    void setReceptor0Angle (const Vector<Float> & receptor0Angle);
//    void setRowIds (const Vector<uInt> & rowIds);
//    void setSelectedNVisibilityChannels (const Vector<Int> & nVisibilityChannels);
//    void setSelectedSpectralWindows (const Vector<Int> & spectralWindows);
//    void setTopoFreqs (const Vector<Double> & lsrFreq, const Vector<Double> & selFreq_p);
//    void setVisibilityShape (const IPosition & pvisibilityShape);
//    void updateCoordInfo (const VisBuffer2 *, const Bool dirDependent=True);
//
//    static MDirection unsharedCopyDirection (const MDirection & direction);
//    static void unsharedCopyDirectionVector (Vector<MDirection> & direction);
//    static MEpoch unsharedCopyEpoch (const MEpoch & mEpoch);
//    static MPosition unsharedCopyPosition (const MPosition & position);

private:

//    Bool fillAllBeamOffsetsZero ();
//    Vector <String> fillAntennaMounts ();
//    Cube <RigidVector <Double, 2> > fillBeamOffsets ();
//    Cube <Double> fillReceptorAngles ();

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


//template<typename T>
//void VisBufferAsync2::copyVector (const Vector<T> & from, Vector<T> & to)
//{
//    // Make an independent copy of the vector.
//    // N.B.: the independence is only at the top Vector level
//    //       so any deep dependence is not undone
//
//    Vector<T> tmp = from;
//    to = tmp.copy();
//}


} // end namespace vi

} // end namespace casa


#endif /* VISIBILITYBUFFERASYNC_H_ */
