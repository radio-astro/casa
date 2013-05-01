//# AttValTol.h: templated class for tolerant AttributeValues
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

#ifndef TRIALDISPLAY_ATTVALTOL_H
#define TRIALDISPLAY_ATTVALTOL_H

#include <casa/aips.h>
#include <display/Display/AttVal.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class AttValBase;

// <summary> 
// Templated class for storing Attributes which have a tolerance.
// </summary>

// <use visibility=export>
 
// <reviewed reviewer="" date="yyyy/mm/dd" tests="tAttribute" demos="">
// </reviewed>
 
// <prerequisite>
// <li> <linkto class="AttributeValue">AttributeValue</linkto>
// </prerequisite>

// <etymology> 
// "AttributeValueTol" is a contraction of "Attribute Value" and 
// "Tolerance", and stores an Attribute which has associated with 
// it some tolerance to be observed in matching with other 
// Attributes.
// </etymology>

// <synopsis>
// An AttributeValueTol differs from a <linkto class="AttributeValue">
// AttributeValue</linkto> in that it has associated with it a certain
// tolerance, which will be observed when matching the value with
// other AttributeValues.  This means that values do not have to match
// exactly in order for the AttributeValueTols to match.  Tolerant
// matching is defined as <src>abs(val1-val2) <= tolerance1</src>.
// Remember though that in general both Attributes must independently
// match each other for a match to exist, and so it is normally also a 
// requirement that <src>abs(val1 - val2) <= tolerance2</src> when the
// second Attribute also has tolerance specified.
// </synopsis>

// <example>
// If we have 
//
// <srcBlock>
// AttributeValueTol<Float> Att1(3.0, 1.2, False);
// AttributeValueTol<Float> Att2(2.0, 1.1, False);
// AttributeValueTol<Float> Att3(2.1, 0.5, False);
// </srcBlock>
//
// then:
// <li> <src>Att1==Att2</src> returns <src>True</src>,
// <li> <src>Att2==Att3</src> returns <src>True</src>,
// <li> but <src>Att3==Att1</src> returns <src>False</src>.
// </example>
//
// <motivation>
// The motivation for this class is to be able to provide some
// in-built tolerance ("fuzziness") to AttributeValues.  For example,
// this approach might be used in selecting channel maps to display
// when matching on radial velocity.
// </motivation>
// <todo asof="2000/01/17">
// </todo>


template <class T> class AttributeValueTol : public AttributeValue<T> {

 public:
  // Constructor for a scalar, with specified scalar tolerance and
  // strictness.
  AttributeValueTol(const T &value, const T &tolerance, const Bool strict);
  
  // Constructor for a <src>Vector</src> value, with specified scalar
  // tolerance and strictness.
  AttributeValueTol(const Vector<T> &value, const T &tolerance, const Bool strict);

  // Copy contructor.
  AttributeValueTol(const AttributeValueTol<T> &other);
  
  // Destructor.
  virtual ~AttributeValueTol();

  // Assignment operator.
  const AttributeValueTol<T> &operator=(const AttributeValueTol<T>& other);

  // Return a new copy of the AttributeValueTol (virtual constructor).
  virtual AttributeValueBase *clone() const;

  // Change or retrieve the scalar tolerance.
  // <group>
  virtual void setTolerance(T value) {itsValueTolerance = value;};
  virtual T getTolerance() const {return itsValueTolerance;};
  // </group>

  // Return class name 
  virtual String className() const {return String("AttributeValueTol");};

protected:

  // Implements when the values of two Attributes match, taking note 
  // of tolerance in this particular implementation.
  virtual Bool matches(const AttributeValueBase& other) const;

 private:

  // The value of the tolerance
  T itsValueTolerance;

  // Does the actual matching
  Bool myMatch(const AttributeValue<T>& other) const;

  // Default constructor
  AttributeValueTol();

  //# Make parent members known.
public:
  using AttributeValue<T>::getType;
  using AttributeValue<T>::getValue;
  using AttributeValue<T>::getStrictness;
};

} //# NAMESPACE CASA - END

#ifndef AIPS_NO_TEMPLATE_SRC
#include <display/Display/AttValTol.tcc>
#endif //# AIPS_NO_TEMPLATE_SRC

#endif
