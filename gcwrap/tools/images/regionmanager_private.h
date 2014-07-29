/* Private parts of image component */

casa::PtrHolder<casa::LogIO> _log;
casa::PtrHolder<casa::CasacRegionManager> _regMan;

// Helper method for doing unions, because the code
// in this method is needed in multiple places.
static casa::ImageRegion* dounion(
	const casa::PtrHolder<casa::Record>& regions
);

void setup();



