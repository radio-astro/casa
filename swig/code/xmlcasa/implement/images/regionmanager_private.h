/* Private parts of image component */

casa::LogIO *itsLog;
casa::CasacRegionManager *itsRegMan;
casa::Bool itsIsSetup;
casa::ImageRegion *unionRegion;


// Helper method for doing unions, because the code
// in this method is needed in multiple places.
casa::ImageRegion* dounion(casa::Record*& regions);

// Just a little routine that creates the casa::RegionManger
// object and the log object.
void setup();



