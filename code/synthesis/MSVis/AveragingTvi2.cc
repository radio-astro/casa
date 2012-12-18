#include <synthesis/MSVis/AveragingTvi2.h>
#include <synthesis/MSVis/VisBufferImpl2.h>
#include <synthesis/MSVis/UtilJ.h>

namespace casa {

namespace vi {

namespace { // unamed namespace

class VbAvg : public VisBufferImpl2 {

public:

    VbAvg (const VisBuffer2 * vb);

    void accumulate (VisBuffer2 * vb);
    void finalizeAverage ();
    Bool vbPastInterval (VisBuffer2 * vb) const;

protected:

    Bool accumulateCubeData (VisBuffer2 * vb, Int baselineIndex, Int correlation,
                             Int channel, Int row, Bool clearAccumulation);
    Int getBaselineIndex (Int antenna1, Int antenna2) const;
    Int nBaselines () const;
    void setupBaselineIndices (Int nAntennas);

private:

    Matrix<Int> baselineIndices_p;
    Cube<Int> counts_p;
    Vector<Int> countsBaseline_p;

};

class VbSet {

public:

    Bool accumulate (Int ddId, VisBuffer2 *);
    void add (Int ddId, VisBuffer2 * vb);
    Bool contains (Int ddId) const;

protected:


private:

    typedef map<Int, VbAvg *> Map;

