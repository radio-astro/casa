//# AttributeBuffer.cc: Buffer for Attribute in Display Library
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
#include <casa/BasicSL/String.h>
#include <casa/Arrays/Vector.h>
#include <display/Display/AttributeBuffer.h>
#include <display/Display/Attribute.h>
#include <display/Display/AttValBase.h>
#include <display/Display/AttVal.h>
#include <display/Display/AttValTol.h>


namespace casa { //# NAMESPACE CASA - BEGIN

	AttributeBuffer::AttributeBuffer()
		: attributes(0),
		  nonDeletable(0)
	{}


	AttributeBuffer::AttributeBuffer(const AttributeBuffer& other)
		: attributes(0),
		  nonDeletable(0) {
		other.addBuff(*this);
	}


	const AttributeBuffer& AttributeBuffer::operator=(const AttributeBuffer& other) {
		if (&other != this) {
			erase();
			other.addBuff(*this);
		}
		return *this;
	}


	AttributeBuffer::~AttributeBuffer() {
// Get rid of the attributes. The destructor of the PtrBlock does not delete
// these. It calls delete [] array, but array is an array of void *, so
// nothing happens.
		for (uInt i = 0; i < attributes.nelements(); i++) {
			delete attributes[i];
		}
	}


	Int AttributeBuffer::nelements() const {
		return attributes.nelements();
	}



	void AttributeBuffer::add(const AttributeBuffer& otherBuf) {
		otherBuf.addBuff(*this);
	}


	void AttributeBuffer::set(const AttributeBuffer& otherBuf) {
		otherBuf.setBuff(*this);
	}


	void AttributeBuffer::addBuff(AttributeBuffer& other) const {
		for (Int i = 0; i < nelements(); i++) {
			other.add( *(attributes[i]), nonDeletable[i] );
		}
	}

	void AttributeBuffer::setBuff(AttributeBuffer& other) const {
		for (Int i = 0; i < nelements(); i++) {
			other.set(*(attributes[i]));
		}
	}




	void AttributeBuffer::add(const Attribute& newAttribute, const Bool permanent) {

		// check if an attribute with the same name exist
		if ( !exists(newAttribute.getName()) ) {
			Attribute *tmp = newAttribute.clone();
			addAttributeToBuffer(tmp, permanent);
		}

	}

	void AttributeBuffer::add(const String& name, const uInt newValue,
	                          const uInt tolerance, const Bool strict,
	                          const Bool permanent) {
		Vector<uInt> tmp(1);
		tmp = newValue;
		add(name, tmp, tolerance, strict, permanent);
	}

	void AttributeBuffer::add(const String& name, const Int newValue,
	                          const Int tolerance, const Bool strict,
	                          const Bool permanent) {
		Vector<Int> tmp(1);
		tmp = newValue;
		add(name, tmp, tolerance, strict, permanent);
	}
	void AttributeBuffer::add(const String& name, const Float newValue,
	                          const Float tolerance, const Bool strict,
	                          const Bool permanent) {
		Vector<Float> tmp(1);
		tmp = newValue;
		add(name, tmp, tolerance, strict, permanent);
	}

	void AttributeBuffer::add(const String& name, const Double newValue,
	                          const Double tolerance, const Bool strict,
	                          const Bool permanent) {
		Vector<Double> tmp(1);
		tmp = newValue;
		add(name, tmp, tolerance, strict, permanent);
	}

	void AttributeBuffer::add(const String& name, const Bool newValue,
	                          const Bool strict, const Bool permanent) {
		Vector<Bool> tmp(1);
		tmp = newValue;
		add(name, tmp, strict, permanent);
	}

	void AttributeBuffer::add(const String& name, const String& newValue,
	                          const Bool strict, const Bool permanent) {
		Vector<String> tmp(1);
		tmp = newValue;
		add(name, tmp,  strict, permanent);
	}

	void AttributeBuffer::add(const String& name, const Quantity newValue,
	                          const Bool strict, const Bool permanent) {
		Vector<Quantity> tmp(1);
		tmp = newValue;
		add(name, tmp, strict, permanent);
	}

