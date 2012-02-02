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


#if     !defined(_ACCUMMODE_H)

#include <CAccumMode.h>
#define _ACCUMMODE_H
#endif 

#if     !defined(_ACCUMMODE_HH)

#include "Enum.hpp"

using namespace AccumModeMod;

template<>
 struct enum_set_traits<AccumMode> : public enum_set_traiter<AccumMode,3,AccumModeMod::UNDEFINED> {};

template<>
class enum_map_traits<AccumMode,void> : public enum_map_traiter<AccumMode,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<AccumMode,EnumPar<void> >
     (AccumModeMod::FAST,ep((int)AccumModeMod::FAST,"FAST","un-documented")));
    m_.insert(pair<AccumMode,EnumPar<void> >
     (AccumModeMod::NORMAL,ep((int)AccumModeMod::NORMAL,"NORMAL","un-documented")));
    m_.insert(pair<AccumMode,EnumPar<void> >
     (AccumModeMod::UNDEFINED,ep((int)AccumModeMod::UNDEFINED,"UNDEFINED","un-documented")));
    return true;
  }
  static map<AccumMode,EnumPar<void> > m_;
};
#define _ACCUMMODE_HH
#endif
