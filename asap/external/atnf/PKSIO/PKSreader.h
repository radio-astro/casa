//#---------------------------------------------------------------------------
//# PKSreader.h: Class to read Parkes multibeam data.
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
//# $Id: PKSreader.h,v 19.24 2009-09-29 07:33:39 cal103 Exp $
//#---------------------------------------------------------------------------
//# Original: 2000/08/02, Mark Calabretta, ATNF
//#---------------------------------------------------------------------------

#ifndef ATNF_PKSREADER_H
#define ATNF_PKSREADER_H

#include <atnf/PKSIO/PKSmsg.h>
#include <atnf/PKSIO/PKSrecord.h>

#include <casa/aips.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/BasicSL/String.h>

#include <casa/namespace.h>

// <summary>
// Class to read Parkes multibeam data.
// </summary>

// Return an appropriate PKSreader for a Parkes Multibeam dataset.
class PKSreader* getPKSreader(
        const String name,
        const Int retry,
        const Int interpolate,
        String &format);

// As above, but search a list of directories for it.
class PKSreader* getPKSreader(
        const String name,
        const Vector<String> directories,
        const Int retry,
        const Int interpolate,
        Int    &iDir,
        String &format);

// Open an appropriate PKSreader for a Parkes Multibeam dataset.
class PKSreader* getPKSreader(
        const String name,
        const Int retry,
        const Int interpolate,
        String &format,
        Vector<Bool> &beams,
        Vector<Bool> &IFs,
        Vector<uInt> &nChan,
        Vector<uInt> &nPol,
        Vector<Bool> &haveXPol,
        Bool   &haveBase,
        Bool   &haveSpectra);

// As above, but search a list of directories for it.
class PKSreader* getPKSreader(
        const String name,
        const Vector<String> directories,
        const Int retry,
        const Int interpolate,
        Int    &iDir,
        String &format,
        Vector<Bool> &beams,
        Vector<Bool> &IFs,
        Vector<uInt> &nChan,
        Vector<uInt> &nPol,
        Vector<Bool> &haveXPol,
        Bool   &haveBase,
        Bool   &haveSpectra);

class PKSreader : public PKSmsg
{
  public:
    // Destructor.
    virtual ~PKSreader() {};

    // Open the dataset.
    virtual Int open(
        const String inName,
        Vector<Bool> &beams,
        Vector<Bool> &IFs,
        Vector<uInt> &nChan,
        Vector<uInt> &nPol,
        Vector<Bool> &haveXPol,
        Bool   &haveBase,
        Bool   &haveSpectra) = 0;

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
        Double &bandwidth) = 0;

    // Get frequency parameters for each IF.
    virtual Int getFreqInfo(
        Vector<Double> &startFreq,
        Vector<Double> &endFreq) = 0;

    // Set data selection criteria.  Channel numbering is 1-relative, zero or
    // negative channel numbers are taken to be offsets from the last channel.
    // Coordinate system selection (only supported for SDFITS input):
    //   0: equatorial (RA,Dec),
    //   1: horizontal (Az,El),
    //   2: feed-plane,
    //   3: zenithal position angle of feed and elevation, (ZPA,El).
    virtual uInt select(
        const Vector<Bool> beamSel,
        const Vector<Bool> IFsel,
        const Vector<Int>  startChan,
        const Vector<Int>  endChan,
        const Vector<Int>  refChan,
        const Bool getSpectra = True,
        const Bool getXPol    = False,
        const Int  coordSys   = 0) = 0;

    // Find the range of the data selected in time and position.
    virtual Int findRange(
        Int    &nRow,
        Int    &nSel,
        Vector<Double> &timeSpan,
        Matrix<Double> &positions) = 0;

    // Read the next data record.
    virtual Int read(PKSrecord &pksrec) = 0;

    // Close the input file.
    virtual void close() = 0;

  protected:
    Bool  cGetSpectra, cGetXPol;
    Int   cCoordSys;

    Vector<uInt> cNChan, cNPol;
    Vector<Bool> cHaveXPol;
};

#endif
