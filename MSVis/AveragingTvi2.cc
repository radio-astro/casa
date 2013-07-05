#include <casa/Arrays/ArrayMath.h>
#include <casa/BasicMath/Functors.h>
#include <synthesis/MSVis/AveragingTvi2.h>
#include <synthesis/MSVis/VisBufferComponents2.h>
#include <synthesis/MSVis/VisBuffer2.h>
#include <synthesis/MSVis/VisBufferImpl2.h>
#include <synthesis/MSVis/UtilJ.h>
#include <synthesis/MSVis/VisibilityIterator2.h>

namespace casa {

namespace vi {

namespace avg {


///////////////////////////////////////////////////////////
//
// VbAvg: Averaging VisBuffer
//
/*

Definition: A baseline sample (baseline for short) is a table row
with a particular (antenna1, antenna2) pair at a particular time.


Averaging does not cross chunk boundaries of the input VI so the
input VI determines what values are averaged together.  For example,
if the input VI is allows data from multiple scans into the same chunk
then some averaging across scans can occur; in this case the scan number
of the result will be the scan number of the first baseline instance
seen.

Row-level value treatment
=========================

The average is approximately computed on a per baseline basis:

averaged_baseline (antenna1, antenna2) =
    sumOverAveragingInterval (baseline (antenna1, antenna2)) /
    nBaselinesFoundInInterval

How row-level values are computed
---------------------------------

Time - Set to time of first baseline making up the average plus
       half of the averaging interval.
Antenna1 - Copied from instance of baseline
Antenna2 - Copied from instance of baseline
Feed1 - Copied from instance of baseline
Feed2 - Copied from instance of baseline
Flag_Row - The flag row is the logical "and" of the flag rows
           averaged together.
Data_Desc_Id - Copied from instance of baseline
Processor_Id - Copied from instance of baseline
Field_Id - Copied from instance of baseline
Interval - Set to averaging interval
Exposure - Minimum of the interval and the sum of the exposures
           from unflagged rows
Time_Centroid - sum (timeCentroid[i] * exposure[i]) / sum (exposure[i])
Scan_Number - Copied from instance of baseline
Sigma - ???
Array_Id - Copied from instance of baseline
Observation_Id - Copied from instance of baseline
State_Id - Copied from instance of baseline
Uvw - Weighted average of the UVW values for the baseline
Weight - ???

Cube-level value treatment
--------------------------

For each baseline (i.e., antenna1, antenna2 pair) the average is
computed for each correlation (co) and channel (ch) of the data cube.

Data - sum (f(weightSpectrum (co, ch)) * data (co, ch)) /
       sum (f(weightSpectrum (co, ch)))
       f :== optional function applied to the weights; default is unity function.
Corrected_Data - Same was for Data however, VI setup can disable producing
                 averaged Corrected_Data
Model_Data - Same was for Data however, VI setup can disable producing
             averaged Model_Data
Weight_Spectrum - sum (weightSpectrum (co, ch))
Flag - Each averaged flag (correlation, channel) is the logical "and"
       of the flag (correlation, channel) making up the average.

*/


class BaselineIndex : private boost::noncopyable {

public:

    BaselineIndex ();
    ~BaselineIndex ();

    Int operator () (Int antenna1, Int antenna2, Int spectralWindow);
    void configure (Int nAntennas, Int nSpw, const VisBuffer2 * vb);


private:

    enum {Empty = -1};

    class SpwIndex : public Matrix<Int>{

    public:

        SpwIndex (Int n) : Matrix<Int> (n, n, Empty), nBaselines_p (0) {}

        Int
        getBaselineNumber (Int antenna1, Int antenna2)
        {
            Int i = (* this) (antenna1, antenna2);

            if (i == Empty){

                i = nBaselines_p ++;
                (* this) (antenna1, antenna2) = i;
            }

            return i;
        }

    private:

        Int nBaselines_p;
    };

