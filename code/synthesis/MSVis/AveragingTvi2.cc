#include <casa/Arrays/ArrayMath.h>
#include <casa/BasicMath/Functors.h>
#include <synthesis/MSVis/AveragingTvi2.h>
#include <synthesis/MSVis/AveragingVi2Factory.h>
#include <synthesis/MSVis/MsRows.h>
#include <synthesis/MSVis/VisBufferComponents2.h>
#include <synthesis/MSVis/VisBuffer2.h>
#include <synthesis/MSVis/VisBufferImpl2.h>
#include <synthesis/MSVis/Vbi2MsRow.h>
#include <synthesis/MSVis/UtilJ.h>
#include <synthesis/MSVis/VisibilityIterator2.h>
#include <boost/tuple/tuple.hpp>
#include <set>

using std::set;

namespace casa {

namespace vi {

namespace avg {

using casa::ms::MsRow;


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

        SpwIndex (Int n) : Matrix<Int> (n, n, Empty) {}

        Int
        getBaselineNumber (Int antenna1, Int antenna2, Int & nBaselines)
        {
            Int i = (* this) (antenna1, antenna2);

            if (i == Empty){

                i = nBaselines ++;
                (* this) (antenna1, antenna2) = i;
            }

            return i;
        }

    private:

    };

    typedef vector<SpwIndex *> Index;

    SpwIndex * addSpwIndex (Int spw);
    Matrix<Int> * createMatrix ();
    void destroy();
    SpwIndex * getSpwIndex (Int spw);

    Index index_p;
    Int nAntennas_p;
    Int nBaselines_p;
    Int nSpw_p;

};

BaselineIndex::BaselineIndex ()
: nAntennas_p (0),
  nBaselines_p (0),
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
    if (spwIndex == 0){
        addSpwIndex (spectralWindow);
    }

    Int i = spwIndex->getBaselineNumber (antenna1, antenna2, nBaselines_p);

    return i;
}



BaselineIndex::SpwIndex *
BaselineIndex::addSpwIndex (Int i)
{
    // Delete an existing SpwIndex so that we start fresh

    delete index_p [i];

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
    nBaselines_p = 0;

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
    AssertCc (spw < (int) index_p.size());

    SpwIndex * spwIndex = index_p [spw];

    if (spwIndex == 0){
        spwIndex = addSpwIndex (spw);
    }

    return spwIndex;
}

class VbAvg;

class MsRowAvg : public ms::Vbi2MsRow {

public:

    MsRowAvg (Int row, const VbAvg * vb);

    // Constructor for read/write access

    MsRowAvg (Int row, VbAvg * vb);

    virtual ~MsRowAvg () {}

    Bool baselinePresent () const;
    Vector<Bool> correlationFlagsMutable ();
    Matrix<Int> counts ();
    Int countsBaseline () const;
    Matrix<Bool> flagsMutable () { return Vbi2MsRow::flagsMutable();}
    Double intervalLast () const;
    Double timeFirst () const;
    Double timeLast () const;
    Vector<Double> uvwFirst ();

    void setBaselinePresent (Bool isPresent);
    void setCounts (const Matrix<Int> &);
    void setCountsBaseline (Int);
    void setIntervalLast (Double);
    void setTimeFirst (Double);
    void setTimeLast (Double);

private:

    VbAvg * vbAvg_p; // [use]
};

class VbAvg : public VisBufferImpl2 {

public:

    friend class MsRowAvg;

    VbAvg (const AveragingParameters & averagingParameters);

    void accumulate (const VisBuffer2 * vb, const Subchunk & subchunk);
    Bool empty () const;
    void finalizeBufferFilling ();
    void finalizeAverages ();
    MsRowAvg * getRow (Int row) const;
    MsRowAvg * getRowMutable (Int row);
    Bool isComplete () const;
    void markEmpty ();
    int nSpectralWindowsInBuffer () const;
    void setBufferToFill (VisBufferImpl2 *);
    void startChunk (ViImplementation2 *);

protected:

    class Doing {
    public:

        Doing ()
        : correctedData_p (False),
          modelData_p (False),
          observedData_p (False),
          weightSpectrum_p (False),
          weightSpectrumCorrected_p (False)
        {}

        Bool correctedData_p;
        Bool modelData_p;
        Bool observedData_p;
        Bool weightSpectrum_p;
        Bool weightSpectrumCorrected_p;
    };

    class AccumulationParameters {

    public:

        AccumulationParameters (MsRow * rowInput, MsRowAvg * rowAveraged,
                                Doing doing)
        : correctedIn (doing.correctedData_p ? rowInput->corrected()
                                            : Matrix<Complex> ()),
          correctedOut (doing.correctedData_p ? rowAveraged->corrected()
                                             : Matrix<Complex> ()),
          modelIn (doing.modelData_p ? rowInput->model()
                                    : Matrix<Complex> ()),
          modelOut (doing.modelData_p ? rowAveraged->model()
                                     : Matrix<Complex> ()),
          observedIn (doing.observedData_p ? rowInput->observed()
                                          : Matrix<Complex> ()),
          observedOut (doing.observedData_p ? rowAveraged->observed() : Matrix<Complex> ()),
          usingWeightSpectrum_p (doing.weightSpectrum_p),
          weightIn (rowInput->weight()),
          weightOut (rowAveraged->weight()),
          weightSpectrumCorrectedIn (doing.weightSpectrumCorrected_p ? rowInput->weightSpectrumCorrected()
                                                                    : Matrix<Float> ()),
          weightSpectrumCorrectedOut (doing.weightSpectrumCorrected_p ? rowAveraged->weightSpectrumCorrected()
                                                                     : Matrix<Float> ()),
          weightSpectrumIn (rowInput->weightSpectrum()),
          weightSpectrumOut (rowAveraged->weightSpectrum())
        {}

        const Matrix<Complex> correctedIn;
        Matrix<Complex>       correctedOut;
        const Matrix<Complex> modelIn;
        Matrix<Complex>       modelOut;
        const Matrix<Complex> observedIn;
        Matrix<Complex>       observedOut;
        Bool                  usingWeightSpectrum_p;
        const Vector<Float>   weightIn;
        Vector<Float>         weightOut;
        const Matrix<Float>   weightSpectrumCorrectedIn;
        Matrix<Float>         weightSpectrumCorrectedOut;
        const Matrix<Float>   weightSpectrumIn;
        Matrix<Float>         weightSpectrumOut;
    };

    pair<Bool, Vector<Double> > accumulateCubeData (MsRow * rowInput, MsRowAvg * rowAveraged);
    void accumulateElementForCubes (AccumulationParameters * accumulationParameters,
                                    Bool zeroAccumulation,
                                    Int correlation,
                                    Int channel);
    template<typename T>
    void
    accumulateElementForCube (const T & unweightedValue,
                              Float weight,
                              Bool zeroAccumulation,
                              T & accumulator);

    template <typename T>
    T accumulateRowDatum (const T & averagedValue, const T & inputValue,
                          Bool resetAverage);



