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


#if     !defined(_FIELDCODE_H)

#include <CFieldCode.h>
#define _FIELDCODE_H
#endif 

#if     !defined(_FIELDCODE_HH)

#include "Enum.hpp"

using namespace FieldCodeMod;

template<>
 struct enum_set_traits<FieldCodeMod::FieldCode> : public enum_set_traiter<FieldCodeMod::FieldCode,1,FieldCodeMod::NONE> {};

template<>
class enum_map_traits<FieldCodeMod::FieldCode,void> : public enum_map_traiter<FieldCodeMod::FieldCode,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<FieldCodeMod::FieldCode,EnumPar<void> >
     (FieldCodeMod::NONE,ep((int)FieldCodeMod::NONE,"NONE","un-documented")));
    return true;
  }
  static map<FieldCodeMod::FieldCode,EnumPar<void> > m_;
};
#define _FIELDCODE_HH
#endif