	void AttributeBuffer::add(const String& name, const Vector<uInt>& newValue,
	                          const uInt tolerance, const Bool strict,
	                          const Bool permanent) {
		// check if an attribute with the same name exist
		if ( !exists(name) ) {
			Attribute *tmp = new Attribute(name, newValue, tolerance, strict);
			addAttributeToBuffer(tmp, permanent);
		}
	}
	void AttributeBuffer::add(const String& name, const Vector<Int>& newValue,
	                          const Int tolerance, const Bool strict,
	                          const Bool permanent) {
		// check if an attribute with the same name exist
		if ( !exists(name) ) {
			Attribute *tmp = new Attribute(name, newValue, tolerance, strict);
			addAttributeToBuffer(tmp, permanent);
		}
	}

	void AttributeBuffer::add(const String& name, const Vector<Float>& newValue,
	                          const Float tolerance, const Bool strict,
	                          const Bool permanent) {
		// check if an attribute with the same name exist
		if ( !exists(name) ) {
			Attribute *tmp = new Attribute(name, newValue, tolerance, strict);
			addAttributeToBuffer(tmp, permanent);
		}
	}

	void AttributeBuffer::add(const String& name, const Vector<Double>& newValue,
	                          const Double tolerance, const Bool strict,
	                          const Bool permanent) {
		// check if an attribute with the same name exist
		if ( !exists(name) ) {
			Attribute *tmp = new Attribute(name, newValue, tolerance, strict);
			addAttributeToBuffer(tmp, permanent);
		}
	}

	void AttributeBuffer::add(const String& name, const Vector<Bool>& newValue,
	                          const Bool strict, const Bool permanent) {
		// check if an attribute with the same name exist
		if ( !exists(name) ) {
			Attribute *tmp = new Attribute(name, newValue, strict);
			addAttributeToBuffer(tmp, permanent);

		}
	}

	void AttributeBuffer::add(const String& name, const Vector<String>& newValue,
	                          const Bool strict, const Bool permanent) {
		// check if an attribute with the same name exist
		if ( !exists(name) ) {
			Attribute *tmp = new Attribute(name, newValue, strict);
			addAttributeToBuffer(tmp, permanent);
		}
	}

	void AttributeBuffer::add(const String& name, const Vector<Quantity>& newValue,
	                          const Bool strict, const Bool permanent) {
		// check if an attribute with the same name exists
		if (!exists(name)) {
			Attribute *tmp = new Attribute(name, newValue, strict);
			addAttributeToBuffer(tmp, permanent);
		}
	}

	void AttributeBuffer::add(const String& name, uInt *newValue,
	                          const uInt tolerance, const Bool strict,
	                          const Bool permanent) {
		// check if an attribute with the same name exist
		if ( !exists(name) ) {
			Attribute *tmp = new Attribute(name, newValue, tolerance, strict);
			addAttributeToBuffer(tmp, permanent);
		}
	}
	void AttributeBuffer::add(const String& name, Int *newValue,
	                          const Int tolerance, const Bool strict,
	                          const Bool permanent) {
		// check if an attribute with the same name exist
		if ( !exists(name) ) {
			Attribute *tmp = new Attribute(name, newValue, tolerance, strict);
			addAttributeToBuffer(tmp, permanent);
		}
	}

	void AttributeBuffer::add(const String& name, Float *newValue,
	                          const Float tolerance, const Bool strict,
	                          const Bool permanent) {
		// check if an attribute with the same name exist
		if ( !exists(name) ) {
			Attribute *tmp = new Attribute(name, newValue, tolerance, strict);
			addAttributeToBuffer(tmp, permanent);
		}
	}

	void AttributeBuffer::add(const String& name, Double  *newValue,
	                          const Double tolerance, const Bool strict,
	                          const Bool permanent) {
		// check if an attribute with the same name exist
		if ( !exists(name) ) {
			Attribute *tmp = new Attribute(name, newValue, tolerance, strict);
			addAttributeToBuffer(tmp, permanent);
		}
	}

	void AttributeBuffer::add(const String& name, Bool *newValue,
	                          const Bool strict, const Bool permanent) {
		// check if an attribute with the same name exist
		if ( !exists(name) ) {
			Attribute *tmp = new Attribute(name, newValue, strict);
			addAttributeToBuffer(tmp, permanent);
		}
	}

	void AttributeBuffer::add(const String& name,String *newValue,
	                          const Bool strict, const Bool permanent) {
		// check if an attribute with the same name exist
		if ( !exists(name) ) {
			Attribute *tmp = new Attribute(name, newValue, strict);
			addAttributeToBuffer(tmp, permanent);
		}
	}

