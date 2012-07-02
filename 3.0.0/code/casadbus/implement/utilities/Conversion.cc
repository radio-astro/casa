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
#include <dbus-c++/dbus.h>


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
		}
	    }
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
	    }
	}

	void show( const variant &v ) { show(v,0); }

	std::map<std::string, DBus::Variant> *fromRecord( const Record &theRec ) {
	    std::map<std::string, DBus::Variant> *transcribedRec = new std::map<std::string, DBus::Variant>( );

	    DBus::Variant result;
	    if ( 0 ) {
	      DBus::MessageIter resulti = result.writer( );
	      const std::string resultsig = "{" + DBus::type<std::string>::sig() + DBus::type<DBus::Variant>::sig() + "}";
	      DBus::MessageIter mapi = resulti.new_array(resultsig.c_str());

	      DBus::MessageIter dicti = mapi.new_dict_entry( );
	      dicti.append_string("vector");
	      std::string arraysig   = DBus::type<int>::sig( );
	      DBus::MessageIter varianti = dicti.new_variant(arraysig.c_str( ));
	      varianti.append_int32( 199 );
	      dicti.close_container(varianti);
	      mapi.close_container(dicti);

	      dicti = mapi.new_dict_entry( );
	      dicti.append_string("shape");
	      varianti = dicti.new_variant(arraysig.c_str( ));
	      varianti.append_int32( 200 );
	      dicti.close_container(varianti);
	      mapi.close_container(dicti);

	      resulti.close_container(mapi);
	    } else {
	      DBus::MessageIter resulti = result.writer( );
	      DBus::MessageIter structi = resulti.new_struct( );
	      std::string arraysig   = DBus::type<int>::sig( );
	      DBus::MessageIter arrayi = structi.new_array(arraysig.c_str());
	      arrayi.append_int32( 199 );
	      arrayi.append_int32( 299 );
	      structi.close_container(arrayi);
	      std::string shapesig = DBus::type<int>::sig( );
	      DBus::MessageIter shapei = structi.new_array(shapesig.c_str());
	      shapei.append_int32( 2 );
	      shapei.append_int32( 1 );
	      structi.close_container(shapei);
	      resulti.close_container(structi);
	    }

	    transcribedRec->insert(std::make_pair("one is the lonelyist" ,result));
	    return transcribedRec;
	}


