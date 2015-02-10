//# GBTLO1File.h: Digests the LO1 FITS file.
//# Copyright (C) 2001,2002
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

#ifndef NRAO_GBTLO1FILE_H
#define NRAO_GBTLO1FILE_H

//#! Includes go here

#include <casa/Arrays/Vector.h>
#include <nrao/FITS/GBTFITSBase.h>
#include <nrao/FITS/GBTStateTable.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MDoppler.h>
#include <measures/Measures/MFrequency.h>
#include <measures/Measures/MPosition.h>
#include <casa/Quanta/MVDirection.h>
#include <casa/Quanta/MVFrequency.h>
#include <tables/Tables/ScalarColumn.h>
#include <casa/BasicSL/String.h>

//# Forward Declarations
namespace casa { //# NAMESPACE CASA - BEGIN
class Table;
class FITSTable;
} //# NAMESPACE CASA - END


// <summary>
// Digests the LO1 FITS file.
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
// </prerequisite>
//
// <etymology>
// </etymology>
//
// <synopsis>
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// </motivation>
//
//
// <thrown>
//#! A list of exceptions thrown if errors are discovered in the function.
//#! This tag will appear in the body of the header file, preceding the
//#! declaration of each function which throws an exception.
//    <li>
//    <li>
// </thrown>
//
// <todo asof="yyyy/mm/dd">
// </todo>

class GBTLO1File : public GBTFITSBase
{
public:
    // This constructs a GBTLO1File that has no primary keywords.
    // or associated tables
    GBTLO1File();

    virtual ~GBTLO1File();

    // attach this object to a new FITS file.  If there are problems with the
    // file, the return value is False and appropriate warnings are sent to the
    // logger.  The resulting object is not attached to any file.
    virtual Bool attach(const String &fitsFile);

    // Is this object attached to a FITS file.  This base class is never
    // attached to a file.
    virtual Bool isAttached() const {return itsAttached;}

    // The name of the attached FITS file.  Returns an empty string if
    // it is not attached.
    virtual const String &file() const {return itsName;}

    // The position of the antenna.
    const MPosition &position() const {return itsPosition;}

    // The requested rest frequency
    const MVFrequency &restfrq() const {return itsRestfrq;}

    // The requested doppler tolerance in Hz
    Double reqdptol() const {return itsReqdptol;}

    // Get values from the LO1TABLE at the specified time (UTC) - or
    // the first row before that time if there is not a row near that time.
    // <group>
    // Telescope pointing direction.
    const MDirection &dir(const MVEpoch &time) const;
    // lo1freq, this also requires a state. The appropriate offset for
    // that state is from the LO1STATE table and is added to the frequency
    // from the LO1TABLE.  If that state isn't found, an offset of 0.0 is used.
    const MVFrequency &lo1freq(const MVEpoch &time, uInt state) const ;
    // radial velocity of the reference frame wrt the observer
    Double vframe(const MVEpoch &time) const;
    // radial velociy of the source wrt the observer
    Double rvsys(const MVEpoch &time) const;
    // </group>

    // the SOUVEL binary table
    const Table &souvel() const {return *itsSouvel;}

    // the STATE table used here
    const GBTStateTable &state() const {return itsState;}

    // The DMJD column
    const ROScalarColumn<Double> &dmjd() const {return itsDMJDCol;}

    // Are there non-zero offsets (implies frequency switching)
    Bool isFreqSwitched() const;

    // Return the first LO1 setting found here plus any state offset from STATE
    // table, one value per state.  Also set is the factor which, when multiplied
    // by the TOPO centric frequencies gives the reference frame frequencies 
    // corresponding to the the VELOCITY at the same time as the returned LO1 setting.
    // The desired reference frame, velocity definition,
    // and source velocity in the reference frame and definition
    // are set.
    Vector<Double> firstLO1(Double &factor, MFrequency::Types &refframe,
		    MDoppler::Types &doptype, Double &vsource) const;

private:
    Bool itsAttached;
    String itsName;

    MPosition itsPosition;
    MVFrequency itsRestfrq;
    Double itsReqdptol;

    //# freqoff column from LO1STATE
    Vector<Double> itsFreqoff;

    mutable MDirection itsCurrentDir;
    mutable MVEpoch itsCurrentTime;
    mutable Int itslo1Row;
    mutable MVFrequency itsLo1freq;

    ROScalarColumn<Double> itsDMJDCol, itsRACol, itsDECCol,
	itsLO1FREQCol, itsVFRAMECol, itsRVSYSCol;

    Double itsEquinox;
    MDirection::Types itsDirType;

    Table *itsSouvel, *itsLO1Table;
    GBTStateTable itsState;

    // initialize with default, unattached values
    void init();

    // these functions do the appropriate thing, but do not verify that
    // FITSTable is set to the appropriate extension.
    // <group>
    void setLO1Table(FITSTable &lo1Tab);
    void setLO1State(FITSTable &stateTab);
    void setSouvel(FITSTable &souvelTab);
    // </group>

    // sets itslo1Row to the appropriate row in the lo1 table where the 
    // DMJD value is near or just prior to the requested time.
    void lo1Row(const MVEpoch &time) const;

    // undefined and unavailable
    GBTLO1File(const GBTLO1File &other);
    GBTLO1File &operator=(const GBTLO1File &other);
};

#endif
