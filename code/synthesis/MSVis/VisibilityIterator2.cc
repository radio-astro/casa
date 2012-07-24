/*
 * VisibilityIterator2.cc
 *
 *  Created on: Jun 4, 2012
 *      Author: jjacobs
 */

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
#include <synthesis/MSVis/StokesVector.h>
#include <synthesis/MSVis/VisBufferComponents.h>
#include <synthesis/MSVis/VisImagingWeight.h>
#include <synthesis/MSVis/VisibilityIterator2.h>
#include <synthesis/MSVis/VisibilityIteratorImpl2.h>
#include <synthesis/MSVis/VisibilityIteratorImplAsync2.h>
#include <synthesis/MSVis/UtilJ.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ScalarColumn.h>

#include <map>
#include <set>
#include <vector>

#define CheckImplementationPointerR() Assert (readImpl_p != NULL);
#define CheckImplementationPointerW() Assert (writeImpl_p != NULL);

namespace casa {

namespace asyncio {

#if 0
PrefetchColumns
PrefetchColumns::prefetchColumnsAll ()
{
    // Create the set of all columns

    PrefetchColumns pc;

    for (int i = 0; i < VisBufferComponents::N_VisBufferComponents; ++ i){
        pc.insert ((VisBufferComponents::EnumType) i);
    }

    return pc;
}

PrefetchColumns
PrefetchColumns::prefetchAllColumnsExcept (Int firstColumn, ...)
{
    // Build a set of Prefetch columns named except.
    // The last arg must be either negative or
    // greater than or equal to N_PrefetchColumnIds

    va_list vaList;

    va_start (vaList, firstColumn);

    int id = firstColumn;
    PrefetchColumns except;

    while (id >= 0 && id < VisBufferComponents::N_VisBufferComponents){
        except.insert ((VisBufferComponents::EnumType)id);
        id = va_arg (vaList, int);
    }

    va_end (vaList);

    // Get the set of all columns and then subtract off the
    // caller specified columns.  Return the result

    PrefetchColumns allColumns = prefetchColumnsAll();
    PrefetchColumns result;

    set_difference (allColumns.begin(), allColumns.end(),
                    except.begin(), except.end(),
                    inserter (result, result.begin()));

    return result;

}

String
PrefetchColumns::columnName (Int id)
{
    assert (id >= 0 && id < VisBufferComponents::N_VisBufferComponents);

    // This method is called only occasionally for debuggin so at this time a
    // brute-force implmentation is acceptable.

    map<Int,String> names;

    names [VisBufferComponents::Ant1] = "Ant1";
    names [VisBufferComponents::Ant2] = "Ant2";
    names [VisBufferComponents::ArrayId] = "ArrayId";
    names [VisBufferComponents::Channel] = "Channel";
    names [VisBufferComponents::Cjones] = "Cjones";
    names [VisBufferComponents::CorrType] = "CorrType";
    names [VisBufferComponents::Corrected] = "Corrected";
    names [VisBufferComponents::CorrectedCube] = "CorrectedCube";
    names [VisBufferComponents::Direction1] = "Direction1";
    names [VisBufferComponents::Direction2] = "Direction2";
    names [VisBufferComponents::Exposure] = "Exposure";
    names [VisBufferComponents::Feed1] = "Feed1";
    names [VisBufferComponents::Feed1_pa] = "Feed1_pa";
    names [VisBufferComponents::Feed2] = "Feed2";
    names [VisBufferComponents::Feed2_pa] = "Feed2_pa";
    names [VisBufferComponents::FieldId] = "FieldId";
    names [VisBufferComponents::Flag] = "Flag";
    names [VisBufferComponents::FlagCategory] = "FlagCategory";
    names [VisBufferComponents::FlagCube] = "FlagCube";
    names [VisBufferComponents::FlagRow] = "FlagRow";
    names [VisBufferComponents::Freq] = "Freq";
    names [VisBufferComponents::ImagingWeight] = "ImagingWeight";
    names [VisBufferComponents::Model] = "Model";
    names [VisBufferComponents::ModelCube] = "ModelCube";
    names [VisBufferComponents::NChannel] = "NChannel";
    names [VisBufferComponents::NCorr] = "NCorr";
    names [VisBufferComponents::NRow] = "NRow";
    names [VisBufferComponents::ObservationId] = "ObservationId";
    names [VisBufferComponents::Observed] = "Observed";
    names [VisBufferComponents::ObservedCube] = "ObservedCube";
    names [VisBufferComponents::PhaseCenter] = "PhaseCenter";
    names [VisBufferComponents::PolFrame] = "PolFrame";
    names [VisBufferComponents::ProcessorId] = "ProcessorId";
    names [VisBufferComponents::Scan] = "Scan";
    names [VisBufferComponents::Sigma] = "Sigma";
    names [VisBufferComponents::SigmaMat] = "SigmaMat";
    names [VisBufferComponents::SpW] = "SpW";
    names [VisBufferComponents::StateId] = "StateId";
    names [VisBufferComponents::Time] = "Time";
    names [VisBufferComponents::TimeCentroid] = "TimeCentroid";
    names [VisBufferComponents::TimeInterval] = "TimeInterval";
    names [VisBufferComponents::Weight] = "Weight";
    names [VisBufferComponents::WeightMat] = "WeightMat";
    names [VisBufferComponents::WeightSpectrum] = "WeightSpectrum";
    names [VisBufferComponents::Uvw] = "Uvw";
    names [VisBufferComponents::UvwMat] = "UvwMat";

    return names [id];
}


PrefetchColumns
PrefetchColumns::prefetchColumns (Int firstColumn, ...)
{
    // Returns a set of Prefetch columns.  The last arg must be either negative or
    // greater than or equal to N_PrefetchColumnIds

    va_list vaList;

    va_start (vaList, firstColumn);

    Int id = firstColumn;
    PrefetchColumns pc;

    while (id >= 0 && id < VisBufferComponents::N_VisBufferComponents){
        pc.insert ((VisBufferComponents::EnumType) id);
        id = va_arg (vaList, Int);
    }

    va_end (vaList);

    return pc;
}

#endif

};  // end namespace asyncio

String
FrequencySelection::frameName (FrameOfReference referenceFrame)
{
    String names [] =
    { "AsSelected",
      "Channels",
      "Topo",
      "Lsrk"
    };

    ThrowIf (referenceFrame < 0 || referenceFrame >= N_FramesOfReference,
             utilj::format ("Unknown frame of reference id: %d", referenceFrame));

    return names [referenceFrame];
}

FrequencySelection::FrameOfReference
FrequencySelection::getFrameOfReference () const
{
    return referenceFrame_p;
}

String
FrequencySelection::toString () const
{
    return utilj::format ("frame='%s'", frameName (referenceFrame_p).c_str());
}

FrequencySelections::FrequencySelections ()
{}

void
FrequencySelections::add (const FrequencySelection & selection)
{
    if (! selections_p.empty()){
        ThrowIf (getFrameOfReference() != selection.getFrameOfReference(),
                 utilj::format ("Frequency selection #i has incompatible frame of reference %d (!= %d)",
                                selections_p.size() + 1,
                                selection.getFrameOfReference(),
                                getFrameOfReference()));
    }

    selections_p.push_back (selection.clone());
}

Int
FrequencySelections::size () const
{
    return (Int) selections_p.size();
}

ROVisibilityIterator2::ROVisibilityIterator2 ()
: nMS_p (0), readImpl_p (NULL)
{}

ROVisibilityIterator2::ROVisibilityIterator2 (const MeasurementSet& ms,
                                            const Block<Int>& sortColumns,
                                            Double timeInterval,
                                            const Factory & factory)
{
    construct (NULL, Block<MeasurementSet> (1, ms), sortColumns, True,
               timeInterval, False, factory);
}

ROVisibilityIterator2::ROVisibilityIterator2 (const MeasurementSet& ms,
                                            const Block<Int>& sortColumns,
                                            const Bool addDefaultSortCols,
                                            Double timeInterval)
{
    construct (NULL, Block<MeasurementSet> (1, ms), sortColumns, addDefaultSortCols,
               timeInterval, False, Factory ());
}

ROVisibilityIterator2::ROVisibilityIterator2 (const Block<MeasurementSet>& mss,
                                            const Block<Int>& sortColumns,
                                            Double timeInterval)
{
    construct (NULL, mss, sortColumns, True, timeInterval, False, Factory ());
}

ROVisibilityIterator2::ROVisibilityIterator2 (const Block<MeasurementSet>& mss,
                                            const Block<Int>& sortColumns,
                                            const Bool addDefaultSortCols,
                                            Double timeInterval)
{
    construct (NULL, mss, sortColumns, addDefaultSortCols, timeInterval, False, Factory());
}

ROVisibilityIterator2::ROVisibilityIterator2(const asyncio::PrefetchColumns * prefetchColumns,
                                           const MeasurementSet& ms,
                                           const Block<Int>& sortColumns,
                                           const Bool addDefaultSortCols,
                                           Double timeInterval)
{
    construct (prefetchColumns, Block<MeasurementSet> (1, ms), sortColumns, addDefaultSortCols,
               timeInterval, False, Factory ());
}

ROVisibilityIterator2::ROVisibilityIterator2 (const asyncio::PrefetchColumns * prefetchColumns,
                                            const Block<MeasurementSet>& mss,
                                            const Block<Int>& sortColumns,
                                            const Bool addDefaultSortCols,
                                            Double timeInterval)
{
    construct (prefetchColumns, mss, sortColumns, addDefaultSortCols, timeInterval, False, Factory());
}

ROVisibilityIterator2::ROVisibilityIterator2 (const asyncio::PrefetchColumns * prefetchColumns,
                                            const Block<MeasurementSet>& mss,
                                            const Block<Int>& sortColumns,
                                            const Bool addDefaultSortCols,
                                            Double timeInterval,
                                            Bool writable)
{
    construct (prefetchColumns, mss, sortColumns, addDefaultSortCols, timeInterval, writable, Factory());
}

ROVisibilityIterator2::ROVisibilityIterator2 (const asyncio::PrefetchColumns * prefetchColumns,
                                            const ROVisibilityIterator2 & other)
{
    // For potentially creating an asynchronous ROVI from a synchronous one

    Bool createAsAsynchronous = prefetchColumns != NULL && isAsynchronousIoEnabled ();

    if (createAsAsynchronous){

        Bool writable = dynamic_cast <const VisibilityIterator2 *> (& other) != NULL;

        readImpl_p = new ViReadImplAsync2 (* prefetchColumns, * other.readImpl_p, writable);
    }
    else{
        readImpl_p = new VisibilityIteratorReadImpl2 (* other.readImpl_p, this);
    }
}

ROVisibilityIterator2::ROVisibilityIterator2 (const ROVisibilityIterator2 & other)
{
    if (other.readImpl_p == NULL){
        readImpl_p = NULL;
    }
    else {
        readImpl_p = other.readImpl_p -> clone(this);
    }
}


ROVisibilityIterator2::~ROVisibilityIterator2 ()
{
    delete readImpl_p;
}

void
ROVisibilityIterator2::construct (const asyncio::PrefetchColumns * prefetchColumns,
                                 const Block<MeasurementSet>& mss,
                                 const Block<Int>& sortColumns,
                                 const Bool addDefaultSortCols,
                                 Double timeInterval,
                                 Bool writable,
                                 const Factory & factory)
{
    readImpl_p = factory (prefetchColumns, mss, sortColumns, addDefaultSortCols, timeInterval);

    if (readImpl_p == NULL) {

        // Factory didn't create the read implementation so decide whether to create a
        // synchronous or asynchronous read implementation.

        Bool createAsAsynchronous = prefetchColumns != NULL && isAsynchronousIoEnabled ();

        if (createAsAsynchronous){
            readImpl_p = new ViReadImplAsync2 (mss, * prefetchColumns, sortColumns,
                                              addDefaultSortCols, timeInterval, writable);
        }
        else{
            readImpl_p = new VisibilityIteratorReadImpl2 (this, mss, sortColumns,
                                                         addDefaultSortCols, timeInterval);
        }
    }

    nMS_p = mss.nelements();

    if (! writable){
        readImpl_p->originChunks();
    }
}

ROVisibilityIterator2 &
ROVisibilityIterator2::operator= (const ROVisibilityIterator2 &other)
{
    // Kill off the current read implementation

    if (readImpl_p != NULL){
        delete readImpl_p;
        readImpl_p = NULL;
    }

    // If the other's read implementation is not null then
    // put a clone of it into this object's read implementation.

    if (other.readImpl_p != NULL){

        readImpl_p = other.readImpl_p->clone (this);
    }
    return * this;
}

ROVisibilityIterator2 &
ROVisibilityIterator2::operator++ (int)
{
    advance ();

    return * this;
}

ROVisibilityIterator2 &
ROVisibilityIterator2::operator++ ()
{
    advance ();

    return * this;
}

void
ROVisibilityIterator2::advance ()
{
    CheckImplementationPointerR ();
    readImpl_p->advance ();
}

Bool
ROVisibilityIterator2::allBeamOffsetsZero () const
{
    CheckImplementationPointerR ();
    return readImpl_p->allBeamOffsetsZero ();
}

void
ROVisibilityIterator2::allSelectedSpectralWindows (Vector<Int>& spws, Vector<Int>& nvischan)
{
    CheckImplementationPointerR ();
    readImpl_p->allSelectedSpectralWindows (spws, nvischan);
}

void
ROVisibilityIterator2::antenna1 (Vector<Int>& ant1) const
{
    CheckImplementationPointerR ();
    readImpl_p->antenna1 (ant1);
}

void
ROVisibilityIterator2::antenna2 (Vector<Int>& ant2) const
{
    CheckImplementationPointerR ();
    readImpl_p->antenna2 (ant2);
}

const Vector<String>&
ROVisibilityIterator2::antennaMounts () const
{
    CheckImplementationPointerR ();
    return readImpl_p->antennaMounts ();
}

const Block<Int>& ROVisibilityIterator2::getSortColumns() const
{
  CheckImplementationPointerR();
  return readImpl_p->getSortColumns();
}

const MeasurementSet &
ROVisibilityIterator2::getMeasurementSet () const
{
    CheckImplementationPointerR ();
    return readImpl_p->getMeasurementSet ();
}


const MeasurementSet&
ROVisibilityIterator2::ms () const
{
    CheckImplementationPointerR ();
    return readImpl_p->ms ();
}

Int
ROVisibilityIterator2::arrayId () const
{
    CheckImplementationPointerR ();
    return readImpl_p->arrayId ();
}


void
ROVisibilityIterator2::attachColumns (const Table &t)
{
    CheckImplementationPointerR ();
    readImpl_p->attachColumns (t);
}

const Table
ROVisibilityIterator2::attachTable () const
{
    CheckImplementationPointerR ();
    return readImpl_p->attachTable ();
}

void
ROVisibilityIterator2::attachVisBuffer (VisBuffer& vb)
{
    CheckImplementationPointerR ();
    readImpl_p->attachVisBuffer (vb);
}

Vector<MDirection>
ROVisibilityIterator2::azel (Double time) const
{
    CheckImplementationPointerR ();
    return readImpl_p->azel (time);
}

MDirection
ROVisibilityIterator2::azel0 (Double time) const
{
    CheckImplementationPointerR ();
    return readImpl_p->azel0 (time);
}

void
ROVisibilityIterator2::azelCalculate (Double time, MSDerivedValues & msd, Vector<MDirection> & azel,
                                     Int nAnt, const MEpoch & mEpoch0)
{
    VisibilityIteratorReadImpl2::azelCalculate (time, msd, azel, nAnt, mEpoch0);
}

void
ROVisibilityIterator2::azel0Calculate (Double time, MSDerivedValues & msd,
		                      MDirection & azel0, const MEpoch & mEpoch0)
{
    VisibilityIteratorReadImpl2::azel0Calculate (time, msd, azel0, mEpoch0);
}

void
ROVisibilityIterator2::channel (Vector<Int>& chan) const
{
    CheckImplementationPointerR ();
    readImpl_p->channel (chan);
}

Int
ROVisibilityIterator2::channelGroupSize () const
{
    CheckImplementationPointerR ();
    return readImpl_p->channelGroupSize ();
}

Int
ROVisibilityIterator2::channelIndex () const
{
    CheckImplementationPointerR ();
    return readImpl_p->channelIndex ();
}

void
ROVisibilityIterator2::jonesC (Vector<SquareMatrix<Complex,2> >& cjones) const
{
    CheckImplementationPointerR ();
    readImpl_p->jonesC (cjones);
}


void
ROVisibilityIterator2::corrType (Vector<Int>& corrTypes) const
{
    CheckImplementationPointerR ();
    readImpl_p->corrType (corrTypes);
}

Int
ROVisibilityIterator2::dataDescriptionId () const
{
    CheckImplementationPointerR ();
    return readImpl_p->dataDescriptionId ();
}

void
ROVisibilityIterator2::detachVisBuffer (VisBuffer& vb)
{
    CheckImplementationPointerR ();
    readImpl_p->detachVisBuffer (vb);
}

void
ROVisibilityIterator2::doChannelSelection ()
{
    CheckImplementationPointerR ();
    readImpl_p->doChannelSelection ();
}

Bool
ROVisibilityIterator2::existsColumn (VisBufferComponents::EnumType id) const
{
    CheckImplementationPointerR ();

    return readImpl_p->existsColumn (id);
}

Bool
ROVisibilityIterator2::existsFlagCategory() const
{
  CheckImplementationPointerR ();
  return readImpl_p->existsFlagCategory();
}

Bool
ROVisibilityIterator2::existsWeightSpectrum () const
{
    CheckImplementationPointerR ();
    return readImpl_p->existsWeightSpectrum ();
}

void
ROVisibilityIterator2::exposure (Vector<Double>& expo) const
{
    CheckImplementationPointerR ();
    readImpl_p->exposure (expo);
}

Vector<Float>
ROVisibilityIterator2::feed_paCalculate(Double time, MSDerivedValues & msd,
  									    Int nAntennas, const MEpoch & mEpoch0,
									    const Vector<Float> & receptor0Angle)
{
    return VisibilityIteratorReadImpl2::feed_paCalculate (time, msd, nAntennas, mEpoch0, receptor0Angle);
}



void
ROVisibilityIterator2::feed1 (Vector<Int>& fd1) const
{
    CheckImplementationPointerR ();
    readImpl_p->feed1 (fd1);
}

void
ROVisibilityIterator2::feed2 (Vector<Int>& fd2) const
{
    CheckImplementationPointerR ();
    readImpl_p->feed2 (fd2);
}

Vector<Float>
ROVisibilityIterator2::feed_pa (Double time) const
{
    CheckImplementationPointerR ();
    return readImpl_p->feed_pa (time);
}

Int
ROVisibilityIterator2::fieldId () const
{
    CheckImplementationPointerR ();
    return readImpl_p->fieldId ();
}

String
ROVisibilityIterator2::fieldName () const
{
    CheckImplementationPointerR ();
    return readImpl_p->fieldName ();
}

void
ROVisibilityIterator2::flag (Cube<Bool>& flags) const
{
    CheckImplementationPointerR ();
    readImpl_p->flag (flags);
}

void
ROVisibilityIterator2::flag (Matrix<Bool>& flags) const
{
    CheckImplementationPointerR ();
    readImpl_p->flag (flags);
}

void
ROVisibilityIterator2::flagCategory (Array<Bool>& flagCategories) const
{
    CheckImplementationPointerR ();
    readImpl_p->flagCategory (flagCategories);
}

void
ROVisibilityIterator2::flagRow (Vector<Bool>& rowflags) const
{
    CheckImplementationPointerR ();
    readImpl_p->flagRow (rowflags);
}

void
ROVisibilityIterator2::floatData (Cube<Float>& fcube) const
{
    CheckImplementationPointerR ();
    readImpl_p->floatData (fcube);
}

void
ROVisibilityIterator2::frequency (Vector<Double>& freq) const
{
    CheckImplementationPointerR ();
    readImpl_p->frequency (freq);
}

const Cube<RigidVector<Double, 2> >&
ROVisibilityIterator2::getBeamOffsets () const
{
    CheckImplementationPointerR ();
    return readImpl_p->getBeamOffsets ();
}

//ROArrayColumn
//<Double> &
//ROVisibilityIterator2::getChannelFrequency () const
//{
//    CheckImplementationPointerR ();
//    return readImpl_p->getChannelFrequency ();
//}
//
//Block<Int>
//ROVisibilityIterator2::getChannelGroupNumber () const
//{
//    CheckImplementationPointerR ();
//    return readImpl_p->getChannelGroupNumber ();
//}
//
//Block<Int>
//ROVisibilityIterator2::getChannelIncrement () const
//{
//    CheckImplementationPointerR ();
//    return readImpl_p->getChannelIncrement ();
//}

void
ROVisibilityIterator2::getChannelSelection (Block< Vector<Int> >& blockNGroup,
                                           Block< Vector<Int> >& blockStart,
                                           Block< Vector<Int> >& blockWidth,
                                           Block< Vector<Int> >& blockIncr,
                                           Block< Vector<Int> >& blockSpw)
{
    CheckImplementationPointerR ();
    readImpl_p->getChannelSelection (blockNGroup, blockStart, blockWidth, blockIncr, blockSpw);
}

//Block<Int>
//ROVisibilityIterator2::getChannelStart () const
//{
//    CheckImplementationPointerR ();
//    return readImpl_p->getChannelStart ();
//}
//
//Block<Int>
//ROVisibilityIterator2::getChannelWidth () const
//{
//    CheckImplementationPointerR ();
//    return readImpl_p->getChannelWidth ();
//}

void
ROVisibilityIterator2::getCol (const ROScalarColumn<Bool> &column, Vector<Bool> &array, Bool resize ) const
{
    CheckImplementationPointerR ();
    readImpl_p->getCol (column, array, resize);
}

void
ROVisibilityIterator2::getCol (const ROScalarColumn<Int> &column, Vector<Int> &array, Bool resize ) const
{
    CheckImplementationPointerR ();
    readImpl_p->getCol (column, array, resize);
}

void
ROVisibilityIterator2::getCol (const ROScalarColumn<Double> &column, Vector<Double> &array, Bool resize ) const
{
    CheckImplementationPointerR ();
    readImpl_p->getCol (column, array, resize);
}

void
ROVisibilityIterator2::getCol (const ROArrayColumn<Bool> &column, Array<Bool> &array, Bool resize ) const
{
    CheckImplementationPointerR ();
    readImpl_p->getCol (column, array, resize);
}

void
ROVisibilityIterator2::getCol (const ROArrayColumn<Float> &column, Array<Float> &array, Bool resize ) const
{
    CheckImplementationPointerR ();
    readImpl_p->getCol (column, array, resize);
}

void
ROVisibilityIterator2::getCol (const ROArrayColumn<Double> &column, Array<Double> &array, Bool resize ) const
{
    CheckImplementationPointerR ();
    readImpl_p->getCol (column, array, resize);
}

void
ROVisibilityIterator2::getCol (const ROArrayColumn<Complex> &column, Array<Complex> &array, Bool resize ) const
{
    CheckImplementationPointerR ();
    readImpl_p->getCol (column, array, resize);
}

void
ROVisibilityIterator2::getCol (const ROArrayColumn<Bool> &column, const Slicer &slicer, Array<Bool> &array, Bool resize ) const
{
    CheckImplementationPointerR ();
    readImpl_p->getCol (column, slicer, array, resize);
}

void
ROVisibilityIterator2::getCol (const ROArrayColumn<Float> &column, const Slicer &slicer, Array<Float> &array, Bool resize ) const
{
    CheckImplementationPointerR ();
    readImpl_p->getCol (column, slicer, array, resize);
}

void
ROVisibilityIterator2::getCol (const ROArrayColumn<Complex> &column, const Slicer &slicer, Array<Complex> &array, Bool resize ) const
{
    CheckImplementationPointerR ();
    readImpl_p->getCol (column, slicer, array, resize);
}

void
ROVisibilityIterator2::getDataColumn (DataColumn whichOne, const Slicer& slicer,
                                     Cube<Complex>& data) const
{
    CheckImplementationPointerR ();
    readImpl_p->getDataColumn (whichOne, slicer, data);
}

void
ROVisibilityIterator2::getDataColumn (DataColumn whichOne, Cube<Complex>& data) const
{
    CheckImplementationPointerR ();
    readImpl_p->getDataColumn (whichOne, data);
}

Int
ROVisibilityIterator2::getDataDescriptionId () const
{
    CheckImplementationPointerR ();
    return readImpl_p->getDataDescriptionId ();
}

void
ROVisibilityIterator2::getFloatDataColumn (const Slicer& slicer, Cube<Float>& data) const
{
    CheckImplementationPointerR ();
    readImpl_p->getFloatDataColumn (slicer, data);
}

void
ROVisibilityIterator2::getFloatDataColumn (Cube<Float>& data) const
{
    CheckImplementationPointerR ();
    readImpl_p->getFloatDataColumn (data);
}

//void
//ROVisibilityIterator2::getInterpolatedFloatDataFlagWeight () const
//{
//    CheckImplementationPointerR ();
//    readImpl_p->getInterpolatedFloatDataFlagWeight ();
//}

//void
//ROVisibilityIterator2::getInterpolatedVisFlagWeight (DataColumn whichOne) const
//{
//    CheckImplementationPointerR ();
//    readImpl_p->getInterpolatedVisFlagWeight (whichOne);
//}

void
ROVisibilityIterator2::getLsrInfo (Block<Int> & channelGroupNumber,
                                  Block<Int> & channelIncrement,
                                  Block<Int> & channelStart,
                                  Block<Int> & channelWidth,
                                  MPosition & observatoryPositon,
                                  MDirection & phaseCenter,
                                  Bool & velocitySelection) const
{
    CheckImplementationPointerR ();
    readImpl_p->getLsrInfo (channelGroupNumber, channelIncrement, channelStart, channelWidth, observatoryPositon, phaseCenter, velocitySelection);
}

MEpoch
ROVisibilityIterator2::getEpoch () const
{
    CheckImplementationPointerR ();
    return readImpl_p->getEpoch ();
}

const MSDerivedValues &
ROVisibilityIterator2::getMSD () const
{
    CheckImplementationPointerR ();
    return readImpl_p->getMSD ();
}

Int
ROVisibilityIterator2::getMeasurementSetId () const
{
    CheckImplementationPointerR ();
    return readImpl_p->getMeasurementSetId ();
}

std::vector<MeasurementSet>
ROVisibilityIterator2::getMeasurementSets () const
{
    CheckImplementationPointerR ();
    return readImpl_p->getMeasurementSets ();
}

Int
ROVisibilityIterator2::getNAntennas () const
{
    CheckImplementationPointerR ();
    return readImpl_p->getNAntennas ();
}

//MFrequency::Types
//ROVisibilityIterator2::getObservatoryFrequencyType () const
//{
//    CheckImplementationPointerR ();
//    return readImpl_p->getObservatoryFrequencyType ();
//}

//MPosition
//ROVisibilityIterator2::getObservatoryPosition () const
//{
//    CheckImplementationPointerR ();
//
//    return readImpl_p->getObservatoryPosition ();
//}

//MDirection
//ROVisibilityIterator2::getPhaseCenter () const
//{
//    CheckImplementationPointerR ();
//
//    return readImpl_p->getPhaseCenter ();
//}

VisibilityIteratorReadImpl2 *
ROVisibilityIterator2::getReadImpl () const
{
    return readImpl_p;
}

void
ROVisibilityIterator2::getSpwInFreqRange (Block<Vector<Int> >& spw,
                                         Block<Vector<Int> >& start,
                                         Block<Vector<Int> >& nchan,
                                         Double freqStart,
                                         Double freqEnd,
                                         Double freqStep,
                                         MFrequency::Types freqFrame)
{
    CheckImplementationPointerR ();
    readImpl_p->getSpwInFreqRange (spw, start, nchan, freqStart, freqEnd, freqStep, freqFrame);
}

void ROVisibilityIterator2::getFreqInSpwRange(Double& freqStart, Double& freqEnd, MFrequency::Types freqframe){
  CheckImplementationPointerR ();
  readImpl_p->getFreqInSpwRange(freqStart, freqEnd, freqframe);
}


SubChunkPair
ROVisibilityIterator2::getSubchunkId () const
{
    CheckImplementationPointerR ();
    return readImpl_p->getSubchunkId ();
}

void
ROVisibilityIterator2::getTopoFreqs ()
{
    CheckImplementationPointerR ();
    readImpl_p->getTopoFreqs ();
}

void
ROVisibilityIterator2::getTopoFreqs (Vector<Double> & lsrFreq, Vector<Double> & selFreq)
{
    CheckImplementationPointerR ();
    readImpl_p->getTopoFreqs (lsrFreq, selFreq);
}

VisBuffer *
ROVisibilityIterator2::getVisBuffer ()
{
    CheckImplementationPointerR ();
    return readImpl_p->getVisBuffer ();
}

Double
ROVisibilityIterator2::hourang (Double time) const
{
    CheckImplementationPointerR ();
    return readImpl_p->hourang (time);
}

Double
ROVisibilityIterator2::hourangCalculate (Double time, MSDerivedValues & msd, const MEpoch & mEpoch0)
{
    return VisibilityIteratorReadImpl2::hourangCalculate (time, msd, mEpoch0);
}

//Matrix<Float>&
//ROVisibilityIterator2::imagingWeight (Matrix<Float>& wt) const
//{
//    CheckImplementationPointerR ();
//    return readImpl_p->imagingWeight (wt);
//}

const VisImagingWeight &
ROVisibilityIterator2::getImagingWeightGenerator () const
{
    CheckImplementationPointerR ();
    return readImpl_p->getImagingWeightGenerator ();
}


//Bool
//ROVisibilityIterator2::isAsyncEnabled () const
//{
//    CheckImplementationPointerR ();
//
//    return readImpl_p->isAsyncEnabled ();
//}


Bool
ROVisibilityIterator2::isAsynchronous () const
{
    Bool isAsync = readImpl_p != NULL && dynamic_cast<const ViReadImplAsync2 *> (readImpl_p) != NULL;

    return isAsync;
}


Bool
ROVisibilityIterator2::isAsynchronousIoEnabled()
{
    // Determines whether asynchronous I/O is enabled by looking for the
    // expected AipsRc value.  If not found then async i/o is disabled.

    Bool isEnabled;
    AipsrcValue<Bool>::find (isEnabled, getAipsRcBase () + ".enabled", False);

    return isEnabled;
}

Bool
ROVisibilityIterator2::isInSelectedSPW (const Int& spw)
{
    CheckImplementationPointerR ();
    return readImpl_p->isInSelectedSPW (spw);
}

Bool
ROVisibilityIterator2::isWritable () const
{
    CheckImplementationPointerR ();
    return readImpl_p->isWritable ();
}

Bool
ROVisibilityIterator2::more () const
{
    CheckImplementationPointerR ();
    return readImpl_p->more ();
}

Bool
ROVisibilityIterator2::moreChunks () const
{
    CheckImplementationPointerR ();
    return readImpl_p->moreChunks ();
}

const ROMSColumns &
ROVisibilityIterator2::msColumns () const
{
    CheckImplementationPointerR ();
    return readImpl_p->msColumns ();
}

Int
ROVisibilityIterator2::msId () const
{
    CheckImplementationPointerR ();
    return readImpl_p->msId ();
}

Int
ROVisibilityIterator2::nCorr () const
{
    CheckImplementationPointerR ();
    return readImpl_p->nCorr ();
}

Int
ROVisibilityIterator2::nRow () const
{
    CheckImplementationPointerR ();
    return readImpl_p->nRow ();
}

Int
ROVisibilityIterator2::nRowChunk () const
{
    CheckImplementationPointerR ();
    return readImpl_p->nRowChunk ();
}

Int
ROVisibilityIterator2::nSubInterval () const
{
    CheckImplementationPointerR ();
    return readImpl_p->nSubInterval ();
}

Bool
ROVisibilityIterator2::newArrayId () const
{
    CheckImplementationPointerR ();
    return readImpl_p->newArrayId ();
}

Bool
ROVisibilityIterator2::newFieldId () const
{
    CheckImplementationPointerR ();
    return readImpl_p->newFieldId ();
}

Bool
ROVisibilityIterator2::newMS () const
{
    CheckImplementationPointerR ();
    return readImpl_p->newMS ();
}

Bool
ROVisibilityIterator2::newSpectralWindow () const
{
    CheckImplementationPointerR ();
    return readImpl_p->newSpectralWindow ();
}

ROVisibilityIterator2&
ROVisibilityIterator2::nextChunk ()
{
    CheckImplementationPointerR ();
    readImpl_p->nextChunk ();

    return * this;
}

Int
ROVisibilityIterator2::numberAnt ()
{
    CheckImplementationPointerR ();
    return readImpl_p->numberAnt ();
}

Int
ROVisibilityIterator2::numberCoh ()
{
    CheckImplementationPointerR ();
    return readImpl_p->numberCoh ();
}

Int
ROVisibilityIterator2::numberDDId ()
{
    CheckImplementationPointerR ();
    return readImpl_p->numberDDId ();
}

Int
ROVisibilityIterator2::numberPol ()
{
    CheckImplementationPointerR ();
    return readImpl_p->numberPol ();
}

Int
ROVisibilityIterator2::numberSpw ()
{
    CheckImplementationPointerR ();
    return readImpl_p->numberSpw ();
}

void
ROVisibilityIterator2::observationId (Vector<Int>& obsids) const
{
    CheckImplementationPointerR ();
    readImpl_p->observationId (obsids);
}

void
ROVisibilityIterator2::origin ()
{
    CheckImplementationPointerR ();
    readImpl_p->origin ();
}

void
ROVisibilityIterator2::originChunks ()
{
    CheckImplementationPointerR ();
    readImpl_p->originChunks ();
}

void
ROVisibilityIterator2::originChunks (Bool forceRewind)
{
    CheckImplementationPointerR ();
    readImpl_p->originChunks (forceRewind);
}

Vector<Float>
ROVisibilityIterator2::parang (Double time) const
{
    CheckImplementationPointerR ();
    return readImpl_p->parang (time);
}

const Float&
ROVisibilityIterator2::parang0 (Double time) const
{
    CheckImplementationPointerR ();
    return readImpl_p->parang0 (time);
}


Float
ROVisibilityIterator2::parang0Calculate (Double time, MSDerivedValues & msd, const MEpoch & epoch0)
{
    return VisibilityIteratorReadImpl2::parang0Calculate (time, msd, epoch0);
}

Vector<Float>
ROVisibilityIterator2::parangCalculate (Double time, MSDerivedValues & msd,
                                       int nAntennas, const MEpoch mEpoch0)
{
    return VisibilityIteratorReadImpl2::parangCalculate (time, msd, nAntennas, mEpoch0);
}

const MDirection&
ROVisibilityIterator2::phaseCenter () const
{
    CheckImplementationPointerR ();
    return readImpl_p->phaseCenter ();
}

Int
ROVisibilityIterator2::polFrame () const
{
    CheckImplementationPointerR ();
    return readImpl_p->polFrame ();
}

Int
ROVisibilityIterator2::polarizationId () const
{
    CheckImplementationPointerR ();
    return readImpl_p->polarizationId ();
}

void
ROVisibilityIterator2::processorId (Vector<Int>& procids) const
{
    CheckImplementationPointerR ();
    readImpl_p->processorId (procids);
}

const Cube<Double>&
ROVisibilityIterator2::receptorAngles () const
{
    CheckImplementationPointerR ();
    return readImpl_p->receptorAngles ();
}

Vector<Float>
ROVisibilityIterator2::getReceptor0Angle ()
{
    CheckImplementationPointerR ();
    return readImpl_p->getReceptor0Angle();
}

//Vector<uInt> getRowIds ();

void
ROVisibilityIterator2::rowIds (Vector<uInt>& rowids) const
{
    CheckImplementationPointerR ();
    readImpl_p->rowIds (rowids);
}

Vector<uInt>
ROVisibilityIterator2::getRowIds() const
{
    CheckImplementationPointerR();
    return readImpl_p->getRowIds();
}

void
ROVisibilityIterator2::scan (Vector<Int>& scans) const
{
    CheckImplementationPointerR ();
    readImpl_p->scan (scans);
}

ROVisibilityIterator2&
ROVisibilityIterator2::selectChannel (Int nGroup,
                                     Int start,
                                     Int width,
                                     Int increment,
                                     Int spectralWindow)
{
    CheckImplementationPointerR ();
    readImpl_p->selectChannel (nGroup, start, width, increment, spectralWindow);
    readImpl_p->originChunks ();

    return * this;
}

ROVisibilityIterator2&
ROVisibilityIterator2::selectChannel (Block< Vector<Int> >& blockNGroup,
                                     Block< Vector<Int> >& blockStart,
                                     Block< Vector<Int> >& blockWidth,
                                     Block< Vector<Int> >& blockIncr,
                                     Block< Vector<Int> >& blockSpw)
{
    CheckImplementationPointerR ();
    readImpl_p->selectChannel (blockNGroup, blockStart, blockWidth, blockIncr, blockSpw);

    return * this;
}

ROVisibilityIterator2&
ROVisibilityIterator2::selectVelocity (Int nChan,
                                      const MVRadialVelocity& vStart,
                                      const MVRadialVelocity& vInc,
                                      MRadialVelocity::Types rvType,
                                      MDoppler::Types dType,
                                      Bool precise)
{
    CheckImplementationPointerR ();

    readImpl_p->selectVelocity (nChan, vStart, vInc, rvType, dType, precise);

    return * this;
}

void
ROVisibilityIterator2::setAsyncEnabled (Bool enable)
{
    CheckImplementationPointerR ();
    readImpl_p->setAsyncEnabled (enable);
}

void
ROVisibilityIterator2::setFrequencySelection (const FrequencySelection & selection)
{
    FrequencySelections selections;
    selections.add (selection);
    setFrequencySelection (selections);
}

void
ROVisibilityIterator2::setFrequencySelection (const FrequencySelections & selection)
{
    ThrowIf (selection.size () != nMS_p,
             utilj::format ("Frequency selection size, %d, does not VisibilityIterator # of MSs, %d.",
                     nMS_p, selection.size()));

    CheckImplementationPointerR ();
    readImpl_p->setFrequencySelectin (selection);
}


Double ROVisibilityIterator2::getInterval() const
{
    CheckImplementationPointerR ();
    return readImpl_p->getInterval();
}

void
ROVisibilityIterator2::setInterval (Double timeInterval)
{
    CheckImplementationPointerR ();
    readImpl_p->setInterval (timeInterval);
}

void
ROVisibilityIterator2::setRowBlocking (Int nRows) // for use by Async I/O *ONLY
{
    CheckImplementationPointerR ();
    readImpl_p->setRowBlocking (nRows);
}

void
ROVisibilityIterator2::setSelTable ()
{
    CheckImplementationPointerR ();
    readImpl_p->setSelTable ();
}

void
ROVisibilityIterator2::setState ()
{
    CheckImplementationPointerR ();
    readImpl_p->setState ();
}

void
ROVisibilityIterator2::setTileCache ()
{
    CheckImplementationPointerR ();
    readImpl_p->setTileCache ();
}

void
ROVisibilityIterator2::sigma (Vector<Float>& sig) const
{
    CheckImplementationPointerR ();
    readImpl_p->sigma (sig);
}

void
ROVisibilityIterator2::sigmaMat (Matrix<Float>& sigmat) const
{
    CheckImplementationPointerR ();
    readImpl_p->sigmaMat (sigmat);
}

void
ROVisibilityIterator2::slicesToMatrices (Vector<Matrix<Int> >& matv,
                                        const Vector<Vector<Slice> >& slicesv,
                                        const Vector<Int>& widthsv) const
{
    CheckImplementationPointerR ();
    readImpl_p->slicesToMatrices (matv, slicesv, widthsv);
}

void
ROVisibilityIterator2::slurp () const
{
    CheckImplementationPointerR ();
    readImpl_p->slurp ();
}

String
ROVisibilityIterator2::sourceName () const
{
    CheckImplementationPointerR ();
    return readImpl_p->sourceName ();
}

Int
ROVisibilityIterator2::spectralWindow () const
{
    CheckImplementationPointerR ();
    return readImpl_p->spectralWindow ();
}

void
ROVisibilityIterator2::stateId (Vector<Int>& stateids) const
{
    CheckImplementationPointerR ();
    readImpl_p->stateId (stateids);
}

void
ROVisibilityIterator2::time (Vector<Double>& t) const
{
    CheckImplementationPointerR ();
    readImpl_p->time (t);
}

void
ROVisibilityIterator2::timeCentroid (Vector<Double>& t) const
{
    CheckImplementationPointerR ();
    readImpl_p->timeCentroid (t);
}

void
ROVisibilityIterator2::timeInterval (Vector<Double>& ti) const
{
    CheckImplementationPointerR ();
    readImpl_p->timeInterval (ti);
}

void
ROVisibilityIterator2::updateSlicer ()
{
    CheckImplementationPointerR ();
    readImpl_p->updateSlicer ();
}

void
ROVisibilityIterator2::update_rowIds () const
{
    CheckImplementationPointerR ();
    readImpl_p->update_rowIds ();
}

void
ROVisibilityIterator2::useImagingWeight (const VisImagingWeight& imWgt)
{
    CheckImplementationPointerR ();
    readImpl_p->useImagingWeight (imWgt);
}

void
ROVisibilityIterator2::uvw (Vector<RigidVector<Double, 3> >& uvwvec) const
{
    CheckImplementationPointerR ();
    readImpl_p->uvw (uvwvec);
}

void
ROVisibilityIterator2::uvwMat (Matrix<Double>& uvwmat) const
{
    CheckImplementationPointerR ();
    readImpl_p->uvwMat (uvwmat);
}

ROVisibilityIterator2&
ROVisibilityIterator2::velInterpolation (const String& type)
{
    CheckImplementationPointerR ();

    readImpl_p->velInterpolation (type);

    return * this;
}

void
ROVisibilityIterator2::visibility (Cube<Complex>& vis,
                                  DataColumn whichOne) const
{
    CheckImplementationPointerR ();
    readImpl_p->visibility (vis, whichOne);
}

void
ROVisibilityIterator2::visibility (Matrix<CStokesVector>& vis,
                                  DataColumn whichOne) const
{
    CheckImplementationPointerR ();
    readImpl_p->visibility (vis, whichOne);
}

IPosition
ROVisibilityIterator2::visibilityShape () const
{
    CheckImplementationPointerR ();
    return readImpl_p->visibilityShape ();
}

void
ROVisibilityIterator2::weight (Vector<Float>& wt) const
{
    CheckImplementationPointerR ();
    readImpl_p->weight (wt);
}

void
ROVisibilityIterator2::weightMat (Matrix<Float>& wtmat) const
{
    CheckImplementationPointerR ();
    readImpl_p->weightMat (wtmat);
}

void
ROVisibilityIterator2::weightSpectrum (Cube<Float>& wtsp) const
{
    CheckImplementationPointerR ();
    readImpl_p->weightSpectrum (wtsp);
}



VisibilityIterator2::VisibilityIterator2 ()
: ROVisibilityIterator2 (),
  writeImpl_p (NULL)
{}

VisibilityIterator2::VisibilityIterator2 (MeasurementSet & ms,
                                        const Block<Int>& sortColumns,
                                        Double timeInterval)
: ROVisibilityIterator2 (NULL, Block<MeasurementSet> (1, ms), sortColumns,
                        True, timeInterval, True)

{
    construct ();
}

VisibilityIterator2::VisibilityIterator2 (MeasurementSet & ms,
                                        const Block<Int>& sortColumns,
                                        const Bool addDefaultSortCols,
                                        Double timeInterval)
: ROVisibilityIterator2 (NULL, Block<MeasurementSet> (1, ms), sortColumns,
                        addDefaultSortCols, timeInterval, True)
{
    construct ();
}

VisibilityIterator2::VisibilityIterator2 (const Block<MeasurementSet>& mss,
                                        const Block<Int>& sortColumns,
                                        Double timeInterval)
: ROVisibilityIterator2 (NULL, mss, sortColumns, True, timeInterval, True)
{
    construct ();
}

VisibilityIterator2::VisibilityIterator2 (const Block<MeasurementSet>& mss,
                                        const Block<Int>& sortColumns,
                                        const Bool addDefaultSortCols,
                                        Double timeInterval)
: ROVisibilityIterator2 (NULL, mss, sortColumns, addDefaultSortCols, timeInterval, True)
{
    construct ();
}

VisibilityIterator2::VisibilityIterator2 (const asyncio::PrefetchColumns * prefetchColumns,
                                        MeasurementSet & ms,
                                        const Block<Int>& sortColumns,
                                        const Bool addDefaultSortCols,
                                        Double timeInterval)
: ROVisibilityIterator2 (prefetchColumns, Block<MeasurementSet> (1, ms), sortColumns,
                        addDefaultSortCols, timeInterval, True)
{
    construct ();
}


VisibilityIterator2::VisibilityIterator2 (const asyncio::PrefetchColumns * prefetchColumns,
                                        const Block<MeasurementSet>& mss,
                                        const Block<Int>& sortColumns,
                                        const Bool addDefaultSortCols,
                                        Double timeInterval)
: ROVisibilityIterator2 (prefetchColumns, mss, sortColumns,
                        addDefaultSortCols, timeInterval, True)
{
    construct ();
}


VisibilityIterator2::VisibilityIterator2 (const VisibilityIterator2 & other)
: ROVisibilityIterator2 (other)
{
    if (other.writeImpl_p == NULL){
        writeImpl_p = NULL;
    }
    else{
        writeImpl_p = other.writeImpl_p->clone(this);
    }
}

VisibilityIterator2::VisibilityIterator2 (const asyncio::PrefetchColumns * prefetchColumns,
                                          const VisibilityIterator2 & other)
: ROVisibilityIterator2 (prefetchColumns, other)
{
    Bool createAsAsynchronous = prefetchColumns != NULL && isAsynchronousIoEnabled ();

    if (createAsAsynchronous){
        writeImpl_p = new ViWriteImplAsync2 (* prefetchColumns, * other.writeImpl_p, this);
    }
    else{
        writeImpl_p = other.writeImpl_p -> clone(this);
    }
}


VisibilityIterator2::~VisibilityIterator2 ()
{
    delete writeImpl_p;
}

VisibilityIterator2 &
VisibilityIterator2::operator= (const VisibilityIterator2 & other)
{
    // Let the superclass handle its part of the assignment

    ROVisibilityIterator2::operator= (other);

    // Kill off the current write implementation

    if (writeImpl_p != NULL){
        delete writeImpl_p;
        writeImpl_p = NULL;
    }

    // If the other's read implementation is not null then
    // put a clone of it into this object's read implementation.

    if (other.writeImpl_p != NULL){

        writeImpl_p = other.writeImpl_p->clone (this);
    }
    return * this;
}

VisibilityIterator2 &
VisibilityIterator2::operator++ (int)
{
    advance ();

    return * this;
}

VisibilityIterator2 &
VisibilityIterator2::operator++ ()
{
    advance();

    return * this;
}

void
VisibilityIterator2::attachColumns (const Table &t)
{
    CheckImplementationPointerW ();

    writeImpl_p->attachColumns (t);
}

void
VisibilityIterator2::construct ()
{
    if (isAsynchronous ()){
        writeImpl_p = new ViWriteImplAsync2 (this);
    }
    else{
        writeImpl_p = new VisibilityIteratorWriteImpl2 (this);
    }

    /////////////////  readImpl_p->originChunks();
}

VisibilityIteratorWriteImpl2 *
VisibilityIterator2::getWriteImpl () const
{
    return writeImpl_p;
}

Bool
VisibilityIterator2::isWritable () const
{
    return True;
}

void
VisibilityIterator2::setFlag (const Matrix<Bool>& flag)
{
    CheckImplementationPointerW ();
    writeImpl_p->setFlag (flag);
}

void
VisibilityIterator2::setFlag (const Cube<Bool>& flag)
{
    CheckImplementationPointerW ();
    writeImpl_p->setFlag (flag);
}
void
VisibilityIterator2::setFlagCategory(const Array<Bool>& flagCategory)
{
    CheckImplementationPointerW ();
    writeImpl_p->setFlagCategory (flagCategory);
}

void
VisibilityIterator2::setFlagRow (const Vector<Bool>& rowflags)
{
    CheckImplementationPointerW ();
    writeImpl_p->setFlagRow (rowflags);
}

void
VisibilityIterator2::setSigma (const Vector<Float>& sig)
{
    CheckImplementationPointerW ();
    writeImpl_p->setSigma (sig);
}

void
VisibilityIterator2::setSigmaMat (const Matrix<Float>& sigmat)
{
    CheckImplementationPointerW ();
    writeImpl_p->setSigmaMat (sigmat);
}

void
VisibilityIterator2::setVis (const Matrix<CStokesVector>& vis, DataColumn whichOne)
{
    CheckImplementationPointerW ();
    writeImpl_p->setVis (vis, whichOne);
}

void
VisibilityIterator2::setVis (const Cube<Complex>& vis, DataColumn whichOne)
{
    CheckImplementationPointerW ();
    writeImpl_p->setVis (vis, whichOne);
}

void
VisibilityIterator2::setVisAndFlag (const Cube<Complex>& vis, const Cube<Bool>& flag,
                                   DataColumn whichOne)
{
    CheckImplementationPointerW ();
    writeImpl_p->setVisAndFlag (vis, flag, whichOne);
}

void
VisibilityIterator2::setWeight (const Vector<Float>& wt)
{
    CheckImplementationPointerW ();
    writeImpl_p->setWeight (wt);
}

void
VisibilityIterator2::setWeightMat (const Matrix<Float>& wtmat)
{
    CheckImplementationPointerW ();
    writeImpl_p->setWeightMat (wtmat);
}

void
VisibilityIterator2::setWeightSpectrum (const Cube<Float>& wtsp)
{
    CheckImplementationPointerW ();
    writeImpl_p->setWeightSpectrum (wtsp);
}

void VisibilityIterator2::putModel(const RecordInterface& rec, Bool iscomponentlist, Bool incremental){
  CheckImplementationPointerW ();
  writeImpl_p->putModel(rec, iscomponentlist, incremental);

}



void VisibilityIterator2::writeBack (VisBuffer * vb)
{
    CheckImplementationPointerW ();
    writeImpl_p->writeBack (vb);
}

} // end namespace casa

