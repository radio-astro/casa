#if ! defined (VisibilityIterator_Test_H_20120912_1001)
#define VisibilityIterator_Test_H_20120912_1001

#include <casa/aips.h>
#include <casa/aipstype.h>
#include <casa/BasicSL/String.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <synthesis/MSVis/UtilJ.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ScalarColumn.h>
#include <boost/tuple/tuple.hpp>

#include <map>
#include <set>

namespace casa {

    class MeasurementSet;
    class NewMSSimulator;
    class ROVisibilityIterator;


    namespace vi {

        class VisibilityIterator2;
        class VisBuffer2;

    }
}

namespace casa {
namespace vi {
namespace test {

class MsFactory;

class TestWidget {

public:

    TestWidget (const String & name) : name_p (name) {}

    virtual ~TestWidget () {}

    virtual String name () const = 0;

    virtual boost::tuple <MeasurementSet *, Int, Bool> createMs (){
        return boost::make_tuple ((MeasurementSet *) 0, 0, False);
        // Useless placeholder implementation
    }

    virtual boost::tuple <Block<const MeasurementSet *>, Int, Bool> createMss (){
        return boost::make_tuple (Block<const MeasurementSet *> (), 0, False);
        // Useless placeholder implementation
    }

    virtual void endOfChunk (VisibilityIterator2 & /*vi*/, VisBuffer2 * /*vb*/) {}
    virtual void nextChunk (VisibilityIterator2 & /*vi*/, VisBuffer2 * /*vb*/) {}
    virtual void nextSubchunk (VisibilityIterator2 & /*vi*/, VisBuffer2 * /*vb*/) {}
    virtual Bool noMoreData (VisibilityIterator2 & /*vi*/, VisBuffer2 * /*vb*/, int /*nRows*/)
    { return False;}
    virtual void startOfData (VisibilityIterator2 & /*vi*/, VisBuffer2 * /*vb*/) {}
    virtual bool usesMultipleMss () const { return False;}

private:

    String name_p;
};

class BasicChannelSelection : public TestWidget {

public:

    BasicChannelSelection ();
    ~BasicChannelSelection ();


    virtual boost::tuple <MeasurementSet *, Int, Bool> createMs ();
    virtual String name () const { return "BasicChannelSelection";}
    virtual void nextSubchunk (VisibilityIterator2 & /*vi*/, VisBuffer2 * /*vb*/);
    virtual Bool noMoreData (VisibilityIterator2 & /*vi*/, VisBuffer2 * /*vb*/, int nRows);
    virtual void startOfData (VisibilityIterator2 & /*vi*/, VisBuffer2 * /*vb*/);

protected:

    virtual int getUnderlyingCorrelation (Int spectralWindow, Int correlation);
    void setFactor (Int newFactor) { factor_p = newFactor;}
    void setCorrelationSlices (const Vector <Vector <Slice> > & slices);

private:


    void checkChannelAndFrequency (Int rowId, Int row, Int channel, Int channelIncrement, Int channelOffset,
                                   Int spectralWindow, const VisBuffer2 * vb);
    void checkFlagCube (Int rowId, Int spectralWindow, Int row, Int channel, Int correlation,
                        Int channelOffset, Int channelIncrement, VisBuffer2 * vb);
    void checkFlagCategory (Int rowId, Int spectralWindow, Int row, Int channel, Int correlation,
                            Int channelOffset, Int channelIncrement, VisBuffer2 * vb);
    void checkRowScalar (Double value, Double offset, Int rowId, const char * name, Int factor = 1);
    void checkRowScalars (VisBuffer2 * vb);
    void checkSigmaWeight (Int nCorrelations, const Matrix<Float> & values, Double offset, Int rowId,
                           Int row, const char * name, Int factor);
    void checkUvw (VisBuffer2 * vb, Int nRows, Int rowId, Int row);
    void checkVisCube (Int rowId, Int spectralWindow, Int row, Int channel, Int correlation,
                       const Cube<Complex> & cube, const String & tag,
                       Int channelOffset, Int channelIncrement, Int cubeDelta);
    void checkWeightSpectrum (Int rowId, Int spectralWindow, Int row, Int channel,
                              Int correlation, Int channelOffset, Int channelIncrement,
                              const VisBuffer2 * vb);

