#if ! defined (VisibilityIterator_Test_H_20120912_1001)
#define VisibilityIterator_Test_H_20120912_1001

#include <casa/aips.h>
#include <casa/aipstype.h>
#include <casa/BasicSL/String.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <msvis/MSVis/UtilJ.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ScalarColumn.h>
#include <tuple>

#include <map>
#include <set>

namespace casacore{

    class MeasurementSet;
    class NewMSSimulator;
}

namespace casa {

    class ROVisibilityIterator;


    namespace vi {

        class VisibilityIterator2;
        class VisBuffer2;

    }

namespace vi {
namespace test {

class MsFactory;

class TestWidget {

public:

    TestWidget (const casacore::String & name) : name_p (name) {}

    virtual ~TestWidget () {}

    virtual casacore::String name () const = 0;

    virtual std::tuple <casacore::MeasurementSet *, casacore::Int, casacore::Bool> createMs (){
        return std::make_tuple ((casacore::MeasurementSet *) 0, 0, false);
        // Useless placeholder implementation
    }

    virtual std::tuple <casacore::Block<const casacore::MeasurementSet *>, casacore::Int, casacore::Bool> createMss (){
        return std::make_tuple (casacore::Block<const casacore::MeasurementSet *> (), 0, false);
        // Useless placeholder implementation
    }

    virtual void endOfChunk (casa::vi::VisibilityIterator2 & /*vi*/, casa::vi::VisBuffer2 * /*vb*/) {}
    virtual void nextChunk (casa::vi::VisibilityIterator2 & /*vi*/, casa::vi::VisBuffer2 * /*vb*/) {}
    virtual void nextSubchunk (casa::vi::VisibilityIterator2 & /*vi*/, casa::vi::VisBuffer2 * /*vb*/) {}
    virtual casacore::Bool noMoreData (casa::vi::VisibilityIterator2 & /*vi*/, casa::vi::VisBuffer2 * /*vb*/, int /*nRows*/)
    { return false;}
    virtual void startOfData (casa::vi::VisibilityIterator2 & /*vi*/, casa::vi::VisBuffer2 * /*vb*/) {}
    virtual bool usesMultipleMss () const { return false;}

private:

    casacore::String name_p;
};

class BasicChannelSelection : public TestWidget {

public:

    BasicChannelSelection ();
    ~BasicChannelSelection ();


    virtual std::tuple <casacore::MeasurementSet *, casacore::Int, casacore::Bool> createMs ();
    virtual casacore::String name () const { return "BasicChannelSelection";}
    virtual void nextSubchunk (casa::vi::VisibilityIterator2 & /*vi*/, casa::vi::VisBuffer2 * /*vb*/);
    virtual casacore::Bool noMoreData (casa::vi::VisibilityIterator2 & /*vi*/, casa::vi::VisBuffer2 * /*vb*/, int nRows);
    virtual void startOfData (casa::vi::VisibilityIterator2 & /*vi*/, casa::vi::VisBuffer2 * /*vb*/);

protected:

    virtual int getUnderlyingCorrelation (casacore::Int spectralWindow, casacore::Int correlation);
    void setFactor (casacore::Int newFactor) { factor_p = newFactor;}
    void setCorrelationSlices (const casacore::Vector <casacore::Vector <casacore::Slice> > & slices);

private:


    void checkChannelAndFrequency (casacore::Int rowId, casacore::Int row, casacore::Int channel, casacore::Int channelIncrement, casacore::Int channelOffset,
                                   casacore::Int spectralWindow, const casa::vi::VisBuffer2 * vb);
    void checkCorrelations (casacore::Int spectralWindow, casa::vi::VisBuffer2 * vb);
    void checkFlagCube (casacore::Int rowId, casacore::Int spectralWindow, casacore::Int row, casacore::Int channel, casacore::Int correlation,
                        casacore::Int channelOffset, casacore::Int channelIncrement, casa::vi::VisBuffer2 * vb);
    void checkFlagCategory (casacore::Int rowId, casacore::Int spectralWindow, casacore::Int row, casacore::Int channel, casacore::Int correlation,
                            casacore::Int channelOffset, casacore::Int channelIncrement, casa::vi::VisBuffer2 * vb);
    void checkRowScalar (casacore::Double value, casacore::Double offset, casacore::Int rowId, const char * name, casacore::Int factor = 1);
    void checkRowScalars (casa::vi::VisBuffer2 * vb);
    void checkSigmaWeight (casacore::Int nCorrelations, const casacore::Matrix<casacore::Float> & values, casacore::Double offset, casacore::Int rowId,
                           casacore::Int row, const char * name, casacore::Int factor);
    void checkUvw (casa::vi::VisBuffer2 * vb, casacore::Int nRows, casacore::Int rowId, casacore::Int row);
    void checkVisCube (casacore::Int rowId, casacore::Int spectralWindow, casacore::Int row, casacore::Int channel, casacore::Int correlation,
                       const casacore::Cube<casacore::Complex> & cube, const casacore::String & tag,
                       casacore::Int channelOffset, casacore::Int channelIncrement, casacore::Int cubeDelta);
    void checkWeightSpectrum (casacore::Int rowId, casacore::Int spectralWindow, casacore::Int row, casacore::Int channel,
                              casacore::Int correlation, casacore::Int channelOffset, casacore::Int channelIncrement,
                              const casa::vi::VisBuffer2 * vb);

    casacore::Vector< casacore::Vector <casacore::Slice> > correlationSlices_p;
    casacore::Int factor_p;
    MsFactory * msf_p;
    const casacore::Int nAntennas_p;
    const casacore::Int nFlagCategories_p;
    casacore::Int nRowsToProcess_p;
    casacore::Int nSweeps_p;

};

class MultipleMss : public TestWidget {

public:

