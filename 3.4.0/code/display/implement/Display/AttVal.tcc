//# AttVal.cc: type-dependent interface for values of Attributes
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
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <display/Display/AttValBase.h>
#include <display/Display/AttVal.h>


namespace casa { //# NAMESPACE CASA - BEGIN

template <class T>
AttributeValue<T>::AttributeValue(const T &value, const Bool strict)
: AttributeValueBase(AttValue::AtInvalid, strict),
  itsValue(0)
{
  setValue(value);
  setType();
}

template <class T>
AttributeValue<T>::AttributeValue(const Vector<T> &value, const Bool strict) 
: AttributeValueBase(AttValue::AtInvalid, strict),
  itsValue(0)
{
  itsValue = value;
  setType();
}

template <class T>
AttributeValue<T>::~AttributeValue() 
{}

template <class T>
AttributeValue<T>::AttributeValue(const AttributeValue<T> &other) 
: AttributeValueBase(other.itsValueType, other.itsStrictness),
  itsValue(other.itsValue.copy())
{}


template <class T>
const AttributeValue<T> &AttributeValue<T>::operator=(const AttributeValue<T>& other) 
{
  if (&other != this) {
     AttributeValueBase::operator=(other);
     setValue(other.itsValue);
  }
  return *this;
}

template <class T>
void AttributeValue<T>::setValue(const T &value) 
{
  itsValue.resize(1);
  itsValue(0) = value;
}

template <class T>
void AttributeValue<T>::setValue(const Vector<T> &value) 
{ 
  itsValue.resize(0);
  itsValue = value;
}

template <class T>
AttributeValueBase* AttributeValue<T>::clone() const 
{
  return new AttributeValue<T>(*this);
}


template <class T>
Bool AttributeValue<T>::matches(const AttributeValueBase& other) const 
{
  return myMatch(myCast(other));
}

template <class T>
void AttributeValue<T>::operator+=(const AttributeValueBase& other) 
{
  if (!AttributeValueBase::myMatch(other)) return;
//  
  Vector<T>& thisValue = itsValue;  
  const Vector<T>& otherValue = myCast(other).itsValue;
//
  if (thisValue.nelements() != otherValue.nelements()) return;
//
  thisValue += otherValue;
}


template <class T>
void AttributeValue<T>::setType () 
{
  T* p = 0;
  AttributeValueBase::setType (AttValue::whatType(p));
}

template <class T>
Bool AttributeValue<T>::myMatch(const AttributeValue<T>& other) const 
{
  const Vector<T>& thisValue = itsValue;
  const Vector<T>& otherValue = other.itsValue;
//
  if (getStrictness()) {
    if (thisValue.nelements() != otherValue.nelements()) return False;
    for (uInt i = 0; i < thisValue.nelements(); i++) {
       if (thisValue(i) != otherValue(i)) return False;
    }
    return True;
  } else {
    for (uInt i = 0; i < thisValue.nelements(); i++) {
      for (uInt j = 0; j < otherValue.nelements(); j++) {
        if (thisValue(i) == otherValue(j)) return True;
      }
    }
  }
  return False;
}


template <class T>
const AttributeValue<T>& AttributeValue<T>::myCast (const AttributeValueBase& other) const
{
  return dynamic_cast<const AttributeValue<T>& >(other);
}

} //# NAMESPACE CASA - END

