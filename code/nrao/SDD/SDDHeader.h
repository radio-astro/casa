//# SDDHeader.h: a class containing the header information from an SDDFile
//# Copyright (C) 1999,2001
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

#ifndef NRAO_SDDHEADER_H
#define NRAO_SDDHEADER_H

//#! Includes go here
#include <casa/aips.h>
#include <casa/Arrays/Vector.h>

#include <casa/namespace.h>
//# Forward declarations
namespace casa { //# NAMESPACE CASA - BEGIN
class String;
} //# NAMESPACE CASA - END

class SDDBlock;
#include <casa/iosfwd.h>

#include <casa/namespace.h>
// <summary>
// a class containing the header information from an SDDFile
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> SDDFile
//   <li> SDDIndex
//   <li> SDDHeader
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

class SDDHeader {
public:
    // String Header Values
    enum StringHeader {
	OBSID=0,                      // obsever initials, 8 characters
	OBSERVER,                     // observer name, 16 characters
	TELESCOP,                     // Telescope descriptor, 8 characters
	PROJID,                       // project ID, 8 characters
	OBJECT,                       // source name, 16 characters
	OBSMODE,                      // type of data + observing mode, 8 characters
	FRONTEND,                     // front end descriptor, 8 characters
	BACKEND,                      // back end descriptor, 8 characters
	PRECIS,                       // Data Precision, 8 characters, ignored!
	PT_MODEL,                     // Pointing Model descriptor, 8 characters
	CL11TYPE,                     // Class 11 type descriptor, 8 characters
	COORDCD,                      // Coordinate System Code, 8 characters
	FRAME,                        // XY Reference Frame Code, 8 characters
	VELDEF,                       // Velocity Definition and Reference, 8 characters
	TYPECAL,                      // Type of Calibration, 8 characters
	CFFORM,                       // Center Frequency Formula
	OPENPAR,                      // Class 10, open parameter, 80 characters
	POLARIZ,                      // Polarization descriptor, 8 characters
	RX_INFO,                      // Receiver information, 16 characters
	LAST_STRING
    };
    
    
    // R*8 (Double) Header values 
    enum BasicInformation {
	HEADLEN=0,                    // length of header in bytes
	DATALEN,                      // length of data in bytes
	SCAN,                         // Scan number
	SAVENUM=14,                   // index location for this scan
	NORECORD,                     // # of individual records for RECORDS scan
				      // OR # of OTF spectra in this row
	RECORDID,                     // The record number for this scan
				      // OR the # of this OTF spectra
	LAST_BASIC
    };

    enum PointingParameters {
	XPOINT=0,                     // Total Az/RA Pointing Correction (arcsec)
	YPOINT,                       // Total El/Dec Pointing Correction (arcsec)
	UXPNT,                        // User Az/RA Pointing Correction (arcsec)
	UYPNT,                        // User El/Dec Pointing Correction (arcsec)
	PTCON0,                       // Pointing Constants(0) (arcmin)
	PTCON1,                       // Pointing Constants(1) (arcmin)
	PTCON2,                       // Pointing Constants(2) (arcmin)
	PTCON3,                       // Pointing Constants(3) (arcmin)
	ORIENT,                       // Receiver box or secondary orientation (Degrees)
	FOCUSR,                       // Radial Focus (mm)
	FOCUSV,                       // North-South Focus (mm)
	FOCUSH,                       // East-West Focus (mm)
	LAST_POINTING=13
    };

    enum ObservingParameters {
	UTDATE=0,                     // Universal Time Date (YYYY.MMDD)
	UT,                           // Universay Time (hours)
	LST,                          // LST (hours)
	NORCHAN,                      // Number of receiver channels
	NOSWVAR,                      // Number of switching variables
	NOPHASE,                      // Number of phases per cycle
	CYCLEN,                       // Length of Cycle (seconds)
	SAMPRAT,                      // Length of Sample (seconds)
	PHASEID=9,                    // Phase number of this RECORDS data
	LAST_OBSERVING
    };
	
