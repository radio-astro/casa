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
 * File EntityId.cpp
 */
//#define _POSIX_C_SOURCE

#include <regex.h>
#include <EntityId.h>
#include <OutOfBoundsException.h>
#include <InvalidArgumentException.h>
using asdm::OutOfBoundsException;
using asdm::InvalidArgumentException;

namespace asdm {

	EntityId EntityId::getEntityId(StringTokenizer &t) 
		throw (InvalidArgumentException) {
		try {
			string value = t.nextToken();
			return EntityId (value);
		} catch (OutOfBoundsException err) {
			throw InvalidArgumentException("Unexpected end-of-string!");
		}
	}

	/**
	 * Returns a null string if the string x contains a valid
	 * entity-id.  Otherwise, the string contains the error message.
	 */
	string EntityId::validate(string x) {
		string msg = "Invalid format for EntityId: " + x;
		string result = "";
		
		// Check the entityId for the correct format.
		 char * rexp = "^uid://X[a-fA-F0-9]\\+/X[a-fA-F0-9]\\+\\(/X[a-fA-F0-9]\\+\\)\\?$";
		 regex_t preg;
		 regcomp(&preg, rexp, REG_NOSUB);
		 if ( regexec(&preg, x.c_str(), 0, 0, 0) )
		 	result =  msg;
		 	
		 regfree(&preg);
		
//		if (x.length() == 0 || x.length() != 33 || 
//				(x.substr(0,7) != "uid://X") ||
//				x.at(23) != '/' || x.at(24) != 'X')
//			return msg;
//		for (int i = 7; i < 23; ++i) {
//			if (!((x.at(i) >= '0' && x.at(i) <= '9') ||
//					(x.at(i) >= 'a' && x.at(i) <= 'f')))
//				return msg;
//		}
//		for (int i = 25; i < 33; ++i) {
//			if (!((x.at(i) >= '0' && x.at(i) <= '9') ||
//					(x.at(i) >= 'a' && x.at(i) <= 'f')))
//				return msg;
//		}
		 return string("");		
	}

	EntityId::EntityId(const string &id) throw (InvalidArgumentException) {
		string msg = validate(id);
		if (msg.length() != 0)
			throw InvalidArgumentException(msg);
		this->id = id;		
	}

#ifndef WITHOUT_ACS
	EntityId::EntityId(IDLEntityId &x) throw (InvalidArgumentException) {
		string tmp(x.value);
		string msg = validate(tmp);
		if (msg.length() != 0)
			throw InvalidArgumentException(msg);
		this->id = tmp;		
	}
#endif
	
	void EntityId::toBin(EndianOSStream& eoss) {
		eoss.writeString(id);
	}
	
	EntityId EntityId::fromBin(EndianISStream& eiss) {
		return EntityId(eiss.readString());		
	}
} // End namespace asdm
