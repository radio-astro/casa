//# Attribute.h: arbitrary name-value pairs used in the display classes
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

#ifndef TRIALDISPLAY_ATTRIBUTE_H
#define TRIALDISPLAY_ATTRIBUTE_H

#include <casa/aips.h>
#include <display/Display/AttValBase.h>

namespace casacore{

	class String;
}

namespace casa { //# NAMESPACE CASA - BEGIN


// <summary>
// Arbitrary name-value pairs used in the display classes.
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="tAttribute" demos="">
// </reviewed>

// <prerequisite>
// <li> <linkto class="AttributeValueBase">AttributeValueBase</linkto>
// <li> <linkto class="AttributeValue">AttributeValue</linkto>
// <li> <linkto class="AttributeValueTol">AttributeValueTol</linkto>
// </prerequisite>

// <etymology>
// An Attribute characterises something by associating a value with a
// name.
// </etymology>

// <synopsis>
// An Attribute is the combination of a name and a value.  The name is
// a casacore::String, while the value can be of any of the types casacore::Int, casacore::Float,
// casacore::Double, casacore::Bool, casacore::String or <linkto class=casacore::Quantum>Quantum</linkto>,
// or a <linkto class=casacore::Vector>Vector</linkto> of any of these types.
//
// Attributes can be compared to see if they match.  They can also be
// made "fuzzy" by providing a user-specified tolerance, and then
// matching means <src>abs(val1-val2) <= tol</src>.  An Attribute can
// also be "strict" if required: if so, then Attributes with Vector
// values are deemed to match if their values match element-wise.
// Otherwise, the Attributes match if any one element in one
// Attribute's value matches any one element in the other Attribute's
// value.  In the latter case, the Attribute casacore::Vector values do not have
// to be conformant.
// </synopsis>
//
// <example>
// The following example shows the construction and retrieval of an
// Attribute.
// <srcblock>
// Attribute att("axisname", "Right ascension (J2000)");
// ...
//
// AttributeValue<casacore::String>* pAv = dynamic_cast<AttributeValue<casacore::String>*>(att.getAttributeValue());
// casacore::String axisname = pAv->getValue()(0);
// if (axisname == "Right ascension (J2000)") {
//   ...
// } else {
//   throw(casacore::AipsError("Doing nothing because axisname Attribute unsuitable"));
// }
// </srcblock>
//
// The following example uses the Attribute equality operator to
// determine some state.
// <srcblock>
// /* "itsAxisName" is some private casacore::String */
// Attribute att("axisname", itsAxisName);
// ...
// Attribute currentAtt("axisname", "casacore::Stokes");
// if (currentAtt == att) {
//   ...
// } else {
//   ...
// }
// </srcblock>
//
// A more complete example, exhibiting the Attribute interface to a
// specific display class, the <linkto
// class=WorldCanvas>WorldCanvas</linkto>, follows.
// <srcblock>
// /* assume we are working on a WorldCanvas "wcanvas" */
// /* at some point, an Attribute has been set elsewhere like this:
//    wcanvas.setAttribute("ColorModel", "RGB");
// */
// ...
// /* somewhere later in the program ... */
// if (wcanvas.existsAttribute("ColorModel")) {
//   casacore::String colormodel;
//   wcanvas.getAttribute("ColorModel", colormodel);
//   if (colormodel == "RGB") {
//     ...
//   } else {
//     ...
//   }
// } else {
//   ...
// }
// </srcblock>
// </example>

// <motivation>
// The main motivation for introducing Attributes is to be able to
// attach arbitrary name-value pairs to various objects in the display
// classes.  This allows the storage, retrieval and comparison of
// arbitrary information from the various display objects.
// </motivation>

// <todo asof="2000/01/17">
// <li> Re-write <src>setValue</src> function.
// </todo>

	class Attribute {

	public:

		// Constructor taking an AttributeValueBase.  By inheriting from
		// AttributeValueBase, additional types of Attributes can be
		// supported.
		Attribute(const casacore::String &name, const AttributeValueBase& value);

		// Copy constructor.
		Attribute(const Attribute& other);

