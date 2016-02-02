/*
 * MSDataRecord.h
 *
 *  Created on: Jan 27, 2016
 *      Author: nakazato
 */

#ifndef SINGLEDISH_FILLER_MSDATARECORD_H_
#define SINGLEDISH_FILLER_MSDATARECORD_H_

#include <casacore/casa/BasicSL/String.h>
#include <casacore/measures/Measures/MPosition.h>
#include <casacore/ms/MeasurementSets/MeasurementSet.h>
#include <casacore/ms/MeasurementSets/MSMainColumns.h>

namespace casa { //# NAMESPACE CASA - BEGIN

struct MSDataRecord {
  typedef MeasurementSet AssociatingTable;
  typedef MSMainColumns AssociatingColumns;

  // mandatory
  String name;
  MPosition position;
  String type;
  String mount;
  Double dish_diameter;

  // optional
  String station;
  MPosition offset;

  // method
  void clear() {
    name = "";
    position = MPosition();
    type = "";
    mount = "";
    dish_diameter = -1.0;
    station = "";
    offset = MPosition();
  }

  MSDataRecord &operator=(MSDataRecord const &other) {
    name = other.name;
    position = other.position;
    type = other.type;
    mount = other.mount;
    dish_diameter = other.dish_diameter;
    station = other.station;
    offset = other.offset;
    return *this;
  }

  void add(AssociatingTable &table, AssociatingColumns &/*columns*/) {
    table.addRow(1, True);
  }

  Bool fill(uInt irow, AssociatingColumns &columns) {
    if (columns.nrow() <= irow) {
      return False;
    }

    columns.name().put(irow, name);
    columns.positionMeas().put(irow, position);
    columns.type().put(irow, type);
    columns.mount().put(irow, mount);
    columns.dishDiameter().put(irow, dish_diameter);
    if (station.size() > 0) {
      columns.station().put(irow, station);
    }
    if (anyNE(offset.getAngle().getValue(), 0.0)) {
      columns.offsetMeas().put(irow, offset);
    }
    return True;
  }
};

} //# NAMESPACE CASA - END

#endif /* SINGLEDISH_FILLER_MSDATARECORD_H_ */
