
// -----------------------------------------------------------------------------

/*

CalStats.cc

Description:
------------
This file contains member functions for the CalStats class.

Classes:
--------
CalStats - This class calculates statistics on CASA caltables.

Modification history:
---------------------
2011 Nov 11 - Nick Elias, NRAO
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
// Start of CalStats class
// -----------------------------------------------------------------------------

/*

CalStats

Description:
------------
This class calculates statistics of new CASA caltables.

NB: At present this class gets data and calculates fit statistics, but other
things such as histogram statistics will be added later.

In a nutshell:
--------------
* This class can get data (no statistics calculated) and calculate fit
  statistics from these data.
* Hooks are present in the code for calculating histogram statistics in the
  future.
* This class employs internal iterators to move uniformly through the data
  cubes.  This process is invisible to the user.
* The input data are cubes whose axes are feed, frequency, and time.  There are
  two iteration axes and one non-iteration axis, which means that this class
  returns ONE-dimensional quantities (data, fits, or histograms) for each
  iteration.  This class does not deal with multi-dimensional fits and
  histograms.
* The feed axis is always an iteration axis (fits cannot be calculated along
  it).  The user-defined iteration axis is either frequency or time, which means
  that the other axis is either time or frequency.
* The input data are obtained from the NewCalTable and NewCalTabIter classes
  that iterate along antenna1, antenna2, spw, etc.
* Once an instance is created, the stats<T> member function is the main user
  interface to calculate statistics.  The choice of T determines which function
  is used (getting data, fit statistics, histogram statistics).
* To minimize the amount of duplicated code, some of the member functions are
  templated and some of the templated member functions are specialized.  The
  class is not templated.

Nested classes:
---------------
AXES - This nested class contains the axes for the CalStats class.
DATA - This nested class contains the data for the CalStats class.
ARG<T> - This nested template class contains the arguments for the
         CalStats::stats<T>() template member function.
OUT<T> - This nested template class contains the outputs for the
         CalStats::stats<T>() template member function.

Class public member functions:
------------------------------
CalStats  - This constructor saves input abscissae and data cubes to internal
            copies so that statistics can be calculated.
~CalStats - This destructor deallocates the internal memory of an instance.

Class public state member functions:
------------------------------------
axisIterID    - This member function returns the iteration axis IDs.
axisNonIterID - This member function returns the non-iteration axis ID.
axisIterFeed  - This member function returns the feed iteration axis values.
axisIterUser  - This member function returns the user-defined iteration axis
                values.
axisNonIter   - This member function returns the non-iteration axis values.
statsShape    - This member function returns the shape of the output statistics
                cube.
data          - This member function returns the input data.
dataErr       - This member function returns the input data errors.
flag          - This member function returns the input flags.

Class template public stats member functions:
---------------------------------------------
stats<T> - This member function is the main user interface for calculating the
           the statistics for all iterations.  Allowed T: CalStats::NONE only
	   returns the input data, CalStatsFitter::FIT calculates fit
           statistics, and CalStatsHist::HIST calculates histogram statistics.

Class specialized template public stats member functions:
---------------------------------------------------------
statsWrap<T> - This member function wraps statistics functions and provides a
               uniform interface to stats<T>().  Allowed T: CalStats::NONE only
	       returns the input data, CalStatsFitter::FIT calculates fit
               statistics, and CalStatsHist::HIST calculates histogram
               statistics.

Class protected member functions:
---------------------------------
CalStats  - This default constructor is unused by this class and unavailable
            when an instance is created.
CalStats  - This copy constructor is unused by this class and unavailable when
            an instance is created.
operator= - This operator= function is unused by this class and unavailable when
            an instance is created.
next      - This member function simultaneously iterates all of the internal
            copies of the input data cubes.
reset     - This member function simultaneously resets all of the internal
            copies of the input data cubes.

Modification history:
---------------------
2011 Nov 11 - Nick Elias, NRAO
              Initial version created with public member functions CalStats()
              (generic) and ~CalStats(); protected member functions CalStats()
              (default), CalStats() (copy), operator=(), next() and reset().
2011 Nov 15 - Nick Elias, NRAO
              Moved the CalTypes namespace and its members, originally defined
              in this file, to the CalStatsFitter class.  Also, the
              CalTypes::AXIS typedef was replaced by CalStats::AXIS.
2011 Dec 08 - Nick Elias, NRAO
              Added the fit axis vector to the internal variables.
2011 Dec 11 - Nick Elias, NRAO
              The structure CalStats::FIT was added and replaces the
              CalStatsFitter::FIT structure output of the calcFit() public stats
              member function (the latter is now part of the former).  Added
              init() and dealloc() public member functions.
2011 Dec 14 - Nick Elias, NRAO
              The structures CalStats::AXES, CalStats::DATA, and CalStats::NONE
              added.  The nested class CalStats::OUT<T> added (C++ does not
              allow templated typedefs, so a nested class is used instead).
              Public member function getData() added.  Public static member
              functions initAxes(), initGet(), initResultNone(),
              initResultFit(), and dealloc() added (overloaded for
              CalStats::DATA and CalStats::OUT<CalStatsFitter::FIT>).  Removed
              public member functions init() and dealloc() (non-overloaded
              version).
2011 Dec 15 - Nick Elias, NRAO
              Private member functions next() and reset() now protected member
              functions.  State public member functions axisIterID(),
              axisNonIterID(), axisIterFeed(), axisIterUser(), axisNonIter(),
              statsShape(), data(), dataErr(), and flag() added.
2011 Dec 16 - Nick Elias, NRAO
              Public member functions getData() and calcFit() replaced by
              stats<T>() template public member function.  Specialized template
              public member function statsWrap<T>() added.  Public static member
              functions initAxes(), initData(), initResultNone(), and
              initResultFit() replaced by template public static member function
              init<T>().
2011 Dec 21 - Nick Elias, NRAO
              Template public static member functions init<T>() and dealloc<T>
              removed because all of their duties are subsumed by the nested
              classes AXES, DATA, ARG, and OUT (they were previously structures).

*/