		// Constructors that take a single value.  The value stored in an
		// Attribute can be made fuzzy by giving some tolerance
		// <src>tol</src>, and when the values can be compared to see if
		// they match, the tolerance is taken into account where it makes
		// sense.  The parameter <src>strict</src> determines how the
		// matching is done for the values.  If <src>strict == true</src>,
		// the values have to match elementwise (a single value is
		// considered a casacore::Vector of length 1).  If <src>strict == false</src>,
		// the values match if any one element in one value matches any one
		// elemnt in the other value.
		// <group>
		Attribute(const casacore::String &name, const casacore::uInt value,
		          const casacore::uInt tolerance = 0, const casacore::Bool strict = false);
		Attribute(const casacore::String &name, const casacore::Int value,
		          const casacore::Int tolerance = 0, const casacore::Bool strict = false);
		Attribute(const casacore::String &name, const casacore::Float value,
		          const casacore::Float tolerance = 0.0, const casacore::Bool strict = false);
		Attribute(const casacore::String &name, const casacore::Double value,
		          const casacore::Double tolerance = 0.0, const casacore::Bool strict = false);
		Attribute(const casacore::String &name, const casacore::Quantity value,
		          const casacore::Bool strict = false);
		Attribute(const casacore::String &name, const casacore::Quantity value,
		          const casacore::Quantity tolerance, casacore::Bool strict = false);
		Attribute(const casacore::String &name, const casacore::Bool value,
		          const casacore::Bool strict = false);
		Attribute(const casacore::String &name, const casacore::String value,
		          const casacore::Bool strict = false);
		// </group>

		// Contructors that take a <src>casacore::Vector</src> of the various
		// types. Once again, the value stored in an Attribute can be made
		// fuzzy by specifying <src>tol</src> or strict by putting
		// <src>strict = true</src>.
		// <group>
		Attribute(const casacore::String &name, const casacore::Vector<casacore::uInt> value,
		          const casacore::uInt tolerance = 0, const casacore::Bool strict = false);
		Attribute(const casacore::String &name, const casacore::Vector<casacore::Int> value,
		          const casacore::Int tolerance = 0, const casacore::Bool strict = false);
		Attribute(const casacore::String &name, const casacore::Vector<casacore::Float> value,
		          const casacore::Float tolerance = 0.0, const casacore::Bool strict = false);
		Attribute(const casacore::String &name, const casacore::Vector<casacore::Double> value,
		          const casacore::Double tolerance = 0.0, const casacore::Bool strict = false);
		Attribute(const casacore::String &name, const casacore::Vector<casacore::Quantity> value,
		          const casacore::Bool strict = false);
		Attribute(const casacore::String &name, const casacore::Vector<casacore::Quantity> value,
		          const casacore::Quantity tolerance, const casacore::Bool strict = false);
		Attribute(const casacore::String &name, const casacore::Vector<casacore::Bool> value,
		          const casacore::Bool strict = false);
		Attribute(const casacore::String &name, const casacore::Vector<casacore::String> value,
		          const casacore::Bool strict = false);
		//</group>

		// Constructors that take a pointer to a variable. This makes the
		// Attribute an alias for the variable, changing the variable
		// changes the value of the Attribute and vice versa.  Other
		// functionality the same as for the other constructors.
// <group>
		Attribute(const casacore::String &name, casacore::uInt *value,
		          const casacore::uInt tolerance = 0, const casacore::Bool strict = false);
		Attribute(const casacore::String &name, casacore::Int *value,
		          const casacore::Int tolerance = 0, const casacore::Bool strict = false);
		Attribute(const casacore::String &name, casacore::Float *value,
		          const casacore::Float tolerance = 0.0, const casacore::Bool strict = false);
		Attribute(const casacore::String &name, casacore::Double  *value,
		          const casacore::Double tolerance = 0.0, const casacore::Bool strict = false);
		Attribute(const casacore::String &name, casacore::Bool *value,
		          const casacore::Bool strict = false);
		Attribute(const casacore::String &name, casacore::String *value,
		          const casacore::Bool strict = false);
		Attribute(const casacore::String &name, casacore::Quantity *value,
		          const casacore::Bool strict = false);
		Attribute(const casacore::String &name, casacore::Quantity *value,
		          const casacore::Quantity tolerance, const casacore::Bool strict = false);
		// </group>

