//# SynthesisUtilMethods.h: Imager functionality sits here; 
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003
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
//#
//# $Id$

#ifndef SYNTHESIS_SYNTHESISUTILS_H
#define SYNTHESIS_SYNTHESISUTILS_H

#include <casa/aips.h>
#include <casa/OS/Timer.h>
#include <casa/Containers/Record.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/QuantumHolder.h>
#include <measures/Measures/MDirection.h>
#include <casa/Quanta/MVAngle.h>
#include <measures/Measures/MFrequency.h>
#include <casa/Utilities/DataType.h>
#include <stdcasa/StdCasa/CasacSupport.h>
#include <coordinates/Coordinates/Projection.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/CoordinateSystem.h>

namespace casa { //# NAMESPACE CASA - BEGIN

  class SynthesisParams;
  class SynthesisParamsImage;
  class SynthesisParamsSelect;
  
  class SynthesisUtilMethods
  {
  public:
    // Default constructor

    SynthesisUtilMethods();
    ~SynthesisUtilMethods();
    
    // All functions here are stand-alone, self-contained methods.
    
    
    // Partitioning syntax for Selection parameters
    //
    // Input Record (example) :
    //  { 'ms0' : { 'msname':xxx1, 'spw':yyy } ,
    //     'ms1' : { 'msname':xxx2, 'spw':yyy } }
    //
    //  Output Record (example for partitioning on spw) : 
    //  { '0' : { 'ms0' : { 'msname':xxx1, 'spw':yyy1 } ,
    //              'ms1' : { 'msname':xxx2, 'spw':yyy1 }    }
    //    '1' : { 'ms0' : { 'msname':xxx1, 'spw':yyy2 } ,
    //               'ms1' : { 'msname':xxx2, 'spw':yyy2 }   }   }
    
    // Data partitioning rules for CONTINUUM imaging
    Record continuumDataPartition(Record &selpars, const Int npart);
    
    // Data partitioning rules for CUBE imaging
    Record cubeDataPartition(Record &selpars, Int npart);
    
    // Image cube partitioning rules for CUBE imaging
    Record cubeImagePartition(Record &impars, Int npart);
    
    
  protected:

  };

class SynthesisParams
{
public:
  SynthesisParams(){};
  virtual ~SynthesisParams(){};
  virtual void fromRecord(Record &inrec)=0;
  virtual void setDefaults()=0;
  virtual String verify()=0;
  virtual Record toRecord()=0;
protected:
  // All return strings are error messages. Null if valid.
  String readVal(Record &rec, String id, String& val);
  String readVal(Record &rec, String id, Int& val);
  String readVal(Record &rec, String id, Float& val);
  String readVal(Record &rec, String id, Bool& val);
  String readVal(Record &rec, String id, Vector<Int>& val);
  String readVal(Record &rec, String id, Vector<Float>& val);
  String readVal(Record &rec, String id, Vector<String>& val);
  String stringToQuantity(String instr, Quantity& qa);
  String stringToMDirection(String instr, MDirection& md);
  String readVal(Record &rec, String id, Quantity& val);
  String readVal(Record &rec, String id, MDirection& val);
  // Others..
  String MDirectionToString(MDirection val);
  String QuantityToString(Quantity val);
};

  class SynthesisParamsSelect : public SynthesisParams
{
public:

  SynthesisParamsSelect();
  ~SynthesisParamsSelect();

  void fromRecord(Record &inrec);
  void setDefaults();
  String verify();
  Record toRecord();

  String msname, spw, freqbeg, freqend;
  MFrequency::Types freqframe;
  String field, antenna, timestr, scan, obs, state, uvdist,taql;
  Bool usescratch,readonly,incrmodel;

};


 
  class SynthesisParamsImage: public SynthesisParams
{
public:

  SynthesisParamsImage();
  ~SynthesisParamsImage();

  void fromRecord(Record &inrec);
  void setDefaults();
  String verify();
  Record toRecord();

  // Generate Coordinate System 
  CoordinateSystem buildCoordinateSystem(MeasurementSet& msobj) const;
  Vector<Int> decideNPolPlanes(const String& stokes) const;
  IPosition shp() const;

  // Sky coordinates
  String imageName, stokes, startModel;
  Vector<Int> imsize;
  Vector<Quantity> cellsize;
  Projection projection;
  MDirection phaseCenter;
  Int facets;

  // Spectral coordinates ( TT : Add other params here  )
  Int nchan, nTaylorTerms;
  Quantity freqStart, freqStep, refFreq;
  MFrequency::Types freqFrame;
  Vector<Quantity> restFreq;

  Bool overwrite;

};


  class SynthesisParamsGrid: public SynthesisParams
{
public:

  SynthesisParamsGrid();
  ~SynthesisParamsGrid();

  void fromRecord(Record &inrec);
  void setDefaults();
  String verify();
  Record toRecord();

  // FTMachine setup
  String ftmachine, convFunc;
  Int wprojplanes;
  Bool useDoublePrec, useAutoCorr; 
  Float padding;

  // Moving phase center ? 
  Quantity distance;
  MDirection trackDir;
  Bool trackSource; 
  
  // For wb-aprojection ftm.
  Bool aTermOn, psTermOn,mTermOn,wbAWP,doPointing, doPBCorr, conjBeams;
  String cfCache;
  Float computePAStep, rotatePAStep;
  

};


} //# NAMESPACE CASA - END

#endif
