#if ! defined (CASA_VI_TEST_MsFactory_H_130114_1357)
#define CASA_VI_TEST_MsFactory_H_130114_1357

#include <casa/string.h>
#include <ms/MeasurementSets.h>
#include <msvis/MSVis/UtilJ.h>

#include <utility>

namespace casa {

    class MeasurementSet;
    class NewMSSimulator;
    class ROVisibilityIterator;

namespace vi {

    class VisibilityIterator2;
    class VisBuffer2;

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

class GenerateFlagCategory : public GeneratorBase {

public:

    virtual Bool
    operator () (const FillState & fillState, Int channel, Int correlation, Int /*category*/) const {

        Bool result;

        switch (correlation){

        case 0:

            result = fillState.rowNumber_p & 0x1;
            break;

        case 1:

            result = fillState.rowNumber_p & 0x2;
            break;

        case 2:

            result = channel & 0x1;
            break;

        case 3:

            result = channel & 0x2;
            break;

        default:

            Assert (False);
            break;

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
        return fillState.time_p; // same as time for this model
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
                  channel * 10 + correlation + 1;
        return r;
    }
};

class GenerateWeightSpectrumCorrected : public Generator<Float> {

public:

    Float
    operator () (const FillState & fillState, Int channel, Int correlation) const {

        Float r = fillState.rowNumber_p * 1000 + fillState.spectralWindow_p * 100 +
                  channel * 10 + correlation + 2;
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
    void addWeightSpectrum (Bool addIt);

    void addCubeColumn (MSMainEnums::PredefinedColumns columnId,
                        const String & dataStorageMangerName);


    void addFeeds (Int nFeeds);
    void addField (const String & name,
                   const MDirection & direction);
    void addSpectralWindow (const String & name,
                            Int nChannels,
                            Double frequency,
                            Double frequencyDelta,
                            const String & stokes);
    void addSpectralWindows (int nSpectralWindows);

    void setDataGenerator (MSMainEnums::PredefinedColumns, GeneratorBase * generator);

    void setIncludeAutocorrelations (Bool includeThem);

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
    void fillWeightSpectrumCorrectedCube (FillState & fillState);
    void fillFlagCube (FillState & fillState);
    void fillFlagCategories (const FillState & fillState);

private:

    typedef std::set<MSMainEnums::PredefinedColumns> ColumnIds;

    class Columns {

    public:

        Columns & operator= (const Columns & other);

        ScalarColumn<Int>    antenna1_p;
        ScalarColumn<Int>    antenna2_p;
        ScalarColumn<Int>    array_p;
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
        ArrayColumn<Float>   weightSpectrumCorrected_p;
        ArrayColumn<Float>   weight_p;

    };

    class Generators {

    public:

        typedef std::map<MSMainEnums::PredefinedColumns, const GeneratorBase *> GeneratorMap;

        Generators ()
        {
            generatorMap_p [MSMainEnums::ANTENNA1] = new GenerateAntenna1 ();
            generatorMap_p [MSMainEnums::ANTENNA2] = new GenerateAntenna2 ();
            generatorMap_p [MSMainEnums::ARRAY_ID] = new GenerateConstant<Int> (17);
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
            generatorMap_p [MSMainEnums::CORRECTED_WEIGHT_SPECTRUM] = new GenerateWeightSpectrumCorrected ();

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

            ThrowIf (i == generatorMap_p.end(),
                     String::format ("No such key: %d", key));

            return i->second;
        }

        void
        set (MSMainEnums::PredefinedColumns key, const GeneratorBase * newValue)
        {
            GeneratorMap::iterator i = generatorMap_p.find (key);

            if (i != generatorMap_p.end()){
                delete i->second;
                i->second =  newValue;
            }
            else{
                generatorMap_p [key] = newValue;
            }

        }

    private:

        GeneratorMap generatorMap_p;

    };

    Bool addWeightSpectrum_p;
    Columns columns_p;
    ColumnIds columnIds_p;
    Generators generators_p;
    Bool includeAutocorrelations_p;
    MeasurementSet * ms_p;
    Int nAntennas_p;
    Int nRows_p;
    NewMSSimulator * simulator_p;
    Double timeEnd_p;
    Double timeInterval_p;
    Double timeStart_p;
};

} // end namespace test

} // end namespace vi

} // end namespace casa

#endif // ! defined (CASA_VI_TEST_MsFactory_H_130114_1357)
