//# tImager.cc:  this tests Imager
//# Copyright (C) 1996,1997,1999,2001
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#include <casa/iostream.h>
#include <casa/aips.h>
#include <casa/Exceptions/Error.h>
#include <casa/Containers/Block.h>
#include <measures/Measures/MRadialVelocity.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <casa/Logging/LogIO.h>
#include <synthesis/MeasurementEquations/Imager.h>

#include <casa/namespace.h>

int main()
{
  using namespace std;
  using namespace casa;
  try{
    cout <<"--Starting to add an entry to the history table." << endl;
    MeasurementSet ms( "/home/rrusk/testing/3C273XC1.ms", Table::Update );
    cout << "--MeasurementSet created." << endl;

    Bool compress = False;
    Imager* imgr = new Imager( ms, compress );
    cout <<"--Imager created for MeasurementSet object. " << endl;

    LogSink logSink_p=LogSink(LogMessage::NORMAL, False);	  
    logSink_p.clearLocally();
    LogIO os(LogOrigin("tImager", "main()", WHERE), logSink_p);

    int nx = 100;
    int ny = 100;
    Quantity cellx( 30, "arcsec" );
    Quantity celly( 30, "arcsec" ); 
    Vector<Int> spwids(2);
    spwids[0] = 1;
    spwids[1] = 2; 
    Int fieldid = 1;

    os << "nx=" << nx << " ny=" << ny
       << " cellx='" << cellx.getValue() << cellx.getUnit()
       << "' celly='" << celly.getValue() << celly.getUnit()
       << "' spwids=" << spwids
       << " field=" <<   fieldid;

    imgr->lock();

    imgr->writeCommand( os );
    cout << "--Imager::writeCommand() called." << endl;

    os << "The history table name is " << ms.historyTableName()
       << LogIO::POST;
    imgr->writeHistory(os);

    imgr->unlock();

    cout << "--Imager::writeHistory() called." << endl;	  

  }catch( AipsError e ){
    cout << "Exception ocurred." << endl;
    cout << e.getMesg() << endl;
  }
};
