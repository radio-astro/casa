//# tMsPlot.cc: Miscellaneous information related to an image
//# Copyright (C) 1998,1999,2000,2002
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
//#
//# $Id$
//#
//#--------------------------------------------------------------------------
//# Change Log
//# Date	Name		Description
//#

#include <msvis/MSPlot/MsPlot.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <tables/Tables/Table.h>

#include <casa/Inputs/Input.h>
#include <casa/Logging/LogIO.h>

#include <casa/namespace.h>


int main( int argc, char **argv)
{
  LogIO os(LogOrigin("tMsPlot", "main()", WHERE));

  // Get inputs
  Input inputs(1);
  inputs.create( "file", "", "Path to the measurement set used for testing" );  // This specifies the inputs we are looking for.

  inputs.readArguments( argc, argv );
  String inFile = inputs.getString( "in" );

  // Create the plotter for plotting the Measurement Set. We need to
  // first create a MeasurementSet to give to the plotter.
  //MeasurementSet MS( inFile );
  MsPlot msPlotter( inFile );

  // List of public methods:
  /*
    reset(rmplotter)

    settitle
    setxlabel
    setylabel
    setplotoption -- may not want to test this here, or make it a seperate
                     test for when it is moved elsewhere.
    getplotoption -- ditto
    unsetplotoption -- ditto
    
    setData( antennaNames, antennaIndex, spwNames, spwIndex, fieldNames,
             fieldIndex, uvDists, times, correlations )
    iterPlotStart( dataStr, iterAxesStr )
    iterPlotNext()
    iterPlotStop()
    plot()
    setSpectral
    createSpecSubMS
    flagsData
    resetMS
    anteannaNames

    # probably should be private methods
    setaxes( xAxes, yAxes ) -- maybe
    setTitleLabel
    antennaPosistions
    polarNchannel
    derivedValues
    readTime
    doesColumnExist()
    addTotalChanNum
    resetAntennaFlagRow
   */

  
  exit(0);
}
