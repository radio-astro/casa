//# WCCoordinateHandler.cc: base transform routines for WorldCanvas
//# Copyright (C) 1993,1994,1995,1996,1997,1998,1999,2000
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

#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/VectorIter.h>
#include <casa/Exceptions/Error.h>
#include <casa/Utilities/Assert.h>
#include <display/DisplayCanvas/WCCoordinateHandler.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Default Constructor Required
WCCoordinateHandler::WCCoordinateHandler() {
}

Bool WCCoordinateHandler::linToWorld(Matrix<Double> &world, 
				     Vector<Bool> &failures,
				     const Matrix<Double> &lin) {
  uInt nTransforms = lin.nrow();
  AlwaysAssert(nTransforms == world.nrow(), AipsError);
  Bool retval = True;

  Vector<Double> worldOut(2);
  for (uInt i = 0; i < nTransforms; i++) {
    if (failures(i) == True) { 
      retval = False; 
      continue;
    }
    Bool succ = linToWorld(worldOut, lin.row(i));
    if (succ == False) {
      failures(i) = True; 
      retval = False;
    } else {
      world(i,0) = worldOut(0); 
      world(i,1) = worldOut(1); 
    }
  }
  return retval;
}

Bool WCCoordinateHandler::worldToLin(Matrix<Double> &lin, 
				     Vector<Bool> &failures,
				     const Matrix<Double> &world) {
  uInt nTransforms = world.nrow();
  AlwaysAssert(nTransforms == lin.nrow(), AipsError);
  Bool retval = True;

  Vector<Double> linOut(2);
  for (uInt i = 0; i < nTransforms; i++) {
    if (failures(i) == True) { 
      retval = False; 
      continue; 
    }
    Bool succ = worldToLin(linOut, world.row(i));
    if (succ == False) {
	failures(i) = True; 
	retval = False;
    } else {
      lin(i,0) = linOut(0); 
      lin(i,1) = linOut(1);
    }
  }
  return retval;
}

// Destructor
WCCoordinateHandler::~WCCoordinateHandler() {
}


} //# NAMESPACE CASA - END

