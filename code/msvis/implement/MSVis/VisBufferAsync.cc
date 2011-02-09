/*
 * VisibilityBufferAsync.cc
 *
 *  Created on: Nov 1, 2010
 *      Author: jjacobs
 */

#include "UtilJ.h"
using namespace casa::utilj;

#include "VisBufferAsync.h"
#include "VisibilityIterator.h"
#include "VisibilityIteratorAsync.h"
#include "VLAT.h"

#include <ms/MeasurementSets/MSColumns.h>
#include <typeinfo>

#define Log(level, ...) \
    {if (VlaData::loggingInitialized_p && level <= VlaData::logLevel_p) \
         Logger::log (__VA_ARGS__);};

namespace casa {

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

VisBufferAsync::VisBufferAsync (ROVisibilityIteratorAsync & iter)
  : VisBuffer ()
{
    construct ();

    iter.attachVisBuffer (*this);
    visIter_p = & iter;
}


VisBufferAsync::~VisBufferAsync ()
{
    Log (2, "Destroying VisBufferAsync; addr=0x%016x\n", this);

    if (visIter_p != NULL){
        detachFromVisIter();
    }

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

    if (other.corrSorted_p)
        throw(AipsError("Cannot assign a VisBuffer that has had correlations sorted!"));

    if (this != & other){

        // Detach from visibility iterator if attached

        if (visIter_p != NULL){
            detachFromVisIter ();
        }

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


Vector<MDirection>
VisBufferAsync::azel(Double time) const
{
    Vector<MDirection> azel;

    MSDerivedValues msd;
    msd.setMeasurementSet (* measurementSet_p);

    ROVisibilityIteratorAsync::azelCalculate (time, msd, azel, nAntennas_p, mEpoch_p);

    return azel;
}

MDirection
VisBufferAsync::azel0(Double time) const
{
    MDirection azel0;
    MSDerivedValues msd;
    msd.setMeasurementSet (* measurementSet_p);


    ROVisibilityIteratorAsync::azel0Calculate (time, msd, azel0, mEpoch_p);

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

    delete msColumns_p;
    msColumns_p = NULL;

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
    lsrFreq_p.resize (0);
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

    clear ();
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
    lsrFreq_p = other.lsrFreq_p;
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


VisBuffer *
VisBufferAsync::create (ROVisibilityIterator & iter)
{
    return create (& iter);
}

VisBuffer *
VisBufferAsync::create (ROVisibilityIterator * iter)
{
    VisBuffer * result = NULL;
    ROVisibilityIteratorAsync * via = dynamic_cast<ROVisibilityIteratorAsync *> (iter);

    bool createAsynchronous = ROVisibilityIteratorAsync::isAsynchronousIoEnabled () &&
                              via != NULL;

    if (createAsynchronous){

        // Asynchronous I/O is enabled so return a
        // VisBufferAsync

        Assert (via != NULL); // mixing VB with other than a ROVIA is not good
        result = new VisBufferAsync (* via);
    }
    else {

        // By default return a normal VisBuffer

        Assert (via == NULL); // mixing VB with a ROVIA is not good

        result = new VisBuffer (* iter);
    }

    String reason;
    if (ROVisibilityIteratorAsync::isAsynchronousIoEnabled ()){
        if (via == NULL){
            reason = format (" (synchronous iterator received: %s)", typeid (* iter).name());
        }
        else{
            reason = format ("Async; addr=0x%016x", result);
        }

    }
    else{
        reason = " (async I/O disabled)";
    }

    Log (2, "Created VisBuffer%s\n", reason.c_str());
    //printBacktrace (cerr, "VisBufferAsync creation");

    return result;
}


Int
VisBufferAsync::dataDescriptionId() const
{
    return dataDescriptionId_p;
}

void
VisBufferAsync::detachFromVisIter ()
{
	if (visIter_p == NULL){
		throw AipsError ("No VisIter to detach from!", __FILE__, __LINE__);
	}

	visIter_p->detachVisBuffer(* this);

	visIter_p = NULL;
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

        MSDerivedValues msd;
        msd.setMeasurementSet (* measurementSet_p);

        feedpa.assign (ROVisibilityIteratorAsync::feed_paCalculate (time, msd, nAntennas_p,
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

Double
VisBufferAsync::hourang(Double time) const
{
    MSDerivedValues msd;
    msd.setMeasurementSet (* measurementSet_p);

    Double hourang = ROVisibilityIteratorAsync::hourangCalculate (time, msd, mEpoch_p);

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

void
VisBufferAsync::lsrFrequency(const Int& spw, Vector<Double>& freq, Bool& convert) const
{

    if (velSelection_p) {
        freq.assign (lsrFreq_p);
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
    MSDerivedValues msd;
    msd.setMeasurementSet (* measurementSet_p);

    Vector<Float> parang = ROVisibilityIteratorAsync::parangCalculate (time, msd, nAntennas_p, mEpoch_p);

    return parang;
}

Float
VisBufferAsync::parang0(Double time) const
{
    MSDerivedValues msd;
    msd.setMeasurementSet (* measurementSet_p);

    Float parang0 = ROVisibilityIteratorAsync::parang0Calculate (time, msd, mEpoch_p);

    return parang0;
}


Vector<uInt>&
VisBufferAsync::rowIds()
{
    return rowIds_p;
}

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
    observatoryPosition_p = observatoryPosition;
    phaseCenter_p = phaseCenter;
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
    mEpoch_p = mEpoch;
}

void
VisBufferAsync::setModelVisCube(Complex c)
{
    modelVisCube_p.resize(visibilityShape_p);
    modelVisCube_p.set(c);
    modelVisCubeOK_p=True;
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
    lsrFreq_p.assign (lsrFreq);
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
VisBufferAsync::setVisibilityShape (const IPosition & visibilityShape)
{
    visibilityShape_p = visibilityShape;
}

void
VisBufferAsync::updateCoordInfo(const VisBuffer * other)
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
    copyVector (other->feed1_pa_p, feed1_pa_p);
    feed1_paOK_p = true;
    copyVector (other->feed2_pa_p, feed2_pa_p);
    feed2_paOK_p = true;
    //copyVector (direction1_p);
    //copyVector (direction2_p);
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

        VisBufferAsync * vba;

        if (attachVi){
            vba = new VisBufferAsync (* rovia);
        }
        else{
            vba = new VisBufferAsync ();
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

        VisBufferAsync * vbaNew = new VisBufferAsync (* vba);

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
