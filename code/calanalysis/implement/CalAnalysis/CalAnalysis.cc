
// -----------------------------------------------------------------------------

/*

CalAnalysis.cc

Description:
------------
This file contains member functions for the CalAnalysis class.

Classes:
--------
CalAnalysis - This class acts as the interface between the ROCTIter and CalStats
              classes.

Modification history:
---------------------
2012 Jan 20 - Nick Elias, NRAO
              Initial version.
2012 Jan 25 - Nick Elias, NRAO
              Logging capability added.  Error checking added.
2012 Feb 14 - Nick Elias, NRAO
              Updated this code to reflect changes in NewCalTabIter (now
              ROCTIter) and other classes.

*/

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <calanalysis/CalAnalysis/CalAnalysis.h>

// -----------------------------------------------------------------------------
// Start of casa namespace
// -----------------------------------------------------------------------------

namespace casa {

// -----------------------------------------------------------------------------
// Start of CalAnalysis class
// -----------------------------------------------------------------------------

/*

CalAnalysis

Description:
------------
This class acts as the interface between the ROCTIter and CalAnalysis classes.

In a nutshell:
--------------
* The constructor gets the information from the new format calibration table.
* The stats<T>() function calculates statistics (the type depends on T) and
  returns the results.
  - The user can specify the field(s), antenna(s), time range, feeds, spectral
    windows, frequencies, and the second iteration axis for the CalStats class
    (time or frequency).
  - The inputs are checked and fixed, if possible.
  - The iteration loop goes over field, antenna1, and antenna2.  If a set of
    field, antenna1 and antenna2 numbers from an iteration is consistent with
    the inputs, statistics are calculated.  This is not the most efficient way,
    but the NewCalTable class doesn't have another way to access data and the
    time for each iteration is very fast.
  - For each iteration, the dimensions of the value, value error, and flag cubes
    provided by ROCTIter are feed x frequency(spw) x row(spw,time).  This shape
    is not useful for calculating statistics with CalStats, so the parse<T>()
    function slices and dices the cubes into feed x frequency x time.
  - The parsed cubes are further refined by the select<T>() function to include
    only the feeds, frequencies, and times selected by the input parameters.
  - The resulting cubes are fed to the CalStats class and its stats<T>()
    function calculates the desired statistics which are stored in a vector
    of OUTPUT<T> instances.

NB: There are a lot of get/set member functions.  Unfortunately, they could not
    be overloaded with the same names.

Nested classes:
---------------
OUTPUT<T> - This nested class contains the outputs for the
            CalAnalysis::stats<T>() template member function.

Class public member functions:
------------------------------
CalAnalysis  - This constructor gets information from the new format calibration
               table for further processing by the stats<T>() function.
~CalAnalysis - This destructor deallocates the internal memory of an instance.
calName      - This member function returns the new format calibration table
               name private variable.
msName       - This member function returns the associated MS name private
               variable.
visCal       - This member function returns the visibility calibration type
               private variable.
parType      - This member function returns the parameter type ("Complex" or
               "Float") private variable.
polBasis     - This member function returns the polarization basis ("L" or "C")
               private variable.
feed         - This member function returns the feeds private variable.
time         - This member function returns the times private variable.
spw          - This member function returns the spws private variable.
numspw       - This member function returns the number of spws private variable.

Class template public member functions:
---------------------------------------
stats<T> - This member function is the main user interface for calculating the
           statistics for all iterations.  Allowed T: CalStats::NONE only
           returns the input data, CalStatsFitter::FIT calculates fit
           statistics, and CalStatsHist::HIST calculates histogram statistics.

Class template public static member functions:
----------------------------------------------
exists<T> - This member function determines whether a value appears in a vector.
unique<T> - This member function returns a unique vector from an input vector.

Class private member functions:
-------------------------------
calNameGet  - This member function gets the new format calibration table name
              from the new format calibration table.
calNameSet  - This member function sets the new format calibration table name
              private variable.
msNameGet   - This member function gets the associated MS name from the new
              format calibration table.
msNameSet   - This member function sets the associated MS name private variable.
visCalGet   - This member function gets the visibility calibration type from the
              new format calibration table.
visCalSet   - This member function sets the visibility calibration type private
              variable.
parTypeGet  - This member function gets the parameter type ("Complex" or
              "Float") from the new format calibration table.
parTypeSet  - This member function sets the parameter type ("Complex" or
              "Float") private variable.
polBasisGet - This member function gets the polarization basis ("L" or "C") from
              the new format calibration table.
polBasisSet - This member function sets the polarization basis ("L" or "C")
              private variable.
feedGet     - This member function gets the feeds from the new format
              calibration table.
feedSet     - This member function sets the feeds private variables.
feedCheck   - This member function checks the input feed vector and returns the
              fixed feed vector.
timeGet     - This member function gets the times from the new format
              calibration table.
timeSet     - This member function sets the times private variables.
timeCheck   - This member function checks the time range and returns the
              corresponding time vector.
spwGet      - This member function gets the spws from the new format calibration
              table.
spwSet      - This member function sets the spws private variables.
spw_channel - This member functions checks the input spectral window and channel
              vectors and returns the fixed spectral window and channel vectors.
freq        - This member function creates the total frequency vector based on
              the spectral window and channel vectors.

Class template private member functions:
----------------------------------------
parse<T>  - This member function reshapes the cubes provided by class
            ROCTIter to dimensions required by class CalStats.
select<T> - This member function selects the desired feeds, frequencies, and
            times from an input cube.

Class protected member functions:
---------------------------------
CalAnalysis - This default constructor is unused by this class and unavailable
              when an instance is created.
CalAnalysis - This copy constructor is unused by this class and unavailable when
              an instance is created.
operator=   - This operator= function is unused by this class and unavailable
              when an instance is created.

Modification history:
---------------------
2012 Jan 20 - Nick Elias, NRAO
              Initial version created with public member functions CalAnalysis()
              (generic), ~CalAnalysis(); template static public member function
              stats<T>(); template public member functions exists<T>() and
              unique<T>(); private member functions tableType(), polBasisGet(),
              feedCheck(), timeCheck(), spw_channel(), freq(); template private
              member functions parse<T>(), and select<T>(); and protected member
	      functions CalAnalysis() (default), CalAnalysis() (copy), and
              operator=().
2012 Feb 14 - Nick Elias, NRAO
              Updated this code to reflect changes in NewCalTabIter (now
              ROCTIter) and other classes.  Added the RAP enum.
2012 Mar 13 - Nick Elias, NRAO
              Public member function tableType() renamed to parTypeGet().
              Private member functions msNameGet() and visCalGet() added.
2012 Mar 14 - Nick Elias, NRAO
              Spectral window ID, start channel, and stop channel added to the
              nested OUTPUT<T> class.
2012 Apr 03 - Nick Elias, NRAO
              Private member function calNameGet() added.  Public member
              functions calName(), msName(), visCal(), parType(), and polBasis()
              added.
2012 Apr 04 - Nick Elias, NRAO
              Private member functions calNameSet(), msNameSet(), visCalSet(),
              parTypeSet(), polBasisSet(), feedGet(), feedSet(), timeGet(),
              timeSet(), spwGet(), and spwSet() added.  Public member functions
              feed(), time(), spw(), and numspw() added.

*/

// -----------------------------------------------------------------------------
// Start of CalAnalysis public member functions
// -----------------------------------------------------------------------------

/*

CalAnalysis::CalAnalysis (generic)

Description:
------------
This constructor gets information from the new format calibration table for
further processing by the stats<T>() function.

Inputs:
-------
oTableName - This reference to a String instance contains the new format
             calibration table name.

Outputs:
--------
None.

Modification history:
---------------------
2012 Jan 20 - Nick Elias, NRAO
              Initial version.
2012 Jan 25 - Nick Elias, NRAO
              Error checking added.

*/

// -----------------------------------------------------------------------------

CalAnalysis::CalAnalysis( const String& oTableName ) {

  // Reconstitute the new format calibration table in memory

  try {
    poNCT = new NewCalTable( oTableName, Table::Old, Table::Memory );
  }
  catch ( AipsError oAE ) {
    throw( oAE );
  }


  // Create the iterator for the new format calibration table

  Block<String> oColIter( 3 );
  oColIter[0] = String( "ANTENNA2" );
  oColIter[1] = String( "ANTENNA1" );
  oColIter[2] = String( "FIELD_ID" );

  poNCTIter = new ROCTIter( *poNCT, oColIter );
  poNCTIter->reset();


  // Set the private variables corresponding to the new format calibration
  // table name, MS name, visibility calibration type, parameter type, and
  // polarization basis

  calNameSet( calNameGet( oTableName ) );

  msNameSet( msNameGet( oTableName ) );
  visCalSet( visCalGet( oTableName ) );
  parTypeSet( parTypeGet( oTableName ) );
  polBasisSet( polBasisGet( oTableName ) );


  // Set the private variables correspnding the complete feed vector, the
  // complete time vector, the complete spectral window vector, xxx

  feedSet( feedGet( oTableName ) );
  timeSet( timeGet( oTableName ) );

  spwSet( spwGet( oTableName ) );


  // Get the CHAN_FREQ column from the spectral window subtable

  String oSPWName( oTableName + "/SPECTRAL_WINDOW" );
  CTSpectralWindow oCalSPW( oSPWName, Table::Old );

  ROArrayColumn<Double> oFreqACD( ROMSSpWindowColumns(oCalSPW).chanFreq() );


  // Get the number of frequencies for each spectral window and the total number
  // of frequencies

  oNumFreq = Vector<uInt>( uiNumSPW );
  for ( uInt s=0; s<uiNumSPW; s++ ) oNumFreq[s] = oFreqACD(s).nelements();

  uiNumFreq = 0;
  for ( uInt s=0; s<uiNumSPW; s++ ) uiNumFreq += oNumFreq[s];


  // Create the total frequency vector

  oFreq = Vector<Double>( uiNumFreq );

  for ( uInt s=0,fMin=0,fMax=0; s<uiNumSPW; s++ ) {

    if ( s > 0 ) fMin += oNumFreq[s-1];
    fMax += oNumFreq[s];

    Vector<Double> oFreqTemp( oFreqACD(s) );
    for ( uInt f=fMin; f<fMax; f++ ) oFreq[f] = oFreqTemp[f-fMin];

  }


  // Return

  return;

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::~CalAnalysis

Description:
------------
This destructor deallocates the internal memory of an instance.

Inputs:
-------
None.

Outputs:
--------
None.

Modification history:
---------------------
2012 Jan 20 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

CalAnalysis::~CalAnalysis( void ) {

  // Deallocate the memory of the private variables

  delete poNCT;
  delete poNCTIter;


  // Return

  return;

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::calName

Description:
------------
This member function returns the new format calibration table name private
variable.

Inputs:
-------
None.

Outputs:
--------
The reference to the String instance containing the new format calibration table
name, returned via the function value.

Modification history:
---------------------
2012 Apr 03 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

String& CalAnalysis::calName( void ) const {

  // Copy the private variable containing the new format calibration table name
  // and return it

  String* poCalName = new String( oCalName );

  return( *poCalName );

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::msName

Description:
------------
This member function returns the MS name private variable.

Inputs:
-------
None.

Outputs:
--------
The reference to the String instance containing the MS name, returned via the
function value.

Modification history:
---------------------
2012 Apr 03 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

String& CalAnalysis::msName( void ) const {

  // Copy the private variable containing the MS name and return it

  String* poMSName = new String( oMSName );

  return( *poMSName );

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::visCal

Description:
------------
This member function returns the visibility calibration type private variable.

Inputs:
-------
None.

Outputs:
--------
The reference to the String instance containing the visibility calibration type,
returned via the function value.

Modification history:
---------------------
2012 Apr 03 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

String& CalAnalysis::visCal( void ) const {

  // Copy the private variable containing the visibility calibration type and
  // return it

  String* poVisCal = new String( oVisCal );

  return( *poVisCal );

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::parType

Description:
------------
This member function returns the parameter type ("Complex" or "Float") private
variable.

Inputs:
-------
None.

Outputs:
--------
The reference to the String instance containing the parameter type, returned via
the function value.

Modification history:
---------------------
2012 Apr 03 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

String& CalAnalysis::parType( void ) const {

  // Copy the private variable containing the parameter type and return it

  String* poParType = new String( oParType );

  return( *poParType );

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::polBasis

Description:
------------
This member function returns the polarization basis ("L" or "C") private
variable.

Inputs:
-------
None.

Outputs:
--------
The reference to the String instance containing the polarization basis, returned
via the function value.

Modification history:
---------------------
2012 Apr 03 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

String& CalAnalysis::polBasis( void ) const {

  // Copy the private variable containing the polarization basis and return it

  String* poPolBasis = new String( oPolBasis );

  return( *poPolBasis );

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::feed

Description:
------------
This member function returns the feeds private variable.

Inputs:
-------
None.

Outputs:
--------
The reference to the Vector<String> instance containing the feeds, returned via
the function value.

Modification history:
---------------------
2012 Apr 04 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

Vector<String>& CalAnalysis::feed( void ) const {

  // Copy the private variable containing the feeds and return it

  Vector<String>* poFeed = new Vector<String>( oFeed );

  return( *poFeed );

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::spw

Description:
------------
This member function returns the spws private variable.

Inputs:
-------
None.

Outputs:
--------
The reference to the Vector<uInt> instance containing the spws, returned via
the function value.

Modification history:
---------------------
2012 Apr 04 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

Vector<uInt>& CalAnalysis::spw( void ) const {

  // Copy the private variable containing the spws and return it

  Vector<uInt>* poSPW = new Vector<uInt>( oSPW );

  return( *poSPW );

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::numspw

Description:
------------
This member function returns the number of spws private variable.

Inputs:
-------
None.

Outputs:
--------
The reference to the uInt variable containing the spws, returned via the
function value.

Modification history:
---------------------
2012 Apr 04 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

uInt& CalAnalysis::numspw( void ) const {

  // Copy the private variable containing the number of spws and return it

  uInt* puiNumSPW = new uInt( uiNumSPW );

  return( *puiNumSPW );

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::time

Description:
------------
This member function returns the times private variable.

Inputs:
-------
None.

Outputs:
--------
The reference to the Vector<Double> instance containing the times, returned via
the function value.

Modification history:
---------------------
2012 Apr 04 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

Vector<Double>& CalAnalysis::time( void ) const {

  // Copy the private variable containing the times and return it

  Vector<Double>* poTime = new Vector<Double>( oTime );

  return( *poTime );

}

// -----------------------------------------------------------------------------
// End of CalAnalysis public member functions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Start of CalAnalysis private member functions
// -----------------------------------------------------------------------------

/*

CalAnalysis::calNameGet

Description:
------------
This member function gets the new format calibration table name from the new
format calibration table.

NB: This function is somewhat trivial, but it is included because of the other
get member functions of this class.

Inputs:
-------
oTableName - This reference to a String instance contains the new format
             calibration table name.

Outputs:
--------
The reference to the String instance containing the new format calibration table
name, returned via the function value.

Modification history:
---------------------
2012 Apr 03 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

String& CalAnalysis::calNameGet( const String& oTableName ) {

  // Get the new format calibration table name and return it

  String* poTableName = new String( oTableName );

  return( *poTableName );

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::calNameSet

Description:
------------
This member function sets the new format calibration table name private
variable.

Inputs:
-------
oCalNameIn - This reference to a String instance contains the new format
             calibration table name.

Outputs:
--------
None.

Modification history:
---------------------
2012 Apr 04 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

void CalAnalysis::calNameSet( const String& oCalNameIn ) {

  // Set the new format calibration table name and return

  oCalName = String( oCalNameIn );

  return;

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::msNameGet

Description:
------------
This member function gets the associated MS name from the new format calibration
table.

Inputs:
-------
oTableName - This reference to a String instance contains the new format
             calibration table name.

Outputs:
--------
The reference to the String instance containing the MS name, returned via the
function value.

Modification history:
---------------------
2012 Mar 13 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

String& CalAnalysis::msNameGet( const String& oTableName ) {

  // Get the record containing the main table keywords

  Table oTable( oTableName );
  TableProxy oTP( oTable );
  Record oR( oTP.getKeywordSet( String("") ) );


  // Get the associated MS name and return it

  uInt uiIndex = oR.fieldNumber( String( "MSName" ) );
  String* poMSName = new String( RecordFieldPtr<String>(oR,uiIndex).get() );

  return( *poMSName );

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::msNameSet

Description:
------------
This member function sets the associated MS name private variable.

Inputs:
-------
oMSNameIn - This reference to a String instance contains the assciated MS name.

Outputs:
--------
None.

Modification history:
---------------------
2012 Apr 04 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

void CalAnalysis::msNameSet( const String& oMSNameIn ) {

  // Set the assciated MS name and return

  oMSName = String( oMSNameIn );

  return;

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::visCalGet

Description:
------------
This member function gets the visibility calibration type from the new format
calibration table.

Inputs:
-------
oTableName - This reference to a String instance contains the new format
             calibration table name.

Outputs:
--------
The reference to the String instance containing the visibility calibration type
instance, returned via the function value.

Modification history:
---------------------
2012 Mar 13 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

String& CalAnalysis::visCalGet( const String& oTableName ) {

  // Get the record containing the main table keywords

  Table oTable( oTableName );
  TableProxy oTP( oTable );
  Record oR( oTP.getKeywordSet( String("") ) );


  // Get the visibility calibration type and return it

  uInt uiIndex = oR.fieldNumber( String( "VisCal" ) );
  String* poVisCal = new String( RecordFieldPtr<String>(oR,uiIndex).get() );

  return( *poVisCal );

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::visCalSet

Description:
------------
This member function sets the visibility calibration type private variable.

Inputs:
-------
oVisCalIn - This reference to a String instance contains the visibility
            calibration type.

Outputs:
--------
None.

Modification history:
---------------------
2012 Apr 04 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

void CalAnalysis::visCalSet( const String& oVisCalIn ) {

  // Set the visibility calibration type and return

  oVisCal = String( oVisCalIn );

  return;

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::parTypeGet

Description:
------------
This member function gets the parameter column type ("Complex" or "Float") from
the new format calibration table.

Inputs:
-------
oTableName - This reference to a String instance contains the new format
             calibration table name.

Outputs:
--------
The reference to the String instance containing the parameter column type,
returned via the function value.

Modification history:
---------------------
2012 Jan 20 - Nick Elias, NRAO
              Initial version.
2012 Mar 13 - Nick Elias, NRAO
              Function renamed to parType().

*/

// -----------------------------------------------------------------------------

String& CalAnalysis::parTypeGet( const String& oTableName ) {

  // Get the record containing the main table keywords

  Table oTable( oTableName );
  TableProxy oTP( oTable );
  Record oR( oTP.getKeywordSet( String("") ) );


  // Get the parameter column type and return it

  uInt uiIndex = oR.fieldNumber( String( "ParType" ) );
  String* poParType = new String( RecordFieldPtr<String>(oR,uiIndex).get() );

  return( *poParType );

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::parTypeSet

Description:
------------
This member function sets the parameter column type ("Complex" or "Float")
private variable.

Inputs:
-------
oParTypeIn - This reference to a String instance contains the parameter column
             type.

Outputs:
--------
None.

Modification history:
---------------------
2012 Apr 04 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

void CalAnalysis::parTypeSet( const String& oParTypeIn ) {

  // Set the parameter column type and return

  oParType = String( oParTypeIn );

  return;

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::polBasisGet

Description:
------------
This member function gets the polarization basis ("L" or "C") from the new
format calibration table.

Inputs:
-------
oTableName - This reference to a String instance contains the new format
             calibration table name.

Outputs:
--------
The reference to the String instance containing the polarization basis, returned
via the function value.

Modification history:
---------------------
2012 Jan 20 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

String& CalAnalysis::polBasisGet( const String& oTableName ) {

  // Get the record containing the main table keywords

  Table oTable( oTableName );
  TableProxy oTP( oTable );
  Record oR( oTP.getKeywordSet( String("") ) );


  // Get the polarization basis, make it upper case, keep only the initial
  // letter, and return it

  uInt uiIndex = oR.fieldNumber( String( "PolBasis" ) );
  String* poPolBasis = new String( RecordFieldPtr<String>(oR,uiIndex).get() );

  poPolBasis->upcase();
  poPolBasis->operator=( poPolBasis->operator[](0) );

  return( *poPolBasis );

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::polBasisSet

Description:
------------
This member function sets the polarization basis ("L" or "C") private variable.

Inputs:
-------
oPolBasisIn - This reference to a String instance contains the polarization
              basis.

Outputs:
--------
None.

Modification history:
---------------------
2012 Apr 04 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

void CalAnalysis::polBasisSet( const String& oPolBasisIn ) {

  // Set the polarization basis and return

  oPolBasis = String( oPolBasisIn );

  return;

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::feedGet

Description:
------------
This member function gets the feeds from the new format calibration table.

Inputs:
-------
oTableName - This reference to a String instance contains the new format
             calibration table name.

Outputs:
--------
The reference to the Vector<String> instance containing the feeds, returned via
the function value.

Modification history:
---------------------
2012 Apr 04 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

Vector<String>& CalAnalysis::feedGet( const String& oTableName ) {

  // Create a temporary new format calibration table instance and its iterator
  // instance

  NewCalTable oNCT( oTableName, Table::Old, Table::Memory );

  Block<String> oColIter( 3 );
  oColIter[0] = String( "ANTENNA2" );
  oColIter[1] = String( "ANTENNA1" );
  oColIter[2] = String( "FIELD_ID" );

  ROCTIter oNCTIter( oNCT, oColIter );
  oNCTIter.reset();


  // Get the feeds from the new format calibration table and return them

  uInt uiNumFeedTemp = oNCTIter.cparam().shape()[0];
  Vector<String>* poFeed = new Vector<String>( uiNumFeedTemp, "" );

  if ( uiNumFeedTemp == 1 ) {
    poFeed->operator[](0) = "S";
  } else {
    if ( polBasisGet(oTableName) == 'L' ) {
      poFeed->operator[](0) = "X";
      poFeed->operator[](1) = "Y";
    } else {
      poFeed->operator[](0) = "R";
      poFeed->operator[](1) = "L";
    }
  }

  return( *poFeed );

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::feedSet

Description:
------------
This member function sets the feeds private variables.

Inputs:
-------
oFeedIn - This reference to a Vector<String> instance contains the feeds.

Outputs:
--------
None.

Modification history:
---------------------
2012 Apr 04 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

void CalAnalysis::feedSet( const Vector<String>& oFeedIn ) {

  // Set the number of feeds and feeds and return

  uiNumFeed = oFeedIn.nelements();
  oFeed = Vector<String>( oFeedIn.copy() );

  return;

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::feedCheck

Description:
------------
This member function checks the input feed vector and returns the fixed feed
vector.

Inputs:
-------
oFeedIn - This reference to a Vector<String> instance contains the feeds ("R"
          and "L" for circular, "X" and "Y" for linear).

Outputs:
--------
oFeedOut - This reference to the Vector<String> instance contains the checked
           and fixed feeds.
The reference to the success Bool variable, returned via the function value.

Modification history:
---------------------
2012 Jan 20 - Nick Elias, NRAO
              Initial version.
2012 Mar 13 - Nick Elias, NRAO
              Now checking for null.

*/

// -----------------------------------------------------------------------------

Bool CalAnalysis::feedCheck( const Vector<String>& oFeedIn,
    Vector<String>& oFeedOut ) const {

  // Check the input feed values

  if ( oFeedIn.nelements() == 0 ) return( False );

  for ( uInt f=0; f<oFeedIn.nelements(); f++ ) {
    if ( oFeedIn[f] == String("") ) return( False );
    if ( !exists<String>( oFeedIn[f], oFeed ) ) return( False );
  }


  // Get the sorted unique feed vector

  if ( oFeedOut.nelements() != 0 ) oFeedOut.resize();
  oFeedOut = unique<String>( oFeedIn );

  if ( oFeedOut.nelements() > 2 ) return( False );

  if ( oFeedOut.nelements() == 2 ) {
    if ( oFeedOut[0] == "L" && oFeedOut[1] == "R" ) {
      oFeedOut[0] = "R";
      oFeedOut[1] = "L";
    }
  }


  // Return True

  return( True );

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::timeGet

Description:
------------
This member function gets the times from the new format calibration table.

Inputs:
-------
oTableName - This reference to a String instance contains the new format
             calibration table name.

Outputs:
--------
The reference to the Vector<Double> instance containing the times, returned via
the function value.

Modification history:
---------------------
2012 Apr 04 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

Vector<Double>& CalAnalysis::timeGet( const String& oTableName ) {

  // Create a temporary new format calibration table instance and its iterator
  // instance

  NewCalTable oNCT( oTableName, Table::Old, Table::Memory );

  Block<String> oColIter( 3 );
  oColIter[0] = String( "ANTENNA2" );
  oColIter[1] = String( "ANTENNA1" );
  oColIter[2] = String( "FIELD_ID" );

  ROCTIter oNCTIter( oNCT, oColIter );
  oNCTIter.reset();


  // Get the times from the new format calibration table and return them

  Vector<Double>* poTime =
      new Vector<Double>( unique<Double>( oNCTIter.time() ) );

  return( *poTime );

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::timeSet

Description:
------------
This member function sets the times private variables.

Inputs:
-------
oTimeIn - This reference to a Vector<Double> instance contains the times.

Outputs:
--------
None.

Modification history:
---------------------
2012 Apr 04 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

void CalAnalysis::timeSet( const Vector<Double>& oTimeIn ) {

  // Set the number of times and times and return

  uiNumTime = oTimeIn.nelements();
  oTime = Vector<Double>( oTimeIn.copy() );

  return;

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::timeCheck

Description:
------------
This member function checks the time range and returns the corresponding time
vector.

Inputs:
-------
dStartTimeIn - This reference to a Double variable contains the start time.
dStopTimeIn  - This reference to a Double variable contains the stop time.

Outputs:
--------
oTimeOut - This reference to the Vector<Double> instance containing the time
           stamps.
The reference to the success Bool variable, returned via the function value.

Modification history:
---------------------
2012 Jan 20 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

Bool CalAnalysis::timeCheck( const Double& dStartTimeIn,
    const Double& dStopTimeIn, Vector<Double>& oTimeOut ) const {

  // Check the start and stop times

  if ( dStartTimeIn > oTime[uiNumTime-1] || dStopTimeIn < oTime[0] ) {
    return( False );
  }


  // Get the unique and unsorted time values

  if ( oTimeOut.nelements() != 0 ) oTimeOut.resize();

  for ( uInt t=0,tOut=0; t<uiNumTime; t++ ) {
    if ( oTime[t] >= dStartTimeIn && oTime[t] <= dStopTimeIn ) {
      oTimeOut.resize( ++tOut, True );
      oTimeOut[tOut-1] = oTime[t];
    }
  }


  // Return True

  return( True );

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::spwGet

Description:
------------
This member function gets the spws from the new format calibration table.

Inputs:
-------
oTableName - This reference to a String instance contains the new format
             calibration table name.

Outputs:
--------
The reference to the Vector<uInt> instance containing the spws, returned via
the function value.

Modification history:
---------------------
2012 Apr 04 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

Vector<uInt>& CalAnalysis::spwGet( const String& oTableName ) {

  // Create a temporary new format calibration table instance and its iterator
  // instance

  NewCalTable oNCT( oTableName, Table::Old, Table::Memory );

  Block<String> oColIter( 3 );
  oColIter[0] = String( "ANTENNA2" );
  oColIter[1] = String( "ANTENNA1" );
  oColIter[2] = String( "FIELD_ID" );

  ROCTIter oNCTIter( oNCT, oColIter );
  oNCTIter.reset();


  // Get the spws from the new format calibration table, convert them from Int
  // to uInt, and return them

  Vector<Int> oSPWInt( unique<Int>( poNCTIter->spw() ) );

  Vector<uInt>* poSPW = new Vector<uInt>( oSPWInt.nelements() );
  convertArray<uInt,Int>( *poSPW, oSPWInt );

  return( *poSPW );

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::spwSet

Description:
------------
This member function sets the spws private variables.

Inputs:
-------
oSPWIn - This reference to a Vector<uInt> instance contains the spws.

Outputs:
--------
None.

Modification history:
---------------------
2012 Apr 04 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

void CalAnalysis::spwSet( const Vector<uInt>& oSPWIn ) {

  // Set the number of spws and spws and return

  uiNumSPW = oSPWIn.nelements();
  oSPW = Vector<uInt>( oSPWIn.copy() );

  return;

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::spw_channel

Description:
------------
This member functions checks the input spectral window and start/stop channel
vectors and returns the fixed spectral window and channel vectors.

Inputs:
-------
oSPWIn           - This reference to a Vector<uInt> instance contains the
                   spectral window IDs.
aoStartChannelIn - This reference to a Vector<uInt> instance contains the start
                   channels for the spectral windows.
aoStopChannelIn  - This reference to a Vector<uInt> instance contains the stop
                   channels for the spectral windows.

Outputs:
--------
oSPWIn       - This Vector<uInt> instance contains the checked spectral window
               IDs.
aoChannelOut - This array of Vector<uInt> instances containing the checked
               channel numbers.  Each element of the array corresponds to an
               element of oSPWIn.
The reference to the success Bool variable, returned via the function value.

Modification history:
---------------------
2012 Jan 20 - Nick Elias, NRAO
              Initial version.
2012 Mar 14 - Nick Elias, NRAO
              I replaced the array of vectors containing the channel numbers
              with an array of start channels and an array of stop channels (the
              index corresponds to spectral window).

*/

// -----------------------------------------------------------------------------

Bool CalAnalysis::spw_channel( const Vector<uInt>& oSPWIn,
    const Vector<uInt>& aoStartChannelIn, const Vector<uInt>& aoStopChannelIn,
    Vector<uInt>& oSPWOut, Vector<uInt>* aoChannelOut ) const {

  // Check the input spectral window and start/stop channel values

  uInt uiNumSPWIn = oSPWIn.nelements();
  if ( uiNumSPWIn == 0 ) return( False );

  for ( uInt s=0; s<uiNumSPWIn; s++ ) {
    if ( !exists<uInt>( oSPWIn[s], oSPW ) ) return( False );
    if ( aoStartChannelIn[s] > aoStopChannelIn[s] ) return( False );
  }


  // Get the unique and unsorted spectral window values.  If there are duplicate
  // values, return False.

  if ( oSPWOut.nelements() != 0 ) oSPWOut.resize();
  oSPWOut = unique<uInt>( oSPWIn );

  uInt uiNumSPWOut = oSPWOut.nelements();
  if ( uiNumSPWOut != uiNumSPWIn ) return( False );


  // Get the unique and unsorted channel values

  if ( aoChannelOut == NULL ) {
    aoChannelOut = new Vector<uInt> [uiNumSPWOut];
  }

  for ( uInt s=0; s<uiNumSPWOut; s++ ) {
    uInt uiNumChannel = aoStopChannelIn[s] - aoStartChannelIn[s] + 1;
    aoChannelOut[s].resize( uiNumChannel, False );
    indgen<uInt>( aoChannelOut[s], aoStartChannelIn[s] );
  }


  // Return True

  return( True );

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::freq

Description:
------------
This member function creates the total frequency vector based on the spectral
window and channel vectors.

NB: The spectral window and channel vectors should already be checked by the
CalAnalysis::spw_channel() function before feeding them to this function.

Inputs:
-------
oSPWIn      - This reference to a Vector<uInt> instance contains the spectral
              window IDs.
aoChannelIn - This array of Vector<uInt> instances containing the channel
              numbers.  Each element of the array corresponds to an element of
              oSPWIn.

Outputs:
--------
oFreqOut - This reference to the Vector<Double> instance contains the
           frequencies.
The reference to the success Bool variable, returned via the function value.

Modification history:
---------------------
2012 Jan 20 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

Bool CalAnalysis::freq( const Vector<uInt>& oSPWIn,
    const Vector<uInt>* const aoChannelIn, Vector<Double>& oFreqOut ) const {

  // Check the input spectral window and channel values

  uInt uiNumSPWIn = oSPWIn.nelements();
  if ( uiNumSPWIn == 0 ) return( False );

  for ( uInt s=0; s<uiNumSPWIn; s++ ) {

    if ( !exists<uInt>( oSPWIn[s], oSPW ) ) return( False );

    for ( uInt c=0; c<aoChannelIn[s].nelements(); c++ ) {
      if ( aoChannelIn[s][c] >= oNumFreq[oSPWIn[s]] ) return( False );
    }

  }


  // Get the frequency values

  if ( oFreqOut.nelements() != 0 ) oFreqOut.resize();
  uInt uiNumFreqOut = 0;

  for ( uInt s=0; s<uiNumSPWIn; s++ ) {

    uInt fStart = 0;
    for ( uInt s2=0; s2<oSPWIn[s]; s2++ ) fStart += oNumFreq[s2];

    for ( uInt c=0; c<aoChannelIn[s].nelements(); c++ ) {
      oFreqOut.resize( ++uiNumFreqOut, True );
      oFreqOut[uiNumFreqOut-1] = oFreq[aoChannelIn[s][c]+fStart];
    }

  }


  // Return True

  return( True );

}

// -----------------------------------------------------------------------------
// End of CalAnalysis private member functions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Start of CalAnalysis protected member functions
// -----------------------------------------------------------------------------

/*

CalAnalysis::CalAnalysis (default)

Description:
------------
This default constructor is unused by this class and unavailable when an
instance is created.

Inputs:
-------
None.

Outputs:
--------
None.

Modification history:
---------------------
2012 Jan 20 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

CalAnalysis::CalAnalysis( void ) {}

// -----------------------------------------------------------------------------

/*

CalAnalysis::CalAnalysis (copy)

Description:
------------
This copy constructor is unused by this class and unavailable when an instance
is created.

Inputs:
-------
oCalAnalysis - A reference to a CalAnalysis instance.

Outputs:
--------
None.

Modification history:
---------------------
2012 Jan 20 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

CalAnalysis::CalAnalysis( const CalAnalysis& oCalAnalysis ) {

  // Overwrite this instance and return.  This code will bomb.  I have written
  // it in this way to keep the compiler from spewing warning messages about
  // unused variables.

  *this = oCalAnalysis;

  return;

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::operator=

Description:
------------
This operator= function is unused by this class and unavailable when an instance
is created.

Inputs:
-------
oCalAnalysis - A reference to a CalAnalysis instance.

Outputs:
--------
None.

Modification history:
---------------------
2012 Jan 20 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

CalAnalysis& CalAnalysis::operator=( const CalAnalysis& oCalAnalysis ) {

  // Copy the input instance and return it.  This code will bomb.  I have
  // written it in this way to keep the compiler from spewing warning messages
  // about unused variables.

  CalAnalysis* poCalAnalysis = new CalAnalysis( oCalAnalysis );

  return( *poCalAnalysis );

}

// -----------------------------------------------------------------------------
// End of CalAnalysis protected member functions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// End of CalAnalysis class
// -----------------------------------------------------------------------------

};

// -----------------------------------------------------------------------------
// End of casa namespace
// -----------------------------------------------------------------------------