    enum Positions {
	EPOCH=0,                      // Epoch (years)
	XSOURCE,                      // Commanded Source X (Degrees)
	YSOURCE,                      // Commanded Source Y (Degrees)
	XREF,                         // Commanded Reference X (Degrees)
	YREF,                         // Commanded Reference Y (Degrees)
	EPOCRA,                       // Commanded RA at Epoch (Degrees)
	EPOCDEC,                      // Commanded Dec at Epoch (Degrees)
	GALLONG,                      // Commanded Galactic Longitude (Degrees)
	GALLAT,                       // Commanded Galactic Latitude (Degrees)
	AZ,                           // Commanded Azumuth (Degrees)
	EL,                           // Commanded Elevation (Degrees)
	INDX,                         // Indicated X Position (Degrees)
	INDY,                         // Indicated Y Position (Degrees)
	DESORG0,                      // Descriptive Origin (0) (Degrees)
	DESORG1,                      // Descriptive Origin (1) (Degrees)
	DESORG2,                      // Descriptive Origin (2) (Degrees)
	LAST_POSITIONS=17
    };

    enum Environment {
	TAMB=0,                       // Ambient temperature (C)
	PRESSURE,                     // Ambient Pressure (cm-Hg)
	HUMIDTY,                      // Ambient relative humidty (%)
	REFRAC,                       // Index of Refraction
	DEWPT,                        // Dew Point (C)
	MMH2O,                        // Mm of water
	LAST_ENVIRONMENT
    };

    enum MapParameters {
	SCANANG=0,                    // Map Scanning Angle (Degrees)
	XZERO,                        // X Position at Map Reference Position Zero
	YZERO,                        // Y Position at Map Reference Position Zero
	DELTAXR,                      // Delta X or X Rate (arcsec or arcsec/sec)
	DELTAYR,                      // Delta Y or Y Rate (arcsec or arcsec/sec)
	NOPTS,                        // Number of Grid Points
	NOXPTS,                       // Number of X Grid Points
	NOYPTS,                       // Number of Y Grid Points
	XCELL0,                       // Starting X Grid Cell Number
	YCELL0,                       // Starting Y Grid Cell Number
	LAST_MAP=11
    };
	
    enum DataParameters {
	BHWHM=0,                      // Beam Halfwidth at Half Maximum (arcsec)
	OFFSCAN,                      // Off Scan Number
	BADCHV,                       // Bad Channel Value (K)
	RVSYS,                        // Velocity Correction (km/s)
	VELOCITY,                     // Velocity with respect to Reference (km/s)
	LAST_DATA=7
    };

    enum EngineeringParameters {
	APPEFF=0,                     // Antenna Aperture Efficiency
	BEAMEFF,                      // Antenna Beam Efficiency
	ANTGAIN,                      // Antenna Gain
	ETAL,                         // Rear spill and scattering efficiency
	ETAFSS,                       // Forward spill and scattering efficiency
	LAST_ENGINEERING
    };

    enum GreenBank {
	L1=0,                         // MHz
	L1F1,                         // MHz
	L1F2,                         // MHz
	L2,                           // MHz
	L2F1,                         // MHz
	L2F2,                         // MHz
	LA,                           // MHz
	LB,                           // MHz
	LC,                           // MHz
	LD,                           // MHz
	LEVCORR,                      // Level Correction (Volts)
	PTFUDGE0,                     // Pointing Fudge (0) (arcmin)
	PTFUDGE1,                     // Pointing Fudge (1) (arcmin)
	RHO,                          // Degrees
	THETA,                        // Degrees
	LAST_GREENBANK=18
    };

