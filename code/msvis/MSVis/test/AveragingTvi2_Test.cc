#include <casa/BasicSL.h>
#include <casa/string.h>
#include <casa/Logging.h>
#include <ms/MeasurementSets.h>
#include <msvis/MSVis/VisBuffer2.h>
#include <msvis/MSVis/VisibilityIterator2.h>
#include <msvis/MSVis/test/MsFactory.h>
#include <msvis/MSVis/AveragingVi2Factory.h>

#include <boost/tuple/tuple.hpp>
#include <map>
#include <memory>
#include <utility>
#include <cmath>

using namespace std;

namespace casa {

namespace vi {

namespace test {



#define TestErrorIf(c,m) {if (c) ThrowTestError (m);}

#define ThrowTestError(m) \
    throw TestError (m, __func__, __FILE__, __LINE__);

class Arguments : public std::map<String, String> {};
class Environment : public std::map<String, String> {};

template<typename T, typename U>
Bool
deltaEq(const T & act, const U & exp, Double tol)
{
    Bool eq;

    if (exp != 0){
        eq = abs((act - exp) / exp) < tol;
    }
    else{
        eq = abs(act) < tol;
    }

    return eq;
}


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

////////////////////////////////////////////////////////////
//
// CubeCheckers
//

class CubeChecker {

public:

    virtual ~CubeChecker () {}

    virtual void checkCube (VisBuffer2 * vb, const VisibilityIterator2 * vi, Int rowId,
                            Int row, Int channel, Int correlation, Int subchunkIndex) = 0;

protected:

    void throwError (const String & expected, const String & actual,
                     const VisBuffer2 * vb, Int rowId, Int row, Int channel, Int correlation,
                     Int subchunkIndex, const String & objectName, const String & extra = String ()) const;
};

class WeightChecker : public CubeChecker {

public:

    WeightChecker (Float factor) : factor_p ((Float) factor) {}

    void
    checkCube (VisBuffer2 * vb, const VisibilityIterator2 * /*vi*/, Int rowId,
               Int row, Int channel, Int correlation, Int subchunkIndex)
    {
        Float weight = vb->weightSpectrum() (correlation, channel, row);

        Bool ok = deltaEq (weight, factor_p, .001);

        if (! ok){
            String expected = String::format ("%f", factor_p);
            String actual = String::format ("%f", weight);

            throwError (expected, actual, vb, rowId, row, channel, correlation, subchunkIndex,
                        "weightSpectrum", String::format ("factor=%d", factor_p));
        }
    }

private:

    Float factor_p;
};

class CheckerboardFlagCubeChecker : public CubeChecker {

public:

    typedef const Cube<Complex> & (VisBuffer2::* Accessor) () const;

    CheckerboardFlagCubeChecker () : name_p ("FlagCube") {}

    void checkCube (VisBuffer2 * vb, const VisibilityIterator2 * /*vi*/, Int rowId,
                    Int row, Int channel, Int correlation, Int subchunkIndex)
    {
        Bool actualFlag = vb->flagCube() (correlation, channel, row);

        Bool expectedFlag = ((correlation & 0x1) ^ (channel & 0x1)) != 0;

        if (actualFlag != expectedFlag){

            String expected = String::format ("%d", expectedFlag);
            String actual = String::format ("%d", actualFlag);

            throwError (expected, actual, vb, rowId, row, channel, correlation, subchunkIndex, name_p);
        }
    }

private:

    Accessor accessor_p;
    Int averagingFactor_p;
    Float factor_p;
    String name_p;

};

typedef vector<pair<Int, Int> > EndBoundaryConditions;

class ComplexCubeRampChecker : public CubeChecker {

public:

    typedef const Cube<Complex> & (VisBuffer2::* Accessor) () const;

    ComplexCubeRampChecker (Int averagingFactor, const String & name,Accessor accessor,
                            Int lastRow, Int nRowsInPartialAverage, Float factor = 1)
    : accessor_p (accessor), averagingFactor_p (averagingFactor),
      endBoundaryConditions_p (EndBoundaryConditions (1, make_pair (lastRow, nRowsInPartialAverage))),
      factor_p (factor), name_p (name)
    {}

    ComplexCubeRampChecker (Int averagingFactor, const String & name, Accessor accessor,
                            const EndBoundaryConditions & endBoundaryConditions,
                            Float factor = 1)
    : accessor_p (accessor), averagingFactor_p (averagingFactor),
      endBoundaryConditions_p (endBoundaryConditions),
      factor_p (factor), name_p (name)
    {}

    void checkCube (VisBuffer2 * vb, const VisibilityIterator2 * /*vi*/, Int rowId,
                    Int row, Int channel, Int correlation, Int subchunkIndex)
    {
        Float imaginary = (((vb->antenna1 ()(row) * 10 + vb->antenna2() (row)) * 10 +
                vb->spectralWindows()(row)) * 10 + correlation) * 10 + channel;

        Int ddId = vb->dataDescriptionIds()(0);
        Assert (ddId < (Int) endBoundaryConditions_p.size());
        Int lastRow = endBoundaryConditions_p [ddId].first;
        Int nRowsInPartialAverage = endBoundaryConditions_p [ddId].second;

        Float real = -1;
        if (rowId < lastRow){

            real = (subchunkIndex * averagingFactor_p) + ((averagingFactor_p - 1) / 2.0);
        }
        else{
            real = (subchunkIndex * averagingFactor_p) + ((nRowsInPartialAverage - 1) / 2.0);
        }

        real *= factor_p;

        Complex z0 (real, imaginary);
        //z0 *= factor_p;
        Complex z = (vb ->* accessor_p) () (correlation, channel, row);

        Bool ok = abs (imaginary - z.imag()) < averagingFactor_p * 0.01;
        ok = ok && abs (real - z.real()) <  averagingFactor_p * 0.01;

        if (! ok){
            String expected = String::format ("(%f,%f)", z0.real(), z0.imag());
            String actual = String::format ("(%f,%f)", z.real(), z.imag());

            throwError (expected, actual, vb, rowId, row, channel, correlation, subchunkIndex, name_p,
                        String::format ("averagingFactor=%d", averagingFactor_p));
        }
    }

private:

    Accessor accessor_p;
    Int averagingFactor_p;
    EndBoundaryConditions endBoundaryConditions_p;
    Float factor_p;
    String name_p;

};



//def f (n): return [(sum(x[i:i+n:2])/(n/2),i+n/2-1) for i in range (0,19,n)]

class FlagRowRampChecker : public CubeChecker {

public:

    FlagRowRampChecker (Int averagingFactor, const String & name)
    : averagingFactor_p (averagingFactor), name_p (name)
    {
        TestErrorIf (averagingFactor_p > 1 && averagingFactor_p % 2 != 0,
                     "Averaging factor must be 1 or even.")
    }

    void checkCube (VisBuffer2 * vb, const VisibilityIterator2 * /*vi*/, Int rowId,
                    Int row, Int channel, Int correlation, Int subchunkIndex)
    {
        // Assumes that every other time sample is flagged (i.e., t=1,3,5,... are flagged)

        Float imaginary = (((vb->antenna1 ()(row) * 10 + vb->antenna2() (row)) * 10 +
                          vb->spectralWindows()(row)) * 10 + correlation) * 10 + channel;

        Float real;

        if (averagingFactor_p == 1){
            real = subchunkIndex * averagingFactor_p;
        }
        else{
            real = (subchunkIndex * averagingFactor_p) + (averagingFactor_p / 2.0 - 1);
        }

        Complex z0 (real, imaginary);
        //z0 *= factor_p;
        Complex z = vb->visCube() (correlation, channel, row);

        Bool ok = imaginary == z.imag();
        ok = ok && abs (real - z.real()) <  averagingFactor_p * 0.01;

        if (! ok){
            String expected = String::format ("(%f,%f)", z0.real(), z0.imag());
            String actual = String::format ("(%f,%f)", z.real(), z.imag());

            throwError (expected, actual, vb, rowId, row, channel, correlation, subchunkIndex, name_p,
                        String::format ("averagingFactor=%d", averagingFactor_p));
        }
    }

private:

    Int averagingFactor_p;
    String name_p;

};


////////////////////////////////////////////////////////////
//
// RowCheckers
//

class RowChecker {

public:

    virtual ~RowChecker () {}

    virtual void checkRow (VisBuffer2 * vb, const VisibilityIterator2 * vi, Int rowId,
                            Int row, Int subchunkIndex) = 0;

protected:

    void throwError (const String & expected, const String & actual,
                     const VisBuffer2 * vb, Int rowId, Int row,
                     Int subchunkIndex, const String & objectName,
                     const String & extra = String ()) const
    {
        String message = String::format ("For %s: expected %s got %s at subchunk=%d, "
                                        "msRow=%d, vbRow=%d, t=%f\n%s",
                                        objectName.c_str(),
                                        expected.c_str(),
                                        actual.c_str(),
                                        subchunkIndex,
                                        rowId,
                                        row,
                                        vb->time()(row),
                                        extra.c_str());
        ThrowTestError (message);
    }
};

class UvwCheckerDefault : public RowChecker {

public:

    UvwCheckerDefault (Int averagingFactor, Int nBaselines, Int lastRow, Int nRowsInPartialAverage)
    : averagingFactor_p (averagingFactor),
      lastRow_p (lastRow),
      nBaselines_p (nBaselines),
      nRowsInPartialAverage_p (nRowsInPartialAverage)
    {}

    virtual
    void
    checkRow (VisBuffer2 * vb, const VisibilityIterator2 * /*vi*/, Int rowId,
              Int row, Int subchunkIndex)
    {
        Vector<Double> actual = vb->uvw().column (row);
        Vector<Double> expected (3, 0);

        Int originalFirstRow = nBaselines_p * subchunkIndex * averagingFactor_p + row;
        Double base = (originalFirstRow + (averagingFactor_p - 1) * nBaselines_p / 2.0);

        if (rowId >= lastRow_p){
            base = (originalFirstRow + (nRowsInPartialAverage_p - 1) * nBaselines_p / 2.0);
        }

        expected (0) = base * 10;
        expected (1) = base * 10 + 1;
        expected (2) = base * 10 + 2;

        Bool equal = True;

        for (Int i = 0; i < 3; i++){
            equal = equal && abs (actual (i) - expected(i)) < 1e5;
        }

        if (! equal){
            String expectedStr = String::format ("(%f,%f,%f)", expected (0), expected (1), expected (2));
            String actualStr = String::format ("(%f,%f,%f)", actual (0), actual (1), actual (2));

            throwError (expectedStr, actualStr, vb, rowId, row, subchunkIndex, "UVW");
        }
    }

protected:

    Int averagingFactor_p;
    Int lastRow_p;
    Int nBaselines_p;
    Int nRowsInPartialAverage_p;

};

template <typename T, typename Accessor>
class ConstantRowChecker : public RowChecker {

public:

    ConstantRowChecker (const T & expected, Accessor accessor, const String & name)
    : accessor_p (accessor), expected_p (expected), name_p (name) {}

    void
    checkRow (VisBuffer2 * vb, const VisibilityIterator2 * /*vi*/, Int rowId,
              Int row, Int subchunkIndex){

        Bool actual = (vb ->* accessor_p) () (row);


        if (actual != expected_p){

            throwError (String::format ("%d", expected_p),
                        String::format ("%d", actual),
                        vb, rowId, row, subchunkIndex, name_p);

        }
    }

private:

    Accessor accessor_p;
    T expected_p;
    String name_p;
};

template <typename T, typename Accessor>
ConstantRowChecker<T,Accessor> * generateConstantRowChecker (const T & expected, Accessor accessor,
                                                             const String & name)
{
    return new ConstantRowChecker<T,Accessor> (expected, accessor, name);
}



class SigmaWeightChecker : public RowChecker {

public:

