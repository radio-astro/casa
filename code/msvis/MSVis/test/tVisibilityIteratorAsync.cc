//# tVisibilityIterator.cc: Tests the Synthesis MeasurementSet Iterator
//# Copyright (C) 1995,1999,2000,2001
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#include <synthesis/MSVis/VisibilityIterator.h>
#include <synthesis/MSVis/VisBufferAsync.h>
#include <synthesis/MSVis/UtilJ.h>
#include <tables/Tables.h>
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <casa/iomanip.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/BasicSL/Constants.h>
#include <casa/System/Aipsrc.h>
#include <memory>
#include <mcheck.h>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>

using namespace std;
using namespace casa::utilj;
using namespace casa;
using namespace casa::asyncio;
using namespace boost;

#include <tables/Tables/ForwardCol.h>

#include <casa/namespace.h>

namespace casa {

class BufferInfo {
public:
    BufferInfo (Int chunk, Int subchunk, Double time) : chunk_p (chunk), subchunk_p (subchunk), time_p (time) {}

    Int chunk_p;
    Int subchunk_p;
    Double time_p;

    bool operator== (const BufferInfo & other) const
    {
        bool result = chunk_p == other.chunk_p && subchunk_p == other.subchunk_p &&
                      abs (time_p - other.time_p) <= tolerance_p;

        return result;
    }

    bool operator!= (const BufferInfo & other) const
    {
        return ! (*this == other);
    }
    static void setTolerance (Double tolerance) { tolerance_p = tolerance;}
    static Double tolerance_p;
};

Double BufferInfo::tolerance_p = 0;

typedef vector<BufferInfo> Buffers;

class Rovia_Test_Configuration;

class Rovia_Test {

public:

    static Bool checkScalars (const String & msNameSync,
                              const String & msNameAsync);
    static Bool compareComponents (const String & msNameSync,
                                   const String & msNameAsync,
                                   const String & componentName,
                                   String (* compareComponentSubchunk) (int, int, Rovia_Test_Configuration &));
    static String compareFlagCubes (int chunkNumber, int subchunkNumber, Rovia_Test_Configuration & rtc);
    static String compareImagingWeights (int chunkNumber, int subchunkNumber, Rovia_Test_Configuration & rtc);
    static Bool compareTimesAsyncToSync (const String & msName);
    static Bool compareWeightsAsyncToSync (const String & msNameSync, const String & msNameAsync);
    static void fillWeights (VisibilityIterator & vi, VisBuffer & vb);
    static void fillWeightsBoth (const String & msNameSync, const String & msNameAsync);
    static void sweepAsync (const String & msName, Buffers & buffers);
    static void sweepSync (const String & msName, Buffers & buffers);
    static void sweepVi (ROVisibilityIterator & vi, VisBuffer & vb, Buffers & buffers);

};

class Rovia_Test_Configuration {

public:

    Rovia_Test_Configuration (const String & msNameSync, const String & msNameAsync,
                              const PrefetchColumns * prefetchColumnsParam = NULL)
    : msAsync (0),
      msSync (0),
      vbAsync (),
      vbSync (0),
      viAsync (0),
      viSync (0)
    {
        msSync = new MeasurementSet (msNameSync, Table::Update);
        Block<Int> bi(0); // create empty block with sortColumns

        viSync = new ROVisibilityIterator (* msSync, bi);
        vbSync = new VisBuffer (* viSync);


        PrefetchColumns prefetchColumns;

        if (prefetchColumnsParam == NULL){
            prefetchColumns =
                    PrefetchColumns::prefetchColumns(VisBufferComponents::Ant1,
                                                     VisBufferComponents::Ant2,
                                                     VisBufferComponents::Freq,
                                                     VisBufferComponents::Time,
                                                     VisBufferComponents::ObservedCube,
                                                     VisBufferComponents::Sigma,
                                                     VisBufferComponents::Flag,
                                                     VisBufferComponents::Uvw,
                                                     -1);
        }
        else{
            prefetchColumns = * prefetchColumnsParam;
        }

        msAsync = new MeasurementSet (msNameAsync, Table::Update);

        viAsync = new ROVisibilityIterator (& prefetchColumns, * msAsync, bi);
        VisBufferAutoPtr p (viAsync);
        vbAsync = dynamic_cast<VisBufferAsync *> (p.release());
    }

