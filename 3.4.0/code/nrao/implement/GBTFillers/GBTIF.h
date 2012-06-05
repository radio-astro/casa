//# GBTIF: this describes a GBT IF
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

#ifndef NRAO_GBTIF_H
#define NRAO_GBTIF_H

#include <casa/aips.h>

#include <nrao/GBTFillers/GBTCorrelation.h>

#include <casa/Arrays/Vector.h>
#include <casa/Containers/Block.h>
#include <measures/Measures/MDoppler.h>
#include <measures/Measures/MFrequency.h>
#include <casa/Quanta/MVFrequency.h>

//# Forward Declarations
namespace casa { //# NAMESPACE CASA - BEGIN
class String;
} //# NAMESPACE CASA - END

class GBTIFFiller;
class GBTLO1File;
class GBTCorrelation;

#include <casa/namespace.h>

// <summary>
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="tGBTIF.cc" demos="">
// </reviewed>

// <prerequisite>
//   <li> GBT FITS files
//   <li> GBTIFFiller
//   <li> GBTLO1File
//   <li> MS
// </prerequisite>
//
// <etymology>
// This describes a GBT IF and its associated correlations.
// </etymology>
//
// <motivation>
// It is necessary to know what the unique IFs are given a backend
// FITS file, an IF fits file, and any LO1 fits files.  It is also
// necessary to associated each correlation with an IF.
// </motivation>
//

class GBTIF
{
public:
    // This is used where there is NO associated IF FITS file
    // returns defaults.  This is constructed with one
    // GBTCorrelation - the default.
    GBTIF(uInt nchan, uInt nstate);
    
    // Matches bank and port in the iffiller (which has already
    // been constructed with the backend already selected) to
    // get the IF information.  Uses the appropriate LO1 file
    // for that row to get the LO1 doppler tracking and frequency
    // switching information along with the LO1 offsets which
    // finishes the description of the frequency axis.
    // ifFreq is the IF frequency to be used in the sky frequency
    // formula corresponding to channel 0.
    // bw is the total bandwith.  increases indicates
    // whether the IF frequency increases with increasing channel
    // number.  For the ACS backend, some bandwidths
    // decrease with increasing channel number.
    // If ifFreq is < 0, then the value in CENTER_IF is used.
    // If bw is < 0, then the value in BANDWDTH is used.
    GBTIF(uInt nchan, uInt nstate, const String &bank, Int port,
	  Double ifFreq, Double bw, Bool increases,
	  const GBTIFFiller &iffiller,
	  const GBTLO1File &lo1aFile, const GBTLO1File &lo1bFile,
	  Bool continuum);

    // Copy constructor.
    GBTIF(const GBTIF &other);

    ~GBTIF();

    // Assignment operator. Uses copy semantics.
    GBTIF &operator=(const GBTIF &other);

    // Comparison operator.
    Bool operator==(const GBTIF &other) const;
    
    // This one adds another associated IF row.  The intent is that
    // when you collect all of the rows associated with each unique IF.
    // This is a row from the IF fits file which shares this frequency
    // axis information.  Each row then contributes to one of the
    // associated correlations.
    void addIFRow(Int whichRow);

    // The vector of associated IF rows (rows from the IF fits file
    // which all share this frequency axis). 
    const Vector<Int> &ifrows() const {return itsIFrows;}

    // The number of associated GBTCorrelations
    uInt ncorr() const {return itsNcorr;}

    // Add this correlation to this GBTIF.
    void addCorr(const GBTCorrelation &corr);

    // Get a specific correlation.
    GBTCorrelation &getCorr(Int whichCorr) {return *(itsCorrs[whichCorr]);}

    // Get a specific const correlation.
    const GBTCorrelation &getCorr(Int whichCorr) const {return *(itsCorrs[whichCorr]);}

    // The values which define this IF.  These are used
    // in the comparison operators and they are also used
    // when filling the spectral window table.  These
    // describe a linear frequency axis.  The comparison
    // operators also require that nstate be the same
    // for equality to be true.
    // <group>
    // The number of channels along the frequency axis.
    // This is the same as used in the constructor.
    uInt nchan() const {return itsNchan;}

    // The number of states.
    uInt nstate() const {return itsNstate;}

    // The frequency at the refChan(), in Hz.
    // This defaults to refChan() if insufficient information
    // is available to construct the whole frequency axis.
    Double refFreq() const {return itsRefFreq;}

    // The channel spacing (may be negative) with positive 
    // increasing channel number, in Hz.  This defaults to 1.0.
    Double deltaFreq() const {return itsDeltaFreq;}

    // The reference channel.  This will be nchan/2 or 0 if nchan==1.
    Double refChan() const {return itsRefChan;}

    // The reference frame.  This defaults to Mfrequency::TOPO.
    MFrequency::Types refFrame() const {return itsRefFrame;}

    // The tolerance, in Hz.  This defaults to 10.0.  The
    // comparison operators use this when comparing 
    // refFreq and deltaFreq.
    Double tolerance() const {return itsTol;}

    // The rest frequency from the appropriate LO1 file.
    const MVFrequency &restFreq() const {return itsRestFreq;}

    // The doppler type from the LO1 file
    MDoppler::Types dopType() const {return itsDopType;}

    // The source velocity from the LO1 file
    Double vsource() const {return itsVsource;}
    // </group>

    // Offsets in refFreq for each state from the LO1 table.
    // These will be all 0 for non-frequency switched data,
    // and if the LO1 file is missing.
    const Vector<Double> &offsets() const {return itsOffsets;}

    // The SPECTRAL_WINDOW_ID appropriate for this state.
    // Returns -1 if unset.
    Int spwId(Int state) const {return itsSpwId[state];}

    // Set the associated SPECTRAL_WINDOW_ID for the given state.
    void setSpwId(Int swid, Int state) {itsSpwId[state] = swid;}
private:
    uInt itsNchan, itsNstate;
    Double itsRefFreq, itsRefChan, itsDeltaFreq, itsTol, itsVsource;
    MFrequency::Types itsRefFrame;
    MVFrequency itsRestFreq;
    MDoppler::Types itsDopType;

    //# these are always both itsNstate elements long
    Vector<Double> itsOffsets;
    Vector<Int> itsSpwId;

    Vector<Int> itsIFrows;

    uInt itsNcorr;
    Block<GBTCorrelation *> itsCorrs;

    void clearCorrs();

    //# undefined and unavailable
    GBTIF();
};

//#ifndef AIPS_NO_TEMPLATE_SRC
//#include <nrao/GBTFillers/GBTIF.tcc>
//#endif //# AIPS_NO_TEMPLATE_SRC
#endif


