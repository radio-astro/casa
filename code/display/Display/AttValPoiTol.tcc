//# AttValPoiTol.cc: template for tolerant and aliased AttributeValues
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
#include <casa/Arrays/ArrayMath.h>
#include <display/Display/AttValBase.h>
#include <display/Display/AttValTol.h>
#include <display/Display/AttValPoiTol.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	template <class T>
	AttributeValuePoiTol<T>::AttributeValuePoiTol(T* value,
	        const T &tolerance,
	        const Bool strict)
		: AttributeValueTol<T>(*value, tolerance, strict),
		  itsVectorPointerPtr(0),
		  itsScalarPointerPtr(value),
		  itsPointerType(whatType(value))
	{}

	template <class T>
	AttributeValuePoiTol<T>::AttributeValuePoiTol(Vector<T> *value,
	        const T &tolerance,
	        const Bool strict)
		: AttributeValueTol<T>(*value, tolerance, strict),
		  itsVectorPointerPtr(value),
		  itsScalarPointerPtr(0),
		  itsPointerType(whatType(value))
	{}


	template <class T>
	AttributeValuePoiTol<T>::~AttributeValuePoiTol()
	{}


	template <class T>
	AttributeValuePoiTol<T>::AttributeValuePoiTol(const AttributeValuePoiTol<T>& other)
		:  AttributeValueTol<T>(other),
		   itsVectorPointerPtr(other.itsVectorPointerPtr),
		   itsScalarPointerPtr(other.itsScalarPointerPtr),
		   itsPointerType(other.itsPointerType)
	{}

	template <class T>
	const AttributeValuePoiTol<T> &AttributeValuePoiTol<T>::operator=(const AttributeValuePoiTol<T> &other) {
		if (&other != this) {
			AttributeValueTol<T>::operator=(other);
			itsVectorPointerPtr = other.itsVectorPointerPtr;
			itsScalarPointerPtr = other.itsScalarPointerPtr;
			itsPointerType = other.itsPointerType;
		}
		return *this;
	}

	template <class T>
	void AttributeValuePoiTol<T>::setValue(const T &value) {
		AttributeValueTol<T>::setValue(value);
		updatePointerValue();
	}

	template <class T>
	void AttributeValuePoiTol<T>::setValue(const Vector<T> &value) {
		AttributeValueTol<T>::setValue(value);
		updatePointerValue();
	}


	template <class T>
	AttributeValueBase* AttributeValuePoiTol<T>::clone() const {
		return new AttributeValuePoiTol<T>(*this);
	}


	template <class T>
	void AttributeValuePoiTol<T>::operator+=(const AttributeValueBase &other) {

// Check other is of the correct class

		if (other.className() != String("AttributeValuePoiTol")) return;

//
		if (getType() != other.getType()) return;
		if (getType() == AttValue::AtInvalid) return;
//
		const AttributeValuePoiTol<T>& tmp = myCast(other);
		if (getPointerType() != tmp.getPointerType()) return;
//
		AttributeValue<T>::operator+=(other);
//
		updatePointerValue();
	}

	template <class T>
	void AttributeValuePoiTol<T>::updatePointerValue() const {

// If the new value that was set was a vector but we started
// off life with a scalar, too bad.  All that can happen
// is that the scalar is updated with the first value of the
// vector.  This is because the memory of itsScalarPtr and
// itsVectorPtr do not long belong to this object.
// I [nebk] think this is a design error.

		if (itsScalarPointerPtr != 0) {
			*itsScalarPointerPtr = getValue()(0);
			if (getValue().nelements() > 1) {
				cerr << "AttValPoiTol::updatePointerValue - discarding vector elements.  This is" << endl;
				cerr << "AttValPoiTol::updatePointerValue - a design flaw. Please defect this error" << endl;
			}
		} else {
			const Vector<T>& value = getValue();
			itsVectorPointerPtr->resize(0);
			*itsVectorPointerPtr = value;
		}
	}


// get the DataType of the aliased variable
	template <class T>
	DataType AttributeValuePoiTol<T>::getPointerType() const {
		return itsPointerType;
	}


	template <class T>
	const AttributeValuePoiTol<T>& AttributeValuePoiTol<T>::myCast (const AttributeValueBase& other) const {
		return dynamic_cast<const AttributeValuePoiTol<T>& >(other);
	}


} //# NAMESPACE CASA - END

