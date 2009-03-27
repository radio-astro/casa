//#---------------------------------------------------------------------------
//# NROHeader.h: Base class for NRO header data.
//#---------------------------------------------------------------------------
//# Copyright (C) 2000-2006
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
//#---------------------------------------------------------------------------
//# Original: 2008/10/30, Takeshi Nakazato, NAOJ
//#---------------------------------------------------------------------------

#ifndef NRO_HEADER_H
#define NRO_HEADER_H

#include <string> 
#include <stdio.h>
#include <vector> 

using namespace std ;

// <summary>
// <linkto class=NROHeader>NROHeader</linkto> is a base class 
// for an observation data header of NRO 45m and ASTE raw data. 
// </summary>

// <use visibility=global>

// <reviewed reviewer="" date="" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> 
// </prerequisite>
//
// <etymology>
// NRO and ASTE raw data format consists of two major components; data 
// header and scan record. <linkto class=NROHeader>NROHeader</linkto> is 
// a representation of the data header.
// </etymology>
//
// <synopsis>
// <linkto class=NROHeader>NROHeader</linkto> is a representation of 
// the data header that is a meta-data describing observation settings 
// and conditions during the observing session.
// </synopsis>
//
// <example>
// 
// </example>
//
// <motivation>
// <linkto class=NROHeader>NROHeader</linkto> are defined to read 
// meta-data described in the data header. Although basic data structure 
// is same for NRO 45m and ASTE, there are slight difference in the 
// data size. Thus <linkto class=NROHeader>NROHeader</linkto> is 
// needed as a base class for the data header.
// </motivation>
//
// <to do asof="">
//   <li>
// </to do>  
class NROHeader 
{
 public:
  // Constructor 
  NROHeader() ;

  // Destructor 
  virtual ~NROHeader() ;

  // Data initialization
  virtual void initialize() ;

  // Finalization
  virtual void finalize() ;

  // Fill data header from file
  virtual int fill( FILE *fp, bool sameEndian ) = 0 ;
  virtual int fill( string name ) = 0 ;