    typedef const Matrix<Float> & (VisBuffer2::* Accessor) () const;

    SigmaWeightChecker (Int averagingFactor, Accessor accessor, const String & name,
                        Int lastRow, Int nRowsInPartialAverage, double channelFraction,
                        Bool isSigma = False)
    : accessor_p (accessor), averagingFactor_p (averagingFactor),
      channelFraction_p (channelFraction),
      endBoundaryConditions_p (EndBoundaryConditions (1, make_pair (lastRow, nRowsInPartialAverage))),
      isSigma_p (isSigma),
      name_p (name)
    {}

    SigmaWeightChecker (Int averagingFactor, Accessor accessor, const String & name,
                        const EndBoundaryConditions & endBoundaryConditions,
                        double channelFraction, Bool isSigma = False)
    : accessor_p (accessor), averagingFactor_p (averagingFactor),
      channelFraction_p (channelFraction),
      endBoundaryConditions_p (endBoundaryConditions),
      isSigma_p (isSigma),
      name_p (name)
    {}


    virtual void checkRow (VisBuffer2 * vb, const VisibilityIterator2 * /*vi*/, Int rowId,
                           Int row, Int subchunkIndex){

        // The input value to the average is 1.0 so the result should simply be the sum
        // of number of samples in the average.  This will be averagingFactor_p except
        // if the last average only has a partial number of samples.

        for (Int c = 0; c < vb->nCorrelations(); c ++){

            Int ddId = vb->dataDescriptionIds()(0);
            Assert (ddId < (Int) endBoundaryConditions_p.size());
            Int lastRow = endBoundaryConditions_p [ddId].first;
            Int nRowsInPartialAverage = endBoundaryConditions_p [ddId].second;


            Float actual = (vb ->* accessor_p)()(c, row);
            Float expected = (rowId < lastRow) ? averagingFactor_p * 1.0f * channelFraction_p
                                               : nRowsInPartialAverage * 1.0f * channelFraction_p;
            if (isSigma_p){
                expected = 1.0 / std::sqrt (expected);
            }
            Bool ok = deltaEq (actual, expected, .001);

            if (! ok){

                throwError (String::format ("%f", expected),
                            String::format ("%f", actual),
                            vb, rowId, row, subchunkIndex, name_p,
                            String::format ("correlation=%d", c));

            }
        }
    }

private:

    Accessor accessor_p;
    Int averagingFactor_p;
    Double channelFraction_p;
    EndBoundaryConditions endBoundaryConditions_p;
    Bool isSigma_p;
    String name_p;
};



class SimpleExposureChecker : public RowChecker {
public:

    SimpleExposureChecker (Int averagingFactor, Double interval, Int lastRow, Int nRowsInPartialAverage)
    : averagingFactor_p (averagingFactor),
      endBoundaryConditions_p (EndBoundaryConditions (1, make_pair (lastRow, nRowsInPartialAverage))),
      interval_p (interval)
    {}

    SimpleExposureChecker (Int averagingFactor, Double interval,
                           const EndBoundaryConditions & endBoundaryConditions)
    : averagingFactor_p (averagingFactor),
      endBoundaryConditions_p (endBoundaryConditions),
      interval_p (interval)
    {}

    virtual void checkRow (VisBuffer2 * vb, const VisibilityIterator2 * /*vi*/, Int rowId,
                           Int row, Int subchunkIndex){

        Double expected = -1;

        Int ddId = vb->dataDescriptionIds()(0);
        Assert (ddId < (Int) endBoundaryConditions_p.size());
        Int lastRow = endBoundaryConditions_p [ddId].first;
        Int nRowsInPartialAverage = endBoundaryConditions_p [ddId].second;

        if (rowId >= lastRow){
            expected = interval_p * nRowsInPartialAverage;
        }
        else {
            expected = interval_p * averagingFactor_p;
        }

        Bool ok = deltaEq (vb->exposure ()(row), expected, .001);

        if (! ok){

            throwError (String::format ("%f", expected),
                        String::format ("%f", vb->exposure()(row)),
                        vb, rowId, row, subchunkIndex, "Exposure");
        }
    }

private:

    Int averagingFactor_p;
    EndBoundaryConditions endBoundaryConditions_p;
    Double interval_p;
};

class SimpleIntervalChecker : public RowChecker {
public:

    SimpleIntervalChecker  (Int averagingFactor, Double interval, Int lastRow, Int nRowsInPartialAverage)
    : averagingFactor_p (averagingFactor),
      endBoundaryConditions_p (EndBoundaryConditions (1, make_pair (lastRow, nRowsInPartialAverage))),
      interval_p (interval)
    {}

    SimpleIntervalChecker  (Int averagingFactor, Double interval,
                           const EndBoundaryConditions & endBoundaryConditions)
    : averagingFactor_p (averagingFactor),
      endBoundaryConditions_p (endBoundaryConditions),
      interval_p (interval)
    {}

    virtual void checkRow (VisBuffer2 * vb, const VisibilityIterator2 * /*vi*/, Int rowId,
                           Int row, Int subchunkIndex){

        Double expected = -1;

        Int ddId = vb->dataDescriptionIds()(0);
        Assert (ddId < (Int) endBoundaryConditions_p.size());
        Int lastRow = endBoundaryConditions_p [ddId].first;
        Int nRowsInPartialAverage = endBoundaryConditions_p [ddId].second;

        if (rowId >= lastRow){
            expected = interval_p * nRowsInPartialAverage;
        }
        else {
            expected = interval_p * averagingFactor_p;
        }

        Bool ok = deltaEq (vb->timeInterval ()(row), expected, .001);

        if (! ok){

            throwError (String::format ("%f", expected),
                        String::format ("%f", vb->timeInterval()(row)),
                        vb, rowId, row, subchunkIndex, "Interval");
        }
    }

private:

    Int averagingFactor_p;
    EndBoundaryConditions endBoundaryConditions_p;
    Double interval_p;
};


class SimpleTimeCentroidChecker : public RowChecker {
public:

    SimpleTimeCentroidChecker() {}

    SimpleTimeCentroidChecker (Int averagingFactor, Double interval, Int lastRow, Int nRowsInPartialAverage)
    : averagingFactor_p (averagingFactor),
      endBoundaryConditions_p (EndBoundaryConditions (1, make_pair (lastRow, nRowsInPartialAverage))),
      interval_p (interval)
    {}

    SimpleTimeCentroidChecker (Int averagingFactor, Double interval,
                               const EndBoundaryConditions & endBoundaryConditions)
    : averagingFactor_p (averagingFactor),
      endBoundaryConditions_p (endBoundaryConditions),
      interval_p (interval)
    {}

    virtual void checkRow (VisBuffer2 * vb, const VisibilityIterator2 * /*vi*/, Int rowId,
                           Int row, Int subchunkIndex){

        Double expected = vb->time () (row);
        Double actual = vb->timeCentroid ()(row);

//        Int ddId = vb->dataDescriptionIds()(0);
//        Assert (ddId < (Int) endBoundaryConditions_p.size());
//        Int lastRow = endBoundaryConditions_p [ddId].first;
//        Int nRowsInPartialAverage = endBoundaryConditions_p [ddId].second;

//        if (rowId >= lastRow && nRowsInPartialAverage > 0){
//
//            // Time is the time of the first row making up the average plus 1/2
//            // of the averaging interval (same as interval_p * averagingFactor_p).
//            // For a partial interval the time centroid will be only 1/2 of the
//            // the samples used (nRowsInPartialAverage_p) times the intersample interval.
//
//            Double delta = (averagingFactor_p - nRowsInPartialAverage) * interval_p * 0.5;
//
//            expected = vb->time() (row) - delta;
//        }

        Bool ok = deltaEq (actual, expected, .001);

        if (! ok){

            throwError (String::format ("%f", expected),
                        String::format ("%f", actual),
                        vb, rowId, row, subchunkIndex, "TimeCentroid");

        }
    }

private:

    Int averagingFactor_p;
    EndBoundaryConditions endBoundaryConditions_p;
    Double interval_p;
};

class AlternatingFlagRowChecker : public RowChecker {

public:

    AlternatingFlagRowChecker (Int averagingFactor, Double interval)
    : averagingFactor_p (averagingFactor), interval_p (interval) {}

    virtual void checkRow (VisBuffer2 * vb, const VisibilityIterator2 * /*vi*/, Int rowId,
                           Int row, Int subchunkIndex){

        if (averagingFactor_p == 1){

            // Check the flags which should alternate

            Bool expected = ((Int) (vb->time()(row) + 0.1) % 2 == 1);
            Bool actual = vb->flagRow() (row);

            if (expected != actual){

                throwError (String::format ("%d", expected),
                            String::format ("%d", actual),
                            vb, rowId, row, subchunkIndex, "flagRow");
            }

        }
        else{

            // The flags should always be clear

            if (vb->flagRow()(row)){

                throwError (String::format ("0"),
                            String::format ("%d", vb->flagRow()(row)),
                            vb, rowId, row, subchunkIndex, "flagRow");

            }

            // Now check the time

            //Double expected = subchunkIndex * averagingFactor_p + (averagingFactor_p - interval_p) * 0.5 ;
            //Double expected = (subchunkIndex * averagingFactor_p + (averagingFactor_p - 1) * interval_p) * 0.5 ;

            Double expected = subchunkIndex * averagingFactor_p + (averagingFactor_p - 2) * interval_p * 0.5;
                // Does not work with partial subchunks!!!

            Bool ok = deltaEq (vb->time()(row), expected, .001);

            if (! ok){

                throwError (String::format ("%f", expected),
                            String::format ("%f", vb->time()(row)),
                            vb, rowId, row, subchunkIndex, "time");
            }

            // Check the time centroid
            //
            // For intervals with an odd number of samples the time centroid will
            // be the center of the interval.  For even-sized intervals the centroid
            // will shift a 1/2 interval ahead of the center because the first sample
            // is always flagged.

            if (averagingFactor_p % 2 == 1){

                expected -= 0.5 * interval_p;
            }

            ok = deltaEq (vb->time()(row), expected, .001);

            if (! ok){

                throwError (String::format ("%f", expected),
                            String::format ("%f", vb->time()(row)),
                            vb, rowId, row, subchunkIndex, "timeCentroid");
            }


        }

    }

private:

    Int averagingFactor_p;
    Double interval_p;
};

class SimpleTimeChecker : public RowChecker {

public:

    SimpleTimeChecker  (Int averagingFactor, Double interval, Double averagingInterval,
                        Int lastRow, Int nRowsInPartialAverage)
    : averagingFactor_p (averagingFactor),
      endBoundaryConditions_p (EndBoundaryConditions (1, make_pair (lastRow, nRowsInPartialAverage))),
      interval_p (interval),
      middleOfAveragingInterval_p ((averagingInterval - interval) * 0.5)
      {}

    SimpleTimeChecker  (Int averagingFactor, Double interval, Double averagingInterval,
                        const EndBoundaryConditions & endBoundaryConditions)
    : averagingFactor_p (averagingFactor),
      endBoundaryConditions_p (endBoundaryConditions),
      interval_p (interval),
      middleOfAveragingInterval_p ((averagingInterval - interval) * 0.5)
    {}

//    SimpleTimeChecker (Int averagingFactor, Double interval, Double averagingInterval)
//    : averagingFactor_p (averagingFactor),
//      interval_p (interval),
//      middleOfAveragingInterval_p ((averagingInterval - interval) * 0.5)
//    {}

