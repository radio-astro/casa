//# PassiveCachingDD.cc: passive implementation of a CachingDisplayData
//# Copyright (C) 1999,2000,2002
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#include <casa/aips.h>
#include <display/DisplayDatas/PassiveCachingDD.h>

namespace casa { //# NAMESPACE CASA - BEGIN

PassiveCachingDD::PassiveCachingDD() :
  CachingDisplayData() {
}

PassiveCachingDD::~PassiveCachingDD() {
}

Bool PassiveCachingDD::linToWorld(Vector<Double> &, const Vector<Double> &) {
  return False;
}

Bool PassiveCachingDD::worldToLin(Vector<Double> &, const Vector<Double> &) {
  return False;
}

String PassiveCachingDD::showPosition(const Vector<Double> &, const Bool &) {
  return String("");
}

String PassiveCachingDD::showValue(const Vector<Double> &) {
  return String("");
}
void PassiveCachingDD::refreshEH(const WCRefreshEvent &ev) {
  CachingDisplayData::refreshEH(ev);
}

Vector<String> PassiveCachingDD::worldAxisNames() {
  Vector<String> axisNames(2);
  axisNames = String("");
  return axisNames;
}

Vector<String> PassiveCachingDD::worldAxisUnits() {
  Vector<String> axisUnits(2);
  axisUnits = String("_");
  return axisUnits;
}

const uInt PassiveCachingDD::nelements(const WorldCanvasHolder 
				       &wcHolder) const {
  return 0;
}

const uInt PassiveCachingDD::nelements() const {
  return 0;
}

void PassiveCachingDD::setDefaultOptions() {
  CachingDisplayData::setDefaultOptions();
}

Bool PassiveCachingDD::setOptions(Record &rec, Record &recOut) {
  Bool ret = CachingDisplayData::setOptions(rec, recOut);
  return ret;
}

Record PassiveCachingDD::getOptions() {
  Record rec = CachingDisplayData::getOptions();
  return rec;
}

void PassiveCachingDD::cleanup() {
}

AttributeBuffer PassiveCachingDD::optionsAsAttributes() {
  AttributeBuffer buffer = CachingDisplayData::optionsAsAttributes();
  return buffer;
}

PassiveCachingDD::PassiveCachingDD(const PassiveCachingDD &) {
}

void PassiveCachingDD::operator=(const PassiveCachingDD &) {
}

} //# NAMESPACE CASA - END