    ~Rovia_Test_Configuration ()
    {
        delete msAsync;
        delete msSync;
        delete vbSync;
        delete viAsync;
        delete viSync;
    }

    MeasurementSet * msAsync;
    MeasurementSet * msSync;
    VisBufferAsync * vbAsync;
    VisBuffer * vbSync;
    ROVisibilityIterator * viAsync;
    ROVisibilityIterator * viSync;
};




}

#define VIA ROVisibilityIteratorAsync
int
main(int argc, char **argv)
{
    // register forward col engine
    ForwardColumnEngine::registerClass();

    uInt vid = Aipsrc::registerRC ("VisibilityIterator.disabled", "");
    Aipsrc::set (vid, "True"); // force async enabled

    if (argc<2) {
        cout <<"Usage: tVisibilityIterator ms-table-name"<<endl;
        exit(1);
    }
    // try to iterate over a pre-existing MS table


    try {

        mtrace();

        Bool ok = True;
        ok = Rovia_Test::checkScalars (argv[1], argv[2]) && ok;
        ok = Rovia_Test::compareComponents (argv[1], argv[2], "Imaging Weights", Rovia_Test::compareImagingWeights) && ok;
        ok = Rovia_Test::compareTimesAsyncToSync (argv[1]) && ok;
        ok = Rovia_Test::compareComponents (argv[1], argv[2], "Flag Cubes", Rovia_Test::compareFlagCubes) && ok;
        ok = Rovia_Test::compareWeightsAsyncToSync (argv[1], argv[2]) && ok;

        if (ok){
            cout << "--- Passed all tests!" << endl;
        }
        else{
            cout << "***" << endl;
            cout << "***" << " Failed one or more tests ;-(" << endl;
            cout << "***" << endl;
        }

        cout << "Exiting scope of tVisibilityIterator" << endl;

    } catch (AipsError x) {
        cout << "Caught exception " << endl;
        cout << x.getMesg() << endl;
        return 1;
    }
    cout << "Done." << endl;
    return 0;
}

