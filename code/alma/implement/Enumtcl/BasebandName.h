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
 struct enum_set_traits<BasebandName> : public enum_set_traiter<BasebandName,10,BasebandNameMod::BB_ALL> {};

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
     (BasebandNameMod::NOBB,ep((int)BasebandNameMod::NOBB,"NOBB","un-documented")));
    m_.insert(pair<BasebandName,EnumPar<void> >
     (BasebandNameMod::BB_1,ep((int)BasebandNameMod::BB_1,"BB_1","un-documented")));
    m_.insert(pair<BasebandName,EnumPar<void> >
     (BasebandNameMod::BB_2,ep((int)BasebandNameMod::BB_2,"BB_2","un-documented")));
    m_.insert(pair<BasebandName,EnumPar<void> >
     (BasebandNameMod::BB_3,ep((int)BasebandNameMod::BB_3,"BB_3","un-documented")));
    m_.insert(pair<BasebandName,EnumPar<void> >
     (BasebandNameMod::BB_4,ep((int)BasebandNameMod::BB_4,"BB_4","un-documented")));
    m_.insert(pair<BasebandName,EnumPar<void> >
     (BasebandNameMod::BB_5,ep((int)BasebandNameMod::BB_5,"BB_5","un-documented")));
    m_.insert(pair<BasebandName,EnumPar<void> >
     (BasebandNameMod::BB_6,ep((int)BasebandNameMod::BB_6,"BB_6","un-documented")));
    m_.insert(pair<BasebandName,EnumPar<void> >
     (BasebandNameMod::BB_7,ep((int)BasebandNameMod::BB_7,"BB_7","un-documented")));
    m_.insert(pair<BasebandName,EnumPar<void> >
     (BasebandNameMod::BB_8,ep((int)BasebandNameMod::BB_8,"BB_8","un-documented")));
    m_.insert(pair<BasebandName,EnumPar<void> >
     (BasebandNameMod::BB_ALL,ep((int)BasebandNameMod::BB_ALL,"BB_ALL","un-documented")));
    return true;
  }
  static map<BasebandName,EnumPar<void> > m_;
};
#define _BASEBANDNAME_HH
#endif