    void accumulateExposure (const VisBuffer2 *);
    void accumulateOneRow (MsRow * rowInput, MsRowAvg * rowAveraged,
                           const Subchunk & subchunk);
    void accumulateRowData (MsRow * rowInput, MsRowAvg * rowAveraged, Double adjustedWeight,
                            Bool rowFlagged);
    void accumulateTimeCentroid (const VisBuffer2 * input);
    void captureIterationInfo (VisBufferImpl2 * dstVb, const VisBuffer2 * srcVb,
                               const Subchunk & subchunk);
    void copyBaseline (Int sourceBaseline, Int targetBaseline);
    template<typename T>
    void copyBaselineScalar (Int sourceBaseline, Int targetBaseline,
                             Vector<T> & columnVector);
    template<typename T>
    void copyCubePlaneIf (Bool condition, Int sourceBaseline,
                          Int targetBaseline, Cube<T> & cube);
    void copyIdValues (MsRow * rowInput, MsRowAvg * rowAveraged);
    void copyIdValue (Int inputId, Int & averagedId);
    void finalizeBaseline (MsRowAvg *);
    void finalizeBaselineIfNeeded (MsRow * rowInput, MsRowAvg * rowAveraged,
                                   const Subchunk & subchunk);
    void finalizeCubeData (MsRowAvg *);
    void finalizeRowData (MsRowAvg *);
    AccumulationParameters * getAccumulationParameters (MsRow * rowInput,
                                                        MsRowAvg * rowAveraged);
    Int getBaselineIndex (const MsRow *) const;
    void initializeBaseline (MsRow * rowInput, MsRowAvg * rowAveraged,
                             const Subchunk & subchunk);
    Int nBaselines () const;
    void prepareIds (const VisBuffer2 * vb);
    void removeMissingBaselines ();
    void setupVbAvg (const VisBuffer2 *);
    void setupArrays (Int nCorrelations, Int nChannels, Int nBaselines);
    void setupBaselineIndices (Int nAntennas, const VisBuffer2 * vb);
    void transferBaseline (MsRowAvg *);

    template <typename T>
    static T
    distance (const Vector<T> & p1, const Vector<T> & p2)
    {
        assert (p1.size() ==3 && p2.size() == 3);

        T distance = 0;

        for (Int i = 0; i < 3; i++){
            distance += pow (p1[i] - p2[i], 2);
        }

        return sqrt (distance);
    }

private:

    Double averagingInterval_p;
    AveragingOptions averagingOptions_p;
    mutable BaselineIndex baselineIndex_p; // map of antenna1,antenna2 to row number in this VB.
    Vector<Bool> baselinePresent_p; // indicates whether baseline has any data
    VisBufferImpl2 * bufferToFill_p;
    Bool complete_p; // average is complete
    Matrix<Bool> correlationFlags_p; // used for weight accumulation
    Cube<Int> counts_p; // number of items summed together for each correlation, channel, baseline item.
    Vector<Int> countsBaseline_p; // number of items summed together for each baseline.
    Doing doing_p;
    Bool empty_p; // true when buffer hasn't seen any data
    Vector<Double> intervalLast_p;
    Double maxTimeDistance_p;
    Double maxUvwDistance_p;
    Bool needIterationInfo_p;
    VisBufferComponents2 optionalComponentsToCopy_p;
    Int rowIdGenerator_p;
    Double sampleInterval_p;
    Double startTime_p; // time of the first sample in average
    Vector<Double> timeFirst_p;
    Vector<Double> timeLast_p;
    Matrix<Double> uvwFirst_p;
    Bool usingUvwDistance_p;
};

///////////////////////////////////////////////////////////
//
// Set of Averaging VisBuffers, one per active DD ID


//class VbSet {
//
//public:
//
//    VbSet (const AveragingParameters & averagingParameters);
//    ~VbSet ();
//
//    void accumulate (const VisBuffer2 *, const Subchunk & subchunk);
//    void finalizeBufferFilling ();
//    void setBufferToFill (VisBuffer2 *);
//
//
//    VbAvg * add (Int ddId);
//    Bool anyAveragesReady(Int ddid = -1) const;
//    Bool contains (Int ddId) const;
////    void finalizeAverage (Int ddId);
//    void finalizeAverages ();
//    void finalizeRowIfNeeded (ms::MsRow * rowInput, avg::MsRowAvg * rowAveraged, const Subchunk & subchunk);
//    void flush (Bool okIfNonempty = False, ViImplementation2 * vi = 0); // delete all averagers
//    Int getFirstReadyDdid () const;
//    void transferAverage (Int ddId, VisBuffer2 * vb);
//    Bool vbPastAveragingInterval (const VisBuffer2 * vb) const;
//    void zero ();
//
//protected:
//
//    void seeIfCubeColumnsExist (ViImplementation2 * vi);
//
//private:
//
//    typedef map<Int, VbAvg *> Averagers;
//
//    const Double averagingInterval_p;
//    AveragingOptions averagingOptions_p;
//    const AveragingParameters averagingParameters_p;
//    Bool doingCorrectedData_p;
//    Bool doingModelData_p;
//    Bool doingObservedData_p;
//    Bool doingWeightSpectrum_p;
//    Bool doingWeightSpectrumCorrected_p;
//    Averagers vbAveragers_p;
//};

MsRowAvg::MsRowAvg (Int row, const VbAvg * vb)
: Vbi2MsRow (row, vb),
  vbAvg_p (const_cast<VbAvg *> (vb))
{}

// Constructor for read/write access

MsRowAvg::MsRowAvg (Int row, VbAvg * vb)
: Vbi2MsRow (row, vb),
  vbAvg_p (vb)
{}

Bool
MsRowAvg::baselinePresent () const
{
    return vbAvg_p->baselinePresent_p (row ());
}


Matrix<Int>
MsRowAvg::counts ()
{
    return vbAvg_p->counts_p.xyPlane (row ());
}

Vector<Bool>
MsRowAvg::correlationFlagsMutable ()
{
    return vbAvg_p->correlationFlags_p.column (row());
}

Int
MsRowAvg::countsBaseline () const
{
    return vbAvg_p->countsBaseline_p (row ());
}

void
MsRowAvg::setBaselinePresent (Bool value)
{
    vbAvg_p->baselinePresent_p (row ()) = value;
}


void
MsRowAvg::setCountsBaseline (Int value)
{
    vbAvg_p->countsBaseline_p (row ()) = value;
}

Double
MsRowAvg::intervalLast () const
{
    return vbAvg_p->intervalLast_p (row ());
}


Double
MsRowAvg::timeFirst () const
{
    return vbAvg_p->timeFirst_p (row ());
}

Double
MsRowAvg::timeLast () const
{
    return vbAvg_p->timeLast_p (row ());
}

Vector<Double>
MsRowAvg::uvwFirst ()
{
    return vbAvg_p->uvwFirst_p.column (row());
}


void
MsRowAvg::setCounts (const Matrix<Int> & value)
{
    vbAvg_p->counts_p.xyPlane  (row()) = value;
}

void
MsRowAvg::setIntervalLast (Double value)
{
    vbAvg_p->intervalLast_p (row ()) = value;
}


void
MsRowAvg::setTimeFirst (Double value)
{
    vbAvg_p->timeFirst_p (row ()) = value;
}

void
MsRowAvg::setTimeLast (Double value)
{
    vbAvg_p->timeLast_p (row ()) = value;
}

