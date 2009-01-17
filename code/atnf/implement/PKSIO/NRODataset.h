//#---------------------------------------------------------------------------
//# NRODataset.h: Class for NRO 45m and ASTE data.
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

#ifndef NRO_DATASET_H
#define NRO_DATASET_H

#include <string>
#include <stdio.h>
#include <vector>

using namespace std ;

#define CHANNEL_MAX 2048

// <summary>
// <linkto class=NRODataset>NRODataset</linkto> is a class 
// that represents a single scan record 
// (scan header + data record) for NRO 45m and ASTE raw data. 
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
// header and scan record. NROData is a representation of the scan record 
// that contains a scan header and a spectral data.
// </etymology>
//
// <synopsis>
// <linkto class=NRODataset>NRODataset</linkto> is a struct that is an 
// implementation of a single scan record.
// All attributes are public, i.e. able to access directly.
// </synopsis>
//
// <example>
// 
// </example>
//
// <motivation>
// <linkto class=NRODataset>NRODataset</linkto> are defined to import 
// NRO and ASTE raw data effictively. 
// It enable to read scan record all at once instead of to read each 
// attributes individually.
// </motivation>
//
// <to do asof="">
//   <li>
// </to do>  
struct NRODataset
{
  // Type of file record
  char LSFIL[4] ;

  // Scan number
  int ISCAN ;

  // Integration start time with format of "YYYYMMDDHHMMSS.sss"
  char LAVST[24] ;

  // Scan type (ON or ZERO)
  char SCANTP[8] ;

  // Offset position of the scan RA/GL/AZ [rad]
  double DSCX ;

  // Offset position of the scan DEC/GB/EL [rad]
  double DSCY ;

  // Absolute position of the scan RA/GL/AZ [rad]
  double SCX ;

  // Absolute position of the scan DEC/GB/EL [rad]
  double SCY ;

  // Position of the scan in the program RA/GL/AZ [rad]
  double PAZ ;

  // Position of the scan in the program DEC/GB/EL [rad]
  double PEL ;

  // Real position of the scan RA/GL/AZ [rad]
  double RAZ ;

  // Real position of the scan DEC/GB/EL [rad]
  double REL ;

  // X-coordinate value [rad]
  double XX ;

  // Y-coordinate value [rad]
  double YY ;

  // Array type (beam or IF)
  char ARRYT[4] ;

  // Ambient temperature [Celcius]
  float TEMP ;

  // Air pressure [hPa]
  float PATM ;

  // Pressure of water vapor [hPa]
  float PH2O ;

  // Wind speed [m/s]
  float VWIND ;

  // Wind direction [rad]
  float DWIND ;

  // Atmospheric optical depth
  float TAU ;

  // System noise temperature [K]
  float TSYS ;

  // Atmospheric temperature [K]
  float BATM ;

  // Line number of executable
  int LINE ;

  // Dummy data
  int IDMY1[4] ;

  // Recessional velocity of the antenna [m/s]
  double VRAD ;

  // Central frequency in the rest frame [Hz]
  double FREQ0 ;

  // Tracking frequency in the rest frame [Hz]
  double FQTRK ;

  // Frequency of first IF [Hz]
  double FQIF1 ;

  // ALC control voltage
  double ALCV ;

  // OFF position before and after the integration
  double OFFCD[2][2] ;

  // Data flag  0: effective  1: flagged
  int IDMY0 ;

  // Dummy data
  int IDMY2 ;

  // Correction for Doppler frequency shift
  double DPFRQ ;

  // Dummy data
  char CDMY1[144] ;

  // Scaling factor of the array
  double SFCTR ;

  // Offset for array data
  double ADOFF ;

  // Spectral data: Originally, the data are double array. 
  // But they are quantized and converted to the int array with 
  // a scalling factor and an offset value. Additionally, 
  // this int array is stored into the char array.
  char LDATA[CHANNEL_MAX*12/8] ; 
} ;

#endif /* NRO_DATASET_H */
