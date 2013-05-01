#include <casa/BasicSL.h>
#include <casa/string.h>
#include <ms/MeasurementSets.h>
#include <synthesis/MSVis/VisBuffer2.h>
#include <synthesis/MSVis/VisibilityIterator2.h>
#include <synthesis/MSVis/test/MsFactory.h>

#include <boost/tuple/tuple.hpp>
#include <map>
#include <memory>
#include <utility>

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

    WeightChecker (Int averagingFactor) : averagingFactor_p ((Float) averagingFactor) {}

    void
    checkCube (VisBuffer2 * vb, const VisibilityIterator2 * /*vi*/, Int rowId,
               Int row, Int channel, Int correlation, Int subchunkIndex)
    {
        Float weight = vb->weightSpectrum() (correlation, channel, row);

        Bool ok = deltaEq (weight, averagingFactor_p, .001);

        if (! ok){
            String expected = String::format ("%f", averagingFactor_p);
            String actual = String::format ("%f", weight);

            throwError (expected, actual, vb, rowId, row, channel, correlation, subchunkIndex,
                        "weightSpectrum", String::format ("averagingFactor=%d", averagingFactor_p));
        }
    }

private:

    Float averagingFactor_p;
};


class ComplexCubeRampChecker : public CubeChecker {

public:

    typedef const Cube<Complex> & (VisBuffer2::* Accessor) () const;

    ComplexCubeRampChecker (Int averagingFactor, const String & name,Accessor accessor, Float factor = 1)
    : accessor_p (accessor), averagingFactor_p (averagingFactor), factor_p (factor), name_p (name) {}

    void checkCube (VisBuffer2 * vb, const VisibilityIterator2 * /*vi*/, Int rowId,
                    Int row, Int channel, Int correlation, Int subchunkIndex)
    {
        Float imaginary = (((vb->antenna1 ()(row) * 10 + vb->antenna2() (row)) * 10 +
                vb->spectralWindows()(row)) * 10 + correlation) * 10 + channel;

        Float real = (subchunkIndex * averagingFactor_p) + ((averagingFactor_p - 1) / 2.0);
        real *= factor_p;

        Complex z0 (real, imaginary);
        //z0 *= factor_p;
        Complex z = (vb ->* accessor_p) () (correlation, channel, row);

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

    Accessor accessor_p;
    Int averagingFactor_p;
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
        ThrowTestError (String::format ("For %s: expected %s got %s at subchunk=%d, "
                                        "msRow=%d, vbRow=%d, t=%f\n%s",
                                        objectName.c_str(),
                                        expected.c_str(),
                                        actual.c_str(),
                                        subchunkIndex,
                                        rowId,
                                        row,
                                        vb->time()(row),
                                        extra.c_str()));
    }
};



class SigmaWeightChecker : public RowChecker {

public:

    typedef const Matrix<Float> & (VisBuffer2::* Accessor) () const;

    SigmaWeightChecker (Int averagingFactor, Accessor accessor, const String & name)
    : accessor_p (accessor), averagingFactor_p (averagingFactor), name_p (name)
    {}

    virtual void checkRow (VisBuffer2 * vb, const VisibilityIterator2 * /*vi*/, Int rowId,
                           Int row, Int subchunkIndex){

        for (Int c = 0; c < vb->nCorrelations(); c ++){

            Float actual = (vb ->* accessor_p)()(c, row);
            Bool ok = deltaEq (actual, averagingFactor_p, .001);

            if (! ok){

                throwError (String::format ("%f", averagingFactor_p * 1.0),
                            String::format ("%f", actual),
                            vb, rowId, row, subchunkIndex, name_p,
                            String::format ("correlation=%d", c));

            }
        }
    }

private:

    Accessor accessor_p;
    Int averagingFactor_p;
    String name_p;

};



class SimpleIntervalChecker : public RowChecker {

public:

    SimpleIntervalChecker (Int averagingFactor) : averagingFactor_p (averagingFactor) {}

    virtual void checkRow (VisBuffer2 * vb, const VisibilityIterator2 * /*vi*/, Int rowId,
                           Int row, Int subchunkIndex){

        Bool ok = deltaEq (vb->timeInterval()(row), averagingFactor_p, .001);

        if (! ok){

            throwError (String::format ("%f", averagingFactor_p * 1.0),
                        String::format ("%f", vb->timeInterval()(row)),
                        vb, rowId, row, subchunkIndex, "Interval");

        }
    }

private:

    Int averagingFactor_p;

};

class AlternatingFlagRowChecker : public RowChecker {

public:

    AlternatingFlagRowChecker (Int averagingFactor) : averagingFactor_p (averagingFactor) {}

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

            Bool ok = deltaEq (vb->time()(row), subchunkIndex * averagingFactor_p, .001);

            if (! ok){

                throwError (String::format ("%f", subchunkIndex * 1.0),
                            String::format ("%f", vb->time()(row)),
                            vb, rowId, row, subchunkIndex, "time");
            }

        }

    }

private:

    Int averagingFactor_p;
};

class SimpleTimeChecker : public RowChecker {

public:

    SimpleTimeChecker (Int averagingFactor) : averagingFactor_p (averagingFactor) {}

    virtual void checkRow (VisBuffer2 * vb, const VisibilityIterator2 * /*vi*/, Int rowId,
                           Int row, Int subchunkIndex){

        Bool ok = deltaEq (vb->time()(row), averagingFactor_p * subchunkIndex, .001);

        if (! ok){

            throwError (String::format ("%f", averagingFactor_p * subchunkIndex * 1.0),
                        String::format ("%f", vb->time()(row)),
                        vb, rowId, row, subchunkIndex, "");

        }
    }

private:

    Int averagingFactor_p;

};

class SimpleRowChecker : public RowChecker {

public:

    typedef const Vector<Int> & (VisBuffer2::* Accessor) () const;

    SimpleRowChecker (const String & name, Accessor accessor, Int expectedValue)
    : accessor_p (accessor),
      expectedValue_p (expectedValue),
      name_p (name)
    {}

    virtual void checkRow (VisBuffer2 * vb, const VisibilityIterator2 * /*vi*/, Int rowId,
                           Int row, Int subchunkIndex){

        Int actualValue = (vb ->* accessor_p) () (row);
        Bool ok = actualValue == expectedValue_p;

        if (! ok){

            throwError (String::format ("%d", expectedValue_p),
                        String::format ("%d", actualValue),
                        vb, rowId, row, subchunkIndex, name_p);

        }
    }

private:

    Accessor accessor_p;
    Int expectedValue_p;
    String name_p;

};



class Tester {

public:

    friend class TestSuite;

    Tester ();
    void doTests (Int nArgs, char * args []);

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
    checkRows (VisBuffer2 * vb, VisibilityIterator2 * vi, Int subchunk,
               Int firstRow);

    void clearCheckers(Bool deleteIt = True);

    void doSimpleTest (MeasurementSet * ms, Int interval, Int chunkInterval, Int averagingFactor);
    void doSimpleTests (MeasurementSet * ms);

    void addCubeChecker (CubeChecker * cubeChecker);
    void addRowChecker (RowChecker * rowChecker);


private:

    typedef vector<CubeChecker *> CubeCheckers;
    typedef vector<RowChecker *> RowCheckers;

    CubeCheckers cubeCheckers_p;
    vector<String> failedTests_p;
    RowCheckers rowCheckers_p;
    MsFactory * msf_p;
    Int nTestsAttempted_p;
    Int nTestsPassed_p;
};

////////////////////////////////////////////////////////////
//
// Generators
//

class GenerateAlternatingFlagRows : public Generator<Bool>{

public:

    virtual Bool operator() (const FillState & fillState,
                             Int /*channel*/, Int /*correlation*/) const {

        Int timeIndex = fillState.rowNumber_p / (fillState.nAntennas_p * (fillState.nAntennas_p - 1) / 2);

        Bool theFlag = (Int) (timeIndex + 0.1) % 2 == 1;

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

    : TestSuite (environment, arguments, tester) {}

    void execute ()
    {
        printf ("+++ Starting SimpleTests ...\n");

        String msName (getParameter ("msName", "AveragingTvi2.ms"));
        Int nRows;
        auto_ptr<MeasurementSet> ms;

        MeasurementSet * msTmp;
        boost::tie (msTmp, nRows) = createMs (msName);
        ms.reset (msTmp);

        doSimpleTest (ms.get(), 1, 10, 1); // interval, chunkInterval, factor

        doSimpleTest (ms.get(), 1, 10, 2);

        doSimpleTest (ms.get(), 1, 12, 3);

        doSimpleTest (ms.get(), 1, 12, 4);

        doSimpleTest (ms.get(), 1, 15, 5);

        doSimpleTest (ms.get(), 1, 12, 6);

        doSimpleTest (ms.get(), 1, 16, 8);

        doSimpleTest (ms.get(), 1, 10, 10);

        doSimpleTest (ms.get(), 1, 20, 10);

        doSimpleTest (ms.get(), 1, 12, 12);

        doSimpleTest (ms.get(), 1, 24, 12);

        printf ("--- ... completed SimpleTests\n");
    }

    static String getName () { return "SimpleTests";}

protected:

    pair<MeasurementSet *,Int>
    createMs (const String & msName)
    {
        system (String::format ("rm -r %s", msName.c_str()).c_str());

        MsFactory * msFactory = new MsFactory (msName);

        msFactory->setTimeInfo (0,120,1);
        msFactory->addSpectralWindows(1); // only on spw for now
        msFactory->addAntennas(4);
        msFactory->addFeeds (10); // needs antennas and spws to be setup first

        for (Int i = 0; i < 10; i++){
            msFactory->addField (String::format ("field%d", i), MDirection());
        }

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

        msFactory->setDataGenerator(MSMainEnums::DATA, new GenerateRamp());

        msFactory->setDataGenerator(MSMainEnums::CORRECTED_DATA, new GenerateRamp(2));

        msFactory->setDataGenerator(MSMainEnums::MODEL_DATA, new GenerateRamp(3));

        msFactory->setDataGenerator(MSMainEnums::FLAG, new GenerateConstant<Bool> (False));
        msFactory->setDataGenerator(MSMainEnums::FLAG_ROW, new GenerateConstant<Bool> (False));

        pair<MeasurementSet *, Int> p = msFactory->createMs ();

        return make_pair (p.first, p.second);
    }


    void
    doSimpleTest (MeasurementSet * ms, Int interval, Int chunkInterval, Int averagingFactor)
    {
        printf ("\nStarting averaging of %d samples ...\n", averagingFactor);

        clearCheckers();
        addCubeChecker (new ComplexCubeRampChecker (averagingFactor, "Observed", & VisBuffer2::visCube));
        addCubeChecker (new ComplexCubeRampChecker (averagingFactor, "Corrected",
                                                    & VisBuffer2::visCubeCorrected, 2));
        addCubeChecker (new ComplexCubeRampChecker (averagingFactor, "Model",
                                                    & VisBuffer2::visCubeModel, 3));

        addRowChecker (new SimpleRowChecker ("scan", & VisBuffer2::scan, 10));
        addRowChecker (new SimpleRowChecker ("observationId", & VisBuffer2::observationId, 11));
        addRowChecker (new SimpleRowChecker ("arrayId", & VisBuffer2::arrayId, 12));
        addRowChecker (new SimpleRowChecker ("feed1", & VisBuffer2::feed1, 9));
        addRowChecker (new SimpleRowChecker ("feed2", & VisBuffer2::feed2, 8));
        addRowChecker (new SimpleRowChecker ("fieldId", & VisBuffer2::fieldId, 7));
        addRowChecker (new SimpleRowChecker ("stateId", & VisBuffer2::stateId, 17));
        addRowChecker (new SimpleRowChecker ("processorId", & VisBuffer2::processorId, 18));

        AveragingTvi2Factory factory;
        VisibilityIterator2 * vi = factory.createVi (ms, interval, chunkInterval, averagingFactor);

        checkMs (vi);

        delete vi;

        printf ("\n...completed averaging of %d samples ...\n", averagingFactor);
    }

private:


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
        boost::tie (msTmp, nRows) = createMs (msName);
        ms.reset (msTmp);

        doTest (ms.get(), 1, 10, 1); // interval, chunkInterval, factor

        doTest (ms.get(), 1, 10, 2);

        doTest (ms.get(), 1, 12, 4);

        printf ("--- ... completed RowFlaggingTests\n");
    }

    static String getName () { return "RowFlaggingTests";}

protected:

    pair<MeasurementSet *,Int>
    createMs (const String & msName)
    {
        system (String::format ("rm -r %s", msName.c_str()).c_str());

        MsFactory * msFactory = new MsFactory (msName);

        msFactory->setTimeInfo (0, 120, 1);
        msFactory->addSpectralWindows(1); // only on spw for now

        auto_ptr<GenerateRamp> rampGenerator (new GenerateRamp());
        msFactory->setDataGenerator(MSMainEnums::DATA, rampGenerator.get());

        auto_ptr<GenerateConstant<Bool> > generateFalse (new GenerateConstant<Bool> (False));
        msFactory->setDataGenerator(MSMainEnums::FLAG, generateFalse.get());

        auto_ptr<GenerateAlternatingFlagRows> generateAlternatingFlagRows (new GenerateAlternatingFlagRows());
        msFactory->setDataGenerator(MSMainEnums::FLAG_ROW, generateAlternatingFlagRows.get());

        pair<MeasurementSet *, Int> p = msFactory->createMs ();

        return make_pair (p.first, p.second);
    }

