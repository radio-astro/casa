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

class FillState {

public:

    Int antenna1_p;
    Int antenna2_p;
    Int nAntennas_p;
    Int nFlagCategories_p;
    Int nChannels_p;
    Int nCorrelations_p;
    Int rowNumber_p;
    Int spectralWindow_p;
    Double time_p;
    Double timeDelta_p;
    Double uvw_p [3];

};

class GeneratorBase {

public:

    virtual ~GeneratorBase () {}
};

template <typename T>
class Generator : public GeneratorBase {
public:

    virtual ~Generator () {}

    virtual T operator() (const FillState &, Int channel, Int correlation) const = 0;

};

template <typename T>
class GenerateConstant : public Generator<T>{

public:

    GenerateConstant (const T & c) : c_p (c) {}

    T
    operator () (const FillState &, Int, Int ) const {
        return c_p;
    }

private:

    T c_p;
};

class GenerateAntenna1 : public Generator<Int> {

public:

    Int
    operator () (const FillState & fillState, Int, Int ) const {
        return fillState.antenna1_p;
    }
};

class GenerateAntenna2 : public Generator<Int> {


public:

    Int
    operator () (const FillState & fillState, Int, Int ) const {
        return fillState.antenna2_p;
    }
};

class GenerateDdi: public Generator<Int> {

public:

    Int
    operator () (const FillState & fillState, Int, Int ) const {
        return fillState.spectralWindow_p;
    }
};

class GenerateExposure : public Generator<Double> {

public:

    Double
    operator () (const FillState & fillState, Int, Int ) const {
        return fillState.timeDelta_p * .98;
    }
};

class GenerateFlag : public Generator<Bool> {

public:

    Bool
    operator () (const FillState & /*fillState*/, Int channel, Int correlation) const {

        // Generate a of T F T F ... for even rows and F T F T ... for odd rows

        Bool value = (channel % 2 == 0) == (correlation % 2 == 0);
        return value;
    }
};

class GenerateFlagRow : public Generator<Bool> {

public:

    Bool
    operator () (const FillState & fillState, Int /*channel*/, Int /*correlation*/) const {

        // Generate a of T F T F ... for even rows and F T F T ... for odd rows

        Bool value = (fillState.rowNumber_p % 3 == 0) || (fillState.rowNumber_p % 5 == 0);
        return value;
    }
};

class GenerateFlagCategory : public Generator <Vector <Bool> > {

    Vector<Bool>
    operator () (const FillState & fillState, Int channel, Int correlation) const {

        Vector <Bool> result (fillState.nFlagCategories_p);

        result [0] = (fillState.rowNumber_p % 2) ^ (channel % 2) ^ (correlation % 2);

        for (int i = 1; i < fillState.nFlagCategories_p; i ++){

            result [i] = ! result [i - 1];
        }

        return result;
    }
};


template <typename T>
class GenerateUsingRow : public Generator<T> {


public:

    GenerateUsingRow (T c) : c_p (c) {}

    T
    operator () (const FillState & fillState, Int, Int ) const {
        return fillState.rowNumber_p * 100 + c_p;
    }

private:

    const T c_p;
};


class GenerateTime : public Generator<Double> {


public:

    Double
    operator () (const FillState & fillState, Int, Int ) const {
        return fillState.time_p;
    }
};

class GenerateTimeCentroid : public Generator<Double> {

public:

    Double
    operator () (const FillState & fillState, Int, Int ) const {
        return fillState.time_p + fillState.timeDelta_p * .01;
    }
};

class GenerateTimeInterval : public Generator<Double> {

public:

    Double
    operator () (const FillState & fillState, Int, Int ) const {
        return fillState.timeDelta_p;
    }
};

class GenerateUvw : public Generator <Vector <Double> > {

    Vector<Double>
    operator () (const FillState & fillState, Int, Int ) const {

        Vector <Double> result (3);
        result [0] = fillState.rowNumber_p * 10;
        result [1] = result [0] + 1;
        result [2] = result [0] + 2;
        return result;
    }
};

class GenerateVisibility : public Generator<Complex> {

public:

    GenerateVisibility (Int c) : c_p (c) {}

    Complex
    operator () (const FillState & fillState, Int channel, Int correlation) const {

        Float r = fillState.rowNumber_p * 10 + fillState.spectralWindow_p;
        Float i = channel * 100 + correlation * 10 + c_p;
        return Complex (r, i);
    }

private:

    const Int c_p;
};


class GenerateWeightSpectrum : public Generator<Float> {

public:

    Float
    operator () (const FillState & fillState, Int channel, Int correlation) const {

        Float r = fillState.rowNumber_p * 1000 + fillState.spectralWindow_p * 100 +
                  channel * 10 + correlation;
        return r;
    }
};




class MsFactory {

public:

    MsFactory (const String & name);

    ~MsFactory ();

