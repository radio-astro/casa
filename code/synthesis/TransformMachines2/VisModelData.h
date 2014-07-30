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
#ifndef TRANSFORM2_VISMODELDATA_H
#define TRANSFORM2_VISMODELDATA_H
#include <casa/aips.h>
#include <casa/Containers/Record.h>
#include <casa/Containers/Block.h>
#include <casa/Arrays/Cube.h>
#include <synthesis/TransformMachines2/ComponentFTMachine.h>
#include <msvis/MSVis/VisModelDataI.h>
#include <msvis/MSVis/VisBuffer.h> //here only for the pure virtual function that uses this

namespace casa { //# NAMESPACE CASA - BEGIN
//#forward
  namespace vi{class VisBuffer2;}
  class ComponentList;  
  class MeasurementSet;
  template <class T> class Vector;
  template <class T> class CountedPtr;
 namespace refim{ //namespace refim
 	class FTMachine;
// <summary>
// Object to provide MODEL_DATA visibilities on demand
// </summary>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//  FTMachine and ComponentFTMachine
//
// </prerequisite>
//
// <etymology>
// Vis for Visibility
// ModelData is self explanatory
// </etymology>
//
// <synopsis>
// This Class also offer a lot of helper functions to query add and delete/modify models
// To be noted the models that are supported are FTMachines and Componentlist
// A given FIELD_ID  can have multiple models added to the same record 
// (i.e for e.g multiple spw models or multiple outlier images  or in the 
// case of componentlist multiple component list models. 
// A model may be valid for multiple fields e.g a mosaic
// To save storage in such cases multiple field id will point to the same model
// Look for the keyword "definedmodel_field_x" in the Source table keywords (or the main 
// table if source is not present).
// This will point to another key which is going to point to the model.
// Thus multiple "definedmodel_field_x"  may point to the same key.
// If there is no valid SOURCE table the key will hold a Record of the model to use 
// in the main table 
// Otherwise in the SOURCE table the key will hold the row of the SOURCE table in whose 
// SOURCE_MODEL cell the Record of the model to use will be resident.
// Now a given model Record in a SOURCE_MODEL cell  
// can hold multiple FTMachine's or Compnentlist (e.g multiple direction images or 
// spw images  associated with a given field)  
// and they are all cumulative (while respecting spw selection conditions) 
// when a request is made for the model visibility
// </synopsis>
//
// <example>
// <srcblock>
//  MeasurementSet theMS(.....)

//   VisibilityIterator vi(theMS,sort);
//   VisBuffer vb(vi);
//    MDirection myDir=vi.msColumns().field().phaseDirMeas(0);
//    ComponentList cl;
//    SkyComponent otherPoint(ComponentType::POINT);
//    otherPoint.flux() = Flux<Double>(0.00001, 0.0, 0.0, 0.00000);
//    otherPoint.shape().setRefDirection(myDir);
//    cl.add(otherPoint);
//    Record clrec;
//     clrec.define("type", "componentlist");
//   ///Define the fields ans spw for which this model is valid
//    clrec.define("fields", field);
//   clrec.define("spws", Vector<Int>(1, 0));
//    clrec.defineRecord("container", container);
//    Record outRec;
//    outRec.define("numcl", 1);
//    outRec.defineRecord("cl_0", clrec);

//     Vector<Int>spws(1,0);
//     Save model to the MS
//      VisModelData.putModel(theMS, container, field, spws, Vector<Int>(1,0), Vector<Int>(1,63), Vector<Int>(1,1), True, False);
//
//////now example to serve model visibility
// vi.origin();
///////////////
//VisModelData vm;
////           String modelkey; 
//            Int snum;
//   Bool hasmodkey=VisModelData::isModelDefined(vb.fieldId(), vi.ms(), modelkey, snum);
// Setup vm to serve the model for the fieldid()
//   if( hasmodkey){
//                      TableRecord modrec;
//                       VisModelData::getModelRecord(modelkey, modrec, visIter_p->ms())
//                       vm.addModel(modrec, Vector<Int>(1, msId()), vb);
//                      }
//    for (vi.originChunks();vi.moreChunks(); vi.nextChunk()){
//      for (vi.origin(); vi.more(); vi++){
//       This fills the vb.modelVisCube with the appropriate model visibility
//	  vm.getModelVis(vb);
//	cerr << "field " << vb.fieldId() << "  spw " << vb.spectralWindow() <<" max " <<  max(amplitude(vb.modelVisCube())) << endl;
 
//      }
//    }
// </srcblock>
// </example>
//
// <motivation>
// </motivation>
//
// <todo asof="2013/05/24">
// Allow validity of models for a given section of time only
//
// </todo>

