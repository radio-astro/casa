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
 struct enum_set_traits<PositionMethod> : public enum_set_traiter<PositionMethod,2,PositionMethodMod::PHASE_FITTING> {};

template<>
class enum_map_traits<PositionMethod,void> : public enum_map_traiter<PositionMethod,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<PositionMethod,EnumPar<void> >
     (PositionMethodMod::DELAY_FITTING,ep((int)PositionMethodMod::DELAY_FITTING,"DELAY_FITTING","Delays are measured for each source; the delays are used for fitting antenna position errors.")));
    m_.insert(pair<PositionMethod,EnumPar<void> >
     (PositionMethodMod::PHASE_FITTING,ep((int)PositionMethodMod::PHASE_FITTING,"PHASE_FITTING","Phases are measured for each source; these phases are used to fit antenna position errors.")));
    return true;
  }
  static map<PositionMethod,EnumPar<void> > m_;
};
#define _POSITIONMETHOD_HH
#endif