	void AttributeBuffer::add(const String& name, Quantity *newValue,
	                          const Bool strict, const Bool permanent) {
		// check if an attribute with the same name exists
		if (!exists(name)) {
			Attribute *tmp = new Attribute(name, newValue, strict);
			addAttributeToBuffer(tmp, permanent);
		}
	}



	void AttributeBuffer::add(const String& name, Vector<uInt>* newValue,
	                          const uInt tolerance, const Bool strict,
	                          const Bool permanent) {
		// check if an attribute with the same name exist
		if ( !exists(name) ) {
			Attribute *tmp = new Attribute(name, newValue, tolerance, strict);
			addAttributeToBuffer(tmp, permanent);
		}
	}
	void AttributeBuffer::add(const String& name, Vector<Int>* newValue,
	                          const Int tolerance, const Bool strict,
	                          const Bool permanent) {
		// check if an attribute with the same name exist
		if ( !exists(name) ) {
			Attribute *tmp = new Attribute(name, newValue, tolerance, strict);
			addAttributeToBuffer(tmp, permanent);
		}
	}

	void AttributeBuffer::add(const String& name, Vector<Float>* newValue,
	                          const Float tolerance, const Bool strict,
	                          const Bool permanent) {
		// check if an attribute with the same name exist
		if ( !exists(name) ) {
			Attribute *tmp = new Attribute(name, newValue, tolerance, strict);
			addAttributeToBuffer(tmp, permanent);
		}
	}

	void AttributeBuffer::add(const String& name, Vector<Double>* newValue,
	                          const Double tolerance, const Bool strict,
	                          const Bool permanent) {
		// check if an attribute with the same name exist
		if ( !exists(name) ) {
			Attribute *tmp = new Attribute(name, newValue, tolerance, strict);
			addAttributeToBuffer(tmp, permanent);
		}
	}

	void AttributeBuffer::add(const String& name, Vector<Bool>* newValue,
	                          const Bool strict, const Bool permanent) {
		// check if an attribute with the same name exist
		if ( !exists(name) ) {
			Attribute *tmp = new Attribute(name, newValue, strict);
			addAttributeToBuffer(tmp, permanent);
		}
	}

	void AttributeBuffer::add(const String& name, Vector<String>* newValue,
	                          const Bool strict, const Bool permanent) {
		// check if an attribute with the same name exist
		if ( !exists(name) ) {
			Attribute *tmp = new Attribute(name, newValue, strict);
			addAttributeToBuffer(tmp, permanent);
		}
	}

	void AttributeBuffer::add(const String& name, Vector<Quantity>* newValue,
	                          const Bool strict, const Bool permanent) {
		// only add attribute if one with the same name doesn't already exist
		if (!exists(name)) {
			Attribute *tmp = new Attribute(name, newValue, strict);
			addAttributeToBuffer(tmp, permanent);
		}
	}


	void AttributeBuffer::set(const Attribute& newAttribute) {
		Int index;

		// check if an attribute with the same name exist
		if ( exists(newAttribute.getName(), index) ) {
			attributes[index]->setValue(newAttribute);
		} else {
			add(newAttribute, False);
		}

	}


	void AttributeBuffer::set(const String& name, const uInt newValue,
	                          const uInt tolerance, const Bool strict) {
		Vector<uInt> tmp(1);
		tmp = newValue;
		set(name, tmp, tolerance, strict);
	}

	void AttributeBuffer::set(const String& name, const Int newValue,
	                          const Int tolerance, const Bool strict) {
		Vector<Int> tmp(1);
		tmp = newValue;
		set(name, tmp, tolerance, strict);
	}



	void AttributeBuffer::set(const String& name, const Bool newValue,
	                          const Bool strict) {
		Vector<Bool> tmp(1);
		tmp = newValue;
		set(name, tmp, strict);
	}



	void AttributeBuffer::set(const String& name, const Float newValue,
	                          const Float tolerance, const Bool strict) {
		Vector<Float> tmp(1);
		tmp = newValue;
		set(name, tmp, tolerance, strict);
	}


	void AttributeBuffer::set(const String& name, const Double newValue,
	                          const Double tolerance, const Bool strict) {
		Vector<Double> tmp(1);
		tmp = newValue;
		set(name, tmp, tolerance, strict);
	}