VbAvg::VbAvg (const AveragingParameters & averagingParameters)
: VisBufferImpl2 (VbRekeyable),
  averagingInterval_p (averagingParameters.getAveragingInterval ()),
  averagingOptions_p (averagingParameters.getOptions()),
  complete_p (False),
  doing_p (), // all false until determined later on
  empty_p (True),
  maxTimeDistance_p (averagingParameters.getAveragingInterval() * (0.999)),
        // Shrink it just a bit for roundoff
  maxUvwDistance_p (averagingParameters.getMaxUvwDistance()),
  rowIdGenerator_p (0),
  usingUvwDistance_p (averagingParameters.getOptions().contains (AveragingOptions::BaselineDependentAveraging))
{}

void
VbAvg::accumulate (const VisBuffer2 * vb, const Subchunk & subchunk)
{
    if (empty_p){
        setupVbAvg (vb);
    }

    if (needIterationInfo_p){
        captureIterationInfo (bufferToFill_p, vb, subchunk);
        needIterationInfo_p = False;
    }

    assert (bufferToFill_p != 0);

    MsRowAvg * rowAveraged = getRowMutable (0);
    MsRow * rowInput = vb->getRow (0);

    for (Int row = 0; row < vb->nRows(); row ++){

        rowInput->changeRow (row);
        Int baselineIndex = getBaselineIndex (rowInput);

        rowAveraged->changeRow (baselineIndex);

        accumulateOneRow (rowInput, rowAveraged, subchunk);
    }

    delete rowAveraged;
    delete rowInput;
}

void
VbAvg::accumulateOneRow (MsRow * rowInput, MsRowAvg * rowAveraged, const Subchunk & subchunk)
{
    finalizeBaselineIfNeeded (rowInput, rowAveraged, subchunk);

    if (! rowAveraged->baselinePresent()){

        initializeBaseline (rowInput, rowAveraged, subchunk);

    }

    // Accumulate data that is matrix-valued (e.g., vis, visModel, etc.).
    // The computation of time centroid requires the use of the weight column
    // adjusted for the flag cube.  Get the before and after weight accumulation
    // and the difference is the adjusted weight for this row.

    Vector<Double> adjustedWeights;
    Bool rowFlagged;
    boost::tie (rowFlagged, adjustedWeights) = accumulateCubeData (rowInput, rowAveraged);

    Double adjustedWeight = 0;

    for (Int c = 0; c < nCorrelations(); c++){
        adjustedWeight += rowAveraged->correlationFlagsMutable() (c) ? 0 : adjustedWeights (c);
    }

    // Accumulate the non matrix-valued data

    accumulateRowData (rowInput, rowAveraged, adjustedWeight, rowFlagged);

}

//void
//VbAvg::accumulate (const VisBuffer2 * vb, const Subchunk & subchunk)
//{
//    // "Add" the contents of this buffer to the accumulation.
//
//    if (empty_p){
//
//        // Initialize the buffer if this is the first time bit of data that it is
//        // being used after either creation or clearing.
//
//        prepareForFirstData (vb, subchunk);
//
//        empty_p = False;
//    }
//
//    // Averaging can be computed as flagged or unflagged.  If all the inputs to a
//    // datum are flagged, then the averaged datum (e.g., a visibility point)
//    // will also be flagged.  For an unflagged averaged datum, it will represent
//    // the average of all of the unflagged points for that datum.  This is done
//    // by assuming the accumulation is flagged and continuing to accumulate
//    // flagged points until the first unflagged point for a datum is encountered;
//    // when this happens the count is zeroed and the averaged datum's flag is cleared.
//
//    // Loop over all of the rows in the VB.  Map each one to a baseline and then accumulate
//    // the values for each correlation,channel cell.  Each row in the accumulating VB corresponds
//    // to one baseline (i.e., pair of (antenna1, antenna2) where antenna1 <= antenna2).
//
//    ThrowIf (vb->nRows() > nBaselines(),
//             String::format ("Expected %d baselines in VisBuffer but it contained %d rows",
//                             nBaselines(), nRows()));
//
//    for (Int row = 0; row < vb->nRows(); row ++){
//
//        // Accumulate data for fields that are scalars (and uvw) in each row
//
//        accumulateRowData (vb, row);
//
//        // Accumulate data that is matrix-valued (e.g., vis, visModel, etc.)
//
//        accumulateCubeData (vb, row);
//    }
//}

void
VbAvg::finalizeBufferFilling ()
{
    bufferToFill_p->appendRowsComplete();
    bufferToFill_p = 0; // decouple
}

VbAvg::AccumulationParameters *
VbAvg::getAccumulationParameters (MsRow * rowInput, MsRowAvg * rowAveraged)
{
    AccumulationParameters * accumulationParameters =
            new AccumulationParameters (rowInput, rowAveraged, doing_p);

    return accumulationParameters;
}

template<typename T>
void
VbAvg::accumulateElementForCube (const T & unweightedValue,
                                 Float weight,
                                 Bool zeroAccumulation,
                                 T & accumulator)
{
    // Update the sum for this model visibility cube element.

    T accumulation = zeroAccumulation ? 0
                                      : accumulator;

    accumulator = accumulation + unweightedValue * weight;
}


pair<Bool, Vector<Double> >
VbAvg::accumulateCubeData (MsRow * rowInput, MsRowAvg * rowAveraged)
{
    // Accumulate the sums needed for averaging of cube data (e.g., visibility).

    const Matrix<Bool> inputFlags = rowInput->flags ();
    Matrix<Bool> averagedFlags = rowAveraged->flagsMutable ();
    Matrix<Int> counts = rowAveraged->counts ();
    Vector<Bool> correlationFlagged = rowAveraged->correlationFlagsMutable ();

    AccumulationParameters * accumulationParameters = getAccumulationParameters (rowInput, rowAveraged);

    const Int nChannels = inputFlags.shape()(1);
    const Int nCorrelations = inputFlags.shape()(0);

    Vector<Double> adjustedWeight = Vector<Double> (nCorrelations, 0);
    Bool rowFlagged = True;  // True if all correlations and all channels flagged

    for (Int channel = 0; channel < nChannels; channel ++){

        for (Int correlation = 0; correlation < nCorrelations; correlation ++){

            // Based on the current flag state of the accumulation and the current flag
            // state of the correlation,channel, accumulate the data (or not).  Accumulate
            // flagged data until the first unflagged datum appears.  Then restart the
            // accumulation with that datum.

            Bool inputFlagged = inputFlags (correlation, channel);
            rowFlagged = rowFlagged && inputFlagged;
            Bool accumulatorFlagged = averagedFlags (correlation, channel);

            if (! accumulatorFlagged && inputFlagged){
                continue;// good accumulation, bad data so toss it.
            }

            // If changing from flagged to unflagged for this cube element, reset the
            // accumulation count to 1; otherwise increment the count.

            Bool flagChange = accumulatorFlagged != inputFlagged || // real flag change
                              counts (correlation, channel) == 0;   // first time

            if (flagChange){
                counts (correlation, channel) = 1;
            }
            else{
                counts (correlation, channel) += 1;
            }

            averagedFlags (correlation, channel) = accumulatorFlagged && inputFlagged;

            // Accumulate the sum for each cube element

            accumulateElementForCubes (accumulationParameters,
                                       flagChange, // zeroes out accumulation
                                       correlation,
                                       channel);

            // Handle the update of the weight column.  Although it's a row-level value it
            // depends on the flag matrix.  A weight is accumulated to a correlation only when
            // flag(correation,channel) is unflagged (except when we're accumulating data tbefore the
            // first unflagged value for a correlation is seen.

            Bool correlationFlagChanged = correlationFlagged (correlation) && ! inputFlagged;
            correlationFlagged (correlation) = correlationFlagged (correlation) && inputFlagged;

            if (! inputFlagged){

                Double weight = accumulationParameters->weightSpectrumIn (correlation, channel);
                adjustedWeight (correlation) += weight;
                accumulateElementForCube ((float) weight, 1.0f, correlationFlagChanged,
                                          accumulationParameters->weightOut (correlation));
            }

        }
    }

    delete accumulationParameters;

    return std::make_pair (rowFlagged, adjustedWeight);
}

