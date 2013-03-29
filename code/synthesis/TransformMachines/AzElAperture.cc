// -*- C++ -*-
//# AzElAperture.cc: Implementation of the AzElAperture class
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
//
#include <synthesis/TransformMachines/AzElAperture.h>
#include <ostream>
namespace casa{
  //
  void AzElAperture::rotate(const VisBuffer& vb, CFCell& cfc, const Double& rotAngleIncr) 
    {
      LogIO log_l(LogOrigin("AzElAperture", "rotate"));
      // If the A-Term is a No-Op, the resulting CF is rotationally
      // symmetric.
      if (isNoOp()) return;

      Double actualPA = getPA(vb), currentCFPA = cfc.pa_p.getValue("rad");
      Double dPA = currentCFPA-actualPA;
      if (fabs(dPA) > fabs(rotAngleIncr))
	//      if (fabs(actualPA-currentCFPA) > 0.0)
	{
	  Array<TT> inData;
	  inData.assign(*cfc.getStorage());
	  // log_l  << actualPA << " " << currentCFPA << LogIO::POST;
	  // cerr << "dPA = " << actualPA << " " << currentCFPA << " " 
	  //      << actualPA - currentCFPA << " " << &(*cfc.getStorage()) << endl;
	  
	  SynthesisUtils::rotateComplexArray(log_l, inData, cfc.coordSys_p,
					     *cfc.getStorage(),
					     dPA);//,"LINEAR");
					     // currentCFPA-actualPA);//,"LINEAR");
	  // Update the PA value in the CF-Cache
	  cfc.pa_p=Quantity(actualPA, "rad");

	  // static Int ii=0;
	  // {
	  //   ostringstream name;
	  //   name << "cfc." << ii << ".im";
	  //   cfc.makePersistent(name.str().c_str());
	  //   ii++;
	  // }
	}
    };
};
