#include <casa/Arrays/ArrayMath.h>
#include <casa/BasicMath/Functors.h>
#include <synthesis/MSVis/AveragingTvi2.h>
#include <synthesis/MSVis/VisBufferImpl2.h>
#include <synthesis/MSVis/UtilJ.h>

namespace casa {

namespace vi {

namespace avg {

///////////////////////////////////////////////////////////
//
// Code to provide interface to weight function

class WeightFunctionBase {
public:

    virtual ~WeightFunctionBase () {}
    Float operator() (Float f) { return apply (f);}
    virtual Float apply (Float) = 0;
};

template<typename F>
class WeightFunction : public WeightFunctionBase {
public:

    // Provide either a unary function, Float (*) (Float), or
    // a functor class having a Float operator() (Float) method.

    WeightFunction (F f) : function_p (f) {}

    Float apply (Float f) { return function_p (f);}

private:

    F function_p;
};

template<typename F>
WeightFunction<F> * generateWeightFunction (F f) { return new WeightFunction<F> (f);}

Float unity (Float) { return 1.0;}

///////////////////////////////////////////////////////////
//
// Averaging VisBuffer

class VbAvg : public VisBufferImpl2 {

public:

    typedef Float (* WeightFunction) (Float);

    VbAvg (Double averagingInterval, WeightFunctionBase * weightFunction);

    void accumulate (const VisBuffer2 * vb);
    Bool empty () const;
    void finalizeAverage ();
    Bool isComplete () const;
    void markEmpty ();
    void prepareForFirstData (const VisBuffer2 *);
    Bool vbPastAveragingInterval (const VisBuffer2 * vb) const;

protected:

    void accumulateCubeData (const VisBuffer2 * input);
    void accumulateCubeDatum (const VisBuffer2 * vb, Int baselineIndex,
                              Int correlation, Int channel, Int row,
                              Bool clearAccumulation);
    void accumulateExposure (const VisBuffer2 *);
    void accumulateRowData (const VisBuffer2 * input);
    void accumulateTimeCentroid (const VisBuffer2 * input);
    void copyBaseline (Int sourceBaseline, Int targetBaseline);
    template<typename T>
    void copyBaselineScalar (Int sourceBaseline, Int targetBaseline, Vector<T> & columnVector);
    template<typename T>
    void copyCubePlaneIf (Bool condition, Int sourceBaseline, Int targetBaseline, Cube<T> & cube);
    void copyRowIdValues (const VisBuffer2 * input, Int row, Int baselineIndex);
    void copyRowIdValue (const VisBuffer2 * vb,
                         Int vbRow,
                         const Vector<Int> & (VisBuffer2::* getter) () const,
                         void (VisBuffer2::* setter) (const Vector<Int> &),
                         Int baselineIndex);
    void finalizeCubeData ();
    void finalizeRowData ();
    Int getBaselineIndex (Int antenna1, Int antenna2) const;
    Float getWeight (const VisBuffer2 * input, Int correlation, Int channel, Int row) const;
    Int nBaselines () const;
    void prepareIds (const VisBuffer2 * vb);
    void removeMissingBaselines ();
    void setupBaselineIndices (Int nAntennas, const VisBuffer2 * vb);

private:

    Double averagingInterval_p;
    Matrix<Int> baselineIndices_p; // map of antenna1,antenna2 to row number in this VB.
    Bool complete_p; // average is complete
    Cube<Int> counts_p; // number of items summed together for each correlation, channel, baseline item.
    Vector<Int> countsBaseline_p; // number of items summed together for each baseline.
    Bool doingCorrectedData_p;
    Bool doingModelData_p;
    Bool doingWeightSpectrum_p;
    Bool empty_p; // true when buffer hasn't seen any data
    Double startTime_p; // time of the first sample in average
    WeightFunctionBase * weightFunction_p;
    Cube<Float> weightSum_p; // accumulation of weights for weighted averaging
};

///////////////////////////////////////////////////////////
//
// Set of Averaging VisBuffers, one per active DD ID


class VbSet {

public:

    VbSet (Double averagingInterval, WeightFunctionBase * weightFunction);
    ~VbSet ();

