//#---------------------------------------------------------------------------
//# NRODataset.h: Base class for NRO dataset.
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
//# Original: 2009/02/27, Takeshi Nakazato, NAOJ
//#---------------------------------------------------------------------------

#ifndef NRO_DATASET_H
#define NRO_DATASET_H

#include <string> 
#include <stdio.h>
#include <vector> 
#include <iostream>

//#include <casa/aips.h>
#include <casa/Logging/LogIO.h>
#include <atnf/PKSIO/NRODataRecord.h>
#include <casa/namespace.h>

#define SCAN_HEADER_SIZE 424 


using namespace std ;

//enum OS_ENDIAN { BIG_ENDIAN,
//                 LITTLE_ENDIAN,
//                 UNKNOWN_ENDIAN } ;

// <summary>
// Base class for NRO dataset.
// </summary>
class NRODataset
{
 public:
  // Constructor 
  NRODataset( string name ) ;

  // Destructor 
  virtual ~NRODataset() ;

  // Data initialization
  virtual void initialize() ;

  // open file
  virtual int open() ;

  // close file
  virtual void close() ;

  // Fill data header from file
  virtual int fillHeader() = 0 ;

  // Fill data record
  virtual int fillRecord( int i ) ;

  // simple getter
  string getLOFIL() { return LOFIL ; } ;
  string getVER() { return VER ; } ;
  string getGROUP() { return GROUP ; } ;
  string getPROJ() { return PROJ ; } ;
  string getSCHED() { return SCHED ; } ;
  string getOBSVR() { return OBSVR ; } ;
  string getLOSTM() { return LOSTM ; } ;
  string getLOETM() { return LOETM ; } ;
  int getARYNM() { return ARYNM ; } ;
  int getNSCAN() { return NSCAN ; } ;
  string getTITLE() { return TITLE ; } ;
  string getOBJ() { return OBJ ; } ;
  string getEPOCH() { return EPOCH ; } ;
  double getRA0() { return RA0 ; } ;
  double getDEC0() { return DEC0 ; } ;
  double getGLNG0() { return GLNG0 ; } ;
  double getGLAT0() { return GLAT0 ; } ;
  int getNCALB() { return NCALB ; } ;
  int getSCNCD() { return SCNCD ; } ;
  string getSCMOD() { return SCMOD ; } ;
  double getURVEL() { return URVEL ; } ;
  string getVREF() { return VREF ; } ;
  string getVDEF() { return VDEF ; } ;
  string getSWMOD() { return SWMOD ; } ;
  double getFRQSW() { return FRQSW ; } ;   
  double getDBEAM() { return DBEAM ; } ;   
  double getMLTOF() { return MLTOF ; } ;    
  double getCMTQ() { return CMTQ ; } ;     
  double getCMTE() { return CMTE ; } ;
  double getCMTSOM() { return CMTSOM ; } ;
  double getCMTNODE() { return CMTNODE ; } ;
  double getCMTI() { return CMTI ; } ;
  string getCMTTM() { return CMTTM ; } ;
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
  string getSITE() { return SITE ; } ;
  vector<string> getRX() { return RX ; } ;
  vector<double> getHPBW() { return HPBW ; } ;
  vector<double> getEFFA() { return EFFA ; } ;
  vector<double> getEFFB() { return EFFB ; } ;
  vector<double> getEFFL() { return EFFL ; } ;
  vector<double> getEFSS() { return EFSS ; } ;
  vector<double> getGAIN() { return GAIN ; } ;
  vector<string> getHORN() { return HORN ; } ;
  vector<string> getPOLTP() { return POLTP ; } ;
  vector<double> getPOLDR() { return POLDR ; } ;
  vector<double> getPOLAN() { return POLAN ; } ;
  vector<double> getDFRQ() { return DFRQ ; } ;
  vector<string> getSIDBD() { return SIDBD ; } ;
  vector<int> getREFN() { return REFN ; } ;
  vector<int> getIPINT() { return IPINT ; } ;
  vector<int> getMULTN() { return MULTN ; } ;
  vector<double> getMLTSCF() { return MLTSCF ; } ;
  vector<string> getLAGWIND() { return LAGWIND ; } ;
  vector<double> getBEBW() { return BEBW ; } ;
  vector<double> getBERES() { return BERES ; } ;
  vector<double> getCHWID() { return CHWID ; } ;
  vector<int> getARRY() { return ARRY ; } ;
  vector<int> getNFCAL() { return NFCAL ; } ;
  vector<double> getF0CAL() { return F0CAL ; } ;
  vector< vector<double> > getFQCAL() { return FQCAL ; } ;
  vector< vector<double> > getCHCAL() { return CHCAL ; } ;
  vector< vector<double> > getCWCAL() { return CWCAL ; } ;
  string getCDMY1() { return CDMY1 ; } ;
  vector<double> getDSBFC() { return DSBFC ;} ;
  int getDataSize() { return datasize_ ; } ;
  int getRowNum() { return rowNum_ ; } ;

