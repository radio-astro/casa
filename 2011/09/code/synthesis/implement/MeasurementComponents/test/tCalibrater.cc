//# tCalibrater.cc:  This tests Calibrater
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
#include <casa/Logging/LogIO.h>
#include <synthesis/MeasurementComponents/Calibrater.h>

int main()
{
  using namespace std;
  using namespace casa;
  try{
     cout <<"--Starting the creation or update of the history table." << endl;
     Calibrater* cal = new Calibrater( );
     cout <<"--Calibrater object created. " << endl;
     MeasurementSet ms( "/home/rrusk/testing/3C273XC1.ms" );
     cout << "--MeasurementSet Object created." << endl;
     cal->initialize(ms);
     cout << "--initialize() called." << endl;
     //LogIO os;
     LogSink logSink_p=LogSink(LogMessage::NORMAL, False);	  
     logSink_p.clearLocally();
     LogIO os(LogOrigin("tCalibrater", "main()", WHERE), logSink_p);
	  
/*
     os << "nx= " << nx << " ny= " << ny << " cellx= " << cellx.getValue() 
        << cellx.getUnit() << " celly= " 
        << celly.getValue() << celly.getUnit()
		  << " spwids= " << spwids
		  << " field= " <<   fieldid;

	  cal->writeCommand( os );
	  cout << "--Calibrater::writeCommand() called." << endl;
*/

     os << "Put Calibrater info into the history table." << endl;

     // what is the history table name?
     cout<< "The history table name is " << ms.historyTableName() << endl;

     /*
     LogSinkInterface& sink = os.localSink();
     int k = 0;
     cout<< "sink.getPriority(k)= " << sink.getPriority(k) << endl;
     cout<< "sink.getLocation(k)= " << sink.getLocation(k) << endl;
     cout<< "sink.getMessage(k)= " << sink.getMessage(k) << endl;
     */

     cal->writeHistory( os );
     cout << "--Calibrater::writeHistory() called." << endl;	  
  } catch( AipsError e ){
    cout << "Exception ocurred." << endl;
    cout << e.getMesg() << endl;
  }
};