// -----------------------------------------------------------------------------
// Start of CalStats public member functions
// -----------------------------------------------------------------------------

/*

CalStats::CalStats

Description:
------------
This constructor saves input abscissae and data cubes to internal copies so that
statistics can be calculated.

NB: There must always be two iteration axes because statistics are performed
only on the third.  The FEED axis is always included as an iteration axis by
default because one cannot perform a fit along it.  The other axis is defined
by the user as an input parameter.

Inputs:
-------
oData           - This Cube<Float> instance contains the data.
oDataErr        - This Cube<Float> instance contains the data errors.
oFlag           - This Cube<Bool> instance contains the flags.
oFeed           - This Vector<String> instance is the feed abscissae.
oFrequency      - This Vector<Float> instance is the frequency abscissae.
oTime           - This Vector<Float> instance is the time abscissae.
eAxisIterUserID - This CalStats::AXIS enum contains either the FREQUENCY or TIME
                  iteration axes (user defined).

Outputs:
--------
None.

Modification history:
---------------------
2011 Nov 11 - Nick Elias, NRAO
              Initial version.
2011 Nov 15 - Nick Elias, NRAO
              The CalTypes::AXIS enum has been replaced with CalStats::AXIS.
2011 Dec 19 - Nick Elias, NRAO
              To avoid data referencing issues, I invoked the copy() method of
              classes with the Array<T> base class.  To save memory, I may go
              back to the original way if I'm confident that it's OK.

*/

// -----------------------------------------------------------------------------

