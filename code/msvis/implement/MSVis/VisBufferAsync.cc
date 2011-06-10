/*
 * VisibilityBufferAsync.cc
 *
 *  Created on: Nov 1, 2010
 *      Author: jjacobs
 */

#include "UtilJ.h"
using namespace casa::utilj;

#include <casa/Containers/Record.h>

#include "VisBufferAsync.h"
#include "VisBufferAsyncWrapper.h"
#include "VisibilityIterator.h"
#include "VisibilityIteratorAsync.h"
#include "VLAT.h"
#include <algorithm>

using std::transform;

#include <ms/MeasurementSets/MSColumns.h>
#include <typeinfo>

#define Log(level, ...) \
    {if (VlaData::loggingInitialized_p && level <= VlaData::logLevel_p) \
         Logger::get()->log (__VA_ARGS__);};

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

}

VisBufferAsync::VisBufferAsync ()
  : VisBuffer ()
{
    construct();
}

VisBufferAsync::VisBufferAsync (const VisBufferAsync & vb)
: VisBuffer ()
{
//    const VisBufferAsync * vb = dynamic_cast<const VisBufferAsync *> (& vb0);
//    ThrowIf (vb == NULL, "Cannot copy VisBuffer into VisBufferAsync");

    construct ();

    * this = vb;
}

//VisBufferAsync::VisBufferAsync (ROVisibilityIteratorAsync & iter)
//  : VisBuffer ()
//{
//    construct ();
//
//    Log (2, "VisBufferAsync::VisBufferAsync: attaching in constructor this=%08x\n", this);
//    attachToVisIterAsync (iter);
//
//}


VisBufferAsync::~VisBufferAsync ()
{
    Log (2, "Destroying VisBufferAsync; addr=0x%016x\n", this);

    Assert (visIter_p == NULL); // Should never be attached at the
                                // synchronous level

    delete msColumns_p;
    //delete msd_p;
}

VisBufferAsync &
VisBufferAsync::operator= (const VisBufferAsync & other)
{
    if (this != & other){

        assign (other, True);
    }

    return * this;
}

void
VisBufferAsync::allSelectedSpectralWindows(Vector<Int> & spectralWindows,
                                           Vector<Int>& nVisibilityChannels)
{
    spectralWindows.assign (selectedSpectralWindows_p);
    nVisibilityChannels.assign (selectedNVisibilityChannels_p);
}

VisBufferAsync &
VisBufferAsync::assign (const VisBuffer & other, Bool copy)
{

    //    assert (copy); // The copy parameter is ignored since it makes no sense to
    //                   // assign a VBA without copying its values

    Log (2, "Assign from VisBufferAsync @ 0x%08x to VisBufferAsync @ 0x%08x\n", & other, this);

    Assert (dynamic_cast<const VisBufferAsync *> (& other) != NULL);
    Assert (visIter_p == NULL); // shouldn't be attached at sync level

    if (other.corrSorted_p)
        throw(AipsError("Cannot assign a VisBuffer that has had correlations sorted!"));

    if (this != & other){

        // Detach from visibility iterator if attached

        if (copy){

            // Let the standard VisBuffer to the copying of values
            // from the old VisBuffer

            copyCache (other);

            // Copy over the async values

            copyAsyncValues (dynamic_cast<const VisBufferAsync &> (other));

            // Do not retain any connection to the other's visibility
            // iterator.

            visIter_p = NULL;

        }
    }

    return * this;
}

void
VisBufferAsync::attachToVisIter(ROVisibilityIterator & iter)
{
    Assert (isFilling_p == True); // Only allowed while being filled
    Assert (visIter_p == NULL);   // Shouldn't already be attached

    VisBuffer::attachToVisIter(iter);
}

