//# MSDefiner.h : class for the creation of a MeasurementSet
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

#ifndef NFRA_MSDEFINER_H
#define NFRA_MSDEFINER_H

#include <ms/MeasurementSets/MeasurementSet.h>
#include <nfra/Wsrt/DataSource.h>


#include <casa/namespace.h>
// <summary> 
// Class for the creation of a MeasurementSet.
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <synopsis>
// This class defines how to create the MeasurementSet.
// </synopsis>


class MSDefiner
{
public: 
    // Constructor: remember name of MeasurementSet to be created
    MSDefiner(const String& msName, DataSource* pds);

    // Destructor:
    ~MSDefiner();

    // Return name of class
    String className() const
	{return "MSDefiner";}

    // Return name of MeasurementSet
    const String& msName() const
	{return itsMSName;}

    // Create MeasurementSet
    MeasurementSet run() const;

    // Create MS descriptor
    TableDesc msTableDesc() const;

private:   
    String itsMSName;
    DataSource* itspDS;
};

#endif
