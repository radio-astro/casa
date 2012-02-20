/*
 * ALMA - Atacama Large Millimeter Array
 * (c) European Southern Observatory, 2002
 * (c) Associated Universities Inc., 2002
 * Copyright by ESO (in the framework of the ALMA collaboration),
 * Copyright by AUI (in the framework of the ALMA collaboration),
 * All rights reserved.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307  USA
 *
 * File EntityRef.h
 */

#ifndef EntityRef_CLASS
#define EntityRef_CLASS

#include <iostream>
#include <string>
#include <vector>
using namespace std;

#include <EntityId.h>
#include <PartId.h>

#ifndef WITHOUT_ACS
#include <asdmIDLTypesC.h>
using asdmIDLTypes::IDLEntityRef;
#endif

#include <StringTokenizer.h>
#include <InvalidArgumentException.h>
#include <InvalidDataException.h>
using asdm::StringTokenizer;
using asdm::InvalidArgumentException;
using asdm::InvalidDataException;

namespace asdm {

class EntityRef;
ostream & operator << ( ostream &, const EntityRef & );

/**
 * The EntityRef class is an identification of a persistant
 * entity in the ALMA archive.  It easily maps onto an EntityRefT
 * object in ACS system entities.
 * 
 * @version 1.00 Jan. 7, 2005
 * @author Allen Farris
 */
class EntityRef {
    friend ostream & operator << ( ostream &, const EntityRef & );

public:
	static EntityRef getEntityRef(StringTokenizer &t) throw(InvalidArgumentException);

	EntityRef();
	EntityRef(const string &s);
#ifndef WITHOUT_ACS
	EntityRef(IDLEntityRef &);
#endif
	EntityRef(string entityId, string partId, string entityTypeName,
			string instanceVersion);
	virtual ~EntityRef();

	bool operator == (const EntityRef &) const;
	bool equals(const EntityRef &) const;
	bool operator != (const EntityRef &) const;

	bool isNull() const;

	string toString() const;
	string toXML() const throw(InvalidDataException);
#ifndef WITHOUT_ACS
	IDLEntityRef toIDLEntityRef() const;
#endif
	void setFromXML(string xml) ;
	
   /**
	 * Write the binary representation of this to a EndianOSStream.
	 */
	void toBin(EndianOSStream& eoss) const ;

   /**
    * Write the binary representation of a vector of EntityRef to an EndianOSStream.
    */
	static void toBin(const vector<EntityRef>& entityRef,  EndianOSStream& eoss);
	 
   /**
	 * Read the binary representation of an EntityRef from a EndianIStream
	 * and use the read value to set an  EntityRef.
	 * @param eis the EndianStream to be read
	 * @return an EntityRef
	 */
	static EntityRef fromBin(EndianIStream& eis);
	/**
	 * Read the binary representation of  a vector of  EntityRef from an EndianIStream
	 * and use the read value to set a vector of  EntityRef.
	 * @param dis the EndianIStream to be read
	 * @return a vector of EntityRef
	 */
	static vector<EntityRef> from1DBin(EndianIStream & eis);

	EntityId getEntityId() const;
	PartId getPartId() const;
	string getEntityTypeName() const;
	string getInstanceVersion() const;
	void setEntityId(EntityId e);
	void setPartId(PartId s);
	void setEntityTypeName(string s);
	void setInstanceVersion(string s);

private:
	EntityId entityId;
	PartId partId;
	string entityTypeName;
	string instanceVersion;

	string getXMLValue(string xml, string parm) const;
	string validXML() const;

};

// EntityRef constructors
inline EntityRef::EntityRef(const string &s) {
	setFromXML(s);
}

// EntityRef destructor
inline EntityRef::~EntityRef() { }

inline bool EntityRef::isNull() const {
	return entityId.isNull();
}

inline string EntityRef::toString() const {
	return toXML();
}

inline bool EntityRef::equals(const EntityRef &x) const {
	return *this == x;
}

// Getters and Setters

inline EntityId EntityRef::getEntityId() const {
	return entityId;
}

inline PartId EntityRef::getPartId() const {
	return partId;
}

inline string EntityRef::getEntityTypeName() const {
	return entityTypeName;
}

inline string EntityRef::getInstanceVersion() const {
	return instanceVersion;
}

inline void EntityRef::setEntityId(EntityId e) {
	entityId = e;
}

inline void EntityRef::setPartId(PartId p) {
	partId = p;
}

inline void EntityRef::setEntityTypeName(string s) {
	entityTypeName = s;
}

inline void EntityRef::setInstanceVersion(string s) {
	instanceVersion = s;
}

// Friend functions

inline ostream & operator << ( ostream &o, const EntityRef &x ) {
	o << x.toXML();
	return o;
}

} // End namespace asdm

#endif /* EntityRef_CLASS */