Vector<MDirection>
VisBufferAsync::azel(Double time) const
{
    Vector<MDirection> azel;

    //MSDerivedValues msd;
    //msd.setMeasurementSet (* measurementSet_p);

    ROVisibilityIteratorAsync::azelCalculate (time, * msd_p, azel, nAntennas_p, mEpoch_p);

    return azel;
}

MDirection
VisBufferAsync::azel0(Double time) const
{
    MDirection azel0;
    //MSDerivedValues msd;
    //msd.setMeasurementSet (* measurementSet_p);


    ROVisibilityIteratorAsync::azel0Calculate (time, * msd_p, azel0, mEpoch_p);

    return azel0;
}

void
VisBufferAsync::checkVisIter (const char * func, const char * file, int line) const
{
    // This is called from a VisBuffer fill method.  Throw an error if the this is not
    // part of the VLAT filling operation or if there is not visibility iterator attached

    if (! isFilling_p || visIter_p == NULL){
        Log (1, "VisBufferAsync: request for column not in prefetched set (in call to %s)\n)",
             func);
        throw AipsErrorTrace ("VisBufferAsync: request for column not in prefetched set (in call to "
                              + String (func) + ")", file, line);
    }
}


void
VisBufferAsync::clear ()
{
    Log (2, "Clearing VisBufferAsync; addr=0x%016x\n", this);

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
    chanFreqs_p = NULL;
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
    if (lsrFrequencyOK_p)
        lsrFrequency_p.resize();
    measurementSet_p = NULL;
    if (modelVisCubeOK_p)
        modelVisCube_p.resize();
    if (modelVisibilityOK_p)
        modelVisibility_p.resize();
    delete msColumns_p;
    msColumns_p = NULL;
    msID_p = -1;
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

    // Wipe fields that are part of VisBufferAsync only

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

void
VisBufferAsync::construct ()
{
    Log (2, "Constructing VisBufferAsync; addr=0x%016x\n", this);
    msColumns_p = NULL;
    visIter_p = NULL;
    isFilling_p = False;
    msd_p = new MSDerivedValues();

    clear ();
}

void
VisBufferAsync::copyCache (const VisBuffer & other)
{
    VisBuffer::copyCache (other);
}


void
VisBufferAsync::copyAsyncValues (const VisBufferAsync & other)
{

    chanFreqs_p = other.chanFreqs_p;
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

    msID_p = other.msID_p;
    nAntennas_p = other.nAntennas_p;
    obsMFreqTypes_p = other.obsMFreqTypes_p;
    observatoryPosition_p = other.observatoryPosition_p;
    phaseCenter_p = other.phaseCenter_p;
    receptor0Angle_p = other.receptor0Angle_p;
    rowIds_p = other.rowIds_p;
    selectedNVisibilityChannels_p = other.selectedNVisibilityChannels_p;
    selectedSpectralWindows_p = other.selectedSpectralWindows_p;
    selFreq_p = other.selFreq_p;
    velSelection_p = other.velSelection_p;
    visibilityShape_p = other.visibilityShape_p;
}


//VisBuffer *
//VisBufferAsync::create (const VisBuffer & other)
//{
//
//    VisBuffer * result;
//
//    if (ROVisibilityIteratorAsync::isAsynchronousIoEnabled ()){
//
//        result = new VisBufferAsync (other);
//
//    }
//    else {
//
//
//        result = new VisBuffer (other);
//    }
//
//    return result;
//}


//VisBuffer *
//VisBufferAsync::create (ROVisibilityIterator & iter)
//{
//    return create (& iter);
//}
//
//VisBuffer *
//VisBufferAsync::create (ROVisibilityIterator * iter)
//{
//    VisBuffer * result = NULL;
//    ROVisibilityIteratorAsync * via = dynamic_cast<ROVisibilityIteratorAsync *> (iter);
//
//    bool createAsynchronous = ROVisibilityIteratorAsync::isAsynchronousIoEnabled () &&
//                              via != NULL;
//
//    if (createAsynchronous){
//
//        // Asynchronous I/O is enabled so return a
//        // VisBufferAsync
//
//        Assert (via != NULL); // mixing VB with other than a ROVIA is not good
//        result = new VisBufferAsync (* via);
//    }
//    else {
//
//        // By default return a normal VisBuffer
//
//        Assert (via == NULL); // mixing VB with a ROVIA is not good
//
//        result = new VisBuffer (* iter);
//    }
//
//    String reason;
//    if (ROVisibilityIteratorAsync::isAsynchronousIoEnabled ()){
//        if (via == NULL){
//            reason = format (" (synchronous iterator received: %s)", typeid (* iter).name());
//        }
//        else{
//            reason = format ("Async; addr=0x%016x", result);
//        }
//
//    }
//    else{
//        reason = " (async I/O disabled)";
//    }
//
//    Log (2, "Created VisBuffer%s\n", reason.c_str());
//    //printBacktrace (cerr, "VisBufferAsync creation");
//
//    return result;
//}


Int
VisBufferAsync::dataDescriptionId() const
{
    return dataDescriptionId_p;
}

void
VisBufferAsync::detachFromVisIter ()
{

	if (isFilling_p){
	    VisBuffer::detachFromVisIter();
	}
}


Vector<Float>
VisBufferAsync::feed_pa(Double time) const
{
    Vector<Float> feedpa;

    if (visIter_p != NULL){

        // This method can be called during filling; if so then let it
        // work the original way (possible hole for detached VBAs).

        feedpa = VisBuffer::feed_pa (time);
    }
    else{

        //MSDerivedValues msd;
        //msd.setMeasurementSet (* measurementSet_p);

        feedpa.assign (ROVisibilityIteratorAsync::feed_paCalculate (time, * msd_p, nAntennas_p,
                                                                    mEpoch_p, receptor0Angle_p));
    }

    return feedpa;
}

void
VisBufferAsync::fillFrom (const VisBufferAsync & other)
{
    // Almost like assignment except that the attachment to the
    // Visibility iterator is preserved.  Only used for copying
    // data from the buffer ring into the user's VB

    Log (2, "Fill from VisBufferAsync @ 0x%08x to VisBufferAsync @ 0x%08x\n", & other, this);

    copyCache (other);
    copyAsyncValues (other);

}

Vector<MDirection>&
VisBufferAsync::fillDirection1()
{
    // Perform filling in the normal way

    VisBuffer::fillDirection1();

    // Now install unshared copies of the direction objects

    unsharedCopyDirectionVector (direction1_p);

    return direction1_p;
}



Vector<MDirection>&
VisBufferAsync::fillDirection2()
{
    // Perform filling in the normal way

    VisBuffer::fillDirection2();

    // Now install unshared copies of the direction objects

    unsharedCopyDirectionVector (direction2_p);

    return direction2 ();
}

MDirection &
VisBufferAsync::fillPhaseCenter()
{
    // Perform filling in the normal way

    VisBuffer::fillPhaseCenter();

    // Now convert the value to an unshared one.

    phaseCenter_p = unsharedCopyDirection (phaseCenter_p);

    return phaseCenter_p;
}


Double
VisBufferAsync::hourang(Double time) const
{
    //MSDerivedValues msd;
    //msd.setMeasurementSet (* measurementSet_p);

    Double hourang = ROVisibilityIteratorAsync::hourangCalculate (time, * msd_p, mEpoch_p);

    return hourang;
}

void
VisBufferAsync::invalidate ()
{
    // This is called by synchronous code.  Just ignore it since
    // the cache validity is being managed by async code.  To really
    // invalidate use invalidateAsync.
}

void
VisBufferAsync::invalidateAsync ()
{
    // A way for the new code to invalidate the buffer that
    // can be distinguished from the old way.

    VisBuffer::invalidate ();
}

Vector<Double>&
VisBufferAsync::lsrFrequency ()
{
    return VisBuffer::lsrFrequency();
}

const Vector<Double>&
VisBufferAsync::lsrFrequency () const
{
    return VisBuffer::lsrFrequency();
}


void
VisBufferAsync::lsrFrequency(const Int& spw, Vector<Double>& freq, Bool& convert) const
{

    if (velSelection_p) {
        freq.assign (lsrFrequency_p);
        return;
    }

    // Calculate the values using stored information and the static calculation
    // function provided by ROVI.

    ROVisibilityIterator::lsrFrequency(spw,
                                       freq,
                                       convert,
                                       channelStart_p,
                                       channelWidth_p,
                                       channelIncrement_p,
                                       channelGroupNumber_p,
                                       * chanFreqs_p,
                                       * obsMFreqTypes_p,
                                       mEpoch_p,
                                       observatoryPosition_p,
                                       phaseCenter_p);
}

const ROMSColumns &
VisBufferAsync::msColumns() const
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
VisBufferAsync::msId () const
{
    return msID_p;
}

Bool
VisBufferAsync::newMS() const
{
    return newMS_p;
}


Int
VisBufferAsync::numberAnt () const
{
  return msColumns().antenna().nrow(); // for single (sub)array only..
}

Int
VisBufferAsync::numberCoh () const
{
    return nCoh_p;
}


Vector<Float>
VisBufferAsync::parang(Double time) const
{
    //MSDerivedValues msd;
    //msd.setMeasurementSet (* measurementSet_p);

    Vector<Float> parang = ROVisibilityIteratorAsync::parangCalculate (time, * msd_p, nAntennas_p, mEpoch_p);

    return parang;
}

Float
VisBufferAsync::parang0(Double time) const
{
    //MSDerivedValues msd;
    //msd.setMeasurementSet (* measurementSet_p);

    Float parang0 = ROVisibilityIteratorAsync::parang0Calculate (time, * msd_p, mEpoch_p);

    return parang0;
}


//Vector<uInt>&
//VisBufferAsync::rowIds()
//{
//    return rowIds_p;
//}

void
VisBufferAsync::setDataDescriptionId (Int id)
{
    dataDescriptionId_p = id;
}

void
VisBufferAsync::setCorrectedVisCube(Complex c)
{
    correctedVisCube_p.resize(visibilityShape_p);
    correctedVisCube_p.set(c);
    correctedVisCubeOK_p=True;
}

void
VisBufferAsync::setCorrectedVisCube (const Cube<Complex> & vis)
{
    VisBuffer::setCorrectedVisCube (vis);
}


void
VisBufferAsync::setModelVisCube (const Cube<Complex> & vis)
{
    VisBuffer::setModelVisCube (vis);
}

void
VisBufferAsync::setModelVisCube (const Vector<Float> & stokes)
{
    VisBuffer::setModelVisCube (stokes);
}


void
VisBufferAsync::setFilling (Bool isFilling)
{
    isFilling_p = isFilling;
}

void
VisBufferAsync::setLsrInfo (const Block<Int> & channelStart,
                            const Block<Int> & channelWidth,
                            const Block<Int> & channelIncrement,
                            const Block<Int> & channelGroupNumber,
                            const ROArrayColumn <Double> * chanFreqs,
                            const ROScalarColumn<Int> * obsMFreqTypes,
                            const MPosition & observatoryPosition,
                            const MDirection & phaseCenter,
                            Bool velocitySelection)
{
    channelStart_p = channelStart;
    channelWidth_p = channelWidth;
    channelIncrement_p = channelIncrement;
    chanFreqs_p = chanFreqs;
    channelGroupNumber_p = channelGroupNumber;
    obsMFreqTypes_p = obsMFreqTypes;
    observatoryPosition_p = unsharedCopyPosition (observatoryPosition);
    phaseCenter_p = unsharedCopyDirection (phaseCenter);
    velSelection_p = velocitySelection;
}


void
VisBufferAsync::setMeasurementSet (const MeasurementSet & ms)
{
    measurementSet_p = & ms;
    //msd_p->setMeasurementSet (ms);
}

void
VisBufferAsync::setMeasurementSetId (Int id, Bool isNew)
{
    oldMSId_p = id;
    msOK_p = True;
    newMS_p = isNew;
}


void
VisBufferAsync::setMEpoch (const MEpoch & mEpoch)
{
    mEpoch_p = unsharedCopyEpoch (mEpoch);
}

void
VisBufferAsync::setModelVisCube(Complex c)
{
    modelVisCube_p.resize(visibilityShape_p);
    modelVisCube_p.set(c);
    modelVisCubeOK_p=True;
}

void
VisBufferAsync::setMSD (const MSDerivedValues & msd)
{

    msd_p->setEpoch (mEpoch_p);

    if (newMS_p){
        // set antennas

        const Vector<MPosition> & antennaPositions = msd.getAntennaPositions();
        Vector<MPosition> unsharedAntennaPositions (antennaPositions.nelements());

        for (Vector<MPosition>::const_iterator ap = antennaPositions.begin();
             ap != antennaPositions.end();
             ap ++){

            unsharedAntennaPositions = unsharedCopyPosition (* ap);

        }

        msd_p->setAntennaPositions (unsharedAntennaPositions);
    }

    msd_p->setObservatoryPosition (observatoryPosition_p);

    msd_p->setFieldCenter (phaseCenter_p);

    msd_p->setVelocityFrame (msd.getRadialVelocityType ());

}


void
VisBufferAsync::setNAntennas (Int nAntennas)
{
    nAntennas_p = nAntennas;
}

void
VisBufferAsync::setNCoh (Int nCoh)
{
    nCoh_p = nCoh;
}


void
VisBufferAsync::setReceptor0Angle (const Vector<Float> & angles)
{
    receptor0Angle_p = angles;
}

void
VisBufferAsync::setRowIds (const Vector<uInt> & rowIds)
{
    rowIds_p = rowIds;
}

void
VisBufferAsync::setSelectedNVisibilityChannels (const Vector<Int> & nVisibilityChannels)
{
    selectedNVisibilityChannels_p.assign (nVisibilityChannels);
}

void
VisBufferAsync::setSelectedSpectralWindows (const Vector<Int> & spectralWindows)
{
    selectedSpectralWindows_p.assign (spectralWindows);
}


void
VisBufferAsync::setTopoFreqs (const Vector<Double> & lsrFreq, const Vector<Double> & selFreq)
{
    lsrFrequency_p.assign (lsrFreq);
    selFreq_p.assign (selFreq);
}


void
VisBufferAsync::setVisCube(Complex c)
{
    visCube_p.resize(visibilityShape_p);
    visCube_p.set(c);
    visCubeOK_p=True;
}

void
VisBufferAsync::setVisCube (const Cube<Complex>& vis)
{
    VisBuffer::setVisCube (vis);
}


void
VisBufferAsync::setVisibilityShape (const IPosition & visibilityShape)
{
    visibilityShape_p = visibilityShape;
}

MDirection
VisBufferAsync::unsharedCopyDirection (const MDirection & direction)
{
    return asyncio::unsharedCopyMeasure (direction, & MeasureHolder::asMDirection);
}

void
VisBufferAsync::unsharedCopyDirectionVector (Vector<MDirection> & direction)
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
VisBufferAsync::unsharedCopyEpoch (const MEpoch & epoch)
{
    return asyncio::unsharedCopyMeasure (epoch, & MeasureHolder::asMEpoch);
}

MPosition
VisBufferAsync::unsharedCopyPosition (const MPosition & position)
{
    return asyncio::unsharedCopyMeasure (position, & MeasureHolder::asMPosition);
}

void
VisBufferAsync::updateCoordInfo(const VisBuffer * other, const Bool dirDepend)
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


VisBufferAutoPtr::VisBufferAutoPtr ()
{
    visBuffer_p = NULL;
}

VisBufferAutoPtr::VisBufferAutoPtr (VisBufferAutoPtr & other)
{
    // Take ownership of the other's object

    visBuffer_p = other.visBuffer_p;
    other.visBuffer_p = NULL;
}

VisBufferAutoPtr::VisBufferAutoPtr (VisBuffer & vb)
{
    constructVb (& vb);
}

VisBufferAutoPtr::VisBufferAutoPtr (VisBuffer * vb)
{
    constructVb (vb);
}

VisBufferAutoPtr::VisBufferAutoPtr (ROVisibilityIterator & rovi)
{
    construct (& rovi, True);
}


VisBufferAutoPtr::VisBufferAutoPtr (ROVisibilityIterator * rovi)
{
    construct (rovi, True);
}

VisBufferAutoPtr::~VisBufferAutoPtr ()
{
    delete visBuffer_p;
}

VisBufferAutoPtr &
VisBufferAutoPtr::operator= (VisBufferAutoPtr & other)
{
    if (this != & other){

        delete visBuffer_p;  // release any currently referenced object

        // Take ownership of the other's object

        visBuffer_p = other.visBuffer_p;
        other.visBuffer_p = NULL;
    }

    return * this;
}

VisBuffer &
VisBufferAutoPtr::operator* () const
{
    assert (visBuffer_p != NULL);

    return * visBuffer_p;
}

VisBuffer *
VisBufferAutoPtr::operator-> () const
{
    assert (visBuffer_p != NULL);

    return visBuffer_p;
}

void
VisBufferAutoPtr::construct (ROVisibilityIterator * rovi, Bool attachVi)
{
    ROVisibilityIteratorAsync * rovia = dynamic_cast<ROVisibilityIteratorAsync *> (rovi);

    if (rovia != NULL){

        // Create an asynchronous VisBuffer

        VisBufferAsyncWrapper * vba;

        if (attachVi){
            vba = new VisBufferAsyncWrapper (* rovia);
        }
        else{
            vba = new VisBufferAsyncWrapper ();
        }

        visBuffer_p = vba;
    }
    else{

        // This is a synchronous VI so just create a synchronous VisBuffer.

        if (attachVi){
            visBuffer_p = new VisBuffer (* rovi);
        }
        else{
            visBuffer_p = new VisBuffer ();
        }
    }
}

void
VisBufferAutoPtr::constructVb (VisBuffer * vb)
{
    VisBufferAsync * vba = dynamic_cast<VisBufferAsync *> (vb);

    if (vba != NULL){

        // Create an asynchronous VisBuffer

        VisBufferAsyncWrapper * vbaNew = new VisBufferAsyncWrapper (* vba);

        visBuffer_p = vbaNew;
    }
    else{

        // This is a synchronous VI so just create a synchronous VisBuffer.

        visBuffer_p = new VisBuffer (* vb);
    }
}

VisBuffer *
VisBufferAutoPtr::get () const
{
    return visBuffer_p;
}


VisBuffer *
VisBufferAutoPtr::release ()
{
    VisBuffer * result = visBuffer_p;
    visBuffer_p = NULL;

    return result;
}


void
VisBufferAutoPtr::set (VisBuffer & vb)
{
    delete visBuffer_p;
    visBuffer_p = & vb;
}

void
VisBufferAutoPtr::set (VisBuffer * vb)
{
    delete visBuffer_p;
    visBuffer_p = vb;
}

void
VisBufferAutoPtr::set (ROVisibilityIterator * rovi)
{
    delete visBuffer_p;
    construct (rovi, False);
}

void
VisBufferAutoPtr::set (ROVisibilityIterator & rovi)
{
    delete visBuffer_p;
    construct (& rovi, False);
}


} // end namespace casa