  // getter
  char *getLOFIL() { return LOFIL ; } ;
  char *getVER() { return VER ; } ;
  char *getGROUP() { return GROUP ; } ;
  char *getPROJ() { return PROJ ; } ;
  char *getSCHED() { return SCHED ; } ;
  char *getOBSVR() { return OBSVR ; } ;
  char *getLOSTM() { return LOSTM ; } ;
  char *getLOETM() { return LOETM ; } ;
  int getARYNM() { return ARYNM ; } ;
  int getNSCAN() { return NSCAN ; } ;
  char *getTITLE() { return TITLE ; } ;
  char *getOBJ() { return OBJ ; } ;
  char *getEPOCH() { return EPOCH ; } ;
  double getRA0() { return RA0 ; } ;
  double getDEC0() { return DEC0 ; } ;
  double getGLNG0() { return GLNG0 ; } ;
  double getGLAT0() { return GLAT0 ; } ;
  int getNCALB() { return NCALB ; } ;
  int getSCNCD() { return SCNCD ; } ;
  char *getSCMOD() { return SCMOD ; } ;
  double getURVEL() { return URVEL ; } ;
  char *getVREF() { return VREF ; } ;
  char *getVDEF() { return VDEF ; } ;
  char *getSWMOD() { return SWMOD ; } ;
  double getFRQSW() { return FRQSW ; } ;   
  double getDBEAM() { return DBEAM ; } ;   
  double getMLTOF() { return MLTOF ; } ;    
  double getCMTQ() { return CMTQ ; } ;     
  double getCMTE() { return CMTE ; } ;
  double getCMTSOM() { return CMTSOM ; } ;
  double getCMTNODE() { return CMTNODE ; } ;
  double getCMTI() { return CMTI ; } ;
  char *getCMTTM() { return CMTTM ; } ;
  double getSBDX() { return SBDX ; } ;
  double getSBDY() { return SBDY ; } ;
  double getSBDZ1() { return SBDZ1 ; } ;
  double getSBDZ2() { return SBDZ2 ; } ;
  double getDAZP() { return DAZP ; } ;
  double getDELP() { return DELP ; } ;
  int getCHBIND() { return CHBIND ; } ;
  int getNUMCH() { return NUMCH ; } ;
  int getCHMIN() { return CHMIN ; } ;
  int getCHMAX() { return CHMAX ; } ;
  double getALCTM() { return ALCTM ; } ;
  double getIPTIM() { return IPTIM ; } ;
  double getPA() { return PA ; } ;
  int getSCNLEN() { return SCNLEN ; } ;
  int getSBIND() { return SBIND ; } ;
  int getIBIT() { return IBIT ; } ;
  char *getSITE() { return SITE ; } ;
  vector<char *> getRX() { return RX ; } ;
  vector<double> getHPBW() { return HPBW ; } ;
  vector<double> getEFFA() { return EFFA ; } ;
  vector<double> getEFFB() { return EFFB ; } ;
  vector<double> getEFFL() { return EFFL ; } ;
  vector<double> getEFSS() { return EFSS ; } ;
  vector<double> getGAIN() { return GAIN ; } ;
  vector<char *> getHORN() { return HORN ; } ;
  vector<char *> getPOLTP() { return POLTP ; } ;
  vector<double> getPOLDR() { return POLDR ; } ;
  vector<double> getPOLAN() { return POLAN ; } ;
  vector<double> getDFRQ() { return DFRQ ; } ;
  vector<char *> getSIDBD() { return SIDBD ; } ;
  vector<int> getREFN() { return REFN ; } ;
  vector<int> getIPINT() { return IPINT ; } ;
  vector<int> getMULTN() { return MULTN ; } ;
  vector<double> getMLTSCF() { return MLTSCF ; } ;
  vector<char *> getLAGWIND() { return LAGWIND ; } ;
  vector<double> getBEBW() { return BEBW ; } ;
  vector<double> getBERES() { return BERES ; } ;
  vector<double> getCHWID() { return CHWID ; } ;
  vector<int> getARRY() { return ARRY ; } ;
  vector<int> getNFCAL() { return NFCAL ; } ;
  vector<double> getF0CAL() { return F0CAL ; } ;
  vector< vector<double> > getFQCAL() { return FQCAL ; } ;
  vector< vector<double> > getCHCAL() { return CHCAL ; } ;
  vector< vector<double> > getCWCAL() { return CWCAL ; } ;
  char *getCDMY1() { return CDMY1 ; } ;
  vector<double> getDSBFC() { return DSBFC ;} ;
  int getDataSize() { return datasize_ ; } ;

 protected:
  // Endian conversion for int variable
  void convertEndian( int &value ) ;

  // Endian convertion for float variable
  void convertEndian( float &value ) ;

  // Endian conversion for double variable
  void convertEndian( double &value ) ;

  // Read char data
  int readChar( char *v, FILE *f, int size ) ;

  // Read int data
  int readInt( int &v, FILE *f, bool b ) ;

  // Read float data
  int readFloat( float &v, FILE *f, bool b ) ;

  // Read double data
  int readDouble( double &v, FILE *f, bool b ) ;

  // Type of file record
  char *LOFIL ;

  // Version 
  char *VER ;

  // Group name
  char *GROUP ;

  // Project name
  char *PROJ ;

  // Name of observation scheduling file
  char *SCHED ;

  // Name of observer
  char *OBSVR ;

  // Observation start time with format of "YYYYMMDDHHMMSS" (UTC)
  char *LOSTM ;

  // observation end time with format of "YYYYMMDDHHMMSS" (UTC)
  char *LOETM ;

  // Number of arrays (beams and IFs)
  int ARYNM ;

  // Number of scans
  int NSCAN ;

  // Title of observation
  char *TITLE ;

  // Name of target object 
  char *OBJ ;

  // Equinox (B1950 or J2000)
  char *EPOCH ;

  // Right ascension [rad]
  double RA0 ;

  // Declination [rad]
  double DEC0 ;

  // Galactic longitude [rad]
  double GLNG0 ;

  // Galactic latitude [rad]
  double GLAT0 ;

  // Calibration interval
  int NCALB ;

  // Scan coordinate  (0: RADEC  1: LB  2: AZEL)
  int SCNCD ;

  // Scan sequence pattern
  char *SCMOD ;

  // User-defined recessional velocity [m/s]
  double URVEL ;

