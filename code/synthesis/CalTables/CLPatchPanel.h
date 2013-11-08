//# CLPatchPanel.h: Definition for Calibration patch panel
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
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#

#ifndef CALTABLES_CLPATCHPANEL_H
#define CALTABLES_CLPATCHPANEL_H

#include <synthesis/CalTables/NewCalTable.h>
#include <synthesis/CalTables/CTColumns.h>
#include <synthesis/CalTables/CTTimeInterp1.h>
#include <synthesis/CalTables/RIorAParray.h>
#include <synthesis/CalTables/VisCalEnum.h>
#include <scimath/Mathematics/InterpolateArray1D.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSField.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <casa/aips.h>

//#include <casa/BasicSL/Constants.h>
//#include <casa/OS/File.h>
//#include <casa/Logging/LogMessage.h>
//#include <casa/Logging/LogSink.h>

namespace casa { //# NAMESPACE CASA - BEGIN


// A specialized key class for Calibration patch panel elements
class CalPatchKey
{
public:
  CalPatchKey(IPosition keyids);
  virtual ~CalPatchKey() {};
  virtual Bool operator<(const CalPatchKey& other) const;
  virtual String print() const =0;
private:
  Vector<Int> cpk_;
};

class MSCalPatchKey : virtual public CalPatchKey
{
public:
  MSCalPatchKey(Int obs,Int fld,Int ent,Int spw,Int ant=-1);
  virtual ~MSCalPatchKey(){};
  virtual String print() const;
private:
  Int obs_,fld_,ent_,spw_,ant_;
};


class CTCalPatchKey : virtual public CalPatchKey
{
public:
  CTCalPatchKey(Int clsl,Int obs,Int fld,Int spw,Int ant=-1);
  virtual ~CTCalPatchKey(){};
  virtual String print() const;
private:
  Int clsl_,obs_,fld_,spw_,ant_;
};

// A base class for calmaps
class CalMap
{
public:

  // Null ctor (defaults everything)
  CalMap();

  // Construct from a Vector
  CalMap(const Vector<Int>& calmap);  

  // Construct from a Record (int:int; TBD: need an MS to parse strings)
  //  CalMap(const Record calmap):

  // Construct from a String (an algorithm or selection)
  //CalMap(String calmaptype,const NewCalTable& ct, const MeasurementSet ms,
  //	 Vector<Int> availids=Vector<Int>());
  // Construct from String and 2 Vectors (default, and explict to/from override)
  // TBD

  // Index method returns the ct id mapped to the specified msid
  Int operator()(Int msid) const; // ,const Vector<Int> availids=Vector<Int>()) const;

  // Return the ctids required for a set a msids (from among those available)
  //  TBD: what to do if nominally required ctid is UNavailable (return only the avail ones?)
  Vector<Int> ctids(const Vector<Int>& msids) const;//   const Vector<Int> availctids=Vector<Int>(0)) const;

  // Return the msids that will be satisfied by a specific ctid
  Vector<Int> msids(Int ctid,const Vector<Int>& superset=Vector<Int>()) const;

  // Return the verbatim (Vector) map
  Vector<Int> vmap() const { return vcalmap_; };

protected:
  Vector<Int> vcalmap_;  // simple
  //Matrix<Int> mcalmap_;  // per-id prioritized, e.g., 'nearest'
};

class FieldCalMap : public CalMap
{
public:

  // Null ctor
  FieldCalMap();

  // Construct from Vector<Int> (override CalMap)
  FieldCalMap(const Vector<Int>& calmap);

  // Algorithmic ctor that uses MS and CT meta info
  FieldCalMap(const String fieldcalmap, const MeasurementSet& ms, const NewCalTable& ct);

private:

  // Calculate the simple nearest field map
  void setNearestFieldMap(const MeasurementSet& ms, const NewCalTable& ct);
  void setNearestFieldMap(const NewCalTable& ctasms, const NewCalTable& ct);
  void setNearestFieldMap(const ROMSFieldColumns& msfc, const ROCTColumns& ctc);

