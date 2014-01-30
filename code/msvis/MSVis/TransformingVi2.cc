#include <msvis/MSVis/TransformingVi2.h>

#include <measures/Measures/MDirection.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MPosition.h>
#include <msvis/MSVis/UtilJ.h>
#include <msvis/MSVis/VisibilityIterator2.h>
#include <msvis/MSVis/VisBuffer2.h>

//#include <casa/Arrays/Cube.h>
//#include <casa/Arrays/Matrix.h>
//#include <casa/Arrays/Slicer.h>
//#include <casa/BasicSL/String.h>
//#include <casa/Containers/Stack.h>
//#include <casa/Quanta/MVDoppler.h>
//#include <casa/aips.h>
//#include <measures/Measures/MCDoppler.h>
//#include <measures/Measures/MDoppler.h>
//#include <measures/Measures/MeasConvert.h>
//#include <measures/Measures/Stokes.h>
//#include <ms/MeasurementSets/MSDerivedValues.h>
//#include <ms/MeasurementSets/MSIter.h>
//#include <ms/MeasurementSets/MSColumns.h>
//#include <ms/MeasurementSets/MeasurementSet.h>
//#include <scimath/Mathematics/RigidVector.h>
//#include <scimath/Mathematics/SquareMatrix.h>
//#include <msvis/MSVis/StokesVector.h>
//#include <msvis/MSVis/VisBufferComponents2.h>
//#include <msvis/MSVis/VisImagingWeight.h>
//#include <msvis/MSVis/VisibilityIterator2.h>
//#include <tables/Tables/ArrayColumn.h>
//#include <tables/Tables/ScalarColumn.h>

#define Tvi2NotImplemented() {Throw ("Not implemented.");}

using namespace casa::utilj;

