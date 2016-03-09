/*
 * FieldRecord.h
 *
 *  Created on: Jan 27, 2016
 *      Author: nakazato
 */

#ifndef SINGLEDISH_FILLER_FIELDRECORD_H_
#define SINGLEDISH_FILLER_FIELDRECORD_H_

#include <casacore/casa/Arrays/Matrix.h>
#include <casacore/casa/BasicSL/String.h>
#include <casacore/measures/Measures/MDirection.h>
#include <casacore/ms/MeasurementSets/MSField.h>
#include <casacore/ms/MeasurementSets/MSFieldColumns.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// forward decleration
template<class Reader>
class SingleDishMSFiller;

namespace sdfiller { //# NAMESPACE SDFILLER - BEGIN

struct FieldRecord {
  typedef MSField AssociatingTable;
  typedef MSFieldColumns AssociatingColumns;

private:
  // external resource
  Record source_map;
  AssociatingTable table;

public:
  // meta
  Int field_id;
  String source_name;
  MDirection::Types frame;

  // mandatory
  String name;
  Double time;
  Matrix<Double> direction;

  // optional
  String code;

  // for dummy entry
  Int const d_source_id = -1;
  String const d_name = "";
  Int const d_num_poly = 0;
  Double const d_time = 0.0;
  Matrix<Double> const d_direction = Matrix<Double>(2, 1, 0.0);

  // method
  void clear() {
    field_id = -1;
    source_name = "";
    name = "";
    time = -1.0;
    direction.resize();
    frame = MDirection::N_Types;
    code = "";
  }

  FieldRecord &operator=(FieldRecord const &other) {
    source_map = other.source_map;
    table = other.table;
    field_id = other.field_id;
    source_name = other.source_name;
    frame = other.frame;
    name = other.name;
    time = other.time;
    direction = other.direction;
    code = other.code;
    return *this;
  }

  void add(AssociatingTable &table, AssociatingColumns &columns) {
    uInt ufield_id = (uInt) field_id;
    uInt nrow = table.nrow();
    if (nrow == 0) {
      // set direction frame information
      setFrame(columns);
    }
    if (nrow <= ufield_id) {
      table.addRow(ufield_id - nrow + 1);
      uInt new_nrow = table.nrow();
      for (uInt i = nrow; i < new_nrow - 1; ++i) {
        columns.sourceId().put(i, d_source_id);
        columns.name().put(i, d_name);
        columns.time().put(i, d_time);
        columns.numPoly().put(i, d_num_poly);
        columns.delayDir().put(i, d_direction);
        columns.phaseDir().put(i, d_direction);
        columns.referenceDir().put(i, d_direction);
      }
    }
  }

  Bool fill(uInt /*irow*/, AssociatingColumns &columns) {
    if (field_id < 0) {
      return False;
    }

    uInt nrow = columns.nrow();
    if (nrow == 0) {
      // set direction frame information
      setFrame(columns);
    }

    if (table.nrow() <= (uInt) field_id) {
      return False;
    }

    columns.name().put(field_id, name);
    columns.time().put(field_id, time);
    Int source_id = -1;
    if (source_map.isDefined(source_name)) {
      source_id = source_map.asInt(source_name);
    }
    columns.sourceId().put(field_id, source_id);
    columns.delayDir().put(field_id, direction);
    columns.phaseDir().put(field_id, direction);
    columns.referenceDir().put(field_id, direction);
    Int num_poly = direction.shape()[1] - 1;
    columns.numPoly().put(field_id, num_poly);
    if (code.size() > 0) {
      columns.code().put(field_id, code);
    }
    return True;
  }

private:
  void setFrame(AssociatingColumns &columns) {
    TableRecord &record = columns.delayDir().rwKeywordSet();
    Record meas_info = record.asRecord("MEASINFO");
    String direction_frame = MDirection::showType(frame);
    meas_info.define("Ref", direction_frame);
    record.defineRecord("MEASINFO", meas_info);
    columns.phaseDir().rwKeywordSet().defineRecord("MEASINFO", meas_info);
    columns.referenceDir().rwKeywordSet().defineRecord("MEASINFO", meas_info);
  }

  template<class Reader>
  friend class casa::SingleDishMSFiller;
};

} //# NAMESPACE SDFILLER - END
} //# NAMESPACE CASA - END

#endif /* SINGLEDISH_FILLER_FIELDRECORD_H_ */
