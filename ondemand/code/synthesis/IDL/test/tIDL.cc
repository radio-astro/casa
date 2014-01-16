//# tIDL.cc:  this tests IDL
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
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <synthesis/IDL/IDL.h>

#ifdef HAVE_IDL_LIB
#include <trial/IDL/IDLexport.h>
#endif

#include <casa/namespace.h>

int main()
{
#ifdef HAVE_IDL_LIB
  {
    IDL idl(0);
    idl.runCommand("print,'hello'");
    Matrix<Float> test(5,4);
    test=1.0;
    idl.sendArray(test, "AARRAY");
    idl.runCommand("print,AARRAY");
    idl.runCommand("BARRAY=SQRT(2*AARRAY)");
    idl.runCommand("print,BARRAY");
    std::cout << test << std::endl;
    std::cout << idl.getArray("BARRAY") << std::endl;
  }
  return 0;
#else
  std::cout << "IDL is not available" << std::endl;
  return 3;     // test is skipped
#endif

};