    enum Tucson {
	SYNFREQ=0,                    // Synthesizer Frequency (MHz)
	LOFACT,                       // LO Factor
	HARMONIC,                     //
	LOIF,                         // MHz
	FIRSTIF,                      // MHz
        RAZOFF,                       // Reference Azimuth Offset (arcsec)
        RELOFF,                       // Reference Elevation Offset (arcsec)
	BMTHROW,                      // Beam Throw (arcsec)
	BMORENT,                      // Beam Orientation (Degrees)
	BASEOFF,                      // Baseline offset (K)
	OBSTOL,                       // Observing tolerance (arcsec)
	SIDEBAND,                     //
	WL,                           // Wavelength (mm)
	GAINS,                        // GAIN scan number
	PBEAM0,                       // + Beam(0) (arcsec)
	PBEAM1,                       // + Beam(1) (arcsec)
	MBEAM0,                       // - Beam(0) (arcsec)
	MBEAM1,                       // - Beam(1) (arcsec)
	SROFF0,                       // RA/Dec Offsets (0) (arcsec)
 	SROFF1,                       // RA/Dec Offsets (1) (arcsec)
	SROFF2,                       // RA/Dec Offsets (2) (arcsec)
	SROFF3,                       // RA/Dec Offsets (3) (arcsec)
	FOFFSIG,                      // Frequency Switching Signal Offset (MHz)
	FOFFREF1,                     // Frequency SwitchinG Reference Offset 1 (MHz)
	FOFFREF2,                     // Frequency SwitchinG Reference Offset 2 (MHz)
	LAST_TUCSON
    };

    enum OriginalPhaseBlock {
	ORIGINAL_VARVAL=0,            // Variable Value
	LAST_ORIGINAL_PHASE=3
    };
    enum OriginalPhaseBlockStrings {
	ORIGINAL_VARDES=1,            // Variable Descriptor, 8 Characters
	ORIGINAL_PHASTB               // Phase Table - 8 characters
    };

    enum NewPhaseBlockBasics {
	NOSWVARF=0,                   // Number of Fast Switching Variables
	NUMCYC,                       // Number of Slow Cycles per Scan
	NUMCYCF,                      // Number of Fast Cycles per Scan
	NOPHASEF,                     // Number of Fast Phases per Cycle
	CYCLENF,                      // Length of Fast Cycle (seconds)
	SAMPTIMF,                     // Length of Fast Phase Sample (seconds)
	LAST_NEW_PHASE_BASICS
    };
    enum NewPhaseBlock {
	VARVAL=0,                     // Variable Value
	LAST_NEW_PHASE=6
    };
    enum NewPhaseBlockStrings {
	VARDES=1,                     // Variable Descript, 8 characters
	PHASTB                        // Phase Table, 32 characters
    };

    enum ReceiverBlock {
	OBSFREQ=0,                    // Observed Frequency
	RESTFREQ,                     // Rest Frequency
	FREQRES,                      // Frequency Resolution (LINE) (MHz) or
				      // Scale Factor (CONT) (K/Count)
	BW,                           // Bandwidth (MHz)
	TRX,                          // Receiver Temperature (K)
	TCAL,                         // Calibration Temperature (K)
	STSYS,                        // Source System Temperature (K)
	RTSYS,                        // Reference System Temperature (K)
	TSOURCE,                      // Source Temperature (K)
	TRMS,                         // RMS of Mean (K)
	REFPT,                        // Reference Point Number
	X0,                           // X Value at REFPT (km/sec or degrees)
	DELTAX,                       // Delta X (km/sec or degrees)
	INTTIME,                      // Total integration time (seconds)
	NOINT,                        // Number of integrations
	SPN,                          // Starting Point Number
	TAUH2O,                       // H2O Opacity
	TH2O,                         // H2O Temperature (K)
	TAUO2,                        // O2 Opacity
	TO2,                          // O2 Temperature (K)
	EFFINT=21,                    // Effective Integration Time (sec)
	LAST_RECEIVER=24
    };

    enum ReductionParameters {
	NOSTAC=0,                       // Number of scans stacked
	FSCAN,                        // First scan in stack
	LSCAN,                        // Last scan in stack
	LAMP,                         // Line amplitude (K)
	LWID,                         // Line width (km/sec)
	ILI,                          // Integrated Line Intensity (K km/sec)
	RMS,                          // RMS Noise (K)
	LAST_REDUCTION
    };

    // construct an empty header block, minimal header, no data array
    SDDHeader();
    // construct one from istream, uses start of istream to determine how 
    // many records to use
    SDDHeader(istream& in);
    // copy constructor, true copy
    SDDHeader(const SDDHeader& other);