  // get various parameters
  NRODataRecord *getRecord( int i ) ;
  virtual vector< vector<double> > getSpectrum() ;
  virtual vector<double> getSpectrum( int i ) ;
  virtual int getIndex( int irow ) ;
  virtual int getPolarizationNum() ;
  virtual vector<double> getStartIntTime() ;
  virtual double getStartIntTime( int i ) ;
  virtual double getMJD( char *time ) ;
  virtual vector<bool> getIFs() ;
  virtual vector<double> getFrequencies( int i ) ;
  virtual uInt getArrayId( string type ) ;

 protected:
  // fill header information
  virtual int fillHeader( int sameEndian ) = 0 ;

  // Endian conversion for int variable
  void convertEndian( int &value ) ;

  // Endian convertion for float variable
  void convertEndian( float &value ) ;

  // Endian conversion for double variable
  void convertEndian( double &value ) ;

  // Endian conversion for NRODataRecord
  void convertEndian( NRODataRecord *r ) ;

  // Read char data
  int readHeader( char *v, int size ) ;

  // Read int data
  int readHeader( int &v, int b ) ;

  // Read float data
  int readHeader( float &v, int b ) ;

  // Read double data
  int readHeader( double &v, int b ) ;

  // Release DataRecord
  void releaseRecord() ;

  // show primary information
  void show() ;

  // Type of file record
  string LOFIL ;

  // Version 
  string VER ;

  // Group name
  string GROUP ;

  // Project name
  string PROJ ;

  // Name of observation scheduling file
  string SCHED ;

  // Name of observer
  string OBSVR ;

  // Observation start time with format of "YYYYMMDDHHMMSS" (UTC)
  string LOSTM ;

  // observation end time with format of "YYYYMMDDHHMMSS" (UTC)
  string LOETM ;

  // Number of arrays (beams and IFs)
  int ARYNM ;

  // Number of scans
  int NSCAN ;

  // Title of observation
  string TITLE ;

  // Name of target object 
  string OBJ ;

  // Equinox (B1950 or J2000)
  string EPOCH ;

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
  string SCMOD ;

  // User-defined recessional velocity [m/s]
  double URVEL ;

  // Reference frame for recessional velocity  (LSR or HEL or GAL)
  string VREF ;

  // Definition of recessional velocity  (RAD or OPT)
  string VDEF ;

  // Switching mode  (POS or BEAM or FREQ)
  string SWMOD ;

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
  string CMTTM ;

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
  string SITE ;

  // Dummy data
  string CDMY1 ;

  // Type of detector frontend 
  vector<string> RX ;

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
  vector<string> HORN ;

  // Type of polarization  (CIRC or LINR)
  vector<string> POLTP ;

  // Rotation direction of circular polarization
  vector<double> POLDR ;

  // Polarization angle of linear polarization
  vector<double> POLAN ;

  // Switching frequency of frequcency switching [Hz]
  vector<double> DFRQ ;

  // Type of sideband  (LSB or USB or DSB)
  vector<string> SIDBD ;

  // Identifier of reference synthesizer
  vector<int> REFN ;

  // Temperature of calibrator
  vector<int> IPINT ;

  // Beam id of the multi-beam detector
  vector<int> MULTN ;

  // Scaling factor of the multi-beam detector
  vector<double> MLTSCF ;

  // Type of LAG window  (NONE or HANN or HAMM or BLCK)
  vector<string> LAGWIND ;

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

  // number of scan
  int scanNum_ ;

  // number of row
  int rowNum_ ;

  // length of scan (byte)
  int scanLen_ ;

  // length of spectral data (byte)
  int dataLen_ ;

  // Data size of the header [bytes]
  int datasize_ ;

  // maximum channel number
  int chmax_ ;

  // Current data id
  int dataid_ ;

  // Data record
  NRODataRecord *record_ ;

  // input filename
  string filename_ ;

  // file pointer
  FILE *fp_ ;

  // OS endian
  int endian_ ;
  int same_ ;

  // Logger
  //LogIO os ;
} ;


#endif /* NRO_HEADER_H */
