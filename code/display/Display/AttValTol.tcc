//# AttValTol.cc: templated class for tolerant AttributeValues
//# Copyright (C) 1996,1997,1999,2000,2001
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
#include <casa/Utilities/DataType.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Quanta/QMath.h>
#include <casa/Quanta/QLogical.h>
#include <casa/BasicMath/Math.h>
#include <display/Display/AttValBase.h>
#include <display/Display/AttVal.h>
#include <display/Display/AttValTol.h>

namespace casa { //# NAMESPACE CASA - BEGIN

template <class T>
AttributeValueTol<T>::AttributeValueTol(const T &value, const T &tolerance,
                                        const Bool strict) 
: AttributeValue<T>(value, strict), 
  itsValueTolerance(tolerance) 
{}

template <class T>
AttributeValueTol<T>::AttributeValueTol(const Vector<T> &value, 
                                        const T &tolerance, 
                                        const Bool strict) 
: AttributeValue<T>(value, strict), 
  itsValueTolerance(tolerance) 
{}

template <class T>
AttributeValueTol<T>::~AttributeValueTol() 
{}

template <class T>
AttributeValueTol<T>::AttributeValueTol(const AttributeValueTol<T> &other) 
: AttributeValue<T>(other),
  itsValueTolerance(other.itsValueTolerance)
{}

template <class T>
const AttributeValueTol<T> &AttributeValueTol<T>::operator=(const AttributeValueTol<T> &other) 
{
  if (&other != this) {
     AttributeValue<T>::operator=(other);
     itsValueTolerance = other.itsValueTolerance;
  }
  return *this;
}


template <class T>
AttributeValueBase* AttributeValueTol<T>::clone() const 
{
  return new AttributeValueTol<T>(*this);
}



template <class T>
Bool AttributeValueTol<T>::matches(const AttributeValueBase& other) const 
{
  return myMatch(AttributeValue<T>::myCast(other));
}

template <class T>
Bool AttributeValueTol<T>::myMatch(const AttributeValue<T>& other) const 
{
  Vector<T> thisValue = getValue();
  const Vector<T>& otherValue = other.getValue();
//
  if (getStrictness()) {
    if (thisValue.nelements() != otherValue.nelements()) return False;
    //if( !allTrue(nearAbs(thisValue, otherValue, itsValueTolerance))) return False;
     for (uInt i = 0; i < thisValue.nelements(); i++) {
        if (abs(thisValue(i) - otherValue(i)) > itsValueTolerance) return False;
     }
     return True;
  } else {
     //if(anyTrue(nearAbs(thisValue, otherValue, itsValueTolerance))) return True;
     for (uInt i = 0; i < thisValue.nelements(); i++) {
        for (uInt j = 0; j < otherValue.nelements(); j++) {
           if (abs(thisValue(i) - otherValue(j)) <= itsValueTolerance) return True;
        }
     }
  }
  return False;
}


} //# NAMESPACE CASA - END

