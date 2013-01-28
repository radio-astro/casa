#include <casa/BasicSL.h>
#include <casa/string.h>
#include <ms/MeasurementSets.h>
#include <synthesis/MSVis/VisBuffer2.h>
#include <synthesis/MSVis/VisibilityIterator2.h>

#include <boost/tuple/tuple.hpp>
#include <map>
#include <memory>
#include <utility>

using namespace std;

#include "MsFactory.cc"

namespace casa {

namespace vi {

namespace test {



#define TestErrorIf(c,m) {if (c) ThrowTestError (m);}

#define ThrowTestError(m) \
    throw TestError (m, __func__, __FILE__, __LINE__);

class TestError : public std::exception {

public:

    TestError (const String & message, const String & function,
               const String & file, int lineNumber)
    : file_p (file),
      function_p (function),
      lineNumber_p (lineNumber),
      message_p (message)
    {}

    ~TestError () throw () {}

    const char * what () const throw () {
        what_p = String::format ("TestError: %s (in %s at %s:%d)", message_p.c_str(),
                                 function_p.c_str(), file_p.c_str(), lineNumber_p);
        return what_p.c_str();
    }

private:

    String file_p;
    String function_p;
    Int lineNumber_p;
    String message_p;
    mutable String what_p;

};


class Tester {

public:

    void doTests (Int nArgs, char * args []);


protected:

    typedef std::map<String, String> Arguments;

    pair<MeasurementSet *,Int> createMs (const String &);
    Arguments parseArgs (int nArgs, char * args []) const;
    void sweepMs (const String & msName, Double interval,
                  Double chunkInterval, Int averagingFactor);
    void checkRowData (VisBuffer2 * vb, VisibilityIterator2 * vi, Int subchunk,
                       Int firstRow, Int averagingFactor);
    void checkVisCube (Int rowId, Int spectralWindow, Int row, Int channel, Int correlation,
                       const Cube<Complex> & cube, const String & tag,
                       Int antenna1, Int antenna2, Int subchunkIndex, Int averagingFactor,
                       Double time);
    void checkVisCubes (VisBuffer2 * vb, VisibilityIterator2 * vi, Int subchunk,
                        Int firstRow, Int averagingFactor);

private:

    MsFactory * msf_p;
};

} // end namespace test
} // end namespace vi
} // end namespace casa

int
main (int nArgs, char * args [])
{
    casa::vi::test::Tester tester;

    tester.doTests (nArgs, args);
}

