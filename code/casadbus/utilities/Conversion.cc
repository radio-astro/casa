//# Conversion.cc: conversion utilities for DBus values in C++
//# Copyright (C) 2009
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
//# $Id: $
#include <casadbus/utilities/Conversion.h>


namespace casa {
    namespace dbus {

#define LOCAL_FILE_DEBUG 0

#define SET_VARIANT(VARTYPE,RECFUNC)					\
    {									\
    DBus::Variant v;							\
    DBus::MessageIter vi = v.writer( );					\
    vi.append_ ## VARTYPE (theRec. RECFUNC );				\
    transcribedRec->insert(std::make_pair(theRec.name(i).c_str(),v));	\
    }

	variant toVariant( const DBus::Variant &src ) {
#if LOCAL_FILE_DEBUG
	    std::cout << "****************************** " << src.signature( ) << " *****************************a" << std::endl;
#endif
	    if ( src.signature( ) == "s" ) {
		return variant( (src.operator std::string( )) );
	    } else if ( src.signature( ) == "i" ) {
		return variant( (src.operator int( )) );
	    } else if ( src.signature( ) == "u") {
		return variant( (double) (src.operator unsigned( )) );
	    } else if ( src.signature( ) == "d" ) {
		return variant( (src.operator double( )) );
	    } else if ( src.signature( ) == "b" ) {
		return variant( (src.operator bool( )) );
	    } else if ( src.signature( ) == "a{sv}" ) {
		DBus::MessageIter ri = src.reader( );
		std::map<std::string,DBus::Variant> vm;
		::operator >>(ri,vm);
		record rec;
		for ( std::map<std::string,DBus::Variant>::iterator iter = vm.begin( );
		      iter != vm.end( ); ++iter ) {
		    rec.insert(iter->first,toVariant(iter->second));
		}
		return variant(rec);
	    }
	    return variant( );
	}

	static void fromVariant( ::DBus::MessageIter &vi, const variant &src ) {
	    switch ( src.type() ) {
	    case variant::INT:
		::operator <<(vi,src.getInt( ));
		break;
	    case variant::BOOL:
		::operator <<(vi,src.getBool( ));
		break;
	    case variant::STRING:
		::operator <<(vi,src.getString( ));
		break;
	    case variant::DOUBLE:
		::operator <<(vi,src.getDouble( ));
		break;
	    case variant::BOOLVEC:
		::operator <<(vi,src.getBoolVec( ));
		break;
	    case variant::INTVEC:
		::operator <<(vi,src.getIntVec( ));
		break;
	    case variant::DOUBLEVEC:
		::operator <<(vi,src.getDoubleVec( ));
		break;
	    case variant::STRINGVEC:
		::operator <<(vi,src.getStringVec( ));
		break;
	    case variant::RECORD:
		{   const record &rec = src.getRecord( );
		    ::DBus::MessageIter ait = vi.new_array("{sv}");
		    for ( record::const_iterator rit = rec.begin( ); rit != rec.end( ); ++rit ) {
		        ::DBus::MessageIter eit = ait.new_dict_entry( );
			::operator <<(eit,rit->first);
			::DBus::MessageIter val = eit.new_variant( rit->second.sig() );
			fromVariant( val, rit->second );
			eit.close_container(val);
			ait.close_container(eit);
		    }
		    vi.close_container(ait);
		    break;
		}
	    default:
	        break;
	    }
	}

	std::map<std::string,variant> toStdMap( const std::map<std::string,DBus::Variant> &src ) {
		typedef std::map<std::string,variant> std_variant_map_t;
		std_variant_map_t result;
		typedef std::map<std::string,DBus::Variant> dbus_variant_map_t;
		for ( dbus_variant_map_t::const_iterator it=src.begin( ); it != src.end( ); ++it ) {
			result.insert(std_variant_map_t::value_type(it->first,toVariant(it->second)));
		}
		return result;
	}

      /************** causes problems with linux **************
       ************** without extra includes...  **************/
      /*static void debug_log(const char *format, ...) {
	    va_list args;
	    va_start(args, format);

	    fprintf(stderr, "dbus-c++: ");
	    vfprintf(stderr, format, args);
	    fprintf(stderr, "\n");

	    va_end(args);
        }
      */

