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
// Base class specific for NRO data header.
// </summary>

// 
// NROHeader
//
// Base class for NRO data header.
//
class NROHeader 
{
 public:
  // constructor 
  NROHeader() ;

  // destructor 
  virtual ~NROHeader() ;

  // data initialization
  virtual void initialize() ;

  // finalization
  virtual void finalize() ;

  // fill header from file
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

  // endian conversion
  void convertEndian( int &value ) ;
  void convertEndian( float &value ) ;
  void convertEndian( double &value ) ;

  // read data
  int readChar( char *v, FILE *f, int size ) ;
  int readInt( int &v, FILE *f, bool b ) ;
  int readFloat( float &v, FILE *f, bool b ) ;
  int readDouble( double &v, FILE *f, bool b ) ;

 protected:
  char *LOFIL ;     // type of file record
  char *VER ;       // version 
  char *GROUP ;     // group name
  char *PROJ ;      // project name
  char *SCHED ;     // name of observation instruction file
  char *OBSVR ;     // name of observer
  char *LOSTM ;     // observation start time (YYYYMMDDHHMMSS)
  char *LOETM ;     // observation end time (YYYYMMDDHHMMSS)
  int ARYNM ;       // number of arrays (beams)
  int NSCAN ;       // number of scans
  char *TITLE ;     // title of observation
  char *OBJ ;       // name of object 
  char *EPOCH ;     // equinox B1950 or J2000
  double RA0 ;      // right ascension [rad]
  double DEC0 ;     // celestial declination [rad]
  double GLNG0 ;    // Galactic longitude [rad]
  double GLAT0 ;    // Galactic latitude [rad]
  int NCALB ;       // calibration interval
  int SCNCD ;       // scan coordinate  0: RADEC  1: LB  2: AZEL
  char *SCMOD ;     // scan sequence pattern
  double URVEL ;    // user-defined recessional velocity
  char *VREF ;      // reference frame for recessional velocity  LSR/HEL/GAL
  char *VDEF ;      // definition of recessional velocity  RAD/OPT
  char *SWMOD ;     // switching mode  POS/BEAM/FREQ
  double FRQSW ;    // switching frequency [Hz]
  double DBEAM ;    // off-beam angle of beam switching [rad]
  double MLTOF ;    // initial inclination angle of multi-beam array 
  double CMTQ ;     // comet: perihelion distance
  double CMTE ;     // comet: eccentricity
  double CMTSOM ;   // comet: argument of perihelion
  double CMTNODE ;  // comet: longitude of the ascending node 
  double CMTI ;     // comet: orbital inclination angle
  char *CMTTM ;     // comet: time of the perihelion passage
  double SBDX ;     // correction for position of subreflector DX [mm] 
  double SBDY ;     // correction for position of subreflector DY [mm] 
  double SBDZ1 ;    // correction for position of subreflector DZ1 [mm] 
  double SBDZ2 ;    // correction for position of subreflector DZ2 [mm] 
  double DAZP ;     // correction for pointing on azimuth [rad]
  double DELP ;     // correction for pointing on elevation [rad]
  int CHBIND ;      // channel bind 
  int NUMCH ;       // number of channel after channel bind
  int CHMIN ;       // channel range (minimum)
  int CHMAX ;       // channel range (maximum)
  double ALCTM ;    // ALC time constant
  double IPTIM ;    // interval to get data from spectrometer
  double PA ;       // position angle of the map
  int SCNLEN ;      // length of scan record
  int SBIND ;       // space bind
  int IBIT ;        // quantization bit number (fixed to 12)
  char *SITE ;      // site (antenna) name  45m/ASTE
  vector<char *> RX ;        // type of detector frontend 
  vector<double> HPBW ;    // HPBW [rad]
  vector<double> EFFA ;    // aperture efficiencies 
  vector<double> EFFB ;    // beam efficiencies 
  vector<double> EFFL ;    // antenna efficiencies 
  vector<double> EFSS ;    // FSS efficiencies 
  vector<double> GAIN ;    // antenna gain 
  vector<char *> HORN ;      // type of polarization at feed horn  R/L/H/V
  vector<char *> POLTP ;     // type of polarization  CIRC/LINR
  vector<double> POLDR ;   // rotation direction of circular polarization
  vector<double> POLAN ;   // angle of linear polarization
  vector<double> DFRQ ;    // frequency of frequcency switching [Hz]
  vector<char *> SIDBD ;     // type of sideband  LSB/USB/DSB
  vector<int> REFN ;       // identifier of reference synthesizer
  vector<int> IPINT ;      // temperature of calibrator
  vector<int> MULTN ;      // channel id of the multi-beam detector
  vector<double> MLTSCF ;  // scaling factor of the multi-beam detector
  vector<char *> LAGWIND ;   // type of LAG window  NONE/HANN/HAMM/BLCK
  vector<double> BEBW ;    // bandwidth at backend
  vector<double> BERES ;   // spectral resolution at backend
  vector<double> CHWID ;   // channel width at backend
  vector<int> ARRY ;       // array usage  1: used  0: not used
  vector<int> NFCAL ;      // freq. cal.: number of measurement (max 10)
  vector<double> F0CAL ;   // freq. cal.: central frequency [Hz]
  vector< vector<double> > FQCAL ;   // freq. cal.: measured central freq. [Hz]
  vector< vector<double> > CHCAL ;   // freq. cal.: measured channel number
  vector< vector<double> > CWCAL ;   // freq. cal.: measured channel width
  char *CDMY1 ;     // dummy
  vector<double> DSBFC ;   // DSB scaling factor

  int datasize_ ;
} ;


#endif /* NRO_HEADER_H */
