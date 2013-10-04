/*
 * VisibilityIterator2.cc
 *
 *  Created on: Jun 4, 2012
 *      Author: jjacobs
 */

#include <synthesis/MSVis/VisibilityIterator2.h>

#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Slicer.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/Stack.h>
#include <casa/Quanta/MVDoppler.h>
#include <casa/aips.h>
#include <casa/System/AipsrcValue.h>
#include <measures/Measures/MCDoppler.h>
#include <measures/Measures/MDoppler.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/Stokes.h>
#include <ms/MeasurementSets/MSDerivedValues.h>
#include <ms/MeasurementSets/MSIter.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <scimath/Mathematics/RigidVector.h>
#include <scimath/Mathematics/SquareMatrix.h>
#include <synthesis/MSVis/AveragingTvi2.h>
#include <synthesis/MSVis/ViFrequencySelection.h>
#include <synthesis/MSVis/StokesVector.h>
#include <synthesis/MSVis/VisBuffer2.h>
#include <synthesis/MSVis/VisBufferComponents2.h>
#include <synthesis/MSVis/VisImagingWeight.h>
#include <synthesis/MSVis/VisibilityIteratorImpl2.h>
#include <synthesis/MSVis/VisibilityIteratorImplAsync2.h>
#include <synthesis/MSVis/UtilJ.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ScalarColumn.h>

#include <cstdarg>
#include <map>
#include <set>
#include <utility>
#include <vector>

using namespace std;

#define CheckImplementationPointerR() Assert (impl_p != NULL);
#define CheckImplementationPointerW() Assert (impl_p != NULL);