    // clean up the pointers
    ~SDDHeader();

    // assignment operator, true copy
    SDDHeader& operator=(const SDDHeader& other);

    // fill an existing SDDHeader from istream,  uses start of istream to determine how
    // many records to use, returns number of bytes actually read
    // the header is resized if necessary
    uInt fill(istream& in);

    // write to ostream, returns the number of bytes actually written
    uInt write(ostream& out) const;

    // empty out all the header words
    void empty();

    // Is it an NRAO telescope (12m or 140')
    Bool isNRAO() const;
    // Is it the 12m
    Bool is12m() const;
    // Is it the 140'
    Bool is140ft() const;

    // see if this is an OTF scan
    Bool isOTF() const;

    // The number of data vectors present (only !=1 for OTF amd POLZ data at present)
    uInt nvectors() const;

    // functions to retrieve and set parameters
    // for now, its just simpler to adopt the unipops rule and
    // do no sanity checks on any values (except the string header words)

    // for string header words, they must be less than the approproate length
    // or that string will not be set.
    String get(StringHeader field) const;
    Bool put(StringHeader field, const String& value);

    Double get(BasicInformation field) const;
    Bool put(BasicInformation field, const Double& value);

    Double get(PointingParameters field) const;
    Bool put(PointingParameters field, const Double& value);

    Double get(ObservingParameters field) const;
    Bool put(ObservingParameters field, const Double& value);

    Double get(Positions field) const;
    Bool put(Positions field, const Double& value);
    
    Double get(Environment field) const;
    Bool put(Environment field, const Double& value);
    
    Double get(MapParameters field) const;
    Bool put(MapParameters field, const Double& value);

    Double get(DataParameters field) const;
    Bool put(DataParameters field, const Double& value);

    Double get(EngineeringParameters field) const;
    Bool put(EngineeringParameters field, const Double& value);
    
    Double get(GreenBank field) const;
    Bool put(GreenBank field, const Double& value);

    Double get(Tucson field) const;
    Bool put(Tucson field, const Double& value);

    Double get(ReceiverBlock field) const;
    Bool put(ReceiverBlock field, const Double& value);

    Double get(ReductionParameters field) const;
    Bool put(ReductionParameters field, const Double& value);

    // Old class 11, need to specify which block as well as the field
    Double get(OriginalPhaseBlock, uInt whichBlock=0) const;
    Bool put(OriginalPhaseBlock field, const Double& value, 
	     uInt whichBlock=0);
    // String get and put
    String get(OriginalPhaseBlockStrings field, uInt whichBlock=0) const;
    Bool put(OriginalPhaseBlockStrings field, const String& value, 
	     uInt whichBlock=0);

    // New class 11, the basics are only at the start of the class
    Double get(NewPhaseBlockBasics field) const;
    Bool put(NewPhaseBlockBasics field, const Double& value);
    // And for the rest, the block needs to be specified
    Double get(NewPhaseBlock field, uInt whichBlock=0) const;
    Bool put(NewPhaseBlock field, const Double& value, uInt whichBlock);
    // Strings need get and put
    String get(NewPhaseBlockStrings field, uInt whichBlock=0) const;
    Bool put(NewPhaseBlockStrings field, const String& value, 
	     uInt whichBlock=0);

private:
    // this holds the preamble information
    Vector<Int> *preamble_p;
    // this actually holds the header
    SDDBlock *hdu_p;

    // helpful to stash these here
    Bool isOtf_p;
    uInt nvectors_p;

    // maps
    //  the GNU compiler doesn't seem to like it that this is static
    // I don't think its crucial so punt for now
    //   static Vector<uInt> nchars_p;
    Vector<uInt> nchars_p;

    Vector<uInt> strOffset_p;

    Double getFromClass(uInt whichClass, uInt offset) const;
    Bool putToClass(uInt whichClass, uInt offset, const Double& value);

    void init_nchars();
    void init_strOffset();
    uInt setOffset(uInt whichClass, uInt number) const;

    void putPreamble();
    void getPreamble();
};

#endif
