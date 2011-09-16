//#---------------------------------------------------------------------------
//# PKSSDwriter.h: Class to write Parkes multibeam data to an SDFITS file.
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
//# $Id: PKSSDwriter.h,v 19.17 2009-09-29 07:33:38 cal103 Exp $
//# Original: 2000/07/21, Mark Calabretta, ATNF
//#---------------------------------------------------------------------------

#ifndef ATNF_PKSSDWRITER_H
#define ATNF_PKSSDWRITER_H

#include <atnf/PKSIO/PKSwriter.h>
#include <atnf/PKSIO/PKSrecord.h>
#include <atnf/PKSIO/SDFITSwriter.h>

#include <casa/aips.h>
#include <casa/stdio.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/BasicSL/Complex.h>
#include <casa/BasicSL/String.h>

#include <casa/namespace.h>

// <summary>
// Class to write Parkes multibeam data to an SDFITS file.
// </summary>

class PKSSDwriter : public PKSwriter
{
  public:
    // Default constructor.
    PKSSDwriter();

    // Destructor.
    virtual ~PKSSDwriter();

    // Set message disposition.
    virtual Int setMsg(
        FILE *fd = 0x0);

    // Create the SDFITS file and write static data.
    virtual Int create(
        const String sdName,
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
        const Bool   haveBase);

    // Write the next data record.
    virtual Int write(
        const PKSrecord &pksrec);

    // Write a history record.
    virtual Int history(const String text);
    virtual Int history(const char *text);

    // Close the SDFITS file.
    virtual void close();

  private:
    // Masks declaration in parent class.
    Vector<uInt> cHaveXPol;

    SDFITSwriter cSDwriter;
};

#endif
