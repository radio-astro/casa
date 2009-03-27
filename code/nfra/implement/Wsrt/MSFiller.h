//# MSFfiller.h : class for filling a MeasurementSet from a DataSource
//# Copyright (C) 1996,1997,1999,2000
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
//# $Id$


#ifndef NFRA_MSFILLER_H
#define NFRA_MSFILLER_H

#include <ms/MeasurementSets/MeasurementSet.h>
#include <nfra/Wsrt/DataSource.h>


#include <casa/namespace.h>
// <summary> 
// Class for filling a MeasurementSet from a DataSource.
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <synopsis>
// This class is filling a MeasurementSet from a DataSource object.
// </synopsis>


class MSFiller
{
public:
    // Constructor: remember DataSource and MeasurementSet
    MSFiller (DataSource* pds,MeasurementSet* pms);

    // Destructor
    ~MSFiller ();

    // Return name of class
    String className() const;
 
    // Do fill the MeasurementSet from the DataSource
    void run (Int bandnr,
	      Int nChanPerTile,
	      Int nBasePerTile);

private:
    DataSource* itspDS;
    MeasurementSet* itspMS;
    Int itsBand;

    // Fill the main table; called by run()
    void fill(MeasurementSet&,
	      Int nChanPerTile,
	      Int nBasePerTile);

    // Fill the subtables; called by run()
    // <group>
    void fill(MSAntenna&);
    void fill(MSDataDescription&); 
    void fill(MSFeed&); 
    void fill(MSField&); 
    void fill(MSObservation&); 
    void fill(MSPointing&); 
    void fill(MSPolarization&); 
    void fill(MSProcessor&); 
    void fill(MSSpectralWindow&); 
    void fill(MSState&); 
    void fill(MSSysCal&); 
    void fill(MSWeather&); 
    // </group>
};


inline String MSFiller::className() const
{
    return "MSFiller";
}

 
#endif
