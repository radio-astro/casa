/*
 * AntennaRecord.h
 *
 *  Created on: Jan 27, 2016
 *      Author: nakazato
 */

#ifndef SINGLEDISH_FILLER_ANTENNARECORD_H_
#define SINGLEDISH_FILLER_ANTENNARECORD_H_

#include <casacore/casa/BasicSL/String.h>
#include <casacore/measures/Measures/MPosition.h>
#include <casacore/ms/MeasurementSets/MSAntenna.h>
#include <casacore/ms/MeasurementSets/MSAntennaColumns.h>

namespace casa { //# NAMESPACE CASA - BEGIN
namespace sdfiller { //# NAMESPACE SDFILLER - BEGIN

struct AntennaRecord {
  typedef MSAntenna AssociatingTable;
  typedef MSAntennaColumns AssociatingColumns;

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

  AntennaRecord &operator=(AntennaRecord const &other) {
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

} //# NAMESPACE SDFILLER - END
} //# NAMESPACE CASA - END

#endif /* SINGLEDISH_FILLER_ANTENNARECORD_H_ */