namespace casa {

namespace vi {

TransformingVi2::TransformingVi2 (VisibilityIterator2 * vi, ViImplementation2 * inputVi)
: inputVii_p (inputVi),
  vb_p (0),
  vi_p (vi)
{}

TransformingVi2::~TransformingVi2 ()
{
    delete inputVii_p;
    delete vb_p;
}

void
TransformingVi2::next ()
{
     getVii()->next ();
}

Bool
TransformingVi2::allBeamOffsetsZero ()  const
{
    return getVii()->allBeamOffsetsZero ();
}

void
TransformingVi2::antenna1 (Vector<Int> & ant1)  const
{
     getVii()->antenna1 (ant1);
}

void
TransformingVi2::antenna2 (Vector<Int> & ant2)  const
{
     getVii()->antenna2 (ant2);
}

const Vector<String> &
TransformingVi2::antennaMounts ()  const
{
    return getVii()->antennaMounts ();
}

void
TransformingVi2::arrayIds (Vector<Int>& ids)  const
{
    return getVii()->arrayIds (ids);
}

const Vector<MDirection> &
TransformingVi2::azel (Double time)  const
{
    return getVii()->azel (time);
}

MDirection
TransformingVi2::azel0 (Double time)  const
{
    return getVii()->azel0 (time);
}

void
TransformingVi2::corrType (Vector<Int> & corrTypes)  const
{
     getVii()->corrType (corrTypes);
}

Int
TransformingVi2::dataDescriptionId ()  const
{
    return getVii()->dataDescriptionId ();
}

void
TransformingVi2::dataDescriptionIds(casa::Vector<int>& ddIds) const
{
    return getVii()->dataDescriptionIds (ddIds);
}

Bool
TransformingVi2::existsColumn (VisBufferComponent2 id)  const
{
    return getVii()->existsColumn (id);
}

void
TransformingVi2::exposure (Vector<Double> & expo)  const
{
     getVii()->exposure (expo);
}

void
TransformingVi2::feed1 (Vector<Int> & fd1)  const
{
     getVii()->feed1 (fd1);
}

void
TransformingVi2::feed2 (Vector<Int> & fd2)  const
{
     getVii()->feed2 (fd2);
}

const Vector<Float> &
TransformingVi2::feed_pa (Double time)  const
{
    return getVii()->feed_pa (time);
}

void
TransformingVi2::fieldIds (Vector<Int>& ids)  const
{
    return getVii()->fieldIds (ids);
}

String
TransformingVi2::fieldName ()  const
{
    return getVii()->fieldName ();
}

void
TransformingVi2::flag (Cube<Bool> & flags)  const
{
     getVii()->flag (flags);
}

void
TransformingVi2::flag (Matrix<Bool> & flags)  const
{
     getVii()->flag (flags);
}

void
TransformingVi2::flagCategory (Array<Bool> & flagCategories)  const
{
     getVii()->flagCategory (flagCategories);
}

Bool
TransformingVi2::flagCategoryExists ()  const
{
    return getVii()->flagCategoryExists ();
}

void
TransformingVi2::flagRow (Vector<Bool> & rowflags)  const
{
     getVii()->flagRow (rowflags);
}

void
TransformingVi2::floatData (Cube<Float> & fcube)  const
{
     getVii()->floatData (fcube);
}

const Cube<RigidVector<Double, 2> > &
TransformingVi2::getBeamOffsets ()  const
{
    return getVii()->getBeamOffsets ();
}

Vector<Int>
TransformingVi2::getChannels (Double time, Int frameOfReference)  const
{
    return getVii()->getChannels (time, frameOfReference);
}

Vector<Int>
TransformingVi2::getCorrelations () const
{
    return getVii()->getCorrelations();
}

MEpoch
TransformingVi2::getEpoch ()  const
{
    return getVii()->getEpoch ();
}

Vector<Double>
TransformingVi2::getFrequencies (Double time, Int frameOfReference)  const
{
    return getVii()->getFrequencies (time, frameOfReference);
}

const VisImagingWeight &
TransformingVi2::getImagingWeightGenerator ()  const
{
    return getVii()->getImagingWeightGenerator ();
}

Double
TransformingVi2::getInterval ()  const
{
    return getVii()->getInterval ();
}

Int
TransformingVi2::getNMs () const
{
    return getVii()->getNMs ();
}


MFrequency::Types
TransformingVi2::getObservatoryFrequencyType ()  const
{
    return getVii()->getObservatoryFrequencyType ();
}

MPosition
TransformingVi2::getObservatoryPosition ()  const
{
    return getVii()->getObservatoryPosition ();
}

Vector<Float>
TransformingVi2::getReceptor0Angle ()
{
    return getVii()->getReceptor0Angle ();
}

Int
TransformingVi2::getReportingFrameOfReference ()  const
{
    return getVii()->getReportingFrameOfReference ();
}

void
TransformingVi2::getRowIds (Vector<uInt> & rowids)  const
{
     getVii()->getRowIds (rowids);
}

const SortColumns &
TransformingVi2::getSortColumns ()  const
{
    return getVii()->getSortColumns ();
}

const SpectralWindowChannels &
TransformingVi2::getSpectralWindowChannels (Int msId, Int spectralWindowId)  const
{
    return getVii()->getSpectralWindowChannels (msId, spectralWindowId);
}

Subchunk
TransformingVi2::getSubchunkId ()  const
{
    return getVii()->getSubchunkId ();
}

ViImplementation2 *
TransformingVi2::getVii () const
{
    Assert (inputVii_p != 0);

    return inputVii_p;
}

VisBuffer2 *
TransformingVi2::getVisBuffer ()
{
    ThrowIf (vb_p == 0, "This VI implementation does not provide a VisBuffer");

    return vb_p;
}


Double
TransformingVi2::hourang (Double time)  const
{
    return getVii()->hourang (time);
}

Bool
TransformingVi2::isNewArrayId ()  const
{
    return getVii()->isNewArrayId ();
}

Bool
TransformingVi2::isNewFieldId ()  const
{
    return getVii()->isNewFieldId ();
}

Bool
TransformingVi2::isNewMs ()  const
{
    return getVii()->isNewMs ();
}

Bool
TransformingVi2::isNewSpectralWindow ()  const
{
    return getVii()->isNewSpectralWindow ();
}

Bool
TransformingVi2::isWritable ()  const
{
    return getVii()->isWritable ();
}

void
TransformingVi2::jonesC (Vector<SquareMatrix<Complex, 2> > & cjones)  const
{
     getVii()->jonesC (cjones);
}

Bool
TransformingVi2::more ()  const
{
    return getVii()->more ();
}

Bool
TransformingVi2::moreChunks ()  const
{
    return getVii()->moreChunks ();
}

const MeasurementSet &
TransformingVi2::ms ()  const
{
    return getVii()->ms ();
}

Int
TransformingVi2::msId ()  const
{
    return getVii()->msId ();
}

Int
TransformingVi2::nAntennas ()  const
{
    return getVii()->nAntennas ();
}

Int
TransformingVi2::nDataDescriptionIds ()  const
{
    return getVii()->nDataDescriptionIds ();
}

Int
TransformingVi2::nPolarizationIds ()  const
{
    return getVii()->nPolarizationIds ();
}

Int
TransformingVi2::nRows ()  const
{
    return getVii()->nRows ();
}

Int
TransformingVi2::nRowsInChunk ()  const
{
    return getVii()->nRowsInChunk ();
}

Int
TransformingVi2::nRowsViWillSweep ()  const
{
    return getVii()->nRowsViWillSweep ();
}

Int
TransformingVi2::nSpectralWindows ()  const
{
    return getVii()->nSpectralWindows ();
}

void
TransformingVi2::nextChunk ()
{
     getVii()->nextChunk ();
}

void
TransformingVi2::observationId (Vector<Int> & obsids)  const
{
     getVii()->observationId (obsids);
}

void
TransformingVi2::origin ()
{
     getVii()->origin ();
}

void
TransformingVi2::originChunks (Bool forceRewind)
{
     getVii()->originChunks (forceRewind);
}

const Vector<Float> &
TransformingVi2::parang (Double time)  const
{
    return getVii()->parang (time);
}

const Float &
TransformingVi2::parang0 (Double time)  const
{
    return getVii()->parang0 (time);
}

const MDirection &
TransformingVi2::phaseCenter ()  const
{
    return getVii()->phaseCenter ();
}

Int
TransformingVi2::polFrame ()  const
{
    return getVii()->polFrame ();
}

Int
TransformingVi2::polarizationId ()  const
{
    return getVii()->polarizationId ();
}

void
TransformingVi2::processorId (Vector<Int> & procids)  const
{
     getVii()->processorId (procids);
}

const Cube<Double> &
TransformingVi2::receptorAngles ()  const
{
    return getVii()->receptorAngles ();
}

void
TransformingVi2::scan (Vector<Int> & scans)  const
{
     getVii()->scan (scans);
}

void
TransformingVi2::setFrequencySelections (const FrequencySelections & selection)
{
     getVii()->setFrequencySelections (selection);
}

void
TransformingVi2::setInterval (Double timeInterval)
{
     getVii()->setInterval (timeInterval);
}

void
TransformingVi2::setReportingFrameOfReference (Int frame)
{
     getVii()->setReportingFrameOfReference (frame);
}

void
TransformingVi2::setRowBlocking (Int nRows)
{
     getVii()->setRowBlocking (nRows);
}

void
TransformingVi2::setVisBuffer (VisBuffer2 * vb)
{
    ThrowIf (vb_p != 0, "A VisBuffer is already associated with this VI");

    vb_p = vb;
}


void
TransformingVi2::sigma (Matrix<Float> & sig)  const
{
     getVii()->sigma (sig);
}

void
TransformingVi2::slurp ()  const
{
     getVii()->slurp ();
}

String
TransformingVi2::sourceName ()  const
{
    return getVii()->sourceName ();
}

Int
TransformingVi2::spectralWindow ()  const
{
    return getVii()->spectralWindow ();
}

void
TransformingVi2::spectralWindows (Vector<Int> & spws) const
{
    getVii()->spectralWindows (spws);
}


void
TransformingVi2::stateId (Vector<Int> & stateids)  const
{
     getVii()->stateId (stateids);
}

const vi::SubtableColumns &
TransformingVi2::subtableColumns ()  const
{
    return getVii()->subtableColumns ();
}

void
TransformingVi2::time (Vector<Double> & t)  const
{
     getVii()->time (t);
}

void
TransformingVi2::timeCentroid (Vector<Double> & t)  const
{
     getVii()->timeCentroid (t);
}

void
TransformingVi2::timeInterval (Vector<Double> & ti)  const
{
     getVii()->timeInterval (ti);
}

void
TransformingVi2::useImagingWeight (const VisImagingWeight & imWgt)
{
     getVii()->useImagingWeight (imWgt);
}

void
TransformingVi2::uvw (Matrix<Double> & uvwmat)  const
{
     getVii()->uvw (uvwmat);
}

void
TransformingVi2::visibilityCorrected (Cube<Complex> & vis)  const
{
     getVii()->visibilityCorrected (vis);
}

//void
//TransformingVi2::visibilityCorrected (Matrix<CStokesVector> & vis)  const
//{
//     getVii()->visibilityCorrected (vis);
//}

void
TransformingVi2::visibilityModel (Cube<Complex> & vis)  const
{
     getVii()->visibilityModel (vis);
}

//void
//TransformingVi2::visibilityModel (Matrix<CStokesVector> & vis)  const
//{
//     getVii()->visibilityModel (vis);
//}

void
TransformingVi2::visibilityObserved (Cube<Complex> & vis)  const
{
     getVii()->visibilityObserved (vis);
}

//void
//TransformingVi2::visibilityObserved (Matrix<CStokesVector> & vis)  const
//{
//     getVii()->visibilityObserved (vis);
//}

IPosition
TransformingVi2::visibilityShape ()  const
{
    return getVii()->visibilityShape ();
}

void
TransformingVi2::weight (Matrix<Float> & wt)  const
{
     getVii()->weight (wt);
}

void
TransformingVi2::weightSpectrum (Cube<Float> & wtsp)  const
{
     getVii()->weightSpectrum (wtsp);
}

void
TransformingVi2::weightSpectrumCorrected (Cube<Float> & wtsp)  const
{
     getVii()->weightSpectrumCorrected (wtsp);
}

Bool
TransformingVi2::weightSpectrumExists ()  const
{
    return getVii()->weightSpectrumExists ();
}

Bool
TransformingVi2::weightSpectrumCorrectedExists ()  const
{
    return getVii()->weightSpectrumExists ();
}

void
TransformingVi2::writeBackChanges (VisBuffer2 * /*vb*/)
{
    Tvi2NotImplemented ();
}

void
TransformingVi2::writeFlag (const Matrix<Bool> & /*flag*/)
{
    Tvi2NotImplemented ();
}

void
TransformingVi2::writeFlag (const Cube<Bool> & /*flag*/)
{
    Tvi2NotImplemented ();
}

void
TransformingVi2::writeFlagCategory (const Array<Bool>& /*fc*/)
{
    Tvi2NotImplemented ();
}

void
TransformingVi2::writeFlagRow (const Vector<Bool> & /*rowflags*/)
{
    Tvi2NotImplemented ();
}

void
TransformingVi2::writeModel (const RecordInterface& /*rec*/, Bool /*iscomponentlist*/,
                            Bool /*incremental*/)
{
    Tvi2NotImplemented ();
}

void
TransformingVi2::writeSigma (const Matrix<Float> & /*sig*/)
{
    Tvi2NotImplemented ();
}

//void
//TransformingVi2::writeSigmaMat (const Matrix<Float> & /*sigmat*/)
//{
//    Tvi2NotImplemented ();
//}

//void
//TransformingVi2::writeVisCorrected (const Matrix<CStokesVector> & /*visibilityStokes*/)
//{
//    Tvi2NotImplemented ();
//}

void
TransformingVi2::writeVisCorrected (const Cube<Complex> & /*vis*/)
{
    Tvi2NotImplemented ();
}

//void
//TransformingVi2::writeVisModel (const Matrix<CStokesVector> & /*visibilityStokes*/)
//{
//    Tvi2NotImplemented ();
//}

void
TransformingVi2::writeVisModel (const Cube<Complex> & /*vis*/)
{
    Tvi2NotImplemented ();
}

//void
//TransformingVi2::writeVisObserved (const Matrix<CStokesVector> & /*visibilityStokes*/)
//{
//    Tvi2NotImplemented ();
//}

void
TransformingVi2::writeVisObserved (const Cube<Complex> & /*vis*/)
{
    Tvi2NotImplemented ();
}

void
TransformingVi2::writeWeight (const Matrix<Float> & /*wt*/)
{
    Tvi2NotImplemented ();
}

//void
//TransformingVi2::writeWeightMat (const Matrix<Float> & /*wtmat*/)
//{
//    Tvi2NotImplemented ();
//}

void
TransformingVi2::writeWeightSpectrum (const Cube<Float> & /*wtsp*/)
{
    Tvi2NotImplemented ();
}

void
TransformingVi2::writeWeightSpectrumCorrected (const Cube<Float> & /*wtsp*/)
{
    Tvi2NotImplemented ();
}

void
TransformingVi2::setWeightScaling (CountedPtr <WeightScaling> weightScaling)
{
    getVii()->setWeightScaling(weightScaling);
}

Bool
TransformingVi2::hasWeightScaling () const
{
    return getVii()->hasWeightScaling();
}

CountedPtr<WeightScaling>
TransformingVi2::getWeightScaling () const
{
    return getVii()->getWeightScaling();
}


} // end namespace vi

} // end namespace casa
