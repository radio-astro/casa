//# SolvableVisCalMetaInfo.h: Definitions of interface for SolvableVisCalMetaInfo
//# Copyright (C) 1996,1997,2000,2001,2002,2003
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
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#ifndef CALTABLES_CALSETMETAINFO_H
#define CALTABLES_CALSETMETAINFO_H

#include <casa/aips.h>
#include <ms/MeasurementSets/MSAntennaColumns.h>
#include <ms/MeasurementSets/MSAntenna.h>
#include <ms/MeasurementSets/MSFieldColumns.h>
#include <ms/MeasurementSets/MSField.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>
#include <ms/MeasurementSets/MSSpectralWindow.h>

// #include <casa/Logging/LogMessage.h>
// #include <casa/Logging/LogSink.h>
// #include <casa/Logging/LogIO.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// **********************************************************
//  CalSetMetaInfo
//

class CalSetMetaInfo {
public:

  CalSetMetaInfo():
    antTableName("ANTENNA"), fieldTableName("FIELD"), spwTableName("SPECTRAL_WINDOW"),
    calTableMSAC(NULL), calTableMSFC(NULL), calTableMSSpC(NULL),
    calAntTable(NULL), calFieldTable(NULL), calSpwTable(NULL)  
  {rootName="";};

  CalSetMetaInfo(const String& root);
  
  ~CalSetMetaInfo()   {cleanUp();};

  void setRootName(const String& root) {rootName=root;};
  String fullSubTableName(String& subTableName,const String& base) 
  {return String(base + "/" + subTableName);};
  String fullSubTableName(String& subTableName) 
  {return fullSubTableName(subTableName,rootName);};

  void reset(const String& root);
  String getAntName(Int ant);
  String getFieldName(Int field);
  String getSpwName(Int spw);
  Double  getSpwRefFreq(Int spw);
  // Vector<Double>  getSpwChanWidth(Int spw);
  Double  getSpwTotalBandwidth(Int spw);

  Vector<String> getFieldNames();
  Vector<String> getAntNames();
  Vector<String> getSpwNames();
  Vector<Double> getSpwRefFreqs();
  Bool verify(CalSetMetaInfo& otherCSMI);

  void copyMetaTables(const String& target);

protected:

  void cleanUp()
  {
    if (calTableMSAC  != NULL) {delete calTableMSAC;  calTableMSAC  = NULL;}
    if (calTableMSFC  != NULL) {delete calTableMSFC;  calTableMSFC  = NULL;}
    if (calTableMSSpC != NULL) {delete calTableMSSpC; calTableMSSpC = NULL;}

    if (calAntTable   != NULL) {delete calAntTable;   calAntTable   = NULL;}
    if (calFieldTable != NULL) {delete calFieldTable; calFieldTable = NULL;}
    if (calSpwTable   != NULL) {delete calSpwTable;   calSpwTable   = NULL;}
  }
private:
  String rootName, antTableName, fieldTableName, spwTableName;
  ROMSAntennaColumns *calTableMSAC;
  ROMSFieldColumns *calTableMSFC;
  ROMSSpWindowColumns *calTableMSSpC;
  MSAntenna *calAntTable;
  MSField *calFieldTable;
  MSSpectralWindow *calSpwTable;
};




} //# NAMESPACE CASA - END

#endif
