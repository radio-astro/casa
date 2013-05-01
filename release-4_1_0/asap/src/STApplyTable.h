//
// C++ Interface: STApplyTable
//
// Description:
//
// Base class for application tables.
//
// Author: Takeshi Nakazato <takeshi.nakazato@nao.ac.jp> (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAP_APPLY_TABLE_H
#define ASAP_APPLY_TABLE_H

#include <casa/Arrays/Vector.h>
#include <casa/Logging/LogIO.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ScalarColumn.h>
#include <measures/TableMeasures/ScalarMeasColumn.h>

#include "Scantable.h"
#include "STSelector.h"
#include "STCalEnum.h"

namespace asap {

/**
Abstract base class for all application tables.

@author Takeshi Nakazato
@date $Date:$
@version $Revision:$
*/
class STApplyTable  {
public:
  STApplyTable() {;}
  STApplyTable(const Scantable& parent, const casa::String& name);
  STApplyTable(const casa::String &name);

  virtual ~STApplyTable();

  /**
   * Add extra columns. To be implemented in derived class
   */
  virtual void setup() = 0;

  /***
   * Name of the table
   ***/
  virtual const casa::String& name() const = 0;

  const casa::Table& table() const { return table_; }
  casa::Table table() { return table_; }
  void attach();
  void attachBaseColumns();
  virtual void attachOptionalColumns() = 0;

  casa::uInt nrow() {return table_.nrow();}

  casa::Vector<casa::uInt> getScan() {return scanCol_.getColumn();}
  casa::Vector<casa::uInt> getCycle() {return cycleCol_.getColumn();}
  casa::Vector<casa::uInt> getBeam() {return beamCol_.getColumn();}
  casa::Vector<casa::uInt> getIF() {return ifCol_.getColumn();}
  casa::Vector<casa::uInt> getPol() {return polCol_.getColumn();}
  casa::Vector<casa::Double> getTime() {return timeCol_.getColumn();}

  void setSelection(STSelector &sel, bool sortByTime=false);
  void unsetSelection();
  casa::String caltype();

  void save(const casa::String &name);

  virtual casa::uInt nchan(casa::uInt ifno) = 0;

  // static methods
  static STCalEnum::CalType getCalType(const casa::String &name);
  static STCalEnum::CalType getCalType(casa::CountedPtr<STApplyTable> tab);
  static STCalEnum::CalType getCalType(STApplyTable *tab);

protected:
  void setbasedata(casa::uInt irow, casa::uInt scanno, casa::uInt cycleno,
                   casa::uInt beamno, casa::uInt ifno, casa::uInt polno, 
                   casa::uInt freqid, casa::Double time);
  casa::Block<casa::Double> getFrequenciesRow(casa::uInt id);

  casa::Table table_, originaltable_;
  casa::ScalarColumn<casa::uInt> scanCol_, cycleCol_, beamCol_, ifCol_, polCol_, freqidCol_;
  casa::ScalarColumn<casa::Double> timeCol_;
  casa::MEpoch::ScalarColumn timeMeasCol_;
  STSelector sel_;
  casa::LogIO os_;

private:
  static STCalEnum::CalType stringToType(const casa::String &caltype);
};

}

#endif