void
VbAvg::accumulateElementForCubes (AccumulationParameters * accumulationParameters,
                                  Bool zeroAccumulation,
                                  Int correlation,
                                  Int channel)
{
    // Zero accumulation is used to restart the accumulation when the flagged state of the
    // element's accumulation changes from flagged to unflagged.

    // Get the weight to use for averaging.  If arithmetic averaging is being used the function
    // will return 1.0.  Otherwise if will return some function of the weight or weight-spectrum
    // (e.g., square, square-root, etc.).

    Double weight = accumulationParameters->weightSpectrumIn (correlation, channel);

    Float weightCorrected =  1.0f;

    if (doing_p.weightSpectrumCorrected_p){

        weightCorrected = accumulationParameters->weightSpectrumCorrectedIn (correlation, channel);

        accumulateElementForCube (weightCorrected,
                                  1.0f, zeroAccumulation,
                                  accumulationParameters->weightSpectrumCorrectedOut (correlation, channel));
    }

    // Update the sum for the three visibility cubes (corrected and model cubes only if present)

    if (doing_p.observedData_p){

        float weightToUse = 1.0f;

        if (! averagingOptions_p.contains (AveragingOptions::ObservedUseNoWeights)){
            weightToUse = weight;
        }

        accumulateElementForCube (accumulationParameters->observedIn (correlation, channel),
                                  weightToUse, zeroAccumulation,
                                  accumulationParameters->observedOut(correlation, channel));
    }

    if (doing_p.correctedData_p){

        float weightToUse = 1.0f;

        if (averagingOptions_p.contains (AveragingOptions::CorrectedUseCorrectedWeights)){
            weightToUse = weightCorrected;
        }
        else if (averagingOptions_p.contains (AveragingOptions::CorrectedUseWeights)){
            weightToUse = weight;
        }

        accumulateElementForCube (accumulationParameters->correctedIn (correlation, channel),
                                  weightToUse, zeroAccumulation,
                                  accumulationParameters->correctedOut (correlation, channel));
    }

    if (doing_p.modelData_p){

        float weightToUse = 1.0f;

        if (averagingOptions_p.contains (AveragingOptions::ModelUseCorrectedWeights)){
            weightToUse = weightCorrected;
        }
        else if (averagingOptions_p.contains (AveragingOptions::ModelUseWeights)){
            weightToUse = weight;
        }

        accumulateElementForCube (accumulationParameters->modelIn (correlation, channel),
                                  weightToUse, zeroAccumulation,
                                  accumulationParameters->modelOut (correlation, channel));
    }


    // The result of averaging for the weight spectrum is to sum up the weights rather
    // than average them.  The sum is over the raw weights (i.e., rather than optionally
    // applying a function to the weight as is done for other cube data).

    accumulateElementForCube ((float) weight, 1.0f, zeroAccumulation,
                              accumulationParameters->weightSpectrumOut (correlation, channel));

}

template <typename T>
T
VbAvg::accumulateRowDatum (const T & averagedValue, const T & inputValue, Bool resetAverage)
{
    if (resetAverage){
        return inputValue;
    }
    else{
        return inputValue + averagedValue;
    }
}

void
VbAvg::accumulateRowData (MsRow * rowInput, MsRowAvg * rowAveraged,
                          Double adjustedWeight, Bool rowFlagged)
{

    // Grab working copies of the values to be accumulated.

    Bool accumulatorRowFlagged = rowAveraged->isRowFlagged();
    Bool flagChange = accumulatorRowFlagged != rowFlagged || // actual change
                      rowAveraged->countsBaseline() == 0; // first time

    if (! accumulatorRowFlagged && rowFlagged){
        // good accumulation, bad data --> skip it
    }
    else{

        // Update the row's accumulations; if the flagChanged then zero out the
        // previous (flagged) accumulation first.

        rowAveraged->setCountsBaseline (accumulateRowDatum (rowAveraged->countsBaseline(),
                                                            1,
                                                            flagChange));

        // The WEIGHT column is handled under accumulateCubeData because of the
        // interrelationship between weight and weightSpectrum.  The SIGMA column is
        // handled in finalizeBaseline for similar reasons.

        accumulatorRowFlagged = accumulatorRowFlagged && rowFlagged;
        rowAveraged->setRowFlag (accumulatorRowFlagged);

        rowAveraged->setExposure (accumulateRowDatum (rowAveraged->exposure(),
                                                      rowInput->exposure (),
                                                      flagChange));

        // While accumulating flagged values, the weights will be zero, so accumulate
        // an arithmetic average until the accumulator becomes unflagged.

        Double weightToUse;

        if (accumulatorRowFlagged){
            weightToUse = 1;
        }
        else{
            weightToUse = adjustedWeight;
        }

        Double weightedTC = (rowInput->timeCentroid() - rowAveraged->timeFirst()) * weightToUse;
        rowAveraged->setTimeCentroid (accumulateRowDatum (rowAveraged->timeCentroid(),
                                                          weightedTC,
                                                          flagChange));

        Vector<Double> weightedUvw = rowInput->uvw() * weightToUse;
        rowAveraged->setUvw (accumulateRowDatum (rowAveraged->uvw (),
                                                 weightedUvw,
                                                 flagChange));

        // Capture a couple pieces of data

        rowAveraged->setTimeLast (rowInput->time());

        rowAveraged->setIntervalLast (rowInput->interval());
    }
}

//Vector<Float>
//VbAvg::adjustWeightForFlags (MsRow * rowInput)
//{
//    Matrix<Bool> flags = rowInput->flags();
//    Vector<Float> adjustedWeight = rowInput->weight();
//
//    for (Int correlation = 0; correlation < nCorrelations(); correlation++){
//
//        // Sum up the number of good channels in this correlation
//
//        Int sum = 0;
//
//        for (Int channel = 0; channel < nChannels(); channel ++){
//
//            if (! flags (correlation, channel)){
//
//                sum ++;
//            }
//        }
//
//        // Adjust the weight by multiplying by the fraction of good channels.
//
//        Float factor = ((float) sum) / nChannels();
//        adjustedWeight [correlation] *= factor;
//    }
//
//    return adjustedWeight;
//}