	DBus::Variant fromVariant( const variant & src ) {
// 	    DBus::debug_log = debug_log;

#if LOCAL_FILnE_DEBUG
	    std::cout << "****************************** " << src.typeString( ) << " *****************************b" << std::endl;
#endif
	    DBus::Variant result;
	    ::DBus::MessageIter vi = result.writer();
	    fromVariant( vi, src );
	    return result;
	}

	std::map<std::string,DBus::Variant> fromStdMap( const std::map<std::string,variant> &src ) {
		typedef std::map<std::string,DBus::Variant> out_t;
		typedef std::map<std::string,variant> in_t;
		out_t result;
		for ( in_t::const_iterator it = src.begin(); it != src.end( ); ++it ) {
			DBus::Variant val;
			::DBus::MessageIter vi = val.writer();
			fromVariant( vi, it->second );
			result[it->first] = val;
		}
		return result;	
	}

	static void show( const variant &v, int level ) {
#if LOCAL_FILE_DEBUG
	    std::cout << "****************************** " << v.typeString( ) << " *****************************c" << std::endl;
#endif
	    switch ( v.type() ) {
	    case variant::INT:
		std::cout << "[int: " << v.getInt( ) << "]";
		if ( level == 0 ) std::cout << std::endl;
		break;
	    case variant::BOOL:
		std::cout << "[bool: " << v.getBool( ) << "]";
		if ( level == 0 ) std::cout << std::endl;
		break;
	    case variant::STRING:
		std::cout << "[string: '" << v.getString( ) << "']";
		if ( level == 0 ) std::cout << std::endl;
		break;
	    case variant::RECORD:
		{   const record &rec = v.getRecord( );
		    std::cout << "{ ";
		    for ( record::const_iterator iter = rec.begin(); iter != rec.end(); ++iter ) {
			if ( iter != rec.begin() ) std::cout << ", ";
			std::cout << iter->first << ": ";
			show( iter->second, 1 );
		    }
		    std::cout << " }";
		    if ( level == 0 ) std::cout << std::endl;
		}
		break;

	    default:
	        break;
	    }
	}

	void show( const variant &v ) { show(v,0); }

		Record toRecord(const std::map<std::string,DBus::Variant> &mapIn) {
			Record returnRecord;
    
			std::map<std::string,DBus::Variant>::const_iterator iter;
			for ( iter = mapIn.begin(); iter != mapIn.end(); ++iter) {
				DBus::MessageIter dbusMI = (*iter).second.reader();
      
				switch (dbusMI.type()) {
					case 'b':
						returnRecord.define(RecordFieldId((*iter).first),dbusMI.get_bool());
						break;
					case 'i': //DBus::Type::Type_INT32:
						returnRecord.define(RecordFieldId((*iter).first),dbusMI.get_int32());
						break;
					case 'd': //DBus::Type::TYPE_DOUBLE:
						returnRecord.define(RecordFieldId((*iter).first),dbusMI.get_double());
						break;
					case 's':
						returnRecord.define(RecordFieldId((*iter).first),dbusMI.get_string());
						break;
					default:
						std::cerr << "toRecord (" << __FILE__ << ":" << __LINE__ << ") unsupported type: " << (char) dbusMI.type() << std::endl;
				}
			}
			return returnRecord;
		}


		std::map<std::string,DBus::Variant> fromRecord( const Record &record ) {
			std::map<std::string,DBus::Variant> returnMap;
    
			for (unsigned int idx = 0; idx < record.nfields(); ++idx) {
				RecordFieldId id(idx);      
				switch (record.dataType(id)) {
					case TpBool:
						{
							DBus::Variant v;
							v.writer().append_bool(record.asBool(id));
							returnMap[record.name(id)] = v;
						}
						break;
					case TpInt:
						{
							DBus::Variant v;
							v.writer().append_int32(record.asInt(id));
							returnMap[record.name(id)] = v;
						}
						break;
					case TpFloat:
						{
							DBus::Variant v;
							v.writer().append_double(record.asFloat(id));
							returnMap[record.name(id)] = v;
						}
						break;
					case TpDouble:
						{
							DBus::Variant v;
							v.writer().append_double(record.asDouble(id));
							returnMap[record.name(id)] = v;
						}
						break;
					case TpString:
						{
							DBus::Variant v;
							v.writer().append_string(record.asString(id).c_str( ));
							returnMap[record.name(id)] = v;
						}
						break;
					default:
						std::cerr << "fromRecord (" << __FILE__ << ":" << __LINE__ << ") unsupported type" << std::endl;
				}
			}
			return returnMap;
		}
    }
}
