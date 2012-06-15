//# GBTGOFile.h: GBTGOFile serves up the information from a GBT GO FITS file
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

#ifndef NRAO_GBTGOFILE_H
#define NRAO_GBTGOFILE_H

//#! Includes go here

#include <nrao/FITS/GBTFITSBase.h>

#include <measures/Measures/MDirection.h>
#include <measures/Measures/MeasFrame.h>
#include <measures/Measures/MRadialVelocity.h>
#include <casa/Quanta/MVFrequency.h>
#include <casa/BasicSL/String.h>

//# Forward Declarations

// <summary>
// GBTGOFile serves up the information from a GBT GO FITS file.
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

class GBTGOFile : public GBTFITSBase
{
public:
    // initialize with default values, no primary keywords
    GBTGOFile();

    // initialize with the indicated GO file.
    GBTGOFile(const String &gofile);

    ~GBTGOFile() {;}

    // attach this object to a new FITS file.  If there are problems with
    // the file, the return value is Falso and appropriate warnings are
    // sent to the logger.  The resulting object is not attached to any file.
    Bool attach(const String &gofile);

    // the file name used at construction or most recent reopen
    const String &file() const {return itsName;}

    // is this a valid GO file (only consists of a null array PDA
    // with no extensions).
    Bool isValid() const {return itsValid;}

    // is this attached to a FITS file
    Bool isAttached() const {return itsAttached;}

    // standard fields - these are NOT marked as handled.  handleKeyword
    // must be used for that.
    // <group>
    const String &observer() const {return itsObserver;}
    const String &procname() const {return itsProcname;}
    const String &proctype() const {return itsProctype;}
    Int procseqn() const {return itsProcseqn;}
    Int procsize() const {return itsProcsize;}
    const String obstype() const {return itsObstype;}
    const String &swtchsig() const {return itsSwtchsig;}
    const String &swstate() const {return itsSwstate;}
    Int laston() const {return itsLaston;}
    Int lastoff() const {return itsLastoff;}
    // reference direction - this includes the RADESYS, RA, DEC and
    // COORDSYS keywords.
    const MDirection &refdir() const {return itsCmddir;}
    const Double &equinox() const {return itsEquinox;}
    // rest frequency - incorporates RESTFRQ and VELDEF values.
    const MVFrequency &restfrq() const {return itsRestfrq;}
    // Velocity of line being tracked  - incorporates VELOCITY and VELDEF values.
    const MRadialVelocity &velocity() const {return itsVelocity;}
    // </group>

    // intended sky frequency
    const MVFrequency &skyfreq() const {return itsSkyfrq;}

    // the raw keyword values as found in the GO file
    // <group>
    const String &coordsys() const {return itsCoordsys;}
    const String &radesys() const {return itsRadesys;}
    const Double &majorCoord() const {return itsMajor;}
    const Double &minorCoord() const {return itsMinor;}
    const Double &velocityKW() const {return itsVelocityKW;}
    const String &veldef() const {return itsVeldef;}
    // </group>

    // Does this have the doppler keywords, otherwise it will
    // have the SKYFREQ keyword
    Bool isDoppler() const {return itsDoppler;}

    // Does this use the OTHER coordinate system?  If so, the refdir 
    // direction is not useful.
    Bool isOtherCoordsys() const {return itsOtherCoordsys;}
private:
    Bool itsValid, itsAttached;

    Int itsProcseqn, itsProcsize, itsLaston, itsLastoff;

    String itsObserver, itsProcname, itsProctype, itsSwtchsig, itsName, 
	itsObstype, itsSwstate, itsCoordsys, itsRadesys, itsVeldef;

    MDirection itsCmddir;

    Double itsEquinox, itsMajor, itsMinor, itsVelocityKW;

    MRadialVelocity itsVelocity;
    MVFrequency itsRestfrq, itsSkyfrq;

    Bool itsDoppler, itsOtherCoordsys;

    MeasFrame itsFrame;

    void init();

    // unavailable
    GBTGOFile(const GBTGOFile &other);
    GBTGOFile &operator=(const GBTGOFile &other);
};

#endif


