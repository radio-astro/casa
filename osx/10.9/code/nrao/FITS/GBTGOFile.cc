//# GBTGOFile.cc: GBTGOFile serves up the information from a GBT GO FITS file
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
//# $Id$

//# Includes

#include <nrao/FITS/GBTGOFile.h>

#include <casa/Arrays/Vector.h>
#include <fits/FITS/fitsio.h>
#include <fits/FITS/fits.h>
#include <fits/FITS/hdu.h>
#include <fits/FITS/FITSTable.h>
#include <casa/BasicMath/Math.h>
#include <casa/OS/Path.h>
#include <casa/Logging.h>
#include <casa/Utilities/Regex.h>
#include <measures/Measures/MDoppler.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MeasTable.h>


GBTGOFile::GBTGOFile()
{attach("");}

GBTGOFile::GBTGOFile(const String &gofile)
{
    // set antenna in frame to telescop, if possible
    MPosition pos;
    if (MeasTable::Observatory(pos,telescop())) {
	itsFrame.set(pos);
    }
    attach(gofile);
}

Bool GBTGOFile::attach(const String &gofile)
{
    Regex leading("^ *");
    init();
    itsName = gofile;
    // if this is a null string, "", we're done
    if (gofile != "") {
	itsAttached = False;
	itsValid = True;
	// open the file
	Path filePath(gofile);
	FitsInput io(filePath.expandedName().chars(), FITS::Disk);
	if (io.err() || io.eof()) {
	    // some sort of warning is issued
	    LogIO os(LogOrigin("GBTGOFile","reopen"));
	    os << LogIO::WARN << WHERE
	       << "There was a problem opening the file : " << filePath.expandedName() << "\n";
	    if (io.err()) {
		os << "FITS Error : " << io.err();
	    } else {
		os << "End of file encountered before any data was read.  File may be empty."
		   << LogIO::POST;
	    }
	    os << "\nThis file will be ignored" << LogIO::POST;
	} else {
	    if (io.hdutype() != FITS::PrimaryArrayHDU) {
		LogIO os(LogOrigin("GBTGOFile","reopen"));
		os << LogIO::WARN << WHERE
		   << "The first FITS Header in the file " << filePath.expandedName()
		   << " is not of the expected type\n" 
		   << "This file will be ignored." << LogIO::POST;
	    } else {
		if (io.datatype() != FITS::BYTE) {
		    LogIO os(LogOrigin("GBTGOFile","reopen"));
		    os << LogIO::WARN << WHERE
		       << "Unexpected data type for first header data unit in the FITS file "
		       << filePath.expandedName() << "\n"
		       << "This file will be ignored" << LogIO::POST;
		} else {
		    BytePrimaryArray pa(io);

		    setPrimaryKeys(FITSTabular::keywordsFromHDU(pa));
		    // at this point the file is valid
		    itsValid = True;
		    itsAttached = True;

		    // pick off the keywords that are there that we know about
		    // leave it to the consumer to mark these as used
		    if (primaryKeys().fieldNumber("OBSERVER") >= 0) {
			itsObserver = primaryKeys().asString("OBSERVER");
		    } else {
			itsObserver = "";
		    }

		    if (primaryKeys().fieldNumber("PROCNAME") >= 0) {
			itsProcname = primaryKeys().asString("PROCNAME");
		    } else {
			itsProcname = "";
		    }

		    if (primaryKeys().fieldNumber("PROCTYPE") >= 0) {
			itsProctype = primaryKeys().asString("PROCTYPE");
		    } else {
			itsProctype = "";
		    }

		    if (primaryKeys().fieldNumber("PROCSEQN") >= 0) {
			if (baseVersion() < 0 || deviceVersion() < 1 &&
			    primaryKeys().dataType("PROCSEQN") == TpString) {
			    // early versions mistakenly code this as a string
			    itsProcseqn = stringToInt(primaryKeys().asString("PROCSEQN"));
			} else {
			    itsProcseqn = primaryKeys().asInt("PROCSEQN");
			}
		    } else {
			itsProcseqn = 0;
		    }

		    if (primaryKeys().fieldNumber("PROCSIZE") >= 0) {
			if (baseVersion() < 0 || deviceVersion() < 1 &&
			    primaryKeys().dataType("PROCSIZE") == TpString) {
			    // early versions mistakenly code this as a string
			    itsProcsize = stringToInt(primaryKeys().asString("PROCSIZE"));
			} else {
			    itsProcsize = primaryKeys().asInt("PROCSIZE");
			}
		    } else {
			itsProcsize = 0;
		    }

		    if (primaryKeys().fieldNumber("OBSTYPE") >= 0) {
			itsObstype = primaryKeys().asString("OBSTYPE");
		    } else {
			itsObstype = "";
		    }

		    if (primaryKeys().fieldNumber("SWTCHSIG") >= 0) {
			itsSwtchsig = primaryKeys().asString("SWTCHSIG");
		    } else {
			itsSwtchsig = "";
		    }

		    if (primaryKeys().fieldNumber("SWSTATE") >= 0) {
			itsSwstate = primaryKeys().asString("SWSTATE");
		    } else {
			itsSwstate = "";
		    }

		    if (primaryKeys().fieldNumber("LASTON") >= 0) {
			itsLaston = primaryKeys().asInt("LASTON");
		    } else {
			itsLaston = 0;
		    }

		    if (primaryKeys().fieldNumber("LASTOFF") >= 0) {
			itsLastoff = primaryKeys().asInt("LASTOFF");
		    } else {
			itsLastoff = 0;
		    }

		    // the position stuff
		    itsEquinox = 2000.0;
		    itsMajor = itsMinor = 0.0;
		    itsCoordsys = "RADEC";
		    itsRadesys = "FK5";
		    itsOtherCoordsys = False;
		    Bool hasMajor = False;
		    Bool hasMinor = False;

		    if (baseVersion() > 0 && deviceVersion() > 0) {
			if (primaryKeys().fieldNumber("COORDSYS") >= 0) {
			    itsCoordsys = primaryKeys().asString("COORDSYS");
			}
			if (primaryKeys().fieldNumber("RADESYS") >= 0) {
			    itsRadesys = primaryKeys().asString("RADESYS");
			}
			if (primaryKeys().fieldNumber("EQUINOX") >= 0) {
			    itsEquinox = primaryKeys().asDouble("EQUINOX");
			}
			// look for appropriate major and minor values
			if (itsCoordsys == "GALACTIC") {
			    if (primaryKeys().fieldNumber("GLON") >= 0) {
				itsMajor = primaryKeys().asDouble("GLON");
				hasMajor = True;
			    }
			    if (primaryKeys().fieldNumber("GLAT") >= 0) {
				itsMinor = primaryKeys().asDouble("GLAT");
				hasMinor = True;
			    }
			} else if (itsCoordsys == "AZEL") {
			    if (primaryKeys().fieldNumber("AZ") >= 0) {
				itsMajor = primaryKeys().asDouble("AZ");
				hasMajor = True;
			    }
			    if (primaryKeys().fieldNumber("EL") >= 0) {
				itsMinor = primaryKeys().asDouble("EL");
				hasMinor = True;
			    }
			} else {
			    // RA-like coordinates
			    // early versions of Turtle wrote this out
			    // in hours instead of degrees - <= 2.5
			    Double factor = 1.0;
			    if (instrume() == "Turtle" && baseVersion() <= 2 &&
				deviceVersion() <= 5) factor = 15.0;

			    if (itsCoordsys == "HADEC") {
				if (primaryKeys().fieldNumber("HA") >= 0) {
				    itsMajor = factor*primaryKeys().asDouble("HA");
				    hasMajor = True;
				}
			    } else {
				if (primaryKeys().fieldNumber("RA") >= 0) {
				    itsMajor = factor*primaryKeys().asDouble("RA");
				    hasMajor = True;
				}
			    }
			    if (primaryKeys().fieldNumber("DEC") >= 0) {
				itsMinor = primaryKeys().asDouble("DEC");
				hasMinor = True;
			    }
			}
			if (!hasMajor || !hasMinor) {
			    // turtle bug, COORDSYS doesn't match long,lat keywords
			    if (!hasMajor) {
				// fall back to RA

				// early versions of Turtle wrote this out
				// in hours instead of degrees - <= 2.5
				Double factor = 1.0;
				if (instrume() == "Turtle" && baseVersion() <= 2 &&
				    deviceVersion() <= 5) factor = 15.0;

				if (primaryKeys().fieldNumber("RA") >= 0) {
				    itsMajor = factor*primaryKeys().asDouble("RA");
				    hasMajor = True;
				}
				if (primaryKeys().fieldNumber("DEC") >= 0) {
				    itsMinor = primaryKeys().asDouble("DEC");
				    hasMinor = True;
				}
			    }
			    LogIO os(LogOrigin("GBTGOFile","reopen"));
			    os << LogIO::WARN << WHERE
			       << "The longitude and/or latitude keywords are not as expected for the COORDSYS value in the GO FITS file for scan "
			       << scan()
			       << "\n";
			    if (hasMajor && hasMinor) {
				os << "RA and/or DEC keyword values used as necessary in their place. This is a known turtle bug.\n";
			    } else {
				// nothing to fall back on
				os << "No RA and/or DEC keyword values to fall back on, this is an unexpected problem.\n";
			    }
			    os << LogIO::POST;
			}
		    } else {
			// older version - Turtle never wrote these
			// if RAJ2000 exists, DECJ2000 exists, use them
			if (primaryKeys().fieldNumber("RAJ2000") >= 0 &&
			    primaryKeys().fieldNumber("DECJ2000") >= 0) {
			    if (primaryKeys().dataType("RAJ2000") == TpString) {
				// very early versions wrote to strings
				itsMajor = stringToDouble(primaryKeys().asString("RAJ2000"));
			    } else {
				itsMajor = primaryKeys().asDouble("RAJ2000");
			    }
			    if (primaryKeys().dataType("DECJ2000") == TpString) {
				itsMinor = stringToDouble(primaryKeys().asString("DECJ2000"));
			    } else {
				itsMinor = primaryKeys().asDouble("DECJ2000");
			    }
			    itsEquinox = 2000.0;
			    itsCoordsys = "RADEC";
			    itsRadesys = "FK5";
			} else {
			    if (primaryKeys().fieldNumber("EQUINOX") >= 0) {
				if (primaryKeys().dataType("EQUINOX") == TpString) {
				    itsEquinox = stringToDouble(primaryKeys().asString("EQUINOX"));
				} else {
				    itsEquinox = primaryKeys().asDouble("EQUINOX");
				}
			    }
			    if (primaryKeys().fieldNumber("RADECSYS") >= 0) {
				itsCoordsys = primaryKeys().asString("RADECSYS");
				// strip off leading spaces - present in early version
				itsCoordsys = itsCoordsys.after(leading);
				// to upper case
				itsCoordsys = upcase(itsCoordsys);
				if (itsCoordsys == "B1950") {
				    itsCoordsys = "RADEC";
				    itsRadesys = "FK4";
				    itsEquinox = 1950.0;
				} else if (itsCoordsys == "APPARENTRADEC"){
				    itsCoordsys = "RADEC";
				    itsRadesys = "GAPPT";
				} else {
				    if (itsCoordsys == "RADECOFDATE") {
					itsRadesys = "FK5";
					itsCoordsys = "RADEC";
				    }
				}
			    }
			    if (primaryKeys().fieldNumber("MAJOR") >= 0) {
				if (primaryKeys().dataType("MAJOR") == TpString) {
				    itsMajor = stringToDouble(primaryKeys().asString("MAJOR"));
				} else {
				    itsMajor = primaryKeys().asDouble("MAJOR");
				}
			    }
			    if (primaryKeys().fieldNumber("MINOR") >= 0) {
				if (primaryKeys().dataType("MINOR") == TpString) {
				    itsMinor = stringToDouble(primaryKeys().asString("MINOR"));
				} else {
				    itsMinor = primaryKeys().asDouble("MINOR");
				}
			    }
			}
		    }
		    MDirection::Types coordType;
		    coordType = MDirection::J2000;
		    Bool useFrame = False;
		    // frame information isn't generally necessary
		    MeasFrame frame;
		    if (itsCoordsys == "RADEC") {
			if (itsRadesys == "FK5") {
			    if (near(itsEquinox, Double(2000.0))) {
				coordType = MDirection::J2000;
			    } else {
				// add the equinox to the frame as epoch
				Double mjd = 
				    (itsEquinox-2000.0)/100.0*MeasData::JDCEN + 
				    MeasData::MJD2000;
				itsFrame.set(MEpoch(Quantity(mjd,Unit("d"))));
				useFrame = True;
				coordType = MDirection::JMEAN;
			    }
			} else if (itsRadesys == "FK4") {
			    coordType = MDirection::B1950;
			} else if (itsRadesys == "GAPPT") {
			    // use DATE-OBS as the frame epoch
			    itsFrame.set(dateObs());
			    useFrame = True;
			    coordType = MDirection::APP;
			} else {
			    // invalid RADESYS for RADEC COORDSYS - make this look like "OTHER"
			    itsOtherCoordsys = False;
			    LogIO os(LogOrigin("GBTGOFile","reopen"));
			    os << LogIO::WARN << WHERE
			       << "Unexpected RADESYS value of "
			       << itsRadesys << " in "
			       << filePath.expandedName() << "\n"
			       << "FK5 will be assumed.\n"
			       << "The FIELD position for this scan may not be valid." << LogIO::POST;
			}
		    } else if (itsCoordsys == "HADEC") {
			// use DATE-OBS as the frame epoch
			itsFrame.set(dateObs());
			useFrame = True;
			coordType = MDirection::HADEC;
		    } else if (itsCoordsys == "AZEL") {
			// use DATE-OBS as the frame epoch
			itsFrame.set(dateObs());
			useFrame = True;
			coordType = MDirection::AZEL;
		    } else if (itsCoordsys == "GALACTIC") {
			coordType = MDirection::GALACTIC;
		    } else if (itsCoordsys == "OTHER") {
			// refdir will be invalid, but this file is okay
			itsOtherCoordsys = True;
			LogIO os(LogOrigin("GBTGOFile","reopen"));
			os << LogIO::WARN << WHERE
			   << "OTHER COORDSYS indicated in GO file : "
			   << filePath.expandedName() << "\n"
			   << "The FIELD position for this scan will not be valid." << LogIO::POST;
		    } else {
			// unrecognized COORDSYS value, assume OTHER and issue a warning
			itsOtherCoordsys = True;
			LogIO os(LogOrigin("GBTGOFile","reopen"));
			os << LogIO::WARN << WHERE
			   << "Unrecognized COORDSYS value of "
			   << itsCoordsys  << " in "
			   << filePath.expandedName() << "\n"
			   << "The FIELD position for this scan will not be valid." << LogIO::POST;
		    } 
		    if (useFrame) {
			itsCmddir = MDirection(Quantity(itsMajor,"deg"),
					       Quantity(itsMinor, "deg"),
					       MDirection::Ref(coordType, itsFrame));
		    } else {
			itsCmddir = MDirection(Quantity(itsMajor,"deg"),
					       Quantity(itsMinor, "deg"),
					       coordType);
		    }
		}
		    					       
		// the velocity related fields
		itsVelocityKW = 0.0;
		Double restf = 0.0;
		itsDoppler = False;
		if (primaryKeys().fieldNumber("RESTFRQ") >= 0) {
		    restf = primaryKeys().asDouble("RESTFRQ");
		} 
		itsRestfrq = MVFrequency(Quantity(restf,"Hz"));
		Double skyf = 0.0;
		if (primaryKeys().fieldNumber("SKYFREQ") >= 0) {
		    skyf = primaryKeys().asDouble("SKYFREQ");
		} 
		itsSkyfrq = MVFrequency(Quantity(skyf,"Hz"));
		if (primaryKeys().fieldNumber("VELOCITY") >= 0) {
		    itsVelocityKW = primaryKeys().asDouble("VELOCITY");
		    itsDoppler = True;
		}
		MDoppler::Types dopType = MDoppler::RADIO;
		MRadialVelocity::Types radvType = MRadialVelocity::TOPO;
		if (primaryKeys().fieldNumber("VELDEF") >= 0) {
		    itsVeldef = primaryKeys().asString("VELDEF");
		    // must be 8-characters long
		    if (itsVeldef.length() == 8) {
			// split it - ignore character at 4 "-"
			String defn = itsVeldef.before(4);
			String frame = itsVeldef.after(4);
			// translate to equivalent aips++ code
			if (defn == "VELO") {
			    dopType = MDoppler::BETA;
			} else if (defn == "VRAD") {
			    dopType = MDoppler::RADIO;
			} else if (defn == "VOPT") {
			    dopType = MDoppler::OPTICAL;
			} 
			if (frame == "TOP") {
			    radvType = MRadialVelocity::TOPO;
			} else if (frame == "GEO") {
			    radvType = MRadialVelocity::GEO;
			} else if (frame == "HEL" || frame == "BAR") {
			    radvType = MRadialVelocity::BARY;
			} else if (frame == "LSR") {
			    radvType = MRadialVelocity::LSRK;
			} else if (frame == "LSD") {
			    radvType = MRadialVelocity::LSRD;
			} else if (frame == "GAL") {
			    radvType = MRadialVelocity::GALACTO;
			} else {
			    radvType = MRadialVelocity::TOPO;
			}
		    }
		    MDoppler vdop(Quantity(itsVelocityKW,"m/s"), dopType);
		    itsVelocity = MRadialVelocity::fromDoppler(vdop,
							       radvType);
		} else {
		    itsVelocity = MRadialVelocity(Quantity(0.0,"m/s"),
						  MRadialVelocity::TOPO);
		    itsVelocityKW = 0.0;
		    itsVeldef = "VRAD-TOP";
		}
	    }
	}
    }
    return itsValid;
}

void GBTGOFile::init()
{
    itsValid = False;
    itsAttached = False;

    itsName = "";

    setPrimaryKeys(Record());

    itsProcseqn = itsProcsize = 0;

    itsLaston = itsLastoff = 0;

    itsObserver = itsProcname = itsProctype = itsSwtchsig = itsObstype = itsSwstate = "";

    itsCmddir = MDirection();

    itsEquinox = 2000.0;
    itsMajor = itsMinor = 0.0;
    itsCoordsys = "RADEC";
    itsRadesys = "FK5";

    itsVelocity = MRadialVelocity(Quantity(0.0,"m/s"),
				  MRadialVelocity::TOPO);
    itsRestfrq = itsSkyfrq = MVFrequency(Quantity(0.0,"Hz"));

    itsVelocityKW = 0.0;
    itsVeldef = "VRAD-TOP";
    itsDoppler = True;

    itsOtherCoordsys = False;
}
