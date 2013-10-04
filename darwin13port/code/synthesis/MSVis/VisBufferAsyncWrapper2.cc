#include <synthesis/MSVis/VisBufferAsyncWrapper2.h>

#include "AsynchronousTools.h"
using namespace casa::async;

#include "UtilJ.h"
using namespace casa::utilj;

#define CheckWrap() \
        if (wrappedVb_p == NULL){\
            Throw ("VisBufferAsyncWrapper: No attached VBA");\
        }

#define Log(level, ...) \
        {if (casa::asyncio::AsynchronousInterface::logThis (level)) \
    Logger::get()->log (__VA_ARGS__);};

namespace casa { //# NAMESPACE CASA - BEGIN

namespace vi {

VisBufferWrapperAsync2::VisBufferWrapperAsync2 (VisibilityIterator2 & iter)
: wrappedVb_p (NULL)
{
    Assert (iter.isAsynchronous ());
}

VisBufferWrapperAsync2::~VisBufferWrapperAsync2 ()
{
    delete wrappedVb_p;
}

void
VisBufferWrapperAsync2::copy (const VisBuffer2 & other, Bool fetchIfNeeded)
{
    wrappedVb_p->copy (other, fetchIfNeeded);
}

void
VisBufferWrapperAsync2::copyComponents (const VisBuffer2 & other,
                                        const VisBufferComponents2 & components,
                                        Bool fetchIfNeeded)
{
    wrappedVb_p->copyComponents (other, components, fetchIfNeeded);
}

void
VisBufferWrapperAsync2::copyCoordinateInfo(const VisBuffer2 * other, Bool includeDirections,
                                           Bool fetchIfNeeded)
{
    wrappedVb_p->copyCoordinateInfo(other, includeDirections, fetchIfNeeded);
}

void
VisBufferWrapperAsync2::setShape (Int nCorrelations, Int nChannels, Int nRows)
{
    wrappedVb_p->setShape (nCorrelations, nChannels, nRows);
}

const VisibilityIterator2 *
VisBufferWrapperAsync2::getVi () const
{
    return wrappedVb_p->getVi ();
}

Bool
VisBufferWrapperAsync2::isAttached () const
{
    return wrappedVb_p->isAttached ();
}

Bool
VisBufferWrapperAsync2::isFillable () const
{
    return wrappedVb_p->isFillable();
}

void
VisBufferWrapperAsync2::writeChangesBack ()
{
    wrappedVb_p->writeChangesBack ();
}

void
VisBufferWrapperAsync2::dirtyComponentsAdd (const VisBufferComponents2 & additionalDirtyComponents)
{
    wrappedVb_p->dirtyComponentsAdd (additionalDirtyComponents);
}
void
VisBufferWrapperAsync2::dirtyComponentsClear ()
{
    wrappedVb_p->dirtyComponentsClear ();
}

VisBufferComponents2
VisBufferWrapperAsync2::dirtyComponentsGet () const
{
    return wrappedVb_p->dirtyComponentsGet ();
}

void
VisBufferWrapperAsync2::dirtyComponentsSet (const VisBufferComponents2 & dirtyComponents)
{
    wrappedVb_p->dirtyComponentsSet (dirtyComponents);
}

const Matrix<Float> &
VisBufferWrapperAsync2::imagingWeight() const
{
    return wrappedVb_p-> imagingWeight();
}

Double
VisBufferWrapperAsync2::getFrequency (Int rowInBuffer, Int frequencyIndex,
                                      Int frame) const
{
    return wrappedVb_p->getFrequency (rowInBuffer, frequencyIndex, frame);
}

const Vector<Double> &
VisBufferWrapperAsync2::getFrequencies (Int rowInBuffer, Int frame) const
{
    return wrappedVb_p->getFrequencies (rowInBuffer, frame);
}
Int
VisBufferWrapperAsync2::getChannelNumber (Int rowInBuffer, Int frequencyIndex) const
{
    return wrappedVb_p->getChannelNumber (rowInBuffer, frequencyIndex);
}

const Vector<Int> &
VisBufferWrapperAsync2::getChannelNumbers (Int rowInBuffer) const
{
    return wrappedVb_p->getChannelNumbers (rowInBuffer);
}

void
VisBufferWrapperAsync2::normalize()
{
    wrappedVb_p->normalize();
}

void
VisBufferWrapperAsync2::resetWeightsUsingSigma ()
{
    wrappedVb_p->resetWeightsUsingSigma ();
}

Bool
VisBufferWrapperAsync2::isNewArrayId () const
{
    return wrappedVb_p->isNewArrayId ();
}
Bool
VisBufferWrapperAsync2::isNewFieldId () const
{
    return wrappedVb_p-> isNewFieldId ();
}

Bool
VisBufferWrapperAsync2::isNewMs() const
{
    return wrappedVb_p->isNewMs();
}

Bool
VisBufferWrapperAsync2::isNewSpectralWindow () const
{
    return wrappedVb_p->isNewSpectralWindow ();
}

Bool
VisBufferWrapperAsync2::isWritable () const
{
    return wrappedVb_p->isWritable ();
}

Int
VisBufferWrapperAsync2::msId() const
{
    return wrappedVb_p->msId();
}

String
VisBufferWrapperAsync2::msName (Bool stripPath) const
{
    return wrappedVb_p->msName (stripPath);
}

Subchunk
VisBufferWrapperAsync2::getSubchunk () const
{
    return wrappedVb_p-> getSubchunk ();
}

const Vector<Int> &
VisBufferWrapperAsync2::antenna1 () const
{
    return wrappedVb_p->antenna1 ();
}

void
VisBufferWrapperAsync2::setAntenna1 (const Vector<Int> & value)
{
    wrappedVb_p-> setAntenna1 (value);
}

const Vector<Int> &
VisBufferWrapperAsync2::antenna2 () const
{
    return wrappedVb_p->antenna2 ();
}

void
VisBufferWrapperAsync2::setAntenna2 (const Vector<Int> & value)
{
    wrappedVb_p-> setAntenna2 (value);
}

Int
VisBufferWrapperAsync2::arrayId () const
{
    return wrappedVb_p->arrayId ();
}

void
VisBufferWrapperAsync2::setArrayId (Int value)
{
    wrappedVb_p-> setArrayId (value);
}

Int
VisBufferWrapperAsync2::dataDescriptionId () const
{
    return wrappedVb_p->dataDescriptionId ();
}

void
VisBufferWrapperAsync2::setDataDescriptionId (Int value)
{
    wrappedVb_p-> setDataDescriptionId (value);
}

const Vector<MDirection> &
VisBufferWrapperAsync2::direction1 () const
{
    return wrappedVb_p->direction1 ();
}

const Vector<MDirection> &
VisBufferWrapperAsync2::direction2 () const
{
    return wrappedVb_p-> direction2 ();
}

const Vector<Double> &
VisBufferWrapperAsync2::exposure () const
{
    return wrappedVb_p->exposure ();
}

void
VisBufferWrapperAsync2::setExposure (const Vector<Double> & value)
{
    wrappedVb_p-> setExposure (value);
}

const Vector<Int> &
VisBufferWrapperAsync2::feed1 () const
{
    return wrappedVb_p->feed1 ();
}

void
VisBufferWrapperAsync2::setFeed1 (const Vector<Int> & value)
{
    wrappedVb_p-> setFeed1 (value);
}

const Vector<Int> &
VisBufferWrapperAsync2::feed2 () const
{
    return wrappedVb_p->feed2 ();
}

void
VisBufferWrapperAsync2::setFeed2 (const Vector<Int> & value)
{
    wrappedVb_p-> setFeed2 (value);
}

Int
VisBufferWrapperAsync2::fieldId () const
{
    return wrappedVb_p->fieldId ();
}

void
VisBufferWrapperAsync2::setFieldId (Int value)
{
    wrappedVb_p-> setFieldId (value);
}

const Matrix<Bool> &
VisBufferWrapperAsync2::flag () const
{
    return wrappedVb_p->flag ();
}

void
VisBufferWrapperAsync2::setFlag (const Matrix<Bool>& value)
{
    wrappedVb_p-> setFlag (value);
}

const Array<Bool> &
VisBufferWrapperAsync2::flagCategory () const
{
    return wrappedVb_p->flagCategory ();
}

void
VisBufferWrapperAsync2::setFlagCategory (const Array<Bool>& value)
{
    wrappedVb_p-> setFlagCategory (value);
}

const Cube<Bool> &
VisBufferWrapperAsync2::flagCube () const
{
    return wrappedVb_p->flagCube ();
}

void
VisBufferWrapperAsync2::setFlagCube (const Cube<Bool>& value)
{
    wrappedVb_p-> setFlagCube (value);
}

const Vector<Bool> &
VisBufferWrapperAsync2::flagRow () const
{
    return wrappedVb_p->flagRow ();
}

void
VisBufferWrapperAsync2::setFlagRow (const Vector<Bool>& value)
{
    wrappedVb_p-> setFlagRow (value);
}

const Vector<Int> &
VisBufferWrapperAsync2::observationId () const
{
    return wrappedVb_p->observationId ();
}

void
VisBufferWrapperAsync2::setObservationId (const Vector<Int> & value)
{
    wrappedVb_p-> setObservationId (value);
}

const Vector<Int> &
VisBufferWrapperAsync2::processorId () const
{
    return wrappedVb_p->processorId ();
}

void
VisBufferWrapperAsync2::setProcessorId (const Vector<Int> & value)
{
    wrappedVb_p-> setProcessorId (value);
}

const Vector<Int> &
VisBufferWrapperAsync2::scan () const
{
    return wrappedVb_p->scan ();
}

void
VisBufferWrapperAsync2::setScan (const Vector<Int> & value)
{
    wrappedVb_p-> setScan (value);
}

const Vector<Float> &
VisBufferWrapperAsync2::sigma () const
{
    return wrappedVb_p->sigma ();
}

void
VisBufferWrapperAsync2::setSigma (const Vector<Float> & value)
{
    wrappedVb_p-> setSigma (value);
}

const Matrix<Float> &
VisBufferWrapperAsync2::sigmaMat () const
{
    return wrappedVb_p->sigmaMat ();
}

const Vector<Int> &
VisBufferWrapperAsync2::stateId () const
{
    return wrappedVb_p-> stateId ();
}

void
VisBufferWrapperAsync2::setStateId (const Vector<Int> & value)
{
    wrappedVb_p->setStateId (value);
}

const Vector<Double> &
VisBufferWrapperAsync2::time () const
{
    return wrappedVb_p-> time ();
}

void
VisBufferWrapperAsync2::setTime (const Vector<Double> & value)
{
    wrappedVb_p->setTime (value);
}

const Vector<Double> &
VisBufferWrapperAsync2::timeCentroid () const
{
    return wrappedVb_p-> timeCentroid ();
}

void
VisBufferWrapperAsync2::setTimeCentroid (const Vector<Double> & value)
{
    wrappedVb_p->setTimeCentroid (value);
}

const Vector<Double> &
VisBufferWrapperAsync2::timeInterval () const
{
    return wrappedVb_p-> timeInterval ();
}

void
VisBufferWrapperAsync2::setTimeInterval (const Vector<Double> & value)
{
    wrappedVb_p->setTimeInterval (value);
}

const Matrix<Double> &
VisBufferWrapperAsync2::uvw () const
{
    return wrappedVb_p-> uvw ();
}

void
VisBufferWrapperAsync2::setUvw (const Matrix<Double> & value)
{
    wrappedVb_p->setUvw (value);
}

const Vector<Float> &
VisBufferWrapperAsync2::weight () const
{
    return wrappedVb_p-> weight ();
}

void
VisBufferWrapperAsync2::setWeight (const Vector<Float>& value)
{
    wrappedVb_p->setWeight (value);
}

const Matrix<Float> &
VisBufferWrapperAsync2::weightMat () const
{
    return wrappedVb_p-> weightMat ();
}

void
VisBufferWrapperAsync2::setWeightMat (const Matrix<Float>& value)
{
    wrappedVb_p->setWeightMat (value);
}

const Cube<Float> &
VisBufferWrapperAsync2::weightSpectrum () const
{
    return wrappedVb_p-> weightSpectrum ();
}

void
VisBufferWrapperAsync2::setWeightSpectrum (const Cube<Float>& value)
{
    wrappedVb_p->setWeightSpectrum (value);
}

const Cube<Complex> &
VisBufferWrapperAsync2::visCube () const
{
    return wrappedVb_p->visCube ();
}

void
VisBufferWrapperAsync2::setVisCube(const Complex & c)
{
    wrappedVb_p->setVisCube(c);
}

void
VisBufferWrapperAsync2::setVisCube (const Cube<Complex> & cube)
{
    wrappedVb_p->setVisCube (cube);
}

const Matrix<CStokesVector> &
VisBufferWrapperAsync2::vis () const
{
    return wrappedVb_p-> vis ();
}

void
VisBufferWrapperAsync2::setVis (Matrix<CStokesVector> & vis)
{
    wrappedVb_p->setVis (vis);
}

const Cube<Complex> &
VisBufferWrapperAsync2::visCubeCorrected () const
{
    return wrappedVb_p->visCubeCorrected ();
}

void
VisBufferWrapperAsync2::setVisCubeCorrected (const Cube<Complex> & cube)
{
    wrappedVb_p-> setVisCubeCorrected (cube);
}

const Matrix<CStokesVector> &
VisBufferWrapperAsync2::visCorrected () const
{
    return wrappedVb_p->visCorrected ();
}

void
VisBufferWrapperAsync2::setVisCorrected (const Matrix<CStokesVector> & vis)
{
    wrappedVb_p-> setVisCorrected (vis);
}

const Cube<Float> &
VisBufferWrapperAsync2::visCubeFloat () const
{
    return wrappedVb_p->visCubeFloat ();
}

void
VisBufferWrapperAsync2::setVisCubeFloat (const Cube<Float> & cube)
{
    wrappedVb_p-> setVisCubeFloat (cube);
}

const Cube<Complex> &
VisBufferWrapperAsync2::visCubeModel () const
{
    return wrappedVb_p->visCubeModel ();
}

void
VisBufferWrapperAsync2::setVisCubeModel(const Complex & c)
{
    wrappedVb_p->setVisCubeModel(c);
}

void
VisBufferWrapperAsync2::setVisCubeModel(const Cube<Complex>& vis)
{
    wrappedVb_p->setVisCubeModel(vis);
}

void
VisBufferWrapperAsync2::setVisCubeModel(const Vector<Float>& stokes)
{
    wrappedVb_p->setVisCubeModel(stokes);
}

const Matrix<CStokesVector> &
VisBufferWrapperAsync2::visModel () const
{
    return wrappedVb_p->visModel ();
}

void
VisBufferWrapperAsync2::setVisModel (Matrix<CStokesVector> & vis)
{
    wrappedVb_p-> setVisModel (vis);
}

MDirection
VisBufferWrapperAsync2::azel0 (Double time) const
{
    return wrappedVb_p->azel0 (time);
}

const Vector<MDirection> &
VisBufferWrapperAsync2::azel(Double time) const
{
    return wrappedVb_p->azel(time);
}

const Vector<SquareMatrix<Complex, 2> > &
VisBufferWrapperAsync2::cjones () const
{
    return wrappedVb_p->cjones ();
}

const Vector<Int> &
VisBufferWrapperAsync2::correlationTypes () const
{
    return wrappedVb_p->correlationTypes ();
}

const Vector<Float> &
VisBufferWrapperAsync2::feedPa(Double time) const
{
    return wrappedVb_p->feedPa(time);
}

const Vector<Float> &
VisBufferWrapperAsync2::feedPa1 () const
{
    return wrappedVb_p->feedPa1 ();
}

const Vector<Float> &
VisBufferWrapperAsync2::feedPa2 () const
{
    return wrappedVb_p->feedPa2 ();
}

Double
VisBufferWrapperAsync2::hourang(Double time) const
{
    return wrappedVb_p->hourang(time);
}

Int
VisBufferWrapperAsync2::nAntennas () const
{
    return wrappedVb_p->nAntennas ();
}

Int
VisBufferWrapperAsync2::nChannels () const
{
    return wrappedVb_p->nChannels ();
}

Int
VisBufferWrapperAsync2::nCorrelations () const
{
    return wrappedVb_p->nCorrelations ();
}

Int
VisBufferWrapperAsync2::nRows () const
{
    return wrappedVb_p->nRows ();
}

Float
VisBufferWrapperAsync2::parang0(Double time) const
{
    return wrappedVb_p->parang0(time);
}

const Vector<Float> &
VisBufferWrapperAsync2::parang(Double time) const
{
    return wrappedVb_p->parang(time);
}

const MDirection &
VisBufferWrapperAsync2::phaseCenter () const
{
    return wrappedVb_p->phaseCenter ();
}

Int
VisBufferWrapperAsync2::polarizationFrame () const
{
    return wrappedVb_p->polarizationFrame ();
}

Int
VisBufferWrapperAsync2::polarizationId () const
{
    return wrappedVb_p->polarizationId ();
}

const Vector<uInt> &
VisBufferWrapperAsync2::rowIds () const
{
    return wrappedVb_p->rowIds ();
}

Int
VisBufferWrapperAsync2::spectralWindow () const
{
    return wrappedVb_p->spectralWindow ();
}

void
VisBufferWrapperAsync2::wrap (VisBuffer2 * vb)
{
    assert (vb != wrappedVb_p);

    delete wrappedVb_p;

    wrappedVb_p = vb;
}

}
 // end namespace vi

}
 // end namespace casa