namespace casa {

namespace vi {

namespace test {

#define DeltaEq(act,exp,tol) ((exp != 0) ? (abs(((act)-(exp))/(exp)) < tol) : (abs(act)<tol))

void
Tester::checkRowData (VisBuffer2 * vb, VisibilityIterator2 * vi, Int subchunk,
                      Int firstRow, Int averagingFactor)
{
    for (Int row = 0; row < vb->nRows(); row ++){

        TestErrorIf (! DeltaEq (vb->timeInterval()(row), averagingFactor, .001),
                     String::format ("For interval: expected %f got %f at subchunk=%d, msRow=%d",
                                     vb->timeInterval()(row),
                                     (Double) averagingFactor,
                                     subchunk,
                                     row+firstRow));

        TestErrorIf (! DeltaEq (vb->time()(row), averagingFactor * subchunk, .001),
                     String::format ("For time: expected %f got %f at subchunk=%d, msRow=%d",
                                     vb->time()(row),
                                     averagingFactor,
                                     subchunk,
                                     row+firstRow));

        // Need to add tests for some of the other row data

    }
}

void
Tester::checkVisCubes (VisBuffer2 * vb, VisibilityIterator2 * vi, Int subchunk,
                       Int firstRow, Int averagingFactor)
{
    for (Int row = 0; row < vb->nRows(); row ++){

        for (Int channel = 0; channel < vb->nChannels(); channel ++){

            for (Int correlation = 0; correlation < vb->nCorrelations(); correlation ++){

                checkVisCube (firstRow + row, vb->spectralWindow(), row, channel, correlation,
                              vb->visCube(), "Visibility", vb->antenna1()(row),
                              vb->antenna2()(row), subchunk, averagingFactor, vb->time()(row));

            }
        }
    }
}

void
Tester::checkVisCube (Int rowId, Int spectralWindow, Int row, Int channel, Int correlation,
                      const Cube<Complex> & cube, const String & tag, Int antenna1,
                      Int antenna2, Int subchunkIndex, Int averagingFactor, Double time)
{
    Float imaginary = (((antenna1 * 10 + antenna2) * 10 +
                         spectralWindow) * 10 + correlation) * 10 + channel;


    Float real = (subchunkIndex * averagingFactor) + ((averagingFactor - 1) / 2);

    Complex z0 (real, imaginary);
    //z0 *= factor_p;
    Complex z = cube (correlation, channel, row);

    Bool ok = imaginary == z.imag();
    ok = ok && abs (real - z.real()) <  averagingFactor * 0.01;

    TestErrorIf (! ok,
                 String::format("Expected (%f,%f), got (%f,%f) for %s vis at:\n"
                         "spw=%d, vbRow=%d, msRow=%d, ch=%d, corr=%d, a1=%d, a2=%d, subchunk=%d, t=%f",
                         z0.real(), z0.imag(),
                         z.real(), z.imag(),
                         tag.c_str(),
                         spectralWindow,
                         row,
                         rowId,
                         channel,
                         correlation,
                         antenna1,
                         antenna2,
                         subchunkIndex,
                         time));
}


void
Tester::doTests (Int nArgs, char * args [])
{
    pair<Bool,Bool> result;

    try {
        Arguments arguments = parseArgs (nArgs, args);

        // Create MS to be averaged

        MeasurementSet * ms;
        Int nRows;

        String msName ("AveragingTvi2.ms");
        boost::tie (ms, nRows) = createMs (msName);

        sweepMs (msName, 1, 10, 1); // interval, chunkInterval, factor


    }
    catch (TestError & e){

        fprintf (stderr, "*** TestError while executing test %s:\n-->%s\n",
                 "", e.what());
    }
    catch (AipsError & e){

        fprintf (stderr, "*** AipsError while executing test %s:\n-->%s\n",
                 "", e.what());

    }
    catch (...){

        fprintf (stderr, "*** Unknown exception while executing test %s\n***\n*** Exiting ***\n",
                 "");
    }

}

class GenerateRamp : public Generator<Complex> {

public:

    GenerateRamp () : counter_p (4096, 0), previousTime_p (4096, -1) {}

    Complex
    operator() (const FillState & fillState, Int channel, Int correlation) const
    {
        // Generate a ramp that increments every new set of baselines.

        if (! DeltaEq (fillState.time_p, previousTime_p [fillState.spectralWindow_p], 1E10)){

            counter_p [fillState.spectralWindow_p] += 1;
            previousTime_p [fillState.spectralWindow_p] = fillState.time_p;
        }

        Float imaginary = (((fillState.antenna1_p * 10 + fillState.antenna2_p) * 10 +
                            fillState.spectralWindow_p) * 10 + correlation) * 10 + channel;

        Float real = counter_p [fillState.spectralWindow_p];

        return Complex (real, imaginary);
    }

private:

    mutable Vector <Int> counter_p;
    mutable Vector <Double> previousTime_p;
};

pair<MeasurementSet *, Int>
Tester::createMs (const String & msName)
{
    system (String::format ("rm -r %s", msName.c_str()).c_str());

    msf_p = new MsFactory (msName);

    msf_p->setTimeInfo (0,1000,1);

    auto_ptr<GenerateRamp> rampGenerator (new GenerateRamp());
    msf_p->setDataGenerator(MSMainEnums::DATA, rampGenerator.get());

    pair<MeasurementSet *, Int> p = msf_p->createMs ();

    return make_pair (p.first, p.second);
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

void
Tester::sweepMs (const String & msName, Double interval,
                 Double chunkInterval, Int averagingFactor)
{
    MeasurementSet ms (msName);

    AveragingTvi2Factory factory;
    VisibilityIterator2 * vi = factory.createVi (& ms, interval, chunkInterval, averagingFactor);
    VisBuffer2 * vb = vi->getVisBuffer();

    Int subchunk = 0;
    Int firstRow = 0;

    for (vi->originChunks(); vi->moreChunks(); vi->nextChunk()){
        for (vi->origin(); vi->more(); vi->next()){

            checkVisCubes (vb, vi, subchunk, firstRow, averagingFactor);

            checkRowData (vb, vi, subchunk, firstRow, averagingFactor);

            subchunk ++;

            firstRow += vb->nRows();

        }
    }
}



} // end namespace test
} // end namespace vi
} // end namespace casa