void
VbAvg::copyIdValues (MsRow * rowInput, MsRowAvg * rowAveraged)
{
    rowAveraged->setAntenna1 (rowInput->antenna1());
    rowAveraged->setAntenna2 (rowInput->antenna2());
    rowAveraged->setArrayId (rowInput->arrayId());
    rowAveraged->setDataDescriptionId (rowInput->dataDescriptionId());
    rowAveraged->setFeed1 (rowInput->feed1());
    rowAveraged->setFeed2 (rowInput->feed2());
    rowAveraged->setFieldId (rowInput->fieldId());
    rowAveraged->setProcessorId (rowInput->processorId());
    rowAveraged->setScanNumber (rowInput->scanNumber());
    rowAveraged->setSpectralWindow (rowInput->spectralWindow());
    rowAveraged->setObservationId (rowInput->observationId());
    rowAveraged->setStateId (rowInput->stateId());
}

void
VbAvg::copyIdValue (Int inputId, Int & averagedId)
{
    if (averagedId < 0){
        averagedId = inputId;
    }
}

Bool
VbAvg::empty () const
{
    return empty_p;
}

Int
VbAvg::getBaselineIndex (const MsRow * msRow) const
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

    Int antenna1 = msRow->antenna1 ();
    Int antenna2 = msRow->antenna2 ();
    Int spw = msRow->spectralWindow ();

    Int index = baselineIndex_p (antenna1, antenna2, spw);

    return index;
}

void
VbAvg::finalizeAverages ()
{
    if (empty_p){
        return; // nothing to finalize
    }

    MsRowAvg * msRowAvg = getRowMutable (0);

    for (Int baseline = 0; baseline < nBaselines(); baseline ++){

        msRowAvg->changeRow (baseline);

        if (msRowAvg->baselinePresent()){
            finalizeBaseline (msRowAvg);
        }

    }

    delete msRowAvg;

    empty_p = True;
}

void
VbAvg::finalizeBaseline (MsRowAvg * msRowAvg)
{
    // Software is no longer supposed to rely on the row flag.
    // Setting it to false insures that legacy software will
    // have to look at the flag cubes.

    msRowAvg->setRowFlag(False);

    finalizeCubeData (msRowAvg);

    finalizeRowData (msRowAvg);

    transferBaseline (msRowAvg);
}


void
VbAvg::finalizeCubeData (MsRowAvg * msRow)
{
    // Divide each of the data cubes in use by the sum of the appopriate weights.

    typedef Divides <Complex, Float, Complex> DivideOp;
    DivideOp op;

    if (doing_p.observedData_p){

        Matrix<Complex> observed = msRow->observedMutable();

        if (! averagingOptions_p.contains (AveragingOptions::ObservedUseNoWeights)){
            arrayTransformInPlace<Complex, Float, DivideOp > (observed,
                                                              msRow->weightSpectrum (), op);
        }
        else{
            arrayTransformInPlace<Complex, Int, DivideOp > (observed,
                                                            msRow->counts (), op);
        }
    }

    if (doing_p.correctedData_p){

        Matrix<Complex> corrected = msRow->correctedMutable();

        if (averagingOptions_p.contains (AveragingOptions::CorrectedUseWeights)){
            arrayTransformInPlace<Complex, Float, DivideOp > (corrected,
                                                              msRow->weightSpectrum (), op);
        }
        else if (averagingOptions_p.contains (AveragingOptions::CorrectedUseCorrectedWeights)){
            arrayTransformInPlace<Complex, Float, DivideOp > (corrected,
                                                              msRow->weightSpectrumCorrected (), op);
        }
        else{
            arrayTransformInPlace<Complex, Int, DivideOp > (corrected,
                                                            msRow->counts (), op);
        }
    }

    if (doing_p.modelData_p){

        Matrix<Complex> model = msRow->modelMutable();

        if (averagingOptions_p.contains (AveragingOptions::ModelUseWeights)){
            arrayTransformInPlace<Complex, Float, DivideOp > (model,
                                                              msRow->weightSpectrum (), op);
        }
        else if (averagingOptions_p.contains (AveragingOptions::ModelUseCorrectedWeights)){
            arrayTransformInPlace<Complex, Float, DivideOp > (model,
                                                              msRow->weightSpectrumCorrected (), op);
        }
        else{
            arrayTransformInPlace<Complex, Int, DivideOp > (model,
                                                            msRow->counts (), op);
        }
    }
}

float
weightToSigma (Float weight)
{
    return abs (weight) > 1e-9 ? 1.0 / std::sqrt (weight)
                               : -1; // bogosity indicator
}

void
VbAvg::finalizeRowData (MsRowAvg * msRow)
{
    Int n = msRow->countsBaseline ();

    Vector<Float> sigma;
    sigma = msRow->weight();
    arrayTransformInPlace (sigma, weightToSigma);
    msRow->setSigma (sigma);

    // Adjust the weights to zero out completely flagged correlations

    Double weight = 0;
    for (Int c = 0; c < nCorrelations(); c++){
        if (msRow->correlationFlagsMutable() (c)){
           msRow->setWeight (c, 0.0);
        }
        else{
            weight += msRow->weight (c);
        }
    }

    if (n != 0){

        if (weight == 0){

            // The weights are all zero so compute an arithmetic average
            // so that a somewhat value can go into these columns (i.e. rather than NaN).

            weight = msRow->countsBaseline();
        }

        msRow->setTimeCentroid (msRow->timeCentroid() / weight + msRow->timeFirst());

        msRow->setUvw (msRow->uvw() / weight);

        // Exposure is a simple sum, not an average so it is already
        // done at this point.
    }

    // Fill in the time and the interval
    //
    // The time of a sample is centered over the integration time period.
    // Thus find the center of the averaged interval it is necessary to
    // slide it back by 1/2 an interval.

    Double dT = msRow->timeLast () - msRow->timeFirst();

    Double centerOfInterval = msRow->timeFirst () + dT / 2;

    msRow->setTime (centerOfInterval);

    if (dT != 0){

        // The interval is the center-to-center time + half of the intervals of
        // the first and the last sample (if dT == 0 then the interval is
        // already the interval of the first sample and is correct)

        Double interval = dT + msRow->interval() / 2 + msRow->intervalLast() / 2;
        msRow->setInterval (interval);
    }
}

void
VbAvg::finalizeBaselineIfNeeded (MsRow * rowInput, MsRowAvg * rowAveraged, const Subchunk & /*subchunk*/)
{
    if (! rowAveraged->baselinePresent()){
        return;
    }

    // Finalization is needed if either the uvw distance or the time distance between the input
    // baseline and the averaged baseline is above the maximum

    Bool needed = usingUvwDistance_p;

    if (needed) {
        Double deltaUvw = distance (rowInput->uvw(), rowAveraged->uvwFirst ());
        needed = deltaUvw > maxUvwDistance_p;
    }

    needed = needed || (rowInput->time() - rowAveraged->timeFirst()) > maxTimeDistance_p;

    if (needed){

        // Finalize the data so that the final averaging products and then move them to
        // output buffer.

        finalizeBaseline (rowAveraged);
    }
}

MsRowAvg *
VbAvg::getRow (Int row) const
{
    return new MsRowAvg (row, this);
}

