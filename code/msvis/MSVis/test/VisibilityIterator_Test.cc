#include "VisibilityIterator_Test.h"

#include <casa/aips.h>
#include <casa/BasicSL.h>
#include <ms/MeasurementSets.h>
#include <tables/Tables.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableCopy.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/TiledDataStMan.h>
#include <tables/Tables/TiledShapeStMan.h>
#include <ms/MeasurementSets/MSAntenna.h>
#include <ms/MeasurementSets/NewMSSimulator.h>
#include <measures/Measures/MeasTable.h>
#include <msvis/MSVis/UtilJ.h>
#include <msvis/MSVis/VisBuffer.h>
#include <msvis/MSVis/ViFrequencySelection.h>
#include <msvis/MSVis/VisibilityIterator2.h>
#include <msvis/MSVis/VisibilityIteratorImpl2.h>
#include <msvis/MSVis/FinalTvi2.h>
#include <msvis/MSVis/VisBuffer2.h>
#include <msvis/MSVis/SubMS.h>
#include <msvis/MSVis/test/MsFactory.h>
#include <boost/tuple/tuple.hpp>

using namespace std;
using namespace casa;
using namespace casa::vi;

int
main (int nArgs, char * args [])
{
    using namespace casa::vi::test;

    Tester tester;

    bool allPassed = tester.doTests (nArgs, args);

    CopyMs msCopier;
    msCopier.doit ("ngc5921.ms");
    return 0;

//    MsFactory msf ("test.ms");
//
//    casa::MeasurementSet * ms;
//    int nRows;
//    boost::tie (ms, nRows) = msf.createMs ();
//
//    printMs (ms);

    return allPassed ? 0 : 1;
}

