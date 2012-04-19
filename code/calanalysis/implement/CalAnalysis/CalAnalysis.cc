
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
               name.
msName       - This member function returns the associated MS name.
visCal       - This member function returns the visibility calibration type.
parType      - This member function returns the parameter type ("Complex" or
               "Float").
polBasis     - This member function returns the polarization basis ("L" or "C").
field        - This member function returns the field numbers.
antenna      - This member function returns the antenna numbers.
time         - This member function returns the times.
feed         - This member function returns the feeds.
numspw       - This member function returns the number of spectral windows.
spw          - This member function returns the spectral windows.
numChannel   - This member function returns the number of channels for each
               spectral window.
freq         - This member function returns the frequencies for each spectral
               window.

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
calNameGet   - This member function gets the new format calibration table name
               from the new format calibration table.
calNameSet   - This member function sets the new format calibration table name
               private variable.
msNameGet    - This member function gets the associated MS name from the new
               format calibration table.
msNameSet    - This member function sets the associated MS name private
               variable.
visCalGet    - This member function gets the visibility calibration type from
               the new format calibration table.
visCalSet    - This member function sets the visibility calibration type private
               variable.
parTypeGet   - This member function gets the parameter type ("Complex" or
               "Float") from the new format calibration table.
parTypeSet   - This member function sets the parameter type ("Complex" or
               "Float") private variable.
polBasisGet  - This member function gets the polarization basis ("L" or "C")
               from the new format calibration table.
polBasisSet  - This member function sets the polarization basis ("L" or "C")
               private variable.
fieldGet     - This member function gets the field numbers from the new format
               calibration table.
fieldSet     - This member function sets the field numbers private variables.
fieldCheck   - This member function checks the input field vector and returns
               the fixed field vector.
antennaGet   - This member function gets the antenna numbers from the new format
               calibration table.
antennaSet   - This member function sets the antenna numbers private variables.
antennaCheck - This member function checks the input antenna vector and returns
               the fixed antenna vector.
timeGet      - This member function gets the times from the new format
               calibration table.
timeSet      - This member function sets the times private variables.
timeCheck    - This member function checks the time range and returns the
               corresponding time vector.
feedGet      - This member function gets the feeds from the new format
               calibration table.
feedSet      - This member function sets the feeds private variables.
feedCheck    - This member function checks the input feed vector and returns the
               fixed feed vector.
spwInfoGet   - This member function gets the spectral window information from
               the new format calibration table.
spwInfoSet   - This member function sets the spectral window information private
               variables.

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
2012 Apr 17 - Nick Elias, NRAO
              Nested class CalAnalysis::SPW_INFO() added.  Private member
              functions fieldGet(), fieldSet(), fieldCheck(), antennaGet(),
              antennaSet(), antennaCheck(), spwInfoGet(), and spwInfoSet()
              added.  Public member functions field(), antenna(), numChannel(),
              and freq() added.  Private member functions spwGet(), spwSet(),
              and spw_channel() removed.

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
2012 Apr 17 - Nick Elias, NRAO
              Called new member functions to make this member function shorter.

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


  // Set the private variables corresponding to the field vector, antenna
  // vector, time vector, feed vector, and spectral window information

  fieldSet( fieldGet( oTableName ) );
  antennaSet( antennaGet( oTableName ) );
  timeSet( timeGet( oTableName ) );

  feedSet( feedGet( oTableName ) );

  spwInfoSet( spwInfoGet( oTableName ) );


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
This member function returns the new format calibration table.

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
This member function returns the MS name.

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
This member function returns the visibility calibration type.

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
This member function returns the parameter type ("Complex" or "Float").

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
This member function returns the polarization basis ("L" or "C").

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

CalAnalysis::field

Description:
------------
This member function returns the field numbers.

Inputs:
-------
None.

Outputs:
--------
The reference to the Vector<uInt> instance containing the fields, returned via
the function value.

Modification history:
---------------------
2012 Apr 17 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

