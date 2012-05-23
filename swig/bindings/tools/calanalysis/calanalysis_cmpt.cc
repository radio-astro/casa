
/***
 * Framework independent implementation file for calanalysis...
 *
 * Implement the calanalysis component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#include <calanalysis_cmpt.h>

using namespace std;
using namespace casa;

namespace casac {

// --- //

calanalysis::calanalysis( void ) {

  // Initialize the pointer to the CalAnalysis() instance and return

  poCA = NULL;

  return;

}

// --- //

calanalysis::~calanalysis( void ) {

  //  If the CalAnalysis instance is non-NULL deallocate it and return

  if ( poCA == NULL ) return;

  delete poCA;
  poCA = NULL;

  return;

}

// --- //

bool calanalysis::open( const std::string& caltable ) {

  // Is a calibration table already open?

  if ( poCA != NULL ) {
    LogIO log( LogOrigin( "CalAnalysis", "CalAnalysis()", WHERE ) );
    log << LogIO::WARN << "File " << poCA->calName() << " is already open"
        << LogIO::POST;
    return( false );
  }


  // Allocate the CalAnalysis instance and return true

  try {
    poCA = new CalAnalysis( String(caltable) );
  }

  catch( AipsError oAE ) {
    LogIO log( LogOrigin( "CalAnalysis", "CalAnalysis()", WHERE ) );
    log << LogIO::WARN << oAE.getMesg() << LogIO::POST;
    return( false );
  }

  return( true );

}

// --- //

bool calanalysis::close( void ) {

  // Deallocate the CalAnalysis instance and return true

  if ( poCA == NULL ) return( false );

  delete poCA;
  poCA = NULL;

  return( true );

}

// --- //

std::string calanalysis::calname( void ) {

  // If the CalAnalysis instance is valid return the caltable name otherwise
  // return a null string

  if ( poCA == NULL ) {
    LogIO log( LogOrigin( "CalAnalysis", "calname()", WHERE ) );
    log << LogIO::WARN << "Open caltable first" << LogIO::POST;
    return( string() );
  }

  return( poCA->calName() );

}

// --- //

std::string calanalysis::msname( void ) {

  // Return the associated caltable name

  if ( poCA == NULL ) {
    LogIO log( LogOrigin( "CalAnalysis", "msname()", WHERE ) );
    log << LogIO::WARN << "Open caltable first" << LogIO::POST;
    return( string() );
  }

  return( poCA->msName() );

}

// --- //

std::string calanalysis::viscal( void ) {

  // Return the caltable type (B, G, T, etc.)

  if ( poCA == NULL ) {
    LogIO log( LogOrigin( "CalAnalysis", "viscal()", WHERE ) );
    log << LogIO::WARN << "Open caltable first" << LogIO::POST;
    return( string() );
  }

  return( poCA->visCal() );

}

// --- //

std::string calanalysis::partype( void ) {

  // If the CalAnalysis instance is valid return "Complex" for a CPARAM column
  // or "Float" for a PARAM colum, otherwise a null string

  if ( poCA == NULL ) {
    LogIO log( LogOrigin( "CalAnalysis", "partype()", WHERE ) );
    log << LogIO::WARN << "Open caltable first" << LogIO::POST;
    return( string() );
  }

  return( poCA->parType() );

}

// --- //

std::string calanalysis::polbasis( void ) {

  // If the CalAnalysis instance is valid return the polarization basis
  // otherwise return a null string

  if ( poCA == NULL ) {
    LogIO log( LogOrigin( "CalAnalysis", "polbasis()", WHERE ) );
    log << LogIO::WARN << "Open caltable first" << LogIO::POST;
    return( string() );
  }

  return( poCA->polBasis() );

}

// --- //

int calanalysis::numfield( void ) {

  // Is the CalAnalysis instance valid?

  if ( poCA == NULL ) {
    LogIO log( LogOrigin( "CalAnalysis", "numfield()", WHERE ) );
    log << LogIO::WARN << "Open caltable first" << LogIO::POST;
    return( 0 );
  }


  // Return the number of fields

  return( poCA->field().nelements() );

}

// --- //

std::vector<std::string> calanalysis::field( const bool name ) {

  // Is the CalAnalysis instance valid?

  if ( poCA == NULL ) {
    LogIO log( LogOrigin( "CalAnalysis", "field()", WHERE ) );
    log << LogIO::WARN << "Open caltable first" << LogIO::POST;
    return( std::vector<std::string>() );
  }


  // Get the fields, either by name or number

  Vector<uInt> oField( poCA->field() );
  uInt uiNumField = oField.nelements();

  std::vector<std::string> field( uiNumField );

  if ( name ) {

    Table oTable( poCA->calName()+String("/FIELD"), Table::Old );
    ROScalarColumn<String> oROSC( oTable, String("NAME") );

    Vector<String> oFieldString;
    oROSC.getColumn( oFieldString, True );

    for ( uInt f=0; f<uiNumField; f++ ) field[f] = oFieldString[f];

  } else {

    for ( uInt f=0; f<uiNumField; f++ ) field[f] = uint2string( oField[f] );

  }


  // Return the fields

  return( field );

}

// --- //

int calanalysis::numantenna( void ) {

  // Is the CalAnalysis instance valid?

  if ( poCA == NULL ) {
    LogIO log( LogOrigin( "CalAnalysis", "numantenna()", WHERE ) );
    log << LogIO::WARN << "Open caltable first" << LogIO::POST;
    return( 0 );
  }


  // Return the number of antennas

  return( poCA->antenna().nelements() );

}

// --- //

std::vector<std::string> calanalysis::antenna( const bool name ) {

  // Is the CalAnalysis instance valid?

  if ( poCA == NULL ) {
    LogIO log( LogOrigin( "CalAnalysis", "antenna()", WHERE ) );
    log << LogIO::WARN << "Open caltable first" << LogIO::POST;
    return( std::vector<std::string>() );
  }


  // Get the antennas, either by name or number

  Vector<uInt> oAntenna( poCA->antenna() );
  uInt uiNumAntenna = oAntenna.nelements();

  std::vector<std::string> antenna( uiNumAntenna );

  if ( name ) {

    Table oTable( poCA->calName()+String("/ANTENNA"), Table::Old );
    ROScalarColumn<String> oROSC( oTable, String("NAME") );

    Vector<String> oAntennaString;
    oROSC.getColumn( oAntennaString, True );

    for ( uInt a=0; a<uiNumAntenna; a++ ) antenna[a] = oAntennaString[a];

  } else {

    for ( uInt a=0; a<uiNumAntenna; a++ ) {
      antenna[a] = uint2string( oAntenna[a] );
    }

  }


  // Return the antennas

  return( antenna );

}

// --- //

int calanalysis::numantenna1( void ) {

  // Is the CalAnalysis instance valid?

  if ( poCA == NULL ) {
    LogIO log( LogOrigin( "CalAnalysis", "numantenna1()", WHERE ) );
    log << LogIO::WARN << "Open caltable first" << LogIO::POST;
    return( 0 );
  }


  // Return the number of antenna 1s

  return( poCA->antenna1().nelements() );

}

// --- //

std::vector<std::string> calanalysis::antenna1( const bool name ) {

  // Is the CalAnalysis instance valid?

  if ( poCA == NULL ) {
    LogIO log( LogOrigin( "CalAnalysis", "antenna1()", WHERE ) );
    log << LogIO::WARN << "Open caltable first" << LogIO::POST;
    return( std::vector<std::string>() );
  }


  // Get all of the antenna numbers

  std::vector<std::string> antenna( this->antenna(name) );


  // Load the antenna 1 names of numbers and return them

  Vector<uInt> oAntenna1( poCA->antenna1() );
  uInt uiNumAntenna1 = oAntenna1.nelements();

  std::vector<std::string> antenna1( uiNumAntenna1 );

  for ( uInt a1=0; a1<uiNumAntenna1; a1++ ) {
    antenna1[a1] = antenna[oAntenna1[a1]];
  }

  return( antenna1 );

}

// --- //

int calanalysis::numantenna2( void ) {

  // Is the CalAnalysis instance valid?

  if ( poCA == NULL ) {
    LogIO log( LogOrigin( "CalAnalysis", "numantenna2()", WHERE ) );
    log << LogIO::WARN << "Open caltable first" << LogIO::POST;
    return( 0 );
  }


  // Return the number of antenna 2s

  return( poCA->antenna2().nelements() );

}

// --- //

std::vector<std::string> calanalysis::antenna2( const bool name ) {

  // Is the CalAnalysis instance valid?

  if ( poCA == NULL ) {
    LogIO log( LogOrigin( "CalAnalysis", "antenna2()", WHERE ) );
    log << LogIO::WARN << "Open caltable first" << LogIO::POST;
    return( std::vector<std::string>() );
  }


  // Get all of the antenna numbers

  std::vector<std::string> antenna( this->antenna(name) );


  // Load the antenna 2 names of numbers and return them

  Vector<Int> oAntenna2( poCA->antenna2() );
  uInt uiNumAntenna2 = oAntenna2.nelements();

  std::vector<std::string> antenna2( uiNumAntenna2 );

  for ( uInt a2=0; a2<uiNumAntenna2; a2++ ) {
    if ( oAntenna2[a2] != -1 ) {
      antenna2[a2] = antenna[oAntenna2[a2]];
    } else {
      if ( name ) {
        antenna2[a2] = std::string( "NoName" );
      } else {
        antenna2[a2] = std::string( "-1" );
      }
    }
  }

  return( antenna2 );

}

// --- //

int calanalysis::numfeed( void ) {

  // Is the CalAnalysis instance valid?

  if ( poCA == NULL ) {
    LogIO log( LogOrigin( "CalAnalysis", "numfeed()", WHERE ) );
    log << LogIO::WARN << "Open caltable first" << LogIO::POST;
    return( 0 );
  }


  // Return the number of feeds

  return( poCA->feed().nelements() );

}

// --- //

std::vector<std::string> calanalysis::feed( void ) {

  // Is the CalAnalysis instance valid?

  if ( poCA == NULL ) {
    LogIO log( LogOrigin( "CalAnalysis", "feed()", WHERE ) );
    log << LogIO::WARN << "Open caltable first" << LogIO::POST;
    return( std::vector<std::string>() );
  }


  // Get the feeds and return them

  Vector<String> oFeed( poCA->feed() );
  uInt uiNumFeed = oFeed.nelements();

  std::vector<std::string> feed( uiNumFeed );
  for ( uInt f=0; f<uiNumFeed; f++ ) feed[f] = oFeed[f];

  return( feed );

}

// --- //

int calanalysis::numtime( void ) {

  // Is the CalAnalysis instance valid?

  if ( poCA == NULL ) {
    LogIO log( LogOrigin( "CalAnalysis", "numtime()", WHERE ) );
    log << LogIO::WARN << "Open caltable first" << LogIO::POST;
    return( 0 );
  }


  // Return the number of times

  return( poCA->time().nelements() );

}

// --- //

std::vector<double> calanalysis::time( void ) {

  // Is the CalAnalysis instance valid?

  if ( poCA == NULL ) {
    LogIO log( LogOrigin( "CalAnalysis", "time()", WHERE ) );
    log << LogIO::WARN << "Open caltable first" << LogIO::POST;
    return( std::vector<double>() );
  }


  // Get the times and return them

  Vector<Double> oTime( poCA->time() );
  uInt uiNumTime = oTime.nelements();

  std::vector<double> time( uiNumTime );
  for ( uInt t=0; t<uiNumTime; t++ ) time[t] = oTime[t];

  return( time );

}

// --- //

int calanalysis::numspw( void ) {

  // Is the CalAnalysis instance valid?

  if ( poCA == NULL ) {
    LogIO log( LogOrigin( "CalAnalysis", "numspw()", WHERE ) );
    log << LogIO::WARN << "Open caltable first" << LogIO::POST;
    return( 0 );
  }


  // Return the number of spectral windows

  return( poCA->numspw() );

}

// --- //

std::vector<std::string> calanalysis::spw( const bool name ) {

  // Is the CalAnalysis instance valid?

  if ( poCA == NULL ) {
    LogIO log( LogOrigin( "CalAnalysis", "spw()", WHERE ) );
    log << LogIO::WARN << "Open caltable first" << LogIO::POST;
    return( std::vector<std::string>() );
  }


  // Get the spectral windows, either by name or number

  Vector<uInt> oSPW( poCA->spw() );
  uInt uiNumSPW = oSPW.nelements();

  std::vector<std::string> spw( uiNumSPW );

  if ( name ) {

    Table oTable( poCA->calName()+String("/SPECTRAL_WINDOW"), Table::Old );
    ROScalarColumn<String> oROSC( oTable, String("NAME") );

    Vector<String> oSPWString;
    oROSC.getColumn( oSPWString, True );

    for ( uInt s=0; s<uiNumSPW; s++ ) spw[s] = oSPWString[s];

  } else {

    for ( uInt s=0; s<uiNumSPW; s++ ) spw[s] = uint2string( oSPW[s] );

  }


  // Return the spectral windows

  return( spw );

}

// --- //

std::vector<int> calanalysis::numchannel( void ) {

  // Is the CalAnalysis instance valid?

  if ( poCA == NULL ) {
    LogIO log( LogOrigin( "CalAnalysis", "numchannel()", WHERE ) );
    log << LogIO::WARN << "Open caltable first" << LogIO::POST;
    return( std::vector<int>() );
  }


  // Get the numbers of channels and return them

  Vector<uInt> oNumChannel( poCA->numChannel() );
  uInt uiNumSPW = oNumChannel.nelements();

  std::vector<int> numchannel( uiNumSPW );
  for ( uInt s=0; s<uiNumSPW; s++ ) numchannel[s] = (int) oNumChannel[s];

  return( numchannel );

}

// --- //

::casac::record* calanalysis::freq( void ) {

  // Is the CalAnalysis instance valid?

  ::casac::record* poRecord = new ::casac::record();

  if ( poCA == NULL ) {
    LogIO log( LogOrigin( "CalAnalysis", "freq()", WHERE ) );
    log << LogIO::WARN << "Open caltable first" << LogIO::POST;
    return( poRecord );
  }


  // Get the frequencies and return them

  Vector<Vector<Double> > voFreq( poCA->freq() );
  uInt uiNumSPW = voFreq.nelements();

  for ( uInt s=0; s<uiNumSPW; s++ ) {
    std::vector<double> freq( voFreq[s].nelements() );
    for ( uInt c=0; c<voFreq[s].nelements(); c++ ) freq[c] = voFreq[s][c];
    poRecord->insert( std::string( uint2string(s) ), freq );
  }

  return( poRecord );

}

// --- //

::casac::record* calanalysis::get( const ::casac::variant& field,
    const ::casac::variant& antenna, const ::casac::variant& timerange,
    const ::casac::variant& spw, const ::casac::variant& feed,
    const std::string& axis, const std::string& ap, const bool norm,
    const bool unwrap, const double jumpmax ) {

  // Initialize the output dictionary

  ::casac::record* poRecord = new ::casac::record();


  // If there is no CalAnalysis() instance, return without doing anything

  if ( poCA == NULL ) {
    LogIO log( LogOrigin( "calanalysis", "get()", WHERE ) );
    log << LogIO::WARN << "Open calibration analysis tool first"
        << LogIO::POST;
    return( poRecord );
  }


  // Parse the input variables and return the populated INPUT nested class

  CalAnalysis::INPUT oInput;

  Bool bInput = parseInput( field, antenna, timerange, spw, feed, axis, ap,
      norm, unwrap, jumpmax, oInput );

  if ( !bInput ) {
    LogIO log( LogOrigin( "calanalysis", "get()", WHERE ) );
    log << LogIO::WARN << "Invalid input(s)" << LogIO::POST;
    return( poRecord );
  }


  // Initialize the arguments (NONE)

  CalStats::ARG<CalStats::NONE> oArg;


  // Call the CalAnalysis::stats<T>() member function for getting

  Vector<CalAnalysis::OUTPUT<CalStats::NONE> > oOutput;

  try {
    oOutput = poCA->stats<CalStats::NONE>( oInput, oArg );
  }

  catch( AipsError oAipsError ) {
    LogIO log( LogOrigin( "calanalysis", "get()", WHERE ) );
    log << LogIO::WARN << oAipsError.getMesg() << LogIO::POST;
    return( poRecord );
  }


  // Initialize the number of outputs and the iteration number.  The final
  // iteration number should be identical to the number of outputs times the
  // number of rows times the number of columns.  Each row and column represents
  // an iteration in the CalStats() instance.

  uInt uiNumOutput = oOutput.nelements();

  uInt uiNumIter = 0;


  // Load the output record with the caltable data.  Each iteration corresponds
  // to a single field, antenna1, antenna2, feed, and frequency or time.

  for ( uInt o=0; o<uiNumOutput; o++ ) {

    uInt uiNumRow = oOutput[o].oOut.shape()[0];
    uInt uiNumCol = oOutput[o].oOut.shape()[1];

    for ( uInt row=0; row<uiNumRow; row++ ) {
      for ( uInt col=0; col<uiNumCol; col++ ) {

        ::casac::record oRecIter;

        Bool bWriteInput = writeInput( oOutput[o], row, col, oRecIter );

        if ( !bWriteInput ) {
          LogIO log( LogOrigin( "calanalysis", "get()", WHERE ) );
          log << LogIO::WARN << "Error writing inputs" << LogIO::POST;
          return( poRecord );
	}

        Bool bWriteData = writeData( oOutput[o], row, col, oRecIter );

        if ( !bWriteData ) {
          LogIO log( LogOrigin( "calanalysis", "get()", WHERE ) );
          log << LogIO::WARN << "Error writing data" << LogIO::POST;
          return( poRecord );
	}

        std::string sNumIter( String::toString(uiNumIter).c_str() );
        poRecord->insert( sNumIter, oRecIter );

        uiNumIter++;

      }
    }

  }


  // Return the record

  return( poRecord );

}

// --- //

::casac::record* calanalysis::fit( const ::casac::variant& field,
    const ::casac::variant& antenna, const ::casac::variant& timerange,
    const ::casac::variant& spw, const ::casac::variant& feed,
    const std::string& axis, const std::string& ap, const bool norm,
    const bool unwrap, const double jumpmax, const std::string& order,
    const std::string& type, const bool weight ) {

  // Initialize the output dictionary

  ::casac::record* poRecord = new ::casac::record();


  // If there is no CalAnalysis() instance, return without doing anything

  if ( poCA == NULL ) {
    LogIO log( LogOrigin( "calanalysis", "fit()", WHERE ) );
    log << LogIO::WARN << "Open calibration analysis tool first"
        << LogIO::POST;
    return( poRecord );
  }


  // Parse the input variables and return the populated INPUT nested class

  CalAnalysis::INPUT oInput;

  Bool bInput = parseInput( field, antenna, timerange, spw, feed, axis, ap,
      norm, unwrap, jumpmax, oInput );

  if ( !bInput ) {
    LogIO log( LogOrigin( "calanalysis", "fit()", WHERE ) );
    log << LogIO::WARN << "Invalid input(s)" << LogIO::POST;
    return( poRecord );
  }


  // Initialize the order, type, and weight parameters for the arguments

  CalStats::ARG<CalStatsFitter::FIT> oArg;

  Bool bArg = parseArg( order, type, weight, oArg );

  if ( !bArg ) {
    LogIO log( LogOrigin( "calanalysis", "fit()", WHERE ) );
    log << LogIO::WARN << "Invalid fit argument(s)" << LogIO::POST;
    return( poRecord );
  }


  // Call the CalAnalysis::stats<T>() member function for getting and fitting

  Vector<CalAnalysis::OUTPUT<CalStatsFitter::FIT> > oOutput;

  try {
    oOutput = poCA->stats<CalStatsFitter::FIT>( oInput, oArg );
  }

  catch( AipsError oAipsError ) {
    LogIO log( LogOrigin( "calanalysis", "fit()", WHERE ) );
    log << LogIO::WARN << oAipsError.getMesg() << LogIO::POST;
    return( poRecord );
  }


  // Initialize the number of outputs and the iteration number.  The final
  // iteration number should be identical to the number of outputs times the
  // number of rows times the number of columns.  Each row and column represents
  // an iteration in the CalStats() instance.

  uInt uiNumOutput = oOutput.nelements();

  uInt uiNumIter = 0;


  // Load the output record with the caltable data.  Each iteration corresponds
  // to a single field, antenna1, antenna2, feed, and frequency or time.

  for ( uInt o=0; o<uiNumOutput; o++ ) {

    uInt uiNumRow = oOutput[o].oOut.shape()[0];
    uInt uiNumCol = oOutput[o].oOut.shape()[1];

    for ( uInt row=0; row<uiNumRow; row++ ) {
      for ( uInt col=0; col<uiNumCol; col++ ) {

        ::casac::record oRecIter;

        Bool bWriteInput = writeInput( oOutput[o], row, col, oRecIter );

        if ( !bWriteInput ) {
          LogIO log( LogOrigin( "calanalysis", "fit()", WHERE ) );
          log << LogIO::WARN << "Error writing inputs" << LogIO::POST;
          return( poRecord );
	}

        Bool bWriteData = writeData( oOutput[o], row, col, oRecIter );

        if ( !bWriteData ) {
          LogIO log( LogOrigin( "calanalysis", "fit()", WHERE ) );
          log << LogIO::WARN << "Error writing data" << LogIO::POST;
          return( poRecord );
	}

        Bool bWriteFit = writeFit( oArg, oOutput[o], row, col, oRecIter );

        if ( !bWriteFit ) {
          LogIO log( LogOrigin( "calanalysis", "fit()", WHERE ) );
          log << LogIO::WARN << "Error writing fit" << LogIO::POST;
          return( poRecord );
	}

        std::string sNumIter( String::toString(uiNumIter).c_str() );
        poRecord->insert( sNumIter, oRecIter );

        uiNumIter++;

      }
    }

  }


  // Return the record

  return( poRecord );

}

// --- //

std::string calanalysis::uint2string(const unsigned int &number) {
   stringstream ss;
   ss << number;
   return( ss.str() );
}

// --- //

Bool calanalysis::parseInput( const ::casac::variant& field,
    const ::casac::variant& antenna, const ::casac::variant& timerange,
    const ::casac::variant& spw, const ::casac::variant& feed,
    const std::string& axis, const std::string& ap, const bool& norm,
    const bool& unwrap, const double& jumpmax, CalAnalysis::INPUT& oInput ) {

  // Parse the field variant

  Bool bField = parseField( field, oInput.oField );

  if ( !bField ) {
    LogIO log( LogOrigin( "calanalysis", "parseInput()", WHERE ) );
    log << LogIO::WARN << "Invalid field(s)" << LogIO::POST;
    return( False );
  }


  // Parse the antenna variant

  Bool bAntenna = parseAntenna( antenna, oInput.oAntenna1, oInput.oAntenna2 );

  if ( !bAntenna ) {
    LogIO log( LogOrigin( "calanalysis", "parseInput()", WHERE ) );
    log << LogIO::WARN << "Invalid antenna(s)" << LogIO::POST;
    return( False );
  }


  // Parse the time range variant

  Bool bTimeRange = parseTimeRange( timerange, oInput.dStartTime,
      oInput.dStopTime, oInput.oTime );

  if ( !bTimeRange ) {
    LogIO log( LogOrigin( "calanalysis", "parseInput()", WHERE ) );
    log << LogIO::WARN << "Invalid time(s)" << LogIO::POST;
    return( False );
  }


  // Parse the spw variant

  Bool bSPW = parseSPW( spw, oInput.oSPW, oInput.oChannel );

  if ( !bSPW ) {
    LogIO log( LogOrigin( "calanalysis", "parseInput()", WHERE ) );
    log << LogIO::WARN << "Invalid spw(s) or channel(s)" << LogIO::POST;
    return( False );
  }


  // Parse the feed variant

  Bool bFeed = parseFeed( feed, oInput.oFeed );

  if ( !bFeed ) {
    LogIO log( LogOrigin( "calanalysis", "parseInput()", WHERE ) );
    log << LogIO::WARN << "Invalid feed(s)" << LogIO::POST;
    return( False );
  }


  // Parse the user-defined iteration axis string

  Bool bAxis = parseAxis( axis, oInput.eAxisIterUserID );

  if ( !bAxis ) {
    LogIO log( LogOrigin( "calanalysis", "parseInput()", WHERE ) );
    log << LogIO::WARN << "Invalid user-defined iteration axis" << LogIO::POST;
    return( False );
  }


  // Parse the (real, amplitude, phase) RAP

  Bool bRAP = parseRAP( ap, oInput.eRAP );

  if ( !bRAP ) {
    LogIO log( LogOrigin( "calanalysis", "parseInput()", WHERE ) );
    log << LogIO::WARN << "Invalid amplitude/phase parameter" << LogIO::POST;
    return( False );
  }


  // Set the amplitude normalization and phase unwrapping boolean

  oInput.bNorm = norm;

  oInput.bUnwrap = unwrap;


  // Parse the jumpmax double

  Bool bJumpMax = parseJumpMax( jumpmax, oInput.dJumpMax );

  if ( !bJumpMax ) {
    LogIO log( LogOrigin( "calanalysis", "parseInput()", WHERE ) );
    log << LogIO::WARN << "Invalid maximum jump parameter" << LogIO::POST;
    return( False );
  }


  // Return True

  return( True );

}

// --- //

Bool calanalysis::parseField( const ::casac::variant& field,
    Vector<uInt>& oField ) {

  // Initialize the instances required for calibration table selection

  NewCalTable oNCT( poCA->calName(), Table::Old );
  CTInterface oNCTI( oNCT );

  MSSelection oMSS;


  // Convert the input variant into a String instance, trim it, and set it to
  // "*" if it is equal to ""

  String oFieldString( toCasaString( field ) );
  oFieldString.trim();

  if ( oFieldString == String("") ) oFieldString = String( "*" );


  // Get the field selection

  try {

    oMSS.setFieldExpr( oFieldString );
    oMSS.toTableExprNode( &oNCTI );

    Vector<Int> oFieldTemp( oMSS.getFieldList() );
    oField.resize( oFieldTemp.nelements(), False );

    convertArray<uInt,Int>( oField, oFieldTemp );

  }

  catch ( AipsError oAE ) {
    oField.resize();
    return( False );
  }


  // Check the length of the output vector

  if ( oField.nelements() == 0 ) return( False );


  // Return True

  return( True );

}

// --- //

Bool calanalysis::parseAntenna( const ::casac::variant& antenna,
    Vector<uInt>& oAntenna1, Vector<Int>& oAntenna2 ) {

  // Initialize the instances required for calibration table selection

  NewCalTable oNCT( poCA->calName(), Table::Old );
  CTInterface oNCTI( oNCT );

  MSSelection oMSS;


  // Convert the input variant into a String instance, trim it, and set it to
  // "*" if it is equal to ""

  String oAntennaString( toCasaString( antenna ) );
  oAntennaString.trim();

  if ( oAntennaString == String("") ) oAntennaString = String( "*" );


  // Get the antenna1 and antenna2 selections

  try {

    oMSS.setAntennaExpr( oAntennaString );
    oMSS.toTableExprNode( &oNCTI );

    Vector<Int> oAntenna1Temp( oMSS.getAntenna1List() );
    oAntenna1.resize( oAntenna1Temp.nelements(), False );

    convertArray<uInt,Int>( oAntenna1, oAntenna1Temp );

    oAntenna2.resize();
    if ( poCA->antenna2()[0] != -1 ) {
      oAntenna2 = oMSS.getAntenna2List();
    } else {
      oAntenna2 = Vector<Int>( 1, -1 );
    }

  }

  catch ( AipsError oAE ) {
    oAntenna1.resize();
    oAntenna2.resize();
    return( False );
  }


  // Check the lengths of the output vectors

  if ( oAntenna1.nelements() == 0 || oAntenna2.nelements() == 0 ) {
    return( False );
  }


  // Return True

  return( True );

}

// --- //

Bool calanalysis::parseTimeRange( const ::casac::variant& timerange,
    Double& dStartTime, Double& dStopTime, Vector<Double>& oTime ) {

  // Initialize the instances required for calibration table selection

  NewCalTable oNCT( poCA->calName(), Table::Old );
  CTInterface oNCTI( oNCT );

  MSSelection oMSS;


  // Convert the input variant to a String instance and trim it

  String oTimeRangeString( toCasaString( timerange ) );
  oTimeRangeString.trim();


  // Get the start time and stop time selections

  Vector<Double> oTimeRange;

  if ( oTimeRangeString != String("") ) {

    oTimeRange = timerange.toDoubleVec();

    if ( oTimeRange.nelements() != 2 ) {
      dStartTime = 0.0;
      dStopTime = 0.0;
      return( False );
    }

    dStartTime = oTimeRange[0];
    dStopTime = oTimeRange[1];

  } else {

    oTimeRange = poCA->time();

    dStartTime = oTimeRange[0];
    dStopTime = oTimeRange[oTimeRange.nelements()-1];

  }


  // Get all of the times corresponding to the time range

  Vector<Double> oTimeTemp( poCA->time() );
  oTime.resize();

  for ( uInt tt=0, uiNumTime=0; tt<oTimeTemp.nelements(); tt++ ) {
    if ( oTimeTemp[tt] >= dStartTime && oTimeTemp[tt] <= dStopTime ) {
      oTime.resize( ++uiNumTime, True );
      oTime[uiNumTime-1] = oTimeTemp[tt];
    }
  }


  // Return True

  return( True );

}

// --- //

Bool calanalysis::parseSPW( const ::casac::variant& spw,
    Vector<uInt>& oSPW, Vector<Vector<uInt> >& oChannel ) {

  // Initialize the instances required for calibration table selection

  NewCalTable oNCT( poCA->calName(), Table::Old );
  CTInterface oNCTI( oNCT );

  MSSelection oMSS;


  // Convert the input variant into a String instance, trim it, and set it to
  // "*" if it is equal to ""

  String oSPWString( toCasaString( spw ) );
  oSPWString.trim();

  if ( oSPWString == String("") ) oSPWString = String( "*" );


  // Get the spectral windows and corresponding channels.  Duplicate spectral
  // windows and channels are removed and the remaining are sorted.

  uInt uiNumSPW;

  try {

    oMSS.setSpwExpr( oSPWString );
    oMSS.toTableExprNode( &oNCTI );

    Vector<Int> oSPWTemp( oMSS.getSpwList() );
    oSPW.resize( oSPWTemp.nelements() );
    convertArray<uInt,Int>( oSPW, oSPWTemp );

    Matrix<Int> oChanListTemp( oMSS.getChanList() );
    Matrix<uInt> oChanList( oChanListTemp.shape() );
    convertArray<uInt,Int>( oChanList, oChanListTemp );

    uiNumSPW = oSPW.nelements();

    Vector<Int> oSPWRev( max(oSPW)+1, -1 );
    for ( uInt s=0; s<uiNumSPW; s++ ) oSPWRev[oSPW[s]] = (Int) s;

    uInt uiNumRow = oChanList.shape()[0];
    Vector<Vector<uInt> > oChannelTemp( uiNumSPW );

    for ( uInt r=0; r<uiNumRow; r++ ) {
      uInt uiSPW = (uInt) oSPWRev[oChanList(r,0)];
      uInt uiStart = oChanList(r,1);
      uInt uiStop = oChanList(r,2);
      uInt uiStep = oChanList(r,3);
      for ( uInt c=uiStart; c<=uiStop; c+=uiStep ) {
        uInt uiNumChannel = oChannelTemp[uiSPW].nelements();
        oChannelTemp[uiSPW].resize( uiNumChannel+1, True );
        oChannelTemp[uiSPW][uiNumChannel] = c;
      }
    }

    oChannel.resize( uiNumSPW );
    for ( uInt s=0; s<uiNumSPW; s++ ) {
      oChannel[s] = CalAnalysis::unique<uInt>( oChannelTemp[s] );
    }

  }

  catch ( AipsError oAE ) {
    oSPW.resize();
    oChannel.resize();
    return( False );
  }


  // Check the lengths of the vectors

  if ( oSPW.nelements() == 0 ) return( False );

  uInt uiNumChannelAll = 0;
  for ( uInt s=0; s<uiNumSPW; s++ ) uiNumChannelAll += oChannel[s].nelements();
  if ( uiNumChannelAll == 0 ) return( False );


  // Return True

  return( True );

}

// --- //

Bool calanalysis::parseFeed( const ::casac::variant& feed,
    Vector<String>& oFeed ) {

  // Initialize the instances required for calibration table selection

  NewCalTable oNCT( poCA->calName(), Table::Old );
  CTInterface oNCTI( oNCT );

  MSSelection oMSS;


  // Convert the input variant to a String instance and trim it

  String oFeedString( toCasaString( feed ) );
  oFeedString.trim();


  // If the input is the String("") default, get all feeds

  if ( oFeedString == String("") ) {
    oFeed.resize();
    oFeed = poCA->feed();
    return( True );
  }


  // Get the feeds

  try {

    String* aoFeed = new String [10000]; // Arbitrarily large
    uInt uiNumFeed = split( oFeedString, aoFeed, 10000, ',' );
    if ( uiNumFeed > 10000 ) {
      delete [] aoFeed;
      oFeed.resize();
      return( False );
    }

    Vector<String> oFeedTemp( uiNumFeed );

    for ( uInt f=0; f<uiNumFeed; f++ ) {
      aoFeed[f].trim(); aoFeed[f].upcase();
      if ( !CalAnalysis::exists<String>( aoFeed[f], poCA->feed() ) ) {
        delete [] aoFeed;
        oFeed.resize();
        return( False );
      }
      oFeedTemp[f] = aoFeed[f];
    }

    delete [] aoFeed;

    oFeed.resize();
    oFeed = CalAnalysis::unique<String>( oFeedTemp );

    if ( oFeed.nelements() != 1 && oFeed.nelements() != 2 ) {
      oFeed.resize();
      return( False );
    }

  }

  catch ( AipsError oAE ) {
    oFeed.resize();
    return( False );
  }


  // Return the length of the vector

  if ( oFeed.nelements() == 0 ) return( False );


  // Return True

  return( True );

}

// --- //

Bool calanalysis::parseAxis( const std::string& axis,
    CalStats::AXIS& eAxisIterUserID ) {

  // Convert the input string to an uppercase CASA string of length 1

  String axisTemp( toCasaString( axis ) );
  axisTemp.trim();
  if ( axisTemp == "" ) axisTemp = "T";
  axisTemp.upcase(); axisTemp = axisTemp[0];


  // Get the user-defined iteration axis

  eAxisIterUserID = CalStats::INIT;

  if ( axisTemp == "F" ) {
    eAxisIterUserID = CalStats::FREQUENCY;
  } else if ( axisTemp == "T" ) {
    eAxisIterUserID = CalStats::TIME;
  } else {
    return( False );
  }


  // Return True

  return( True );

}

// --- //

Bool calanalysis::parseRAP( const std::string& ap, CalAnalysis::RAP& eRAP ) {

  // Convert the input string to an uppercase CASA string of length 1

  String apTemp( toCasaString( ap ) );
  apTemp.trim();
  if ( apTemp == "" ) apTemp = "A";
  apTemp.upcase(); apTemp = apTemp[0];


  // Get the (real, amplitude, phase) RAP

  eRAP = CalAnalysis::INIT;

  if ( partype() == "Float" ) {
    eRAP = CalAnalysis::REAL;
  } else if ( partype() == "Complex" ) {
    if ( apTemp == "A" ) {
      eRAP = CalAnalysis::AMPLITUDE;
    } else if ( apTemp == "P" ) {
      eRAP = CalAnalysis::PHASE;
    } else {
      return( False );
    }
  } else {
    return( False );
  }


  // Return True

  return( True );

}

// --- //

Bool calanalysis::parseJumpMax( const double& jumpmax, Double& dJumpMax ) {

  // Get the maximum phase jump

  dJumpMax = jumpmax;

  if ( dJumpMax < 0.0 ) return( False );


  // Return True

  return( True );

}

// --- //

Bool calanalysis::parseArg( const std::string& order, const std::string& type,
    const bool& weight, CalStats::ARG<CalStatsFitter::FIT>& oArg ) {

  // Parse the fit order string

  Bool bOrder = parseOrder( order, oArg.eOrder );

  if ( !bOrder ) {
    LogIO log( LogOrigin( "calanalysis", "parseArg()", WHERE ) );
    log << LogIO::WARN << "Invalid fit order" << LogIO::POST;
    return( False );
  }


  // Parse the fit type string

  Bool bType = parseType( type, oArg.eType );

  if ( !bType ) {
    LogIO log( LogOrigin( "calanalysis", "parseArg()", WHERE ) );
    log << LogIO::WARN << "Invalid fit type" << LogIO::POST;
    return( False );
  }


  // Parse the fit weight boolean

  Bool bWeight = parseWeight( weight, oArg.eWeight );

  if ( !bWeight ) {
    LogIO log( LogOrigin( "calanalysis", "parseArg()", WHERE ) );
    log << LogIO::WARN << "Invalid fit weight" << LogIO::POST;
    return( False );
  }


  // Return True

  return( True );

}

// --- //

Bool calanalysis::parseOrder( const std::string& order,
    CalStatsFitter::ORDER& eOrder ) {

  // Convert the input string to an uppercase CASA string of length 1

  String orderTemp( toCasaString( order ) );
  orderTemp.trim();
  if ( orderTemp == "" ) orderTemp = "A";
  orderTemp.upcase(); orderTemp = orderTemp[0];


  // Get the fit order

  eOrder = CalStatsFitter::ORDER_INIT;

  if ( orderTemp == "A" ) {
    eOrder = CalStatsFitter::AVERAGE;
  } else if ( orderTemp == "L" ) {
    eOrder = CalStatsFitter::LINEAR;
  } else if ( orderTemp == "Q" ) {
    eOrder = CalStatsFitter::QUADRATIC;
  } else {
    return( False );
  }


  // Return True

  return( True );

}

// --- //

Bool calanalysis::parseType( const std::string& type,
    CalStatsFitter::TYPE& eType ) {

  // Convert the input string to an uppercase CASA string of length 1

  String typeTemp( toCasaString( type ) );
  typeTemp.trim();
  if ( typeTemp == "" ) typeTemp = "L";
  typeTemp.upcase(); typeTemp = typeTemp[0];


  // Get the fit type

  eType = CalStatsFitter::TYPE_INIT;

  if ( typeTemp == "L" ) {
    eType = CalStatsFitter::LSQ;
  } else if ( typeTemp == "R" ) {
    eType = CalStatsFitter::ROBUST;
  } else {
    return( False );
  }


  // Return True

  return( True );

}

// --- //

Bool calanalysis::parseWeight( const bool& weight,
    CalStatsFitter::WEIGHT& eWeight ) {

  // Get the fit weight

  if ( weight ) {
    eWeight = CalStatsFitter::YES;
  } else {
    eWeight = CalStatsFitter::NO;
  }


  // Return True

  return( True );

}

#include <tools/calanalysis/calanalysis_template.h>

} // casac namespace
