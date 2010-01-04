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


#if     !defined(_BASEBANDNAME_H)

#include <CBasebandName.h>
#define _BASEBANDNAME_H
#endif 

#if     !defined(_BASEBANDNAME_HH)

#include "Enum.hpp"

using namespace BasebandNameMod;

template<>
 struct enum_set_traits<BasebandName> : public enum_set_traiter<BasebandName,8,BasebandNameMod::BB_8> {};

template<>
class enum_map_traits<BasebandName,void> : public enum_map_traiter<BasebandName,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<BasebandName,EnumPar<void> >
     (BasebandNameMod::BB_1,ep((int)BasebandNameMod::BB_1,"BB_1","Baseband one")));
    m_.insert(pair<BasebandName,EnumPar<void> >
     (BasebandNameMod::BB_2,ep((int)BasebandNameMod::BB_2,"BB_2","Baseband two")));
    m_.insert(pair<BasebandName,EnumPar<void> >
     (BasebandNameMod::BB_3,ep((int)BasebandNameMod::BB_3,"BB_3","Baseband three")));
    m_.insert(pair<BasebandName,EnumPar<void> >
     (BasebandNameMod::BB_4,ep((int)BasebandNameMod::BB_4,"BB_4","Baseband four")));
    m_.insert(pair<BasebandName,EnumPar<void> >
     (BasebandNameMod::BB_5,ep((int)BasebandNameMod::BB_5,"BB_5","Baseband five (not ALMA)")));
    m_.insert(pair<BasebandName,EnumPar<void> >
     (BasebandNameMod::BB_6,ep((int)BasebandNameMod::BB_6,"BB_6","Baseband six (not ALMA)")));
    m_.insert(pair<BasebandName,EnumPar<void> >
     (BasebandNameMod::BB_7,ep((int)BasebandNameMod::BB_7,"BB_7","Baseband seven (not ALMA)")));
    m_.insert(pair<BasebandName,EnumPar<void> >
     (BasebandNameMod::BB_8,ep((int)BasebandNameMod::BB_8,"BB_8","Baseband eight (not ALMA)")));
    return true;
  }
  static map<BasebandName,EnumPar<void> > m_;
};
#define _BASEBANDNAME_HH
#endif
