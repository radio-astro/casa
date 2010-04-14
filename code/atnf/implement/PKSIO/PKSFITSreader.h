//#---------------------------------------------------------------------------
//# PKSFITSreader.h: Class to read Parkes Multibeam data from a FITS file.
//#---------------------------------------------------------------------------
//# Copyright (C) 2000-2008
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
//# $Id: PKSFITSreader.h,v 19.17 2008-11-17 06:38:05 cal103 Exp $
//#---------------------------------------------------------------------------
//# This class is basically a wrapper class for reading data from either an
//# MBFITS (single dish variant of RPFITS) or SDFITS file using the relevant
//# lower-level classes.  It translates AIPS++isms to/from basic C++.
//#
//# Original: 2000/08/02, Mark Calabretta, ATNF
//#---------------------------------------------------------------------------

#ifndef ATNF_PKSFITSREADER_H
#define ATNF_PKSFITSREADER_H

#include <atnf/PKSIO/FITSreader.h>
#include <atnf/PKSIO/PKSrecord.h>
#include <atnf/PKSIO/PKSreader.h>

#include <casa/aips.h>
#include <casa/stdio.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/BasicSL/Complex.h>
#include <casa/BasicSL/String.h>

#include <casa/namespace.h>

// <summary>
// Class to read Parkes Multibeam data from a FITS file.
// </summary>

class PKSFITSreader : public PKSreader
{
  public:
    // Default constructor.
    PKSFITSreader(
        const String fitsType,
        const Int    retry = 0,
        const Bool   interpolate = True);

    // Destructor.
    virtual ~PKSFITSreader();

    // Open the FITS file for reading.
    virtual Int open(
        const String fitsName,
        const String antenna,
        Vector<Bool> &beams,
        Vector<Bool> &IFs,
        Vector<uInt> &nChan,
        Vector<uInt> &nPol,
        Vector<Bool> &haveXPol,
        Bool         &haveBase,
        Bool         &haveSpectra);

    // Get parameters describing the data.
    virtual Int getHeader(
        String &observer,
        String &project,
        String &antName,
        Vector<Double> &antPosition,
        String &obsType,
        String &bunit,
        Float  &equinox,
        String &dopplerFrame,
        Double &mjd,
        Double &refFreq,
        Double &bandwidth);

    // Get frequency parameters for each IF.
    virtual Int getFreqInfo(
        Vector<Double> &startFreq,
        Vector<Double> &endFreq);

    // Set data selection criteria.  Channel numbering is 1-relative, zero or
    // negative channel numbers are taken to be offsets from the last channel.
    virtual uInt select(
        const Vector<Bool> beamSel,
        const Vector<Bool> IFsel,
        const Vector<Int>  startChan,
        const Vector<Int>  endChan,
        const Vector<Int>  refChan,
        const Bool getSpectra = True,
        const Bool getXPol    = False,
        const Bool getFeedPos = False,
        const Bool getPointing = False,
        const Int  coordSys   = 0);

    // Find the range of the data selected in time and position.
    virtual Int findRange(
        Int    &nRow,
        Int    &nSel,
        Vector<Double> &timeSpan,
        Matrix<Double> &positions);

    // Read the next data record.
    virtual Int read(PKSrecord &pksrec);

    // Close the FITS file.
    virtual void close();

  private:
    Int    *cBeams, *cIFs;
    uInt   cNBeam, cNIF;
    MBrecord cMBrec;
    FITSreader  *cReader;

    Char* trim(char *string);
};

#endif