		// Constructors that take a pointer to a Vector.  This again makes
		// the Attribute an alias for the casacore::Vector: changing the Vector
		// changes the value of the Attribute, and vice versa.  Other
		// functionality is the same as for the non-aliasing constructors.
		// <group>
		Attribute(const casacore::String &name,  casacore::Vector<casacore::uInt> *value,
		          const casacore::uInt tolerance = 0, const casacore::Bool strict = false);
		Attribute(const casacore::String &name,  casacore::Vector<casacore::Int> *value,
		          const casacore::Int tolerance = 0, const casacore::Bool strict = false);
		Attribute(const casacore::String &name,  casacore::Vector<casacore::Float> *value,
		          const casacore::Float tolerance = 0.0, const casacore::Bool strict = false);
		Attribute(const casacore::String &name,  casacore::Vector<casacore::Double>  *value,
		          const casacore::Double tolerance = 0.0, const casacore::Bool strict = false);
		Attribute(const casacore::String &name, casacore::Vector<casacore::Quantity> *value,
		          const casacore::Bool strict = false);
		Attribute(const casacore::String &name, casacore::Vector<casacore::Quantity> *value,
		          const casacore::Quantity tolerance, const casacore::Bool strict = false);
		Attribute(const casacore::String &name,  casacore::Vector<casacore::Bool> *value,
		          const casacore::Bool strict = false);
		Attribute(const casacore::String &name, casacore::Vector<casacore::String> *value,
		          const casacore::Bool strict = false);
		//</group>

		// Destructor.
		virtual ~Attribute();

		// Create a new copy of the Attribute and return a pointer to the
		// copy (virtual constructor).
		virtual Attribute* clone() const;

		// Attribute matching: returns <src>true</src> if <src>*this</src>
		// and <src>other</src> match, otherwise returns <src>false</src>.
		// <note role=caution>Two Attributes match if their names are
		// different.  If two Attributes have the same name, but the values
		// stored in the Attributes have different types, they do not match.
		// An Attribute of a single value and an Attribute of a casacore::Vector of
		// values of the same type are considered to have the same type, so
		// under certain circumstances (based on the strictness of the
		// Attributes) two such Attributes may match.</note>
		virtual casacore::Bool operator==(const Attribute &other) const;

		// The opposite of the Attribute matching.
		casacore::Bool operator!=(const Attribute &other) const;

		// Attribute addition ... add the value of <src>other</src> to
		// the value of <src>*this</src>.
		virtual void operator+=(const Attribute &other);

		// Set the value of this Attribute to that of the <src>other</src>
		// Attribute, if, and only if, they have the same value type.  <note
		// role=caution> This method makes an important assumption, namely
		// that all AttributeValues that return AttValue::AtInt,
		// AttValue::AtFloat or AttValue::AtDouble are derived from
		// AttributeValueTol and that return AttValue::AtBool,
		// AttValue::AtString or AttValue::AtQuantity are derived from
		// AttributeValue (ie they are like they are created by
		// Attribute). This is ok for <src>*this</src>, but there is no
		// guarantee that it is correct for <src>other</src>.  This
		// assumption was not needed until the AttributeValuePoi class was
		// written.  AttributeValues were not supposed to change but were
		// supposed to be replaced.  Now this is very ugly and should be
		// rewritten.  Indeed, it fails now for Quantities, since both with
		// and without tols are used at the moment. </note>
		virtual void setValue(const Attribute &other);

		// Returns a pointer to the AttributeValue base class object.  You must cast
		// it to AttributeValue<T> and invoke function <src>getValue</src> to
		// get the value of the attribute
		AttributeValueBase* getAttributeValue() const;

		// Return the name of the Attribute.
		casacore::String getName() const;

		// Return the DataType of the value of the Attribute.
		AttValue::ValueType getType() const;

	private:

		// The name of the Attribute.
		casacore::String itsAttributeName;

		// Pointer to the attribute base class
		AttributeValueBase* itsAttributeValue;

		// (Required) default constructor.
		Attribute();

		// Copy assignment.  <note role=caution>This allows the type of the
		// AttributeValue to change, so it is private.</note>
		virtual const Attribute &operator=(const Attribute &other);

	};

	std::ostream &operator<<(std::ostream &os, Attribute &a);


} //# NAMESPACE CASA - END

#endif