CalStats::CalStats( const Cube<Float>& oData, const Cube<Float>& oDataErr,
    const Cube<Bool>& oFlag, const Vector<String>& oFeed,
    const Vector<Float>& oFrequency, const Vector<Float>& oTime,
    const CalStats::AXIS& eAxisIterUserID ) {

  // Check the user-defined iteration axis

  if ( eAxisIterUserID != CalStats::FREQUENCY &&
      eAxisIterUserID != CalStats::TIME ) {
    throw( AipsError( "User-defined iteration axis must be frequency/time." ) );
  }


  // Initialize the internal copies of the iteration and fit axes

  switch ((uInt) eAxisIterUserID) {
  
    case (uInt) CalStats::TIME:
      oAxisIterID = IPosition( 2, CalStats::FEED, CalStats::TIME );
      oAxisIterFeed = Vector<String>( oFeed.copy() );
      oAxisIterUser = Vector<Float>( oTime.copy() );
      eAxisNonIterID = CalStats::AXIS( CalStats::FREQUENCY );
      oAxisNonIter = Vector<Float>( oFrequency.copy() );
      break;

    case (uInt) CalStats::FREQUENCY:
      oAxisIterID = IPosition( 2, CalStats::FEED, CalStats::TIME );
      oAxisIterFeed = Vector<String>( oFeed.copy() );
      oAxisIterUser = Vector<Float>( oFrequency.copy() );
      eAxisNonIterID = CalStats::AXIS( CalStats::TIME );
      oAxisNonIter = Vector<Float>( oTime.copy() );
      break;

    default:
      break;

  }


  // Form the shape of the statistics cube.  To appease the ArrayIterator<T>
  // template used elsewhere in this class, the fit axis is included as a
  // degenerate axis (length=1).

  oStatsShape = IPosition( 3, 1 );

  for ( uInt a=0; a<2; a++ ) {
    uInt uiAxisIter = (uInt) oAxisIterID[a];
    oStatsShape[uiAxisIter] = oData.shape()[uiAxisIter];
  }


  // Initialize the internal copies of the input parameter cubes

  poData = new Cube<Float>( oData.copy() );
  poDataErr = new Cube<Float>( oDataErr.copy() );
  poFlag = new Cube<Bool>( oFlag.copy() );


  // Initialize the input parameter cube iterators and reset them

  poDataIter = new ArrayIterator<Float>( *poData, oAxisIterID, False );
  poDataIter->reset();

  poDataErrIter = new ArrayIterator<Float>( *poDataErr, oAxisIterID, False );
  poDataErrIter->reset();

  poFlagIter = new ArrayIterator<Bool>( *poFlag, oAxisIterID, False );
  poFlagIter->reset();


  // Return

  return;

}

// -----------------------------------------------------------------------------

