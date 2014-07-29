
String oCalTable;
CalAnalysis* poCA;

std::string uint2string( const unsigned int &number );

Bool parseInput( const ::casac::variant& field, const ::casac::variant& antenna,
    const ::casac::variant& timerange, const ::casac::variant& spw,
    const ::casac::variant& feed, const std::string& axis,
    const std::string& ap, const bool& norm, const bool& unwrap,
    const double& jumpmax, CalAnalysis::INPUT& oInput );

Bool parseField( const ::casac::variant& field, Vector<uInt>& oField );

Bool parseAntenna( const ::casac::variant& antenna, Vector<uInt>& oAntenna1,
    Vector<Int>& oAntenna2 );

Bool parseTimeRange( const ::casac::variant& timerange, Double& dStartTime,
    Double& dStopTime, Vector<Double>& oTime );

Bool parseSPW( const ::casac::variant& spw, Vector<uInt>& oSPW,
    Vector<Vector<uInt> >& oChannel );

Bool parseFeed( const ::casac::variant& feed, Vector<String>& oFeed );

Bool parseAxis( const std::string& axis, CalStats::AXIS& eAxisIterUserID );

Bool parseRAP( const std::string& ap, CalAnalysis::RAP& eRAP );

Bool parseJumpMax( const double& jumpmax, Double& dJumpMax );

Bool parseArg( const std::string& order, const std::string& type,
    const bool& weight, CalStats::ARG<CalStatsFitter::FIT>& oArg );

Bool parseOrder( const std::string& order, CalStatsFitter::ORDER& eOrder );

Bool parseType( const std::string& type, CalStatsFitter::TYPE& eType );

Bool parseWeight( const bool& weight, CalStatsFitter::WEIGHT& eWeight );

template <typename T>
Bool writeInput( const CalAnalysis::OUTPUT<T>& oOutput, const uInt& row,
    const uInt& col, ::casac::record& oRecIter );

template <typename T>
Bool writeData( const CalAnalysis::OUTPUT<T>& oOutput, const uInt& row,
    const uInt& col, ::casac::record& oRecIter );

template <typename T>
Bool writeFit( const CalStats::ARG<T>& oArg,
    const CalAnalysis::OUTPUT<T>& oOutput, const uInt& row, const uInt& col,
    ::casac::record& oRecIter );