    MultipleMss ();

    virtual std::tuple <casacore::Block<const casacore::MeasurementSet *>, casacore::Int, casacore::Bool> createMss ();

    virtual casacore::String name () const { return "MultipleMss";}
    virtual void endOfChunk (casa::vi::VisibilityIterator2 & /*vi*/, casa::vi::VisBuffer2 * /*vb*/) {}
    virtual void nextChunk (casa::vi::VisibilityIterator2 & /*vi*/, casa::vi::VisBuffer2 * /*vb*/) {}
    virtual void nextSubchunk (casa::vi::VisibilityIterator2 & /*vi*/, casa::vi::VisBuffer2 * /*vb*/);
    virtual casacore::Bool noMoreData (casa::vi::VisibilityIterator2 & /*vi*/, casa::vi::VisBuffer2 * /*vb*/, int /*nRows*/)
    { return false;}
    virtual void startOfData (casa::vi::VisibilityIterator2 & /*vi*/, casa::vi::VisBuffer2 * /*vb*/);
    virtual bool usesMultipleMss () const;

private:

    int nMss_p;

};

class FrequencyChannelSelection : public BasicChannelSelection {

public:

    FrequencyChannelSelection () {}

    virtual casacore::String name () const { return "FrequencyChannelSelection";}
    virtual void startOfData (casa::vi::VisibilityIterator2 & /*vi*/, casa::vi::VisBuffer2 * /*vb*/);
    casacore::Bool noMoreData (casa::vi::VisibilityIterator2 & /*vi*/, casa::vi::VisBuffer2 * /*vb*/, int nRowsProcessed);
};

class Weighting : public TestWidget {

public:

    Weighting (): TestWidget ("Weighting") {}

    virtual std::tuple <casacore::MeasurementSet *, casacore::Int, casacore::Bool> createMs ();

    virtual casacore::String name () const { return "Weighting";}
    virtual void nextSubchunk (casa::vi::VisibilityIterator2 & /*vi*/, casa::vi::VisBuffer2 * /*vb*/);

private:

    MsFactory * msf_p;
    casacore::Int nRowsToProcess_p;
};

class BasicMutation : public BasicChannelSelection
{
public:

    BasicMutation ();
    ~BasicMutation ();

    virtual std::tuple <casacore::MeasurementSet *, casacore::Int, casacore::Bool> createMs ();
    virtual casacore::String name () const { return "BasicMutation";}
    virtual void nextSubchunk (casa::vi::VisibilityIterator2 & /*vi*/, casa::vi::VisBuffer2 * /*vb*/);
    virtual casacore::Bool noMoreData (casa::vi::VisibilityIterator2 & /*vi*/, casa::vi::VisBuffer2 * /*vb*/, int nRows);

private:

    casacore::Bool firstPass_p;

};

class BasicCorrelationSelection : public BasicChannelSelection
{
public:

    BasicCorrelationSelection ();
    ~BasicCorrelationSelection ();

    virtual casacore::String name () const { return "BasicCorrelationSelection";}

protected:

private:

};


class Tester {
public:

    bool doTests (int nArgs, char * args []);

    std::pair<casacore::Bool, casacore::Bool> sweepMs (TestWidget & testWidget);

protected:

    typedef std::map<casacore::String, casacore::String> Arguments;

    Arguments parseArgs (int nArgs, char * args []) const;

private:

    casacore::Int nTestsAttempted_p;
    casacore::Int nTestsPassed_p;

    template <typename T> casacore::Bool doTest ();

};

class PerformanceComparator {

public:

    enum {Old=1, New=2, Both=3};

    PerformanceComparator (const casacore::String & ms);
    void compare (casacore::Int, casacore::Int, casacore::Int);

protected:

    void compareOne (casa::ROVisibilityIterator * oldVi,
                     casa::vi::VisibilityIterator2 * newVi,
                     casacore::Int nSweeps,
                     casacore::Int tests);

    casa::ROVisibilityIterator * createViOld ();
    casa::vi::VisibilityIterator2 * createViNew ();

    casacore::Double sweepViOld (casa::ROVisibilityIterator & vi);
    casacore::Double sweepViNew (casa::vi::VisibilityIterator2 & vi);

private:

    casacore::String ms_p;

};

#define TestErrorIf(c,m) {if (c) ThrowTestError (m);}

#define ThrowTestError(m) \
    throw TestError (m, __func__, __FILE__, __LINE__);

class TestError : public std::exception {

public:

    TestError (const casacore::String & message, const casacore::String & function,
               const casacore::String & file, int lineNumber)
    : file_p (file),
      function_p (function),
      lineNumber_p (lineNumber),
      message_p (message)
    {}

    ~TestError () throw () {}

    const char * what () const throw () {
        what_p = casacore::String::format ("TestError: %s (in %s at %s:%d)", message_p.c_str(),
                                 function_p.c_str(), file_p.c_str(), lineNumber_p);
        return what_p.c_str();
    }

private:

    casacore::String file_p;
    casacore::String function_p;
    casacore::Int lineNumber_p;
    casacore::String message_p;
    mutable casacore::String what_p;

};

class CopyMs {

public:

    void doit (const casacore::String &);

protected:

    void copySubtables (casacore::MeasurementSet * newMs, const casacore::MeasurementSet * oldMs);
    void setupNewPointing(casacore::MeasurementSet * newMs);

};

void printMs (casacore::MeasurementSet * ms);


} // end namespace test
} // end namespace vi
} // end namespace casa

#endif // ! defined (VisibilityIterator_Test_H_20120912_1001
