#ifndef _ASDMVERBATIMFILLER_H_
#define _ASDMVERBATIMFILLER_H_
#include "ASDMTableBase.h"
#include "ASDMTables.h"

#include <set>
#include "ASDM.h"

using namespace std;
using namespace asdm;

class ASDMVerbatimFiller {

 public:
  virtual ~ASDMVerbatimFiller();
  //  ASDMVerbatimFiller(MS* ms_p, const set<const ASDM_TABLE_BASE*>& table); 
  ASDMVerbatimFiller(MS* ms_p, const set<ASDM_TABLE_BASE*>& table); 
  void fill(const ASDM& asdm);
  
 private:
  set<ASDM_TABLE_BASE*> table_;
  ASDMVerbatimFiller();

};
#endif // _ASDMVERBATIMFILLER_H_