    std::pair<MeasurementSet *, Int> createMs ();

    void addAntennas (Int nAntennas);

    //// void addColumn (MSMainEnums::PredefinedColumns columnId);


    void addFeed (const String & name,
                  const Double x,
                  const Double y,
                  const String & polarization);
    void addField (const String & name,
                   const MDirection & direction);
    void addSpectralWindow (const String & name,
                            Int nChannels,
                            Double frequency,
                            Double frequencyDelta,
                            const String & stokes);
    void addSpectralWindows (int nSpectralWindows);

    void setDataGenerator (MSMainEnums::PredefinedColumns, GeneratorBase * generator);

    void setTimeInfo (Double startingTime,
                      Double endingTime,
                      Double interval);

protected:

    void addColumns ();
    void addDefaults ();
    void attachColumns ();
    void fillCollections (FillState & fillState);
    template <typename T>
    void
    fillCube (ArrayColumn<T> & column, const FillState & fillState,
              const GeneratorBase * generator);
    void fillCubes (FillState & fillState);
    void fillData ();
    void fillRows (FillState & fillState);

    template <typename T>
    void
    fillScalar (ScalarColumn<T> & column, const FillState & fillState,
                const GeneratorBase * generator);
    template <typename T>
    void
    fillCollection (ArrayColumn<T> & column, const FillState & fillState,
                    const GeneratorBase * generator);
    void fillScalars (FillState & fillState);
    void fillVisCubeCorrected (FillState & fillState);
    void fillVisCubeModel (FillState & fillState);
    void fillVisCubeObserved (FillState & fillState);
    void fillWeight (FillState & fillState);
    void fillSigma (FillState & fillState);
    void fillUvw (FillState & fillState);
    void fillWeightSpectrumCube (FillState & fillState);
    void fillFlagCube (FillState & fillState);
    void fillFlagCategories (const FillState & fillState);

private:

    typedef std::set<MSMainEnums::PredefinedColumns> ColumnIds;

    class Columns {

    public:

        Columns & operator= (const Columns & other);

        ScalarColumn<Int>    antenna1_p;
        ScalarColumn<Int>    antenna2_p;
        ArrayColumn<Complex> corrVis_p;
        ScalarColumn<Int>    dataDescriptionId_p;
        ScalarColumn<Double> exposure_p;
        ScalarColumn<Int>    feed1_p;
        ScalarColumn<Int>    feed2_p;
        ScalarColumn<Int>    field_p;
        ArrayColumn<Bool>    flagCategory_p;
        ScalarColumn<Bool>   flagRow_p;
        ArrayColumn<Bool>    flag_p;
        ArrayColumn<Float>   floatVis_p;
        ArrayColumn<Complex> modelVis_p;
        ScalarColumn<Int>    observation_p;
        ScalarColumn<Int>    processor_p;
        ScalarColumn<Int>    scan_p;
        ArrayColumn<Float>   sigma_p;
        ScalarColumn<Int>    state_p;
        ScalarColumn<Double> timeCentroid_p;
        ScalarColumn<Double> timeInterval_p;
        ScalarColumn<Double> time_p;
        ArrayColumn<Double>  uvw_p;
        ArrayColumn<Complex> vis_p;
        ArrayColumn<Float>   weightSpectrum_p;
        ArrayColumn<Float>   weight_p;

    };

    class Generators {

    public:

        typedef std::map<MSMainEnums::PredefinedColumns, const GeneratorBase *> GeneratorMap;

        Generators ()
        {
            generatorMap_p [MSMainEnums::ANTENNA1] = new GenerateAntenna1 ();
            generatorMap_p [MSMainEnums::ANTENNA2] = new GenerateAntenna2 ();
            generatorMap_p [MSMainEnums::DATA_DESC_ID] = new GenerateDdi ();
            generatorMap_p [MSMainEnums::EXPOSURE] = new GenerateUsingRow<Double> (8);
            generatorMap_p [MSMainEnums::FEED1] = new GenerateConstant<Int> (0);
            generatorMap_p [MSMainEnums::FEED2] = new GenerateConstant<Int> (0);
            generatorMap_p [MSMainEnums::FIELD_ID] = new GenerateConstant<Int> (0);
            generatorMap_p [MSMainEnums::FLAG_CATEGORY] = new GenerateFlagCategory ();
            generatorMap_p [MSMainEnums::FLAG_ROW] = new GenerateFlagRow ();
            generatorMap_p [MSMainEnums::OBSERVATION_ID] = new GenerateUsingRow<Int> (7);
            generatorMap_p [MSMainEnums::PROCESSOR_ID] = new GenerateConstant<Int> (0);
            generatorMap_p [MSMainEnums::SCAN_NUMBER] = new GenerateUsingRow<Int> (5);
            generatorMap_p [MSMainEnums::STATE_ID] = new GenerateConstant<Int> (0);
            generatorMap_p [MSMainEnums::TIME_CENTROID] = new GenerateUsingRow<Double> (1);
            generatorMap_p [MSMainEnums::INTERVAL] = new GenerateUsingRow<Double> (2);
            generatorMap_p [MSMainEnums::TIME] = new GenerateTime ();
            generatorMap_p [MSMainEnums::FLAG] = new GenerateFlag ();
            generatorMap_p [MSMainEnums::SIGMA] = new GenerateUsingRow<Float> (3);
            generatorMap_p [MSMainEnums::SIGMA_SPECTRUM] = new GenerateConstant<Float> (0);
            generatorMap_p [MSMainEnums::UVW] = new GenerateUvw ();
            generatorMap_p [MSMainEnums::CORRECTED_DATA] = new GenerateVisibility (1);
            generatorMap_p [MSMainEnums::MODEL_DATA] = new GenerateVisibility (2);
            generatorMap_p [MSMainEnums::DATA] = new GenerateVisibility (0);
            generatorMap_p [MSMainEnums::WEIGHT] = new GenerateUsingRow<Float> (4);
            generatorMap_p [MSMainEnums::WEIGHT_SPECTRUM] = new GenerateWeightSpectrum ();

        }