MsRowAvg *
VbAvg::getRowMutable (Int row)
{
    return new MsRowAvg (row, this);
}

void
VbAvg::initializeBaseline (MsRow * rowInput, MsRowAvg * rowAveraged,
                           const Subchunk & /*subchunk*/)
{
    copyIdValues (rowInput, rowAveraged);

    // Size and zero out the counters

    rowAveraged->setInterval (rowInput->interval()); // capture first one
    rowAveraged->setTimeFirst (rowInput->time());
    rowAveraged->setTimeLast (rowInput->time());
    rowAveraged->uvwFirst () = rowInput->uvw ();

    rowAveraged->setCountsBaseline (0);

    IPosition shape = rowInput->flags().shape();
    Int nCorrelations = shape (0);
    Int nChannels = shape (1);

    rowAveraged->setCounts (Matrix<Int> (nCorrelations, nChannels, 0));
    rowAveraged->setWeightSpectrum (Matrix<Float> (nCorrelations, nChannels, 0));
    rowAveraged->setWeight (Vector<Float> (nCorrelations, 0));
    rowAveraged->setTimeCentroid (0.0);

    if (doing_p.weightSpectrumCorrected_p){
        rowAveraged->setWeightSpectrumCorrected (Matrix<Float> (nCorrelations, nChannels, 0));
    }

//    VisBufferComponents2 exclusions =
//        VisBufferComponents2::these(VisibilityObserved, VisibilityCorrected,
//                                    VisibilityModel, CorrType, JonesC, Unknown);
//
//    cacheResizeAndZero(exclusions);

    // Flag everything to start with

    rowAveraged->setRowFlag (True); // only for use during row-value accumulation
    rowAveraged->setFlags(Matrix<Bool> (nCorrelations, nChannels, True));
    rowAveraged->correlationFlagsMutable() = Vector<Bool> (nCorrelations, True);

    rowAveraged->setBaselinePresent(True);
}


Bool
VbAvg::isComplete () const
{
    return complete_p;
}

//void
//VbAvg::markEmpty ()
//{
//    empty_p = True;
//    complete_p = False;
//}

Int
VbAvg::nBaselines () const
{
    return countsBaseline_p.nelements();
}

Int
VbAvg::nSpectralWindowsInBuffer () const
{
    const Vector<Int> & windows = spectralWindows();
    set <Int> s;

    for (uInt i = 0; i < windows.nelements(); i ++){
        s.insert (windows(i));
    }

    return (Int) s.size();

}


void
VbAvg::captureIterationInfo (VisBufferImpl2 * dstVb, const VisBuffer2 * srcVb,
                             const Subchunk & subchunk)
{
    dstVb->setIterationInfo (srcVb->msId(),
                             srcVb->msName(),
                             srcVb->isNewMs(),
                             srcVb->isNewArrayId(),
                             srcVb->isNewFieldId(),
                             srcVb->isNewSpectralWindow(),
                             subchunk,
                             srcVb->getCorrelationTypes (),
                             CountedPtr <WeightScaling> (0));

    // Request info from the VB which will cause it to be filled
    // into cache from the input VII at its current position.

    dstVb->setRekeyable(True);
    dstVb->setShape(srcVb->nCorrelations(), srcVb->nChannels(), nBaselines());

    dstVb->phaseCenter();
    dstVb->nAntennas();
    dstVb->correlationTypes();
    dstVb->polarizationFrame();
    dstVb->polarizationId();
}

//void
//VbAvg::prepareForFirstData (const VisBuffer2 * vb, const Subchunk & subchunk)
//{
//    startTime_p = vb->time() (0);
//    sampleInterval_p = vb->timeInterval() (0);
//
//    Int nAntennas = vb->nAntennas();
//    Int nSpw = vb->getVi()->nSpectralWindows();
//    Int nBaselines = ((nAntennas * (nAntennas + 1)) / 2) * nSpw;
//
//    // Size and zero out the counters
//
//    timeFirst_p = Vector<Double> (nBaselines, vb->time() (0));
//    timeLast_p = Vector<Double> (nBaselines, vb->time() (0));
//    uvwFirst_p = Vector<Double> (nBaselines, vb->uvw().column(0));
//
//    countsBaseline_p = Vector<Int> (nBaselines, 0);
//    counts_p = Cube<Int> (vb->nCorrelations(), vb->nChannels(), nBaselines, 0);
//    weightSum_p = Cube<Float> (vb->nCorrelations(), vb->nChannels(), nBaselines, 0);
//    if (doing_p.weightSpectrumCorrected_p){
//        weightCorrectedSum_p = Cube<Float> (vb->nCorrelations(), vb->nChannels(), nBaselines, 0);
//    }
//
//    baselineIndex_p.configure (nAntennas, nSpw, vb);
//
//    // Reshape the inherited members from VisBuffer2
//
//    captureIterationInfo (vb, subchunk);
//
//    setShape (vb->nCorrelations(), vb->nChannels(), nBaselines, False);
//
//    VisBufferComponents2 exclusions =
//        VisBufferComponents2::these(VisibilityObserved, VisibilityCorrected,
//                                    VisibilityModel, CorrType, JonesC, Unknown);
//    cacheResizeAndZero(exclusions);
//
//    prepareIds (vb);
//
//    // Flag everything to start with
//
//    setFlagCube (Cube<Bool> (vb->nCorrelations(), vb->nChannels(), nBaselines, True));
//    setFlagRow (Vector<Bool> (nBaselines, True));
//
//    complete_p = False;
//}

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

//void
//VbAvg::removeMissingBaselines ()
//{
//    // Some baselines may not be present in the portion of the input data
//    // that made up this average.  However, this is not known until after
//    // all of the data is seen.  Thus at finalization time these bogus
//    // baselines should be removed from the average so as not to pass
//    // flagged but zero-exposure baselines into the output.
//
//
//    Vector<Int> rowsToDelete (nBaselines());
//
//    Int nBaselinesDeleted = 0;
//
//    for (Int baseline = 0; baseline < nBaselines(); baseline ++){
//
//        if (countsBaseline_p (baseline) == 0){
//            rowsToDelete (nBaselinesDeleted) = baseline;
//            nBaselinesDeleted ++;
//        }
//    }
//
//    rowsToDelete.resize (nBaselinesDeleted, True);
//
//    deleteRows (rowsToDelete);
//}

void
VbAvg::setBufferToFill(VisBufferImpl2 * vb)
{
    bufferToFill_p = vb;

    // Set flag so that iteration information will be captured into
    // this VB from the first input VB.

    needIterationInfo_p = True;
}

void
VbAvg::setupVbAvg (const VisBuffer2 * vb)
{
    // Configure the index

    Int nAntennas = vb->nAntennas();
    Int nSpw = vb->getVi()->nSpectralWindows();

    baselineIndex_p.configure (nAntennas, nSpw, vb);

    Int nBaselines = ((nAntennas * (nAntennas + 1)) / 2) * nSpw;

    setShape (vb->nCorrelations(), vb->nChannels(), nBaselines);

    setupArrays (vb->nCorrelations(), vb->nChannels(), nBaselines);

    baselinePresent_p = Vector<Bool> (nBaselines, False);

    empty_p = False;
}