  // Reference frame for recessional velocity  (LSR or HEL or GAL)
  char *VREF ;

  // Definition of recessional velocity  (RAD or OPT)
  char *VDEF ;

  // Switching mode  (POS or BEAM or FREQ)
  char *SWMOD ;

  // Switching frequency [Hz]
  double FRQSW ;

  // Off-beam angle of beam switching [rad]
  double DBEAM ;

  // Initial inclination angle of multi-beam array 
  double MLTOF ;

  // Comet: Perihelion distance
  double CMTQ ;

  // Comet: Eccentricity
  double CMTE ;

  // Comet: Argument of perihelion
  double CMTSOM ;

  // Comet: Longitude of the ascending node 
  double CMTNODE ;

  // Comet: Orbital inclination angle
  double CMTI ;

  // Comet: Time of the perihelion passage
  char *CMTTM ;

  // Correction for position of subreflector DX [mm] 
  double SBDX ;

  // Correction for position of subreflector DY [mm] 
  double SBDY ;

  // Correction for position of subreflector DZ1 [mm] 
  double SBDZ1 ;

  // Correction for position of subreflector DZ2 [mm] 
  double SBDZ2 ;

  // Correction for pointing on azimuth [rad]
  double DAZP ;

  // Correction for pointing on elevation [rad]
  double DELP ;

  // Number of channel binding  
  int CHBIND ;

  // Number of channel after binding
  int NUMCH ;

  // Channel range (minimum)
  int CHMIN ;

  // Channel range (maximum)
  int CHMAX ;

  // ALC time constant
  double ALCTM ;

  // Interval to get data from spectrometer
  double IPTIM ;

  // Position angle of the map
  double PA ;

  // Length of scan record [bytes]
  int SCNLEN ;

  // Range of space binding
  int SBIND ;

  // Quantization bit number (fixed to 12)
  int IBIT ;

  // Site (antenna) name  (45m or ASTE)
  char *SITE ;

  // Dummy data
  char *CDMY1 ;

  // Type of detector frontend 
  vector<char *> RX ;

  // HPBW [rad]
  vector<double> HPBW ;

  // Aperture efficiencies 
  vector<double> EFFA ;

  // Beam efficiencies 
  vector<double> EFFB ;

  // Antenna efficiencies 
  vector<double> EFFL ;

  // FSS efficiencies 
  vector<double> EFSS ;

  // Antenna gain 
  vector<double> GAIN ;

  // Type of polarization at feed horn  (R or L or H or V)
  vector<char *> HORN ;

  // Type of polarization  (CIRC or LINR)
  vector<char *> POLTP ;

  // Rotation direction of circular polarization
  vector<double> POLDR ;

  // Polarization angle of linear polarization
  vector<double> POLAN ;

  // Switching frequency of frequcency switching [Hz]
  vector<double> DFRQ ;

  // Type of sideband  (LSB or USB or DSB)
  vector<char *> SIDBD ;

  // Identifier of reference synthesizer
  vector<int> REFN ;

  // Temperature of calibrator
  vector<int> IPINT ;

  // Beam id of the multi-beam detector
  vector<int> MULTN ;

  // Scaling factor of the multi-beam detector
  vector<double> MLTSCF ;

  // Type of LAG window  (NONE or HANN or HAMM or BLCK)
  vector<char *> LAGWIND ;

  // Bandwidth at backend
  vector<double> BEBW ;

  // Spectral resolution at backend
  vector<double> BERES ;

  // Channel width at backend
  vector<double> CHWID ;

  // Array usage  (1: used  0: not used)
  vector<int> ARRY ;

  // Frequency calibration: Number of measurement (max 10)
  vector<int> NFCAL ;

  // Frequency calibration: Central frequency [Hz]
  vector<double> F0CAL ;

  // Frequency calibration: Measured central frequency [Hz]
  vector< vector<double> > FQCAL ;

  // Frequency calibration: Measured channel number
  vector< vector<double> > CHCAL ;

  // Frequency calibration: Measured channel width [Hz]
  vector< vector<double> > CWCAL ;

  // DSB scaling factor
  vector<double> DSBFC ;

  // Data size of the header [bytes]
  int datasize_ ;
} ;


#endif /* NRO_HEADER_H */
