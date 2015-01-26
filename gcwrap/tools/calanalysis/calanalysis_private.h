
casa::String oCalTable;
casa::CalAnalysis* poCA;

std::string uint2string( const unsigned int &number );

casa::Bool parseInput( const ::casac::variant& field, const ::casac::variant& antenna,
    const ::casac::variant& timerange, const ::casac::variant& spw,
    const ::casac::variant& feed, const std::string& axis,
    const std::string& ap, const bool& norm, const bool& unwrap,
    const double& jumpmax, casa::CalAnalysis::INPUT& oInput );

casa::Bool parseField( const ::casac::variant& field, casa::Vector<casa::uInt>& oField );

casa::Bool parseAntenna( const ::casac::variant& antenna, casa::Vector<casa::uInt>& oAntenna1,
    casa::Vector<casa::Int>& oAntenna2 );

casa::Bool parseTimeRange( const ::casac::variant& timerange, casa::Double& dStartTime,
    casa::Double& dStopTime, casa::Vector<casa::Double>& oTime );

casa::Bool parseSPW( const ::casac::variant& spw, casa::Vector<casa::uInt>& oSPW,
    casa::Vector<casa::Vector<casa::uInt> >& oChannel );

casa::Bool parseFeed( const ::casac::variant& feed, casa::Vector<casa::String>& oFeed );

casa::Bool parseAxis( const std::string& axis, casa::CalStats::AXIS& eAxisIterUserID );

casa::Bool parseRAP( const std::string& ap, casa::CalAnalysis::RAP& eRAP );

casa::Bool parseJumpMax( const double& jumpmax, casa::Double& dJumpMax );

casa::Bool parseArg( const std::string& order, const std::string& type,
    const bool& weight, casa::CalStats::ARG<casa::CalStatsFitter::FIT>& oArg );

casa::Bool parseOrder( const std::string& order, casa::CalStatsFitter::ORDER& eOrder );

casa::Bool parseType( const std::string& type, casa::CalStatsFitter::TYPE& eType );

casa::Bool parseWeight( const bool& weight, casa::CalStatsFitter::WEIGHT& eWeight );

template <typename T>
casa::Bool writeInput( const casa::CalAnalysis::OUTPUT<T>& oOutput, const casa::uInt& row,
    const casa::uInt& col, ::casac::record& oRecIter );

template <typename T>
casa::Bool writeData( const casa::CalAnalysis::OUTPUT<T>& oOutput, const casa::uInt& row,
    const casa::uInt& col, ::casac::record& oRecIter );

template <typename T>
casa::Bool writeFit( const casa::CalStats::ARG<T>& oArg,
    const casa::CalAnalysis::OUTPUT<T>& oOutput, const casa::uInt& row, const casa::uInt& col,
    ::casac::record& oRecIter );
