
// -----------------------------------------------------------------------------

#include <calanalysis/CalAnalysis/CalAnalysis.h>

using namespace casa;

// -----------------------------------------------------------------------------

int main( void ) {

  CalAnalysis oCA( "testNCTdisk.tab" );

  Vector<Int> oField( 1, 0 );

  Vector<Int> oAntenna1( 5 );
  for ( uInt a=0; a<5; a++ ) oAntenna1[a] = a;

  Vector<Int> oAntenna2( 5 );
  for ( uInt a=0; a<5; a++ ) oAntenna2[a] = 2;

  Double dStartTime = 4734187549.8204317093;
  Double dStopTime  = 4734191549.8204317093;

  Vector<String> oFeed( 2 );
  oFeed[0] = "X";
  oFeed[1] = "Y";

  Vector<Int> oSPW( 3 );
  for ( uInt s=0; s<3; s++ ) oSPW[s] = s;

  Vector<Int>* aoChannel = new Vector<Int> [3];
  for ( uInt s=0; s<3; s++ ) {
    aoChannel[s].resize( 20-s-s, False );
    for ( uInt c=s; c<20-s; c++ ) {
      aoChannel[s][c-s] = c;
    }
  }

  CalStats::AXIS eAxisIterUser = CalStats::FREQUENCY;

  CalStats::ARG<CalStats::NONE> oArg;

  Vector<CalAnalysis::OUTPUT<CalStats::NONE> > oOutput;
  oOutput = oCA.stats<CalStats::NONE>( oField, oAntenna1, oAntenna2, dStartTime,
      dStopTime, oFeed, oSPW, aoChannel, eAxisIterUser, CalAnalysis::REAL,
      oArg );

  cout << oOutput[0].oOut.shape() << endl << flush;
  cout << endl << flush;

  for ( uInt o=0; o<oOutput.nelements(); o++ ) {
    cout << oOutput[o].uiField << " " << oOutput[o].uiAntenna1 << " "
         << oOutput[o].uiAntenna2 << endl << flush;
    cout << oOutput[o].oOut(0,0).oData.oData << endl << flush;
  }

  return( 0 );

}
