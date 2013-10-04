//# AttValPoi.h: templated class for aliased AttributeValues
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

#ifndef TRIALDISPLAY_ATTVALPOI_H
#define TRIALDISPLAY_ATTVALPOI_H

#include <casa/aips.h>
#include <display/Display/AttValBase.h>
#include <display/Display/AttVal.h>
#include <casa/Utilities/DataType.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Templated class for storing a pointer to the value of an Attribute.
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="tAttribute" demos="">
// </reviewed>

// <prerequisite>
// <li> <linkto class="AttributeValue">AttributeValue</linkto>
// </prerequisite>

// <etymology>
// "AttributeValuePoi" is a contraction of "Attribute Value" and
// "Pointer", and stores a pointer to the value of an Attribute.
// </etymology>

// <synopsis>
// An AttributeValuePoi differs from a <linkto class="AttributeValue">
// AttributeValue</linkto> in that instead of using its own variable
// to store the actual value like AttributeValue does, it stores a
// pointer to the variable used in the constructor.  This means that
// the AttributeValuePoi is just an alias to this variable, and then
// the user can modify the value of an AttributeValuePoi without using
// its interface.  Alternatively, and perhaps more importantly, the
// Attribute interface can be used to modify members of a class.
// </synopsis>

// <example>
// A simple example will help illustrate the utility of the AttributeValuePoi
// class:
//
// <srcBlock>
// Int varInt = 1;
// AttributeValuePoi<Int> intAtt(&varInt, False);
//
// Vector<Int> bla = intAtt.getValue();
// // bla(0) is 1;
//
// // This wil change also the AttributeValue:
// varInt = 2;
// bla = intAtt.getValue();
// // bla(0) is now 2
//
// intAtt.setValue(5);
// // now also varInt == 5
//
// </srcBlock>
// </example>

// <motivation>
// The motivation for this class is to be able to provide an Attribute
// interface for modifying private members of some classes.  For
// example, this interface is used to control the linear coordinate
// system of the <linkto class="WorldCanvas">WorldCanvas</linkto>.
// </motivation>

// <todo asof="2000/01/17">
// Nothing known.
// </todo>

	template <class T> class AttributeValuePoi : public AttributeValue<T> {

	public:

		// Constructor for a pointer to a scalar.
		AttributeValuePoi(T* value, const Bool strict);

		// Constructor for a pointer to a <src>Vector</src>.
		AttributeValuePoi(Vector<T> *value, const Bool strict);

		// Copy constructor.
		AttributeValuePoi(const AttributeValuePoi<T> &other);

		// Destructor.
		virtual ~AttributeValuePoi();

		// Assignment operator.
		const AttributeValuePoi<T>& operator=(const AttributeValuePoi<T> &other);

		// Return a new copy of the AttributeValuePoi (virtual constructor).
		virtual AttributeValueBase *clone() const;

		// Add <src>other</src> to <src>*this</src>.  Needs to over-ride
		// base class definition because the pointers need to be
		// dereferenced prior to addition.
		virtual void operator+=(const AttributeValueBase& other);

		// Change the value of the AttributeValue.
		// <group>
		virtual void setValue(const T &value);
		virtual void setValue(const Vector<T> &value);
		// </group>

		// Get the DataType of aliased variable.
		virtual DataType getPointerType() const;

		// Return class name
		virtual String className() const {
			return String("AttributeValuePoi");
		};


	private:

		// Update the variable that is aliased to the AttributeValuePoi.
		void updatePointerValue() const;

		// Pointer to the aliased variable. Only one is ever active depending
		// upon how the object was constructed.  The memory allocated to these pointers
		// does not belong to this object.
		// <group>
		Vector<T>* itsVectorPointerPtr;
		T* itsScalarPointerPtr;
		// </group>

		// The pointer DataType
		DataType itsPointerType;

		// Cast from base class
		const AttributeValuePoi<T>& myCast (const AttributeValueBase& other) const;

		// Default constructor
		AttributeValuePoi();

		//# Make parent members known.
	public:
		using AttributeValue<T>::getType;
		using AttributeValue<T>::getValue;
	};


} //# NAMESPACE CASA - END
#ifndef AIPS_NO_TEMPLATE_SRC
#include <display/Display/AttValPoi.tcc>
#endif //# AIPS_NO_TEMPLATE_SRC

#endif