    virtual void checkRow (VisBuffer2 * vb, const VisibilityIterator2 * /*vi*/, Int rowId,
                           Int row, Int subchunkIndex)
    {
        Double expected = -1;

        Int ddId = vb->dataDescriptionIds()(0);
        Assert (ddId < (Int) endBoundaryConditions_p.size());
        Int lastRow = endBoundaryConditions_p [ddId].first;
        Int nRowsInPartialAverage = endBoundaryConditions_p [ddId].second;

        if (rowId >= lastRow){
            expected = averagingFactor_p * subchunkIndex * interval_p
                        + (nRowsInPartialAverage - 1) * 0.5;
        }
        else {
            expected = averagingFactor_p * subchunkIndex * interval_p
                        + middleOfAveragingInterval_p;
        }

        Bool ok = deltaEq (vb->time ()(row), expected, .001);

        if (! ok){

            throwError (String::format ("%f", expected),
                        String::format ("%f", vb->time()(row)),
                        vb, rowId, row, subchunkIndex, "Time");
        }
    }

private:

    Int averagingFactor_p;
    EndBoundaryConditions endBoundaryConditions_p;
    Double interval_p;
    Double middleOfAveragingInterval_p;

};



template<typename T>
class SimpleRowChecker : public RowChecker {

public:

    typedef const Vector<T> & (VisBuffer2::* Accessor) () const;

    SimpleRowChecker (const String & name, Accessor accessor, T expectedValue)
    : accessor_p (accessor),
      expectedValue_p (expectedValue),
      name_p (name)
    {}

    virtual void checkRow (VisBuffer2 * vb, const VisibilityIterator2 * /*vi*/, Int rowId,
                           Int row, Int subchunkIndex){

        T actualValue = (vb ->* accessor_p) () (row);
        Bool ok = actualValue == expectedValue_p;

        if (! ok){

            throwError (formatValue (expectedValue_p),
                        formatValue (actualValue),
                        vb, rowId, row, subchunkIndex, name_p);

        }
    }

protected:

    String
    formatValue (Int arg)
    {
        return String::format ("%d", arg);
    }

    String
    formatValue (Double arg)
    {
        return String::format ("%f", arg);
    }

private:

    Accessor accessor_p;
    T expectedValue_p;
    String name_p;

};

template <typename T>
SimpleRowChecker<T> *
generateSimpleRowChecker (const String & name,
                          typename SimpleRowChecker<T>::Accessor accessor,
                          T expectedValue)
{
    return new SimpleRowChecker<T> (name, accessor, expectedValue);
}



class Tester {

public:

    friend class TestSuite;

    Tester (Int nRowsExpected = -1);
    Bool doTests (Int nArgs, char * args []);

protected:

    pair<MeasurementSet *,Int> createMs (const String &);
    template <typename T>
    void doTest (const Environment & environment, const Arguments & arguments);

    Arguments parseArgs (int nArgs, char * args []) const;
    void sweepMs (const String & msName, Double interval,
                  Double chunkInterval, Int averagingFactor);
    void checkMs (VisibilityIterator2 * vi);

    void checkRowData (VisBuffer2 * vb, VisibilityIterator2 * vi, Int subchunk,
                       Int firstRow, Int averagingFactor);
    void checkVisCube (Int rowId, Int spectralWindow, Int row, Int channel, Int correlation,
                       const Cube<Complex> & cube, const String & tag,
                       Int antenna1, Int antenna2, Int subchunkIndex, Int averagingFactor,
                       Double time);
    void checkVisCubes (VisBuffer2 * vb, VisibilityIterator2 * vi, Int subchunk,
                        Int firstRow, Int averagingFactor);

    void
    checkCubes (VisBuffer2 * vb, VisibilityIterator2 * vi, Int subchunk,
                Int firstRow);

    void
    checkRowCount (Int actualNumberOfRows);

    void
    checkRows (VisBuffer2 * vb, VisibilityIterator2 * vi, Int subchunk,
               Int firstRow);

    void clearCheckers(Bool deleteIt = True);

    void doSimpleTest (MeasurementSet * ms, Int interval, Int chunkInterval, Int averagingFactor);
    void doSimpleTests (MeasurementSet * ms);

    void addCubeChecker (CubeChecker * cubeChecker);
    void addRowChecker (RowChecker * rowChecker);
    void setExpectedRowCount (Int nRowsExpected);

    Bool isTestSelected (const Arguments & arguments, const String & testName) const;


private:

    typedef vector<CubeChecker *> CubeCheckers;
    typedef vector<RowChecker *> RowCheckers;

    CubeCheckers cubeCheckers_p;
    vector<pair <String, String> > testResults_p;
    RowCheckers rowCheckers_p;
    MsFactory * msf_p;
    Int nRowsExpected_p;
    Int nTestsAttempted_p;
    Int nTestsPassed_p;
};

////////////////////////////////////////////////////////////
//
// Generators
//

class GenerateAlternatingFlagRows: public Generator<Bool>{

public:

    virtual Bool operator() (const FillState & fillState,
                             Int /*channel*/, Int /*correlation*/) const {

        Int timeIndex = fillState.rowNumber_p / (fillState.nAntennas_p * (fillState.nAntennas_p - 1) / 2);

        Bool theFlag = timeIndex % 2 == 1;

        return theFlag;
    }

};

class GenerateCheckerboardFlagCube  : public Generator<Bool>{

public:

    virtual Bool operator() (const FillState & /* fillState */,
                             Int channel, Int correlation) const {


        Bool theFlag = ((channel & 0x1) ^ (correlation & 0x1)) != 0;

        return theFlag;
    }
};



class GenerateRamp : public Generator<Complex> {

public:

    GenerateRamp (Float factor = 1) : counter_p (4096, -1), factor_p (factor), previousTime_p (4096, -1) {}

    Complex
    operator() (const FillState & fillState, Int channel, Int correlation) const
    {
        // Generate a ramp that increments every new set of baselines.

        if (! deltaEq (fillState.time_p, previousTime_p [fillState.spectralWindow_p], 1E-10)){

            counter_p [fillState.spectralWindow_p] += 1;
            previousTime_p [fillState.spectralWindow_p] = fillState.time_p;
        }

        Float imaginary = (((fillState.antenna1_p * 10 + fillState.antenna2_p) * 10 +
                            fillState.spectralWindow_p) * 10 + correlation) * 10 + channel;

        Float real = counter_p [fillState.spectralWindow_p] * factor_p;

        return Complex (real, imaginary);
    }

private:

    mutable Vector <Int> counter_p;
    Float factor_p;
    mutable Vector <Double> previousTime_p;
};

class UvwMotionGenerator : public Generator<Vector <Double> > {

public:

    typedef map <pair <Int, Int>, pair<Double, Double> > Velocity;

    UvwMotionGenerator (const Velocity & velocity)
    : velocity_p (velocity)
    {}

    UvwMotionGenerator (Double vU, Double vV, Int nAntennas)
    {
        for (Int a1 = 0; a1 < nAntennas; a1 ++){
            for (Int a2 = 0; a2 < nAntennas; a2 ++){
                velocity_p [make_pair (a1, a2)] = make_pair (vU, vV);
            }
        }
    }

    Vector<Double>
    operator() (const FillState & fillState, Int /*channel*/, Int /*correlation*/) const
    {
        // Generate a ramp that increments every new set of baselines.

        Vector<Double> uvw (3, 0);

        Double t = fillState.time_p;

        pair<Int,Int> baseline (fillState.antenna1_p, fillState.antenna2_p);

        pair<Double, Double> v = velocity_p.at (baseline);

        uvw (0) = t * v.first;
        uvw (1) = t * v.second;

        return uvw;
    }

private:

    Velocity velocity_p;
};

////////////////////////////////////////////////////////////
//
// TestSuites
//

class TestSuite {
public:

    TestSuite (const Environment & environment, const Arguments & arguments,
               Tester & tester)
    : arguments_p (arguments), environment_p (environment), tester_p (tester)
    {}

    virtual ~TestSuite () {}

    void execute ();

protected:

    void addCubeChecker (CubeChecker * checker) { getTester().addCubeChecker (checker);}
    void addRowChecker (RowChecker * checker) { getTester().addRowChecker (checker);}
    void checkMs (VisibilityIterator2 * vi) { getTester().checkMs (vi); }
    void clearCheckers () { getTester().clearCheckers(); }
    void setExpectedRowCount (Int n) { getTester().setExpectedRowCount(n);}

    Arguments & getArguments () { return arguments_p;}

    Environment & getEnvironment () { return environment_p;}

    String
    getParameter (const String & name, const String & defaultValue = "") const {

        String result;

        Arguments::const_iterator i = arguments_p.find (name);
        if (i != arguments_p.end()){
            result = i->second;
        }
        else{
            Environment::const_iterator i = environment_p.find (name);
            if (i != environment_p.end()){
                result = i->second;
            }
            else{
                result = defaultValue;
            }
        }

        return result;
    }

    Tester & getTester () { return tester_p;}

private:

    Arguments arguments_p;
    Environment environment_p;
    Tester & tester_p;
};


class SimpleTests : public TestSuite {

public:

    SimpleTests (const Environment & environment,
                 const Arguments & arguments,
                 Tester & tester)

    : TestSuite (environment, arguments, tester),
      interval_p (1)
    {}

    void execute ()
    {

        // The simple tests check averaging of different intervals
        // on a single spectral window.

        printf ("+++ Starting SimpleTests ...\n");

        String msName (getParameter ("msName", "AveragingTvi2.ms"));
        auto_ptr<MeasurementSet> ms;

        MeasurementSet * msTmp;
        boost::tie (msTmp, nRows_p) = createMs (msName);
        ms.reset (msTmp);

        doSimpleTest (ms.get(), interval_p, 10, 1); // interval, chunkInterval, factor

        doSimpleTest (ms.get(), interval_p, 10, 2);

        doSimpleTest (ms.get(), interval_p, 12, 3);

        doSimpleTest (ms.get(), interval_p, 12, 4);

        doSimpleTest (ms.get(), interval_p, 15, 5);

        doSimpleTest (ms.get(), interval_p, 12, 6);

        doSimpleTest (ms.get(), interval_p, 12, 7);

        doSimpleTest (ms.get(), interval_p, 16, 8);

        doSimpleTest (ms.get(), interval_p, 10, 10);

        doSimpleTest (ms.get(), interval_p, 20, 10);

        doSimpleTest (ms.get(), interval_p, 12, 12);

        doSimpleTest (ms.get(), interval_p, 24, 12);

        printf ("--- ... completed SimpleTests\n");
    }

    static String getName () { return "SimpleTests";}

protected:

    pair<MeasurementSet *,Int>
    createMs (const String & msName)
    {
        system (String::format ("rm -r %s", msName.c_str()).c_str());

        MsFactory * msFactory = new MsFactory (msName);

        nAntennas_p = 4;
        nBaselines_p = ((nAntennas_p - 1) * nAntennas_p) / 2;
        msFactory->setTimeInfo (0, 120, interval_p);
        msFactory->addSpectralWindows(1); // only one spw for now
        msFactory->addAntennas(nAntennas_p);
        msFactory->addFeeds (10); // needs antennas and spws to be setup first
        msFactory->addWeightSpectrum (False);

        for (Int i = 0; i < 10; i++){
            msFactory->addField (String::format ("field%d", i), MDirection());
        }

        // For many of the columns, simply put in a distinct constant to see if
        // the correct data is being processed as well as averaged properly.

        msFactory->setDataGenerator (MSMainEnums::SCAN_NUMBER,
                                     new GenerateConstant<Int> (10));
        msFactory->setDataGenerator (MSMainEnums::OBSERVATION_ID,
                                     new GenerateConstant<Int> (11));
        msFactory->setDataGenerator (MSMainEnums::ARRAY_ID,
                                     new GenerateConstant<Int> (12));
        msFactory->setDataGenerator (MSMainEnums::FEED1,
                                     new GenerateConstant<Int> (9));
        msFactory->setDataGenerator (MSMainEnums::FEED2,
                                     new GenerateConstant<Int> (8));
        msFactory->setDataGenerator (MSMainEnums::FIELD_ID,
                                     new GenerateConstant<Int> (7));
        msFactory->setDataGenerator (MSMainEnums::STATE_ID,
                                     new GenerateConstant<Int> (17));
        msFactory->setDataGenerator (MSMainEnums::PROCESSOR_ID,
                                     new GenerateConstant<Int> (18));
        msFactory->setDataGenerator (MSMainEnums::EXPOSURE,
                                     new GenerateConstant<Double> (interval_p));
        msFactory->setDataGenerator (MSMainEnums::INTERVAL,
                                     new GenerateConstant<Double> (interval_p));
        msFactory->setDataGenerator (MSMainEnums::SIGMA,
                                     new GenerateConstant<Float> (1.0f));
        msFactory->setDataGenerator (MSMainEnums::WEIGHT,
                                     new GenerateConstant<Float> (1.0f));

        // For the data cubes fill it with a ramp.  The real part of the ramp will
        // be multiplied by the factor supplied in the constructor to check that
        // there's no "crosstalk" between the columns.

        msFactory->setDataGenerator(MSMainEnums::DATA, new GenerateRamp());

        msFactory->setDataGenerator(MSMainEnums::CORRECTED_DATA, new GenerateRamp(2));

        msFactory->setDataGenerator(MSMainEnums::MODEL_DATA, new GenerateRamp(3));

        // Set all of the data to be unflagged.

        msFactory->setDataGenerator(MSMainEnums::FLAG, new GenerateConstant<Bool> (False));
        msFactory->setDataGenerator(MSMainEnums::FLAG_ROW, new GenerateConstant<Bool> (False));

        // Set the time centroid to be the middle of the sample interval.

        msFactory->setDataGenerator(MSMainEnums::TIME_CENTROID, new GenerateTimeCentroid ());

        pair<MeasurementSet *, Int> p = msFactory->createMs ();

        return make_pair (p.first, p.second);
    }


    void
    doSimpleTest (MeasurementSet * ms, Double interval, Int chunkInterval, Int averagingFactor)
    {
        printf ("\nStarting averaging of %d samples ...\n", averagingFactor);

        clearCheckers();

        Int lastRow = nRows_p / averagingFactor; // ID of first row of last averaged VB
        Int nRowsInPartialAverage = (nRows_p % averagingFactor) / nBaselines_p;
            // remaining

        addCubeChecker (new ComplexCubeRampChecker (averagingFactor, "Observed", & VisBuffer2::visCube,
                                                    lastRow, nRowsInPartialAverage));
        addCubeChecker (new ComplexCubeRampChecker (averagingFactor, "Corrected",
                                                    & VisBuffer2::visCubeCorrected,
                                                    lastRow, nRowsInPartialAverage, 2));
        addCubeChecker (new ComplexCubeRampChecker (averagingFactor, "Model",
                                                    & VisBuffer2::visCubeModel,
                                                    lastRow, nRowsInPartialAverage, 3));

        addRowChecker (generateSimpleRowChecker ("scan", & VisBuffer2::scan, 10));
        addRowChecker (generateSimpleRowChecker ("observationId", & VisBuffer2::observationId, 11));
        addRowChecker (generateSimpleRowChecker ("arrayId", & VisBuffer2::arrayId, 12));
        addRowChecker (generateSimpleRowChecker ("feed1", & VisBuffer2::feed1, 9));
        addRowChecker (generateSimpleRowChecker ("feed2", & VisBuffer2::feed2, 8));
        addRowChecker (generateSimpleRowChecker ("fieldId", & VisBuffer2::fieldId, 7));
        addRowChecker (generateSimpleRowChecker ("stateId", & VisBuffer2::stateId, 17));
        addRowChecker (generateSimpleRowChecker ("processorId", & VisBuffer2::processorId, 18));
        addRowChecker (new UvwCheckerDefault (averagingFactor, nBaselines_p,
                                              lastRow, nRowsInPartialAverage));

        Double expectedDuration = interval * averagingFactor;
//        addRowChecker (generateSimpleRowChecker ("interval", & VisBuffer2::timeInterval,
//                                                 expectedDuration));
        addRowChecker (new SimpleIntervalChecker (averagingFactor, interval, lastRow, nRowsInPartialAverage));
        addRowChecker (new SimpleTimeChecker (averagingFactor, interval, expectedDuration,
                                              lastRow, nRowsInPartialAverage));

        addRowChecker (new SimpleExposureChecker (averagingFactor, interval, lastRow, nRowsInPartialAverage));

        addRowChecker (new SimpleTimeCentroidChecker (averagingFactor, interval, lastRow, nRowsInPartialAverage));

        Double channelFraction = 1.0; // no flagged channels
        addRowChecker (new SigmaWeightChecker (averagingFactor, & VisBuffer2::weight, "Weight",
                                               lastRow, nRowsInPartialAverage, channelFraction));
        addRowChecker (new SigmaWeightChecker (averagingFactor, & VisBuffer2::sigma, "Sigma",
                                               lastRow, nRowsInPartialAverage, channelFraction, True));

        AveragingParameters parameters (interval * averagingFactor,
                                        chunkInterval,
                                        SortColumns (),
                                        AveragingOptions (AveragingOptions::AverageObserved |
                                                          AveragingOptions::AverageModel |
                                                          AveragingOptions::AverageCorrected |
                                                          AveragingOptions::ObservedUseNoWeights |
                                                          AveragingOptions::ModelUseNoWeights |
                                                          AveragingOptions::CorrectedUseNoWeights));
        VisibilityIterator2 vi (AveragingVi2Factory (parameters, ms));
        vi.setWeightScaling (WeightScaling::generateUnityWeightScaling());

        checkMs (& vi);

        printf ("\n...completed averaging of %d samples ...\n", averagingFactor);
    }

private:

    Int nAntennas_p;
    Int nBaselines_p;
    Double interval_p; // Time for each sample
    Int nRows_p;

};

template<Int N_Windows>
class SimpleTestsNWindows : public TestSuite {

public:

    SimpleTestsNWindows (const Environment & environment,
                         const Arguments & arguments,
                         Tester & tester)

    : TestSuite (environment, arguments, tester),
      interval_p (1)
    {}

    void execute ()
    {
        // The simple tests check averaging of different intervals
        // on a single spectral window.

        printf ("+++ Starting %s ...\n", getName().c_str());

        String msName (getParameter ("msName", "AveragingTvi2.ms"));
        auto_ptr<MeasurementSet> ms;

        MeasurementSet * msTmp;
        boost::tie (msTmp, nRows_p) = createMs (msName);
        ms.reset (msTmp);

        doSimpleTest (ms.get(), interval_p, 10, 1); // interval, chunkInterval, factor

        doSimpleTest (ms.get(), interval_p, 10, 2);

        doSimpleTest (ms.get(), interval_p, 12, 3);

        doSimpleTest (ms.get(), interval_p, 12, 4);

        doSimpleTest (ms.get(), interval_p, 15, 5);

        doSimpleTest (ms.get(), interval_p, 12, 6);

        doSimpleTest (ms.get(), interval_p, 12, 7);

        doSimpleTest (ms.get(), interval_p, 16, 8);

        doSimpleTest (ms.get(), interval_p, 10, 10);

        doSimpleTest (ms.get(), interval_p, 20, 10);

        doSimpleTest (ms.get(), interval_p, 12, 12);

        doSimpleTest (ms.get(), interval_p, 24, 12);

        printf ("--- ... completed SimpleTests\n");
    }

    static String getName () { return String::format ("SimpleTestsNWindows<%d>", N_Windows);}

protected:

    pair<MeasurementSet *,Int>
    createMs (const String & msName)
    {
        system (String::format ("rm -r %s", msName.c_str()).c_str());

        MsFactory * msFactory = new MsFactory (msName);

        nAntennas_p = 4;
        nBaselines_p = ((nAntennas_p - 1) * nAntennas_p) / 2;
        msFactory->setTimeInfo (0, 120, interval_p);
        msFactory->addSpectralWindows(N_Windows); // only on spw for now
        msFactory->addAntennas(nAntennas_p);
        msFactory->addFeeds (10); // needs antennas and spws to be setup first
        msFactory->addWeightSpectrum (False);

        for (Int i = 0; i < 10; i++){
            msFactory->addField (String::format ("field%d", i), MDirection());
        }

        // For many of the columns, simply put in a distinct constant to see if
        // the correct data is being processed as well as averaged properly.

        msFactory->setDataGenerator (MSMainEnums::SCAN_NUMBER,
                                     new GenerateConstant<Int> (10));
        msFactory->setDataGenerator (MSMainEnums::OBSERVATION_ID,
                                     new GenerateConstant<Int> (11));
        msFactory->setDataGenerator (MSMainEnums::ARRAY_ID,
                                     new GenerateConstant<Int> (12));
        msFactory->setDataGenerator (MSMainEnums::FEED1,
                                     new GenerateConstant<Int> (9));
        msFactory->setDataGenerator (MSMainEnums::FEED2,
                                     new GenerateConstant<Int> (8));
        msFactory->setDataGenerator (MSMainEnums::FIELD_ID,
                                     new GenerateConstant<Int> (7));
        msFactory->setDataGenerator (MSMainEnums::STATE_ID,
                                     new GenerateConstant<Int> (17));
        msFactory->setDataGenerator (MSMainEnums::PROCESSOR_ID,
                                     new GenerateConstant<Int> (18));
        msFactory->setDataGenerator (MSMainEnums::EXPOSURE,
                                     new GenerateConstant<Double> (interval_p));
        msFactory->setDataGenerator (MSMainEnums::INTERVAL,
                                     new GenerateConstant<Double> (interval_p));
        msFactory->setDataGenerator (MSMainEnums::WEIGHT,
                                     new GenerateConstant<Float> (1.0f));
        msFactory->setDataGenerator (MSMainEnums::SIGMA,
                                     new GenerateConstant<Float> (1.0f));

        // For the data cubes fill it with a ramp.  The real part of the ramp will
        // be multiplied by the factor supplied in the constructor to check that
        // there's no "crosstalk" between the columns.

        msFactory->setDataGenerator(MSMainEnums::DATA, new GenerateRamp());

        msFactory->setDataGenerator(MSMainEnums::CORRECTED_DATA, new GenerateRamp(2));

        msFactory->setDataGenerator(MSMainEnums::MODEL_DATA, new GenerateRamp(3));

        // Set all of the data to be unflagged.

        msFactory->setDataGenerator(MSMainEnums::FLAG, new GenerateConstant<Bool> (False));
        msFactory->setDataGenerator(MSMainEnums::FLAG_ROW, new GenerateConstant<Bool> (False));

        // Set the time centroid to be the middle of the sample interval.

        msFactory->setDataGenerator(MSMainEnums::TIME_CENTROID, new GenerateTimeCentroid ());

        pair<MeasurementSet *, Int> p = msFactory->createMs ();

        return make_pair (p.first, p.second);
    }

