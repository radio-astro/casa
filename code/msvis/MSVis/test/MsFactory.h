#if ! defined (CASA_VI_TEST_MsFactory_H_130114_1357)
#define CASA_VI_TEST_MsFactory_H_130114_1357

#include <casa/string.h>
#include <ms/MeasurementSets.h>
#include <msvis/MSVis/UtilJ.h>

#include <utility>

namespace casacore{

    class MeasurementSet;
    class NewMSSimulator;
}

namespace casa {

    class ROVisibilityIterator;

namespace vi {

    class VisibilityIterator2;
    class VisBuffer2;

namespace test {

class FillState {

public:

    casacore::Int antenna1_p;
    casacore::Int antenna2_p;
    casacore::Int nAntennas_p;
    casacore::Int nFlagCategories_p;
    casacore::Int nChannels_p;
    casacore::Int nCorrelations_p;
    casacore::Int rowNumber_p;
    casacore::Int spectralWindow_p;
    casacore::Double time_p;
    casacore::Double timeDelta_p;
    casacore::Double uvw_p [3];

};


class GeneratorBase {

public:

    virtual ~GeneratorBase () {}
};

template <typename T>
class Generator : public GeneratorBase {
public:

    virtual ~Generator () {}

    virtual T operator() (const FillState &, casacore::Int channel, casacore::Int correlation) const = 0;

};

template <typename T>
class GenerateConstant : public Generator<T>{

public:

    GenerateConstant (const T & c) : c_p (c) {}

    T
    operator () (const FillState &, casacore::Int, casacore::Int ) const {
        return c_p;
    }

private:

    T c_p;
};

class GenerateAntenna1 : public Generator<casacore::Int> {

public:

    Int
    operator () (const FillState & fillState, casacore::Int, casacore::Int ) const {
        return fillState.antenna1_p;
    }
};

class GenerateAntenna2 : public Generator<casacore::Int> {


public:

    Int
    operator () (const FillState & fillState, casacore::Int, casacore::Int ) const {
        return fillState.antenna2_p;
    }
};

class GenerateDdi: public Generator<casacore::Int> {

public:

    Int
    operator () (const FillState & fillState, casacore::Int, casacore::Int ) const {
        return fillState.spectralWindow_p;
    }
};

class GenerateExposure : public Generator<casacore::Double> {

public:

    Double
    operator () (const FillState & fillState, casacore::Int, casacore::Int ) const {
        return fillState.timeDelta_p * .98;
    }
};

class GenerateFlag : public Generator<casacore::Bool> {

public:

    Bool
    operator () (const FillState & /*fillState*/, casacore::Int channel, casacore::Int correlation) const {

        // Generate a of T F T F ... for even rows and F T F T ... for odd rows

        casacore::Bool value = (channel % 2 == 0) == (correlation % 2 == 0);
        return value;
    }
};

class GenerateFlagRow : public Generator<casacore::Bool> {

public:

    Bool
    operator () (const FillState & fillState, casacore::Int /*channel*/, casacore::Int /*correlation*/) const {

        // Generate a of T F T F ... for even rows and F T F T ... for odd rows

        casacore::Bool value = (fillState.rowNumber_p % 3 == 0) || (fillState.rowNumber_p % 5 == 0);
        return value;
    }
};

class GenerateFlagCategory : public GeneratorBase {

public:

    virtual Bool
    operator () (const FillState & fillState, casacore::Int channel, casacore::Int correlation, casacore::Int /*category*/) const {

        casacore::Bool result;

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

            Assert (false);
            result = false;
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
    operator () (const FillState & fillState, casacore::Int, casacore::Int ) const {
        return fillState.rowNumber_p * 100 + c_p;
    }

private:

    const T c_p;
};


class GenerateTime : public Generator<casacore::Double> {


public:

    Double
    operator () (const FillState & fillState, casacore::Int, casacore::Int ) const {
        return fillState.time_p;
    }
};

class GenerateTimeCentroid : public Generator<casacore::Double> {

public:

    Double
    operator () (const FillState & fillState, casacore::Int, casacore::Int ) const {
        return fillState.time_p; // same as time for this model
    }
};

class GenerateTimeInterval : public Generator<casacore::Double> {

public:

    Double
    operator () (const FillState & fillState, casacore::Int, casacore::Int ) const {
        return fillState.timeDelta_p;
    }
};

class GenerateUvw : public Generator <casacore::Vector <casacore::Double> > {

    casacore::Vector<casacore::Double>
    operator () (const FillState & fillState, casacore::Int, casacore::Int ) const {

        casacore::Vector <casacore::Double> result (3);
        result [0] = fillState.rowNumber_p * 10;
        result [1] = result [0] + 1;
        result [2] = result [0] + 2;
        return result;
    }
};

class GenerateVisibility : public Generator<casacore::Complex> {

public:

