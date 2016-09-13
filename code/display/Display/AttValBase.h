//# AttValBase.h: abstract base class and support class for AttributeValues
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

#ifndef TRIALDISPLAY_ATTVALBASE_H
#define TRIALDISPLAY_ATTVALBASE_H

#include <casa/aips.h>
#include <casa/Utilities/DataType.h>
#include <casa/Arrays/Vector.h>
#include <casa/Quanta/Quantum.h>
#include <casa/iostream.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Provision of type identification services for Attribute classes.
// </summary>
//
// <use visibility=local>
//
// <reviewed reviewer="" date="yyyy/mm/dd" tests="tAttribute" demos="">
// </reviewed>
//
// <etymology>
// "AttValue" is a contraction of "Attribute Value"
// </etymology>
//
// <synopsis>
// AttValue provides type identification services for use in the
// Attribute classes.
// </synopsis>
//
// <example>
// AttValue can be used to provide an enumeration which identifies
// the type of a variable, for example:
// <srcblock>
// casacore::Int i;
// if (AttValue::whatType(&i) != AttValue::AtInt) {
//   throw(casacore::AipsError(casacore::String("Incorrect type identification in AttValue")));
// }
// </srcblock>
// </example>
//
// <motivation>
// This is a support class to unify type identification for the
// various Attribute classes.
// </motivation>
//
// <todo asof="2000/01/17">
// <li> Consider using global whatType function instead of this class.
// </todo>

	class AttValue {

	public:

		// The possible value types.
		enum ValueType {
		    AtuInt,
		    AtInt,
		    AtFloat,
		    AtDouble,
		    AtBool,
		    AtString,
		    AtQuantity,
		    AtInvalid
		};

		// Determine the type of a scalar or casacore::Array variable.
		// <group>
		static AttValue::ValueType whatType(casacore::uInt *) {
			return AttValue::AtuInt;
		};
		static AttValue::ValueType whatType(casacore::Vector<casacore::uInt> *) {
			return AttValue::AtuInt;
		};
		static AttValue::ValueType whatType(casacore::Int *) {
			return AttValue::AtInt;
		};
		static AttValue::ValueType whatType(casacore::Vector<casacore::Int> *) {
			return AttValue::AtInt;
		};
		static AttValue::ValueType whatType(casacore::Float *) {
			return AttValue::AtFloat;
		};
		static AttValue::ValueType whatType(casacore::Vector<casacore::Float> *) {
			return AttValue::AtFloat;
		};
		static AttValue::ValueType whatType(casacore::Double *) {
			return AttValue::AtDouble;
		};
		static AttValue::ValueType whatType(casacore::Vector<casacore::Double> *) {
			return AttValue::AtDouble;
		};
		static AttValue::ValueType whatType(casacore::Bool *) {
			return AttValue::AtBool;
		};
		static AttValue::ValueType whatType(casacore::Vector<casacore::Bool> *) {
			return AttValue::AtBool;
		};
		static AttValue::ValueType whatType(casacore::String *) {
			return AttValue::AtString;
		};
		static AttValue::ValueType whatType(casacore::Vector<casacore::String> *) {
			return AttValue::AtString;
		};
		static AttValue::ValueType whatType(casacore::Quantity *) {
			return AttValue::AtQuantity;
		};
		static AttValue::ValueType whatType(casacore::Vector<casacore::Quantity> *) {
			return AttValue::AtQuantity;
		};
		static AttValue::ValueType whatType(void *) {
			return AttValue::AtInvalid;
		};
		// </group>

	};

// <summary>
// Base class for values of Attributes used in the display classes.
// </summary>
//
// <use visibility=export>
//
// <reviewed reviewer="" date="yyyy/mm/dd" tests="tAttribute" demos="">
// </reviewed>
//
// <etymology>
// "AttributeValueBase" is a concatenation, representing a "Base"
// class for "Attribute Values."
// </etymology>
//
// <synopsis>
// This class is the base for storing Attribute values.  <linkto
// class=Attribute> Attributes </linkto> are simple name-value pairs,
// where the name is a casacore::String, and the value can be a scalar or Array.
//
// This class defines the type-independent interface for Attribute
// values, and the type-dependent interface and implementation is
// provided in <linkto class=AttributeValue> AttributeValue </linkto>.
// This type independent interface allows comparison of Attributes
// of different types.
// </synopsis>
//
// <motivation>
// To provide the non-templated (ie. type-independent) interface of
// AttributeValues in a single place, thus enabling the hiding of
// the templated aspect of Attributes from the end-user.  In particular
// it allows implementation of the comparison operators in the base
// class, regardless of type.
// </motivation>
//
// <todo asof="1999/12/09">
// Nothing known.
// </todo>

	class AttributeValueBase  {

	public:

		// Constructor.
		AttributeValueBase(AttValue::ValueType type, casacore::Bool strict);

		// Copy constructor.
		AttributeValueBase(const AttributeValueBase &other);

		// Destructor.
		virtual ~AttributeValueBase();

		// Copy assignment.
		const AttributeValueBase& operator=(const AttributeValueBase &other);

		// Get the type of the value stored.
		AttValue::ValueType getType() const;

		// Check for equality (and inequality) of two objects derived from
		// AttributeValueBase.  It is implemented in terms of the pure
		// virtual method <src>matches</src>, which must be implemented in
		// derived classes.  The <src>operator==</src> only returns
		// <src>true</src> if <src>this->matches(other)</src> and
		// <src>other.matches(*this)</src> are both <src>true</src>.  This
		// guarantees that if <src> a == b </src> it follows that <src> b ==
		// a </src> (this is enforced this way because <src>a</src> and
		// <src>b</src> can be classes derived differently from
		// AttributeValueBase which can therefore have a different
		// implementation of <src>match()</src>).
		// <group>
		casacore::Bool operator==(const AttributeValueBase &other) const;
		casacore::Bool operator!=(const AttributeValueBase &other) const;
		// </group>

		// Return a new copy of the AttributeValueBase
		virtual AttributeValueBase* clone() const = 0;

		// Set/get the strictness state of this AttributeValue.
		// <group>
		virtual void setStrictness(const casacore::Bool &newStrict);
		virtual casacore::Bool getStrictness() const;
		// </group>

		// Add <src>other</src> to <src>*this</src>.
		virtual void operator+=(const AttributeValueBase& other) = 0;

		// Return class name
		virtual casacore::String className() const {
			return casacore::String("AttributeValueBase");
		};

		virtual void print(std::ostream& os) = 0;

	protected:

		// The type of what is stored.
		AttValue::ValueType itsValueType;

		// Whether the match is strict or not.
		casacore::Bool itsStrictness;

		// Calculate whether <src>*this</src> matches <src>other</src>.
		// Since the meaning of "match" can be different for different
		// types, it is left to the derived class to define this method.
		virtual casacore::Bool matches(const AttributeValueBase &other) const = 0;

		// Set the type of the value stored.
		virtual void setType(const AttValue::ValueType &newType);

		// Check that private data match
		casacore::Bool myMatch(const AttributeValueBase &other) const;
	};

	std::ostream &operator<<(std::ostream &os, AttributeValueBase &av);


} //# NAMESPACE CASA - END

#endif