    void accumulate (const VisBuffer2 *);
    VbAvg * add (Int ddId);
    Bool anyAveragesReady(Int ddid = -1) const;
    Bool contains (Int ddId) const;
    void finalizeAverage (Int ddId);
    void finalizeAverages ();
    void flush (Bool okIfNonempty = False); // delete all averagers
    Int getFirstReadyDdid () const;
    void transferAverage (Int ddId, VisBuffer2 * vb);
    Bool vbPastAveragingInterval (const VisBuffer2 * vb) const;
    void zero ();

protected:

private:

    typedef map<Int, VbAvg *> Averagers;

    const Double averagingInterval_p;
    Averagers vbAveragers_p;
    WeightFunctionBase * weightFunction_p;
};

VbAvg::VbAvg (Double averagingInterval, WeightFunctionBase * weightFunction)
: averagingInterval_p (averagingInterval),
  complete_p (False),
  doingCorrectedData_p (False),
  doingModelData_p (False),
  empty_p (True),
  weightFunction_p (weightFunction)
{}

void
VbAvg::accumulate (const VisBuffer2 * vb)
{
    if (empty_p){

        prepareForFirstData (vb);

        empty_p = False;
    }

    // Averaging can be computed as flagged or unflagged.  If all the inputs to a
    // datum are flagged, then the averaged datum (e.g., a visibility point)
    // will also be flagged.  For an unflagged averaged datum, it will represent
    // the average of all of the unflagged points for that datum.  This is done
    // by assuming the accumulation is flagged and continuing to accumulate
    // flagged points until the first unflagged point for a datum is encountered;
    // when this happens the count is zeroed and the averaged datum's flag is cleared.

#warning "Implement weight and sigma averaging"

    // Loop over all of the rows in the VB.  Map each one to a baseline and then accumulate
    // the values for each correlation,channel cell.  Each row in the accumulating VB corresponds
    // to one baseline (i.e., pair of (antenna1, antenna2) where antenna1 <= antenna2).

    ThrowIf (vb->nRows() > nBaselines(),
             String::format ("Expected %d baselines in VisBuffer but it contained %d rows",
                             nBaselines(), nRows()));

    // Accumulate data for fields that are scalars (and uvw) in each row

    accumulateRowData (vb);

    // Accumulate data that is matrix-valued (e.g., vis, visModel, etc.)

    accumulateCubeData (vb);
}

void
VbAvg::accumulateCubeData (const VisBuffer2 * vb)
{
    // Accumulate the sums needed for averaging of cube data (e.g., visibility).

    const Vector<Int> & antenna1 = vb->antenna1();
    const Vector<Int> & antenna2 = vb->antenna2();

    for (Int row = 0; row < vb->nRows(); row ++){

        Bool inputRowFlagged = vb->flagRow () (row);

        Int baselineIndex = getBaselineIndex (antenna1 [row], antenna2 [row]);

        for (Int channel = 0; channel < vb->nChannels (); channel ++){

            for (Int correlation = 0; correlation < vb->nCorrelations (); correlation ++){

                // Shortcuts to values; note that two of these are mutable references.

                Bool inputFlagged = vb->flagCube () (correlation, channel, row) || inputRowFlagged;
                Bool & accumulatorFlagged = flagCubeRef () (correlation, channel, baselineIndex);
                Int & count = counts_p (correlation, channel, baselineIndex);

                // Based on the current flag state of the accumulation and the current flag
                // state of the correlation,channel, accumulate the data (or not).  Accumulate
                // flagged data until the first unflagged datum appears.  Then restart the
                // accumulation with that datum.

                if (! accumulatorFlagged && inputFlagged){
                    continue;// good accumulation, bad data so toss it.
                }

                // If changing from flagged to unflagged for this cube element, reset the
                // accumulation count to 1; otherwise increment the count.

                Bool flagChange = accumulatorFlagged != inputFlagged;
                count = flagChange ? 1 : count + 1;
                accumulatorFlagged = accumulatorFlagged && inputFlagged;

                // Accumulate the sum for each cube element

                accumulateCubeDatum (vb, baselineIndex, correlation, channel, row, flagChange);
            }
        }
    }
}

void
VbAvg::accumulateCubeDatum (const VisBuffer2 * input, Int baselineIndex, Int correlation,
                           Int channel, Int row, Bool zeroAccumulation)
{
    // Zero accumulation is used to restart the accumulation when the flagged state of the
    // element's accumulation changes from flagged to unflagged.

    // Get the weight to use for averaging.  If arithmetic averaging is being used the function
    // will return 1.0.  Otherwise if will return some function of the weight or weight-spectrum
    // (e.g., square, square-root, etc.).

    Float weight =  getWeight (input, correlation, channel, row);

    weightSum_p (correlation, channel, row) =
        zeroAccumulation ? weight : (weightSum_p (correlation, channel, row) + weight);

    // Update the sum for this visibility cube element.

    Complex weightedValue = input->visCube () (correlation, channel, row) * weight;
    visCubeRef ()(correlation, channel, baselineIndex) = weightedValue +
        (zeroAccumulation ? 0 : visCube () (correlation, channel, row));

    if (doingCorrectedData_p){

        // Update the sum for this corrected visibility cube element.

        Complex weightedValue = input->visCubeCorrected () (correlation, channel, row) * weight;
        visCubeCorrectedRef ()(correlation, channel, baselineIndex) = weightedValue +
             (zeroAccumulation ? 0 : visCubeCorrected () (correlation, channel, row));
    }

    if (doingModelData_p){

        // Update the sum for this model visibility cube element.

        Complex weightedValue = input->visCubeModel () (correlation, channel, row) * weight;
        visCubeModelRef ()(correlation, channel, baselineIndex) = weightedValue +
            (zeroAccumulation ? 0 : visCubeModel () (correlation, channel, row));
    }
}

void
VbAvg::accumulateExposure (const VisBuffer2 * input)
{
    Vector<Double> exposureSum = exposure ();

    for (Int row = 0; row < input->nRows(); row ++){

        Int a1 = input->antenna1() (row);
        Int a2 = input->antenna2() (row);

        Int baselineIndex = getBaselineIndex (a1, a2);

        exposureSum [baselineIndex] += input->exposure () (row);
    }

    setExposure (exposureSum);
}

void
VbAvg::accumulateRowData (const VisBuffer2 * input)
{
    Vector<Double> exposureSum = exposure ();
    Vector<Double> timeCentroidSum = timeCentroid ();
    Matrix<Double> uvwSum = uvw();

    for (Int row = 0; row < input->nRows(); row ++){

        Int a1 = input->antenna1() (row);
        Int a2 = input->antenna2() (row);

        Int baselineIndex = getBaselineIndex (a1, a2);

        Bool & accumulatorRowFlagged = flagRowRef ()(baselineIndex);
        Bool inputRowFlagged = flagRow () (row);

        copyRowIdValues (input, row, baselineIndex);

        if (! accumulatorRowFlagged && input->flagRow() (row)){
            // good accumulation, bad data --> skip it
        }
        else{

            Bool flagChange = accumulatorRowFlagged != inputRowFlagged;

            Int & count = countsBaseline_p (baselineIndex);
            count = flagChange ? 1: count + 1;

            accumulatorRowFlagged = accumulatorRowFlagged && inputRowFlagged;

            exposureSum [baselineIndex] += input->exposure () (row);
            timeCentroidSum [baselineIndex] += input->timeCentroid () (row);

            for (Int i = 0; i < 3; i++){
                uvwSum (i, baselineIndex) += input->uvw ()(i, row);
            }
        }
    }

    setTimeCentroid (timeCentroidSum);
    setExposure (exposureSum);
    setUvw (uvwSum);
}

void
VbAvg::accumulateTimeCentroid (const VisBuffer2 * input)
{
    Vector<Double> timeCentroidSum = timeCentroid ();

    for (Int row = 0; row < input->nRows(); row ++){

        Int a1 = input->antenna1() (row);
        Int a2 = input->antenna2() (row);

        Int baselineIndex = getBaselineIndex(a1, a2);

        timeCentroidSum [baselineIndex] += input->timeCentroid () (row);
    }

    setTimeCentroid (timeCentroidSum);
}

void
VbAvg::copyRowIdValues (const VisBuffer2 * input, Int row, Int baselineIndex)
{
    copyRowIdValue (input, row, & VisBuffer2::antenna1, & VisBuffer2::setAntenna1, baselineIndex);
    copyRowIdValue (input, row, & VisBuffer2::antenna2, & VisBuffer2::setAntenna2, baselineIndex);
    copyRowIdValue (input, row, & VisBuffer2::dataDescriptionIds, & VisBuffer2::setDataDescriptionIds, baselineIndex);
    copyRowIdValue (input, row, & VisBuffer2::feed1, & VisBuffer2::setFeed1, baselineIndex);
    copyRowIdValue (input, row, & VisBuffer2::feed1, & VisBuffer2::setFeed1, baselineIndex);
    copyRowIdValue (input, row, & VisBuffer2::processorId, & VisBuffer2::setProcessorId, baselineIndex);
    copyRowIdValue (input, row, & VisBuffer2::scan, & VisBuffer2::setScan, baselineIndex);
    copyRowIdValue (input, row, & VisBuffer2::observationId, & VisBuffer2::setObservationId, baselineIndex);
    copyRowIdValue (input, row, & VisBuffer2::stateId, & VisBuffer2::setStateId, baselineIndex);
}

void
VbAvg::copyRowIdValue (const VisBuffer2 * vb,
                       Int vbRow,
                       const Vector<Int> & (VisBuffer2::* getter) () const,
                       void (VisBuffer2::* setter) (const Vector<Int> &),
                       Int baselineIndex)
{
    // Get the current value of the ID for this baseline index

    Int thisValue = (this ->* getter) () (baselineIndex);

    if (thisValue < 0){

        // It's invalid so get the values, install the new element
        // and replace the entire new values.

        Vector<Int> values = (this ->* getter) ();

        values (baselineIndex) = (vb ->* getter) () (vbRow);

        (this ->* setter) (values);
    }
}


Bool
VbAvg::empty () const
{
    return empty_p;
}

Int
VbAvg::getBaselineIndex (Int antenna1, Int antenna2) const
{
    // Lookup the baseline index using the prebuilt lookup table.
    //
    // The baseline index is the index in the sequence
    // {(0,0),(1,0),(1,1),(2,0),(2,1),(2,2), ...} (i.e., the index in a
    // 1-d representation of the lower right half of the square matrix of size
    // nAntennas).
    //
    // This handles the case where the baseline ordering in the input VB might
    // shift from VB to VB.

    Int index = baselineIndices_p (antenna1, antenna2);

    ThrowIf (index < 0, String::format ("Unexpected baseline having antenna pair (%d,%d)",
                                         antenna1, antenna2));
        // We expect antenna1 <= antenna2 in CASA-filled vis data

    return index;
}

void
VbAvg::finalizeAverage ()
{
    finalizeCubeData ();

    finalizeRowData ();

    removeMissingBaselines ();

    complete_p = True;
}

void
VbAvg::finalizeCubeData ()
{
    // Divide each of the data cubes in use by the sum of the weights.

    typedef Divides <Complex, Float, Complex> DivideOp;
    DivideOp op;
    arrayTransformInPlace<Complex, Float, DivideOp > (visCubeRef(), weightSum_p, op);

    if (doingCorrectedData_p){
        arrayTransformInPlace<Complex, Float, DivideOp > (visCubeCorrectedRef(), weightSum_p, op);
    }

    if (doingModelData_p){
        arrayTransformInPlace<Complex, Float, DivideOp > (visCubeModelRef (), weightSum_p, op);
    }

    if (doingWeightSpectrum_p){
        setWeightSpectrum (weightSum_p);
    }
}

void
VbAvg::finalizeRowData ()
{
    Vector<Double> exposureAvg = exposure ();
    Vector<Double> timeCentroidAvg = timeCentroid ();
    Matrix<Double> uvwAvg = uvw();

    for (Int baseline = 0; baseline < nBaselines(); baseline ++){

        Int n = countsBaseline_p (baseline);

        if (n != 0){
            exposureAvg (baseline) /= n;
            timeCentroidAvg (baseline) /= n;

            for (Int i = 0; i < 3; i ++){
                uvwAvg (i, baseline) /= n;
            }
        }
    }

    setTimeCentroid (timeCentroidAvg);
    setExposure (exposureAvg);
    setUvw (uvwAvg);

    // Fill in the time and the interval

    Vector<Double> x (nBaselines(), startTime_p);
    setTime (x);

    x = averagingInterval_p;
    setTimeInterval (x);
}

Float
VbAvg::getWeight (const VisBuffer2 * input, Int correlation, Int channel, Int row) const
{
    // Determine how to get the appropriate weight

    Float inputWeight = 1;

    Float outputWeight = (* weightFunction_p) (inputWeight);

    return outputWeight;
}

Bool
VbAvg::isComplete () const
{
    return complete_p;
}

void
VbAvg::markEmpty ()
{
    empty_p = True;
    complete_p = False;
}

Int
VbAvg::nBaselines () const
{
    return countsBaseline_p.nelements();
}

void
VbAvg::prepareForFirstData (const VisBuffer2 * vb)
{
    startTime_p = vb->time() (0);

    Int nAntennas = vb->nAntennas();
    Int nBaselines = (nAntennas * (nAntennas + 1)) / 2;

    // Size and zero out the counters

    countsBaseline_p = Vector<Int> (nBaselines, 0);
    counts_p = Cube<Int> (vb->nCorrelations(), vb->nChannels(), nBaselines, 0);

    weightSum_p.resize (IPosition (3, vb->nCorrelations(), vb->nChannels(), nBaselines));

    setupBaselineIndices (nAntennas, vb);

    // Reshape the inherited members from VisBuffer2

    setShape (vb->nCorrelations(), vb->nChannels(), nBaselines);

    VisBufferComponents2 exclusions =
        VisBufferComponents2::these(VisibilityObserved, VisibilityCorrected,
                                    VisibilityModel, CorrType, JonesC, Unknown);
    cacheResizeAndZero(exclusions);

    prepareIds (vb);

    // Flag everything to start with

    setFlagCube (Cube<Bool> (vb->nCorrelations(), vb->nChannels(), nBaselines, True));
    setFlagRow (Vector<Bool> (nBaselines, True));

    complete_p = False;
}

void
VbAvg::prepareIds (const VisBuffer2 * vb)
{
    // Set these row ID values to indicate they are unknown

    Vector<Int> minusOne (nRows(), -1);

    setAntenna1 (minusOne);
    setAntenna2 (minusOne);
    setDataDescriptionIds (minusOne);
    setFeed1 (minusOne);
    setFeed2 (minusOne);
    setProcessorId (minusOne);
    setScan (minusOne);
    setObservationId (minusOne);
    setStateId (minusOne);

    // Copy the value from the input VB

    setArrayId (vb->arrayId());
    setDataDescriptionId (vb->dataDescriptionId());
    setFieldId (vb->fieldId());
}

void
VbAvg::removeMissingBaselines ()
{
    // Some baselines may not be present in the portion of the input data
    // that made up this average.  However, this is not known until after
    // all of the data is seen.  Thus at finalization time these bogus
    // baselines should be removed from the average so as not to pass
    // flagged but zero-exposure baselines into the output.


    Vector<Int> rowsToDelete (nBaselines());

    Int nBaselinesDeleted = 0;

    for (Int baseline = 0; baseline < nBaselines(); baseline ++){

        if (countsBaseline_p (baseline) == 0){
            rowsToDelete (nBaselinesDeleted) = baseline;
            nBaselinesDeleted ++;
        }
    }

    rowsToDelete.resize (nBaselinesDeleted, True);

    deleteRows (rowsToDelete);
}


void
VbAvg::setupBaselineIndices(int nAntennas, const VisBuffer2 * vb)
{
    // Create a square matrix to hold the mappings between pairs of
    // antennas to the baseline number.  This includes autocorrelations.
    // Only the lower left half of the matrix holds valid baseline indices;
    // the upper right matrix will hold -1s.
    //
    // The decoding routine, getBaselineIndex will throw an exception if
    // an antenna1,antenna2 pair maps would return a -1.

    baselineIndices_p.resize (nAntennas, nAntennas, False);
    baselineIndices_p = -1;
    Int baseline = 0;

    // Use the baseline order of this VisBuffer2 to set up the default mapping
    // from VB baselines to averaged baselines; unless this is a partial VB
    // then this should be optimum and if not, it only applies for the current
    // average.  If the first vb is normal then it should eliminate the need to
    // copy-down the rows of the VisBuffer for baselines that are possible but
    // not present.
    //
    // Also be sure that multiple occurrences of the same baseline do not fould
    // things up.

    for (Int vbRow = 0; vbRow < vb->nRows(); vbRow ++){

        Int a1 = vb->antenna1()(vbRow);
        Int a2 = vb->antenna2()(vbRow);

        if (a2 <= a1 && baselineIndices_p (a1, a2) < 0){
            baselineIndices_p (a1, a2) = baseline ++;
        }

    }

    // Now put a mapping in for any baselines that weren't in the current VB
    // just in case they appear later.  These will be at the end so won't
    // require shifting data down if they are not present.

    for (Int row = 0; row < nAntennas; row ++){

        for (Int column = 0; column <= row; column ++){

            if (baselineIndices_p (row, column) < 0){
                baselineIndices_p (row, column) = baseline ++;
            }
        }
    }
}

Bool
VbAvg::vbPastAveragingInterval (const VisBuffer2 * vb) const
{
    // See if the timestamp of this VB is after the time interval
    // currently being averaged.

    Double t = vb->time()(0);

    Bool isPast = ! empty_p && t >= (startTime_p + averagingInterval_p);

    return isPast;
}

VbSet::VbSet (Double averagingInterval, WeightFunctionBase * weightFunction)
: averagingInterval_p (averagingInterval),
  weightFunction_p (weightFunction)
{
    if (weightFunction_p == 0){

        weightFunction_p = generateWeightFunction (unity);
    }
}

VbSet::~VbSet ()
{
    flush (True); // allow killing nonempty buffers
    delete weightFunction_p;
}

void
VbSet::accumulate (const VisBuffer2 * input)
{
    Int ddId = input->dataDescriptionId();

    VbAvg * vba = vbAveragers_p [ddId];

    if (vba == 0){ // Need a new averager
        vba = add (ddId);
    }

    vba->accumulate (input);
}

VbAvg *
VbSet::add (Int ddId)
{
    VbAvg * newAverager =  new VbAvg (averagingInterval_p, weightFunction_p);

    vbAveragers_p [ddId] = newAverager;

    return newAverager;
}

Bool
VbSet::anyAveragesReady(Int ddid) const
{
    Bool any = False;

    for (Averagers::const_iterator a = vbAveragers_p.begin();
         a != vbAveragers_p.end();
         a++){

        if (a->second->isComplete() &&
            (ddid <= 0 || ddid == a->second->dataDescriptionId())){

            any = True;
            break;
        }
    }

    return any;
}

Bool
VbSet::contains (Int ddId) const
{
    return vbAveragers_p.find (ddId) != vbAveragers_p.end();
}

void
VbSet::finalizeAverage (Int ddId)
{
    vbAveragers_p [ddId]->finalizeAverage();
}

void
VbSet::finalizeAverages ()
{
    for (Averagers::iterator a = vbAveragers_p.begin();
         a != vbAveragers_p.end();
         a ++){

        finalizeAverage (a->first);
    }
}

void
VbSet::flush (Bool okIfNonempty)
{
    // Get rid of all of the averagers.

    for (Averagers::const_iterator a = vbAveragers_p.begin();
         a != vbAveragers_p.end();
         a ++){

        Assert (okIfNonempty || (a->second)->empty());
            // should have been emptied before calling this

        delete a->second;
    }
}

Int
VbSet::getFirstReadyDdid () const
{
    for (Averagers::const_iterator a = vbAveragers_p.begin();
         a != vbAveragers_p.end();
         a ++){

        if (a->second->isComplete()){
            return a->first;
        }
    }

    Assert (False); // ought to have been one that's ready

    return -1; // shouldn't be called
}

void
VbSet::transferAverage (Int ddId, VisBuffer2 * vb)
{
    VbAvg * vba = vbAveragers_p [ddId];

    Assert (vba != 0 && ! vba->empty ());

    // Copy the completed average into the provided VisBuffer

    vb->copy (* vba, False);

    vba->markEmpty ();
}

Bool
VbSet::vbPastAveragingInterval (const VisBuffer2 * vb) const
{
    Int ddId = vb->dataDescriptionId();

    Bool isPast = False;

    if (contains (ddId)){

        Averagers::const_iterator averager = vbAveragers_p.find (ddId);

        isPast = averager->second->vbPastAveragingInterval (vb);
    }

    return isPast;
}

void
VbSet::zero ()
{
    for (Averagers::const_iterator a = vbAveragers_p.begin();
         a != vbAveragers_p.end();
         a ++){

        a->second->markEmpty();
    }
}