void
VbAvg::setupArrays (Int nCorrelations, Int nChannels, Int nBaselines)
{
    setShape (nCorrelations, nChannels, nBaselines);

    // Start out with all of the array-valued components except the
    // optional ones.

    VisBufferComponents2 including =
            VisBufferComponents2::these (Antenna1,
                                         Antenna2,
                                         ArrayId,
                                         CorrType,
                                         DataDescriptionIds,
                                         Exposure,
                                         Feed1,
                                         Feed2,
                                         FieldId,
                                         FlagCategory,
                                         FlagCube,
                                         FlagRow,
                                         ObservationId,
                                         ProcessorId,
                                         RowIds,
                                         Scan,
                                         Sigma,
                                         SpectralWindows,
                                         StateId,
                                         Time,
                                         TimeCentroid,
                                         TimeInterval,
                                         Weight,
                                         WeightSpectrum, // optional column but used internally
                                         Uvw,
                                         Unknown);

    if (doing_p.modelData_p){
        including += VisibilityCubeModel;
    }

    if (doing_p.correctedData_p){
        including += VisibilityCubeCorrected;
    }

    if (doing_p.observedData_p){
        including += VisibilityCubeObserved;
    }

    if (doing_p.weightSpectrumCorrected_p){
        including += WeightSpectrumCorrected;
    }

    cacheResizeAndZero (including);

    correlationFlags_p.reference (Matrix<Bool> (IPosition (2, nCorrelations, nBaselines), True));
    counts_p.reference (Cube<Int> (IPosition (3, nCorrelations, nChannels, nBaselines), 0));
    countsBaseline_p.reference (Vector<Int> (nBaselines, 0)); // number of items summed together for each baseline.
    intervalLast_p.reference (Vector<Double> (nBaselines, 0));
    timeFirst_p.reference (Vector<Double> (nBaselines, 0));
    timeLast_p.reference (Vector<Double> (nBaselines, 0));
    uvwFirst_p.reference (Matrix<Double> (IPosition (2, 3, nBaselines), 0.0));
}

void
VbAvg::startChunk (ViImplementation2 * vi)
{
    empty_p = True;

    // See if the new MS has corrected and/or model data columns

    doing_p.observedData_p = averagingOptions_p.contains (AveragingOptions::AverageObserved);
    doing_p.correctedData_p = vi->existsColumn (VisibilityCubeCorrected) &&
                              averagingOptions_p.contains (AveragingOptions::AverageCorrected);
    doing_p.modelData_p = vi->existsColumn (VisibilityCubeModel) &&
                          averagingOptions_p.contains (AveragingOptions::AverageModel);
    doing_p.weightSpectrum_p = vi->existsColumn (WeightSpectrum);

    // If the use of corrected weights were specified for one of the averages, it's an
    // error if the column does not exist.  Also set the doing flag for corrected weights
    // if it's being used in some way.

    Bool needCorrectedWeights =
            averagingOptions_p.contains (AveragingOptions::ModelUseCorrectedWeights) ||
            averagingOptions_p.contains (AveragingOptions::CorrectedUseCorrectedWeights);

    Bool correctedWeightsMissing = needCorrectedWeights &&
            ! vi->existsColumn (WeightSpectrumCorrected);

    ThrowIf (correctedWeightsMissing,
             "Corrected_weight_spectrum not present but required by provided averaging options");

    doing_p.weightSpectrumCorrected_p = needCorrectedWeights;

    // Set up the flags for row copying

    optionalComponentsToCopy_p = VisBufferComponents2::none();

    if (doing_p.observedData_p){
        optionalComponentsToCopy_p += VisibilityCubeObserved;
    }

    if (doing_p.correctedData_p){
        optionalComponentsToCopy_p += VisibilityCubeCorrected;
    }

    if (doing_p.modelData_p){
        optionalComponentsToCopy_p += VisibilityCubeModel;
    }

    if (doing_p.weightSpectrum_p){
        optionalComponentsToCopy_p += WeightSpectrum;
    }

    if (doing_p.weightSpectrumCorrected_p){
        optionalComponentsToCopy_p += WeightSpectrumCorrected;
    }
}

void
VbAvg::transferBaseline (MsRowAvg * rowAveraged)
{
    rowAveraged->setRowId (rowIdGenerator_p ++);
    bufferToFill_p->appendRow (rowAveraged, nBaselines (), optionalComponentsToCopy_p);

    rowAveraged->setBaselinePresent(False);
}

//VbSet::VbSet (const AveragingParameters & averagingParameters)
//: averagingInterval_p (averagingParameters.getAveragingInterval ()),
//  averagingOptions_p (averagingParameters.getOptions()),
//  averagingParameters_p (averagingParameters),
//  doingCorrectedData_p (False),
//  doingModelData_p (False),
//  doingObservedData_p (False),
//  doingWeightSpectrum_p (False),
//  doingWeightSpectrumCorrected_p (False),
//  vbAveragers_p ()
//{}

