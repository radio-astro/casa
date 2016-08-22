//
// C++ Interface: STCalSkyTable
//
// Description:
//
// ApplyTable for sky calibration.
//
// Author: Takeshi Nakazato <takeshi.nakazato@nao.ac.jp> (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAP_CALSKY_TABLE_H
#define ASAP_CALSKY_TABLE_H

#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ScalarColumn.h>

#include "Scantable.h"
#include "STApplyTable.h"

namespace asap {

/**
ApplyTable for sky calibration

@author Takeshi Nakazato
@date $Date:$
@version $Revision:$
*/
class STCalSkyTable : public STApplyTable {
public:
  STCalSkyTable() {;}
  STCalSkyTable(const Scantable& parent, const casa::String &caltype);
  STCalSkyTable(const casa::String &name);

  virtual ~STCalSkyTable();

  void setup();
  void attachOptionalColumns();

  const casa::String& name() const {return name_;}

  void setdata(casa::uInt irow, casa::uInt scannos, casa::uInt cycleno, 
               casa::uInt beamno, casa::uInt ifno, casa::uInt polno, 
               casa::uInt freqid, casa::Double time, casa::Float elevation, 
               const casa::Vector<casa::Float> &spectra,
	       const casa::Vector<casa::uChar> &flagtra);
  void appenddata(casa::uInt scanno, casa::uInt cycleno, casa::uInt beamno, 
                  casa::uInt ifno, casa::uInt polno, casa::uInt freqid,  
                  casa::Double time, casa::Float elevation, 
                  const casa::Vector<casa::Float> &spectra,
		  const casa::Vector<casa::uChar> &flagtra);
  
  casa::Vector<casa::Float> getElevation() const {return elCol_.getColumn();}
  casa::Matrix<casa::Float> getSpectra() const {return spectraCol_.getColumn();}
  casa::Matrix<casa::uChar> getFlagtra() const {return flagtraCol_.getColumn();}
  casa::uInt nchan(casa::uInt ifno);

  //casa::Vector<casa::Double> getBaseFrequency(casa::uInt whichrow);

private:
  static const casa::String name_;
  const casa::String caltype_;
  casa::ArrayColumn<casa::Float> spectraCol_;
  casa::ArrayColumn<casa::uChar> flagtraCol_;
  casa::ScalarColumn<casa::Float> elCol_;
};

}

#endif