namespace casa {
namespace vi {

//    VisibilityIteratorImpl2 (VisibilityIterator2 * rovi,
//                             const Block<MeasurementSet> & mss,
//                             const Block<Int> & sortColumns,
//                             Bool addDefaultSortCols,
//                             Double timeInterval,
//                             VisBufferType vbType,
//                             Bool isWritable);


VisibilityIterator2 *
VisibilityIterator2::copyingViFactory (const MeasurementSet & srcMs,
                                       MeasurementSet & dstMs)
{
    VisibilityIterator2 * vi = new VisibilityIterator2 ();

    Block <const MeasurementSet *> mss (1, & srcMs);
    Block <Int> sortColumns;

    VisibilityIteratorImpl2 * viReader = new VisibilityIteratorImpl2 (vi, mss, SortColumns(),
                                                                      0, VbPlain, False);
    FinalTvi2 * fVi = new FinalTvi2 (viReader, vi, dstMs, False);

    vi->impl_p = fVi;

    return vi;
}

namespace test {

template <typename T>
void
printMsCubePart (Int row, const Cube<T> & array, const String & name)
{
    IPosition shape = array.shape();

    for (Int i = 0; i < shape (1); i++){
        printf ("%s [0, %2d]: ", name.c_str(), i);
        for (Int j = 0; j < shape (0); j++){

            printf ("%8.0f", array (j, i, row));

        }
        printf ("\n");
    }

    if (array.nelements () == 0){
        printf ("%s is empty\n", name.c_str());
    }
    printf ("   ----------\n");
}

template <typename>
void
printMsCubePart (Int row, const Cube<Bool> & array, const String & name)
{
    IPosition shape = array.shape();

    for (Int i = 0; i < shape (1); i++){
        printf ("%s [0, %2d]: ", name.c_str(), i);
        for (Int j = 0; j < shape (0); j++){

            printf ("%2s", array (j, i, row) ? "T" : "F");

        }
        printf ("\n");
    }

    if (array.nelements () == 0){
        printf ("%s is empty\n", name.c_str());
    }
    printf ("   ----------\n");
}

template <>
void
printMsCubePart (Int row, const Cube<Complex> & array, const String & name)
{
    IPosition shape = array.shape();

    for (Int i = 0; i < shape (1); i++){
        printf ("%s [0, %2d]: ", name.c_str(), i);
        for (Int j = 0; j < shape (0); j++){

            Complex z = array (j, i, row);

            printf ("(%8.0f,%8.0f) ", z.real(), z.imag());

        }
        printf ("\n");
    }

    if (array.nelements () == 0){
        printf ("%s is empty\n", name.c_str());
    }
    printf ("   ----------\n");
}


void
printMs (MeasurementSet * ms)
{
    Block<Int> noSortColumns;
    VisibilityIterator vi (* ms, noSortColumns);
    VisBuffer vb;
    vb.attachToVisIter (vi);

    for (vi.originChunks (); vi.moreChunks(); vi.nextChunk()){

        for (vi.origin(); vi.more (); vi ++){

            printf ("\n");

            for (int i = 0; i < vb.nRow(); i++){

                printf ("r=%d ", vb.rowIds () [i]);
                printf ("t=%.0f ", vb.time () [i]);
                printf ("a1=%d ", vb.antenna1() [i]);
                printf ("a2=%d ", vb.antenna2 () [i]);
                printf ("dd=%d ", vb.dataDescriptionId());
                printf ("sp=%d ", vb.spectralWindow());
                printf ("fld=%d ", vb.fieldId());
                printf ("tc=%3.1f ", vb.timeCentroid() [i]);
                printf ("\n");
                printMsCubePart (i, vb.visCube(), "vis");
                printMsCubePart (i, vb.weightCube(), "wtSpec");
                printMsCubePart<Bool> (i, vb.flagCube(), "flag");
                //printf ("", vb. () [i]);

            }
        }
    }
}

std::pair<Bool,Bool>
Tester::sweepMs (TestWidget & tester)
{
    Block<const MeasurementSet *> mss;
    pair<Bool,Bool> result;
    Bool moreSweeps = False;
    Subchunk subchunk;

    try {

        Int nRows;
        Bool writableVi;
        if (tester.usesMultipleMss()){
            boost::tie (mss, nRows, writableVi) = tester.createMss ();
        }
        else{
            MeasurementSet * ms;
            boost::tie (ms, nRows, writableVi) = tester.createMs ();
            mss = Block<const MeasurementSet *> (1, ms);
        }

        do {
            VisibilityIterator2 * vi = 0;

            if (tester.usesMultipleMss()){
                vi = new VisibilityIterator2 (mss, SortColumns (), writableVi);
            }
            else {
                vi = new VisibilityIterator2 (* mss[0], SortColumns (), writableVi);
            }

            VisBuffer2 * vb = vi->getVisBuffer ();
            Int nRowsProcessed = 0;

            tester.startOfData (* vi, vb);

            for (vi->originChunks (); vi->moreChunks(); vi->nextChunk()){

                tester.nextChunk (* vi, vb);

                for (vi->origin(); vi->more (); vi->next()){

                    subchunk = vi->getSubchunkId();

                    nRowsProcessed += vb->nRows();

                    tester.nextSubchunk (* vi, vb);

                }
                tester.endOfChunk (* vi, vb);
            }

            moreSweeps = tester.noMoreData (* vi, vb, nRowsProcessed);

        } while (moreSweeps);


        result = make_pair (True, False);
    }
    catch (TestError & e){

        fprintf (stderr, "*** TestError at subchunk %s while executing test %s:\n-->%s\n",
                 subchunk.toString().c_str(), tester.name ().c_str(), e.what());
        result = make_pair (False, False);
    }
    catch (AipsError & e){

        fprintf (stderr, "*** AipsError while executing test %s:\n-->%s\n",
                 tester.name ().c_str(), e.what());

        result = make_pair (False, False);
    }
    catch (...){

        fprintf (stderr, "*** Unknown exception while executing test %s\n***\n*** Exiting ***\n",
                 tester.name ().c_str());
        result = make_pair (False, True);
    }

    fflush (stderr); // just in case things are really bad and we croak

    for (uInt i = 0; i < mss.size(); i++){
        delete mss [i];
    }

    return result;
}


template <typename T>
bool
Tester::doTest ()
{
    T t;

    nTestsAttempted_p ++;

    bool ok;
    bool fatal;

    printf ("Performing %s ...\n", t.name().c_str());
    fflush (stdout);
    boost::tie (ok, fatal) = sweepMs (t);
    String result = (ok ? "Passed" : (fatal ? "FATAL ERROR" : "FAILED"));
    printf ("... %s test %s\n", result.c_str(), t.name().c_str());
    fflush (stdout);

    if (ok){
        nTestsPassed_p ++;
    }

    TestErrorIf (fatal, "No message");

    return ok;
}

bool
Tester::doTests (int nArgs, char * args [])
{

    Arguments arguments = parseArgs (nArgs, args);
    nTestsAttempted_p = 0;
    nTestsPassed_p = 0;

    try {

        doTest<BasicChannelSelection> ();

        doTest<BasicCorrelationSelection> ();

        doTest<BasicMutation> ();

        doTest<FrequencyChannelSelection> ();

        doTest<MultipleMss> ();

        doTest<Weighting> ();

#if 0
        int tests = PerformanceComparator::Both;

        if (utilj::containsKey ("--old", arguments)){
            tests = PerformanceComparator::Old;
        }

        if (utilj::containsKey ("--new", arguments)){
            tests = PerformanceComparator::New;
        }

        int nSweeps = 1;
        String sweepNumber = arguments ["--sweeps"];
        if (! sweepNumber.empty()){
            nSweeps = String::toInt (sweepNumber);
            nSweeps = nSweeps <= 0 ? 1 : nSweeps;
        }

        int nChannelTests = 1;
        String nChannelTestsNumber = arguments ["--nChannels"];
        if (! nChannelTestsNumber.empty()){
            nChannelTests = String::toInt (nChannelTestsNumber);
            nChannelTests = nChannelTests <= 1 ? 1 : nChannelTests;
        }

        PerformanceComparator pc ("3c391_ctm_mosaic_spw0.ms");
        pc.compare(tests, nSweeps, nChannelTests);
#endif
    }
    catch (TestError & e){

        fprintf (stderr, "\n\n*** Last error was fatal; ending test!\n");
        fflush (stderr);
        exit (1);
    }

    bool allPassed = nTestsAttempted_p == nTestsPassed_p;
    printf ("Completed testing: ");

    if (allPassed){
        printf ("All %d tests passed ;-)\n", nTestsAttempted_p);
    }
    else{
        printf ("FAILED %d of %d tests attempted ;-0\n", nTestsAttempted_p - nTestsPassed_p, nTestsAttempted_p);
    }

    return allPassed;
}

Tester::Arguments
Tester::parseArgs (int nArgs, char * args []) const
{
    printf ("nArgs=%d\n", nArgs);
    String optionsRaw [] = {"--old", "--new", "--sweeps", "--nChannels", ""};
    set<String> options;

    for (int i = 0; ! optionsRaw [i].empty(); i++){

        options.insert (optionsRaw [i]);
    }

    Arguments result;

    for (int i = 1; i < nArgs; i ++){

        vector<String> splits = utilj::split (String (args[i]), "=");

        if (utilj::contains (splits [0], options)){

            result [splits[0]] = splits.size() > 1 ? splits[1] : "";
        }
        else{

            printf ("*** Unknown option: '%s'\n", splits[0].c_str());
            ThrowTestError ("Unknown command line option");
        }
    }

    return result;
}

BasicChannelSelection::BasicChannelSelection ()
: TestWidget ("BasicChannelSelection"),
  factor_p (1),
  msf_p (0),
  nAntennas_p (4),
  nFlagCategories_p (3),
  nSweeps_p (0)
{}

BasicChannelSelection::~BasicChannelSelection ()
{
    delete msf_p;
}

void
BasicChannelSelection::checkRowScalars (VisBuffer2 * vb)
{
    // Check out the non-cube data for each row

    const Vector<uInt> & rowIds = vb->rowIds ();
    Int nRows = vb->nRows();

    Int expectedAntenna1 = 0;
    Int expectedAntenna2 = 0;
    Double previousTime = -1;
    Bool newTimeExpected = True;

    for (Int row = 0; row < nRows; row ++){

        Int rowId = rowIds (row);

        if (newTimeExpected){
            TestErrorIf (vb->time()(row) + 0.5 < previousTime,
                          String::format ("Backwards time=%f (< %f) at msRow=%d",
                                          vb->time()(row), previousTime, rowId));
            previousTime = vb->time()(row);
            newTimeExpected = False;
        }

        TestErrorIf (vb->time()(row) != previousTime,
                      String::format ("Expected time=%f, got %f at msRow=%d",
                                      previousTime, vb->time()(row), rowId));

        TestErrorIf (vb->antenna1 ()(row) != expectedAntenna1,
                      String::format ("Expected %d for antenna1 (%d); got %d",
                                      expectedAntenna1, rowId, vb->antenna1()(rowId)));

        TestErrorIf (vb->antenna2 ()(row) != expectedAntenna2,
                      String::format ("Expected %d for antenna2 (%d); got %d",
                                      expectedAntenna2, rowId, vb->antenna2()(rowId)));

        expectedAntenna2 ++;

        if (expectedAntenna2 == nAntennas_p){

            expectedAntenna1 ++;
            expectedAntenna2 = expectedAntenna1;
            newTimeExpected = True;
        }

        checkUvw (vb, nRows, rowId, row);

        Bool flagRowExpected = (rowId % 3 == 0) || (rowId % 5 == 0);
        if (factor_p != 1){
            flagRowExpected = ! flagRowExpected;
        }

        TestErrorIf (vb->flagRow () (row) != flagRowExpected,
                      String::format ("Bad flag row value for msRow=%d; expected %d, got %d",
                                      rowId, flagRowExpected, vb->flagRow () (row) ));

        checkRowScalar (vb->exposure()(row), 8, rowId, "scan");
        checkRowScalar (vb->observationId()(row), 7, rowId, "scan");
        checkRowScalar (vb->scan() (row), 5, rowId, "scan");
        checkRowScalar (vb->timeCentroid ()(row), 1, rowId, "timeCentroid");
        checkRowScalar (vb->timeInterval () (row), 2, rowId, "timeInterval");


        checkSigmaWeight (vb->nCorrelations (), vb->sigma (), 3, rowId, row, "sigma", factor_p);
        checkSigmaWeight (vb->nCorrelations (), vb->weight (), 4, rowId, row, "weight", factor_p);
    }
}



void
BasicChannelSelection::checkRowScalar (Double value, Double offset, Int rowId, const char * name,
                                       Int factor)
{
    Double expected = rowId * 100 + offset;
    expected *= factor;

    ThrowIf (value != expected,
                 String::format ("Expected %d for %s (%d); got %d",
                                   expected, name, rowId, value));
}

void
BasicChannelSelection::checkSigmaWeight (Int nCorrelations, const Matrix<Float> & values, Double offset, Int rowId,
                                         Int row, const char * name, Int factor)
{
    for (Int i = 0; i < nCorrelations; i++){

        Double value = values (i, row);

        Double expected = rowId * 100 + offset;
        expected *= factor;

        ThrowIf (value != expected,
                 String::format ("Expected %f for %s (%d, %d); got %f",
                                 expected, name, i, rowId, value));
    }
}


void
BasicChannelSelection::checkUvw (VisBuffer2 * vb, Int nRows, Int rowId, Int row)
{
    // Check UVW

    TestErrorIf (vb->uvw().shape().nelements() != 2 ||
                 vb->uvw().shape()(0) != 3 ||
                 vb->uvw().shape()(1) != nRows,
                 String::format ("Bad uvw shape: expected [3,%d] got [%d,%d]",
                                 nRows, vb->uvw().shape()(0), vb->uvw().shape()(1)));

    for (int i = 0; i < 3; i++){

        Double expected = rowId * 10 + i;

        TestErrorIf (expected != vb->uvw()(i, row),
                     String::format ("Expected %f for uvw (%d, %d); got %f",
                                     expected, rowId, i, vb->uvw()(row, i)));
    }
}


boost::tuple <MeasurementSet *, Int, Bool>
BasicChannelSelection::createMs ()
{
    system ("rm -r BasicChannelSelection.ms");

    msf_p = new MsFactory ("BasicChannelSelection.ms");
    msf_p->setIncludeAutocorrelations(True);

    pair<MeasurementSet *, Int> p = msf_p->createMs ();
    nRowsToProcess_p = p.second;
    return boost::make_tuple (p.first, p.second, False);
}


/*void
BasicChannelSelection::endOfChunk (VisibilityIterator2 & vi, VisBuffer2 * vb, Int nRowsProcessed)
{

}*/

/* void
BasicChannelSelection::nextChunk (VisibilityIterator2 & vi, VisBuffer2 * vb)
{
}*/

void
BasicChannelSelection::nextSubchunk (VisibilityIterator2 & /*vi*/, VisBuffer2 * vb)
{
    // Check out that the subchunk has the appropriate data

    Int spectralWindow = vb->spectralWindows()(0);
    Int nRows = vb->nRows();
    Int nCorrelations = vb->nCorrelations();

    VisBuffer2 * vb2 = VisBuffer2::factory (VbPlain, VbRekeyable);
    vb2->copyCoordinateInfo(vb, false, true);
    delete vb2;

    // The expected channels are 0..4 for spw 1, 6..10 for spw 2 and 3..8 for spw 3

    static const Int info [][3] = {{0, 5, 1 }, {6, 5, 1}, {3, 6, 1}, {0, 5, 2}};

    const Cube<Complex> & visibility = vb->visCube();
    const Cube<Complex> & visibilityCorrected = vb->visCubeCorrected();
    const Cube<Complex> & visibilityModel = vb->visCubeModel();

    const Vector<uInt> & rowIds = vb->rowIds ();

    Int channelOffset = info [spectralWindow][0];
    Int nChannels = info [spectralWindow][1];
    Int channelIncrement = info [spectralWindow][2];

    // Check visibility shapes

    IPosition expectedShape = IPosition (3, nCorrelations, nChannels, nRows);
    TestErrorIf (! visibility.shape().isEqual (expectedShape),
                 String::format("Bad visibility shape; expected %s, got %s; "
                                "at spw=%d, msRow=%d",
                                expectedShape.toString().c_str(),
                                visibility.shape().toString().c_str(),
                                spectralWindow,
                                rowIds (0)));

    TestErrorIf (! visibilityCorrected.shape().isEqual (expectedShape),
                 String::format("Bad corrected visibility shape; expected %s, got %s; "
                                "at spw=%d, msRow=%d",
                                expectedShape.toString().c_str(),
                                visibilityCorrected.shape().toString().c_str(),
                                spectralWindow,
                                rowIds (0)));

    TestErrorIf (! visibilityModel.shape().isEqual (expectedShape),
                 String::format("Bad model visibility shape; expected %s, got %s; "
                                "at spw=%d, msRow=%d",
                                expectedShape.toString().c_str(),
                                visibilityModel.shape().toString().c_str(),
                                spectralWindow,
                                rowIds (0)));

    TestErrorIf (! vb->weightSpectrum ().shape().isEqual (expectedShape),
                 String::format("Bad weight spectrum shape; expected %s, got %s; "
                         "at spw=%d, msRow=%d",
                         expectedShape.toString().c_str(),
                         vb->weightSpectrum ().shape().toString().c_str(),
                         spectralWindow,
                         rowIds (0)));

    TestErrorIf (! vb->weight().shape().isEqual (IPosition (2, nCorrelations, nRows)),
                 String::format("Bad visibility shape; expected %s, got %s; "
                                "at spw=%d, msRow=%d",
                                expectedShape.toString().c_str(),
                                visibility.shape().toString().c_str(),
                                spectralWindow,
                                rowIds (0)));



    // Test flag cube shapes

    IPosition expectedShape2 (IPosition (4, nCorrelations, nChannels, nFlagCategories_p, nRows));
    TestErrorIf (! vb->flagCategory().shape ().isEqual (expectedShape2),
                 String::format("Bad flag category shape; expected %s, got %s; "
                                "spw=%d, msRow=%d",
                                vb->flagCategory().shape().toString().c_str(),
                                expectedShape2.toString().c_str(),
                                spectralWindow,
                                rowIds (0)))

    checkRowScalars (vb);

    for (Int row = 0; row < nRows; row ++){

        const Vector<Int> & channels = vb->getChannelNumbers(row);

        for (Int channel = 0; channel < nChannels; channel ++){

            for (Int correlation = 0; correlation < nCorrelations; correlation ++){

                // Test to see if the default fill pattern for the three types of
                // visibility cubes.

                checkVisCube (rowIds (row), spectralWindow, row, channel, correlation,
                                     visibility, "", channelOffset, channelIncrement, 0);

                checkVisCube (rowIds (row), spectralWindow, row, channel, correlation,
                                     visibilityCorrected, "corrected", channelOffset, channelIncrement, 1);

                checkVisCube (rowIds (row), spectralWindow, row, channel, correlation,
                                     visibilityModel, "model", channelOffset, channelIncrement, 2);

                checkFlagCube (rowIds (row), spectralWindow, row, channel, correlation,
                               channelOffset, channelIncrement, vb);

                checkWeightSpectrum (rowIds (row), spectralWindow, row, channel, correlation,
                                     channelOffset, channelIncrement, vb);

                checkChannelAndFrequency (rowIds (row), row, channel, channelIncrement, channelOffset,
                                          spectralWindow, vb);

                // See if the getChannels method is returning the correct channel number

                Int expectedChannelNumber = channel * channelIncrement + channelOffset;

                TestErrorIf (expectedChannelNumber != channels [channel],
                             String::format ("vb->getChannels()[%d] returned %d; expected %d",
                                             channel, channels [channel], expectedChannelNumber));

                // Check the flag categories

                checkFlagCategory (rowIds (row), spectralWindow, row, channel, correlation,
                                   channelOffset, channelIncrement, vb);
            }
        }
    }
}

void
BasicChannelSelection::checkFlagCategory (Int rowId, Int spectralWindow, Int row, Int channel, Int correlation,
                                          Int channelOffset, Int channelIncrement, VisBuffer2 * vb)
{
    const Array<Bool> & flagCategories = vb->flagCategory();

    // Now check out the flag categories array [nC, nF, nCat, nR]

    Int correlationSelected = getUnderlyingCorrelation (spectralWindow, correlation);
    Int expectedChannelNumber = channel * channelIncrement + channelOffset;

    for (int category = 0; category < nFlagCategories_p; category ++){

        Bool expected;// = (rowId % 2) ^ (channel % 2) ^ (correlation % 2) ^ (category % 2);

        switch (correlationSelected){

        case 0:

            expected = rowId & 0x1;
            break;

        case 1:

            expected = rowId & 0x2;
            break;

        case 2:

            expected = expectedChannelNumber & 0x1;
            break;

        case 3:

            expected = expectedChannelNumber & 0x2;
            break;

        default:

            Assert (False);
            break;

        }



        if (factor_p != 1){
            expected = ! expected;
        }

        Bool value = flagCategories (IPosition (4, correlation, channel, category, row));

        TestErrorIf (value != expected,
                     String::format("Expected %d, got %d for flagCategory at "
                                    "spw=%d, vbRow=%d, msRow=%d, ch=%d, corr=%d, cat=%d",
                                    expected,
                                    value,
                                    spectralWindow,
                                    row,
                                    rowId,
                                    channel,
                                    correlation,
                                    category));

        expected = ! expected;
    }
}

void
BasicChannelSelection::checkChannelAndFrequency (Int rowId, Int row, Int channel, Int channelIncrement, Int channelOffset,
                                                 Int spectralWindow, const VisBuffer2 * vb)
{
    const Vector<Int> & channels = vb->getChannelNumbers(row);

    // See if the getChannels method is returning the correct channel number

    Int expectedChannelNumber = channel * channelIncrement + channelOffset;

    TestErrorIf (expectedChannelNumber != channels [channel],
                 String::format ("vb->getChannels()[%d] returned %d; expected %d",
                                 channel, channels [channel], expectedChannelNumber));

    // Now check the frequency lookup.  Use topo since it should return the same frequency that the
    // bin was recorded at.  The default simulated base frequency should conform is 1E9 * spectral window
    // and the increment is 1E6 * spectral window.  Accept any deviation that is less than a 1/2 of the
    // increment.

    Double frequency = vb->getFrequency(row, channel/*, MFrequency::TOPO*/);

    Double expectedFrequency = 1e9 * (spectralWindow + 1) + 1e6 * (spectralWindow + 1) * expectedChannelNumber;

    ThrowIf (abs (frequency - expectedFrequency) > 0.5e6 * (spectralWindow + 1),
             String::format ("TOPO frequency mismatch: expected %12.5e, got %12.5e at msRow=%d, ch=%d",
                             expectedFrequency, frequency, rowId, channel, channels [channel]));
}

void
BasicChannelSelection::checkVisCube (Int rowId, Int spectralWindow, Int row, Int channel, Int correlation,
                                     const Cube<Complex> & cube, const String & tag,
                                     Int channelOffset, Int channelIncrement, Int cubeDelta)
{
    Float real = 10 * rowId + spectralWindow;
    Int expectedChannelNumber = channel * channelIncrement + channelOffset;
    Int correlationSelected = getUnderlyingCorrelation (spectralWindow, correlation);

    Float imag = 100 * (expectedChannelNumber) + correlationSelected * 10 + cubeDelta;
    Complex z0 (real, imag);
    z0 *= factor_p;

    Complex z = cube (correlation, channel, row);

    TestErrorIf (z != z0,
                 String::format("Expected (%f,%f), got (%f,%f) for %s vis at "
                         "spw=%d, vbRow=%d, msRow=%d, ch=%d, corr=%d",
                         z0.real(), z0.imag(),
                         z.real(), z.imag(),
                         tag.c_str(),
                         spectralWindow,
                         row,
                         rowId,
                         channel,
                         correlationSelected));

}

void
BasicChannelSelection::checkFlagCube (Int rowId, Int spectralWindow, Int row, Int channel, Int correlation,
                                      Int channelOffset, Int channelIncrement, VisBuffer2 * vb)
{
    Int expectedChannelNumber = channel * channelIncrement + channelOffset;

    Int correlationSelected = getUnderlyingCorrelation (spectralWindow, correlation);

    Bool expectedValue = (expectedChannelNumber % 2 == 0) == (correlationSelected % 2 == 0);
    if (factor_p != 1){
        expectedValue = ! expectedValue;
    }

    Bool value = vb->flagCube ()(correlation, channel, row);

    TestErrorIf (expectedValue != value,
                 String::format("Expected %d, got %d for flag cube at "
                         "spw=%d, vbRow=%d, msRow=%d, ch=%d, corr=%d",
                         expectedValue,
                         value,
                         spectralWindow,
                         row,
                         rowId,
                         channel,
                         correlationSelected));
}

void
BasicChannelSelection::checkWeightSpectrum (Int rowId, Int spectralWindow, Int row, Int channel,
                                            Int correlation, Int channelOffset, Int channelIncrement,
                                            const VisBuffer2 * vb)
{
    Int expectedChannelNumber = channel * channelIncrement + channelOffset;

    Int correlationSelected = getUnderlyingCorrelation (spectralWindow, correlation);

    Float expectedValue = rowId * 1000 + spectralWindow * 100 + expectedChannelNumber * 10 +
                          correlationSelected + 1;
    expectedValue *= factor_p;

    Float actualValue = vb->weightSpectrum() (correlation, channel, row);

    TestErrorIf (expectedValue != actualValue,
                 String::format ("Expected %f for weight spectrum (%d,%d,%d); got %f "
                                 "spw=%d, msRow=%d",
                                 expectedValue,
                                 correlationSelected,
                                 channel,
                                 row,
                                 actualValue,
                                 spectralWindow,
                                 rowId));
}


int
BasicChannelSelection::getUnderlyingCorrelation (Int spectralWindow, Int correlation) {

    if (correlationSlices_p.empty()){
        return correlation;
    }

    Vector<Slice> vs = correlationSlices_p (spectralWindow);

    uInt n = 0;

    for (Int sliceIndex = 0; sliceIndex <= (int) vs.nelements(); sliceIndex ++){

        for (Int j = vs(sliceIndex).start(), i = 0;
             i < vs(sliceIndex).length ();
             j += vs(sliceIndex).inc(), i++){

            if ((int) n == correlation){
                return j;
            }
            n ++;
        }
    }

    Throw ("Bugcheck: Failed to find correlation defined in spectral window");

}

Bool
BasicChannelSelection::noMoreData (VisibilityIterator2 & /*vi*/, VisBuffer2 * /*vb*/, int nRowsProcessed)
{
    TestErrorIf (nRowsProcessed != nRowsToProcess_p,
                 String::format ("Expected to process %d rows, but did %d instead.",
                                 nRowsToProcess_p, nRowsProcessed));

    nSweeps_p ++;

    return nSweeps_p < 2;
}

void
BasicChannelSelection::setCorrelationSlices (const Vector <Vector <Slice> > & slices)
{
    correlationSlices_p = slices;
}


void
BasicChannelSelection::startOfData (VisibilityIterator2 & vi, VisBuffer2 * /*vb*/)
{
    // Apply channel selections

    FrequencySelectionUsingChannels selection;
    selection.add (0, 0, 5);
    selection.add (1, 6, 5);
    selection.add (2, 3, 6);
    selection.add (3, 0, 5, 2);

    if (! correlationSlices_p.empty()){

        selection.addCorrelationSlices (correlationSlices_p);
    }


    vi.setFrequencySelection (selection);

}

BasicCorrelationSelection::BasicCorrelationSelection ()
: BasicChannelSelection ()
{
    Vector<Slice> aSlice (1);
    Vector <Vector <Slice> > slices (4);

    aSlice(0) = Slice(0,2);
    slices (0) = aSlice;

    aSlice(0) = Slice (0,2,2);
    slices (1) = aSlice;

    aSlice(0) = Slice (0,2,3);
    slices (2) = aSlice;

    Vector<Slice> twoSlices (2);

    twoSlices(0) = Slice (1,1,1);
    twoSlices(1) = Slice (3,1,1);
    slices (3) = twoSlices;

    setCorrelationSlices (slices);
}

BasicCorrelationSelection::~BasicCorrelationSelection ()
{
}


BasicMutation::BasicMutation ()
: BasicChannelSelection (),
  firstPass_p (True)
{
}

BasicMutation::~BasicMutation ()
{
}

boost::tuple <MeasurementSet *, Int, Bool>
BasicMutation::createMs ()
{
    MeasurementSet * ms;
    Int nRows;
    Bool toss;

    boost::tie (ms, nRows, toss) = BasicChannelSelection::createMs ();

    return boost::make_tuple (ms, nRows, True);
}

class LogicalNot {
public:

