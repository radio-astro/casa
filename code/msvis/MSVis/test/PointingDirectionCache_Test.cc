/*
 * PointingDirectionCache_Test.cc
 *
 *  Created on: Dec 5, 2016
 *      Author: jjacobs
 */

#include <gtest/gtest.h>
#include <msvis/MSVis/PointingDirectionCache.h>

#include <casacore/casa/BasicSL/String.h>
#include <casacore/measures/Measures/MDirection.h>
#include <casacore/casa/Quanta/Quantum.h>
#include <casacore/casa/Arrays/Vector.h>
#include <casacore/casa/Exceptions/Error.h>

#undef casacore

using namespace casa;
using namespace casa::vi;
using namespace casa::vi::pd_cache;
using casacore::String;

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
    int lineNumber_p;
    String message_p;
    mutable String what_p;

};

class TestPointingSource : public casa::vi::PointingSource {

    // Wrapper class to provide pointing subtable information into
    // the cache.

public:

    TestPointingSource (const vector<int> & antennas, double timeMin,
                        double timeMax, double timeInterval,
                        int timesPerBlock, double timeIntervalStored = -1);

    virtual casa::vi::Pointing getPointingRow (int row, double time, bool asMeasure) const override;
    virtual int nRows () const override;

private:

    vector<int> antennas_p;
    mutable int antennaIndex_p;
    mutable int blockIndex_p;
    mutable double blockTimeOrigin_p;
    mutable vector<casa::vi::Pointing> pointings_p;
    mutable double time_p;
    double timeInterval_p;
    double timeIntervalStored_p;
    double timeMax_p;
    double timeMin_p;
    int timesPerBlock_p;
};

TestPointingSource::TestPointingSource (const vector<int> & antennas, double timeMin,
                                        double timeMax, double timeInterval,
                                        int timesPerBlock, double timeIntervalStored)
: antennas_p (antennas),
  antennaIndex_p (0),
  blockIndex_p (-1),
  blockTimeOrigin_p (0),
  time_p (timeMin - timeInterval),
  timeInterval_p (timeInterval),
  timeIntervalStored_p (timeIntervalStored < 0 ? timeInterval : timeIntervalStored),
  timeMax_p (timeMax),
  timeMin_p (timeMin),
  timesPerBlock_p (timesPerBlock)
{}


casa::vi::Pointing
TestPointingSource::getPointingRow (int row, double, bool asMeasure) const
{
    ThrowIf (row < 0, "Negative row requested.");

//printf ("TestPointingSource::getPointingRow row=%d, asMeasure=%d\n", row, asMeasure);


    if (asMeasure){
//printf (".................................. ant=%d, t=%f (1)\n", pointings_p[row].antennaId, pointings_p[row].time);
        return pointings_p [row];
    }

    if (row == 0){

        // Start at the beginning of the file

        antennaIndex_p = 0;
        blockIndex_p = 0;
        blockTimeOrigin_p = timeMin_p;
        time_p = timeMin_p;

    } else {

        blockIndex_p += 1;
        time_p += timeInterval_p;

        if (blockIndex_p >= timesPerBlock_p || time_p > timeMax_p){

            blockIndex_p = 0;
            antennaIndex_p += 1;

            if (antennaIndex_p >= (int) antennas_p.size()){

                antennaIndex_p = 0;
                blockTimeOrigin_p += timesPerBlock_p * timeInterval_p;
                time_p = blockTimeOrigin_p;
            }

            time_p = blockTimeOrigin_p;

        }
    }

    ThrowIf (time_p > timeMax_p, "Requested time after end of simulation.");

    Pointing pointing;
    pointing.time = time_p;
    pointing.interval = timeIntervalStored_p * 0.5;
    pointing.antennaId = antennas_p [antennaIndex_p];
    pointing.row = row;

    double a1 = (pointing.time - timeMin_p) / (timeMax_p - timeMin_p); // [0,1]
    double a2 = (pointing.antennaId / (antennas_p.size() * 1.0));    
    pointing.direction = casacore::MDirection (casacore::Quantity (a1, "rad"),
                                               casacore::Quantity (a2, "rad"));

    pointings_p.push_back (pointing);
//printf (".................................. ant=%d, t=%f (2)\n", pointings_p[row].antennaId, pointings_p[row].time);


//    printf ("Generating pointing[%d]: t=%f, int=%f ant=%d, a1=%f, a2=%f\n",
//            row, pointing.time, pointing.interval, pointing.antennaId, a1, a2);

    pointing.direction = casacore::MDirection ();  // dummy
    pointing.valid = false;
    return pointing;
}


int
TestPointingSource::nRows () const
{
    int nTimes = (timeMax_p - timeMin_p) / timeInterval_p + 1.5;
    return nTimes * antennas_p.size();
}

