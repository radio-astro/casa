//# AttVal.h: type-dependent interface for values of Attributes
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

#ifndef TRIALDISPLAY_ATTVAL_H
#define TRIALDISPLAY_ATTVAL_H

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <display/Display/AttValBase.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Type-dependent interface for values of Attributes.
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="tAttribute" demos="">
// </reviewed>

// <prerequisite>
// <li> AttributeValueBase
// </prerequisite>

// <etymology>
// An <em>AttributeValue</em> stores the value of an Attribute
// </etymology>

// <synopsis>
// An Attribute in the Display Library has a name and a value.  In
// order to facilite easy use of Attributes in user code, the value of
// an Attribute has to be wrapped in a templated class, the
// AttributeValue, which itself is derived from a non-templated base
// class, <linkto class=AttributeValueBase> AttributeValueBase
// </linkto>.  What is stored in the AttributeValue is a value and the
// type of that value.  Only some types are supported; see
// AttributeValueBase for a list. The value is always stored as a
// Vector, even if the value used in the constructor is a single
// value.  The operation needed for the AttributeValue is to be able
// to check whether it matches another AttributeValue.  For two
// AttributeValues to match they must have the same value and be of
// the same type (with some tolerance, see below).  AttributeValues of
// different types (through the base class AttributeValueBase), never
// match.
//
// The parameter <src>strict</src> in some constructors defines
// whether matching has to be done for each element (<src> strict ==
// True </src>), or whether AttributeValues match if any one element
// of one Vector is equal to any other element of the other Vector.
// An AttributeValue created with a scalar type can match an
// AttributeValue created with a Vector of that scalar type.
// </synopsis>
//
// <example>
// A few simple examples of the use of the AttributeValue class follow.
// <srcblock>
// AttributeValue<Int> intAtt1(3, False);
// AttributeValue<Int> intAtt2(3, False);
// AttributeValue<Int> intAtt3(2, False);
// </srcblock>
//
// At this point, <src>intAtt1==intAtt2</src> will return
// <src>True</src>, and <src>intAtt1==intAtt3</src> will return
// <src>False</src>.
//
// <srcblock>
// Vector<Int> vec(2);
// vec(0) = 1;
// vec(1) = 3;
// AttributeValue<Vector<Int> > vecAtt1(vec, False);
// </srcblock>
//
// and now <src>vecAtt1==intAtt1</src> is <src>True</src>, and
// <src>vecAtt1==intAtt3</src> returns <src>False</src>.
//
// Finally,
// <srcblock>
// AttributeValue<Vector<Int> > vecAtt2(vec, True);
// </srcblock>

// gives <src>False</src> for <src>vecAtt2==intAtt1</src>, since
// they cannot match element wise because they have different lengths,
// and similarly <src>vecAtt2==intAtt2</src> is also <src>False</src>.
// </example>

// <motivation>
// and <linkto class=Attribute> Attribute </linkto>.
// </motivation>
//
// <todo asof="1999/12/09">
// Nothing known.
// </todo>

	template <class T> class AttributeValue : public AttributeValueBase {

	public:

		// Construct from a scalar value. The parameter <src>strict</src>
		// defines whether whether matching has to be done for each element
		// (<src>strict == True</src>) (a scalar value AttributeValue is
		// considered a Vector of length one), or whether AttributeValues
		// match if any one element of one Vector is equal to any other
		// element of the other Vector (<src>strict == False</src>).
		AttributeValue(const T &value, const Bool strict);

		// Construct from a <src>Vector</src>.  The parameter
		// <src>strict</src> defines whether whether matching has to be done
		// for each element (<src>strict == True</src>), or whether
		// AttributeValues match if any one element of one Vector is equal
		// to any other element of the other Vector (<src>strict == False</src>).
		AttributeValue(const Vector<T> &value, const Bool strict);

		// Destructor.
		virtual ~AttributeValue();

		// Copy constructor.
		AttributeValue(const AttributeValue<T> &other);

		// Assignment (copy semantics)
		const AttributeValue<T>& operator=(const AttributeValue<T> &other);

		// Set/get the value of the AttributeValue.
		// <group>
		virtual void setValue(const T &value);
		virtual void setValue(const Vector<T> &value);
		virtual Vector<T> getValue() const {
			return itsValue;
		};
		// </group>

		// Returns a new copy of the AttributeValue
		virtual AttributeValueBase* clone() const;

		// Add <src>other</src> to <src>*this</src>.
		virtual void operator+=(const AttributeValueBase& other);

		// Return class name
		virtual String className() const {
			return String("AttributeValue");
		};

		virtual void print(ostream& os) {
			os<<itsValue;
		}

	protected:
		// Implements when the values of two Attributes match or not.  The
		// state of <src>strict</src> determines whether whether matching
		// has to be done for each element (<src> strict == True </src>), or
		// whether AttributeValues match if any one element of one Vector is
		// equal to any other element of the other Vector.
		virtual Bool matches(const AttributeValueBase& other) const;

		// Cast from Base class
		const AttributeValue<T>& myCast (const AttributeValueBase& other) const;


	private:
		// The attribute value
		Vector<T> itsValue;

		// Sett T type in base class
		void setType();

		// Do actual matching
		Bool myMatch(const AttributeValue<T>& other) const;

		// Default constructor
		AttributeValue();
	};

} //# NAMESPACE CASA - END

#ifndef AIPS_NO_TEMPLATE_SRC
#include <display/Display/AttVal.tcc>
#endif

#endif
