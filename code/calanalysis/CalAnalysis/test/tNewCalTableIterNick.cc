 //# tNewCalTableNick.cc: Test program for NewCalTable class
//# Copyright (C) 2011
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id: tNewCalTable.cc 15602 2011-07-14 00:03:34Z tak.tsutsumi $

#include <calibration/CalTables/NewCalTable.h>
#include <calibration/CalTables/NewCalTableDesc.h>
#include <calibration/CalTables/NewCalMainRecord.h>
#include <calibration/CalTables/NewCalMainColumns.h>
#include <calibration/CalTables/NewCalTabIter.h>
//#include <calibration/CalTables/NewCalColumns.h>
#include <ms/MeasurementSets/MSAntennaColumns.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/TableIter.h>
#include <tables/Tables/ScaColDesc.h>
#include <derivedmscal/DerivedMC/DerivedMSCal.h>
#include <casa/Arrays/Cube.h>
#include <casa/Containers/RecordField.h>
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <casa/BasicMath/Math.h>
#include <casa/OS/Timer.h>
#include <casa/namespace.h>

// <summary>
// Test program for NewCalTable class.
// </summary>

// -----------------------------------------------------------------------------

void createNCT( void ) {

  // Create an empty new caltable

  String oTableIn = String( "testNCT.tab" );
  NewCalTableDesc oNCTD = NewCalTableDesc();

  NewCalTable oNCT = NewCalTable( oTableIn, oNCTD, Table::Scratch,
      Table::Memory );


  // Obtain meta information from an MS

  String oCASAPath = getenv( "CASAPATH" );

  String oDataPath = oCASAPath.substr( 0, oCASAPath.find(" ") );
  String oMSData = String( "/data/regression/unittest/clean/point_spw1.ms" );

  oNCT.setMetaInfo( oDataPath + oMSData );


  // Get/form the antenna quantities

  Int numAntenna = oNCT.antenna().nrow();
  Int iRefAnt = numAntenna / 2; // Reference antenna near the middle of the list

  Vector<Int> oAntenna( numAntenna );
  indgen( oAntenna );


  // Get the spectral window and channel quantities

  Int numSPW = oNCT.spectralWindow().nrow();

  ROMSSpWindowColumns oSPWCol( oNCT.spectralWindow() );
  Vector<Int> oNumChannel = oSPWCol.numChan().getColumn();


  // Set the field and time quantities

  Int numField = oNCT.field().nrow();

  Int numTime = 500;
  Double dTime0 = 4734187549.8204317; // Obtained from the MS
  Double dDTime = 10000.0;


  // Populate the new caltable with the NewCalMainColumns() class

  NewCalMainColumns oNCMC( oNCT );

  Double dThisTime = dTime0;
  Int iThisScan = 0;
  Int iThisField = 0;

  Int numParameter = 2;

  for ( Int iTime=0; iTime<numTime; ++iTime ) {

    if ( iTime%10 == 0 ) cout << "time = " << iTime << endl;
    if ( iTime > 0 ) dThisTime += dDTime;

    iThisScan = iTime/100 + 1;
    iThisField = iThisScan % numField;

    for ( Int iThisSPW=0; iThisSPW<numSPW; ++iThisSPW ) {

      RefRows oRow = RefRows( oNCT.nrow(), oNCT.nrow()+numAntenna-1, 1 ); 
      oNCT.addRow( numAntenna );

      Vector<Double> oTime = Vector<Double>( numAntenna, dThisTime );
      oNCMC.time().putColumnCells( oRow, oTime );

      Vector<Int> oField = Vector<Int>( numAntenna, iThisField );
      oNCMC.fieldId().putColumnCells( oRow, oField );

      Vector<Int> oSPW = Vector<Int>( numAntenna, iThisSPW );
      oNCMC.spwId().putColumnCells( oRow, oSPW );

      Vector<Int> oRefAnt = Vector<Int>( numAntenna, iRefAnt );
      oNCMC.antenna1().putColumnCells( oRow, oAntenna );
      oNCMC.antenna2().putColumnCells( oRow, oRefAnt );

      Vector<Int> oScan = Vector<Int>( numAntenna, iThisScan );
      oNCMC.scanNo().putColumnCells( oRow, oScan );

      Cube<Float> oParameter = Cube<Float>( numParameter, oNumChannel(iThisSPW),
          numAntenna );
      oParameter = 0.95;
      oNCMC.param().putColumnCells( oRow, oParameter );

      Cube<Float> oParameterErr = Cube<Float>( numParameter,
          oNumChannel(iThisSPW), numAntenna );
      oParameterErr = 0.01;
      oNCMC.paramerr().putColumnCells( oRow, oParameterErr );

      Cube<Bool> oFlag = Cube<Bool>( numParameter, oNumChannel(iThisSPW),
          numAntenna );
      oFlag = False;
      oNCMC.flag().putColumnCells( oRow, oFlag );

      Cube<Float> oSNR = Cube<Float>( numParameter, oNumChannel(iThisSPW),
          numAntenna );
      oSNR = oParameter / oParameterErr;
      oNCMC.snr().putColumnCells( oRow, oSNR );

      Cube<Float> oWeight = Cube<Float>( numParameter, oNumChannel(iThisSPW),
          numAntenna );
      oWeight = 1.0;
      oNCMC.weight().putColumnCells( oRow, oWeight );

    }

  }


  // Write the new caltable to disk

  String oDiskName = String( "testNCTdisk.tab" );

  oNCT.writeToDisk( oDiskName );


  // Return

  return;

}