    void
    doSimpleTest (MeasurementSet * ms, Double interval, Int chunkInterval, Int averagingFactor)
    {
        printf ("\nStarting averaging of %d samples ...\n", averagingFactor);

        EndBoundaryConditions endBoundaryConditions;
        Int nRowsPerWindow = nRows_p / N_Windows;

        for (Int i = 0; i < N_Windows; i++){
            Int lastRow = (nRowsPerWindow / averagingFactor) * N_Windows + i * nBaselines_p; // ID of first row of last averaged VB
            Int nRowsInPartialAverage = (nRowsPerWindow % averagingFactor) / nBaselines_p;

            if (nRowsInPartialAverage != 0){
                lastRow += i * nBaselines_p * N_Windows;
            }
            // remaining
            endBoundaryConditions.push_back (make_pair (lastRow, nRowsInPartialAverage));
        }

        clearCheckers();

        addCubeChecker (new ComplexCubeRampChecker (averagingFactor, "Observed", & VisBuffer2::visCube,
                                                    endBoundaryConditions));
        addCubeChecker (new ComplexCubeRampChecker (averagingFactor, "Corrected",
                                                    & VisBuffer2::visCubeCorrected,
                                                    endBoundaryConditions, 2));
        addCubeChecker (new ComplexCubeRampChecker (averagingFactor, "Model",
                                                    & VisBuffer2::visCubeModel,
                                                    endBoundaryConditions, 3));

        addRowChecker (generateSimpleRowChecker ("scan", & VisBuffer2::scan, 10));
        addRowChecker (generateSimpleRowChecker ("observationId", & VisBuffer2::observationId, 11));
        addRowChecker (generateSimpleRowChecker ("arrayId", & VisBuffer2::arrayId, 12));
        addRowChecker (generateSimpleRowChecker ("feed1", & VisBuffer2::feed1, 9));
        addRowChecker (generateSimpleRowChecker ("feed2", & VisBuffer2::feed2, 8));
        addRowChecker (generateSimpleRowChecker ("fieldId", & VisBuffer2::fieldId, 7));
        addRowChecker (generateSimpleRowChecker ("stateId", & VisBuffer2::stateId, 17));
        addRowChecker (generateSimpleRowChecker ("processorId", & VisBuffer2::processorId, 18));

        Double expectedDuration = interval * averagingFactor;

        addRowChecker (new SimpleIntervalChecker (averagingFactor, interval,
                                              endBoundaryConditions));
        addRowChecker (new SimpleTimeChecker (averagingFactor, interval, expectedDuration,
                                              endBoundaryConditions));

        addRowChecker (new SimpleExposureChecker (averagingFactor, interval, endBoundaryConditions));

        addRowChecker (new SimpleTimeCentroidChecker (averagingFactor, interval, endBoundaryConditions));

        addRowChecker (new SigmaWeightChecker (averagingFactor, & VisBuffer2::weight, "Weight",
                                               endBoundaryConditions, 1.0));
        addRowChecker (new SigmaWeightChecker (averagingFactor, & VisBuffer2::sigma, "Sigma",
                                               endBoundaryConditions, 1.0, True));

        Block<Int> columns (3, 0);
        columns [0] = MS::ARRAY_ID;
        columns [1] = MS::FIELD_ID;
        columns [2] = MS::TIME;

        AveragingParameters parameters (interval * averagingFactor,
                                        chunkInterval,
                                        SortColumns (columns, False),
                                        AveragingOptions (AveragingOptions::AverageObserved |
                                                          AveragingOptions::AverageModel |
                                                          AveragingOptions::AverageCorrected |
                                                          AveragingOptions::ObservedUseNoWeights |
                                                          AveragingOptions::ModelUseNoWeights |
                                                          AveragingOptions::CorrectedUseNoWeights));

        VisibilityIterator2 vi (AveragingVi2Factory (parameters, ms));
        vi.setWeightScaling (WeightScaling::generateUnityWeightScaling());

        checkMs (& vi);

        printf ("\n...completed averaging of %d samples ...\n", averagingFactor);
    }

private:

    Int nAntennas_p;
    Int nBaselines_p;
    Double interval_p; // Time for each sample
    Int nRows_p;

};


class RowFlaggingTests : public TestSuite {

public:

    RowFlaggingTests (const Environment & environment,
                      const Arguments & arguments,
                      Tester & tester)

    : TestSuite (environment, arguments, tester) {}

    void execute ()
    {
        printf ("+++ Starting RowFlaggingTests ...\n");

        String msName (getParameter ("msName", "AveragingTvi2.ms"));
        Int nRows;
        auto_ptr<MeasurementSet> ms;

        MeasurementSet * msTmp;
        const Double interval = 1.0;
        boost::tie (msTmp, nRows) = createMs (msName, interval);
        ms.reset (msTmp);

        doTest (ms.get(), interval, 10, 1); // interval, chunkInterval, factor

        doTest (ms.get(), interval, 10, 2);

        doTest (ms.get(), interval, 12, 4);

        printf ("--- ... completed RowFlaggingTests\n");
    }

    static String getName () { return "RowFlaggingTests";}

protected:

    pair<MeasurementSet *,Int>
    createMs (const String & msName, Double interval)
    {
        system (String::format ("rm -r %s", msName.c_str()).c_str());

        MsFactory * msFactory = new MsFactory (msName);

        msFactory->setTimeInfo (0, 120, 1);
        msFactory->addSpectralWindows(1); // only on spw for now

        auto_ptr<GenerateRamp> rampGenerator (new GenerateRamp());
        msFactory->setDataGenerator(MSMainEnums::DATA, rampGenerator.get());

        msFactory->setDataGenerator(MSMainEnums::WEIGHT_SPECTRUM,
                                    new GenerateConstant<Float> (1.0));

        auto_ptr<GenerateConstant<Bool> > generateFalse (new GenerateConstant<Bool> (False));
        msFactory->setDataGenerator(MSMainEnums::FLAG, generateFalse.get());

        auto_ptr<GenerateAlternatingFlagRows> generateAlternatingFlagRows (new GenerateAlternatingFlagRows());
        msFactory->setDataGenerator(MSMainEnums::FLAG_ROW, generateAlternatingFlagRows.get());

        msFactory->setDataGenerator (MSMainEnums::INTERVAL,
                                     new GenerateConstant<Double> (interval));

        pair<MeasurementSet *, Int> p = msFactory->createMs ();

        return make_pair (p.first, p.second);
    }

    void
    doTest (MeasurementSet * ms, Double interval, Int chunkInterval, Int averagingFactor)
    {
        printf ("\nStarting averaging of %d samples ...\n", averagingFactor);

        clearCheckers();

        // Checks to see if the row flags, time and centroid behave as expected.

        addRowChecker (new AlternatingFlagRowChecker (averagingFactor, interval));

        // Check to see if the averaged visCube is averaged properly given the
        // flags.

        addCubeChecker (new FlagRowRampChecker (averagingFactor, "FlagRowRampChecker"));

        AveragingParameters parameters (interval * averagingFactor,
                                        chunkInterval,
                                        SortColumns (),
                                        AveragingOptions (AveragingOptions::AverageObserved |
                                                          AveragingOptions::AverageModel |
                                                          AveragingOptions::AverageCorrected |
                                                          AveragingOptions::ModelUseWeights |
                                                          AveragingOptions::CorrectedUseWeights));
        VisibilityIterator2 vi (AveragingVi2Factory (parameters, ms));

        checkMs (& vi);

        printf ("\n...completed averaging of %d samples ...\n", averagingFactor);
    }

private:


};

class CubeFlaggingTests : public TestSuite {

public:

    CubeFlaggingTests (const Environment & environment,
                      const Arguments & arguments,
                      Tester & tester)

    : TestSuite (environment, arguments, tester) {}

    void execute ()
    {
        printf ("+++ Starting CubeFlaggingTests ...\n");

        String msName (getParameter ("msName", "AveragingTvi2.ms"));
        auto_ptr<MeasurementSet> ms;

        MeasurementSet * msTmp;
        boost::tie (msTmp, nRows_p) = createMs (msName);
        ms.reset (msTmp);

        doTest (ms.get(), 1, 10, 1); // interval, chunkInterval, factor

        doTest (ms.get(), 1, 10, 2);

        doTest (ms.get(), 1, 12, 4);

        printf ("--- ... completed CubeFlaggingTests\n");
    }

    static String getName () { return "CubeFlaggingTests";}

protected:

    pair<MeasurementSet *,Int>
    createMs (const String & msName)
    {
        system (String::format ("rm -r %s", msName.c_str()).c_str());

        MsFactory * msFactory = new MsFactory (msName);

        nAntennas_p = 4;
        nBaselines_p = ((nAntennas_p - 1) * nAntennas_p) / 2;
        msFactory->addAntennas (nAntennas_p);
        msFactory->setTimeInfo (0, 120, 1);
        msFactory->addSpectralWindows(1); // only on spw for now

        auto_ptr<GenerateRamp> rampGenerator (new GenerateRamp());
        msFactory->setDataGenerator(MSMainEnums::DATA, rampGenerator.get());

        msFactory->setDataGenerator(MSMainEnums::WEIGHT_SPECTRUM,
                                    new GenerateConstant<Float> (1.0));

        auto_ptr<GenerateConstant<Bool> > generateFalse (new GenerateConstant<Bool> (False));
        msFactory->setDataGenerator(MSMainEnums::FLAG_ROW, generateFalse.get());

        msFactory->setDataGenerator(MSMainEnums::TIME_CENTROID, new GenerateTime());

        auto_ptr<GenerateCheckerboardFlagCube>
        generateCheckerboardFlagCubes (new GenerateCheckerboardFlagCube());

        msFactory->setDataGenerator(MSMainEnums::FLAG, generateCheckerboardFlagCubes.get());

        pair<MeasurementSet *, Int> p = msFactory->createMs ();

        return make_pair (p.first, p.second);
    }

    void
    doTest (MeasurementSet * ms, Int interval, Int chunkInterval, Int averagingFactor)
    {
        printf ("\nStarting averaging of %d samples ...\n", averagingFactor);

        clearCheckers();

        // Averages should be the same as for the simple tests; check to see that the flag
        // cube didn't affect them.

        Int lastRow = nRows_p / averagingFactor; // ID of first row of last averaged VB
        Int nRowsInPartialAverage = (nRows_p % averagingFactor) / nBaselines_p;

        addCubeChecker (new ComplexCubeRampChecker (averagingFactor, "Observed", & VisBuffer2::visCube,
                                                    lastRow, nRowsInPartialAverage));

        addCubeChecker (new CheckerboardFlagCubeChecker ());


        addRowChecker (generateConstantRowChecker (False, & VisBuffer2::flagRow, String ("FlagRow")));

        addRowChecker (new SimpleTimeCentroidChecker());

        AveragingParameters parameters (interval * averagingFactor,
                                        chunkInterval,
                                        SortColumns (),
                                        AveragingOptions (AveragingOptions::AverageObserved |
                                                          AveragingOptions::AverageModel |
                                                          AveragingOptions::AverageCorrected |
                                                          AveragingOptions::ModelUseWeights |
                                                          AveragingOptions::CorrectedUseWeights));
        VisibilityIterator2 vi (AveragingVi2Factory (parameters, ms));

        checkMs (& vi);

        printf ("\n...completed averaging of %d samples ...\n", averagingFactor);
    }

private:

    Int nAntennas_p;
    Int nBaselines_p;
    Int nRows_p;

};


class WeightingTests : public TestSuite {

public:

    WeightingTests (const Environment & environment,
                    const Arguments & arguments,
                    Tester & tester)

    : TestSuite (environment, arguments, tester) {}

    void execute ()
    {
        printf ("+++ Starting WeightingTests ...\n");

        String msName (getParameter ("msName", "AveragingTvi2.ms"));
        Int nRows;
        auto_ptr<MeasurementSet> ms;

        MeasurementSet * msTmp;
        boost::tie (msTmp, nRows) = createMs (msName);
        ms.reset (msTmp);

        doSimpleTest (ms.get(), 1, 10, 1); // interval, chunkInterval, factor

        doSimpleTest (ms.get(), 1, 10, 2);

        doSimpleTest (ms.get(), 1, 12, 3);

        doSimpleTest (ms.get(), 1, 12, 3, True);

        printf ("--- ... completed WeightingTests\n");
    }

    static String getName () { return "WeightingTests";}


protected:

    pair<MeasurementSet *,Int>
    createMs (const String & msName)
    {
        system (String::format ("rm -r %s", msName.c_str()).c_str());

        MsFactory * msFactory = new MsFactory (msName);

        nAntennas_p = 4;
        nBaselines_p = ((nAntennas_p - 1) * nAntennas_p) / 2;
        msFactory->addAntennas (nAntennas_p);

        msFactory->setTimeInfo (0,120,1);
        msFactory->addSpectralWindows(1); // only on spw for now

        msFactory->setDataGenerator(MSMainEnums::WEIGHT_SPECTRUM,
                                    new GenerateConstant<Float> (0.1));

        msFactory->setDataGenerator(MSMainEnums::SIGMA,
                                    new GenerateConstant<Float> (1.0));

        msFactory->setDataGenerator(MSMainEnums::WEIGHT,
                                    new GenerateConstant<Float> (1.0));

        auto_ptr<GenerateRamp> rampGenerator (new GenerateRamp());
        msFactory->setDataGenerator(MSMainEnums::DATA, rampGenerator.get());

        auto_ptr<GenerateConstant<Bool> > generateFalse (new GenerateConstant<Bool> (False));
        msFactory->setDataGenerator(MSMainEnums::FLAG, generateFalse.get());
        msFactory->setDataGenerator(MSMainEnums::FLAG_ROW, generateFalse.get());

        pair<MeasurementSet *, Int> p = msFactory->createMs ();

        return make_pair (p.first, p.second);
    }

    static float doubler (float x) { return 2 * x;}


    void
    doSimpleTest (MeasurementSet * ms, Int interval, Int chunkInterval, Int averagingFactor,
                  Bool useDoubler = False)
    {
        printf ("\nStarting averaging of %d samples ...\n", averagingFactor);

        clearCheckers();

        Int lastRow = nRows_p / averagingFactor; // ID of first row of last averaged VB
        Int nRowsInPartialAverage = (nRows_p % averagingFactor) / nBaselines_p;

        addCubeChecker (new ComplexCubeRampChecker (averagingFactor, "Observed", & VisBuffer2::visCube,
                                                    lastRow, nRowsInPartialAverage));
        addCubeChecker (new WeightChecker (averagingFactor * 0.1));

        addRowChecker (new SigmaWeightChecker (averagingFactor, & VisBuffer2::weight, "Weight",
                                               lastRow, nRowsInPartialAverage, 1.0));
        addRowChecker (new SigmaWeightChecker (averagingFactor, & VisBuffer2::sigma, "Sigma",
                                               lastRow, nRowsInPartialAverage, 1.0, True));

        CountedPtr <WeightScaling> weightScaling = useDoubler ? generateWeightScaling (doubler)
                                                              : WeightScaling::generateIdentityWeightScaling();

        AveragingParameters parameters (interval * averagingFactor,
                                        chunkInterval,
                                        SortColumns (),
                                        AveragingOptions (AveragingOptions::AverageObserved |
                                                          AveragingOptions::AverageModel |
                                                          AveragingOptions::AverageCorrected |
                                                          AveragingOptions::ModelUseWeights |
                                                          AveragingOptions::CorrectedUseWeights));
        VisibilityIterator2 vi (AveragingVi2Factory (parameters, ms));
        vi.setWeightScaling (weightScaling);

        checkMs (& vi);

        printf ("\n...completed averaging of %d samples ...\n", averagingFactor);
    }

private:

    Int nAntennas_p;
    Int nBaselines_p;
    Int nRows_p;

};

class WeightSelectionTests : public TestSuite {

public:

    WeightSelectionTests (const Environment & environment,
                 const Arguments & arguments,
                 Tester & tester)

    : TestSuite (environment, arguments, tester),
      interval_p (1)
    {}

    void execute ()
    {

        // The simple tests check averaging of different intervals
        // on a single spectral window.

        printf ("+++ Starting WeightSelectionTests ...\n");

        String msName (getParameter ("msName", "AveragingTvi2.ms"));
        auto_ptr<MeasurementSet> ms;

        {
            printf ("... +++ Starting test using normal weight spectrum\n");

            MeasurementSet * msTmp;
            boost::tie (msTmp, nRows_p) = createMs (msName, 2.0f, 1000.0f);
            ms.reset (msTmp);

            doWeightedTest (ms.get(), interval_p, 10, 1); // interval, chunkInterval, factor

            doWeightedTest (ms.get(), interval_p, 10, 2);

            doWeightedTest (ms.get(), interval_p, 12, 3);

            printf ("... --- Completed test using normal weight spectrum\n");
        }

        {

            printf ("... +++ Starting test using corrected weight spectrum\n");

            MeasurementSet * msTmp;
            boost::tie (msTmp, nRows_p) = createMs (msName, 1000.0f, 2.0f);
            ms.reset (msTmp);

            doCorrectedWeightingTest (ms.get(), interval_p, 10, 1); // interval, chunkInterval, factor

            doCorrectedWeightingTest (ms.get(), interval_p, 10, 2);

            doCorrectedWeightingTest (ms.get(), interval_p, 12, 3);

            printf ("... --- Completed test using corrected weight spectrum\n");
        }

        printf ("--- ... completed WeightSelectionTests\n");
    }

    static String getName () { return "WeightSelectionTests";}

protected:

    pair<MeasurementSet *,Int>
    createMs (const String & msName, float weightSpectrum, float correctedWeightSpectrum)
    {
        system (String::format ("rm -r %s", msName.c_str()).c_str());

        MsFactory * msFactory = new MsFactory (msName);

        nAntennas_p = 4;
        nBaselines_p = ((nAntennas_p - 1) * nAntennas_p) / 2;
        msFactory->setTimeInfo (0, 120, interval_p);
        msFactory->addSpectralWindows(1); // only on spw for now
        msFactory->addAntennas(nAntennas_p);
        msFactory->addFeeds (10); // needs antennas and spws to be setup first

        for (Int i = 0; i < 10; i++){
            msFactory->addField (String::format ("field%d", i), MDirection());
        }

        msFactory->addWeightSpectrum (true);
        msFactory->addCorrectedWeightSpectrum(true);

        // For many of the columns, simply put in a distinct constant to see if
        // the correct data is being processed as well as averaged properly.

        msFactory->setDataGenerator (MSMainEnums::SCAN_NUMBER,
                                     new GenerateConstant<Int> (10));
        msFactory->setDataGenerator (MSMainEnums::OBSERVATION_ID,
                                     new GenerateConstant<Int> (11));
        msFactory->setDataGenerator (MSMainEnums::ARRAY_ID,
                                     new GenerateConstant<Int> (12));
        msFactory->setDataGenerator (MSMainEnums::FEED1,
                                     new GenerateConstant<Int> (9));
        msFactory->setDataGenerator (MSMainEnums::FEED2,
                                     new GenerateConstant<Int> (8));
        msFactory->setDataGenerator (MSMainEnums::FIELD_ID,
                                     new GenerateConstant<Int> (7));
        msFactory->setDataGenerator (MSMainEnums::STATE_ID,
                                     new GenerateConstant<Int> (17));
        msFactory->setDataGenerator (MSMainEnums::PROCESSOR_ID,
                                     new GenerateConstant<Int> (18));
        msFactory->setDataGenerator (MSMainEnums::EXPOSURE,
                                     new GenerateConstant<Double> (interval_p));
        msFactory->setDataGenerator (MSMainEnums::INTERVAL,
                                     new GenerateConstant<Double> (interval_p));
        msFactory->setDataGenerator (MSMainEnums::SIGMA,
                                     new GenerateConstant<Float> (1.0f));
        msFactory->setDataGenerator (MSMainEnums::WEIGHT,
                                     new GenerateConstant<Float> (1000.0f));

        // For the data cubes fill it with a ramp.  The real part of the ramp will
        // be multiplied by the factor supplied in the constructor to check that
        // there's no "crosstalk" between the columns.

        msFactory->setDataGenerator(MSMainEnums::DATA, new GenerateRamp());

        msFactory->setDataGenerator(MSMainEnums::CORRECTED_DATA, new GenerateRamp(2));

        msFactory->setDataGenerator(MSMainEnums::MODEL_DATA, new GenerateRamp(3));

        msFactory->setDataGenerator(MSMainEnums::WEIGHT_SPECTRUM,
                                    new GenerateConstant<Float> (weightSpectrum));
        msFactory->setDataGenerator(MSMainEnums::CORRECTED_WEIGHT_SPECTRUM,
                                    new GenerateConstant<Float> (correctedWeightSpectrum));

        // Set all of the data to be unflagged.

        msFactory->setDataGenerator(MSMainEnums::FLAG, new GenerateConstant<Bool> (False));
        msFactory->setDataGenerator(MSMainEnums::FLAG_ROW, new GenerateConstant<Bool> (False));

        // Set the time centroid to be the middle of the sample interval.

        msFactory->setDataGenerator(MSMainEnums::TIME_CENTROID, new GenerateTimeCentroid ());

        pair<MeasurementSet *, Int> p = msFactory->createMs ();

        return make_pair (p.first, p.second);
    }


    void
    doCorrectedWeightingTest (MeasurementSet * ms, Double interval, Int chunkInterval, Int averagingFactor)
    {
        printf ("\nStarting averaging of %d samples ...\n", averagingFactor);

        clearCheckers();

        Int lastRow = nRows_p / averagingFactor; // ID of first row of last averaged VB
        Int nRowsInPartialAverage = (nRows_p % averagingFactor) / nBaselines_p;
            // remaining

        addCubeChecker (new ComplexCubeRampChecker (averagingFactor, "Corrected",
                                                    & VisBuffer2::visCubeCorrected,
                                                    lastRow, nRowsInPartialAverage, 2));
        addCubeChecker (new ComplexCubeRampChecker (averagingFactor, "Model",
                                                    & VisBuffer2::visCubeModel,
                                                    lastRow, nRowsInPartialAverage, 3));

        AveragingParameters parameters (interval * averagingFactor,
                                        chunkInterval,
                                        SortColumns (),
                                        AveragingOptions (AveragingOptions::AverageObserved |
                                                          AveragingOptions::AverageModel |
                                                          AveragingOptions::AverageCorrected |
                                                          AveragingOptions::ModelUseCorrectedWeights |
                                                          AveragingOptions::CorrectedUseCorrectedWeights));
        VisibilityIterator2 vi (AveragingVi2Factory (parameters, ms));
        vi.setWeightScaling (WeightScaling::generateUnityWeightScaling());

        checkMs (& vi);

        printf ("\n...completed averaging of %d samples ...\n", averagingFactor);
    }

    void
    doWeightedTest (MeasurementSet * ms, Double interval, Int chunkInterval, Int averagingFactor)
    {
        printf ("\nStarting averaging of %d samples ...\n", averagingFactor);

        clearCheckers();

        Int lastRow = nRows_p / averagingFactor; // ID of first row of last averaged VB
        Int nRowsInPartialAverage = (nRows_p % averagingFactor) / nBaselines_p;
            // remaining

        addCubeChecker (new ComplexCubeRampChecker (averagingFactor, "Observed",
                                                    & VisBuffer2::visCube,
                                                    lastRow, nRowsInPartialAverage));
        addCubeChecker (new ComplexCubeRampChecker (averagingFactor, "Corrected",
                                                    & VisBuffer2::visCubeCorrected,
                                                    lastRow, nRowsInPartialAverage, 2));
        addCubeChecker (new ComplexCubeRampChecker (averagingFactor, "Model",
                                                    & VisBuffer2::visCubeModel,
                                                    lastRow, nRowsInPartialAverage, 3));

        AveragingParameters parameters (interval * averagingFactor,
                                        chunkInterval,
                                        SortColumns (),
                                        AveragingOptions (AveragingOptions::AverageObserved |
                                                          AveragingOptions::AverageModel |
                                                          AveragingOptions::AverageCorrected |
                                                          AveragingOptions::ModelUseWeights |
                                                          AveragingOptions::CorrectedUseWeights));
        VisibilityIterator2 vi (AveragingVi2Factory (parameters, ms));
        vi.setWeightScaling (WeightScaling::generateUnityWeightScaling());

        checkMs (& vi);

        printf ("\n...completed averaging of %d samples ...\n", averagingFactor);
    }

private:

    Int nAntennas_p;
    Int nBaselines_p;
    Double interval_p; // Time for each sample
    Int nRows_p;

};

typedef map<pair<Int, Int>, pair<Double, Double> > Velocity;


class HardUvwChecker : public RowChecker {

public:

    HardUvwChecker (Double maxUvwDistance, const Velocity & velocity, Double lastTime)
    : lastTime_p (lastTime),
      maxUvwDistance_p (maxUvwDistance),
      velocity_p (velocity)
    {}

    void checkRow (VisBuffer2 * vb, const VisibilityIterator2 * /*vi*/, Int rowId,
                   Int row, Int subchunkIndex)
    {
        // N.B.: assumes deltaT between samples is 1.0

        Int a1 = vb->antenna1() (row);
        Int a2 = vb->antenna2() (row);
        pair<Int, Int> baseline (a1, a2);
        Double t = vb->time() (row);

        pair<Double, Double> vUV = velocity_p [make_pair (a1, a2)];

        Double v = sqrt (pow (vUV.first, 2) + pow (vUV.second, 2));

        Int nAveraged = (v != 0) ? utilj::round (std::floor (maxUvwDistance_p / v)) + 1
                                 : INT_MAX;

        if (nAveraged + tBaseline_p [baseline] > lastTime_p){
            nAveraged = utilj::round (lastTime_p - tBaseline_p [baseline]) + 1;
        }

        Double expected = (nAveraged - 1) * 0.5 + tBaseline_p [baseline];
        Double actual = t;
        Double delta = expected - actual;

        if (abs (delta) > .01){

            throwError (String::toString (expected), String::toString (actual),
                        vb, rowId, row, subchunkIndex, "time",
                        String::format ("baseline=(%d,%d)", a1, a2));
        }

        tBaseline_p [baseline] += nAveraged;

    }


private:

    map <pair <Int, Int>, Double> tBaseline_p;
    Double lastTime_p;
    Double maxUvwDistance_p;
    Velocity velocity_p;
};

class BaselineDependentAveraging : public TestSuite {

public:

    BaselineDependentAveraging (const Environment & environment,
                                const Arguments & arguments,
                                Tester & tester)

    : TestSuite (environment, arguments, tester),
      interval_p (1)
    {}

    void execute ()
    {

        // These tests check averaging of different intervals
        // on a single spectral window.
        //
        // They should mimic the results from normal time-based averaging.

        printf ("+++ Starting BaselineDependentAveraging ...\n");

        String msName (getParameter ("msName", "AveragingTvi2.ms"));

        {

            printf ("+++ +++ Doing uniform baseline averaging ... \n");

            auto_ptr<MeasurementSet> ms;

            MeasurementSet * msTmp;
            boost::tie (msTmp, nRows_p) = createMs (msName, 1, 0);
            ms.reset (msTmp);

            doSimpleTest (ms.get(), interval_p, 10, 1, 0.1); // interval, chunkInterval, factor

            doSimpleTest (ms.get(), interval_p, 10, 2, 1.0);

            doSimpleTest (ms.get(), interval_p, 12, 3, 2.0);

            printf ("--- --- ... completed uniform baseline averaging ... \n");
        }

        {

            printf ("+++ +++ Doing nonuniform baseline averaging ... \n");

            auto_ptr<MeasurementSet> ms;

            MeasurementSet * msTmp;

            Velocity velocity;

            for (Int a1 = 0; a1 < nAntennas_p; a1 ++){
                for (Int a2 = 0; a2 < nAntennas_p; a2 ++){
                    Double vV = 0;
                    Double vU = a1;

                    velocity [make_pair (a1, a2)] = make_pair (vU, vV);
                }
            }

            boost::tie (msTmp, nRows_p) = createMs (msName, velocity);
            ms.reset (msTmp);

            doHarderTest (ms.get(), interval_p, 10, 1, 0.1, velocity); // interval, chunkInterval, factor

            doHarderTest (ms.get(), interval_p, 10, 2, 1.0, velocity);

            doHarderTest (ms.get(), interval_p, 12, 3, 2.0, velocity);

            doHarderTest (ms.get(), interval_p, 12, 3, 3.0, velocity);

            doHarderTest (ms.get(), interval_p, 12, 3, 4.0, velocity);

            doHarderTest (ms.get(), interval_p, 12, 3, 1000, velocity);


            printf ("--- --- ... completed nonuniform baseline averaging ... \n");
        }


        printf ("--- ... completed BaselineDependentAveraging\n");

    }

    static String getName () { return "BaselineDependentAveraging";}

protected:

    pair<MeasurementSet *,Int>
    createMs (const String & msName, Double vU, Double vV)
    {
        nAntennas_p = 4;

        Velocity velocity;

        for (Int a1 = 0; a1 < nAntennas_p; a1 ++){
            for (Int a2 = 0; a2 < nAntennas_p; a2 ++){
                velocity [make_pair (a1, a2)] = make_pair (vU, vV);
            }
        }

        return createMs (msName, velocity);
    }


    pair<MeasurementSet *,Int>
    createMs (const String & msName, const Velocity & velocity)
    {
        system (String::format ("rm -r %s", msName.c_str()).c_str());

        MsFactory * msFactory = new MsFactory (msName);

        nAntennas_p = 4;
        nBaselines_p = ((nAntennas_p - 1) * nAntennas_p) / 2;
        msFactory->setTimeInfo (0, 120, interval_p);
        msFactory->addSpectralWindows(1); // only one spw for now
        msFactory->addAntennas(nAntennas_p);
        msFactory->addFeeds (10); // needs antennas and spws to be setup first
        msFactory->addWeightSpectrum (False);

        for (Int i = 0; i < 10; i++){
            msFactory->addField (String::format ("field%d", i), MDirection());
        }

        // For many of the columns, simply put in a distinct constant to see if
        // the correct data is being processed as well as averaged properly.

        msFactory->setDataGenerator (MSMainEnums::UVW,
                                     new UvwMotionGenerator (velocity));

        msFactory->setDataGenerator (MSMainEnums::SCAN_NUMBER,
                                     new GenerateConstant<Int> (10));
        msFactory->setDataGenerator (MSMainEnums::OBSERVATION_ID,
                                     new GenerateConstant<Int> (11));
        msFactory->setDataGenerator (MSMainEnums::ARRAY_ID,
                                     new GenerateConstant<Int> (12));
        msFactory->setDataGenerator (MSMainEnums::FEED1,
                                     new GenerateConstant<Int> (9));
        msFactory->setDataGenerator (MSMainEnums::FEED2,
                                     new GenerateConstant<Int> (8));
        msFactory->setDataGenerator (MSMainEnums::FIELD_ID,
                                     new GenerateConstant<Int> (7));
        msFactory->setDataGenerator (MSMainEnums::STATE_ID,
                                     new GenerateConstant<Int> (17));
        msFactory->setDataGenerator (MSMainEnums::PROCESSOR_ID,
                                     new GenerateConstant<Int> (18));
        msFactory->setDataGenerator (MSMainEnums::EXPOSURE,
                                     new GenerateConstant<Double> (interval_p));
        msFactory->setDataGenerator (MSMainEnums::INTERVAL,
                                     new GenerateConstant<Double> (interval_p));
        msFactory->setDataGenerator (MSMainEnums::SIGMA,
                                     new GenerateConstant<Float> (1.0f));
        msFactory->setDataGenerator (MSMainEnums::WEIGHT,
                                     new GenerateConstant<Float> (1.0f));

        // For the data cubes fill it with a ramp.  The real part of the ramp will
        // be multiplied by the factor supplied in the constructor to check that
        // there's no "crosstalk" between the columns.

        msFactory->setDataGenerator(MSMainEnums::DATA, new GenerateRamp());

        msFactory->setDataGenerator(MSMainEnums::CORRECTED_DATA, new GenerateRamp(2));

        msFactory->setDataGenerator(MSMainEnums::MODEL_DATA, new GenerateRamp(3));

        // Set all of the data to be unflagged.

        msFactory->setDataGenerator(MSMainEnums::FLAG, new GenerateConstant<Bool> (False));
        msFactory->setDataGenerator(MSMainEnums::FLAG_ROW, new GenerateConstant<Bool> (False));

        // Set the time centroid to be the middle of the sample interval.

        msFactory->setDataGenerator(MSMainEnums::TIME_CENTROID, new GenerateTimeCentroid ());

        pair<MeasurementSet *, Int> p = msFactory->createMs ();

        return make_pair (p.first, p.second);
    }


    void
    doSimpleTest (MeasurementSet * ms, Double interval, Int chunkInterval, Int averagingFactor,
                  Double maxUvwDistance)
    {
        printf ("\nStarting averaging of %d samples ...\n", averagingFactor);

        clearCheckers();

        Int lastRow = nRows_p / averagingFactor; // ID of first row of last averaged VB
        Int nRowsInPartialAverage = (nRows_p % averagingFactor) / nBaselines_p;
            // remaining

        addCubeChecker (new ComplexCubeRampChecker (averagingFactor, "Observed", & VisBuffer2::visCube,
                                                    lastRow, nRowsInPartialAverage));
        addCubeChecker (new ComplexCubeRampChecker (averagingFactor, "Corrected",
                                                    & VisBuffer2::visCubeCorrected,
                                                    lastRow, nRowsInPartialAverage, 2));
        addCubeChecker (new ComplexCubeRampChecker (averagingFactor, "Model",
                                                    & VisBuffer2::visCubeModel,
                                                    lastRow, nRowsInPartialAverage, 3));

        addRowChecker (generateSimpleRowChecker ("scan", & VisBuffer2::scan, 10));
        addRowChecker (generateSimpleRowChecker ("observationId", & VisBuffer2::observationId, 11));
        addRowChecker (generateSimpleRowChecker ("arrayId", & VisBuffer2::arrayId, 12));
        addRowChecker (generateSimpleRowChecker ("feed1", & VisBuffer2::feed1, 9));
        addRowChecker (generateSimpleRowChecker ("feed2", & VisBuffer2::feed2, 8));
        addRowChecker (generateSimpleRowChecker ("fieldId", & VisBuffer2::fieldId, 7));
        addRowChecker (generateSimpleRowChecker ("stateId", & VisBuffer2::stateId, 17));
        addRowChecker (generateSimpleRowChecker ("processorId", & VisBuffer2::processorId, 18));

        Double expectedDuration = interval * averagingFactor;
//        addRowChecker (generateSimpleRowChecker ("interval", & VisBuffer2::timeInterval,
//                                                 expectedDuration));
        addRowChecker (new SimpleIntervalChecker (averagingFactor, interval, lastRow, nRowsInPartialAverage));
        addRowChecker (new SimpleTimeChecker (averagingFactor, interval, expectedDuration,
                                              lastRow, nRowsInPartialAverage));

        addRowChecker (new SimpleExposureChecker (averagingFactor, interval, lastRow, nRowsInPartialAverage));

        addRowChecker (new SimpleTimeCentroidChecker (averagingFactor, interval, lastRow, nRowsInPartialAverage));

        Double channelFraction = 1.0; // no flagged channels
        addRowChecker (new SigmaWeightChecker (averagingFactor, & VisBuffer2::weight, "Weight",
                                               lastRow, nRowsInPartialAverage, channelFraction));
        addRowChecker (new SigmaWeightChecker (averagingFactor, & VisBuffer2::sigma, "Sigma",
                                               lastRow, nRowsInPartialAverage, channelFraction, True));


        AveragingParameters parameters (interval * averagingFactor,
                                        chunkInterval,
                                        SortColumns (),
                                        AveragingOptions (AveragingOptions::BaselineDependentAveraging |
                                                          AveragingOptions::AverageObserved |
                                                          AveragingOptions::AverageModel |
                                                          AveragingOptions::AverageCorrected |
                                                          AveragingOptions::ObservedUseNoWeights |
                                                          AveragingOptions::ModelUseNoWeights |
                                                          AveragingOptions::CorrectedUseNoWeights),
                                        maxUvwDistance);
        VisibilityIterator2 vi (AveragingVi2Factory (parameters, ms));
        vi.setWeightScaling (WeightScaling::generateUnityWeightScaling());

        checkMs (& vi);

        printf ("\n...completed averaging of %d samples ...\n", averagingFactor);
    }

