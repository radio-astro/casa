//# tCCList.cc:  this tests CCList
//# Copyright (C) 1996,1997,1999,2001
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

#include <casa/iostream.h>
#include <casa/aips.h>
#include <casa/Containers/Block.h>
#include <synthesis/MeasurementEquations/CCList.h>

#include <casa/namespace.h>
int main()
{
  {
    uInt npol = 1;
    uInt ndim = 2;
    CCList list1 (npol, ndim, 10);  // 1 pol, 2-D, 10 components    
    DebugAssert(list1.nPol() == npol , AipsError);
    DebugAssert(list1.nDim() == ndim , AipsError);
    
    Block<Float> flux(npol);
    Block<Int>   pos(ndim); 
    for (uInt i = 0; i< 15; i++) {
      for (uInt j = 0; j < ndim; j++){
	pos[j] = 100*i + j;
      }
      for (uInt j = 0; j < npol; j++){
	flux[j] = 100*i + j;
      }
      list1.addComp( flux, pos );
      cout << " i, nComp(), freeComp() = " << i << " " << list1.nComp()<<
	" "<< list1.freeComp() << endl;
    }
    Float *flux_p = list1.fluxPtr();
    Int *pos_p = list1.positionPtr();

    cout << " Retrieve some CCs " << endl;
    for (uInt i=0;i<15;i++) {
      cout <<  *pos_p <<" "<< *(pos_p+1) <<" "<< *flux_p << endl;
      pos_p+= list1.nDim();
      flux_p+= list1.nPol();
    }

    list1.resize(100);
    flux_p = list1.pixelFlux(10);
    pos_p = list1.pixelPosition(10);
    cout << "10 th cc: " << endl;
    cout << *pos_p <<" "<< *(pos_p+1)<<" "<< *flux_p << endl;

  }
};
