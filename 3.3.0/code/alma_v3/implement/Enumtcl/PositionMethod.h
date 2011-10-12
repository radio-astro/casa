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


#if     !defined(_POSITIONMETHOD_H)

#include <CPositionMethod.h>
#define _POSITIONMETHOD_H
#endif 

#if     !defined(_POSITIONMETHOD_HH)

#include "Enum.hpp"

using namespace PositionMethodMod;

template<>
 struct enum_set_traits<PositionMethodMod::PositionMethod> : public enum_set_traiter<PositionMethodMod::PositionMethod,2,PositionMethodMod::PHASE_FITTING> {};

template<>
class enum_map_traits<PositionMethodMod::PositionMethod,void> : public enum_map_traiter<PositionMethodMod::PositionMethod,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<PositionMethodMod::PositionMethod,EnumPar<void> >
     (PositionMethodMod::DELAY_FITTING,ep((int)PositionMethodMod::DELAY_FITTING,"DELAY_FITTING","un-documented")));
    m_.insert(pair<PositionMethodMod::PositionMethod,EnumPar<void> >
     (PositionMethodMod::PHASE_FITTING,ep((int)PositionMethodMod::PHASE_FITTING,"PHASE_FITTING","un-documented")));
    return true;
  }
  static map<PositionMethodMod::PositionMethod,EnumPar<void> > m_;
};
#define _POSITIONMETHOD_HH
#endif