	void AttributeBuffer::set(const String& name, const String& newValue,
	                          const Bool strict) {
		Vector<String> tmp(1);
		tmp = newValue;
		set(name, tmp, strict);
	}

	void AttributeBuffer::set(const String& name, const Quantity newValue,
	                          const Bool strict) {
		Vector<Quantity> tmp(1);
		tmp = newValue;
		set(name, tmp, strict);
	}


	void AttributeBuffer::set(const String& name, const Vector<Int>& newValue,
	                          const Int tolerance, const Bool strict) {
		// check if an attribute with the same name exist
		if ( exists(name) ) {
			if (getDataType(name) == AttValue::AtInt) {
				AttributeValueTol<Int> *value_p =
				    dynamic_cast<AttributeValueTol<Int> *>(getAttributeValue(name));
				value_p->setValue(newValue);
				value_p->setTolerance(tolerance);
				value_p->setStrictness(strict);
			} else {
				// will trow an exception
			}
		} else {
			add(name, newValue, tolerance, strict, False);
		}
	}

	void AttributeBuffer::set(const String& name, const Vector<uInt>& newValue,
	                          const uInt tolerance, const Bool strict) {
		// check if an attribute with the same name exist
		if ( exists(name) ) {
			if (getDataType(name) == AttValue::AtuInt) {
				AttributeValueTol<uInt> *value_p =
				    dynamic_cast<AttributeValueTol<uInt> *>(getAttributeValue(name));
				value_p->setValue(newValue);
				value_p->setTolerance(tolerance);
				value_p->setStrictness(strict);
			} else {
				// will trow an exception
			}
		} else {
			add(name, newValue, tolerance, strict, False);
		}
	}


	void AttributeBuffer::set(const String& name, const Vector<Float>& newValue,
	                          const Float tolerance, const Bool strict) {
		// check if an attribute with the same name exist
		if ( exists(name) ) {
			if (getDataType(name) == AttValue::AtFloat) {
				AttributeValueTol<Float> *value_p =
				    dynamic_cast<AttributeValueTol<Float> *>(getAttributeValue(name));
				value_p->setValue(newValue);
				value_p->setTolerance(tolerance);
				value_p->setStrictness(strict);
			}
		} else {
			add(name, newValue, tolerance, strict, False);
		}
	}


	void AttributeBuffer::set(const String& name, const Vector<Double>& newValue,
	                          const Double tolerance, const Bool strict) {
		// check if an attribute with the same name exist
		if ( exists(name) ) {
			if (getDataType(name) == AttValue::AtDouble) {
				AttributeValueTol<Double> *value_p =
				    dynamic_cast<AttributeValueTol<Double> *>(getAttributeValue(name));
				value_p->setValue(newValue);
				value_p->setTolerance(tolerance);
				value_p->setStrictness(strict);
			}
		} else {
			add(name, newValue, tolerance, strict, False);
		}
	}


	void AttributeBuffer::set(const String& name, const Vector<Bool>& newValue,
	                          const Bool strict) {
		// check if an attribute with the same name exist
		if ( exists(name) ) {
			if (getDataType(name) == AttValue::AtBool) {
				AttributeValue<Bool> *value_p =
				    dynamic_cast<AttributeValue<Bool> *>(getAttributeValue(name));
				value_p->setValue(newValue);
				value_p->setStrictness(strict);
			}
		} else {
			add(name, newValue, strict, False);
		}
	}


	void AttributeBuffer::set(const String& name, const Vector<String>& newValue,
	                          const Bool strict) {
		// check if an attribute with the same name exist
		if ( exists(name) ) {
			if (getDataType(name) == AttValue::AtString) {
				AttributeValue<String> *value_p =
				    dynamic_cast<AttributeValue<String> *>(getAttributeValue(name));
				value_p->setValue(newValue);
				value_p->setStrictness(strict);
			}
		} else {
			add(name, newValue, strict, False);
		}
	}

	void AttributeBuffer::set(const String& name, const Vector<Quantity>& newValue,
	                          const Bool strict) {
		// check if an attribute with the same name exists
		if (exists(name)) {
			if (getDataType(name) == AttValue::AtQuantity) {
				AttributeValue<Quantity> *value_p =
				    dynamic_cast<AttributeValue<Quantity> *>(getAttributeValue(name));
				value_p->setValue(newValue);
				value_p->setStrictness(strict);
			}
		} else {
			add(name, newValue, strict, False);
		}
	}



