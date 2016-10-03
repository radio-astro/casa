// The Private declarations of the MsPlot component.

// An instance of the CASA message logger.
casacore::LogIO *itsLog;

// The MsPlot class that does all of the work.  The msplot class is the
// interface to the plotter for measurement sets.
casa::MsPlot *itsMsPlot;

//private methods:

// Converts the subplot integer into three digits representing
// the nrows, ncols, and panel number that TablePlot uses.
casacore::Bool parseSubplot( const casacore::Int threeDigits,
	casacore::Int& nrows,
	casacore::Int& ncols,
	casacore::Int& panel );

// Generic function that handles all the calls to plotxy for the
// specific plots handled by the msplot tool.
casacore::Bool
plotter( const std::string& type,
	const std::string& column,
	const std::string& value,
	const std::vector<std::string>& iteration,
	casacore::Bool checkOnly=casacore::False );

// A chunk of checks for the plotter parameters that makes sure that
// that if something isn't supported by a certain plot type a warning
// is generated.

casacore::Bool
checkPlotParams( casacore::String& type,
	casacore::String &column,
	casacore::String &value,
	casacore::Vector<casacore::String>& iteration );


// Safety method used to make sure the user has called open and we
// have a measurment set to us.
casacore::Bool checkForOpenMS();

// Couple of routines to add debuggin info. to every method.
void debugFnEntry( casacore::String Fn );
void debugFnExit( casacore::String Fn );