    GenerateVisibility (casacore::Int c) : c_p (c) {}

    Complex
    operator () (const FillState & fillState, casacore::Int channel, casacore::Int correlation) const {

        casacore::Float r = fillState.rowNumber_p * 10 + fillState.spectralWindow_p;
        casacore::Float i = channel * 100 + correlation * 10 + c_p;
        return casacore::Complex (r, i);
    }

private:

    const casacore::Int c_p;
};


class GenerateWeightSpectrum : public Generator<casacore::Float> {

public:

    Float
    operator () (const FillState & fillState, casacore::Int channel, casacore::Int correlation) const {

        casacore::Float r = fillState.rowNumber_p * 1000 + fillState.spectralWindow_p * 100 +
                  channel * 10 + correlation + 1;
        return r;
    }
};

class GenerateWeightSpectrumCorrected : public Generator<casacore::Float> {

public:

    Float
    operator () (const FillState & fillState, casacore::Int channel, casacore::Int correlation) const {

        casacore::Float r = fillState.rowNumber_p * 1000 + fillState.spectralWindow_p * 100 +
                  channel * 10 + correlation + 2;
        return r;
    }
};


class MsFactory {

public:

    MsFactory (const casacore::String & name);

    ~MsFactory ();

    std::pair<casacore::MeasurementSet *, casacore::Int> createMs ();

    void addAntennas (casacore::Int nAntennas);

    //// void addColumn (casacore::MSMainEnums::PredefinedColumns columnId);
    void addWeightSpectrum (casacore::Bool addIt);

    void addCubeColumn (casacore::MSMainEnums::PredefinedColumns columnId,
                        const casacore::String & dataStorageMangerName);


    void addFeeds (casacore::Int nFeeds);
    void addField (const casacore::String & name,
                   const casacore::MDirection & direction);
    void addSpectralWindow (const casacore::String & name,
                            casacore::Int nChannels,
                            casacore::Double frequency,
                            casacore::Double frequencyDelta,
                            const casacore::String & stokes);
    void addSpectralWindows (int nSpectralWindows);

    void setDataGenerator (casacore::MSMainEnums::PredefinedColumns, GeneratorBase * generator);

    void setIncludeAutocorrelations (casacore::Bool includeThem);

    void setTimeInfo (casacore::Double startingTime,
                      casacore::Double endingTime,
                      casacore::Double interval);

protected:

    void addColumns ();
    void addDefaults ();
    void attachColumns ();
    void fillCollections (FillState & fillState);
    template <typename T>
    void
    fillCube (casacore::ArrayColumn<T> & column, const FillState & fillState,
              const GeneratorBase * generator);
    void fillCubes (FillState & fillState);
    void fillData ();
    void fillRows (FillState & fillState);

