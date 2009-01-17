//# DefaultWCCoordinateHandler.cc: default coordinate handling for WorldCanvas
//# Copyright (C) 1993,1994,1995,1996,1997,1998,1999,2000,2001
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

#include <display/DisplayCanvas/DefaultWCCoordinateHandler.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Default Constructor Required
DefaultWCCoordinateHandler::DefaultWCCoordinateHandler() {
}

Bool DefaultWCCoordinateHandler::linToWorld(Vector<Double> &world,
					    const Vector<Double> &lin) {
  if (world.nelements()!=2) world.resize(2);
  world(0) = lin(0);
  world(1) = lin(1);
  return True;
}

Bool DefaultWCCoordinateHandler::worldToLin(Vector<Double> &lin,
					    const Vector<Double> &world) {
  if (lin.nelements()!=2) lin.resize(2);
  lin(0) = world(0);
  lin(1) = world(1);
  return True;
}

Bool DefaultWCCoordinateHandler::linToWorld(Matrix<Double> & world,
                                            Vector<Bool> & failures,
                                            const Matrix<Double> & lin) {
   return WCCoordinateHandler::linToWorld(world, failures, lin);
}

Bool DefaultWCCoordinateHandler::worldToLin(Matrix<Double> & lin,
                                            Vector<Bool> & failures, 
                                            const Matrix<Double> & world) {
   return WCCoordinateHandler::worldToLin(lin, failures, world);
}


Vector<String> DefaultWCCoordinateHandler::worldAxisNames() {
  Vector<String> axNames(2);
  axNames(0)  = "Pixel";
  axNames(1)  = "Pixel";
  return axNames;
}

Vector<String> DefaultWCCoordinateHandler::worldAxisUnits() {
  Vector<String> axNames(2);
  axNames(0) = "Pixel";
  axNames(1) = "Pixel";
  return axNames;
}

// Destructor
DefaultWCCoordinateHandler::~DefaultWCCoordinateHandler() {
}


} //# NAMESPACE CASA - END