    Vector< Vector <Slice> > correlationSlices_p;
    Int factor_p;
    MsFactory * msf_p;
    const Int nAntennas_p;
    const Int nFlagCategories_p;
    Int nRowsToProcess_p;
    Int nSweeps_p;

};

class MultipleMss : public TestWidget {

public:

    MultipleMss ();

    virtual boost::tuple <Block<const MeasurementSet *>, Int, Bool> createMss ();

    virtual String name () const { return "MultipleMss";}
    virtual void endOfChunk (VisibilityIterator2 & /*vi*/, VisBuffer2 * /*vb*/) {}
    virtual void nextChunk (VisibilityIterator2 & /*vi*/, VisBuffer2 * /*vb*/) {}
    virtual void nextSubchunk (VisibilityIterator2 & /*vi*/, VisBuffer2 * /*vb*/);
    virtual Bool noMoreData (VisibilityIterator2 & /*vi*/, VisBuffer2 * /*vb*/, int /*nRows*/)
    { return False;}
    virtual void startOfData (VisibilityIterator2 & /*vi*/, VisBuffer2 * /*vb*/);
    virtual bool usesMultipleMss () const;

private:

    int nMss_p;

};

class FrequencyChannelSelection : public BasicChannelSelection {

public:

    FrequencyChannelSelection () {}

    virtual String name () const { return "FrequencyChannelSelection";}
    virtual void startOfData (VisibilityIterator2 & /*vi*/, VisBuffer2 * /*vb*/);
    Bool noMoreData (VisibilityIterator2 & /*vi*/, VisBuffer2 * /*vb*/, int nRowsProcessed);
};

class Weighting : public TestWidget {

public:

    Weighting (): TestWidget ("Weighting") {}

    virtual boost::tuple <MeasurementSet *, Int, Bool> createMs ();

    virtual String name () const { return "Weighting";}
    virtual void nextSubchunk (VisibilityIterator2 & /*vi*/, VisBuffer2 * /*vb*/);

private:

    MsFactory * msf_p;
    Int nRowsToProcess_p;
};

class BasicMutation : public BasicChannelSelection
{
public:

    BasicMutation ();
    ~BasicMutation ();

    virtual boost::tuple <MeasurementSet *, Int, Bool> createMs ();
    virtual String name () const { return "BasicMutation";}
    virtual void nextSubchunk (VisibilityIterator2 & /*vi*/, VisBuffer2 * /*vb*/);
    virtual Bool noMoreData (VisibilityIterator2 & /*vi*/, VisBuffer2 * /*vb*/, int nRows);

private:

    Bool firstPass_p;

};

class BasicCorrelationSelection : public BasicChannelSelection
{
public:

    BasicCorrelationSelection ();
    ~BasicCorrelationSelection ();

    virtual String name () const { return "BasicCorrelationSelection";}

protected:

private:

};


class Tester {
public:

    bool doTests (int nArgs, char * args []);

    std::pair<Bool, Bool> sweepMs (TestWidget & testWidget);

protected:

    typedef std::map<String, String> Arguments;

    Arguments parseArgs (int nArgs, char * args []) const;

private:

    Int nTestsAttempted_p;
    Int nTestsPassed_p;

    template <typename T> Bool doTest ();

};

class PerformanceComparator {

public:

    enum {Old=1, New=2, Both=3};

    PerformanceComparator (const String & ms);
    void compare (Int, Int, Int);

protected:

    void compareOne (ROVisibilityIterator * oldVi,
                     VisibilityIterator2 * newVi,
                     Int nSweeps,
                     Int tests);

    ROVisibilityIterator * createViOld ();
    VisibilityIterator2 * createViNew ();

    Double sweepViOld (ROVisibilityIterator & vi);
    Double sweepViNew (VisibilityIterator2 & vi);

private:

    String ms_p;

};

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

class CopyMs {

public:

    void doit (const String &);

protected:

    void copySubtables (MeasurementSet * newMs, const MeasurementSet * oldMs);
    void setupNewPointing(MeasurementSet * newMs);

};

void printMs (MeasurementSet * ms);


} // end namespace test
} // end namespace vi
} // end namespace casa

#endif // ! defined (VisibilityIterator_Test_H_20120912_1001
