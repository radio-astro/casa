#include "ASDMVerbatimFiller.h"


ASDMVerbatimFiller::ASDMVerbatimFiller() {;}
//ASDMVerbatimFiller::ASDMVerbatimFiller(MS* ms_p, const set<const ASDM_TABLE_BASE*>& table) {
ASDMVerbatimFiller::ASDMVerbatimFiller(MS* ms_p, const set<ASDM_TABLE_BASE*>& table) {
  table_ = table;
  for(set<ASDM_TABLE_BASE*>::iterator iter = table_.begin();
      iter != table_.end();
      ++iter)
    (*iter)->buildAndAttachTable(ms_p);  
}


ASDMVerbatimFiller::~ASDMVerbatimFiller() {;}

void ASDMVerbatimFiller::fill(const ASDM& asdm) {
  for (set<ASDM_TABLE_BASE*>::const_iterator iter = table_.begin(); iter!=table_.end(); ++iter)
    (*iter)->fill(asdm);
}