    void
    doTest (MeasurementSet * ms, Int interval, Int chunkInterval, Int averagingFactor)
    {
        printf ("\nStarting averaging of %d samples ...\n", averagingFactor);

        clearCheckers();
        addRowChecker (new AlternatingFlagRowChecker (averagingFactor));
        addCubeChecker (new FlagRowRampChecker (averagingFactor, "FlagRowRampChecker"));

        AveragingTvi2Factory factory;
        VisibilityIterator2 * vi = factory.createVi (ms, interval, chunkInterval, averagingFactor);

        checkMs (vi);

        delete vi;

        printf ("\n...completed averaging of %d samples ...\n", averagingFactor);
    }

private:


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

        msFactory->setTimeInfo (0,120,1);
        msFactory->addSpectralWindows(1); // only on spw for now

        msFactory->setDataGenerator(MSMainEnums::WEIGHT_SPECTRUM,
                                    new GenerateConstant<Float> (1.0));

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
        addCubeChecker (new ComplexCubeRampChecker (averagingFactor, "Observed", & VisBuffer2::visCube));
        addCubeChecker (new WeightChecker (averagingFactor));

        addRowChecker (new SigmaWeightChecker (averagingFactor, & VisBuffer2::sigma, "Sigma"));
        addRowChecker (new SigmaWeightChecker (averagingFactor, & VisBuffer2::weight, "Weight"));

        WeightFunction * weightFunction = useDoubler ? generateWeightFunction (doubler)
                                                     : WeightFunction::generateIdentityWeightFunction();

        AveragingTvi2Factory factory;
        VisibilityIterator2 * vi = factory.createVi (ms, interval, chunkInterval, averagingFactor,
                                                     weightFunction);

        checkMs (vi);

        delete vi;

        printf ("\n...completed averaging of %d samples ...\n", averagingFactor);
    }

private:


};



Tester::Tester ()
: nTestsAttempted_p (0),
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

template <typename Test>
void
Tester::doTest (const Environment & environment, const Arguments & arguments)
{
    nTestsAttempted_p ++;

    printf ("+++ Starting execution of test %s\n", Test::getName().c_str());
    fflush (stdout);
    Bool failed = True;

    try {

        Test test (environment, arguments, * this);
        test.execute();

        printf ("--- Successfully completed execution of test %s\n", Test::getName().c_str());
        fflush (stdout);

        nTestsPassed_p ++;
        failed = False;
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

    if (failed){
        failedTests_p.push_back (Test::getName());
    }
}


void
Tester::doTests (Int nArgs, char * args [])
{
    pair<Bool,Bool> result;

    try {

        Environment environment;
        Arguments arguments = parseArgs (nArgs, args);

        doTest<SimpleTests> (environment, arguments);

        doTest<WeightingTests> (environment, arguments);

        doTest<RowFlaggingTests> (environment, arguments);


//        SimpleTests simpleTests (environment, arguments, * this);
//        simpleTests.execute();
//
//        WeightingTests weightingTests (environment, arguments, * this);
//        weightingTests.execute();
//
//        RowFlaggingTests rowFlaggingTests (environment, arguments, * this);
//        rowFlaggingTests.execute();

        if (nTestsAttempted_p == nTestsPassed_p){

            printf ("\n***\n*** Passed all %d tests attempted ;-)\n***\n", nTestsAttempted_p);
        }
        else{
            printf ("\n???\n??? Failed %d of %d tests attempted ;-(\n???\n??? Tests failing:\n\n",
                    nTestsAttempted_p - nTestsPassed_p, nTestsAttempted_p);

            for (vector<String>::const_iterator i = failedTests_p.begin();
                 i != failedTests_p.end();
                 i ++){

                printf ("???    o %s\n", i->c_str());
            }
            printf ("???\n");

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

}


Arguments
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
Tester::checkMs (VisibilityIterator2 * vi)
{
    VisBuffer2 * vb = vi->getVisBuffer();

    Int chunk = 0;
    Int subchunk = 0;
    Int firstRow = 0;

    for (vi->originChunks(); vi->moreChunks(); vi->nextChunk()){
        for (vi->origin(); vi->more(); vi->next()){

            checkRows (vb, vi, subchunk, firstRow);

            checkCubes (vb, vi, subchunk, firstRow);

            subchunk ++;

            firstRow += vb->nRows();

        }
        chunk ++;
    }
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

int
main (int nArgs, char * args [])
{
    casa::vi::test::Tester tester;

    tester.doTests (nArgs, args);
}