void
doTest (const vector<int> & antennas, double timeMin, double timeMax, double timeInterval,
        double timesPerBlock, bool wrap = false)
{
    TestPointingSource pointingSource (antennas, timeMin, timeMax, timeInterval, timesPerBlock);

    casa::vi::PointingDirectionCache cache (antennas.size(), pointingSource);

    int nRepeats = (wrap) ? 2 : 1;

    for (int repeat = 0; repeat < nRepeats; repeat ++){

        for (double time = timeMin; time <= timeMax; time += timeInterval){

            for (int antenna = 0; antenna < (int) antennas.size(); antenna ++){

                casacore::MDirection direction;
                bool ok;
                std::tie (ok, direction) = cache.getPointingDirection (antenna, time, casacore::MDirection());
                TestErrorIf (! ok, String::format ("Cache miss for ant=%d, t=%f", antenna, time));
                casacore::Quantum<casacore::Vector<double>> qv = direction.getAngle();
                casacore::Vector<double> dv = qv.getValue();
                double angle1 = dv [0];
                double angle2 = dv [1];

                double expected1 = (time - timeMin) / (timeMax - timeMin);
                double expected2 = (antenna / (antennas.size() * 1.0));

                double delta1 = (expected1 - angle1);
                double delta2 = (expected2 - angle2);

                TestErrorIf (abs(delta1) > 1e-6 || abs(delta2) > 1e-6,
                             String::format ("Error: expected %f != %f or expected %f != %f; t=%f, ant=%d",
                                             expected1, angle1, expected2, angle2, time, antenna));
            }
        }
    }
}

void
testMiss (const vector<int> & antennas, double timeMin, double timeMax, double timeInterval,
        double timesPerBlock)
{
    TestPointingSource pointingSource (antennas, timeMin, timeMax, timeInterval, timesPerBlock);

    casa::vi::PointingDirectionCache cache (antennas.size(), pointingSource);

    EXPECT_THROW (cache.getPointingDirection (0, timeMax + 2 * timeInterval, casacore::MDirection()),
                  casacore::AipsError);
}

void
testGapMiss (const vector<int> & antennas, double timeMin, double timeMax, double timeInterval,
             double timesPerBlock)
{
    TestPointingSource pointingSource (antennas, timeMin, timeMax, timeInterval, timesPerBlock,
                                       timeInterval * 0.5);

    casa::vi::PointingDirectionCache cache (antennas.size(), pointingSource);

    EXPECT_THROW (cache.getPointingDirection (0, timeMin + 0.5 * timeInterval, casacore::MDirection()),
                  casacore::AipsError);
}

void
testBadMiss (const vector<int> & antennas, double timeMin, double timeMax, double timeInterval,
             double timesPerBlock)
{
    // Request a time before the earliest one in the data source.

    TestPointingSource pointingSource (antennas, timeMin, timeMax, timeInterval, timesPerBlock);

    casa::vi::PointingDirectionCache cache (antennas.size(), pointingSource);

    EXPECT_THROW (cache.getPointingDirection (0, timeMin - timeInterval, casacore::MDirection()),
                  casacore::AipsError);
}

int
main (int nArgs, char * args [])
{
    ::testing::InitGoogleTest(& nArgs, args);

    return RUN_ALL_TESTS();

}

TEST (PointingDirectionCache, SmallVeryRegular)
{
    doTest (vector<int> ({0, 1, 2}), 100, 200, 10, 1);
}

TEST (PointingDirectionCache, SmallWithTimeBlocking)
{
    doTest (vector<int> ({0, 1, 2}), 100, 200, 10, 5);
}

TEST (PointingDirectionCache, SmallIrregular)
{
    doTest (vector<int> ({0, 2, 1}), 100, 200, 10, 1);
    doTest (vector<int> ({2, 1, 0}), 100, 200, 10, 1);
}

TEST (PointingDirectionCache, SmallIrregularTimeBlocking)
{
    doTest (vector<int> ({0, 2, 1}), 100, 200, 10, 5);
    doTest (vector<int> ({2, 1, 0}), 100, 200, 10, 5);
}

TEST (PointingDirectionCache, SmallWrap)
{
    doTest (vector<int> ({0, 1, 2}), 100, 200, 10, 1, true);
}

TEST (PointingDirectionCache, TestMiss)
{
    testMiss (vector<int> ({0, 1, 2}), 100, 200, 10, 1);
}

TEST (PointingDirectionCache, TestGapMiss)
{
    testGapMiss (vector<int> ({0, 1, 2}), 100, 200, 10, 1);
}

TEST (PointingDirectionCache, TestBadMiss)
{
    testGapMiss (vector<int> ({0, 1, 2}), 100, 200, 10, 1);
}

TEST (PointingDirectionCache, BigVeryRegular)
{
    doTest (vector<int> ({0, 1, 2}), 100, 20000, 10, 1);
}

TEST (PointingDirectionCache, BigWithTimeBlocking)
{
    doTest (vector<int> ({0, 1, 2}), 100, 20000, 10, 5);
}

TEST (PointingDirectionCache, BigIrregular)
{
    doTest (vector<int> ({0, 2, 1}), 100, 20000, 10, 1);
    doTest (vector<int> ({2, 1, 0}), 100, 20000, 10, 1);
}

TEST (PointingDirectionCache, BigIrregularTimeBlocking)
{
    doTest (vector<int> ({0, 2, 1}), 100, 20000, 10, 5);
    doTest (vector<int> ({2, 1, 0}), 100, 20000, 10, 5);
}

TEST (PointingDirectionCache, BigVeryRegularWrap)
{
    doTest (vector<int> ({0, 1, 2}), 100, 20000, 10, 1, true);
}

TEST (PointingDirectionCache, TestBigMiss)
{
    testMiss (vector<int> ({0, 1, 2}), 100, 20000, 10, 1);
}




