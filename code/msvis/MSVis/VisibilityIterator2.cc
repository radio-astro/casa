/*
 * VisibilityIterator2.cc
 *
 *  Created on: Jun 4, 2012
 *      Author: jjacobs
 */

#include <msvis/MSVis/VisibilityIterator2.h>

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
#include <ms/MSOper/MSDerivedValues.h>
#include <ms/MeasurementSets/MSIter.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <scimath/Mathematics/RigidVector.h>
#include <scimath/Mathematics/SquareMatrix.h>
#include <msvis/MSVis/AveragingTvi2.h>
#include <msvis/MSVis/ViFrequencySelection.h>
#include <msvis/MSVis/StokesVector.h>
#include <msvis/MSVis/VisBuffer2.h>
#include <msvis/MSVis/VisBufferComponents2.h>
#include <msvis/MSVis/VisImagingWeight.h>
#include <msvis/MSVis/VisibilityIteratorImpl2.h>
#include <msvis/MSVis/VisibilityIteratorImplAsync2.h>
#include <msvis/MSVis/UtilJ.h>
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
    ViImplementation2 * newImpl = factory.createVi ();

    impl_p = newImpl;
}

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
        impl_p = new VisibilityIteratorImpl2 (mss, sortColumns, timeInterval, VbPlain, writable);
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


const MeasurementSet&
VisibilityIterator2::ms () const
{
    CheckImplementationPointerR ();
    return impl_p->ms ();
}

Bool
VisibilityIterator2::existsColumn (VisBufferComponent2 id) const
{
    CheckImplementationPointerR ();

    return impl_p->existsColumn (id);
}

Bool
VisibilityIterator2::weightSpectrumExists () const
{
    CheckImplementationPointerR ();
    return impl_p->weightSpectrumExists();
}

ViImplementation2 *
VisibilityIterator2::getImpl () const
{
    return impl_p;
}

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
    return impl_p->getVisBuffer (this);
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
    AipsrcValue<Bool>::find (isEnabled, getAipsRcBase () + "async.enabled", False);

    return isEnabled;
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


void
VisibilityIterator2::nextChunk ()
{
    CheckImplementationPointerR ();
    impl_p->nextChunk ();
}

Int
VisibilityIterator2::nDataDescriptionIds () const
{
    CheckImplementationPointerR ();
    return impl_p->nDataDescriptionIds ();
}
//
Int
VisibilityIterator2::nPolarizationIds () const
{
    CheckImplementationPointerR ();
    return impl_p->nPolarizationIds ();
}
//
Int
VisibilityIterator2::nSpectralWindows () const
{
    CheckImplementationPointerR ();
    return impl_p->nSpectralWindows ();
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

void
VisibilityIterator2::setRowBlocking (Int nRows) // for use by Async I/O *ONLY
{
    CheckImplementationPointerR ();
    impl_p->setRowBlocking (nRows);
}

void
VisibilityIterator2::slurp () const
{
    CheckImplementationPointerR ();
    impl_p->slurp ();
}

Int
VisibilityIterator2::nRowsInChunk () const
{
    CheckImplementationPointerR ();
    return impl_p->nRowsInChunk ();
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



const vi::SubtableColumns &
VisibilityIterator2::subtableColumns () const
{
    CheckImplementationPointerR ();
    return impl_p->subtableColumns ();
}

void
VisibilityIterator2::writeFlag (const Cube<Bool>& flag)
{
    CheckImplementationPointerW ();
    impl_p->writeFlag (flag);
}

void
VisibilityIterator2::writeFlagRow (const Vector<Bool>& rowflags)
{
    CheckImplementationPointerW ();
    impl_p->writeFlagRow (rowflags);
}

void
VisibilityIterator2::writeWeightSpectrum (const Cube<Float>& wtsp)
{
    CheckImplementationPointerW ();
    impl_p->writeWeightSpectrum (wtsp);
}
//
void
VisibilityIterator2::initWeightSpectrum (const Cube<Float>& wtsp)
{
    CheckImplementationPointerW ();
    impl_p->initWeightSpectrum (wtsp);
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