    Bool operator() (Bool b) { return ! b;}
};


void
BasicMutation::nextSubchunk (VisibilityIterator2 & vi, VisBuffer2 * vb)
{
    // Perform the standard value check

    BasicChannelSelection::nextSubchunk (vi, vb);

    // On the first pass through, toggle the data: (x -> - x and b -> ~b)

    if (firstPass_p){

        Cube<Complex> cube;

        cube = vb->visCube();
        cube = - cube;
        vb->setVisCube (cube);

        cube = vb->visCubeCorrected();
        cube = - cube;
        vb->setVisCubeCorrected (cube);

        cube = vb->visCubeModel();
        cube = - cube;
        vb->setVisCubeModel (cube);

        Cube<Float> cubeF;

        cubeF = vb->weightSpectrum ();
        cubeF = - cubeF;
        vb->setWeightSpectrum (cubeF);

        Cube<Bool> cubeB;

        cubeB = vb->flagCube();
        cubeB = arrayTransformResult (cubeB, LogicalNot());
        vb->setFlagCube (cubeB);

        Array<Bool> flagCategory = vb->flagCategory();
        flagCategory = arrayTransformResult (flagCategory, LogicalNot());
        vb->setFlagCategory (flagCategory);

        Matrix<Float> v;

        v = vb->weight();
        v = - v;
        vb->setWeight (v);

        v = vb->sigma();
        v = - v;
        vb->setSigma (v);

        Vector<Bool> vB;
        vB = vb->flagRow();
        vB = arrayTransformResult (vB, LogicalNot());
        vb->setFlagRow (vB);

        vb->writeChangesBack();
    }
}

Bool
BasicMutation::noMoreData (VisibilityIterator2 & /*vi*/, VisBuffer2 * /*vb*/, int /*nRows*/)
{
    Bool moreSweeps = firstPass_p;
    firstPass_p = False;
    setFactor (-1);

    return moreSweeps;
}


void
FrequencyChannelSelection::startOfData (VisibilityIterator2 & vi, VisBuffer2 * /*vb*/)
{
    // Apply channel selections

    FrequencySelectionUsingFrame selection (MFrequency::TOPO);

    selection.add (0, 1e9, 1.0043e9);
    selection.add (1, 2.012E9, 2.0203e9);
    selection.add (2, 3.009e9, 3.024e9);

    vi.setFrequencySelection (selection);
}

Bool
FrequencyChannelSelection::noMoreData (VisibilityIterator2 & /*vi*/, VisBuffer2 * /*vb*/, int /*nRowsProcessed*/)
{
//    TestErrorIf (nRowsProcessed != nRowsToProcess_p,
//                 String::format ("Expected to process %d rows, but did %d instead.",
//                                 nRowsToProcess_p, nRowsProcessed));

    return False;
}



////////////////  Template for new TestWidget Subclasses //////////////////

//TheWidget::TheWidget () : TestWidget ("TheWidget") {}

/*pair<MeasurementSet * ms, Int>
TheWidget::createMs ()
{
}*/


/*void
TheWidget::endOfChunk (VisibilityIterator2 & vi, VisBuffer2 * vb)
{
}*/

/* void
TheWidget::nextChunk (VisibilityIterator2 & vi, VisBuffer2 * vb)
{
}*/

/* void
TheWidget::nextSubchunk (VisibilityIterator2 & vi, VisBuffer2 * vb)
{
}*/

/* void
TheWidget::noMoreData (VisibilityIterator2 & vi, VisBuffer2 * vb, int nRows)
{
}*/

/* void
TheWidget::startOfData (VisibilityIterator2 & vi, VisBuffer2 * vb)
{
}*/

PerformanceComparator::PerformanceComparator (const String & ms)
: ms_p (ms)
{}

void
PerformanceComparator::compare (int tests, int nSweeps, int nChannelTests)
{
    printf ("\n========== Performance Comparison Begin =========\n");

    MeasurementSet ms (ms_p);

    printf ("--- MeasurementSet: %s\n", ms_p.c_str());

    Block<int> sortColumns;

    ROVisibilityIterator oldVi (ms, sortColumns, True);

    VisibilityIterator2 newVi (ms, SortColumns ());

    printf ("\n--- Default channel selection ---\n");

    //////compareOne (& oldVi, & newVi, 11);

    for (int i = 1; i <= nChannelTests; i++){

        printf ("\n--- Selecting %d channels ---\n", i);

        Block< Vector<Int> > groupSize (1, Vector<Int> (1, 1));
        Block< Vector<Int> > spectralWindow (1, Vector<Int> (1, 0));
        Block< Vector<Int> > start (1, Vector<Int> (1, 0));
        Block< Vector<Int> > increment (1, Vector<Int> (1, 1));
        Block< Vector<Int> > count (1, Vector<Int> (1, i));

        oldVi.selectChannel (groupSize, start, count, increment, spectralWindow);

        FrequencySelections fs;
        FrequencySelectionUsingChannels fsuc;
        fsuc.add (0, 0, i);
        fs.add (fsuc);

        newVi.setFrequencySelection (fs);

        compareOne (& oldVi, & newVi, nSweeps, tests);
    }

    printf ("\n========== Performance Comparison Complete =========\n");
}


void
PerformanceComparator::compareOne (ROVisibilityIterator * oldVi,
                                   VisibilityIterator2 * newVi,
                                   int nSweeps,
                                   int tests)
{
    using namespace utilj;

    IoStatistics oldIo1;
    ThreadTimes oldTime1 = ThreadTimes::getTime ();

    if (tests & PerformanceComparator::Old){

        printf ("... Starting old sweeps\n");

        try {

            for (int i = 0; i < nSweeps; i++){
                Double d = sweepViOld (* oldVi);
                printf ("... old sweep %d completed. (%f)\n", i, d);
            }
        }
        catch (AipsError & e){
            printf ("*** Caught exception while sweeping oldVi: %s\n", e.what());
            throw;
        }
    }

    ThreadTimes oldTime2 = ThreadTimes::getTime ();
    IoStatistics oldIo2;

    IoStatistics newIo1;
    ThreadTimes newTime1 = ThreadTimes::getTime ();

    if (tests & PerformanceComparator::New){

        printf ("... Starting new sweeps\n");

        try{
            for (int i = 0; i < nSweeps; i++){
                Double d = sweepViNew (* newVi);
                printf ("... new sweep %d completed. (%f)\n", i, d);
            }
        }
        catch (AipsError & e){
            printf ("*** Caught exception while sweeping newVi: %s\n", e.what());
            throw;
        }
    }

    ThreadTimes newTime2 = ThreadTimes::getTime ();
    IoStatistics newIo2;

    IoStatistics oldIo = oldIo2 - oldIo1;
    IoStatistics newIo = newIo2 - newIo1;
    DeltaThreadTimes oldDt = oldTime2 - oldTime1;
    DeltaThreadTimes newDt = newTime2 - newTime1;

    printf ("--- Stats ---  nSweeps=%d\n\n", nSweeps);
    printf ("Old VI: %s; %s\n", oldIo.report ().c_str(), oldDt.formatStats().c_str());
    printf ("New VI: %s; %s\n", newIo.report ().c_str(), newDt.formatStats().c_str());
    IoStatistics ratioIo = newIo / oldIo;
    printf ("old/new: %s ; elapsed: %6.2f %% cpu: %6.2f %%\n", ratioIo.report (100, "%").c_str(),
            newDt.elapsed () / oldDt.elapsed() * 100,
            newDt.cpu() / oldDt.cpu() * 100);

}

Double
PerformanceComparator::sweepViNew (VisibilityIterator2 & vi)
{
    VisBuffer2 * vb = vi.getVisBuffer();
    Double sum = 0;

    for (vi.originChunks (); vi.moreChunks(); vi.nextChunk()){

        for (vi.origin(); vi.more(); vi.next()){

            Int nRows = vb->nRows();
            Int nChannels = vb->nChannels();
            Int nCorrelations = vb->nCorrelations();

            for (Int row = 0; row < nRows; row ++){
                for (Int channel = 0; channel < nChannels; channel ++){
                    for (Int correlation = 0; correlation < nCorrelations; correlation ++){
                        Complex c = vb->visCube ()(correlation, channel, row);
                        c = c * c;
                        sum += c.real();
                    }
                }
            }
        }
    }

    return sum;
}

Double
PerformanceComparator::sweepViOld (ROVisibilityIterator & vi)
{
    VisBuffer vb (vi);
    Double sum = 0;

    for (vi.originChunks (); vi.moreChunks(); vi.nextChunk()){

        for (vi.origin(); vi.more(); vi++){

            Int nRows = vb.nRow();
            Int nChannels = vb.nChannel();
            Int nCorrelations = vb.nCorr();

            for (Int row = 0; row < nRows; row ++){
                for (Int channel = 0; channel < nChannels; channel ++){
                    for (Int correlation = 0; correlation < nCorrelations; correlation ++){
                        Complex c = vb.visCube ()(correlation, channel, row);
                        c = c * c;
                        sum += c.real();
                    }
                }
            }
        }
    }

    return sum;
}

//  MeasurementSet* SubMS::setupMS(const String& MSFileName, const Int nchan,
//                                 const Int nCorr, const String& telescop,
//                                 const Vector<MS::PredefinedColumns>& colNames,
//                                 const Int obstype,
//                                 const Bool compress,
//				 const asdmStManUseAlternatives asdmStManUse)

void
CopyMs::copySubtables (MeasurementSet * newMs, const MeasurementSet * oldMs)
{
    SubMS::copyCols (newMs->antenna(), oldMs->antenna (), True);
    SubMS::copyCols (newMs->dataDescription(), oldMs->dataDescription (), True);
    if (! newMs->doppler().isNull()){
        SubMS::copyCols (newMs->doppler(), oldMs->doppler (), True);
    }
    SubMS::copyCols (newMs->feed(), oldMs->feed (), True);
    SubMS::copyCols (newMs->field(), oldMs->field (), True);
    SubMS::copyCols (newMs->flagCmd(), oldMs->flagCmd (), True);
    if (! newMs->freqOffset().isNull()){
        SubMS::copyCols (newMs->freqOffset(), oldMs->freqOffset (), True);
    }
    SubMS::copyCols (newMs->history(), oldMs->history (), True);
    TableCopy::copyRows (newMs->observation(), oldMs->observation (), True);
    setupNewPointing (newMs);
    SubMS::copyCols (newMs->pointing(), oldMs->pointing (), True);
    SubMS::copyCols (newMs->polarization(), oldMs->polarization (), True);
    SubMS::copyCols (newMs->processor(), oldMs->processor (), True);
    if (! newMs->source().isNull()){
        TableCopy::copyRows (newMs->source(), oldMs->source (), True);
    }
    SubMS::copyCols (newMs->spectralWindow(), oldMs->spectralWindow (), True);
    SubMS::copyCols (newMs->state(), oldMs->state (), True);
    if (! newMs->sysCal().isNull()){
        SubMS::copyCols (newMs->sysCal(), oldMs->sysCal (), True);
    }
    if (! newMs->weather().isNull()){
        SubMS::copyCols (newMs->weather(), oldMs->weather (), True);
    }
}

void
CopyMs::setupNewPointing(MeasurementSet * newMs)
{
  // Swiped from SubMs so I could get past this problem.

  SetupNewTable pointingSetup(newMs->pointingTableName(),
                              MSPointing::requiredTableDesc(), Table::New);
  // POINTING can be large, set some sensible defaults for storageMgrs
  IncrementalStMan ismPointing ("ISMPointing");
  StandardStMan ssmPointing("SSMPointing", 32768);
  pointingSetup.bindAll(ismPointing, True);
  pointingSetup.bindColumn(MSPointing::columnName(MSPointing::DIRECTION),
                           ssmPointing);
  pointingSetup.bindColumn(MSPointing::columnName(MSPointing::TARGET),
                           ssmPointing);
  pointingSetup.bindColumn(MSPointing::columnName(MSPointing::TIME),
                           ssmPointing);
  newMs->rwKeywordSet().defineTable(MS::keywordName(MS::POINTING),
                                     Table(pointingSetup));
  newMs->initRefs();
}


void
CopyMs::doit (const String & oldMsName)
{
    casa::MeasurementSet oldMs (oldMsName);

    String newMsName = String::format ("%s.copy", oldMsName.c_str());

    system (String::format ("test -d %s && rm -r %s", newMsName.c_str(), newMsName.c_str()).c_str());
    system ("casapy --nogui -c \"execfile('/home/orion/casa/trunk/code/msvis/MSVis/test/makeEmptyCopy.py')\"");


//    subMs.setmsselect("", "", "", "0");
//
//    String all ("ALL");
//    Bool ok = subMs.makeSubMS(newMsName, all);
//    ThrowIf (! ok, "Method makeSubMS failed");


    MeasurementSet newMs (newMsName, Table::Update);
//    uInt nRows = newMs.nrow();
//    Vector<uInt> allRows (nRows);
//    for (uInt i = 0; i < nRows; i++) allRows [i] = i;
//    newMs.removeRow (allRows);


//    MeasurementSet * newMs = SubMS::setupMS (newMsName, nChannels, nCorrelations, "VLA",
//                                             columnNames);
//    // SubMS::createSubtables(* newMs, Table::New);
//
//    newMs->closeSubTables();
//
//    copySubtables (newMs, & oldMs);

    //SetupNewTable newSetup (newMsName, oldMs.tableDesc(), Table::NewNoReplace);

    //casa::MeasurementSet newMs (newSetup, 0, False);
    //newMs.createDefaultSubtables(Table::NewNoReplace);

    VisibilityIterator2 * vi = VisibilityIterator2::copyingViFactory (oldMs, newMs);
    VisBuffer2 * vb = vi->getVisBuffer ();

    for (vi->originChunks (); vi->moreChunks (); vi->nextChunk ()){
        for (vi->origin (); vi->more (); vi->next()){

            vb->writeChangesBack ();
        }
    }

    newMs.flush();
}

MultipleMss::MultipleMss () : TestWidget ("MultipleMss"), nMss_p (3) {}

boost::tuple <Block<const MeasurementSet *>, Int, Bool>
MultipleMss::createMss (){

    Block <const MeasurementSet *> mss (nMss_p, 0);
    Int nRows = 0;

    for (int i = 0; i < nMss_p; i++){

        String msName = String::format ("MultipleMss%d.ms", i);
        system (String::format ("rm -r %s", msName.c_str()).c_str());
        MsFactory * msf = new MsFactory (msName);
        msf->setIncludeAutocorrelations(True);
        msf->addSpectralWindow("spw", i+5, 0, 100, "LL RR RL LR");

        pair<MeasurementSet *, Int> p = msf->createMs ();

        nRows += p.second;
        mss [i] = p.first;

        delete msf;
    }

    return boost::make_tuple (mss, nRows, False);
}

void
MultipleMss::nextSubchunk (VisibilityIterator2 & /*vi*/, VisBuffer2 * vb)
{
    // Check out that the subchunk has the appropriate data

    Int spectralWindow = vb->spectralWindows()(0);
    TestErrorIf (spectralWindow != 0,
                 String::format ("Bad spectral window id: expected 0, got %d", spectralWindow));
    //Int nRows = vb->nRows();

    Int msId = vb->msId();

    const Cube<Complex> & visibility = vb->visCube();
    const Vector<uInt> & rowIds = vb->rowIds ();


    TestErrorIf (visibility.shape()[0] != 4,
                 String::format ("Bad # of polarizations: expected 4, got %d"
                                 "; at msRow=%d, msId=%d",
                                 visibility.shape()[0], rowIds[0], msId));

    TestErrorIf (visibility.shape()[1] != 5,
                 String::format ("Bad # of channels: expected 4, got %d"
                                 " at msRow=%d, msId=%d",
                                 visibility.shape()[1], rowIds[0], msId));

    Vector<Int> channels = vb->getChannelNumbers (0);

    for (Int i = 0; i < 5; i++){
        TestErrorIf (channels[i] != msId + i,
                     String::format ("Bad channel number: expected %d, got %d"
                                     " at msRow=%d, msId=%d",
                                     msId + i, channels[i], rowIds[0], msId));
    }
}




void
MultipleMss::startOfData (VisibilityIterator2 & vi, VisBuffer2 * /*vb*/)
{
    // Apply channel selections

    FrequencySelections selections;

    for (int i=0; i < nMss_p; i++){

        FrequencySelectionUsingChannels selection;

        selection.add (0, i, 5);

        selections.add (selection);
    }

    vi.setFrequencySelection (selections);
}


bool
MultipleMss::usesMultipleMss () const {
    return True;
}

boost::tuple <MeasurementSet *, Int, Bool>
Weighting::createMs ()
{
    system ("rm -r Weighting.ms");

    msf_p = new MsFactory ("Weighting.ms");
    msf_p->setIncludeAutocorrelations(True);
    msf_p->addSpectralWindow("spw", 5, 0, 100, "LL RR RL LR");

    msf_p->addWeightSpectrum(False);
    msf_p->addCorrectedWeightSpectrum(True);

    pair<MeasurementSet *, Int> p = msf_p->createMs ();
    nRowsToProcess_p = p.second;
    return boost::make_tuple (p.first, p.second, False);
}

void
Weighting::nextSubchunk (VisibilityIterator2 & vi, VisBuffer2 * vb)
{
    TestErrorIf (vi.weightSpectrumExists(),
                 "This test requires that the weight spectrum column not exist.");

    // TestErrorIf (! vi.weightSpectrumCorrectedExists(),
    //              "This test requires that the corrected weight spectrum column exist.");

    Matrix <Float> weight;
    weight = vb->weight();  // get an unshared copy
    const Cube <float> & weightSpectrum = vb->weightSpectrum();
    IPosition wsShape = weightSpectrum.shape();
    IPosition wShape = weight.shape();

    TestErrorIf (wsShape (0) != wShape (0) || wsShape (2) != wShape (1),
                 "Incompatible weight and weight spectrum shapes: " + wShape.toString() + " vs. "
                 + wsShape.toString());

    Int nCorrelations = wsShape (0);
    Int nChannels = wsShape (1);
    Int nRows = wsShape (2);
    const Vector<uInt> & rowIds = vb->rowIds();

    for (Int row = 0; row < nRows; row ++){
        for (Int correlation = 0; correlation < nCorrelations; correlation ++){

            Float sum = 0;

            for (Int channel = 0; channel < nChannels; channel ++){
                sum = sum + weightSpectrum (correlation, channel, row);
            }

            Float delta = abs ((sum - weight (correlation, row)) / weight (correlation, row));
            TestErrorIf (delta > 1e-5,
                         String::format ("Sum of weight spectrum != weight at"
                                         " row=%d, correlation=%d, delta=%f",
                                         row, correlation, delta));
        }
    }

    // const Cube<Float> & correctedWeightSpectrum = vb->weightSpectrumCorrected();

    // for (Int row = 0; row < nRows; row ++){
    //     for (Int correlation = 0; correlation < nCorrelations; correlation ++){
    //         for (Int channel = 0; channel < nChannels; channel ++){

    //             Float expected = rowIds(row) * 1000 /*+ spw * 100*/ + channel * 10 + correlation + 2;
    //             Float actual = correctedWeightSpectrum (correlation, channel, row);

    //             TestErrorIf (actual != expected,
    //                          String::format ("CorrectedWeightSpectrum incorrect at "
    //                                  "row=%d, channel=%d, correlation=%d;\n"
    //                                  "expected=%f but got %f\n",
    //                                  row, channel, correlation, expected, actual));
    //         }
    //     }
    // }
}


} // end namespace test
} // end namespace vi
} // end namespace casa