namespace casa {

namespace vi {

SortColumns::SortColumns (const Block<Int> & columnIds, Bool addDefaultColumns)
: addDefaultColumns_p (addDefaultColumns),
  columnIds_p (columnIds)
{}

Bool
SortColumns::shouldAddDefaultColumns () const
{
    return addDefaultColumns_p;
}

const Block<Int> &
SortColumns::getColumnIds () const
{
    return columnIds_p;
}


CountedPtr <WeightScaling>
WeightScaling::generateUnityWeightScaling ()
{
    return generateWeightScaling (WeightScaling::unity);
}

CountedPtr <WeightScaling>
WeightScaling::generateIdentityWeightScaling ()
{
    return  generateWeightScaling (WeightScaling::identity);
}

CountedPtr <WeightScaling>
WeightScaling::generateSquareWeightScaling ()
{
    return  generateWeightScaling (WeightScaling::square);
}

Float WeightScaling::unity (Float) { return 1.0;}
Float WeightScaling::identity (Float x) { return x;}
Float WeightScaling::square (Float x) { return x * x;}

VisibilityIterator2::VisibilityIterator2()
: impl_p (0)
{
}

VisibilityIterator2::VisibilityIterator2(const MeasurementSet& ms,
                                         const SortColumns & sortColumns,
                                         Bool isWritable,
                                         const VisBufferComponents2 * prefetchColumns,
                                         Double timeInterval)
{
    Block<const MeasurementSet *> mss (1, & ms);
    construct (prefetchColumns, mss, sortColumns,
               timeInterval, isWritable);
}

VisibilityIterator2::VisibilityIterator2 (const Block<const MeasurementSet *>& mss,
                                          const SortColumns & sortColumns,
                                          Bool isWritable,
                                          const VisBufferComponents2 * prefetchColumns,
                                          Double timeInterval)
{
    construct (prefetchColumns, mss, sortColumns, timeInterval, isWritable);
}

VisibilityIterator2::VisibilityIterator2 (const ViFactory & factory)
: impl_p (0)
{
    ViImplementation2 * newImpl = factory.createVi (this);

    impl_p = newImpl;
}

//VisibilityIterator2::VisibilityIterator2 (const VisBufferComponents2 * prefetchColumns,
//                                          const Block<MeasurementSet>& mss,
//                                          const Block<Int>& sortColumns,
//                                          const Bool addDefaultSortCols,
//                                          Double timeInterval,
//                                          Bool writable)
//{
//    construct (prefetchColumns, mss, sortColumns, addDefaultSortCols,
//               timeInterval, writable);
//}



void
VisibilityIterator2::construct (const VisBufferComponents2 * prefetchColumns,
                                const Block<const MeasurementSet *>& mss,
                                const SortColumns & sortColumns,
                                Double timeInterval,
                                Bool writable)
{

    // Factory didn't create the read implementation so decide whether to create a
    // synchronous or asynchronous read implementation.

    Bool createAsAsynchronous = prefetchColumns != NULL && isAsynchronousIoEnabled ();

    if (createAsAsynchronous){
        //            impl_p = new ViImplAsync2 (mss, * prefetchColumns, sortColumns,
        //                                               addDefaultSortCols, timeInterval, writable);
    }
    else{
        impl_p = new VisibilityIteratorImpl2 (this, mss, sortColumns, timeInterval, VbPlain, writable);
    }
}

VisibilityIterator2::~VisibilityIterator2 ()
{
    delete impl_p;
}

void
VisibilityIterator2::next()
{
    CheckImplementationPointerR ();
    impl_p->next ();
}

Bool
VisibilityIterator2::allBeamOffsetsZero () const
{
    CheckImplementationPointerR ();
    return impl_p->allBeamOffsetsZero ();
}

//void
//VisibilityIterator2::allSelectedSpectralWindows (Vector<Int>& spws, Vector<Int>& nvischan)
//{
//    CheckImplementationPointerR ();
//    impl_p->allSelectedSpectralWindows (spws, nvischan);
//}

void
VisibilityIterator2::antenna1 (Vector<Int>& ant1) const
{
    CheckImplementationPointerR ();
    impl_p->antenna1 (ant1);
}

void
VisibilityIterator2::antenna2 (Vector<Int>& ant2) const
{
    CheckImplementationPointerR ();
    impl_p->antenna2 (ant2);
}

const Vector<String>&
VisibilityIterator2::antennaMounts () const
{
    CheckImplementationPointerR ();
    return impl_p->antennaMounts ();
}

const SortColumns &
VisibilityIterator2::getSortColumns() const
{
  CheckImplementationPointerR();
  return impl_p->getSortColumns();
}

const MeasurementSet&
VisibilityIterator2::ms () const
{
    CheckImplementationPointerR ();
    return impl_p->ms ();
}

void
VisibilityIterator2::arrayIds (Vector<Int>& ids) const
{
    CheckImplementationPointerR ();
    return impl_p->arrayIds (ids);
}


//void
//VisibilityIterator2::attachColumns (const Table &t)
//{
//    CheckImplementationPointerR ();
//    impl_p->attachColumns (t);
//}
//
//const Table
//VisibilityIterator2::attachTable () const
//{
//    CheckImplementationPointerR ();
//    return impl_p->attachTable ();
//}

const Vector<MDirection> &
VisibilityIterator2::azel (Double time) const
{
    CheckImplementationPointerR ();
    return impl_p->azel (time);
}

MDirection
VisibilityIterator2::azel0 (Double time) const
{
    CheckImplementationPointerR ();
    return impl_p->azel0 (time);
}

void
VisibilityIterator2::azelCalculate (Double time, MSDerivedValues & msd, Vector<MDirection> & azel,
                                     Int nAnt, const MEpoch & mEpoch0)
{
    VisibilityIteratorImpl2::azelCalculate (time, msd, azel, nAnt, mEpoch0);
}

void
VisibilityIterator2::azel0Calculate (Double time, MSDerivedValues & msd,
		                      MDirection & azel0, const MEpoch & mEpoch0)
{
    VisibilityIteratorImpl2::azel0Calculate (time, msd, azel0, mEpoch0);
}

void
VisibilityIterator2::jonesC (Vector<SquareMatrix<Complex,2> >& cjones) const
{
    CheckImplementationPointerR ();
    impl_p->jonesC (cjones);
}


void
VisibilityIterator2::corrType (Vector<Int>& corrTypes) const
{
    CheckImplementationPointerR ();
    impl_p->corrType (corrTypes);
}

Int
VisibilityIterator2::dataDescriptionId () const
{
    CheckImplementationPointerR ();
    return impl_p->dataDescriptionId ();
}

void
VisibilityIterator2::dataDescriptionIds (Vector<Int>& ddis) const
{
    CheckImplementationPointerR ();
    impl_p->dataDescriptionIds (ddis);
}

Bool
VisibilityIterator2::existsColumn (VisBufferComponent2 id) const
{
    CheckImplementationPointerR ();

    return impl_p->existsColumn (id);
}

Bool
VisibilityIterator2::existsFlagCategory() const
{
  CheckImplementationPointerR ();
  return impl_p->existsColumn (FlagCategory);
}


Bool
VisibilityIterator2::weightSpectrumExists () const
{
    CheckImplementationPointerR ();
    return impl_p->weightSpectrumExists();
}

Bool
VisibilityIterator2::weightSpectrumCorrectedExists () const
{
    CheckImplementationPointerR ();
    return impl_p->weightSpectrumCorrectedExists();
}

void
VisibilityIterator2::exposure (Vector<Double>& expo) const
{
    CheckImplementationPointerR ();
    impl_p->exposure (expo);
}

Vector<Float>
VisibilityIterator2::feed_paCalculate(Double time, MSDerivedValues & msd,
  									    Int nAntennas, const MEpoch & mEpoch0,
									    const Vector<Float> & receptor0Angle)
{
    return VisibilityIteratorImpl2::feed_paCalculate (time, msd, nAntennas, mEpoch0, receptor0Angle);
}

void
VisibilityIterator2::feed1 (Vector<Int>& fd1) const
{
    CheckImplementationPointerR ();
    impl_p->feed1 (fd1);
}

void
VisibilityIterator2::feed2 (Vector<Int>& fd2) const
{
    CheckImplementationPointerR ();
    impl_p->feed2 (fd2);
}

const Vector<Float> &
VisibilityIterator2::feed_pa (Double time) const
{
    CheckImplementationPointerR ();
    return impl_p->feed_pa (time);
}

void
VisibilityIterator2::fieldIds (Vector<Int>& ids) const
{
    CheckImplementationPointerR ();
    return impl_p->fieldIds (ids);
}

String
VisibilityIterator2::fieldName () const
{
    CheckImplementationPointerR ();
    return impl_p->fieldName ();
}

void
VisibilityIterator2::flag (Cube<Bool>& flags) const
{
    CheckImplementationPointerR ();
    impl_p->flag (flags);
}

void
VisibilityIterator2::flag (Matrix<Bool>& flags) const
{
    CheckImplementationPointerR ();
    impl_p->flag (flags);
}

void
VisibilityIterator2::flagCategory (Array<Bool>& flagCategories) const
{
    CheckImplementationPointerR ();
    impl_p->flagCategory (flagCategories);
}

void
VisibilityIterator2::flagRow (Vector<Bool>& rowflags) const
{
    CheckImplementationPointerR ();
    impl_p->flagRow (rowflags);
}

void
VisibilityIterator2::floatData (Cube<Float>& fcube) const
{
    CheckImplementationPointerR ();
    impl_p->floatData (fcube);
}

const Cube<RigidVector<Double, 2> >&
VisibilityIterator2::getBeamOffsets () const
{
    CheckImplementationPointerR ();
    return impl_p->getBeamOffsets ();
}

Int
VisibilityIterator2::getDataDescriptionId () const
{
    CheckImplementationPointerR ();
    return impl_p->dataDescriptionId ();
}

MEpoch
VisibilityIterator2::getEpoch () const
{
    CheckImplementationPointerR ();
    return impl_p->getEpoch ();
}

Int
VisibilityIterator2::getMeasurementSetId () const
{
    CheckImplementationPointerR ();
    return impl_p->msId ();
}

Int
VisibilityIterator2::getNAntennas () const
{
    CheckImplementationPointerR ();
    return impl_p->nAntennas ();
}

//MFrequency::Types
//VisibilityIterator2::getObservatoryFrequencyType () const
//{
//    CheckImplementationPointerR ();
//    return impl_p->getObservatoryFrequencyType ();
//}

//MPosition
//VisibilityIterator2::getObservatoryPosition () const
//{
//    CheckImplementationPointerR ();
//
//    return impl_p->getObservatoryPosition ();
//}

//MDirection
//VisibilityIterator2::getPhaseCenter () const
//{
//    CheckImplementationPointerR ();
//
//    return impl_p->getPhaseCenter ();
//}

ViImplementation2 *
VisibilityIterator2::getImpl () const
{
    return impl_p;
}

//void
//VisibilityIterator2::getSpwInFreqRange (Block<Vector<Int> >& spw,
//                                         Block<Vector<Int> >& start,
//                                         Block<Vector<Int> >& nchan,
//                                         Double freqStart,
//                                         Double freqEnd,
//                                         Double freqStep,
//                                         MFrequency::Types freqFrame)
//{
//    CheckImplementationPointerR ();
//    impl_p->getSpwInFreqRange (spw, start, nchan, freqStart, freqEnd, freqStep, freqFrame);
//}
//
//void
//VisibilityIterator2::getFreqInSpwRange(Double& freqStart, Double& freqEnd, MFrequency::Types freqframe){
//  CheckImplementationPointerR ();
//  impl_p->getFreqInSpwRange(freqStart, freqEnd, freqframe);
//}

Int
VisibilityIterator2::getReportingFrameOfReference () const
{
    return impl_p->getReportingFrameOfReference ();
}

void
VisibilityIterator2::setReportingFrameOfReference (Int frame)
{
    impl_p->setReportingFrameOfReference (frame);
}


Subchunk
VisibilityIterator2::getSubchunkId () const
{
    CheckImplementationPointerR ();
    return impl_p->getSubchunkId ();
}


VisBuffer2 *
VisibilityIterator2::getVisBuffer ()
{
    CheckImplementationPointerR ();
    return impl_p->getVisBuffer ();
}

Double
VisibilityIterator2::hourang (Double time) const
{
    CheckImplementationPointerR ();
    return impl_p->hourang (time);
}

Double
VisibilityIterator2::hourangCalculate (Double time, MSDerivedValues & msd, const MEpoch & mEpoch0)
{
    return VisibilityIteratorImpl2::hourangCalculate (time, msd, mEpoch0);
}

//Matrix<Float>&
//VisibilityIterator2::imagingWeight (Matrix<Float>& wt) const
//{
//    CheckImplementationPointerR ();
//    return impl_p->imagingWeight (wt);
//}

Vector<Double>
VisibilityIterator2::getFrequencies (Double time, Int frameOfReference) const
{
    CheckImplementationPointerR ();
    return impl_p->getFrequencies (time, frameOfReference);
}

Vector<Int>
VisibilityIterator2::getChannels (Double time, Int frameOfReference) const
{
    CheckImplementationPointerR ();
    return impl_p->getChannels (time, frameOfReference);
}

Vector<Int>
VisibilityIterator2::getCorrelations () const
{
    CheckImplementationPointerR ();
    return impl_p->getCorrelations ();
}




const VisImagingWeight &
VisibilityIterator2::getImagingWeightGenerator () const
{
    CheckImplementationPointerR ();
    return impl_p->getImagingWeightGenerator ();
}

Bool
VisibilityIterator2::isAsynchronous () const
{
//    Bool isAsync = impl_p != NULL && dynamic_cast<const ViImplAsync2 *> (impl_p) != NULL;
//
//    return isAsync;

    return False; // for now
}


Bool
VisibilityIterator2::isAsynchronousIoEnabled()
{
    // Determines whether asynchronous I/O is enabled by looking for the
    // expected AipsRc value.  If not found then async i/o is disabled.

    Bool isEnabled;
    AipsrcValue<Bool>::find (isEnabled, getAipsRcBase () + ".enabled", False);

    return isEnabled;
}

//Bool
//VisibilityIterator2::isInSelectedSPW (const Int& spw)
//{
//    CheckImplementationPointerR ();
//    return impl_p->isInSelectedSPW (spw);
//}

Bool
VisibilityIterator2::isWritable () const
{
    CheckImplementationPointerR ();
    return impl_p->isWritable ();
}

Bool
VisibilityIterator2::more () const
{
    CheckImplementationPointerR ();
    return impl_p->more ();
}

Bool
VisibilityIterator2::moreChunks () const
{
    CheckImplementationPointerR ();
    return impl_p->moreChunks ();
}

Int
VisibilityIterator2::msId () const
{
    CheckImplementationPointerR ();
    return impl_p->msId ();
}

Int
VisibilityIterator2::nRows () const
{
    CheckImplementationPointerR ();
    return impl_p->nRows ();
}

Int
VisibilityIterator2::nRowsInChunk () const
{
    CheckImplementationPointerR ();
    return impl_p->nRowsInChunk ();
}

Bool
VisibilityIterator2::newArrayId () const
{
    CheckImplementationPointerR ();
    return impl_p->isNewArrayId ();
}

Bool
VisibilityIterator2::newFieldId () const
{
    CheckImplementationPointerR ();
    return impl_p->isNewFieldId ();
}

Bool
VisibilityIterator2::isNewMS () const
{
    CheckImplementationPointerR ();
    return impl_p->isNewMs ();
}

Bool
VisibilityIterator2::newSpectralWindow () const
{
    CheckImplementationPointerR ();
    return impl_p->isNewSpectralWindow ();
}

void
VisibilityIterator2::nextChunk ()
{
    CheckImplementationPointerR ();
    impl_p->nextChunk ();
}

Int
VisibilityIterator2::nAntennas () const
{
    CheckImplementationPointerR ();
    return impl_p->nAntennas ();
}

Int
VisibilityIterator2::nRowsViWillSweep () const
{
    CheckImplementationPointerR ();
    return impl_p->nRowsViWillSweep ();
}

Int
VisibilityIterator2::nDataDescriptionIds () const
{
    CheckImplementationPointerR ();
    return impl_p->nDataDescriptionIds ();
}

Int
VisibilityIterator2::nPolarizations () const
{
    CheckImplementationPointerR ();
    return impl_p->nPolarizations ();
}

Int
VisibilityIterator2::nSpectralWindows () const
{
    CheckImplementationPointerR ();
    return impl_p->nSpectralWindows ();
}

void
VisibilityIterator2::observationId (Vector<Int>& obsids) const
{
    CheckImplementationPointerR ();
    impl_p->observationId (obsids);
}

void
VisibilityIterator2::origin ()
{
    CheckImplementationPointerR ();
    impl_p->origin ();
}

void
VisibilityIterator2::originChunks ()
{
    CheckImplementationPointerR ();
    originChunks (False);
}

void
VisibilityIterator2::originChunks (Bool forceRewind)
{
    CheckImplementationPointerR ();
    impl_p->originChunks (forceRewind);
}

const Vector<Float> &
VisibilityIterator2::parang (Double time) const
{
    CheckImplementationPointerR ();
    return impl_p->parang (time);
}

const Float&
VisibilityIterator2::parang0 (Double time) const
{
    CheckImplementationPointerR ();
    return impl_p->parang0 (time);
}


Float
VisibilityIterator2::parang0Calculate (Double time, MSDerivedValues & msd, const MEpoch & epoch0)
{
    return VisibilityIteratorImpl2::parang0Calculate (time, msd, epoch0);
}

Vector<Float>
VisibilityIterator2::parangCalculate (Double time, MSDerivedValues & msd,
                                       int nAntennas, const MEpoch mEpoch0)
{
    return VisibilityIteratorImpl2::parangCalculate (time, msd, nAntennas, mEpoch0);
}

const MDirection&
VisibilityIterator2::phaseCenter () const
{
    CheckImplementationPointerR ();
    return impl_p->phaseCenter ();
}

Int
VisibilityIterator2::polFrame () const
{
    CheckImplementationPointerR ();
    return impl_p->polFrame ();
}

Int
VisibilityIterator2::polarizationId () const
{
    CheckImplementationPointerR ();
    return impl_p->polarizationId ();
}

void
VisibilityIterator2::processorId (Vector<Int>& procids) const
{
    CheckImplementationPointerR ();
    impl_p->processorId (procids);
}

const Cube<Double>&
VisibilityIterator2::receptorAngles () const
{
    CheckImplementationPointerR ();
    return impl_p->receptorAngles ();
}

Vector<Float>
VisibilityIterator2::getReceptor0Angle ()
{
    CheckImplementationPointerR ();
    return impl_p->getReceptor0Angle();
}

void
VisibilityIterator2::getRowIds (Vector <uInt> & value) const
{
    CheckImplementationPointerR();

    impl_p->getRowIds (value);
}

void
VisibilityIterator2::scan (Vector<Int>& scans) const
{
    CheckImplementationPointerR ();
    impl_p->scan (scans);
}

void
VisibilityIterator2::setFrequencySelection (const FrequencySelection & selection)
{
    FrequencySelections selections;
    selections.add (selection);
    setFrequencySelection (selections);
}

void
VisibilityIterator2::setFrequencySelection (const FrequencySelections & selections)
{
    ThrowIf (selections.size () != impl_p->getNMs (),
             String::format ("Frequency selection size, %d, does not VisibilityIterator # of MSs, %d.",
                     impl_p->getNMs (), selections.size()));

    CheckImplementationPointerR ();
    impl_p->setFrequencySelections (selections);
}


Double
VisibilityIterator2::getInterval() const
{
    CheckImplementationPointerR ();
    return impl_p->getInterval();
}

void
VisibilityIterator2::setInterval (Double timeInterval)
{
    CheckImplementationPointerR ();
    impl_p->setInterval (timeInterval);
}

void
VisibilityIterator2::setRowBlocking (Int nRows) // for use by Async I/O *ONLY
{
    CheckImplementationPointerR ();
    impl_p->setRowBlocking (nRows);
}

void
VisibilityIterator2::sigma (Matrix<Float>& sig) const
{
    CheckImplementationPointerR ();
    impl_p->sigma (sig);
}

void
VisibilityIterator2::slurp () const
{
    CheckImplementationPointerR ();
    impl_p->slurp ();
}

String
VisibilityIterator2::sourceName () const
{
    CheckImplementationPointerR ();
    return impl_p->sourceName ();
}

Int
VisibilityIterator2::spectralWindow () const
{
    CheckImplementationPointerR ();
    return impl_p->spectralWindow ();
}

void
VisibilityIterator2::spectralWindows (Vector<Int>& spws) const
{
    CheckImplementationPointerR ();
    impl_p->spectralWindows (spws);
}

void
VisibilityIterator2::stateId (Vector<Int>& stateids) const
{
    CheckImplementationPointerR ();
    impl_p->stateId (stateids);
}

const vi::SubtableColumns &
VisibilityIterator2::subtableColumns () const
{
    CheckImplementationPointerR ();
    return impl_p->subtableColumns ();
}

void
VisibilityIterator2::time (Vector<Double>& t) const
{
    CheckImplementationPointerR ();
    impl_p->time (t);
}

void
VisibilityIterator2::timeCentroid (Vector<Double>& t) const
{
    CheckImplementationPointerR ();
    impl_p->timeCentroid (t);
}

void
VisibilityIterator2::timeInterval (Vector<Double>& ti) const
{
    CheckImplementationPointerR ();
    impl_p->timeInterval (ti);
}

void
VisibilityIterator2::useImagingWeight (const VisImagingWeight& imWgt)
{
    CheckImplementationPointerR ();
    impl_p->useImagingWeight (imWgt);
}

void
VisibilityIterator2::uvw (Matrix<Double>& uvw) const
{
    CheckImplementationPointerR ();
    impl_p->uvw (uvw);
}

void
VisibilityIterator2::visibilityCorrected (Cube<Complex>& vis) const
{
    CheckImplementationPointerR ();
    impl_p->visibilityCorrected (vis);
}

void
VisibilityIterator2::visibilityModel (Cube<Complex>& vis) const
{
    CheckImplementationPointerR ();
    impl_p->visibilityModel (vis);
}

void
VisibilityIterator2::visibilityObserved (Cube<Complex>& vis) const
{
    CheckImplementationPointerR ();
    impl_p->visibilityObserved (vis);
}

//void
//VisibilityIterator2::visibilityCorrected (Matrix<CStokesVector>& vis) const
//{
//    CheckImplementationPointerR ();
//    impl_p->visibilityCorrected (vis);
//}

//void
//VisibilityIterator2::visibilityModel (Matrix<CStokesVector>& vis) const
//{
//    CheckImplementationPointerR ();
//    impl_p->visibilityModel (vis);
//}

//void
//VisibilityIterator2::visibilityObserved (Matrix<CStokesVector>& vis) const
//{
//    CheckImplementationPointerR ();
//    impl_p->visibilityObserved (vis);
//}

IPosition
VisibilityIterator2::visibilityShape () const
{
    CheckImplementationPointerR ();
    return impl_p->visibilityShape ();
}

void
VisibilityIterator2::weight (Matrix<Float>& wt) const
{
    CheckImplementationPointerR ();
    impl_p->weight (wt);
}

void
VisibilityIterator2::weightSpectrum (Cube<Float>& wtsp) const
{
    CheckImplementationPointerR ();
    impl_p->weightSpectrum (wtsp);
}

void
VisibilityIterator2::weightSpectrumCorrected (Cube<Float>& wtsp) const
{
    CheckImplementationPointerR ();
    impl_p->weightSpectrumCorrected (wtsp);
}

//void
//VisibilityIterator2::writeFlag (const Matrix<Bool>& flag)
//{
//    CheckImplementationPointerW ();
//    impl_p->writeFlag (flag);
//}

void
VisibilityIterator2::writeFlag (const Cube<Bool>& flag)
{
    CheckImplementationPointerW ();
    impl_p->writeFlag (flag);
}
void
VisibilityIterator2::writeFlagCategory(const Array<Bool>& flagCategory)
{
    CheckImplementationPointerW ();
    impl_p->writeFlagCategory (flagCategory);
}

void
VisibilityIterator2::writeFlagRow (const Vector<Bool>& rowflags)
{
    CheckImplementationPointerW ();
    impl_p->writeFlagRow (rowflags);
}

void
VisibilityIterator2::writeSigma (const Matrix<Float>& sig)
{
    CheckImplementationPointerW ();
    impl_p->writeSigma (sig);
}

//void
//VisibilityIterator2::writeSigmaMat (const Matrix<Float>& sigmat)
//{
//    CheckImplementationPointerW ();
//    impl_p->writeSigmaMat (sigmat);
//}

//void
//VisibilityIterator2::writeVisCorrected (const Matrix<CStokesVector>& vis)
//{
//    CheckImplementationPointerW ();
//    impl_p->writeVisCorrected (vis);
//}

//void
//VisibilityIterator2::writeVisModel (const Matrix<CStokesVector>& vis)
//{
//    CheckImplementationPointerW ();
//    impl_p->writeVisModel (vis);
//}

//void
//VisibilityIterator2::writeVisObserved (const Matrix<CStokesVector>& vis)
//{
//    CheckImplementationPointerW ();
//    impl_p->writeVisObserved (vis);
//}

void
VisibilityIterator2::writeVisCorrected (const Cube<Complex>& vis)
{
    CheckImplementationPointerW ();
    impl_p->writeVisCorrected (vis);
}

void
VisibilityIterator2::writeVisModel (const Cube<Complex>& vis)
{
    CheckImplementationPointerW ();
    impl_p->writeVisModel (vis);
}

void
VisibilityIterator2::writeVisObserved (const Cube<Complex>& vis)
{
    CheckImplementationPointerW ();
    impl_p->writeVisObserved (vis);
}

void
VisibilityIterator2::writeWeight (const Matrix<Float>& wt)
{
    CheckImplementationPointerW ();
    impl_p->writeWeight (wt);
}

//void
//VisibilityIterator2::writeWeightMat (const Matrix<Float>& wtmat)
//{
//    CheckImplementationPointerW ();
//    impl_p->writeWeightMat (wtmat);
//}

void
VisibilityIterator2::writeWeightSpectrum (const Cube<Float>& wtsp)
{
    CheckImplementationPointerW ();
    impl_p->writeWeightSpectrum (wtsp);
}

void
VisibilityIterator2::writeModel(const RecordInterface& rec, Bool iscomponentlist, Bool incremental){
  CheckImplementationPointerW ();
  impl_p->writeModel(rec, iscomponentlist, incremental);

}

void
VisibilityIterator2::writeBackChanges (VisBuffer2 * vb)
{
    CheckImplementationPointerW ();
    impl_p->writeBackChanges (vb);
}

void
VisibilityIterator2::setWeightScaling (CountedPtr<WeightScaling> weightScaling)
{
  CheckImplementationPointerW ();

  impl_p->setWeightScaling (weightScaling);
}

Bool
VisibilityIterator2::hasWeightScaling () const
{
  CheckImplementationPointerW ();

  return impl_p->hasWeightScaling ();
}

SubtableColumns::SubtableColumns (CountedPtr <MSIter> msIter)
: msIter_p (msIter)
{}

const ROMSAntennaColumns&
SubtableColumns::antenna() const
{
    return msIter_p->msColumns().antenna();
}

const ROMSDataDescColumns&
SubtableColumns::dataDescription() const
{
    return msIter_p->msColumns().dataDescription();
}

const ROMSFeedColumns&
SubtableColumns::feed() const
{
    return msIter_p->msColumns().feed();
}

const ROMSFieldColumns&
SubtableColumns::field() const
{
    return msIter_p->msColumns().field();
}

const ROMSFlagCmdColumns&
SubtableColumns::flagCmd() const
{
    return msIter_p->msColumns().flagCmd();
}

const ROMSHistoryColumns&
SubtableColumns::history() const
{
    return msIter_p->msColumns().history();
}

const ROMSObservationColumns&
SubtableColumns::observation() const
{
    return msIter_p->msColumns().observation();
}

const ROMSPointingColumns&
SubtableColumns::pointing() const
{
    return msIter_p->msColumns().pointing();
}

const ROMSPolarizationColumns&
SubtableColumns::polarization() const
{

    return msIter_p->msColumns().polarization();
}

const ROMSProcessorColumns&
SubtableColumns::processor() const
{
    return msIter_p->msColumns().processor();
}

const ROMSSpWindowColumns&
SubtableColumns::spectralWindow() const
{

    return msIter_p->msColumns().spectralWindow();
}

const ROMSStateColumns&
SubtableColumns::state() const
{
    return msIter_p->msColumns().state();
}

const ROMSDopplerColumns&
SubtableColumns::doppler() const
{
    return msIter_p->msColumns().doppler();
}

const ROMSFreqOffsetColumns&
SubtableColumns::freqOffset() const
{
    return msIter_p->msColumns().freqOffset();
}

const ROMSSourceColumns&
SubtableColumns::source() const
{
    return msIter_p->msColumns().source();
}

const ROMSSysCalColumns&
SubtableColumns::sysCal() const
{
    return msIter_p->msColumns().sysCal();
}

const ROMSWeatherColumns&
SubtableColumns::weather() const
{
    return msIter_p->msColumns().weather();
}


} // end namespace vi


} // end namespace casa
