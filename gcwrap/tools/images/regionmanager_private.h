/* Private parts of image component */

casacore::PtrHolder<casacore::LogIO> _log;
casacore::PtrHolder<casa::CasacRegionManager> _regMan;

// Helper method for doing unions, because the code
// in this method is needed in multiple places.
static casacore::ImageRegion* dounion(
	const casacore::PtrHolder<casacore::Record>& regions
);

void setup();



