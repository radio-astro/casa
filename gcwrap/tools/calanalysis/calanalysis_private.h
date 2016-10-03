
casacore::String oCalTable;
CalAnalysis* poCA;

std::string uint2string( const unsigned int &number );

casacore::Bool parseInput( const ::casac::variant& field, const ::casac::variant& antenna,
    const ::casac::variant& timerange, const ::casac::variant& spw,
    const ::casac::variant& feed, const std::string& axis,
    const std::string& ap, const bool& norm, const bool& unwrap,
    const double& jumpmax, CalAnalysis::INPUT& oInput );

casacore::Bool parseField( const ::casac::variant& field, casacore::Vector<casacore::uInt>& oField );

casacore::Bool parseAntenna( const ::casac::variant& antenna, casacore::Vector<casacore::uInt>& oAntenna1,
    casacore::Vector<casacore::Int>& oAntenna2 );

casacore::Bool parseTimeRange( const ::casac::variant& timerange, casacore::Double& dStartTime,
    casacore::Double& dStopTime, casacore::Vector<casacore::Double>& oTime );

casacore::Bool parseSPW( const ::casac::variant& spw, casacore::Vector<casacore::uInt>& oSPW,
    casacore::Vector<casacore::Vector<casacore::uInt> >& oChannel );

casacore::Bool parseFeed( const ::casac::variant& feed, casacore::Vector<casacore::String>& oFeed );

casacore::Bool parseAxis( const std::string& axis, CalStats::AXIS& eAxisIterUserID );

casacore::Bool parseRAP( const std::string& ap, CalAnalysis::RAP& eRAP );

casacore::Bool parseJumpMax( const double& jumpmax, casacore::Double& dJumpMax );

casacore::Bool parseArg( const std::string& order, const std::string& type,
    const bool& weight, CalStats::ARG<CalStatsFitter::FIT>& oArg );

casacore::Bool parseOrder( const std::string& order, CalStatsFitter::ORDER& eOrder );

casacore::Bool parseType( const std::string& type, CalStatsFitter::TYPE& eType );

casacore::Bool parseWeight( const bool& weight, CalStatsFitter::WEIGHT& eWeight );

template <typename T>
casacore::Bool writeInput( const CalAnalysis::OUTPUT<T>& oOutput, const casacore::uInt& row,
    const casacore::uInt& col, ::casac::record& oRecIter );

template <typename T>
casacore::Bool writeData( const CalAnalysis::OUTPUT<T>& oOutput, const casacore::uInt& row,
    const casacore::uInt& col, ::casac::record& oRecIter );

template <typename T>
casacore::Bool writeFit( const CalStats::ARG<T>& oArg,
    const CalAnalysis::OUTPUT<T>& oOutput, const casacore::uInt& row, const casacore::uInt& col,
    ::casac::record& oRecIter );
