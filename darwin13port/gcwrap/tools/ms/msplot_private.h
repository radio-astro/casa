// The Private declarations of the MsPlot component.

// An instance of the CASA message logger.
casa::LogIO *itsLog;

// The MsPlot class that does all of the work.  The msplot class is the
// interface to the plotter for measurement sets.
casa::MsPlot *itsMsPlot;

//private methods:

// Converts the subplot integer into three digits representing
// the nrows, ncols, and panel number that TablePlot uses.
casa::Bool parseSubplot( const casa::Int threeDigits, 
	casa::Int& nrows, 
	casa::Int& ncols, 
	casa::Int& panel );

// Generic function that handles all the calls to plotxy for the
// specific plots handled by the msplot tool.
casa::Bool
plotter( const std::string& type,
	const std::string& column,
	const std::string& value,
	const std::vector<std::string>& iteration,
	casa::Bool checkOnly=casa::False );

// A chunk of checks for the plotter parameters that makes sure that
// that if something isn't supported by a certain plot type a warning
// is generated.

casa::Bool
checkPlotParams( casa::String& type,
	casa::String &column, 
	casa::String &value,
	casa::Vector<casa::String>& iteration );


// Safety method used to make sure the user has called open and we
// have a measurment set to us.
casa::Bool checkForOpenMS();

// Couple of routines to add debuggin info. to every method.
void debugFnEntry( casa::String Fn );
void debugFnExit( casa::String Fn );


