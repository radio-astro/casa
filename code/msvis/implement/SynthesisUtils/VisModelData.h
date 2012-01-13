//# VisModelData.h: Calculate Model Visibilities for a buffer from model images / complist
//# Copyright (C) 2011
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
//# $Id$
#ifndef VISMODELDATA_H
#define VISMODELDATA_H
#include <casa/aips.h>
#include <casa/Containers/Record.h>
#include <casa/Containers/Block.h>
#include <casa/Arrays/Cube.h>
#include <msvis/SynthesisUtils/ComponentFTMachine.h>
namespace casa { //# NAMESPACE CASA - BEGIN
//#forward
  class VisBuffer;
  class ComponentList;  
  class FTMachine;
  class MeasurementSet;
  template <class T> class Vector;
  template <class T> class CountedPtr;
// <summary>
// Object to provide MODEL_DATA visibilities on demand
// 
// </summary>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//  FTMachine and ComponentFTMachine
//
// </prerequisite>
//
// <etymology>


// </etymology>
//
// <synopsis>
// </synopsis>
//
// <example>
// <srcblock>
// </srcblock>
// </example>
//
// <motivation>
// </motivation>
//
// <todo asof="">
// </todo>

class VisModelData {
 public:
  //empty constructor
  VisModelData();
  //From a FTMachine Record
  //VisModelData(const Record& ftmachinerec, const Vector<Int>& validfieldids, const Vector<Int>& msIds);
  virtual ~VisModelData();
  //Add Image/FTMachine to generate visibilities for
  //void addFTMachine(const Record& recordFTMachine, const Vector<Int>& validfieldids, const Vector<Int>& msIds);
  //Add componentlist to generate visibilities for
  //void addCompFTMachine(const ComponentList& cl, const Vector<Int>& validfieldids, 
  //			const Vector<Int>& msIds);
  //For simple model a special case for speed 
  
  void addFlatModel(const Vector<Double>& value, const Vector<Int>& validfieldids, 
		    const Vector<Int>& msIds);

  //add componentlists or ftmachines 
  void addModel(const Record& rec,  const Vector<Int>& msids);

  //put the model data for this VisBuffer in the modelVisCube
  Bool getModelVis(VisBuffer& vb);

  //this is a helper function that writes the model record to the ms 
  static void putModel(const MeasurementSet& thems, const RecordInterface& rec, const Vector<Int>& validfields, const Vector<Int>& spws, const Vector<Int>& starts, const Vector<Int>& nchan,  const Vector<Int>& incr, Bool iscomponentlist=True, Bool incremental=False);

  //helper function to clear the keywordSet of the ms of the model  for the fields 
  //in that ms
  static void clearModel(const MeasurementSet& thems);
  static FTMachine* NEW_FT(const Record& ftrec);
  //check if an addFT or addCompFT is necessary
  //Bool hasFT(Int msid, Int fieldid);
  //Bool hasCL(Int msid, Int fieldid);
  Bool hasModel(Int msid, Int field, Int spw); 
 private:
  void initializeToVis();
  Vector<CountedPtr<ComponentList> >getCL(const Int msId, const Int fieldId, Int spw);
  Vector<CountedPtr<FTMachine> >getFT(const Int msId, const Int fieldId, Int spw);
  Block<Vector<CountedPtr<ComponentList> > > clholder_p;
  Block<Vector<CountedPtr<FTMachine> > > ftholder_p;
  Block<Vector<Double> > flatholder_p;
  CountedPtr<ComponentFTMachine> cft_p;
  Cube<Int> ftindex_p;
  Cube<Int> clindex_p;

};

}//end namespace
#endif // VISMODELDATA_H
