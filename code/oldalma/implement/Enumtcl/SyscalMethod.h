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


#if     !defined(_SYSCALMETHOD_H)

#include <CSyscalMethod.h>
#define _SYSCALMETHOD_H
#endif 

#if     !defined(_SYSCALMETHOD_HH)

#include "Enum.hpp"

using namespace SyscalMethodMod;

template<>
 struct enum_set_traits<SyscalMethod> : public enum_set_traiter<SyscalMethod,2,SyscalMethodMod::SKYDIP> {};

template<>
class enum_map_traits<SyscalMethod,void> : public enum_map_traiter<SyscalMethod,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<SyscalMethod,EnumPar<void> >
     (SyscalMethodMod::TEMPERATURE_SCALE,ep((int)SyscalMethodMod::TEMPERATURE_SCALE,"TEMPERATURE_SCALE","Use single direction data to compute ta* scale")));
    m_.insert(pair<SyscalMethod,EnumPar<void> >
     (SyscalMethodMod::SKYDIP,ep((int)SyscalMethodMod::SKYDIP,"SKYDIP","Use a skydip (observing the sky at various elevations) to get atmospheric opacity")));
    return true;
  }
  static map<SyscalMethod,EnumPar<void> > m_;
};
#define _SYSCALMETHOD_HH
#endif