#if 0
	    std::map<std::string, DBus::Variant> *transcribedRec = new std::map<std::string, DBus::Variant>( );
	    fprintf( stderr, "\t\t\t\there #1\n");
	    for(uInt i=0; i<theRec.nfields(); i++){
		// std::cerr << theRec.name(i) << " " << theRec.dataType(i) << std::endl;
		switch(theRec.dataType(i)){
		case TpBool :
		    SET_VARIANT(bool,asBool(i))
		    break;
		case TpChar :
		case TpUChar :
		    SET_VARIANT(int32,asuChar(i))
		    break;
		case TpShort :
		case TpUShort :
		    SET_VARIANT(int32,asShort(i))
		    break;
		case TpInt :
		    SET_VARIANT(int32,asInt(i))
		    break;
		case TpUInt :
		    SET_VARIANT(uint32,asuInt(i))
		    break;
		case TpFloat :
		    SET_VARIANT(double,asFloat(i))
		    break;
		case TpDouble :
		    SET_VARIANT(double,asDouble(i))
		    break;
		case TpTable :
		case TpString :
		    SET_VARIANT(string,asString(i).c_str( ))
		    break;
		case TpArrayBool :
		    {
		    fprintf( stderr, "\t\t\t\there #2\n");
		    Array<bool> tmpArray = theRec.asArrayBool(i);
		    Vector<int> tmpShape = (tmpArray.shape()).asVector();
		    //std::cerr << "Vector Shape " << tmpShape << std::endl;
		    DBus::Variant result;
		    DBus::MessageIter resulti = result.writer( );
		    const std::string resultsig = "{" + DBus::type<std::string>::sig() + DBus::type<DBus::Variant>::sig() + "}";


		    fprintf( stderr, "\t\t\t\there #3 - %s\n", resultsig.c_str());
		    DBus::MessageIter mapi = resulti.new_array(resultsig.c_str());
		    DBus::MessageIter dicti = mapi.new_dict_entry( );
		    fprintf( stderr, "\t\t\t\there #4 - %s\n", resultsig.c_str());
		    fflush(stderr);
		    dicti.append_string("vector");
// 		    ::operator <<( dicti, std::string("vector") );							// c++ is crappy
		    fprintf( stderr, "\t\t\t\there #5\n");
		    fflush(stderr);
		    const std::string arraysig   = DBus::type<std::vector<int> >::sig( );
		    DBus::MessageIter varianti = dicti.new_variant(arraysig.c_str( ));
		    fprintf( stderr, "\t\t\t\there #6 - %s\n", arraysig.c_str());
		    fflush(stderr);
		    DBus::MessageIter arrayi = varianti.new_array(arraysig.c_str());
		    fprintf( stderr, "\t\t\t\there #7 - %s\n", arraysig.c_str());
		    fflush(stderr);
		    const std::string elementsig = DBus::type<bool>::sig( );
		    fprintf( stderr, "\t\t\t\there #7a - %s\n", elementsig.c_str());
		    fflush(stderr);
		    bool deleteIt;
		    fprintf( stderr, "\t\t\t\there #8\n");
		    fflush(stderr);
		    const bool *vec = tmpArray.getStorage(deleteIt);
		    for ( int x = 0; x < tmpArray.nelements(); ++x ) {
			arrayi.append_int32( vec[x] );
// 			::operator <<( arrayi, (int) vec[x] );
		    }
		    fprintf( stderr, "\t\t\t\there #9\n");
		    tmpArray.freeStorage( vec, deleteIt );
		    varianti.close_container(arrayi);
		    dicti.close_container(varianti);
		    fprintf( stderr, "\t\t\t\there #10\n");
		    mapi.close_container(dicti);
		    dicti = mapi.new_dict_entry( );
		    dicti.append_string( "shape" );
		    fprintf( stderr, "\t\t\t\there #11\n");
		    const std::string shapesig   = DBus::type<std::vector<int> >::sig( );
		    varianti = dicti.new_variant(shapesig.c_str());
		    DBus::MessageIter shapei = varianti.new_array(shapesig.c_str());
		    const int *shape = tmpShape.getStorage(deleteIt);
		    fprintf( stderr, "\t\t\t\there #12\n");
		    for ( int x = 0; x < tmpShape.nelements(); ++x ) {
			::operator <<( shapei, shape[x] );
		    }
		    fprintf( stderr, "\t\t\t\there #13\n");
		    tmpShape.freeStorage( shape, deleteIt );
		    fprintf( stderr, "\t\t\t\there #14\n");
		    varianti.close_container(shapei);
		    dicti.close_container(varianti);
		    fprintf( stderr, "\t\t\t\there #15\n");
		    mapi.close_container(dicti);
		    resulti.close_container(mapi);
		    transcribedRec->insert(std::make_pair(theRec.name(i).c_str(),result));
		    }
		    break;
#endif
#if 0
		case TpArrayChar :
		case TpArrayUChar :
		case TpArrayShort :
		case TpArrayUShort :
		case TpArrayInt :
		case TpArrayUInt :
		    {
		    Array<Int> tmpArray = theRec.asArrayInt(i);
		    Vector<Int> tmpShape = (tmpArray.shape()).asVector();
		    std::vector<Int> vecShape;
		    tmpShape.tovector(vecShape);
		    std::vector<Int> tmpVec;
		    tmpArray.tovector(tmpVec);
		    transcribedRec->insert(theRec.name(i).c_str(), variant(tmpVec, vecShape));
		    }
		    break;
		case TpArrayFloat :
		    {
		    Array<Float> tmpArray = theRec.asArrayFloat(i);
		    Vector<Int> tmpShape = (tmpArray.shape()).asVector();
		    //std::cerr << "Vector Shape " << tmpShape << std::endl;
		    std::vector<Int> vecShape;
		    tmpShape.tovector(vecShape);
		    std::vector<Float> tmpVec;
		    tmpArray.tovector(tmpVec);
		    std::vector<Double> dtmpVec(tmpVec.size());
		    for(unsigned int j=0;j<tmpVec.size();j++)
			dtmpVec[j] = tmpVec[j];
		    transcribedRec->insert(theRec.name(i).c_str(), variant(dtmpVec, vecShape));
		    }
		    break;
		case TpArrayDouble :
		    {
		    Array<Double> tmpArray = theRec.asArrayDouble(i);
		    Vector<Int> tmpShape = (tmpArray.shape()).asVector();
		    //std::cerr << "Vector Shape " << tmpShape << std::endl;
		    std::vector<Int> vecShape;
		    tmpShape.tovector(vecShape);
		    std::vector<Double> tmpVec;
		    tmpArray.tovector(tmpVec);
		    transcribedRec->insert(theRec.name(i).c_str(), variant(tmpVec, vecShape));
		    }
		    break;
		case TpArrayString :
		    {
		    Array<String> tmpArray = theRec.asArrayString(i);
		    Vector<Int> tmpShape = (tmpArray.shape()).asVector();
		    std::vector<Int> vecShape;
		    tmpShape.tovector(vecShape);
		    std::vector<casa::String> tmpVec;
		    tmpArray.tovector(tmpVec);
		    std::vector<std::string> dtmpVec(tmpVec.size());
		    for(unsigned int j=0;j<tmpVec.size();j++)
			dtmpVec[j] = tmpVec[j].c_str();
		    transcribedRec->insert(theRec.name(i).c_str(), variant(dtmpVec, vecShape));
		    }
		    break;
		case TpRecord :
		    {
		    //std::cerr << "casa::fromRecord is Record" << std::endl;
		    record *dummy = fromRecord(theRec.asRecord(i));
		    variant dum2(dummy);
		    transcribedRec->insert(theRec.name(i).c_str(), dum2);
		    }
		    break;
		case TpComplex :
		    std::cerr << "casa::dbus::fromRecord TpComplex, not yet supported" << std::endl;
		    break;
		case TpDComplex :
		    std::cerr << "casa::dbus::fromRecord TpDComplex, not yet supported" << std::endl;
		    break;
		case TpArrayComplex :
		    std::cerr << "casa::dbus::fromRecord TpArrayComplex, not yet supported" << std::endl;
		    break;
		case TpArrayDComplex :
		    std::cerr << "casa::dbus::fromRecord TpArrayDComplex, not yet supported" << std::endl;
		    break;
		case TpOther :
		    std::cerr << "casa::dbus::fromRecord TpOther, oops" << std::endl;
		    break;
		case TpQuantity :
		    std::cerr << "casa::dbus::fromRecord TpQuantity, oops" << std::endl;
		    break;
		case TpArrayQuantity :
		    std::cerr << "casa::dbus::fromRecord TpArrayQuantity, oops" << std::endl;
		    break;
		default :
		    std::cerr << "casa::dbus::fromRecord unknown type, oops" << std::endl;
		    break;
		}
	    }
	    return transcribedRec;
	}
