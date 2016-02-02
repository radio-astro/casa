/*
 * DataRecord.h
 *
 *  Created on: Jan 27, 2016
 *      Author: nakazato
 */

#ifndef SINGLEDISH_FILLER_DATARECORD_H_
#define SINGLEDISH_FILLER_DATARECORD_H_

#include <casacore/casa/BasicSL/String.h>
#include <casacore/casa/Arrays/Matrix.h>
#include <casacore/casa/Arrays/Vector.h>

namespace casa { //# NAMESPACE CASA - BEGIN

struct DataRecord {

  // mandatory
  Double time;
  Double interval;
  Int antenna_id;
  Int field_id;
  Int spw_id;
  Int feed_id;
  Int scan;
  Int subscan;
  uInt polno;
  String intent;
  String pol_type;
  Matrix<Double> direction;
  Vector<Float> data;
  Vector<Bool> flag;
  Bool flag_row;

  // optional
  Vector<Float> tsys;
  Vector<Float> tcal;

  // method
  void clear() {
    time = -1.0;
    interval = -1.0;
    antenna_id = -1;
    field_id = -1;
    spw_id = -1;
    feed_id = -1;
    scan = -1;
    subscan = -1;
    polno = 0;
    intent = "";
    pol_type = "";
    direction.resize(2, 1);
    direction = 0.0;
    data.resize();
    flag.resize();
    flag_row = True;
    tsys.resize();
    tcal.resize();
  }

  DataRecord &operator=(DataRecord const &other) {
    time = other.time;
    interval = other.interval;
    antenna_id = other.antenna_id;
    field_id = other.field_id;
    spw_id = other.spw_id;
    feed_id = other.feed_id;
    scan = other.scan;
    subscan = other.subscan;
    polno = other.polno;
    intent = other.intent;
    pol_type = other.pol_type;
    direction.assign(other.direction);
    data.assign(other.data);
    flag.assign(other.flag);
    flag_row = other.flag_row;
    tsys.assign(other.tsys);
    tcal.assign(other.tcal);

    return *this;
  }
};

} //# NAMESPACE CASA - END

#endif /* SINGLEDISH_FILLER_DATARECORD_H_ */