/*

CalStats::~CalStats

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
2011 Nov 11 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

CalStats::~CalStats( void ) {

  // Deallocate the internal copies of the input parameter cubes

  delete poData;
  delete poDataErr;
  delete poFlag;


  // Deallocate the input parameter cube iterators

  delete poDataIter;
  delete poDataErrIter;
  delete poFlagIter;


  // Return

  return;

}

// -----------------------------------------------------------------------------
// End of CalStats public member functions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Start of CalStats public state member functions
// -----------------------------------------------------------------------------

/*

CalStats::axisIterID

Description:
------------
This member function returns the iteration axis IDs..

Inputs:
-------
None.

Outputs:
--------
The reference to the IPosition instance containing the iteration axis IDs,
returned via the function value.

Modification history:
---------------------
2011 Dec 15 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

IPosition& CalStats::axisIterID( void ) const {

  // Return the reference to the IPosition instance containing the iteration
  // axis IDs

  IPosition* poAxisIterID = new IPosition( oAxisIterID );

  return( *poAxisIterID );

}

// -----------------------------------------------------------------------------

/*

CalStats::axisNonIterID

Description:
------------
This member function returns the non-iteration axis IDs.

Inputs:
-------
None.

Outputs:
--------
The reference to the CalStats::AXIS instance containing the non-iteration axis
IDs, returned via the function value.

Modification history:
---------------------
2011 Dec 15 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

CalStats::AXIS& CalStats::axisNonIterID( void ) const {

  // Return the reference to the CalStats::AXIS instance containing the
  // non-iteration axis IDs

  CalStats::AXIS* peAxisNonIterID = new CalStats::AXIS( eAxisNonIterID );

  return( *peAxisNonIterID );

}

// -----------------------------------------------------------------------------

/*

CalStats::axisIterFeed

Description:
------------
This member function returns the feed iteration axis values.

Inputs:
-------
None.

Outputs:
--------
The reference to the Vector<String> instance containing the feed iteration axis
values, returned via the function value.

Modification history:
---------------------
2011 Dec 15 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

Vector<String>& CalStats::axisIterFeed( void ) const {

  // Return the reference to the Vector<String> instance containing the feed
  // iteration axis values

  Vector<String>* poAxisIterFeed = new Vector<String>( oAxisIterFeed );

  return( *poAxisIterFeed );

}

// -----------------------------------------------------------------------------

/*

CalStats::axisIterUser

Description:
------------
This member function returns the user-defined iteration axis values.

Inputs:
-------
None.

Outputs:
--------
The reference to the Vector<Float> instance containing the user-defined
iteration axis values, returned via the function value.

Modification history:
---------------------
2011 Dec 15 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

Vector<Float>& CalStats::axisIterUser( void ) const {

  // Return the reference to the Vector<Float> instance containing the
  // user-defined iteration axis values

  Vector<Float>* poAxisIterUser = new Vector<Float>( oAxisIterUser );

  return( *poAxisIterUser );

}

// -----------------------------------------------------------------------------

/*

CalStats::axisNonIter

Description:
------------
This member function returns the non-iteration axis values.

Inputs:
-------
None.

Outputs:
--------
The reference to the Vector<Float> instance containing the non-iteration axis
values, returned via the function value.

Modification history:
---------------------
2011 Dec 15 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

Vector<Float>& CalStats::axisNonIter( void ) const {

  // Return the reference to the Vector<Float> instance containing the
  // non-iteration axis values

  Vector<Float>* poAxisNonIter = new Vector<Float>( oAxisNonIter );

  return( *poAxisNonIter );

}

// -----------------------------------------------------------------------------

/*

CalStats::statsShape

Description:
------------
This member function returns the shape of the output statistics cube.

Inputs:
-------
None.

Outputs:
--------
The reference to the IPosition instance containing the shape of the output
statistics cube, returned via the function value.

Modification history:
---------------------
2011 Dec 15 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

IPosition& CalStats::statsShape( void ) const {

  // Return the reference to the IPosition instance containing the shape of the
  // output statistics cube

  IPosition* poStatsShape = new IPosition( oStatsShape );

  return( *poStatsShape );

}

// -----------------------------------------------------------------------------

/*

CalStats::data

Description:
------------
This member function returns the input data.

Inputs:
-------
None.

Outputs:
--------
The reference to the Cube<Float> instance containing the input data, returned
via the function value.

Modification history:
---------------------
2011 Dec 15 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

Cube<Float>& CalStats::data( void ) const {

  // Return the reference to the Cube<Float> instance containing the input data

  return( *poData );

}

// -----------------------------------------------------------------------------

/*

CalStats::dataErr

Description:
------------
This member function returns the input data errors.

Inputs:
-------
None.

Outputs:
--------
The reference to the Cube<Float> instance containing the input data errors,
returned via the function value.

Modification history:
---------------------
2011 Dec 15 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

Cube<Float>& CalStats::dataErr( void ) const {

  // Return the reference to the Cube<Float> instance containing the input data
  // errors

  return( *poDataErr );

}

// -----------------------------------------------------------------------------

/*

CalStats::flag

Description:
------------
This member function returns the input flags.

Inputs:
-------
None.

Outputs:
--------
The reference to the Cube<Bool> instance containing the input flags, returned
via the function value.

Modification history:
---------------------
2011 Dec 15 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

Cube<Bool>& CalStats::flag( void ) const {

  // Return the reference to the Cube<Bool> instance containing the input flags

  return( *poFlag );

}

// -----------------------------------------------------------------------------
// End of CalStats public state member functions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Start of CalStats::statsWrap<T> specialized template public fit member
// functions
// -----------------------------------------------------------------------------

/*

CalStats::statsWrap<CalStats::NONE>

Description:
------------
This member function wraps statistics functions and provides a uniform interface
to stats<T>().  This 

Inputs:
-------
oDummy1 - This Vector<Float> instance is a dummy.
oDummy2 - This Vector<Float> instance is a dummy.
oDummy3 - This Vector<Float> instance is a dummy.
oDummy4 - This Vector<Bool> instance is a dummy.
oDummy5 - This CalStats::ARG<CalStats::NONE> instance is a dummy.

Outputs:
--------
The reference to the CalStats::NONE instance, returned via the function value.

Modification history:
---------------------
2011 Dec 16 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

template <> CalStats::NONE& CalStats::statsWrap<CalStats::NONE>(
    const Vector<Float>& /*oDummy1*/, const Vector<Float>& /*oDummy2*/,
    const Vector<Float>& /*oDummy3*/, Vector<Bool>& /*oDummy4*/,
    const CalStats::ARG<CalStats::NONE>& /*oDummy5*/ ) {

  // Return the reference to a CalStats::NONE instance

  CalStats::NONE* poNone = new CalStats::NONE();

  return( *poNone );

}