    typedef vector<SpwIndex *> Index;

    SpwIndex * addSpwIndex (Int spw);
    Matrix<Int> * createMatrix ();
    void destroy();
    SpwIndex * getSpwIndex (Int spw);

    Index index_p;
    Int nAntennas_p;
    Int nSpw_p;

};

BaselineIndex::BaselineIndex ()
: nAntennas_p (0),
  nSpw_p (0)
{}

BaselineIndex::~BaselineIndex ()
{
    destroy();
}

Int
BaselineIndex::operator () (Int antenna1, Int antenna2, Int spectralWindow)
{
    SpwIndex * spwIndex = getSpwIndex (spectralWindow);

    Int i = spwIndex->getBaselineNumber (antenna1, antenna2);

    return i;
}



BaselineIndex::SpwIndex *
BaselineIndex::addSpwIndex (Int i)
{
    // Delete an existing SpwIndex so that we start fresh

    if (index_p [i] != 0){
        delete index_p [i];
        index_p [i] = 0;
    }

    // Create a new SpwIndex and insert it into the main index.

    index_p [i] = new SpwIndex (nAntennas_p);

    return index_p [i];
}

void
BaselineIndex::configure (Int nAntennas, Int nSpw, const VisBuffer2 * vb)
{
    // Capture the shape parameters

    nAntennas_p = nAntennas;
    nSpw_p = nSpw;

    // Get rid of the existing index

    destroy ();
    index_p = Index (nSpw_p, (SpwIndex *) 0);

    // Fill out the index based on the contents of the first VB.
    // Usually this will determine the pattern for all of the VBs to be
    // averaged together so that is the ordering the index should
    // capture.

    for (Int i = 0; i < vb->nRows(); i++){

        // Eagerly flesh out the Spw's index

        Int spw = vb->spectralWindows() (i);
        Int antenna1 = vb->antenna1()(i);
        Int antenna2 = vb->antenna2()(i);

        (* this) (antenna1, antenna2, spw);
    }
}


void
BaselineIndex::destroy ()
{
    // Delete all the dynaically allocated spectral window indices.
    // The vector destructor will take care of index_p itself.

    for (Index::iterator i = index_p.begin();
         i != index_p.end();
         i++){

        delete (* i);
    }
}

BaselineIndex::SpwIndex *
BaselineIndex::getSpwIndex (Int spw)
{
    SpwIndex * spwIndex = index_p [spw];

    if (spwIndex == 0){
        spwIndex = addSpwIndex (spw);
    }

    return spwIndex;
}




class VbAvg : public VisBufferImpl2 {

public:

    VbAvg (Double averagingInterval, Bool doingCorrectedData,
           Bool doingModelData, Bool doingWeightSpectrum);

    void accumulate (const VisBuffer2 * vb, const Subchunk & subchunk);
    Bool empty () const;
    void finalizeAverage ();
    Bool isComplete () const;
    void markEmpty ();
    void prepareForFirstData (const VisBuffer2 *, const Subchunk & subchunk);
    void transferAverage (VisBuffer2 * vb);
    Bool vbPastAveragingInterval (const VisBuffer2 * vb) const;

protected:

    void accumulateCubeData (const VisBuffer2 * input);
    void accumulateElementForCubes (const VisBuffer2 * vb, Int baselineIndex,
                                    Int correlation, Int channel, Int row,
                                    Bool clearAccumulation);
    template<typename T, typename U>
    void
    accumulateElementForCube (Bool doIt,
                              Int baselineIndex,
                              Bool zeroAccumulation,
                              Int correlation,
                              Int channel,
                              Int row,
                              Float weight,
                              const VisBuffer2 * input,
                              const T & (VisBuffer2::* getter) () const,
                              U & (VisBufferImpl2::* setter) ());

