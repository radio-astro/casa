/*
 * VisibilityBufferAsync.cc
 *
 *  Created on: Nov 1, 2010
 *      Author: jjacobs
 */

#include "UtilJ.h"
using namespace casa::utilj;

#include <casa/Containers/Record.h>

#include <synthesis/MSVis/VisBufferAsync2.h>
#include <synthesis/MSVis/VisBufferAsyncWrapper2.h>
#include <synthesis/MSVis/VisibilityIterator2.h>
#include <synthesis/MSVis/AsynchronousInterface2.h>
#include <synthesis/MSVis/VLAT.h>

#include <algorithm>

using std::transform;

#include <ms/MeasurementSets/MSColumns.h>
#include <typeinfo>

#define Log(level, ...) \
    {if (casa::asyncio::AsynchronousInterface::logThis (level)) \
         casa::async::Logger::get()->log (__VA_ARGS__);};

namespace casa {

namespace asyncio {

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

} // end namespace asyncio

VisBufferAsync2::VisBufferAsync2 ()
  : VisBuffer ()
{
    construct();
}

VisBufferAsync2::VisBufferAsync2 (const VisBufferAsync2 & vb)
: VisBuffer2 ()
{
//    const VisBufferAsync2 * vb = dynamic_cast<const VisBufferAsync2 *> (& vb0);
//    ThrowIf (vb == NULL, "Cannot copy VisBuffer into VisBufferAsync2");

    construct ();

    * this = vb;
}

VisBufferAsync2::VisBufferAsync2 (ROVisibilityIterator & iter)
  : VisBuffer2 ()
{
    construct ();

    Log (2, "VisBufferAsync2::VisBufferAsync2: attaching in constructor this=%08x\n", this);
    attachToVisIter (iter);
}


VisBufferAsync2::~VisBufferAsync2 ()
{
    Log (2, "Destroying VisBufferAsync2; addr=0x%016x\n", this);

    // Should never be attached at the synchronous level

    delete msColumns_p;
    delete msd_p;
}

VisBufferAsync2 &
VisBufferAsync2::operator= (const VisBufferAsync2 & other)
{
    if (this != & other){

        assign (other, True);
    }

    return * this;
}

void
VisBufferAsync2::allSelectedSpectralWindows(Vector<Int> & spectralWindows,
                                           Vector<Int>& nVisibilityChannels)
{
    spectralWindows.assign (selectedSpectralWindows_p);
    nVisibilityChannels.assign (selectedNVisibilityChannels_p);
}

VisBufferAsync2 &
VisBufferAsync2::assign (const VisBuffer2 & other, Bool copy)
{
    if (this != & other){

        //    assert (copy); // The copy parameter is ignored since it makes no sense to
        //                   // assign a VBA without copying its values

        Log (2, "Assign from VisBufferAsync2 @ 0x%08x to VisBufferAsync2 @ 0x%08x\n", & other, this);

        Assert (dynamic_cast<const VisBufferAsync2 *> (& other) != NULL);
        Assert (visIter_p == NULL); // shouldn't be attached at sync level

        if (other.corrSorted_p)
            throw(AipsError("Cannot assign a VisBuffer that has had correlations sorted!"));

        azelCachedTime_p = 0; // flush this cached value
        feedpaCachedTime_p = 0; // flush this cached value
        parangCachedTime_p = 0; // flush this cached value

        if (copy){

            // Let the standard VisBuffer do the copying of values
            // from the old VisBuffer

            copyCache (other, False);

            // Copy over the async values

            copyAsyncValues (dynamic_cast<const VisBufferAsync2 &> (other));

        }
    }

    return * this;
}

Vector<MDirection>
VisBufferAsync2::azel(Double time) const
{
    if (time != azelCachedTime_p){

        azelCachedTime_p = time;

        ROVisibilityIterator::azelCalculate (time, * msd_p, azelCached_p, nAntennas_p, mEpoch_p);
    }

    return azelCached_p;
}

MDirection
VisBufferAsync2::azel0(Double time) const
{
    MDirection azel0;
    //MSDerivedValues msd;
    //msd.setMeasurementSet (* measurementSet_p);


    ROVisibilityIterator::azel0Calculate (time, * msd_p, azel0, mEpoch_p);

    return azel0;
}

void
VisBufferAsync2::checkVisIter (const char * func, const char * file, int line, const char * extra) const
{
    // This is called from a VisBuffer fill method.  Throw an error if the this is not
    // part of the VLAT filling operation or if there is not visibility iterator attached

    if (visIter_p == NULL){
        Log (1, "VisBufferAsync2: request for column not in prefetched set (in call to %s (%s))\n)",
             func, extra);
        throw AipsErrorTrace ("VisBufferAsync2: request for column not in prefetched set (in call to "
                              + String (func) + String (extra) + ")", file, line);
    }
}


void
VisBufferAsync2::clear ()
{
    Log (2, "Clearing VisBufferAsync2; addr=0x%016x\n", this);

    // Put scalars to deterministic values

    dataDescriptionId_p = -1;
    measurementSet_p = NULL;
    nAntennas_p = 0;

    invalidateAsync ();  // set all the flags to indicate caches are empty

    // Since VBA was created using a shallow copy of shared data
    // it is necessary to break the linkage with the shared data
    // while the shared data is locked.  Otherwise there could be
    // some interaction between the base and lookahead threads
    // because of the reference counted data structures, etc.

    // Wipe the fields that are part of VisBuffer proper

    if (antenna1OK_p)
        antenna1_p.resize();
    if (antenna2OK_p)
        antenna2_p.resize();
    chanAveBounds_p.resize();
    //chanFreqs_p = NULL;
    if (channelOK_p)
        channel_p.resize();
    if (cjonesOK_p)
        cjones_p.resize();
    if (corrTypeOK_p)
        corrType_p.resize();
    if (correctedVisCubeOK_p)
        correctedVisCube_p.resize();
    if (correctedVisibilityOK_p)
        correctedVisibility_p.resize();
    if (direction1OK_p)
        direction1_p.resize();
    if (direction2OK_p)
        direction2_p.resize();
    if (feed1OK_p)
        feed1_p.resize();
    if (feed1_paOK_p)
        feed1_pa_p.resize();
    if (feed2OK_p)
        feed2_p.resize();
    if (feed2_paOK_p)
        feed2_pa_p.resize();
    if (flagCubeOK_p)
        flagCube_p.resize();
    if (flagRowOK_p)
        flagRow_p.resize();
    if (flagOK_p)
        flag_p.resize();
    if (frequencyOK_p)
        frequency_p.resize();
    if (imagingWeightOK_p)
        imagingWeight_p.resize();
//    if (lsrFrequencyOK_p)
//        lsrFrequency_p.resize();
    measurementSet_p = NULL;
    if (modelVisCubeOK_p)
        modelVisCube_p.resize();
    if (modelVisibilityOK_p)
        modelVisibility_p.resize();
    delete msColumns_p;
    msColumns_p = NULL;
    nAntennas_p = -1;
    nCoh_p = -1;
    phaseCenter_p = MDirection ();
    if (rowIdsOK_p)
        rowIds_p.resize();
    if (scanOK_p)
        scan_p.resize();
    if (sigmaMatOK_p)
        sigmaMat_p.resize();
    if (sigmaOK_p)
        sigma_p.resize();
    if (timeIntervalOK_p)
        timeInterval_p.resize();
    if (timeOK_p)
        time_p.resize();
    if (uvwMatOK_p)
        uvwMat_p.resize();
    if (uvwOK_p)
        uvw_p.resize();
    if (visCubeOK_p)
        visCube_p.resize();
    if (visibilityOK_p)
        visibility_p.resize();
    weightCube_p.resize();
    if (weightMatOK_p)
        weightMat_p.resize();
    if (weightSpectrumOK_p)
        weightSpectrum_p.resize();
    if (weightOK_p)
        weight_p.resize();

    // Wipe fields that are part of VisBufferAsync2 only

    //delete msd_p;
    //msd_p = new MSDerivedValues();

    channelGroupNumber_p.resize (0);
    channelIncrement_p.resize (0);
    channelStart_p.resize (0);
    channelWidth_p.resize (0);
    lsrFrequency_p.resize (0);
    mEpoch_p = MEpoch();
    observatoryPosition_p = MPosition();
    phaseCenter_p = MDirection();
    receptor0Angle_p.resize (0);
    rowIds_p.resize (0);
    selFreq_p.resize (0);
    selectedNVisibilityChannels_p.resize (0);
    selectedSpectralWindows_p.resize (0);
    visibilityShape_p.resize (0, False);
}

VisBuffer *
VisBufferAsync2::clone ()
{
    return new VisBufferAsync2 (* this);
}


void
VisBufferAsync2::construct ()
{
    Log (2, "Constructing VisBufferAsync2; addr=0x%016x\n", this);

    initializeScalars ();

    azelCachedTime_p = 0; // tag azel as currently uncached
    feedpaCachedTime_p = 0; // tag azel as currently uncached
    parangCachedTime_p = 0; // tag azel as currently uncached
    msColumns_p = NULL;
    visIter_p = NULL;
    isFilling_p = False;
    msd_p = new MSDerivedValues();

    clear ();
}

void
VisBufferAsync2::copyCache (const VisBuffer & other, Bool force)
{
    assert (! force);

    VisBuffer::copyCache (other, False);
}


void
VisBufferAsync2::copyAsyncValues (const VisBufferAsync2 & other)
{

    channelGroupNumber_p = other.channelGroupNumber_p;
    channelIncrement_p = other.channelIncrement_p;
    channelStart_p = other.channelStart_p;
    channelWidth_p = other.channelWidth_p;

    dataDescriptionId_p = other.dataDescriptionId_p;
    lsrFrequency_p = other.lsrFrequency_p;
    measurementSet_p = other.measurementSet_p;
    mEpoch_p = other.mEpoch_p;


    delete msColumns_p; // kill the current one
    msColumns_p = NULL;

    nAntennas_p = other.nAntennas_p;
    newArrayId_p = other.newArrayId_p;
    newFieldId_p = other.newFieldId_p;
    newSpectralWindow_p = other.newSpectralWindow_p;
    nRowChunk_p = other.nRowChunk_p;
//    obsMFreqTypes_p = other.obsMFreqTypes_p;
    observatoryPosition_p = other.observatoryPosition_p;
    phaseCenter_p = other.phaseCenter_p;
    receptor0Angle_p = other.receptor0Angle_p;
    rowIds_p = other.rowIds_p;
    selectedNVisibilityChannels_p = other.selectedNVisibilityChannels_p;
    selectedSpectralWindows_p = other.selectedSpectralWindows_p;
    selFreq_p = other.selFreq_p;
    velSelection_p = other.velSelection_p;
    visibilityShape_p = other.visibilityShape_p;

    setMSD (* other.msd_p);
}

void
VisBufferAsync2::detachFromVisIter ()
{

	if (isFilling_p){
	    VisBuffer::detachFromVisIter();
	}
}

Vector<Float>
VisBufferAsync2::feed_pa(Double time) const
{
    if (time != feedpaCachedTime_p){

        feedpaCachedTime_p = time;

        if (visIter_p != NULL){

            // This method can be called during filling; if so then let it
            // work the original way (possible hole for detached VBAs).

            feedpaCached_p = VisBuffer::feed_pa (time);
        }
        else{

            //MSDerivedValues msd;
            //msd.setMeasurementSet (* measurementSet_p);

            feedpaCached_p.assign (ROVisibilityIterator::feed_paCalculate (time, * msd_p, nAntennas_p,
                                                                           mEpoch_p, receptor0Angle_p));
        }

    }

    return feedpaCached_p;
}

Bool
VisBufferAsync2::fillAllBeamOffsetsZero ()
{
    allBeamOffsetsZero_p = visIter_p->allBeamOffsetsZero();

    return allBeamOffsetsZero_p;
}

Vector <String>
VisBufferAsync2::fillAntennaMounts ()
{
    antennaMounts_p = visIter_p->antennaMounts();

    return antennaMounts_p;
}

Cube <RigidVector <Double, 2> >
VisBufferAsync2::fillBeamOffsets ()
{
    beamOffsets_p = visIter_p->getBeamOffsets ();

    return beamOffsets_p;
}

Cube <Double>
VisBufferAsync2::fillReceptorAngles ()
{
    receptorAngles_p = visIter_p->receptorAngles();

    return receptorAngles_p;
}


void
VisBufferAsync2::fillFrom (const VisBufferAsync2 & other)
{
    // Almost like assignment except that the attachment to the
    // Visibility iterator is preserved.  Only used for copying
    // data from the buffer ring into the user's VB

    Log (2, "Fill from VisBufferAsync2 @ 0x%08x to VisBufferAsync2 @ 0x%08x\n", & other, this);

    copyCache (other, False);
    copyAsyncValues (other);

}

Vector<MDirection>&
VisBufferAsync2::fillDirection1()
{
    // Perform filling in the normal way

    VisBuffer::fillDirection1();

    if (isFilling_p) {

        // Now install unshared copies of the direction objects

        unsharedCopyDirectionVector (direction1_p);

    }

    return direction1_p;
}



Vector<MDirection>&
VisBufferAsync2::fillDirection2()
{
    // Perform filling in the normal way

    VisBuffer::fillDirection2();

    if (isFilling_p){

        // Now install unshared copies of the direction objects

        unsharedCopyDirectionVector (direction2_p);

    }

    return direction2_p;
}

MDirection &
VisBufferAsync2::fillPhaseCenter()
{
    // Perform filling in the normal way

    VisBuffer::fillPhaseCenter();

    // Now convert the value to an unshared one.

    phaseCenter_p = unsharedCopyDirection (phaseCenter_p);
    phaseCenterOK_p = True;

    return phaseCenter_p;
}

Bool
VisBufferAsync2::getAllBeamOffsetsZero () const
{
    return allBeamOffsetsZero_p;
}

const Vector <String> &
VisBufferAsync2::getAntennaMounts () const
{
    return antennaMounts_p;
}

const Cube <RigidVector <Double, 2> > &
VisBufferAsync2::getBeamOffsets () const
{
    return beamOffsets_p;
}


const MeasurementSet &
VisBufferAsync2::getMs () const
{
    return * measurementSet_p;
}

Int
VisBufferAsync2::getNSpw () const
{
    return nSpw_p;
}


MDirection
VisBufferAsync2::getPhaseCenter () const
{
    return phaseCenter_p;
}


const Cube <Double> &
VisBufferAsync2::getReceptorAngles () const
{
    return receptorAngles_p;
}

Double
VisBufferAsync2::hourang(Double time) const
{
    //MSDerivedValues msd;
    //msd.setMeasurementSet (* measurementSet_p);

    Double hourang = ROVisibilityIterator::hourangCalculate (time, * msd_p, mEpoch_p);

    return hourang;
}

void
VisBufferAsync2::initializeScalars ()
{
    // Set all VBA specific scalars to known values to prevent
    // nondeterminism.

    dataDescriptionId_p = -1;
    feedpaCachedTime_p = -1;
    isFilling_p = False;
    measurementSet_p = NULL;
    msColumns_p = NULL;
    msd_p = NULL;
    nAntennas_p = -1;
    nCoh_p = -1;
    newArrayId_p = False;
    newFieldId_p = False;
    newSpectralWindow_p = False;
    nRowChunk_p = -1;
    nSpw_p = -1;
    parangCachedTime_p = -1;
    polarizationId_p = -1;
    velSelection_p = False;
}

void
VisBufferAsync2::invalidate ()
{
    // This is called by synchronous code.  Just ignore it since
    // the cache validity is being managed by async code.  To really
    // invalidate use invalidateAsync.
}

void
VisBufferAsync2::invalidateAsync ()
{
    // A way for the new code to invalidate the buffer that
    // can be distinguished from the old way.

    VisBuffer::invalidate ();
}

void
VisBufferAsync2::lsrFrequency (const Int& spw, Vector<Double>& freq, Bool& convert) const
{
    if (velSelection_p) {
        freq.assign (lsrFrequency_p);
        convert = False;
        return;
    }

    const ROArrayColumn <Double> & chanFreqs = msColumns().spectralWindow().chanFreq();
    const ROScalarColumn<Int> & obsMFreqTypes= msColumns().spectralWindow().measFreqRef();

    MPosition obsPos = observatoryPosition_p;
    MDirection dir = phaseCenter_p;

    ROVisibilityIterator::lsrFrequency (spw, freq, convert, channelStart_p, channelWidth_p, channelIncrement_p,
                                        channelGroupNumber_p, chanFreqs, obsMFreqTypes, mEpoch_p, obsPos, dir);
}

const ROMSColumns &
VisBufferAsync2::msColumns() const
{
    if (isFilling_p){

        Assert (visIter_p != NULL);

        return visIter_p->msColumns ();
    }
    else {

        Assert (measurementSet_p != NULL);

        if (msColumns_p == NULL){
            msColumns_p = new ROMSColumns (* measurementSet_p);
        }

        return * msColumns_p;
    }
}

Int
VisBufferAsync2::msId () const
{
    return oldMSId_p;
}

Bool
VisBufferAsync2::newArrayId () const
{
    return newArrayId_p;
}

Bool
VisBufferAsync2::newFieldId () const
{
    return newFieldId_p;
}

Bool
VisBufferAsync2::newSpectralWindow () const
{
    return newSpectralWindow_p;
}


Bool
VisBufferAsync2::newMS() const
{
    return newMS_p;
}


Int
VisBufferAsync2::numberAnt () const
{
  return nAntennas_p;
}

Int
VisBufferAsync2::numberCoh () const
{
    return nCoh_p;
}


Vector<Float>
VisBufferAsync2::parang(Double time) const
{
    if (time != parangCachedTime_p){

        parangCachedTime_p = time;

        parangCached_p = ROVisibilityIterator::parangCalculate (time, * msd_p, nAntennas_p, mEpoch_p);
    }

    return parangCached_p;
}

Float
VisBufferAsync2::parang0(Double time) const
{
    //MSDerivedValues msd;
    //msd.setMeasurementSet (* measurementSet_p);

    Float parang0 = ROVisibilityIterator::parang0Calculate (time, * msd_p, mEpoch_p);

    return parang0;
}

Int
VisBufferAsync2::polarizationId() const
{
    return polarizationId_p;
}



//Vector<uInt>&
//VisBufferAsync2::rowIds()
//{
//    return rowIds_p;
//}

//void
//VisBufferAsync2::setDataDescriptionId (Int id)
//{
//    dataDescriptionId_p = id;
//}

void
VisBufferAsync2::setCorrectedVisCube(Complex c)
{
    correctedVisCube_p.resize(visibilityShape_p);
    correctedVisCube_p.set(c);
    correctedVisCubeOK_p=True;
}

void
VisBufferAsync2::setCorrectedVisCube (const Cube<Complex> & vis)
{
    VisBuffer::setCorrectedVisCube (vis);
}


void
VisBufferAsync2::setModelVisCube (const Cube<Complex> & vis)
{
    VisBuffer::setModelVisCube (vis);
}

void
VisBufferAsync2::setModelVisCube (const Vector<Float> & stokes)
{
    VisBuffer::setModelVisCube (stokes);
}

void
VisBufferAsync2::setNRowChunk (Int nRowChunk)
{
    nRowChunk_p = nRowChunk;
}


void
VisBufferAsync2::setFilling (Bool isFilling)
{
    isFilling_p = isFilling;
}

void
VisBufferAsync2::setLsrInfo (const Block <Int> & channelGroupNumber,
                            const Block <Int> & channelIncrement,
                            const Block <Int> & channelStart,
                            const Block <Int> & channelWidth,
                            const MPosition & observatoryPosition,
                            const MDirection & phaseCenter,
                            Bool velocitySelection)
{
    channelGroupNumber_p = channelGroupNumber;
    channelIncrement_p = channelIncrement;
    channelStart_p = channelStart;
    channelWidth_p = channelWidth;
    observatoryPosition_p = unsharedCopyPosition (observatoryPosition);
    phaseCenter_p = unsharedCopyDirection (phaseCenter);
    velSelection_p = velocitySelection;
}


void
VisBufferAsync2::setMeasurementSet (const MeasurementSet & ms)
{
    measurementSet_p = & ms;
    //msd_p->setMeasurementSet (ms);
}

void
VisBufferAsync2::setMeasurementSetId (Int id, Bool isNew)
{
    oldMSId_p = id;
    msOK_p = True;
    newMS_p = isNew;
}


void
VisBufferAsync2::setMEpoch (const MEpoch & mEpoch)
{
    mEpoch_p = unsharedCopyEpoch (mEpoch);
}

void
VisBufferAsync2::setModelVisCube(Complex c)
{
    modelVisCube_p.resize(visibilityShape_p);
    modelVisCube_p.set(c);
    modelVisCubeOK_p=True;
}

void
VisBufferAsync2::setMSD (const MSDerivedValues & msd)
{
    msd_p->setEpoch (mEpoch_p);

    // set antennas

    const Vector<MPosition> & antennaPositions = msd.getAntennaPositions();
    Vector<MPosition> unsharedAntennaPositions (antennaPositions.nelements());

    for (Vector<MPosition>::const_iterator ap = antennaPositions.begin();
            ap != antennaPositions.end();
            ap ++){

        unsharedAntennaPositions = unsharedCopyPosition (* ap);
    }

    msd_p->setAntennaPositions (unsharedAntennaPositions);

    msd_p->mount_p.assign (msd.mount_p);

    msd_p->setObservatoryPosition (observatoryPosition_p);

    msd_p->setFieldCenter (phaseCenter_p);

    msd_p->setVelocityFrame (msd.getRadialVelocityType ());
}


void
VisBufferAsync2::setNAntennas (Int nAntennas)
{
    nAntennas_p = nAntennas;
}

void
VisBufferAsync2::setNCoh (Int nCoh)
{
    nCoh_p = nCoh;
}

void
VisBufferAsync2::setNewEntityFlags (bool newArrayId, bool newFieldId, bool newSpectralWindow)
{
    newArrayId_p = newArrayId;
    newFieldId_p = newFieldId;
    newSpectralWindow_p = newSpectralWindow;
}

void
VisBufferAsync2::setNSpw (Int nSpw)
{
    nSpw_p = nSpw;
}


void
VisBufferAsync2::setPolarizationId (Int pId)
{
    polarizationId_p = pId;
}

void
VisBufferAsync2::setReceptor0Angle (const Vector<Float> & angles)
{
    receptor0Angle_p = angles;
}

void
VisBufferAsync2::setRowIds (const Vector<uInt> & rowIds)
{
    rowIds_p = rowIds;
}

void
VisBufferAsync2::setSelectedNVisibilityChannels (const Vector<Int> & nVisibilityChannels)
{
    selectedNVisibilityChannels_p.assign (nVisibilityChannels);
}

void
VisBufferAsync2::setSelectedSpectralWindows (const Vector<Int> & spectralWindows)
{
    selectedSpectralWindows_p.assign (spectralWindows);
}


void
VisBufferAsync2::setTopoFreqs (const Vector<Double> & lsrFreq, const Vector<Double> & selFreq)
{
    lsrFrequency_p.assign (lsrFreq);
    selFreq_p.assign (selFreq);
}

void
VisBufferAsync2::setVisCube(Complex c)
{
    visCube_p.resize(visibilityShape_p);
    visCube_p.set(c);
    visCubeOK_p=True;
}

void
VisBufferAsync2::setVisCube (const Cube<Complex>& vis)
{
    VisBuffer::setVisCube (vis);
}


void
VisBufferAsync2::setVisibilityShape (const IPosition & visibilityShape)
{
    visibilityShape_p = visibilityShape;
}

MDirection
VisBufferAsync2::unsharedCopyDirection (const MDirection & direction)
{
    return asyncio::unsharedCopyMeasure (direction, & MeasureHolder::asMDirection);
}

void
VisBufferAsync2::unsharedCopyDirectionVector (Vector<MDirection> & direction)
{
    // The MDirection object consists of the actual direction a frame of reference
    // object.  The direction component follows copy semnatics while the frame of
    // reference uses a counter pointer and is shared across multiple MDirections.
    // This causes problems now that values are being shared across threads.  Modify
    // the direction vector so that the frame of reference object is only shared
    // within this vector.  N.B., this could cause a problem if there is comparison
    // between the reference frames of the two direction.  Each element doesn't get
    // its own copy because the unsharing operation is klugy and somewhat compute
    // intensive.

    int nElements = direction.shape () [0];

    if (nElements > 0){

        // Take the first reference frame object, make an unshared copy and provide it to
        // any of the other direction components which use the same reference.  If a
        // direction uses a different reference give it its own unshared copy.

        MeasRef<MDirection> uncleanRef = direction[0].getRef ();
        MDirection cleanDirection = unsharedCopyDirection (direction[0]);
        MeasRef<MDirection> cleanRef = cleanDirection.getRef ();
            // Since cleanRef uses a counted pointer to an underlying object
            // the above construction does not cause a dangling pointer problem.

        for (int i = 0; i < nElements; i++){

            // If this element of the direction uses the same reference as the
            // original, then just install a reference to our "clean" reference.
            // N.B., the == comparasion just compares the pointer to the underlying
            // reference frame object!

            if (uncleanRef == direction[i].getRef()){

                direction[i].set (cleanRef);
            }
            else {
                direction[i] = unsharedCopyDirection (direction[i]);
            }
        }
    }
}

MEpoch
VisBufferAsync2::unsharedCopyEpoch (const MEpoch & epoch)
{
    return asyncio::unsharedCopyMeasure (epoch, & MeasureHolder::asMEpoch);
}

MPosition
VisBufferAsync2::unsharedCopyPosition (const MPosition & position)
{
    return asyncio::unsharedCopyMeasure (position, & MeasureHolder::asMPosition);
}

void
VisBufferAsync2::updateCoordInfo(const VisBuffer * other, const Bool dirDepend)
{
    clear ();
    copyVector (other->antenna1_p, antenna1_p);
    antenna1OK_p = true;
    copyVector (other->antenna2_p, antenna2_p);
    antenna2OK_p = true;
    copyVector (other->time_p, time_p);
    timeOK_p = true;
    copyVector (other->frequency_p, frequency_p);
    frequencyOK_p = true;
    copyVector (other->feed1_p, feed1_p);
    feed1OK_p = true;
    copyVector (other->feed2_p, feed2_p);
    feed2OK_p = true;
    if(dirDepend){
      copyVector (other->feed1_pa_p, feed1_pa_p);
      feed1_paOK_p = true;
      copyVector (other->feed2_pa_p, feed2_pa_p);
      feed2_paOK_p = true;
      //copyVector (direction1_p);
      //copyVector (direction2_p);
    }
    else{
      feed1_paOK_p=False; 
      feed2_paOK_p=False;
      direction1OK_p=False;
      direction2OK_p=False;
    }
}




} // end namespace casa


