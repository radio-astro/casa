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
 * File ConversionException.h
 */

#ifndef IDLConversionException_CLASS
#define IDLConversionException_CLASS

#include <string>
using std::string;

namespace enumerations {

/**
 * The IDLConversionException class represents an exception when 
 * an error occurs in converting in restoring an Enumeration constant  from its 
 * IDL representation. 
 */
	class IDLConversionException {

public:

	/**
	 * Create an exception.
	 * The constructor takes a string as a parameter, describing the cause of the exception.
	 * @param m The message associated to the exception.
	 */
	IDLConversionException(string m);

	 /**
	     * Return the name of the exception followed by its cause.
	     */
	string getMessage() const;

protected:

	string message;
};

inline IDLConversionException::IDLConversionException (string m) : 
	message(m) {
}

inline string IDLConversionException::getMessage() const {
	return "IDLConversionException : cannot convert from IDL. " + message;
}

} // End namespace asdm

#endif /* ConversionException_CLASS */