  class VisModelData : public VisModelDataI {
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
  void addModel(const RecordInterface& rec,  const Vector<Int>& msids, const vi::VisBuffer2& vb);
  void addModel(const RecordInterface& /*rec*/,  const Vector<Int>& /*msids*/, const VisBuffer& /*vb*/){throw(AipsError("Called the wrong version of VisModelData"));};


  VisModelDataI * clone ();

  //put the model data for this VisBuffer in the modelVisCube
  Bool getModelVis(vi::VisBuffer2& vb);
  Bool getModelVis(VisBuffer& /*vb*/){throw(AipsError("called the wrong version of VisModelData"));};
  //this is a helper function that writes the model record to the ms 
  void putModelI(const MeasurementSet& thems, const RecordInterface& rec,
		 const Vector<Int>& validfields, const Vector<Int>& spws,
		 const Vector<Int>& starts, const Vector<Int>& nchan,
		 const Vector<Int>& incr, Bool iscomponentlist=True, Bool incremental=False)
  {
    putModel (thems, rec, validfields, spws, starts, nchan, incr, iscomponentlist, incremental);
  }
  static void putModel(const MeasurementSet& thems, const RecordInterface& rec, const Vector<Int>& validfields, const Vector<Int>& spws, const Vector<Int>& starts, const Vector<Int>& nchan,  const Vector<Int>& incr, Bool iscomponentlist=True, Bool incremental=False);

  //helper function to clear the keywordSet of the ms of the model  for the fields 
  //in that ms
  void clearModelI(const MeasurementSet& thems) { clearModel (thems); }
  static void clearModel(const MeasurementSet& thems);
  // ...with field selection and optionally spw
  static void clearModel(const MeasurementSet& thems, const String field, const String spws=String(""));

  //Functions to see if model is defined in the MS either in the SOURCE table or else in the MAIN
  Bool isModelDefinedI(const Int fieldId, const MeasurementSet& thems, String& key, Int& sourceRow)
  {
    return isModelDefined (fieldId, thems, key, sourceRow);
  }
  static Bool isModelDefined(const Int fieldId, const MeasurementSet& thems, String& key, Int& sourceRow);
  static Bool isModelDefined(const String& elkey, const MeasurementSet& thems);

  //Get a given model that is defined by key
  //Forcing user to use a TableRecord rather than Generic RecordInterface ...just so as to avoid a copy.
  Bool getModelRecordI(const String& theKey, TableRecord& theRec, const MeasurementSet& theMs)
  {
    return getModelRecord (theKey, theRec, theMs);
  }
  static Bool getModelRecord(const String& theKey, TableRecord& theRec, const MeasurementSet& theMs);

  // List the fields
  static void listModel(const MeasurementSet& thems);

  static FTMachine* NEW_FT(const Record& ftrec);
  //check if an addFT or addCompFT is necessary
  //Bool hasFT(Int msid, Int fieldid);
  //Bool hasCL(Int msid, Int fieldid);
  //returns a -1 if there is no model for this combination of ms,field,spw...but has not been  // checked yet if there is one
  //returns a -2 if it has been tested before but does have it.
  //returns a 1 if it has a model stored 
  Int hasModel(Int msid, Int field, Int spw); 
 private:
  void initializeToVis();
  Vector<CountedPtr<ComponentList> >getCL(const Int msId, const Int fieldId, Int spw);
  Vector<CountedPtr<FTMachine> >getFT(const Int msId, const Int fieldId, Int spw);
  static Bool addToRec(TableRecord& therec, const Vector<Int>& spws);
  static Bool removeSpwFromMachineRec(RecordInterface& ftclrec, const Vector<Int>& spws);
  static Bool removeFTFromRec(TableRecord& therec, const String& keyval, const Bool relabel=True);
  static Bool removeSpw(TableRecord& therec, const Vector<Int>& spws);
  static Bool putModelRecord(const Vector<Int>& fieldIds, TableRecord& theRec, MeasurementSet& theMS);
  //Remove the Record which has the given key...will exterminate it from both the Source table or Main table
  static void removeRecordByKey(MeasurementSet& theMS, const String& theKey);
  //put the Record by key if sourcerownum=-1 then it is saved in the main table
  //this default should only be used  if the optional SOURCE table in non-existant
  static void putRecordByKey(MeasurementSet& theMS, const String& theKey, const TableRecord& theRec, const Int sourceRowNum=-1);
  static void deleteDiskImage(MeasurementSet& theMS, const String& theKey);
  Block<Vector<CountedPtr<ComponentList> > > clholder_p;
  Block<Vector<CountedPtr<FTMachine> > > ftholder_p;
  Block<Vector<Double> > flatholder_p;
  CountedPtr<ComponentFTMachine> cft_p;
  Cube<Int> ftindex_p;
  Cube<Int> clindex_p;
  static Bool initialize;
};

}// end namespace refim
}//end namespace

#endif // VISMODELDATA_H
