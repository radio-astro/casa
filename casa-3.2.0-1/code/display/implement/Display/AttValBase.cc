//# AttValBase.cc: abstract base class and support class for AttributeValues
//# Copyright (C) 1996,1997,1999,2000,2001,2002
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
#include <display/Display/AttValBase.h>

namespace casa { //# NAMESPACE CASA - BEGIN

AttributeValueBase::AttributeValueBase(AttValue::ValueType type, Bool strict)
: itsValueType(type),
  itsStrictness(strict)
{}


AttributeValueBase::~AttributeValueBase() 
{}

AttributeValueBase::AttributeValueBase(const AttributeValueBase& other) 
: itsValueType(other.itsValueType),
  itsStrictness(other.itsStrictness)
{}

const AttributeValueBase& AttributeValueBase::operator=(const AttributeValueBase& other) 
{
   if (this != &other) {
      itsValueType = other.itsValueType;
      itsStrictness = other.itsStrictness;
   }
   return *this;
}


AttValue::ValueType  AttributeValueBase::getType() const 
{
  return itsValueType;
}

void AttributeValueBase::setStrictness(const Bool &newStrict) 
{
  itsStrictness = newStrict;
}

Bool AttributeValueBase::getStrictness() const 
{
  return itsStrictness;
}

void AttributeValueBase::setType(const AttValue::ValueType &newType) 
{
  itsValueType = newType;
}

Bool AttributeValueBase::operator==(const AttributeValueBase& other) const 
{
  if (!myMatch(other)) return False;
  return ((this->matches(other)) && other.matches(*this));
}

Bool AttributeValueBase::operator!=(const AttributeValueBase& other) const 
{
  return (!(*this==other));  
}


Bool AttributeValueBase::myMatch(const AttributeValueBase &other) const
{  
  if (getType() != other.getType())  return False;
  if (getType() == AttValue::AtInvalid) return False;
  return True;
}  

ostream &operator<<(ostream &os, AttributeValueBase &av) {
  av.print(os); return os;  }


} //# NAMESPACE CASA - END