#endif
#if 0
	Record *toRecord( const std::map<std::string, DBus::Variant> &map ) {
	    Record *transcribedRec = new Record();
	    casac::rec_map::const_iterator rec_end = theRec.end();
	    for(::casac::rec_map::const_iterator rec_it = theRec.begin(); rec_it != rec_end; ++rec_it){
		//std::cerr << (*rec_it).first << " type is: " << (*rec_it).second.type() << std::endl;
		switch((*rec_it).second.type()){
		case ::casac::variant::RECORD :
		    {
		    Record *tmpRecord = toRecord((*rec_it).second.getRecord());
		    transcribedRec->defineRecord(RecordFieldId((*rec_it).first), *tmpRecord);
		    delete tmpRecord;  // Make sure it's a deep copy
		    }
		    break;
		case ::casac::variant::BOOL :
		    {transcribedRec->define(RecordFieldId((*rec_it).first), (*rec_it).second.toBool());}
		    break;
		case ::casac::variant::INT :
		    {transcribedRec->define(RecordFieldId((*rec_it).first), Int((*rec_it).second.getInt()));}
		    break;
		case ::casac::variant::DOUBLE :
		    transcribedRec->define(RecordFieldId((*rec_it).first), (*rec_it).second.getDouble());
		    break;
		case ::casac::variant::COMPLEX :
		    transcribedRec->define(RecordFieldId((*rec_it).first), (*rec_it).second.getComplex());
		    break;
		case ::casac::variant::STRING :
		    transcribedRec->define(RecordFieldId((*rec_it).first), (*rec_it).second.getString());
		    break;
		case ::casac::variant::BOOLVEC :
		    {
		    Vector<Int> shapeVec((*rec_it).second.arrayshape());
		    Vector<Bool>boolVec((*rec_it).second.getBoolVec());
		    IPosition tshape(shapeVec);
		    Array<Bool> boolArr(tshape);
		    int i(0);
		    Array<Bool>::iterator boolArrend = boolArr.end();
		    for(Array<Bool>::iterator iter = boolArr.begin(); iter != boolArrend; ++iter)
			*iter = boolVec[i++];
		    transcribedRec->define(RecordFieldId((*rec_it).first), boolArr);
		    }
		    break;
		case ::casac::variant::INTVEC :
		    {
		    Vector<Int> shapeVec((*rec_it).second.arrayshape());
		    Vector<Int> intVec((*rec_it).second.getIntVec());
		    IPosition tshape(shapeVec);
		    Array<Int> intArr(tshape);
		    int i(0);
		    Array<Int>::iterator intArrend = intArr.end();
		    for(Array<Int>::iterator iter = intArr.begin(); iter != intArrend; ++iter)
			*iter = intVec[i++];
		    transcribedRec->define(RecordFieldId((*rec_it).first), intArr);
		    }
		    break;
		case ::casac::variant::DOUBLEVEC :
		    {
		    Vector<Double> doubleVec((*rec_it).second.getDoubleVec());
		    Vector<Int> shapeVec((*rec_it).second.arrayshape());
		    IPosition tshape(shapeVec);
		    Array<Double> doubleArr(tshape);
		    int i(0);
		    Array<Double>::iterator doubleArrEnd = doubleArr.end();
		    for(Array<Double>::iterator iter = doubleArr.begin(); iter != doubleArrEnd; ++iter)
			*iter = doubleVec[i++];
		    transcribedRec->define(RecordFieldId((*rec_it).first), doubleArr);
		    }
		    break;
		case ::casac::variant::COMPLEXVEC :
		    {
		    Vector<DComplex> complexVec((*rec_it).second.getComplexVec());
		    Vector<Int> shapeVec((*rec_it).second.arrayshape());
		    IPosition tshape(shapeVec);
		    Array<DComplex> complexArr(tshape);
		    Array<DComplex>::iterator complexArrEnd = complexArr.end();
		    int i(0);
		    for(Array<DComplex>::iterator iter = complexArr.begin(); iter != complexArrEnd; ++iter)
			*iter = complexVec[i++];
		    transcribedRec->define(RecordFieldId((*rec_it).first), complexArr);
		    }
		    break;
		case ::casac::variant::STRINGVEC :
		    {
		    Vector<Int> shapeVec((*rec_it).second.arrayshape());
		    std::vector<std::string> tmp = (*rec_it).second.getStringVec();
		    const Array<String> &stringArr(toVectorString(tmp));
		    //stringArr.reform(IPosition(shapeVec));
		    transcribedRec->define(RecordFieldId((*rec_it).first), stringArr);
		    //transcribedRec->define((*rec_it).first, Vector<String>((*rec_it).second.getStringVec()));
		    }
		    break;
		default :
		    std::cerr << "Unknown type: " << (*rec_it).second.type() << std::endl;
		    break;
		}
	    }
	    return transcribedRec;
	}
#endif
    }
}