    Map map_p;
};

VbAvg::VbAvg (VisBuffer2 * vb)
{
    Int nAntennas = vb->nAntennas();
    Int nBaselines = (nAntennas * (nAntennas + 1)) / 2;

    // Size and zero out the counters

    countsBaseline_p = Vector<Int> (nBaselines, 0);
    counts_p = Cube<Int> (vb->nCorrelations(), vb->nChannels(), nBaselines, 0);

    setupBaselineIndices (nAntennas);

    // Reshape the inherited members from VisBuffer2

    setShape (vb->nCorrelations(), vb->nChannels(), nBaselines);

    // Flag everything to start with

    setFlagCube (Cube<Bool> (vb->nCorrelations(), vb->nChannels(), nBaselines, True));
    setFlagRow (Vector<Bool> (nBaselines()));

}

VbAvg::nBaselines () const
{
    return countsBaseline_p.nelements();
}

void
VbAvg::setupBaselineIndices(int nAntennas)
{
    // Create a square matrix to hold the mappings between pairs of
    // antennas to the baseline number.  This includes autocorrelations.
    // Only the lower left half of the matrix holds valid baseline indices;
    // the upper right matrix will hold -1s.
    //
    // The decoding routine, getBaselineIndex will throw an exception if
    // an antenna1,antenna2 pair maps would return a -1.

    baselineIndices_p.resize (nAntennas, nAntennas, False);
    Int baseline = 0;

    for (Int row = 0; row < nAntennas; row ++){

        for (Int column = 0; column < nAntennas; column ++){

            baselineIndices_p (row, column) = (row <= column) ? baseline ++ : -1;

        }
    }
}

void
VbAvg::accumulate (VisBuffer2 * vb)
{
    // Averaging can be computed as flagged or unflagged.  If all the inputs to a
    // datum are flagged, then the averaged datum (e.g., a visibility point)
    // will also be flagged.  For an unflagged averaged datum, it will represent
    // the average of all of the unflagged points for that datum.  This is done
    // by assuming the accumulation is flagged and continuing to accumulate
    // flagged points until the first unflagged point for a datum is encountered;
    // when this happens the count is zeroed and the averaged datum's flag is cleared.

    Bool allFlagged = True;
    const Vector<Int> & antenna1 = vb->antenna1();
    const Vector<Int> & antenna2 = vb->antenna2();
    const Vector<Bool> & flagRow = vb->flagRow();
    const Cube<Bool> & flagCube = vb->flagCube();
    const Cube<Bool> & thisFlagCube = vb->flagCubeRef();

#warning "Implement weight and sigma averaging"

    // Loop over all of the rows in the VB.  Map each one to a baseline and then accumulate
    // the values for each correlation,channel cell.  Each row in the accumulating VB corresponds
    // to one baseline (i.e., pair of (antenna1, antenna2) where antenna1 <= antenna2).

    ThrowIf (nRows > nBaselines(),
             String::format ("Expected %d baselines in VisBuffer but it contained %d rows",
                             nBaselines(), nRows));

    for (Int row = 0; row < vb->nRows(); row ++){

        Bool inputRowFlagged = flagRow (row);

        allFlagged = False;

        Int baselineIndex = getBaselineIndex (antenna1 [row], antenna2 [row]);

        accumulateRowData (vb, baselineIndex, row);

        for (Int channel = 0; channel < nChannels; channel ++){

            for (Int correlation = 0; correlation < nCorrelations; correlation ++){

                // Shortcuts to values; note that two of these are mutable references.

                Bool inputFlagged = flagCubeRef (correlation, channel, row) || inputRowFlagged;
                Bool & accumulatorFlagged = thisFlagCube (correlation, channel, baselineIndex);
                Int & count = counts_p (correlation, channel, baselineIndex);

                // Based on the current flag state of the accumulation and the current flag
                // state of the correlation,channel, accumulate the data (or not).

                if (! accumulatorFlagged && inputFlagged){
                    // good accumulation, bad data so toss it.
                }
                else{
                    Bool flagChange = accumulatorFlagged != inputFlagged;
                    count = flagChange ? count + 1 : 1;
                    accumulatorFlagged = accumulatorFlagged && inputFlagged;

                    accumulateCubeData (vb, baselineIndex, correlation, channel, row, flagChange);
                }
            }
        }
    }
}

void
VbAvg::accumulateCubeData (const VisBuffer2 * input, Int baselineIndex, Int correlation,
                           Int channel, Int row, Bool zeroAccumulation)
{

    Complex sum = zeroAccumulation ? Complex (0,0) : visCube() (correlation, channel, baseline);
    sum += input->visCube () (correlation, channel, row);
    visCubeRef ()(correlation, channel, baseline) = sum;

    if (doingCorrectedData){
        Complex sum = zeroAccumulation ? Complex (0,0) : visCubeCorrected() (correlation, channel, baseline);
        sum += input->visCubeCorrected () (correlation, channel, row);
        visCubeCorrectedRef ()(correlation, channel, baseline) = sum;
    }

    if (doingModelData){
        Complex sum = zeroAccumulation ? Complex (0,0) : visCubeModel() (correlation, channel, baseline);
        sum += input->visCubeModel () (correlation, channel, row);
        visCubeModelRef ()(correlation, channel, baseline) = sum;
    }

}

void
VbAvg::accumulateRowData (const VisBuffer2 * input, Int baselineIndex, Int row, Bool inputFlagged)
{

    Bool & accumulatorRowFlagged = flagRowRef ()(baselineIndex);

    if (! accumulatorRowFlagged && inputFlagged){
        // good accumulation, bad data --> skip it
    }
    else{

        Bool flagChange = accumulatorRowFlagged != inputFlagged;

        Int & count = count_p (baselineIndex);
        count = flagChange ? count + 1 : 1;

        accumulatorRowFlagged = accumulatorRowFlagged && inputFlagged;

        // Accumulate UVW

        // Accumulate Time Centroid

        // Accumulate Exposure

    }
}


Int
VbAvg::getBaselineIndex (Int antenna1, Int antenna2) const
{
    // Lookup the baseline index using the prebuilt lookup table.
    //
    // The baseline index is the index in the sequence {(0,0),(1,0),(1,1),(2,0),(2,1),(2,2), ...}
    // (i.e., the index in a 1-d representation of the lower right half of the square matrix of size
    // nAntennas.

    Int index = baselineIndices_p (antenna1, antenna2);

    ThrowIf (index >= 0, String::format ("Unexpected baseline having antenna pair (%d,%d)",
                                         antenna1, antenna2));
        // We expect antenna1 <= antenna2 in CASA-filled vis data

    return index;
}


Int
VbSet::accumulate (Int ddId, VisBuffer2 * input)
{
    VbAvg * vba = map_p [ddId];

    if (vba->vbPastInterval (input)){

        vba->finalizeAverage ();

        return CompleteButInputUnused;
    }

    vba->accumulate (input);
}


void
VbSet::add (Int ddId, const VisBuffer2 * vb)
{
    map [ddId] = new VbAvg (vb);
}

Bool
VbSet::contains (Int ddId)
{
    return map_p.find (ddId) != map_p.end();
}

} // end unnamed namespace

AveragingTvi2::AveragingTvi2 (ViImplementation2 * inputVi)
: TransformingVi2 (inputVi)
{
}

AveragingTvi2::~AveragingTvi2 (ViImplementation2 * inputVi)
{
}

void
AveragingTvi2::ingestData ()
{
    Bool needMoreInputData = True;

    while (needMoreInputData){

        if (! inputViiAdvanced){

            advanceInputVii ();
            inputViiAdvanced_p = True;
        }


        if (reachedAveragingBoundary ()){
            break;
        }

        needMoreInputData = processInputSubchunk ();

        inputViiAdvanced_p = False;
    }
}

void
AveragingTvi2::next ()
{
}

void
AveragingTvi2::nextChunk ()
{
}

Bool
AveragingTvi2::more () const
{
}

Bool
AveragingTvi2::moreChunks () const
{
}

void
AveragingTvi2::origin ()
{
}

void
AveragingTvi2::originChunks ()
{
}

Bool
AveragingTvi2::processInputSubchunk ()
{
    VisBuffer2 * vb = getVii()->getVisBuffer();

    Int ddId = vb->dataDescriptionId();

    if (! vbSet_p->contains (ddId)){
        vbSet_p->add (ddId);
    }

    Bool completedAverage = vbSet_p->accumulate (ddId, vb);

    return completedAverage;
}

} // end namespace vi

} // end namespace casa
