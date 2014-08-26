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
#include <scimath/Mathematics/GaussianBeam.h>

#include <msvis/MSVis/VisibilityIterator.h>

//#include <synthesis/ImagerObjects/TmpSwitch.h>

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
    //uniform contiguous partition in frequency step
    // Note that the spw selection will have a "-1"
    //if there is no overlap in the data selection provided and 
    // frequency range provided
    static Record cubeDataPartition(const Record &selpars, const Int npart, const Double freqBeg, const Double freqEnd, const MFrequency::Types eltype=MFrequency::LSRK);

    // freqBeg and freqEnd are frequency range  of the sub image cubes defined in frame set here
    // number of partions is obviously the length of freqBeg and freqEnd 
    // Use this for non uniform width of imge frequencies
    static Record cubeDataPartition(const Record& selpars, const Vector<Double>& freqBeg, const Vector<Double>& freqEnd, const MFrequency::Types frame=MFrequency::LSRK);
    
    //CoordinateSystem and number of channels of original cube is passed in 
    //Output record is the ms's and data selection for each part.
    // also the Vector of outCsys and outnChan are the  coordinatesystems
    // and nchannel of the sub cube for each part.
    // The image is divided  in n part along spectral channel
    //now if one of the sub cube has no match the  the spw selection will have a "-1"
    //for that part. The caller will have to deal with that for load balancing etc..
    //  Output Record (example for partitioning on spw) : 
    //  { '0' : { 'ms0' : { 'msname':xxx1, 'spw': '0:5~10' } ,
    //              'ms1' : { 'msname':xxx2, 'spw':'0:20~25' },   
    //                 'nchan': 6, 
    //                 'coordsys': { A record of the coordinatesystem of subcube 0}}
    //    '1' : { 'ms0' : { 'msname':xxx1, 'spw':'0:9~14' } ,
    //               'ms1' : { 'msname':xxx2, 'spw':'0:24~29' },
    //               'nchan':6, 
    //                'coordsys': { A record of the coordinatesystem of subcube 1} }
    //   }
    // the coordsys keyed record can be use in CoordinateSystem:;restore to reconstrucr
    // it is the same as the parameter outCsys
    static Record cubeDataImagePartition(const Record & selpars, const CoordinateSystem&
				    incsys, const Int npart, const Int nchannel, 
				    Vector<CoordinateSystem>& outCsys,
				    Vector<Int>& outnChan);

    // Image cube partitioning rules for CUBE imaging
    Record cubeImagePartition(Record &impars, Int npart);
    
    
  protected:
    static String mergeSpwSel(const Vector<Int>& fspw, const Vector<Int>& fstart, const Vector<Int>& fnchan, const Matrix<Int>& spwsel);

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
  String stringToQuantity(String instr, Quantity& qa) const;
  String stringToMDirection(String instr, MDirection& md);
  String readVal(Record &rec, String id, Quantity& val);
  String readVal(Record &rec, String id, MDirection& val);
  // Others..
  String MDirectionToString(MDirection val);
  String QuantityToString(Quantity val);
  String recordQMToString(Record &rec);
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
  String field, antenna, timestr, scan, obs, state, uvdist,taql,intent;
  Bool usescratch,readonly,incrmodel;

  String datacolumn;

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
  CoordinateSystem buildCoordinateSystem(ROVisibilityIterator* rvi);
  CoordinateSystem buildCoordinateSystem(vi::VisibilityIterator2* vi2);
  CoordinateSystem buildCoordinateSystemCore(MeasurementSet& msobj, 
					     Vector<Int> spwids, Int fld, 
					     Double freqmin, Double freqmax);

  Vector<Int> decideNPolPlanes(const String& stokes) const;
  IPosition shp() const;
  Bool getImFreq(Vector<Double>& ChanFreq, Vector<Double>& ChanWidth, 
		 Double& refPix, String& specmode,
		 const MEpoch& obsEpoch, const MPosition& obsPosition,
		 const Vector<Double>& dataChanFreqs, const Vector<Double>& dataFreqRes,
		 const MFrequency::Types& dataFrame, const Quantity& qrestfreq, 
		 const Double& freqmin, const Double& freqmax,
		 const MDirection& phaseCenter );
  
  String findSpecMode(const String& mode) const;
  String MDopToVelString(Record &rec);

  // Sky coordinates
  String imageName, stokes, startModel;
  Vector<Int> imsize;
  Vector<Quantity> cellsize;
  Projection projection;
  Bool useNCP;
  MDirection phaseCenter;
  Int phaseCenterFieldId;

  // Spectral coordinates ( TT : Add other params here  )
  Int nchan, nTaylorTerms, chanStart, chanStep;
  Quantity freqStart, freqStep, refFreq, velStart, velStep;
  MFrequency::Types freqFrame;
  MFrequency mFreqStart, mFreqStep;
  MRadialVelocity mVelStart, mVelStep;
  Vector<Quantity> restFreq;
  String start, step, frame, veltype, mode, reffreq, sysvel, sysvelframe;
  // private variable to store ref frame defined in Quantity or Measure 
  // in start or step parameters and veltype from measure (e.g. MDoppler)
  String qmframe, mveltype;  
  String tststr;
  // for holding quantity or measure records
  Record startRecord, stepRecord, reffreqRecord, sysvelRecord, restfreqRecord;
  //freqframe coversion?
  Bool freqFrameValid;

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

  // Facets for gridding.
  Int facets;

  // Spectral axis interpolation
  String interpolation;

  // Moving phase center ? 
  Quantity distance;
  MDirection trackDir;
  Bool trackSource; 
  
  // For wb-aprojection ftm.
  Bool aTermOn, psTermOn,mTermOn,wbAWP,doPointing, doPBCorr, conjBeams;
  String cfCache;
  Float computePAStep, rotatePAStep;

  // Mapper Type.
  String mType;

};


  class SynthesisParamsDeconv: public SynthesisParams
{
public:

  SynthesisParamsDeconv();
  ~SynthesisParamsDeconv();

  void fromRecord(Record &inrec);
  void setDefaults();
  String verify();
  Record toRecord();

  String imageName, algorithm, startModel;
  Int deconvolverId; // maybe remove ? It's only to tag summary info.
  Int nTaylorTerms; 
  Vector<Float> scales;
  String maskString;

  GaussianBeam restoringbeam;
  String usebeam;

  };


} //# NAMESPACE CASA - END

#endif
