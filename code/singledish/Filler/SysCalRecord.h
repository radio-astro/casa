/*
 * SysCalRecord.h
 *
 *  Created on: Jan 27, 2016
 *      Author: nakazato
 */

#ifndef SINGLEDISH_FILLER_SYSCALRECORD_H_
#define SINGLEDISH_FILLER_SYSCALRECORD_H_

#include <casacore/casa/BasicSL/String.h>
#include <casacore/measures/Measures/MPosition.h>
#include <casacore/ms/MeasurementSets/MSSysCal.h>
#include <casacore/ms/MeasurementSets/MSSysCalColumns.h>

namespace casa { //# NAMESPACE CASA - BEGIN

struct SysCalRecord {
  typedef MSSysCal AssociatingTable;
  typedef MSSysCalColumns AssociatingColumns;

  // mandatory
  Int antenna_id;
  Int feed_id;
  Int spw_id;
  Double time;
  Double interval;

  // optional
  Vector<Float> tcal;
  Matrix<Float> tcal_spectrum;
  Vector<Float> tsys;
  Matrix<Float> tsys_spectrum;

  // method
  void clear() {
    antenna_id = -1;
    feed_id = -1;
    spw_id = -1;
    time = 0.0;
    interval = 0.0;
    tcal.resize();
    tcal_spectrum.resize();
    tsys.resize();
    tsys_spectrum.resize();
  }

  SysCalRecord &operator=(SysCalRecord const &other) {
      antenna_id = other.antenna_id;
      feed_id = other.feed_id;
      spw_id = other.spw_id;
      time = other.time;
      interval = other.interval;
      tcal = other.tcal;
      tcal_spectrum = other.tcal_spectrum;
      tsys = other.tsys;
      tsys_spectrum = other.tsys_spectrum;
  }

  void add(AssociatingTable &table, AssociatingColumns &/*columns*/) {
    table.addRow(1, True);
  }

  Bool fill(uInt irow, AssociatingColumns &columns) {
    if (columns.nrow() <= irow) {
      return False;
    }

    columns.antennaId().put(irow, antenna_id);
    columns.feedId().put(irow, feed_id);
    columns.spectralWindowId().put(irow, spw_id);
    columns.time().put(irow, time);
    columns.interval().put(irow, interval);
    if (tcal.size() > 0) {
      columns.tcal().put(irow, tcal);
    }
    if (tcal_spectrum.size() > 0) {
      columns.tcalSpectrum().put(irow, tcal_spectrum);
    }
    if (tsys.size() > 0) {
      columns.tsys().put(irow, tsys);
    }
    if (tsys_spectrum.size() > 0) {
      columns.tsysSpectrum().put(irow, tsys_spectrum);
    }
    return True;
  }
};

} //# NAMESPACE CASA - END

#endif /* SINGLEDISH_FILLER_SYSCALRECORD_H_ */
