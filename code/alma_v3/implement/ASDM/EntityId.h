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
 * File EntityId.h
 */

#ifndef EntityId_CLASS
#define EntityId_CLASS

#include <iostream>
#include <string>
using namespace std;

#ifndef WITHOUT_ACS
#include <asdmIDLTypesC.h>
using asdmIDLTypes::IDLEntityId;
#endif

#include <StringTokenizer.h>
#include <InvalidArgumentException.h>
using asdm::StringTokenizer;
using asdm::InvalidArgumentException;

#include "EndianStream.h"
using asdm::EndianOSStream;
using asdm::EndianIStream;

namespace asdm {

class EntityId;
ostream & operator << ( ostream &, const EntityId & );
istream & operator >> ( istream &, EntityId&);

/**
 * description
 * 
 * @version 1.00 Jan. 7, 2005
 * @author Allen Farris
 * @author Michel Caillat, added == and =! operators. June 2005.
 * 
 */
class EntityId {
    friend ostream & operator << ( ostream &, const EntityId & );
	friend istream & operator >> ( istream &, EntityId&);

public:
	static EntityId getEntityId(StringTokenizer &t) 
		throw (InvalidArgumentException) ;
	static string validate(string x);

	EntityId();
	EntityId(const EntityId &);
	EntityId(const string &id) throw (InvalidArgumentException);
#ifndef WITHOUT_ACS
	EntityId(IDLEntityId &) throw (InvalidArgumentException);
#endif
	virtual ~EntityId();

	EntityId& operator = (const EntityId &);

	bool equals(const EntityId &) const;
	bool operator == (const EntityId&) const;
	bool operator != (const EntityId&) const;	

	string toString() const;
#ifndef WITHOUT_ACS
	IDLEntityId toIDLEntityId() const;
#endif

	/**
	 * Write the binary representation of this to a EndianOSStream.
	 */		
	void toBin(EndianOSStream& eoss) const;
	
	/**
	 * Read the binary representation of an EntityId from a EndianIStream
	 * and use the read value to set an  EntityId.
	 * @param eis the EndianStream to be read
	 * @return an EntityId
	 * @throw InvalidArgumentException
	 */
	static EntityId fromBin(EndianIStream& eis);
	
	bool isNull() const;

	string getId() const;
	void setId(string e);

private:
	string id;

};

// EntityId constructors
inline EntityId::EntityId() : id("") { }

inline EntityId::EntityId(const EntityId &t) : id(t.id) { }

// EntityId destructor
inline EntityId::~EntityId() { }

inline EntityId& EntityId::operator = ( const EntityId &x ) {
	id = x.id;
	return *this;
}

inline bool EntityId::isNull() const {
	return id.length() == 0;	
}

inline bool EntityId::equals(const EntityId &x) const {
	return id == x.id;
}

inline bool EntityId::operator == (const EntityId &x) const {
	return id == x.id;	
}

inline bool EntityId::operator != (const EntityId &x) const {
	return id != x.id;	
}

inline string EntityId::toString() const {
	return id;
}

#ifndef WITHOUT_ACS
inline IDLEntityId EntityId::toIDLEntityId() const {
	IDLEntityId x;
	x.value = CORBA::string_dup(id.c_str());
	return x;
}
#endif

inline string EntityId::getId() const {
	return id;
}

inline void EntityId::setId(string s) {
	id = s;
}

// Friend functions

inline ostream & operator << ( ostream &o, const EntityId &x ) {
	o << x.id;
	return o;
}

inline istream & operator >> ( istream &i, EntityId &x ) {
	i >> x.id;
	return i;
}

} // End namespace asdm

#endif /* EntityId_CLASS */