  // Parse field selection map
  void setSelectedFieldMap(const String& fieldsel,
			   const MeasurementSet& ms,const NewCalTable& ct);

  // User's specification
  String fieldcalmap_;


};




// A class to parse the contents of a single CalLib slice 
//  i.e., for one caltable, one setup
class CalLibSlice
{
public:
  CalLibSlice(String obs,String fld, String ent, String spw,
	      String tinterp,String finterp,
	      Vector<Int> obsmap=Vector<Int>(1,-1), 
	      Vector<Int> fldmap=Vector<Int>(1,-1), 
	      Vector<Int> spwmap=Vector<Int>(1,-1), 
	      Vector<Int> antmap=Vector<Int>(1,-1));
  CalLibSlice(const Record& clslice,
	      const MeasurementSet& ms=MeasurementSet(),
	      const NewCalTable& ct=NewCalTable());


  String obs,fld,ent,spw;
  String tinterp,finterp;
  CalMap obsmap, fldmap, spwmap, antmap;

  // validation
  static Bool validateCLS(const Record& clslice);

  // Extract as a record
  Record asRecord();

  String state();

};


class CLPPResult
{
public:

  CLPPResult();
  CLPPResult(const IPosition& shape);
  CLPPResult(uInt nPar,uInt nFPar,uInt nChan,uInt nelem);
  CLPPResult& operator=(const CLPPResult& other); // avoids deep Array copies

  void resize(uInt nPar,uInt nFPar,uInt nChan,uInt nelem);

  Matrix<Float> operator()(Int ielem) { return result_.xyPlane(ielem); };
  Matrix<Float> result(Int ielem) { return result_.xyPlane(ielem); };
  Matrix<Bool> resultFlag(Int ielem) { return resultFlag_.xyPlane(ielem); };

  Cube<Float> result_;
  Cube<Bool> resultFlag_;

  //private:

};





class MSSelectableTable;

class CLPatchPanel
{
public:

  // From caltable name and MS shapes
  CLPatchPanel(const String& ctname,
	       const Record& callib,
	       VisCalEnum::MatrixType mtype,
	       Int nPar);

   // From caltable name and (selected) MS 
  CLPatchPanel(const String& ctname,
	       const MeasurementSet& ms,
	       const Record& callib,
	       VisCalEnum::MatrixType mtype,
	       Int nPar);
 
  // Destructor
  virtual ~CLPatchPanel();

  // Interpolate, given input obs, field, intent, spw, timestamp, & (optionally) freq 
  //    returns T if new result (anywhere,anyhow)
  //  For Complex params (calls Float version)
  Bool interpolate(Cube<Complex>& resultC, Cube<Bool>& resFlag,
		   Int obs, Int fld, Int ent, Int spw, 
		   Double time, Double freq=-1.0);
  //  For Float params
  Bool interpolate(Cube<Float>& resultR, Cube<Bool>& resFlag,
		   Int obs, Int fld, Int ent, Int spw, 
		   Double time, Double freq=-1.0);

  // Interpolate, given input obs, field, intent, spw, timestamp, & freq list
  //   This is for freqdep interpolation context
  //    returns T if new result (anywhere,anyhow)
  //  For Complex params (calls Float version)
  Bool interpolate(Cube<Complex>& resultC, Cube<Bool>& resFlag,
		   Int obs, Int fld, Int ent, Int spw, 
		   Double time, const Vector<Double>& freq);
  //  For Float params
  Bool interpolate(Cube<Float>& resultR, Cube<Bool>& resFlag,
		   Int obs, Int fld, Int ent, Int spw, 
		   Double time, const Vector<Double>& freq);

  // Const access to various state
  // TBD

  // Report state
  void listmappings();
  void state();

private:

  // Null ctor does nothing
  CLPatchPanel() :mtype_(VisCalEnum::GLOBAL) {};



