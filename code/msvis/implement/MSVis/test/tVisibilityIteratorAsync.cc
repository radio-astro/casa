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

#include <msvis/MSVis/VisibilityIterator.h>
#include <msvis/MSVis/VisBufferAsync.h>
#include <msvis/MSVis/UtilJ.h>
#include <tables/Tables.h>
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <casa/iomanip.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/BasicSL/Constants.h>
#include <memory>
#include <mcheck.h>

using namespace std;
using namespace casa::utilj;
using namespace casa;
using namespace casa::asyncio;

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

class Rovia_Test {

public:

    static void compareTimesAsyncToSync (const String & msName);
    static void compareWeightsAsyncToSync (const String & msNameSync, const String & msNameAsync);
    static void fillWeights (VisibilityIterator & vi, VisBuffer & vb);
    static void fillWeightsBoth (const String & msNameSync, const String & msNameAsync);
    static void sweepAsync (const String & msName, Buffers & buffers);
    static void sweepSync (const String & msName, Buffers & buffers);
    static void sweepVi (ROVisibilityIterator & vi, VisBuffer & vb, Buffers & buffers);
};

}

#define VIA ROVisibilityIteratorAsync
int
main(int argc, char **argv)
{
    // register forward col engine
    ForwardColumnEngine::registerClass();

    if (argc<2) {
        cout <<"Usage: tVisibilityIterator ms-table-name"<<endl;
        exit(1);
    }
    // try to iterate over a pre-existing MS table


    try {

        mtrace();

        Rovia_Test::compareTimesAsyncToSync (argv[1]);
        Rovia_Test::compareWeightsAsyncToSync (argv[1], argv[2]);

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

void
Rovia_Test::compareWeightsAsyncToSync (const String & msNameSync, const String & msNameAsync)
{
    cout << "\n--- Starting Write Tests:\n\n";

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

    cout << "Comparison of written 'weight' data was successful!!!" << endl;

done:

    return;

}

void
Rovia_Test::compareTimesAsyncToSync (const String & msName)
{
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
            aText = format ("(%4d,%4d, %15.3f)", aIter->chunk_p, aIter->subchunk_p, aIter->time_p);
            aT = aIter->time_p;
            aIter++;
        }
        else{
            aText = "-----------";
        }

        if (sIter != syncBuffers.end()){
            sText = format ("(%4d,%4d, %15.3f)", sIter->chunk_p, sIter->subchunk_p, sIter->time_p);
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

    if (syncBuffers.size() != asyncBuffers.size()){
        cout << "Different number of buffers: nSync=" << syncBuffers.size()
             << "; nAsync=" << asyncBuffers.size() << endl;
    }
    else {
        cout << "ROVI and ROVIA results match.  " << asyncBuffers.size() << " buffers read." << endl;
    }
    Int n = asyncBuffers.size();
    Double avg = sumDt / n;
    Double stdDev = sqrt (sumDtSq / n - avg * avg) * (n / (n - 1.0));
    cout << format ("max(dt)=%f; avg(dt)=%f; stdev(dt)=%f (%f %%)", maxDt, avg, stdDev, stdDev / avg * 100) << endl;

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

	    cout << "Writing subchunk (" << chunkNumber << "," << subchunkNumber << ") [0] = " << n << endl;

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

}

