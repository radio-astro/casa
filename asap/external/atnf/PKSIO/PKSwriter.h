//#---------------------------------------------------------------------------
//# PKSwriter.h: Class to write out Parkes multibeam data.
//#---------------------------------------------------------------------------
//# livedata - processing pipeline for single-dish, multibeam spectral data.
//# Copyright (C) 2000-2009, Australia Telescope National Facility, CSIRO
//#
//# This file is part of livedata.
//#
//# livedata is free software: you can redistribute it and/or modify it under
//# the terms of the GNU General Public License as published by the Free
//# Software Foundation, either version 3 of the License, or (at your option)
//# any later version.
//#
//# livedata is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with livedata.  If not, see <http://www.gnu.org/licenses/>.
//#
//# Correspondence concerning livedata may be directed to:
//#        Internet email: mcalabre@atnf.csiro.au
//#        Postal address: Dr. Mark Calabretta
//#                        Australia Telescope National Facility, CSIRO
//#                        PO Box 76
//#                        Epping NSW 1710
//#                        AUSTRALIA
//#
//# http://www.atnf.csiro.au/computing/software/livedata.html
//# $Id: PKSwriter.h,v 19.17 2009-09-29 07:33:39 cal103 Exp $
//#---------------------------------------------------------------------------

#ifndef ATNF_PKSWRITER_H
#define ATNF_PKSWRITER_H

#include <atnf/PKSIO/PKSrecord.h>

#include <casa/aips.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/BasicSL/Complex.h>
#include <casa/BasicSL/String.h>

#include <casa/namespace.h>

// <summary>
// Class to write out Parkes multibeam data.
// </summary>

class PKSwriter
{
  public:
    // Destructor.
    virtual ~PKSwriter() {};

    // Create the output file and and write static data.
    virtual Int create(
        const String outName,
        const String observer,
        const String project,
        const String antName,
        const Vector<Double> antPosition,
        const String obsMode,
        const String bunit,
        const Float  equinox,
        const String dopplerFrame,
        const Vector<uInt> nChan,
        const Vector<uInt> nPol,
        const Vector<Bool> haveXPol,
        const Bool havebase) = 0;

    // Write the next data record.
    virtual Int write (
        const PKSrecord &pksrec) = 0;

    // Write a history record.
    virtual Int history(const String text) {return 0;};
    virtual Int history(const char *text)  {return 0;};

    // Close the output file.
    virtual void close() = 0;

  protected:
    Bool cHaveBase;
    uInt cNIF;
    Vector<Bool> cHaveXPol;
    Vector<uInt> cNChan, cNPol;
};

#endif