  // Methods to support layered selection
  void selectOnCTorMS(Table& ctout,MSSelectableTable& msst,
		      const String& obs, const String& fld, 
		      const String& ent,
		      const String& spw, const String& ant,
		      const String& taql);
  void selectOnMS(MeasurementSet& msout,const MeasurementSet& msin,
		  const String& obs, const String& fld, 
		  const String& ent,
		  const String& spw, const String& ant);
  void selectOnCT(NewCalTable& ctout,const NewCalTable& ctin,
		  const String& obs, const String& fld, 
		  const String& spw, const String& ant1);

  // Extract unique indices from caltables
  Vector<Int> getCLuniqueObsIds(NewCalTable& ct) {return getCLuniqueIds(ct,"obs"); };
  Vector<Int> getCLuniqueFldIds(NewCalTable& ct) {return getCLuniqueIds(ct,"fld"); };
  Vector<Int> getCLuniqueSpwIds(NewCalTable& ct) {return getCLuniqueIds(ct,"spw"); };
  Vector<Int> getCLuniqueIds(NewCalTable& ct, String vcol);

  // Extract unique indices (net selected) from MS
  Vector<Int> getMSuniqueIds(MeasurementSet& ms, String vcol);
  
  // Set generic antenna/baseline map
  //  void setElemMap();

  // Resample in frequency
  void resampleInFreq(Matrix<Float>& fres,Matrix<Bool>& fflg,const Vector<Double>& fout,
		      Matrix<Float>& tres,Matrix<Bool>& tflg,const Vector<Double>& fin,
		      String finterp);

  // Baseline index from antenna indices: (assumes a1<=a2 !!)
  //  inline Int blnidx(const Int& a1, const Int& a2, const Int& nAnt) { return  a1*nAnt-a1*(a1+1)/2+a2; };

  // Translate freq axis interpolation string
  InterpolateArray1D<Double,Float>::InterpolationMethod ftype(String& strtype);


  // PRIVATE DATA:
  
  // The Caltable
  NewCalTable ct_, ctasms_;

  // Associated (selected) MS
  MeasurementSet ms_;

  // Matrix type
  VisCalEnum::MatrixType mtype_;

  // Are parameters fundamentally complex?
  Bool isCmplx_;

  // The number of (Float) parameters (per-chan, per-element)
  Int nPar_, nFPar_;

  //  InterpolateArray1D<Double,Float>::InterpolationMethod ia1dmethod_;

  // CalTable freq axis info
  Vector<Int> nChanIn_;
  Vector<Vector<Double> > freqIn_;

  // Obs, Field, Spw, Ant _output_ (MS) sizes 
  //   calibration required for up to this many
  Int nMSObs_, nMSFld_, nMSSpw_, nMSAnt_, nMSElem_;

  // Obs, Field, Spw, Ant _input_ (CalTable) sizes
  //  patch panels should not violate these (point to larger indices)
  Int nCTObs_, nCTFld_, nCTSpw_, nCTAnt_, nCTElem_;

  // Maps
  std::map<CTCalPatchKey,CLPPResult> clTres_;
  std::map<CTCalPatchKey,String> ciname_;
  std::map<CTCalPatchKey,CTTimeInterp1*> ci_;
  

  std::map<MSCalPatchKey,CLPPResult> msTres_,msFres_;
  std::map<MSCalPatchKey,String> msciname_;
  std::map<MSCalPatchKey,Int> ctspw_;
  std::map<MSCalPatchKey,String> finterp_;
  std::map<MSCalPatchKey,CTTimeInterp1*> msci_;

  // Keep track of last cal result address (per spw)
  Vector<Float*> lastresadd_;  // [nMSspw_]

  // Control conjugation of baseline-based solutions when mapping requires
  //  Vector<Bool> conjTab_;


  Cube<Float> result_;
  Cube<Bool> resFlag_;

  LogIO logsink_;


};


} //# NAMESPACE CASA - END

#endif
