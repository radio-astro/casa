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
 * File Entity.h
 */

#ifndef Entity_CLASS
#define Entity_CLASS

#include <iostream>
#include <string>
using namespace std;

#ifndef WITHOUT_ACS
#include <asdmIDLTypesC.h>
using asdmIDLTypes::IDLEntity;
#endif

#include <EntityId.h>

#include <StringTokenizer.h>
#include <InvalidArgumentException.h>
#include <InvalidDataException.h>
using asdm::StringTokenizer;
using asdm::InvalidArgumentException;
using asdm::InvalidDataException;

#include "EndianStream.h"
using asdm::EndianOSStream;
using asdm::EndianISStream;

namespace asdm {

class Entity;
ostream & operator << ( ostream &, const Entity & );

/**
 * The Entity class is an identification of a persistant
 * entity in the ALMA archive.  It easily maps onto an EntityT
 * object in ACS system entities.
 * 
 * @version 1.00 Jan. 7, 2005
 * @author Allen Farris
 */
class Entity {
    friend ostream & operator << ( ostream &, const Entity & );
	friend istream & operator >> ( istream &, Entity&);

public:
	static Entity getEntity(StringTokenizer &t) throw(InvalidArgumentException);

	Entity();
	Entity(const string &s);
#ifndef WITHOUT_ACS
	Entity(IDLEntity &);
#endif
	Entity(string entityId, string entityIdEncrypted, string entityTypeName,
			string entityVersion, string instanceVersion);
	virtual ~Entity();

	bool operator == (const Entity &) const;
	bool equals(const Entity &) const;
	bool operator != (const Entity &) const;

	bool isNull() const;

	string toString() const;
	string toXML() const throw(InvalidDataException);
#ifndef WITHOUT_ACS
	IDLEntity toIDLEntity() const;
#endif
	void setFromXML(string xml) throw(InvalidArgumentException);
	
	/**
	 * Write the binary representation of this to a EndianOSStream.
	 */		
	void toBin(EndianOSStream& eoss);
	
	/**
	 * Read the binary representation of an Enity from a EndianISStream
	 * and use the read value to set an  Entity.
	 * @param eiss the EndianStream to be read
	 * @return an Entity
	 */
	static Entity fromBin(EndianISStream& eiss);
	
	EntityId getEntityId() const;
	string getEntityIdEncrypted() const;
	string getEntityTypeName() const;
	string getEntityVersion() const;
	string getInstanceVersion() const;
	void setEntityId(EntityId e);
	void setEntityIdEncrypted(string s);
	void setEntityTypeName(string s);
	void setEntityVersion(string s);
	void setInstanceVersion(string s);

private:
	EntityId entityId;
	string entityIdEncrypted;
	string entityTypeName;
	string entityVersion;
	string instanceVersion;

	string getXMLValue(string xml, string parm) const;
	string validXML() const;

};

// Entity constructors
inline Entity::Entity(const string &s) {
	setFromXML(s);
}

// Entity destructor
inline Entity::~Entity() { }

inline bool Entity::isNull() const {
	return entityId.isNull();
}

inline string Entity::toString() const {
	return toXML();
}

inline bool Entity::equals(const Entity &x) const {
	return *this == x;
}

// Getters and Setters

inline EntityId Entity::getEntityId() const {
	return entityId;
}

inline string Entity::getEntityIdEncrypted() const {
	return entityIdEncrypted;
}

inline string Entity::getEntityTypeName() const {
	return entityTypeName;
}

inline string Entity::getEntityVersion() const {
	return entityVersion;
}

inline string Entity::getInstanceVersion() const {
	return instanceVersion;
}

inline void Entity::setEntityId(EntityId e) {
	entityId = e;
}

inline void Entity::setEntityIdEncrypted(string s) {
	entityIdEncrypted = s;
}

inline void Entity::setEntityTypeName(string s) {
	entityTypeName = s;
}

inline void Entity::setEntityVersion(string s) {
	entityVersion = s;
}

inline void Entity::setInstanceVersion(string s) {
	instanceVersion = s;
}

// Friend functions

inline ostream & operator << ( ostream &o, const Entity &x ) {
	o << x.toXML();
	return o;
}

} // End namespace asdm

#endif /* Entity_CLASS */
