//# Attribute.cc: arbitrary name-value pairs used in the display classes
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

#include <casa/BasicSL/String.h>
#include <casa/Arrays/Vector.h>
#include <display/Display/AttValBase.h>
#include <display/Display/AttVal.h>
#include <display/Display/AttValTol.h>
#include <display/Display/AttValPoi.h>
#include <display/Display/AttValPoiTol.h>
#include <display/Display/Attribute.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Constructor taking AtttributeValueBase.
Attribute::Attribute(const String &name, const AttributeValueBase &value) 
: itsAttributeName(name),
  itsAttributeValue(value.clone())
{}

// Remainder of constructors for all types.
Attribute::Attribute(const String &name, const uInt value, 
                     const uInt tolerance, const Bool strict) 
: itsAttributeName(name),
  itsAttributeValue(new AttributeValueTol<uInt>(value, tolerance, strict))
{}

Attribute::Attribute(const String &name, uInt *value, 
                     const uInt tolerance, const Bool strict) 
: itsAttributeName(name),
  itsAttributeValue(new AttributeValuePoiTol<uInt>(value, tolerance, strict))
{}

Attribute::Attribute(const String &name, const Int value, 
                     const Int tolerance, const Bool strict) 
: itsAttributeName(name),
  itsAttributeValue(new AttributeValueTol<Int>(value, tolerance, strict))
{}

Attribute::Attribute(const String &name, Int *value, 
                     const Int tolerance, const Bool strict)
: itsAttributeName(name),
  itsAttributeValue(new AttributeValuePoiTol<Int>(value, tolerance, strict))
{}

Attribute::Attribute(const String &name, const Float value, 
                     const Float tolerance, const Bool strict) 
: itsAttributeName(name),
  itsAttributeValue(new AttributeValueTol<Float>(value, tolerance, strict))
{}

Attribute::Attribute(const String &name,  Float *value, 
                     const Float tolerance, const Bool strict) 
: itsAttributeName(name),
  itsAttributeValue(new AttributeValuePoiTol<Float>(value, tolerance, strict))
{}

Attribute::Attribute(const String &name, const Double value, 
                     const Double tolerance, const Bool strict) 
: itsAttributeName(name),
  itsAttributeValue (new AttributeValueTol<Double>(value, tolerance, strict))
{}

Attribute::Attribute(const String &name,  Double  *value, 
                     const Double tolerance, const Bool strict) 
: itsAttributeName(name),
  itsAttributeValue(new AttributeValuePoiTol<Double>(value, tolerance, strict))
{}

Attribute::Attribute(const String &name, const Bool value, const Bool strict) 
: itsAttributeName(name),
  itsAttributeValue(new AttributeValue<Bool>(value, strict))
{}

Attribute::Attribute(const String &name,  Bool *value, const Bool strict) 
: itsAttributeName(name),
  itsAttributeValue(new AttributeValuePoi<Bool>(value, strict))
{}

Attribute::Attribute(const String &name, const String value, const Bool strict) 
: itsAttributeName(name),
  itsAttributeValue(new AttributeValue<String>(value, strict))
{}

Attribute::Attribute(const String &name,  String *value, const Bool strict) 
: itsAttributeName(name),
  itsAttributeValue(new AttributeValuePoi<String>(value, strict))
{}

Attribute::Attribute(const String &name, const Quantity value, const Bool strict) 
: itsAttributeName(name),
  itsAttributeValue(new AttributeValue<Quantity>(value, strict))
{}

Attribute::Attribute(const String &name, const Quantity value, 
		     const Quantity tolerance, const Bool strict) 
: itsAttributeName(name),
  itsAttributeValue(new AttributeValueTol<Quantity>(value, tolerance, strict))
{}

Attribute::Attribute(const String &name,  Quantity *value, const Bool strict) 
: itsAttributeName(name),
  itsAttributeValue(new AttributeValuePoi<Quantity>(value, strict))
{}

Attribute::Attribute(const String &name,  Quantity *value, 
		     const Quantity tolerance, const Bool strict)
: itsAttributeName(name),
  itsAttributeValue(new AttributeValuePoiTol<Quantity>(value, tolerance, strict))
{} 

Attribute::Attribute(const String &name, const Vector<uInt> value,
                     const uInt tolerance, const Bool strict)
: itsAttributeName(name),
  itsAttributeValue(new AttributeValueTol<uInt>(value, tolerance, strict))
{}

Attribute::Attribute(const String &name,  Vector<uInt> *value,
                     const uInt tolerance, const Bool strict)
: itsAttributeName(name),
  itsAttributeValue(new AttributeValuePoiTol<uInt>(value, tolerance, strict))
{}

Attribute::Attribute(const String &name, const Vector<Int> value,
                     const Int tolerance, const Bool strict)
