/*
 *
 * /////////////////////////////////////////////////////////////////
 * // WARNING!  DO NOT MODIFY THIS FILE!                          //
 * //  ---------------------------------------------------------  //
 * // | This is generated code using a C++ template function!   | //
 * // ! Do not modify this file.                                | //
 * // | Any changes will be lost when the file is re-generated. | //
 * //  ---------------------------------------------------------  //
 * /////////////////////////////////////////////////////////////////
 *
 */


#if     !defined(_PRIMITIVEDATATYPE_H)

#include <CPrimitiveDataType.h>
#define _PRIMITIVEDATATYPE_H
#endif 

#if     !defined(_PRIMITIVEDATATYPE_HH)

#include "Enum.hpp"

using namespace PrimitiveDataTypeMod;

template<>
 struct enum_set_traits<PrimitiveDataType> : public enum_set_traiter<PrimitiveDataType,11,PrimitiveDataTypeMod::STRING_TYPE> {};

template<>
class enum_map_traits<PrimitiveDataType,void> : public enum_map_traiter<PrimitiveDataType,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<PrimitiveDataType,EnumPar<void> >
     (PrimitiveDataTypeMod::BOOL_TYPE,ep((int)PrimitiveDataTypeMod::BOOL_TYPE,"BOOL_TYPE","")));
    m_.insert(pair<PrimitiveDataType,EnumPar<void> >
     (PrimitiveDataTypeMod::BYTE_TYPE,ep((int)PrimitiveDataTypeMod::BYTE_TYPE,"BYTE_TYPE","")));
    m_.insert(pair<PrimitiveDataType,EnumPar<void> >
     (PrimitiveDataTypeMod::SHORT_TYPE,ep((int)PrimitiveDataTypeMod::SHORT_TYPE,"SHORT_TYPE","")));
    m_.insert(pair<PrimitiveDataType,EnumPar<void> >
     (PrimitiveDataTypeMod::INT_TYPE,ep((int)PrimitiveDataTypeMod::INT_TYPE,"INT_TYPE","")));
    m_.insert(pair<PrimitiveDataType,EnumPar<void> >
     (PrimitiveDataTypeMod::LONGLONG_TYPE,ep((int)PrimitiveDataTypeMod::LONGLONG_TYPE,"LONGLONG_TYPE","")));
    m_.insert(pair<PrimitiveDataType,EnumPar<void> >
     (PrimitiveDataTypeMod::UNSIGNED_SHORT_TYPE,ep((int)PrimitiveDataTypeMod::UNSIGNED_SHORT_TYPE,"UNSIGNED_SHORT_TYPE","")));
    m_.insert(pair<PrimitiveDataType,EnumPar<void> >
     (PrimitiveDataTypeMod::UNSIGNED_INT_TYPE,ep((int)PrimitiveDataTypeMod::UNSIGNED_INT_TYPE,"UNSIGNED_INT_TYPE","")));
    m_.insert(pair<PrimitiveDataType,EnumPar<void> >
     (PrimitiveDataTypeMod::UNSIGNED_LONGLONG_TYPE,ep((int)PrimitiveDataTypeMod::UNSIGNED_LONGLONG_TYPE,"UNSIGNED_LONGLONG_TYPE","")));
    m_.insert(pair<PrimitiveDataType,EnumPar<void> >
     (PrimitiveDataTypeMod::FLOAT_TYPE,ep((int)PrimitiveDataTypeMod::FLOAT_TYPE,"FLOAT_TYPE","")));
    m_.insert(pair<PrimitiveDataType,EnumPar<void> >
     (PrimitiveDataTypeMod::DOUBLE_TYPE,ep((int)PrimitiveDataTypeMod::DOUBLE_TYPE,"DOUBLE_TYPE","")));
    m_.insert(pair<PrimitiveDataType,EnumPar<void> >
     (PrimitiveDataTypeMod::STRING_TYPE,ep((int)PrimitiveDataTypeMod::STRING_TYPE,"STRING_TYPE","")));
    return true;
  }
  static map<PrimitiveDataType,EnumPar<void> > m_;
};
#define _PRIMITIVEDATATYPE_HH
#endif