//VbSet::~VbSet ()
//{
//    flush (True); // allow killing nonempty buffers
//}
//
//void
//VbSet::accumulate (const VisBuffer2 * input, const Subchunk & subchunk)
//{
//    Int ddId = input->dataDescriptionIds()(0);
//
//    if (! utilj::containsKey (ddId, vbAveragers_p)){ // Need a new averager
//        add (ddId);
//    }
//
//    VbAvg * vba = vbAveragers_p [ddId];
//
//    vba->accumulate (input, subchunk);
//}
//
//VbAvg *
//VbSet::add (Int ddId)
//{
//    VbAvg::Doing doing;
//
//    doing.correctedData_p = doingCorrectedData_p;
//    doing.modelData_p = doingModelData_p;
//    doing.observedData_p = doingObservedData_p;
//    doing.weightSpectrum_p = doingWeightSpectrum_p;
//    doing.weightSpectrumCorrected_p = doingWeightSpectrumCorrected_p;
//
//    VbAvg * newAverager =  new VbAvg (doing, averagingParameters_p);
//
//    vbAveragers_p [ddId] = newAverager;
//
//    return newAverager;
//}
//
//Bool
//VbSet::anyAveragesReady(Int ddid) const
//{
//    Bool any = False;
//
//    for (Averagers::const_iterator a = vbAveragers_p.begin();
//         a != vbAveragers_p.end();
//         a++){
//
//        if (a->second->isComplete() &&
//            (ddid < 0 || ddid == a->second->dataDescriptionIds()(0))){
//
//            any = True;
//            break;
//        }
//    }
//
//    return any;
//}
//
//Bool
//VbSet::contains (Int ddId) const
//{
//    return vbAveragers_p.find (ddId) != vbAveragers_p.end();
//}
//
////void
////VbSet::finalizeAverage (Int ddId)
////{
////    vbAveragers_p [ddId]->finalizeAverage();
////}
//
//void
//VbSet::finalizeAverages ()
//{
////    for (Averagers::iterator a = vbAveragers_p.begin();
////         a != vbAveragers_p.end();
////         a ++){
////
////        finalizeAverage (a->first);
////    }
//}
//
//void
//VbSet::flush (Bool okIfNonempty, ViImplementation2 * vi)
//{
//    // Get rid of all of the averagers.  This is done at
//    // destruction time and when a sweeping into a new MS.
//
//    for (Averagers::const_iterator a = vbAveragers_p.begin();
//         a != vbAveragers_p.end();
//         a ++){
//
//        Assert (okIfNonempty || (a->second)->empty());
//            // should have been emptied before calling this
//
//        delete a->second;
//    }
//
//    vbAveragers_p.clear();
//
//    seeIfCubeColumnsExist (vi);
//}
//
//void
//VbSet::seeIfCubeColumnsExist (ViImplementation2 * vi)
//{
//    if (vi != 0){
//
//        // See if the new MS has corrected and/or model data columns
//
//        doingObservedData_p = averagingOptions_p.contains (AveragingOptions::AverageObserved);
//        doingCorrectedData_p = vi->existsColumn (VisibilityCubeCorrected) &&
//                               averagingOptions_p.contains (AveragingOptions::AverageCorrected);
//        doingModelData_p = vi->existsColumn (VisibilityCubeModel) &&
//                               averagingOptions_p.contains (AveragingOptions::AverageModel);
//        doingWeightSpectrum_p = vi->existsColumn (WeightSpectrum);
//
//        // If the use of corrected weights were specified for one of the averages, it's an
//        // error if the column does not exist.  Also set the doing flag for corrected weights
//        // if it's being used in some way.
//
//        Bool needCorrectedWeights =
//            averagingOptions_p.contains (AveragingOptions::ModelUseCorrectedWeights) ||
//            averagingOptions_p.contains (AveragingOptions::CorrectedUseCorrectedWeights);
//
//        Bool correctedWeightsMissing = needCorrectedWeights &&
//                                       ! vi->existsColumn (WeightSpectrumCorrected);
//
//        ThrowIf (correctedWeightsMissing,
//                 "Corrected_weight_spectrum not present but required by provided averaging options");
//
//        doingWeightSpectrumCorrected_p = needCorrectedWeights;
//    }
//}
//
//Int
//VbSet::getFirstReadyDdid () const
//{
//    for (Averagers::const_iterator a = vbAveragers_p.begin();
//         a != vbAveragers_p.end();
//         a ++){
//
//        if (a->second->isComplete()){
//            return a->first;
//        }
//    }
//
//    Assert (False); // ought to have been one that's ready
//
//    return -1; // shouldn't be called
//}
//
////void
////VbSet::transferAverage (Int ddId, VisBuffer2 * vb)
////{
////    Assert (utilj::containsKey (ddId, vbAveragers_p));
////
////    VbAvg * vba = vbAveragers_p [ddId];
////
////    Assert (vba != 0 && ! vba->empty ());
////
////    // Copy the completed average into the provided VisBuffer, but
////    // first reshape the VB if it's shape is different.
////
////    vba->transferAverage (vb);
////
////}
//
//void
//VbSet::zero ()
//{
//    for (Averagers::const_iterator a = vbAveragers_p.begin();
//         a != vbAveragers_p.end();
//         a ++){
//
//        a->second->markEmpty();
//    }
//}

    ///////////////////////
    //                   //
    // End Namespace avg //
    //                   //
    ///////////////////////

} // end avg

using namespace avg;

AveragingTvi2::AveragingTvi2 (VisibilityIterator2 * vi,
                              ViImplementation2 * inputVi,
                              const AveragingParameters & averagingParameters)
: TransformingVi2 (vi, inputVi),
  averagingInterval_p (averagingParameters.getAveragingInterval()),
  averagingOptions_p (averagingParameters.getOptions()),
  averagingParameters_p (averagingParameters),
  ddidLastUsed_p (-1),
  inputViiAdvanced_p (False),
  vbAvg_p (new VbAvg (averagingParameters))
{

    validateInputVi (inputVi);

    // Position input Vi to the first subchunk

    getVii()->originChunks();
    getVii()->origin ();

    setVisBuffer (VisBuffer2::factory (vi, VbPlain, VbNoOptions));
}

AveragingTvi2::~AveragingTvi2 ()
{
    delete vbAvg_p;
}

//void
//AveragingTvi2::advanceInputVii ()
//{
//    Assert (False);
//
//    // Attempt to advance to the next subchunk
//
//    getVii()->next ();
//
//    if (! getVii()->more()){
//
//        // No more subchunks so advance to the next chunk
//
//        getVii()->nextChunk();
//
//        if (! getVii()->moreChunks()){
//            return; // no more chunks
//        }
//
//        // Position to the first subchunk
//
//        getVii()->origin();
//    }
//}

//Int
//AveragingTvi2::determineDdidToUse() const
//{
//    if (ddidLastUsed_p >= 0 && vbSet_p->anyAveragesReady (ddidLastUsed_p)){
//        return ddidLastUsed_p;
//    }
//
//    return vbSet_p->getFirstReadyDdid();
//}

Bool
AveragingTvi2::more () const
{
    return more_p;
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

    if (getVii()->more()){
        getVii()->next();
    }

    produceSubchunk ();

    subchunk_p.incrementSubChunk();
}

void
AveragingTvi2::nextChunk ()
{
    // New chunk, so toss all of the accumulators

    vbAvg_p->startChunk (getVii());

    // Advance the input to the next chunk as well.

    getVii()->nextChunk ();

    subchunk_p.incrementChunk();

    more_p = False;
}

void
AveragingTvi2::origin ()
{
    // Position input VI to the start of the chunk

    subchunk_p.resetSubChunk();

    getVii()->origin();

    // Get the first subchunk ready.

    produceSubchunk ();
}

void
AveragingTvi2::originChunks (Bool forceRewind)
{
    vbAvg_p->startChunk (getVii());

    getVii()->originChunks(forceRewind);
    more_p = False;

    subchunk_p.resetToOrigin();
}

void
AveragingTvi2::produceSubchunk ()
{
    VisBufferImpl2 * vbToFill = dynamic_cast<VisBufferImpl2 *> (getVisBuffer());
    assert (vbToFill != 0);

    vbToFill->setFillable (True); // New subchunk, so it's fillable

    vbAvg_p->setBufferToFill (vbToFill);

    Int nBaselines = nAntennas() * (nAntennas() -1) / 2;
        // This is just a heuristic to keep output VBs from being too small

    while (getVii()->more()){

        const VisBuffer2 * vb = getVii()->getVisBuffer();

        vbAvg_p->accumulate (vb, subchunk_p);
        Int nWindows = vbAvg_p->nSpectralWindowsInBuffer ();

        if (vbToFill->appendSize() < nBaselines * nWindows){
            getVii()->next();
        }
        else{
            break;
        }
    };

    if (! getVii()->more()){

        vbAvg_p->finalizeAverages ();
    }

    vbAvg_p->finalizeBufferFilling ();

    more_p = getVii()->more() || // more to read
             vbToFill->nRows() > 0; // some to process

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

//Bool
//AveragingTvi2::subchunksReady() const
//{
//    Bool ready = vbSet_p->anyAveragesReady();
//
//    return ready;
//}

void
AveragingTvi2::validateInputVi (ViImplementation2 *)
{
    // Validate that the input VI is compatible with this VI.
  //#warning "Implement AveragingTvi2::validateInputVi"
}

} // end namespace vi

} // end namespace casa
