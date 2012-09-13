#include <VisibilityIterator_Test.h>

#include <ms/MeasurementSets/MeasurementSet.h>
#include <tables/Tables/Table.h>
#include <ms/MeasurementSets/MSAntenna.h>
#include <synthesis/MeasurementEquations/Simulator.h>

namespace casa {
namespace vi {
namespace test {

MsFactory::MsFactory ()
{
}

MeasurementSet *
MsFactory::createMs (const String & msName)
{

    Simulator simulator (msName);



    simulator.close ();
}

void
MsFactory::fillData ()
{
    for nIntegrations;
        for nDDs;

            fillIntegration ();


}

void
MsFactory::fillIntegration ()
{
    for a1;
        for a2;

            fillCubes ();

            fillCollections ();

            fillScalars ();
}

void
MsFactory::fillCubes (){


    fillVisCubeCorrected ();
    fillVisCubeModel ();
    fillVisCubeObserved ();

    fillSigmaCube ();
    fillWeightCube ();
    fillFlagCube ();

}

void
MsFactory::fillCollections ()
{
}

void
MsFactory::fillScalars ()
{
}


}

//void
//MsFactory::genrateAntennaTable ()
//{
//    SetupNewTable tableSetup ("ANTENNA", MSAntenna::requiredTableDescription(), Table::New);
//
//    Int nRows;
//    Bool initialize;
//
//    MSAntenna (tableSetup, nRows, initialize);
//// SetupNewTable &newTab, uInt nrrow,
////			       Bool initialize)
}

} // end namespace casa
} // end namespace vi
} // end namespace test