    ///////////////////////////
    //                       //
    // End Namespace avg //
    //                       //
    ///////////////////////////

} // end unnamed avg

using namespace avg;

AveragingTvi2::AveragingTvi2 (ViImplementation2 * inputVi, Double averagingInterval,
                              Int nAveragesPerChunk, WeightFunctionBase * weightFunction)
: TransformingVi2 (inputVi),
  averagingInterval_p (averagingInterval),
  ddidLastUsed_p (-1),
  inputViiAdvanced_p (False),
  nAveragesPerChunk_p (nAveragesPerChunk),
  vbSet_p (new VbSet (averagingInterval, weightFunction))
{
    validateInputVi (inputVi);

    // Position input Vi to the first subchunk

    getVii()->originChunks();
    getVii()->origin ();

    setVisBuffer (new VisBufferImpl2 ());
}

AveragingTvi2::~AveragingTvi2 ()
{
    delete vbSet_p;
}

void
AveragingTvi2::advanceInputVii ()
{
    Assert (False);

    // Attempt to advance to the next subchunk

    getVii()->next ();

    if (! getVii()->more()){

        // No more subchunks so advance to the next chunk

        getVii()->nextChunk();

        if (! getVii()->moreChunks()){
            return; // no more chunks
        }

        // Position to the first subchunk

        getVii()->origin();
    }
}

Int
AveragingTvi2::determineDdidToUse() const
{
    if (ddidLastUsed_p >= 0 && vbSet_p->anyAveragesReady (ddidLastUsed_p)){
        return ddidLastUsed_p;
    }

    return vbSet_p->getFirstReadyDdid();
}

Bool
AveragingTvi2::more () const
{
    return subchunkExists_p;
}

Bool
AveragingTvi2::moreChunks () const
{
    return getVii()->moreChunks();
}

void
AveragingTvi2::next ()
{
    subchunkExists_p = False;

    produceSubchunk ();
}

void
AveragingTvi2::nextChunk ()
{
    // New chunk, so toss all of the accumulators

    vbSet_p->flush (True);

    // Advance the input to the next chunk as well.

    getVii()->nextChunk ();

    subchunkExists_p = False;
}

void
AveragingTvi2::origin ()
{
    // Position input VI to the start of the chunk

    getVii()->origin();
    inputViiAdvanced_p = True;
    subchunkExists_p = False;

    // Zero out the accumulators

    vbSet_p->zero();

    // Get the first subchunk ready.

    produceSubchunk ();
}

void
AveragingTvi2::originChunks ()
{
    vbSet_p->flush (True);
    getVii()->originChunks();
    subchunkExists_p = False;
}

void
AveragingTvi2::processInputSubchunk (const VisBuffer2 * vb)
{
    vbSet_p->accumulate (vb);
}

void
AveragingTvi2::produceSubchunk ()
{
    Bool needMoreInputData = ! subchunksReady();

    while (needMoreInputData){

        if (! inputViiAdvanced_p){

            getVii()->next();
            inputViiAdvanced_p = getVii()->more();
        }

        if (! getVii()->more()){

            vbSet_p->finalizeAverages ();

            break;
        }

        const VisBuffer2 * vb = getVii()->getVisBuffer();

        if (vbSet_p->vbPastAveragingInterval(vb)){

            vbSet_p->finalizeAverage (vb->dataDescriptionId());

            break; // should have an output subchunk now
        }

        processInputSubchunk (vb);

        needMoreInputData = ! subchunksReady();

        inputViiAdvanced_p = False;
    }

    if (subchunksReady()){

        // Transfer the completed average into the VisBuffer

        Int ddId = determineDdidToUse ();

        vbSet_p->transferAverage(ddId, getVisBuffer());

        ddidLastUsed_p = ddId;

        subchunkExists_p = True;
    }
}

Bool
AveragingTvi2::reachedAveragingBoundary()
{
    // An average can be terminated for a variety of reasons:
    // o the time interval has elapsed
    // o the current MS is completed
    // o no more input data
    // o other (e.g, change of scan, etc.)

    Bool reachedIt = False;
    VisBuffer2 * vb = getVii()->getVisBuffer();

    if (! getVii()->more()  && ! getVii ()->moreChunks()){

        reachedIt = True; // no more input data
    }
    else if (vb->isNewMs()){
        reachedIt = True; // new MS
    }

    return reachedIt;

}

Bool
AveragingTvi2::subchunksReady() const
{
    Bool ready = vbSet_p->anyAveragesReady();

    return ready;
}

void
AveragingTvi2::validateInputVi (ViImplementation2 *)
{
    // Validate that the input VI is compatible with this VI.
#warning "Implement AveragingTvi2::validateInputVi"
}

} // end namespace vi

} // end namespace casa