Vector<uInt>& CalAnalysis::field( void ) const {

  // Copy the private variable containing the field numbers and return it

  Vector<uInt>* poField = new Vector<uInt>( oField.copy() );

  return( *poField );

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::antenna

Description:
------------
This member function returns the antenna numbers.

Inputs:
-------
None.

Outputs:
--------
The reference to the Vector<uInt> instance containing the antenna numbers,
returned via the function value.

Modification history:
---------------------
2012 Apr 17 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

Vector<uInt>& CalAnalysis::antenna( void ) const {

  // Copy the private variable containing the antenna numbers and return it

  Vector<uInt>* poAntenna = new Vector<uInt>( oAntenna.copy() );

  return( *poAntenna );

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::time

Description:
------------
This member function returns the times.

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

  Vector<Double>* poTime = new Vector<Double>( oTime.copy() );

  return( *poTime );

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::feed

Description:
------------
This member function returns the feeds.

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

  Vector<String>* poFeed = new Vector<String>( oFeed.copy() );

  return( *poFeed );

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::numspw

Description:
------------
This member function returns the number of spectral windows.

Inputs:
-------
None.

Outputs:
--------
The reference to the uInt variable containing the spectral windows, returned via
the function value.

Modification history:
---------------------
2012 Apr 04 - Nick Elias, NRAO
              Initial version.
2012 Apr 17 - Nick Elias, NRAO
              Modified to handle the SPW_INFO instance.

*/

// -----------------------------------------------------------------------------

uInt& CalAnalysis::numspw( void ) const {

  // Copy the private variable containing the number of spectral windows and
  // return it

  uInt* puiNumSPW = new uInt( oSPWInfo.uiNumSPW );

  return( *puiNumSPW );

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::spw

Description:
------------
This member function returns the spectral windows.

Inputs:
-------
None.

Outputs:
--------
The reference to the Vector<uInt> instance containing the spectral windows,
returned via the function value.

Modification history:
---------------------
2012 Apr 04 - Nick Elias, NRAO
              Initial version.
2012 Apr 17 - Nick Elias, NRAO
              Modified to handle the SPW_INFO instance.

*/

// -----------------------------------------------------------------------------

Vector<uInt>& CalAnalysis::spw( void ) const {

  // Copy the private variable containing the spectral windows and return it

  Vector<uInt>* poSPW = new Vector<uInt>( oSPWInfo.oSPW.copy() );

  return( *poSPW );

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::numChannel

Description:
------------
This member function returns the number of channels for each spectral window.

Inputs:
-------
None.

Outputs:
--------
The array of pointers to the Vector<uInt> instances containing the number of
channels for each spectral window, returned via the function value.

Modification history:
---------------------
2012 Apr 17 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

Vector<uInt>& CalAnalysis::numChannel( void ) const {

  // Copy the private variable containing the number of channels for each
  // spectral window and return it

  Vector<uInt>* poNumChannel = new Vector<uInt>( oSPWInfo.oNumChannel.copy() );

  return( *poNumChannel );

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::freq

Description:
-----------
This member function returns the frequencies for each spectral window.

Inputs:
-------
None.

Outputs:
--------
The array of pointers to the Vector<uInt> instances containing the frequencies
for each spectral window, returned via the function value.

Modification history:
---------------------
2012 Apr 17 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

Vector<Vector<Double> > CalAnalysis::freq( void ) const {

  // Copy the private variable containing the frequencies for each spectral
  // window and return it

  uInt uiNumSPW = oSPWInfo.uiNumSPW;

  Vector<Vector<Double> > voFreq( uiNumSPW );
  for ( uInt s=0; s<uiNumSPW; s++ ) voFreq[s] = oSPWInfo.voFreq[s].copy();

  return( voFreq );

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

CalAnalysis::fieldGet

Description:
------------
This member function gets the field numbers from the new format calibration
table.

Inputs:
-------
oTableName - This reference to a String instance contains the new format
             calibration table name.

Outputs:
--------
The reference to the Vector<uInt> instance containing the field numbers,
returned via the function value.

Modification history:
---------------------
2012 Apr 17 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

Vector<uInt>& CalAnalysis::fieldGet( const String& oTableName ) {

  // Create a temporary field subtable instance and get the number of fields
  // (the number of rows)

  Table oTableField( oTableName+"/FIELD", Table::Old );
  uInt uiNumRow = oTableField.nrow();


  // Create the vector containing the field numbers and return it

  Vector<uInt>* poField = new Vector<uInt>( uiNumRow );
  indgen<uInt>( *poField, 0 );

  return( *poField );


}

// -----------------------------------------------------------------------------

/*

CalAnalysis::fieldSet

Description:
------------
This member function sets the field numbers private variables.

Inputs:
-------
oFieldIn - This reference to a Vector<uInt> instance contains the field numbers.

Outputs:
--------
None.

Modification history:
---------------------
2012 Apr 17 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

void CalAnalysis::fieldSet( const Vector<uInt>& oFieldIn ) {

  // Set the number of fields and fields and return

  uiNumField = oFieldIn.nelements();
  oField = Vector<uInt>( oFieldIn.copy() );

  return;

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::fieldCheck

Description:
------------
This member function checks the input field vector and returns the fixed field
vector.

Inputs:
-------
oFieldIn - This reference to a Vector<uInt> instance contains the field numbers.

Outputs:
--------
oFieldOut - This reference to the Vector<uInt> instance contains the checked and
            fixed field numbers.
The reference to the success Bool variable, returned via the function value.

Modification history:
---------------------
2012 Apr 17 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

Bool CalAnalysis::fieldCheck( const Vector<uInt>& oFieldIn,
    Vector<uInt>& oFieldOut ) const {

  // Check the input field numbers

  if ( oFieldIn.nelements() == 0 ) return( False );

  for ( uInt f=0; f<oFieldIn.nelements(); f++ ) {
    if ( !exists<uInt>( oFieldIn[f], oField ) ) return( False );
  }


  // Get the sorted unique field number vector

  if ( oFieldOut.nelements() != 0 ) oFieldOut.resize();
  oFieldOut = unique<uInt>( oFieldIn );


  // Return True

  return( True );

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::antennaGet

Description:
------------
This member function gets the antenna numbers from the new format calibration
table.

Inputs:
-------
oTableName - This reference to a String instance contains the new format
             calibration table name.

Outputs:
--------
The reference to the Vector<uInt> instance containing the antenna numbers,
returned via the function value.

Modification history:
---------------------
2012 Apr 17 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

Vector<uInt>& CalAnalysis::antennaGet( const String& oTableName ) {

  // Create a temporary antenna subtable instance and get the number of antennas
  // (the number of rows)

  Table oTableAntenna( oTableName+"/ANTENNA", Table::Old );
  uInt uiNumRow = oTableAntenna.nrow();


  // Create the vector containing the antenna numbers and return it

  Vector<uInt>* poAntenna = new Vector<uInt>( uiNumRow );
  indgen<uInt>( *poAntenna, 0 );

  return( *poAntenna );


}

// -----------------------------------------------------------------------------

/*

CalAnalysis::antennaSet

Description:
------------
This member function sets the antenna numbers private variables.

Inputs:
-------
oAntennaIn - This reference to a Vector<uInt> instance contains the antenna
             numbers.

Outputs:
--------
None.

Modification history:
---------------------
2012 Apr 17 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

void CalAnalysis::antennaSet( const Vector<uInt>& oAntennaIn ) {

  // Set the number of antennas and antennas and return

  uiNumAntenna = oAntennaIn.nelements();
  oAntenna = Vector<uInt>( oAntennaIn.copy() );

  return;

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::antennaCheck

Description:
------------
This member function checks the input antenna vector and returns the fixed
antenna vector.

Inputs:
-------
oAntennaIn - This reference to a Vector<uInt> instance contains the antenna
             numbers.

Outputs:
--------
oAntennaOut - This reference to the Vector<uInt> instance contains the checked
              and fixed antenna numbers.
The reference to the success Bool variable, returned via the function value.

Modification history:
---------------------
2012 Apr 17 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

Bool CalAnalysis::antennaCheck( const Vector<uInt>& oAntennaIn,
    Vector<uInt>& oAntennaOut ) const {

  // Check the input antenna numbers

  if ( oAntennaIn.nelements() == 0 ) return( False );

  for ( uInt a=0; a<oAntennaIn.nelements(); a++ ) {
    if ( !exists<uInt>( oAntennaIn[a], oAntenna ) ) return( False );
  }


  // Get the sorted unique antenna number vector

  if ( oAntennaOut.nelements() != 0 ) oAntennaOut.resize();
  oAntennaOut = unique<uInt>( oAntennaIn );


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

  if ( dStartTimeIn > dStopTimeIn ) return( False );
  if ( dStartTimeIn > oTime[uiNumTime-1] ) return( False );
  if ( dStopTimeIn < oTime[0] ) return( False );


  // Get the unique time values

  if ( oTimeOut.nelements() != 0 ) oTimeOut.resize();

  for ( uInt t=0,tOut=0; t<uiNumTime; t++ ) {
    if ( oTime[t] >= dStartTimeIn && oTime[t] <= dStopTimeIn ) {
      oTimeOut.resize( ++tOut, True );
      oTimeOut[tOut-1] = oTime[t];
    }
  }

  if ( oTimeOut.nelements() == 0 ) return( False );


  // Return True

  return( True );

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


  // Get the unique feed vector

  if ( oFeedOut.nelements() != 0 ) oFeedOut.resize();
  oFeedOut = unique<String>( oFeedIn );

  if ( oFeedOut.nelements() > 2 ) return( False );

  if ( oFeedOut.nelements() == 2 ) {
    if ( oFeedOut[0] == "L" && oFeedOut[1] == "R" ) {
      oFeedOut[0] = "R";
      oFeedOut[1] = "L";
    }
    if ( oFeedOut[0] == "Y" && oFeedOut[1] == "X" ) {
      oFeedOut[0] = "X";
      oFeedOut[1] = "Y";
    }
  }


  // Return True

  return( True );

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::spwInfoGet

Description:
------------
This member function gets the spectral window information from the new format
calibration table.

Inputs:
-------
oTableName - This reference to a String instance contains the new format
             calibration table name.

Outputs:
--------
The reference to the SPW_INFO instance containing the spws, returned via the
function value.

Modification history:
---------------------
2012 Apr 17 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

CalAnalysis::SPW_INFO& CalAnalysis::spwInfoGet( const String& oTableName ) {

  // Get the spectral window information and return it

  CalAnalysis::SPW_INFO* poSPWInfo;
  poSPWInfo = new CalAnalysis::SPW_INFO( oTableName );

  return( *poSPWInfo );

}

// -----------------------------------------------------------------------------

/*

CalAnalysis::spwInfoSet

Description:
------------
This member function sets the spectral window information private variables.

Inputs:
-------
oSPWInfoIn - This reference to the SPW_INFO instance contains the spectral
             window information.

Outputs:
--------
None.

Modification history:
---------------------
2012 Apr 17 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

void CalAnalysis::spwInfoSet( const CalAnalysis::SPW_INFO& oSPWInfoIn ) {

  // Set the spectral window information and return

  oSPWInfo = CalAnalysis::SPW_INFO( oSPWInfoIn );

  return;

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

// -----------------------------------------------------------------------------
// Start of CalAnalysis::SPW_INFO nested class public member functions
// -----------------------------------------------------------------------------

CalAnalysis::SPW_INFO::SPW_INFO( const String& oTableName ) {

  String oSPWName( oTableName + "/SPECTRAL_WINDOW" );
  CTSpectralWindow oCalSPW( oSPWName, Table::Old );
  ROArrayColumn<Double> oFreqACD( ROMSSpWindowColumns(oCalSPW).chanFreq() );

  bValid = True;

  uiNumSPW = oFreqACD.nrow();

  oSPW = Vector<uInt>( uiNumSPW );
  indgen<uInt>( oSPW, 0 );

  oNumChannel = Vector<uInt>( uiNumSPW );
  for ( uInt s=0; s<uiNumSPW; s++ ) oNumChannel[s] = oFreqACD( s ).nelements();

  voFreq = Vector<Vector<Double> >( uiNumSPW );
  for ( uInt s=0; s<uiNumSPW; s++ ) voFreq[s] = oFreqACD( s );

  return;

}

// --- //

CalAnalysis::SPW_INFO::SPW_INFO( const SPW_INFO& oSPWInfoIn ) {

  bValid = oSPWInfoIn.bValid;

  uiNumSPW = oSPWInfoIn.uiNumSPW;
  oSPW = oSPWInfoIn.oSPW.copy();

  oNumChannel = oSPWInfoIn.oNumChannel.copy();

  voFreq = Vector<Vector<Double> >( uiNumSPW );
  for ( uInt s=0; s<uiNumSPW; s++ ) voFreq[s] = oSPWInfoIn.voFreq[s].copy();

  return;

}

// --- //

CalAnalysis::SPW_INFO::SPW_INFO( void ) {
  bValid = False;
  uiNumSPW = 0;
  oSPW = Vector<uInt>();
  oNumChannel = Vector<uInt>();
  voFreq = Vector<Vector<Double> >();
  return;
}

// --- //

CalAnalysis::SPW_INFO::~SPW_INFO( void ) {}

// --- //

CalAnalysis::SPW_INFO& CalAnalysis::SPW_INFO::subset(
    const Vector<uInt>& oSPWIn, const Vector<uInt>& oStartChannelIn,
    const Vector<uInt>& oStopChannelIn ) {

  uInt uiNumSPWIn = oSPWIn.nelements();

  CalAnalysis::SPW_INFO* poSPWInfo = new CalAnalysis::SPW_INFO();

  Vector<uInt>* poSPWOut = new Vector<uInt>();
  if ( !spwCheck( oSPWIn, *poSPWOut ) ) {
    delete poSPWOut;
    return( *poSPWInfo );
  }
  if ( poSPWOut->nelements() != uiNumSPWIn ) {
    delete poSPWOut;
    return( *poSPWInfo );
  }
  delete poSPWOut;

  for ( uInt s=0; s<uiNumSPWIn; s++ ) {
    if ( oStartChannelIn[s] > oStopChannelIn[s] ) return( *poSPWInfo );
  }

  poSPWInfo->bValid = True;

  poSPWInfo->uiNumSPW = uiNumSPWIn;
  poSPWInfo->oSPW = oSPWIn.copy();

  poSPWInfo->oNumChannel = Vector<uInt>( uiNumSPWIn );
  for ( uInt s=0; s<uiNumSPWIn; s++ ) {
    poSPWInfo->oNumChannel[s] = oStopChannelIn[s] - oStartChannelIn[s] + 1;
  }

  poSPWInfo->voFreq = Vector<Vector<Double> >( uiNumSPWIn );
  for ( uInt s=0; s<uiNumSPWIn; s++ ) {
    poSPWInfo->voFreq[s] = Vector<Double>( poSPWInfo->oNumChannel[s] );
    for ( uInt c=oStartChannelIn[s]; c<=oStopChannelIn[s]; c++ ) {
      poSPWInfo->voFreq[s][c] = voFreq[oSPWIn[s]][c];
    }
  }

  return( *poSPWInfo );

}

// --- //

Bool& CalAnalysis::SPW_INFO::freq( const Vector<uInt>& oSPWIn,
    const Vector<uInt>& oStartChannelIn, const Vector<uInt>& oStopChannelIn,
    Vector<Double>& oFreqOut ) {

  CalAnalysis::SPW_INFO oSPWInfoSub(
      subset( oSPWIn, oStartChannelIn, oStopChannelIn ) );

  Bool *pbValid = new Bool;

  if ( !oSPWInfoSub.bValid ) {
    *pbValid = False;
    return( *pbValid );
  }

  if ( oFreqOut.nelements() != 0 ) oFreqOut.resize();

  for ( uInt s=0; s<oSPWInfoSub.uiNumSPW; s++ ) {
    uInt uiNumChannel0 = oFreqOut.nelements();
    uInt uiNumChannel = oSPWInfoSub.oNumChannel[s];
    oFreqOut.resize( uiNumChannel0+uiNumChannel, True );
    for ( uInt c=0; c<uiNumChannel; c++ ) {
      oFreqOut[c+uiNumChannel0] = oSPWInfoSub.voFreq[s][c];
    }
  }

  *pbValid = True;

  return( *pbValid );

}

// --- //

Bool CalAnalysis::SPW_INFO::spwCheck( const Vector<uInt>& oSPWIn,
    Vector<uInt>& oSPWOut ) {

  uInt uiNumSPWIn = oSPWIn.nelements();

  if ( uiNumSPWIn == 0 ) return( False );

  for ( uInt s=0; s<uiNumSPWIn; s++ ) {
    if ( !exists<uInt>( oSPWIn[s], oSPW ) ) return( False );
  }

  if ( oSPWOut.nelements() != 0 ) oSPWOut.resize();
  oSPWOut = unique<uInt>( oSPWIn );

  return( True );

}

// --- //

CalAnalysis::SPW_INFO& CalAnalysis::SPW_INFO::operator=(
    const CalAnalysis::SPW_INFO& oSPWInfoIn ) {

  if ( this != &oSPWInfoIn ) {
    bValid = oSPWInfoIn.bValid;
    uiNumSPW = oSPWInfoIn.uiNumSPW;
    oSPW = oSPWInfoIn.oSPW.copy();
    oNumChannel = oSPWInfoIn.oNumChannel.copy();
    voFreq = Vector<Vector<Double> >( uiNumSPW );
    for ( uInt s=0; s<uiNumSPW; s++ ) {
      voFreq[s] = oSPWInfoIn.voFreq[s].copy();
    }
  }

  return( *this );

}

// -----------------------------------------------------------------------------
// End of CalAnalysis::SPW_INFO nested class public member functions
// -----------------------------------------------------------------------------

};

// -----------------------------------------------------------------------------
// End of casa namespace
// -----------------------------------------------------------------------------
