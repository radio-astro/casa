//# AttributeBuffer.h: Buffer to store Attributes
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

#ifndef TRIALDISPLAY_ATTRIBUTEBUFFER_H
#define TRIALDISPLAY_ATTRIBUTEBUFFER_H

#include <casa/aips.h>
#include <casa/Containers/Block.h>
#include <casa/Arrays/Vector.h>
#include <display/Display/AttValBase.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	class String;
	class AttValBase;
	class Attribute;


// <summary> Buffer for storing Attributes </summary>
// <use visibility=local>
//
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
//
// <prerequisite>
//   <li> Attributes
// </prerequisite>
//
// <etymology>
//  <em>Buffer</em> for storing <em>Attributes</em>
// </etymology>
//
// <synopsis>
//
// A number of classes in the Display Library have Attributes (ie. 'things'
// that have a name and a value). An AttributeBuffer can be used to store
// these Attributes internally and do some simple operations, like checking if
// a certain Attribute exists etc. An AttributeBuffer has a 'fat' interface
// for creating Attributes and getting the value of them that hides most of
// the type issues and should make it less work for a programmer to handle
// Attributes.
//
// AttributeBuffers can be used for a number of things. First, of course, they
// can be used to store and organize information. For example, a class can
// store its state parameters in an AttributeBuffer for its own
// convenience. By creating a userinterface to its AttributeBuffer, a class
// can create a standard interface to its state. One possibility here is that
// by using Attributes that are constructed using a pointer (see
// AttributeValuePoi or AttributeValuePoiTol) and by defining a userinterface
// in the class to modify the Attributes of that class, one automatically
// creates a <em>direct</em> userinterface to (some of) the internal variables
// of that class. This is used for example in the WorldCanvas. Another point
// is that Attributes can be defined with arbitrary name and on several types,
// so if a class has a userinterface to its AttributeBuffer, a programmer who
// uses that class can define what information is stored in the class at run
// time. For example, if one can register an event handler in a class A, one
// can store information in class A using the Attribute interface of class A.
// The event handler knows where to look for the information (namely in the
// object of class A it is registered in) so it can find it, while the
// class  A does not have to know what kind (name, type,...) information
// is needed by the event handler, but it can still store it.
//
// Another use of AttributeBuffer is that one can use it to store the state of
// something. Later in the program the new state can be compared with the old
// state using the <src>matches</src> member function of AttributeBuffer and
// the appropriate action can be taken. This is for example used in the
// ImageDDImage to decide whether a cache should be flushed or
// not. AttributeBuffers match if all their Attributes match. For the logic of
// matching Attributes, see Attributes. An example may make this clearer:
//
// <srcblock>
//
// // We have an AttributeBuffer to store the old state:
// AttributeBuffer oldState;
// // and store some state information
// oldState.add("stateVar1", state1);
// oldState.add("stateVar2", state2);
// oldState.add("stateVar3", state3);
//        .
//        .
//        .
// // later in the program we want to check if the current state matches the
// // old state
//
// // AttributeBuffer for the new state
// AttributeBuffer newState;
// // and store state (state1, state2 or state3 may have changed)
// newState.add("stateVar1", state1);
// newState.add("stateVar2", state2);
// newState.add("stateVar3", state3);
//
// // now the new an old state can be compared
// if { newState.matches(oldState) }
//   use my cache
// } else {
//   delete the cache and make new one
// }
// </srcblock>
//
// A similar application of AttributeBuffers is the way they are used by the
// WorldCanvasHolder and the DisplayData (and Animator) to define what data is
// displayed on a canvas. To define what is displayed, one sets a number of
// Attributes on the WorldCanvasHolder (where they are called restrictions and
// they are stored in an AttributeBuffer) and possibly a number of Attributes
// on the DisplayData (where they are also called restrictions and also stored
// in an AttributeBuffer). Some DisplayData have already pre-defined
// restrictions (like the ImageDisplayData), but the program can add to these
// at run time. When a refresh of the WorldCanvas happens, only those data are
// displayed for which the buffer of the DisplayData matches that of the
// WorldCanvasHolder. This gives a very easy to use way of defining what is
// displayed, and this is how for example the Animator works (one simply
// changes some restrictions and do a refresh). See Animator for examples.
//
//
// AttributeBuffers can also be used for passing information in a compact and
// generic way. This is used eg. by the WorldCanvasHolder in the sizeControl
// event handling. The WorldCanvasHolder does not have to know what
// information is passed and eg. what type it is. The code there looks like:
//
// <srcblock>
// Bool WorldCanvasHolder::sizeControlEH(WorldCanvas *wCanvas)
//
//   AttributeBuffer sizeControlAtts;
//
//    // rewind the List of DisplayDatas registered withe this WorldCanvasHolder
//    displayListIter->toStart();
//
//    // temp
//    DisplayData *dData;
//
//    // loop over DisplayDatas that are registered
//    while ( !displayListIter->atEnd() ) {
//      // get DisplayData from List and let this displaydata do its sizeControl
//      dData = (DisplayData *) displayListIter->getRight();
//      if ( !dData->sizeControl(*this, sizeControlAtts)) {
//        // something is very wrong so abort refresh
//        return False;
//      }
//      // next DisplayData
//      displayListIter++;
//    }
//
//    // copy the Attributes set by the DisplayData to the WorldCanvas
//    // We don't know what is being set, but we can still set it....
//    wCanvas->setAttributes(sizeControlAtts);
//
//    // things went ok
//    return True;
//  }
// </srcblock>
// </synopsis>
//
// <example>
// <srcBlock>
// </srcBlock>
// </example>
//
// <motivation>
//
// For efficient handling of all the Attributes that a class in the Display
// Library can have, a buffer is needed, with an interface that makes handling
// of Attributes relatively easy.
//
// </motivation>
//


	class AttributeBuffer {

	public:

		// Constructor of empty buffer
		AttributeBuffer();

		// Copy constructor. Copy semantics.
		AttributeBuffer(const AttributeBuffer& other);

		// Assignement operator
		const AttributeBuffer& operator=(const AttributeBuffer& other);

		// Destructor
		~AttributeBuffer();

		// Return number of Attributes in the buffer
		Int  nelements() const;

		// Define  new Attributes. If an Attribute of the same name exists, nothing
		// happens. If <src>permanent == True</src>, the Attribute cannot be deleted
		// from the Buffer.
		// <group>
		void add(const AttributeBuffer& otherBuf);
		void add(const Attribute& newAttribute, const Bool permanent = False);
		//</group>

		// Add new Attributes. For type uInt, Int, Float and Double, the Attribute
		// has tolerance (see AttributeValueTol), for Bool and String it has not
		// (obviously). <src>strict</src> defines how Attribute match. See
		// AttributeValue for the explanation of <src>strict</src> <group>
		void add(const String& name, const uInt newValue,
		         const uInt tolerance = 0, const Bool strict = False,
		         const Bool permanent = False);

		void add(const String& name, const Int newValue,
		         const Int tolerance = 0, const Bool strict = False,
		         const Bool permanent = False);

		void add(const String& name, const Float newValue,
		         const Float tolerance = 0.0, const Bool strict = False,
		         const Bool permanent = False);

		void add(const String& name, const Double newValue,
		         const Double tolerance = 0.0, const Bool strict = False,
		         const Bool permanent = False);

		void add(const String& name, const Bool newValue,
		         const Bool strict = False, const Bool permanent = False);

		void add(const String& name, const String& newValue,
		         const Bool strict = False, const Bool permanent = False);

		void add(const String& name, const Quantity newValue,
		         const Bool strict = False, const Bool permanent = False);


		void add(const String& name, const Vector<uInt>& newValue,
		         const uInt tolerance = 0, const Bool strict = False,
		         const Bool permanent = False);

		void add(const String& name, const Vector<Int>& newValue,
		         const Int tolerance = 0, const Bool strict = False,
		         const Bool permanent = False);

		void add(const String& name, const Vector<Float>& newValue,
		         const Float tolerance = 0.0, const Bool strict = False,
		         const Bool permanent = False);

		void add(const String& name, const Vector<Double>& newValue,
		         const Double tolerance = 0.0, const Bool strict = False,
		         const Bool permanent = False);

		void add(const String& name, const Vector<Bool>& newValue,
		         const Bool strict = False, const Bool permanent = False);

		void add(const String& name, const Vector<String>& newValue,
		         const Bool strict = False, const Bool permanent = False);

		void add(const String& name, const Vector<Quantity>& newValue,
		         const Bool strict = False, const Bool permanent = False);

		// </group>

		// Add new Attributes. These are the pointer versions. Using these members
		// will create Attributes based on AttributeValueTol or
		// AttributeValuePoiTol. This means that if the Attribute is modified, the
		// variable used to define the Attribute also changes and vice versa.
		// <group>
		void add(const String& name,  uInt *newValue,
		         const uInt tolerance = 0, const Bool strict = False,
		         const Bool permanent = False);

		void add(const String& name,  Int *newValue,
		         const Int tolerance = 0, const Bool strict = False,
		         const Bool permanent = False);

		void add(const String& name, Float *newValue,
		         const Float tolerance = 0.0, const Bool strict = False,
		         const Bool permanent = False);

		void add(const String& name, Double  *newValue,
		         const Double tolerance = 0.0, const Bool strict = False,
		         const Bool permanent = False);

		void add(const String& name, Bool *newValue,
		         const Bool strict = False, const Bool permanent = False);

		void add(const String& name, String *newValue,
		         const Bool strict = False, const Bool permanent = False);

		void add(const String& name, Quantity *newValue,
		         const Bool strict = False, const Bool permanent = False);


		void add(const String& name, Vector<uInt> *newValue,
		         const uInt tolerance = 0, const Bool strict = False,
		         const Bool permanent = False);

		void add(const String& name, Vector<Int> *newValue,
		         const Int tolerance = 0, const Bool strict = False,
		         const Bool permanent = False);

		void add(const String& name,  Vector<Float> *newValue,
		         const Float tolerance = 0.0, const Bool strict = False,
		         const Bool permanent = False);

		void add(const String& name,  Vector<Double> *newValue,
		         const Double tolerance = 0.0, const Bool strict = False,
		         const Bool permanent = False);

		void add(const String& name,  Vector<Bool> *newValue,
		         const Bool strict = False, const Bool permanent = False);

		void add(const String& name, Vector<String> *newValue,
		         const Bool strict = False, const Bool permanent = False);

		void add(const String& name, Vector<Quantity> *newValue,
		         const Bool strict = False, const Bool permanent = False);

		// </group>


		// Set the value of an Attribute. If the Attribute does not exist, it is
		// created (using the corresponding add function with permanent set to
		// False), otherwise the value of the existing Attribute, if it is of the
		// correct type) will be changed. If the Attribute has a different type than
		// the variable used in the set function, nothing happens (but I may change
		// my mind and  throw an exception).
		// <group>
		void set(const AttributeBuffer& otherBuf);
		void set(const Attribute& newAttribute);

		void set(const String& name, const uInt newValue,
		         const uInt tolerance = 0, const Bool strict = False);

		void set(const String& name, const Int newValue,
		         const Int tolerance = 0, const Bool strict = False);

		void set(const String& name, const Float newValue,
		         const Float tolerance = 0.0, const Bool strict = False);

		void set(const String& name, const Double newValue,
		         const Double tolerance = 0.0, const Bool strict = False);

		void set(const String& name, const Bool newValue,
		         const Bool strict = False);

		void set(const String& name, const String& newValue,
		         const Bool strict = False);

		void set(const String& name, const Quantity newValue,
		         const Bool strict = False);


		void set(const String& name, const Vector<uInt>& newValue,
		         const uInt tolerance = 0, const Bool strict = False);

		void set(const String& name, const Vector<Int>& newValue,
		         const Int tolerance = 0, const Bool strict = False);

		void set(const String& name, const Vector<Float>& newValue,
		         const Float tolerance = 0.0, const Bool strict = False);

		void set(const String& name, const Vector<Double>& newValue,
		         const Double tolerance = 0.0, const Bool strict = False);

		void set(const String& name, const Vector<Bool>& newValue,
		         const Bool strict = False);

		void set(const String& name, const Vector<String>& newValue,
		         const Bool strict = False);

		void set(const String& name, const Vector<Quantity>& newValue,
		         const Bool strict = False);

		// </group>

		// Get tha value of the named Attribute.  Returns <src>True</src> for
		// success, and <src>False</src> for failure.  This can happen if the
		// caller asked for the wrong type.
		template <class T> Bool getValue(const String &name, Vector<T> &value) const;
		template <class T> Bool getValue(const String &name, T &value) const;

		// Get the pointer to the Attribute if it exists, else get 0
		Attribute *getAttribute(const String& name) const;

		// Get pointer to the AttributeValue if it exists, else get 0
		AttributeValueBase *getAttributeValue(const String& name) const;

		// Get the data type of the Attribute
		AttValue::ValueType getDataType(const String& name) const;

		// Function to see if Attribute res matches any Attribute in the
		// AttributeBuffer *this.
		Bool matches(const Attribute& res) const;

		// Function to see if  every Attribute in the AttributeBuffer resBuf
		// matches every Attribute in the AttributeBuffer *this. Returns
		// True if this is the case, returns False if for at least one Attribute
		// in resBuf there is a mismatch.
		Bool matches(const AttributeBuffer& resBuf) const;

		// AttributeBuffer addition arithmetic.  Go through <src>*this</src>
		// buffer, and for those Attributes who have equivalents in
		// <src>other</src>, sum the values.
		void operator+=(const AttributeBuffer &other);

		// Remove Attributes from the AttributeBuffer. Only works on Attributes that
		// are not permanent
		// <group>
		void remove(const String& name);
		void clear();
		// </group>

		// Check if an Attribute with name name exists
		Bool exists(const String& name) const;

		// Add the Attributes of *this to other
		void addBuff(AttributeBuffer& other) const;

		// Set the Attributes of *this to other
		void setBuff(AttributeBuffer& other) const;

	private:

		friend ostream &operator<<(ostream &, AttributeBuffer &);

		// PtrBlock for the Attributes. Should change this to a list
		PtrBlock<Attribute *> attributes;

		// Store if an Attribute is permamnent or not
		Block<Bool> nonDeletable;

		// Internal routine to add an Attribute to the Buffer
		void addAttributeToBuffer(Attribute *newAttribute, const Bool permanent);

		// Check if an Attribute exists and return the index in the PtrBlock
		Bool exists(const String& name, Int& found) const;

		// Remove Attributes from the AttributeBuffer. Also erases  Attributes that
		// are permanent. Only used in operator=.
		void erase();

	};

	ostream &operator<<(ostream &os, AttributeBuffer &ab);


} //# NAMESPACE CASA - END

#ifndef AIPS_NO_TEMPLATE_SRC
#include <display/Display/AttBufferTemplates.tcc>
#endif

#endif