: itsAttributeName(name),
  itsAttributeValue(new AttributeValueTol<Int>(value, tolerance, strict))
{}

Attribute::Attribute(const String &name,  Vector<Int> *value,
                     const Int tolerance, const Bool strict)
: itsAttributeName(name),
  itsAttributeValue(new AttributeValuePoiTol<Int>(value, tolerance, strict))
{}

Attribute::Attribute(const String &name, const Vector<Float> value,
                     const Float tolerance, const Bool strict)
: itsAttributeName(name),
  itsAttributeValue(new AttributeValueTol<Float>(value, tolerance, strict)) 
{}

Attribute::Attribute(const String &name,  Vector<Float> *value,
                     const Float tolerance, const Bool strict)
: itsAttributeName(name),
  itsAttributeValue(new AttributeValuePoiTol<Float>(value, tolerance, strict))
{}

Attribute::Attribute(const String &name, const Vector<Double> value,
                     const Double tolerance, const Bool strict)
: itsAttributeName(name),
  itsAttributeValue(new AttributeValueTol<Double>(value, tolerance, strict))
{}

Attribute::Attribute(const String &name,  Vector<Double>  *value,
                     const Double tolerance, const Bool strict)
: itsAttributeName(name),
  itsAttributeValue(new AttributeValuePoiTol<Double>(value, tolerance, strict))
{}

Attribute::Attribute(const String &name, const Vector<Bool> value,
                     const Bool strict)
: itsAttributeName(name),
  itsAttributeValue(new AttributeValue<Bool>(value, strict))
{}

Attribute::Attribute(const String &name,  Vector<Bool> *value,
                     const Bool strict)
: itsAttributeName(name),
  itsAttributeValue(new AttributeValuePoi<Bool>(value, strict))
{}

Attribute::Attribute(const String &name, const Vector<String> value,
                     const Bool strict)
: itsAttributeName(name),
  itsAttributeValue(new AttributeValue<String>(value, strict))
{}

Attribute::Attribute(const String &name,  Vector<String> *value,
                     const Bool strict)
: itsAttributeName(name),
  itsAttributeValue(new AttributeValuePoi<String>(value, strict))
{}

Attribute::Attribute(const String &name, const Vector<Quantity> value,
		     const Bool strict)
: itsAttributeName(name),
  itsAttributeValue(new AttributeValue<Quantity>(value, strict))
{}

Attribute::Attribute(const String &name, const Vector<Quantity> value,
		     const Quantity tolerance, const Bool strict)
: itsAttributeName(name),
  itsAttributeValue(new AttributeValueTol<Quantity>(value, tolerance, strict))
{}

Attribute::Attribute(const String &name,  Vector<Quantity> *value,
		     const Bool strict)
: itsAttributeName(name),
  itsAttributeValue(new AttributeValuePoi<Quantity>(value, strict))
{}

Attribute::Attribute(const String &name,  Vector<Quantity> *value,
		     const Quantity tolerance, const Bool strict)
: itsAttributeName(name),
  itsAttributeValue(new AttributeValuePoiTol<Quantity>(value, tolerance, strict))
{}


Attribute::Attribute(const Attribute &other) 
: itsAttributeName(other.itsAttributeName),
  itsAttributeValue(other.getAttributeValue()->clone())
{}


Attribute::~Attribute() 
{
  delete itsAttributeValue;
}


Attribute *Attribute::clone() const
{
  return new Attribute(itsAttributeName, *itsAttributeValue);
}

Bool Attribute::operator==(const Attribute &other) const 
{
  // if the names of the Attributes are different, return that they match
  if (itsAttributeName != other.getName())  return True;
  return ((*itsAttributeValue) == *(other.getAttributeValue()));
}

Bool Attribute::operator!=(const Attribute &other) const 
{
  if (*this == other) {
    return False;
  } else {
    return True;
  }
}

// Attribute addition operator
void Attribute::operator+=(const Attribute &other) 
{
  // check that the names match, do nothing if they don't.

  if (itsAttributeName != other.getName()) return;
  (*itsAttributeValue) += *(other.getAttributeValue());
}

// Set the value of this Attribute.  <note role=warning> The code of
// this routine was ok, until the AttributeValuePoi came along.
// AttributesValues were not supposed to change but were supposed to
// be replaced.  Now is very ugly and should be rewritten.  We assume
// here that all AttributeValues that return TpArrayInt, TpArrayFloat
// or TpArrayDouble are derived from AttributeValueTol and TpArrayBool
// and TpArrayString derived from AttributeValue (ie. they are like
// they are created by Attribute). This is ok for *this, but there is
// no guarantee that it is correct for other.