    void accumulateExposure (const VisBuffer2 *);
    void accumulateRowData (const VisBuffer2 * input);
    void accumulateTimeCentroid (const VisBuffer2 * input);
    void captureIterationInfo (const VisBuffer2 * vb, const Subchunk & subchunk);
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
    Int getBaselineIndex (const VisBuffer2 * vb, Int row) const;
    Int nBaselines () const;
    void prepareIds (const VisBuffer2 * vb);
    void removeMissingBaselines ();
    void setupBaselineIndices (Int nAntennas, const VisBuffer2 * vb);

private:

    Double averagingInterval_p;
    mutable BaselineIndex baselineIndex_p; // map of antenna1,antenna2 to row number in this VB.
    Bool complete_p; // average is complete
    Cube<Int> counts_p; // number of items summed together for each correlation, channel, baseline item.
    Vector<Int> countsBaseline_p; // number of items summed together for each baseline.
    Bool doingCorrectedData_p;
    Bool doingModelData_p;
    Bool doingWeightSpectrum_p;
    Bool empty_p; // true when buffer hasn't seen any data
    Double sampleInterval_p;
    Double startTime_p; // time of the first sample in average
    Cube<Float> weightSum_p; // accumulation of weights for weighted averaging
};

///////////////////////////////////////////////////////////
//
// Set of Averaging VisBuffers, one per active DD ID


class VbSet {

public:

    VbSet (Double averagingInterval);
    ~VbSet ();

    void accumulate (const VisBuffer2 *, const Subchunk & subchunk);
    VbAvg * add (Int ddId);
    Bool anyAveragesReady(Int ddid = -1) const;
    Bool contains (Int ddId) const;
    void finalizeAverage (Int ddId);
    void finalizeAverages ();
    void flush (Bool okIfNonempty = False, ViImplementation2 * vi = 0); // delete all averagers
    Int getFirstReadyDdid () const;
    void transferAverage (Int ddId, VisBuffer2 * vb);
    Bool vbPastAveragingInterval (const VisBuffer2 * vb) const;
    void zero ();

protected:

    void seeIfCubeColumnsExist (ViImplementation2 * vi);

private:

    typedef map<Int, VbAvg *> Averagers;