    void
    doHarderTest (MeasurementSet * ms, Double interval, Int chunkInterval, Int averagingFactor,
                  Double maxUvwDistance, const Velocity & velocity)
    {
        printf ("\nStarting averaging with maxUvwDistance=%f ...\n", maxUvwDistance);

        clearCheckers();

//        Int lastRow = nRows_p / averagingFactor; // ID of first row of last averaged VB
//        Int nRowsInPartialAverage = (nRows_p % averagingFactor) / nBaselines_p;
            // remaining

        addRowChecker (new HardUvwChecker (maxUvwDistance, velocity, 119));

        setExpectedRowCount (getNRowsExpected (velocity, maxUvwDistance, 120));

        AveragingParameters parameters (interval * averagingFactor * 10000,
                                        chunkInterval * 10000,
                                        SortColumns (),
                                        AveragingOptions (AveragingOptions::BaselineDependentAveraging |
                                                          AveragingOptions::AverageObserved |
                                                          AveragingOptions::AverageModel |
                                                          AveragingOptions::AverageCorrected |
                                                          AveragingOptions::ObservedUseNoWeights |
                                                          AveragingOptions::ModelUseNoWeights |
                                                          AveragingOptions::CorrectedUseNoWeights),
                                        maxUvwDistance);
        VisibilityIterator2 vi (AveragingVi2Factory (parameters, ms));
        vi.setWeightScaling (WeightScaling::generateUnityWeightScaling());

        checkMs (& vi);

        printf ("\n...completed averaging with maxUvwDistance=%f ...\n", maxUvwDistance);
    }

    Int
    getNRowsExpected (const Velocity & velocity, Double maxUvwDistance, Int nRowsInput)
    {
        Int nRows = 0;

        for (Int a1 = 0; a1 < nAntennas_p - 1; a1++){
            for (Int a2 = a1 + 1; a2 < nAntennas_p; a2 ++){


                pair<Double, Double> vB = velocity.at (make_pair (a1, a2));
                Double v = sqrt (pow (vB.first, 2) + pow (vB.second, 2));

                Int nAverages;
                if (v != 0){
                    Int nPerAverage = utilj::round (std::floor (maxUvwDistance / v)) + 1;
                    nAverages = utilj::round (std::ceil (((double) nRowsInput) / nPerAverage));
                }
                else{
                    nAverages = 1;
                }

                nRows += nAverages;
            }
        }

        return nRows;
    }


private:

    Int nAntennas_p;
    Int nBaselines_p;
    Double interval_p; // Time for each sample
    Int nRows_p;

};

Tester::Tester (Int nRowsExpected)
: nRowsExpected_p (nRowsExpected),
  nTestsAttempted_p (0),
  nTestsPassed_p (0)
{
    clearCheckers(False);
}

void
Tester::addCubeChecker (CubeChecker * cubeChecker)
{
    cubeCheckers_p.push_back (cubeChecker);
}

void
Tester::addRowChecker (RowChecker * rowChecker)
{
    rowCheckers_p.push_back (rowChecker);
}

void
Tester::clearCheckers (Bool deleteIt)
{

    for (CubeCheckers::iterator cc = cubeCheckers_p.begin();
         cc != cubeCheckers_p.end();
         cc ++){

        if (deleteIt){
            delete * cc;
        }
    }

    cubeCheckers_p.clear();

    for (RowCheckers::iterator rc = rowCheckers_p.begin();
         rc != rowCheckers_p.end();
         rc ++){

        if (deleteIt){
            delete * rc;
        }
    }

    rowCheckers_p.clear();
}

Bool
Tester::isTestSelected (const Arguments & arguments, const String & testName) const
{
    if (! utilj::containsKey ("--test", arguments))
    {
        return True;
    }

    String selectedTest = arguments.find ("--test")->second;

    if (downcase (selectedTest) == "all"){
        return True;
    }

    if (selectedTest.size () > testName.size()){

        return False;
    }

    String testNameTruncated = testName.substr (0, selectedTest.size());

    Bool enabled = downcase (testNameTruncated) == downcase (selectedTest);

    return enabled;
}


template <typename Test>
void
Tester::doTest (const Environment & environment, const Arguments & arguments)
{
    if (! isTestSelected (arguments, Test::getName())){
        return;
    }

    nTestsAttempted_p ++;
    Bool failed = True;

    try {

        Test test (environment, arguments, * this);

        printf ("+++ Starting execution of test %s\n", Test::getName().c_str());
        fflush (stdout);

        test.execute();

        printf ("--- Successfully completed execution of test %s\n", Test::getName().c_str());
        fflush (stdout);

        nTestsPassed_p ++;
        failed = False;
    }
    catch (TestError & e){

        fprintf (stderr, "\n***\n***\n*** TestError while executing test %s:\n***\n-->%s\n***\n***\n",
                 Test::getName().c_str(), e.what());
    }
    catch (AipsError & e){

        fprintf (stderr, "\n*** AipsError while executing test %s:\n\n-->%s\n\n",
                 Test::getName().c_str(), e.what());

    }
    catch (...){

        fprintf (stderr, "\n*** Unknown exception while executing test %s\n***\n*** Exiting ***\n\n",
                 Test::getName().c_str());
    }

    testResults_p.push_back (make_pair (Test::getName(), failed ? "FAILED" : "passed"));

    nRowsExpected_p = -1;
}


Bool
Tester::doTests (Int nArgs, char * args [])
{
    Bool ok = False;

    try {

        Environment environment;
        Arguments arguments = parseArgs (nArgs, args);

        doTest <SimpleTests> (environment, arguments);

        doTest <WeightingTests> (environment, arguments);

        // RowFlagging is out these days
        //doTest <RowFlaggingTests> (environment, arguments);

        doTest <CubeFlaggingTests> (environment, arguments);

        doTest <SimpleTestsNWindows<2> > (environment, arguments);

        doTest <WeightSelectionTests> (environment, arguments);

        doTest <BaselineDependentAveraging> (environment, arguments);

        if (nTestsAttempted_p == nTestsPassed_p){

            printf ("\n...\n... Passed all %d tests attempted ;-)\n...\n", nTestsAttempted_p);
            ok = True;
        }
        else{
            printf ("\n???\n??? FAILED %d of %d tests attempted ;-(\n...\n",
                    nTestsAttempted_p - nTestsPassed_p, nTestsAttempted_p);
        }

        printf ("... Summary:\n\n");

        for (vector<pair <String, String> >::const_iterator i = testResults_p.begin();
             i != testResults_p.end();
             i ++){

            printf ("...    o %s --> %s\n", i->first.c_str(), i->second.c_str());
        }

        fflush (stdout);
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

    return ok;

}


Arguments
Tester::parseArgs (int nArgs, char * args []) const
{
    String optionsRaw [] = {"--old", "--new", "--sweeps", "--nChannels", "--test", ""};
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
Tester::setExpectedRowCount (Int n)
{
    nRowsExpected_p = n;
}

void
Tester::checkMs (VisibilityIterator2 * vi)
{
    VisBuffer2 * vb = vi->getVisBuffer();

    Int chunk = 0;
    Int subchunk = 0;
    Int firstRow = 0;
    map<Int, Int> firstSubchunk;

    for (vi->originChunks(); vi->moreChunks(); vi->nextChunk()){
        for (vi->origin(); vi->more(); vi->next()){

            Int ddId = vb->dataDescriptionIds()(0);

            if (vb->isNewSpectralWindow() &&
                ! utilj::containsKey (ddId, firstSubchunk)){

                firstSubchunk [ddId] = subchunk;
            }

            Int subchunkOrigin = firstSubchunk [ddId];

            checkRows (vb, vi, subchunk - subchunkOrigin, firstRow);

            checkCubes (vb, vi, subchunk - subchunkOrigin, firstRow);

            subchunk ++;

            firstRow += vb->nRows();

        }
        chunk ++;
    }

    checkRowCount (firstRow);
}

void
CubeChecker::throwError (const String & expected, const String & actual,
                         const VisBuffer2 * vb, Int rowId, Int row, Int channel, Int correlation,
                         Int subchunkIndex, const String & objectName, const String & extra) const
{
    String message =

        String::format("Expected %s, got %s for %s cube at:\n"
                       "spw=%d, vbRow=%d, msRow=%d, ch=%d, corr=%d, a1=%d, a2=%d, subchunk=%d, t=%f\n%s",
                       expected.c_str(),
                       actual.c_str(),
                       objectName.c_str(),
                       vb->spectralWindows()(0),
                       row,
                       rowId,
                       channel,
                       correlation,
                       vb->antenna1 () (row),
                       vb->antenna2 () (row),
                       subchunkIndex,
                       vb->time() (row),
                       extra.c_str());

    ThrowTestError (message);
}

void
Tester::checkCubes (VisBuffer2 * vb, VisibilityIterator2 * vi, Int subchunk,
                    Int firstRow)
{
    for (Int row = 0; row < vb->nRows(); row ++){

        for (Int channel = 0; channel < vb->nChannels(); channel ++){

            for (Int correlation = 0; correlation < vb->nCorrelations(); correlation ++){

                for (CubeCheckers::iterator cc = cubeCheckers_p.begin();
                     cc != cubeCheckers_p.end();
                     cc ++){

                    (*cc)->checkCube (vb, vi, firstRow + row,
                                      row, channel, correlation,
                                      subchunk);
                }
            }
        }
    }
}

void
Tester::checkRowCount (Int actualNumberOfRows)
{
    if (nRowsExpected_p >= 0){

        if (actualNumberOfRows != nRowsExpected_p){

            ThrowTestError (String::format ("Expected to process %d rows but received %d rows.",
                                            nRowsExpected_p, actualNumberOfRows));
        }

        printf ("... Processed expected number of rows: %d\n", actualNumberOfRows);
    }
    else{
        printf ("... Processed %d rows.\n", actualNumberOfRows);
    }
}


void
Tester::checkRows (VisBuffer2 * vb, VisibilityIterator2 * vi, Int subchunk,
                   Int firstRow)
{
    for (Int row = 0; row < vb->nRows(); row ++){

        for (RowCheckers::iterator rc = rowCheckers_p.begin();
             rc != rowCheckers_p.end();
             rc ++){

            (*rc)->checkRow (vb, vi, firstRow + row,
                             row, subchunk);
        }
    }
}





} // end namespace test
} // end namespace vi
} // end namespace casa

using namespace casa;

int
main (int nArgs, char * args [])
{
    LogSink::globalSink().filter (LogFilter (LogMessage::WARN));

    LogIO os;

    casa::vi::test::Tester tester;

    casa::Bool ok = tester.doTests (nArgs, args);

    exit (ok ? 0 : 1);
}


