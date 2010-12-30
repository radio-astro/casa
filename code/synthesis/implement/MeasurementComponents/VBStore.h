// -*- C++ -*-
//# VBStore.h: Definition of the VBStore class
//# Copyright (C) 1997,1998,1999,2000,2001,2002,2003
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
#ifndef SYNTHESIS_VBSTORE_H
#define SYNTHESIS_VBSTORE_H

namespace casa { //# NAMESPACE CASA - BEGIN
  class VBStore
  {
  public:
    VBStore() {};
    ~VBStore() {};
    inline Int nRow_() {return nRow;};
    Matrix<Double>& uvw_() {return uvw;};
    Vector<Bool>& rowFlag_() {return rowFlag;};
    Cube<Bool>& flagCube_() {return flagCube;};
    Matrix<Float>& imagingWeight_() {return imagingWeight;};
    Cube<Complex>& visCube_() {return visCube;};
    Vector<Double>& freq_() {return freq;};

    Int nRow;
    Matrix<Double> uvw;
    Vector<Bool> rowFlag;
    Cube<Bool> flagCube;
    Matrix<Float> imagingWeight;
    Cube<Complex> visCube;
    Vector<Double> freq;
  };

} //# NAMESPACE CASA - END
#endif
