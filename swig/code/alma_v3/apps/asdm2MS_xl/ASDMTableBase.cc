#include "ASDMTableBase.h"

ASDM_TABLE_BASE::ASDM_TABLE_BASE() {table_p_ = 0;}

ASDM_TABLE_BASE::~ASDM_TABLE_BASE() {}

Table* ASDM_TABLE_BASE::table_p() {return table_p_;}

const string& ASDM_TABLE_BASE::name() const { return name_; }

void ASDM_TABLE_BASE::buildAndAttachTable(MS* attachMS) {
  SetupNewTable tableSetup(attachMS->tableName() + "/" + String(name_),
		       tableDesc(),
		       Table::New);
  table_p_ = new Table(tableSetup, TableLock(TableLock::PermanentLockingWait));
  AlwaysAssert(table_p_, AipsError);
  attachMS->rwKeywordSet().defineTable(name_, *table_p_);
}


