#include <casa/aips.h>

#if ! defined (MSVIS_VisModelDataI_H)
#define MSVIS_VisModelDataI_H

namespace casa {

  class MeasurementSet;
  class RecordInterface;
  class String;
  class TableRecord;
  class VisBuffer;

  class VisModelDataI;

  template <typename T> class Vector;


class VisModelDataI {

 public:

  typedef VisModelDataI * (* Factory) ();

  static bool setFactory (Factory);

  //empty constructor
  VisModelDataI() {}

  //From a FTMachine Record
  //VisModelDataI(const Record& ftmachinerec, const Vector<Int>& validfieldids, const Vector<Int>& msIds);
  virtual ~VisModelDataI() {}

  //Add Image/FTMachine to generate visibilities for
  //void addFTMachine(const Record& recordFTMachine, const Vector<Int>& validfieldids, const Vector<Int>& msIds);
  //Add componentlist to generate visibilities for
  //void addCompFTMachine(const ComponentList& cl, const Vector<Int>& validfieldids, 
  //			const Vector<Int>& msIds);
  //For simple model a special case for speed 
  
  // void addFlatModel(const Vector<Double>& value, const Vector<Int>& validfieldids, 
  // 		    const Vector<Int>& msIds);

  // //add componentlists or ftmachines 
  virtual void addModel(const RecordInterface& rec,  const Vector<Int>& msids, const VisBuffer& vb) = 0;

  static VisModelDataI * create ();

  // //put the model data for this VisBuffer in the modelVisCube
  virtual Bool getModelVis(VisBuffer& vb) = 0;

  // //this is a helper function that writes the model record to the ms 

  virtual void putModelI (const MeasurementSet& thems, const RecordInterface& rec,
		  	  const Vector<Int>& validfields, const Vector<Int>& spws,
			  const Vector<Int>& starts, const Vector<Int>& nchan,
			  const Vector<Int>& incr, Bool iscomponentlist=True, Bool incremental=False) = 0;

  // //helper function to clear the keywordSet of the ms of the model  for the fields 
  // //in that ms
  virtual void clearModelI(const MeasurementSet& thems) = 0;
  // // ...with field selection and optionally spw
  // static void clearModel(const MeasurementSet& thems, const String field, const String spws=String(""));

  // //Functions to see if model is defined in the MS either in the SOURCE table or else in the MAIN
  virtual Bool isModelDefinedI(const Int fieldId, const MeasurementSet& thems, String& key, Int& sourceRow) = 0;
  // static Bool isModelDefined(const String& elkey, const MeasurementSet& thems);

  // //Get a given model that is defined by key
  // //Forcing user to use a TableRecord rather than Generic RecordInterface ...just so as to avoid a copy.
  virtual Bool getModelRecordI(const String& theKey, TableRecord& theRec, const MeasurementSet& theMs) = 0;

  // // List the fields
  // static void listModel(const MeasurementSet& thems);

  // static FTMachine* NEW_FT(const Record& ftrec);
  // //check if an addFT or addCompFT is necessary
  // //Bool hasFT(Int msid, Int fieldid);
  // //Bool hasCL(Int msid, Int fieldid);
  // //returns a -1 if there is no model for this combination of ms,field,spw...but has not been  // checked yet if there is one
  // //returns a -2 if it has been tested before but does have it.
  // //returns a 1 if it has a model stored 
  virtual Int hasModel(Int msid, Int field, Int spw) = 0; 

 private:

  static Factory factory_p;

};



}

#endif // ! defined (MSVIS_VisModelDataI_H)