// -----------------------------------------------------------------------------

/*

CalStats::statsWrap<CalStatsFitter::FIT>

Description:
------------
This member function wraps statistics functions and provides a uniform interface
to stats<T>().

Inputs:
-------
oAbs      - This Vector<Float> instance contains the non-iteration axis
            abscissae.
oData     - This Vector<Float> instance contains the data.
oDataErr  - This Vector<Float> instance contains the data errors.
oFlag     - This Vector<Bool> instance contains the flags.
oArg      - This CalStats::ARG<CalStats::NONE> contains the extra arguments.

Outputs:
--------
oFlag - This Vector<Bool> instance contains the flags.
The reference to the CalStatsFitter::FIT instance, returned via the function
value.

Modification history:
---------------------
2011 Dec 16 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

template <> CalStatsFitter::FIT& CalStats::statsWrap<CalStatsFitter::FIT>(
    const Vector<Float>& oAbs, const Vector<Float>& oData,
    const Vector<Float>& oDataErr, Vector<Bool>& oFlag,
    const CalStats::ARG<CalStatsFitter::FIT>& oArg ) {

  // Perform the fit and return the reference to a CalStatsFitter::FIT instance

  CalStatsFitter::FIT* poFit = new CalStatsFitter::FIT();
  *poFit = CalStatsFitter::fit( oAbs, oData, oDataErr, oFlag, oArg.eOrder,
      oArg.eType, oArg.eWeight );

  return( *poFit );

}

// -----------------------------------------------------------------------------
// Start of CalStats::statsWrap<T> specialized template public fit member
// functions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Start of CalStats protected member functions
// -----------------------------------------------------------------------------

/*

CalStats::CalStats (default)

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
2011 Nov 11 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

CalStats::CalStats( void ) {}

// -----------------------------------------------------------------------------

/*

CalStats::CalStats (copy)

Description:
------------
This copy constructor is unused by this class and unavailable when an instance
is created.

Inputs:
-------
oCalStats - A CalStats instance.

Outputs:
--------
None.

Modification history:
---------------------
2011 Nov 11 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

CalStats::CalStats( const CalStats& oCalStats ) {

  // Overwrite this instance and return.  This code will bomb.  I have written
  // it in this way to keep the compiler from spewing warning messages about
  // unused variables.

  *this = oCalStats;

  return;

}

// -----------------------------------------------------------------------------

/*

CalStats::operator=

Description:
------------
This operator= function is unused by this class and unavailable when an instance
is created.

Inputs:
-------
oCalStats - A CalStats instance.

Outputs:
--------
None.

Modification history:
---------------------
2011 Nov 11 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

CalStats& CalStats::operator=( const CalStats& oCalStats ) {

  // Copy the input instance and return it.  This code will bomb.  I have
  // written it in this way to keep the compiler from spewing warning messages
  // about unused variables.

  CalStats* poCalStats = new CalStats( oCalStats );

  return( *poCalStats );

}

// -----------------------------------------------------------------------------

/*

CalStats::next

Description:
------------
This member function simultaneously iterates all of the internal copies of the
input data cubes.

Inputs:
-------
None.

Outputs:
--------
None.

Modification history:
---------------------
2011 Nov 11 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

void CalStats::next( void ) {

  // Iterate the input data cube iterators

  poDataIter->next();
  poDataErrIter->next();
  poFlagIter->next();


  // Return

  return;

}

// -----------------------------------------------------------------------------

/*

CalStats::reset

Description:
------------
This member function simultaneously resets the iterators of all of the internal
copies of the input data cubes.

Inputs:
-------
None.

Outputs:
--------
None.

Modification history:
---------------------
2011 Nov 11 - Nick Elias, NRAO
              Initial version.

*/

// -----------------------------------------------------------------------------

void CalStats::reset( void ) {

  // Reset the input data cube iterators

  poDataIter->reset();
  poDataErrIter->reset();
  poFlagIter->reset();


  // Return

  return;

}

// -----------------------------------------------------------------------------
// End of CalStats protected member functions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// End of CalStats class
// -----------------------------------------------------------------------------

};

// -----------------------------------------------------------------------------
// End of casa namespace
// -----------------------------------------------------------------------------
