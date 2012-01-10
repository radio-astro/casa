
// -----------------------------------------------------------------------------

#include <cstdlib>

#include <math.h>
float rgauss( void );

#include <calanalysis/CalAnalysis/CalStats.h>
#include <calanalysis/CalAnalysis/CalStatsDerived.h>

using namespace casa;

// -----------------------------------------------------------------------------

int main( void ) {

  // Initialize the shape of the input cubes

  uInt uiNumPol = 2;
  uInt uiNumFreq = 64;
  uInt uiNumTime = 100;

  IPosition oShape( 3, uiNumPol, uiNumFreq, uiNumTime );


  // Initialize the input data cube (constant across frequency axis)

  Cube<Complex> oData( oShape );

  for ( uInt p=0; p<uiNumPol; p++ ) {
    for ( uInt t=0; t<uiNumTime; t++ ) {
      Float fData = 1.0 + 0.1*((Float) rand())/((Float) RAND_MAX);
      for ( uInt f=0; f<uiNumFreq; f++ ) {
        oData.operator()(p,f,t) = Complex( fData+0.1*rgauss(), 0.1*rgauss() );
      }
    }
  }


  // Initialize the input data error cube

  Cube<Complex> oDataErr( oShape );

  for ( uInt p=0; p<uiNumPol; p++ ) {
    for ( uInt t=0; t<uiNumTime; t++ ) {
      Float fDataErr = 0.1;
      for ( uInt f=0; f<uiNumFreq; f++ ) {
        oDataErr.operator()(p,f,t) = Complex( fDataErr, fDataErr );
      }
    }
  }


  // Initialize the input flag cube

  Cube<Bool> oFlag( oShape, False );


  // Initialize the polarization (feed) abscissa

  Vector<String> oFeed( uiNumPol );
  oFeed[0] = "R";
  oFeed[1] = "L";


  // Initialize the frequency abscissa

  Vector<Float> oFreq( uiNumFreq );
  for ( uInt f=0; f<uiNumFreq; f++ ) oFreq[f] = f*2.0E+06 + 10.0E+09;


  // Initialize the time abscissa

  Vector<Float> oTime( uiNumTime );
  for ( uInt t=0; t<uiNumTime; t++ ) oTime[t] = (Float) t;


  // Initialize the user-supplied iteration axis (the polarization axis is
  // always an iteration axis, by default)

  CalStats::AXIS eAxis = CalStats::TIME;


  // Create a CalStatsReal object and get the data iterated along the
  // polarization and time axes

  CalStatsPhase oCSP( oData, oDataErr, oFlag, oFeed, oFreq, oTime, eAxis,
      False );

  CalStats::ARG<CalStats::NONE> oArg;
  Matrix<CalStats::OUT<CalStats::NONE> >
      oMatrix( oCSP.stats<CalStats::NONE>( oArg ) );


  // Write the data for each polarization and time axis

  for ( uInt p=0; p<uiNumPol; p++ ) {
    for ( uInt t=0; t<uiNumTime; t++ ) {
      cout << endl << flush;
      cout << p << ' ' << t << endl << flush;
      cout << oMatrix(p,t).oAxes.eAxisIterFeedID << ' '
           << oMatrix(p,t).oAxes.eAxisIterUserID << ' '
           << oMatrix(p,t).oAxes.eAxisNonIterID << ' '
           << endl << flush;
      cout << oMatrix(p,t).oAxes.sFeed << ' '
           << oMatrix(p,t).oAxes.fAxisIterUser << endl << flush;
      cout << oMatrix(p,t).oData.oAbs << endl << flush;
      cout << oMatrix(p,t).oData.oData << endl << flush;
      cout << oMatrix(p,t).oData.oDataErr << endl << flush;
      cout << oMatrix(p,t).oData.oFlag << endl << flush;
    }
  }


  // Return 0

  return( 0 );

}

// -----------------------------------------------------------------------------

float rgauss( void ) {

  static int iset=0;
  static float gset;
  float fac,r,v1,v2;

  if ( iset == 0 ) {
    do {
      v1 = 2.0 * (((Float) rand())/((Float) RAND_MAX) - 0.5);
      v2 = 2.0 * (((Float) rand())/((Float) RAND_MAX) - 0.5);
      r = v1*v1 + v2*v2;
    } while ( r >= 1.0 || r == 0.0 );
    fac = sqrt( -2.0*log(r)/r );
    gset = v1*fac;
    iset = 1;
    return( v2*fac );
  } else {
    iset = 0;
    return( gset );
  }

}
