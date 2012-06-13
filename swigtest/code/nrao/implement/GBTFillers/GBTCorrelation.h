//# GBTCorrelation: GBTCorrelation holds spw and pol info
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

#ifndef NRAO_GBTCORRELATION_H
#define NRAO_GBTCORRELATION_H

#include <casa/aips.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Containers/SimOrdMap.h>
#include <casa/BasicSL/String.h>

#include <casa/namespace.h>

//# Forward Declarations
class GBTFeed;

// <summary>
// GBTCorrelation holds spectral window and polarization information
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="tGBTCorrelation.cc" demos="">
// </reviewed>

// <prerequisite>
//   <li> The POLARIZATION table of the MeasurementSet.
//   <li> The SPECTRAL_WINDOW table.
//   <li> The DATA_DESCRIPTION table.
//   <li> The GBT FITS files.
//   <li> The GBT filler.
// </prerequisite>
//
// <etymology>
// This holds information that describes the correlation type 
// appropriate for one row of a MS.  This is used for GBT data
// and within the GBT filler.  This information includes the
// data description id and the two components of the data description
// table (spectral window id and polarization id).  It also 
// includes information necessary to fill the polarization table.
// Finally, it keeps a record of which sampler row contributed to
// each pair of receptors in a given correlation.
// </etymology>
//
// <motivation>
// When this information was being held by a larger class, it was getting
// confused with other issues related to that class.  By putting it in a
// smaller, simpler class the maintenance will be easier and usage will
// be less confusing.
// </motivation>

class GBTCorrelation
{
public:
    // Construct one with no feed information.  This will have
    // one correlation labelled "XX".  The number of states
    // and channels must be specified at construction time.
    // Bank defaults to the empty string.
    GBTCorrelation(Int nstate, Int nchan);

    // Use the indicated FEED to prepare to record the
    // sampler row information for each receptor pair.
    // Initially has space available for each possible
    // combination of receptor pairs.  When freeze is
    // called, those pairs not actually set via setSamplerRow
    // are removed and numCorr then reflects the actual
    // receptor pairs in use (as do corrType and corrProduct).
    // The number of states and channels must be specified at construction.
    // The bank is used to discriminate against similar correlations
    // in the ACS but from different banks.
    GBTCorrelation(const GBTFeed &feed, Int nstate, Int nchan,
		   const String &bank);

    // copy constructor
    GBTCorrelation(const GBTCorrelation &other);

    ~GBTCorrelation() {;}

    // Assignment operator, uses copy syntax.
    GBTCorrelation &operator=(const GBTCorrelation &other);

    // comparison operator.  Checks shape, corrType, corrProduct
    // and receptors.
    Bool operator==(const GBTCorrelation &other) const;

    // The number of states used in the constructor.
    Int nstate() const {return itsNstate;}

    // The bank used in the constructor.
    const String &bank() const {return itsBank;}

    // Set the name of the other bank for switched data
    void setsrbank(const String &srbank) {itsSrbank = srbank;}

    // Get the name of the other bank for switched data
    const String &srbank() const {return itsSrbank;}

    // The DATA_DESC_ID appropriate for this correlation and the
    // requested state.  Returns -1 if unset.
    Int dataDescId(Int state) const {return itsDataDescId[state];}

    // Set the associated DATA_DESC_ID for the given state.
    // It can be set and changed after freeze has been called.
    // This will typically be the last item set.  
    void setDataDescId(Int ddid, Int state) {itsDataDescId[state] = ddid;}

    // The POLARIZATION_ID appropriate for this correlation and state.
    // Returns -1 if unset.
    Int polId() const {return itsPolId;}

    // Set the associated POLARIZATION_ID for the given state.
    // It can be set and changed after freeze has been called.
    // This will typically be set before DATA_DESC_ID is available.
    // It can be set and changed after freeze has been called.
    void setPolId(Int pid) {itsPolId = pid;}

    // Set up information about a given sampler row.
    // If the constructor without GBTFeed was used, then this
    // always is associated with the XX correlation, 
    // irrespective of the actual values of polA and polB.
    // If not, then this returns False if the polA or polB
    // receptors are not found in the feed used in the constructor.
    // This returns False if this polA and polB combination
    // has already been set here.  Returns False after freeze() 
    // has been called.  The names of the two receptors from the IF
    // fits file are also passed in.  These are later available
    // via receptors()
    Bool setSamplerRow(Int samplerRow, const String &polA, const String &polB,
		       const String &receptorA, const String &receptorB);

    // Merge the unfrozen aspects of otherCorr with this GBTCorr.  Returns
    // False if the setSamplerRow would have returned False had these been
    // added in that way or if the bank in otherCorr does not match
    // the one used in the constructor for this object.  The bank check
    // is only done when bankCheck is True.  That should only be done
    // for ACS data.
    Bool mergeCorr(const GBTCorrelation &otherCorr, Bool bankCheck);

    // Does the same checks that mergeCorr does without actually merging
    // anything.
    Bool mergeCheck(const GBTCorrelation &otherCorr, Bool bankCheck);

    // Freeze this correlation with the sampler row information already
    // provided.  Additional calls to setSamplerRow return False and are
    // ignored.
    void freeze();

    // The samplerRows associated with corrType.  The actual
    // returned value is not specified if it has not yet been
    // frozen.
    const Vector<Int> &samplerRows() const {return itsSamplerRows;}

    // the number of correlations.  Returns 0 if called before
    // freeze() has been called.
    Int numCorr() const {return itsFrozen ? itsNumCorr:0;}

    // The polarization of the numCorr correlations.  The
    // actual returned value is not specified if it has not
    // yet been frozen.
    const Vector<Int> &corrType() const {return itsCorrType;}

    // The polarizations as strings.  Auto-correlations are returned
    // as single letters (i.e. "XX" becomes "X").
    const Vector<String> &corrTypeAsString() const {return itsCorrTypeString;}

    // The receptor cross-products.  The actual returned value is
    // not specified if it has not yet been frozen
    const Matrix<Int> &corrProduct() const {return itsCorrProduct;}

    // The receptors names used, one for each correlation pair.
    // For auto-correlation data, this will be just the original
    // receptor name.  For cross-correlation data, it will be
    // receptor A name "x" receptor B name e.g. "R1xL1".  This
    // is used when getting the tcal and trx values from the
    // rcvr tables.
    const Vector<String> &receptors() const {return itsReceptors;}

    // The shape of this correlation
    const IPosition &shape() const {return itsShape;}
private:
    Int itsNstate, itsPolId;
    uInt itsNumCorr, itsNumPol;
    Bool itsFrozen, itsIsDefault;
    String itsBank, itsSrbank;

    IPosition itsShape;

    //# this is always itsNstate elements long
    Vector<Int> itsDataDescId;

    //# maps from the polarization string to a receptor (index into feed.polType()).
    SimpleOrderedMap<String, Int> itsRecptMap;

    Vector<Int> itsCorrType, itsSamplerRows;
    Matrix<Int> itsCorrProduct;
    Vector<String> itsReceptors, itsCorrTypeString;

    // unimplemented and unavailable
    GBTCorrelation();
};

//#ifndef AIPS_NO_TEMPLATE_SRC
//#include <nrao/GBTFillers/GBTCorrelation.tcc>
//#endif //# AIPS_NO_TEMPLATE_SRC
#endif