namespace casa {

#define CheckScalar(x, y) \
    (!(rtc.vbAsync->x != y &&  (cout << "*** Scalar check failed; " << #x << " unitialized." << endl)))

Bool
Rovia_Test::checkScalars (const String & msNameSync,
                          const String & msNameAsync)
{
    Rovia_Test_Configuration rtc (msNameSync, msNameAsync);

    rtc.viAsync->origin();

    Bool ok = True;

    ok = CheckScalar (dataDescriptionId_p, -1) && ok;
    ok = CheckScalar (nAntennas_p, -1) && ok;
    ok = CheckScalar (nCoh_p, -1) && ok;
    ok = CheckScalar (newArrayId_p, False) && ok;
    ok = CheckScalar (newFieldId_p, False) && ok;
    ok = CheckScalar (newSpectralWindow_p, False) && ok;
    ok = CheckScalar (nRowChunk_p, -1) && ok;
    ok = CheckScalar (nSpw_p, -1) && ok;
    ok = CheckScalar (polarizationId_p, -1) && ok;

    if (ok){
        cout << "--- Scalar check passed." << endl;
    }
    else{
        cout << "***\n*** Scalar check FAILED.\n***" << endl;
    }

    return ok;
}


String
Rovia_Test::compareFlagCubes (int chunkNumber, int subchunkNumber, Rovia_Test_Configuration & rtc)
{
    ostringstream out;

    Cube<Bool> flagCubeSync = rtc.vbSync->flagCube ();
    Cube<Bool> flagCubeAsync = rtc.vbAsync->flagCube ();

    boost::tuple<Int,Int,Int> dimsSync (flagCubeSync.nrow(), flagCubeSync.ncolumn(),flagCubeSync.nplane());
    boost::tuple<Int,Int,Int> dimsAsync (flagCubeAsync.nrow(), flagCubeAsync.ncolumn(),flagCubeAsync.nplane());

    if (dimsSync != dimsAsync){
        out << "Flag Cube dims mismatch at (" << chunkNumber << "," << subchunkNumber << ")" << endl;
        out << "Sync dims = " << get<0> (dimsSync) << ". "
                << get<1> (dimsSync) << ". "
                << get<2> (dimsSync) << endl;
        out << "Async dims = " << get<0> (dimsAsync) << ". "
                << get<1> (dimsAsync) << ". "
                << get<2> (dimsAsync) << endl;

        return out.str();
    }

    for (int i = 0; i < (int) flagCubeSync.nrow(); i++){
        for (int j = 0; j < (int) flagCubeSync.ncolumn(); j++){
            for (int k = 0; k < (int) flagCubeSync.nplane (); k++){

                if (flagCubeSync (i,j,k) != flagCubeAsync (i,j,k)){

                    out << "Flag Cube dims mismatch at (" << chunkNumber << "," << subchunkNumber << ")" << endl;
                    out << "flagCubeSync (" << i << "," << j << "," << k << ") = " << flagCubeSync(i,j,k) << endl;
                    out << "flagCubeAsync (" << i << "," << j << "," << k << ") = " << flagCubeAsync(i,j,k) << endl;

                    return out.str();
                }

            }
        }
    }

    return "";
}

//void
//Rovia_Test::compareFlagCubes (const String & msNameSync, const String & msNameAsync)
//{
//
//    cout << "---Comparing flag cubes ..." << endl;
//
//    PrefetchColumns prefetchColumns =
//            PrefetchColumns::prefetchColumns(VisBufferComponents::Ant1,
//                                             VisBufferComponents::Ant2,
//                                             VisBufferComponents::FlagCube,
//                                             VisBufferComponents::Time,
//                                             VisBufferComponents::ObservedCube,
//                                             VisBufferComponents::Sigma,
//                                             VisBufferComponents::Flag,
//                                             VisBufferComponents::Uvw,
//                                             -1);
//
//    Rovia_Test_Configuration rtc (msNameSync, msNameAsync, & prefetchColumns);
//
//    int chunkNumber, subchunkNumber;
//
//    for (rtc.viSync->originChunks(), rtc.viAsync->originChunks (), chunkNumber = 0;
//         rtc.viSync->moreChunks();
//         rtc.viSync->nextChunk(), rtc.viAsync->nextChunk(), chunkNumber ++){
//
//        for (rtc.viSync->origin (), rtc.viAsync->origin(), subchunkNumber = 0;
//             rtc.viSync->more();
//             (* rtc.viSync) ++, (* rtc.viAsync) ++, subchunkNumber ++){
//        }
//
//        Cube<Bool> flagCubeSync = rtc.vbSync->flagCube ();
//        Cube<Bool> flagCubeAsync = rtc.vbAsync->flagCube ();
//
//        boost::tuple<Int,Int,Int> dimsSync (flagCubeSync.nrow(), flagCubeSync.ncolumn(),flagCubeSync.nplane());
//        boost::tuple<Int,Int,Int> dimsAsync (flagCubeAsync.nrow(), flagCubeAsync.ncolumn(),flagCubeAsync.nplane());
//
//        if (dimsSync != dimsAsync){
//            cout << "Flag Cube dims mismatch at (" << chunkNumber << "," << subchunkNumber << ")" << endl;
//            cout << "Sync dims = " << get<0> (dimsSync) << ". "
//                                   << get<1> (dimsSync) << ". "
//                                   << get<2> (dimsSync) << endl;
//            cout << "Async dims = " << get<0> (dimsAsync) << ". "
//                                    << get<1> (dimsAsync) << ". "
//                                    << get<2> (dimsAsync) << endl;
//
//            goto done;
//        }
//
//        for (int i = 0; i < (int) flagCubeSync.nrow(); i++){
//            for (int j = 0; j < (int) flagCubeSync.ncolumn(); j++){
//                for (int k = 0; k < (int) flagCubeSync.nplane (); k++){
//
//                    if (flagCubeSync (i,j,k) != flagCubeAsync (i,j,k)){
//
//                        cout << "Flag Cube dims mismatch at (" << chunkNumber << "," << subchunkNumber << ")" << endl;
//                        cout << "flagCubeSync (" << i << "," << j << "," << k << ") = " << flagCubeSync(i,j,k) << endl;
//                        cout << "flagCubeAsync (" << i << "," << j << "," << k << ") = " << flagCubeAsync(i,j,k) << endl;
//                        goto done;
//                    }
//
//                }
//            }
//        }
//
//    }
//
//    cout << "... Passed Flag Cube Test" << endl;
//
//    return;
//
//done:
//
//    cout << "*** Failed Flag Cube Test" << endl;
//
//    return;
//}

Bool
Rovia_Test::compareWeightsAsyncToSync (const String & msNameSync, const String & msNameAsync)
{
    cout << "\n--- Starting Write Tests ...\n\n";

    // Fill the weights column of both MSs with a ramp

    fillWeightsBoth (msNameSync, msNameAsync);

    // Now see if they match up.

    Bool ok = True;

    MeasurementSet msSync (msNameSync, Table::Update);
    Block<Int> bi(0); // create empty block with sortColumns

    ROVisibilityIterator syncVi (msSync, bi);
    VisBuffer syncVb (syncVi);

    MeasurementSet msAsync (msNameAsync, Table::Update);

    ROVisibilityIterator asyncVi (msAsync, bi);
    VisBuffer asyncVb (asyncVi);

    int chunkNumber, subchunkNumber;

    for (syncVi.originChunks(), asyncVi.originChunks (), chunkNumber = 0;
         syncVi.moreChunks();
         syncVi.nextChunk(), asyncVi.nextChunk(), chunkNumber ++){

        for (syncVi.origin (), asyncVi.origin(), subchunkNumber = 0;
             syncVi.more();
             syncVi ++, asyncVi ++, subchunkNumber ++){

            Vector<Float> syncWeights = syncVb.weight ();
            Vector<Float> asyncWeights = asyncVb.weight ();

            Vector<Float>::iterator iSync, iAsync;
            for (iSync = syncWeights.begin(), iAsync = asyncWeights.begin();
                 iSync != syncWeights.end();
                 iSync ++, iAsync ++){

                if (abs(* iSync - * iAsync) > 1e-6){

                    ok = false;

                    cout << "Comparison of written 'weight' failed.\n"
                         << "Expected " << * iSync << " but got " << * iAsync << ".\n"
                         << "Position = (" << chunkNumber << "," << subchunkNumber << ")\n";

                    goto done;
                }

            }

        }
    }

    cout << "... Comparison of written 'weight' data was successful!!!" << endl;
    return True;

done:

    cout << "*** Failed write test " << endl;
    return False;

}

Bool
Rovia_Test::compareTimesAsyncToSync (const String & msName)
{
    cout << "---Comparing Times ..." << endl;

    Buffers asyncBuffers, syncBuffers;
    sweepSync (msName, syncBuffers);
    sweepAsync (msName, asyncBuffers);

    if (syncBuffers.size() != asyncBuffers.size()){
        cout << "Different number of buffers: nSync=" << syncBuffers.size()
             << "; nAsync=" << asyncBuffers.size() << endl;
    }


    Buffers::const_iterator aIter=asyncBuffers.begin(), sIter=syncBuffers.begin();
    BufferInfo::setTolerance(.001);
    Double maxDt = 0;
    Double sumDt = 0;
    Double sumDtSq = 0;

    while (aIter != asyncBuffers.end() && sIter != syncBuffers.end()){
        Bool different = (aIter == asyncBuffers.end() || sIter == syncBuffers.end()) ||
                         * aIter != * sIter;

        String aText, sText;
        Double aT=0, sT=0;

        if (aIter != asyncBuffers.end()){
            aText = String::format ("(%4d,%4d, %15.3f)", aIter->chunk_p, aIter->subchunk_p, aIter->time_p);
            aT = aIter->time_p;
            aIter++;
        }
        else{
            aText = "-----------";
        }

        if (sIter != syncBuffers.end()){
            sText = String::format ("(%4d,%4d, %15.3f)", sIter->chunk_p, sIter->subchunk_p, sIter->time_p);
            sT = sIter->time_p;
            sIter++;
        }
        else{
            sText = "-----------";
        }

        Double dt = 0;

        if (aT != 0 && sT != 0){
            Double dt = aT - sT;
            maxDt = max (maxDt, abs (dt));
            sumDt += dt;
            sumDtSq += dt * dt;
        }

        if (different || true)
            cout << "a=" << aText << "; b=" << sText << "; dt=" << dt << endl;
    }

    Bool result = True;

    if (syncBuffers.size() != asyncBuffers.size()){
        cout << "Different number of buffers: nSync=" << syncBuffers.size()
             << "; nAsync=" << asyncBuffers.size() << endl;
        cout << "*** Failed Time Comparison test" << endl;

        result = False;

    }
    else {
        cout << "ROVI and ROVIA results match.  " << asyncBuffers.size() << " buffers read." << endl;
        cout << "... Passed Time Comparison test" << endl;
    }
    Int n = asyncBuffers.size();
    Double avg = sumDt / n;
    Double stdDev = sqrt (sumDtSq / n - avg * avg) * (n / (n - 1.0));
    cout << String::format ("max(dt)=%f; avg(dt)=%f; stdev(dt)=%f (%f %%)", maxDt, avg, stdDev, stdDev / avg * 100) << endl;

    return result;

}

Bool
Rovia_Test::compareComponents (const String & msNameSync, const String & msNameAsync,
                               const String & componentName,
                               String (* compareComponentSubchunk) (int, int, Rovia_Test_Configuration &))
{

    cout << "---Comparing " << componentName << " ..." << endl;

    PrefetchColumns prefetchColumns =
            PrefetchColumns::prefetchColumns(VisBufferComponents::Ant1,
                                             VisBufferComponents::Ant2,
                                             VisBufferComponents::FlagCube,
                                             VisBufferComponents::Time,
                                             VisBufferComponents::ObservedCube,
                                             VisBufferComponents::Sigma,
                                             VisBufferComponents::Flag,
                                             VisBufferComponents::Uvw,
                                             VisBufferComponents::Weight,
                                             -1);

    Rovia_Test_Configuration rtc (msNameSync, msNameAsync, & prefetchColumns);

    int chunkNumber, subchunkNumber;

    for (rtc.viSync->originChunks(), rtc.viAsync->originChunks (), chunkNumber = 0;
            rtc.viSync->moreChunks();
            rtc.viSync->nextChunk(), rtc.viAsync->nextChunk(), chunkNumber ++){

        for (rtc.viSync->origin (), rtc.viAsync->origin(), subchunkNumber = 0;
                rtc.viSync->more();
                (* rtc.viSync) ++, (* rtc.viAsync) ++, subchunkNumber ++){

            String message = compareComponentSubchunk (chunkNumber, subchunkNumber, rtc);

            if (! message.empty()){

                cout << message << endl;
                goto done;
            }
        }
    }

    cout << "... Passed " << componentName << " Test" << endl;

    return True;

done:

    cout << "***" << endl;
    cout << "*** Failed " << componentName << " Test" << endl;
    cout << "***" << endl;

    return False;
}


void
Rovia_Test::sweepAsync (const String & msName, Buffers & buffers)
{
    MeasurementSet ms(msName, Table::Update);

    Block<Int> bi(0); // create empty block with sortColumns

    PrefetchColumns prefetchColumns =
        PrefetchColumns::prefetchColumns(VisBufferComponents::Ant1,
                                         VisBufferComponents::Ant2,
                                         VisBufferComponents::Freq,
                                         VisBufferComponents::Time,
                                         VisBufferComponents::ObservedCube,
                                         VisBufferComponents::Sigma,
                                         VisBufferComponents::Flag,
                                         VisBufferComponents::Uvw,
                                         -1);
    ROVisibilityIterator vi (& prefetchColumns, ms, bi);
    VisBufferAutoPtr vb (vi); // will always do async when passed an ROVIA
    if (! vi.isAsynchronous ()){
        throw AipsError ("Async VI is not async!", __FILE__, __LINE__);
    }

    sweepVi (vi, * vb, buffers);
}

void
Rovia_Test::sweepSync (const String & msName, Buffers & buffers)
{
   MeasurementSet ms(msName, Table::Update);

    Block<Int> bi(0); // create empty block with sortColumns

    ROVisibilityIterator vi (ms,bi);
    VisBuffer vb (vi);

    sweepVi (vi, vb, buffers);
}


void
Rovia_Test::sweepVi (ROVisibilityIterator & vi, VisBuffer & vb, Buffers & buffers)
{
    buffers.clear();

    Int chunkNumber, subchunkNumber;
    for (vi.originChunks(), chunkNumber = 0; vi.moreChunks(); vi.nextChunk(), chunkNumber ++){
        for (vi.origin (), subchunkNumber = 0; vi.more(); vi ++, subchunkNumber ++){
            buffers.push_back (BufferInfo (chunkNumber, subchunkNumber, vb.time()[0]));
        }
    }
}

void
Rovia_Test::fillWeights (VisibilityIterator & vi, VisBuffer & vb)
{
    Int chunkNumber, subchunkNumber;

    int n = 0;

    for (vi.originChunks(), chunkNumber = 0; vi.moreChunks(); vi.nextChunk(), chunkNumber ++){
        for (vi.origin (), subchunkNumber = 0; vi.more(); vi ++, subchunkNumber ++){

            Vector<Float> weights = vb.weight ();

	    //cout << "Writing subchunk (" << chunkNumber << "," << subchunkNumber << ") [0] = " << n << endl;

            for (Vector<Float>::iterator i = weights.begin(); i != weights.end(); i++){
                * i = n ++;
            }

            vi.setWeight (weights);
        }
    }
}

void
Rovia_Test:: fillWeightsBoth (const String & msNameSync, const String & msNameAsync)
{

    MeasurementSet msSync (msNameSync, Table::Update);
    Block<Int> bi(0); // create empty block with sortColumns

    VisibilityIterator syncVi (msSync, bi);
    VisBuffer syncVb (syncVi);

    fillWeights (syncVi, syncVb);

    MeasurementSet msAsync (msNameAsync, Table::Update);

    PrefetchColumns prefetchColumns =
        PrefetchColumns::prefetchColumns(VisBufferComponents::Ant1,
                                         VisBufferComponents::Ant2,
                                         VisBufferComponents::Freq,
                                         VisBufferComponents::Time,
                                         VisBufferComponents::ObservedCube,
                                         VisBufferComponents::Sigma,
                                         VisBufferComponents::Flag,
                                         VisBufferComponents::Weight,
                                         VisBufferComponents::Uvw,
                                         -1);
    VisibilityIterator asyncVi (& prefetchColumns, msAsync, bi);
    VisBufferAutoPtr asyncVb (asyncVi);

    if (! asyncVi.isAsynchronous ()){
        throw AipsError ("Async VI is not async!", __FILE__, __LINE__);
    }

    fillWeights (asyncVi, * asyncVb);
}

String
Rovia_Test::compareImagingWeights (int chunkNumber, int subchunkNumber, Rovia_Test_Configuration & rtc)
{
    static Bool initialized = False;


    if (! initialized){
        initialized = True;
        rtc.viSync->useImagingWeight (VisImagingWeight ("natural"));
        rtc.viAsync->useImagingWeight (VisImagingWeight ("natural"));
    }

    Matrix<Float> iwSync = rtc.vbSync->imagingWeight();
    Matrix<Float> iwAsync = rtc.vbAsync->imagingWeight();

    Float syncMin, syncMax;
    IPosition minPos, maxPos;

    minMax (syncMin, syncMax, minPos, maxPos, iwSync);

    Bool ok = allNearAbs (iwSync, iwAsync, syncMax * 1e-6);

    String message = ok ? "" : String::format ("Imaging weight comparison failure at subchunk (%d,%d)",
                                              chunkNumber, subchunkNumber);

    if (! ok){

        if (iwSync.nrow() != iwAsync.nrow() || iwSync.ncolumn() != iwAsync.ncolumn()){
            message += String::format ("\nDimension mismatch: sync (%d,%d) != async (%d,%d)",
                                      iwSync.nrow(), iwSync.ncolumn(), iwAsync.nrow() , iwAsync.ncolumn());
        }

        for (int i = 0; i < (int) iwSync.nrow(); i++){
            for (int j = 0; j < (int) iwSync.ncolumn(); j++){
                if (abs(iwSync(i,j) - iwAsync(i,j)) > 1e-6){
                    message += String::format ("\nValue mismatch at (%d,%d): sync=%f, async=%f",
                                              i, j, iwSync(i,j), iwAsync(i,j));
                }
            }
        }
    }

    return message;
}


}