void Attribute::setValue(const Attribute &other) 
{
  if ((other.getType() != itsAttributeValue->getType()) ||
      (other.getName() != itsAttributeName)) {
    return;
  }

  switch (other.getType()){
  case AttValue::AtuInt :
    {
      AttributeValueTol<uInt>* tmpI = dynamic_cast<AttributeValueTol<uInt> *>(itsAttributeValue);
      AttributeValueTol<uInt>* tmpI2 = dynamic_cast<AttributeValueTol<uInt> *>(other.getAttributeValue());
      if (tmpI2==0) return;
//
      tmpI->setValue(tmpI2->getValue());
      tmpI->setTolerance(tmpI2->getTolerance());
      tmpI->setStrictness(tmpI2->getStrictness());
      break;
    }
  case AttValue::AtInt :
    {
      AttributeValueTol<Int>* tmpI = dynamic_cast<AttributeValueTol<Int> *>(itsAttributeValue);
      AttributeValueTol<Int>* tmpI2 = dynamic_cast<AttributeValueTol<Int> *>(other.getAttributeValue());
      if (tmpI2==0) return;
//
      tmpI->setValue(tmpI2->getValue());
      tmpI->setTolerance(tmpI2->getTolerance());
      tmpI->setStrictness(tmpI2->getStrictness());
      break;
    }
  case AttValue::AtFloat :
    {
      AttributeValueTol<Float>* tmpI = dynamic_cast<AttributeValueTol<Float> *>(itsAttributeValue);
      AttributeValueTol<Float>* tmpI2 = dynamic_cast<AttributeValueTol<Float> *>(other.getAttributeValue());
      if (tmpI2==0) return;
//
      tmpI->setValue(tmpI2->getValue());
      tmpI->setTolerance(tmpI2->getTolerance());
      tmpI->setStrictness(tmpI2->getStrictness());
      break;
    }      
  case AttValue::AtDouble :
    {
      AttributeValueTol<Double>* tmpI = dynamic_cast<AttributeValueTol<Double> *>(itsAttributeValue);
      AttributeValueTol<Double>* tmpI2 = dynamic_cast<AttributeValueTol<Double> *>(other.getAttributeValue());
      if (tmpI2==0) return;
//
      tmpI->setValue(tmpI2->getValue());
      tmpI->setTolerance(tmpI2->getTolerance());
      tmpI->setStrictness(tmpI2->getStrictness());
      break;
    }      
  case AttValue::AtBool :
    {
      AttributeValue<Bool>* tmpI = dynamic_cast<AttributeValue<Bool> *>(itsAttributeValue);
      AttributeValue<Bool>* tmpI2 = dynamic_cast<AttributeValue<Bool> *>(other.getAttributeValue());
//
      tmpI->setValue(tmpI2->getValue());
      tmpI->setStrictness(tmpI2->getStrictness());
      break;
    }
  case AttValue::AtString :
    {
      AttributeValue<String>* tmpI = dynamic_cast<AttributeValue<String> *>(itsAttributeValue);
      AttributeValue<String>* tmpI2 = dynamic_cast<AttributeValue<String> *>(other.getAttributeValue());
//
      tmpI->setValue(tmpI2->getValue());
      tmpI->setStrictness(tmpI2->getStrictness());
      break;
    }  
  case AttValue::AtQuantity :
    {
      AttributeValue<Quantity>* tmpI = dynamic_cast<AttributeValue<Quantity> *>(itsAttributeValue);
      AttributeValue<Quantity>* tmpI2 = dynamic_cast<AttributeValue<Quantity> *>(other.getAttributeValue());
//
      tmpI->setValue(tmpI2->getValue());
      tmpI->setStrictness(tmpI2->getStrictness());
      break;
    }
  default : {
    *this = other;
    break;
    }
  }  
}


AttributeValueBase* Attribute::getAttributeValue() const 
{
  // maybe should return a copy:
  // return itsAttributeValue->clone();
  // but that also gives problems....
  return itsAttributeValue;
}

// Get the name of the Attribute.
String Attribute::getName() const 
{
  return itsAttributeName;
}

// Return the DataType of this Attribute.
AttValue::ValueType Attribute::getType() const 
{
  return itsAttributeValue->getType();
}

// (Required) default constructor (private).
Attribute::Attribute() {
}

// Copy assignment (private).
const Attribute &Attribute::operator=(const Attribute &other) 
{
  if (&other != this) {
    delete itsAttributeValue;
    itsAttributeName = other.getName();
    // this class needs an assignment operator, but the following
    // implementation allows the type of the Attribute to change.  For a
    // while, this is what I wanted, but now I am not so sure that this is a
    // good feature of Attributes. So I made this method private
    itsAttributeValue = other.getAttributeValue()->clone();
  }
  return *this;
}

ostream &operator<<(ostream &os, Attribute &a) {
  os<<a.getName()<<"="<<(*(a.getAttributeValue()))<<"  ";
  return os;  }


} //# NAMESPACE CASA - END

