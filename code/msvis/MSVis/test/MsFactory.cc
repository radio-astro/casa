#include "MsFactory.h"
#include <msvis/MSVis/UtilJ.h>
#include <ms/MSOper/NewMSSimulator.h>
#include <measures/Measures/MeasTable.h>
#include <tables/DataMan/TiledShapeStMan.h>


#include <cmath>

using namespace std;

namespace casa {

namespace vi {

namespace test {


MsFactory::MsFactory (const String & msName)
 : addWeightSpectrum_p (True),
   includeAutocorrelations_p (False),
   simulator_p (new NewMSSimulator (msName)),
   timeStart_p (-1)
{
    ms_p = new MeasurementSet (* simulator_p->getMs ()); //
}

MsFactory::~MsFactory ()
{
    delete ms_p;
}

void
MsFactory::addAntennas (Int nAntennas)
{
    Vector<Double> x (nAntennas), y (nAntennas), z (nAntennas),
                   diameter (nAntennas), offset (nAntennas);

    x [0] = 0;
    y [0] = 0;
    z [0] = 0;

    Vector<String> mount (nAntennas), name (nAntennas), pad (nAntennas);

    for (Int i = 1; i < nAntennas; i++){

        Double angle = ((i - 1) % 3) * (2 * 3.14159 / 3.0);
        Double radius = (i - 1) / 3.0 * 100;

        x [i] = radius * cos (angle);
        y [i] = radius * sin (angle);
        z [0] = 0;

        name [i] = String::format ("a%02d", i);
        pad [i] = String::format ("p%02d", i);
    }

    diameter = 10;
    offset = 0;
    mount = "ALT-AZ";

    MPosition vlaPosition;
    MeasTable::Observatory(vlaPosition, "VLA");

    simulator_p->initAnt ("Simulated", x, y, z, diameter, offset,
                          mount, name, pad, "local", vlaPosition);

    nAntennas_p = nAntennas;

}


//void
//MsFactory::addColumn (MSMainEnums::PredefinedColumns columnId)
//{
//    columnIds_p.insert (columnId);
//}


void
MsFactory::addDefaults ()
{
    // Configure fields if not present.

    Int nFields;
    Vector<String> sourceName;
    Vector<MDirection> sourceDirection;
    Vector<String> calCode;

    simulator_p->getFields (nFields, sourceName, sourceDirection, calCode);

    if (nFields == 0){

        Quantity ra (85.25, "deg");   // 05 41.0  -02 25 (horsehead nebula)
        Quantity dec (-2.417, "deg");
        MDirection direction (ra, dec);

        addField ("HorseHeadNebula", direction);
    }

    // Configure antennas if not present

    String telescope;
    Int nAntennas;
    Matrix<Double> xyz;
    Vector<Double> diameter;
    Vector<Double> offset;
    Vector<String> mount;
    Vector<String> name;
    Vector<String> pad;
    String coordinateSystem;
    MPosition referenceLocation;

    Bool ok = simulator_p->getAnt (telescope, nAntennas, & xyz, diameter, offset,
                                   mount, name, pad, coordinateSystem, referenceLocation);

    if (! ok){
        addAntennas (4);
    }

    // Configure feeds

    Vector<Double> x (2) , y (2);
    Vector<String> polarization (2);

    x[0] = 0;
    y[0] = .005;
    polarization [0] = "R";

    x[1] = 0;
    y[1] = .005;
    polarization [1] = "L";

    simulator_p->initFeeds ("", x, y, polarization);

    Int nSpectralWindows;
    Vector<String> spWindowNames;
    Vector<Int> nChannels;
    Vector<Quantity> startFrequencies;
    Vector<Quantity> frequencyDeltas;
    Vector<String> stokesString;

    ok = simulator_p->getSpWindows(nSpectralWindows,
                                   spWindowNames,
                                   nChannels,
                                   startFrequencies,
                                   frequencyDeltas,
                                   stokesString);

    if (! ok || nSpectralWindows == 0){

        addSpectralWindows (4);
    }

    if (timeStart_p < 0){
        timeStart_p = 0;
        timeInterval_p = 1;
        timeEnd_p = 15;
    }

}

void
MsFactory::addField (const String & name,
                     const MDirection & direction)
{
    simulator_p->initFields (name, direction, "");
}

void
MsFactory::addFeeds (Int nFeeds)
{
    Vector<Double> x (nFeeds, 0);
    Vector<String> polarization (nFeeds, "RR");
    simulator_p->initFeeds ("", x, x, polarization);
}

void
MsFactory::addSpectralWindows (int nSpectralWindows)
{
    for (Int i = 0; i < nSpectralWindows; i++){

        String name = String::format ("sp%d", i);

        addSpectralWindow (name,
                           10 + i,
                           1e9 * (i + 1),
                           1e6 * (i + 1),
                           "LL RR RL LR");
    }
}

void
MsFactory::addSpectralWindow (const String & name,
                              Int nChannels,
                              Double frequency,
                              Double frequencyDelta,
                              const String & stokes)
{
    simulator_p->initSpWindows (name,
                                nChannels,
                                Quantity (frequency, "Hz"),
                                Quantity (frequencyDelta, "Hz"),
                                Quantity (frequencyDelta, "Hz"),
                                MFrequency::TOPO,
                                stokes);

//    ThrowIf (! ok, String::format ("Failed to add spectral window "
//                                   "(name=%s, nChannels=%d, fr=%f, dFr=%f, stokes=%s",
//                                   name.c_str(), nChannels, frequency, frequencyDelta,
//                                   stokes));
}

void
MsFactory::addWeightSpectrum (Bool addIt)
{
    addWeightSpectrum_p = addIt;
}

void
MsFactory::addColumns ()
{
    if (addWeightSpectrum_p){
        addCubeColumn (MS::WEIGHT_SPECTRUM, "WeightSpectrumTiled");
    }
}

void
MsFactory::addCubeColumn (MSMainEnums::PredefinedColumns columnId,
                          const String & dataStorageManagerName)
{
    TableDesc tableDescription;// = ms_p->actualTableDesc ();

    MS::addColumnToDesc (tableDescription, columnId, 2);
    IPosition tileShape (3, 4, 100, 100);

    TiledShapeStMan storageManager (dataStorageManagerName, tileShape);

    ms_p->addColumn (tableDescription, storageManager);

    ms_p->flush();
}


void
MsFactory::attachColumns ()
{
    const ColumnDescSet & cds = ms_p->tableDesc ().columnDescSet ();

    columns_p.antenna1_p.attach (* ms_p, MS::columnName (MS::ANTENNA1));
    columns_p.antenna2_p.attach (* ms_p, MS::columnName (MS::ANTENNA2));
    columns_p.array_p.attach (* ms_p, MS::columnName (MS::ARRAY_ID));

    if (cds.isDefined ("CORRECTED_DATA")) {
        columns_p.corrVis_p.attach (* ms_p, "CORRECTED_DATA");
    }

    columns_p.dataDescriptionId_p.attach (* ms_p, MS::columnName (MS::DATA_DESC_ID));
    columns_p.exposure_p.attach (* ms_p, MS::columnName (MS::EXPOSURE));
    columns_p.field_p.attach (* ms_p, MS::columnName (MS::FIELD_ID));
    columns_p.feed1_p.attach (* ms_p, MS::columnName (MS::FEED1));
    columns_p.feed2_p.attach (* ms_p, MS::columnName (MS::FEED2));
    columns_p.flag_p.attach (* ms_p, MS::columnName (MS::FLAG));
    columns_p.flagCategory_p.attach (* ms_p, MS::columnName (MS::FLAG_CATEGORY));
    columns_p.flagRow_p.attach (* ms_p, MS::columnName (MS::FLAG_ROW));

    if (cds.isDefined (MS::columnName (MS::FLOAT_DATA))) {
        columns_p.floatVis_p.attach (* ms_p, MS::columnName (MS::FLOAT_DATA));
        //floatDataFound_p = True;
    } else {
        //floatDataFound_p = False;
    }

    if (cds.isDefined ("MODEL_DATA")) {
        columns_p.modelVis_p.attach (* ms_p, "MODEL_DATA");
    }

    columns_p.observation_p.attach (* ms_p, MS::columnName (MS::OBSERVATION_ID));
    columns_p.processor_p.attach (* ms_p, MS::columnName (MS::PROCESSOR_ID));
    columns_p.scan_p.attach (* ms_p, MS::columnName (MS::SCAN_NUMBER));
    columns_p.sigma_p.attach (* ms_p, MS::columnName (MS::SIGMA));
    columns_p.state_p.attach (* ms_p, MS::columnName (MS::STATE_ID));
    columns_p.time_p.attach (* ms_p, MS::columnName (MS::TIME));
    columns_p.timeCentroid_p.attach (* ms_p, MS::columnName (MS::TIME_CENTROID));
    columns_p.timeInterval_p.attach (* ms_p, MS::columnName (MS::INTERVAL));
    columns_p.uvw_p.attach (* ms_p, MS::columnName (MS::UVW));

    if (cds.isDefined (MS::columnName (MS::DATA))) {
        columns_p.vis_p.attach (* ms_p, MS::columnName (MS::DATA));
    }

    columns_p.weight_p.attach (* ms_p, MS::columnName (MS::WEIGHT));

    if (cds.isDefined ("WEIGHT_SPECTRUM")) {
        columns_p.weightSpectrum_p.attach (* ms_p, "WEIGHT_SPECTRUM");
    }

    if (cds.isDefined ("CORRECTED_WEIGHT_SPECTRUM")) {
        columns_p.weightSpectrumCorrected_p.attach (* ms_p, "CORRECTED_WEIGHT_SPECTRUM");
    }

}

pair<MeasurementSet *, Int>
MsFactory::createMs ()
{
    addColumns ();

    fillData ();

    ms_p->flush();

    pair<MeasurementSet *, Int> result = make_pair (ms_p, nRows_p);

    ms_p = 0; // give up all ownership and access

    return result;
}

void
MsFactory::fillData ()
{
    addDefaults ();

    attachColumns ();

    FillState fillState;

    fillState.rowNumber_p = 0;
    fillState.nAntennas_p = nAntennas_p;
    fillState.nFlagCategories_p = 3;
    fillState.timeDelta_p = timeInterval_p;

    Double time = timeStart_p;

    Int nSpectralWindows;
    Vector<String> spWindowNames;
    Vector<Int> nChannels;
    Vector<Quantity> startFrequencies;
    Vector<Quantity> frequencyDeltas;
    Vector<String> stokesString;

    simulator_p->getSpWindows(nSpectralWindows,
                              spWindowNames,
                              nChannels,
                              startFrequencies,
                              frequencyDeltas,
                              stokesString);

    while (time < timeEnd_p){

        fillState.time_p = time;

        for (Int j = 0; j < nSpectralWindows; j++){

            fillState.spectralWindow_p = j;
            fillState.nChannels_p = nChannels [j];
            vector<String> stokesComponents = utilj::split (stokesString [j], " ", True);
            fillState.nCorrelations_p = stokesComponents.size();

            fillRows (fillState);
        }

        time += timeInterval_p;
    }

    printf ("\n---Total of %d rows filled\n", fillState.rowNumber_p);
    printf ("---Time range %f to %f\n", timeStart_p, timeEnd_p);

    nRows_p = fillState.rowNumber_p;
}

void
MsFactory::fillRows (FillState & fillState)
{
    // Extend the MS to have one row per every unique pairing
    // of antennas.

    Int n = fillState.nAntennas_p + (includeAutocorrelations_p ? 1 : 0);
    Int nNewRows = (n * (n - 1)) / 2;
    ms_p->addRow(nNewRows);

    // Fill in a row for every unique antenna pairing.


    for (Int a1 = 0; a1 < fillState.nAntennas_p; a1 ++){

        fillState.antenna1_p = a1;

        Int firstA2 = includeAutocorrelations_p ? a1 : a1 + 1;

        for (Int a2 = firstA2; a2 < fillState.nAntennas_p; a2 ++){

            fillState.antenna2_p = a2;

            fillCubes (fillState);

            fillFlagCategories (fillState);

            fillCollections (fillState);

            fillScalars (fillState);

            fillState.rowNumber_p ++;
        }
    }
}

template <typename T>
void
MsFactory::fillCube (ArrayColumn<T> & column, const FillState & fillState,
                     const GeneratorBase * generatorBase)
{

    const Generator<T> * generator = dynamic_cast <const Generator<T> *> (generatorBase);

    ThrowIf (generator == 0, "Bad return type on generator");

    Matrix <T> cell (IPosition (2, fillState.nCorrelations_p, fillState.nChannels_p));

    for (Int channel = 0; channel < fillState.nChannels_p; channel ++){

        for (Int correlation = 0;
             correlation < fillState.nCorrelations_p;
             correlation ++){

            cell (correlation, channel) = (* generator) (fillState, channel, correlation);
        }
    }

    column.put (fillState.rowNumber_p, cell);
}

template <typename T>
void
MsFactory::fillScalar (ScalarColumn<T> & column, const FillState & fillState,
                       const GeneratorBase * generatorBase)
{
    const Generator<T> * generator = dynamic_cast <const Generator<T> *> (generatorBase);

    ThrowIf (generator == 0, "Bad return type on generator");

    ThrowIf (column.isNull(),
             String::format ("Column is not attached (%s)", typeid (column).name()));

    T value = (* generator) (fillState, -1, -1);

    column.put (fillState.rowNumber_p, value);
}


void
MsFactory::fillCubes (FillState & fillState)
{
    fillVisCubeCorrected (fillState);
    fillVisCubeModel (fillState);
    fillVisCubeObserved (fillState);

    fillWeightSpectrumCube (fillState);
    fillWeightSpectrumCorrectedCube (fillState);
    fillFlagCube (fillState);
}

void
MsFactory::fillFlagCategories (const FillState & fillState)
{

    const GenerateFlagCategory * generator =
            dynamic_cast <const GenerateFlagCategory *> (generators_p.get(MSMainEnums::FLAG_CATEGORY));

    ThrowIf (generator == 0, "Bad return type on generator");

    Cube <Bool> cell (IPosition (3, fillState.nCorrelations_p, fillState.nChannels_p,
                                  fillState.nFlagCategories_p));

    for (Int channel = 0; channel < fillState.nChannels_p; channel ++){

        for (Int correlation = 0;
             correlation < fillState.nCorrelations_p;
             correlation ++){

            for (Int category = 0;
                 category < fillState.nFlagCategories_p;
                 category ++){

                Bool value = (* generator) (fillState, channel, correlation, category);

                cell (correlation, channel, category) = value;
            }
        }
    }

    columns_p.flagCategory_p.put (fillState.rowNumber_p, cell);
}


void
MsFactory::fillUvw (FillState & fillState)
{
    const GeneratorBase * generatorBase = generators_p.get (MSMainEnums::UVW);
    const Generator<Vector <Double> > * generator =
            dynamic_cast<const Generator<Vector <Double> > *> (generatorBase);

    Vector<Double> uvw (3);
    uvw = (* generator) (fillState, -1, -1);

    columns_p.uvw_p.put (fillState.rowNumber_p, uvw);
}


void
MsFactory::fillVisCubeCorrected (FillState & fillState)
{
    if (! columns_p.corrVis_p.isNull ()){

        fillCube (columns_p.corrVis_p, fillState, generators_p.get(MSMainEnums::CORRECTED_DATA));
    }
}

void
MsFactory::fillWeightSpectrumCube (FillState & fillState)
{
    if (! columns_p.weightSpectrum_p.isNull()){

        fillCube (columns_p.weightSpectrum_p, fillState, generators_p.get(MSMainEnums::WEIGHT_SPECTRUM));
    }
}

void
MsFactory::fillWeightSpectrumCorrectedCube (FillState & fillState)
{
    if (! columns_p.weightSpectrumCorrected_p.isNull()){

        fillCube (columns_p.weightSpectrumCorrected_p, fillState,
                  generators_p.get(MSMainEnums::CORRECTED_WEIGHT_SPECTRUM));
    }
}

void
MsFactory::fillFlagCube (FillState & fillState)
{
    fillCube (columns_p.flag_p, fillState, generators_p.get(MSMainEnums::FLAG));
}


void
MsFactory::fillVisCubeModel (FillState & fillState)
{
    if (! columns_p.modelVis_p.isNull ()){

        fillCube (columns_p.modelVis_p, fillState, generators_p.get(MSMainEnums::MODEL_DATA));
    }
}

void
MsFactory::fillVisCubeObserved (FillState & fillState)
{
    fillCube (columns_p.vis_p, fillState, generators_p.get (MSMainEnums::DATA));
}

void
MsFactory::fillCollections (FillState & fillState)
{
    fillWeight (fillState);
    fillSigma (fillState);
    fillUvw (fillState);
}

void
MsFactory::fillScalars (FillState & fillState)
{
    fillScalar (columns_p.antenna1_p, fillState, generators_p.get (MSMainEnums::ANTENNA1));
    fillScalar (columns_p.antenna2_p, fillState, generators_p.get (MSMainEnums::ANTENNA2));
    fillScalar (columns_p.array_p, fillState, generators_p.get (MSMainEnums::ARRAY_ID));
    fillScalar (columns_p.dataDescriptionId_p, fillState, generators_p.get (MSMainEnums::DATA_DESC_ID));
    fillScalar (columns_p.exposure_p, fillState, generators_p.get (MSMainEnums::EXPOSURE));
    fillScalar (columns_p.feed1_p, fillState, generators_p.get (MSMainEnums::FEED1));
    fillScalar (columns_p.feed2_p, fillState, generators_p.get (MSMainEnums::FEED2));
    fillScalar (columns_p.field_p, fillState, generators_p.get (MSMainEnums::FIELD_ID));
    fillScalar (columns_p.flagRow_p, fillState, generators_p.get (MSMainEnums::FLAG_ROW));
    fillScalar (columns_p.observation_p, fillState, generators_p.get (MSMainEnums::OBSERVATION_ID));
    fillScalar (columns_p.processor_p, fillState, generators_p.get (MSMainEnums::PROCESSOR_ID));
    fillScalar (columns_p.scan_p, fillState, generators_p.get (MSMainEnums::SCAN_NUMBER));
    fillScalar (columns_p.state_p, fillState, generators_p.get (MSMainEnums::STATE_ID));
    fillScalar (columns_p.timeCentroid_p, fillState, generators_p.get (MSMainEnums::TIME_CENTROID));
    fillScalar (columns_p.timeInterval_p, fillState, generators_p.get (MSMainEnums::INTERVAL));
    fillScalar (columns_p.time_p, fillState, generators_p.get (MSMainEnums::TIME));
}

void
MsFactory::fillSigma (FillState & fillState)
{
    Vector<Float> sigmas (fillState.nCorrelations_p);
    const Generator<Float> * generator =
            dynamic_cast <const Generator<Float> *> (generators_p.get (MSMainEnums::SIGMA));

    for (Int i = 0; i < fillState.nCorrelations_p; i ++){
        sigmas (i) = (* generator) (fillState, -1, i);
    }


    columns_p.sigma_p.put (fillState.rowNumber_p, sigmas);

}

void
MsFactory::fillWeight (FillState & fillState)
{
    Vector<Float> weights (fillState.nCorrelations_p);
    const GeneratorBase * generatorBase = generators_p.get (MSMainEnums::WEIGHT);
    const Generator<Float> * generator =
        dynamic_cast <const Generator<Float> *> (generatorBase);

    for (Int i = 0; i < fillState.nCorrelations_p; i ++){
        weights (i) = (* generator) (fillState, -1, i);
    }

    columns_p.weight_p.put (fillState.rowNumber_p, weights);
}

void
MsFactory::setDataGenerator (MSMainEnums::PredefinedColumns column, GeneratorBase * generator)
{
    generators_p.set (column, generator);
}

void
MsFactory::setIncludeAutocorrelations (Bool includeThem)
{
    includeAutocorrelations_p = includeThem;
}

void
MsFactory::setTimeInfo (Double startingTime, Double endingTime, Double interval)
{
    timeStart_p = startingTime;
    timeEnd_p = endingTime;
    timeInterval_p = interval;
}

} // end namespace test

} // end namespace vi

} // end namespace casa