        ~Generators ()
        {
            for (GeneratorMap::const_iterator i = generatorMap_p.begin();
                 i != generatorMap_p.end();
                 i ++){

                delete i->second;
            }
        }

        const GeneratorBase *
        get (MSMainEnums::PredefinedColumns key) const
        {
            GeneratorMap::const_iterator i = generatorMap_p.find (key);

            ThrowIf (i == generatorMap_p.end(), "No such key");

            return i->second;
        }

        void
        set (MSMainEnums::PredefinedColumns key, const GeneratorBase * newValue)
        {
            GeneratorMap::iterator i = generatorMap_p.find (key);

            if (i != generatorMap_p.end()){
                delete i->second;
            }

            i->second =  newValue;
        }

    private:

        GeneratorMap generatorMap_p;

    };

    Columns columns_p;
    ColumnIds columnIds_p;
    Generators generators_p;
    MeasurementSet * ms_p;
    Int nAntennas_p;
    Int nRows_p;
    NewMSSimulator * simulator_p;
    Double timeEnd_p;
    Double timeInterval_p;
    Double timeStart_p;
};

class TestWidget {

public:

    TestWidget (const String & name) : name_p (name) {}

    virtual ~TestWidget () {}

    virtual String name () const = 0;

    virtual boost::tuple <MeasurementSet *, Int, Bool> createMs () = 0;

    virtual void endOfChunk (VisibilityIterator2 & /*vi*/, VisBuffer2 * /*vb*/) {}
    virtual void nextChunk (VisibilityIterator2 & /*vi*/, VisBuffer2 * /*vb*/) {}
    virtual void nextSubchunk (VisibilityIterator2 & /*vi*/, VisBuffer2 * /*vb*/) {}
    virtual Bool noMoreData (VisibilityIterator2 & /*vi*/, VisBuffer2 * /*vb*/, int /*nRows*/)
    { return False;}
    virtual void startOfData (VisibilityIterator2 & /*vi*/, VisBuffer2 * /*vb*/) {}

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

    void setFactor (Int newFactor) { factor_p = newFactor;}

private:


    void checkChannelAndFrequency (Int rowId, Int row, Int channel, Int channelIncrement, Int channelOffset,
                                   Int spectralWindow, const VisBuffer2 * vb);
    void checkFlagCube (Int rowId, Int spectralWindow, Int row, Int channel, Int correlation,
                        Int channelOffset, Int channelIncrement, VisBuffer2 * vb);
    void checkRowScalar (Double value, Double offset, Int rowId, const char * name, Int factor = 1);
    void checkRowScalars (VisBuffer2 * vb);
    void checkVisCube (Int rowId, Int spectralWindow, Int row, Int channel, Int correlation,
                       const Cube<Complex> & cube, const String & tag,
                       Int channelOffset, Int channelIncrement, Int cubeDelta);
    void checkWeightSpectrum (Int rowId, Int spectralWindow, Int row, Int channel,
                              Int correlation, Int channelOffset, Int channelIncrement,
                              const VisBuffer2 * vb);

    Int factor_p;
    MsFactory * msf_p;
    const Int nAntennas_p;
    const Int nFlagCategories_p;
    Int nSweeps_p;

};

class FrequencyChannelSelection : public BasicChannelSelection {

public:

    FrequencyChannelSelection () {}

    virtual String name () const { return "FrequencyChannelSelection";}
    virtual void startOfData (VisibilityIterator2 & /*vi*/, VisBuffer2 * /*vb*/);
    Bool noMoreData (VisibilityIterator2 & /*vi*/, VisBuffer2 * /*vb*/, int nRowsProcessed);
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
