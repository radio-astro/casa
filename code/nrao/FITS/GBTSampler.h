//# GBTSampler: defines GBTSampler, holds SAMPLER table information
//# Copyright (C) 2003
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

#ifndef NRAO_GBTSAMPLER_H
#define NRAO_GBTSAMPLER_H

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/BasicSL/String.h>

#include <casa/namespace.h>
//# Forward Declarations
class GBTBackendTable;

// <summary>
//# This defines GBTSampler, a class to hold backend SAMPLER table information.
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="tGBTSampler.cc" demos="">
// </reviewed>

// <prerequisite>
//   <li> GBT FITS files.
//   <li> GBT ACS FITS file.
//   <li> GBTBackendTable
// </prerequisite>
//
// <etymology>
// This holds information typically found in the SAMPLER table of the GBT
// backend FITS files.
// </etymology>
//
// <motivation>
// It is useful to provide a common interface to access the information
// in the sampler table.  In addition, since only the GBT ACS
// follows the full convention and provides a complete sampler table, it
// is useful to make the other backends appear as if they had this
// table so that downstream code need not know about the differences.
// </motivation>
//
// </todo>

class GBTSampler
{
public:
    // construct from an existing GBTBackendTable and
    // possibly using the number of IF rows (SP backend only)
    GBTSampler(const GBTBackendTable &backendTab, Int nIFrows);

    // Copy-constructor, uses copy semantics.
    GBTSampler(const GBTSampler &other);

    ~GBTSampler() {;}

    // Assignment operator, uses copy semantics.
    GBTSampler &operator=(const GBTSampler &other);

    // The number of rows (samplers) present.
    uInt nrows() const {return itsBankA.nelements();}

    // BANK_A for a given row.
    // For ACS, returns value in BANK_A column
    // SP receiver row is now correctly translated to BANK_A
    // values for Sqr/Cross mode and Square modes.
    // For the DCR, returns value of INPBANK keyword.
    const String &bankA(uInt whichRow) const {return itsBankA[whichRow];}

    // BANK_B for a given row.
    // For ACS, returns value in BANK_B column.
    // SP receiver row is now correctly translated to BANK_B
    // values for Sqr/Cross mode and Square modes.
    // For DCR bankB==bankA.
    const String &bankB(uInt whichRow) const {return itsBankB[whichRow];}

    // PORT_A for a given row.
    // For ACS, returns value in PORT_A column.
    // SP receiver row is now correctly translated to PORT_A
    // values for Sqr/Cross mode and Square modes.
    // For DCR, this is the value of CHANNELID + 1.
    Int portA(uInt whichRow) const {return itsPortA[whichRow];}

    // PORT_B for a given row.
    // For ACS, returns value in PORT_B column.
    // SP receiver row is now correctly translated to PORT_B
    // values for Sqr/Cross mode and Square modes.
    // For DCR portB==portA.
    Int portB(uInt whichRow) const {return itsPortB[whichRow];}

    // Does this SAMPLER have any cross-polarization data.
    // For ACS, this returns True if POLARIZE=="CROSS"
    // For SpectralProcessor this returns true in SquareCross mode.
    // For DCR data this always returns False.
    Bool hasCross() const {return itsHasCross;}

    // Return the number of channels for this row of the sampler table.
    uInt nchan(uInt whichRow) const {return itsNchan[whichRow];}

private:
    Vector<String> itsBankA, itsBankB;
    Vector<Int> itsPortA, itsPortB, itsNchan;
    Bool itsHasCross;

    //# unimplemented an unavailable
    GBTSampler();
};
#endif