    template <typename T>
    void
    fillScalar (casacore::ScalarColumn<T> & column, const FillState & fillState,
                const GeneratorBase * generator);
    template <typename T>
    void
    fillCollection (casacore::ArrayColumn<T> & column, const FillState & fillState,
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

    typedef std::set<casacore::MSMainEnums::PredefinedColumns> ColumnIds;

    class Columns {

    public:

        Columns & operator= (const Columns & other);

        casacore::ScalarColumn<casacore::Int>    antenna1_p;
        casacore::ScalarColumn<casacore::Int>    antenna2_p;
        casacore::ScalarColumn<casacore::Int>    array_p;
        casacore::ArrayColumn<casacore::Complex> corrVis_p;
        casacore::ScalarColumn<casacore::Int>    dataDescriptionId_p;
        casacore::ScalarColumn<casacore::Double> exposure_p;
        casacore::ScalarColumn<casacore::Int>    feed1_p;
        casacore::ScalarColumn<casacore::Int>    feed2_p;
        casacore::ScalarColumn<casacore::Int>    field_p;
        casacore::ArrayColumn<casacore::Bool>    flagCategory_p;
        casacore::ScalarColumn<casacore::Bool>   flagRow_p;
        casacore::ArrayColumn<casacore::Bool>    flag_p;
        casacore::ArrayColumn<casacore::Float>   floatVis_p;
        casacore::ArrayColumn<casacore::Complex> modelVis_p;
        casacore::ScalarColumn<casacore::Int>    observation_p;
        casacore::ScalarColumn<casacore::Int>    processor_p;
        casacore::ScalarColumn<casacore::Int>    scan_p;
        casacore::ArrayColumn<casacore::Float>   sigma_p;
        casacore::ScalarColumn<casacore::Int>    state_p;
        casacore::ScalarColumn<casacore::Double> timeCentroid_p;
        casacore::ScalarColumn<casacore::Double> timeInterval_p;
        casacore::ScalarColumn<casacore::Double> time_p;
        casacore::ArrayColumn<casacore::Double>  uvw_p;
        casacore::ArrayColumn<casacore::Complex> vis_p;
        casacore::ArrayColumn<casacore::Float>   weightSpectrum_p;
        casacore::ArrayColumn<casacore::Float>   weightSpectrumCorrected_p;
        casacore::ArrayColumn<casacore::Float>   weight_p;

    };

    class Generators {

    public:

        typedef std::map<casacore::MSMainEnums::PredefinedColumns, const GeneratorBase *> GeneratorMap;

        Generators ()
        {
            generatorMap_p [casacore::MSMainEnums::ANTENNA1] = new GenerateAntenna1 ();
            generatorMap_p [casacore::MSMainEnums::ANTENNA2] = new GenerateAntenna2 ();
            generatorMap_p [casacore::MSMainEnums::ARRAY_ID] = new GenerateConstant<casacore::Int> (17);
            generatorMap_p [casacore::MSMainEnums::DATA_DESC_ID] = new GenerateDdi ();
            generatorMap_p [casacore::MSMainEnums::EXPOSURE] = new GenerateUsingRow<casacore::Double> (8);
            generatorMap_p [casacore::MSMainEnums::FEED1] = new GenerateConstant<casacore::Int> (0);
            generatorMap_p [casacore::MSMainEnums::FEED2] = new GenerateConstant<casacore::Int> (0);
            generatorMap_p [casacore::MSMainEnums::FIELD_ID] = new GenerateConstant<casacore::Int> (0);
            generatorMap_p [casacore::MSMainEnums::FLAG_CATEGORY] = new GenerateFlagCategory ();
            generatorMap_p [casacore::MSMainEnums::FLAG_ROW] = new GenerateFlagRow ();
            generatorMap_p [casacore::MSMainEnums::OBSERVATION_ID] = new GenerateUsingRow<casacore::Int> (7);
            generatorMap_p [casacore::MSMainEnums::PROCESSOR_ID] = new GenerateConstant<casacore::Int> (0);
            generatorMap_p [casacore::MSMainEnums::SCAN_NUMBER] = new GenerateUsingRow<casacore::Int> (5);
            generatorMap_p [casacore::MSMainEnums::STATE_ID] = new GenerateConstant<casacore::Int> (0);
            generatorMap_p [casacore::MSMainEnums::TIME_CENTROID] = new GenerateUsingRow<casacore::Double> (1);
            generatorMap_p [casacore::MSMainEnums::INTERVAL] = new GenerateUsingRow<casacore::Double> (2);
            generatorMap_p [casacore::MSMainEnums::TIME] = new GenerateTime ();
            generatorMap_p [casacore::MSMainEnums::FLAG] = new GenerateFlag ();
            generatorMap_p [casacore::MSMainEnums::SIGMA] = new GenerateUsingRow<casacore::Float> (3);
            generatorMap_p [casacore::MSMainEnums::SIGMA_SPECTRUM] = new GenerateConstant<casacore::Float> (0);
            generatorMap_p [casacore::MSMainEnums::UVW] = new GenerateUvw ();
            generatorMap_p [casacore::MSMainEnums::CORRECTED_DATA] = new GenerateVisibility (1);
            generatorMap_p [casacore::MSMainEnums::MODEL_DATA] = new GenerateVisibility (2);
            generatorMap_p [casacore::MSMainEnums::DATA] = new GenerateVisibility (0);
            generatorMap_p [casacore::MSMainEnums::WEIGHT] = new GenerateUsingRow<casacore::Float> (4);
            generatorMap_p [casacore::MSMainEnums::WEIGHT_SPECTRUM] = new GenerateWeightSpectrum ();
            generatorMap_p [casacore::MSMainEnums::CORRECTED_WEIGHT_SPECTRUM] = new GenerateWeightSpectrumCorrected ();

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
        get (casacore::MSMainEnums::PredefinedColumns key) const
        {
            GeneratorMap::const_iterator i = generatorMap_p.find (key);

            ThrowIf (i == generatorMap_p.end(),
                     casacore::String::format ("No such key: %d", key));

            return i->second;
        }

        void
        set (casacore::MSMainEnums::PredefinedColumns key, const GeneratorBase * newValue)
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

    casacore::Bool addWeightSpectrum_p;
    Columns columns_p;
    ColumnIds columnIds_p;
    Generators generators_p;
    casacore::Bool includeAutocorrelations_p;
    casacore::MeasurementSet * ms_p;
    casacore::Int nAntennas_p;
    casacore::Int nRows_p;
    casacore::NewMSSimulator * simulator_p;
    casacore::Double timeEnd_p;
    casacore::Double timeInterval_p;
    casacore::Double timeStart_p;
};

} // end namespace test

} // end namespace vi

} // end namespace casa

#endif // ! defined (CASA_VI_TEST_MsFactory_H_130114_1357)
