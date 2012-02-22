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


#if     !defined(_WVRMETHOD_H)

#include <CWVRMethod.h>
#define _WVRMETHOD_H
#endif 

#if     !defined(_WVRMETHOD_HH)

#include "Enum.hpp"

using namespace WVRMethodMod;

template<>
 struct enum_set_traits<WVRMethod> : public enum_set_traiter<WVRMethod,2,WVRMethodMod::EMPIRICAL> {};

template<>
class enum_map_traits<WVRMethod,void> : public enum_map_traiter<WVRMethod,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<WVRMethod,EnumPar<void> >
     (WVRMethodMod::ATM_MODEL,ep((int)WVRMethodMod::ATM_MODEL,"ATM_MODEL","un-documented")));
    m_.insert(pair<WVRMethod,EnumPar<void> >
     (WVRMethodMod::EMPIRICAL,ep((int)WVRMethodMod::EMPIRICAL,"EMPIRICAL","un-documented")));
    return true;
  }
  static map<WVRMethod,EnumPar<void> > m_;
};
#define _WVRMETHOD_HH
#endif