// -----------------------------------------------------------------------------

void readNCT( void ) {

  // Open the new caltable

  String oTableIn = String( "testNCTdisk.tab" );

  NewCalTable oNCT = NewCalTable( oTableIn, Table::Old, Table::Memory );


  // Define the sort columns and the new caltable iterator

  Block<String> oSortCol( 1 );

//  oSortCol[0] = String( "TIME" );
//  oSortCol[1] = String( "SPECTRAL_WINDOW_ID" );
//  oSortCol[1] = String( "ANTENNA1" );
  oSortCol[0] = String( "SCAN_NUMBER" );
      
  NewCalTabIter oNCTIter( oNCT, oSortCol );


  // Iterate through the new caltable and print the results

  Int iIter=0;

  while ( !oNCTIter.pastEnd() ) {

    Table oTab( oNCTIter.table() );

    cout << iIter << ":" << endl;

    cout << "nrow = " << oNCTIter.nrow() << endl;
    cout << "spw = " << oNCTIter.spw() << endl;
    cout << "antenna1 = " << oNCTIter.antenna1() << endl;
    cout << "antenna2 = " << oNCTIter.antenna2() << endl;
    cout << "scan = " << oNCTIter.scan() << endl;
    cout << "time = " << oNCTIter.time() << endl;
    cout << "param = " << oNCTIter.param() << endl;
//    cout << "rows = " << oTab.rowNumbers() << endl; 
//    cout << "rows (NCT) = " << oTab.rowNumbers( oNCT ) << endl; 
    cout << endl << flush;

    oNCTIter.next();
    ++iIter;

  }


  // Return

  return;

}

// -----------------------------------------------------------------------------

int main( void ) {

  try {

//    cout << endl;
//    cout << "createNCT() ..." << endl << endl;
//    createNCT();

    cout << endl;
    cout << "readNCT() ..." << endl << endl;
    readNCT();

  } catch ( AipsError x ) {

    cout << "Exception: " << x.getMesg() << endl;
    exit( 1 );

  } catch ( ... ) {

    cout << "Unknown exception ..." << endl;
    exit( 1 );

  }

  cout << "OK" << endl << endl;

  return( 0 );

};
