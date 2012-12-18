//
// C++ Interface: STApplyTable
//
// Description:
//
// Base class for application tables.
//
// Author: Takeshi Nakazato 
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAPAPPLYTABLE_H
#define ASAPAPPLYTABLE_H

#include <casa/Arrays/Vector.h>
#include <casa/Logging/LogIO.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ScalarColumn.h>
#include <measures/TableMeasures/ScalarMeasColumn.h>

#include "Scantable.h"
#include "STSelector.h"

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

  casa::Int nrow() {return table_.nrow();}

  casa::Vector<casa::uInt> getScan() {return scanCol_.getColumn();}
  casa::Vector<casa::uInt> getCycle() {return cycleCol_.getColumn();}
  casa::Vector<casa::uInt> getBeam() {return beamCol_.getColumn();}
  casa::Vector<casa::uInt> getIF() {return ifCol_.getColumn();}
  casa::Vector<casa::uInt> getPol() {return polCol_.getColumn();}
  casa::Vector<casa::Double> getTime() {return timeCol_.getColumn();}

  void setSelection(STSelector &sel);
  void unsetSelection();

protected:
  void setbasedata(casa::uInt irow, casa::uInt scanno, casa::uInt cycleno,
                   casa::uInt beamno, casa::uInt ifno, casa::uInt polno, 
                   casa::Double time);

  casa::Table table_, originaltable_;
  casa::ScalarColumn<casa::uInt> scanCol_, cycleCol_, beamCol_, ifCol_, polCol_;
  casa::ScalarColumn<casa::Double> timeCol_;
  casa::MEpoch::ScalarColumn timeMeasCol_;
  STSelector sel_;
  casa::LogIO os_;

private:
};

}

#endif
