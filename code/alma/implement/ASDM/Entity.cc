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
 * File Entity.cpp
 */

#include <Entity.h>
#include <OutOfBoundsException.h>
#include <InvalidArgumentException.h>
#include <InvalidDataException.h>
using asdm::OutOfBoundsException;
using asdm::InvalidArgumentException;
using asdm::InvalidDataException;

namespace asdm {

	Entity Entity::getEntity(StringTokenizer &t) 
		throw (InvalidArgumentException) {
		try {
			string s = t.nextToken("<>");
			if (s == " ")
				s = t.nextToken();
			Entity e;
			e.setFromXML(s);
			return e;
		} catch (OutOfBoundsException err) {
			throw InvalidArgumentException("Unexpected end-of-string!");
		}
	}

	Entity::Entity() : entityId() {
		entityIdEncrypted = "";
		entityTypeName = "";
		entityVersion = "";
		instanceVersion = "";
	}

#ifndef WITHOUT_ACS
	Entity::Entity(IDLEntity &x) : entityId(string(x.entityId)) {
		entityIdEncrypted = string(x.entityIdEncrypted);
		entityTypeName = string(x.entityTypeName);
		entityVersion = string(x.entityVersion);
		instanceVersion = string(x.instanceVersion);
	}
#endif

	Entity::Entity(string id, string sEncrypted, string sTypeName,
			string sVersion, string sInstanceVersion) : entityId(id){
		entityIdEncrypted = sEncrypted;
		entityTypeName = sTypeName;
		entityVersion = sVersion;
		instanceVersion = sInstanceVersion;
	}

	bool Entity::operator == (const Entity& e) const {
		return 	entityId.getId() == e.entityId.getId() &&
				entityIdEncrypted == e.entityIdEncrypted &&
				entityTypeName == e.entityTypeName &&
				entityVersion == e.entityVersion &&
				instanceVersion == e.instanceVersion;
	}

	bool Entity::operator != (const Entity& e) const {
		return 	entityId.getId() != e.entityId.getId() ||
				entityIdEncrypted != e.entityIdEncrypted ||
				entityTypeName != e.entityTypeName ||
				entityVersion != e.entityVersion ||
				instanceVersion != e.instanceVersion;
	}

	string Entity::getXMLValue(string xml, string parm) const {
		string::size_type n = xml.find(parm+"=",0);
		if (n == string::npos)
			return "";
		string::size_type beg = xml.find("\"",n + parm.length());
		if (beg == string::npos)
			return "";
		beg++;
		string::size_type end = xml.find("\"",beg);
		if (end == string::npos)
			return "";
		return xml.substr(beg,(end - beg));
	}

	string Entity::validXML() const {
		// Check for any null values.
		string msg = "Null values detected in Entity " + entityId.getId();
		if (entityId.isNull() ||
			entityIdEncrypted.length() == 0 ||
			entityTypeName.length() == 0 ||
			entityVersion.length() == 0 ||
			instanceVersion.length() == 0)
			return msg;
		// Check the entityId for the correct format.
		return EntityId::validate(entityId.toString());
	}

	/**
	 * Return the values of this Entity as an XML-formated string.
	 * As an example, for the Main table in the ASDM, the toXML 
	 * method would give:
	 * <ul>
	 * <li>	&lt;Entity 
	 * <li>		entityId="uid://X0000000000000079/X00000000" 
	 * <li>		entityIdEncrypted="none" 
	 * <li>		entityTypeName="Main" 
	 * <li>		schemaVersion="1" 
	 * <li>		documentVersion="1"/&gt;
	 * </ul>
	 * 
	 * @return The values of this Entity as an XML-formated string.
	 * @throws IllegalStateException if the values of this Entity do not conform to the proper XML format.
	 */
	string Entity::toXML() const throw(InvalidDataException) {
		string msg = validXML();
		if (msg.length() != 0) 
			throw InvalidDataException(msg);
		string s = "<Entity entityId=\"" + entityId.toString() +
			"\" entityIdEncrypted=\"" + entityIdEncrypted +
			"\" entityTypeName=\"" + entityTypeName + 
			"\" schemaVersion=\"" + entityVersion +
			"\" documentVersion=\"" + instanceVersion + "\"/>";
		return s;
	}

#ifndef WITHOUT_ACS
	IDLEntity Entity::toIDLEntity() const {
		IDLEntity e;
		e.entityId = CORBA::string_dup(entityId.getId().c_str());
		e.entityIdEncrypted = CORBA::string_dup(entityIdEncrypted.c_str());
		e.entityTypeName = CORBA::string_dup(entityTypeName.c_str());
		e.entityVersion = CORBA::string_dup(entityVersion.c_str());
		e.instanceVersion = CORBA::string_dup(instanceVersion.c_str());
		return e;
	}
#endif

	void Entity::setFromXML(string xml) throw(InvalidArgumentException) {
		entityId.setId(getXMLValue(xml,"entityId"));
		entityIdEncrypted = getXMLValue(xml,"entityIdEncrypted");
		entityTypeName = getXMLValue(xml,"entityTypeName");
		entityVersion = getXMLValue(xml,"schemaVersion");
		instanceVersion = getXMLValue(xml,"documentVersion");
		if (entityIdEncrypted.length() == 0 ||
			entityTypeName.length() == 0 ||
			entityVersion.length() == 0 ||
			instanceVersion.length() == 0)
			throw InvalidArgumentException("Null values detected in Entity " + entityId.toString());
	}
		
	void Entity::toBin(EndianOSStream& eoss) {
		entityId.toBin(eoss);
		eoss.writeString(entityIdEncrypted);
		eoss.writeString(entityTypeName);
		eoss.writeString(entityVersion);
		eoss.writeString(instanceVersion);
	}
	
	 Entity Entity::fromBin(EndianISStream& eiss) {
		Entity entity;
		entity.setEntityId(EntityId(eiss.readString()));
		entity.setEntityIdEncrypted(eiss.readString());
		entity.setEntityTypeName(eiss.readString());
		entity.setEntityVersion(eiss.readString());
		entity.setInstanceVersion(eiss.readString());
		return entity;
	}

} // End namespace asdm
