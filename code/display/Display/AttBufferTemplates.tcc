//# AttBufferTemplates.cc: AttributeBuffer member templates
//# Copyright (C) 2000,2001
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

#include <casa/Exceptions/Error.h>
#include <casa/Arrays/Vector.h>
#include <casa/Utilities/DataType.h>
#include <display/Display/Attribute.h>
#include <display/Display/AttVal.h>
#include <display/Display/AttributeBuffer.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	template <class T>
	casacore::Bool AttributeBuffer::getValue(const casacore::String &name, T &value) const {
		casacore::Vector<T> tmp;
		if (getValue(name, tmp)) {
			value = tmp(0);
			return true;
		} else {
			return false;
		}
	}

	template <class T>
	casacore::Bool AttributeBuffer::getValue(const casacore::String &name, casacore::Vector<T> &value) const {
		casacore::Vector<T>* tmp = NULL;
		casacore::DataType dtype = whatType(tmp);
		/*AttValue::ValueType valType;
		switch(dtype) {
		case TpArrayUInt:
			valType = AttValue::AtuInt;
			break;
		case TpArrayInt:
			valType = AttValue::AtInt;
			break;
		case TpArrayBool:
			valType = AttValue::AtBool;
			break;
		case TpArrayFloat:
			valType = AttValue::AtFloat;
			break;
		case TpArrayDouble:
			valType = AttValue::AtDouble;
			break;
		case TpArrayString:
			valType = AttValue::AtString;
			break;
		case TpArrayQuantity:
			valType = AttValue::AtQuantity;
			break;
		default:
			throw(casacore::AipsError("Invalid template for AttributeBuffer::getValue"));
		}*/
		if ( dtype != casacore::TpArrayUInt && dtype != casacore::TpArrayInt &&
		     dtype != casacore::TpArrayBool &&
			 dtype != casacore::TpArrayFloat && dtype != casacore::TpArrayDouble &&
			dtype != casacore::TpArrayString && dtype != casacore::TpArrayQuantity ){
			throw(casacore::AipsError("Invalid template for AttributeBuffer::getValue"));			
		}
		for (casacore::uInt i = 0; i < attributes.nelements(); i++) {
			if (name == (attributes[i])->getName()) {
				AttributeValue<T>* tmp =
				    dynamic_cast<AttributeValue<T>* >(attributes[i]->getAttributeValue());
				if (tmp) {
					value = tmp->getValue();
					return true;
				} else {
					return false;
				}
			}
		}
		return false;
	}



} //# NAMESPACE CASA - END

