//# <MirTypeAssert.h>: a base class for auto-checking of primitive type sizes
//# Copyright (C) 2000,2001,2002,2003
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
//#
//# $Id: MirTypeAssert.h,v 1.1 2009/09/03 18:54:47 pteuben Exp $

#ifndef BIMA_MIRTYPEASSERT_H
#define BIMA_MIRTYPEASSERT_H

#include <casa/Exceptions/Error.h>

#include <casa/namespace.h>
// <summary>
// an exception indicating that assumptions about the size of primitive 
// types are not satisfied.
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <etymology>
// Current support of Miriad uv-IO is based on certain assumptions about use 
// of memory, specifically the size of primitive types.
// </etymology>
//
// <synopsis>
// This exception is used to indicate that certain assumptions about the size
// of primitive types do not hold.  Miriad IO in aips++ assumes that the sizes 
// of Int, Float, and Double are equal to the corresponding sizes of int, float,
// and Double; this exception is thrown when these conditions are not met.
//
// Normally, applications do not create and throw this exception directly as
// it has been integrated into the MirTypeAssert class which
// automatically checks the memory assumptions upon construction.  Any class
// that depends on this assumption can inherit from MirTypeAssert.  
// </synopsis>
//
// <motivation>
// See summary above.
// </motivation>
//
class MiriadMemoryAssumptionError : public AipsError { 
public:

    // An exception with a message indicating the unsupported feature
    MiriadMemoryAssumptionError(const String &msg);

    virtual ~MiriadMemoryAssumptionError() throw();

protected:
    // Message-less exceptions should not be created excepted possibly by 
    // subclasses
    MiriadMemoryAssumptionError();
};

// <summary>
// a base for classes that rely on assumptions regarding privitive type sizes.
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <etymology>
// Current support of Miriad uv-IO is based on certain assumptions about use 
// of memory, specifically the size of primitive types.
// </etymology>
//
// <synopsis>
// This exception is used to indicate that certain assumptions about the size
// of primitive types do not hold.  Miriad IO in aips++ assumes that the sizes 
// of Int, Float, and Double are equal to the corresponding sizes of int, float,
// and Double; this exception is thrown when these conditions are not met.
//
// Normally, applications do not create and throw this exception directly as
// it has been integrated into the MirTypeAssert class which
// automatically checks the memory assumptions upon construction.  Any class
// that depends on this assumption can inherit from MirTypeAssert.  
// </synopsis>
//
// <motivation>
// See summary above.
// </motivation>
//
class MirTypeAssert { 
public:
    MirTypeAssert();

protected:
    static Bool ok;
};

#endif
