//# AttValPoiTol.h: template for tolerant and aliased AttributeValues
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

#ifndef TRIALDISPLAY_ATTVALPOITOL_H
#define TRIALDISPLAY_ATTVALPOITOL_H

#include <casa/aips.h>
#include <display/Display/AttValTol.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Templated class for aliased, tolerant AttributeValues.
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="tAttribute" demos="">
// </reviewed>

// <prerequisite>
// <li> <linkto class="AttributeValue">AttributeValue</linkto>
// <li> <linkto class="AttributeValuePoi">AttributeValuePoi</linkto>
// <li> <linkto class="AttributeValueTol">AttributeValueTol</linkto>
// </prerequisite>

// <etymology>
// "AttributeValuePoiTol" is a contraction of "Attribute Value",
// "Pointer" and "Tolerance", and stores an Attribute by aliasing
// it with a pointer and associating it with some tolerance.
// </etymology>

// <synopsis>
// An AttributeValuePoiTol combines the aliasing (via a pointer)
// provided by the <linkto class="AttributeValuePoi">
// AttributeValuePoi </linkto> class, and the tolerance provided by
// the <linkto class="AttributeValueTol"> AttributeValueTol </linkto>
// class.  See these classes for examples.
// </synopsis>

// <motivation>
// The motivation for this class is to be able to provide in-built
// tolerance and aliasing for AttributeValues.
// </motivation>

// <todo asof="2000/01/17">
// </todo>

	template <class T> class AttributeValuePoiTol : public AttributeValueTol<T> {

	public:

		// Constructor for a pointer to a scalar, and a tolerance.
		AttributeValuePoiTol(T* value, const T &tolerance, const Bool strict);

		// Constructor for a pointer to a <src>Vector</src>, and a scalar
		// tolerance.
		AttributeValuePoiTol(Vector<T>* value, const T &tolerance,
		                     const Bool strict);

		// Copy constructor.
		AttributeValuePoiTol(const AttributeValuePoiTol<T> &other);

		// Destructor.
		virtual ~AttributeValuePoiTol();

		// Assignment operator.
		const AttributeValuePoiTol<T>& operator=(const AttributeValuePoiTol<T> &other);

		// Return a new copy of the AttributeValuePoiTol (virtual constructor).
		virtual AttributeValueBase *clone() const;

		// Add <src>other</src> to <src>*this</src>.  Needs to over-ride
		// base class definition because the pointers need to be
		// dereferenced prior to addition.
		virtual void operator+=(const AttributeValueBase &other);


		// Change the value of the AttributeValue.
		// <group>
		virtual void setValue(const T& value);
		virtual void setValue(const Vector<T>& value);
		// </group>

		// Get the DataType of the aliased variable.
		virtual DataType getPointerType() const;

		// Return class name
		virtual String className() const {
			return String("AttributeValuePoiTol");
		};

	private:

		// Update the variable that is aliased to the AttributeValuePoiTol.
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

		// Cast from Base class
		const AttributeValuePoiTol<T>& myCast (const AttributeValueBase& other) const;

		// Default constructor
		AttributeValuePoiTol();


		//# Make parent members known.
	public:
		using AttributeValueTol<T>::getType;
		using AttributeValueTol<T>::getValue;
		using AttributeValueTol<T>::getStrictness;
	};


} //# NAMESPACE CASA - END
#ifndef AIPS_NO_TEMPLATE_SRC
#include <display/Display/AttValPoiTol.tcc>
#endif
#endif
