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
 * File Tag.cpp
 */

#include <Tag.h>
#include <OutOfBoundsException.h>
#include <InvalidArgumentException.h>
using asdm::OutOfBoundsException;
using asdm::InvalidArgumentException;

using namespace std;

namespace asdm {	

Tag::Tag() : tag(""), type(0) { }
Tag::Tag(unsigned int i) {
	tag =  Integer::toString(i);
	type = (TagType *) TagType::NoType;	
}

Tag::Tag(unsigned int i, const TagType* t) {
	tag =  Integer::toString(i);	
	type = (TagType*) t;		
}

Tag::Tag(const Tag &t) : tag(t.tag), type(t.type) { }

// Tag destructor
Tag::~Tag() { }

Tag& Tag::operator = ( const Tag &x ) {
	tag = x.tag;
	type = x.type;
	return *this;
}

bool Tag::operator == (const Tag &x) const {
	return tag == x.tag && type == x.type;	
}

bool Tag::operator != (const Tag &x) const {
	return tag != x.tag || type != x.type;	
}

bool Tag::operator < (const Tag &x) const {
	return tag < x.tag;	
}

bool Tag::equals(const Tag &x) const {
	return tag == x.tag && type == x.type;
}

bool Tag::isNull() const {
	return (tag.length() == 0 || type == 0);	
}

string Tag::toString() const {
	if (isNull()) return "null_0";
	return type->toString()+"_"+tag;
}

#ifndef WITHOUT_ACS
Tag::Tag(IDLTag &x) {
	try {
		*this = Tag::parseTag(string(x.type_value));
	}
	catch (TagFormatException e) {
		// Let's ignore this exception.
	}
}

IDLTag Tag::toIDLTag() const {
	IDLTag x;
	x.value = CORBA::string_dup(tag.c_str());
	x.type_value = CORBA::string_dup(toString().c_str());
	return x;
}
#endif

string Tag::getTag() const {
	return tag;
}

unsigned int Tag::getTagValue() const {
	return (isNull() ? 0 : Integer::parseInt(tag));
} 

TagType* Tag::getTagType() {
	return type;
}

Tag Tag::parseTag (string s) {
		// An empty string is parsed into a null (or empty) Tag.
		if (s.size() == 0) return Tag();
		
		// Locate the type,value separaor '_'.
		string::size_type loc = s.find( "_", 0 );
		if (loc == string::npos)
			throw TagFormatException("Error: \"" + s + "\" cannot be parsed into a Tag");
		
		// Split s into two strings type and value.
		string stype = s.substr(0, loc);
		string svalue;
		if (loc < (s.size() - 1)) svalue = s.substr(loc+1, s.size() - loc);	
		
		// If the type part == "null" then return a null Tag.
		if (stype == "null")  return Tag();
		
		// Determine the Tag type.
		const TagType*  type = TagType::getTagType(stype);
		if (type == (TagType *) 0) 
			throw TagFormatException("Error: \"" + s + "\" has an invalid type part\"" + stype);
			
		// Determine the Tag value.
		int value = 0;
		try {
				value = Integer::parseInt(svalue);
		}
		catch (NumberFormatException e) {
			throw TagFormatException("Error: \"" + s + "\" has an invalid value part\"" + svalue); 
		}	
		return Tag(value, type);
	}

	void Tag:: toBin(EndianOSStream& eoss) {
		eoss.writeString(this->toString());
	} 
	
	void Tag::toBin(const vector<Tag>& tag, EndianOSStream& eoss) {
		eoss.writeInt((int) tag.size());
		for (unsigned int i = 0; i < tag.size(); i++) {
			eoss.writeString(tag.at(i).toString());
		}
	}
	
	Tag Tag::fromBin(EndianISStream& eiss) {
		string s = eiss.readString();
		return Tag::parseTag(s);		
	}
	
	 vector<Tag> Tag::from1DBin(EndianISStream& eiss) {
		int dim1 = eiss.readInt();
		
		vector<Tag> tag;
		for (int i = 0;  i < dim1; i++)
			tag.push_back(Tag::fromBin(eiss));
			
		return tag;
	}
	
} // End namespace asdm
