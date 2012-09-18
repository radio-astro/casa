#if ! defined (VisibilityIterator_Test_H_20120912_1001)
#define VisibilityIterator_Test_H_20120912_1001

#include <casa/aips.h>
#include <casa/aipstype.h>
#include <casa/BasicSL/String.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <synthesis/MSVis/UtilJ.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ScalarColumn.h>

#include <map>
#include <set>

namespace casa {

    class MeasurementSet;
    class NewMSSimulator;
}

namespace casa {
namespace vi {
namespace test {

class FillState {

public:

    Int antenna1_p;
    Int antenna2_p;
    Int nAntennas_p;
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

class GenerateExposure : public Generator<Double> {

public:

    Double
    operator () (const FillState & fillState, Int, Int ) const {
        return fillState.timeDelta_p * .98;
    }
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

    Vector <Double>
    operator () (const FillState & fillState, Int, Int ) const {

        Vector <Double> result (3);
        result [0] = fillState.rowNumber_p * 10;
        result [1] = result [0] + 1;
        result [2] = result [0] + 2;
        return result;
    }
};


class MsFactory {

public:

    MsFactory (const String & name);

    ~MsFactory ();

    MeasurementSet * createMs ();

    void addAntennas (Int nAntennas);

    //// void addColumn (MSMainEnums::PredefinedColumns columnId);

    void addDataGenerator (MSMainEnums::PredefinedColumns, GeneratorBase * generator);

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

    void setTimeInfo (Double startingTime,
                      Double endingTime,
                      Double interval);

protected:

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

private:

    typedef std::set<MSMainEnums::PredefinedColumns> ColumnIds;

    class Columns {

    public:

        Columns & operator= (const Columns & other);

        ScalarColumn<Int>    antenna1_p;
        ScalarColumn<Int>    antenna2_p;
        ArrayColumn<Complex> corrVis_p;
        ScalarColumn<Double> exposure_p;
        ScalarColumn<Int>    feed1_p;
        ScalarColumn<Int>    feed2_p;
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
            generatorMap_p [MSMainEnums::EXPOSURE] = new GenerateExposure ();
            generatorMap_p [MSMainEnums::FEED1] = new GenerateConstant<Int> (0);
            generatorMap_p [MSMainEnums::FEED2] = new GenerateConstant<Int> (0);
            generatorMap_p [MSMainEnums::FLAG_ROW] = new GenerateConstant<Bool> (False);
            generatorMap_p [MSMainEnums::OBSERVATION_ID] = new GenerateConstant<Int> (0);
            generatorMap_p [MSMainEnums::PROCESSOR_ID] = new GenerateConstant<Int> (1);
            generatorMap_p [MSMainEnums::SCAN_NUMBER] = new GenerateConstant<Int> (1);
            generatorMap_p [MSMainEnums::STATE_ID] = new GenerateConstant<Int> (11);
            generatorMap_p [MSMainEnums::TIME_CENTROID] = new GenerateTimeCentroid ();
            generatorMap_p [MSMainEnums::INTERVAL] = new GenerateTimeInterval ();
            generatorMap_p [MSMainEnums::TIME] = new GenerateTime ();
            generatorMap_p [MSMainEnums::FLAG] = new GenerateConstant<Bool> (False);
            generatorMap_p [MSMainEnums::SIGMA] = new GenerateConstant<Float> (0);
            generatorMap_p [MSMainEnums::SIGMA_SPECTRUM] = new GenerateConstant<Float> (0);
            generatorMap_p [MSMainEnums::UVW] = new GenerateUvw ();
            generatorMap_p [MSMainEnums::CORRECTED_DATA] = new GenerateConstant<Complex> (Complex (0));
            generatorMap_p [MSMainEnums::MODEL_DATA] = new GenerateConstant<Complex> (Complex (0));
            generatorMap_p [MSMainEnums::DATA] = new GenerateConstant<Complex> (Complex (0));
            generatorMap_p [MSMainEnums::WEIGHT] = new GenerateConstant<Float> (1);
            generatorMap_p [MSMainEnums::WEIGHT_SPECTRUM] = new GenerateConstant<Float> (1);

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
    NewMSSimulator * simulator_p;
    Double timeEnd_p;
    Double timeInterval_p;
    Double timeStart_p;
};

} // end namespace casa
} // end namespace vi
} // end namespace test

#endif // ! defined (VisibilityIterator_Test_H_20120912_1001
