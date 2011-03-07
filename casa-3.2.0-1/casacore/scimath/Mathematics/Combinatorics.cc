//# Copyright (C) 2010 by ESO (in the framework of the ALMA collaboration)
//# Copyright (C) 1995,1996,1997,1998,1999,2000,2001,2002
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
//# $Id: $
//   

#include <scimath/Mathematics/Combinatorics.h>

namespace casa { //# NAMESPACE CASA - BEGIN

    Vector<uInt> Combinatorics::_factorialCache(0);


    uInt Combinatorics::factorial(const uInt n) {
        if (n < _factorialCache.size()) {
            return _factorialCache[n];
        }
        uInt oldSize = _factorialCache.size();
        if (_factorialCache.size() < 2) {
            _factorialCache.resize(2);
            _factorialCache[0] = 1;
            _factorialCache[1] = 1;
        }
        else {
            _factorialCache.resize(n+1, True);
        }
        if (n < 2) {
            return 1;
        }
        for (uInt i=oldSize-1; i<=n; i++) {
            _factorialCache[i] = i * _factorialCache[i-1];
        }
        return _factorialCache[_factorialCache.size()-1];
    }

    uInt Combinatorics::choose(const uInt n, const uInt k) {
        if (k > n) {
            throw AipsError("k cannot be greater than n");
        }
        return factorial(n)/(factorial(k)*factorial(n-k));
    }
} //# NAMESPACE CASA - END

