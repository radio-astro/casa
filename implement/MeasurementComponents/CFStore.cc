// -*- C++ -*-
//# CFStore.cc: Implementation of the CFStore class
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
#include <synthesis/MeasurementComponents/CFStore.h>
namespace casa{

  CFStore& CFStore::operator=(const CFStore& other)
  {
    if (&other != this)
      {
	data=other.data; 
	coordSys=other.coordSys; 
	sampling=other.sampling;
	xSupport=other.xSupport;
	ySupport=other.ySupport;
	pa=other.pa;
      }
    return *this;
  };

  void CFStore::show(const char *Mesg, ostream& os)
  {
    if (!null())
      {
	if (Mesg != NULL)
	  os << Mesg << endl;
	os << "Data Shape: " << data->shape() << endl;
	os << "Sampling: " << sampling << endl;
	os << "xSupport: " << xSupport << endl;
	os << "ySupport: " << ySupport << endl;
	os << "PA = " << pa.get("deg") << endl
	  ;
      }
  };
  
  void CFStore::resize(Int nw, IPosition imShape, Bool retainValues)
  {
    xSupport.resize(nw,retainValues); ySupport.resize(nw,retainValues);
    sampling.resize(1);
    if ((imShape.nelements() > 0) && (!data.null()))
      data->resize(imShape,retainValues);
  }
} // end casa namespace