    const Double averagingInterval_p;
    Bool doingCorrectedData_p;
    Bool doingModelData_p;
    Bool doingWeightSpectrum_p;
    Averagers vbAveragers_p;
};

VbAvg::VbAvg (Double averagingInterval, Bool doingCorrectedData,
              Bool doingModelData, Bool doingWeightSpectrum)
: VisBufferImpl2 (VbRekeyable),
  averagingInterval_p (averagingInterval),
  complete_p (False),
  doingCorrectedData_p (doingCorrectedData),
  doingModelData_p (doingModelData),
  doingWeightSpectrum_p (doingWeightSpectrum),
  empty_p (True)
{}

void
VbAvg::accumulate (const VisBuffer2 * vb, const Subchunk & subchunk)
{
    // "Add" the contents of this buffer to the accumulation.

    if (empty_p){

        // Initialize the buffer if this is the first time bit of data that it is
        // being used after either creation or clearing.

        prepareForFirstData (vb, subchunk);

        empty_p = False;
    }

    // Averaging can be computed as flagged or unflagged.  If all the inputs to a
    // datum are flagged, then the averaged datum (e.g., a visibility point)
    // will also be flagged.  For an unflagged averaged datum, it will represent
    // the average of all of the unflagged points for that datum.  This is done
    // by assuming the accumulation is flagged and continuing to accumulate
    // flagged points until the first unflagged point for a datum is encountered;
    // when this happens the count is zeroed and the averaged datum's flag is cleared.

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

    for (Int row = 0; row < vb->nRows(); row ++){

        Bool inputRowFlagged = vb->flagRow () (row);

        Int baselineIndex = getBaselineIndex (vb, row);

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

                accumulateElementForCubes (vb, baselineIndex, correlation, channel, row, flagChange);
            }
        }
    }
}

template<typename T, typename U>
void
VbAvg::accumulateElementForCube (Bool doIt,
                                 Int baselineIndex,
                                 Bool zeroAccumulation,
                                 Int correlation,
                                 Int channel,
                                 Int row,
                                 Float weight,
                                 const VisBuffer2 * input,
                                 const T & (VisBuffer2::* getter) () const,
                                 U & (VisBufferImpl2::* setter) ())
{
    if (doIt){

        // Update the sum for this model visibility cube element.

        typedef typename T::value_type  E;

        E value = (input ->* getter) () (correlation, channel, row);
        E weightedValue = value * weight;
        E accumulation = zeroAccumulation ? 0
                                          : (this ->* getter) () (correlation, channel, row);

        (this ->* setter) () (correlation, channel, baselineIndex) = weightedValue + accumulation;
    }
}

//void
//VbAvg::accumulateCubeElement (const VisBuffer2 * input, Int baselineIndex, Int correlation,
//                              Int channel, Int row, Bool zeroAccumulation)
//{
//    // Zero accumulation is used to restart the accumulation when the flagged state of the
//    // element's accumulation changes from flagged to unflagged.
//
//    // Get the weight to use for averaging.  If arithmetic averaging is being used the function
//    // will return 1.0.  Otherwise if will return some function of the weight or weight-spectrum
//    // (e.g., square, square-root, etc.).
//
//    Float weight =  input->getWeightScaled (correlation, channel, row);
//
//    weightSum_p (correlation, channel, row) =
//        zeroAccumulation ? weight : (weightSum_p (correlation, channel, row) + weight);
//
//    // Update the sum for this visibility cube element.
//
//    Complex weightedValue = input->visCube () (correlation, channel, row) * weight;
//    visCubeRef ()(correlation, channel, baselineIndex) = weightedValue +
//        (zeroAccumulation ? 0 : visCube () (correlation, channel, row));
//
//    // The result of averaging for the weight spectrum is to sum up the weights rather
//    // than average them.  The sum is over the raw weights (i.e., rather than optionally
//    // applying a function to the weight as is done for other cube data).
//
//    if (doingWeightSpectrum_p){
//
//        weightSpectrumRef() (correlation, channel, baselineIndex) =
//            input->weightSpectrum () (correlation, channel, row) +
//            (zeroAccumulation ? 0 : weightSpectrum () (correlation, channel, row));
//    }
//
//    if (doingCorrectedData_p){
//
//        // Update the sum for this corrected visibility cube element.
//
//        Complex weightedValue = input->visCubeCorrected () (correlation, channel, row) * weight;
//        visCubeCorrectedRef ()(correlation, channel, baselineIndex) = weightedValue +
//             (zeroAccumulation ? 0 : visCubeCorrected () (correlation, channel, row));
//    }
//
//    if (doingModelData_p){
//
//        // Update the sum for this model visibility cube element.
//
//        Complex weightedValue = input->visCubeModel () (correlation, channel, row) * weight;
//        visCubeModelRef ()(correlation, channel, baselineIndex) = weightedValue +
//            (zeroAccumulation ? 0 : visCubeModel () (correlation, channel, row));
//    }
//}

void
VbAvg::accumulateElementForCubes (const VisBuffer2 * input, Int baselineIndex, Int correlation,
                                  Int channel, Int row, Bool zeroAccumulation)
{
    // Zero accumulation is used to restart the accumulation when the flagged state of the
    // element's accumulation changes from flagged to unflagged.

    // Get the weight to use for averaging.  If arithmetic averaging is being used the function
    // will return 1.0.  Otherwise if will return some function of the weight or weight-spectrum
    // (e.g., square, square-root, etc.).

    Float weight =  input->getWeightScaled (correlation, channel, row);

    weightSum_p (correlation, channel, row) =
        zeroAccumulation ? weight : (weightSum_p (correlation, channel, row) + weight);

    // Update the sum for the three visibility cubes (corrected and model cubes only if present)

    accumulateElementForCube (True, baselineIndex, zeroAccumulation,
                              correlation, channel, row, weight, input,
                              & VisBuffer2::visCube, & VbAvg::visCubeRef);

    accumulateElementForCube (doingCorrectedData_p, baselineIndex, zeroAccumulation,
                              correlation, channel, row, weight, input,
                              & VisBuffer2::visCubeCorrected, & VbAvg::visCubeCorrectedRef);

    accumulateElementForCube (doingModelData_p, baselineIndex, zeroAccumulation,
                              correlation, channel, row, weight, input,
                              & VisBuffer2::visCubeModel, & VbAvg::visCubeModelRef);

    // The result of averaging for the weight spectrum is to sum up the weights rather
    // than average them.  The sum is over the raw weights (i.e., rather than optionally
    // applying a function to the weight as is done for other cube data).

    accumulateElementForCube (doingWeightSpectrum_p, baselineIndex, zeroAccumulation,
                              correlation, channel, row, 1.0f, input,
                              & VisBuffer2::weightSpectrum, & VbAvg::weightSpectrumRef);
}


void
VbAvg::accumulateExposure (const VisBuffer2 * input)
{
    Vector<Double> exposureSum = exposure ();

    for (Int row = 0; row < input->nRows(); row ++){

        Int baselineIndex = getBaselineIndex (input, row);

        exposureSum [baselineIndex] += input->exposure () (row);
    }

    setExposure (exposureSum);
}

void
VbAvg::accumulateRowData (const VisBuffer2 * input)
{
    // Grab working copies of the values to be accumulated.

    Vector<Double> exposureSum = exposure ();
    Matrix<Float> sigmaSum = sigma();
    Vector<Double> timeCentroidSum = timeCentroid ();
    Matrix<Double> uvwSum = uvw();
    Matrix<Float> weightSum = weight();

    for (Int row = 0; row < input->nRows(); row ++){

        Int baselineIndex = getBaselineIndex (input, row);

        Bool & accumulatorRowFlagged = flagRowRef ()(baselineIndex);
        Bool inputRowFlagged = input->flagRow () (row);

        copyRowIdValues (input, row, baselineIndex);

        if (! accumulatorRowFlagged && inputRowFlagged){
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

            for (Int i = 0; i < nCorrelations(); i ++){
                sigmaSum (i, baselineIndex) += input->sigma() (i, row);
                weightSum (i, baselineIndex) += input->weight () (i, row);
            }
        }
    }

    setExposure (exposureSum);
    setSigma (sigmaSum);
    setTimeCentroid (timeCentroidSum);
    setUvw (uvwSum);
    setWeight (weightSum);
}

void
VbAvg::copyRowIdValues (const VisBuffer2 * input, Int row, Int baselineIndex)
{
    copyRowIdValue (input, row, & VisBuffer2::antenna1, & VisBuffer2::setAntenna1, baselineIndex);
    copyRowIdValue (input, row, & VisBuffer2::antenna2, & VisBuffer2::setAntenna2, baselineIndex);
    copyRowIdValue (input, row, & VisBuffer2::dataDescriptionIds, & VisBuffer2::setDataDescriptionIds, baselineIndex);
    copyRowIdValue (input, row, & VisBuffer2::feed1, & VisBuffer2::setFeed1, baselineIndex);
    copyRowIdValue (input, row, & VisBuffer2::feed2, & VisBuffer2::setFeed2, baselineIndex);
    copyRowIdValue (input, row, & VisBuffer2::processorId, & VisBuffer2::setProcessorId, baselineIndex);
    copyRowIdValue (input, row, & VisBuffer2::scan, & VisBuffer2::setScan, baselineIndex);
    copyRowIdValue (input, row, & VisBuffer2::spectralWindows, & VisBuffer2::setSpectralWindows, baselineIndex);
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
VbAvg::getBaselineIndex (const VisBuffer2 * vb, Int row) const
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

    Int antenna1 = vb->antenna1 () (row);
    Int antenna2 = vb->antenna2 () (row);
    Int spw = vb->spectralWindows () (row);

    Int index = baselineIndex_p (antenna1, antenna2, spw);

    return index;
}

void
VbAvg::finalizeAverage ()
{
    if (empty_p){
        return; // nothing to finalize
    }

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
}

void
VbAvg::finalizeRowData ()
{
    Vector<Double> timeCentroidAvg = timeCentroid ();
    Matrix<Double> uvwAvg = uvw();

    for (Int baseline = 0; baseline < nBaselines(); baseline ++){

        Int n = countsBaseline_p (baseline);

        if (n != 0){

            timeCentroidAvg (baseline) /= n;

            for (Int i = 0; i < 3; i ++){
                uvwAvg (i, baseline) /= n;
            }

            // Exposure is a simple sum, not an average to it is already
            // done at thhis point.
        }
    }

    setTimeCentroid (timeCentroidAvg);
    setUvw (uvwAvg);

    // Fill in the time and the interval
    //
    // The time of a sample is centered over the integration time period.
    // Thus find the center of the averaged interval it is necessary to
    // slide it back by 1/2 an interval.

    Double centerOfInterval = startTime_p + (averagingInterval_p - sampleInterval_p) * 0.5;
    Vector<Double> x (nBaselines(), centerOfInterval);
    setTime (x);

    x = averagingInterval_p;
    setTimeInterval (x);
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
VbAvg::captureIterationInfo (const VisBuffer2 * vb, const Subchunk & subchunk)
{
    setIterationInfo (vb->msId(),
                      vb->msName(),
                      vb->isNewMs(),
                      vb->isNewArrayId(),
                      vb->isNewFieldId(),
                      vb->isNewSpectralWindow(),
                      subchunk,
                      vb->getCorrelationNumbers (),
                      CountedPtr <WeightScaling> (0));
}

void
VbAvg::prepareForFirstData (const VisBuffer2 * vb, const Subchunk & subchunk)
{
    startTime_p = vb->time() (0);
    sampleInterval_p = vb->timeInterval() (0);

    Int nAntennas = vb->nAntennas();
    Int nSpw = vb->getVi()->nSpectralWindows();
    Int nBaselines = ((nAntennas * (nAntennas + 1)) / 2) * nSpw;

    // Size and zero out the counters

    countsBaseline_p = Vector<Int> (nBaselines, 0);
    counts_p = Cube<Int> (vb->nCorrelations(), vb->nChannels(), nBaselines, 0);
    weightSum_p = Cube<Float> (vb->nCorrelations(), vb->nChannels(), nBaselines, 0);

    baselineIndex_p.configure (nAntennas, nSpw, vb);

    // Reshape the inherited members from VisBuffer2

    captureIterationInfo (vb, subchunk);

    setShape (vb->nCorrelations(), vb->nChannels(), nBaselines, False);

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

    Vector<Int> minusOne (nBaselines(), -1);

    setAntenna1 (minusOne);
    setAntenna2 (minusOne);
    setDataDescriptionIds (minusOne);
    setFeed1 (minusOne);
    setFeed2 (minusOne);
    setProcessorId (minusOne);
    setScan (minusOne);
    setObservationId (minusOne);
    setSpectralWindows (minusOne);
    setStateId (minusOne);

    // Copy the value from the input VB

    Vector<Int> tmp (nBaselines(), vb->arrayId()(0));

    setArrayId (tmp);

    tmp = vb->dataDescriptionIds()(0);
    setDataDescriptionIds (tmp);

    tmp = vb->fieldId()(0);
    setFieldId (tmp);
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


//void
//VbAvg::setupBaselineIndices(int nAntennas, const VisBuffer2 * vb)
//{
//    // Create a square matrix to hold the mappings between pairs of
//    // antennas to the baseline number.  This includes autocorrelations.
//    // Only the lower left half of the matrix holds valid baseline indices;
//    // the upper right matrix will hold -1s.
//    //
//    // The decoding routine, getBaselineIndex will throw an exception if
//    // an antenna1,antenna2 pair maps would return a -1.
//
//    baselineIndex_p->configure (nAntennas,
//
//    baselineIndex_p.resize (nAntennas, nAntennas, False);
//    baselineIndex_p = -1;
//    Int baseline = 0;
//
//    // Use the baseline order of this VisBuffer2 to set up the default mapping
//    // from VB baselines to averaged baselines; unless this is a partial VB
//    // then this should be optimum and if not, it only applies for the current
//    // average.  If the first vb is normal then it should eliminate the need to
//    // copy-down the rows of the VisBuffer for baselines that are possible but
//    // not present.
//    //
//    // Also be sure that multiple occurrences of the same baseline do not fould
//    // things up.
//
//    for (Int vbRow = 0; vbRow < vb->nRows(); vbRow ++){
//
//        Int a1 = vb->antenna1()(vbRow);
//        Int a2 = vb->antenna2()(vbRow);
//
//        if (a1 <= a2 && baselineIndex_p (a1, a2) < 0){
//            baselineIndex_p (a1, a2) = baseline ++;
//        }
//    }
//
//    // Now put a mapping in for any baselines that weren't in the current VB
//    // just in case they appear later.  These will be at the end so won't
//    // require shifting data down if they are not present.
//
//    for (Int row = 0; row < nAntennas; row ++){
//
//        for (Int column = 0; column <= row; column ++){
//
//            if (baselineIndex_p (row, column) < 0){
//                baselineIndex_p (row, column) = baseline ++;
//            }
//        }
//    }
//}

void
VbAvg::transferAverage (VisBuffer2 * vb2)
{
    VisBufferImpl2 * vb = dynamic_cast<VisBufferImpl2 *> (vb2);
    Assert (vb != 0);

    Bool changeShape = vb->getSubchunk().atOrigin() || // may not have since info now
                       vb->nCorrelations() != nCorrelations() ||
                       vb->nChannels() != nChannels() ||
                       vb->nRows() != nRows();

    if (changeShape){

        Bool wasRekeyable = vb->isRekeyable ();
        vb->setRekeyable(True);

        vb->setShape (nCorrelations(), nChannels(), nRows(), True);

        vb->setRekeyable (wasRekeyable);
    }

    vb->setIterationInfo (msId(),
                          msName(),
                          isNewMs(),
                          isNewArrayId(),
                          isNewFieldId(),
                          isNewSpectralWindow(),
                          getSubchunk(),
                          getCorrelationNumbers (),
                          CountedPtr <WeightScaling> (0));

    vb->copy (* this, False);

    markEmpty ();
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

VbSet::VbSet (Double averagingInterval)
: averagingInterval_p (averagingInterval),
  doingCorrectedData_p (False),
  doingModelData_p (False),
  doingWeightSpectrum_p (False),
  vbAveragers_p ()
{}

VbSet::~VbSet ()
{
    flush (True); // allow killing nonempty buffers
}

void
VbSet::accumulate (const VisBuffer2 * input, const Subchunk & subchunk)
{
    Int ddId = input->dataDescriptionIds()(0);


    if (! utilj::containsKey (ddId, vbAveragers_p)){ // Need a new averager
        add (ddId);
    }

    VbAvg * vba = vbAveragers_p [ddId];

    vba->accumulate (input, subchunk);
}

VbAvg *
VbSet::add (Int ddId)
{
    VbAvg * newAverager =  new VbAvg (averagingInterval_p,
                                      doingCorrectedData_p,
                                      doingModelData_p,
                                      doingWeightSpectrum_p);

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
            (ddid < 0 || ddid == a->second->dataDescriptionIds()(0))){

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
VbSet::flush (Bool okIfNonempty, ViImplementation2 * vi)
{
    // Get rid of all of the averagers.  This is done at
    // destruction time and when a sweeping into a new MS.

    for (Averagers::const_iterator a = vbAveragers_p.begin();
         a != vbAveragers_p.end();
         a ++){

        Assert (okIfNonempty || (a->second)->empty());
            // should have been emptied before calling this

        delete a->second;
    }

    vbAveragers_p.clear();

    seeIfCubeColumnsExist (vi);
}

void
VbSet::seeIfCubeColumnsExist (ViImplementation2 * vi)
{
    if (vi != 0){

        // See if the new MS has corrected and/or model data columns

        doingCorrectedData_p = vi->existsColumn (VisibilityCubeCorrected);
        doingModelData_p = vi->existsColumn (VisibilityCubeModel);
        doingWeightSpectrum_p = vi->existsColumn (WeightSpectrum);
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
    Assert (utilj::containsKey (ddId, vbAveragers_p));

    VbAvg * vba = vbAveragers_p [ddId];

    Assert (vba != 0 && ! vba->empty ());

    // Copy the completed average into the provided VisBuffer, but
    // first reshape the VB if it's shape is different.

    vba->transferAverage (vb);

}

Bool
VbSet::vbPastAveragingInterval (const VisBuffer2 * vb) const
{
    Int ddId = vb->dataDescriptionIds()(0);

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

    ///////////////////////
    //                   //
    // End Namespace avg //
    //                   //
    ///////////////////////

} // end unnamed avg

using namespace avg;

AveragingTvi2::AveragingTvi2 (VisibilityIterator2 * vi, ViImplementation2 * inputVi, Double averagingInterval)
: TransformingVi2 (vi, inputVi),
  averagingInterval_p (averagingInterval),
  ddidLastUsed_p (-1),
  inputViiAdvanced_p (False),
  vbSet_p (new VbSet (averagingInterval))
{
    validateInputVi (inputVi);

    // Position input Vi to the first subchunk

    getVii()->originChunks();
    getVii()->origin ();

    setVisBuffer (VisBuffer2::factory (vi, VbPlain, VbNoOptions));
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

    subchunk_p.incrementSubChunk();
}

void
AveragingTvi2::nextChunk ()
{
    // New chunk, so toss all of the accumulators

    vbSet_p->flush (True, getVii());

    // Advance the input to the next chunk as well.

    getVii()->nextChunk ();

    subchunk_p.incrementChunk();

    subchunkExists_p = False;
}

void
AveragingTvi2::origin ()
{
    // Position input VI to the start of the chunk

    subchunk_p.resetSubChunk();

    getVii()->origin();
    inputViiAdvanced_p = True;
    subchunkExists_p = False;

    // Zero out the accumulators

    vbSet_p->zero();

    // Get the first subchunk ready.

    produceSubchunk ();
}

void
AveragingTvi2::originChunks (Bool forceRewind)
{
    vbSet_p->flush (True, getVii());
    getVii()->originChunks(forceRewind);
    subchunkExists_p = False;

    subchunk_p.resetToOrigin();
}

void
AveragingTvi2::processInputSubchunk (const VisBuffer2 * vb)
{
    vbSet_p->accumulate (vb, subchunk_p);
}

void
AveragingTvi2::produceSubchunk ()
{
    Bool needMoreInputData = ! subchunksReady();

    while (needMoreInputData){

        if (! inputViiAdvanced_p && getVii()->more()){

            getVii()->next();
            inputViiAdvanced_p = getVii()->more();
        }

        if (! getVii()->more()){

            vbSet_p->finalizeAverages ();

            break;
        }

        const VisBuffer2 * vb = getVii()->getVisBuffer();

        if (vbSet_p->vbPastAveragingInterval(vb)){

            vbSet_p->finalizeAverage (vb->dataDescriptionIds()(0));

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