	AttributeValueBase* AttributeBuffer::getAttributeValue(const String& name) const {
		for (uInt i = 0; i < attributes.nelements(); i++) {
			if ( name == (attributes[i])->getName() ) {
				return (attributes[i])->getAttributeValue();
			}
		}
		// should throw exception
		return 0;
	}


	Attribute* AttributeBuffer::getAttribute(const String& name) const {
		for (uInt i = 0; i < attributes.nelements(); i++) {
			if ( name == (attributes[i])->getName() ) {
				return attributes[i];
			}
		}
		// should throw exception?
		return 0;
	}



	void AttributeBuffer::remove(const String& name) {
		for (uInt i = 0; i < attributes.nelements(); i++) {
			if ( name == (attributes[i])->getName() ) {
				// first delete
				if (!nonDeletable[i]) {
					delete attributes[i];
					// then remove, not the other way around...
					attributes.remove(i);
					nonDeletable.remove(i);
					// There can be only one attribute of the correct name, so can exit
				}
				return;
			}
		}
	}


// Clear the the non-permanent attributes from the buffer.

	void AttributeBuffer::clear() {


		uInt i = 0;
		vector<uInt> indicesToDelete;
		while (i < attributes.nelements() ) {
			if (!nonDeletable[i]) {
				indicesToDelete.push_back(i);
				// do not increment i!!!!
			/*} else {
				i++;
			}*/
			}
			i++;
		}

		//Reverse order so we don't screw up the indices.
		for ( int j = static_cast<int>(indicesToDelete.size() - 1); j >= 0; j-- ){
			int deleteIndex = indicesToDelete[j];
			delete attributes[deleteIndex];
			attributes.remove(deleteIndex);
			nonDeletable.remove( deleteIndex);
		}
	}

// Clear the whole buffer, permanent attributes also.

	void AttributeBuffer::erase() {
		while (attributes.nelements() > 0 ) {
			delete attributes[0];
			// then remove, not the other way around...
			attributes.remove(0);
			nonDeletable.remove(0);
		}
	}



	Bool AttributeBuffer::exists(const String& name) const {
		for (uInt i = 0; i < attributes.nelements(); i++) {
			if ( name == (attributes[i])->getName() ) {
				return True;
			}
		}
		return False;
	}


	Bool AttributeBuffer::exists(const String& name, Int& found) const {
		for (uInt i = 0; i < attributes.nelements(); i++) {
			if ( name == (attributes[i])->getName() ) {
				found = i;
				return True;
			}
		}
		found = -1;
		return False;
	}



	AttValue::ValueType AttributeBuffer::getDataType(const String& name) const {
		for (uInt i = 0; i < attributes.nelements(); i++) {
			if ( name == (attributes[i])->getName() ) {
				return attributes[i]->getType();
			}
		}
		return AttValue::AtInvalid;
	}




	void AttributeBuffer::addAttributeToBuffer(Attribute *newAttribute,
	        const Bool permanent) {
		attributes.resize(attributes.nelements()+1);
		attributes[attributes.nelements()-1] = newAttribute;
		nonDeletable.resize(nonDeletable.nelements()+1);
		nonDeletable[nonDeletable.nelements()-1] = permanent;
	}

	Bool AttributeBuffer::matches(const Attribute& res) const {

		for (uInt i = 0; i < attributes.nelements(); i++) {
			if  ( ( *(attributes[i]) ) != res )  {
				return False;
			}
		}
		return True;
	}



	Bool AttributeBuffer::matches(const AttributeBuffer& resBuf) const {
		for (uInt i = 0; i < attributes.nelements(); i++) {
			if  ( !resBuf.matches(*(attributes[i])) ) {
				return False;
			}
		}
		return True;
	}

// AttributeBuffer addition arithmetic.
	void AttributeBuffer::operator+=(const AttributeBuffer &other) {
		String name;
		for (uInt i = 0; i < attributes.nelements(); i++) {
			name = (attributes[i])->getName();
			Attribute *att = other.getAttribute(name);
			if (att) {
				*(attributes[i]) += *att;
			}
		}
		return;
	}


	ostream &operator<<(ostream &os, AttributeBuffer &ab) {
		os<<"{";
		for (uInt i = 0; i < ab.attributes.nelements(); i++) os<<(*(ab.attributes[i]));
		os<<"}";
		return os;
	}


} //# NAMESPACE CASA - END

