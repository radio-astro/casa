
// -----------------------------------------------------------------------------

/*

CalStatsAXES.cc

Description:
------------
This file contains member functions for the CalStats::AXES class.

Classes:
--------
CalStats::AXES - This nested class contains the axes for the CalStats class.

Modification history:
---------------------
2011 Dec 23 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <calanalysis/CalAnalysis/CalStats.h>

// -----------------------------------------------------------------------------
// Start of casa namespace
// -----------------------------------------------------------------------------

namespace casa {

// -----------------------------------------------------------------------------
// Start of CalStats::AXES class
// -----------------------------------------------------------------------------

/*

CalStats::AXESs

Description:
------------
This nested class contains the axes for the CalStats class.

Class public member functions:
------------------------------
AXES      - This constructor is the default that initializes variables.
AXES      - This construtor copies the input instance to the present instance.
~AXES     - This destructor destroys the instance.
operator= - This function sets one instance to another.

Modification history:
---------------------
2011 Dec 23 - Nick Elias, NRAO
              Initial version containing AXES() (default), AXES() (copy),
              ~AXES(), and operator=().

*/

// -----------------------------------------------------------------------------
// Start of CalStats::AXES public member functions
// -----------------------------------------------------------------------------

/*

CalStats::AXES::AXES (default)

Description:
------------
This constructor is the default that initializes variables.

Inputs:
-------
None.

Outputs:
--------
None.

Modification history:
---------------------
2011 Dec 23 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

CalStats::AXES::AXES( void ) {

  // Initialize the public variables and return

  eAxisIterFeedID = INIT;
  eAxisIterUserID = INIT;
  eAxisNonIterID = INIT;
  sFeed = String();
  fAxisIterUser = 0.0;

  return;

}

// -----------------------------------------------------------------------------

/*

CalStats::AXES::AXES (copy)

Description:
------------
This construtor copies the input instance to the present instance.

Inputs:
-------
oAxes - This CalStats::AXES instance contains the axes information.

Outputs:
--------
None.

Modification history:
---------------------
2011 Dec 23 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

CalStats::AXES::AXES( const CalStats::AXES& oAxes ) {

  // Copy the public variables from the input instance to this instance and
  // return

  eAxisIterFeedID = oAxes.eAxisIterFeedID;
  eAxisIterUserID = oAxes.eAxisIterUserID;
  eAxisNonIterID = oAxes.eAxisNonIterID;
  sFeed = String( oAxes.sFeed );
  fAxisIterUser = oAxes.fAxisIterUser;

  return;

}

// -----------------------------------------------------------------------------

/*

CalStats::AXES::~AXES

Description:
------------
This destructor destroys the instance.

Inputs:
-------
None.

Outputs:
--------
None.

Modification history:
---------------------
2011 Dec 23 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

CalStats::AXES::~AXES( void ) {}

// -----------------------------------------------------------------------------

/*

CalStats::AXES::operator=

Description:
------------
This function sets one instance to another.

Inputs:
-------
oAxes - This CalStats::AXES instance contains the axes information.

Outputs:
--------
None.

Modification history:
---------------------
2011 Dec 23 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

CalStats::AXES& CalStats::AXES::operator=( const CalStats::AXES& oAxes ) {

  // If this instance is not the same as the input instance, copy the public
  // variables to this one and return

  if ( this != &oAxes ) {
    eAxisIterFeedID = oAxes.eAxisIterFeedID;
    eAxisIterUserID = oAxes.eAxisIterUserID;
    eAxisNonIterID = oAxes.eAxisNonIterID;
    sFeed = String( oAxes.sFeed );
    fAxisIterUser = oAxes.fAxisIterUser;
  }
  
  return( *this );

}

// -----------------------------------------------------------------------------
// End of CalStats::AXES public member functions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// End of CalStats::AXES class
// -----------------------------------------------------------------------------

};

// -----------------------------------------------------------------------------
// End of casa namespace
// -----------------------------------------------------------------------------
