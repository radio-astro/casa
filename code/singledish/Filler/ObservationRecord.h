/*
 * ObservationRecord.h
 *
 *  Created on: Jan 27, 2016
 *      Author: nakazato
 */

#ifndef SINGLEDISH_FILLER_OBSERVATIONRECORD_H_
#define SINGLEDISH_FILLER_OBSERVATIONRECORD_H_

#include <casacore/casa/BasicSL/String.h>
#include <casacore/ms/MeasurementSets/MSObservation.h>
#include <casacore/ms/MeasurementSets/MSObsColumns.h>

namespace casa { //# NAMESPACE CASA - BEGIN
namespace sdfiller { //# NAMESPACE SDFILLER - BEGIN

struct ObservationRecord {
  typedef MSObservation AssociatingTable;
  typedef MSObservationColumns AssociatingColumns;

  // mandatory
  String telescope_name;
  Vector<Double> time_range;
  String observer;
  String project;

  // optional
  String schedule_type;
  Double release_date;
  Vector<String> schedule;
  Vector<String> log;

  // method
  void clear() {
    telescope_name = "";
    time_range.resize();
    observer = "";
    project = "";
    schedule_type = "";
    release_date = -1.0;
    schedule = "";
    log.resize();
  }

  ObservationRecord &operator=(ObservationRecord const &other) {
    telescope_name = other.telescope_name;
    time_range = other.time_range;
    observer = other.observer;
    project = other.project;
    schedule_type = other.schedule_type;
    release_date = other.release_date;
    schedule = other.schedule;
    log = other.log;
    return *this;
  }

  void add(AssociatingTable &table, AssociatingColumns &/*columns*/) {
    table.addRow(1, True);
  }

  Bool fill(uInt irow, AssociatingColumns &columns) {
    if (columns.nrow() <= irow) {
      return False;
    }

    columns.telescopeName().put(irow, telescope_name);
    columns.timeRange().put(irow, time_range);
    columns.observer().put(irow, observer);
    columns.project().put(irow, project);
    if (schedule_type.size() > 0) {
      columns.scheduleType().put(irow, schedule_type);
    }
    if (0.0 <= release_date) {
      columns.releaseDate().put(irow, release_date);
    }
    if (schedule.size() > 0) {
      columns.schedule().put(irow, schedule);
    }
    if (log.size() > 0) {
      columns.log().put(irow, log);
    }
    return True;
  }
};

} //# NAMESPACE SDFILLER - END
} //# NAMESPACE CASA - END

#endif /* SINGLEDISH_FILLER_OBSERVATIONRECORD_H_ */
